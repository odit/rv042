/* Connection state tracking for netfilter.  This is separated from,
   but required by, the NAT layer; it can also be used by an iptables
   extension. */

/* (C) 1999-2001 Paul `Rusty' Russell  
 * (C) 2002-2004 Netfilter Core Team <coreteam@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * 23 Apr 2001: Harald Welte <laforge@gnumonks.org>
 * 	- new API and handling of conntrack/nat helpers
 * 	- now capable of multiple expectations for one master
 * 16 Jul 2002: Harald Welte <laforge@gnumonks.org>
 * 	- add usage/reference counts to ip_conntrack_expect
 *	- export ip_conntrack[_expect]_{find_get,put} functions
 * */

#include <linux/config.h>
#include <linux/types.h>
#include <linux/icmp.h>
#include <linux/ip.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/proc_fs.h>
#include <linux/vmalloc.h>
#include <net/checksum.h>
#include <net/ip.h>
#include <linux/stddef.h>
#include <linux/sysctl.h>
#include <linux/slab.h>
#include <linux/random.h>
#include <linux/jhash.h>
#include <linux/err.h>
#include <linux/percpu.h>
#include <linux/moduleparam.h>
#include <linux/notifier.h>

/* ip_conntrack_lock protects the main hash table, protocol/helper/expected
   registrations, conntrack timers*/
#define ASSERT_READ_LOCK(x)
#define ASSERT_WRITE_LOCK(x)

#include <linux/netfilter_ipv4/ip_conntrack.h>
#include <linux/netfilter_ipv4/ip_conntrack_protocol.h>
#include <linux/netfilter_ipv4/ip_conntrack_helper.h>
#include <linux/netfilter_ipv4/ip_conntrack_core.h>
#include <linux/netfilter_ipv4/listhelp.h>
#include <linux/netfilter_ipv4/ioctl.h>
#include <linux/sw_template_cpld.c>

/**
 *	Global Variable 
 */
#ifdef CONFIG_NK_DYNAMIC_PORT_NUM
	/* Switch Type */
	int useSwitch;
	EXPORT_SYMBOL(useSwitch);
	int Start_Check_Reg=0;
	EXPORT_SYMBOL(Start_Check_Reg);

	#define SWITCH005		5
	#define SWITCH363		363
	#define SWITCH650		650
	#define SWITCH1100		1100
	#define SWITCH1150		1150
	#define SWITCH1450		1450
	#define SWITCH2000		2000
	#define SWITCH2050		2050
	#define SWITCH2100		2100
	#define SWITCH3000		3000
	#define SWITCHUNKNOW		9999
	
	int DYNAMIC_NUM_LAN;
	EXPORT_SYMBOL(DYNAMIC_NUM_LAN);
	int DYNAMIC_NUM_WAN;
	EXPORT_SYMBOL(DYNAMIC_NUM_WAN);
	int DYNAMIC_NUM_DMZ;
	EXPORT_SYMBOL(DYNAMIC_NUM_DMZ);
	int DYNAMIC_MAX_SESSION;
	EXPORT_SYMBOL(DYNAMIC_MAX_SESSION);
	int DYNAMIC_HASH_SIZE;
	EXPORT_SYMBOL(DYNAMIC_HASH_SIZE);

	#undef CONFIG_NK_NUM_LAN
	#define CONFIG_NK_NUM_LAN	DYNAMIC_NUM_LAN
	#undef CONFIG_NK_NUM_WAN
	#define CONFIG_NK_NUM_WAN	DYNAMIC_NUM_WAN
	#undef CONFIG_NK_NUM_DMZ
	#define CONFIG_NK_NUM_DMZ	DYNAMIC_NUM_DMZ
#endif

#define IP_CONNTRACK_VERSION	"2.4"

#if 0
#define DEBUGP printk
#else
#define DEBUGP(format, args...)
#endif

#define DEBUG 0

#define SORTING_ENTRY 20
#define MAX_WAN_NUM (CONFIG_NK_NUM_WAN+CONFIG_NK_NUM_DMZ)
//#define CAL_CNT_PERIOD 60
#ifdef CONFIG_NK_DYNAMIC_PORT_NUM
extern interface_session_t wan_interface_session[CONFIG_NK_NUM_MAX_WAN];
#else
extern interface_session_t wan_interface_session[MAX_WAN_NUM];
#endif

#ifdef CONFIG_NK_SESSION_LIMIT
extern u_int32_t session_limit_status;
extern unsigned int days_match;
extern unsigned int time_start;
extern unsigned int time_stop;
extern unsigned int scheduler;
extern session_limit_t *nk_session_limit[128];
#endif
extern smart_qos_ip_t *nk_smart_qos_ip_list;
smart_qos_ip_t *tmp_smart_qos_ip_list=NULL;

#ifdef CONFIG_NK_IPFILTER_SUPPORT_SORTING
void nk_update_delta_count(void);
void update_delta_timer_set(void);
static struct timer_list update_delta_timer = {
	function:&nk_update_delta_count
};
#ifdef CONFIG_NK_DYNAMIC_PORT_NUM
int session_num_pre[CONFIG_NK_NUM_MAX_WAN], session_num[CONFIG_NK_NUM_MAX_WAN], session_num_tmp[CONFIG_NK_NUM_MAX_WAN];
#else
int session_num_pre[MAX_WAN_NUM], session_num[MAX_WAN_NUM], session_num_tmp[MAX_WAN_NUM];
#endif

int hashtmp_size=0;
#endif

extern struct firewall_setting fw_setting;

//20090520
extern ip_mac_bind_entry_t *nk_ip_mac_bind_entry_list[256];
extern ip_mac_bind_entry_t *nk_ip_mac_learn_list[256];

//20091218
extern nk_agingtime_t *nk_nk_agingtime_list[128];

#ifdef CONFIG_NK_RESTRICT_APP
#include <net/tcp.h>
#define LIMIT_FOR_CHANGE_ORDER 10
unsigned int check_block_qq(struct sk_buff **pskb);
extern restrict_app_t restrict_app;
extern exception_qq_t *exception_qq_list;
typedef union {
  unsigned int iNum;
  char cNum[4];
} qq_number_t;
#endif

DEFINE_RWLOCK(ip_conntrack_lock);

/* ip_conntrack_standalone needs this */
atomic_t ip_conntrack_count = ATOMIC_INIT(0);

void (*ip_conntrack_destroyed)(struct ip_conntrack *conntrack) = NULL;
LIST_HEAD(ip_conntrack_expect_list);
struct ip_conntrack_protocol *ip_ct_protos[MAX_IP_CT_PROTO];
static LIST_HEAD(helpers);
unsigned int ip_conntrack_htable_size = 0;
int ip_conntrack_max;
struct list_head *ip_conntrack_hash;
static kmem_cache_t *ip_conntrack_cachep __read_mostly;
static kmem_cache_t *ip_conntrack_expect_cachep __read_mostly;
struct ip_conntrack ip_conntrack_untracked;
unsigned int ip_ct_log_invalid;
static LIST_HEAD(unconfirmed);
static int ip_conntrack_vmalloc;

static unsigned int ip_conntrack_next_id = 1;
static unsigned int ip_conntrack_expect_next_id = 1;
#ifdef CONFIG_IP_NF_CONNTRACK_EVENTS
struct notifier_block *ip_conntrack_chain;
struct notifier_block *ip_conntrack_expect_chain;

DEFINE_PER_CPU(struct ip_conntrack_ecache, ip_conntrack_ecache);

/* deliver cached events and clear cache entry - must be called with locally
 * disabled softirqs */
static inline void
__ip_ct_deliver_cached_events(struct ip_conntrack_ecache *ecache)
{
	DEBUGP("ecache: delivering events for %p\n", ecache->ct);
	if (is_confirmed(ecache->ct) && !is_dying(ecache->ct) && ecache->events)
		notifier_call_chain(&ip_conntrack_chain, ecache->events,
				    ecache->ct);
	ecache->events = 0;
	ip_conntrack_put(ecache->ct);
	ecache->ct = NULL;
}

/* Deliver all cached events for a particular conntrack. This is called
 * by code prior to async packet handling or freeing the skb */
void ip_ct_deliver_cached_events(const struct ip_conntrack *ct)
{
	struct ip_conntrack_ecache *ecache;
	
	local_bh_disable();
	ecache = &__get_cpu_var(ip_conntrack_ecache);
	if (ecache->ct == ct)
		__ip_ct_deliver_cached_events(ecache);
	local_bh_enable();
}

void __ip_ct_event_cache_init(struct ip_conntrack *ct)
{
	struct ip_conntrack_ecache *ecache;

	/* take care of delivering potentially old events */
	ecache = &__get_cpu_var(ip_conntrack_ecache);
	BUG_ON(ecache->ct == ct);
	if (ecache->ct)
		__ip_ct_deliver_cached_events(ecache);
	/* initialize for this conntrack/packet */
	ecache->ct = ct;
	nf_conntrack_get(&ct->ct_general);
}

/* flush the event cache - touches other CPU's data and must not be called while
 * packets are still passing through the code */
static void ip_ct_event_cache_flush(void)
{
	struct ip_conntrack_ecache *ecache;
	int cpu;

	for_each_cpu(cpu) {
		ecache = &per_cpu(ip_conntrack_ecache, cpu);
		if (ecache->ct)
			ip_conntrack_put(ecache->ct);
	}
}
#else
static inline void ip_ct_event_cache_flush(void) {}
#endif /* CONFIG_IP_NF_CONNTRACK_EVENTS */

DEFINE_PER_CPU(struct ip_conntrack_stat, ip_conntrack_stat);

static int ip_conntrack_hash_rnd_initted;
static unsigned int ip_conntrack_hash_rnd;

static u_int32_t __hash_conntrack(const struct ip_conntrack_tuple *tuple,
			    unsigned int size, unsigned int rnd)
{
	return (jhash_3words(tuple->src.ip,
	                     (tuple->dst.ip ^ tuple->dst.protonum),
	                     (tuple->src.u.all | (tuple->dst.u.all << 16)),
	                     rnd) % size);
}

static u_int32_t
hash_conntrack(const struct ip_conntrack_tuple *tuple)
{
	return __hash_conntrack(tuple, ip_conntrack_htable_size,
				ip_conntrack_hash_rnd);
}

int
ip_ct_get_tuple(const struct iphdr *iph,
		const struct sk_buff *skb,
		unsigned int dataoff,
		struct ip_conntrack_tuple *tuple,
		const struct ip_conntrack_protocol *protocol)
{
	/* Never happen */
	if (iph->frag_off & htons(IP_OFFSET)) {
		printk("ip_conntrack_core: Frag of proto %u.\n",
		       iph->protocol);
		return 0;
	}

	tuple->src.ip = iph->saddr;
	tuple->dst.ip = iph->daddr;
	tuple->dst.protonum = iph->protocol;
	tuple->dst.dir = IP_CT_DIR_ORIGINAL;

	return protocol->pkt_to_tuple(skb, dataoff, tuple);
}

int
ip_ct_invert_tuple(struct ip_conntrack_tuple *inverse,
		   const struct ip_conntrack_tuple *orig,
		   const struct ip_conntrack_protocol *protocol)
{
	inverse->src.ip = orig->dst.ip;
	inverse->dst.ip = orig->src.ip;
	inverse->dst.protonum = orig->dst.protonum;
	inverse->dst.dir = !orig->dst.dir;

	return protocol->invert_tuple(inverse, orig);
}


/* ip_conntrack_expect helper functions */
void ip_ct_unlink_expect(struct ip_conntrack_expect *exp)
{
	ASSERT_WRITE_LOCK(&ip_conntrack_lock);
	IP_NF_ASSERT(!timer_pending(&exp->timeout));
	list_del(&exp->list);
	CONNTRACK_STAT_INC(expect_delete);
	exp->master->expecting--;
	ip_conntrack_expect_put(exp);
}

static void expectation_timed_out(unsigned long ul_expect)
{
	struct ip_conntrack_expect *exp = (void *)ul_expect;

	write_lock_bh(&ip_conntrack_lock);
	ip_ct_unlink_expect(exp);
	write_unlock_bh(&ip_conntrack_lock);
	ip_conntrack_expect_put(exp);
}

struct ip_conntrack_expect *
__ip_conntrack_expect_find(const struct ip_conntrack_tuple *tuple)
{
	struct ip_conntrack_expect *i;
	
	list_for_each_entry(i, &ip_conntrack_expect_list, list) {
		if (ip_ct_tuple_mask_cmp(tuple, &i->tuple, &i->mask)) {
			atomic_inc(&i->use);
			return i;
		}
	}
	return NULL;
}

/* Just find a expectation corresponding to a tuple. */
struct ip_conntrack_expect *
ip_conntrack_expect_find(const struct ip_conntrack_tuple *tuple)
{
	struct ip_conntrack_expect *i;
	
	read_lock_bh(&ip_conntrack_lock);
	i = __ip_conntrack_expect_find(tuple);
	read_unlock_bh(&ip_conntrack_lock);

	return i;
}

/* If an expectation for this connection is found, it gets delete from
 * global list then returned. */
static struct ip_conntrack_expect *
find_expectation(const struct ip_conntrack_tuple *tuple)
{
	struct ip_conntrack_expect *i;

	list_for_each_entry(i, &ip_conntrack_expect_list, list) {
		/* If master is not in hash table yet (ie. packet hasn't left
		   this machine yet), how can other end know about expected?
		   Hence these are not the droids you are looking for (if
		   master ct never got confirmed, we'd hold a reference to it
		   and weird things would happen to future packets). */
		if (ip_ct_tuple_mask_cmp(tuple, &i->tuple, &i->mask)
		    && is_confirmed(i->master)) {
			if (i->flags & IP_CT_EXPECT_PERMANENT) {
				atomic_inc(&i->use);
				return i;
			} else if (del_timer(&i->timeout)) {
				ip_ct_unlink_expect(i);
				return i;
			}
		}
	}
	return NULL;
}

/* delete all expectations for this conntrack */
void ip_ct_remove_expectations(struct ip_conntrack *ct)
{
	struct ip_conntrack_expect *i, *tmp;

	/* Optimization: most connection never expect any others. */
	if (ct->expecting == 0)
		return;

	list_for_each_entry_safe(i, tmp, &ip_conntrack_expect_list, list) {
		if (i->master == ct && del_timer(&i->timeout)) {
			ip_ct_unlink_expect(i);
			ip_conntrack_expect_put(i);
		}
	}
}

