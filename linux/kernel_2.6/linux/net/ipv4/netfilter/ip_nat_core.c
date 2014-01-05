/* NAT for netfilter; shared with compatibility layer. */

/* (C) 1999-2001 Paul `Rusty' Russell
 * (C) 2002-2004 Netfilter Core Team <coreteam@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/types.h>
#include <linux/timer.h>
#include <linux/skbuff.h>
#include <linux/netfilter_ipv4.h>
#include <linux/vmalloc.h>
#include <net/checksum.h>
#include <net/icmp.h>
#include <net/ip.h>
#include <net/tcp.h>  /* For tcp_prot in getorigdst */
#include <linux/icmp.h>
#include <linux/udp.h>
#include <linux/jhash.h>
 
#define ASSERT_READ_LOCK(x)
#define ASSERT_WRITE_LOCK(x)

#include <linux/netfilter_ipv4/ip_conntrack.h>
#include <linux/netfilter_ipv4/ip_conntrack_core.h>
#include <linux/netfilter_ipv4/ip_conntrack_protocol.h>
#include <linux/netfilter_ipv4/ip_nat.h>
#include <linux/netfilter_ipv4/ip_nat_protocol.h>
#include <linux/netfilter_ipv4/ip_nat_core.h>
#include <linux/netfilter_ipv4/ip_nat_helper.h>
#include <linux/netfilter_ipv4/ip_conntrack_helper.h>
#include <linux/netfilter_ipv4/listhelp.h>
#include <linux/netfilter_ipv4/ioctl.h>

#ifdef CONFIG_NK_IPSEC_MULTIPLE_PASS_THROUGH
	#include <linux/netfilter_ipv4/ipsec_multiple_pass_through.h>
#endif

/** Global Variable **/
#ifdef CONFIG_NK_DYNAMIC_PORT_NUM
#include <linux/dynamic_port_num.h>
#endif
#ifdef CONFIG_NK_URL_TMUFE_FILTER
#include "url_filter/url_filter_mod.c"
#endif

#if 0
#define DEBUGP printk
#else
#define DEBUGP(format, args...)
#endif

#ifdef CONFIG_NK_IPFILTER_SUPPORT_SORTING
#define SORTING_ENTRY 20
#define MAX_WAN_NUM (CONFIG_NK_NUM_WAN+CONFIG_NK_NUM_DMZ)
#ifdef CONFIG_NK_DYNAMIC_PORT_NUM
extern interface_session_t wan_interface_session[CONFIG_NK_NUM_MAX_WAN];
#else
extern interface_session_t wan_interface_session[MAX_WAN_NUM];
#endif
extern struct firewall_setting fw_setting;
#endif

DEFINE_RWLOCK(ip_nat_lock);

//Rain add for Anti-Virus -->
#ifdef CONFIG_NK_ANTI_VIRUS
#define CLAMAV_U_PID	0
#define CLAMAV_VIRUS	1
#define CLAMAV_PASS	2
#define CLAMAV_ERROR	3
#define CLAMAV_CLOSE	4

#define NL_CLAMAV	28
#define CLAMAV_BUFF_LEN	500

struct
{
	__u32 pid;
	rwlock_t lock;
}user_proc;

struct packet_info
{
	char *data;
	int length;
	char *queue;
	int protocol;
};

struct msg_kernel
{
	struct nlmsghdr hdr;
	char *data;
};

struct tasklet_info
{
	struct tasklet_struct	tasklet;
	char *data;
	int result;
};

struct	CLAMAV_BUFF
{
	struct	CLAMAV_BUFF	*prev;
	struct	sk_buff		*pclamavskb;
	struct	CLAMAV_BUFF	*next;
};
struct CLAMAV_BUFF *clamav_head = NULL;

static int device_status = 0;
static struct sock *nlfd;
int Malloc_Clamav_Num = 0;
struct tasklet_info clamav_task;
extern struct anti_virus_setting av_setting;
#endif
//<--Rain

#ifdef CONFIG_NK_URL_TMUFE_FILTER
extern url_filter_setting_t url_filter_setting;
extern url_filter_statics_t url_filter_statics;
extern url_filter_lic_t url_filter_lic;
int (*url_filter_enqueue)(char *) = NULL;
EXPORT_SYMBOL(url_filter_enqueue);

#define URL_IMG_PATH  "images/clinksys.gif"
#endif

/* Calculated at init based on memory size */
static unsigned int ip_nat_htable_size;

static struct list_head *bysource;

#define MAX_IP_NAT_PROTO 256
static struct ip_nat_protocol *ip_nat_protos[MAX_IP_NAT_PROTO];

static inline struct ip_nat_protocol *
__ip_nat_proto_find(u_int8_t protonum)
{
	return ip_nat_protos[protonum];
}

struct ip_nat_protocol *
ip_nat_proto_find_get(u_int8_t protonum)
{
	struct ip_nat_protocol *p;

	/* we need to disable preemption to make sure 'p' doesn't get
	 * removed until we've grabbed the reference */
	preempt_disable();
	p = __ip_nat_proto_find(protonum);
	if (!try_module_get(p->me))
		p = &ip_nat_unknown_protocol;
	preempt_enable();

	return p;
}
EXPORT_SYMBOL_GPL(ip_nat_proto_find_get);

void
ip_nat_proto_put(struct ip_nat_protocol *p)
{
	module_put(p->me);
}
EXPORT_SYMBOL_GPL(ip_nat_proto_put);

/* We keep an extra hash for each conntrack, for fast searching. */
static inline unsigned int
hash_by_src(const struct ip_conntrack_tuple *tuple)
{
	/* Original src, to ensure we map it consistently if poss. */
	return jhash_3words(tuple->src.ip, tuple->src.u.all,
			    tuple->dst.protonum, 0) % ip_nat_htable_size;
}

/* Noone using conntrack by the time this called. */
static void ip_nat_cleanup_conntrack(struct ip_conntrack *conn)
{
	if (!(conn->status & IPS_NAT_DONE_MASK))
		return;

	write_lock_bh(&ip_nat_lock);
	list_del(&conn->nat.info.bysource);
	write_unlock_bh(&ip_nat_lock);
}

/* We do checksum mangling, so if they were wrong before they're still
 * wrong.  Also works for incomplete packets (eg. ICMP dest
 * unreachables.) */
u_int16_t
ip_nat_cheat_check(u_int32_t oldvalinv, u_int32_t newval, u_int16_t oldcheck)
{
	u_int32_t diffs[] = { oldvalinv, newval };
	return csum_fold(csum_partial((char *)diffs, sizeof(diffs),
				      oldcheck^0xFFFF));
}
EXPORT_SYMBOL(ip_nat_cheat_check);

/* Is this tuple already taken? (not by us) */
int
ip_nat_used_tuple(const struct ip_conntrack_tuple *tuple,
		  const struct ip_conntrack *ignored_conntrack)
{
	/* Conntrack tracking doesn't keep track of outgoing tuples; only
	   incoming ones.  NAT means they don't have a fixed mapping,
	   so we invert the tuple and look for the incoming reply.

	   We could keep a separate hash if this proves too slow. */
	struct ip_conntrack_tuple reply;

	invert_tuplepr(&reply, tuple);
	return ip_conntrack_tuple_taken(&reply, ignored_conntrack);
}
EXPORT_SYMBOL(ip_nat_used_tuple);

