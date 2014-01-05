/*
 * This is a module which is used for rejecting packets.
 * Added support for customized reject packets (Jozsef Kadlecsik).
 * Added support for ICMP type-3-code-13 (Maciej Soltysiak). [RFC 1812]
 */

/* (C) 1999-2001 Paul `Rusty' Russell
 * (C) 2002-2004 Netfilter Core Team <coreteam@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/config.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/icmp.h>
#include <net/icmp.h>
#include <net/ip.h>
#include <net/tcp.h>
#include <net/route.h>
#include <net/dst.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_ipv4/ipt_REJECT.h>
#ifdef CONFIG_BRIDGE_NETFILTER
#include <linux/netfilter_bridge.h>
#endif

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Netfilter Core Team <coreteam@netfilter.org>");
MODULE_DESCRIPTION("iptables REJECT target module");

#if 0
#define DEBUGP printk
#else
#define DEBUGP(format, args...)
#endif

static inline struct rtable *route_reverse(struct sk_buff *skb, 
					   struct tcphdr *tcph, int hook)
{
	struct iphdr *iph = skb->nh.iph;
	struct dst_entry *odst;
	struct flowi fl = {};
	struct rtable *rt;

	/* We don't require ip forwarding to be enabled to be able to
	 * send a RST reply for bridged traffic. */
	if (hook != NF_IP_FORWARD
#ifdef CONFIG_BRIDGE_NETFILTER
	    || (skb->nf_bridge && skb->nf_bridge->mask & BRNF_BRIDGED)
#endif
	   ) {
		fl.nl_u.ip4_u.daddr = iph->saddr;
		if (hook == NF_IP_LOCAL_IN)
			fl.nl_u.ip4_u.saddr = iph->daddr;
		fl.nl_u.ip4_u.tos = RT_TOS(iph->tos);

		if (ip_route_output_key(&rt, &fl) != 0)
			return NULL;
	} else {
		/* non-local src, find valid iif to satisfy
		 * rp-filter when calling ip_route_input. */
		fl.nl_u.ip4_u.daddr = iph->daddr;
		if (ip_route_output_key(&rt, &fl) != 0)
			return NULL;

		odst = skb->dst;
		if (ip_route_input(skb, iph->saddr, iph->daddr,
		                   RT_TOS(iph->tos), rt->u.dst.dev) != 0) {
			dst_release(&rt->u.dst);
			return NULL;
		}
		dst_release(&rt->u.dst);
		rt = (struct rtable *)skb->dst;
		skb->dst = odst;

		fl.nl_u.ip4_u.daddr = iph->saddr;
		fl.nl_u.ip4_u.saddr = iph->daddr;
		fl.nl_u.ip4_u.tos = RT_TOS(iph->tos);
	}

	if (rt->u.dst.error) {
		dst_release(&rt->u.dst);
		return NULL;
	}

	fl.proto = IPPROTO_TCP;
	fl.fl_ip_sport = tcph->dest;
	fl.fl_ip_dport = tcph->source;

	xfrm_lookup((struct dst_entry **)&rt, &fl, NULL, 0);

	return rt;
}