//static void
void
clean_from_lists(struct ip_conntrack *ct)
{
	unsigned int ho, hr;
	
	//DEBUGP("clean_from_lists(%p)\n", ct);
	//printk(KERN_EMERG "clean_from_lists(%p)\n", ct);
	ASSERT_WRITE_LOCK(&ip_conntrack_lock);

	ho = hash_conntrack(&ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple);
	hr = hash_conntrack(&ct->tuplehash[IP_CT_DIR_REPLY].tuple);
	LIST_DELETE(&ip_conntrack_hash[ho], &ct->tuplehash[IP_CT_DIR_ORIGINAL]);
	LIST_DELETE(&ip_conntrack_hash[hr], &ct->tuplehash[IP_CT_DIR_REPLY]);
	//count number of entry of each hash
	//ip_conntrack_hash[ho].num--;
	//ip_conntrack_hash[hr].num--;
	/* Destroy all pending expectations */
	ip_ct_remove_expectations(ct);
}

static void
destroy_conntrack(struct nf_conntrack *nfct)
{
	struct ip_conntrack *ct = (struct ip_conntrack *)nfct;
	struct ip_conntrack_protocol *proto;

	//printk(KERN_EMERG "destroy_conntrack(%p)\n", ct);
	//printk(KERN_EMERG "destroy_conntrack(%p)\n", ct);
	IP_NF_ASSERT(atomic_read(&nfct->use) == 0);
	IP_NF_ASSERT(!timer_pending(&ct->timeout));

	ip_conntrack_event(IPCT_DESTROY, ct);
	set_bit(IPS_DYING_BIT, &ct->status);

	/* To make sure we don't get any weird locking issues here:
	 * destroy_conntrack() MUST NOT be called with a write lock
	 * to ip_conntrack_lock!!! -HW */
	proto = __ip_conntrack_proto_find(ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.protonum);
	if (proto && proto->destroy)
		proto->destroy(ct);

	if (ip_conntrack_destroyed)
		ip_conntrack_destroyed(ct);

	write_lock_bh(&ip_conntrack_lock);
	/* Expectations will have been removed in clean_from_lists,
	 * except TFTP can create an expectation on the first packet,
	 * before connection is in the list, so we need to clean here,
	 * too. */
	ip_ct_remove_expectations(ct);

	#if defined(CONFIG_IP_NF_MATCH_LAYER7) || defined(CONFIG_IP_NF_MATCH_LAYER7_MODULE)
	if(ct->layer7.app_proto)
		kfree(ct->layer7.app_proto);
	if(ct->layer7.app_data)
		kfree(ct->layer7.app_data);
	#endif

	/* We overload first tuple to link into unconfirmed list. */
	if (!is_confirmed(ct)) {
		BUG_ON(list_empty(&ct->tuplehash[IP_CT_DIR_ORIGINAL].list));
		list_del(&ct->tuplehash[IP_CT_DIR_ORIGINAL].list);
	}

	CONNTRACK_STAT_INC(delete);
	write_unlock_bh(&ip_conntrack_lock);

	if (ct->master)
		ip_conntrack_put(ct->master);
#if 0
#ifdef CONFIG_NK_SESSION_LIMIT
    if( (ct->in_device) && (!strcmp(ct->in_device, "eth0")) && (ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip!=wan_interface_session[0].lanip) && (session_limit_status!=0) &&
	(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip>0) && (ct->slimit==1) )
    {
	//if(DEBUG) printk(KERN_EMERG "destroy ct %u.%u.%u.%u -> %u.%u.%u.%u\n", NIPQUAD(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip), NIPQUAD(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip));
	#ifdef CONFIG_NK_SESSION_LIMIT_ENHANCE /* Session Limit Enhance incifer 2006/09/03 */

		nk_session_limit_enhance_delete(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip,ntohs(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.all),ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum);
	#else
		nk_session_limit_delete(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip);
	#endif
    }
#endif
#endif

	DEBUGP("destroy_conntrack: returning ct=%p to slab\n", ct);
	//printk(KERN_EMERG "destroy_conntrack: returning ct=%p to slab\n", ct);
	ip_conntrack_free(ct);
}

static void death_by_timeout(unsigned long ul_conntrack)
{
	struct ip_conntrack *ct = (void *)ul_conntrack;

	//printk(KERN_EMERG "death_by_timeout(%p)\n", ct);

#if 0
	#ifdef CONFIG_NK_SESSION_LIMIT
	//if(DEBUG) printk(KERN_EMERG "early_drop %u.%u.%u.%u -> %u.%u.%u.%u, in_dev=%s\n", NIPQUAD(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip), NIPQUAD(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip), ct->in_device);
	if( (ct->in_device!=NULL) && (!strcmp(ct->in_device, wan_interface_session[0].laninterface)) && (session_limit_status!=0) && (ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip>0)
	  && (ct->slimit==1) && (ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip!=wan_interface_session[0].lanip))
	{
		//if(DEBUG) printk(KERN_EMERG "early_drop %u.%u.%u.%u -> %u.%u.%u.%u, in_dev=%s\n", NIPQUAD(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip), NIPQUAD(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip), ct->in_device);
	#ifdef CONFIG_NK_SESSION_LIMIT_ENHANCE /* Session Limit Enhance incifer 2006/09/03 */
		nk_session_limit_enhance_delete(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip,ntohs(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.all),ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum);
	#else
		nk_session_limit_delete(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip);
	#endif
	}
	#endif
#endif
	write_lock_bh(&ip_conntrack_lock);
	/* Inside lock so preempt is disabled on module removal path.
	 * Otherwise we can get spurious warnings. */
	CONNTRACK_STAT_INC(delete_list);
	clean_from_lists(ct);
	write_unlock_bh(&ip_conntrack_lock);
	ip_conntrack_put(ct);
}

static inline int
conntrack_tuple_cmp(const struct ip_conntrack_tuple_hash *i,
		    const struct ip_conntrack_tuple *tuple,
		    const struct ip_conntrack *ignored_conntrack)
{
	ASSERT_READ_LOCK(&ip_conntrack_lock);
	return tuplehash_to_ctrack(i) != ignored_conntrack
		&& ip_ct_tuple_equal(tuple, &i->tuple);
}

struct ip_conntrack_tuple_hash *
__ip_conntrack_find(const struct ip_conntrack_tuple *tuple,
		    const struct ip_conntrack *ignored_conntrack)
{
	struct ip_conntrack_tuple_hash *h;
	unsigned int hash = hash_conntrack(tuple);

	ASSERT_READ_LOCK(&ip_conntrack_lock);
	list_for_each_entry(h, &ip_conntrack_hash[hash], list) {
		if (conntrack_tuple_cmp(h, tuple, ignored_conntrack)) {
			CONNTRACK_STAT_INC(found);
			return h;
		}
		CONNTRACK_STAT_INC(searched);
	}

	return NULL;
}

/* Find a connection corresponding to a tuple. */
struct ip_conntrack_tuple_hash *
ip_conntrack_find_get(const struct ip_conntrack_tuple *tuple,
		      const struct ip_conntrack *ignored_conntrack)
{
	struct ip_conntrack_tuple_hash *h;

	read_lock_bh(&ip_conntrack_lock);
	h = __ip_conntrack_find(tuple, ignored_conntrack);
	if (h)
		atomic_inc(&tuplehash_to_ctrack(h)->ct_general.use);
	read_unlock_bh(&ip_conntrack_lock);

	return h;
}

static void __ip_conntrack_hash_insert(struct ip_conntrack *ct,
					unsigned int hash,
					unsigned int repl_hash) 
{
	ct->id = ++ip_conntrack_next_id;
	list_prepend(&ip_conntrack_hash[hash],
		     &ct->tuplehash[IP_CT_DIR_ORIGINAL].list);
	list_prepend(&ip_conntrack_hash[repl_hash],
		     &ct->tuplehash[IP_CT_DIR_REPLY].list);
}

void ip_conntrack_hash_insert(struct ip_conntrack *ct)
{
	unsigned int hash, repl_hash;

	hash = hash_conntrack(&ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple);
	repl_hash = hash_conntrack(&ct->tuplehash[IP_CT_DIR_REPLY].tuple);

	write_lock_bh(&ip_conntrack_lock);
	__ip_conntrack_hash_insert(ct, hash, repl_hash);
	write_unlock_bh(&ip_conntrack_lock);
}

/* Confirm a connection given skb; places it in hash table */
int
__ip_conntrack_confirm(struct sk_buff **pskb)
{
	unsigned int hash, repl_hash;
	struct ip_conntrack *ct;
	enum ip_conntrack_info ctinfo;

	ct = ip_conntrack_get(*pskb, &ctinfo);

	/* ipt_REJECT uses ip_conntrack_attach to attach related
	   ICMP/TCP RST packets in other direction.  Actual packet
	   which created connection will be IP_CT_NEW or for an
	   expected connection, IP_CT_RELATED. */
	if (CTINFO2DIR(ctinfo) != IP_CT_DIR_ORIGINAL)
		return NF_ACCEPT;

	hash = hash_conntrack(&ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple);
	repl_hash = hash_conntrack(&ct->tuplehash[IP_CT_DIR_REPLY].tuple);

	/* We're not in hash table, and we refuse to set up related
	   connections for unconfirmed conns.  But packet copies and
	   REJECT will give spurious warnings here. */
	/* IP_NF_ASSERT(atomic_read(&ct->ct_general.use) == 1); */

	/* No external references means noone else could have
           confirmed us. */
	IP_NF_ASSERT(!is_confirmed(ct));
	DEBUGP("Confirming conntrack %p\n", ct);
	//printk(KERN_EMERG "Confirming conntrack %p\n", ct);

	write_lock_bh(&ip_conntrack_lock);

	/* See if there's one in the list already, including reverse:
           NAT could have grabbed it without realizing, since we're
           not in the hash.  If there is, we lost race. */
	if (!LIST_FIND(&ip_conntrack_hash[hash],
		       conntrack_tuple_cmp,
		       struct ip_conntrack_tuple_hash *,
		       &ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple, NULL)
	    && !LIST_FIND(&ip_conntrack_hash[repl_hash],
			  conntrack_tuple_cmp,
			  struct ip_conntrack_tuple_hash *,
			  &ct->tuplehash[IP_CT_DIR_REPLY].tuple, NULL)) {
		/* Remove from unconfirmed list */
		list_del(&ct->tuplehash[IP_CT_DIR_ORIGINAL].list);

		__ip_conntrack_hash_insert(ct, hash, repl_hash);
		/* Timer relative to confirmation time, not original
		   setting time, otherwise we'd get timer wrap in
		   weird delay cases. */

		ct->timeout.expires += jiffies;
		add_timer(&ct->timeout);
		atomic_inc(&ct->ct_general.use);
		set_bit(IPS_CONFIRMED_BIT, &ct->status);
		CONNTRACK_STAT_INC(insert);
		write_unlock_bh(&ip_conntrack_lock);
		if (ct->helper)
			ip_conntrack_event_cache(IPCT_HELPER, *pskb);
#ifdef CONFIG_IP_NF_NAT_NEEDED
		if (test_bit(IPS_SRC_NAT_DONE_BIT, &ct->status) ||
		    test_bit(IPS_DST_NAT_DONE_BIT, &ct->status))
			ip_conntrack_event_cache(IPCT_NATINFO, *pskb);
#endif
		ip_conntrack_event_cache(master_ct(ct) ?
					 IPCT_RELATED : IPCT_NEW, *pskb);

		//count number of entry of each hash	
		//ip_conntrack_hash[hash].num++;
		//ip_conntrack_hash[repl_hash].num++;

		return NF_ACCEPT;
	}

	CONNTRACK_STAT_INC(insert_failed);
	write_unlock_bh(&ip_conntrack_lock);

	return NF_DROP;
}

/* Returns true if a connection correspondings to the tuple (required
   for NAT). */
int
ip_conntrack_tuple_taken(const struct ip_conntrack_tuple *tuple,
			 const struct ip_conntrack *ignored_conntrack)
{
	struct ip_conntrack_tuple_hash *h;

	read_lock_bh(&ip_conntrack_lock);
	h = __ip_conntrack_find(tuple, ignored_conntrack);
	read_unlock_bh(&ip_conntrack_lock);

	return h != NULL;
}

/* There's a small race here where we may free a just-assured
   connection.  Too bad: we're in trouble anyway. */
static inline int unreplied(const struct ip_conntrack_tuple_hash *i)
{
	return !(test_bit(IPS_ASSURED_BIT, &tuplehash_to_ctrack(i)->status));
}

static int early_drop(struct list_head *chain)
{
	/* Traverse backwards: gives us oldest, which is roughly LRU */
	struct ip_conntrack_tuple_hash *h;
	struct ip_conntrack *ct = NULL;
	int dropped = 0;

	read_lock_bh(&ip_conntrack_lock);
	h = LIST_FIND_B(chain, unreplied, struct ip_conntrack_tuple_hash *);
	if (h) {
		ct = tuplehash_to_ctrack(h);
		atomic_inc(&ct->ct_general.use);
	}
	read_unlock_bh(&ip_conntrack_lock);

	if (!ct)
		return dropped;

	if (del_timer(&ct->timeout)) {
		death_by_timeout((unsigned long)ct);
		dropped = 1;
		CONNTRACK_STAT_INC(early_drop);
	}
	ip_conntrack_put(ct);
	return dropped;
}

static inline int helper_cmp(const struct ip_conntrack_helper *i,
			     const struct ip_conntrack_tuple *rtuple)
{
	return ip_ct_tuple_mask_cmp(rtuple, &i->tuple, &i->mask);
}

static struct ip_conntrack_helper *
__ip_conntrack_helper_find( const struct ip_conntrack_tuple *tuple)
{
	return LIST_FIND(&helpers, helper_cmp,
			 struct ip_conntrack_helper *,
			 tuple);
}

struct ip_conntrack_helper *
ip_conntrack_helper_find_get( const struct ip_conntrack_tuple *tuple)
{
	struct ip_conntrack_helper *helper;

	/* need ip_conntrack_lock to assure that helper exists until
	 * try_module_get() is called */
	read_lock_bh(&ip_conntrack_lock);

	helper = __ip_conntrack_helper_find(tuple);
	if (helper) {
		/* need to increase module usage count to assure helper will
		 * not go away while the caller is e.g. busy putting a
		 * conntrack in the hash that uses the helper */
		if (!try_module_get(helper->me))
			helper = NULL;
	}

	read_unlock_bh(&ip_conntrack_lock);

	return helper;
}

void ip_conntrack_helper_put(struct ip_conntrack_helper *helper)
{
	module_put(helper->me);
}

struct ip_conntrack_protocol *
__ip_conntrack_proto_find(u_int8_t protocol)
{
	return ip_ct_protos[protocol];
}

/* this is guaranteed to always return a valid protocol helper, since
 * it falls back to generic_protocol */