/* If we source map this tuple so reply looks like reply_tuple, will
 * that meet the constraints of range. */
static int
in_range(const struct ip_conntrack_tuple *tuple,
	 const struct ip_nat_range *range)
{
	struct ip_nat_protocol *proto = 
				__ip_nat_proto_find(tuple->dst.protonum);

	/* If we are supposed to map IPs, then we must be in the
	   range specified, otherwise let this drag us onto a new src IP. */
	if (range->flags & IP_NAT_RANGE_MAP_IPS) {
		if (ntohl(tuple->src.ip) < ntohl(range->min_ip)
		    || ntohl(tuple->src.ip) > ntohl(range->max_ip))
			return 0;
	}

	if (!(range->flags & IP_NAT_RANGE_PROTO_SPECIFIED)
	    || proto->in_range(tuple, IP_NAT_MANIP_SRC,
			       &range->min, &range->max))
		return 1;

	return 0;
}

static inline int
same_src(const struct ip_conntrack *ct,
	 const struct ip_conntrack_tuple *tuple)
{
	return (ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum
		== tuple->dst.protonum
		&& ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip
		== tuple->src.ip
		&& ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u.all
		== tuple->src.u.all);
}

/* Only called for SRC manip */
static int
find_appropriate_src(const struct ip_conntrack_tuple *tuple,
		     struct ip_conntrack_tuple *result,
		     const struct ip_nat_range *range)
{
	unsigned int h = hash_by_src(tuple);
	struct ip_conntrack *ct;

	read_lock_bh(&ip_nat_lock);
	list_for_each_entry(ct, &bysource[h], nat.info.bysource) {
		if (same_src(ct, tuple)) {
			/* Copy source part from reply tuple. */
			invert_tuplepr(result,
				       &ct->tuplehash[IP_CT_DIR_REPLY].tuple);
			result->dst = tuple->dst;

			if (in_range(result, range)) {
				read_unlock_bh(&ip_nat_lock);
				return 1;
			}
		}
	}
	read_unlock_bh(&ip_nat_lock);
	return 0;
}

/* For [FUTURE] fragmentation handling, we want the least-used
   src-ip/dst-ip/proto triple.  Fairness doesn't come into it.  Thus
   if the range specifies 1.2.3.4 ports 10000-10005 and 1.2.3.5 ports
   1-65535, we don't do pro-rata allocation based on ports; we choose
   the ip with the lowest src-ip/dst-ip/proto usage.
*/
static void
find_best_ips_proto(struct ip_conntrack_tuple *tuple,
		    const struct ip_nat_range *range,
		    const struct ip_conntrack *conntrack,
		    enum ip_nat_manip_type maniptype)
{
	u_int32_t *var_ipp;
	/* Host order */
	u_int32_t minip, maxip, j;

	/* No IP mapping?  Do nothing. */
	if (!(range->flags & IP_NAT_RANGE_MAP_IPS))
		return;

	if (maniptype == IP_NAT_MANIP_SRC)
		var_ipp = &tuple->src.ip;
	else
		var_ipp = &tuple->dst.ip;

	/* Fast path: only one choice. */
	if (range->min_ip == range->max_ip) {
		*var_ipp = range->min_ip;
		return;
	}

	/* Hashing source and destination IPs gives a fairly even
	 * spread in practice (if there are a small number of IPs
	 * involved, there usually aren't that many connections
	 * anyway).  The consistency means that servers see the same
	 * client coming from the same IP (some Internet Banking sites
	 * like this), even across reboots. */
	minip = ntohl(range->min_ip);
	maxip = ntohl(range->max_ip);
	j = jhash_2words(tuple->src.ip, tuple->dst.ip, 0);
	*var_ipp = htonl(minip + j % (maxip - minip + 1));
}

/* Manipulate the tuple into the range given.  For NF_IP_POST_ROUTING,
 * we change the source to map into the range.  For NF_IP_PRE_ROUTING
 * and NF_IP_LOCAL_OUT, we change the destination to map into the
 * range.  It might not be possible to get a unique tuple, but we try.
 * At worst (or if we race), we will end up with a final duplicate in
 * __ip_conntrack_confirm and drop the packet. */
static void
get_unique_tuple(struct ip_conntrack_tuple *tuple,
		 const struct ip_conntrack_tuple *orig_tuple,
		 const struct ip_nat_range *range,
		 struct ip_conntrack *conntrack,
		 enum ip_nat_manip_type maniptype)
{
	struct ip_nat_protocol *proto;

	/* 1) If this srcip/proto/src-proto-part is currently mapped,
	   and that same mapping gives a unique tuple within the given
	   range, use that.

	   This is only required for source (ie. NAT/masq) mappings.
	   So far, we don't do local source mappings, so multiple
	   manips not an issue.  */
	if (maniptype == IP_NAT_MANIP_SRC) {
		if (find_appropriate_src(orig_tuple, tuple, range)) {
			DEBUGP("get_unique_tuple: Found current src map\n");
			if (!ip_nat_used_tuple(tuple, conntrack))
				return;
		}
	}

	/* 2) Select the least-used IP/proto combination in the given
	   range. */
	*tuple = *orig_tuple;
	find_best_ips_proto(tuple, range, conntrack, maniptype);

	/* 3) The per-protocol part of the manip is made to map into
	   the range to make a unique tuple. */

	proto = ip_nat_proto_find_get(orig_tuple->dst.protonum);

	/* Only bother mapping if it's not already in range and unique */
	if ((!(range->flags & IP_NAT_RANGE_PROTO_SPECIFIED)
	     || proto->in_range(tuple, maniptype, &range->min, &range->max))
	    && !ip_nat_used_tuple(tuple, conntrack)) {
		ip_nat_proto_put(proto);
		return;
	}

	/* Last change: get protocol to try to obtain unique tuple. */
	proto->unique_tuple(tuple, range, maniptype, conntrack);

	ip_nat_proto_put(proto);
}

