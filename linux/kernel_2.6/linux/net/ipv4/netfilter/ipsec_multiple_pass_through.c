/**
	include file
**/
#include <linux/netfilter_ipv4/ipsec_multiple_pass_through.h>
#include <linux/ip.h>
#include <linux/udp.h>

/* for LIST_FIND */
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_ipv4/listhelp.h>

/* incifer 2008/08 */
#include <linux/nkdef.h>

/**
	Macro
**/
/* Debug Message Option */
#if 0
	#define DEBUGP(arg...) printk ( KERN_EMERG arg )
#else
	#define DEBUGP(format, arg...)
#endif


/**
	define global variable
**/
static LIST_HEAD(trigger_isakmp);
/* for using trigger_isakmp */
static DEFINE_SPINLOCK(ip_isakmp_lock);


/**
	updata trig's timeout
**/
static void isakmp_refresh(struct isakmp_trigger *trig, unsigned long extra_jiffies)
{
	spin_lock_bh(&ip_isakmp_lock);

	if (del_timer(&trig->timeout)) 
	{
		trig->timeout.expires = jiffies + extra_jiffies;
		add_timer(&trig->timeout);
	}

	spin_unlock_bh(&ip_isakmp_lock);
}

/**
	remove trig from trigger_list, and free it
**/
static void __del_isakmp(struct isakmp_trigger *trig)
{
    list_del(&trig->list);
    kfree(trig);
}

/**
	trigger_list timeout function
**/
static void isakmp_timeout(unsigned long trig)
{
    struct isakmp_trigger *isakmp_trig = (struct isakmp_trigger *)trig;

    DEBUGP("*** %s: trigger list %p timeout: sip[%u.%u.%u.%u] -> dip[%u.%u.%u.%u] init_cookie[0x%x-%x].\n", __func__,
			isakmp_trig,
			NIPQUAD(isakmp_trig->srcip),
			NIPQUAD(isakmp_trig->dstip),
			isakmp_trig->init_cookie[0],
			isakmp_trig->init_cookie[1]);

	spin_lock_bh(&ip_isakmp_lock);

    __del_isakmp(isakmp_trig);

	spin_unlock_bh(&ip_isakmp_lock);
}

static unsigned int add_new_isakmp(struct isakmp_trigger *trig)
{
	struct isakmp_trigger *isakmp_new;

	isakmp_new = (struct isakmp_trigger *)
				kmalloc(sizeof(struct isakmp_trigger), GFP_ATOMIC);

	spin_lock_bh(&ip_isakmp_lock);
	if (!isakmp_new)
	{
		spin_unlock_bh(&ip_isakmp_lock);
		DEBUGP("*** %s: failed allocate trigger list\n", __func__);
		return -ENOMEM;
	}
	
	DEBUGP("*** %s: add new trigger: sip[%u.%u.%u.%u], dip[%u.%u.%u.%u], init_cookie =[%x%x]\n", __func__,
			NIPQUAD(trig->srcip),
			NIPQUAD(trig->dstip),
			trig->init_cookie[0],
			trig->init_cookie[1]);
	
	memset(isakmp_new, 0, sizeof(*trig));
	INIT_LIST_HEAD(&isakmp_new->list);
	memcpy(isakmp_new, trig, sizeof(*trig));

	/* add to global table of trigger */
	list_add(&isakmp_new->list, &trigger_isakmp);	
	
	/* add and start timer if required */
	init_timer(&isakmp_new->timeout);
	isakmp_new->timeout.data = (unsigned long)isakmp_new;
	isakmp_new->timeout.function = isakmp_timeout;
	isakmp_new->timeout.expires = jiffies + (ISAKMP_TIMEOUT * HZ);
	add_timer(&isakmp_new->timeout);
	spin_unlock_bh(&ip_isakmp_lock);

	return 0;
}

int isakmp_packet_out_matched(const struct isakmp_trigger *i,
		const u_int32_t init_cookie1,
		const u_int32_t init_cookie2)
{
    DEBUGP("*** %s: i=%p.\n keep [0x%x-%x] -- new[0x%x-%x].\n", __func__, i,
		i->init_cookie[0],i->init_cookie[1],
		init_cookie1, init_cookie2);

    return ((i->init_cookie[0] == init_cookie1) && 
    			(i->init_cookie[1] == init_cookie2));
}

unsigned int isakmp_packet_out(const struct isakmp_hdr *isakmp_h,
		struct ip_conntrack *ct,
		enum ip_conntrack_info ctinfo)
{
	struct isakmp_trigger trig, *found;

	/* Check if the trigger range has already existed in 'trigger_list'. */
	found = LIST_FIND(&trigger_isakmp, isakmp_packet_out_matched,
			struct isakmp_trigger *, isakmp_h->init_cookie[0], isakmp_h->init_cookie[1]);