struct ip_conntrack_protocol *
ip_conntrack_proto_find_get(u_int8_t protocol)
{
	struct ip_conntrack_protocol *p;

	preempt_disable();
	p = __ip_conntrack_proto_find(protocol);
	if (p) {
		if (!try_module_get(p->me))
			p = &ip_conntrack_generic_protocol;
	}
	preempt_enable();
	
	return p;
}

void ip_conntrack_proto_put(struct ip_conntrack_protocol *p)
{
	module_put(p->me);
}

struct ip_conntrack *ip_conntrack_alloc(struct ip_conntrack_tuple *orig,
					struct ip_conntrack_tuple *repl)
{
	struct ip_conntrack *conntrack;

	if (!ip_conntrack_hash_rnd_initted) {
		get_random_bytes(&ip_conntrack_hash_rnd, 4);
		ip_conntrack_hash_rnd_initted = 1;
	}

	if (ip_conntrack_max
	    && atomic_read(&ip_conntrack_count) >= ip_conntrack_max) {
		unsigned int hash = hash_conntrack(orig);
		/* Try dropping from this hash chain. */
		if (!early_drop(&ip_conntrack_hash[hash])) {
			if (net_ratelimit())
				printk(KERN_EMERG
				       "ip_conntrack: table full, dropping"
				       " packet.\n");
			return ERR_PTR(-ENOMEM);
		}
	}

	conntrack = kmem_cache_alloc(ip_conntrack_cachep, GFP_ATOMIC);
	if (!conntrack) {
		printk(KERN_EMERG "Can't allocate conntrack.\n");
		return ERR_PTR(-ENOMEM);
	}

	memset(conntrack, 0, sizeof(*conntrack));
	atomic_set(&conntrack->ct_general.use, 1);
	conntrack->ct_general.destroy = destroy_conntrack;
	conntrack->tuplehash[IP_CT_DIR_ORIGINAL].tuple = *orig;
	conntrack->tuplehash[IP_CT_DIR_REPLY].tuple = *repl;
	conntrack->tr.query=0;
	conntrack->tr.update=0;
	conntrack->tr.flag=0;

	/* Don't set timer yet: wait for confirmation */
	init_timer(&conntrack->timeout);
	conntrack->timeout.data = (unsigned long)conntrack;
	conntrack->timeout.function = death_by_timeout;
#ifdef CONFIG_NK_SESSION_LIMIT
	if(session_limit_status!=0)
		conntrack->slimit=1;
	else conntrack->slimit=0;
	conntrack->slimit2=0;
#endif
	atomic_inc(&ip_conntrack_count);

	return conntrack;
}

void
ip_conntrack_free(struct ip_conntrack *conntrack)
{

	//printk(KERN_EMERG "ip_conntrack_free(%p)\n", conntrack);
#if 1
#ifdef CONFIG_NK_SESSION_LIMIT
//    if( (conntrack->in_device) && (conntrack->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip!=wan_interface_session[0].lanip) && (session_limit_status!=0) &&
//	(conntrack->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip>0) && (conntrack->slimit==1) )
//    if( (conntrack->in_device!=NULL) && (!strcmp(conntrack->in_device, wan_interface_session[0].laninterface)) && (session_limit_status!=0) && (conntrack->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip>0) && (conntrack->slimit==1) )
    if( (session_limit_status!=0) && (conntrack->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip>0) && (conntrack->slimit==1) && (conntrack->slimit2==1))

    {
	//if(DEBUG) printk(KERN_EMERG "destroy ct %u.%u.%u.%u -> %u.%u.%u.%u\n", NIPQUAD(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip), NIPQUAD(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip));
	#ifdef CONFIG_NK_SESSION_LIMIT_ENHANCE /* Session Limit Enhance incifer 2006/09/03 */

		nk_session_limit_enhance_delete(conntrack->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip,ntohs(conntrack->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.all),conntrack->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum);
	#else
		nk_session_limit_delete(conntrack->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip);
	#endif
    }
#endif
#endif
	memset(conntrack, 0, sizeof(struct ip_conntrack));
	atomic_dec(&ip_conntrack_count);
	kmem_cache_free(ip_conntrack_cachep, conntrack);
}

/* Allocate a new conntrack: we return -ENOMEM if classification
 * failed due to stress.   Otherwise it really is unclassifiable */
static struct ip_conntrack_tuple_hash *
init_conntrack(struct ip_conntrack_tuple *tuple,
	       struct ip_conntrack_protocol *protocol,
	       struct sk_buff *skb)
{
	struct ip_conntrack *conntrack;
	struct ip_conntrack_tuple repl_tuple;
	struct ip_conntrack_expect *exp;
	struct sockaddr_in src_ip, dst_ip, lanmask;
	int idx;
#ifdef CONFIG_NK_SESSION_LIMIT
	session_limit_t *session_limit_p;
#endif
	
	if (!ip_ct_invert_tuple(&repl_tuple, tuple, protocol)) {
		DEBUGP("Can't invert tuple.\n");
		return NULL;
	}
#ifdef CONFIG_SUPPORT_IP_MAC_BINDING
	//printk("conntrack->in_device=%s\n", conntrack->in_device);
	//strcpy(wan_interface_session[0].laninterface, "eth0");
	/** Debug -- incifer 2009/03 **/
// 	printk(KERN_EMERG "Debug %s: Manip: %u.%u.%u.%u, Dst IP: %u.%u.%u.%u\n", __func__, NIPQUAD(tuple->src.ip), NIPQUAD(tuple->dst.ip));
// 	printk(KERN_EMERG "Debug %s: Protocol Num[%d], Protocol Name[%s]\n", __func__, protocol->proto, protocol->name);
// 	printk(KERN_EMERG "Debug %s: Dev[%s], Src IP: %u.%u.%u.%u Dst IP: %u.%u.%u.%u\n", __func__, skb->dev, NIPQUAD(skb->nh.iph->saddr), NIPQUAD(skb->nh.iph->daddr));
// 	printk(KERN_EMERG "Debug %s: laninterface: %s, lanip: %u.%u.%u.%u\n", __func__, wan_interface_session[0].laninterface, NIPQUAD(wan_interface_session[0].lanip));
// 	printk(KERN_EMERG "-----------------------------------------------------------------------\n");
	if(skb->dev!=NULL)
        {
                //printk("nk_ip_mac_binding_check:laninterface=%s\n", wan_interface_session[0].laninterface);
		if(!strcmp(skb->dev, wan_interface_session[0].laninterface) && 
			protocol->proto != 50 &&/* ESP[50], we dont need check ESP Pkt or Function: Show New IP will occur error -- incifer 2009/03 */
		  (skb->nh.iph->saddr>0) && (skb->nh.iph->daddr!=wan_interface_session[0].lanip) )
                        if(nk_ip_mac_binding_check(skb, skb->nh.iph->saddr)==-1) return ERR_PTR(-ENOMEM);
        }
#endif

/*2007/03/12/, Add session limit check, Yami*/
#ifdef CONFIG_NK_SESSION_LIMIT
//printk(KERN_EMERG "port=%d\n", ntohs(tuple->dst.u.tcp.port));
    if ( (skb->dev!=NULL) && (!strcmp(skb->dev, wan_interface_session[0].laninterface)) && (skb->nh.iph->daddr!=wan_interface_session[0].lanip) && (session_limit_status!=0) &&
	(skb->nh.iph->saddr>0) )
    {
   	//printk(KERN_EMERG "1. do session limit check  src=%u.%u.%u.%u dst=%u.%u.%u.%u, lanip=%u.%u.%u.%u\n", NIPQUAD(skb->nh.iph->saddr), NIPQUAD(skb->nh.iph->daddr), 
	//		NIPQUAD(wan_interface_session[0].lanip) );
	#ifdef CONFIG_NK_SESSION_LIMIT_ENHANCE
//	if(((skb->nh.iph->saddr)&(wan_interface_session[0].lanmask)) == ((skb->nh.iph->daddr)&(wan_interface_session[0].lanmask)))
//	{
		//printk(KERN_EMERG "LAN <-> LAN session ! %u.%u.%u.%u -> %u.%u.%u.%u\n", NIPQUAD(skb->nh.iph->saddr), NIPQUAD(skb->nh.iph->daddr));
//	}
//	else
//	{
	
//		if(session_limit_enhance_rule_check(skb->nh.iph->saddr, ntohs(tuple->dst.u.tcp.port), skb->nh.iph->protocol)==1)
//		{
//			if(DEBUG) printk(KERN_EMERG "session_limit_enhance_rule_check EXCEPTION, ACCEPT\n");
//		}
//		else
//		{
			if(nk_session_limit_enhance_checking(skb->nh.iph->saddr, ntohs(tuple->dst.u.tcp.port), skb->nh.iph->protocol)==-1)
				return ERR_PTR(-ENOMEM);
//		}
//	}
	#else
	if(nk_session_limit_checking(skb->iph->saddr)==-1)
		return ERR_PTR(-ENOMEM);
	#endif
    }
#endif
	conntrack = ip_conntrack_alloc(tuple, &repl_tuple);
	if (conntrack == NULL || IS_ERR(conntrack))
		return (struct ip_conntrack_tuple_hash *)conntrack;

	if (!protocol->new(conntrack, skb)) {
		ip_conntrack_free(conntrack);
		return NULL;
	}

	if(skb->dev) conntrack->in_device = skb->dev;

	write_lock_bh(&ip_conntrack_lock);
	exp = find_expectation(tuple);

	if (exp) {
		DEBUGP("conntrack: expectation arrives ct=%p exp=%p\n",
			conntrack, exp);
		//printk(KERN_EMERG "conntrack: expectation arrives ct=%p exp=%p\n",
		//	conntrack, exp);
		/* Welcome, Mr. Bond.  We've been expecting you... */
		__set_bit(IPS_EXPECTED_BIT, &conntrack->status);
		conntrack->master = exp->master;
#ifdef CONFIG_IP_NF_CONNTRACK_MARK
		conntrack->mark = exp->master->mark;
#endif
#if defined(CONFIG_IP_NF_TARGET_MASQUERADE) || \
    defined(CONFIG_IP_NF_TARGET_MASQUERADE_MODULE)
		/* this is ugly, but there is no other place where to put it */
		conntrack->nat.masq_index = exp->master->nat.masq_index;
#endif
		nf_conntrack_get(&conntrack->master->ct_general);
		CONNTRACK_STAT_INC(expect_new);
	} else {
		conntrack->helper = __ip_conntrack_helper_find(&repl_tuple);

		CONNTRACK_STAT_INC(new);
	}

	/* Overload tuple linked list to put us in unconfirmed list. */
	list_add(&conntrack->tuplehash[IP_CT_DIR_ORIGINAL].list, &unconfirmed);

	write_unlock_bh(&ip_conntrack_lock);

	if (exp) {
		if (exp->expectfn)
			exp->expectfn(conntrack, exp);
		ip_conntrack_expect_put(exp);
	}
//2007/11/14
// #ifdef CONFIG_SUPPORT_IP_MAC_BINDING
// 	//printk("conntrack->in_device=%s\n", conntrack->in_device);
// 	//strcpy(wan_interface_session[0].laninterface, "eth0");
// 	if(conntrack->in_device!=NULL)
//         {
//                 //printk("nk_ip_mac_binding_check:laninterface=%s\n", wan_interface_session[0].laninterface);
// 		if(!strcmp(conntrack->in_device, wan_interface_session[0].laninterface) && 
// 		  (skb->nh.iph->saddr>0) && (skb->nh.iph->daddr!=wan_interface_session[0].lanip) )
//                         nk_ip_mac_binding_check(skb, skb->nh.iph->saddr);
//         }
// #endif

#ifdef CONFIG_NK_SESSION_LIMIT
#ifdef CONFIG_NK_SESSION_LIMIT_ENHANCE
//    if ( (skb->dev!=NULL) && (!strcmp(skb->dev, wan_interface_session[0].laninterface)) && (skb->nh.iph->daddr!=wan_interface_session[0].lanip) && (session_limit_status!=0) &&
//	(skb->nh.iph->saddr>0) )
    if ( (skb->nh.iph->daddr!=wan_interface_session[0].lanip) && (session_limit_status!=0) )
    {
		exp = session_limit_enhance_rule_check(skb->nh.iph->saddr, ntohs(tuple->dst.u.tcp.port), skb->nh.iph->protocol, 0);
		//if(session_limit_enhance_rule_check(skb->nh.iph->saddr, ntohs(tuple->dst.u.tcp.port), skb->nh.iph->protocol, 0)!=1) 
		//20091125, Yami, check every session
		if(exp!=2)
		{
			idx = skb->nh.iph->saddr&0x7f; //128
			
			for(session_limit_p = nk_session_limit[idx];
				session_limit_p;
				session_limit_p = session_limit_p->next)
			{
				if(session_limit_p->ip == skb->nh.iph->saddr)
				{
					session_limit_p->session_cnt++;
					if( skb->nh.iph->protocol == IPPROTO_TCP )
						session_limit_p->tcp_session_cnt++;
					if( skb->nh.iph->protocol == IPPROTO_UDP )
						session_limit_p->udp_session_cnt++;
					//printk(KERN_EMERG "update session cnt: session_limit_p[%u.%u.%u.%u] cnt[%d] tcp[%d] udp[%d]\n", NIPQUAD(session_limit_p->ip), session_limit_p->session_cnt, session_limit_p->tcp_session_cnt, session_limit_p->udp_session_cnt);
					conntrack->slimit2=1;
					break;
				}
			}
		}
    }
#endif
#endif

	return &conntrack->tuplehash[IP_CT_DIR_ORIGINAL];
}

/* On success, returns conntrack ptr, sets skb->nfct and ctinfo */
static inline struct ip_conntrack *
resolve_normal_ct(struct sk_buff *skb,
		  struct ip_conntrack_protocol *proto,
		  int *set_reply,
		  unsigned int hooknum,
		  enum ip_conntrack_info *ctinfo)
{
	struct ip_conntrack_tuple tuple;
	struct ip_conntrack_tuple_hash *h;
	struct ip_conntrack *ct;

	IP_NF_ASSERT((skb->nh.iph->frag_off & htons(IP_OFFSET)) == 0);

	if (!ip_ct_get_tuple(skb->nh.iph, skb, skb->nh.iph->ihl*4, 
				&tuple,proto))
		return NULL;

	/* look for tuple match */
	h = ip_conntrack_find_get(&tuple, NULL);
	if (!h) {
		h = init_conntrack(&tuple, proto, skb);
		if (!h)
			return NULL;
		if (IS_ERR(h))
			return (void *)h;
	}
	ct = tuplehash_to_ctrack(h);