unsigned int
ip_nat_setup_info(struct ip_conntrack *conntrack,
		  const struct ip_nat_range *range,
		  unsigned int hooknum)
{
	struct ip_conntrack_tuple curr_tuple, new_tuple;
	struct ip_nat_info *info = &conntrack->nat.info;
	int have_to_hash = !(conntrack->status & IPS_NAT_DONE_MASK);
	enum ip_nat_manip_type maniptype = HOOK2MANIP(hooknum);

	IP_NF_ASSERT(hooknum == NF_IP_PRE_ROUTING
		     || hooknum == NF_IP_POST_ROUTING
		     || hooknum == NF_IP_LOCAL_IN
		     || hooknum == NF_IP_LOCAL_OUT);
	BUG_ON(ip_nat_initialized(conntrack, maniptype));

	/* What we've got will look like inverse of reply. Normally
	   this is what is in the conntrack, except for prior
	   manipulations (future optimization: if num_manips == 0,
	   orig_tp =
	   conntrack->tuplehash[IP_CT_DIR_ORIGINAL].tuple) */
	invert_tuplepr(&curr_tuple,
		       &conntrack->tuplehash[IP_CT_DIR_REPLY].tuple);

	get_unique_tuple(&new_tuple, &curr_tuple, range, conntrack, maniptype);

	if (!ip_ct_tuple_equal(&new_tuple, &curr_tuple)) {
		struct ip_conntrack_tuple reply;

		/* Alter conntrack table so will recognize replies. */
		invert_tuplepr(&reply, &new_tuple);
		ip_conntrack_alter_reply(conntrack, &reply);

		/* Non-atomic: we own this at the moment. */
		if (maniptype == IP_NAT_MANIP_SRC)
			conntrack->status |= IPS_SRC_NAT;
		else
			conntrack->status |= IPS_DST_NAT;
	}

	/* Place in source hash if this is the first time. */
	if (have_to_hash) {
		unsigned int srchash
			= hash_by_src(&conntrack->tuplehash[IP_CT_DIR_ORIGINAL]
				      .tuple);
		write_lock_bh(&ip_nat_lock);
		list_add(&info->bysource, &bysource[srchash]);
		write_unlock_bh(&ip_nat_lock);
	}

	/* It's done. */
	if (maniptype == IP_NAT_MANIP_DST)
		set_bit(IPS_DST_NAT_DONE_BIT, &conntrack->status);
	else
		set_bit(IPS_SRC_NAT_DONE_BIT, &conntrack->status);

	return NF_ACCEPT;
}
EXPORT_SYMBOL(ip_nat_setup_info);

/* Returns true if succeeded. */
static int
manip_pkt(u_int16_t proto,
	  struct sk_buff **pskb,
	  unsigned int iphdroff,
	  const struct ip_conntrack_tuple *target,
	  enum ip_nat_manip_type maniptype)
{
	struct iphdr *iph;
	struct ip_nat_protocol *p;

	if (!skb_make_writable(pskb, iphdroff + sizeof(*iph)))
		return 0;

	iph = (void *)(*pskb)->data + iphdroff;

	/* Manipulate protcol part. */
	p = ip_nat_proto_find_get(proto);
	if (!p->manip_pkt(pskb, iphdroff, target, maniptype)) {
		ip_nat_proto_put(p);
		return 0;
	}
	ip_nat_proto_put(p);

	iph = (void *)(*pskb)->data + iphdroff;

	if (maniptype == IP_NAT_MANIP_SRC) {
		iph->check = ip_nat_cheat_check(~iph->saddr, target->src.ip,
						iph->check);
		iph->saddr = target->src.ip;
	} else {
		iph->check = ip_nat_cheat_check(~iph->daddr, target->dst.ip,
						iph->check);
		iph->daddr = target->dst.ip;
	}
	return 1;
}

#ifdef CONFIG_NK_IPSEC_MULTIPLE_PASS_THROUGH
	struct fire_spec_output
	{
		struct fire_spec_output *next;
		u32 saddr;
		u32 daddr;
		u32 protocol_type;
		u32 src_port;
		u32 dst_port;
		struct net_device *dev;
		int age;
	};
	extern struct fire_spec_output *fire_spec_output_head;
	extern rwlock_t FireSpecLock;
#endif

//Rain add for Anti-Virus -->
#ifdef CONFIG_NK_ANTI_VIRUS
char* clamav_enqueue(struct sk_buff *pskb)
{
	struct	CLAMAV_BUFF	*new_node;
	

	if(Malloc_Clamav_Num > CLAMAV_BUFF_LEN)
		return NULL;
	
	new_node = (struct CLAMAV_BUFF *)kmalloc(sizeof(struct CLAMAV_BUFF), GFP_KERNEL);
	if(new_node == NULL)
		return NULL;
	
	memset((char *)new_node, 0, sizeof(struct CLAMAV_BUFF));
	Malloc_Clamav_Num++;
	
	new_node->pclamavskb = pskb;
	
	if(clamav_head == NULL)
		clamav_head = new_node;
	
	new_node->next = clamav_head;
	clamav_head->prev = new_node;
	new_node->prev = new_node;
	clamav_head = new_node;
	
	return (char *)new_node;
}

void clamav_dequeue(unsigned long dequeue_data)
{
	struct	CLAMAV_BUFF	*remove_node;
	struct tasklet_info	*clamav_task;
	struct tcphdr *tcph;
	struct ip_conntrack	*ct = NULL;
	enum ip_conntrack_info	ctinfo;
	int scan_act = 0, clamav_result = 0, payloadln = 0, httpdataln = 0;
	char *httpdata;
	
	
	if(dequeue_data == 0)
		return;
		
	clamav_task = (struct tasklet_info *)dequeue_data;
	clamav_result = clamav_task->result;

	if(Malloc_Clamav_Num <= 0)
		return;
			
	if(clamav_result == CLAMAV_CLOSE)  //remove all
	{
		int i;
		
		for(i = 0; i < Malloc_Clamav_Num; i++)
		{
			if(clamav_head != NULL)
			{
				remove_node = clamav_head;
				clamav_head = remove_node->next;
				kfree_skb(remove_node->pclamavskb);
				kfree(remove_node);
			}
		}
		Malloc_Clamav_Num = 0;
		clamav_head = NULL;
	}
	else  //remove specific
	{
		remove_node = (struct CLAMAV_BUFF *)clamav_task->data;
		if(remove_node == NULL)
			return;
		
		if(clamav_head == remove_node)
		{
			clamav_head = remove_node->next;
			remove_node->next->prev = remove_node->next;
		}
		else
		{
			remove_node->next->prev = remove_node->prev;
			remove_node->prev->next = remove_node->next;
		}

		Malloc_Clamav_Num--;
		if(Malloc_Clamav_Num == 0)
			clamav_head = NULL;

		if(clamav_result == CLAMAV_VIRUS)
		{
			tcph = (void *)(remove_node->pclamavskb->nh.iph) + remove_node->pclamavskb->nh.iph->ihl * 4;
			
			if(tcph->source == htons(80) || tcph->dest == htons(80))  //HTTP
			{
				if(av_setting.av_enabled && av_setting.http_enabled)
					scan_act = av_setting.http_action;
			}
			else if(tcph->source == htons(21) || tcph->dest == htons(21))  //FTP
			{
				if(av_setting.av_enabled && av_setting.ftp_enabled)
					scan_act = av_setting.ftp_action;
			}
			else
				scan_act = 1;   //Pass Packet
			
			switch(scan_act)
			{
			case 2:   //Destroy
				payloadln = remove_node->pclamavskb->len - remove_node->pclamavskb->nh.iph->ihl*4 - tcph->doff*4;
	
				if(!strncmp((char *)tcph + tcph->doff*4, "HTTP", 4))
				{
					httpdata = strstr((char *)tcph + tcph->doff*4, "\r\n\r\n");
					if(httpdata)
					{
						httpdataln = payloadln - (httpdata - ((char *)tcph + tcph->doff*4)) - 4;
						if(httpdataln > 0)
							memset(httpdata, 0, httpdataln);
					}
				}
				else
					memset((char *)tcph + tcph->doff*4, 0, payloadln);
				break;
			case 3:  //Reset
				ct = ip_conntrack_get(remove_node->pclamavskb, &ctinfo);
				if (ct != NULL)
					ct->timeout.expires = 0;
		
				tcph->rst = 1;
				break;
			}
			
		}

		ip_rcv_finish(remove_node->pclamavskb);
		kfree(remove_node);
	}
		
	return;
}

