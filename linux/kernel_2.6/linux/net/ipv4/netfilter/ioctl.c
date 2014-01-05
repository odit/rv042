#include <linux/module.h>
#include <linux/skbuff.h>
#include <net/sock.h>
#include <linux/cdev.h>
#include <linux/in.h>

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/syscalls.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/fcntl.h>
#include <asm/uaccess.h>

#if 0
#include <stdio.h>
#include <linux/netfilter_ipv4/ipt_webstr.h>
#include <linux/netfilter_ipv4/ipt_time.h>
#endif
#include <linux/netfilter_ipv4/ioctl.h>
#include <linux/netfilter_ipv4/ip_conntrack.h>
#include <linux/netfilter_ipv4/ip_conntrack_core.h>
#include <linux/netfilter_ipv4/listhelp.h>
#include "../../../../nk_switch/Voice_cpld.h"
#include "../../../../nk_switch/switch.h"

#if CONFIG_NK_URL_TMUFE_FILTER
#include "url_filter/url_filter_mod.h"
#endif

MODULE_LICENSE("GPL");
/**
 *	Global Variable 
 */
int err;
dev_t dev;
struct cdev *my_cdev;

/** Global Variable **/
#ifdef CONFIG_NK_DYNAMIC_PORT_NUM
#include <linux/dynamic_port_num.h>
#endif


/* This rwlock protects the main hash table, protocol/helper/expected
   registrations, conntrack timers*/
#define ASSERT_READ_LOCK(x)
#define ASSERT_WRITE_LOCK(x)

/* arp spoof protect incifer 2006/06/28 */
#ifdef CONFIG_SUPPORT_ARP_SPOOF_PROTECT
	#include <linux/if_arp.h>
#endif
#define SORTING_ENTRY 20
#define MAX_WAN_NUM (CONFIG_NK_NUM_WAN+CONFIG_NK_NUM_DMZ)
#ifdef CONFIG_NK_DYNAMIC_PORT_NUM
interface_session_t wan_interface_session[CONFIG_NK_NUM_MAX_WAN];
#else
interface_session_t wan_interface_session[MAX_WAN_NUM];
#endif

//Rain add for Anti-Virus
#ifdef CONFIG_NK_ANTI_VIRUS
struct anti_virus_setting av_setting;
#endif
struct firewall_setting fw_setting;

#define DEBUG 0
#define NMACQUAD(mac) mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]
static int url_ioctl(struct inode *inode, struct file *file,
		      unsigned int cmd, unsigned long arg);

/* purpose     : 0012927    author : paul.chen    date : 2010-07-21 */
/* description : dhcp-relay error when dhcp server behind nat       */
u_int32_t lanip=0;

#ifdef CONFIG_NK_DYNAMIC_PORT_NUM
extern int session_num_pre[CONFIG_NK_NUM_MAX_WAN], session_num[CONFIG_NK_NUM_MAX_WAN];
#else
extern int session_num_pre[MAX_WAN_NUM], session_num[MAX_WAN_NUM];
#endif

extern unsigned int ip_conntrack_htable_size;
extern int hashtmp_size;

#ifdef CONFIG_SUPPORT_IP_MAC_BINDING
#define IP_MAC_ENTRY_HASH_SIZE 256
#define IP_MAC_ENTRY_HASH_IDX 255

ip_mac_bind_entry_t *nk_ip_mac_bind_entry_list[IP_MAC_ENTRY_HASH_SIZE];
ip_mac_bind_entry_t *nk_ip_mac_learn_list[IP_MAC_ENTRY_HASH_SIZE];
ip_mac_bind_entry_t *ip_mac_bind_entry,*ip_mac_learn_entry;
int ip_mac_number=0;
nk_fw_opt_t nk_fw_opts;
int ip_mac_learn_entry_cnt;
#endif

#ifdef CONFIG_NK_IPSEC_NETBIOS_BC
extern unsigned int NetbiosHashGet(unsigned int key);
#define BIOSHASHMAX 256
struct nat_ipsec_bc_head {
	nat_ipsec_bc_t *start;
	spinlock_t BiosBroadLock;
};
struct nat_ipsec_bc_head nat_ipsec_bc_ip_list[BIOSHASHMAX];
#endif

#ifdef CONFIG_NK_IPSEC_SPLITDNS
//--->20100106 trenchen : support split dns
struct ipsec_split_dns_head ipsec_split_dns_list;
//<---
#endif

#if 0
/*2006/07/05 Ryoko add support netbios*/
#ifdef CONFIG_IPSEC_NETBIOS_BC
nat_ipsec_bc_t *nat_ipsec_bc_ip_list = NULL;	//add Ryoko for netbios
#endif
#endif

nk_agingtime_t *nk_nk_agingtime_list[128];
int nk_aging_entry_cnt;

#ifdef CONFIG_NK_SESSION_LIMIT
u_int32_t session_limit_status = 0;
u_int32_t session_limit_max_session1 = 0;
u_int32_t session_limit_max_session2 = 0;
u_int32_t session_limit_max_tcp = 0;
u_int32_t session_limit_max_udp = 0;
u_int32_t session_limit_block_minute1 = 0;
u_int32_t session_limit_block_minute2 = 0;
unsigned int scheduler = 0;
unsigned int days_match = 0;
unsigned int time_start = 0;
unsigned int time_stop = 0;
session_limit_t *nk_session_limit[128];
int session_limit_entry_cnt;
#endif
#ifdef CONFIG_NK_SESSION_LIMIT_ENHANCE/* Session Limit Enhance incifer 2006/09/03 */
session_limit_enhance_node_list_t *nk_session_limit_enhance_node_list[256];
static void session_limit_enhance_order(session_limit_enhance_list_t *);
int session_limit_enhance_rule_check(u_int32_t ip,int port, int protocol, int cnt);
session_limit_enhance_list_t *nk_session_limit_enhance_list;
session_limit_enhance_list_t *nk_session_limit_enhance_alldomain_list;
session_limit_enhance_list_t *nk_session_limit_enhance_cclass_list;
session_limit_enhance_node_list_t *node;

DEFINE_RWLOCK(SLE_LOCK);
DEFINE_RWLOCK(SLE_ALLDOMAIN_LOCK);
DEFINE_RWLOCK(SLE_CCLASS_LOCK);
#endif
DEFINE_RWLOCK(SLE_NODE_LOCK);

#ifdef CONFIG_NK_SESSION_LIMIT
void session_limit_timer(void);
void nk_session_limit_timer_set(void);
static struct timer_list nk_session_limit_timer = {
	function:&session_limit_timer
};
#endif

#ifdef CONFIG_NK_RESTRICT_APP
restrict_app_t restrict_app;
exception_qq_t *exception_qq_list = NULL;
#endif

//#ifdef CONFIG_NK_QOS_SCHED
#if 1
smart_qos_ip_t *nk_smart_qos_ip_list=NULL;
smart_qos_ip_t *nk_smart_qos_ip_up_list = NULL;
smart_qos_ip_t *nk_smart_qos_ip_down_list = NULL;
#endif

extern int nk_pingofdeath;
extern int nk_denypolicy;

voice_setting_t vs;
void nk_voice_alert_queue(unsigned char* message, int priority, int interval);
EXPORT_SYMBOL(nk_voice_alert_queue);
EXPORT_SYMBOL(vs);
alarm_msg_t *nk_msg_low=NULL;
alarm_msg_t *nk_msg_high=NULL;
alarm_msg_t *nk_msg_played=NULL;
EXPORT_SYMBOL(nk_msg_low);
EXPORT_SYMBOL(nk_msg_high);
EXPORT_SYMBOL(nk_msg_played);
char ip_msg[120];
char* ip2vmsg(u32 ip);
EXPORT_SYMBOL(ip_msg);
EXPORT_SYMBOL(ip2vmsg);
struct tm
{
	int tm_sec;                   /* Seconds.     [0-60] (1 leap second) */
	int tm_min;                   /* Minutes.     [0-59] */
	int tm_hour;                  /* Hours.       [0-23] */
	int tm_mday;                  /* Day.         [1-31] */
	int tm_mon;                   /* Month.       [0-11] */
	int tm_year;                  /* Year - 1900.  */
	int tm_wday;                  /* Day of week. [0-6] */
	int tm_yday;                  /* Days in year.[0-365] */
	int tm_isdst;                 /* DST.         [-1/0/1]*/
};


//==============
static smart_qos_ip_t *copy_qos_ip = NULL;
//==============

/*
 * This is the interface device's file_operations structure
 */
static struct file_operations url_fops = {
	.owner		= THIS_MODULE,
	.ioctl		= url_ioctl,
};

ip_session_t *nk_ip_session_list;
/* arp spoof protect incifer 2006/06/28 */
#ifdef CONFIG_SUPPORT_ARP_SPOOF_PROTECT
void nk_update_arp_protect(void);
void update_arp_timer_set(void);
void del_arp_timer(void);
static struct timer_list update_arp_timer = {
	function:&nk_update_arp_protect
};
static nk_send_arp_t nk_send_arp_fw;
static int nk_send_arp_enable = 0;
static void nk_set_static_arp(ip_mac_bind_entry_t*);
static void nk_del_static_arp(ip_mac_bind_entry_t*);
#endif
//-->

#if CONFIG_NK_IPFILTER_SUPPORT_SORTING
//extern traffic_t *nk_traffic_p;
ip_sort_t *ip_sort_list; 
#endif

/* add by chihmou, support pptp trunking 2008/06/11 */
#ifdef CONFIG_NK_PPTP_TRUNKING
#include <linux/pptp_trunking.h>
#endif

ip_session_t *ip_list;

web_service_port_t *web_service_port_list = NULL;

/* support IpBalance Struct Aging -- incifer 2009/01 */
#ifdef CONFIG_IPBALANCE_STRUCT_AGING
//	#define IPBALANCE_STRUCT_AGING_START_TIME		1
//	#define IPBALANCE_STRUCT_AGING_PERIOD_TIME	60
	int proc_ipbalance_struct_aging_start_time=1;
	int proc_ipbalance_struct_aging_period_time=180;
	void ipbalance_struct_check(void);
	void set_ipbalance_struct_aging_timer(int t);
	void del_ipbalance_struct_aging_timer(void);
	struct timer_list ipbalance_struct_aging_timer = {
		function:&ipbalance_struct_check
	};
	/* copy from route.c */
	struct IpBalanceInfo {
		struct IpBalanceInfo *next;
		u32 srcip;
		struct net_device *out;
		unsigned char table;
		unsigned long lastuse;
	/* support IP balance enhancement: flush cache after 40s -- incifer 2008/11 */
	#ifdef CONFIG_IPBALANCE_ENHANCE
		/**
			record interface
		**/
		int inf;
	#endif
	};
	struct IpBalanceHead {
		struct IpBalanceInfo *next;
		struct IpBalanceInfo **end;
	};
	extern struct IpBalanceHead IpBalInfoHead[256];
	extern unsigned int IpSecNum;


/**
	@search fragment introduction:
	we search "session cnt" == 0 from nk_session_limit[].
	nk_session_limit[] has 128 buckets(idx num), and we search "session cnt" == 0 from 16 buckets(X) per time.
	128(idx num)/16(X) = 8(Y)
	nk_session_limit[] divides into 8(Y) portions, and we search a portion of nk_session_limit[].
	that is to say that a portion has 16 buckets(X).
	session_idx_start is the portion we search now.
**/
	int session_idx_start=0;
	#define X_BUCKET		16
	#define Y_PORTION	8
#endif

#ifdef CONFIG_NK_URL_TMUFE_FILTER
url_filter_setting_t url_filter_setting;
url_filter_statics_t url_filter_statics;
url_filter_lic_t url_filter_lic;
url_filter_debug_t url_filter_debug;
url_class_info url_info;
int (*restart_url_filter_mod)(void ) = NULL;
EXPORT_SYMBOL(restart_url_filter_mod);
#endif

/*********************************************/
/*
 * File operations functions for control device
 */