	/* It exists; we have (non-exclusive) reference. */
	if (DIRECTION(h) == IP_CT_DIR_REPLY) {
		*ctinfo = IP_CT_ESTABLISHED + IP_CT_IS_REPLY;
		/* Please set reply bit if this packet OK */
		*set_reply = 1;
	} else {
		/* Once we've had two way comms, always ESTABLISHED. */
		if (test_bit(IPS_SEEN_REPLY_BIT, &ct->status)) {
			DEBUGP("ip_conntrack_in: normal packet for %p\n",
			       ct);
		        *ctinfo = IP_CT_ESTABLISHED;
		} else if (test_bit(IPS_EXPECTED_BIT, &ct->status)) {
			DEBUGP("ip_conntrack_in: related packet for %p\n",
			       ct);
			*ctinfo = IP_CT_RELATED;
		} else {
			DEBUGP("ip_conntrack_in: new packet for %p\n",
			       ct);
			*ctinfo = IP_CT_NEW;
		}
		*set_reply = 0;
	}
	skb->nfct = &ct->ct_general;
	skb->nfctinfo = *ctinfo;
	return ct;
}

#include <linux/udp.h>
#include <net/route.h>

/*2007/12/14 trenchen : support netbios broadcast--->*/
#ifdef CONFIG_NK_IPSEC_NETBIOS_BC

#define BIOSHASHMAX 256
struct nat_ipsec_bc_head {
	nat_ipsec_bc_t *start;
	spinlock_t BiosBroadLock;
};
extern struct nat_ipsec_bc_head nat_ipsec_bc_ip_list[BIOSHASHMAX];

unsigned int NetbiosHashGet(unsigned int key){
	unsigned int index=0;
	unsigned int temp1,temp2;

	//hash value got from adding third and second bytes
	temp1 = (key & 0x00ff0000 ) >> 16;
	temp2 = (key & 0x0000ff00 ) >> 8;
	index = (temp1 + temp2) & 0x000000ff;

	return index;
}
void nk_translate_netbios_to_ipsec(struct sk_buff **pskb)
{
	struct iphdr *iphead = NULL;
	struct udphdr *udphead = NULL;
	__u16 dport = NULL;
	unsigned BiosIndex=NULL;
	nat_ipsec_bc_t *pComparEntry;
	struct udphdr *tempudp=NULL;
	struct iphdr *tempheader=NULL;
	struct sk_buff *mc=NULL;
	struct rtable *rt;

	iphead = (*pskb)->nh.iph;

	if(iphead->protocol == IPPROTO_UDP){
		udphead = (struct udphdr *)((u_int32_t *)iphead + iphead->ihl);
		dport = udphead->dest;
		if( dport >= 136 && dport <= 139 ){
			if( !strcmp( (*pskb)->dev->name, "eth0" ) ){
				//forward netbios broadcast to remote
				BiosIndex = NetbiosHashGet(iphead->daddr);
//				printk("nk_trans:index[%u]\n",BiosIndex);
				spin_lock( &(nat_ipsec_bc_ip_list[BiosIndex].BiosBroadLock) );	
				for(pComparEntry=nat_ipsec_bc_ip_list[BiosIndex].start;pComparEntry;pComparEntry=pComparEntry->next){
					if(iphead->daddr==pComparEntry->local_bc){
						mc = skb_copy(*pskb,GFP_ATOMIC);

//						printk("nk_trans:get packet daddr[%x]\n",iphead->daddr);
						tempheader = mc->nh.iph;
						tempudp = (struct udphdr *)((u_int32_t *)tempheader + tempheader->ihl);
						mc->pkt_type = PACKET_HOST;
						tempheader->daddr = pComparEntry->remote_bc;
						tempheader->check = 0;
						tempheader->check = ip_fast_csum( (unsigned char *)tempheader,tempheader->ihl);
						if(tempudp->check)
							tempudp->check = 0;	
						ip_rcv_finish(mc);
					}
				}
				spin_unlock( &(nat_ipsec_bc_ip_list[BiosIndex].BiosBroadLock) );
			} else {
				//forward netbios broadcast to local
				BiosIndex = NetbiosHashGet(iphead->daddr);
//				printk("nk_trans to local:index[%u]\n",BiosIndex);
				spin_lock( &(nat_ipsec_bc_ip_list[BiosIndex].BiosBroadLock) );	
				for(pComparEntry=nat_ipsec_bc_ip_list[BiosIndex].start;pComparEntry;pComparEntry=pComparEntry->next){
					if(iphead->daddr==pComparEntry->local_bc){
						struct flowi fl={ .oif = 0,
								.nl_u = { .ip4_u =
										{ .daddr = iphead->daddr,
										.saddr = 0,
										.tos = 0 } },
								.proto = IPPROTO_UDP,
								.uli_u = { .ports =
										{ .sport = 0,
										.dport = 0 } } };

						mc = skb_copy(*pskb,GFP_ATOMIC);
//						printk("nk_trans to local:get packet daddr[%x]\n",iphead->daddr);

						mc->pkt_type = PACKET_HOST;
						if( __ip_route_output_key(&rt,&fl) ){
							kfree_skb(mc);
							return;
						}
						mc->csum=0;
						mc->dst = &rt->u.dst;
						mc->dev = mc->dst->dev;
						ip_finish_output(mc);
						break;
					}
				}
				spin_unlock( &(nat_ipsec_bc_ip_list[BiosIndex].BiosBroadLock) );
			}
		}
	}

	return;
}
#endif
/*<-------trenchen netbios broadcast*/

#ifdef CONFIG_NK_IPSEC_SPLITDNS
//--->20100107 trenchen : support split dns
extern struct ipsec_split_dns_head ipsec_split_dns_list;

#define DNS_SPLIT_HASH_SIZE 256
#define DNS_SPLIT_HASH_MASK 0xff
#define MAX_DNS_SPLIT_HASH_NUM 1500
dns_split_hash_t *dns_split_hash_table[DNS_SPLIT_HASH_SIZE];
static atomic_t cDnsSplitNum;
spinlock_t SplitdnsHashLock;

int nk_translate_spiltdns_to_ipsec(struct sk_buff **pskb)
{
	struct iphdr *iphead = NULL;
	struct udphdr *udphead = NULL;
	__u16 dport = NULL;
	__u16 sport = NULL;
	char *DNS = 0;
	unsigned int dns_QR = 0;
	unsigned int dns_ID = 0;
	unsigned int Flan=0;
	struct sk_buff *cskbuff=0;
	dns_split_hash_t *split_hash_p=0;

	if(ipsec_split_dns_list.start){
		iphead = (*pskb)->nh.iph;
		if(iphead->protocol == IPPROTO_UDP){
			udphead = (struct udphdr *)((u_int32_t *)iphead + iphead->ihl);
			dport = udphead->dest;
			sport = udphead->source;
			//if(dport==53){//dns port
				DNS =(char *)udphead+8;
				dns_QR = (*(unsigned int *)DNS & 0x00008000) >> 15;
				dns_ID = (*(unsigned int *)DNS & 0xffff0000) >> 16;
				Flan = !strcmp( (*pskb)->dev->name, "eth0" );
				//printk("split dns: QR[%u] flan[%u]\n",dns_QR,Flan);
				if(dport==53 && dns_QR == 0 && Flan){//from lan request, need forward to remote?
					char RDname[60];
					char *temp;
					unsigned int jump=0;
					ipsec_split_dns_t *listentry;
					
					sprintf(RDname,"%s",DNS+12);
					
					//domain name in dns packet need modify, lack '.'
					for(temp=RDname;*temp;temp+=jump){
						jump=*temp+1;
						*temp='.';
					}
					//printk("split dns: [%s] DNS request packet from lan\n",RDname);

					//search list
					spin_lock(&(ipsec_split_dns_list.SplitdnsLock)); 
					for(listentry=ipsec_split_dns_list.start;listentry;listentry=listentry->next){
						//search entry domain name
						for(jump=0;jump<listentry->NumDname;++jump){
							if( strstr(RDname+1,listentry->domain_name[jump]) ){
								break;
							}
						}
						if(jump<listentry->NumDname)
							break;
					}
					spin_unlock(&(ipsec_split_dns_list.SplitdnsLock));

					if(!listentry){
						//printk("split dns: not found match domain name\n");
					}else{
						//printk("split dns: get match\n");

						if( atomic_read(&cDnsSplitNum) ==MAX_DNS_SPLIT_HASH_NUM ){
							//hash entry full, kill the first one
							for(jump=0;jump<DNS_SPLIT_HASH_SIZE;++jump){
								split_hash_p = 0;
								spin_lock(&(SplitdnsHashLock));
								if(dns_split_hash_table[jump]){
									split_hash_p = dns_split_hash_table[jump];
									dns_split_hash_table[jump] = split_hash_p->next;
								}
								spin_unlock(&(SplitdnsHashLock));
								if(split_hash_p){
									//printk("split dns: cache full kill [%x] entry\n",split_hash_p);
									kfree(split_hash_p);
									atomic_dec(&cDnsSplitNum);
								}
							}
						}

						if(split_hash_p = kmalloc(sizeof(dns_split_hash_t), GFP_ATOMIC)){
							//fill split_hash_p
							
							split_hash_p->dns_id = dns_ID;
							split_hash_p->dstip = iphead->daddr;
							split_hash_p->dns_split_server = 1;
							split_hash_p->rdns_ip[0] = listentry->ip[0];
							split_hash_p->dns_split_ignore = 0;
							split_hash_p->next = 0;
							
							//change first packet
							iphead->daddr = listentry->ip[0];
							iphead->check = 0;
							iphead->check = ip_fast_csum((unsigned char *)iphead,iphead->ihl);
							if(udphead->check)
								udphead->check = 0;

							for(jump=1;jump < listentry->NumIp;++jump){
								cskbuff = skb_copy(*pskb,GFP_ATOMIC);
								if(cskbuff){
									//creat remainder packet
									iphead = cskbuff->nh.iph;
									udphead = (struct udphdr *)((u_int32_t *)iphead + iphead->ihl);
									cskbuff->pkt_type = PACKET_HOST;
									iphead->daddr = listentry->ip[jump];
									iphead->check = 0;
									iphead->check = ip_fast_csum((unsigned char *)iphead,iphead->ihl);
									if(udphead->check)
										udphead->check = 0;
									ip_rcv_finish(cskbuff);
									split_hash_p->rdns_ip[jump] = listentry->ip[jump];
									++split_hash_p->dns_split_server;
								}
							}
							spin_lock(&(SplitdnsHashLock));
							split_hash_p->next = dns_split_hash_table[split_hash_p->dns_id & DNS_SPLIT_HASH_MASK];
							dns_split_hash_table[split_hash_p->dns_id & DNS_SPLIT_HASH_MASK] = split_hash_p;
#if 0
							for(jump=0;jump<DNS_SPLIT_HASH_SIZE;++jump){
								split_hash_p=dns_split_hash_table[jump];
								if(split_hash_p)
									printk("[now hash is %x]\n",jump);
								for(; split_hash_p; split_hash_p=split_hash_p->next){
									printk("     [entry %x]\n",split_hash_p);
								}
							}
#endif
							spin_unlock(&(SplitdnsHashLock));
							//update entry num
							atomic_inc(&cDnsSplitNum);
						}
					}

				}else if(sport==53 && dns_QR == 1 && !Flan){//come from remote dns
					unsigned int count=0;
					unsigned int dns_RC = 0;
					struct rtable *rt;
					struct flowi fl={ .oif = 0,
							.nl_u = { .ip4_u =
									{ .daddr = 0,
									.saddr = 0,
									.tos = 0 } },
							.proto = IPPROTO_UDP,
							.uli_u = { .ports =
									{ .sport = 0,
									.dport = 0 } } };

					//search hash list
					//printk("split dns:com from remote\n");
					dns_split_hash_t **split_hash_pprev=0;
					spin_lock(&(SplitdnsHashLock));
					split_hash_pprev = &dns_split_hash_table[dns_ID & DNS_SPLIT_HASH_MASK];
					for(;*split_hash_pprev;split_hash_pprev=&((*split_hash_pprev)->next)){
						split_hash_p = *split_hash_pprev;
						if( split_hash_p->dns_id == dns_ID ){
							//printk("find the same id[%u]\n",dns_ID);
							for(count=0;count < split_hash_p->dns_split_server;++count){
								if(iphead->saddr==split_hash_p->rdns_ip[count]){
									//find 
									//printk("split dns:find list [%x] ipd[%x]\n",split_hash_p,split_hash_p->dstip);
									goto FIND_LIST;
								}
							}
						}
					}
					spin_unlock(&(SplitdnsHashLock));
					//printk("cant not find ip[%x]id[%u]\n",iphead->saddr,dns_ID);
					return 0;
FIND_LIST:
					dns_RC = (*(unsigned int *)DNS & 0x0000000f);
					//printk("find list RC[%u] ignore[%u] server[%u]\n",dns_RC,split_hash_p->dns_split_ignore,split_hash_p->dns_split_server);
					if( dns_RC==0 || (split_hash_p->dns_split_ignore == split_hash_p->dns_split_server-1) ){
						//kill list entry
						*split_hash_pprev = split_hash_p->next;
						atomic_dec(&cDnsSplitNum);
						spin_unlock(&(SplitdnsHashLock));
						//change packet
#if 0
						iphead->saddr = split_hash_p->dstip;
						kfree(split_hash_p);
						iphead->check = 0;
						iphead->check = ip_fast_csum((unsigned char *)iphead,iphead->ihl);
						if(udphead->check)
								udphead->check = 0;
#endif
						cskbuff = skb_copy(*pskb,GFP_ATOMIC);
						if(cskbuff){
							iphead = cskbuff->nh.iph;
							udphead = (struct udphdr *)((u_int32_t *)iphead + iphead->ihl);
							cskbuff->pkt_type = PACKET_HOST;
							iphead->saddr = split_hash_p->dstip;
							kfree(split_hash_p);
							iphead->check = 0;
							iphead->check = ip_fast_csum((unsigned char *)iphead,iphead->ihl);
							if(udphead->check)
								udphead->check = 0;
							fl.nl_u.ip4_u.daddr = iphead->daddr;
							if( __ip_route_output_key(&rt,&fl) ){
								kfree_skb(cskbuff);
							}else{
								cskbuff->csum = 0;
								cskbuff->dst = &rt->u.dst;
								cskbuff->dev = cskbuff->dst->dev;
								ip_finish_output(cskbuff);
							}
						}else{
							kfree(split_hash_p);
						}
						//printk("send packet ip[%x]\n",iphead->saddr);
						return -1;
					}else{
						++split_hash_p->dns_split_ignore;
						spin_unlock(&(SplitdnsHashLock));
						//printk("no such name\n");
						//need drop packet
						return -1;
					}
				}
			//}
		}
	}
	return 0;
}
//<----------------
#endif