static void kernel_receive(struct sock *sk, int len)
{
	do
	{
		struct sk_buff *skb;

		while((skb = skb_dequeue(&sk->sk_receive_queue)) != NULL)
		{
			struct msg_kernel *message = NULL;
			
			if(skb->len >= sizeof(struct nlmsghdr))
			{
				message = (struct msg_kernel *)skb->data;
				if((message->hdr.nlmsg_len >= sizeof(struct nlmsghdr))&& (skb->len >= message->hdr.nlmsg_len))
				{
					if(message->hdr.nlmsg_type == CLAMAV_U_PID)
					{
						printk(KERN_EMERG "Kernel: nlmsg_type[CLAMAV_U_PID]\n");
						user_proc.pid = message->hdr.nlmsg_pid;
						device_status = 1;
					}
					else if(message->hdr.nlmsg_type == CLAMAV_CLOSE)
					{
						printk(KERN_EMERG "Kernel: nlmsg_type[CLAMAV_CLOSE]\n");
						device_status = 0;
						clamav_task.result = message->hdr.nlmsg_type;
						tasklet_schedule(&clamav_task.tasklet);
					}
					else
					{
						clamav_task.data = message->data;
						clamav_task.result = message->hdr.nlmsg_type;
						tasklet_schedule(&clamav_task.tasklet);
					}	
				}
			}
			kfree_skb(skb);
		}
	}while(nlfd && nlfd->sk_receive_queue.qlen);
}

int send_to_user(char *payload, int payloadln, char *queue_data, int protocol_type)
{
	int ret;
	int size;
	unsigned char *old_tail;
	struct sk_buff *skb;
	struct nlmsghdr *nlh;
	struct packet_info *packet;


	size = NLMSG_SPACE(sizeof(struct packet_info));

	skb = alloc_skb(size, GFP_ATOMIC);
	old_tail = skb->tail;

	nlh = NLMSG_PUT(skb, 0, 0, 1, size-sizeof(*nlh));
	packet = NLMSG_DATA(nlh);
	memset(packet, 0, sizeof(struct packet_info));
	packet->data = payload;
	packet->length = payloadln;
	packet->queue = queue_data;
	packet->protocol = protocol_type;
	
	nlh->nlmsg_len = skb->tail - old_tail;
	NETLINK_CB(skb).dst_group = 0;
	ret = netlink_unicast(nlfd, skb, user_proc.pid, MSG_DONTWAIT);

	return ret;
	
nlmsg_failure:
	if(skb)
		kfree_skb(skb);

	return -1;
}
#endif
//<-- Rain

#ifdef CONFIG_NK_URL_TMUFE_FILTER
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

void url_tmufe_filter_block(const struct sk_buff *pskb, int block_status, unsigned int debug_mode)
{
    struct iphdr *iph = (pskb)->nh.iph;
    struct tcphdr *tcph = (void *)iph + iph->ihl*4;
    unsigned char *data = (void *)tcph + tcph->doff*4;
    unsigned int datalen = (pskb)->len - (iph->ihl*4) - (tcph->doff*4);
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

	// start to modify skb content
	typedef struct tcphdr tcphdr_t;
	static char block_htm[600];

	/*purpose     : 0013239 author : Ben date : 2010-10-06*/
	/*description : Fix can't show correct block message on Firefox,Safari,Chrome*/
        if (block_status == URL_BLOCK)
	{
		snprintf(block_htm, sizeof(block_htm),
		"<html><head><title></title></head>"
//		"<body background=\"http://%u.%u.%u.%u/images/body_bg.jpg\"><font face=Verdana size=2>"
		"<body>"
		"The URL you are attempting to access has been blocked. Your organization's policy prohibits accessing this web site."
//		"<li><a href=\"http://reclassify.wrs.trendmicro.com\">Submit this URL to TrendLabs for reclassification</a></li>"
//		"</font></body></html>", NIPQUAD(wan_interface_session[0].lanip)
		"</body></html>"
		);
		if (debug_mode) printk(KERN_EMERG "%s: block_status == URL_BLOCK\n",__FUNCTION__);
	}
	else if (block_status == URL_WRS_BLOCK)
	{
		snprintf(block_htm, sizeof(block_htm),
		"<html><head><title></title></head>"
//		"<body background=\"http://%u.%u.%u.%u/images/body_bg.jpg\"><font face=Verdana size=2>"
		"<body>"
		"The URL you are attempting to access could potentially be a security risk. Trend Micro ProtectLink Gateway has blocked the URL."
//		"<li><a href=\"http://reclassify.wrs.trendmicro.com\">Submit this URL to TrendLabs for reclassification</a></li>"
//		"</font></body></html>", NIPQUAD(wan_interface_session[0].lanip)
		"</body></html>"
		);
		if (debug_mode) printk(KERN_EMERG "%s: block_status == URL_WRS_BLOCK\n",__FUNCTION__);
	}
	else
	{
		snprintf(block_htm, sizeof(block_htm),
		"<html><head><title></title></head>"
//		"<body background=\"http://%u.%u.%u.%u/images/body_bg.jpg\"><font face=Verdana size=2>"
		"<body>"
		"The router is very busy at the moment. Please try after a few minutes."
//		"</font></body></html>", NIPQUAD(wan_interface_session[0].lanip)
		"</body></html>"
		);
		if (debug_mode) printk(KERN_EMERG "%s: block_status == else\n",__FUNCTION__);
	}

	mc = skb_copy(pskb,GFP_ATOMIC);
	mc->input_dev=NULL;
	mc->dev=NULL;
	mc->pkt_type = PACKET_HOST;
	if( __ip_route_output_key(&rt,&fl) )
	{
		kfree_skb(mc);
		if (debug_mode)
		printk(KERN_EMERG "%s: __ip_route_output_key fail",__FUNCTION__);
		return;
	}
	mc->csum=0;
	mc->dst = &rt->u.dst;
	mc->dev = mc->dst->dev;


	//header definition
	struct iphdr *new_iph = (mc)->nh.iph;
	struct tcphdr *new_tcph = (void *)new_iph + new_iph->ihl*4;
	unsigned char *new_data = (void *)new_tcph + new_tcph->doff*4;
	unsigned int newdatalen = 0;

	memset((char *)new_data,0, datalen);
	/* fill ip header */
	iph->protocol = IPPROTO_TCP;
	new_iph->saddr = iph->daddr;
	new_iph->daddr = iph->saddr;
	new_iph->ttl = 127;
	new_iph->check =0;
	new_iph->id =0;
	new_iph->frag_off =0;
	new_iph->check = ip_fast_csum((void *)new_iph, new_iph->ihl);
	/* fill tcp header */
	new_tcph->source = tcph->dest;
	new_tcph->dest = tcph->source;
	tcp_flag_word(new_tcph) = (TCP_FLAG_ACK | TCP_FLAG_PSH | TCP_FLAG_FIN);
	if (tcp_flag_word(new_tcph) & TCP_FLAG_ACK)
		new_tcph->seq = tcph->ack_seq;
	else
		new_tcph->seq = htonl(1);
	new_tcph->doff = (sizeof(tcphdr_t)>>2);
	/* the ack should acknowledge all the packet, find the size */
	new_tcph->ack_seq = htonl(ntohl(tcph->seq) + datalen);
	if ((int)datalen<=strlen(block_htm))
	{
		if (debug_mode) printk(KERN_EMERG "%s: datalen\n",__FUNCTION__);
		memcpy((char *)new_tcph + sizeof(tcphdr_t), block_htm, (int)datalen);
	}
	else
	{
		if (debug_mode) printk(KERN_EMERG "%s: strlen(block_htm)\n",__FUNCTION__);
		memcpy((char *)new_tcph + sizeof(tcphdr_t), block_htm, strlen(block_htm));
	}
	new_tcph->check = csum_tcpudp_magic(new_iph->saddr, new_iph->daddr, datalen, IPPROTO_TCP,csum_partial((char *)new_tcph,datalen,0));
#if 0
	struct sk_buff *mc1=NULL;
	mc1 = skb_copy(mc,GFP_ATOMIC);
	struct iphdr *new_iph1 = (mc1)->nh.iph;
	struct tcphdr *new_tcph1 = (void *)new_iph1 + new_iph1->ihl*4;
	tcp_flag_word(new_tcph1) = (TCP_FLAG_ACK | TCP_FLAG_PSH |TCP_FLAG_RST);
	new_tcph1->check = csum_tcpudp_magic(new_iph1->saddr, new_iph1->daddr, datalen, IPPROTO_TCP,csum_partial((char *)new_tcph1,datalen,0));
	new_tcph1->doff = (sizeof(tcphdr_t)>>2);
#endif
	ip_finish_output(mc);
//	ip_finish_output(mc1);
#if 1
	struct sk_buff *mc1=NULL;
	mc1 = skb_copy(pskb,GFP_ATOMIC);
	nk_modifyPkt_for_reset(mc1);
        ip_rcv_finish(mc1);
#endif
#if 0
	nk_modifyPkt_for_reset(pskb);
        ip_rcv_finish(pskb);
#endif
	if (debug_mode) printk(KERN_EMERG "%s: finish\n",__FUNCTION__);
	return;
}
#endif