static int url_ioctl(struct inode *inode, struct file *file,
		      unsigned int cmd, unsigned long arg)
{
   //extern int ip_conntrack_num;
   int ret = 0;
   int l;
/*2007/12/14 trenchen : support netbios broadcast*/
#ifdef CONFIG_NK_IPSEC_NETBIOS_BC
//#ifdef CONFIG_IPSEC_NETBIOS_BC
	nat_ipsec_bc_t *new_bc_ip, *del_bc_ip, **bc_ip_list; //david
	int BiosIndex=0;
	int error = 0;	
#endif

#ifdef CONFIG_NK_IPSEC_SPLITDNS
	//-->20100106 trenchen : support split dns
	ipsec_split_dns_t *spdnsnew = 0;
	int errorsdns = 0;
	//<----
#endif

   switch (cmd) {
#if 0
    case SIOCGETSESSIONNUMBER:
        {
	   copy_to_user(NULL, &ip_conntrack_num, sizeof(ip_conntrack_num));
	   printk("Session number= %d\n", ip_conntrack_num);
	   break;
	}
#endif	
/* add by chihmou, support pptp trunking 2008/06/11 */
#ifdef CONFIG_NK_PPTP_TRUNKING
    case SIOCSETPPTPTRUNKINGTABLE :
	{	struct RulePortTable *temp;
		struct PortTableHead *PortTableHeadPtr=NULL;
		extern int RulePortTableSize;
		extern struct RulePortTable *RulePortTableHead;
		extern rwlock_t RulePortTableLock;
		int count=0;
		printk(KERN_EMERG "chihmou debug: SIOCSETPPTPTRUNKINGTABLE, start!!\n");
		PortTableHeadPtr = kmalloc(sizeof(struct PortTableHead), GFP_ATOMIC);
		if( !PortTableHeadPtr )
			break;
		copy_from_user(PortTableHeadPtr, arg, sizeof(struct PortTableHead));
		temp = kmalloc(sizeof(struct RulePortTable)*(PortTableHeadPtr->TableSize), GFP_ATOMIC);
		copy_from_user(temp, PortTableHeadPtr->RulePortPtr, sizeof(struct RulePortTable)*(PortTableHeadPtr->TableSize));

		write_lock_bh(RulePortTableLock);
		RulePortTableSize = PortTableHeadPtr->TableSize;
		RulePortTableHead = temp;
		write_unlock_bh(RulePortTableLock);

		kfree(PortTableHeadPtr);

		//test
// 		printk(KERN_EMERG "chihmou debug : RulePortTableSize = %d\n", RulePortTableSize);
		for(count=0; count < RulePortTableSize; count++)
		{
			printk(KERN_EMERG "count[%d] sport[%d] dport[%d] type[%d]\n", count, RulePortTableHead[count].sport, RulePortTableHead[count].eport, RulePortTableHead[count].type);
		}
		printk(KERN_EMERG "chihmou debug: SIOCSETPPTPTRUNKINGTABLE, finish!!\n");
		break;
	}
    case SIOCDISPPTPTRUNKINGTABLE:
 	{	struct RulePortTable *temp;
		extern int RulePortTableSize;
		extern struct RulePortTable *RulePortTableHead;
		extern rwlock_t RulePortTableLock;

//		printk(KERN_EMERG "chihmou debug: %s, SIOCDISPPTPTRUNKINGTABLE, start!!\n", __FILE__);
		temp = RulePortTableHead;
		write_lock_bh(RulePortTableLock);
		RulePortTableSize = 0;
		RulePortTableHead = NULL;
		write_unlock_bh(RulePortTableLock);
		kfree(temp);
//		printk(KERN_EMERG "chihmou debug: %s, SIOCDISPPTPTRUNKINGTABLE, finish!!\n", __FILE__);	
		break;

	}
#endif

#if CONFIG_NK_IPFILTER_SUPPORT_SORTING
#ifdef CONFIG_NK_IPFILTER_SUPPORT_SORTING
    // session upload sort ; outbound sort type=5
    case SIOCGETSESSIONUPSORT :
	{
		int i,max_sort;
		struct ip_conntrack_tuple_hash *h;
		struct ip_conntrack *ct;
		u_int64_t pre_max, tmp_max;
		ip_session_t *ip_session;
		traffic_t *tr, *nk_traffic_p=NULL;
		traffic_t *sort_tr[SORTING_ENTRY], *tmp_tr, *tmp_tr2;
		int k,tmp=0;

		if(fw_setting.tr_enable==0)
			break;

		//printk(KERN_EMERG "SIOCGETSESSIONUPSORT, sortType=5\n");
		max_sort = SORTING_ENTRY;
		ip_session = kmalloc(SORTING_ENTRY * sizeof(ip_session_t), GFP_ATOMIC);
	
		pre_max = 0xffffffffffffffff;
		bzero((char *)ip_session, SORTING_ENTRY * sizeof(ip_session_t));

		write_lock_bh(&ip_conntrack_lock);
		for (i = 0; i < ip_conntrack_htable_size; i++) 
		{
			list_for_each_entry(h, &ip_conntrack_hash[i], list) 
			{
				ct = tuplehash_to_ctrack(h);
				if( (ct) && (ct->tr.query==0) && 
				    (ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip!=
				     ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip) )
//				    ((ct->status & IPS_SEEN_REPLY)) )
				{
					tr = kmalloc(sizeof(struct traffic), GFP_ATOMIC);
					if(tr)
					{
						memset(tr, 0, sizeof(struct traffic));
						tr->next = nk_traffic_p;
						tr->ct = ct;
						nk_traffic_p = tr;
					}
					ct->tr.query=1;
				}
			}
		}
		write_unlock_bh(&ip_conntrack_lock);

		for(i=0; i<max_sort; i++)
		{
			tmp_max = 0;
			tmp_tr = 0;
			for (tr = nk_traffic_p; tr; tr = tr->next)
			{
				if(tr->ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum!=6 && tr->ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum!=17)
					continue;
		
				if(tr->ct->tr.up_bytes_delta>tmp_max && tr->ct->tr.up_bytes_delta<pre_max)
				{
					tmp_tr = tr;
					tmp_max = tr->ct->tr.up_bytes_delta;
				}
			}
			sort_tr[i] = tmp_tr;
			pre_max = tmp_max;
		}

		for(i=0; i<max_sort && sort_tr[i]; i++)
		{
			for(k=0;k<MAX_WAN_NUM;k++)
			{
#ifdef CONFIG_NK_TRANSPARENT_BRIDGE_RANGE_NUM
				//printk(KERN_EMERG "dip=%u.%u.%u.%u wan(%d)ip=%u.%u.%u.%u\n", NIPQUAD(sort_tr[i]->ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip), k+1, NIPQUAD(wan_interface_session[k].wan_ip));
				if(sort_tr[i]->ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip==wan_interface_session[k].wan_ip)
				{
					tmp=1;
					break;
				}

				for(l=0;l<CONFIG_NK_TRANSPARENT_BRIDGE_RANGE_NUM;l++)
				{
				   if((sort_tr[i]->ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip>=wan_interface_session[k].internallanip1[l])&&
				   (sort_tr[i]->ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip<=wan_interface_session[k].internallanip2[l]))
					{
						tmp=1;
						break;
					}
				}
#else
				//printk(KERN_EMERG "dip=%u.%u.%u.%u wan(%d)ip=%u.%u.%u.%u\n", NIPQUAD(sort_tr[i]->ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip), k+1, NIPQUAD(wan_interface_session[k].wan_ip));
				if((sort_tr[i]->ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip==wan_interface_session[k].wan_ip) ||
				   ((sort_tr[i]->ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip>=wan_interface_session[k].internallanip1)&&
				   (sort_tr[i]->ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip<=wan_interface_session[k].internallanip2)) ||
				   ((sort_tr[i]->ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip>=wan_interface_session[k].internallanip3)&&
				   (sort_tr[i]->ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip<=wan_interface_session[k].internallanip4)) )
				{
					tmp=1;
					break;
				}
#endif
			}
			if(tmp==1) //incoming session
			{
				ip_session[i].src_ip = sort_tr[i]->ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.ip;
				ip_session[i].dst_ip = sort_tr[i]->ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.ip;
				ip_session[i].proto = sort_tr[i]->ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum;
				ip_session[i].src_port = sort_tr[i]->ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.u.all;
				ip_session[i].dst_port = sort_tr[i]->ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u.all;
//				printk(KERN_EMERG "1.src=%u.%u.%u.%u:%u dst=%u.%u.%u.%u:%u\n\n", NIPQUAD(ip_session[i].src_ip), ntohs(ip_session[i].src_port), NIPQUAD(ip_session[i].dst_ip), ntohs(ip_session[i].dst_port));
			}
			else
			{
				ip_session[i].src_ip = sort_tr[i]->ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip;
				ip_session[i].dst_ip = sort_tr[i]->ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip;
				ip_session[i].proto = sort_tr[i]->ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum;
				ip_session[i].src_port = sort_tr[i]->ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u.all;
				ip_session[i].dst_port = sort_tr[i]->ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.all;
//				printk(KERN_EMERG "2.src=%u.%u.%u.%u dst=%u.%u.%u.%u\n\n", NIPQUAD(ip_session[i].src_ip), NIPQUAD(ip_session[i].dst_ip));
			}
			ip_session[i].up_bytes = sort_tr[i]->ct->tr.up_bytes_delta;
			tmp=0;

			ip_session[i].down_bytes = sort_tr[i]->ct->tr.up_bytes_delta;
			//printk(KERN_EMERG "%u.%u.%u.%u:%u->%u.%u.%u.%u:%u, %x, proto=%x\n", NIPQUAD(ip_session[i].src_ip), ip_session[i].src_port,
			//NIPQUAD(ip_session[i].dst_ip), ip_session[i].dst_port, ip_session[i].down_bytes, ip_session[i].proto);

		}
	    	copy_to_user((ip_session_t *)arg, &ip_session[0], SORTING_ENTRY * sizeof(ip_session_t));

	    	kfree(ip_session);
#if 0
		for (tr = nk_traffic_p; tr; tr = tr->next)
			kfree(tr);
#endif
		tmp_tr2 = nk_traffic_p;
		while(tmp_tr2)
		{
			nk_traffic_p = tmp_tr2->next;
			kfree(tmp_tr2);
			tmp_tr2 = nk_traffic_p;
		}

		write_lock_bh(&ip_conntrack_lock);
		for (i = 0; i < ip_conntrack_htable_size; i++) 
		{
			list_for_each_entry(h, &ip_conntrack_hash[i], list) 
			{
				if(h)
				{
					ct = tuplehash_to_ctrack(h);
					if(ct)
						ct->tr.query = 0;
				}
			}
		}
		write_unlock_bh(&ip_conntrack_lock);
            	break;
	}

    // session download sort, Sort type=4
    case SIOCGETSESSIONDOWNSORT :
    	{
		int i,max_sort, k;
		struct ip_conntrack_tuple_hash *h;
		struct ip_conntrack *ct;
		u_int64_t pre_max, tmp_max;
		ip_session_t *ip_session;
		traffic_t *tr, *nk_traffic_p=NULL;
		traffic_t *sort_tr[SORTING_ENTRY], *tmp_tr, *tmp_tr2;
		int tmp=0;

		if(fw_setting.tr_enable==0)
			break;

		//printk(KERN_EMERG "SIOCGETSESSIONDOWNSORT, sortType=4\n");
		max_sort = SORTING_ENTRY;
		ip_session = kmalloc(SORTING_ENTRY * sizeof(ip_session_t), GFP_ATOMIC);
	
		pre_max = 0xffffffffffffffff;
		bzero((char *)ip_session, SORTING_ENTRY * sizeof(ip_session_t));

		write_lock_bh(&ip_conntrack_lock);
		for (i = 0; i < ip_conntrack_htable_size; i++) 
		{
			list_for_each_entry(h, &ip_conntrack_hash[i], list) 
			{
				ct = tuplehash_to_ctrack(h);
				if( (ct) && (ct->tr.query==0) && 
				    (ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip!=
				     ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip) )
//				    ((ct->status & IPS_SEEN_REPLY)) )
				{
					tr = kmalloc(sizeof(struct traffic), GFP_ATOMIC);
					if(tr)
					{
						memset(tr, 0, sizeof(struct traffic));
						tr->next = nk_traffic_p;
						tr->ct = ct;
						nk_traffic_p = tr;
					}
					ct->tr.query=1;
				}
			}
		}
		write_unlock_bh(&ip_conntrack_lock);

		for(i=0; i<max_sort; i++)
		{
			tmp_max = 0;
			tmp_tr = 0;
			for (tr = nk_traffic_p; tr; tr = tr->next)
			{
				if(tr->ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum!=6 && tr->ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum!=17)
					continue;
				if(tr->ct->tr.down_bytes_delta>tmp_max && tr->ct->tr.down_bytes_delta<pre_max)
				{
					tmp_tr = tr;
					tmp_max = tr->ct->tr.down_bytes_delta;
				}
			}
			sort_tr[i] = tmp_tr;
			pre_max = tmp_max;
		}

		for(i=0; i<max_sort && sort_tr[i]; i++)
		{
#ifdef CONFIG_NK_TRANSPARENT_BRIDGE_RANGE_NUM
			for(k=0;k<MAX_WAN_NUM;k++)
			{
				//printk(KERN_EMERG "dip=%u.%u.%u.%u wan(%d)ip=%u.%u.%u.%u\n", NIPQUAD(sort_tr[i]->ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip), k+1, NIPQUAD(wan_interface_session[k].wan_ip));
				if(sort_tr[i]->ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip==wan_interface_session[k].wan_ip)
				{
					tmp=1;
					break;
				}

				for(l=0;l<CONFIG_NK_TRANSPARENT_BRIDGE_RANGE_NUM;l++)
				{
					if((sort_tr[i]->ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip>=wan_interface_session[k].internallanip1[l])&&
					(sort_tr[i]->ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip<=wan_interface_session[k].internallanip2[l]))
					{
						tmp=1;
						break;
					}
				}
			}
#else	
			//printk(KERN_EMERG "dip=%u.%u.%u.%u wanip=%u.%u.%u.%u\n", NIPQUAD(sort_tr[i]->ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip), NIPQUAD(wan_interface_session[2].wan_ip));
			for(k=0;k<MAX_WAN_NUM;k++)
			{
				//printk(KERN_EMERG "dip=%u.%u.%u.%u wan(%d)ip=%u.%u.%u.%u\n", NIPQUAD(sort_tr[i]->ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip), k+1, NIPQUAD(wan_interface_session[k].wan_ip));
				if((sort_tr[i]->ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip==wan_interface_session[k].wan_ip) ||
				   ((sort_tr[i]->ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip>=wan_interface_session[k].internallanip1)&&
				   (sort_tr[i]->ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip<=wan_interface_session[k].internallanip2)) ||
				   ((sort_tr[i]->ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip>=wan_interface_session[k].internallanip3)&&
				   (sort_tr[i]->ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip<=wan_interface_session[k].internallanip4)) )
				{
					tmp=1;
					break;
				}
			}
#endif
			if(tmp==1) //incoming session
			{
				ip_session[i].src_ip = sort_tr[i]->ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.ip;
				ip_session[i].dst_ip = sort_tr[i]->ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.ip;
				ip_session[i].proto = sort_tr[i]->ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum;
				ip_session[i].src_port = sort_tr[i]->ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.u.all;
				ip_session[i].dst_port = sort_tr[i]->ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u.all;
				ip_session[i].down_bytes = sort_tr[i]->ct->tr.down_bytes_delta;
//				printk(KERN_EMERG "1.src=%u.%u.%u.%u:%u dst=%u.%u.%u.%u:%u\n\n", NIPQUAD(ip_session[i].src_ip), ntohs(ip_session[i].src_port), NIPQUAD(ip_session[i].dst_ip), ntohs(ip_session[i].dst_port));
			}
			else
			{
				ip_session[i].src_ip = sort_tr[i]->ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip;
				ip_session[i].dst_ip = sort_tr[i]->ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip;
				ip_session[i].proto = sort_tr[i]->ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum;
				ip_session[i].src_port = sort_tr[i]->ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u.all;
				ip_session[i].dst_port = sort_tr[i]->ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.all;
				ip_session[i].down_bytes = sort_tr[i]->ct->tr.down_bytes_delta;
//				printk(KERN_EMERG "2.src=%u.%u.%u.%u dst=%u.%u.%u.%u\n\n", NIPQUAD(ip_session[i].src_ip), NIPQUAD(ip_session[i].dst_ip));
			}

			tmp=0;
		}
	    	copy_to_user((ip_session_t *)arg, &ip_session[0], SORTING_ENTRY * sizeof(ip_session_t));

	    	kfree(ip_session);
#if 0
		for (tr = nk_traffic_p; tr; tr = tr->next)
			kfree(tr);
#endif
		tmp_tr2 = nk_traffic_p;
		while(tmp_tr2)
		{
			nk_traffic_p = tmp_tr2->next;
			kfree(tmp_tr2);
			tmp_tr2 = nk_traffic_p;
		}
		write_lock_bh(&ip_conntrack_lock);
		for (i = 0; i < ip_conntrack_htable_size; i++) 
		{
			list_for_each_entry(h, &ip_conntrack_hash[i], list) 
			{
				ct = tuplehash_to_ctrack(h);
				ct->tr.query = 0;
			}
		}
		write_unlock_bh(&ip_conntrack_lock);
            	break;
	}


    // ip download sort; SortType == 0, inbound source address
    case SIOCGETIPDOWNSORT :
	{
		int i, max_sort;
		ip_sort_t *ip_list, *tmp_ip_sort;
		ip_sort_t *ip_sort, *tmp_s;
		u32 pre_max, tmp_max;
		smart_qos_ip_t *tmp_qos_ip;

		//if(fw_setting.tr_enable==0)
		//	break;
		max_sort = 0;
		ip_list = NULL;
		ip_sort = kmalloc(SORTING_ENTRY * sizeof(ip_sort_t), GFP_ATOMIC); //test

		if(nk_smart_qos_ip_list)
		{
			tmp_qos_ip = nk_smart_qos_ip_list;
			while(tmp_qos_ip)
			{
				//printk(KERN_EMERG "%u.%u.%u.%u\n",  NIPQUAD(tmp_qos_ip->ip.s_addr));
#if 0
				for(tmp_ip_sort = ip_list; tmp_ip_sort; tmp_ip_sort = tmp_ip_sort->next)
				{
					if(tmp_ip_sort->ip == tmp_qos_ip->ip.s_addr)
					{
						tmp_ip_sort->delta_byte_cnt+=tmp_qos_ip->down_bw;
						break;
					}
				}
#endif

				if(tmp_qos_ip->ip.s_addr>0)
				{
					tmp_ip_sort =(ip_sort_t *)kmalloc(sizeof(ip_sort_t), GFP_ATOMIC);
					if(!tmp_ip_sort)
						goto free_tmp_ip_sort;
					
					bzero((char *)tmp_ip_sort, sizeof(ip_sort_t));
					//tmp_ip_sort->delta_byte_cnt=nat->nat_up_bytes_delta;
					tmp_ip_sort->delta_byte_cnt = tmp_qos_ip->down_bw;
					tmp_ip_sort->ip = tmp_qos_ip->ip.s_addr;
					tmp_ip_sort->next = ip_list;
					ip_list = tmp_ip_sort;
					max_sort++;
					//printk(KERN_EMERG "type=0, Add: %u.%u.%u.%u\n",  NIPQUAD(tmp_ip_sort->ip));
				}
				tmp_qos_ip = tmp_qos_ip->next;
			}
		}
#if xxx
            /* build the ip list */
		write_lock_bh(&ip_conntrack_lock);
		for (i = 0; i < ip_conntrack_htable_size; i++) 
		{
			list_for_each_entry(h, &ip_conntrack_hash[i], list) 
			{
				
				ct = tuplehash_to_ctrack(h);
				
				if( (ct) && (ct->tr.query==0) )
				{
					if(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum!=6 && ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum!=17)
						continue;
					/* ignore traffic from lan pc to router*/
					if((ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip == lanip) || (ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.ip == lanip))
						continue;
					if(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip==ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip)
						continue;
//					if (!(ct->status & IPS_SEEN_REPLY))
//						continue;
/*
					if( (ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip!=wan_interface_session[0].wan_ip)&&
					(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip!=wan_interface_session[1].wan_ip)&&
					(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip!=wan_interface_session[0].wan_ip)&&
					(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip!=wan_interface_session[1].wan_ip)&&
					(ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.ip!=wan_interface_session[0].wan_ip)&&
					(ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.ip!=wan_interface_session[1].wan_ip)&&
					(ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.ip!=wan_interface_session[0].wan_ip)&&
					(ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.ip!=wan_interface_session[1].wan_ip) )
						continue;
*/	
					for(tmp_ip_sort = ip_list; tmp_ip_sort;
					tmp_ip_sort = tmp_ip_sort->next )
					{
/*						if(((ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.ip==wan_interface_session[0].wan_ip)||
							(ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.ip==wan_interface_session[1].wan_ip)) &&
						(tmp_ip_sort->ip == ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip))*/
						if(tmp_ip_sort->ip == ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip)
						{
							tmp_ip_sort->delta_byte_cnt+=ct->tr.down_bytes_delta;
							break;
						}
/*						else if(((ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip==wan_interface_session[0].wan_ip)||
							(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip==wan_interface_session[1].wan_ip)) &&
						(tmp_ip_sort->ip == ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.ip))*/
						else if(tmp_ip_sort->ip == ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.ip)

						{
							tmp_ip_sort->delta_byte_cnt+=ct->tr.down_bytes_delta;
							break;
						}
					}
	
					if(!tmp_ip_sort)
					{
						tmp_ip_sort =(ip_sort_t *)kmalloc(sizeof(ip_sort_t), GFP_ATOMIC);
						if(!tmp_ip_sort)
						goto free_tmp_ip_sort;
				
						bzero((char *)tmp_ip_sort, sizeof(ip_sort_t));
						tmp_ip_sort->delta_byte_cnt=ct->tr.down_bytes_delta;

#ifdef CONFIG_NK_TRANSPARENT_BRIDGE_RANGE_NUM
						for(k=0;k<MAX_WAN_NUM;k++)
						{
							if(ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.ip==wan_interface_session[k].wan_ip)
							{
								sdir=2; //session direction, 2=forwaring
								break;
							}
							for(l=0;l<CONFIG_NK_TRANSPARENT_BRIDGE_RANGE_NUM;l++)
							{
								if((ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.ip>=wan_interface_session[k].internallanip1[l])&&
								(ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.ip<=wan_interface_session[k].internallanip2[l]))
								{
									sdir=2; //session direction, 2=forwaring
									break;
								}
							}
						}
#else
						for(k=0;k<MAX_WAN_NUM;k++)
						{
							if( (ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.ip==wan_interface_session[k].wan_ip) ||
							((ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.ip>=wan_interface_session[k].internallanip1)&&
							(ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.ip<=wan_interface_session[k].internallanip2)) ||
							((ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.ip>=wan_interface_session[k].internallanip3)&&
							(ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.ip<=wan_interface_session[k].internallanip4)) )
							{
								sdir=2; //session direction, 2=forwaring
								break;
							}
	
						}
#endif
/*						if((ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.ip==wan_interface_session[0].wan_ip)||
						(ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.ip==wan_interface_session[1].wan_ip)) //forwarding*/
						if(sdir==2)
							tmp_ip_sort->ip = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip;
						else	tmp_ip_sort->ip = ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.ip;
/*						
						else if((ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip==wan_interface_session[0].wan_ip)||
						(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip==wan_interface_session[1].wan_ip))
							tmp_ip_sort->ip = ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.ip;
*/
						tmp_ip_sort->next = ip_list;
						ip_list = tmp_ip_sort;
						max_sort++;
					}
					ct->tr.query = 1;
				}
			}
	    	}
		write_unlock_bh(&ip_conntrack_lock);
#endif
		max_sort =SORTING_ENTRY;
		pre_max = 0xffffffff;
		bzero((char *)ip_sort, SORTING_ENTRY * sizeof(ip_sort_t));
	
		/* sorting ip list to ip_sort */
		for(i=0; i<max_sort; i++)
		{
			tmp_max = 0;
			tmp_ip_sort = NULL;
			for (tmp_s = ip_list; tmp_s; tmp_s = tmp_s->next)
			{
				if(tmp_s->delta_byte_cnt>tmp_max && tmp_s->delta_byte_cnt<pre_max)
				{
					tmp_ip_sort = tmp_s;
					tmp_max = tmp_s->delta_byte_cnt;
				}
			}
			if(!tmp_ip_sort)
			continue;
	
			ip_sort[i] = *tmp_ip_sort;
			pre_max = tmp_max;
		}
//debug
#if 0
for(i=0; i<max_sort; i++)
{
	if(ip_sort[i].ip>0)
		printk(KERN_EMERG "%u.%u.%u.%u dl=%d\n", NIPQUAD(ip_sort[i].ip), ntohs(ip_sort[i].delta_byte_cnt));
	else break;
}
#endif
		copy_to_user((ip_sort_t *)arg, &ip_sort[0], SORTING_ENTRY * sizeof(ip_sort_t));
	
		kfree(ip_sort);

#if 0
		write_lock_bh(&ip_conntrack_lock);
		for (i = 0; i < ip_conntrack_htable_size; i++) 
		{
			list_for_each_entry(h, &ip_conntrack_hash[i], list) 
			{
				if(h)
				{
					ct = tuplehash_to_ctrack(h);
					if(ct)
						ct->tr.query = 0;
				}
			}
		}
		write_unlock_bh(&ip_conntrack_lock);
#endif
free_tmp_ip_sort:
            /* free ip list */
            for (tmp_s = ip_list; tmp_s; tmp_s = tmp_ip_sort)
            {
                tmp_ip_sort = tmp_s->next;
                kfree(tmp_s);
            }

            break;
	}
    // ip upload sort; SortType == 1
    case SIOCGETIPUPSORT :
	{
		int i, max_sort;
		ip_sort_t *ip_list, *tmp_ip_sort;
		ip_sort_t *ip_sort, *tmp_s;
		u32 pre_max, tmp_max;
		smart_qos_ip_t *tmp_qos_ip;

		//if(fw_setting.tr_enable==0)
		//	break;
		
		//printk(KERN_EMERG "SIOCGETIPUPSORT, sortType=1\n");
		max_sort = 0;
		ip_list = NULL;
		ip_sort = kmalloc(SORTING_ENTRY * sizeof(ip_sort_t), GFP_ATOMIC); //test

		if(nk_smart_qos_ip_list!=NULL)
		{
			tmp_qos_ip = nk_smart_qos_ip_list;
			while(tmp_qos_ip)
			{
				if(tmp_qos_ip->ip.s_addr>0)
				{
					tmp_ip_sort =(ip_sort_t *)kmalloc(sizeof(ip_sort_t), GFP_ATOMIC);
					if(!tmp_ip_sort)
						goto free_tmp_ip_sort2;
					
					bzero((char *)tmp_ip_sort, sizeof(ip_sort_t));
					//tmp_ip_sort->delta_byte_cnt=nat->nat_up_bytes_delta;
					tmp_ip_sort->delta_byte_cnt = tmp_qos_ip->up_bw;
					tmp_ip_sort->ip = tmp_qos_ip->ip.s_addr;
					tmp_ip_sort->next = ip_list;
					ip_list = tmp_ip_sort;
					max_sort++;
					//printk(KERN_EMERG "type=1, Add: %u.%u.%u.%u %x \n",  NIPQUAD(tmp_ip_sort->ip), tmp_ip_sort->delta_byte_cnt);
				}
				tmp_qos_ip = tmp_qos_ip->next;
			}
		}
		//max_sort = ips_stats.iss_active<SORTING_ENTRY?ips_stats.iss_active:SORTING_ENTRY;
		//if(max_sort>SORTING_ENTRY)
		max_sort =SORTING_ENTRY;
		pre_max = 0xffffffff;
		bzero((char *)ip_sort, SORTING_ENTRY * sizeof(ip_sort_t));
	
		/* sorting ip list to ip_sort */
		for(i=0; i<max_sort; i++)
		{
			tmp_max = 0;
			tmp_ip_sort = NULL;
			for (tmp_s = ip_list; tmp_s; tmp_s = tmp_s->next)
			{
				if(tmp_s->delta_byte_cnt>tmp_max && tmp_s->delta_byte_cnt<pre_max)
				{
					tmp_ip_sort = tmp_s;
					tmp_max = tmp_s->delta_byte_cnt;
				}
			}
			if(!tmp_ip_sort)
			continue;
	
			ip_sort[i] = *tmp_ip_sort;
			pre_max = tmp_max;
		}

//debug
#if 0
for(i=0; i<max_sort; i++)
{
	if(ip_sort[i].ip>0)
		printk(KERN_EMERG "%u.%u.%u.%u dl=%d\n", NIPQUAD(ip_sort[i].ip), ntohs(ip_sort[i].delta_byte_cnt));
	else break;
}
#endif
		copy_to_user((ip_sort_t *)arg, &ip_sort[0], SORTING_ENTRY * sizeof(ip_sort_t));
		
		kfree(ip_sort);

free_tmp_ip_sort2:
		/* free ip list */
		for (tmp_s = ip_list; tmp_s; tmp_s = tmp_ip_sort)
		{
			tmp_ip_sort = tmp_s->next;
			kfree(tmp_s);
		}

            break;
	}
    // service download sort; SortType==2
    case SIOCGETSERVDOWNSORT :
	{
		int i, max_sort,k,sdir=0;
		service_sort_t *service_list, *tmp_service_sort;
		//service_sort_t service_sort[SORTING_ENTRY], *tmp_s;
		service_sort_t *service_sort, *tmp_s;
		u32 pre_max, tmp_max;
		struct ip_conntrack_tuple_hash *h;
		struct ip_conntrack *ct;

		//if(fw_setting.tr_enable==0)
		//	break;
	
		//printk(KERN_EMERG "SIOCGETSERVDOWNSORT, sortType=2\n");

		max_sort = 0;
		service_list = NULL;
		service_sort = kmalloc(SORTING_ENTRY * sizeof(service_sort_t), GFP_ATOMIC); //test

		/* build the service list */
		write_lock_bh(&ip_conntrack_lock);
		for (i = 0; i < ip_conntrack_htable_size; i++) 
		{
			list_for_each_entry(h, &ip_conntrack_hash[i], list) 
			{
				if(!h) continue;
				ct = tuplehash_to_ctrack(h);
				if(!ct) continue;
				if(ct->tr.query!=0)
					continue;
				if(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum!=6 && ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum!=17)
				continue;
				if((ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip == wan_interface_session[0].lanip) || (ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.ip == wan_interface_session[0].lanip))
					continue;
//				if (!(ct->status & IPS_SEEN_REPLY))
//					continue;
/*				if( (ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip!=wan_interface_session[0].wan_ip)&&
				(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip!=wan_interface_session[1].wan_ip)&&
				(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip!=wan_interface_session[0].wan_ip)&&
				(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip!=wan_interface_session[1].wan_ip)&&
				(ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.ip!=wan_interface_session[0].wan_ip)&&
				(ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.ip!=wan_interface_session[1].wan_ip)&&
				(ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.ip!=wan_interface_session[0].wan_ip)&&
				(ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.ip!=wan_interface_session[1].wan_ip) )
					continue;
*/
				for(tmp_service_sort = service_list; tmp_service_sort;
				tmp_service_sort = tmp_service_sort->next )
				{
					if(tmp_service_sort->port_num == ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.all
						&& tmp_service_sort->proto == ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum)
					{
						tmp_service_sort->delta_byte_cnt+=ct->tr.down_bytes_delta;
						break;
					}
				}

				if(!tmp_service_sort)
				{
					tmp_service_sort =(service_sort_t *)kmalloc(sizeof(service_sort_t), GFP_ATOMIC);
					if(!tmp_service_sort)
						goto free_tmp_service_sort;
			
					bzero((char *)tmp_service_sort, sizeof(service_sort_t));
					tmp_service_sort->delta_byte_cnt=ct->tr.down_bytes_delta;
#if CONFIG_NK_TRANSPARENT_BRIDGE_RANGE_NUM
					for(k=0;k<MAX_WAN_NUM;k++)
					{
						if(ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.ip==wan_interface_session[k].wan_ip)
						{
							sdir=2; //session direction, 2=forwaring
							break;
						}

						for(l=0;l<CONFIG_NK_TRANSPARENT_BRIDGE_RANGE_NUM;l++)
						{
							if((ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.ip>=wan_interface_session[k].internallanip1)&&
							(ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.ip<=wan_interface_session[k].internallanip2))
							{
								sdir=2; //session direction, 2=forwaring
								break;
							}
						}
					}
#else
					for(k=0;k<MAX_WAN_NUM;k++)
					{
						if( (ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.ip==wan_interface_session[k].wan_ip) ||
						((ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.ip>=wan_interface_session[k].internallanip1)&&
						(ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.ip<=wan_interface_session[k].internallanip2)) ||
						((ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.ip>=wan_interface_session[k].internallanip3)&&
						(ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.ip<=wan_interface_session[k].internallanip4)) )
						{
							sdir=2; //session direction, 2=forwaring
							break;
						}
						else sdir=0;	
					}
#endif			
/*					if( (ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip==wan_interface_session[0].wan_ip) ||
						(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip==wan_interface_session[1].wan_ip) )*/
					if(sdir==2)
						tmp_service_sort->port_num = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u.all;
					else if(sdir==0)
						tmp_service_sort->port_num = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.all;
					tmp_service_sort->proto = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum;
					tmp_service_sort->next = service_list;
					service_list = tmp_service_sort;
					max_sort++;
				}
				ct->tr.query = 1;
			}	
		}	
		write_unlock_bh(&ip_conntrack_lock);

		max_sort = SORTING_ENTRY;
		pre_max = 0xffffffff;
		bzero((char *)service_sort, SORTING_ENTRY * sizeof(service_sort_t));

		/* sorting service list to service_sort */
		for(i=0; i<max_sort; i++)
		{
			tmp_max = 0;
			tmp_service_sort = NULL;
			for (tmp_s = service_list; tmp_s; tmp_s = tmp_s->next)
			{
				if(tmp_s->delta_byte_cnt>tmp_max && tmp_s->delta_byte_cnt<pre_max)
				{
					tmp_service_sort = tmp_s;
					tmp_max = tmp_s->delta_byte_cnt;
				}
			}
			if(!tmp_service_sort)
				continue;
			service_sort[i].delta_byte_cnt = tmp_service_sort->delta_byte_cnt;
			service_sort[i].port_num = tmp_service_sort->port_num;
			service_sort[i].proto = tmp_service_sort->proto;
			pre_max = tmp_max;
		}

		copy_to_user((service_sort_t *)arg, &service_sort[0], SORTING_ENTRY * sizeof(service_sort_t));
	
		kfree(service_sort);

		write_lock_bh(&ip_conntrack_lock);
		for (i = 0; i < ip_conntrack_htable_size; i++) 
		{
			list_for_each_entry(h, &ip_conntrack_hash[i], list) 
			{
				if(h)
				{
					ct = tuplehash_to_ctrack(h);
					if(ct)	
						ct->tr.query = 0;
				}
			}
		}
		write_unlock_bh(&ip_conntrack_lock);

free_tmp_service_sort:
		/* free service list */
		for (tmp_s = service_list; tmp_s; tmp_s = tmp_service_sort)
		{
			tmp_service_sort = tmp_s->next;
			kfree(tmp_s);
		}

          	break;
        }

    // service upload sort; SortType==3
    case SIOCGETSERVUPSORT :
	{
		int i, max_sort;
		service_sort_t *service_list, *tmp_service_sort;
		//service_sort_t service_sort[SORTING_ENTRY], *tmp_s;
		service_sort_t *service_sort, *tmp_s;
		u32 pre_max, tmp_max;
		struct ip_conntrack_tuple_hash *h;
		struct ip_conntrack *ct;

		//if(fw_setting.tr_enable==0)
		//	break;

		//printk(KERN_EMERG "SIOCGETSERVUPSORT, sortType=3\n");

		service_list = NULL;
		service_sort = kmalloc(SORTING_ENTRY * sizeof(service_sort_t), GFP_ATOMIC); //test
		max_sort = 0;

            /* build the service list */
		write_lock_bh(&ip_conntrack_lock);
		for (i = 0; i < ip_conntrack_htable_size; i++) 
		{
			list_for_each_entry(h, &ip_conntrack_hash[i], list) 
			{
				if(!h) continue;
				ct = tuplehash_to_ctrack(h);
				if(!ct) continue;
				if(ct->tr.query!=0)
					continue;
				if(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum!=6 && ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum!=17)
					continue;
				if((ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip == wan_interface_session[0].lanip) || (ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.ip == wan_interface_session[0].lanip))
					continue;
//				if (!(ct->status & IPS_SEEN_REPLY))
//					continue;
/*
				if( (ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip!=wan_interface_session[0].wan_ip)&&
				(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip!=wan_interface_session[1].wan_ip)&&
				(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip!=wan_interface_session[0].wan_ip)&&
				(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip!=wan_interface_session[1].wan_ip)&&
				(ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.ip!=wan_interface_session[0].wan_ip)&&
				(ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.ip!=wan_interface_session[1].wan_ip)&&
				(ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.ip!=wan_interface_session[0].wan_ip)&&
				(ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.ip!=wan_interface_session[1].wan_ip) )
					continue;
*/
				for(tmp_service_sort = service_list; tmp_service_sort;
				tmp_service_sort = tmp_service_sort->next )
				{
					if(((tmp_service_sort->port_num) == ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.all)
						&& ((tmp_service_sort->proto) == (ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum)))
					{
						tmp_service_sort->delta_byte_cnt=tmp_service_sort->delta_byte_cnt+ct->tr.up_bytes_delta;
						break;
					}
				}
	
				if(!tmp_service_sort)
				{
					tmp_service_sort =(service_sort_t *)kmalloc(sizeof(service_sort_t), GFP_ATOMIC);
					if(!tmp_service_sort)
						goto free_tmp_service_sort2;
			
					bzero((char *)tmp_service_sort, sizeof(service_sort_t));
					tmp_service_sort->delta_byte_cnt=ct->tr.up_bytes_delta;
					//if(nat->nat_dir==FR_INBOUND)
					//	tmp_service_sort->port_num = ntohs(nat->nat_inport);
					//if(nat->nat_dir==FR_OUTBOUND)
						tmp_service_sort->port_num = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.all;
					tmp_service_sort->proto = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum;
					tmp_service_sort->next = service_list;
					service_list = tmp_service_sort;
					max_sort++;
				}
				ct->tr.query = 1;
			}
		}
		write_unlock_bh(&ip_conntrack_lock);

            //max_sort = ips_stats.iss_active<SORTING_ENTRY?ips_stats.iss_active:SORTING_ENTRY;
            //if(max_sort>SORTING_ENTRY)
		max_sort =SORTING_ENTRY;
            	pre_max = 0xffffffff;
            	bzero((char *)service_sort, SORTING_ENTRY * sizeof(service_sort_t));

            /* sorting service list to service_sort */
            for(i=0; i<max_sort; i++)
            {
                tmp_max = 0;
                tmp_service_sort = NULL;
                for (tmp_s = service_list; tmp_s; tmp_s = tmp_s->next)
                {
                    if(tmp_s->delta_byte_cnt>tmp_max && tmp_s->delta_byte_cnt<pre_max)
                    {
                        tmp_service_sort = tmp_s;
                        tmp_max = tmp_s->delta_byte_cnt;
                    }
                }
                if(!tmp_service_sort)
                    continue;
                service_sort[i].delta_byte_cnt = tmp_service_sort->delta_byte_cnt;
                service_sort[i].port_num = tmp_service_sort->port_num;
                service_sort[i].proto = tmp_service_sort->proto;
                pre_max = tmp_max;
            }

		copy_to_user((service_sort_t *)arg, &service_sort[0], SORTING_ENTRY * sizeof(service_sort_t));
		
		kfree(service_sort);

		write_lock_bh(&ip_conntrack_lock);
		for (i = 0; i < ip_conntrack_htable_size; i++) 
		{
			list_for_each_entry(h, &ip_conntrack_hash[i], list) 
			{
				if(h)
				{
					ct = tuplehash_to_ctrack(h);
					if(ct)
						ct->tr.query = 0;
				}
			}
		}

free_tmp_service_sort2:
		write_unlock_bh(&ip_conntrack_lock);
            
	    /* free service list */
            for (tmp_s = service_list; tmp_s; tmp_s = tmp_service_sort)
            {
                tmp_service_sort = tmp_s->next;
                kfree(tmp_s);
            }
            break;
        }
    case SIOCCLRIPSESSION :	//Port from RV4
        {
            ip_session_t *ip_session;

            while(ip_list)
            {
                ip_session = ip_list;
                ip_list = ip_list->next;
                kfree(ip_session);
            }
	    break;
        }
    case SIOCGETIPSESSION :
    	{
		int max_sort, i;
		ip_session_t ip_session[100];
		static int cnt;
		u_int32_t ip, port;
		struct ip_conntrack_tuple_hash *h;
		struct ip_conntrack *ct;

		//if(fw_setting.tr_enable==0)
		//	break;

		copy_from_user(&ip_session, (struct ip_session *)arg, sizeof(ip_session_t));
		ip = ip_session[0].src_ip;
		port = ip_session[0].dst_port;
		max_sort = 100;
		bzero((char *)ip_session, max_sort * sizeof(ip_session_t));

		
		if(ip)
		{
			cnt=0;
			for (i = 0; i < ip_conntrack_htable_size; i++) 
			{
				if(cnt>=max_sort) break;
				write_lock_bh(&ip_conntrack_lock);
				list_for_each_entry(h, &ip_conntrack_hash[i], list) 
				{
					if(cnt>=max_sort) break;
					ct = tuplehash_to_ctrack(h);
					if((ct)&&(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum==6 || ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum==17)&&
					   (ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip != wan_interface_session[0].lanip)&&
					    (ct->tr.query==0) && (cnt<max_sort))
					{
						//Outbound sessions
						if(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip == ip)
						{
							ip_session[cnt].src_ip = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip;
							ip_session[cnt].dst_ip = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip;
							ip_session[cnt].src_port = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u.all;
							ip_session[cnt].dst_port = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.all;
							ip_session[cnt].proto = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum;
							//strcpy(ip_session[cnt].wan_if, (struct net_device *)(tr->ct->dev)->name);
							ip_session[cnt].up_bytes = ct->tr.up_bytes_delta;
							ip_session[cnt].down_bytes = ct->tr.down_bytes_delta;
							ip_session[cnt].wan_ip = ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.ip;
							cnt++;
						}
						//Inbound sessions
						else if(ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.ip == ip)
						{
							ip_session[cnt].src_ip = ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.ip;
							ip_session[cnt].dst_ip = ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.ip;
							ip_session[cnt].src_port = ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.u.all;
							ip_session[cnt].dst_port = ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u.all;
							ip_session[cnt].proto = ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.protonum;
							ip_session[cnt].up_bytes = ct->tr.up_bytes_delta;
							ip_session[cnt].down_bytes = ct->tr.down_bytes_delta;
							ip_session[cnt].wan_ip = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip;
							cnt++;
						}
					}
					ct->tr.query = 1;
				}
				write_unlock_bh(&ip_conntrack_lock);
			}
		}
		else if(port > 0 && port < 65536)
		{
			cnt=0;
//			printk(KERN_EMERG "port=%d, \n", port);
			for (i = 0; i < ip_conntrack_htable_size; i++) 
			{
				if(cnt>=max_sort) break;
				write_lock_bh(&ip_conntrack_lock);
				list_for_each_entry(h, &ip_conntrack_hash[i], list) 
				{
					if(cnt>=max_sort) break;
					ct = tuplehash_to_ctrack(h);
					if( (cnt<max_sort) && (ct)&&(ct->tr.query==0)&&(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum==6 || ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum==17)&&
					   ((ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip != wan_interface_session[0].lanip)))
					{
						if((ntohs(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u.all) == port) ||
						   (ntohs(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.all) == port)||
						   (ntohs(ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.u.all) == port) ||
						   (ntohs(ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u.all) == port))
						{
							ip_session[cnt].src_ip = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip;
							ip_session[cnt].dst_ip = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip;
							ip_session[cnt].src_port = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u.all;
							ip_session[cnt].dst_port = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.all;
							ip_session[cnt].proto = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum;
							//strcpy(ip_session[cnt].wan_if, (struct net_device *)(tr->ct->dev)->name);
							ip_session[cnt].up_bytes = ct->tr.up_bytes_delta;
							ip_session[cnt].down_bytes = ct->tr.down_bytes_delta;
							ip_session[cnt].wan_ip = ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.ip;
							cnt++;
						}
						ct->tr.query = 1;
					}
				}
				write_unlock_bh(&ip_conntrack_lock);
			}
		}

		quick_sort(0, cnt-1, ip_session);

		copy_to_user((ip_session_t *)arg, &ip_session[0], max_sort * sizeof(ip_session_t));

		write_lock_bh(&ip_conntrack_lock);
		for (i = 0; i < ip_conntrack_htable_size; i++) 
		{
			list_for_each_entry(h, &ip_conntrack_hash[i], list) 
			{
				if(h)
				{
					ct = tuplehash_to_ctrack(h);
					if(ct)
						ct->tr.query = 0;
				}
			}
		}
		write_unlock_bh(&ip_conntrack_lock);

//debug
#if 0
printk(KERN_EMERG "cnt=%d\n", cnt);
for(i=0;i<cnt;i++)
{
		printk(KERN_EMERG "%u.%u.%u.%u:%u->%u.%u.%u.%u:%u, down:%x, up:%x, proto=%x\n", NIPQUAD(ip_session[i].src_ip), ip_session[i].src_port,
			NIPQUAD(ip_session[i].dst_ip), ip_session[i].dst_port, ip_session[i].down_bytes, ip_session[i].up_bytes, ip_session[i].proto);

}
#endif
		break;
	}

#endif

#endif	
    // get wan interface info from firewall
    case SIOCSENDWANNAME :
	{
		int j;
		printk(KERN_EMERG "SIOCSENDWANNAME=================================================\n");
		//copy_from_user(&ip_sort, (struct ip_service_sort *)arg, sizeof(struct ip_sort));
		copy_from_user(&wan_interface_session[0], (struct wan_interface_session *)arg, MAX_WAN_NUM * sizeof(interface_session_t));
#if 0
		//printk(KERN_EMERG "MAX_WAN_NUM=%d\n", MAX_WAN_NUM);
		for(i=0; i<MAX_WAN_NUM; i++)
		{

			printk(KERN_EMERG "name[%d][%s], ip[%u.%u.%u.%u]\n", i, wan_interface_session[i].name, NIPQUAD(wan_interface_session[i].wan_ip));
			printk(KERN_EMERG "WAN%d, ip[%u.%u.%u.%u], \n", i+1, NIPQUAD(wan_interface_session[i].internallanip1[1]));
			printk(KERN_EMERG "WAN%d, ip[%u.%u.%u.%u], \n", i+1, NIPQUAD(wan_interface_session[i].internallanip2[1]));
			printk(KERN_EMERG "WAN%d, ip[%u.%u.%u.%u], \n", i+1, NIPQUAD(wan_interface_session[i].internallanip1[2]));	
			printk(KERN_EMERG "WAN%d, ip[%u.%u.%u.%u], \n", i+1, NIPQUAD(wan_interface_session[i].internallanip2[2]));
		}
#endif
		lanip=wan_interface_session[0].lanip;
		printk(KERN_EMERG "[%s] lanip=%u.%u.%u.%u, mask=%u.%u.%u.%u\n", wan_interface_session[0].laninterface, NIPQUAD(wan_interface_session[0].lanip),
		NIPQUAD(wan_interface_session[0].lanmask));
#if 0
		for(i=0;i<5;i++)
		{
			printk(KERN_EMERG "ip[%u.%u.%u.%u] mask[%u.%u.%u.%u]\n", NIPQUAD(wan_interface_session[0].msubnet[i]), NIPQUAD(wan_interface_session[0].msubmask[i]));
		}
#endif

#if 1
		for(j=0; j<MAX_WAN_NUM; j++)
		{
			printk(KERN_EMERG "ip[%u.%u.%u.%u] mask[%u.%u.%u.%u]\n", NIPQUAD(wan_interface_session[j].wan_ip), NIPQUAD(wan_interface_session[j].wan_mask));
#if 0
			if(wan_interface_session[j].wan_ip==0) continue;

			for(k=0;k<3;k++)
			{
				printk(KERN_EMERG "rqw=%u.%u.%u.%u, rip1=%u.%u.%u.%u, rip2=%u.%u.%u.%u\n", NIPQUAD(wan_interface_session[j].routerinfo[k].rgw), NIPQUAD(wan_interface_session[j].routerinfo[k].rip1), NIPQUAD(wan_interface_session[j].routerinfo[k].rip2));
			}
#endif
		}
#endif
		break;
        }
#if 1
    // get fw settings from webBoot
    case SIOCSENDFWSET :
	{
		copy_from_user(&fw_setting, (struct firewall_setting *)arg, sizeof(struct firewall_setting));
		printk(KERN_EMERG "tr_enable=%d, smartqos=%d, period=%d\n", fw_setting.tr_enable, fw_setting.smart_qos, fw_setting.cal_cnt_period);
		#if CONFIG_NK_IPFILTER_SUPPORT_SORTING
		update_delta_timer_set();
		#endif
		nk_pingofdeath = fw_setting.pingofdeath;
		nk_denypolicy = fw_setting.denypolicy;
		hashtmp_size = 0;
		break;
        }
#endif
	//Rain add for Anti-Virus -->
#ifdef CONFIG_NK_ANTI_VIRUS
	case SIOCSENDANTIVIRUSSET :
	{
		copy_from_user(&av_setting, (struct anti_virus_setting *)arg, sizeof(struct anti_virus_setting));
		break;
        }
#endif
        // <-- Rain
        
#ifdef CONFIG_SUPPORT_IP_MAC_BINDING
    case SIOCGETIPMACLEARN :
	{
		int idx=0;
		ip_mac_bind_entry_t *ip_mac_learn_list, *nk_ip_mac_p=NULL, get_ip_mac_learn;
		ip_mac_bind_entry_t *ip_mac_bind_entry_s;
		u_int8_t *bind_mac;
		int temp=0, i, l;
		
		//printk(KERN_EMERG "SIOCGETIPMACLEARN\n");

		//copy_to_user((ip_mac_bind_entry_t *)arg, &nk_ip_mac_learn_list[0], sizeof(ip_mac_bind_entry_t *)*IP_MAC_ENTRY_HASH_SIZE);

		//tmp_entry = nk_ip_mac_learn_list[0];
		for(idx=0; idx<IP_MAC_ENTRY_HASH_SIZE; idx++)
		{
			ip_mac_bind_entry_s = nk_ip_mac_learn_list[idx];
			for(;ip_mac_bind_entry_s;ip_mac_bind_entry_s = ip_mac_bind_entry_s->next)
			{
				bind_mac = &(ip_mac_bind_entry_s->mac[0]);
				//printk(KERN_EMERG "getlist:mac[%x-%x-%x-%x-%x-%x], ip = %u.%u.%u.%u\n",bind_mac[0], bind_mac[1], bind_mac[2], bind_mac[3], bind_mac[4], bind_mac[5],
				//NIPQUAD(ip_mac_bind_entry->ip));
				/*20071025, check ip */
#ifdef CONFIG_NK_TRANSPARENT_BRIDGE_RANGE_NUM
				if((ip_mac_bind_entry_s->ip&wan_interface_session[0].lanmask)==(wan_interface_session[0].lanip&wan_interface_session[0].lanmask))
					temp=1;
				for(i=0; i<MAX_WAN_NUM; i++)
				{
					for(l=0;l<CONFIG_NK_TRANSPARENT_BRIDGE_RANGE_NUM;l++)
					{
						if((ip_mac_bind_entry_s->ip>=wan_interface_session[i].internallanip1[l]) || (ip_mac_bind_entry_s->ip<=wan_interface_session[i].internallanip2[l]))//transparent bridge
						temp=1;
					}
				}
				if(((ip_mac_bind_entry_s->ip&wan_interface_session[0].msubmask[0])==(wan_interface_session[0].msubnet[0]&wan_interface_session[0].msubmask[0])) &&
					((ip_mac_bind_entry_s->ip&wan_interface_session[0].msubmask[1])==(wan_interface_session[0].msubnet[1]&wan_interface_session[0].msubmask[1])) &&
					((ip_mac_bind_entry_s->ip&wan_interface_session[0].msubmask[2])==(wan_interface_session[0].msubnet[2]&wan_interface_session[0].msubmask[2])) &&
					((ip_mac_bind_entry_s->ip&wan_interface_session[0].msubmask[3])==(wan_interface_session[0].msubnet[3]&wan_interface_session[0].msubmask[3])) &&
					((ip_mac_bind_entry_s->ip&wan_interface_session[0].msubmask[4])==(wan_interface_session[0].msubnet[4]&wan_interface_session[0].msubmask[4])))
						temp=1;
#else
				if( 
					((ip_mac_bind_entry_s->ip&wan_interface_session[0].lanmask)==(wan_interface_session[0].lanip&wan_interface_session[0].lanmask)) || //lan subnet
					(((ip_mac_bind_entry_s->ip>=wan_interface_session[0].internallanip1) || (ip_mac_bind_entry_s->ip<=wan_interface_session[0].internallanip2)) ||//transparent bridge
					((ip_mac_bind_entry_s->ip>=wan_interface_session[0].internallanip3) || (ip_mac_bind_entry_s->ip<=wan_interface_session[0].internallanip4)) ||//transparent bridge
					((ip_mac_bind_entry_s->ip>=wan_interface_session[1].internallanip1) || (ip_mac_bind_entry_s->ip<=wan_interface_session[1].internallanip2)) ||//transparent bridge
					((ip_mac_bind_entry_s->ip>=wan_interface_session[1].internallanip3) || (ip_mac_bind_entry_s->ip<=wan_interface_session[1].internallanip4)) ||//transparent bridge
					((ip_mac_bind_entry_s->ip>=wan_interface_session[2].internallanip1) || (ip_mac_bind_entry_s->ip<=wan_interface_session[2].internallanip2)) ||//transparent bridge
					((ip_mac_bind_entry_s->ip>=wan_interface_session[2].internallanip3) || (ip_mac_bind_entry_s->ip<=wan_interface_session[2].internallanip4)) ||//transparent bridge
					((ip_mac_bind_entry_s->ip>=wan_interface_session[3].internallanip1) || (ip_mac_bind_entry_s->ip<=wan_interface_session[3].internallanip2)) ||//transparent bridge
					((ip_mac_bind_entry_s->ip>=wan_interface_session[3].internallanip3) || (ip_mac_bind_entry_s->ip<=wan_interface_session[3].internallanip4)) ||//transparent bridge
					((ip_mac_bind_entry_s->ip>=wan_interface_session[4].internallanip1) || (ip_mac_bind_entry_s->ip<=wan_interface_session[4].internallanip2)) ||//transparent bridge
					((ip_mac_bind_entry_s->ip>=wan_interface_session[4].internallanip3) || (ip_mac_bind_entry_s->ip<=wan_interface_session[4].internallanip4))) ||//transparent bridge
					(((ip_mac_bind_entry_s->ip&wan_interface_session[0].msubmask[0])==(wan_interface_session[0].msubnet[0]&wan_interface_session[0].msubmask[0])) &&
					((ip_mac_bind_entry_s->ip&wan_interface_session[0].msubmask[1])==(wan_interface_session[0].msubnet[1]&wan_interface_session[0].msubmask[1])) &&
					((ip_mac_bind_entry_s->ip&wan_interface_session[0].msubmask[2])==(wan_interface_session[0].msubnet[2]&wan_interface_session[0].msubmask[2])) &&
					((ip_mac_bind_entry_s->ip&wan_interface_session[0].msubmask[3])==(wan_interface_session[0].msubnet[3]&wan_interface_session[0].msubmask[3])) &&
					((ip_mac_bind_entry_s->ip&wan_interface_session[0].msubmask[4])==(wan_interface_session[0].msubnet[4]&wan_interface_session[0].msubmask[4]))) )
					{
						temp=1;
					}
#endif
				if(temp==1)
				{
					ip_mac_learn_list = (ip_mac_bind_entry_t *)kmalloc(sizeof(ip_mac_bind_entry_t), GFP_ATOMIC);
					if(ip_mac_learn_list)
					{
						bzero((char *)ip_mac_learn_list, sizeof(ip_mac_bind_entry_t));
						ip_mac_learn_list->next = nk_ip_mac_p;
						ip_mac_learn_list->ip = ip_mac_bind_entry_s->ip;
						memcpy(&ip_mac_learn_list->mac[0], &(ip_mac_bind_entry_s->mac[0]), 6);
						//printk(KERN_EMERG "!!! %d)ip=%u.%u.%u.%u\n", cnt+1, NIPQUAD(ip_mac_learn_list->ip));
						nk_ip_mac_p = ip_mac_learn_list;
					}
				}
			}
		}
		printk(KERN_EMERG "\n");
		get_ip_mac_learn.next = nk_ip_mac_p;
#if 0
		while(get_ip_mac_learn.next)
		{
			tmp = get_ip_mac_learn.next;
			printk(KERN_EMERG "222 ip=%u.%u.%u.%u\n\n", NIPQUAD(tmp->ip));
			get_ip_mac_learn.next = tmp->next;
		}
		get_ip_mac_learn.next = nk_ip_mac_p;
#endif
		copy_to_user((ip_mac_bind_entry_t *)arg, &get_ip_mac_learn, sizeof(ip_mac_bind_entry_t));
		break;
	}
#endif
    case SIOCSENDARPTOGW :
	{
            	nk_send_arp_t nk_send_arp_s;
		struct net_device *odev;

		//printk(KERN_EMERG "SIOCSENDARPTOGW\n");

        	copy_from_user(&nk_send_arp_s, (struct nk_send_arp *)arg, sizeof(nk_send_arp_t));
//printk(KERN_EMERG "Send ARP from[%x] to[%x], dev=%s\n", nk_send_arp_s.src_ip, nk_send_arp_s.dst_ip, odev->name);
		if(!strncmp(nk_send_arp_s.dev_name, "eth", 3))
		{
			odev = __dev_get_by_name(nk_send_arp_s.dev_name);
	
			arp_send(1, 0x0806, nk_send_arp_s.dst_ip, odev,
				nk_send_arp_s.src_ip, NULL, NULL, NULL);
		}

	    break;
	}
#if 1
    // wan session number
    case SIOCGETWANSESSION :
	{
	    	//int j;
		//for(j=0; j<MAX_WAN_NUM; j++)
		//	session_num[j]=session_num[j];
		//session_num[4]=0;
		copy_to_user((int *)arg, &session_num, MAX_WAN_NUM * sizeof(int));
	    	break;
	}
   case SIOGETWANNEWSESSION :
       {
	    int i;
	    int new_session_num[MAX_WAN_NUM];

	    for(i=0;i<MAX_WAN_NUM;i++)
	    {
	    	//printk(KERN_EMERG "session_num_pre=%d, session_num=%d....", session_num_pre[i], session_num[i]);
		new_session_num[i]=session_num[i]-session_num_pre[i];
	    	if(new_session_num[i]<0) new_session_num[i]=0;
		//printk(KERN_EMERG "new_session=%d\n", new_session_num[i]);
	    }
	    copy_to_user((int *)arg, &new_session_num, MAX_WAN_NUM * sizeof(int));
	    break;
       }
#endif
    case SIOCCLEARBADSESSION :
	{
		struct ip_conntrack *ct;
		struct ip_conntrack_tuple_hash *h;
		int i;

		//printk(KERN_EMERG "SIOCCLEARBADSESSION\n");
		write_lock_bh(&ip_conntrack_lock);
		for (i = 0; i < ip_conntrack_htable_size; i++) 
		{
			list_for_each_entry(h, &ip_conntrack_hash[i], list) 
			{
				ct = tuplehash_to_ctrack(h);
				if( (ct) && (ct->tr.query==0) )
				{
					//if(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip==ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.ip)
					if (!(ct->status & IPS_SEEN_REPLY)) //unplied session
					{
						//printk(KERN_EMERG "");
						ct->timeout.expires=0;
						ct->tr.query=1;
					}
				}
			}
		}


		for (i = 0; i < ip_conntrack_htable_size; i++) 
		{
			list_for_each_entry(h, &ip_conntrack_hash[i], list) 
			{
				ct = tuplehash_to_ctrack(h);
				if(ct)
					ct->tr.query = 0;
			}
		}
		write_unlock_bh(&ip_conntrack_lock);

		break;
	}
    case SIOCCLEARUDPSESSION :
	{
		struct ip_conntrack *ct;
		struct ip_conntrack_tuple_hash *h;
		int i;

		printk(KERN_EMERG "Clear udp session\n");
		write_lock_bh(&ip_conntrack_lock);
		for (i = 0; i < ip_conntrack_htable_size; i++) 
		{
			list_for_each_entry(h, &ip_conntrack_hash[i], list) 
			{
				ct = tuplehash_to_ctrack(h);
				if( (ct) && (ct->tr.query==0) )
				{
					//if(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip==ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.ip)
					if((ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum==17))
					{
						//printk(KERN_EMERG "Clear udp session %d\n", i);
						ct->timeout.expires=-1*HZ;
						ct->tr.query=1;
					}
				}
			}
		}


		for (i = 0; i < ip_conntrack_htable_size; i++) 
		{
			list_for_each_entry(h, &ip_conntrack_hash[i], list) 
			{
				ct = tuplehash_to_ctrack(h);
				if(ct)
					ct->tr.query = 0;
			}
		}
		write_unlock_bh(&ip_conntrack_lock);

		break;
	}
	//20100518 selena: delete conntrack entry when NSD detect that some WAN is down
	case SIOCDELETEDIPSESSION :
	{
		struct ip_conntrack_tuple_hash *h;
		struct ip_conntrack *ct;
		int i;
		u_int32_t dip;

		dip = arg;
 		write_lock_bh(&ip_conntrack_lock);
 		for (i = 0; i < ip_conntrack_htable_size; i++) 
 		{
 			list_for_each_entry(h, &ip_conntrack_hash[i], list) 
 			{
 				ct = tuplehash_to_ctrack(h);
 				if( (ct) && (ct->tr.query==0) )
 				{
 					if(ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.ip==dip)
 					{
						if (del_timer(&ct->timeout))
						{
							ct->timeout.expires=0;
							add_timer(&ct->timeout);
						}
 						ct->tr.query=1;
 					}
 				}
 			}
 		}
 
 		for (i = 0; i < ip_conntrack_htable_size; i++) 
 		{
 			list_for_each_entry(h, &ip_conntrack_hash[i], list) 
 			{
 				ct = tuplehash_to_ctrack(h);
 				if(ct)
 					ct->tr.query = 0;
 			}
 		}
 		write_unlock_bh(&ip_conntrack_lock);

		break;
	}
    case SIOCCLEARIPSESSION :
	{
		struct ip_conntrack *ct;
		struct ip_conntrack_tuple_hash *h;
		int i;
		u_int32_t dip;

		//printk(KERN_EMERG "SIOCCLEARIPSESSION\n");
		copy_from_user(&dip, (u_int32_t *)arg, 4);

		//printk(KERN_EMERG "delete ip=%u.%u.%u.%u\n", NIPQUAD(dip));
		write_lock_bh(&ip_conntrack_lock);
		for (i = 0; i < ip_conntrack_htable_size; i++) 
		{
			list_for_each_entry(h, &ip_conntrack_hash[i], list) 
			{
				ct = tuplehash_to_ctrack(h);
				if( (ct) && (ct->tr.query==0) )
				{
					if((ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip==dip) || 
					   (ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.ip==dip))
					{
						//printk(KERN_EMERG "");
						ct->timeout.expires=0;
						ct->tr.query=1;
					}
				}
			}
		}

		for (i = 0; i < ip_conntrack_htable_size; i++) 
		{
			list_for_each_entry(h, &ip_conntrack_hash[i], list) 
			{
				ct = tuplehash_to_ctrack(h);
				if(ct)
					ct->tr.query = 0;
			}
		}
		write_unlock_bh(&ip_conntrack_lock);

		break;
	}
#if 0
    case SIOCCLEANIPCONN :
	{
		u_int32_t dmzhost;
		int i;

		//printk("delete dmzhost session!!!\n");
		copy_from_user(&dmzhost, (u_int32_t *)arg, 4);
		//printk(KERN_EMERG "dmzhost->ip=%u.%u.%u.%u\n", NIPQUAD(dmzhost));

		/* Kill all session of this ip*/
		//WRITE_LOCK(&ip_conntrack_lock);
		ASSERT_WRITE_LOCK(&ip_conntrack_lock);
		for (i = 0; i < ip_conntrack_htable_size; i++) {

			LIST_FIND(&ip_conntrack_hash[i], cleardmzsession,
			struct ip_conntrack_tuple_hash *, dmzhost);
		}
		//WRITE_UNLOCK(&ip_conntrack_lock);
		break;
	}
    case SIOCCLEANIPCONN2 :
	{
		int i;
		port_forwarding_t *port_forwarding;
		struct ip_conntrack *ct;
		struct ip_conntrack_tuple_hash *h;
		

        	//printk(KERN_EMERG "delete forwarding session!!!\n");
		if(!(port_forwarding = kmalloc(sizeof(port_forwarding_t),GFP_ATOMIC)))
            		break;
		//printk(KERN_EMERG "start copy from user ...\n");
		copy_from_user(port_forwarding, (port_forwarding_t *)arg, sizeof(port_forwarding_t));
		//printk(KERN_EMERG "end copy from user ...\n");

		printk(KERN_EMERG "delete for_ip=%u.%u.%u.%u, port=%d\n", NIPQUAD( port_forwarding->ip), ntohs(port_forwarding->port));
		/* Kill all session of this ip*/
		//printk(KERN_EMERG "start search session ...\n");
		//WRITE_LOCK(&ip_conntrack_lock);
		/*ASSERT_WRITE_LOCK(&ip_conntrack_lock);
		for (i = 0; i < ip_conntrack_htable_size; i++) {
			LIST_FIND(&ip_conntrack_hash[i], clear_forwarding_session,
			struct ip_conntrack_tuple_hash *, port_forwarding->ip, port_forwarding->port);
		}*/
		write_lock_bh(&ip_conntrack_lock);
		for (i = 0; i < ip_conntrack_htable_size; i++) 
		{
			list_for_each_entry(h, &ip_conntrack_hash[i], list) 
			{
				ct = tuplehash_to_ctrack(h);
				if ((ct->tr.query==0) && (ntohs(port_forwarding->port)==0) &&
					(ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.ip == port_forwarding->ip))
				{
					ct->timeout.expires=0;
					printk(KERN_EMERG "clear_forwarding_session %u.%u.%u.%u -> %u.%u.%u.%u expire=%d\n",
					NIPQUAD(ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.ip), NIPQUAD(ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.ip),
					ct->timeout.expires);
				}
				else if( (ct->tr.query==0) && (ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.ip == port_forwarding->ip) && 
				    (ntohs(ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.u.all)==ntohs(port_forwarding->port)))
				{
					ct->timeout.expires=0;
					printk(KERN_EMERG "clear_forwarding_session %u.%u.%u.%u -> %u.%u.%u.%u expire=%d\n",
					NIPQUAD(ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.ip), NIPQUAD(ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.ip),
					ct->timeout.expires);
				}
				ct->tr.query=1;
			}
		}
	
		for (i = 0; i < ip_conntrack_htable_size; i++) 
		{
			list_for_each_entry(h, &ip_conntrack_hash[i], list) 
			{
				ct = tuplehash_to_ctrack(h);
				ct->tr.query=0;
			}
		}
		write_unlock_bh(&ip_conntrack_lock);

		kfree(port_forwarding);
		break;
	}
#endif
	/* arp spoof protect incifer 2006/06/28 */
#ifdef CONFIG_SUPPORT_ARP_SPOOF_PROTECT
	/* send arp */
	case ARPSPOOFPROTECT:
	{
		int isenable = 0;
		nk_send_arp_t nk_send_arp_s;
		struct net_device *odev;

		//printk(KERN_EMERG "firewall ARPSPOOFPROTECT\n");

		copy_from_user(&nk_send_arp_s,(struct nk_send_arp *)arg,sizeof(nk_send_arp_t));

		odev = __dev_get_by_name(nk_send_arp_s.dev_name);

		//printk(KERN_EMERG "Send ARP from[%u.%u.%u.%u] to [%u.%u.%u.%u], dev=%s arp_num[%d]\n", NIPQUAD(nk_send_arp_s.src_ip), NIPQUAD(nk_send_arp_s.dst_ip), odev->name, nk_send_arp_s.arp_num);

		nk_send_arp_fw.dst_ip = nk_send_arp_s.dst_ip;
		nk_send_arp_fw.src_ip = nk_send_arp_s.src_ip;
		nk_send_arp_fw.arp_num = nk_send_arp_s.arp_num;
		strcpy(nk_send_arp_fw.dev_name,nk_send_arp_s.dev_name);

		if((nk_send_arp_s.src_ip == 0) && (nk_send_arp_s.dst_ip == 0))
		{
			//printk(KERN_EMERG "Disable ARP Spoof\n");
			nk_send_arp_enable = 0;
			del_arp_timer();
		}
		else
		{
			//printk(KERN_EMERG "Enable ARP Spoof\n");
			nk_send_arp_enable = 1;

			del_arp_timer();
			init_timer(&update_arp_timer);
			update_arp_timer.function = nk_update_arp_protect;
			update_arp_timer_set();
		}
		break;
	}
#ifdef CONFIG_SUPPORT_IP_MAC_BINDING
	case SIOCSETSTATICARP:
	{
		ip_mac_bind_entry_t *ip_mac_bind_entry_s;
		struct in_addr aaa;
		int idx,num;
		u_int8_t mac[6];

		//printk(KERN_EMERG "firewall SIOSETSTATICARP\n");
		if(nk_send_arp_enable == 0)
		{
			//printk(KERN_EMERG "do not set static arp\n");
			break;
		}
		//printk(KERN_EMERG "set learn list static arp\n");

//2007/11/14 dont set static arp
// 		for(idx=0;idx<16;idx++)
// 		{
// 			ip_mac_bind_entry_s = nk_ip_mac_learn_list[idx];
// 			for(;ip_mac_bind_entry_s;ip_mac_bind_entry_s = ip_mac_bind_entry_s->next)
// 			{
// 				aaa.s_addr = ip_mac_bind_entry_s->ip;
// 				memcpy(mac,ip_mac_bind_entry_s->mac,sizeof(u_int8_t)*6);
// 				num =ip_mac_bind_entry_s->number;
// 				nk_set_static_arp(ip_mac_bind_entry_s);
// 				//printk(KERN_EMERG "ip[%u.%u.%u.%u],mac[%02x-%02x-%02x-%02x-%02x-%02x],num[%d]",NIPQUAD(aaa.s_addr),mac[0],mac[1],mac[2],mac[3],mac[4],mac[5],num);
// 			}
// 			//printk(KERN_EMERG "\n");
// 		}

		/* Arp Spoof Protect:show ip&mac binding rule set static arp incifer 2006/08/17 */
		//printk(KERN_EMERG "set ip mac bind list static arp\n");
		for(idx=0; idx<IP_MAC_ENTRY_HASH_SIZE; idx++)
		{
			ip_mac_bind_entry_s = nk_ip_mac_bind_entry_list[idx];

			for(;ip_mac_bind_entry_s;ip_mac_bind_entry_s = ip_mac_bind_entry_s->next)
			{
				aaa.s_addr = ip_mac_bind_entry_s->ip;
				memcpy(mac,ip_mac_bind_entry_s->mac,sizeof(u_int8_t)*6);
				num =ip_mac_bind_entry_s->number;
				nk_set_static_arp(ip_mac_bind_entry_s);
				//printk(KERN_EMERG "ip[%u.%u.%u.%u],mac[%02x-%02x-%02x-%02x-%02x-%02x],num[%d]\n",NIPQUAD(aaa.s_addr),mac[0],mac[1],mac[2],mac[3],mac[4],mac[5],num);
			}
			//printk(KERN_EMERG "\n");
		}

		break;
	}
	case SIOCCLRSTATICARP:
	{
		ip_mac_bind_entry_t *ip_mac_bind_entry_s;
		struct in_addr aaa;
		int idx,num;
		u_int8_t mac[6];

		//printk(KERN_EMERG "SIOCCLRSTATICARP\n");
		for(idx=0; idx<IP_MAC_ENTRY_HASH_SIZE; idx++)
		{
			ip_mac_bind_entry_s = nk_ip_mac_bind_entry_list[idx];

			for(;ip_mac_bind_entry_s;ip_mac_bind_entry_s = ip_mac_bind_entry_s->next)
			{
				aaa.s_addr = ip_mac_bind_entry_s->ip;
				memcpy(mac,ip_mac_bind_entry_s->mac,sizeof(u_int8_t)*6);
				num =ip_mac_bind_entry_s->number;
				//printk(KERN_EMERG "ip[%u.%u.%u.%u], mac[%X:%X:%X:%X:%X:%X]\n", NIPQUAD(aaa.s_addr), mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
				nk_del_static_arp(ip_mac_bind_entry_s);
			}
		}

		break;
	}
#endif
#endif
	//-->
/* Arp Spoof Protect:set nk_ip_mac_bind_entry_list incifer 2006/08/17 */
#ifdef CONFIG_SUPPORT_ARP_SPOOF_PROTECT
	case SIOCADDIPMACBIND:
	{
		ip_mac_bind_entry_t *tmp,*tmp_entry;
		int idx;
		//printk(KERN_EMERG "SIOCADDIPMACBIND\n");
		tmp_entry = (ip_mac_bind_entry_t *)kmalloc(sizeof(ip_mac_bind_entry_t), GFP_ATOMIC);
		if(!tmp_entry)
		{
			printk(KERN_EMERG "kmalloc ip_mac_bind_entry_t fail\n");
			return -1;
		}
		copy_from_user(tmp_entry,(ip_mac_bind_entry_t *)arg,sizeof(ip_mac_bind_entry_t));
		//idx = 0;  2007/11/14
		idx = tmp_entry->mac[5]&IP_MAC_ENTRY_HASH_IDX;
		for(tmp = nk_ip_mac_bind_entry_list[idx]; tmp; tmp=tmp->next)
		{
			if(tmp->ip == tmp_entry->ip &&
				tmp->mac[5]==tmp_entry->mac[5] &&
				tmp->mac[4]==tmp_entry->mac[4] &&
				tmp->mac[3]==tmp_entry->mac[3]    )
			{
				kfree(tmp_entry);
				//printk(KERN_EMERG "goto entry_exist\n\n");
				goto entry_exist;
			}
		}
		tmp_entry->next = nk_ip_mac_bind_entry_list[idx];
		nk_ip_mac_bind_entry_list[idx] = tmp_entry;

		//printk(KERN_EMERG "ip_mac_bind_entry ip[%d.%d.%d.%d],mac[%02x-%02x-%02x-%02x-%02x-%02x],num[%d]\n",NIPQUAD(ip_mac_bind_entry->ip),NMACQUAD(ip_mac_bind_entry->mac),ip_mac_bind_entry->number);
	entry_exist:
		//printk(KERN_EMERG "=============END==============123\n\n");
		break;
	}
	case SIOCCLRIPMACBIND:
	{
		ip_mac_bind_entry_t *tmp, *tmp_entry;
		int i;
		//printk(KERN_EMERG "SIOCCLRIPMACBIND\n");
		for(i=0; i<IP_MAC_ENTRY_HASH_SIZE; i++)
		{
			tmp_entry = nk_ip_mac_bind_entry_list[i];
			while(tmp_entry)
			{
				tmp = tmp_entry;
				tmp_entry = tmp_entry->next;
				kfree(tmp);
			}
			nk_ip_mac_bind_entry_list[i] = NULL;
		}
		break;
	}
#endif
//-->

/*2007/12/14 trenchen : support netbios broadcast--->*/
#ifdef CONFIG_NK_IPSEC_NETBIOS_BC
    case SIOCADIPSCEBC :	//david
		{
			int count=0;

//			printk("ipsec ioctl in SIOCADIPSCEBC\n");
			if (!(new_bc_ip = kmalloc(sizeof(nat_ipsec_bc_t), GFP_ATOMIC))) {
				printk("ipsec netbios: error alloc memory fail\n");
				return -1;
			}

	    		error = copy_from_user(new_bc_ip, (struct nat_ipsec_bc_t *)arg, sizeof(nat_ipsec_bc_t));
			if(error){
				kfree(new_bc_ip);
				printk("ipsec netbios: error copy from user\n");
				break;
			}

			//insert entry at head
			BiosIndex = NetbiosHashGet(new_bc_ip->local_bc);

			spin_lock_bh( &(nat_ipsec_bc_ip_list[BiosIndex].BiosBroadLock) );
			new_bc_ip->next = nat_ipsec_bc_ip_list[BiosIndex].start;
			nat_ipsec_bc_ip_list[BiosIndex].start = new_bc_ip;
			spin_unlock_bh( &(nat_ipsec_bc_ip_list[BiosIndex].BiosBroadLock) );

#if 0			
			for(count=0;count<BIOSHASHMAX;count++){
				spin_lock_bh( &(nat_ipsec_bc_ip_list[count].BiosBroadLock) );
				if(nat_ipsec_bc_ip_list[count].start){
					for(bc_ip_list=&(nat_ipsec_bc_ip_list[count].start);*bc_ip_list;bc_ip_list=&(*bc_ip_list)->next){
						printk(KERN_EMERG "\nioctl set name[%s] local[%x] remote[%x] \n",(*bc_ip_list)->tunnel_name,(*bc_ip_list)->local_bc,(*bc_ip_list)->remote_bc);
					}
				}
				spin_unlock_bh( &(nat_ipsec_bc_ip_list[count].BiosBroadLock) );
			}
#endif
			break;
		}
    case SIOCRMIPSCEBC :
		{

			if (!(new_bc_ip = kmalloc(sizeof(nat_ipsec_bc_t), GFP_ATOMIC))){
				printk("ipsec netbios: error alloc memory fail\n");
				return -1;
			}
			error = copy_from_user(new_bc_ip, (struct nat_ipsec_bc_t *)arg, sizeof(nat_ipsec_bc_t));
			if(error){
				kfree(new_bc_ip);
				printk("ipsec netbios: error copy from user\n");
				break;
			}
			
			del_bc_ip = 0;
			BiosIndex = NetbiosHashGet(new_bc_ip->local_bc);
			spin_lock_bh( &(nat_ipsec_bc_ip_list[BiosIndex].BiosBroadLock) );
			for(bc_ip_list=&(nat_ipsec_bc_ip_list[BiosIndex].start);*bc_ip_list;bc_ip_list=&(*bc_ip_list)->next){
				if( !strcmp( (*bc_ip_list)->tunnel_name, new_bc_ip->tunnel_name) ){
					del_bc_ip = *bc_ip_list;
					*bc_ip_list = (*bc_ip_list)->next;
					break;
				}
			}
			spin_unlock_bh( &(nat_ipsec_bc_ip_list[BiosIndex].BiosBroadLock) );
			if(del_bc_ip)
				kfree(del_bc_ip);
			kfree(new_bc_ip);
//			printk(KERN_EMERG "ioctl del name[%s] local[%x] remote[%x]\n",new_bc_ip->tunnel_name,new_bc_ip->local_bc,new_bc_ip->remote_bc);
			break;
		}
#endif
/*<--------trenchen netbios broadcast*/

#ifdef CONFIG_NK_IPSEC_SPLITDNS
//--->20100104 trenchen : split dns
    case SIOCADDNSSPL :
		{
			//ipsec_split_dns_t *temp;

			//printk("split dns set in kerenl\n");
			if( (spdnsnew = kmalloc(sizeof(ipsec_split_dns_t), GFP_ATOMIC))==0 ){
				printk("split dns : error alloc memory fail\n");
				return -1;
			}
			errorsdns = copy_from_user(spdnsnew, (ipsec_split_dns_t *)arg, sizeof(ipsec_split_dns_t));
			if(errorsdns){
				kfree(spdnsnew);
				printk("split dns : error copy from user\n");
				break;
			}
			spin_lock_bh(&(ipsec_split_dns_list.SplitdnsLock)); 
			spdnsnew->next = ipsec_split_dns_list.start;
			ipsec_split_dns_list.start = spdnsnew;

#if 0
			for(temp=ipsec_split_dns_list.start;temp;temp=temp->next){
				printk("split dns tunnel[%s] \n",temp->tunnel_name);
			}
#endif

			spin_unlock_bh(&(ipsec_split_dns_list.SplitdnsLock));
			break;
		}

    case SIOCRMDNSSPL :
		{
			ipsec_split_dns_t **splitdnslist;
			ipsec_split_dns_t *splitdnsdel;

			//printk("split dns del in kerenl\n");

			splitdnsdel = 0;
			splitdnslist = 0;
			if( (spdnsnew = kmalloc(sizeof(ipsec_split_dns_t), GFP_ATOMIC))==0 ){
				printk("split dns : error alloc memory fail\n");
				return -1;
			}
			errorsdns = copy_from_user(spdnsnew, (ipsec_split_dns_t *)arg, sizeof(ipsec_split_dns_t));
			if(errorsdns){
				kfree(spdnsnew);
				printk("split dns : error copy from user\n");
				break;
			}
			
			spin_lock_bh(&(ipsec_split_dns_list.SplitdnsLock)); 
			for(splitdnslist = &(ipsec_split_dns_list.start); *splitdnslist; splitdnslist = &(*splitdnslist)->next){
				if( !strcmp( (*splitdnslist)->tunnel_name, spdnsnew->tunnel_name) ){
					splitdnsdel = *splitdnslist;
					*splitdnslist = (*splitdnslist)->next;
					break;
				}
			}
			spin_unlock_bh(&(ipsec_split_dns_list.SplitdnsLock));
			
			if(splitdnsdel)
				kfree(splitdnsdel);
			kfree(spdnsnew);

			break;
		}
//<---------------------------------------
#endif

#if 0
/*2006/07/05 Ryoko add support netbios*/
#ifdef CONFIG_IPSEC_NETBIOS_BC
    case SIOCADIPSCEBC :	//david
		{
//			printk(KERN_EMERG "ioctl SIOCADIPSCEBC\n");
			if (!(new_bc_ip = kmalloc(sizeof(nat_ipsec_bc_t), GFP_ATOMIC)))
			return -1;
//	    	error = copy_from_user(&new_bc_ip, (struct new_bc_ip *)arg, sizeof(nat_ipsec_bc_t));
	    	error = copy_from_user(new_bc_ip, (struct nat_ipsec_bc_t *)arg, sizeof(nat_ipsec_bc_t));
//			printk(KERN_EMERG "ioctl copy_from_user\n");

#if 0
		printk(KERN_EMERG "ioctl copy_from_user start\n");
		printk(KERN_EMERG "local_bc=%u\n",new_bc_ip->local_bc);
		printk(KERN_EMERG "remote_bc=%u\n",new_bc_ip->remote_bc);
#endif
			if (!error)
			{
					for (bc_ip_list = &nat_ipsec_bc_ip_list;
						*bc_ip_list;
						bc_ip_list = &(*bc_ip_list)->next)
					{
						if(!strcmp((*bc_ip_list)->tunnel_name, new_bc_ip->tunnel_name))
						{
							memcpy((char *)(*bc_ip_list)+4, (char *)new_bc_ip+4,sizeof(nat_ipsec_bc_t)-4);
							break;
						}
					}
					if(*bc_ip_list==NULL)
					{
				//printk("David: add IPSEC BC name[%s], %x => %x \n",new_bc_ip->tunnel_name,new_bc_ip->local_bc,new_bc_ip->remote_bc);
						new_bc_ip->next = nat_ipsec_bc_ip_list;
						nat_ipsec_bc_ip_list = new_bc_ip;
					}
			}
			else
			{
				kfree(new_bc_ip);
			}
			break;
		}
    case SIOCRMIPSCEBC :
		{
			if (!(new_bc_ip = kmalloc(sizeof(nat_ipsec_bc_t), GFP_ATOMIC)))
				return -1;
				error = copy_from_user(new_bc_ip, (struct nat_ipsec_bc_t *)arg, sizeof(nat_ipsec_bc_t));

			if (!error)
			{
					for (bc_ip_list = &nat_ipsec_bc_ip_list;
						*bc_ip_list;        )
					{
						if(!strcmp((*bc_ip_list)->tunnel_name, new_bc_ip->tunnel_name))
						{
				//printk("David: remove IPSEC BC name[%s], %x => %x \n",new_bc_ip->tunnel_name,new_bc_ip->local_bc,new_bc_ip->remote_bc);
						del_bc_ip = *bc_ip_list;
						*bc_ip_list = (*bc_ip_list)->next;
						kfree(del_bc_ip);
						}
						else
						bc_ip_list = &(*bc_ip_list)->next;
					}

					kfree(new_bc_ip);
			}
			else
			{
				kfree(new_bc_ip);
			}
			break;
		}
#endif
#endif

/* support set fw_options incifer 1128 */
#ifdef CONFIG_SUPPORT_IP_MAC_BINDING
	case SIOCFWOPTS:
	{
		//int i;
		copy_from_user(&nk_fw_opts,(nk_fw_opt_t *)arg,sizeof(nk_fw_opt_t));
		printk(KERN_EMERG "wrong ip[%d],not_list[%d]\n",nk_fw_opts.nk_fw_block_wrong_ip,nk_fw_opts.nk_fw_block_not_on_list);
#if 0
		if(nk_fw_opts.nk_fw_block_wrong_ip || nk_fw_opts.nk_fw_block_not_on_list)
		{
			for(i=0; i<16; i++)
			{
				for(;ip_mac_bind_entry = nk_ip_mac_learn_list[i];)
				{
					nk_ip_mac_learn_list[i] = ip_mac_bind_entry->next;
					kfree(ip_mac_bind_entry);
				}
			}
			ip_mac_number = 0;
			ip_mac_learn_entry_cnt = 0;
		}
#endif
		break;
	}
#endif

#ifdef CONFIG_NK_SESSION_LIMIT
    case SIOCSESSIONLIMITSETUP :
	{
		session_limit_setting_t session_limit_setting;
		int i, tmp=0;
		session_limit_t *session_limit_p;
	
		/* if no wanip, disable session limit*/
		for(i=0;i<MAX_WAN_NUM;i++)
		{
			if(wan_interface_session[i].wan_ip!=0)
				tmp=1;
		}
		if(tmp==0)
		{
			session_limit_status=0;
			break;
		}

		copy_from_user(&session_limit_setting, (struct session_limit_setting *)arg, sizeof(session_limit_setting_t));
	
		session_limit_status = session_limit_setting.session_limit_status;
		session_limit_max_session1 = session_limit_setting.session_limit_max_session1;
		session_limit_max_session2 = session_limit_setting.session_limit_max_session2;
		session_limit_max_tcp = session_limit_setting.max_tcp;
		session_limit_max_udp = session_limit_setting.max_udp;
		session_limit_block_minute1 = session_limit_setting.session_limit_block_minute1;
		session_limit_block_minute2 = session_limit_setting.session_limit_block_minute2;
		//if(session_limit_max_session1==session_limit_max_session2==0)
			//session_limit_status=0;
		scheduler = session_limit_setting.scheduler;
		days_match = session_limit_setting.days_match;
		time_start = session_limit_setting.time_start;
		time_stop = session_limit_setting.time_stop;
		printk(KERN_EMERG "status=%d, max1=%d, max2=%d, tcp=%d, udp=%d \n", session_limit_status,
			session_limit_max_session1, session_limit_max_session2, session_limit_max_tcp, session_limit_max_udp);
		//if(DEBUG) printk(KERN_EMERG "!!!days=%d, start=%d, stop=%d\n", session_limit_setting.days_match, session_limit_setting.time_start, session_limit_setting.time_stop);

		//20091202, clear block timer if status = 1 or 4
		if( (session_limit_status==1) || (session_limit_status==4) )
		{
			if(&nk_session_limit_timer)
				del_timer(&nk_session_limit_timer);

			//printk(KERN_EMERG "set block-timer to 0\n");
			for(i=0; i<128; i++)
			{
				for(session_limit_p = nk_session_limit[i];
					session_limit_p;
					session_limit_p = session_limit_p->next)
						session_limit_p->block_timer=0;
			}
		}
	
		if(session_limit_status==0)
		{
			for(i=0; i<128; i++)
			{
				for(; session_limit_p = nk_session_limit[i];)
				{
					nk_session_limit[i] = session_limit_p->next;
					kfree(session_limit_p);
				}
			}
			session_limit_entry_cnt = 0;
		}

		break;
        }
    case SIOCGETSESSIONLIMITBLOCKIP :
	{
		int i=0;
		session_limit_t *session_limit_p;
		if(session_limit_status==0)
			printk(KERN_EMERG "session limit is disabled\n");
		else
			printk(KERN_EMERG "show session limit blocked ip:\n");
		for(i=0; i<128; i++)
		{
			for(session_limit_p = nk_session_limit[i];
				session_limit_p;
				session_limit_p = session_limit_p->next)
			{
				//nk_session_limit[i] = session_limit_p->next;
				//printk(KERN_EMERG "%u.%u.%u.%u, cnt=%d\n", NIPQUAD(session_limit_p->ip), session_limit_p->session_cnt);
				if(session_limit_status==1)
				{
					if(session_limit_p->session_cnt>=session_limit_max_session1)
						printk(KERN_EMERG "******************\n");
					printk(KERN_EMERG "%u.%u.%u.%u (%d)\n", NIPQUAD(session_limit_p->ip), session_limit_p->session_cnt);
					if(session_limit_p->session_cnt>=session_limit_max_session1)
						printk(KERN_EMERG "******************\n");
				}
				else if(session_limit_status==2)
				{
					if(session_limit_p->block_timer>0)
						printk(KERN_EMERG "******************\n");
					printk(KERN_EMERG "%u.%u.%u.%u(%d), block time %d\n", NIPQUAD(session_limit_p->ip), session_limit_p->session_cnt, session_limit_p->block_timer);
					if(session_limit_p->block_timer>0)
						printk(KERN_EMERG "******************\n");
				}
			}
		}
		printk(KERN_EMERG "\n");
	}
#endif
#ifdef CONFIG_NK_SESSION_LIMIT_ENHANCE
    case SIOCSESSIONLIMITENHANCEINIT:
    {
        session_limit_enhance_node_list_t *session_limit_enhance_node,*tmp;
        session_limit_enhance_list_t *tmp2;
        int i;

        /* init nk_session_limit_enhance_alldomain_list */
        write_lock_bh(&SLE_ALLDOMAIN_LOCK);
        tmp2 = nk_session_limit_enhance_alldomain_list;
        while(tmp2)
        {
            nk_session_limit_enhance_alldomain_list = tmp2->next;
            kfree(tmp2);
            tmp2 = nk_session_limit_enhance_alldomain_list;
        }
        nk_session_limit_enhance_alldomain_list = NULL;
        write_unlock_bh(&SLE_ALLDOMAIN_LOCK);

        /* init nk_session_limit_enhance_cclass_list */
        write_lock_bh(&SLE_CCLASS_LOCK);
        tmp2 = nk_session_limit_enhance_cclass_list;
        while(tmp2)
        {
            nk_session_limit_enhance_cclass_list = tmp2->next;
            kfree(tmp2);
            tmp2 = nk_session_limit_enhance_cclass_list;
        }
        nk_session_limit_enhance_cclass_list = NULL;
        write_unlock_bh(&SLE_CCLASS_LOCK);

        /* init nk_session_limit_enhance_list */
//         write_lock_bh(&SLE_LOCK);
//         tmp2 = nk_session_limit_enhance_list;
//         while(tmp2)
//         {
//             nk_session_limit_enhance_list = tmp2->next;
//             kfree(tmp2);
//             tmp2 = nk_session_limit_enhance_list;
//         }
//         nk_session_limit_enhance_list = NULL;
//         write_unlock_bh(&SLE_LOCK);

        /* init nk_session_limit_enhance_node_list */
        write_lock_bh(&SLE_NODE_LOCK);
        for(i=0; i<256; i++)
        {
            session_limit_enhance_node = nk_session_limit_enhance_node_list[i];
            while(session_limit_enhance_node)
            {
                tmp = session_limit_enhance_node;
                session_limit_enhance_node = session_limit_enhance_node->next;
                kfree(tmp);
            }
            nk_session_limit_enhance_node_list[i] = NULL;
        }
        write_unlock_bh(&SLE_NODE_LOCK);
        break;
    }
    case SIOCSESSIONLIMITENHANCE:
    {
        session_limit_enhance_list_t *session_limit_enhance;

        if(!(session_limit_enhance = kmalloc(sizeof(session_limit_enhance_list_t),GFP_ATOMIC)))
            break;
        copy_from_user(session_limit_enhance, (session_limit_enhance_list_t *)arg, sizeof(session_limit_enhance_list_t));
	//printk(KERN_EMERG "SIOCSESSIONLIMITENHANCEn ip(%u.%u.%u.%u~%u.%u.%u.%u) port(%d~%d),typeName(%d), max_session(%d)\n",
	//NIPQUAD(session_limit_enhance->sip), NIPQUAD(session_limit_enhance->eip), session_limit_enhance->sport,session_limit_enhance->eport,session_limit_enhance->typeName, session_limit_enhance->max_session);
	session_limit_enhance_order(session_limit_enhance);

        break;
    }
#endif
//#ifdef CONFIG_NK_QOS_SCHED
#if 1
    case SIOCQOSIPSORT:
    {
	printk(KERN_EMERG "SIOCQOSIPSORT\n");
#if 0
        int i=0, max_sort=0, k, sdir=0;
        smart_qos_ip_t *tmp_qos_ip,get_qos_ip;
	struct ip_conntrack *ct;
	struct ip_conntrack_tuple_hash *h;

        //printk(KERN_EMERG "SIOCQOSIPSORT\n");
        //copy_from_user(&get_qos_ip,(smart_qos_ip_t *)arg,sizeof(smart_qos_ip_t));

	write_lock_bh(&ip_conntrack_lock);
	for (i = 0; i < ip_conntrack_htable_size; i++) 
	{
		list_for_each_entry(h, &ip_conntrack_hash[i], list) 
		{
			if(h)
			{
				ct = tuplehash_to_ctrack(h);
				//printk(KERN_EMERG "CT: ip[%u.%u.%u.%u] update[%d]\n", NIPQUAD(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip), ct->tr.update);
				if( (ct) && (ct->tr.update==0) )
				{
					
					for(k=0;k<MAX_WAN_NUM;k++)
					{
						if( (ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip==wan_interface_session[k].wan_ip) ||
						    ((ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip>=wan_interface_session[k].internallanip1)&&
						    (ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip<=wan_interface_session[k].internallanip2)) ||
						    ((ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip>=wan_interface_session[k].internallanip3)&&
						    (ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip<=wan_interface_session[k].internallanip4)) )
						{
							sdir=2; //session direction, 2=forwaring
							break;
						}
						
					}
	
					for(tmp_qos_ip = nk_smart_qos_ip_list;tmp_qos_ip;tmp_qos_ip = tmp_qos_ip->next)
					{
						if((tmp_qos_ip->ip.s_addr == ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip) ||
						   (tmp_qos_ip->ip.s_addr == ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.ip))
						{
							tmp_qos_ip->up_bw += ct->tr.up_bytes_delta;
							tmp_qos_ip->down_bw += ct->tr.down_bytes_delta;
							tmp_qos_ip->session ++;
							break;
						}
					}
					if(!tmp_qos_ip)
					{
						tmp_qos_ip = (smart_qos_ip_t *)kmalloc(sizeof(smart_qos_ip_t),GFP_ATOMIC);
						if(tmp_qos_ip)
						{
							bzero((char *)tmp_qos_ip, sizeof(smart_qos_ip_t));
							if(sdir==2)
								tmp_qos_ip->ip.s_addr = ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.ip;
							else
								tmp_qos_ip->ip.s_addr = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip;
		
							tmp_qos_ip->up_bw = ct->tr.up_bytes_delta;
							tmp_qos_ip->down_bw = ct->tr.down_bytes_delta;
							tmp_qos_ip->session = 1;
							tmp_qos_ip->next = nk_smart_qos_ip_list;
							nk_smart_qos_ip_list = tmp_qos_ip;
							max_sort++;
						}
					}
					ct->tr.update=1;
				}
			}
		}
	}

        //get_qos_ip.next = nk_smart_qos_ip_list;
        //////////////copy_to_user((smart_qos_ip_t *)arg,&get_qos_ip,sizeof(smart_qos_ip_t));

//	printk(KERN_EMERG "tmp_qos_ip=%x\n", tmp_qos_ip);
//	kfree(tmp_qos_ip);
#if 0
        //====debug=========
        //tmp_qos_ip = nk_smart_qos_ip_list;
        //for(i=0;tmp_qos_ip;tmp_qos_ip = tmp_qos_ip->next)
	for(tmp_qos_ip = nk_smart_qos_ip_list;tmp_qos_ip;tmp_qos_ip = tmp_qos_ip->next)
        {
            printk(KERN_EMERG "ip[%u.%u.%u.%u],up_bw[%d],down_bw[%d],session[%d]\n",NIPQUAD(tmp_qos_ip->ip.s_addr),tmp_qos_ip->up_bw,tmp_qos_ip->down_bw,tmp_qos_ip->session);
        }
#endif
        //==================
	for (i = 0; i < ip_conntrack_htable_size; i++) 
	{
		list_for_each_entry(h, &ip_conntrack_hash[i], list) 
		{
			if(h)
			{
				ct = tuplehash_to_ctrack(h);
				if(ct)
					ct->tr.update = 0;
			}
		}
	}
	write_unlock_bh(&ip_conntrack_lock);
	//copy_to_user((smart_qos_ip_t *)arg,&get_qos_ip,sizeof(smart_qos_ip_t));
#endif
        break;
    }
    case SIOCCLRQOSIPSORT:
    {

        smart_qos_ip_t *tmp_qos_ip;

	printk(KERN_EMERG "SIOCCLRQOSIPSORT\n");
#if 0
        tmp_qos_ip = nk_smart_qos_ip_list;
        while(tmp_qos_ip)
        {
            nk_smart_qos_ip_list = tmp_qos_ip->next;
            kfree(tmp_qos_ip);
            tmp_qos_ip = nk_smart_qos_ip_list;
        }
        nk_smart_qos_ip_list = NULL;
#endif
        //---------------------------------
	if(nk_smart_qos_ip_up_list!=NULL)
	{
		tmp_qos_ip = nk_smart_qos_ip_up_list;
		while(tmp_qos_ip)
		{
		nk_smart_qos_ip_up_list = tmp_qos_ip->next;
		kfree(tmp_qos_ip);
		tmp_qos_ip = nk_smart_qos_ip_up_list;
		}
		nk_smart_qos_ip_up_list = NULL;
	}
        //---------------------------------
	if(nk_smart_qos_ip_down_list!=NULL)
	{
		tmp_qos_ip = nk_smart_qos_ip_down_list;
		while(tmp_qos_ip)
		{
		nk_smart_qos_ip_down_list = tmp_qos_ip->next;
		kfree(tmp_qos_ip);
		tmp_qos_ip = nk_smart_qos_ip_down_list;
		}
		nk_smart_qos_ip_down_list = NULL;
	}
        //---------------------------------
	copy_qos_ip = NULL;

        break;
    }

    /*case SIOCQOSIPCOPY:
    {
        smart_qos_ip_t *tmp_qos_ip,*tmp,get_qos_ip;
        u_int32_t up_bw=0,session=0;
        struct in_addr ip;
	
	printk("SIOCQOSIPCOPY\n");
	copy_from_user(&get_qos_ip,(smart_qos_ip_t *)arg,sizeof(smart_qos_ip_t));
 printk("2_ip[%u.%u.%u.%u],up_bw[%d],down_bw[%d],session[%d]\n",NIPQUAD(get_qos_ip.ip.s_addr),get_qos_ip.up_bw,get_qos_ip.down_bw,get_qos_ip.session);

	tmp_qos_ip = get_qos_ip.next;

 printk("3_ip[%u.%u.%u.%u],up_bw[%d],down_bw[%d],session[%d]\n",NIPQUAD(tmp_qos_ip->ip.s_addr),tmp_qos_ip->up_bw,tmp_qos_ip->down_bw,tmp_qos_ip->session);
	copy_to_user((smart_qos_ip_t *)arg,tmp_qos_ip,sizeof(smart_qos_ip_t));
	break;	
    }*/
    case SIOCQOSIPCOPY:
    {
        //u_int32_t up_bw=0,session=0;
        struct in_addr;
	
	printk("SIOCQOSIPCOPY\n");
	//tmp_qos_ip = get_qos_ip.next;
	if(copy_qos_ip)
	{
		//printk("3_ip[%u.%u.%u.%u],up_bw[%d],down_bw[%d],session[%d]\n",NIPQUAD(copy_qos_ip->ip.s_addr),copy_qos_ip->up_bw,copy_qos_ip->down_bw,copy_qos_ip->session);
		//ip = copy_qos_ip->ip.s_addr;
		copy_to_user((smart_qos_ip_t *)arg, copy_qos_ip, sizeof(smart_qos_ip_t));
		copy_qos_ip = copy_qos_ip->next;
	}
	break;	
    }
    case SIOCQOSIPSORTUP:
    {
        smart_qos_ip_t *tmp_qos_ip,*tmp;
	smart_qos_ip_t get_qos_ip;
        u_int64_t up_bw=0;
	u_int32_t session=0;
        struct in_addr;
//==========add for write file================
	int fd;
	mm_segment_t old_fs = get_fs();
	char pBuf[20];
	unsigned int catch_up_ip = 0;

	printk("SIOCQOSIPSORTUP\n");
	if(nk_smart_qos_ip_list==NULL) 
	{
		printk("nk_smart_qos_ip_list==NULL\n");
		break;	
	}
	printk("SIOCQOSIPSORTUP-1\n");
//==========================================
//================open file=======================
	set_fs(KERNEL_DS);
	fd = sys_open("/tmp/smart_qos_up_ip", O_WRONLY|O_CREAT|O_TRUNC, 0666);
	if(fd < 0)
	{
		printk(KERN_EMERG "!!!!!!!!sys_open error!!!!!\n");	
		break;
	}
//================================================
        copy_from_user(&get_qos_ip,(smart_qos_ip_t *)arg,sizeof(smart_qos_ip_t));
	//printk(KERN_EMERG "===get_qos_ip.up_bw=%d\n", get_qos_ip.up_bw);
        //if(!error)
        //{
	catch_up_ip = 0;
            if(get_qos_ip.ip.s_addr == 0)
            {
                up_bw = get_qos_ip.up_bw;
                session = get_qos_ip.session;
                tmp_qos_ip = nk_smart_qos_ip_list;
                while(tmp_qos_ip)
                {
		    //printk(KERN_EMERG "tmp_qos_ip->up_bw=%d", tmp_qos_ip->up_bw);
                    if(tmp_qos_ip->up_bw >= up_bw || tmp_qos_ip->session >= session)
                    {
                        tmp = (smart_qos_ip_t *)kmalloc(sizeof(smart_qos_ip_t),GFP_ATOMIC);
                        tmp->ip.s_addr = tmp_qos_ip->ip.s_addr;
                        tmp->up_bw = tmp_qos_ip->up_bw;
                        tmp->down_bw = tmp_qos_ip->down_bw;
                        tmp->session = tmp_qos_ip->session;
                        tmp->next = nk_smart_qos_ip_up_list;
//======================write IP to file=================
			sprintf(pBuf, "%u.%u.%u.%u\n", NIPQUAD(tmp->ip.s_addr));
			sys_write(fd, pBuf, strlen(pBuf));
			catch_up_ip ++;
//=====================================================
                        nk_smart_qos_ip_up_list = tmp;
                    }
                    tmp_qos_ip = tmp_qos_ip->next;
                }
                get_qos_ip.next = nk_smart_qos_ip_up_list;
		copy_qos_ip = nk_smart_qos_ip_up_list;//=========
                copy_to_user((smart_qos_ip_t *)arg,&get_qos_ip,sizeof(smart_qos_ip_t));
// //================close file=======================
// 		sys_close(fd);
// 		set_fs(old_fs);
// 		printk(KERN_EMERG "****************catch [%d] up ip in SIOCQOSIPSORTUP***************\n", catch_up_ip);
// //================================================
            }
            else
            {
                tmp_qos_ip = nk_smart_qos_ip_list;
                while(tmp_qos_ip)
                {
                    if(tmp_qos_ip->ip.s_addr == get_qos_ip.ip.s_addr)
                    {
                        tmp = (smart_qos_ip_t *)kmalloc(sizeof(smart_qos_ip_t),GFP_ATOMIC);
                        tmp->ip.s_addr = tmp_qos_ip->ip.s_addr;
                        tmp->up_bw = tmp_qos_ip->up_bw;
                        tmp->down_bw = tmp_qos_ip->down_bw;
                        tmp->session = tmp_qos_ip->session;
                        tmp->next = nk_smart_qos_ip_up_list;
                        nk_smart_qos_ip_up_list = tmp;

                        get_qos_ip.next = nk_smart_qos_ip_up_list;
                        copy_to_user((smart_qos_ip_t *)arg,&get_qos_ip,sizeof(smart_qos_ip_t));
                        break;
                    }
                    tmp_qos_ip = tmp_qos_ip->next;
                }
            }
//		printk(KERN_EMERG "tmp=%x\n", tmp);
//		kfree(tmp);
//================close file=======================
		sys_close(fd);
		set_fs(old_fs);
		printk(KERN_EMERG "****************catch [%d] up ip in SIOCQOSIPSORTUP***************\n", catch_up_ip);
//================================================

#if 0
            //====debug=========
            tmp_qos_ip = nk_smart_qos_ip_up_list;
            for(i=0;tmp_qos_ip;tmp_qos_ip = tmp_qos_ip->next,i++)
            {
                printk("!!!!!!!!!!!!1%d:ip[%u.%u.%u.%u],up_bw[%d],down_bw[%d],session[%d]\n",i,NIPQUAD(tmp_qos_ip->ip.s_addr),tmp_qos_ip->up_bw,tmp_qos_ip->down_bw,tmp_qos_ip->session);
            }
            //==================
#endif
        //}
        break;
    }
    case SIOCQOSIPSORTDOWN:
    {
        smart_qos_ip_t *tmp_qos_ip,*tmp,get_qos_ip;
        u_int64_t down_bw=0;
	u_int32_t session=0;
        struct in_addr;
//==========add for write file================
	int fd;
	mm_segment_t old_fs = get_fs();
	char pBuf[20];
	unsigned int catch_down_ip = 0;
	
	printk("SIOCQOSIPSORTDOWN\n");
	if(nk_smart_qos_ip_list==NULL)
	{
		printk("nk_smart_qos_ip_list==NULL\n");
		 break;
	}
	printk("SIOCQOSIPSORTDOWN-1\n");

//==========================================
//================open file=======================
	set_fs(KERNEL_DS);
	fd = sys_open("/tmp/smart_qos_down_ip", O_WRONLY|O_CREAT|O_TRUNC, 0666);
	if(fd < 0)
	{
		printk(KERN_EMERG "!!!!!!!!sys_open error!!!!!\n");	
		break;
	}
//================================================
	//printk(KERN_EMERG "SIOCQOSIPSORT*************DOWN\n");
        copy_from_user(&get_qos_ip,(smart_qos_ip_t *)arg,sizeof(smart_qos_ip_t));
	//printk(KERN_EMERG "===get_qos_ip.down_bw=%d\n", get_qos_ip.down_bw);
        //if(!error)
        //{
            if(get_qos_ip.ip.s_addr == 0)
            {
                down_bw = get_qos_ip.down_bw;
                session = get_qos_ip.session;
                tmp_qos_ip = nk_smart_qos_ip_list;
                while(tmp_qos_ip)
                {
                    if(tmp_qos_ip->down_bw >= down_bw || tmp_qos_ip->session >= session)
                    {
                        tmp = (smart_qos_ip_t *)kmalloc(sizeof(smart_qos_ip_t),GFP_ATOMIC);
                        tmp->ip.s_addr = tmp_qos_ip->ip.s_addr;
                        tmp->up_bw = tmp_qos_ip->up_bw;
                        tmp->down_bw = tmp_qos_ip->down_bw;
                        tmp->session = tmp_qos_ip->session;
                        tmp->next = nk_smart_qos_ip_down_list;
//======================write IP to file=================
			sprintf(pBuf, "%u.%u.%u.%u\n", NIPQUAD(tmp->ip.s_addr));
			sys_write(fd, pBuf, strlen(pBuf));
			catch_down_ip++;
//=====================================================
                        nk_smart_qos_ip_down_list = tmp;

                    }
                    tmp_qos_ip = tmp_qos_ip->next;
                }
                get_qos_ip.next = nk_smart_qos_ip_down_list;
		copy_qos_ip = nk_smart_qos_ip_down_list;//=========
                copy_to_user((smart_qos_ip_t *)arg,&get_qos_ip,sizeof(smart_qos_ip_t));
/*//================close file=======================
		sys_close(fd);
		set_fs(old_fs);
		printk(KERN_EMERG "****************catch [%d] dwon ip in SIOCQOSIPSORTDOWN***************\n", catch_down_ip);
//================================================*/
            }
            else
            {
                tmp_qos_ip = nk_smart_qos_ip_list;
                while(tmp_qos_ip)
                {
                    if(tmp_qos_ip->ip.s_addr == get_qos_ip.ip.s_addr)
                    {
                        tmp = (smart_qos_ip_t *)kmalloc(sizeof(smart_qos_ip_t),GFP_ATOMIC);
                        tmp->ip.s_addr = tmp_qos_ip->ip.s_addr;
                        tmp->up_bw = tmp_qos_ip->up_bw;
                        tmp->down_bw = tmp_qos_ip->down_bw;
                        tmp->session = tmp_qos_ip->session;
                        tmp->next = nk_smart_qos_ip_down_list;
                        nk_smart_qos_ip_down_list = tmp;

                        get_qos_ip.next = nk_smart_qos_ip_down_list;
                        copy_to_user((smart_qos_ip_t *)arg,&get_qos_ip,sizeof(smart_qos_ip_t));
                        break;
                    }
                    tmp_qos_ip = tmp_qos_ip->next;
                }
            }
//================close file=======================
		sys_close(fd);
		set_fs(old_fs);
		printk(KERN_EMERG "****************catch [%d] dwon ip in SIOCQOSIPSORTDOWN***************\n", catch_down_ip);
//================================================

//		printk(KERN_EMERG "tmp=%x\n", tmp);
//		kfree(tmp);
#if 0
            //====debug=========
            tmp_qos_ip = nk_smart_qos_ip_down_list;
            for(i=0;tmp_qos_ip;tmp_qos_ip = tmp_qos_ip->next,i++)
            {
                printk("###########%d:ip[%u.%u.%u.%u],up_bw[%d],down_bw[%d],session[%d]\n",i,NIPQUAD(tmp_qos_ip->ip.s_addr),tmp_qos_ip->up_bw,tmp_qos_ip->down_bw,tmp_qos_ip->session);
            }
            //==================
#endif
        //}
        break;
    }
#endif
//-->
	case SENDARPTOTRANSPARENTBRIDGE:
	{
		nk_send_arp_t nk_send_arp_s;
		struct net_device *odev;

		copy_from_user(&nk_send_arp_s,(struct nk_send_arp *)arg,sizeof(nk_send_arp_t));

		if(!strncmp(nk_send_arp_s.dev_name, "eth", 3))
		{
			odev = __dev_get_by_name(nk_send_arp_s.dev_name);	
			//printk(KERN_EMERG "src ip[%u.%u.%u.%u], dst ip[%u.%u.%u.%u], dev name[%s]\n", NIPQUAD(nk_send_arp_s.src_ip), NIPQUAD(nk_send_arp_s.dst_ip), nk_send_arp_s.dev_name);
			arp_send(2, 0x0806, nk_send_arp_s.dst_ip, odev, nk_send_arp_s.src_ip, NULL, NULL, NULL);
		}
		break;
	}
/* support IpBalance Struct Aging -- incifer 2009/01 */
#ifdef CONFIG_IPBALANCE_STRUCT_AGING
	case IPBALANCE_STRUCT_AGING:
	{
		int i_ipbase_mode=0;

		copy_from_user(&i_ipbase_mode,(int *)arg,sizeof(int));

		del_ipbalance_struct_aging_timer();
		if(i_ipbase_mode)
		{
			printk(KERN_EMERG "IPBALANCE_STRUCT_AGING: Start Aging Timer!!!\n");
			init_timer(&ipbalance_struct_aging_timer);
			ipbalance_struct_aging_timer.function = ipbalance_struct_check;
			set_ipbalance_struct_aging_timer(proc_ipbalance_struct_aging_start_time);
		}
		else
		{
			printk(KERN_EMERG "IPBALANCE_STRUCT_AGING: Stop Aging Timer!!!\n");
			/* Do nothing, because U have deleted timer above. */
		}

		break;
	}
#endif
#ifdef CONFIG_NK_RESTRICT_APP
	case SIOCRESTRICTAPP:
	{
		copy_from_user(&restrict_app, (restrict_app_t *)arg, sizeof(restrict_app_t));
		break;
	}

	case SIOCEXCEPTIONQQCLEARLIST:
	{
		exception_qq_t *exception_qq_p;

		//free exception QQ number list
		while(exception_qq_list)
		{
			exception_qq_p = exception_qq_list;
			exception_qq_list = exception_qq_p->next;
			kfree(exception_qq_p);
		}
		break;
	}

	case SIOCEXCEPTIONQQADDLIST:
	{
		exception_qq_t *exception_qq_p;

		exception_qq_p = kmalloc(sizeof(exception_qq_t),GFP_ATOMIC);
		if(!exception_qq_p)
			break;

		copy_from_user(exception_qq_p, (exception_qq_t *)arg, sizeof(exception_qq_t));
		exception_qq_p->next = exception_qq_list;
		exception_qq_list = exception_qq_p;
		break;
	}
#endif

	case SIOCWEBSERVICEQUERY:
	{
		web_service_port_t *web_service_port_p, *temp, **temp_p;

		web_service_port_p = kmalloc(sizeof(web_service_port_t), GFP_ATOMIC);
		if(!web_service_port_p)
			break;

		copy_from_user(web_service_port_p, (web_service_port_t *)arg, sizeof(web_service_port_t));
		//printk("Netfilter SIOCWEBSERVICEQUERY  %u.%u.%u.%u : %u\n", NIPQUAD(web_service_port_p->sip), web_service_port_p->sport);

		for(temp_p=&web_service_port_list; temp=*temp_p; )
		{
			if(temp->sport == web_service_port_p->sport &&
			   temp->sip == web_service_port_p->sip      )
			{
				web_service_port_p->dport = temp->dport;
				//printk("Netfilter find dport: %u\n", web_service_port_p->dport);
				copy_to_user((web_service_port_t *)arg, web_service_port_p, sizeof(web_service_port_t));
				
				*temp_p = temp->next;
				kfree(temp);
				continue;
			}
			temp_p=&(temp->next);
		}
		break;
	}
    case SIOCGETSESSIONNUMBER :
	{
		int session_count=0, i;
		struct ip_conntrack_tuple_hash *h;
		struct ip_conntrack *ct;

		write_lock_bh(&ip_conntrack_lock);
		for (i = 0; i < ip_conntrack_htable_size; i++) 
		{
			list_for_each_entry(h, &ip_conntrack_hash[i], list) 
			{
				ct = tuplehash_to_ctrack(h);
				if( (ct)&&(ct->tr.query==0)&&(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip != wan_interface_session[0].lanip) )
					session_count+=1;
					ct->tr.query=1;
			}
		}
		write_unlock_bh(&ip_conntrack_lock);

		printk(KERN_EMERG "session_count=%d\n", session_count);

		write_lock_bh(&ip_conntrack_lock);
		for (i = 0; i < ip_conntrack_htable_size; i++) 
		{
			list_for_each_entry(h, &ip_conntrack_hash[i], list) 
			{
				if(h)
				{
					ct = tuplehash_to_ctrack(h);
					if(ct)
						ct->tr.query = 0;
				}
			}
		}
		write_unlock_bh(&ip_conntrack_lock);

	    	break;
	}
	case SIOCCLRAGINGTIME:
	{
		//nk_agingtime_t *aging_list;
		nk_agingtime_t *aging_time_node, *tmp;
		int i;

		write_lock_bh(&SLE_NODE_LOCK);
		for(i=0; i<128; i++)
		{
			aging_time_node = nk_nk_agingtime_list[i];
			while(aging_time_node)
			{
				tmp = aging_time_node;
				aging_time_node = aging_time_node->next;
				kfree(tmp);
			}
			nk_nk_agingtime_list[i] = NULL;
		}
		write_unlock_bh(&SLE_NODE_LOCK);
		break;
	}
	case SIOCAGINGTIME:
	{
		nk_agingtime_t *aging_list, *nk_aging_p;
		int i, idx;


		if(!(aging_list = kmalloc(sizeof(nk_agingtime_t),GFP_ATOMIC)))
		break;
		copy_from_user(aging_list, (nk_agingtime_t *)arg, sizeof(nk_agingtime_t));

		//printk(KERN_EMERG "proto=%d, port=%d, aging=%d\n", aging_list->protocol, aging_list->port, aging_list->timeout);
	
		//init

		idx = aging_list->port&0x7f; //128

		for(nk_aging_p = nk_nk_agingtime_list[idx];
			nk_aging_p;
			nk_aging_p = nk_aging_p->next)
		{
			if(nk_aging_p->protocol == aging_list->protocol)
			{
			//printk(KERN_EMERG "session_limit_p[%u.%u.%u.%u] found, cnt[%d], timer[%d]\n",NIPQUAD(session_limit_p->ip),session_limit_p->session_cnt,session_limit_p->block_timer);
				break;
			}
		}

		if(nk_aging_p==NULL)
		{
			nk_aging_p = (nk_agingtime_t *)kmalloc(sizeof(nk_agingtime_t), GFP_ATOMIC);
		
			if(!nk_aging_p)
				return -1;
			nk_aging_p->protocol = aging_list->protocol;
			nk_aging_p->port = aging_list->port;
			nk_aging_p->timeout = aging_list->timeout;	
			nk_aging_p->next = nk_nk_agingtime_list[idx];
			nk_nk_agingtime_list[idx] = nk_aging_p;
		}
	

		for(i=0; i<128; i++)
		{
			for(nk_aging_p = nk_nk_agingtime_list[i];
				nk_aging_p;
				nk_aging_p = nk_aging_p->next)
			{
				printk(KERN_EMERG "idx=%d, protocol=%d, port=%d, timeout=%d\n", i, nk_aging_p->protocol, nk_aging_p->port, nk_aging_p->timeout);
			}
		}




		break;
	}

#ifdef CONFIG_NK_SUPPORT_USB_3G	
	case USB_LED_ON:
	{
		int iusbport=0;
		extern int USB_PORT3_OFF;
		extern int USB_PORT4_OFF;

		copy_from_user(&iusbport,(int *)arg,sizeof(int));
		
		switch(iusbport)
		{
			case 3:
					USB_PORT3_OFF = 0;	
					break;
			case 4:
					USB_PORT4_OFF = 0;				
					break;
		}
		
		USB_Set_LED();  //drivers/usb/storage/usb.c
	}		
	case USB_LED_OFF:
	{
		int i,iusbport=0;
		extern int USB_PORT3_OFF;
		extern int USB_PORT4_OFF;		
	
		copy_from_user(&iusbport,(int *)arg,sizeof(int));	
		
		switch(iusbport)
		{
			case 3:
					USB_PORT3_OFF = 1;	
					break;
			case 4:
					USB_PORT4_OFF = 1;				
					break;
		}		
		
		USB_Set_LED();  //drivers/usb/storage/usb.c
	}		
#endif
#if CONFIG_NK_URL_TMUFE_FILTER
	case SIOSENDURLFILTERSETTING:
	{
		copy_from_user(&url_filter_setting, (url_filter_setting_t *)arg, sizeof(url_filter_setting_t));
		break;
	}
	case SIORECVURLFILTERSETTING:
	{
		copy_to_user((url_filter_setting_t *)arg, &url_filter_setting, sizeof(url_filter_setting_t));
		break;
	}
	case SIOSENDURLFILTERSTATICS:
	{
		copy_from_user(&url_filter_statics, (url_filter_statics_t *)arg, sizeof(url_filter_statics_t));
		break;
	}
	case SIORECVURLFILTERSTATICS:
	{
		copy_to_user((url_filter_statics_t *)arg, &url_filter_statics, sizeof(url_filter_statics_t));
		break;
	}
	case SIOSENDURLFILTERLIC:
	{
		copy_from_user(&url_filter_lic, (url_filter_lic_t *)arg, sizeof(url_filter_lic_t));
		break;
	}
	case SIORECVURLFILTERLIC:
	{
		copy_to_user(&url_filter_lic, (url_filter_lic_t *)arg, sizeof(url_filter_lic_t));
		break;
	}
/*
	case SIORESETURLFILTER:
	{
		if(restart_url_filter_mod)
			restart_url_filter_mod();
		break;
	}
*/
	case SIOURLFILTERINIT:
	{
		url_filter_chardev_init();
		break;
	}
	case SIOURLFILTERDEBUG:
	{
		copy_from_user(&url_filter_debug, (url_filter_debug_t *)arg, sizeof(url_filter_debug_t));
		break;
	}
#endif

    default:
      ret = -EBADRQC;
   };
   return ret;
}