/* Send RST reply */
static void send_reset(struct sk_buff *oldskb, int hook)
{
	struct sk_buff *nskb;
	struct iphdr *iph = oldskb->nh.iph;
	struct tcphdr _otcph, *oth, *tcph;
	struct rtable *rt;
	u_int16_t tmp_port;
	u_int32_t tmp_addr;
	unsigned int tcplen;
	int needs_ack;
	int hh_len;

	/* IP header checks: fragment. */
	if (oldskb->nh.iph->frag_off & htons(IP_OFFSET))
		return;

	oth = skb_header_pointer(oldskb, oldskb->nh.iph->ihl * 4,
				 sizeof(_otcph), &_otcph);
	if (oth == NULL)
 		return;

	/* No RST for RST. */
	if (oth->rst)
		return;

	/* Check checksum */
	tcplen = oldskb->len - iph->ihl * 4;
	if (((hook != NF_IP_LOCAL_IN && oldskb->ip_summed != CHECKSUM_HW) ||
	     (hook == NF_IP_LOCAL_IN &&
	      oldskb->ip_summed != CHECKSUM_UNNECESSARY)) &&
	    csum_tcpudp_magic(iph->saddr, iph->daddr, tcplen, IPPROTO_TCP,
	                      oldskb->ip_summed == CHECKSUM_HW ? oldskb->csum :
	                      skb_checksum(oldskb, iph->ihl * 4, tcplen, 0)))
		return;

	if ((rt = route_reverse(oldskb, oth, hook)) == NULL)
		return;

	hh_len = LL_RESERVED_SPACE(rt->u.dst.dev);

	/* We need a linear, writeable skb.  We also need to expand
	   headroom in case hh_len of incoming interface < hh_len of
	   outgoing interface */
	nskb = skb_copy_expand(oldskb, hh_len, skb_tailroom(oldskb),
			       GFP_ATOMIC);
	if (!nskb) {
		dst_release(&rt->u.dst);
		return;
	}

	dst_release(nskb->dst);
	nskb->dst = &rt->u.dst;

	/* This packet will not be the same as the other: clear nf fields */
	nf_reset(nskb);
	nskb->nfmark = 0;
#ifdef CONFIG_BRIDGE_NETFILTER
	nf_bridge_put(nskb->nf_bridge);
	nskb->nf_bridge = NULL;
#endif

	tcph = (struct tcphdr *)((u_int32_t*)nskb->nh.iph + nskb->nh.iph->ihl);

	/* Swap source and dest */
	tmp_addr = nskb->nh.iph->saddr;
	nskb->nh.iph->saddr = nskb->nh.iph->daddr;
	nskb->nh.iph->daddr = tmp_addr;
	tmp_port = tcph->source;
	tcph->source = tcph->dest;
	tcph->dest = tmp_port;

	/* Truncate to length (no data) */
	tcph->doff = sizeof(struct tcphdr)/4;
	skb_trim(nskb, nskb->nh.iph->ihl*4 + sizeof(struct tcphdr));
	nskb->nh.iph->tot_len = htons(nskb->len);

	if (tcph->ack) {
		needs_ack = 0;
		tcph->seq = oth->ack_seq;
		tcph->ack_seq = 0;
	} else {
		needs_ack = 1;
		tcph->ack_seq = htonl(ntohl(oth->seq) + oth->syn + oth->fin
				      + oldskb->len - oldskb->nh.iph->ihl*4
				      - (oth->doff<<2));
		tcph->seq = 0;
	}

	/* Reset flags */
	((u_int8_t *)tcph)[13] = 0;
	tcph->rst = 1;
	tcph->ack = needs_ack;

	tcph->window = 0;
	tcph->urg_ptr = 0;

	/* Adjust TCP checksum */
	tcph->check = 0;
	tcph->check = tcp_v4_check(tcph, sizeof(struct tcphdr),
				   nskb->nh.iph->saddr,
				   nskb->nh.iph->daddr,
				   csum_partial((char *)tcph,
						sizeof(struct tcphdr), 0));

	/* Adjust IP TTL, DF */
	nskb->nh.iph->ttl = dst_metric(nskb->dst, RTAX_HOPLIMIT);
	/* Set DF, id = 0 */
	nskb->nh.iph->frag_off = htons(IP_DF);
	nskb->nh.iph->id = 0;

	/* Adjust IP checksum */
	nskb->nh.iph->check = 0;
	nskb->nh.iph->check = ip_fast_csum((unsigned char *)nskb->nh.iph, 
					   nskb->nh.iph->ihl);

	/* "Never happens" */
	if (nskb->len > dst_mtu(nskb->dst))
		goto free_nskb;

	nf_ct_attach(nskb, oldskb);

	NF_HOOK(PF_INET, NF_IP_LOCAL_OUT, nskb, NULL, nskb->dst->dev,
		dst_output);
	return;

 free_nskb:
	kfree_skb(nskb);
}

static void nk_modifyPkt_for_reset(const struct sk_buff *skb)
{
	struct iphdr *iph = (skb)->nh.iph;
	struct tcphdr *tcph = (void *)iph + iph->ihl*4;
	unsigned char *data = (void *)tcph + tcph->doff*4;
	unsigned int datalen = (skb)->len - (iph->ihl*4) - (tcph->doff*4);
	typedef struct tcphdr tcphdr_t;
	// start to modify tcp header
	tcp_flag_word(tcph) = (TCP_FLAG_RST);
	tcph->doff = (sizeof(tcphdr_t)>>2);
	tcph->check = csum_tcpudp_magic(iph->saddr, iph->daddr, datalen, IPPROTO_TCP,csum_partial((char *)tcph,datalen,0));
	return;
}