#define URL_FILTER_UNHANDLE 	 	0
#define URL_FILTER_DROP		 	1
#define URL_FILTER_HANDLE	 	2

/* Do packet manipulations according to ip_nat_setup_info. */
unsigned int ip_nat_packet(struct ip_conntrack *ct,
			   enum ip_conntrack_info ctinfo,
			   unsigned int hooknum,
			   struct sk_buff **pskb)
{
	int i, k, l;
	enum ip_conntrack_dir dir = CTINFO2DIR(ctinfo);
	unsigned long statusbit;
	enum ip_nat_manip_type mtype = HOOK2MANIP(hooknum);
#if CONFIG_NK_IPFILTER_SUPPORT_SORTING
	int j, tmp=0;
#endif
#ifdef CONFIG_NK_URL_TMUFE_FILTER
	int url_filter_ret;
#endif

	//if (mtype == IP_NAT_MANIP_SRC)
	//if(dir == IP_CT_DIR_ORIGINAL)//mark 20090520
	{	
		(*pskb)->orig_src_ip = (*pskb)->nh.iph->saddr;
		//printk(KERN_EMERG "fill orig_src_ip = %u.%u.%u.%u\n", NIPQUAD((*pskb)->nh.iph->saddr));
	}


	if(fw_setting.qos)
	{
		if((*pskb)->dev)
		{
			for(i=0; i<MAX_WAN_NUM; i++)
			{
				if(!strcmp((*pskb)->dev->name,wan_interface_session[i].name))
				{
					(*pskb)->src_interface_num=i+1;
					break;
				}
			}
		}
	}

#if CONFIG_NK_IPFILTER_SUPPORT_SORTING
if (mtype == IP_NAT_MANIP_DST)
{
		if( (fw_setting.tr_enable==1)||(fw_setting.smart_qos==1) )
		{
			ct->tr.b_cnt += ntohs((*pskb)->len);

			//printk(KERN_EMERG "%u.%u.%u.%u - > %u.%u.%u.%u dir=%d (%u.%u.%u.%u)\n", NIPQUAD((*pskb)->nh.iph->saddr),
			//NIPQUAD((*pskb)->nh.iph->daddr), dir, NIPQUAD(wan_interface_session[2].wan_ip));

			
#ifdef CONFIG_NK_TRANSPARENT_BRIDGE_RANGE_NUM
			for(j=0; j<MAX_WAN_NUM; j++)
			{
				if((*pskb)->nh.iph->daddr == wan_interface_session[j].wan_ip)
				{
					//ct->tr.down_bytes += ntohs((*pskb)->len);
					tmp=1;
					break;
				}

				for(l=0;l<CONFIG_NK_TRANSPARENT_BRIDGE_RANGE_NUM;l++)
				{
					if(((*pskb)->nh.iph->daddr>=wan_interface_session[j].internallanip1[l])&&
					((*pskb)->nh.iph->daddr<=wan_interface_session[j].internallanip2[l]))
					{
						tmp=1;
						break;
					}
				}
			}
#else
			for(j=0; j<MAX_WAN_NUM; j++)
			{
				if(( (*pskb)->nh.iph->daddr == wan_interface_session[j].wan_ip) ||
				(((*pskb)->nh.iph->daddr>=wan_interface_session[j].internallanip1)&&
				((*pskb)->nh.iph->daddr<=wan_interface_session[j].internallanip2))||
				(((*pskb)->nh.iph->daddr>=wan_interface_session[j].internallanip3)&&
				((*pskb)->nh.iph->daddr<=wan_interface_session[j].internallanip4)))
				{
					//ct->tr.down_bytes += ntohs((*pskb)->len);
					tmp=1;
					break;
				}
			}
#endif
			/*DMZ port*/
			if( ((*pskb)->nh.iph->daddr>=wan_interface_session[0].dmz_start)&&
			    ((*pskb)->nh.iph->daddr<=wan_interface_session[0].dmz_end) )
				tmp=1;

			if(((*pskb)->nh.iph->daddr&wan_interface_session[0].dmz_mask)==(wan_interface_session[0].dmz_start&wan_interface_session[0].dmz_mask) )
				tmp=1;


//router+nat mode
			for(j=0; j<MAX_WAN_NUM; j++)
			{
				if(wan_interface_session[j].wan_ip>0)
				{
					for(k=0;k<3;k++)
					{
						if(wan_interface_session[j].routerinfo[k].rip1>0)
						{
							if((((*pskb)->nh.iph->daddr>=wan_interface_session[j].routerinfo[k].rip1)&&
							((*pskb)->nh.iph->daddr<=wan_interface_session[j].routerinfo[k].rip2)) || (((*pskb)->nh.iph->daddr>=wan_interface_session[j].routerinfo[k].rip3)&&
							((*pskb)->nh.iph->daddr<=wan_interface_session[j].routerinfo[k].rip4)) )
							{
								tmp=1;
								break;
							}
							else if((*pskb)->nh.iph->daddr==wan_interface_session[j].routerinfo[k].rgw)
							{
								tmp=1;
								break;
							}
						}
					}
				}
			}











			if(dir==1)
			{
				if(tmp==1)
					ct->tr.down_bytes += ntohs((*pskb)->len);
				else
					ct->tr.up_bytes += ntohs((*pskb)->len);
//printk(KERN_EMERG "dir=1, up_bytes=%d\n", ntohs((*pskb)->len));
			}
			else if(dir==0)
			{
				if(tmp==1) 
				{
					ct->tr.down_bytes += ntohs((*pskb)->len);
					//printk(KERN_EMERG "dir=0,tmp=1, down_bytes=%d\n", ntohs((*pskb)->len));
				}
				else
				{
					ct->tr.up_bytes += ntohs((*pskb)->len);
					//printk(KERN_EMERG "dir=0,tmp=0, up_bytes=%d\n", ntohs((*pskb)->len));
				}
			}
		}

		//if (info->manips[i].maniptype == IP_NAT_MANIP_SRC)
		if( (fw_setting.tr_enable==1)||(fw_setting.smart_qos==1) )
			(*pskb)->conntrack_ptr = (void *)ct;
}
#endif	
	if((ct->helper)||((ntohs(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.all)==80) || ((ntohs(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.all)==8080)))) //if need ALG, do not do nat acc
	{
		//printk(KERN_EMERG "alg=%s\n", ct->helper->name);
		(*pskb)->alg = 1;
	}
	else
		(*pskb)->alg = 0;
	//printk(KERN_EMERG "NAT1 : [%x] [%d] -> [%x] [%d] skb->alg(%x) = %d\n", (*pskb)->nh.iph->saddr, *(((uint32_t*) (*pskb)->nh.iph) + (*pskb)->nh.iph->ihl) >> 16, (*pskb)->nh.iph->daddr, *(((uint32_t*) (*pskb)->nh.iph) + (*pskb)->nh.iph->ihl) & 0xff , &((*pskb)->alg), (*pskb)->alg);

#ifdef CONFIG_NK_IPSEC_MULTIPLE_PASS_THROUGH
	if(hooknum == NF_IP_PRE_ROUTING)
	{
		if((*pskb)->nh.iph->protocol == IPPROTO_UDP)
		{
			struct udphdr *udph = (void *)((*pskb)->nh.iph) + (*pskb)->nh.iph->ihl * 4;

			if(udph->source == htons(500) || udph->dest == htons(500))
			{
				int ret;

				ret = isakmp_help(ct, ctinfo, hooknum, pskb);

				if(ret == NF_ACCEPT)
				{
					/* set 1, avoid NAT ACC */
					(*pskb)->pass_through = 1;

					return NF_ACCEPT;
				}
			}
		}
	}
	else if(hooknum == NF_IP_POST_ROUTING)
	{
		if((*pskb)->nh.iph->protocol == IPPROTO_UDP)
		{
			struct udphdr *udph = (void *)((*pskb)->nh.iph) + (*pskb)->nh.iph->ihl * 4;

			if(udph->source == htons(500) || udph->dest == htons(500))
			{
				if(strcmp((*pskb)->dev->name, "eth0"))
				{
					struct fire_spec_output *fire_spec_output_ptr;

					/* support Multi WAN */
					fire_spec_output_ptr = kmalloc(sizeof(struct fire_spec_output), GFP_ATOMIC);

					if(fire_spec_output_ptr)
					{
						fire_spec_output_ptr->saddr = (*pskb)->nh.iph->saddr;
						fire_spec_output_ptr->daddr = (*pskb)->nh.iph->daddr;
						fire_spec_output_ptr->protocol_type = htons(IPPROTO_ESP);
						fire_spec_output_ptr->src_port = 0;
						fire_spec_output_ptr->dst_port = 0;
						fire_spec_output_ptr->dev = (*pskb)->dev;
						fire_spec_output_ptr->age = 60;
	
						/* purpose     : Routing    author : David    date : 2010-08-03        */
						/* description : Add spin lock to fix crash issue        .             */
						write_lock_bh(&FireSpecLock);
						fire_spec_output_ptr->next = fire_spec_output_head;
						fire_spec_output_head = fire_spec_output_ptr;
						write_unlock_bh(&FireSpecLock);

						//printk("*** %s: fire_spec_output_ptr alloc success, proto[%u]: sip[%u.%u.%u.%u]:%u->dip[%u.%u.%u.%u]:%u dev[%s], idev[%s], odev[%s]\n",
						//__func__, fire_spec_output_ptr->protocol_type, NIPQUAD(fire_spec_output_ptr->saddr),  fire_spec_output_ptr->src_port, NIPQUAD(fire_spec_output_ptr->daddr), fire_spec_output_ptr->dst_port, (*pskb)->dev->name, (*pskb)->input_dev->name, (*pskb)->odev->name);
					}
				}
			}
		}
	}
#endif