/*********************************************/
/*
 * Module initialisation and cleanup follow...
 */
//#if CONFIG_NK_IPFILTER_SUPPORT_SORTING
#if 1
int init_ioctl()
{
	int count;

#ifdef CONFIG_NK_DYNAMIC_PORT_NUM
// 	printk(KERN_EMERG "Debug %s #%d %s: useSwitch1450[%d]/useSwitch1100[%d], CONFIG_NK_NUM_LAN[%d], CONFIG_NK_NUM_WAN[%d], CONFIG_NK_NUM_DMZ[%d]\n", __FILE__, __LINE__, __func__, useSwitch1450, useSwitch1100, CONFIG_NK_NUM_LAN, CONFIG_NK_NUM_WAN, CONFIG_NK_NUM_DMZ);
#endif

	/* Register the control device, /dev/url */

	printk(KERN_EMERG "nk: Register the control device, /dev/%s \n", URL_NAME);
	dev = MKDEV(URL_MAJOR, 0);

	/* Attempt to register the URL control device */
	if((err=register_chrdev_region(dev, 1, URL_NAME)) != 0)
	//if ((major = register_chrdev(URL_MAJOR, URL_NAME, &url_fops)) < 0) 
	{
			printk(KERN_EMERG "error: register_chrdev_region() #%d\n", err);
			return err;
	}

	my_cdev = cdev_alloc();
	my_cdev->owner = THIS_MODULE;
	my_cdev->ops = &url_fops;
	if((err=cdev_add(my_cdev, dev, 1)) != 0)
	{
		printk(KERN_EMERG "error: cdev_add() #%d\n", err);
		return err;
	}

/*2007/12/14 trenhcne : support netbios broadcast*/
#ifdef CONFIG_NK_IPSEC_NETBIOS_BC
	//init ipsec netbios broadcast data head and lock
	for( count=0; count < BIOSHASHMAX; count++) {
		nat_ipsec_bc_ip_list[count].start = 0;
		spin_lock_init( &(nat_ipsec_bc_ip_list[count].BiosBroadLock) );
	}	
#endif

#ifdef CONFIG_NK_IPSEC_SPLITDNS
	//---->20100106 trenchen : support split dns
	ipsec_split_dns_list.start = 0;
	spin_lock_init( &(ipsec_split_dns_list.SplitdnsLock) );	
	//<-----
#endif

	#if CONFIG_NK_IPFILTER_SUPPORT_SORTING
	//update_delta_timer_set();
	#endif
	#ifdef CONFIG_SUPPORT_IP_MAC_BINDING
	bzero(&nk_ip_mac_learn_list[0], sizeof(ip_mac_bind_entry_t *)*IP_MAC_ENTRY_HASH_SIZE);
	bzero(&nk_ip_mac_bind_entry_list[0], sizeof(ip_mac_bind_entry_t *)*IP_MAC_ENTRY_HASH_SIZE);
	//memcpy(&nk_ip_mac_learn_list[0], 0, sizeof(ip_mac_bind_entry_t *)*16);
	//printk(KERN_EMERG  "reset nk_ip_mac_learn_list\n");
	#endif
	//printk(KERN_EMERG "finished init_ioctl===================================\n");
	return 0;
}