/* Netfilter hook itself. */
unsigned int ip_conntrack_in(unsigned int hooknum,
			     struct sk_buff **pskb,
			     const struct net_device *in,
			     const struct net_device *out,
			     int (*okfn)(struct sk_buff *))
{
	struct ip_conntrack *ct;
	enum ip_conntrack_info ctinfo;
	struct ip_conntrack_protocol *proto;
	int set_reply = 0;
	int ret;

	//printk("ip_conntrack_in\n");
	/* Previously seen (loopback or untracked)?  Ignore. */
	if ((*pskb)->nfct) {
		CONNTRACK_STAT_INC(ignore);
		return NF_ACCEPT;
	}

	/* Never happen */
	if ((*pskb)->nh.iph->frag_off & htons(IP_OFFSET)) {
		if (net_ratelimit()) {
		printk(KERN_ERR "ip_conntrack_in: Frag of proto %u (hook=%u)\n",
		       (*pskb)->nh.iph->protocol, hooknum);
		}
		return NF_DROP;
	}

/* Doesn't cover locally-generated broadcast, so not worth it. */
#if 0
	/* Ignore broadcast: no `connection'. */
	if ((*pskb)->pkt_type == PACKET_BROADCAST) {
		printk("Broadcast packet!\n");
		return NF_ACCEPT;
	} else if (((*pskb)->nh.iph->daddr & htonl(0x000000FF)) 
		   == htonl(0x000000FF)) {
		printk("Should bcast: %u.%u.%u.%u->%u.%u.%u.%u (sk=%p, ptype=%u)\n",
		       NIPQUAD((*pskb)->nh.iph->saddr),
		       NIPQUAD((*pskb)->nh.iph->daddr),
		       (*pskb)->sk, (*pskb)->pkt_type);
	}
#endif

//#ifdef CONFIG_NK_DOS_ENHANCEMENT
#if 0
    //only inspect the incoming packet.
    if(!out)
    {
	switch ((*pskb)->nh.iph->protocol) {
	case IPPROTO_TCP:
		if(dos_tcp_syn_enabled)
		{
			tcphdr *tcp;

			tcp = (tcphdr *)((char *)(*pskb)->nh.iph + (*pskb)->nh.iph->ihl**4);
			//if ((tcp->th_flags & TH_OPENING) == TH_SYN)
			//{
				//if(dos_check((*pskb)->nh.iph.saddr, (*pskb)->nh.iph.daddr, in, 0) == -1)
				//	return -1;
			//}
		}
		break;
#if 0
	case IPPROTO_UDP:
		if(dos_udp_enabled)
		{
			if(dos_check(ip_p->ip_src.s_addr, ip_p->ip_dst.s_addr, dev, 1) == -1)
				return -1;
		}
		break;

	case IPPROTO_ICMP:
		if(dos_icmp_enabled)
		{
			if(dos_check(ip_p->ip_src.s_addr, ip_p->ip_dst.s_addr, dev, 2) == -1)
				return -1;
		}
		break;
#endif
	}
    }
#endif

	proto = __ip_conntrack_proto_find((*pskb)->nh.iph->protocol);

	/* It may be an special packet, error, unclean...
	 * inverse of the return code tells to the netfilter
	 * core what to do with the packet. */
	if (proto->error != NULL 
	    && (ret = proto->error(*pskb, &ctinfo, hooknum)) <= 0) {
		CONNTRACK_STAT_INC(error);
		CONNTRACK_STAT_INC(invalid);
		return -ret;
	}

	if (!(ct = resolve_normal_ct(*pskb, proto,&set_reply,hooknum,&ctinfo))) {
		/* Not valid part of a connection */
		CONNTRACK_STAT_INC(invalid);
		return NF_ACCEPT;
	}

	if (IS_ERR(ct)) {
		/* Too stressed to deal. */
		CONNTRACK_STAT_INC(drop);
		return NF_DROP;
	}

	IP_NF_ASSERT((*pskb)->nfct);

	ret = proto->packet(ct, *pskb, ctinfo);
	if (ret < 0) {
		/* Invalid: inverse of the return code tells
		 * the netfilter core what to do*/
		nf_conntrack_put((*pskb)->nfct);
		(*pskb)->nfct = NULL;
		CONNTRACK_STAT_INC(invalid);
		return -ret;
	}

	if (set_reply && !test_and_set_bit(IPS_SEEN_REPLY_BIT, &ct->status))
		ip_conntrack_event_cache(IPCT_STATUS, *pskb);

/*2007/12/14 trenhcen : support netbios broadcast-->*/
#ifdef CONFIG_NK_IPSEC_NETBIOS_BC
	if(hooknum==NF_IP_PRE_ROUTING)
		nk_translate_netbios_to_ipsec(pskb);
#endif

#ifdef CONFIG_NK_IPSEC_SPLITDNS
	//-->20100107 trenchen : support split dns
	if(hooknum==NF_IP_PRE_ROUTING)
		if( nk_translate_spiltdns_to_ipsec(pskb)== -1)
			return NF_DROP;
	//<-------
#endif

#ifdef CONFIG_NK_RESTRICT_APP
	if(restrict_app.block_qq)
	{
		if(check_block_qq(pskb)==NF_DROP)
			return NF_DROP;
	}
#endif

	return ret;
}

int invert_tuplepr(struct ip_conntrack_tuple *inverse,
		   const struct ip_conntrack_tuple *orig)
{
	return ip_ct_invert_tuple(inverse, orig, 
				  __ip_conntrack_proto_find(orig->dst.protonum));
}

/* Would two expected things clash? */
static inline int expect_clash(const struct ip_conntrack_expect *a,
			       const struct ip_conntrack_expect *b)
{
	/* Part covered by intersection of masks must be unequal,
           otherwise they clash */
	struct ip_conntrack_tuple intersect_mask
		= { { a->mask.src.ip & b->mask.src.ip,
		      { a->mask.src.u.all & b->mask.src.u.all } },
		    { a->mask.dst.ip & b->mask.dst.ip,
		      { a->mask.dst.u.all & b->mask.dst.u.all },
		      a->mask.dst.protonum & b->mask.dst.protonum } };

	return ip_ct_tuple_mask_cmp(&a->tuple, &b->tuple, &intersect_mask);
}

static inline int expect_matches(const struct ip_conntrack_expect *a,
				 const struct ip_conntrack_expect *b)
{
	return a->master == b->master
		&& ip_ct_tuple_equal(&a->tuple, &b->tuple)
		&& ip_ct_tuple_equal(&a->mask, &b->mask);
}

/* Generally a bad idea to call this: could have matched already. */
void ip_conntrack_unexpect_related(struct ip_conntrack_expect *exp)
{
	struct ip_conntrack_expect *i;

	write_lock_bh(&ip_conntrack_lock);
	/* choose the the oldest expectation to evict */
	list_for_each_entry_reverse(i, &ip_conntrack_expect_list, list) {
		if (expect_matches(i, exp) && del_timer(&i->timeout)) {
			ip_ct_unlink_expect(i);
			write_unlock_bh(&ip_conntrack_lock);
			ip_conntrack_expect_put(i);
			return;
		}
	}
	write_unlock_bh(&ip_conntrack_lock);
}

/* We don't increase the master conntrack refcount for non-fulfilled
 * conntracks. During the conntrack destruction, the expectations are 
 * always killed before the conntrack itself */
struct ip_conntrack_expect *ip_conntrack_expect_alloc(struct ip_conntrack *me)
{
	struct ip_conntrack_expect *new;

	new = kmem_cache_alloc(ip_conntrack_expect_cachep, GFP_ATOMIC);
	if (!new) {
		DEBUGP("expect_related: OOM allocating expect\n");
		return NULL;
	}
	new->master = me;
	atomic_set(&new->use, 1);
	return new;
}

void ip_conntrack_expect_put(struct ip_conntrack_expect *exp)
{
	if (atomic_dec_and_test(&exp->use))
		kmem_cache_free(ip_conntrack_expect_cachep, exp);
}

static void ip_conntrack_expect_insert(struct ip_conntrack_expect *exp)
{
	atomic_inc(&exp->use);
	exp->master->expecting++;
	list_add(&exp->list, &ip_conntrack_expect_list);

	init_timer(&exp->timeout);
	exp->timeout.data = (unsigned long)exp;
	exp->timeout.function = expectation_timed_out;
	exp->timeout.expires = jiffies + exp->master->helper->timeout * HZ;
	add_timer(&exp->timeout);

	exp->id = ++ip_conntrack_expect_next_id;
	atomic_inc(&exp->use);
	CONNTRACK_STAT_INC(expect_create);
}

/* Race with expectations being used means we could have none to find; OK. */
static void evict_oldest_expect(struct ip_conntrack *master)
{
	struct ip_conntrack_expect *i;

	list_for_each_entry_reverse(i, &ip_conntrack_expect_list, list) {
		if (i->master == master) {
			if (del_timer(&i->timeout)) {
				ip_ct_unlink_expect(i);
				ip_conntrack_expect_put(i);
			}
			break;
		}
	}
}

static inline int refresh_timer(struct ip_conntrack_expect *i)
{
	if (!del_timer(&i->timeout))
		return 0;

	i->timeout.expires = jiffies + i->master->helper->timeout*HZ;
	add_timer(&i->timeout);
	return 1;
}

int ip_conntrack_expect_related(struct ip_conntrack_expect *expect)
{
	struct ip_conntrack_expect *i;
	int ret;

	DEBUGP("ip_conntrack_expect_related %p\n", related_to);
	DEBUGP("tuple: "); DUMP_TUPLE(&expect->tuple);
	DEBUGP("mask:  "); DUMP_TUPLE(&expect->mask);

	write_lock_bh(&ip_conntrack_lock);
	list_for_each_entry(i, &ip_conntrack_expect_list, list) {
		if (expect_matches(i, expect)) {
			/* Refresh timer: if it's dying, ignore.. */
			if (refresh_timer(i)) {
				ret = 0;
				goto out;
			}
		} else if (expect_clash(i, expect)) {
			ret = -EBUSY;
			goto out;
		}
	}

	/* Will be over limit? */
	if (expect->master->helper->max_expected && 
	    expect->master->expecting >= expect->master->helper->max_expected)
		evict_oldest_expect(expect->master);

	ip_conntrack_expect_insert(expect);
	ip_conntrack_expect_event(IPEXP_NEW, expect);
	ret = 0;
out:
	write_unlock_bh(&ip_conntrack_lock);
 	return ret;
}

/* Alter reply tuple (maybe alter helper).  This is for NAT, and is
   implicitly racy: see __ip_conntrack_confirm */
void ip_conntrack_alter_reply(struct ip_conntrack *conntrack,
			      const struct ip_conntrack_tuple *newreply)
{
	write_lock_bh(&ip_conntrack_lock);
	/* Should be unconfirmed, so not in hash table yet */
	IP_NF_ASSERT(!is_confirmed(conntrack));

	DEBUGP("Altering reply tuple of %p to ", conntrack);
	DUMP_TUPLE(newreply);

	conntrack->tuplehash[IP_CT_DIR_REPLY].tuple = *newreply;
	if (!conntrack->master && conntrack->expecting == 0)
		conntrack->helper = __ip_conntrack_helper_find(newreply);
	write_unlock_bh(&ip_conntrack_lock);
}

int ip_conntrack_helper_register(struct ip_conntrack_helper *me)
{
	BUG_ON(me->timeout == 0);
	write_lock_bh(&ip_conntrack_lock);
	list_prepend(&helpers, me);
	write_unlock_bh(&ip_conntrack_lock);

	return 0;
}

struct ip_conntrack_helper *
__ip_conntrack_helper_find_byname(const char *name)
{
	struct ip_conntrack_helper *h;

	list_for_each_entry(h, &helpers, list) {
		if (!strcmp(h->name, name))
			return h;
	}

	return NULL;
}

static inline int unhelp(struct ip_conntrack_tuple_hash *i,
			 const struct ip_conntrack_helper *me)
{
	if (tuplehash_to_ctrack(i)->helper == me) {
 		ip_conntrack_event(IPCT_HELPER, tuplehash_to_ctrack(i));
		tuplehash_to_ctrack(i)->helper = NULL;
	}
	return 0;
}

void ip_conntrack_helper_unregister(struct ip_conntrack_helper *me)
{
	unsigned int i;
	struct ip_conntrack_expect *exp, *tmp;

	/* Need write lock here, to delete helper. */
	write_lock_bh(&ip_conntrack_lock);
	LIST_DELETE(&helpers, me);

	/* Get rid of expectations */
	list_for_each_entry_safe(exp, tmp, &ip_conntrack_expect_list, list) {
		if (exp->master->helper == me && del_timer(&exp->timeout)) {
			ip_ct_unlink_expect(exp);
			ip_conntrack_expect_put(exp);
		}
	}
	/* Get rid of expecteds, set helpers to NULL. */
	LIST_FIND_W(&unconfirmed, unhelp, struct ip_conntrack_tuple_hash*, me);
	for (i = 0; i < ip_conntrack_htable_size; i++)
		LIST_FIND_W(&ip_conntrack_hash[i], unhelp,
			    struct ip_conntrack_tuple_hash *, me);
	write_unlock_bh(&ip_conntrack_lock);

	/* Someone could be still looking at the helper in a bh. */
	synchronize_net();
}