	if(found)
	{
		DEBUGP("*** %s: list found then refresh it\n", __func__);
		isakmp_refresh(found, ISAKMP_TIMEOUT * HZ);
	}
	else
	{
		DEBUGP("*** %s: list not found then create new trigger\n", __func__);
		/* Create new trigger */
		memset(&trig, 0, sizeof(trig));
		trig.srcip = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip;
		trig.dstip = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip;
		trig.init_cookie[0] = isakmp_h->init_cookie[0];
		trig.init_cookie[1] = isakmp_h->init_cookie[1];
		/* Add the new 'trig' to list 'trigger_list'. */
		add_new_isakmp(&trig);
	}

	return NF_ACCEPT;
}

int isakmp_packet_in_matched(const struct isakmp_trigger *i,
		const u_int32_t init_cookie1,
		const u_int32_t init_cookie2)
{
    DEBUGP("*** %s: i=%p.\n keep [0x%x-%x] -- new[0x%x-%x].\n", __func__, i,
			i->init_cookie[0],i->init_cookie[1],
			init_cookie1, init_cookie2);
	
    return ((i->init_cookie[0] == init_cookie1) && 
    			(i->init_cookie[1] == init_cookie2));
}

unsigned int isakmp_packet_in(
		const struct isakmp_hdr *isakmp_h,
		struct ip_conntrack *ct,
		enum ip_conntrack_info ctinfo,
		u_int32_t *sip, u_int32_t *dip)
{
	struct isakmp_trigger *found;

	*sip = 0;
	*dip = 0;

	/* Check if the trigger-ed range has already existed in 'trigger_list'. */
	found = LIST_FIND(&trigger_isakmp, isakmp_packet_in_matched,
			struct isakmp_trigger *, isakmp_h->init_cookie[0], isakmp_h->init_cookie[1]);

	if(found)
	{
		DEBUGP("*** %s: list found then refresh it, sip[%u.%u.%u.%u]->dip[%u.%u.%u.%u]\n", __func__, NIPQUAD(found->srcip), NIPQUAD(found->dstip));

		/* update the destroying timer. */
		isakmp_refresh(found, ISAKMP_TIMEOUT * HZ);
		*sip = found->srcip;
		*dip = found->dstip;

		/* Accept it, or the imcoming packet could be dropped in the FORWARD chain */
		return NF_ACCEPT;
	}
	DEBUGP("*** %s: list not found\n", __func__);
	/* Our job is the interception. Not Match.*/

	return NF_DROP;
}

unsigned int ip_nat_isakmp(
		struct sk_buff **pskb,
		const struct isakmp_hdr *isakmp_h,
		enum ip_conntrack_info ctinfo,
		struct ip_conntrack *ct,
		unsigned int sip, unsigned int dip)
{
	struct iphdr *iph = (*pskb)->nh.iph;
	int dir = CTINFO2DIR(ctinfo);

	if(dir ==IP_CT_DIR_REPLY)
	{
		DEBUGP("%s: before NAT: saddr[%u.%u.%u.%u], daddr[%u.%u.%u.%u]\n", __func__,
			NIPQUAD(iph->saddr), NIPQUAD(iph->daddr));

		if (skb_cloned(*pskb) && !(*pskb)->sk)
		{
			struct sk_buff *nskb = skb_copy(*pskb, GFP_ATOMIC);
				
			if (!nskb) 
				return NF_DROP;
			
			kfree_skb(*pskb);
			*pskb = nskb;
		}

		/* change dest ip */
		iph->check = ip_nat_cheat_check(~iph->daddr, sip, iph->check);
		iph->daddr = sip;

		DEBUGP("*** %s: after NAT: sip[%u.%u.%u.%u], dip[%u.%u.%u.%u], init cookie[%x%x], next_payload[%x]\n", __func__,
			NIPQUAD(iph->saddr), NIPQUAD(iph->daddr),
			isakmp_h->init_cookie[0], isakmp_h->init_cookie[1],
			isakmp_h->next_payload);
	}

	return NF_ACCEPT;
}