#endif


#ifdef CONFIG_SUPPORT_IP_MAC_BINDING
int nk_ip_mac_binding_check(struct sk_buff *skb, u_int32_t ip)
{

    u_int8_t *src_mac, *bind_mac, idx;
    int ip_match, on_list;
    int i;
    voice_message_t vm;
    char vmac[60], tmp_ip[120];
    u32 voice_ip;
	//printk(KERN_EMERG "nk_ip_mac_binding_check\n");
//Fix PPTP login client can not go out when IP&Mac Binding is enabled problem.
/*    //if(!on_list )
    {
        pptp_ip_list_t *pptp_ip_list_p;

	for(pptp_ip_list_p = nk_pptp_ip_list; pptp_ip_list_p; pptp_ip_list_p = pptp_ip_list_p->next)
	{
//printk("pptp_ip_list_p->ip[%x], ip[%x]**************\n", pptp_ip_list_p->ip, ip);
		if(ip == pptp_ip_list_p->ip)
			return 0;
	}
    }
*/

   if ( skb->mac.raw == NULL ) return 0;//for upnp incifer 2009/0525

   src_mac = (char *)(skb->mac.raw)+6;

    //idx = 0;
    idx = src_mac[5]&0xff;

  //printk(KERN_EMERG "input:%u.%u.%u.%u, %x-%x-%x-%x-%x-%x\n", NIPQUAD(ip),src_mac[0],src_mac[1],src_mac[2],src_mac[3],src_mac[4],src_mac[5]);

    // learn ip and mac.
    ip_mac_bind_entry = nk_ip_mac_learn_list[idx];

    for( ; ip_mac_bind_entry; ip_mac_bind_entry = ip_mac_bind_entry->next)
    {
	bind_mac = &(ip_mac_bind_entry->mac[0]);
	//printk(KERN_EMERG "list:mac[%x-%x-%x-%x-%x-%x], ip_mac_bind_entry->ip = %u.%u.%u.%u\n",bind_mac[0], bind_mac[1], bind_mac[2], bind_mac[3], bind_mac[4], bind_mac[5],
	//NIPQUAD(ip_mac_bind_entry->ip));
	if((bind_mac[5]==src_mac[5]) && (bind_mac[4]==src_mac[4]) &&
	   (bind_mac[3]==src_mac[3]) && (bind_mac[2]==src_mac[2]) &&
	   (bind_mac[1]==src_mac[1]) && (*bind_mac==*src_mac)    )
	{
	    if(ip_mac_bind_entry->ip != ip)
	    {
	      // old mac, update ip.
              ip_mac_bind_entry->ip = ip;
	      //printk(KERN_EMERG "learn ip_mac : update ip\n");
	      //if(nk_fw_options.arp_spoof_protect)
	        //  nk_set_static_arp(ip_mac_bind_entry);
	    }
	    break;
	}
//printk(KERN_EMERG "check bind_mac[%x-%x-%x-%x-%x-%x]\n",bind_mac[0], bind_mac[1], bind_mac[2], bind_mac[3], bind_mac[4], bind_mac[5]);
    }
#if 1
    // new mac, add mac and ip.
    // if mac is not on binding list, and block_not_on_list and block_wrong_ip
    // are not enabled, do learn it.
    if(ip_mac_bind_entry==NULL && !nk_fw_opts.nk_fw_block_wrong_ip && !nk_fw_opts.nk_fw_block_not_on_list)
//if(ip_mac_bind_entry==NULL)
    {
    	/* if learn entry cnt > 511 ,then free learn entry */
    	if(ip_mac_learn_entry_cnt > 1024)
	{
		for(i=0; i<IP_MAC_ENTRY_HASH_SIZE; i++)
		{
			for(;ip_mac_bind_entry = nk_ip_mac_learn_list[i];)
			{
				nk_ip_mac_learn_list[i] = ip_mac_bind_entry->next;
				kfree(ip_mac_bind_entry);
			}
		}
		ip_mac_number = 0;
		ip_mac_learn_entry_cnt = 0;
	}
	//-->
        ip_mac_bind_entry = (ip_mac_bind_entry_t *)kmalloc(sizeof(ip_mac_bind_entry_t),
                            GFP_ATOMIC);
        if(!ip_mac_bind_entry)
            return -1;
        memcpy(&(ip_mac_bind_entry->mac[0]), src_mac, 6);
        ip_mac_bind_entry->ip = ip;

	//if(nk_fw_options.arp_spoof_protect)
	  //  nk_set_static_arp(ip_mac_bind_entry);

	ip_mac_bind_entry->next = nk_ip_mac_learn_list[idx];
        nk_ip_mac_learn_list[idx] = ip_mac_bind_entry;
	ip_mac_number++;
	ip_mac_learn_entry_cnt++;
	printk(KERN_EMERG "add new: cnt[%d] learn mac[%x-%x-%x-%x-%x-%x], ip[%u.%u.%u.%u], idx[%x]\n",ip_mac_learn_entry_cnt,src_mac[0], src_mac[1], src_mac[2], src_mac[3], src_mac[4], src_mac[5], NIPQUAD(ip), idx);
    }

    //check ip and mac.
    ip_mac_learn_entry = nk_ip_mac_bind_entry_list[idx];
    ip_match = 0;
    on_list = 0;
    for( ; ip_mac_learn_entry; )
    {
        bind_mac = &(ip_mac_learn_entry->mac[0]);
	if((bind_mac[5]==src_mac[5]) && (bind_mac[4]==src_mac[4]) &&
	   (bind_mac[3]==src_mac[3]) && (bind_mac[2]==src_mac[2]) &&
	   (bind_mac[1]==src_mac[1]) && (*bind_mac==*src_mac)    )
	{
	    on_list = 1;
	    voice_ip = ip_mac_learn_entry->ip;
	    if((ip_mac_learn_entry->ip==0) || (ip_mac_learn_entry->ip == ip))
	        ip_match = 1;
	    break;
	}
//printk(KERN_EMERG "check bind_mac[%x-%x-%x-%x-%x-%x]\n",bind_mac[0], bind_mac[1], bind_mac[2], bind_mac[3], bind_mac[4], bind_mac[5]);
	ip_mac_learn_entry = ip_mac_learn_entry->next;
    }
//printk(KERN_EMERG "src_mac[%x-%x-%x-%x-%x-%x], idx[%x], on_list[%d], ip_match[%d]\n",src_mac[0], src_mac[1], src_mac[2], src_mac[3], src_mac[4], src_mac[5], idx, on_list, ip_match);


#endif

/* Arp Spoof Protect:new session set static arp incifer 2006/08/18 */
#ifdef CONFIG_SUPPORT_ARP_SPOOF_PROTECT
    // If this PC is not on bind list, add to static ARP.
//2007/11/14 dont set static arp
// 	if(!on_list && nk_send_arp_enable == 1 && ip_mac_bind_entry)
// 	{
// 		int num;
// 		u_int8_t mac[6];
// 		struct in_addr aaa;
// 
// 		aaa.s_addr = ip_mac_bind_entry->ip;
// 		memcpy(mac,ip_mac_bind_entry->mac,sizeof(u_int8_t)*6);
// 		num =ip_mac_bind_entry->number;
// 		nk_set_static_arp(ip_mac_bind_entry);
// 		//printk(KERN_EMERG "learn new: set static arp:ip[%u.%u.%u.%u],mac[%02x-%02x-%02x-%02x-%02x-%02x],num[%d]\n",NIPQUAD(aaa.s_addr),mac[0],mac[1],mac[2],mac[3],mac[4],mac[5],num);
// 	}
#endif
//printk(KERN_EMERG "VER 1.01 nk_fw_block_not_on_list=%d,on_list=%d\n",nk_fw_opts.nk_fw_block_not_on_list,on_list);
//printk(KERN_EMERG "vs.enable=%d,vs.block_not_on_list_check=%d,vs.block_not_on_list_time=%d\n",vs.enable,vs.block_not_on_list_check,vs.block_not_on_list_time);
    if(nk_fw_opts.nk_fw_block_not_on_list && !on_list )
    {
	if( (vs.enable==vs.block_not_on_list_check==1)&& (vs.block_not_on_list_time>0) )
	{
		sprintf(vmac, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
			(src_mac[0]&0xf0)>>4,(src_mac[0]&0x0f),
			(src_mac[1]&0xf0)>>4,(src_mac[1]&0x0f),
			(src_mac[2]&0xf0)>>4,(src_mac[2]&0x0f),
			(src_mac[3]&0xf0)>>4,(src_mac[3]&0x0f),
			(src_mac[4]&0xf0)>>4,(src_mac[4]&0x0f),
			(src_mac[5]&0xf0)>>4,(src_mac[5]&0x0f));

//		ip2vmsg(ip);
		if(vs.block_wrong_ip_add_mac == 1)
		{
			if(vs.block_wrong_ip_add_ip == 1)
			sprintf(vm.message, "10000%d%s%s%s%sff", vs.block_not_on_list_time, NEW_HOST, vmac, IP_ADDRESS, ip2vmsg(ip));
			else
			sprintf(vm.message, "10000%d%s%sff", vs.block_not_on_list_time, NEW_HOST, vmac);
		}
		else
		{
			if(vs.block_wrong_ip_add_ip == 1)
			sprintf(vm.message, "10000%d%s%s%sff", vs.block_not_on_list_time, LANADDPC, IP_ADDRESS, ip2vmsg(ip));
			else
			sprintf(vm.message, "10000%d%sff", vs.block_not_on_list_time, LANADDPC);
		}
		vs.block_not_on_list_period=0;
		//printk("msg=%s\n", vm.message);
//printk(KERN_EMERG "vm.message=%s\n",vm.message);
		nk_voice_alert_queue(vm.message, LOW_PRI, vs.block_not_on_list_period);
	}
	return -1;
    }
//printk(KERN_EMERG "VER 1.01 nk_fw_block_wrong_ip=%d,on_list=%d\n",nk_fw_opts.nk_fw_block_wrong_ip,on_list);
//printk(KERN_EMERG "vs.enable=%d,vs.block_wrong_ip_check=%d,vs.block_wrong_ip_time=%d\n",vs.enable,vs.block_wrong_ip_check,vs.block_wrong_ip_time);
#if 1
	if(nk_fw_opts.nk_fw_block_wrong_ip)
	{
		if(on_list && !ip_match)
		{
#if 1
			if( (vs.enable==vs.block_wrong_ip_check==1)&& (vs.block_wrong_ip_time>0) )
			{
#if 1
				sprintf(vmac, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
					(src_mac[0]&0xf0)>>4,(src_mac[0]&0x0f),
					(src_mac[1]&0xf0)>>4,(src_mac[1]&0x0f),
					(src_mac[2]&0xf0)>>4,(src_mac[2]&0x0f),
					(src_mac[3]&0xf0)>>4,(src_mac[3]&0x0f),
					(src_mac[4]&0xf0)>>4,(src_mac[4]&0x0f),
					(src_mac[5]&0xf0)>>4,(src_mac[5]&0x0f));
			
#endif	
#if 1
//            ip2vmsg(voice_ip);
//	    sprintf(tmp_ip, "%s", ip_msg);
				sprintf(tmp_ip, "%s", ip2vmsg(voice_ip));
#endif	
				//printk("tmp_ip=%s\n", tmp_ip);
//	    sprintf(tmp_ip, "%s", "010001");

#if 1
				if(vs.block_wrong_ip_mac==1)
					{
						if(vs.block_wrong_ip_ip == 1)
					sprintf(vm.message, "10000%d%s%s%s%sff", vs.block_wrong_ip_time, WRONG_IP, vmac, IP_ADDRESS, tmp_ip);
						else
						sprintf(vm.message, "10000%d%s%sff", vs.block_wrong_ip_time, WRONG_IP, vmac);
					}
#endif
#if 1	
				else
					{
						if(vs.block_wrong_ip_ip == 1)
					sprintf(vm.message, "10000%d%s%s%sff", vs.block_wrong_ip_time, LANCHANGEIP, IP_ADDRESS, tmp_ip);
						else
						sprintf(vm.message, "10000%d%sff", vs.block_wrong_ip_time, LANCHANGEIP);
					}
#endif
				vs.block_wrong_ip_period=0;
				//printk("msg=%s\n", vm.message);vmac
//printk(KERN_EMERG "nk_fw_block_wrong_ip vm.message=%s\n",vm.message);
				nk_voice_alert_queue(vm.message, LOW_PRI, vs.block_wrong_ip_period);
			}
#endif
		 return -1;
		}
	}
#endif
    return 0;
}
#endif