	if (mtype == IP_NAT_MANIP_SRC)
		statusbit = IPS_SRC_NAT;
	else
		statusbit = IPS_DST_NAT;

	/* Invert if this is reply dir. */
	if (dir == IP_CT_DIR_REPLY)
		statusbit ^= IPS_NAT_MASK;

	/* Non-atomic: these bits don't change. */
	if (ct->status & statusbit) {
		struct ip_conntrack_tuple target;

		/* We are aiming to look like inverse of other direction. */
		invert_tuplepr(&target, &ct->tuplehash[!dir].tuple);

		//printk(KERN_EMERG "(*pskb)->conntrack_ptr=%p\n", (*pskb)->conntrack_ptr);

#if 0		
		/* add orig src ip, for qos, Yami */
		if (mtype == IP_NAT_MANIP_SRC)
		{
			(*pskb)->orig_src_ip = (*pskb)->nh.iph->saddr;
			printk(KERN_EMERG "fill orig_src_ip = %u.%u.%u.%u\n", NIPQUAD((*pskb)->orig_src_ip));
		}
		if((*pskb)->dev)
		{
			for(i=0; i<MAX_WAN_NUM; i++)
			{
				if(!strcmp((*pskb)->dev->name,wan_interface_session[i].name))
				{
					(*pskb)->src_interface_num=i+1;
					printk(KERN_EMERG "fill interface num=%d\n", (*pskb)->src_interface_num);
					break;
				}
			}
			//printk("dev=%s, num=%d\n", (*pskb)->dev->name, (*pskb)->src_interface_num);
		}
#endif

		if (!manip_pkt(target.dst.protonum, pskb, 0, &target, mtype))
			return NF_DROP;
	}
	
#ifdef CONFIG_NK_ANTI_VIRUS
	//Rain add for Anti-Virus -->
	if(hooknum == NF_IP_PRE_ROUTING)
	{
		(*pskb)->http = 0;
		(*pskb)->ftp = 0;
		if(device_status && av_setting.av_enabled && (*pskb)->nh.iph->protocol == IPPROTO_TCP)
		{
			struct tcphdr *tcph = (void *)((*pskb)->nh.iph) + (*pskb)->nh.iph->ihl * 4;
	
			if(av_setting.http_enabled && (tcph->source == htons(80) || tcph->dest == htons(80)))
			{		
				int payloadln = 0, httpdataln = 0;

				//for NAT ACC
				(*pskb)->http = 1;
	
				payloadln = (*pskb)->len - (*pskb)->nh.iph->ihl*4 - tcph->doff*4;
				if(payloadln > 8)
				{
					char *payload_data, *httpdata;	
					
					if(!strncmp((char *)tcph + tcph->doff*4, "HTTP", 4))
					{
						httpdata = strstr((char *)tcph + tcph->doff*4, "\r\n\r\n");
						if(httpdata)
						{
							httpdataln = payloadln - (httpdata - ((char *)tcph + tcph->doff*4)) - 4;
							if(httpdataln > 0)
							{
								payload_data = clamav_enqueue(*pskb);
								if(payload_data)
								{
									send_to_user(httpdata + 4, httpdataln, payload_data, 1);
									return NF_STOLEN;
								}
							}
						}
					}
					else
					{
						payload_data = clamav_enqueue(*pskb);
						if(payload_data)
						{
							send_to_user((char *)tcph + tcph->doff*4, payloadln, payload_data, 1);
							return NF_STOLEN;
						}
					}
				}
			}
		}		
	}
#endif
	//<--Rain

#ifdef CONFIG_NK_URL_TMUFE_FILTER
	if(hooknum == NF_IP_PRE_ROUTING)
	{
	    if (url_filter_lic.lic_valid==1 && (url_filter_setting.filter_enable==1 || url_filter_setting.wrs_enable==1))
	    {
	        if((*pskb)->nh.iph->protocol == IPPROTO_TCP)
	        {
	            struct tcphdr *tcph = (void *)((*pskb)->nh.iph) + (*pskb)->nh.iph->ihl * 4;

	            if(tcph->dest == htons(80))
	            {
	                int payloadln = 0, httpdataln = 0;
	                httpinfo_t htinfo;
	                char tmplanip[30];

	                payloadln = (*pskb)->len - (*pskb)->nh.iph->ihl*4 - tcph->doff*4;
	                if(payloadln > 8)
	                {
	                    char *payload_data, *httpdata;

	                    if(!strncmp((char *)tcph + tcph->doff*4, "GET ", sizeof("GET ") - 1) || !strncmp((char *)tcph + tcph->doff*4, "POST ", sizeof("POST ") - 1))
	                    {
/*
	                        if (url_filter_statics.url_debug_pkt_cnt.queue_pkt_cnt >= url_filter_setting.url_setting.queue_len)
	                        {
	                            if (url_filter_setting.overflow_control == 1)//block
	                            {
	                                url_tmufe_filter_block(*pskb, URL_OVERFLOW_BLOCK);
	                                return NF_DROP;
	                            }
	                        }
	                        else
	                        {
*/
/*
	                            if (get_http_info(pskb, &htinfo))
	                            {
	                                sprintf(tmplanip, "%u.%u.%u.%u", NIPQUAD(wan_interface_session[0].lanip));
	                                if (!strcmp(htinfo.host, tmplanip))//skip UI page
	                                    return NF_ACCEPT;
	                                if (strstr(htinfo.host, ".trendmicro.com"))//skip trendmicro page
	                                    return NF_ACCEPT;
	                            }
	                            else
	                            {
	                                printk(KERN_EMERG "Error : get_http_info fail!\n");
	                                return NF_ACCEPT;
	                            }
*/
	                            if( url_filter_enqueue )
	                            {
	                                url_filter_ret = url_filter_enqueue(pskb);
	                            }
	                            else
	                            {
	                                return NF_ACCEPT;
	                            }
	                            //<--
	                            //CA2 : NF_DROP=0,NF_ACCEPT=1,NF_STOLEN=2,NF_QUEUE=3,NF_REPEAT=4,NF_STOP=5
	                            //CA2 : URL_FILTER_UNHANDLE=0,URL_FILTER_DROP=1,URL_FILTER_HANDLE=2
	                            if (url_filter_ret == URL_FILTER_UNHANDLE)
	                                return NF_ACCEPT;
	                            else if (url_filter_ret == URL_FILTER_DROP)
	                                return NF_DROP;
	                            else if (url_filter_ret == URL_FILTER_HANDLE)
	                                return NF_STOLEN;
/*
	                        }
*/
	                    }
	                }
	            }
	        }
	    }
	}
#endif
	