static void nk_return_url_block(const struct sk_buff *skb)
{
	struct iphdr *iph = (skb)->nh.iph;
	struct tcphdr *tcph = (void *)iph + iph->ihl*4;
	unsigned char *data = (void *)tcph + tcph->doff*4;
	unsigned int datalen = (skb)->len - (iph->ihl*4) - (tcph->doff*4);
	struct sk_buff *mc=NULL;
	struct rtable *rt;
	struct flowi fl={ .oif = 0,
			.nl_u = { .ip4_u =
					{ .daddr = iph->saddr,
					.saddr = 0,
					.tos = 0 } },
			.proto = IPPROTO_TCP,
			.uli_u = { .ports =
					{ .sport = tcph->dest,
					.dport =tcph->source  } } };
	mc = skb_copy(skb,GFP_ATOMIC);
	mc->input_dev=NULL;
	mc->dev=NULL;
	mc->pkt_type = PACKET_HOST;
	if( __ip_route_output_key(&rt,&fl) )
	{
		kfree_skb(mc);
		DEBUGP("%s: __ip_route_output_key fail",__FUNCTION__);
		return;
	}
	mc->csum=0;
	mc->dst = &rt->u.dst;
	mc->dev = mc->dst->dev;

	// start to modify skb content
	typedef struct tcphdr tcphdr_t;
	static char block_info[500];
	static char block_htm[1000];
	char tmp[100];
	int	 tlen = 0;
	snprintf(block_info, sizeof(block_info),
		"<html>\n<head>\n  <title></title>\n"
		"</head>\n<body bgcolor=\"ffffff\">\n  <h2><h2>\n"
		"  <p>\n  This URLs or Page has been blocked. \n</body>\n</html>\n");
 
	sprintf(block_htm, "HTTP/1.1 200 OK\r\n");
	strcat(block_htm, "Content-type: text/html\r\n");
	strcat(block_htm, "Pragma: no cache\r\n");
	sprintf(tmp, "Content-length: %d\r\n", strlen(block_info));
	strcat(block_htm, tmp);
	strcat(block_htm, "Connection: close\r\n\r\n");
	strcat(block_htm, block_info);

	//header definition
	struct iphdr *new_iph = (mc)->nh.iph;
	struct tcphdr *new_tcph = (void *)new_iph + new_iph->ihl*4;
	unsigned char *new_data = (void *)new_tcph + new_tcph->doff*4;
	/* clear memory */
	memset((char *)new_data,0, datalen);
	/* fill ip header */
	iph->protocol = IPPROTO_TCP;
	new_iph->saddr = iph->daddr;
	new_iph->daddr = iph->saddr;
	new_iph->ttl = 127;
	new_iph->check =0;
	new_iph->check = ip_fast_csum((void *)new_iph, new_iph->ihl);
	/* fill tcp header */
	new_tcph->source = tcph->dest;
	new_tcph->dest = tcph->source;
	tcp_flag_word(new_tcph) = (TCP_FLAG_ACK | TCP_FLAG_PSH);
	if (tcp_flag_word(new_tcph) & TCP_FLAG_ACK)
		new_tcph->seq = tcph->ack_seq;
	else
		new_tcph->seq = htonl(1);
	new_tcph->doff = (sizeof(tcphdr_t)>>2);
	/* the ack should acknowledge all the packet, find the size */
	new_tcph->ack_seq = htonl(ntohl(tcph->seq) + datalen);
	memcpy((char *)new_tcph + sizeof(tcphdr_t), block_htm, strlen(block_htm));
	new_tcph->check = csum_tcpudp_magic(new_iph->saddr, new_iph->daddr, datalen, IPPROTO_TCP,csum_partial((char *)new_tcph,datalen,0));
	
	ip_finish_output(mc);
	nk_modifyPkt_for_reset(skb);

	return;
}

static inline void send_unreach(struct sk_buff *skb_in, int code)
{
	icmp_send(skb_in, ICMP_DEST_UNREACH, code, 0);
}	