/* arp spoof protect incifer 2006/06/28 */
#ifdef CONFIG_SUPPORT_ARP_SPOOF_PROTECT
void nk_update_arp_protect(void)
{
	struct net_device *odev;

	//printk(KERN_EMERG "nk_update_arp_protect\n");

	if(nk_send_arp_enable == 0)
	{
		//printk(KERN_EMERG "nk_send_arp_enable = 0\n");
	}
	else
	{
		//printk(KERN_EMERG "nk_send_arp_enable != 0\n");
		if(!strncmp(nk_send_arp_fw.dev_name, "eth", 3))
		{
			odev = __dev_get_by_name(nk_send_arp_fw.dev_name);
			//printk(KERN_EMERG "odev=%s\n", nk_send_arp_fw.dev_name);
			arp_send(2, 0x0806,nk_send_arp_fw.dst_ip,odev,nk_send_arp_fw.src_ip,NULL,NULL,NULL);
		}
	}

	update_arp_timer_set();

}
void update_arp_timer_set(void)
{
	update_arp_timer.expires = jiffies + 1 * HZ/nk_send_arp_fw.arp_num;
	add_timer(&update_arp_timer);
}
void del_arp_timer(void)
{
	//printk(KERN_EMERG "del_arp_timer\n");

	del_timer(&update_arp_timer);
}
static void nk_set_static_arp(ip_mac_bind_entry_t *entry_t)
{
	struct arpreq req;

	memset((char *) &req, 0, sizeof(req));
	((struct sockaddr_in *) &req.arp_pa)->sin_addr.s_addr = entry_t->ip;
	req.arp_pa.sa_family = AF_INET;

	memcpy((char *) &(req.arp_ha.sa_data), (char *) entry_t->mac, 6);
	req.arp_ha.sa_family = 1;
	
	req.arp_flags = ATF_PERM | ATF_COM;
	
	arp_req_set(&req, NULL);
}
static void nk_del_static_arp(ip_mac_bind_entry_t *entry_t)
{
	struct arpreq req;

	memset((char *) &req, 0, sizeof(req));
	((struct sockaddr_in *) &req.arp_pa)->sin_addr.s_addr = entry_t->ip;
	req.arp_pa.sa_family = AF_INET;

	memcpy((char *) &(req.arp_ha.sa_data), (char *) entry_t->mac, 6);
	req.arp_ha.sa_family = 1;
	
	req.arp_flags = ATF_PERM | ATF_COM;
	
	arp_req_delete(&req, NULL);
}
#endif
//-->