/* Refresh conntrack for this many jiffies and do accounting if do_acct is 1 */
void __ip_ct_refresh_acct(struct ip_conntrack *ct, 
		        enum ip_conntrack_info ctinfo,
			const struct sk_buff *skb,
			unsigned long extra_jiffies,
			int do_acct)
{
	int event = 0;
	int idx;
	nk_agingtime_t *aging_time_node, *nk_aging_p, *tmp;

	IP_NF_ASSERT(ct->timeout.data == (unsigned long)ct);
	IP_NF_ASSERT(skb);

	write_lock_bh(&ip_conntrack_lock);

	/* If not in hash table, timer will not be active yet */
	if (!is_confirmed(ct)) {
		ct->timeout.expires = extra_jiffies;
		event = IPCT_REFRESH;
	} else {
		/* Need del_timer for race avoidance (may already be dying). */
		if (del_timer(&ct->timeout)) {
			if(ct->timeout.expires>0)
			{
				ct->timeout.expires = jiffies + extra_jiffies;

				if (ct->status & IPS_SEEN_REPLY)
				{
					idx = ntohs(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.all)&0x7f; //128
					//printk(KERN_EMERG "idx=%d\n", idx);
					for(nk_aging_p = nk_nk_agingtime_list[idx];
						nk_aging_p;
						nk_aging_p = nk_aging_p->next)
					{
						
						if( ((nk_aging_p->protocol == ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum) || (nk_aging_p->protocol==0) )
						&&	(nk_aging_p->port == ntohs(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.all)) )
						{
							if ( ((ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum==6) && (ct->proto.tcp.state==TCP_CONNTRACK_ESTABLISHED)) || (ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum==17) )
								ct->timeout.expires = nk_aging_p->timeout*HZ + jiffies;
							//printk(KERN_EMERG "aging->protocol=%d aging->port=%d, aging->timeout\n", nk_aging_p->protocol, nk_aging_p->port, nk_aging_p->timeout);
							//printk(KERN_EMERG "ct->timeout.expires=%d\n", ct->timeout.expires);
							break;
						//printk(KERN_EMERG "idx=%d, protocol=%d, port=%d, timeout=%d\n", i, nk_aging_p->protocol, nk_aging_p->port, nk_aging_p->timeout);
						}
					}
				}
			}
			add_timer(&ct->timeout);
			event = IPCT_REFRESH;
		}
	}

#ifdef CONFIG_IP_NF_CT_ACCT
	if (do_acct) {
		ct->counters[CTINFO2DIR(ctinfo)].packets++;
		ct->counters[CTINFO2DIR(ctinfo)].bytes += 
						ntohs(skb->nh.iph->tot_len);
		if ((ct->counters[CTINFO2DIR(ctinfo)].packets & 0x80000000)
		    || (ct->counters[CTINFO2DIR(ctinfo)].bytes & 0x80000000))
			event |= IPCT_COUNTER_FILLING;
	}
#endif

	write_unlock_bh(&ip_conntrack_lock);

	/* must be unlocked when calling event cache */
	if (event)
		ip_conntrack_event_cache(event, skb);
}

#if defined(CONFIG_IP_NF_CONNTRACK_NETLINK) || \
    defined(CONFIG_IP_NF_CONNTRACK_NETLINK_MODULE)
/* Generic function for tcp/udp/sctp/dccp and alike. This needs to be
 * in ip_conntrack_core, since we don't want the protocols to autoload
 * or depend on ctnetlink */
int ip_ct_port_tuple_to_nfattr(struct sk_buff *skb,
			       const struct ip_conntrack_tuple *tuple)
{
	NFA_PUT(skb, CTA_PROTO_SRC_PORT, sizeof(u_int16_t),
		&tuple->src.u.tcp.port);
	NFA_PUT(skb, CTA_PROTO_DST_PORT, sizeof(u_int16_t),
		&tuple->dst.u.tcp.port);
	return 0;

nfattr_failure:
	return -1;
}

int ip_ct_port_nfattr_to_tuple(struct nfattr *tb[],
			       struct ip_conntrack_tuple *t)
{
	if (!tb[CTA_PROTO_SRC_PORT-1] || !tb[CTA_PROTO_DST_PORT-1])
		return -EINVAL;

	t->src.u.tcp.port =
		*(u_int16_t *)NFA_DATA(tb[CTA_PROTO_SRC_PORT-1]);
	t->dst.u.tcp.port =
		*(u_int16_t *)NFA_DATA(tb[CTA_PROTO_DST_PORT-1]);

	return 0;
}
#endif

/* Returns new sk_buff, or NULL */
struct sk_buff *
ip_ct_gather_frags(struct sk_buff *skb, u_int32_t user)
{
	skb_orphan(skb);

	local_bh_disable(); 
	skb = ip_defrag(skb, user);
	local_bh_enable();

	if (skb)
		ip_send_check(skb->nh.iph);
	return skb;
}

/* Used by ipt_REJECT. */
static void ip_conntrack_attach(struct sk_buff *nskb, struct sk_buff *skb)
{
	struct ip_conntrack *ct;
	enum ip_conntrack_info ctinfo;

	/* This ICMP is in reverse direction to the packet which caused it */
	ct = ip_conntrack_get(skb, &ctinfo);
	
	if (CTINFO2DIR(ctinfo) == IP_CT_DIR_ORIGINAL)
		ctinfo = IP_CT_RELATED + IP_CT_IS_REPLY;
	else
		ctinfo = IP_CT_RELATED;

	/* Attach to new skbuff, and increment count */
	nskb->nfct = &ct->ct_general;
	nskb->nfctinfo = ctinfo;
	nf_conntrack_get(nskb->nfct);
}

static inline int
do_iter(const struct ip_conntrack_tuple_hash *i,
	int (*iter)(struct ip_conntrack *i, void *data),
	void *data)
{
	return iter(tuplehash_to_ctrack(i), data);
}

/* Bring out ya dead! */
static struct ip_conntrack_tuple_hash *
get_next_corpse(int (*iter)(struct ip_conntrack *i, void *data),
		void *data, unsigned int *bucket)
{
	struct ip_conntrack_tuple_hash *h = NULL;

	write_lock_bh(&ip_conntrack_lock);
	for (; *bucket < ip_conntrack_htable_size; (*bucket)++) {
		h = LIST_FIND_W(&ip_conntrack_hash[*bucket], do_iter,
				struct ip_conntrack_tuple_hash *, iter, data);
		if (h)
			break;
	}
	if (!h)
		h = LIST_FIND_W(&unconfirmed, do_iter,
				struct ip_conntrack_tuple_hash *, iter, data);
	if (h)
		atomic_inc(&tuplehash_to_ctrack(h)->ct_general.use);
	write_unlock_bh(&ip_conntrack_lock);

	return h;
}

void
ip_ct_iterate_cleanup(int (*iter)(struct ip_conntrack *i, void *), void *data)
{
	struct ip_conntrack_tuple_hash *h;
	unsigned int bucket = 0;

	while ((h = get_next_corpse(iter, data, &bucket)) != NULL) {
		struct ip_conntrack *ct = tuplehash_to_ctrack(h);
		/* Time to push up daises... */
		if (del_timer(&ct->timeout))
		{
			death_by_timeout((unsigned long)ct);
		}
		/* ... else the timer will get him soon. */

		ip_conntrack_put(ct);
	}
}

/* Fast function for those who don't want to parse /proc (and I don't
   blame them). */
/* Reversing the socket's dst/src point of view gives us the reply
   mapping. */
static int
getorigdst(struct sock *sk, int optval, void __user *user, int *len)
{
	struct inet_sock *inet = inet_sk(sk);
	struct ip_conntrack_tuple_hash *h;
	struct ip_conntrack_tuple tuple;
	
	IP_CT_TUPLE_U_BLANK(&tuple);
	tuple.src.ip = inet->rcv_saddr;
	tuple.src.u.tcp.port = inet->sport;
	tuple.dst.ip = inet->daddr;
	tuple.dst.u.tcp.port = inet->dport;
	tuple.dst.protonum = IPPROTO_TCP;

	/* We only do TCP at the moment: is there a better way? */
	if (strcmp(sk->sk_prot->name, "TCP")) {
		DEBUGP("SO_ORIGINAL_DST: Not a TCP socket\n");
		return -ENOPROTOOPT;
	}

	if ((unsigned int) *len < sizeof(struct sockaddr_in)) {
		DEBUGP("SO_ORIGINAL_DST: len %u not %u\n",
		       *len, sizeof(struct sockaddr_in));
		return -EINVAL;
	}

	h = ip_conntrack_find_get(&tuple, NULL);
	if (h) {
		struct sockaddr_in sin;
		struct ip_conntrack *ct = tuplehash_to_ctrack(h);

		sin.sin_family = AF_INET;
		sin.sin_port = ct->tuplehash[IP_CT_DIR_ORIGINAL]
			.tuple.dst.u.tcp.port;
		sin.sin_addr.s_addr = ct->tuplehash[IP_CT_DIR_ORIGINAL]
			.tuple.dst.ip;
		memset(sin.sin_zero, 0, sizeof(sin.sin_zero));

		DEBUGP("SO_ORIGINAL_DST: %u.%u.%u.%u %u\n",
		       NIPQUAD(sin.sin_addr.s_addr), ntohs(sin.sin_port));
		ip_conntrack_put(ct);
		if (copy_to_user(user, &sin, sizeof(sin)) != 0)
			return -EFAULT;
		else
			return 0;
	}
	DEBUGP("SO_ORIGINAL_DST: Can't find %u.%u.%u.%u/%u-%u.%u.%u.%u/%u.\n",
	       NIPQUAD(tuple.src.ip), ntohs(tuple.src.u.tcp.port),
	       NIPQUAD(tuple.dst.ip), ntohs(tuple.dst.u.tcp.port));
	return -ENOENT;
}

static struct nf_sockopt_ops so_getorigdst = {
	.pf		= PF_INET,
	.get_optmin	= SO_ORIGINAL_DST,
	.get_optmax	= SO_ORIGINAL_DST+1,
	.get		= &getorigdst,
};

static int kill_all(struct ip_conntrack *i, void *data)
{
	return 1;
}

void ip_conntrack_flush(void)
{
	ip_ct_iterate_cleanup(kill_all, NULL);
}

static void free_conntrack_hash(struct list_head *hash, int vmalloced,int size)
{
	if (vmalloced)
		vfree(hash);
	else
		free_pages((unsigned long)hash, 
			   get_order(sizeof(struct list_head) * size));
}

/* Mishearing the voices in his head, our hero wonders how he's
   supposed to kill the mall. */
void ip_conntrack_cleanup(void)
{
	ip_ct_attach = NULL;

	/* This makes sure all current packets have passed through
           netfilter framework.  Roll on, two-stage module
           delete... */
	synchronize_net();

	ip_ct_event_cache_flush();
 i_see_dead_people:
	ip_conntrack_flush();
	if (atomic_read(&ip_conntrack_count) != 0) {
		schedule();
		goto i_see_dead_people;
	}
	/* wait until all references to ip_conntrack_untracked are dropped */
	while (atomic_read(&ip_conntrack_untracked.ct_general.use) > 1)
		schedule();

	kmem_cache_destroy(ip_conntrack_cachep);
	kmem_cache_destroy(ip_conntrack_expect_cachep);
	free_conntrack_hash(ip_conntrack_hash, ip_conntrack_vmalloc,
			    ip_conntrack_htable_size);
	nf_unregister_sockopt(&so_getorigdst);
}

static struct list_head *alloc_hashtable(int size, int *vmalloced)
{
	struct list_head *hash;
	unsigned int i;

	*vmalloced = 0; 
	hash = (void*)__get_free_pages(GFP_KERNEL, 
				       get_order(sizeof(struct list_head)
						 * size));
	if (!hash) { 
		*vmalloced = 1;
		printk(KERN_WARNING"ip_conntrack: falling back to vmalloc.\n");
		hash = vmalloc(sizeof(struct list_head) * size);
	}

	if (hash)
		for (i = 0; i < size; i++)
			INIT_LIST_HEAD(&hash[i]);

	return hash;
}

static int set_hashsize(const char *val, struct kernel_param *kp)
{
	int i, bucket, hashsize, vmalloced;
	int old_vmalloced, old_size;
	int rnd;
	struct list_head *hash, *old_hash;
	struct ip_conntrack_tuple_hash *h;

	//printk(KERN_EMERG "set_hashsize\n");
	/* On boot, we can set this without any fancy locking. */
	if (!ip_conntrack_htable_size)
		return param_set_int(val, kp);

	hashsize = simple_strtol(val, NULL, 0);
	if (!hashsize)
		return -EINVAL;

	hash = alloc_hashtable(hashsize, &vmalloced);
	if (!hash)
		return -ENOMEM;

	/* We have to rehash for the new table anyway, so we also can 
	 * use a new random seed */
	get_random_bytes(&rnd, 4);

	write_lock_bh(&ip_conntrack_lock);
	for (i = 0; i < ip_conntrack_htable_size; i++) {
		while (!list_empty(&ip_conntrack_hash[i])) {
			h = list_entry(ip_conntrack_hash[i].next,
				       struct ip_conntrack_tuple_hash, list);
			list_del(&h->list);
			bucket = __hash_conntrack(&h->tuple, hashsize, rnd);
			list_add_tail(&h->list, &hash[bucket]);
		}
		//ip_conntrack_hash[i].num=0;
	}
	old_size = ip_conntrack_htable_size;
	old_vmalloced = ip_conntrack_vmalloc;
	old_hash = ip_conntrack_hash;

	ip_conntrack_htable_size = hashsize;
	ip_conntrack_vmalloc = vmalloced;
	ip_conntrack_hash = hash;
	ip_conntrack_hash_rnd = rnd;
	write_unlock_bh(&ip_conntrack_lock);

	free_conntrack_hash(old_hash, old_vmalloced, old_size);
	return 0;
}

module_param_call(hashsize, set_hashsize, param_get_uint,
		  &ip_conntrack_htable_size, 0600);

#ifdef CONFIG_NK_DYNAMIC_PORT_NUM
#include <linux/cpld.h>
#endif

#ifdef CONFIG_NK_DYNAMIC_PORT_NUM
void disable_bootbus(void)
{
	uint64_t        mask;

	mask = cvmx_read_csr(CVMX_GPIO_BOOT_ENA);

	mask = mask & DISABLE_BOOTBUS_MASK;
	cvmx_write_csr(CVMX_GPIO_BOOT_ENA, mask);
}