static unsigned int reject(struct sk_buff **pskb,
			   const struct net_device *in,
			   const struct net_device *out,
			   unsigned int hooknum,
			   const void *targinfo,
			   void *userinfo)
{
	const struct ipt_reject_info *reject = targinfo;

	/* Our naive response construction doesn't deal with IP
           options, and probably shouldn't try. */
	if ((*pskb)->nh.iph->ihl<<2 != sizeof(struct iphdr))
		return NF_DROP;

	/* WARNING: This code causes reentry within iptables.
	   This means that the iptables jump stack is now crap.  We
	   must return an absolute verdict. --RR */
    	switch (reject->with) {
    	case IPT_ICMP_NET_UNREACHABLE:
    		send_unreach(*pskb, ICMP_NET_UNREACH);
    		break;
    	case IPT_ICMP_HOST_UNREACHABLE:
    		send_unreach(*pskb, ICMP_HOST_UNREACH);
    		break;
    	case IPT_ICMP_PROT_UNREACHABLE:
    		send_unreach(*pskb, ICMP_PROT_UNREACH);
    		break;
    	case IPT_ICMP_PORT_UNREACHABLE:
    		send_unreach(*pskb, ICMP_PORT_UNREACH);
    		break;
    	case IPT_ICMP_NET_PROHIBITED:
    		send_unreach(*pskb, ICMP_NET_ANO);
    		break;
	case IPT_ICMP_HOST_PROHIBITED:
    		send_unreach(*pskb, ICMP_HOST_ANO);
    		break;
    	case IPT_ICMP_ADMIN_PROHIBITED:
		send_unreach(*pskb, ICMP_PKT_FILTERED);
		break;
	case IPT_TCP_RESET:
		send_reset(*pskb, hooknum);
		break;
	case IPT_TCP_URLBLOCK:
		nk_return_url_block(*pskb);
		return NF_ACCEPT;
		break;
	case IPT_ICMP_ECHOREPLY:
		/* Doesn't happen. */
		break;
	}

	return NF_DROP;
}

static int check(const char *tablename,
		 const void *e_void,
		 void *targinfo,
		 unsigned int targinfosize,
		 unsigned int hook_mask)
{
 	const struct ipt_reject_info *rejinfo = targinfo;
	const struct ipt_entry *e = e_void;

 	if (targinfosize != IPT_ALIGN(sizeof(struct ipt_reject_info))) {
  		DEBUGP("REJECT: targinfosize %u != 0\n", targinfosize);
  		return 0;
  	}

	/* Only allow these for packet filtering. */
	if (strcmp(tablename, "filter") != 0) {
		DEBUGP("REJECT: bad table `%s'.\n", tablename);
		return 0;
	}
	if ((hook_mask & ~((1 << NF_IP_LOCAL_IN)
			   | (1 << NF_IP_FORWARD)
			   | (1 << NF_IP_LOCAL_OUT))) != 0) {
		DEBUGP("REJECT: bad hook mask %X\n", hook_mask);
		return 0;
	}

	if (rejinfo->with == IPT_ICMP_ECHOREPLY) {
		printk("REJECT: ECHOREPLY no longer supported.\n");
		return 0;
	} else if (rejinfo->with == IPT_TCP_RESET) {
		/* Must specify that it's a TCP packet */
		if (e->ip.proto != IPPROTO_TCP
		    || (e->ip.invflags & IPT_INV_PROTO)) {
			DEBUGP("REJECT: TCP_RESET invalid for non-tcp\n");
			return 0;
		}
	} else if (rejinfo->with == IPT_TCP_URLBLOCK) {
		/* Must specify that it's a TCP packet */
		if (e->ip.proto != IPPROTO_TCP
		    || (e->ip.invflags & IPT_INV_PROTO)) {
			DEBUGP("REJECT: IPT_TCP_URLBLOCK\n");
			return 0;
		}
	}

	return 1;
}

static struct ipt_target ipt_reject_reg = {
	.name		= "REJECT",
	.target		= reject,
	.checkentry	= check,
	.me		= THIS_MODULE,
};

static int __init init(void)
{
	return ipt_register_target(&ipt_reject_reg);
}

static void __exit fini(void)
{
	ipt_unregister_target(&ipt_reject_reg);
}

module_init(init);
module_exit(fini);