#ifdef CONFIG_NK_SESSION_LIMIT

int nk_session_limit_delete(u_int32_t ip)
{
    int idx;
    session_limit_t *session_limit_p;

    //if session limit is disable, return.
    if(!session_limit_status)
        return 0;

    idx = ip&0x7f; //128

    for(session_limit_p = nk_session_limit[idx];
        session_limit_p;
	session_limit_p = session_limit_p->next)
    {
	if(session_limit_p->ip == ip && session_limit_p->session_cnt>0)
	{
	    session_limit_p->session_cnt--;
	    break;
	}
if(DEBUG) printk(KERN_EMERG "session_limit_p[%x] found\n",session_limit_p->ip);
    }
}

int nk_session_limit_checking(u_int32_t ip)
{
    int idx, i;
    session_limit_t *session_limit_p;
    //struct in_addr src_ip;
    struct sockaddr_in src_ip;
//sin.sin_addr.s_addr
    src_ip.sin_addr.s_addr = ip;

    //if session limit is disable, return.
    if(session_limit_status==0)
        return 0;

    idx = ip&0x7f; //128

    for(session_limit_p = nk_session_limit[idx];
        session_limit_p;
	session_limit_p = session_limit_p->next)
    {
	if(session_limit_p->ip == ip)
	{
if(DEBUG) printk(KERN_EMERG "session_limit_p[%x] found, cnt[%d], timer[%d]\n",session_limit_p->ip,session_limit_p->session_cnt,session_limit_p->block_timer);
	    break;
	}
    }

    // new mac, add mac and ip.
    if(session_limit_p==NULL)
    {
        if(session_limit_entry_cnt > 511)
	{
		for(i=0; i<128; i++)
		{
			for(; session_limit_p = nk_session_limit[i];)
			{
				nk_session_limit[i] = session_limit_p->next;
				kfree(session_limit_p);
			}
		}
		session_limit_entry_cnt = 0;
	}

	session_limit_p = (session_limit_t *)kmalloc(sizeof(session_limit_t), GFP_ATOMIC);
        if(!session_limit_p)
            return -1;
        session_limit_p->ip = ip;
        session_limit_p->session_cnt = 0;
	session_limit_p->block_timer = 0;
        session_limit_p->next = nk_session_limit[idx];
        nk_session_limit[idx] = session_limit_p;
	session_limit_entry_cnt++;
if(DEBUG) printk(KERN_EMERG "session_limit_p[%x] add new\n",session_limit_p->ip);
    }

    //session_limit_p->session_cnt++;

    return 0;
}
#endif