	return NF_ACCEPT;
}
EXPORT_SYMBOL_GPL(ip_nat_packet);

/* Dir is direction ICMP is coming from (opposite to packet it contains) */
int ip_nat_icmp_reply_translation(struct sk_buff **pskb,
				  struct ip_conntrack *ct,
				  enum ip_nat_manip_type manip,
				  enum ip_conntrack_dir dir)
{
	struct {
		struct icmphdr icmp;
		struct iphdr ip;
	} *inside;
	struct ip_conntrack_tuple inner, target;
	int hdrlen = (*pskb)->nh.iph->ihl * 4;
	unsigned long statusbit;

	if (!skb_make_writable(pskb, hdrlen + sizeof(*inside)))
		return 0;

	inside = (void *)(*pskb)->data + (*pskb)->nh.iph->ihl*4;

	/* We're actually going to mangle it beyond trivial checksum
	   adjustment, so make sure the current checksum is correct. */
	if ((*pskb)->ip_summed != CHECKSUM_UNNECESSARY) {
		hdrlen = (*pskb)->nh.iph->ihl * 4;
		if ((u16)csum_fold(skb_checksum(*pskb, hdrlen,
						(*pskb)->len - hdrlen, 0)))
			return 0;
	}

	/* Must be RELATED */
	IP_NF_ASSERT((*pskb)->nfctinfo == IP_CT_RELATED ||
		     (*pskb)->nfctinfo == IP_CT_RELATED+IP_CT_IS_REPLY);

	/* Redirects on non-null nats must be dropped, else they'll
           start talking to each other without our translation, and be
           confused... --RR */
	if (inside->icmp.type == ICMP_REDIRECT) {
		/* If NAT isn't finished, assume it and drop. */
		if ((ct->status & IPS_NAT_DONE_MASK) != IPS_NAT_DONE_MASK)
			return 0;

		if (ct->status & IPS_NAT_MASK)
			return 0;
	}

	DEBUGP("icmp_reply_translation: translating error %p manp %u dir %s\n",
	       *pskb, manip, dir == IP_CT_DIR_ORIGINAL ? "ORIG" : "REPLY");

	if (!ip_ct_get_tuple(&inside->ip, *pskb, (*pskb)->nh.iph->ihl*4 +
	                     sizeof(struct icmphdr) + inside->ip.ihl*4,
	                     &inner,
			     __ip_conntrack_proto_find(inside->ip.protocol)))
		return 0;