void Get_CPLD_Value() {
	int i, number, flag;

// 	printk(KERN_EMERG "Debug #%d %s: \n", __LINE__, __func__);

	/** Get CPLD Value **/
	for ( i = 0; i < nk_CPLD_input_len; i++ ) {
		number = 0;
		nk_input_data[i] = number;
		nk_CPLD_state.len[i] = number + '0';
		nk_CPLD_state.return_number[i]=1;
	}
	nk_CPLD_state.len[nk_CPLD_input_len] = '\0';
	nk_CPLD_state.length = nk_CPLD_input_len;

	CPLD_control(nk_CPLD_state.len);

// 	for(i=0;i<nk_CPLD_input_len;i++)
// 		printk(KERN_EMERG "Debug #%d %s: CPLD return[%d]=%x \n", __LINE__, __func__, i, nk_CPLD_state.return_number[i]);

	/** Check CPLD Value **/
	// 1. check 1450 cpld
	flag = 0;
	memcpy(nk_MSBq, nk_MSBq_1450, sizeof(int)*nk_MSBq_len);
	memcpy(nk_MSBp, nk_MSBp_1450, sizeof(int)*nk_MSBp_len);
	for ( i = 0; i < nk_CPLD_input_len; i++ ) {
		shift_nk_MSBq();
		shift_nk_MSBp();
		if((XOR_nk_MSBq() ^ XOR_nk_MSBp() ^ nk_input_data[i]) != nk_CPLD_state.return_number[i])
			flag = 1;//fail
	}

	if( !flag ) {//Model is 1450
		useSwitch = SWITCH1450;
		DYNAMIC_NUM_LAN = 5;
		DYNAMIC_NUM_WAN = 8;
		DYNAMIC_NUM_DMZ = 0;
		printk ( KERN_EMERG "Kernel dectect model is GQF1450\n" );
		return;
	}

	// 2. check 1100 cpld
	flag = 0;
	memcpy(nk_MSBq, nk_MSBq_1100, sizeof(int)*nk_MSBq_len);
	memcpy(nk_MSBp, nk_MSBp_1100, sizeof(int)*nk_MSBp_len);
	for ( i = 0; i < nk_CPLD_input_len; i++ ) {
		shift_nk_MSBq();
		shift_nk_MSBp();
		if((XOR_nk_MSBq() ^ XOR_nk_MSBp() ^ nk_input_data[i]) != nk_CPLD_state.return_number[i])
			flag = 1;//fail
	}

	if( !flag ) {//Model is 1100
		useSwitch = SWITCH1100;
		DYNAMIC_NUM_LAN = 8;
		DYNAMIC_NUM_WAN = 5;
		DYNAMIC_NUM_DMZ = 0;
		printk ( KERN_EMERG "Kernel dectect model is GQF1100\n" );
		return;
	}

	// 3. check 650 cpld
	flag = 0;
	memcpy(nk_MSBq, nk_MSBq_650, sizeof(int)*nk_MSBq_len);
	memcpy(nk_MSBp, nk_MSBp_650, sizeof(int)*nk_MSBp_len);
	for ( i = 0; i < nk_CPLD_input_len; i++ ) {
		shift_nk_MSBq();
		shift_nk_MSBp();
		if((XOR_nk_MSBq() ^ XOR_nk_MSBp() ^ nk_input_data[i]) != nk_CPLD_state.return_number[i])
			flag = 1;//fail
	}


	if( !flag ) {//Model is 650
		useSwitch = SWITCH650;
		DYNAMIC_NUM_LAN = 5;
		DYNAMIC_NUM_WAN = 5;
		DYNAMIC_NUM_DMZ = 0;
		printk ( KERN_EMERG "Kernel dectect model is GQF650\n" );
		return;
	}

	/* Default */
	if ( flag ) {
		useSwitch = SWITCH1100;
		DYNAMIC_NUM_LAN = 8;
		DYNAMIC_NUM_WAN = 5;
		DYNAMIC_NUM_DMZ = 0;
		printk ( KERN_EMERG "Kernel dectect unkown model, assume model is GQF1100\n" );
	}

}
#endif
int __init ip_conntrack_init(void)
{
	unsigned int i;
	int ret;

#ifdef CONFIG_NK_DYNAMIC_PORT_NUM
	disable_bootbus();

	printk ( KERN_EMERG "\n--------------------------- Detecting Model ---------------------------\n" );

	Sw_Template_Get_CPLD_Value_Multi_Model();

	printk ( KERN_EMERG "Modle basic Info: useSwitch[%d], LAN[%d], WAN[%d], DMZ[%d]\n", useSwitch, CONFIG_NK_NUM_LAN, CONFIG_NK_NUM_WAN, CONFIG_NK_NUM_DMZ );
	printk ( KERN_EMERG "--------------------------- End Detect --------------------------------\n\n" );
#endif


	/*Yami 20090520, clear list first*/
	for(i=0; i<256; i++)
	{
		nk_ip_mac_bind_entry_list[i] = NULL;
		nk_ip_mac_learn_list[i]=NULL;
	}

	/* Idea from tcp.c: use 1/16384 of memory.  On i386: 32MB
	 * machine has 256 buckets.  >= 1GB machines have 8192 buckets. */
 	if (!ip_conntrack_htable_size) {
		//ip_conntrack_htable_size
		//	= (((num_physpages << PAGE_SHIFT) / 16384)
		//	   / sizeof(struct list_head));
		//if (num_physpages > (1024 * 1024 * 1024 / PAGE_SIZE))
		//ip_conntrack_htable_size = 8192*2;
		//ip_conntrack_htable_size = 262140;
		ip_conntrack_htable_size = DYNAMIC_HASH_SIZE;
		//ip_conntrack_htable_size = 512;
		//if (ip_conntrack_htable_size < 16)
		//	ip_conntrack_htable_size = 16;
	}
	//ip_conntrack_max = 8 * ip_conntrack_htable_size;
	ip_conntrack_max = DYNAMIC_MAX_SESSION;

	printk("ip_conntrack version %s (%u buckets, %d max)"
	       " - %Zd bytes per conntrack\n", IP_CONNTRACK_VERSION,
	       ip_conntrack_htable_size, ip_conntrack_max,
	       sizeof(struct ip_conntrack));

	ret = nf_register_sockopt(&so_getorigdst);
	if (ret != 0) {
		printk(KERN_ERR "Unable to register netfilter socket option\n");
		return ret;
	}

	ip_conntrack_hash = alloc_hashtable(ip_conntrack_htable_size,
					    &ip_conntrack_vmalloc);
	if (!ip_conntrack_hash) {
		printk(KERN_ERR "Unable to create ip_conntrack_hash\n");
		goto err_unreg_sockopt;
	}

	ip_conntrack_cachep = kmem_cache_create("ip_conntrack",
	                                        sizeof(struct ip_conntrack), 0,
	                                        0, NULL, NULL);
	if (!ip_conntrack_cachep) {
		printk(KERN_ERR "Unable to create ip_conntrack slab cache\n");
		goto err_free_hash;
	}

	ip_conntrack_expect_cachep = kmem_cache_create("ip_conntrack_expect",
					sizeof(struct ip_conntrack_expect),
					0, 0, NULL, NULL);
	if (!ip_conntrack_expect_cachep) {
		printk(KERN_ERR "Unable to create ip_expect slab cache\n");
		goto err_free_conntrack_slab;
	}

	/* Don't NEED lock here, but good form anyway. */
	write_lock_bh(&ip_conntrack_lock);
	for (i = 0; i < MAX_IP_CT_PROTO; i++)
		ip_ct_protos[i] = &ip_conntrack_generic_protocol;
	/* Sew in builtin protocols. */
	ip_ct_protos[IPPROTO_TCP] = &ip_conntrack_protocol_tcp;
	ip_ct_protos[IPPROTO_UDP] = &ip_conntrack_protocol_udp;
	ip_ct_protos[IPPROTO_ICMP] = &ip_conntrack_protocol_icmp;
	write_unlock_bh(&ip_conntrack_lock);

	/* For use by ipt_REJECT */
	ip_ct_attach = ip_conntrack_attach;

	/* Set up fake conntrack:
	    - to never be deleted, not in any hashes */
	atomic_set(&ip_conntrack_untracked.ct_general.use, 1);
	/*  - and look it like as a confirmed connection */
	set_bit(IPS_CONFIRMED_BIT, &ip_conntrack_untracked.status);

#ifdef CONFIG_NK_IPSEC_SPLITDNS
	//-->20100108 trenchen : support split dns
	spin_lock_init( &(SplitdnsHashLock) );	
	//<-------
#endif

	init_ioctl();

	return ret;

err_free_conntrack_slab:
	kmem_cache_destroy(ip_conntrack_cachep);
err_free_hash:
	free_conntrack_hash(ip_conntrack_hash, ip_conntrack_vmalloc,
			    ip_conntrack_htable_size);
err_unreg_sockopt:
	nf_unregister_sockopt(&so_getorigdst);

	return -ENOMEM;
}

#ifdef CONFIG_NK_IPFILTER_SUPPORT_SORTING
int num=0;
extern web_service_port_t *web_service_port_list;