#ifdef CONFIG_NK_SESSION_LIMIT_ENHANCE
static void session_limit_enhance_order(session_limit_enhance_list_t *input)/* Session Limit Enhance:order list incifer 2006/09/03 */
{
    int sidx,eidx,i;
    session_limit_enhance_node_list_t *tmp;
    session_limit_enhance_list_t *tmp2;

    sidx = input->sip & 0xff;
    eidx = input->eip & 0xff;
    /* input add to nk_session_limit_enhance_list */
//     write_lock_bh(&SLE_LOCK);
//     tmp2 = input;
//     tmp2->next = nk_session_limit_enhance_list;
//     nk_session_limit_enhance_list = input;
//     write_unlock_bh(&SLE_LOCK);

    if(input->sip == 0x00 && input->eip == 0x00)/* 0.0.0.0 ~ 0.0.0.0 */
    {
        write_lock_bh(&SLE_ALLDOMAIN_LOCK);
        tmp2 = input;
        tmp2->next = nk_session_limit_enhance_alldomain_list;
        nk_session_limit_enhance_alldomain_list = input;
        write_unlock_bh(&SLE_ALLDOMAIN_LOCK);
        return;
    }
    else if(sidx == 0 && eidx == 0)/* x.x.x.0 ~ x.x.x.0 */
    {
        write_lock_bh(&SLE_CCLASS_LOCK);
        tmp2 = input;
        tmp2->next = nk_session_limit_enhance_cclass_list;
        nk_session_limit_enhance_cclass_list = input;
        write_unlock_bh(&SLE_CCLASS_LOCK);
        return;
    }
    else/* x.x.x.x ~ x.x.x.x */
    {
        write_lock_bh(&SLE_NODE_LOCK);
        for(i=sidx;i<=eidx;i++)
        {
            if(!(node = (session_limit_enhance_node_list_t *)kmalloc(sizeof(session_limit_enhance_node_list_t), GFP_ATOMIC)))
            {
                printk("node cant alloc memery!!!\n\n");
                write_unlock_bh(&SLE_NODE_LOCK);
                return;
            }
            node->source = input;
            node->next = nk_session_limit_enhance_node_list[i];
            nk_session_limit_enhance_node_list[i] = node;
        }
        write_unlock_bh(&SLE_NODE_LOCK);
    }
}
int session_limit_enhance_rule_check(u_int32_t ip,int port, int protocol, int cnt)/* Session Limit Enhance:check list incifer 2006/09/03 */
{
    session_limit_enhance_node_list_t *tmp;
    session_limit_enhance_list_t *tmp2;
    int idx,proto;

    if(DEBUG) printk(KERN_EMERG "~~~~~~~~~~~~~~rule_check:ip=%u.%u.%u.%u port=%d protocol=%d\n", NIPQUAD(ip), port, protocol);

    if(protocol == IPPROTO_TCP)
        proto = 1;
    else if(protocol == IPPROTO_UDP)
        proto = 2;
    else
        proto = 3;
    /*check UDP:53 incifer 2006/09/18 */
    if(proto==2 && port==53)
    {
        //if(DEBUG) printk("proto==2 port==53\n");
	if(DEBUG) printk(KERN_EMERG "session_limit_enhance_rule_check EXCEPTION, ACCEPT\n");
        return 2;
    }
    if(proto==1 && port==67)
    {
	if(DEBUG) printk(KERN_EMERG "session_limit_enhance_rule_check EXCEPTION, ACCEPT\n");
        return 2;
    }

    /* check nk_session_limit_enhance_alldomain_list */
    read_lock_bh(&SLE_ALLDOMAIN_LOCK);
    tmp2 = nk_session_limit_enhance_alldomain_list;
    while(tmp2)
    {
        if(port>=tmp2->sport && port<=tmp2->eport && (proto & tmp2->typeName))
        {
            if(DEBUG) printk(KERN_EMERG "session_limit_enhance_rule_check EXCEPTION, ACCEPT\n");
            read_unlock_bh(&SLE_ALLDOMAIN_LOCK);
            return 1;
        }
        tmp2 = tmp2->next;
    }
    read_unlock_bh(&SLE_ALLDOMAIN_LOCK);

    /* check nk_session_limit_enhance_cclass_list */
    read_lock_bh(&SLE_CCLASS_LOCK);
    tmp2 = nk_session_limit_enhance_cclass_list;
    while(tmp2)
    {
        if(port>=tmp2->sport && port<=tmp2->eport && (proto & tmp2->typeName))
        {
            if((ip & 0xffffff00) == (tmp2->sip & 0xffffff00))
            {
                if(DEBUG) printk(KERN_EMERG "session_limit_enhance_rule_check EXCEPTION, ACCEPT\n");
                read_unlock_bh(&SLE_CCLASS_LOCK);
                return 1;
            }
        }
        tmp2 = tmp2->next;
    }
    read_unlock_bh(&SLE_CCLASS_LOCK);

    /* check nk_session_limit_enhance_node_list */
    idx = ip & 0xff;
    read_lock_bh(&SLE_NODE_LOCK);
    tmp = nk_session_limit_enhance_node_list[idx];
    while(tmp)
    {
        if(port>=tmp->source->sport && port<=tmp->source->eport && (proto & tmp->source->typeName))
        {
            if(ip>=tmp->source->sip && ip<=tmp->source->eip)
            {
		if(tmp->source->max_session==0) 
		{
			//printk(KERN_EMERG "1check exception max session ip=%u.%u.%u.%u, cnt=%d\n", NIPQUAD(ip), cnt);
			read_unlock_bh(&SLE_NODE_LOCK);
			return 1;
		}
		else if(cnt>tmp->source->max_session) 
		{
			//printk(KERN_EMERG "2check exception max session ip=%u.%u.%u.%u, cnt=%d\n", NIPQUAD(ip), cnt);
			read_unlock_bh(&SLE_NODE_LOCK);
			return -1;
		}
                if(DEBUG) printk(KERN_EMERG "session_limit_enhance_rule_check EXCEPTION, ACCEPT\n");
                read_unlock_bh(&SLE_NODE_LOCK);
                return 1;
            }
        }
        tmp = tmp->next;
    }
    read_unlock_bh(&SLE_NODE_LOCK);

    return 0;
}

/*
   2007/3/15, Yami, session limit case 3, clear all session of this ip.
   search conntrack hash table.
*/
static inline int clearsession(const struct ip_conntrack_tuple_hash *i, u_int32_t ip)
{
	struct ip_conntrack *ct;
	ct = tuplehash_to_ctrack(i);
	if ( (ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip == ip) && (ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip!=wan_interface_session[0].lanip) )
		ct->timeout.expires = 0;
	return 0;
}

/*
   2007/3/26, Yami, clear all session of port forwarding host.
   search conntrack hash table.
*/
//static inline int clear_forwarding_session(const struct ip_conntrack_tuple_hash *i, u_int32_t ip, int port)
int clear_forwarding_session(const struct ip_conntrack_tuple_hash *i, u_int32_t ip, int port)
{
	struct ip_conntrack *ct;
	ct = tuplehash_to_ctrack(i);
	if ((ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.ip == ip) && (ntohs(ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.u.all)==port)) {
		//printk(KERN_EMERG "clear_forwarding_session %u.%u.%u.%u -> %u.%u.%u.%u expire=%d\n",
		//NIPQUAD(ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.ip), NIPQUAD(ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.ip),
		//ct->timeout.expires);
		ct->timeout.expires = 0;
	}
	return 0;
}

/*
   2007/3/26, Yami, clear all session of DMZ host.
   search conntrack hash table.
*/
//static inline int cleardmzsession(const struct ip_conntrack_tuple_hash *i, u_int32_t ip)
int cleardmzsession(const struct ip_conntrack_tuple_hash *i, u_int32_t ip)
{
	struct ip_conntrack *ct;
	ct = tuplehash_to_ctrack(i);	
	if (ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.ip == ip) {
		printk(KERN_EMERG "cleardmzsession %u.%u.%u.%u -> %u.%u.%u.%u expire=%d\n",
		NIPQUAD(ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.ip), NIPQUAD(ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.ip),
		ct->timeout.expires);
		ct->timeout.expires = 0;
	}
	return 0;
}