	/* Change inner back to look like incoming packet.  We do the
	   opposite manip on this hook to normal, because it might not
	   pass all hooks (locally-generated ICMP).  Consider incoming
	   packet: PREROUTING (DST manip), routing produces ICMP, goes
	   through POSTROUTING (which must correct the DST manip). */
	if (!manip_pkt(inside->ip.protocol, pskb,
		       (*pskb)->nh.iph->ihl*4
		       + sizeof(inside->icmp),
		       &ct->tuplehash[!dir].tuple,
		       !manip))
		return 0;

	/* Reloading "inside" here since manip_pkt inner. */
	inside = (void *)(*pskb)->data + (*pskb)->nh.iph->ihl*4;
	inside->icmp.checksum = 0;
	inside->icmp.checksum = csum_fold(skb_checksum(*pskb, hdrlen,
						       (*pskb)->len - hdrlen,
						       0));

	/* Change outer to look the reply to an incoming packet
	 * (proto 0 means don't invert per-proto part). */
	if (manip == IP_NAT_MANIP_SRC)
		statusbit = IPS_SRC_NAT;
	else
		statusbit = IPS_DST_NAT;

	/* Invert if this is reply dir. */
	if (dir == IP_CT_DIR_REPLY)
		statusbit ^= IPS_NAT_MASK;

	if (ct->status & statusbit) {
		invert_tuplepr(&target, &ct->tuplehash[!dir].tuple);
		if (!manip_pkt(0, pskb, 0, &target, manip))
			return 0;
	}

	return 1;
}
EXPORT_SYMBOL_GPL(ip_nat_icmp_reply_translation);

/* Protocol registration. */
int ip_nat_protocol_register(struct ip_nat_protocol *proto)
{
	int ret = 0;

	write_lock_bh(&ip_nat_lock);
	if (ip_nat_protos[proto->protonum] != &ip_nat_unknown_protocol) {
		ret = -EBUSY;
		goto out;
	}
	ip_nat_protos[proto->protonum] = proto;
 out:
	write_unlock_bh(&ip_nat_lock);
	return ret;
}
EXPORT_SYMBOL(ip_nat_protocol_register);

/* Noone stores the protocol anywhere; simply delete it. */
void ip_nat_protocol_unregister(struct ip_nat_protocol *proto)
{
	write_lock_bh(&ip_nat_lock);
	ip_nat_protos[proto->protonum] = &ip_nat_unknown_protocol;
	write_unlock_bh(&ip_nat_lock);

	/* Someone could be still looking at the proto in a bh. */
	synchronize_net();
}
EXPORT_SYMBOL(ip_nat_protocol_unregister);

#if defined(CONFIG_IP_NF_CONNTRACK_NETLINK) || \
    defined(CONFIG_IP_NF_CONNTRACK_NETLINK_MODULE)
int
ip_nat_port_range_to_nfattr(struct sk_buff *skb, 
			    const struct ip_nat_range *range)
{
	NFA_PUT(skb, CTA_PROTONAT_PORT_MIN, sizeof(u_int16_t),
		&range->min.tcp.port);
	NFA_PUT(skb, CTA_PROTONAT_PORT_MAX, sizeof(u_int16_t),
		&range->max.tcp.port);

	return 0;

nfattr_failure:
	return -1;
}

int
ip_nat_port_nfattr_to_range(struct nfattr *tb[], struct ip_nat_range *range)
{
	int ret = 0;
	
	/* we have to return whether we actually parsed something or not */

	if (tb[CTA_PROTONAT_PORT_MIN-1]) {
		ret = 1;
		range->min.tcp.port = 
			*(u_int16_t *)NFA_DATA(tb[CTA_PROTONAT_PORT_MIN-1]);
	}
	
	if (!tb[CTA_PROTONAT_PORT_MAX-1]) {
		if (ret) 
			range->max.tcp.port = range->min.tcp.port;
	} else {
		ret = 1;
		range->max.tcp.port = 
			*(u_int16_t *)NFA_DATA(tb[CTA_PROTONAT_PORT_MAX-1]);
	}

	return ret;
}
EXPORT_SYMBOL_GPL(ip_nat_port_nfattr_to_range);
EXPORT_SYMBOL_GPL(ip_nat_port_range_to_nfattr);
#endif

static int __init ip_nat_init(void)
{
	size_t i;

#ifdef CONFIG_NK_DYNAMIC_PORT_NUM
// 	printk(KERN_EMERG "Debug %s #%d %s: useSwitch1450[%d]/useSwitch1100[%d], CONFIG_NK_NUM_LAN[%d], CONFIG_NK_NUM_WAN[%d], CONFIG_NK_NUM_DMZ[%d]\n", __FILE__, __LINE__, __func__, useSwitch1450, useSwitch1100, CONFIG_NK_NUM_LAN, CONFIG_NK_NUM_WAN, CONFIG_NK_NUM_DMZ);
#endif

	/* Leave them the same for the moment. */
	ip_nat_htable_size = ip_conntrack_htable_size;

	/* One vmalloc for both hash tables */
	bysource = vmalloc(sizeof(struct list_head) * ip_nat_htable_size);
	if (!bysource)
		return -ENOMEM;

	/* Sew in builtin protocols. */
	write_lock_bh(&ip_nat_lock);
	for (i = 0; i < MAX_IP_NAT_PROTO; i++)
		ip_nat_protos[i] = &ip_nat_unknown_protocol;
	ip_nat_protos[IPPROTO_TCP] = &ip_nat_protocol_tcp;
	ip_nat_protos[IPPROTO_UDP] = &ip_nat_protocol_udp;
	ip_nat_protos[IPPROTO_ICMP] = &ip_nat_protocol_icmp;
	write_unlock_bh(&ip_nat_lock);

	for (i = 0; i < ip_nat_htable_size; i++) {
		INIT_LIST_HEAD(&bysource[i]);
	}

	/* FIXME: Man, this is a hack.  <SIGH> */
	IP_NF_ASSERT(ip_conntrack_destroyed == NULL);
	ip_conntrack_destroyed = &ip_nat_cleanup_conntrack;

	/* Initialize fake conntrack so that NAT will skip it */
	ip_conntrack_untracked.status |= IPS_NAT_DONE_MASK;
	
	//Rain add for Anti-Virus -->
#ifdef CONFIG_NK_ANTI_VIRUS
	tasklet_init(&clamav_task.tasklet, clamav_dequeue, (unsigned long)&clamav_task);
	nlfd = netlink_kernel_create(NL_CLAMAV, 0, kernel_receive, THIS_MODULE);

	if(!nlfd)
	{
		printk(KERN_EMERG "can not create a netlink socket\n");
		return -1;
	}
#endif
	//<--Rain
	
	return 0;
}

/* Clear NAT section of all conntracks, in case we're loaded again. */
static int clean_nat(struct ip_conntrack *i, void *data)
{
	memset(&i->nat, 0, sizeof(i->nat));
	i->status &= ~(IPS_NAT_MASK | IPS_NAT_DONE_MASK | IPS_SEQ_ADJUST);
	return 0;
}

static void __exit ip_nat_cleanup(void)
{
	ip_ct_iterate_cleanup(&clean_nat, NULL);
	ip_conntrack_destroyed = NULL;
	vfree(bysource);
}

MODULE_LICENSE("GPL");

module_init(ip_nat_init);
module_exit(ip_nat_cleanup);