void nk_update_delta_count(void)
{
	int i,j,k,l;
	struct ip_conntrack_tuple_hash *h;
	struct ip_conntrack *ct;
	int cnt_period;
	int sdir=0, temp=0, s_temp=0;
	smart_qos_ip_t *tmp_qos_ip,get_qos_ip,*tmp_qos_ip2, *tmp_copy_qos_ip, *tmp;

	if(fw_setting.cal_cnt_period!=0)
		cnt_period = fw_setting.cal_cnt_period;
	else	cnt_period = 60;

	k = hashtmp_size;
	if(k!=0) k++;
	if(k>=ip_conntrack_htable_size) k=0;
	//hashtmp_size = k+(ip_conntrack_htable_size/CAL_CNT_PERIOD);
	hashtmp_size = k+(ip_conntrack_htable_size/cnt_period);
	if(hashtmp_size > ip_conntrack_htable_size) 
		hashtmp_size = ip_conntrack_htable_size;
	//printk(KERN_EMERG "k=%d, hashtmp_size=%d\n", k, hashtmp_size);
	//printk(KERN_EMERG "tr_enable=%d, smartqos=%d\n", fw_setting.tr_enable, fw_setting.smart_qos);
	if( (fw_setting.tr_enable==1)||(fw_setting.smart_qos==1) )
	{
		//printk(KERN_EMERG "nk_update_delta_count\n");
		/*save session_num to session_num_pre*/
		if(hashtmp_size == ip_conntrack_htable_size) 
		{
			for(i=0; i<MAX_WAN_NUM; i++)
			{
				session_num_pre[i] = session_num[i];
				session_num[i] = session_num_tmp[i];
				session_num_tmp[i]=0;
				//printk("wan%d = %d\n", i+1, session_num_pre[i]);
			}
		}
#if 0
		for(i=0; i<MAX_WAN_NUM; i++)
		{
			printk(KERN_EMERG "wan%d #session= %d\n", i+1, session_num[i]);
			printk(KERN_EMERG "wan%d #session_pre= %d\n", i+1, session_num_pre[i]);
			printk(KERN_EMERG "wan%d #session_tmp= %d\n", i+1, session_num_tmp[i]);
			printk(KERN_EMERG "\n");
		}
#endif
		write_lock_bh(&ip_conntrack_lock);
		for (i = k; i < hashtmp_size; i++)
		{
			list_for_each_entry(h, &ip_conntrack_hash[i], list)
			{
				ct = tuplehash_to_ctrack(h);
			
				if( (ct) && (ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip!=wan_interface_session[0].lanip) &&
				   (ct->tr.flag==0) )
				{

					ct->tr.ct_bytes_delta = delta_tr(ct->tr.b_cnt, ct->tr.ct_bytes_pre);

					ct->tr.ct_bytes_pre = ct->tr.b_cnt;
						
					ct->tr.down_bytes_delta = delta_tr(ct->tr.down_bytes, ct->tr.down_bytes_pre);
					ct->tr.down_bytes_pre = ct->tr.down_bytes;
					
					ct->tr.up_bytes_delta = delta_tr(ct->tr.up_bytes, ct->tr.up_bytes_pre);
					ct->tr.up_bytes_pre = ct->tr.up_bytes;

					/* count real time session number*/
					for(j=0; j<MAX_WAN_NUM; j++)
					{
						//if(wan_interface_session[j].wan_ip==0) continue;
						if(wan_interface_session[j].wan_ip>0)
						{
							if((ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.ip == wan_interface_session[j].wan_ip) ||
							(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip == wan_interface_session[j].wan_ip))
							{
								session_num_tmp[j]++;
								s_temp=1;
								break;
							}
	#ifdef CONFIG_NK_TRANSPARENT_BRIDGE_RANGE_NUM
							if(s_temp==0)
							{
								for(l=0;l<CONFIG_NK_TRANSPARENT_BRIDGE_RANGE_NUM;l++)
								{
									if((ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip>=wan_interface_session[j].internallanip1[l])&&
								(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip<=wan_interface_session[j].internallanip2[l]))
									{
										session_num_tmp[j]++;
										s_temp=1;
										break;
									}
								}
							}
	#else
							if(s_temp==0)
							{
								if((ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.ip == wan_interface_session[j].wan_ip) ||
								(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip == wan_interface_session[j].wan_ip) ||
								((ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip>=wan_interface_session[j].internallanip1)&&
								(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip<=wan_interface_session[j].internallanip2))||
								((ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip>=wan_interface_session[j].internallanip3)&&
								(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip<=wan_interface_session[j].internallanip4))||
								((ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.ip>=wan_interface_session[j].internallanip1)&&
								(ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.ip<=wan_interface_session[j].internallanip2))||
								((ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.ip>=wan_interface_session[j].internallanip3)&&
								(ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.ip<=wan_interface_session[j].internallanip4)))
								{
									session_num_tmp[j]++;
									s_temp=1;
									break;
								}
							}
#endif
						}
					}

					/* count real time session number for 1-1 nat ip*/
					if(s_temp==0)
					{
						for(j=0; j<MAX_WAN_NUM; j++)
						{
							//printk(KERN_EMERG "ip[%u.%u.%u.%u] mask[%u.%u.%u.%u]\n", NIPQUAD(wan_interface_session[i].wan_ip), NIPQUAD(wan_interface_session[i].wan_mask));
							if(wan_interface_session[j].wan_ip>0)
							{
								if( ( ((ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.ip)&(wan_interface_session[j].wan_mask)) == ((wan_interface_session[j].wan_ip)&(wan_interface_session[j].wan_mask)) ) || ( ((ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip)&(wan_interface_session[j].wan_mask)) == ((wan_interface_session[j].wan_ip)&(wan_interface_session[j].wan_mask)) ) )
								{
									session_num_tmp[j]++;
									s_temp=1;
									//printk(KERN_EMERG "session ++ [%d]\n", session_num_tmp[j]);
									break;
								}
							}
						}
					}
					/* count real time session number for router+nat mode ip, 20090728*/
					if(s_temp==0)
					{
						for(j=0; j<MAX_WAN_NUM; j++)
						{
							if(wan_interface_session[j].wan_ip>0)
							{
								for(k=0;k<3;k++)
								{
									if(wan_interface_session[j].routerinfo[k].rip1>0)
									{
										//printk(KERN_EMERG "sip=%u.%u.%u.%u, rip1=%u.%u.%u.%u-%u.%u.%u.%u, rip2=%u.%u.%u.%u-%u.%u.%u.%u\n", NIPQUAD(ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.ip), NIPQUAD(wan_interface_session[j].routerinfo[k].rip1), NIPQUAD(wan_interface_session[j].routerinfo[k].rip2), NIPQUAD(wan_interface_session[j].routerinfo[k].rip3), NIPQUAD(wan_interface_session[j].routerinfo[k].rip4));
			
										if(((ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.ip>=wan_interface_session[j].routerinfo[k].rip1)&&
										(ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.ip<=wan_interface_session[j].routerinfo[k].rip2)) || ((ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.ip>=wan_interface_session[j].routerinfo[k].rip3)&&
										(ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.ip<=wan_interface_session[j].routerinfo[k].rip4)) || (ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.ip==wan_interface_session[j].routerinfo[k].rgw) )
										{
											session_num_tmp[j]++;
											s_temp=1;
										//	printk(KERN_EMERG "[WAN%d]session ++ [%d]\n", j+1, session_num_tmp[j]);
											break;
										}
									}
								}
							}
						}
					}
					/*Dahon, count session belong to DMZ port*/
					if(s_temp==0)
					{
						if( (ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.ip>=wan_interface_session[0].dmz_start)&&
						(ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.ip<=wan_interface_session[0].dmz_end) )
						{
							session_num_tmp[CONFIG_NK_NUM_WAN]++;
							s_temp=1;
						}
					}
					/* Yami, 2007/9/21 qosipsort for smart qos*/
					sdir=0;
					//printk(KERN_EMERG "MAX_WAN_NUM=%d\n", MAX_WAN_NUM);
					for(k=0;k<MAX_WAN_NUM;k++)
					{
						//printk(KERN_EMERG "wan%d [%u.%u.%u.%u] dst[%u.%u.%u.%u]\n", k+1, NIPQUAD(wan_interface_session[k].wan_ip), NIPQUAD(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip));
						//printk(KERN_EMERG "wan%d %u.%u.%u.%u~%u.%u.%u.%u %u.%u.%u.%u~%u.%u.%u.%u\n", k+1, NIPQUAD(wan_interface_session[k].internallanip1), NIPQUAD(wan_interface_session[k].internallanip2), NIPQUAD(wan_interface_session[k].internallanip3), NIPQUAD(wan_interface_session[k].internallanip4));
#ifdef CONFIG_NK_TRANSPARENT_BRIDGE_RANGE_NUM
						if(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip==wan_interface_session[k].wan_ip)
						{
							sdir=2; //session direction, 2=forwaring
							break;
						}

						for(l=0;l<CONFIG_NK_TRANSPARENT_BRIDGE_RANGE_NUM;l++)
						{
							if ((ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip>=wan_interface_session[k].internallanip1[l])&&
								(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip<=wan_interface_session[k].internallanip2[l]))
							{
								sdir=2; //session direction, 2=forwaring
								break;
							}
						}
#else
						if( (ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip==wan_interface_session[k].wan_ip)  ||
						    ((ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip>=wan_interface_session[k].internallanip1)&&
						    (ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip<=wan_interface_session[k].internallanip2)) ||
						    ((ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip>=wan_interface_session[k].internallanip3)&&
						    (ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip<=wan_interface_session[k].internallanip4)) )
						{
							sdir=2; //session direction, 2=forwaring
							break;
						}
#endif
					}
	
					/*DMZ range*/
					if( (ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip>=wan_interface_session[0].dmz_start)&&
					    (ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip<=wan_interface_session[0].dmz_end) )
						sdir=2;

					/*DMZ subnet*/
					if((ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip&wan_interface_session[0].dmz_mask)==(wan_interface_session[0].dmz_start&wan_interface_session[0].dmz_mask) )
						sdir=2;
					//router+nat mode, 20090804
					for(j=0; j<MAX_WAN_NUM; j++)
					{
						if(wan_interface_session[j].wan_ip>0)
						{
							for(k=0;k<3;k++)
							{
								if(wan_interface_session[j].routerinfo[k].rip1>0)
								{
									if(((ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip>=wan_interface_session[j].routerinfo[k].rip1)&&
									(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip<=wan_interface_session[j].routerinfo[k].rip2)) || ((ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip>=wan_interface_session[j].routerinfo[k].rip3)&&
									(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip<=wan_interface_session[j].routerinfo[k].rip4)) )
									{
										sdir=2;
										break;
									}
									else if(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip==wan_interface_session[j].routerinfo[k].rgw)
									{
										sdir=2;
										break;
									}
								}
							}
						}
					}


					//printk(KERN_EMERG "dir=%d session %u.%u.%u.%u -> %u.%u.%u.%u  %u.%u.%u.%u -> %u.%u.%u.%u\n", sdir,
					//NIPQUAD(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip), NIPQUAD(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip),
					//NIPQUAD(ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.ip), NIPQUAD(ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.ip));

					if((fw_setting.smart_qos==1)||(fw_setting.tr_enable==1))
					{
						for(tmp_qos_ip = tmp_smart_qos_ip_list;tmp_qos_ip;tmp_qos_ip = tmp_qos_ip->next)
						{
							if((tmp_qos_ip->ip.s_addr == ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip) || (tmp_qos_ip->ip.s_addr == ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.ip)
							 || (tmp_qos_ip->ip.s_addr == ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip) || (tmp_qos_ip->ip.s_addr == ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.ip))
							{
								tmp_qos_ip->up_bw += ct->tr.up_bytes_delta;
								tmp_qos_ip->down_bw += ct->tr.down_bytes_delta;
								tmp_qos_ip->session ++;
								break;
							}
						}
						if(!tmp_qos_ip)
						{
							if( (sdir==2 && ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.ip>0) ||
							    (sdir==0 && ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip>0) )
							{
								tmp_qos_ip = (smart_qos_ip_t *)kmalloc(sizeof(smart_qos_ip_t),GFP_ATOMIC);
								if(tmp_qos_ip)
								{
									bzero((char *)tmp_qos_ip, sizeof(smart_qos_ip_t));
									if(sdir==2)
										tmp_qos_ip->ip.s_addr = ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.ip;
									else
										tmp_qos_ip->ip.s_addr = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip;
				
									for(j=0; j<MAX_WAN_NUM; j++)
									{
										if(tmp_qos_ip->ip.s_addr == wan_interface_session[j].wan_ip)
										{
											temp=1;
											break;
										}
									}
									if(temp==0)
									{
										tmp_qos_ip->up_bw = ct->tr.up_bytes_delta;
										tmp_qos_ip->down_bw = ct->tr.down_bytes_delta;
										tmp_qos_ip->session = 1;
										tmp_qos_ip->next = tmp_smart_qos_ip_list;
										tmp_smart_qos_ip_list = tmp_qos_ip;
									}
									else
										kfree(tmp_qos_ip);
									//printk(KERN_EMERG "add new[%u.%u.%u.%u]\n", NIPQUAD(tmp_qos_ip->ip.s_addr));
									//num++;
								}
							}
						}
					}
					ct->tr.flag=1;
				}
			}
		}
		write_unlock_bh(&ip_conntrack_lock);

		if(hashtmp_size == ip_conntrack_htable_size) 
		{
			write_lock_bh(&ip_conntrack_lock);
			for (i = 0; i < hashtmp_size; i++)
			{
				list_for_each_entry(h, &ip_conntrack_hash[i], list) 
				{
					ct = tuplehash_to_ctrack(h);
					ct->tr.flag = 0;
				}
			}
			write_unlock_bh(&ip_conntrack_lock);

			if((fw_setting.smart_qos==1)||(fw_setting.tr_enable==1))
			{
#if 0
				if(tmp_smart_qos_ip_list)
				{
					nk_smart_qos_ip_list=tmp_smart_qos_ip_list;

					tmp_qos_ip2 = tmp_smart_qos_ip_list;
					while(tmp_qos_ip2)
					{
			
						tmp_smart_qos_ip_list = tmp_qos_ip2->next;
						kfree(tmp_qos_ip2);
						tmp_qos_ip2 = tmp_smart_qos_ip_list;
					}
					tmp_smart_qos_ip_list = NULL;

				}
#endif
//====================================================================
				// clear nk_smart_qos_ip_list
				tmp_qos_ip2 = nk_smart_qos_ip_list;
				while(tmp_qos_ip2)
				{
					nk_smart_qos_ip_list = tmp_qos_ip2->next;
					kfree(tmp_qos_ip2);
					tmp_qos_ip2 = nk_smart_qos_ip_list;
				}
				nk_smart_qos_ip_list = NULL;

				//copy tmp_smart_qos_ip_list to nk_smart_qos_ip_list
				tmp_copy_qos_ip = tmp_smart_qos_ip_list;
				while(tmp_copy_qos_ip)
				{
				//printk(KERN_EMERG "tmp_qos_ip->up_bw=%d", tmp_qos_ip->up_bw);
					tmp = (smart_qos_ip_t *)kmalloc(sizeof(smart_qos_ip_t),GFP_ATOMIC);
					tmp->ip.s_addr = tmp_copy_qos_ip->ip.s_addr;
					tmp->up_bw = tmp_copy_qos_ip->up_bw;
					tmp->down_bw = tmp_copy_qos_ip->down_bw;
					tmp->session = tmp_copy_qos_ip->session;
					tmp->next = nk_smart_qos_ip_list;
					nk_smart_qos_ip_list = tmp;
		
				tmp_copy_qos_ip = tmp_copy_qos_ip->next;
				}
				//clear tmp_smart_qos_ip_list
				tmp_qos_ip2 = tmp_smart_qos_ip_list;
				while(tmp_qos_ip2)
				{
					tmp_smart_qos_ip_list = tmp_qos_ip2->next;
					kfree(tmp_qos_ip2);
					tmp_qos_ip2 = tmp_smart_qos_ip_list;
				}
				tmp_smart_qos_ip_list = NULL;

//===================================================================
			}
#if 0
			printk(KERN_EMERG "cpy tmp_smart_qos_ip_list to nk_smart_qos_ip_list\n");
			if(nk_smart_qos_ip_list)
			{
				tmp_qos_ip2 = nk_smart_qos_ip_list;
				while(tmp_qos_ip2)
				{
					printk(KERN_EMERG "1. %u.%u.%u.%u up=%d, down=%d\n", NIPQUAD(tmp_qos_ip2->ip.s_addr), ntohs(tmp_qos_ip2->up_bw), ntohs(tmp_qos_ip2->down_bw));
					tmp_qos_ip2 = tmp_qos_ip2->next;
				}
			}
			printk(KERN_EMERG "====================================\n");
			num=0;
#endif
		}
		
	}
	
	if(web_service_port_list)
	{
		web_service_port_t *temp, **temp_p;

		for(temp_p=&web_service_port_list; temp=*temp_p; )
		{
			temp->age--;
			if( temp->age>0 )
			{
				temp_p=&(temp->next);
				continue;
			}
       //printk("timer del: src[%x] dst[%x] protocol[%d] srcport[%d] dstport[%d]\n",temp->saddr,temp->daddr,temp->protocol_type,temp->src_port,temp->dst_port);
			*temp_p = temp->next;
			kfree(temp);
		}
	}
	
	update_delta_timer_set();
}


void update_delta_timer_set(void)
{
	if(&update_delta_timer)	
		del_timer(&update_delta_timer);
	update_delta_timer.expires = jiffies + 1 * HZ;
	init_timer(&update_delta_timer);
	add_timer(&update_delta_timer);
}

delta_tr(u_int64_t b_cnt, u_int64_t pre)
{
	if(pre>b_cnt)
		return (b_cnt + ~pre + 1);
	else 
	return (b_cnt - pre);
}
#endif

#ifdef CONFIG_NK_RESTRICT_APP
exception_qq_t *search_prev_qqNum(exception_qq_t *user)
{
	exception_qq_t *tmp = NULL;

	tmp = exception_qq_list;
	while(tmp)
	{
		if (tmp->next == user)
			break;
		tmp = tmp->next;
	}

	return tmp;
}

unsigned int check_block_qq(struct sk_buff **pskb)
{
	unsigned char *ptr;
	int i, order, offset, payload_len;
	exception_qq_t *exception_qq_p;
	qq_number_t qqNumber;

	// only check output packet
	if( !(((*pskb)->dev!=NULL) && (!strcmp((*pskb)->dev, wan_interface_session[0].laninterface))) )
	{
		return NF_ACCEPT;
	}

	ptr = (*pskb)->data;

	if( (*pskb)->nh.iph->protocol == IPPROTO_UDP )
	{
		struct udphdr *udp;

		udp = (struct udphdr *)((u_int32_t*)(*pskb)->nh.iph + (*pskb)->nh.iph->ihl);
		if( ntohs(udp->dest) == 8000 )
		{
			for(i=0; i<100; i++)
			{
				if(restrict_app.except_e_ip[i] &&
				   ntohl((*pskb)->nh.iph->saddr) >= ntohl(restrict_app.except_s_ip[i]) &&
				   ntohl((*pskb)->nh.iph->saddr) <= ntohl(restrict_app.except_e_ip[i]) )
				{
					return NF_ACCEPT;
				}
			}

			if(exception_qq_list)
			{
				offset = sizeof(struct iphdr) + sizeof(struct udphdr);
				payload_len = ntohs((*pskb)->nh.iph->tot_len) - offset;
				ptr+=offset;
				if (payload_len <= 2 )
					return NF_DROP;

				if( (*(ptr) == 0x02) && (*(ptr+payload_len-1) == 0x03) )
				{
					memcpy(&qqNumber.cNum[0], ptr+7, 4);
					exception_qq_p = exception_qq_list;
					order = 0;
					while(exception_qq_p)
					{
						if(exception_qq_p->qq_Num==ntohl(qqNumber.iNum))
						{
							//change the QQ number order in exception_qq_list when the order >= LIMIT_FOR_CHANGE_ORDER,
							if(order >= LIMIT_FOR_CHANGE_ORDER)
							{
								exception_qq_t *prev = NULL;
								if(prev = search_prev_qqNum(exception_qq_p))
								{
									prev->next = exception_qq_p->next;
									exception_qq_p->next = exception_qq_list;
									exception_qq_list = exception_qq_p;
								}
							}
							return NF_ACCEPT;
						}
						exception_qq_p=exception_qq_p->next;
						order++;
					}
				}
			}
			return NF_DROP;
		}
	}
	else if( (*pskb)->nh.iph->protocol == IPPROTO_TCP )
	{
		struct tcphdr *tcp;

		tcp = (struct tcphdr *)((u_int32_t*)(*pskb)->nh.iph + (*pskb)->nh.iph->ihl);
		if( (ntohs(tcp->dest) == 80) || (ntohs(tcp->dest) == 443) )
		{
			for(i=0; i<100; i++)
			{
				if(restrict_app.except_e_ip[i] &&
				   ntohl((*pskb)->nh.iph->saddr) >= ntohl(restrict_app.except_s_ip[i]) &&
				   ntohl((*pskb)->nh.iph->saddr) <= ntohl(restrict_app.except_e_ip[i]) )
				{
					return NF_ACCEPT;
				}
			}

			offset = sizeof(struct iphdr) + sizeof(struct tcphdr);
			payload_len = ntohs((*pskb)->nh.iph->tot_len) - offset;
			ptr+=offset;
			if (payload_len <= 2 )
				return NF_ACCEPT;

			if( (*(ptr+2) == 0x02) && (*(ptr+payload_len-1) == 0x03) )
			{
				return NF_DROP;
			}
		}
	}

	return NF_ACCEPT;
}
#endif