int nk_session_limit_enhance_checking(u_int32_t ip, int port, int protocol)
{
	int idx, i, exp;
	session_limit_t *session_limit_p;
	//struct in_addr src_ip;
	struct sockaddr_in src_ip;
	session_limit_enhance_list_t *tmp;
	
	voice_message_t vm;
	char tmp_ip[100];
	
	src_ip.sin_addr.s_addr = ip;
	
	
	//if session limit is disable, return.
	//printk(KERN_EMERG "session_limit_status=%d\n", session_limit_status);
	if(session_limit_status==0)
	{
		if(DEBUG) printk(KERN_EMERG "session limit is disable, return.\n");
		return 0;
	}

	//20091214, if use tcp/udp limit, don't check other protocols, Yami
	if( (session_limit_status==4) && (protocol!=IPPROTO_TCP) && (protocol!=IPPROTO_UDP) )
	{
		//printk(KERN_EMERG "tcp/udp limit, don't check other protocols %d\n", protocol);
		return 0;
	}
//20091125 check exception after count session number
    //printk(KERN_EMERG "+++++++++++++ip=%u.%u.%u.%u port=%d protocol=%d\n", NIPQUAD(ip), port, protocol);
//     if(session_limit_enhance_rule_check(ip, port, protocol))
//     {
//         //printk(KERN_EMERG "session_limit_enhance_rule_check EXCEPTION, ACCEPT\n");
// 	//printk(KERN_EMERG "\n");
// 	return 0;
//     }

    idx = ip&0x7f; //128

    for(session_limit_p = nk_session_limit[idx];
        session_limit_p;
        session_limit_p = session_limit_p->next)
    {
        if(session_limit_p->ip == ip)
        {
            //printk(KERN_EMERG "session_limit_p[%u.%u.%u.%u] found, cnt[%d], timer[%d]\n",NIPQUAD(session_limit_p->ip),session_limit_p->session_cnt,session_limit_p->block_timer);
            break;
        }
    }

// nk_session_limit_timer and ip.
    if(session_limit_p==NULL)
    {
        if(session_limit_entry_cnt > 511)
	{
		for(i=0; i<128; i++)
		{
			for(; session_limit_p = nk_session_limit[i];)
			{
				nk_session_limit[i] = session_limit_p->next;
				kfree(session_limit_p);
			}
		}
		session_limit_entry_cnt = 0;
	}

	session_limit_p = (session_limit_t *)kmalloc(sizeof(session_limit_t), GFP_ATOMIC);

        if(!session_limit_p)
            return -1;
        session_limit_p->ip = ip;
        session_limit_p->session_cnt = 0;
	session_limit_p->tcp_session_cnt = 0;
	session_limit_p->udp_session_cnt = 0;
        session_limit_p->block_timer = 0;
        session_limit_p->next = nk_session_limit[idx];
        nk_session_limit[idx] = session_limit_p;
	session_limit_entry_cnt++;
        if(DEBUG) printk(KERN_EMERG "Add new: session_limit_p[%u.%u.%u.%u] cnt[%d]\n",NIPQUAD(session_limit_p->ip), session_limit_p->session_cnt);
    }

//check exception 20091125
	exp = session_limit_enhance_rule_check(ip, port, protocol, session_limit_p->session_cnt);
	if(exp>=1) //1,2 2==udp 53 & 67
	{
		//printk(KERN_EMERG "session_limit_enhance_rule_check EXCEPTION, ACCEPT\n");
		//printk(KERN_EMERG "\n");
		return 0;
	}
	else if(exp==-1)
		return -1;


//if(DEBUG) printk(KERN_EMERG "Add new ok session_limit_p!\n");	
//    session_limit_p->session_cnt++;
    //printk(KERN_EMERG "update session cnt: session_limit_p[%u.%u.%u.%u] cnt[%d]\n", NIPQUAD(session_limit_p->ip), session_limit_p->session_cnt);	
#if 1
    switch(session_limit_status)
    {
        //printk(KERN_EMERG "start switch(session_limit_status %d)\n", session_limit_status);
	case 1: //single IP can not execeed XX session.
	    //if(DEBUG) printk(KERN_EMERG "Case 1\n");
	    if(DEBUG) printk(KERN_EMERG "session_limit_p[%u.%u.%u.%u] cnt[%d] max[%d]\n",NIPQUAD(session_limit_p->ip), session_limit_p->session_cnt, session_limit_max_session1);
	    if ( (session_limit_max_session1>0) && (session_limit_p->session_cnt > session_limit_max_session1) )
	    {
	        //if(session_limit_p->session_cnt>0)
		//	session_limit_p->session_cnt--;
		if(DEBUG) printk(KERN_EMERG "OUT: BLOCK (IP %u.%u.%u.%u reached the session limitation.)\n", NIPQUAD(session_limit_p->ip));
		//printk(KERN_EMERG "session_limit_p[%u.%u.%u.%u] exece limit\n\n",NIPQUAD(session_limit_p->ip));

		ip2vmsg(ip);
		sprintf(tmp_ip, "%s", ip_msg);
		//printk("tmp_ip=%s\n", tmp_ip);
		if( (vs.enable==vs.session_limit_check==1) && (vs.session_limit_time>0) )
		{
			sprintf(vm.message, "10000%d%s%s%sff", vs.session_limit_time, SESSION_LIMIT, IP_ADDRESS, tmp_ip);
			nk_voice_alert_queue(vm.message, LOW_PRI, vs.session_limit_period);
		}
		return -1;
	    }
	    break;

	case 2: // when single IP execeed XX session, block this IP to add new session for YY minutes.
	    //if(DEBUG) printk(KERN_EMERG "Case 2\n");
	    if(session_limit_p->block_timer>0)
	    {
		//session_limit_p->session_cnt--;
		//printk(KERN_EMERG "session_limit_p[%u.%u.%u.%u] block timer block[%d]\n",NIPQUAD(session_limit_p->ip), session_limit_p->block_timer);
	        return -1;
	    }
	    if ( (session_limit_max_session2>0) && (session_limit_p->session_cnt > session_limit_max_session2) && (session_limit_p->block_timer==0))
	    {
	        //if(session_limit_p->session_cnt>0)
			//session_limit_p->session_cnt--;
		session_limit_p->block_timer = session_limit_block_minute1*60;
		nk_session_limit_timer_set();

		ip2vmsg(ip);
		sprintf(tmp_ip, "%s", ip_msg);
		//printk("tmp_ip=%s\n", tmp_ip);
		if( (vs.enable==vs.session_limit_check==1) && (vs.session_limit_time>0) )
		{
			sprintf(vm.message, "10000%d%s%s%sff", vs.session_limit_time, SESSION_LIMIT, IP_ADDRESS, tmp_ip);
			nk_voice_alert_queue(vm.message, LOW_PRI, vs.session_limit_period);
		}
		return -1;
	    }
	    break;

	case 3: // when single IP execeed XX session, block this IP's all connection for YY minutes.
	    //if(DEBUG) printk(KERN_EMERG "Case 3\n");
	    if(session_limit_p->block_timer>0)
	    {
		session_limit_p->session_cnt=0;
		if(DEBUG) printk(KERN_EMERG "session_limit_p[%u.%u.%u.%u] block timer block, timer=%d, scnt=%d\n",NIPQUAD(session_limit_p->ip), session_limit_p->block_timer, session_limit_p->session_cnt);
	        return -1;
	    }

	    //if(DEBUG) printk(KERN_EMERG "session_limit_p->session_cnt=%d, session_limit_max_session2=%d\n", session_limit_p->session_cnt, session_limit_max_session2);
	    if ( (session_limit_max_session2>0) && (session_limit_p->session_cnt > session_limit_max_session2) && (session_limit_p->block_timer==0))
	    {
		if(DEBUG) printk(KERN_EMERG "Case 3 blocked ...... start timer\n");
		/* set session count = 0*/
		session_limit_p->session_cnt = 0;
		session_limit_p->block_timer = session_limit_block_minute2*60;
		if(DEBUG) printk(KERN_EMERG "!!!!!!!!!!!!%u.%u.%u.%u, timer=%d, scnt=%d\n\n\n", NIPQUAD(session_limit_p->ip), session_limit_p->block_timer, session_limit_p->session_cnt);
		ASSERT_WRITE_LOCK(&ip_conntrack_lock);
		for (i = 0; i < ip_conntrack_htable_size; i++) {

			LIST_FIND(&ip_conntrack_hash[i], clearsession,
			struct ip_conntrack_tuple_hash *, session_limit_p->ip);
		}
		//ASSERT_WRITE_ULOCK(&ip_conntrack_lock);
		//if(DEBUG) printk(KERN_EMERG "End clear session\n");

		/* start block timer */
		nk_session_limit_timer_set();

		ip2vmsg(ip);
		sprintf(tmp_ip, "%s", ip_msg);
		//printk("tmp_ip=%s\n", tmp_ip);
		if( (vs.enable==vs.session_limit_check==1) && (vs.session_limit_time>0) )
		{
			sprintf(vm.message, "10000%d%s%s%sff", vs.session_limit_time, SESSION_LIMIT, IP_ADDRESS, tmp_ip);
			nk_voice_alert_queue(vm.message, LOW_PRI, vs.session_limit_period);
		}

		return -1;
	    }

	    break;
	case 4: //single IP can not execeed XX tcp session & XX tcp session. //20091125
	    //if(DEBUG) printk(KERN_EMERG "Case 1\n");
	    //printk(KERN_EMERG "session_limit_p[%u.%u.%u.%u] cnt[%d] max[%d] tcp_cnt[%d] max_tcp[%d] udp_cnt[%d] max_udp[%d]\n",NIPQUAD(session_limit_p->ip), session_limit_p->session_cnt, session_limit_max_session1, session_limit_p->tcp_session_cnt, session_limit_max_tcp, session_limit_p->udp_session_cnt, session_limit_max_udp);
	    //printk(KERN_EMERG "update session cnt: session_limit_p[%u.%u.%u.%u] cnt[%d] tcp[%d] udp[%d]\n\n", NIPQUAD(session_limit_p->ip), session_limit_p->session_cnt, session_limit_p->tcp_session_cnt, session_limit_p->udp_session_cnt);
	    //20091125 check tcp sessions
		if ( (protocol==IPPROTO_TCP) && (session_limit_max_tcp>0) && (session_limit_p->tcp_session_cnt >= session_limit_max_tcp) )
		{
	
			if(DEBUG) printk(KERN_EMERG "OUT: BLOCK (IP %u.%u.%u.%u reached the session limitation.)\n", NIPQUAD(session_limit_p->ip));
			//printk(KERN_EMERG "tcp session_limit_p[%u.%u.%u.%u] exece limit\n\n",NIPQUAD(session_limit_p->ip));
			return -1;
		}
		//20091125 check udp sessions
		else if ( (protocol==IPPROTO_UDP) && (session_limit_max_udp>0) && (session_limit_p->udp_session_cnt >= session_limit_max_udp) )
		{
	
			if(DEBUG) printk(KERN_EMERG "OUT: BLOCK (IP %u.%u.%u.%u reached the session limitation.)\n", NIPQUAD(session_limit_p->ip));
			//printk(KERN_EMERG "udp ession_limit_p[%u.%u.%u.%u] exece limit\n\n",NIPQUAD(session_limit_p->ip));
			return -1;
		}
		return 0;	
	    	break;
        default:
	    break;
    }
#endif
    return 0;
}

int nk_session_limit_enhance_delete(u_int32_t ip, int port, int protocol)
{
    int idx;
    session_limit_t *session_limit_p;

#if 0//20091125, check every session
    //printk(KERN_EMERG "ip=%u.%u.%u.%u port=%d protocol=%d\n", NIPQUAD(ip), port, protocol);
    if(session_limit_enhance_rule_check(ip, port, protocol,0))
    {
        //printk(KERN_EMERG "session_limit_enhance_rule_check EXCEPTION, ACCEPT\n");
	return 0;
    }
#endif

    idx = ip&0x7f; //128
    //printk(KERN_EMERG "ip=%u.%u.%u.%u, idx=%d\n", NIPQUAD(ip), idx);
    for(session_limit_p = nk_session_limit[idx];
        session_limit_p;
	session_limit_p = session_limit_p->next)
    {
	if(session_limit_p->ip == ip && session_limit_p->session_cnt>0)
	//if(session_limit_p->ip==ip)
	{
	    //printk(KERN_EMERG "delete:session_limit_p[%x] found, count=%d\n",session_limit_p->ip, session_limit_p->session_cnt);
		session_limit_p->session_cnt--;
		if( protocol == IPPROTO_TCP )
			session_limit_p->tcp_session_cnt--;
		if( protocol == IPPROTO_UDP )
			session_limit_p->udp_session_cnt--;

	//printk(KERN_EMERG "delete session cnt: session_limit_p[%u.%u.%u.%u] cnt[%d] tcp[%d] udp[%d]\n", NIPQUAD(session_limit_p->ip), session_limit_p->session_cnt, session_limit_p->tcp_session_cnt, session_limit_p->udp_session_cnt);
	    //if(session_limit_p->session_cnt<=0) session_limit_p->session_cnt=0;
	    //printk(KERN_EMERG "delete:session_limit_p[%u.%u.%u.%u] found, count=%d\n\n",NIPQUAD(session_limit_p->ip), session_limit_p->session_cnt);
	    //printk(KERN_EMERG " \n");
	    //test=1;
		break;
	}
	//else
	//	printk(KERN_EMERG "!!! %u.%u.%u.%u delete sesion count fail !!!\n", NIPQUAD(session_limit_p->ip));
    }
}
#endif


#ifdef CONFIG_NK_SESSION_LIMIT
extern session_limit_t *nk_session_limit[128];

void session_limit_timer(void)
{
    int i;
    session_limit_t *session_limit_p;

    for(i=0; i<128; i++)
    {
      for(session_limit_p = nk_session_limit[i];
          session_limit_p;
	  session_limit_p = session_limit_p->next)
      {
	if(session_limit_p->block_timer>0)
	{
	    session_limit_p->block_timer--;
	    //printk(KERN_EMERG "%u.%u.%u.%u block_timer=%d\n", NIPQUAD(session_limit_p->ip), session_limit_p->block_timer);
	    if(session_limit_p->block_timer>0)
	    	nk_session_limit_timer_set();
	}
#if 0
	else
	{
	    if(DEBUG) printk("session_limit_p[%u.%u.%u.%u] block timer expire\n",NIPQUAD(session_limit_p->ip));
	}
#endif
      }
    }
}
void nk_session_limit_timer_set(void)
{
	//if(DEBUG) printk(KERN_EMERG "nk_session_limit_timer_set\n");
	if(&nk_session_limit_timer)
		del_timer(&nk_session_limit_timer);
	nk_session_limit_timer.expires = jiffies + 1*HZ;
	init_timer(&nk_session_limit_timer);
	add_timer(&nk_session_limit_timer);
	//if(DEBUG) printk(KERN_EMERG "Finished nk_session_limit_timer_set\n");
}

#endif

#if 0
int
is_active(struct sk_buff *skb, unsigned int days_match, unsigned int time_start, unsigned int time_stop, unsigned int slimit_scheduler)
{
	const struct ipt_time_info *info;   /* match info for rule */
	struct tm currenttime;                          /* time human readable */
	unsigned int packet_time;
	//struct timeval kerneltimeval;
	time_t packet_local_time;

	if(slimit_scheduler==0) return 1;
	//printk(KERN_EMERG "is_active:days=%d, start=%d, stop=%d\n", days_match, time_start, time_stop);
	/* if kerneltime=1, we don't read the skb->timestamp but kernel time instead */
/*	if (info->kerneltime)
	{
		do_gettimeofday(&kerneltimeval);
		packet_local_time = kerneltimeval.tv_sec;
	}
	else*/
		//packet_local_time = skb->stamp.tv_sec;
		packet_local_time = skb->tstamp.off_sec;

	/* Transform the timestamp of the packet, in a human readable form */
	localtime(&packet_local_time, &currenttime);

	//printk(KERN_EMERG "tm_wday=%d, tm_hour=%d, tm_min=%d\n", currenttime.tm_wday, currenttime.tm_hour, currenttime.tm_min);
	/* check if we match this timestamp, we start by the days... */
	if (!((1 << currenttime.tm_wday) & days_match))
	{
		//session_limit_status=0;
		if(DEBUG) printk(KERN_EMERG "1.is_active=0\n");
		return 0; /* the day doesn't match */
	}

	/* ... check the time now */
	packet_time = (currenttime.tm_hour * 60 * 60) + (currenttime.tm_min * 60) + currenttime.tm_sec;
	//printk(KERN_EMERG "packet_time=%d\n", packet_time);
	if (time_start < time_stop) {
		if ((packet_time < time_start) || (packet_time > time_stop))
		{
			//session_limit_status=0;
			if(DEBUG) printk(KERN_EMERG "2.is_active=0\n");
			return 0;
		}
	} else {
		if ((packet_time < time_start) && (packet_time > time_stop))
		{	
			//session_limit_status=0;
			if(DEBUG) printk(KERN_EMERG "3.is_active=0\n");
			return 0;
		}
	}

	/* here we match ! */
	if(DEBUG) printk(KERN_EMERG "5.is_active=1\n");
	return 1;
}
#endif

void nk_voice_alert_queue(unsigned char* message, int priority, int interval)
{
	alarm_msg_t *am_l, *am_h, **am_cur, *played_msg, *played_msg_p, **played_msg_pp;
	int num;
	//int i=0,j=0;
	if (vs.enable!=1)
		return;
	for (played_msg_pp = &nk_msg_played;
	     played_msg_p=*played_msg_pp;)
	{
		if(DEBUG) printd("mtimer=%d, interval=%d\n", played_msg_p->mtimer, played_msg_p->interval);
		if(played_msg_p->mtimer>=played_msg_p->interval)
		{
			printd("free %s\n", (*played_msg_pp)->msg);
			//played_msg_p = *played_msg_pp;
			*played_msg_pp = played_msg_p->next;
			kfree(played_msg_p);
		}
		else
		{
			played_msg_pp = &(played_msg_p->next);
		}
	}

	//i=0;
	for (played_msg = nk_msg_played; played_msg; played_msg = played_msg->next)
	{
		//i++;
		//printd("%d %s mtimer=%d, interval=%d\n", i, played_msg->msg, played_msg->mtimer, played_msg->interval);
		if((!strcmp(played_msg->msg, message)) && (played_msg->mtimer<played_msg->interval))
		{
			if(DEBUG) printd("\n%s played in %d sec, do nothing!!\n", played_msg->msg, played_msg->mtimer);
			return;
		}
		else if((!strcmp(played_msg->msg, message)) && (played_msg->mtimer>=played_msg->interval))
			played_msg->mtimer = 0;
	}


	for (am_l = nk_msg_low; am_l; am_l = am_l->next)
	{
		if(!strcmp(am_l->msg, message))
		{
			if(DEBUG) printd("low. duplicate msg %s, do nothing!!\n", am_l->msg);
			return;
		}
	}
	for (am_h = nk_msg_high; am_h; am_h = am_h->next)
	{
		if(!strcmp(am_h->msg, message))
		{
			if(DEBUG) printd("high. duplicate msg %s, do nothing!!\n", am_h->msg);
			return;
		}
	}

	if(interval==0) interval=2;

	if( (priority==LOW_PRI) )
	{
		num = get_queue_number(LOW_PRI);
		if(num>=50) return;

			if(DEBUG) printd("low: Add %s, interval=%d min\n", message, interval);
		am_l = (alarm_msg_t *)kmalloc(sizeof(alarm_msg_t), GFP_ATOMIC);
		if(am_l)
		{
			bzero((char *)am_l, sizeof(alarm_msg_t));
			am_l->next = NULL;
			strcpy(am_l->msg, message);
			am_l->interval = interval*60;
			for(am_cur=&nk_msg_low; *am_cur; am_cur=&(*am_cur)->next) ;
				*am_cur = am_l;
		}
	}
	else if( (priority==HIGH_PRI) )
	{
		if(DEBUG) printd("high: Add %s, interval=%d min\n",message, interval);
		am_h = (alarm_msg_t *)kmalloc(sizeof(alarm_msg_t), GFP_ATOMIC);
		if(am_h)
		{
			bzero((char *)am_h, sizeof(alarm_msg_t));
			am_h->next = NULL;
   			strcpy(am_h->msg, message);
			am_h->interval = interval*60;
			for(am_cur=&nk_msg_high; *am_cur; am_cur=&(*am_cur)->next) ;
				*am_cur = am_h;
		}
	}

}
int get_queue_number(int pri)
{
	int num=0;
	alarm_msg_t *am_l, *am_h, *played_msg;

	if(pri==LOW_PRI)
	{
		for(am_l=nk_msg_low;am_l;am_l=am_l->next)
			num++;
		//printd("low number=%d\n", num);
		return num;
	}
	else if(pri==HIGH_PRI)
	{
		for(am_h=nk_msg_high;am_h;am_h=am_h->next)
			num++;
		//printd("high number=%d\n", num);
		return num;

	}
	return 0;
}
char ip_msg[120];
char* ip2vmsg(u32 ip)
{

	int i;
	ip_msg[0]='\0';

	    for(i=3;i>=0;i--)
	    {
	    	int ip3,ip2,ip1;
		char temp_vm[40], temp_vm1[40], temp_vm2[40];
		u32 temp_ip;
		temp_vm[0]='\0';
		temp_vm1[0]='\0';
		temp_vm2[0]='\0';
	    	temp_ip=(ip>>(i*8))&(0x000000ff);

		ip1=temp_ip%10;
		ip2=(temp_ip%100)/10;
		ip3=temp_ip/100;

		if(ip3>0)
			sprintf(temp_vm,"%02d",ip3);
		else
			temp_vm[0]='\0';


		if(ip2>0)
		{
			sprintf(temp_vm1,"%s%02d",temp_vm,ip2);
		}
		else
		{
			if(ip3!=0)
				sprintf(temp_vm1,"%s%02d",temp_vm,0);
			else
				temp_vm1[0]='\0';
		}

		/*ip1*/
		if(i>0) sprintf(temp_vm2,"%s%02d61",temp_vm1,ip1);
		else sprintf(temp_vm2,"%s%02d",temp_vm1,ip1);

		//printd("tmp_vm=%s\n", temp_vm);
		strcat(ip_msg,temp_vm2);
	    }

	    //printd("IP = %s\n",ip_msg);
	    return (ip_msg);
}

/* support IpBalance Struct Aging -- incifer 2009/01 */
#ifdef CONFIG_IPBALANCE_STRUCT_AGING
void ipbalance_struct_check(void)
{
	static int cnt=0;
	int i=0, index;
	session_limit_t *session_limit_p=NULL;
	struct IpBalanceInfo **entry_tmp=NULL;
	struct IpBalanceInfo *trash=0;

// 	printk(KERN_EMERG "2----------------session_idx_start[%d]--------------------------------------------------------------------\n", session_idx_start);
	for(i=session_idx_start*X_BUCKET;i<(session_idx_start+1)*X_BUCKET;i++)
	{
		for(session_limit_p=nk_session_limit[i];
			session_limit_p;
			session_limit_p=session_limit_p->next)
		{
			if(!session_limit_p->session_cnt)/* search session cnt == 0 */
			{
				index = session_limit_p->ip & 0xff;
// 				printk(KERN_EMERG "\nidx[%03d]  IP:%u.%u.%u.%u, Index:%d has %d Session(s) -------- \n", i, NIPQUAD(session_limit_p->ip), index, session_limit_p->session_cnt);

				entry_tmp = &(IpBalInfoHead[index].next);
				while(*entry_tmp)
				{
// 					printk(KERN_EMERG "IpBalanceInfo IP:%u.%u.%u.%u, Dev:%s, table:%u, lastuse:%u\n",
// 							NIPQUAD((*entry_tmp)->srcip), ((*entry_tmp)->out)->name, (*entry_tmp)->table, (*entry_tmp)->lastuse);
					if((*entry_tmp)->srcip == session_limit_p->ip)/* search entry_tmp->srcip == session_limit_p->ip */
					{
// 						printk(KERN_EMERG "\tDelete IP:%u.%u.%u.%u Dev:%s Success\n!!!", NIPQUAD((*entry_tmp)->srcip), ((*entry_tmp)->out)->name);
						trash=(*entry_tmp);
						IpBalanceDel(index, entry_tmp, trash);
						IpSecNum--;
						kfree( trash );
						trash = 0;
						//showBalanceData();/* for debug */
					}
					else
					{
						entry_tmp = &((*entry_tmp)->next);
					}
				}/** endof while(*entry_tmp) **/
			}/** endof if(!session_limit_p->session_cnt) **/
		}/** endof for(session_limit_p=nk_session_limit[i];...  **/
	}/** endof for(i=session_idx_start*X_BUCKET;i<(session_idx_start+1)*X_BUCKET;i++) **/
	session_idx_start++;
	if(session_idx_start >= Y_PORTION)
		session_idx_start = 0;

	//printk(KERN_EMERG "%s: proc_ipbalance_struct_aging_period_time=%d\n", __func__, proc_ipbalance_struct_aging_period_time);
	set_ipbalance_struct_aging_timer(proc_ipbalance_struct_aging_period_time);
}

void set_ipbalance_struct_aging_timer(int t)
{
	ipbalance_struct_aging_timer.expires = jiffies + t * HZ;
	add_timer(&ipbalance_struct_aging_timer);
}

void del_ipbalance_struct_aging_timer(void)
{
	del_timer(&ipbalance_struct_aging_timer);
}
#endif

void quick_sort(int left, int right, ip_session_t *array)    // [left, right]  
{
	ip_session_t temp;
	if (left < right)
	{
		// divide (partition)
		int pivot = (array[(left+right)/2].down_bytes + array[(left+right)/2].up_bytes);
		int i = left - 1, j = right + 1;
		while (i < j)
		{
			do ++i; while ((array[i].down_bytes+ array[i].up_bytes) > pivot);
			do --j; while ((array[j].down_bytes + array[j].up_bytes) < pivot);
			if (i < j) //swap(i, j, array);
			{
				temp = array[i];
				array[i] = array[j];
				array[j] = temp;
			}
		}
		// then conquer
		quick_sort(left, i-1, array);
		quick_sort(j+1, right, array);
	}
}