/**
	@ctinfo:	nf_conntrack_common.h
				0: IP_CT_ESTABLISHED
				1: IP_CT_RELATED
				2: IP_CT_NEW
				3: IP_CT_IS_REPLY
				4: IP_CT_NUMBER
	@CTINFO2DIR(ctinfo):
				0: IP_CT_DIR_ORIGINAL
				1: IP_CT_DIR_REPLY
	@hooknum:	netfilter_ipv4.h
				0: NF_IP_PRE_ROUTING
				1: NF_IP_LOCAL_IN
				2: NF_IP_FORWARD
				3: NF_IP_LOCAL_OUT
				4: NF_IP_POST_ROUTING
	@HOOK2MANIP(hooknum)
				0: IP_NAT_MANIP_SRC
				1: IP_NAT_MANIP_DST
**/
int isakmp_help(
	struct ip_conntrack *ct,
	enum ip_conntrack_info ctinfo,
	unsigned int hooknum,
	struct sk_buff **pskb)
{
	struct iphdr *iph = (*pskb)->nh.iph;
	struct udphdr *udph = (void *)iph + iph->ihl * 4;
	struct isakmp_hdr _isakmp_h, *isakmp_h;
	int dir = CTINFO2DIR(ctinfo);
	unsigned int sip = 0 , dip = 0;
	int ret=0;

	DEBUGP("<--\n*** %s:\n", __func__);

	/* incifer 2008/08 */
	if(dir == IP_CT_DIR_ORIGINAL && ct->in_device && strcmp(ct->in_device->name, NK_LAN_IF))
		goto EXIT;
	
	isakmp_h = skb_header_pointer(*pskb, (*pskb)->nh.iph->ihl*4 + sizeof(struct udphdr), sizeof(_isakmp_h), &_isakmp_h);

	if(isakmp_h == NULL)
		goto EXIT;

#if 0
	udph->source = htons(500); /* Charles: for IPSec Passthrough, I don't need to change source port */
#endif

	if(dir == IP_CT_DIR_ORIGINAL)
	{
		DEBUGP("*** %s: dir[%d] == IP_CT_DIR_ORIGINAL\n", __func__, ctinfo);
		DEBUGP("*** %s: skb [%u.%u.%u.%u]/[%u] -> dip[%u.%u.%u.%u]/[%u], isakmp_h->init cookie[%x/%x]\n", __func__,
			NIPQUAD(iph->saddr), udph->source,
			NIPQUAD(iph->daddr), udph->dest,
			isakmp_h->init_cookie[0], isakmp_h->init_cookie[1]);
		DEBUGP("*** %s: tuple orig[%u.%u.%u.%u]/[%u] -> dip[%u.%u.%u.%u]/[%u], isakmp_h->init cookie[%x/%x]\n", __func__,
			NIPQUAD(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip), ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u.udp.port,
			NIPQUAD(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip), ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.udp.port,
			isakmp_h->init_cookie[0], isakmp_h->init_cookie[1]);
		DEBUGP("*** %s: tuple reply[%u.%u.%u.%u]/[%u] -> dip[%u.%u.%u.%u]/[%u], isakmp_h->init cookie[%x/%x]\n", __func__,
			NIPQUAD(ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.ip), ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.u.udp.port,
			NIPQUAD(ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.ip), ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u.udp.port,
			isakmp_h->init_cookie[0], isakmp_h->init_cookie[1]);

		ret = isakmp_packet_out(isakmp_h,ct,ctinfo);
	}
	else if(dir == IP_CT_DIR_REPLY)
	{
		DEBUGP("*** %s: dir[%d] == IP_CT_DIR_REPLY\n", __func__, ctinfo);
		DEBUGP("*** %s: skb [%u.%u.%u.%u]/[%u] -> dip[%u.%u.%u.%u]/[%u], isakmp_h->init cookie[%x/%x]\n", __func__,
			NIPQUAD(iph->saddr), udph->source,
			NIPQUAD(iph->daddr), udph->dest,
			isakmp_h->init_cookie[0], isakmp_h->init_cookie[1]);
		DEBUGP("*** %s: tuple reply[%u.%u.%u.%u]/[%u] -> dip[%u.%u.%u.%u]/[%u], isakmp_h->init cookie[%x/%x]\n", __func__,
			NIPQUAD(ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.ip), ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.u.udp.port,
			NIPQUAD(ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.ip), ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u.udp.port,
			isakmp_h->init_cookie[0], isakmp_h->init_cookie[1]);
		DEBUGP("*** %s: tuple orig[%u.%u.%u.%u]/[%u] -> dip[%u.%u.%u.%u]/[%u], isakmp_h->init cookie[%x/%x]\n", __func__,
			NIPQUAD(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip), ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u.udp.port,
			NIPQUAD(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip), ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.udp.port,
			isakmp_h->init_cookie[0], isakmp_h->init_cookie[1]);

		ret = isakmp_packet_in(isakmp_h,ct,ctinfo,&sip,&dip);

		if(ret == NF_DROP)
			goto EXIT;

        /* change dst port when reply */
		udph->dest = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u.udp.port;
		ip_nat_isakmp(pskb, isakmp_h, ctinfo, ct, sip, dip);
	}
	DEBUGP("-->\n*** %s:\n", __func__);

EXIT:
	return ret;
}
