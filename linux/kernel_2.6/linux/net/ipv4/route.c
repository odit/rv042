/*
 * INET		An implementation of the TCP/IP protocol suite for the LINUX
 *		operating system.  INET is implemented using the  BSD Socket
 *		interface as the means of communication with the user level.
 *
 *		ROUTE - implementation of the IP router.
 *
 * Version:	$Id: route.c 6053 2010-08-03 07:54:55Z david.kao $
 *
 * Authors:	Ross Biro
 *		Fred N. van Kempen, <waltje@uWalt.NL.Mugnet.ORG>
 *		Alan Cox, <gw4pts@gw4pts.ampr.org>
 *		Linus Torvalds, <Linus.Torvalds@helsinki.fi>
 *		Alexey Kuznetsov, <kuznet@ms2.inr.ac.ru>
 *
 * Fixes:
 *		Alan Cox	:	Verify area fixes.
 *		Alan Cox	:	cli() protects routing changes
 *		Rui Oliveira	:	ICMP routing table updates
 *		(rco@di.uminho.pt)	Routing table insertion and update
 *		Linus Torvalds	:	Rewrote bits to be sensible
 *		Alan Cox	:	Added BSD route gw semantics
 *		Alan Cox	:	Super /proc >4K 
 *		Alan Cox	:	MTU in route table
 *		Alan Cox	: 	MSS actually. Also added the window
 *					clamper.
 *		Sam Lantinga	:	Fixed route matching in rt_del()
 *		Alan Cox	:	Routing cache support.
 *		Alan Cox	:	Removed compatibility cruft.
 *		Alan Cox	:	RTF_REJECT support.
 *		Alan Cox	:	TCP irtt support.
 *		Jonathan Naylor	:	Added Metric support.
 *	Miquel van Smoorenburg	:	BSD API fixes.
 *	Miquel van Smoorenburg	:	Metrics.
 *		Alan Cox	:	Use __u32 properly
 *		Alan Cox	:	Aligned routing errors more closely with BSD
 *					our system is still very different.
 *		Alan Cox	:	Faster /proc handling
 *	Alexey Kuznetsov	:	Massive rework to support tree based routing,
 *					routing caches and better behaviour.
 *		
 *		Olaf Erb	:	irtt wasn't being copied right.
 *		Bjorn Ekwall	:	Kerneld route support.
 *		Alan Cox	:	Multicast fixed (I hope)
 * 		Pavel Krauz	:	Limited broadcast fixed
 *		Mike McLagan	:	Routing by source
 *	Alexey Kuznetsov	:	End of old history. Split to fib.c and
 *					route.c and rewritten from scratch.
 *		Andi Kleen	:	Load-limit warning messages.
 *	Vitaly E. Lavrov	:	Transparent proxy revived after year coma.
 *	Vitaly E. Lavrov	:	Race condition in ip_route_input_slow.
 *	Tobias Ringstrom	:	Uninitialized res.type in ip_route_output_slow.
 *	Vladimir V. Ivanov	:	IP rule info (flowid) is really useful.
 *		Marc Boucher	:	routing by fwmark
 *	Robert Olsson		:	Added rt_cache statistics
 *	Arnaldo C. Melo		:	Convert proc stuff to seq_file
 *	Eric Dumazet		:	hashed spinlocks and rt_check_expire() fixes.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 */

#include <linux/config.h>
#include <linux/module.h>
#include <asm/uaccess.h>
#include <asm/system.h>
#include <linux/bitops.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/bootmem.h>
#include <linux/string.h>
#include <linux/socket.h>
#include <linux/sockios.h>
#include <linux/errno.h>
#include <linux/in.h>
#include <linux/inet.h>
#include <linux/netdevice.h>
#include <linux/proc_fs.h>
#include <linux/init.h>
#include <linux/skbuff.h>
#include <linux/rtnetlink.h>
#include <linux/inetdevice.h>
#include <linux/igmp.h>
#include <linux/pkt_sched.h>
#include <linux/mroute.h>
#include <linux/netfilter_ipv4.h>
#include <linux/random.h>
#include <linux/jhash.h>
#include <linux/rcupdate.h>
#include <linux/times.h>
#ifdef CONFIG_NK_SB_ENHANCEMENT
#include <linux/route_util.h> /*add for session base enhance by michael lu*/
#endif
#include <net/protocol.h>
#include <net/ip.h>
#include <net/route.h>
#include <net/inetpeer.h>
#include <net/sock.h>
#include <net/ip_fib.h>
#include <net/arp.h>
#include <net/tcp.h>
#include <net/icmp.h>
#include <net/xfrm.h>
#include <net/ip_mp_alg.h>
#ifdef CONFIG_SYSCTL
#include <linux/sysctl.h>
#endif

#include <linux/nkdef.h>

//#define ROUTING_HASH_IMPROVE

#define RT_FL_TOS(oldflp) \
    ((u32)(oldflp->fl4_tos & (IPTOS_RT_MASK | RTO_ONLINK)))

#define IP_MAX_MTU	0xFFF0

#define RT_GC_TIMEOUT (300*HZ)

static int ip_rt_min_delay		= 2 * HZ;
static int ip_rt_max_delay		= 10 * HZ;
static int ip_rt_max_size;
static int ip_rt_gc_timeout		= RT_GC_TIMEOUT;
static int ip_rt_gc_interval		= 60 * HZ;
static int ip_rt_gc_min_interval	= HZ / 2;
static int ip_rt_redirect_number	= 9;
static int ip_rt_redirect_load		= HZ / 50;
static int ip_rt_redirect_silence	= ((HZ / 50) << (9 + 1));
static int ip_rt_error_cost		= HZ;
static int ip_rt_error_burst		= 5 * HZ;
static int ip_rt_gc_elasticity		= 8;
static int ip_rt_mtu_expires		= 10 * 60 * HZ;
static int ip_rt_min_pmtu		= 512 + 20 + 20;
static int ip_rt_min_advmss		= 256;
static int ip_rt_secret_interval	= 10 * 60 * HZ;
static unsigned long rt_deadline;

#define RTprint(a...)	printk(KERN_DEBUG a)

static struct timer_list rt_flush_timer;
static struct timer_list rt_periodic_timer;
static struct timer_list rt_secret_timer;
#ifdef CONFIG_NK_PROTO_BINDING
static struct timer_list fire_spec_timer;
#endif
/*
 *	Interface to generic destination cache.
 */

static struct dst_entry *ipv4_dst_check(struct dst_entry *dst, u32 cookie);
static void		 ipv4_dst_destroy(struct dst_entry *dst);
static void		 ipv4_dst_ifdown(struct dst_entry *dst,
					 struct net_device *dev, int how);
static struct dst_entry *ipv4_negative_advice(struct dst_entry *dst);
static void		 ipv4_link_failure(struct sk_buff *skb);
static void		 ip_rt_update_pmtu(struct dst_entry *dst, u32 mtu);
static int rt_garbage_collect(void);

/*2006/11/1 trenchen : support remote manage*/
struct net_device * nk_multipath_select(u32 daddr , u32 saddr , u32 proto , u32 dport , u32 sport , u8 tos);



//2007/3/5 trenchen : support ip base 
#ifdef CONFIG_NK_PROTO_BINDING

#ifdef CONFIG_NK_SB_ENHANCEMENT
#define SB_USE_DST_HASH
#ifdef SB_USE_DST_ARRAY
#define SB_TABLE_NUM 65536
static struct SessionSrcFlowInfo SBSrcInfo[SB_TABLE_NUM];
static struct SessiondstFlowInfo SBDstInfo[SB_TABLE_NUM][SB_TABLE_NUM];
#else
#define SB_TABLE_NUM 256
#ifdef SB_USE_DST_HASH
static struct SessionSrcHead SBInfoHead[SB_TABLE_NUM][SB_TABLE_NUM];
#else
#define SB_TABLE_NUM 65536
static struct SessionSrcHead SBInfoHead[SB_TABLE_NUM];
#endif
#endif
int session_base_flag= 0;
int session_base_flush= 0;
struct SessionDstInfo *DstTableList;
int DstTableSize;
EXPORT_SYMBOL(DstTableList);
EXPORT_SYMBOL(DstTableSize);
struct SessionLockInfo *LockTableList;
int LockTableSize;
EXPORT_SYMBOL(LockTableList);
EXPORT_SYMBOL(LockTableSize);
static int SessionNumData = 25000;//the session base table number
static int SessionNumDeno = 8;//denominator
static int SessionDenoCounter = 1;//denominator counter
static int SessionDenoFlag = 1;//For Changing new/old flush function
static unsigned int SessionSecNum=0;
#define SessionIPMask 0xffff
#define SessionDstMask 0xffff0000
#define SessionDstHashMove 16
#define SB_SEARCH_DST_RANGE_FAIL -1
#define SB_SEARCH_LOCK_RANGE_FAIL -1
#define SB_SEARCH_LOCK_RANGE_SUCCESS 1
#endif

#ifdef CONFIG_NK_IP_BASE

#define TrashPeriod 3600
#define IpNumData 5000

/* support IpBalance Struct Aging -- incifer 2009/01 */
#ifdef CONFIG_IPBALANCE_STRUCT_AGING
unsigned int IpSecNum=0;
#else
static unsigned int IpSecNum=0;
#endif
int ip_base_flag=0;

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
/* support IP balance enhancement: flush cache after 40s -- incifer 2008/11 */
#ifdef CONFIG_IPBALANCE_ENHANCE
/**
	record wan down time, webBoot.c/capture_wan_down_time() -> ethernet.c/CAPTURE_WAN_DOWN_TIME
**/
unsigned long WAN_DOWN_TIME[CONFIG_NK_NUM_WAN+CONFIG_NK_NUM_DMZ];
EXPORT_SYMBOL(WAN_DOWN_TIME);
/**
	flush cache after FLUSH_CACHE_TIME
**/
#define FLUSH_CACHE_TIME 90000
#endif

struct IpBalanceHead {
	struct IpBalanceInfo *next;
	struct IpBalanceInfo **end;
};
/* remove static, */
/* support IpBalance Struct Aging -- incifer 2009/01 */
#ifdef CONFIG_IPBALANCE_STRUCT_AGING
struct IpBalanceHead IpBalInfoHead[256];
#else
static struct IpBalanceHead IpBalInfoHead[256];
#endif

//TT rwlock_t IpBalLock = RW_LOCK_UNLOCKED;
static DEFINE_RWLOCK(IpBalLock);

/* support IpBalance Struct Aging -- incifer 2009/01 */
#ifdef CONFIG_IPBALANCE_STRUCT_AGING
	extern int proc_ipbalance_struct_aging_start_time;
	extern int proc_ipbalance_struct_aging_period_time;
#endif
static int IpBalanceInsert( int index, struct IpBalanceInfo *entry)
{
	if( IpBalInfoHead[index].end == 0 ) //first
		IpBalInfoHead[index].end = &(IpBalInfoHead[index].next);
	
	*(IpBalInfoHead[index].end) = entry;

	IpBalInfoHead[index].end = &(entry->next);

	entry->next = 0;

	return 0;
}

/* support IpBalance Struct Aging -- incifer 2009/01 */
#ifdef CONFIG_IPBALANCE_STRUCT_AGING
int IpBalanceDel( int index, struct IpBalanceInfo **front, struct IpBalanceInfo *entry)
#else
static int IpBalanceDel( int index, struct IpBalanceInfo **front, struct IpBalanceInfo *entry)
#endif
{

	if( entry->next == 0 )  //last
		IpBalInfoHead[index].end = front;
		
	*front = entry->next;
	
	entry->next = 0;
		

	return 0;
}

#if 1
void showBalanceData(void)
{
	int count=0;
	struct IpBalanceInfo *entry;

	for(count=0;count < 256;count++) {
		entry=IpBalInfoHead[count].next;
		while(entry!=0){
			printk(KERN_EMERG "index=[%d] ip=[%x] table[%d] lastuse[%u]\n", count, entry->srcip, entry->table, entry->lastuse);
			entry=entry->next;
		}
	}
}
#endif

static void IpBalanceCreateNew( u32 InIp, struct fib_result *res)
{
	int mask = InIp & 0xff;	
	unsigned long period;
	struct IpBalanceInfo *insert=0;
	struct IpBalanceInfo *trash=0;
	int count=0;
	int old;

	//no found, creat new
	if( IpSecNum >= IpNumData) {
		trash = 0;
		old = -1;
		for( count=0; count<256; count++) {
			if( IpBalInfoHead[count].next != 0 ){
				if( (period = jiffies-(IpBalInfoHead[count].next)->lastuse) < 0 )
					period = (~0 - (IpBalInfoHead[count].next)->lastuse) + jiffies;
				if( period/HZ > TrashPeriod ){
					trash = IpBalInfoHead[count].next;
					IpBalanceDel( count, &(IpBalInfoHead[count].next), trash);
					kfree(trash);
					IpSecNum--;
					trash = 0;
				}else {
					if( old == -1 )
						old = count;
					else {
						if( ((IpBalInfoHead[old].next)->lastuse) > ((IpBalInfoHead[count].next)->lastuse) )
							old = count;
					}
				}
			}
		}

		if( IpSecNum >= IpNumData ) {
			trash = IpBalInfoHead[old].next;
			IpBalanceDel( old, &(IpBalInfoHead[old].next), trash);
			kfree(trash);
			IpSecNum--;
		}
	}

	insert = kmalloc( sizeof(struct IpBalanceInfo), GFP_ATOMIC);
	if( !insert )
		return;

	insert->next = 0;
	insert->srcip = InIp;
	insert->out = (res->fi->fib_nh[res->nh_sel]).nh_dev;
	insert->table = nk_get_table(res);
	insert->lastuse = jiffies;
/* support IP balance enhancement: flush cache after 40s -- incifer 2008/11 */
#ifdef CONFIG_IPBALANCE_ENHANCE
	/**
		record interface
	**/
	if(!strncmp(((res->fi->fib_nh[res->nh_sel]).nh_dev)->name, "eth", 3))
		sscanf(((res->fi->fib_nh[res->nh_sel]).nh_dev)->name, "eth%d", &(insert->inf));
	else if(!strncmp(((res->fi->fib_nh[res->nh_sel]).nh_dev)->name, "ppp", 3))
		sscanf(((res->fi->fib_nh[res->nh_sel]).nh_dev)->name, "ppp%d", &(insert->inf));
	else
		insert->inf = -1;
#endif
	IpBalanceInsert( mask, insert);
	IpSecNum++;
//	showBalanceData();
	
	return;

}

/* support IP balance enhancement: flush cache after 40s -- incifer 2008/11 */
#ifdef CONFIG_IPBALANCE_ENHANCE
/**
	add a new parameter(int *flag), flag:
**/
static struct net_device *SearchIpOut( u32 InIp, struct fib_result *res, int *flag)
#else
static struct net_device *SearchIpOut( u32 InIp, struct fib_result *res)
#endif
{
	u32 mask = InIp & 0xff;
	struct IpBalanceInfo **temp;
	struct IpBalanceInfo *insert=0;
	struct IpBalanceInfo *trash=0;
	struct net_device *ans;
	int count=0;

	temp = &(IpBalInfoHead[mask].next);
	while( *temp ) {
		if( InIp == (*temp)->srcip && nk_get_table(res) == (*temp)->table ) {
			for( count=0; count < res->fi->fib_nhs; count++) {
				if( (res->fi->fib_nh[count]).nh_dev == (*temp)->out ) {
					(*temp)->lastuse = jiffies;
					ans = (*temp)->out;
					insert = *temp;
					if(insert->next != 0){//last don't need update
						IpBalanceDel( mask, temp, insert);
						IpBalanceInsert( mask, insert);
					}
//					printk("get and update index[%x] ans[%s]\n",mask,ans->name);
//					showBalanceData();
					//2007/5/3 trenchen : set the real interface
					res->nh_sel = count; 
					return ans;
				}
			}

			trash = *temp;
		}

		
		if( trash ) {
			/* support IP balance enhancement: flush cache after 40s -- incifer 2008/11 */
		#ifdef CONFIG_IPBALANCE_ENHANCE
			/**
				compare jiffies with WAN_DOWN_TIME, 
				if delta time < "FLUSH_CACHE_TIME", set *flag to 1, means drop the packet, and we want reserve the trash(cache).
				if delta time > "FLUSH_CACHE_TIME", we free the trash(cache)
			**/
			unsigned long time_tmp=0;

			if((*temp)->inf <1)/* invalid inf */
				goto del_ipstruct;

			time_tmp = jiffies;
			if(((time_tmp - WAN_DOWN_TIME[(*temp)->inf - 1])<FLUSH_CACHE_TIME))
			{
				*flag = -1;
				return 0;
			}
			WAN_DOWN_TIME[(*temp)->inf - 1] = 0;
del_ipstruct:
			IpBalanceDel( mask, temp, trash);
			IpSecNum--;
			kfree( trash );
			trash = 0;
		#else
			IpBalanceDel( mask, temp, trash);
			IpSecNum--;
			kfree( trash );
			trash = 0;
		#endif

		} else
			temp = &((*temp)->next);

	}

	return 0;
}
#endif

#ifdef CONFIG_NK_SB_ENHANCEMENT

static int SearchLockRange(int protocol, int port)
{
	int index=-1;
	int i;

	for(i=0; i<LockTableSize; i++)
	{//compare destination ip, port
		if((LockTableList[i].protocol == protocol || LockTableList[i].protocol == 255) && LockTableList[i].dst_sport <= port && LockTableList[i].dst_eport >= port)
		{
			//printk(KERN_EMERG "22 i=[%d], dstsip[%d], dsteip[%d], dstip[%d], dstsprot[%d], dsteport[%d], dstport[%d]\n", i, DstTableList[i].dst_sip, DstTableList[i].dst_eip, dstip, DstTableList[i].dst_sport, DstTableList[i].dst_eport, fl->fl4_dst_port);
			//if match, remember table index, break this loop and return
			index = SB_SEARCH_LOCK_RANGE_SUCCESS;
			break;
		}
	}

	if(index<0)
		return SB_SEARCH_LOCK_RANGE_FAIL;
	else
		return index;
}
#ifdef SB_USE_DST_HASH
static int SBGetHashMask(u32 in, u32 out)
{
	/*
	source ip:192.168.1.100
	destination ip:140.113.4.1
	get the forth byte of source ip, ex: 100
	get the second byte of destination ip, ex: 113
	*/
	return (in & 0xff)|((out>>8) & 0xff00);
}
#endif
#ifdef SB_USE_DST_ARRAY
static void SessionSrcInfoInit(void)
{
	int count=0;

	for( count=0; count<SB_TABLE_NUM; count++) {
		SBSrcInfo[count].lastuse = jiffies;
	}

        SessionDenoCounter=1;

	return ;
}

#else

#ifdef SB_USE_DST_HASH
static int SBInsert( int src_index, int dst_index, struct SessionSrcInfo *entry)
{
	if( SBInfoHead[src_index][dst_index].end == 0 ) //first
		SBInfoHead[src_index][dst_index].end = &(SBInfoHead[src_index][dst_index].next);
	
	*(SBInfoHead[src_index][dst_index].end) = entry;

	SBInfoHead[src_index][dst_index].end = &(entry->next);

	entry->next = 0;

	return 0;
}
static int SBDel( int src_index, int dst_index, struct SessionSrcInfo **front, struct SessionSrcInfo *entry)
{
	//printk(KERN_EMERG "------ SessionBaseDel -------\n");
	if( entry->next == 0 )  //last
		SBInfoHead[src_index][dst_index].end = front;
		
	*front = entry->next;
	
	entry->next = 0;

	return 0;
}

static void SBFlushStr(void)
{
	struct SessionSrcInfo **temp;
	struct SessionSrcInfo *trash=0;
	int src_index=0, dst_index=0;
for( src_index=0; src_index<SB_TABLE_NUM; src_index++ )
	for( dst_index=0; dst_index<SB_TABLE_NUM; dst_index++) {
		while( SBInfoHead[src_index][dst_index].next != 0 ){
			trash = SBInfoHead[src_index][dst_index].next;
			SBDel( src_index, dst_index, &(SBInfoHead[src_index][dst_index].next), trash);
			kfree(trash);
			SessionSecNum--;
			trash = 0;
		}
	}

        SessionDenoCounter=1;

	return ;
}
#else
static int SBInsert( int index, struct SessionSrcInfo *entry)
{
	if( SBInfoHead[index].end == 0 ) //first
		SBInfoHead[index].end = &(SBInfoHead[index].next);
	
	*(SBInfoHead[index].end) = entry;

	SBInfoHead[index].end = &(entry->next);

	entry->next = 0;

	return 0;
}
static int SBDel( int index, struct SessionSrcInfo **front, struct SessionSrcInfo *entry)
{

	//printk(KERN_EMERG "------ SessionBaseDel -------\n");
	if( entry->next == 0 )  //last
		SBInfoHead[index].end = front;
		
	*front = entry->next;
	
	entry->next = 0;
		

	return 0;
}

static void SBFlushStr(void)
{
	struct SessionSrcInfo **temp;
	struct SessionSrcInfo *trash=0;
	int count=0;

	for( count=0; count<SB_TABLE_NUM; count++) {
		while( SBInfoHead[count].next != 0 ){
			trash = SBInfoHead[count].next;
			SBDel( count, &(SBInfoHead[count].next), trash);
			kfree(trash);
			SessionSecNum--;
			trash = 0;
		}
	}

        SessionDenoCounter=1;

	return ;
}
#endif
#endif
static void SBFlushStrByDeno(struct flowi * fl)
{
#ifdef SB_USE_DST_ARRAY
#else
	struct SessionSrcInfo **temp;
	struct SessionSrcInfo *trash=0;
#endif
#ifdef SB_USE_DST_HASH
	int src_index=0, dst_index=0;
#else
	int count=0;
#endif
        int FlushNum=0, FlushStart=0, FlushEnd=0;
        int RouteLockPortIndex=0, RouteLockPortPlace=0;

        if(SessionDenoCounter > SessionNumDeno)
        {
            SessionDenoCounter = 1;
        }

        FlushNum = SB_TABLE_NUM/SessionNumDeno;
        FlushStart = FlushNum * (SessionDenoCounter-1);
        if(SessionDenoCounter==SessionNumDeno)
            FlushEnd = SB_TABLE_NUM;
        else
            FlushEnd = FlushNum * SessionDenoCounter;

//printk(KERN_EMERG "    %s  FlushStart[%d], FlushEnd[%d], FlushNum[%d]\n", __func__, FlushStart, FlushEnd, FlushNum);

#ifdef SB_USE_DST_ARRAY
	for( count=FlushStart; count<FlushEnd; count++) {
		SBSrcInfo[count].lastuse = jiffies;
	}
#else
#ifdef SB_USE_DST_HASH
    if(session_base_flag)
    {//self-defined
        for( src_index=FlushStart; src_index<FlushEnd; src_index++) 
                for( dst_index=0; dst_index<SB_TABLE_NUM; dst_index++) {
                        while( SBInfoHead[src_index][dst_index].next != 0 ){
                                trash = SBInfoHead[src_index][dst_index].next;
                                SBDel( src_index, dst_index, &(SBInfoHead[src_index][dst_index].next), trash);
                                kfree(trash);
                                SessionSecNum--;
                                trash = 0;
                        }
                }
    }
    else
    {
    //>>>Lock Port -- Michael Lu
        for( src_index=FlushStart; src_index<FlushEnd; src_index++) 
                for( dst_index=0; dst_index<SB_TABLE_NUM; dst_index++) {
                        temp = &(SBInfoHead[src_index][dst_index].next);

                        while( *temp ){
        //                         LockTableList[RouteLockPortIndex] |= 0x1<<RouteLockPortPlace;);
                                if(SearchLockRange((*temp)->protocol, (*temp)->dst_port)>0)
                                {
                   // printk(KERN_EMERG "    %s  Lock this session LockTableList[%d], [%d], [%d]\n", __func__, RouteLockPortIndex, RouteLockPortPlace, (*temp)->dst_port);
                                    temp = &((*temp)->next);
                                }
                                else
                                {
                // printk(KERN_EMERG "    %s  Dont Lock this session\n", __func__);
                                    trash = *temp;
                                    SBDel( src_index, dst_index, temp, trash);
                                    kfree(trash);
                                    SessionSecNum--;
                                    trash = 0;
                                }
                        }
                }
    //<<<Lock Port -- Michael Lu
    }
 
#else
	for( count=FlushStart; count<FlushEnd; count++) {
		while( SBInfoHead[count].next != 0 ){
			trash = SBInfoHead[count].next;
			SBDel( count, &(SBInfoHead[count].next), trash);
			kfree(trash);
			SessionSecNum--;
			trash = 0;
		}
	}
#endif
#endif

        SessionDenoCounter++;

        if(SessionDenoCounter > SessionNumDeno)
        {
           // printk(KERN_EMERG "    %s  SessionDenoCounter[%d], SessionNumDeno[%d]\n", __func__, SessionDenoCounter, SessionNumDeno);
            SessionDenoCounter = 1;
        }

	return ;
}

static int SearchDstRange(struct flowi * fl, u32 dstip)
{
	int index=-1;
	int i;

	for(i=0; i<DstTableSize; i++)
	{//compare destination ip, port
		//printk(KERN_EMERG "11 dstsip[%d], dsteip[%d], dstip[%d], dstsprot[%d], dsteport[%d], dstport[%d]\n", DstTableList[i].dst_sip, DstTableList[i].dst_eip, dstip, DstTableList[i].dst_sport, DstTableList[i].dst_eport, fl->fl4_dst_port);
		if((DstTableList[i].protocol == 255 || DstTableList[i].protocol ==fl->fl4_protocol_type) && DstTableList[i].dst_sip <= dstip && DstTableList[i].dst_eip >= dstip && DstTableList[i].dst_sport <= fl->fl4_dst_port && DstTableList[i].dst_eport >= fl->fl4_dst_port)
		{
			//printk(KERN_EMERG "22 i=[%d], dstsip[%d], dsteip[%d], dstip[%d], dstsprot[%d], dsteport[%d], dstport[%d]\n", i, DstTableList[i].dst_sip, DstTableList[i].dst_eip, dstip, DstTableList[i].dst_sport, DstTableList[i].dst_eport, fl->fl4_dst_port);
			//if match, remember table index, break this loop and return
			index = i;
			break;
		}
	}

	if(index<0)
		return SB_SEARCH_DST_RANGE_FAIL;
	else
		return index;
}

static void SBCreateNewStr(u32 InIp, struct fib_result *res, struct flowi * fl, u32 OutIp, int index)
{
#ifdef SB_USE_DST_HASH
	int src_mask = InIp & 0xff;
	int dst_mask = (OutIp>>16) & 0xff;
#else
	int mask = InIp & SessionIPMask;
#endif
	unsigned long period;
#ifdef SB_USE_DST_ARRAY
        int dst_mask=0;
#else
	struct SessionSrcInfo *insert=0;
	struct SessionSrcInfo *trash=0;
#endif
	int count=0;
	int old;

	//printk(KERN_EMERG "    SessionSecNum[%d] SessionNumData[%d] src_mask[%d] dst_mask[%d]\n", SessionSecNum, SessionNumData, src_mask, dst_mask);
	//no found, creat new
	if( SessionSecNum >= SessionNumData) {
		//printk(KERN_EMERG "    Use Flush[%d][%d][%d], the str is full, clear str [ Start ]\n", SessionDenoFlag, SessionSecNum, SessionNumData);

        if(SessionDenoFlag==1)
			SBFlushStrByDeno(&fl);
		else
		{
#ifdef SB_USE_DST_ARRAY
			SessionSrcInfoInit();
#else
			SBFlushStr();
#endif
			session_base_flush=0;
		}

		//printk(KERN_EMERG "    clear str [ End ]\n");
	}

#ifdef SB_USE_DST_ARRAY
	if(index<0)
	{
                SBSrcInfo[mask].src_ip = InIp;
                dst_mask = (OutIp & SessionDstMask)>>16;
                SBDstInfo[mask][dst_mask].dst_ip=OutIp;
                SBDstInfo[mask][dst_mask].inf = (res->fi->fib_nh[res->nh_sel]).nh_dev;
                SBDstInfo[mask][dst_mask].table = nk_get_table(res);
                SBDstInfo[mask][dst_mask].lastuse = jiffies;
	}
	else
	{
                SBSrcInfo[mask].src_ip = InIp;
                SBDstInfo[mask][index].dst_ip=0;
                SBDstInfo[mask][index].inf = (res->fi->fib_nh[res->nh_sel]).nh_dev;
                SBDstInfo[mask][index].table = nk_get_table(res);
                SBDstInfo[mask][index].lastuse = jiffies;
	}
#else
	insert = kmalloc( sizeof(struct SessionSrcInfo), GFP_ATOMIC);
	if( !insert )
		return;
	if(index<0)
	{
		insert->next=0;
		insert->src_ip = InIp;
		insert->dst_index=index;
		insert->dst_ip=OutIp;
		insert->dst_port=fl->fl4_dst_port;
		insert->protocol=fl->fl4_protocol_type;
		insert->inf = (res->fi->fib_nh[res->nh_sel]).nh_dev;
		insert->table = nk_get_table(res);
		insert->lastuse = jiffies;
	}
	else
	{
		insert->next=0;
		insert->src_ip = InIp;
		insert->dst_index=index;
		insert->dst_ip=0;
		insert->dst_port=0;
		insert->protocol=fl->fl4_protocol_type;
		insert->inf = (res->fi->fib_nh[res->nh_sel]).nh_dev;
		insert->table = nk_get_table(res);
		insert->lastuse = jiffies;
	}
#ifdef SB_USE_DST_HASH
	SBInsert( src_mask, dst_mask, insert );
#else
	SBInsert( mask, insert);
#endif
	SessionSecNum++;
#endif

	return;

}

static struct net_device *SB_1_SearchIpOut( u32 InIp, struct fib_result *res, int index, u32 OutIp)
{
#ifdef SB_USE_DST_HASH
	u32 src_mask = InIp & 0xff;
	u32 dst_mask = (OutIp>>16) & 0xff;
#else
	u32 mask = InIp & SessionIPMask;
#endif
#ifdef SB_USE_DST_ARRAY
#else
	struct SessionSrcInfo **temp;
	struct SessionSrcInfo *insert=0;
	struct SessionSrcInfo *trash=0;
#endif
	struct net_device *ans;
	int count=0;

#ifdef SB_USE_DST_ARRAY

        if( InIp == SBSrcInfo[mask].src_ip && nk_get_table(res) == SBDstInfo[mask][index].table && SBSrcInfo[mask].lastuse < SBDstInfo[mask][index].lastuse && SBDstInfo[mask][index].used )
        {
		for( count=0; count < res->fi->fib_nhs; count++) {
			if( (res->fi->fib_nh[count]).nh_dev == SBDstInfo[mask][index].inf ) {
				SBDstInfo[mask][index].lastuse = jiffies;
				ans = SBDstInfo[mask][index].inf;
				res->nh_sel = count;
				return ans;
			}
		}

		SBDstInfo[mask][index].lastuse = jiffies;
        }

#else
#ifdef SB_USE_DST_HASH
	temp = &(SBInfoHead[src_mask][dst_mask].next);
	while( *temp ) {
	//printk(KERN_EMERG "(*temp)->src_ip[%d], (*temp)->dst_index[%d], index[%d], InIp[%d], mask[%d]\n", (*temp)->src_ip, (*temp)->dst_index, index, InIp, mask);
		if( InIp == (*temp)->src_ip && nk_get_table(res) == (*temp)->table && index == (*temp)->dst_index ) {
			for( count=0; count < res->fi->fib_nhs; count++) {
				if( (res->fi->fib_nh[count]).nh_dev == (*temp)->inf ) {
					(*temp)->lastuse = jiffies;
					ans = (*temp)->inf;
					insert = *temp;
					if(insert->next != 0){//last don't need update
						SBDel( src_mask, dst_mask, temp, insert);
						SBInsert( src_mask, dst_mask, insert);
					}
//					printk("get and update index[%x] ans[%s]\n",mask,ans->name);
//					showBalanceData();
					//2007/5/3 trenchen : set the real interface
					res->nh_sel = count;
					return ans;
				}
			}

			trash = *temp;
		}

		
		if( trash ) {
			SBDel( src_mask, dst_mask, temp, trash);
			SessionSecNum--;
			kfree( trash );



			trash = 0;

		} else
			temp = &((*temp)->next);

	}
#else
	temp = &(SBInfoHead[mask].next);
	while( *temp ) {
	//printk(KERN_EMERG "(*temp)->src_ip[%d], (*temp)->dst_index[%d], index[%d], InIp[%d], mask[%d]\n", (*temp)->src_ip, (*temp)->dst_index, index, InIp, mask);
		if( InIp == (*temp)->src_ip && nk_get_table(res) == (*temp)->table && index == (*temp)->dst_index ) {
			for( count=0; count < res->fi->fib_nhs; count++) {
				if( (res->fi->fib_nh[count]).nh_dev == (*temp)->inf ) {
					(*temp)->lastuse = jiffies;
					ans = (*temp)->inf;
					insert = *temp;
					if(insert->next != 0){//last don't need update
						SBDel( mask, temp, insert);
						SBInsert( mask, insert);
					}
//					printk("get and update index[%x] ans[%s]\n",mask,ans->name);
//					showBalanceData();
					//2007/5/3 trenchen : set the real interface
					res->nh_sel = count;
					return ans;
				}
			}

			trash = *temp;
		}

		
		if( trash ) {
			SBDel( mask, temp, trash);
			SessionSecNum--;
			kfree( trash );



			trash = 0;

		} else
			temp = &((*temp)->next);

	}
#endif
#endif
	return 0;
}
 
static struct net_device *SB_2_SearchIpOut( u32 InIp, struct fib_result *res, u32 OutIp)
{
#ifdef SB_USE_DST_HASH
	u32 src_mask = InIp & 0xff;
	u32 dst_mask = (OutIp>>16) & 0xff;
#else
	u32 mask = InIp & SessionIPMask;
#endif
#ifdef SB_USE_DST_ARRAY
        u32 dst_mask = 0;
#else
	struct SessionSrcInfo **temp;
	struct SessionSrcInfo *insert=0;
	struct SessionSrcInfo *trash=0;
#endif
	struct net_device *ans;
	int count=0;

#ifdef SB_USE_DST_ARRAY

        dst_mask = (OutIp & SessionDstMask)>>16;

        if( InIp == SBSrcInfo[mask].src_ip && nk_get_table(res) == SBDstInfo[mask][dst_mask].table && SBSrcInfo[mask].lastuse > SBDstInfo[mask][dst_mask].lastuse &&  SBDstInfo[mask][dst_mask].used )
        {
		for( count=0; count < res->fi->fib_nhs; count++) {
			if( (res->fi->fib_nh[count]).nh_dev == SBDstInfo[mask][dst_mask].inf ) {
				SBDstInfo[mask][dst_mask].lastuse = jiffies;
				ans = SBDstInfo[mask][dst_mask].inf;
				res->nh_sel = count;
				return ans;
			}
		}

		SBDstInfo[mask][dst_mask].lastuse = jiffies;
        }
#else
#ifdef SB_USE_DST_HASH
	temp = &(SBInfoHead[src_mask][dst_mask].next);
	while( *temp ) {
	//printk(KERN_EMERG "(*temp)->src_ip[%x], (*temp)->dst_index[%x], InIp[%x], mask[%x], (*temp)->dst_ip[%x], OutIp[%x]\n", (*temp)->src_ip, (*temp)->dst_index, InIp, mask, (*temp)->dst_ip, OutIp);
		if( InIp == (*temp)->src_ip && nk_get_table(res) == (*temp)->table && ((*temp)->dst_ip & 0xffff0000) == (OutIp & 0xffff0000)) {
			for( count=0; count < res->fi->fib_nhs; count++) {
				if( (res->fi->fib_nh[count]).nh_dev == (*temp)->inf ) {
					(*temp)->lastuse = jiffies;
					ans = (*temp)->inf;
					insert = *temp;
					if(insert->next != 0){//last don't need update
						SBDel( src_mask, dst_mask, temp, insert);
						SBInsert( src_mask, dst_mask, insert);
					}
//					printk("get and update index[%x] ans[%s]\n",mask,ans->name);
//					showBalanceData();
					//2007/5/3 trenchen : set the real interface
					res->nh_sel = count; 
					return ans;
				}
			}

			trash = *temp;
		}
		
		if( trash ) {
			SBDel( src_mask, dst_mask, temp, trash);
			SessionSecNum--;
			kfree( trash );

			trash = 0;

		} else
			temp = &((*temp)->next);
	}
#else
	temp = &(SBInfoHead[mask].next);
	while( *temp ) {
	//printk(KERN_EMERG "(*temp)->src_ip[%x], (*temp)->dst_index[%x], InIp[%x], mask[%x], (*temp)->dst_ip[%x], OutIp[%x]\n", (*temp)->src_ip, (*temp)->dst_index, InIp, mask, (*temp)->dst_ip, OutIp);
		if( InIp == (*temp)->src_ip && nk_get_table(res) == (*temp)->table && ((*temp)->dst_ip & 0xffff0000) == (OutIp & 0xffff0000)) {
			for( count=0; count < res->fi->fib_nhs; count++) {
				if( (res->fi->fib_nh[count]).nh_dev == (*temp)->inf ) {
					(*temp)->lastuse = jiffies;
					ans = (*temp)->inf;
					insert = *temp;
					if(insert->next != 0){//last don't need update
						SBDel( mask, temp, insert);
						SBInsert( mask, insert);
					}
//					printk("get and update index[%x] ans[%s]\n",mask,ans->name);
//					showBalanceData();
					//2007/5/3 trenchen : set the real interface
					res->nh_sel = count; 
					return ans;
				}
			}

			trash = *temp;
		}
		
		if( trash ) {
			SBDel( mask, temp, trash);
			SessionSecNum--;
			kfree( trash );

			trash = 0;

		} else
			temp = &((*temp)->next);
	}
#endif
#endif
	return 0;
}
#endif
#endif
///////////////////////////////////////////////////


static struct dst_ops ipv4_dst_ops = {
	.family =		AF_INET,
	.protocol =		__constant_htons(ETH_P_IP),
	.gc =			rt_garbage_collect,
	.check =		ipv4_dst_check,
	.destroy =		ipv4_dst_destroy,
	.ifdown =		ipv4_dst_ifdown,
	.negative_advice =	ipv4_negative_advice,
	.link_failure =		ipv4_link_failure,
	.update_pmtu =		ip_rt_update_pmtu,
	.entry_size =		sizeof(struct rtable),
};

#define ECN_OR_COST(class)	TC_PRIO_##class

__u8 ip_tos2prio[16] = {
	TC_PRIO_BESTEFFORT,
	ECN_OR_COST(FILLER),
	TC_PRIO_BESTEFFORT,
	ECN_OR_COST(BESTEFFORT),
	TC_PRIO_BULK,
	ECN_OR_COST(BULK),
	TC_PRIO_BULK,
	ECN_OR_COST(BULK),
	TC_PRIO_INTERACTIVE,
	ECN_OR_COST(INTERACTIVE),
	TC_PRIO_INTERACTIVE,
	ECN_OR_COST(INTERACTIVE),
	TC_PRIO_INTERACTIVE_BULK,
	ECN_OR_COST(INTERACTIVE_BULK),
	TC_PRIO_INTERACTIVE_BULK,
	ECN_OR_COST(INTERACTIVE_BULK)
};


/*
 * Route cache.
 */

/* The locking scheme is rather straight forward:
 *
 * 1) Read-Copy Update protects the buckets of the central route hash.
 * 2) Only writers remove entries, and they hold the lock
 *    as they look at rtable reference counts.
 * 3) Only readers acquire references to rtable entries,
 *    they do so with atomic increments and with the
 *    lock held.
 */

struct rt_hash_bucket {
	struct rtable	*chain;
};
#if defined(CONFIG_SMP) || defined(CONFIG_DEBUG_SPINLOCK)
/*
 * Instead of using one spinlock for each rt_hash_bucket, we use a table of spinlocks
 * The size of this table is a power of two and depends on the number of CPUS.
 */
#if NR_CPUS >= 32
#define RT_HASH_LOCK_SZ	4096
#elif NR_CPUS >= 16
#define RT_HASH_LOCK_SZ	2048
#elif NR_CPUS >= 8
#define RT_HASH_LOCK_SZ	1024
#elif NR_CPUS >= 4
#define RT_HASH_LOCK_SZ	512
#else
#define RT_HASH_LOCK_SZ	256
#endif

static spinlock_t	*rt_hash_locks;
# define rt_hash_lock_addr(slot) &rt_hash_locks[(slot) & (RT_HASH_LOCK_SZ - 1)]
# define rt_hash_lock_init()	{ \
		int i; \
		rt_hash_locks = kmalloc(sizeof(spinlock_t) * RT_HASH_LOCK_SZ, GFP_KERNEL); \
		if (!rt_hash_locks) panic("IP: failed to allocate rt_hash_locks\n"); \
		for (i = 0; i < RT_HASH_LOCK_SZ; i++) \
			spin_lock_init(&rt_hash_locks[i]); \
		}
#else
# define rt_hash_lock_addr(slot) NULL
# define rt_hash_lock_init()
#endif

static struct rt_hash_bucket 	*rt_hash_table;
static unsigned			rt_hash_mask;
static int			rt_hash_log;
static unsigned int		rt_hash_rnd;

static DEFINE_PER_CPU(struct rt_cache_stat, rt_cache_stat);
#define RT_CACHE_STAT_INC(field) \
	(per_cpu(rt_cache_stat, raw_smp_processor_id()).field++)

static int rt_intern_hash(unsigned hash, struct rtable *rth,
				struct rtable **res);

#ifdef ROUTING_HASH_IMPROVE
//Michael Lu add
static unsigned int rt_hash_code(u32 daddr, u32 saddr, u32 dport, u32 sport, u8 tos)
{
        unsigned hash = jhash_3words(daddr, saddr, (u32) tos, rt_hash_rnd);
	hash ^= ((dport & 0x0000FFFF) << 16) | (sport & 0x0000FFFF);
	hash ^= (hash >> 16);

	return (hash & rt_hash_mask);
}
#else

static unsigned int rt_hash_code(u32 daddr, u32 saddr, u8 tos)
{
	return (jhash_3words(daddr, saddr, (u32) tos, rt_hash_rnd)
		& rt_hash_mask);
}
#endif

#ifdef CONFIG_PROC_FS
struct rt_cache_iter_state {
	int bucket;
};

static struct rtable *rt_cache_get_first(struct seq_file *seq)
{
	struct rtable *r = NULL;
	struct rt_cache_iter_state *st = seq->private;

	for (st->bucket = rt_hash_mask; st->bucket >= 0; --st->bucket) {
		rcu_read_lock_bh();
		r = rt_hash_table[st->bucket].chain;
		if (r)
			break;
		rcu_read_unlock_bh();
	}
	return r;
}

static struct rtable *rt_cache_get_next(struct seq_file *seq, struct rtable *r)
{
	struct rt_cache_iter_state *st = rcu_dereference(seq->private);

	r = r->u.rt_next;
	while (!r) {
		rcu_read_unlock_bh();
		if (--st->bucket < 0)
			break;
		rcu_read_lock_bh();
		r = rt_hash_table[st->bucket].chain;
	}
	return r;
}

static struct rtable *rt_cache_get_idx(struct seq_file *seq, loff_t pos)
{
	struct rtable *r = rt_cache_get_first(seq);

	if (r)
		while (pos && (r = rt_cache_get_next(seq, r)))
			--pos;
	return pos ? NULL : r;
}

static void *rt_cache_seq_start(struct seq_file *seq, loff_t *pos)
{
	return *pos ? rt_cache_get_idx(seq, *pos - 1) : SEQ_START_TOKEN;
}

static void *rt_cache_seq_next(struct seq_file *seq, void *v, loff_t *pos)
{
	struct rtable *r = NULL;

	if (v == SEQ_START_TOKEN)
		r = rt_cache_get_first(seq);
	else
		r = rt_cache_get_next(seq, v);
	++*pos;
	return r;
}

static void rt_cache_seq_stop(struct seq_file *seq, void *v)
{
	if (v && v != SEQ_START_TOKEN)
		rcu_read_unlock_bh();
}

static int rt_cache_seq_show(struct seq_file *seq, void *v)
{
	if (v == SEQ_START_TOKEN)
		seq_printf(seq, "%-127s\n",
			   "Iface\tDestination\tGateway \tFlags\t\tRefCnt\tUse\t"
			   "Metric\tSource\t\tMTU\tWindow\tIRTT\tTOS\tHHRef\t"
			   "HHUptod\tSpecDst");
	else {
		struct rtable *r = v;
		char temp[256];

		sprintf(temp, "%s\t%08lX\t%08lX\t%8X\t%d\t%u\t%d\t"
			      "%08lX\t%d\t%u\t%u\t%02X\t%d\t%1d\t%08X",
			r->u.dst.dev ? r->u.dst.dev->name : "*",
			(unsigned long)r->rt_dst, (unsigned long)r->rt_gateway,
			r->rt_flags, atomic_read(&r->u.dst.__refcnt),
			r->u.dst.__use, 0, (unsigned long)r->rt_src,
			(dst_metric(&r->u.dst, RTAX_ADVMSS) ?
			     (int)dst_metric(&r->u.dst, RTAX_ADVMSS) + 40 : 0),
			dst_metric(&r->u.dst, RTAX_WINDOW),
			(int)((dst_metric(&r->u.dst, RTAX_RTT) >> 3) +
			      dst_metric(&r->u.dst, RTAX_RTTVAR)),
			r->fl.fl4_tos,
			r->u.dst.hh ? atomic_read(&r->u.dst.hh->hh_refcnt) : -1,
			r->u.dst.hh ? (r->u.dst.hh->hh_output ==
				       dev_queue_xmit) : 0,
			r->rt_spec_dst);
		seq_printf(seq, "%-127s\n", temp);
        }
  	return 0;
}

static struct seq_operations rt_cache_seq_ops = {
	.start  = rt_cache_seq_start,
	.next   = rt_cache_seq_next,
	.stop   = rt_cache_seq_stop,
	.show   = rt_cache_seq_show,
};

static int rt_cache_seq_open(struct inode *inode, struct file *file)
{
	struct seq_file *seq;
	int rc = -ENOMEM;
	struct rt_cache_iter_state *s = kmalloc(sizeof(*s), GFP_KERNEL);

	if (!s)
		goto out;
	rc = seq_open(file, &rt_cache_seq_ops);
	if (rc)
		goto out_kfree;
	seq          = file->private_data;
	seq->private = s;
	memset(s, 0, sizeof(*s));
out:
	return rc;
out_kfree:
	kfree(s);
	goto out;
}

static struct file_operations rt_cache_seq_fops = {
	.owner	 = THIS_MODULE,
	.open	 = rt_cache_seq_open,
	.read	 = seq_read,
	.llseek	 = seq_lseek,
	.release = seq_release_private,
};


static void *rt_cpu_seq_start(struct seq_file *seq, loff_t *pos)
{
	int cpu;

	if (*pos == 0)
		return SEQ_START_TOKEN;

	for (cpu = *pos-1; cpu < NR_CPUS; ++cpu) {
		if (!cpu_possible(cpu))
			continue;
		*pos = cpu+1;
		return &per_cpu(rt_cache_stat, cpu);
	}
	return NULL;
}

static void *rt_cpu_seq_next(struct seq_file *seq, void *v, loff_t *pos)
{
	int cpu;

	for (cpu = *pos; cpu < NR_CPUS; ++cpu) {
		if (!cpu_possible(cpu))
			continue;
		*pos = cpu+1;
		return &per_cpu(rt_cache_stat, cpu);
	}
	return NULL;
	
}

static void rt_cpu_seq_stop(struct seq_file *seq, void *v)
{

}

static int rt_cpu_seq_show(struct seq_file *seq, void *v)
{
	struct rt_cache_stat *st = v;

	if (v == SEQ_START_TOKEN) {
		seq_printf(seq, "entries  in_hit in_slow_tot in_slow_mc in_no_route in_brd in_martian_dst in_martian_src  out_hit out_slow_tot out_slow_mc  gc_total gc_ignored gc_goal_miss gc_dst_overflow in_hlist_search out_hlist_search\n");
		return 0;
	}
	
	seq_printf(seq,"%08x  %08x %08x %08x %08x %08x %08x %08x "
		   " %08x %08x %08x %08x %08x %08x %08x %08x %08x \n",
		   atomic_read(&ipv4_dst_ops.entries),
		   st->in_hit,
		   st->in_slow_tot,
		   st->in_slow_mc,
		   st->in_no_route,
		   st->in_brd,
		   st->in_martian_dst,
		   st->in_martian_src,

		   st->out_hit,
		   st->out_slow_tot,
		   st->out_slow_mc, 

		   st->gc_total,
		   st->gc_ignored,
		   st->gc_goal_miss,
		   st->gc_dst_overflow,
		   st->in_hlist_search,
		   st->out_hlist_search
		);
	return 0;
}

static struct seq_operations rt_cpu_seq_ops = {
	.start  = rt_cpu_seq_start,
	.next   = rt_cpu_seq_next,
	.stop   = rt_cpu_seq_stop,
	.show   = rt_cpu_seq_show,
};


static int rt_cpu_seq_open(struct inode *inode, struct file *file)
{
	return seq_open(file, &rt_cpu_seq_ops);
}

static struct file_operations rt_cpu_seq_fops = {
	.owner	 = THIS_MODULE,
	.open	 = rt_cpu_seq_open,
	.read	 = seq_read,
	.llseek	 = seq_lseek,
	.release = seq_release,
};

#endif /* CONFIG_PROC_FS */
  
static __inline__ void rt_free(struct rtable *rt)
{
	multipath_remove(rt);
	call_rcu_bh(&rt->u.dst.rcu_head, dst_rcu_free);
}

static __inline__ void rt_drop(struct rtable *rt)
{
	multipath_remove(rt);
	ip_rt_put(rt);
	call_rcu_bh(&rt->u.dst.rcu_head, dst_rcu_free);
}

static __inline__ int rt_fast_clean(struct rtable *rth)
{
	/* Kill broadcast/multicast entries very aggresively, if they
	   collide in hash table with more useful entries */
	return (rth->rt_flags & (RTCF_BROADCAST | RTCF_MULTICAST)) &&
		rth->fl.iif && rth->u.rt_next;
}

static __inline__ int rt_valuable(struct rtable *rth)
{
	return (rth->rt_flags & (RTCF_REDIRECTED | RTCF_NOTIFY)) ||
		rth->u.dst.expires;
}

static int rt_may_expire(struct rtable *rth, unsigned long tmo1, unsigned long tmo2)
{
	unsigned long age;
	int ret = 0;

	if (atomic_read(&rth->u.dst.__refcnt))
		goto out;

	ret = 1;
	if (rth->u.dst.expires &&
	    time_after_eq(jiffies, rth->u.dst.expires))
		goto out;

	age = jiffies - rth->u.dst.lastuse;
	ret = 0;
	if ((age <= tmo1 && !rt_fast_clean(rth)) ||
	    (age <= tmo2 && rt_valuable(rth)))
		goto out;
	ret = 1;
out:	return ret;
}

/* Bits of score are:
 * 31: very valuable
 * 30: not quite useless
 * 29..0: usage counter
 */
static inline u32 rt_score(struct rtable *rt)
{
	u32 score = jiffies - rt->u.dst.lastuse;

	score = ~score & ~(3<<30);

	if (rt_valuable(rt))
		score |= (1<<31);

	if (!rt->fl.iif ||
	    !(rt->rt_flags & (RTCF_BROADCAST|RTCF_MULTICAST|RTCF_LOCAL)))
		score |= (1<<30);

	return score;
}

static inline int compare_keys(struct flowi *fl1, struct flowi *fl2)
{
	return memcmp(&fl1->nl_u.ip4_u, &fl2->nl_u.ip4_u, sizeof(fl1->nl_u.ip4_u)) == 0 &&
	       fl1->oif     == fl2->oif &&
	       fl1->iif     == fl2->iif;
}

#ifdef CONFIG_IP_ROUTE_MULTIPATH_CACHED
static struct rtable **rt_remove_balanced_route(struct rtable **chain_head,
						struct rtable *expentry,
						int *removed_count)
{
	int passedexpired = 0;
	struct rtable **nextstep = NULL;
	struct rtable **rthp = chain_head;
	struct rtable *rth;

	if (removed_count)
		*removed_count = 0;

	while ((rth = *rthp) != NULL) {
		if (rth == expentry)
			passedexpired = 1;

		if (((*rthp)->u.dst.flags & DST_BALANCED) != 0  &&
		    compare_keys(&(*rthp)->fl, &expentry->fl)) {
			if (*rthp == expentry) {
				*rthp = rth->u.rt_next;
				continue;
			} else {
				*rthp = rth->u.rt_next;
				rt_free(rth);
				if (removed_count)
					++(*removed_count);
			}
		} else {
			if (!((*rthp)->u.dst.flags & DST_BALANCED) &&
			    passedexpired && !nextstep)
				nextstep = &rth->u.rt_next;

			rthp = &rth->u.rt_next;
		}
	}

	rt_free(expentry);
	if (removed_count)
		++(*removed_count);

	return nextstep;
}
#endif /* CONFIG_IP_ROUTE_MULTIPATH_CACHED */


/* This runs via a timer and thus is always in BH context. */
static void rt_check_expire(unsigned long dummy)
{
	static unsigned int rover;
	unsigned int i = rover, goal;
	struct rtable *rth, **rthp;
	unsigned long now = jiffies;
	u64 mult;

	mult = ((u64)ip_rt_gc_interval) << rt_hash_log;
	if (ip_rt_gc_timeout > 1)
		do_div(mult, ip_rt_gc_timeout);
	goal = (unsigned int)mult;
	if (goal > rt_hash_mask) goal = rt_hash_mask + 1;
	for (; goal > 0; goal--) {
		unsigned long tmo = ip_rt_gc_timeout;

		i = (i + 1) & rt_hash_mask;
		rthp = &rt_hash_table[i].chain;

		if (*rthp == 0)
			continue;
		spin_lock(rt_hash_lock_addr(i));
		while ((rth = *rthp) != NULL) {
			if (rth->u.dst.expires) {
				/* Entry is expired even if it is in use */
				if (time_before_eq(now, rth->u.dst.expires)) {
					tmo >>= 1;
					rthp = &rth->u.rt_next;
					continue;
				}
			} else if (!rt_may_expire(rth, tmo, ip_rt_gc_timeout)) {
				tmo >>= 1;
				rthp = &rth->u.rt_next;
				continue;
			}

			/* Cleanup aged off entries. */
#ifdef CONFIG_IP_ROUTE_MULTIPATH_CACHED
			/* remove all related balanced entries if necessary */
			if (rth->u.dst.flags & DST_BALANCED) {
				rthp = rt_remove_balanced_route(
					&rt_hash_table[i].chain,
					rth, NULL);
				if (!rthp)
					break;
			} else {
				*rthp = rth->u.rt_next;
				rt_free(rth);
			}
#else /* CONFIG_IP_ROUTE_MULTIPATH_CACHED */
 			*rthp = rth->u.rt_next;
 			rt_free(rth);
#endif /* CONFIG_IP_ROUTE_MULTIPATH_CACHED */
		}
		spin_unlock(rt_hash_lock_addr(i));

		/* Fallback loop breaker. */
		if (time_after(jiffies, now))
			break;
	}
	rover = i;
	mod_timer(&rt_periodic_timer, jiffies + ip_rt_gc_interval);
}

#ifndef CONFIG_NK_ROUTE_CACHE_MODIFY

/* This can run from both BH and non-BH contexts, the latter
 * in the case of a forced flush event.
 */
static void rt_run_flush(unsigned long dummy)
{
	int i;
	struct rtable *rth, *next;

	rt_deadline = 0;

	get_random_bytes(&rt_hash_rnd, 4);

	for (i = rt_hash_mask; i >= 0; i--) {
		spin_lock_bh(rt_hash_lock_addr(i));
		rth = rt_hash_table[i].chain;
		if (rth)
			rt_hash_table[i].chain = NULL;
		spin_unlock_bh(rt_hash_lock_addr(i));

		for (; rth; rth = next) {
			next = rth->u.rt_next;
			rt_free(rth);
		}
	}
}
#else
static void rt_run_flush(struct net_device *dummy)
{
	int i;
	struct rtable *rth, *next,**pre_rth , *tmp;
	struct dst_entry *pdst;

	rt_deadline = 0;
	
	//2007/5/18 trenchen : don't change rt_hash_rnd, or routing cache hash value will change
//	get_random_bytes(&rt_hash_rnd,4);

	if( dummy ) {
		//printk("rt_run_flush:part[%s]\n",dummy->name);
		for (i = rt_hash_mask; i >= 0; i--) {
			spin_lock_bh(rt_hash_lock_addr(i));
			pre_rth = &(rt_hash_table[i].chain);
			while (*pre_rth){
				pdst = &((*pre_rth)->u.dst);
				tmp = 0;
				if (!strcmp((char *)pdst->dev->name,(char *)(dummy->name)) || !strcmp((char *)pdst->dev->name,"lo") || (*pre_rth)->fl.iif==dummy->ifindex ) {
                                    tmp=*pre_rth;
				    *pre_rth=tmp->u.rt_next;
				    if (tmp)
					rt_free(tmp);
				} else	{
                                     pre_rth=&((*pre_rth)->u.rt_next);
				}
			}
			spin_unlock_bh(rt_hash_lock_addr(i));
		}
	} else {
		//printk("rt_run_flush:all\n");
		for (i = rt_hash_mask; i >= 0; i--) {
			spin_lock_bh(rt_hash_lock_addr(i));
			rth = rt_hash_table[i].chain;
			if (rth)
				rt_hash_table[i].chain = NULL;
			spin_unlock_bh(rt_hash_lock_addr(i));

			for (; rth; rth = next) {
				next = rth->u.rt_next;
				rt_free(rth);
			}
		}
	}
}
#endif

static DEFINE_SPINLOCK(rt_flush_lock);

#ifndef CONFIG_NK_ROUTE_CACHE_MODIFY
void rt_cache_flush(int delay)
#else
void rt_cache_flush(int delay,struct net_device *dev)
#endif
{
	unsigned long now = jiffies;
	int user_mode = !in_softirq();

	if (delay < 0)
		delay = ip_rt_min_delay;

	/* flush existing multipath state*/
	multipath_flush();

	spin_lock_bh(&rt_flush_lock);

	if (del_timer(&rt_flush_timer) && delay > 0 && rt_deadline) {
		long tmo = (long)(rt_deadline - now);

		/* If flush timer is already running
		   and flush request is not immediate (delay > 0):

		   if deadline is not achieved, prolongate timer to "delay",
		   otherwise fire it at deadline time.
		 */

		if (user_mode && tmo < ip_rt_max_delay-ip_rt_min_delay)
			tmo = 0;
		
		if (delay > tmo)
			delay = tmo;
	}

	if (delay <= 0) {
		spin_unlock_bh(&rt_flush_lock);
#ifndef CONFIG_NK_ROUTE_CACHE_MODIFY
		rt_run_flush(0);
#else
		rt_run_flush(dev);
#endif
		return;
	}

	if (rt_deadline == 0)
		rt_deadline = now + ip_rt_max_delay;

	mod_timer(&rt_flush_timer, now+delay);
	spin_unlock_bh(&rt_flush_lock);
}

#ifndef CONFIG_NK_ROUTE_CACHE_MODIFY
static void rt_secret_rebuild(unsigned long dummy)
{
	unsigned long now = jiffies;

	rt_cache_flush(0);
	mod_timer(&rt_secret_timer, now + ip_rt_secret_interval);
}
#endif
/*
   Short description of GC goals.

   We want to build algorithm, which will keep routing cache
   at some equilibrium point, when number of aged off entries
   is kept approximately equal to newly generated ones.

   Current expiration strength is variable "expire".
   We try to adjust it dynamically, so that if networking
   is idle expires is large enough to keep enough of warm entries,
   and when load increases it reduces to limit cache size.
 */

static int rt_garbage_collect(void)
{
	static unsigned long expire = RT_GC_TIMEOUT;
	static unsigned long last_gc;
	static int rover;
	static int equilibrium;
	struct rtable *rth, **rthp;
	unsigned long now = jiffies;
	int goal;

	/*
	 * Garbage collection is pretty expensive,
	 * do not make it too frequently.
	 */

	RT_CACHE_STAT_INC(gc_total);

	if (now - last_gc < ip_rt_gc_min_interval &&
	    atomic_read(&ipv4_dst_ops.entries) < ip_rt_max_size) {
		RT_CACHE_STAT_INC(gc_ignored);
		goto out;
	}

	/* Calculate number of entries, which we want to expire now. */
	goal = atomic_read(&ipv4_dst_ops.entries) -
		(ip_rt_gc_elasticity << rt_hash_log);
	if (goal <= 0) {
		if (equilibrium < ipv4_dst_ops.gc_thresh)
			equilibrium = ipv4_dst_ops.gc_thresh;
		goal = atomic_read(&ipv4_dst_ops.entries) - equilibrium;
		if (goal > 0) {
			equilibrium += min_t(unsigned int, goal / 2, rt_hash_mask + 1);
			goal = atomic_read(&ipv4_dst_ops.entries) - equilibrium;
		}
	} else {
		/* We are in dangerous area. Try to reduce cache really
		 * aggressively.
		 */
		goal = max_t(unsigned int, goal / 2, rt_hash_mask + 1);
		equilibrium = atomic_read(&ipv4_dst_ops.entries) - goal;
	}

	if (now - last_gc >= ip_rt_gc_min_interval)
		last_gc = now;

	if (goal <= 0) {
		equilibrium += goal;
		goto work_done;
	}

	do {
		int i, k;

		for (i = rt_hash_mask, k = rover; i >= 0; i--) {
			unsigned long tmo = expire;

			k = (k + 1) & rt_hash_mask;
			rthp = &rt_hash_table[k].chain;
			spin_lock_bh(rt_hash_lock_addr(k));
			while ((rth = *rthp) != NULL) {
				if (!rt_may_expire(rth, tmo, expire)) {
					tmo >>= 1;
					rthp = &rth->u.rt_next;
					continue;
				}
#ifdef CONFIG_IP_ROUTE_MULTIPATH_CACHED
				/* remove all related balanced entries
				 * if necessary
				 */
				if (rth->u.dst.flags & DST_BALANCED) {
					int r;

					rthp = rt_remove_balanced_route(
						&rt_hash_table[k].chain,
						rth,
						&r);
					goal -= r;
					if (!rthp)
						break;
				} else {
					*rthp = rth->u.rt_next;
					rt_free(rth);
					goal--;
				}
#else /* CONFIG_IP_ROUTE_MULTIPATH_CACHED */
				*rthp = rth->u.rt_next;
				rt_free(rth);
				goal--;
#endif /* CONFIG_IP_ROUTE_MULTIPATH_CACHED */
			}
			spin_unlock_bh(rt_hash_lock_addr(k));
			if (goal <= 0)
				break;
		}
		rover = k;

		if (goal <= 0)
			goto work_done;

		/* Goal is not achieved. We stop process if:

		   - if expire reduced to zero. Otherwise, expire is halfed.
		   - if table is not full.
		   - if we are called from interrupt.
		   - jiffies check is just fallback/debug loop breaker.
		     We will not spin here for long time in any case.
		 */

		RT_CACHE_STAT_INC(gc_goal_miss);

		if (expire == 0)
			break;

		expire >>= 1;
#if RT_CACHE_DEBUG >= 2
		printk(KERN_DEBUG "expire>> %u %d %d %d\n", expire,
				atomic_read(&ipv4_dst_ops.entries), goal, i);
#endif

		if (atomic_read(&ipv4_dst_ops.entries) < ip_rt_max_size)
			goto out;
	} while (!in_softirq() && time_before_eq(jiffies, now));

	if (atomic_read(&ipv4_dst_ops.entries) < ip_rt_max_size)
		goto out;
	if (net_ratelimit())
		printk(KERN_WARNING "dst cache overflow\n");
	RT_CACHE_STAT_INC(gc_dst_overflow);
	return 1;

work_done:
	expire += ip_rt_gc_min_interval;
	if (expire > ip_rt_gc_timeout ||
	    atomic_read(&ipv4_dst_ops.entries) < ipv4_dst_ops.gc_thresh)
		expire = ip_rt_gc_timeout;
#if RT_CACHE_DEBUG >= 2
	printk(KERN_DEBUG "expire++ %u %d %d %d\n", expire,
			atomic_read(&ipv4_dst_ops.entries), goal, rover);
#endif
out:	return 0;
}

static int rt_intern_hash(unsigned hash, struct rtable *rt, struct rtable **rp)
{
	struct rtable	*rth, **rthp;
	unsigned long	now;
	struct rtable *cand, **candp;
	u32 		min_score;
	int		chain_length;
	int attempts = !in_softirq();

restart:
	chain_length = 0;
	min_score = ~(u32)0;
	cand = NULL;
	candp = NULL;
	now = jiffies;

	rthp = &rt_hash_table[hash].chain;

	spin_lock_bh(rt_hash_lock_addr(hash));
	while ((rth = *rthp) != NULL) {
#ifdef CONFIG_IP_ROUTE_MULTIPATH_CACHED
		if (!(rth->u.dst.flags & DST_BALANCED) &&
		    compare_keys(&rth->fl, &rt->fl)) {
#else
		if (compare_keys(&rth->fl, &rt->fl)) {
#endif
			/* Put it first */
			*rthp = rth->u.rt_next;
			/*
			 * Since lookup is lockfree, the deletion
			 * must be visible to another weakly ordered CPU before
			 * the insertion at the start of the hash chain.
			 */
			rcu_assign_pointer(rth->u.rt_next,
					   rt_hash_table[hash].chain);
			/*
			 * Since lookup is lockfree, the update writes
			 * must be ordered for consistency on SMP.
			 */
			rcu_assign_pointer(rt_hash_table[hash].chain, rth);

			rth->u.dst.__use++;
			dst_hold(&rth->u.dst);
			rth->u.dst.lastuse = now;
			spin_unlock_bh(rt_hash_lock_addr(hash));

			rt_drop(rt);
			*rp = rth;
			return 0;
		}

		if (!atomic_read(&rth->u.dst.__refcnt)) {
			u32 score = rt_score(rth);

			if (score <= min_score) {
				cand = rth;
				candp = rthp;
				min_score = score;
			}
		}

		chain_length++;

		rthp = &rth->u.rt_next;
	}

	if (cand) {
		/* ip_rt_gc_elasticity used to be average length of chain
		 * length, when exceeded gc becomes really aggressive.
		 *
		 * The second limit is less certain. At the moment it allows
		 * only 2 entries per bucket. We will see.
		 */
		if (chain_length > ip_rt_gc_elasticity) {
			*candp = cand->u.rt_next;
			rt_free(cand);
		}
	}

	/* Try to bind route to arp only if it is output
	   route or unicast forwarding path.
	 */
	if (rt->rt_type == RTN_UNICAST || rt->fl.iif == 0) {
		int err = arp_bind_neighbour(&rt->u.dst);
		if (err) {
			spin_unlock_bh(rt_hash_lock_addr(hash));

			if (err != -ENOBUFS) {
				rt_drop(rt);
				return err;
			}

			/* Neighbour tables are full and nothing
			   can be released. Try to shrink route cache,
			   it is most likely it holds some neighbour records.
			 */
			if (attempts-- > 0) {
				int saved_elasticity = ip_rt_gc_elasticity;
				int saved_int = ip_rt_gc_min_interval;
				ip_rt_gc_elasticity	= 1;
				ip_rt_gc_min_interval	= 0;
				rt_garbage_collect();
				ip_rt_gc_min_interval	= saved_int;
				ip_rt_gc_elasticity	= saved_elasticity;
				goto restart;
			}

			if (net_ratelimit())
				printk(KERN_WARNING "Neighbour table overflow.\n");
			rt_drop(rt);
			return -ENOBUFS;
		}
	}

	rt->u.rt_next = rt_hash_table[hash].chain;
#if RT_CACHE_DEBUG >= 2
	if (rt->u.rt_next) {
		struct rtable *trt;
		printk(KERN_DEBUG "rt_cache @%02x: %u.%u.%u.%u", hash,
		       NIPQUAD(rt->rt_dst));
		for (trt = rt->u.rt_next; trt; trt = trt->u.rt_next)
			printk(" . %u.%u.%u.%u", NIPQUAD(trt->rt_dst));
		printk("\n");
	}
#endif
	rt_hash_table[hash].chain = rt;
	spin_unlock_bh(rt_hash_lock_addr(hash));
	*rp = rt;
	return 0;
}

void rt_bind_peer(struct rtable *rt, int create)
{
	static DEFINE_SPINLOCK(rt_peer_lock);
	struct inet_peer *peer;

	peer = inet_getpeer(rt->rt_dst, create);

	spin_lock_bh(&rt_peer_lock);
	if (rt->peer == NULL) {
		rt->peer = peer;
		peer = NULL;
	}
	spin_unlock_bh(&rt_peer_lock);
	if (peer)
		inet_putpeer(peer);
}

/*
 * Peer allocation may fail only in serious out-of-memory conditions.  However
 * we still can generate some output.
 * Random ID selection looks a bit dangerous because we have no chances to
 * select ID being unique in a reasonable period of time.
 * But broken packet identifier may be better than no packet at all.
 */
static void ip_select_fb_ident(struct iphdr *iph)
{
	static DEFINE_SPINLOCK(ip_fb_id_lock);
	static u32 ip_fallback_id;
	u32 salt;

	spin_lock_bh(&ip_fb_id_lock);
	salt = secure_ip_id(ip_fallback_id ^ iph->daddr);
	iph->id = htons(salt & 0xFFFF);
	ip_fallback_id = salt;
	spin_unlock_bh(&ip_fb_id_lock);
}

void __ip_select_ident(struct iphdr *iph, struct dst_entry *dst, int more)
{
	struct rtable *rt = (struct rtable *) dst;

	if (rt) {
		if (rt->peer == NULL)
			rt_bind_peer(rt, 1);

		/* If peer is attached to destination, it is never detached,
		   so that we need not to grab a lock to dereference it.
		 */
		if (rt->peer) {
			iph->id = htons(inet_getid(rt->peer, more));
			return;
		}
	} else
		printk(KERN_DEBUG "rt_bind_peer(0) @%p\n", 
		       __builtin_return_address(0));

	ip_select_fb_ident(iph);
}

static void rt_del(unsigned hash, struct rtable *rt)
{
	struct rtable **rthp;

	spin_lock_bh(rt_hash_lock_addr(hash));
	ip_rt_put(rt);
	for (rthp = &rt_hash_table[hash].chain; *rthp;
	     rthp = &(*rthp)->u.rt_next)
		if (*rthp == rt) {
			*rthp = rt->u.rt_next;
			rt_free(rt);
			break;
		}
	spin_unlock_bh(rt_hash_lock_addr(hash));
}

void ip_rt_redirect(u32 old_gw, u32 daddr, u32 new_gw,
		    u32 saddr, u8 tos, struct net_device *dev)
{
	int i, k;
	struct in_device *in_dev = in_dev_get(dev);
	struct rtable *rth, **rthp;
	u32  skeys[2] = { saddr, 0 };
	int  ikeys[2] = { dev->ifindex, 0 };

	tos &= IPTOS_RT_MASK;

	if (!in_dev)
		return;

	if (new_gw == old_gw || !IN_DEV_RX_REDIRECTS(in_dev)
	    || MULTICAST(new_gw) || BADCLASS(new_gw) || ZERONET(new_gw))
		goto reject_redirect;

	if (!IN_DEV_SHARED_MEDIA(in_dev)) {
		if (!inet_addr_onlink(in_dev, new_gw, old_gw))
			goto reject_redirect;
		if (IN_DEV_SEC_REDIRECTS(in_dev) && ip_fib_check_default(new_gw, dev))
			goto reject_redirect;
	} else {
		if (inet_addr_type(new_gw) != RTN_UNICAST)
			goto reject_redirect;
	}

	for (i = 0; i < 2; i++) {
		for (k = 0; k < 2; k++) {
#ifdef ROUTING_HASH_IMPROVE
//Michael Lu add
                        unsigned hash = rt_hash_code(daddr, skeys[i] ^ (ikeys[k] << 5), 1, 1, tos);
#else
			unsigned hash = rt_hash_code(daddr,
						     skeys[i] ^ (ikeys[k] << 5),
						     tos);
#endif

			rthp=&rt_hash_table[hash].chain;

			rcu_read_lock();
			while ((rth = rcu_dereference(*rthp)) != NULL) {
				struct rtable *rt;

				if (rth->fl.fl4_dst != daddr ||
				    rth->fl.fl4_src != skeys[i] ||
				    rth->fl.fl4_tos != tos ||
				    rth->fl.oif != ikeys[k] ||
				    rth->fl.iif != 0) {
					rthp = &rth->u.rt_next;
					continue;
				}

				if (rth->rt_dst != daddr ||
				    rth->rt_src != saddr ||
				    rth->u.dst.error ||
				    rth->rt_gateway != old_gw ||
				    rth->u.dst.dev != dev)
					break;

				dst_hold(&rth->u.dst);
				rcu_read_unlock();

				rt = dst_alloc(&ipv4_dst_ops);
				if (rt == NULL) {
					ip_rt_put(rth);
					in_dev_put(in_dev);
					return;
				}

				/* Copy all the information. */
				*rt = *rth;
 				INIT_RCU_HEAD(&rt->u.dst.rcu_head);
				rt->u.dst.__use		= 1;
				atomic_set(&rt->u.dst.__refcnt, 1);
				rt->u.dst.child		= NULL;
				if (rt->u.dst.dev)
					dev_hold(rt->u.dst.dev);
				if (rt->idev)
					in_dev_hold(rt->idev);
				rt->u.dst.obsolete	= 0;
				rt->u.dst.lastuse	= jiffies;
				rt->u.dst.path		= &rt->u.dst;
				rt->u.dst.neighbour	= NULL;
				rt->u.dst.hh		= NULL;
				rt->u.dst.xfrm		= NULL;

				rt->rt_flags		|= RTCF_REDIRECTED;

				/* Gateway is different ... */
				rt->rt_gateway		= new_gw;

				/* Redirect received -> path was valid */
				dst_confirm(&rth->u.dst);

				if (rt->peer)
					atomic_inc(&rt->peer->refcnt);

				if (arp_bind_neighbour(&rt->u.dst) ||
				    !(rt->u.dst.neighbour->nud_state &
					    NUD_VALID)) {
					if (rt->u.dst.neighbour)
						neigh_event_send(rt->u.dst.neighbour, NULL);
					ip_rt_put(rth);
					rt_drop(rt);
					goto do_next;
				}

				rt_del(hash, rth);
				if (!rt_intern_hash(hash, rt, &rt))
					ip_rt_put(rt);
				goto do_next;
			}
			rcu_read_unlock();
		do_next:
			;
		}
	}
	in_dev_put(in_dev);
	return;

reject_redirect:
#ifdef CONFIG_IP_ROUTE_VERBOSE
	if (IN_DEV_LOG_MARTIANS(in_dev) && net_ratelimit())
		printk(KERN_INFO "Redirect from %u.%u.%u.%u on %s about "
			"%u.%u.%u.%u ignored.\n"
			"  Advised path = %u.%u.%u.%u -> %u.%u.%u.%u, "
			"tos %02x\n",
		       NIPQUAD(old_gw), dev->name, NIPQUAD(new_gw),
		       NIPQUAD(saddr), NIPQUAD(daddr), tos);
#endif
	in_dev_put(in_dev);
}

static struct dst_entry *ipv4_negative_advice(struct dst_entry *dst)
{
	struct rtable *rt = (struct rtable*)dst;
	struct dst_entry *ret = dst;

	if (rt) {
		if (dst->obsolete) {
			ip_rt_put(rt);
			ret = NULL;
		} else if ((rt->rt_flags & RTCF_REDIRECTED) ||
			   rt->u.dst.expires) {
#ifdef ROUTING_HASH_IMPROVE
//Michael Lu add
unsigned hash = rt_hash_code(rt->fl.fl4_dst, rt->fl.fl4_src ^ (rt->fl.oif << 5), rt->fl.fl4_dst_port, rt->fl.fl4_src_port, rt->fl.fl4_tos);
#else
unsigned hash = rt_hash_code(rt->fl.fl4_dst, rt->fl.fl4_src ^ (rt->fl.oif << 5), rt->fl.fl4_tos);
#endif

#if RT_CACHE_DEBUG >= 1
			printk(KERN_DEBUG "ip_rt_advice: redirect to "
					  "%u.%u.%u.%u/%02x dropped\n",
				NIPQUAD(rt->rt_dst), rt->fl.fl4_tos);
#endif
			rt_del(hash, rt);
			ret = NULL;
		}
	}
	return ret;
}

/*
 * Algorithm:
 *	1. The first ip_rt_redirect_number redirects are sent
 *	   with exponential backoff, then we stop sending them at all,
 *	   assuming that the host ignores our redirects.
 *	2. If we did not see packets requiring redirects
 *	   during ip_rt_redirect_silence, we assume that the host
 *	   forgot redirected route and start to send redirects again.
 *
 * This algorithm is much cheaper and more intelligent than dumb load limiting
 * in icmp.c.
 *
 * NOTE. Do not forget to inhibit load limiting for redirects (redundant)
 * and "frag. need" (breaks PMTU discovery) in icmp.c.
 */

void ip_rt_send_redirect(struct sk_buff *skb)
{
	struct rtable *rt = (struct rtable*)skb->dst;
	struct in_device *in_dev = in_dev_get(rt->u.dst.dev);

	if (!in_dev)
		return;

	if (!IN_DEV_TX_REDIRECTS(in_dev))
		goto out;

	/* No redirected packets during ip_rt_redirect_silence;
	 * reset the algorithm.
	 */
	if (time_after(jiffies, rt->u.dst.rate_last + ip_rt_redirect_silence))
		rt->u.dst.rate_tokens = 0;

	/* Too many ignored redirects; do not send anything
	 * set u.dst.rate_last to the last seen redirected packet.
	 */
	if (rt->u.dst.rate_tokens >= ip_rt_redirect_number) {
		rt->u.dst.rate_last = jiffies;
		goto out;
	}

	/* Check for load limit; set rate_last to the latest sent
	 * redirect.
	 */
	if (time_after(jiffies,
		       (rt->u.dst.rate_last +
			(ip_rt_redirect_load << rt->u.dst.rate_tokens)))) {
		icmp_send(skb, ICMP_REDIRECT, ICMP_REDIR_HOST, rt->rt_gateway);
		rt->u.dst.rate_last = jiffies;
		++rt->u.dst.rate_tokens;
#ifdef CONFIG_IP_ROUTE_VERBOSE
		if (IN_DEV_LOG_MARTIANS(in_dev) &&
		    rt->u.dst.rate_tokens == ip_rt_redirect_number &&
		    net_ratelimit())
			printk(KERN_WARNING "host %u.%u.%u.%u/if%d ignores "
				"redirects for %u.%u.%u.%u to %u.%u.%u.%u.\n",
				NIPQUAD(rt->rt_src), rt->rt_iif,
				NIPQUAD(rt->rt_dst), NIPQUAD(rt->rt_gateway));
#endif
	}
out:
        in_dev_put(in_dev);
}

static int ip_error(struct sk_buff *skb)
{
	struct rtable *rt = (struct rtable*)skb->dst;
	unsigned long now;
	int code;

	switch (rt->u.dst.error) {
		case EINVAL:
		default:
			goto out;
		case EHOSTUNREACH:
			code = ICMP_HOST_UNREACH;
			break;
		case ENETUNREACH:
			code = ICMP_NET_UNREACH;
			break;
		case EACCES:
			code = ICMP_PKT_FILTERED;
			break;
	}

	now = jiffies;
	rt->u.dst.rate_tokens += now - rt->u.dst.rate_last;
	if (rt->u.dst.rate_tokens > ip_rt_error_burst)
		rt->u.dst.rate_tokens = ip_rt_error_burst;
	rt->u.dst.rate_last = now;
	if (rt->u.dst.rate_tokens >= ip_rt_error_cost) {
		rt->u.dst.rate_tokens -= ip_rt_error_cost;
		icmp_send(skb, ICMP_DEST_UNREACH, code, 0);
	}

out:	kfree_skb(skb);
	return 0;
} 

/*
 *	The last two values are not from the RFC but
 *	are needed for AMPRnet AX.25 paths.
 */

static const unsigned short mtu_plateau[] =
{32000, 17914, 8166, 4352, 2002, 1492, 576, 296, 216, 128 };

static __inline__ unsigned short guess_mtu(unsigned short old_mtu)
{
	int i;
	
	for (i = 0; i < ARRAY_SIZE(mtu_plateau); i++)
		if (old_mtu > mtu_plateau[i])
			return mtu_plateau[i];
	return 68;
}

unsigned short ip_rt_frag_needed(struct iphdr *iph, unsigned short new_mtu)
{
	int i;
	unsigned short old_mtu = ntohs(iph->tot_len);
	struct rtable *rth;
	u32  skeys[2] = { iph->saddr, 0, };
	u32  daddr = iph->daddr;
	u8   tos = iph->tos & IPTOS_RT_MASK;
	unsigned short est_mtu = 0;

	if (ipv4_config.no_pmtu_disc)
		return 0;

	for (i = 0; i < 2; i++) {
#ifdef ROUTING_HASH_IMPROVE
//Michael Lu add
		unsigned hash = rt_hash_code(daddr, skeys[i], 1, 1, tos);
#else
		unsigned hash = rt_hash_code(daddr, skeys[i], tos);
#endif

		rcu_read_lock();
		for (rth = rcu_dereference(rt_hash_table[hash].chain); rth;
		     rth = rcu_dereference(rth->u.rt_next)) {
			if (rth->fl.fl4_dst == daddr &&
			    rth->fl.fl4_src == skeys[i] &&
			    rth->rt_dst  == daddr &&
			    rth->rt_src  == iph->saddr &&
			    rth->fl.fl4_tos == tos &&
			    rth->fl.iif == 0 &&
			    !(dst_metric_locked(&rth->u.dst, RTAX_MTU))) {
				unsigned short mtu = new_mtu;

				if (new_mtu < 68 || new_mtu >= old_mtu) {

					/* BSD 4.2 compatibility hack :-( */
					if (mtu == 0 &&
					    old_mtu >= rth->u.dst.metrics[RTAX_MTU-1] &&
					    old_mtu >= 68 + (iph->ihl << 2))
						old_mtu -= iph->ihl << 2;

					mtu = guess_mtu(old_mtu);
				}
				if (mtu <= rth->u.dst.metrics[RTAX_MTU-1]) {
					if (mtu < rth->u.dst.metrics[RTAX_MTU-1]) { 
						dst_confirm(&rth->u.dst);
						if (mtu < ip_rt_min_pmtu) {
							mtu = ip_rt_min_pmtu;
							rth->u.dst.metrics[RTAX_LOCK-1] |=
								(1 << RTAX_MTU);
						}
						rth->u.dst.metrics[RTAX_MTU-1] = mtu;
						dst_set_expires(&rth->u.dst,
							ip_rt_mtu_expires);
					}
					est_mtu = mtu;
				}
			}
		}
		rcu_read_unlock();
	}
	return est_mtu ? : new_mtu;
}

static void ip_rt_update_pmtu(struct dst_entry *dst, u32 mtu)
{
	if (dst->metrics[RTAX_MTU-1] > mtu && mtu >= 68 &&
	    !(dst_metric_locked(dst, RTAX_MTU))) {
		if (mtu < ip_rt_min_pmtu) {
			mtu = ip_rt_min_pmtu;
			dst->metrics[RTAX_LOCK-1] |= (1 << RTAX_MTU);
		}
		dst->metrics[RTAX_MTU-1] = mtu;
		dst_set_expires(dst, ip_rt_mtu_expires);
	}
}

static struct dst_entry *ipv4_dst_check(struct dst_entry *dst, u32 cookie)
{
	return NULL;
}

static void ipv4_dst_destroy(struct dst_entry *dst)
{
	struct rtable *rt = (struct rtable *) dst;
	struct inet_peer *peer = rt->peer;
	struct in_device *idev = rt->idev;

	if (peer) {
		rt->peer = NULL;
		inet_putpeer(peer);
	}

	if (idev) {
		rt->idev = NULL;
		in_dev_put(idev);
	}
}

static void ipv4_dst_ifdown(struct dst_entry *dst, struct net_device *dev,
			    int how)
{
	struct rtable *rt = (struct rtable *) dst;
	struct in_device *idev = rt->idev;
	if (dev != &loopback_dev && idev && idev->dev == dev) {
		struct in_device *loopback_idev = in_dev_get(&loopback_dev);
		if (loopback_idev) {
			rt->idev = loopback_idev;
			in_dev_put(idev);
		}
	}
}

static void ipv4_link_failure(struct sk_buff *skb)
{
	struct rtable *rt;

	icmp_send(skb, ICMP_DEST_UNREACH, ICMP_HOST_UNREACH, 0);

	rt = (struct rtable *) skb->dst;
	if (rt)
		dst_set_expires(&rt->u.dst, 0);
}

static int ip_rt_bug(struct sk_buff *skb)
{
	printk(KERN_DEBUG "ip_rt_bug: %u.%u.%u.%u -> %u.%u.%u.%u, %s\n",
		NIPQUAD(skb->nh.iph->saddr), NIPQUAD(skb->nh.iph->daddr),
		skb->dev ? skb->dev->name : "?");
	kfree_skb(skb);
	return 0;
}

/*
   We do not cache source address of outgoing interface,
   because it is used only by IP RR, TS and SRR options,
   so that it out of fast path.

   BTW remember: "addr" is allowed to be not aligned
   in IP options!
 */

void ip_rt_get_source(u8 *addr, struct rtable *rt)
{
	u32 src;
	struct fib_result res;

	if (rt->fl.iif == 0)
		src = rt->rt_src;
	else if (fib_lookup(&rt->fl, &res) == 0) {
		src = FIB_RES_PREFSRC(res);
		fib_res_put(&res);
	} else
		src = inet_select_addr(rt->u.dst.dev, rt->rt_gateway,
					RT_SCOPE_UNIVERSE);
	memcpy(addr, &src, 4);
}

#ifdef CONFIG_NET_CLS_ROUTE
static void set_class_tag(struct rtable *rt, u32 tag)
{
	if (!(rt->u.dst.tclassid & 0xFFFF))
		rt->u.dst.tclassid |= tag & 0xFFFF;
	if (!(rt->u.dst.tclassid & 0xFFFF0000))
		rt->u.dst.tclassid |= tag & 0xFFFF0000;
}
#endif

static void rt_set_nexthop(struct rtable *rt, struct fib_result *res, u32 itag)
{
	struct fib_info *fi = res->fi;

	if (fi) {
		if (FIB_RES_GW(*res) &&
		    FIB_RES_NH(*res).nh_scope == RT_SCOPE_LINK)
			rt->rt_gateway = FIB_RES_GW(*res);
		memcpy(rt->u.dst.metrics, fi->fib_metrics,
		       sizeof(rt->u.dst.metrics));
		if (fi->fib_mtu == 0) {
			rt->u.dst.metrics[RTAX_MTU-1] = rt->u.dst.dev->mtu;
			if (rt->u.dst.metrics[RTAX_LOCK-1] & (1 << RTAX_MTU) &&
			    rt->rt_gateway != rt->rt_dst &&
			    rt->u.dst.dev->mtu > 576)
				rt->u.dst.metrics[RTAX_MTU-1] = 576;
		}
#ifdef CONFIG_NET_CLS_ROUTE
		rt->u.dst.tclassid = FIB_RES_NH(*res).nh_tclassid;
#endif
	} else
		rt->u.dst.metrics[RTAX_MTU-1]= rt->u.dst.dev->mtu;

	if (rt->u.dst.metrics[RTAX_HOPLIMIT-1] == 0)
		rt->u.dst.metrics[RTAX_HOPLIMIT-1] = sysctl_ip_default_ttl;
	if (rt->u.dst.metrics[RTAX_MTU-1] > IP_MAX_MTU)
		rt->u.dst.metrics[RTAX_MTU-1] = IP_MAX_MTU;
	if (rt->u.dst.metrics[RTAX_ADVMSS-1] == 0)
		rt->u.dst.metrics[RTAX_ADVMSS-1] = max_t(unsigned int, rt->u.dst.dev->mtu - 40,
				       ip_rt_min_advmss);
	if (rt->u.dst.metrics[RTAX_ADVMSS-1] > 65535 - 40)
		rt->u.dst.metrics[RTAX_ADVMSS-1] = 65535 - 40;

#ifdef CONFIG_NET_CLS_ROUTE
#ifdef CONFIG_IP_MULTIPLE_TABLES
	set_class_tag(rt, fib_rules_tclass(res));
#endif
	set_class_tag(rt, itag);
#endif
        rt->rt_type = res->type;
}

#ifdef ROUTING_HASH_IMPROVE
//Michael Lu add
static int ip_route_input_mc(struct sk_buff *skb, u32 daddr, u32 saddr, u32 dst_port, u32 src_port,
				u8 tos, struct net_device *dev, int our)
#else
static int ip_route_input_mc(struct sk_buff *skb, u32 daddr, u32 saddr,
				u8 tos, struct net_device *dev, int our)
#endif
{
	unsigned hash;
	struct rtable *rth;
	u32 spec_dst;
	struct in_device *in_dev = in_dev_get(dev);
	u32 itag = 0;

	/* Primary sanity checks. */

	if (in_dev == NULL)
		return -EINVAL;

	if (MULTICAST(saddr) || BADCLASS(saddr) || LOOPBACK(saddr) ||
	    skb->protocol != htons(ETH_P_IP))
		goto e_inval;

	if (ZERONET(saddr)) {
		if (!LOCAL_MCAST(daddr))
			goto e_inval;
		spec_dst = inet_select_addr(dev, 0, RT_SCOPE_LINK);
	} else if (fib_validate_source(saddr, 0, tos, 0,
					dev, &spec_dst, &itag) < 0)
		goto e_inval;

	rth = dst_alloc(&ipv4_dst_ops);
	if (!rth)
		goto e_nobufs;

	rth->u.dst.output= ip_rt_bug;

	atomic_set(&rth->u.dst.__refcnt, 1);
	rth->u.dst.flags= DST_HOST;
	if (in_dev->cnf.no_policy)
		rth->u.dst.flags |= DST_NOPOLICY;
	rth->fl.fl4_dst	= daddr;
	rth->rt_dst	= daddr;
	rth->fl.fl4_tos	= tos;
#ifdef CONFIG_IP_ROUTE_FWMARK
	rth->fl.fl4_fwmark= skb->nfmark;
#endif
	rth->fl.fl4_src	= saddr;
	rth->rt_src	= saddr;
#ifdef CONFIG_NET_CLS_ROUTE
	rth->u.dst.tclassid = itag;
#endif
	rth->rt_iif	=
	rth->fl.iif	= dev->ifindex;
	rth->u.dst.dev	= &loopback_dev;
	dev_hold(rth->u.dst.dev);
	rth->idev	= in_dev_get(rth->u.dst.dev);
	rth->fl.oif	= 0;
	rth->rt_gateway	= daddr;
	rth->rt_spec_dst= spec_dst;
	rth->rt_type	= RTN_MULTICAST;
	rth->rt_flags	= RTCF_MULTICAST;
	if (our) {
		rth->u.dst.input= ip_local_deliver;
		rth->rt_flags |= RTCF_LOCAL;
	}

#ifdef CONFIG_IP_MROUTE
	if (!LOCAL_MCAST(daddr) && IN_DEV_MFORWARD(in_dev))
		rth->u.dst.input = ip_mr_input;
#endif
	RT_CACHE_STAT_INC(in_slow_mc);

	in_dev_put(in_dev);
#ifdef ROUTING_HASH_IMPROVE
//Michael Lu add
        hash = rt_hash_code(daddr, saddr ^ (dev->ifindex << 5), dst_port, src_port, tos);
#else
	hash = rt_hash_code(daddr, saddr ^ (dev->ifindex << 5), tos);
#endif
	return rt_intern_hash(hash, rth, (struct rtable**) &skb->dst);

e_nobufs:
	in_dev_put(in_dev);
	return -ENOBUFS;

e_inval:
	in_dev_put(in_dev);
	return -EINVAL;
}


static void ip_handle_martian_source(struct net_device *dev,
				     struct in_device *in_dev,
				     struct sk_buff *skb,
				     u32 daddr,
				     u32 saddr) 
{
	RT_CACHE_STAT_INC(in_martian_src);
#ifdef CONFIG_IP_ROUTE_VERBOSE
	if (IN_DEV_LOG_MARTIANS(in_dev) && net_ratelimit()) {
		/*
		 *	RFC1812 recommendation, if source is martian,
		 *	the only hint is MAC header.
		 */
		printk(KERN_WARNING "martian source %u.%u.%u.%u from "
			"%u.%u.%u.%u, on dev %s\n",
			NIPQUAD(daddr), NIPQUAD(saddr), dev->name);
		if (dev->hard_header_len && skb->mac.raw) {
			int i;
			unsigned char *p = skb->mac.raw;
			printk(KERN_WARNING "ll header: ");
			for (i = 0; i < dev->hard_header_len; i++, p++) {
				printk("%02x", *p);
				if (i < (dev->hard_header_len - 1))
					printk(":");
			}
			printk("\n");
		}
	}
#endif
}

static inline int __mkroute_input(struct sk_buff *skb, 
				  struct fib_result* res, 
				  struct in_device *in_dev, 
				  u32 daddr, u32 saddr, u32 tos, 
				  struct rtable **result) 
{

	struct rtable *rth;
	int err;
	struct in_device *out_dev;
	unsigned flags = 0;
	u32 spec_dst, itag;

	/* get a working reference to the output device */
	out_dev = in_dev_get(FIB_RES_DEV(*res));
	if (out_dev == NULL) {
		if (net_ratelimit())
			printk(KERN_CRIT "Bug in ip_route_input" \
			       "_slow(). Please, report\n");
		return -EINVAL;
	}


	err = fib_validate_source(saddr, daddr, tos, FIB_RES_OIF(*res), 
				  in_dev->dev, &spec_dst, &itag);
	if (err < 0) {
		ip_handle_martian_source(in_dev->dev, in_dev, skb, daddr, 
					 saddr);
		
		err = -EINVAL;
		goto cleanup;
	}

	if (err)
		flags |= RTCF_DIRECTSRC;

	if (out_dev == in_dev && err && !(flags & (RTCF_NAT | RTCF_MASQ)) &&
	    (IN_DEV_SHARED_MEDIA(out_dev) ||
	     inet_addr_onlink(out_dev, saddr, FIB_RES_GW(*res))))
		flags |= RTCF_DOREDIRECT;

	if (skb->protocol != htons(ETH_P_IP)) {
		/* Not IP (i.e. ARP). Do not create route, if it is
		 * invalid for proxy arp. DNAT routes are always valid.
		 */
		if (out_dev == in_dev && !(flags & RTCF_DNAT)) {
			err = -EINVAL;
			goto cleanup;
		}
	}


	rth = dst_alloc(&ipv4_dst_ops);
	if (!rth) {
		err = -ENOBUFS;
		goto cleanup;
	}

	atomic_set(&rth->u.dst.__refcnt, 1);
	rth->u.dst.flags= DST_HOST;
#ifdef CONFIG_IP_ROUTE_MULTIPATH_CACHED
	if (res->fi->fib_nhs > 1)
		rth->u.dst.flags |= DST_BALANCED;
#endif
	if (in_dev->cnf.no_policy)
		rth->u.dst.flags |= DST_NOPOLICY;
	if (in_dev->cnf.no_xfrm)
		rth->u.dst.flags |= DST_NOXFRM;
	rth->fl.fl4_dst	= daddr;
	rth->rt_dst	= daddr;
	rth->fl.fl4_tos	= tos;
#ifdef CONFIG_IP_ROUTE_FWMARK
	rth->fl.fl4_fwmark= skb->nfmark;
#endif
	rth->fl.fl4_src	= saddr;
	rth->rt_src	= saddr;
	rth->rt_gateway	= daddr;
	rth->rt_iif 	=
		rth->fl.iif	= in_dev->dev->ifindex;
	rth->u.dst.dev	= (out_dev)->dev;
	dev_hold(rth->u.dst.dev);
	rth->idev	= in_dev_get(rth->u.dst.dev);
	rth->fl.oif 	= 0;
	rth->rt_spec_dst= spec_dst;

	rth->u.dst.input = ip_forward;
	rth->u.dst.output = ip_output;

	rt_set_nexthop(rth, res, itag);

	rth->rt_flags = flags;

	*result = rth;
	err = 0;
 cleanup:
	/* release the working reference to the output device */
	in_dev_put(out_dev);
	return err;
}						

#ifdef CONFIG_NK_IP_BASE
static inline int ip_mkroute_input_def(struct sk_buff *skb, 
				       struct fib_result* res, 
				       const struct flowi *fl,
				       struct in_device *in_dev,
				       u32 daddr, u32 saddr, u32 tos, int notmultipath)
#else
static inline int ip_mkroute_input_def(struct sk_buff *skb, 
				       struct fib_result* res, 
				       const struct flowi *fl,
				       struct in_device *in_dev,
				       u32 daddr, u32 saddr, u32 tos)
#endif
{
	struct rtable* rth = NULL;
	int err;
	unsigned hash;

#ifdef CONFIG_IP_ROUTE_MULTIPATH
  #ifdef CONFIG_NK_IP_BASE
	if (!notmultipath && res->fi && res->fi->fib_nhs > 1 && fl->oif == 0)
  #else
	if (res->fi && res->fi->fib_nhs > 1 && fl->oif == 0)
  #endif
		fib_select_multipath(fl, res);
#endif

	/* create a routing cache entry */
	err = __mkroute_input(skb, res, in_dev, daddr, saddr, tos, &rth);
	if (err)
		return err;

	/*2006/11/1 trenchen : support protocol binding*/
#ifdef CONFIG_NK_PROTO_BINDING
	rth->fl.fl4_protocol_type = fl->fl4_protocol_type;
	rth->fl.fl4_src_port = fl->fl4_src_port;
	rth->fl.fl4_dst_port = fl->fl4_dst_port;
#endif

	/* put it into the cache */
#ifdef ROUTING_HASH_IMPROVE
//Michael Lu add
	hash = rt_hash_code(daddr, saddr ^ (fl->iif << 5), fl->fl4_dst_port, fl->fl4_src_port, tos);
#else
	hash = rt_hash_code(daddr, saddr ^ (fl->iif << 5), tos);
#endif
	return rt_intern_hash(hash, rth, (struct rtable**)&skb->dst);	
}

#ifdef CONFIG_NK_IP_BASE
static inline int ip_mkroute_input(struct sk_buff *skb, 
				   struct fib_result* res, 
				   const struct flowi *fl,
				   struct in_device *in_dev,
				   u32 daddr, u32 saddr, u32 tos, int notmultipath)
#else
static inline int ip_mkroute_input(struct sk_buff *skb, 
				   struct fib_result* res, 
				   const struct flowi *fl,
				   struct in_device *in_dev,
				   u32 daddr, u32 saddr, u32 tos)
#endif
{
#ifdef CONFIG_IP_ROUTE_MULTIPATH_CACHED
	struct rtable* rth = NULL, *rtres;
	unsigned char hop, hopcount;
	int err = -EINVAL;
	unsigned int hash;

	if (res->fi)
		hopcount = res->fi->fib_nhs;
	else
		hopcount = 1;

	/* distinguish between multipath and singlepath */
	if (hopcount < 2)
#ifdef CONFIG_NK_IP_BASE
		return ip_mkroute_input_def(skb, res, fl, in_dev, daddr,
					    saddr, tos, notmultipath);
#else
		return ip_mkroute_input_def(skb, res, fl, in_dev, daddr,
					    saddr, tos);
#endif
	
	/* add all alternatives to the routing cache */
	for (hop = 0; hop < hopcount; hop++) {
		res->nh_sel = hop;

		/* put reference to previous result */
		if (hop)
			ip_rt_put(rtres);

		/* create a routing cache entry */
		err = __mkroute_input(skb, res, in_dev, daddr, saddr, tos,
				      &rth);
		if (err)
			return err;

		/* put it into the cache */
#ifdef ROUTING_HASH_IMPROVE
//Michael Lu add
                hash = rt_hash_code(daddr, saddr ^ (fl->iif << 5), fl->fl4_dst_port, fl->fl4_src_port, tos);
#else
		hash = rt_hash_code(daddr, saddr ^ (fl->iif << 5), tos);
#endif
		err = rt_intern_hash(hash, rth, &rtres);
		if (err)
			return err;

		/* forward hop information to multipath impl. */
		multipath_set_nhinfo(rth,
				     FIB_RES_NETWORK(*res),
				     FIB_RES_NETMASK(*res),
				     res->prefixlen,
				     &FIB_RES_NH(*res));
	}
	skb->dst = &rtres->u.dst;
	return err;
#else /* CONFIG_IP_ROUTE_MULTIPATH_CACHED  */

#ifdef CONFIG_NK_IP_BASE
	return ip_mkroute_input_def(skb, res, fl, in_dev, daddr, saddr, tos, notmultipath);
#else
	return ip_mkroute_input_def(skb, res, fl, in_dev, daddr, saddr, tos);
#endif

#endif /* CONFIG_IP_ROUTE_MULTIPATH_CACHED  */
}


#ifdef CONFIG_NK_PROTO_BINDING
/***2007/6/5 trenchen : bind ftp port 20,21 to the same interface. copy from SR1****/
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
} *fire_spec_output_head = 0;

EXPORT_SYMBOL(fire_spec_output_head);
//TT rwlock_t FireSpecLock = RW_LOCK_UNLOCKED;
DEFINE_RWLOCK(FireSpecLock);

struct net_device * fire_spec_search(u32 saddr,u32 daddr,u32 protocol_type,u32 src_port,u32 dst_port)
{
    struct fire_spec_output *temp, **temp_p;
    struct net_device *dev;

    //printk(KERN_INFO "fire_spec_search : search src[%x] dst[%x] protocol[%d] srcport[%d] dstport[%d]\n",saddr,daddr,protocol_type,src_port,dst_port);
    write_lock_bh(&FireSpecLock);
    for(temp_p=&fire_spec_output_head; temp=*temp_p ; temp_p=&((*temp_p)->next)) {
	//printk(KERN_INFO "src[%x] dst[%x] protocol[%d] srcport[%d] dstport[%d]\n",temp->saddr,temp->daddr,temp->protocol_type,temp->src_port,temp->dst_port);
       if( saddr==temp->saddr && daddr==temp->daddr
           && ( !temp->protocol_type || protocol_type==temp->protocol_type)
	   && ( !temp->src_port || src_port==temp->src_port)
	   && ( !temp->dst_port || dst_port==temp->dst_port)
          )  {
	     //printk(KERN_INFO "fire_spec_search : src[%x] dst[%x] protocol[%d] srcport[%d] dstport[%d] dev[%s]\n",saddr,daddr,protocol_type,src_port,dst_port,temp->dev->name);
             dev = temp->dev;
             *temp_p = (*temp_p)->next;
	     write_unlock_bh(&FireSpecLock);
             kfree(temp);
	     return dev;
	  }
    }
    //printk(KERN_INFO "fire_spec_search : Can not find out the dev\n");
    write_unlock_bh(&FireSpecLock);
    return NULL;
}

void fire_spec_aging(void)
{
	struct fire_spec_output *temp, **temp_p;

	/* purpose     : Routing    author : David    date : 2010-08-03        */
	/* description : Add spin lock to fix crash issue        .             */
	write_lock_bh(&FireSpecLock);
	//printk("fire_spec_aging ==========\n");
	for(temp_p=&fire_spec_output_head; temp=*temp_p; )
	{
		temp->age-=10;
		if( temp->age>0 )
		{
			temp_p=&(temp->next);
			continue;
		}
       //printk("timer del: src[%x] dst[%x] protocol[%d] srcport[%d] dstport[%d]\n",temp->saddr,temp->daddr,temp->protocol_type,temp->src_port,temp->dst_port);
		*temp_p = temp->next;
		kfree(temp);
	}
	write_unlock_bh(&FireSpecLock);
	
	fire_spec_timer.expires = jiffies + 10*HZ;
	add_timer(&fire_spec_timer);
}

/*******************************************************************/
#endif




/*
 *	NOTE. We drop all the packets that has local source
 *	addresses, because every properly looped back packet
 *	must have correct destination already attached by output routine.
 *
 *	Such approach solves two big problems:
 *	1. Not simplex devices are handled properly.
 *	2. IP spoofing attempts are filtered with 100% of guarantee.
 */

static int ip_route_input_slow(struct sk_buff *skb, u32 daddr, u32 saddr,
			       u8 tos, struct net_device *dev)
{
	struct fib_result res;
	struct in_device *in_dev = in_dev_get(dev);
	struct flowi fl = { .nl_u = { .ip4_u =
				      { .daddr = daddr,
					.saddr = saddr,
					.tos = tos,
					.scope = RT_SCOPE_UNIVERSE,
#ifdef CONFIG_IP_ROUTE_FWMARK
					.fwmark = skb->nfmark
#endif
				      } },
			    .iif = dev->ifindex, 
			    .wan2lan = 0};/*2007/12/26 trenchen : bug fix, protocol binding src ip 192.168.0~0->0.0.0.0~255.255.255.255 the range may cover packet from want to lan make packet be binded to wan error*/
	unsigned	flags = 0;
	u32		itag = 0;
	struct rtable * rth;
	unsigned	hash;
	u32		spec_dst;
	int		err = -EINVAL;
	int		free_res = 0;
//-->add by Michael Lu 2008/10
	int	dst_index = -1;
//<--add by Michael Lu 2008/10

#ifdef CONFIG_NK_IP_BASE
	int	notmultipath = 0;
#endif

#ifdef CONFIG_NK_PROTO_BINDING
	/*2006/11/1 trenchen : protocol binding*/
	struct iphdr *iphdr = NULL;
	struct tcphdr *tcphdr = NULL;
	struct udphdr *udphdr = NULL;
	struct icmphdr *icmphdr = NULL;/* support ICMP -- incifer 2008/10 */
        /*******************************/
#endif
	struct net_device *out_device = NULL;	/*2006/11/1 trenchen : modify for specify out device*/

	/* IP on this device is disabled. */

	if (!in_dev)
		goto out;

#ifdef CONFIG_NK_PROTO_BINDING
	/*2006/11/1 trenchen : protocol binding*/
	iphdr = skb->nh.iph;

	if (iphdr->protocol == IPPROTO_TCP) {
	        tcphdr = (struct tcphdr *)((u_int32_t *)iphdr + iphdr->ihl);
	        if (tcphdr) {
	              fl.fl4_protocol_type = IPPROTO_TCP;
	              fl.fl4_src_port = tcphdr->source;
	              fl.fl4_dst_port = tcphdr->dest;
	        }
	} else if (iphdr->protocol == IPPROTO_UDP) {
	        udphdr = (struct udphdr *)((u_int32_t *)iphdr + iphdr->ihl);
		if (udphdr) {
		       fl.fl4_protocol_type = IPPROTO_UDP;
		       fl.fl4_src_port =udphdr->source;
		       fl.fl4_dst_port =udphdr->dest;
                }
	} else if (iphdr->protocol == IPPROTO_ICMP) {/* support ICMP -- incifer 2008/10 */
	        icmphdr = (struct icmphdr *)((u_int32_t *)iphdr + iphdr->ihl);
		if (icmphdr) {
		       fl.fl4_protocol_type = IPPROTO_ICMP;
		       fl.fl4_src_port =0;
		       fl.fl4_dst_port =0;
                }
	}
#ifdef CONFIG_NK_IPSEC_MULTIPLE_PASS_THROUGH
	else if (iphdr->protocol == IPPROTO_ESP)
	{
		fl.fl4_protocol_type = IPPROTO_ESP;
		fl.fl4_src_port = 0;
		fl.fl4_dst_port = 0;
	}
#endif
	else {
		fl.fl4_protocol_type = 0;
		fl.fl4_src_port = 0;
		fl.fl4_dst_port = 0;
	}
	/****************************************************/
#endif

	/* Check for the most weird martians, which can be not detected
	   by fib_lookup.
	 */

	if (MULTICAST(saddr) || BADCLASS(saddr) || LOOPBACK(saddr))
		goto martian_source;

	if (daddr == 0xFFFFFFFF || (saddr == 0 && daddr == 0))
		goto brd_input;

	/* Accept zero addresses only to limited broadcast;
	 * I even do not know to fix it or not. Waiting for complains :-)
	 */
	if (ZERONET(saddr))
		goto martian_source;

	if (BADCLASS(daddr) || ZERONET(daddr) || LOOPBACK(daddr))
		goto martian_destination;

	/*2006/11/1 trenchen : support remote manage*/
#ifdef CONFIG_NK_PROTO_BINDING
	out_device=nk_multipath_select(daddr, saddr, fl.fl4_protocol_type, fl.fl4_dst_port, fl.fl4_src_port, tos);
#else
	out_device=nk_multipath_select(daddr, saddr, 0, 0, 0, tos);
#endif
	if(out_device && !strcmp(out_device->name,NK_LAN_IF))
		out_device = NULL;
	/***********************************************/

#ifdef CONFIG_NK_PROTO_BINDING
	/***2007/6/5 trenchen : bind ftp port 20,21 to the same interface. copy from SR1****/
	if(!out_device && fire_spec_output_head) {
		out_device = fire_spec_search(saddr, daddr, fl.fl4_protocol_type, fl.fl4_src_port, fl.fl4_dst_port);
	}
#endif

	/*2007/12/26 trenchen : bug fix, protocol binding src ip 192.168.0~0->0.0.0.0~255.255.255.255 the range may cover packet from want to lan make packet be binded to wan error*/
	if( strcmp(in_dev->dev->name,NK_LAN_IF) ){
		fl.wan2lan = 1;
	}


	/*
	 *	Now we are ready to route packet.
	 */
#if 1
	if ((err = nk_fib_lookup(&fl, &res, out_device)) != 0) {
#else
	if ((err = fib_lookup(&fl, &res)) != 0) {
#endif
		if (!IN_DEV_FORWARD(in_dev))
			goto e_hostunreach;
		goto no_route;
	}
	free_res = 1;

	RT_CACHE_STAT_INC(in_slow_tot);

	if (res.type == RTN_BROADCAST)
		goto brd_input;

	if (res.type == RTN_LOCAL) {
		int result;
		result = fib_validate_source(saddr, daddr, tos,
					     loopback_dev.ifindex,
					     dev, &spec_dst, &itag);
		if (result < 0)
			goto martian_source;
		if (result)
			flags |= RTCF_DIRECTSRC;
		spec_dst = daddr;
		goto local_input;
	}

	if (!IN_DEV_FORWARD(in_dev))
		goto e_hostunreach;
	if (res.type != RTN_UNICAST)
		goto martian_destination;

#ifdef CONFIG_NK_IP_BASE
	/**************2007/3/6 trenchen:support ip base*******************************/
	if(ip_base_flag) {
		/* support IP balance enhancement: flush cache after 40s -- incifer 2008/11 */
	#ifdef CONFIG_IPBALANCE_ENHANCE
		int flag=0;
	#endif
		if( !out_device && res.fi && res.fi->fib_nhs > 1 ) {
			notmultipath=1;
			write_lock_bh(&IpBalLock);
			/* support IP balance enhancement: flush cache after 40s -- incifer 2008/11 */
		#ifdef CONFIG_IPBALANCE_ENHANCE
			/**
				add a new parameter(int *flag)
			**/
			if( (out_device = SearchIpOut( saddr, &res, &flag))){
			}
		#else
			if( (out_device = SearchIpOut( saddr, &res))){
			}
		#endif
			/* support IP balance enhancement: flush cache after 40s -- incifer 2008/11 */
		#ifdef CONFIG_IPBALANCE_ENHANCE
			/**
				flag equals to 1, means drop the packet to reserve the cache
			**/
			else if (flag==-1)
			{
				write_unlock_bh(&IpBalLock);
				goto e_inval;
			}
		#endif
			else {
				if( fl.oif == 0)
					fib_select_multipath(&fl,&res);
				IpBalanceCreateNew(saddr,&res);	
			}
			write_unlock_bh(&IpBalLock);
			
			//if( out_device )
				//printk(KERN_INFO "ip_balance src[%x] dst[%x] out[%s]\n",saddr,daddr,out_device->name);
			//else
				//printk(KERN_INFO "ip_balance src[%x] dst[%x]\n",saddr,daddr);
		}
	}
	/******************************************************************************/
#ifdef CONFIG_NK_SB_ENHANCEMENT
//-->add by michael lu 2008/11	
	else
	{
		//printk(KERN_EMERG "session_base_flush [%d]\n",session_base_flush);
		if(session_base_flush>0)
		{
			printk(KERN_EMERG "Change mode. Now start clear table\n");
			write_lock_bh(&IpBalLock);
#ifdef SB_USE_DST_ARRAY
                        SessionSrcInfoInit();
#else
			SBFlushStr();
#endif
			session_base_flush=0;
			write_unlock_bh(&IpBalLock);
			printk(KERN_EMERG "End clear table\n");
		}
	//	printk(KERN_EMERG "start session balance enhancement\n");

		if( !out_device && res.fi && res.fi->fib_nhs > 1 ) {
			notmultipath=1;
			write_lock_bh(&IpBalLock);
			if(session_base_flag)
			{//self-defined
			//printk(KERN_EMERG "session balance flag[%d]\n",session_base_flag);
				dst_index = SearchDstRange(&fl, daddr);
				if(dst_index<0)
				{
				//printk(KERN_EMERG "dst_table dst_index[%d]\n",dst_index);
					if( fl.oif == 0)
						fib_select_multipath(&fl,&res);
					//not in rule, so not need to new a structure for new session
					//SBCreateNewStr(saddr, &res, daddr, dst_index);
				}
				else {
				//printk(KERN_EMERG "dst_table dst_index[%d]\n",dst_index);
					if( (out_device = SB_1_SearchIpOut( saddr, &res, dst_index, daddr)) ){
					//printk(KERN_EMERG "use success SB_1_SearchIpOut\n");
					}
					else {
					//printk(KERN_EMERG "use fail SB_1_SearchIpOut\n");
						if( fl.oif == 0)
							fib_select_multipath(&fl,&res);
						SBCreateNewStr(saddr, &res, &fl, daddr, dst_index);
					}
				}
			}
			else
			{
			//printk(KERN_EMERG "session balance flag[%d]\n",session_base_flag);
				if(out_device = SB_2_SearchIpOut( saddr, &res, daddr) ){
					//printk(KERN_EMERG "use success SB_2_SearchIpOut\n");
				}
				else {
					//printk(KERN_EMERG "use fail SB_2_SearchIpOut\n");
					if( fl.oif == 0)
						fib_select_multipath(&fl,&res);
					SBCreateNewStr(saddr, &res, &fl, daddr, -1);
				}
			}
			write_unlock_bh(&IpBalLock);
		}
	}
//<--add by michael lu 2008/11
#endif
#endif

#ifdef CONFIG_NK_IP_BASE
	err = ip_mkroute_input(skb, &res, &fl, in_dev, daddr, saddr, tos, notmultipath);
#else
	err = ip_mkroute_input(skb, &res, &fl, in_dev, daddr, saddr, tos);
#endif

	if (err == -ENOBUFS)
		goto e_nobufs;
	if (err == -EINVAL)
		goto e_inval;
	
done:
	in_dev_put(in_dev);
	if (free_res)
		fib_res_put(&res);
out:	return err;

brd_input:
	if (skb->protocol != htons(ETH_P_IP))
		goto e_inval;

	if (ZERONET(saddr))
		spec_dst = inet_select_addr(dev, 0, RT_SCOPE_LINK);
	else {
		err = fib_validate_source(saddr, 0, tos, 0, dev, &spec_dst,
					  &itag);
		if (err < 0)
			goto martian_source;
		if (err)
			flags |= RTCF_DIRECTSRC;
	}
	flags |= RTCF_BROADCAST;
	res.type = RTN_BROADCAST;
	RT_CACHE_STAT_INC(in_brd);

local_input:
	rth = dst_alloc(&ipv4_dst_ops);
	if (!rth)
		goto e_nobufs;

	rth->u.dst.output= ip_rt_bug;

	atomic_set(&rth->u.dst.__refcnt, 1);
	rth->u.dst.flags= DST_HOST;
	if (in_dev->cnf.no_policy)
		rth->u.dst.flags |= DST_NOPOLICY;
	rth->fl.fl4_dst	= daddr;
	rth->rt_dst	= daddr;
	rth->fl.fl4_tos	= tos;
#ifdef CONFIG_IP_ROUTE_FWMARK
	rth->fl.fl4_fwmark= skb->nfmark;
#endif
	rth->fl.fl4_src	= saddr;
	rth->rt_src	= saddr;
#ifdef CONFIG_NET_CLS_ROUTE
	rth->u.dst.tclassid = itag;
#endif

#ifdef CONFIG_NK_PROTO_BINDING
	rth->fl.fl4_protocol_type = fl.fl4_protocol_type;
	rth->fl.fl4_src_port = fl.fl4_src_port;
	rth->fl.fl4_dst_port = fl.fl4_dst_port;
#endif

	rth->rt_iif	=
	rth->fl.iif	= dev->ifindex;
	rth->u.dst.dev	= &loopback_dev;
	dev_hold(rth->u.dst.dev);
	rth->idev	= in_dev_get(rth->u.dst.dev);
	rth->rt_gateway	= daddr;
	rth->rt_spec_dst= spec_dst;
	rth->u.dst.input= ip_local_deliver;
	rth->rt_flags 	= flags|RTCF_LOCAL;
	if (res.type == RTN_UNREACHABLE) {
		rth->u.dst.input= ip_error;
		rth->u.dst.error= -err;
		rth->rt_flags 	&= ~RTCF_LOCAL;
	}
	rth->rt_type	= res.type;
#ifdef ROUTING_HASH_IMPROVE
//Michael Lu add
        hash = rt_hash_code(daddr, saddr ^ (fl.iif << 5), fl.fl4_dst_port, fl.fl4_src_port, tos);
#else
	hash = rt_hash_code(daddr, saddr ^ (fl.iif << 5), tos);
#endif
	err = rt_intern_hash(hash, rth, (struct rtable**)&skb->dst);
	goto done;

no_route:
	RT_CACHE_STAT_INC(in_no_route);
	spec_dst = inet_select_addr(dev, 0, RT_SCOPE_UNIVERSE);
	res.type = RTN_UNREACHABLE;
	goto local_input;

	/*
	 *	Do not cache martian addresses: they should be logged (RFC1812)
	 */
martian_destination:
	RT_CACHE_STAT_INC(in_martian_dst);
#ifdef CONFIG_IP_ROUTE_VERBOSE
	if (IN_DEV_LOG_MARTIANS(in_dev) && net_ratelimit())
		printk(KERN_WARNING "martian destination %u.%u.%u.%u from "
			"%u.%u.%u.%u, dev %s\n",
			NIPQUAD(daddr), NIPQUAD(saddr), dev->name);
#endif

e_hostunreach:
        err = -EHOSTUNREACH;
        goto done;

e_inval:
	err = -EINVAL;
	goto done;

e_nobufs:
	err = -ENOBUFS;
	goto done;

martian_source:
	ip_handle_martian_source(dev, in_dev, skb, daddr, saddr);
	goto e_inval;
}

int ip_route_input(struct sk_buff *skb, u32 daddr, u32 saddr,
		   u8 tos, struct net_device *dev)
{
	struct rtable * rth;
	unsigned	hash;
	int iif = dev->ifindex;


#ifdef CONFIG_NK_PROTO_BINDING
	/*2006/11/1 trenchen : protocol binding*/
	__u32 protocol_type;
	__u32 src_port;
	__u32 dst_port;
	struct iphdr *iphdr = NULL;
	struct tcphdr *tcphdr = NULL;
	struct udphdr *udphdr = NULL;
	struct icmphdr *icmphdr = NULL;/* support ICMP -- incifer 2008/10 */
	
	iphdr = skb->nh.iph;
        if (iphdr->protocol == IPPROTO_TCP) {
	        tcphdr = (struct tcphdr *)((u_int32_t *)iphdr + iphdr->ihl);
	        if (tcphdr) {
	              protocol_type = IPPROTO_TCP;
	              src_port = tcphdr->source;
	              dst_port = tcphdr->dest;
	        }
	} else if (iphdr->protocol == IPPROTO_UDP) {
	        udphdr = (struct udphdr *)((u_int32_t *)iphdr + iphdr->ihl);
		if (udphdr) {
		       protocol_type = IPPROTO_UDP;
		       src_port =udphdr->source;
		       dst_port =udphdr->dest;
                }
	} else if (iphdr->protocol == IPPROTO_ICMP) {/* support ICMP -- incifer 2008/10 */
	        icmphdr = (struct icmphdr *)((u_int32_t *)iphdr + iphdr->ihl);
		if (icmphdr) {
		       protocol_type = IPPROTO_ICMP;
		       src_port =0;
		       dst_port =0;
                }
	} else	{
		protocol_type = 0;
		src_port = 0;
		dst_port = 0;
	}
	/**********************************/
#endif

	tos &= IPTOS_RT_MASK;
#ifdef ROUTING_HASH_IMPROVE
//Michael Lu add
	hash = rt_hash_code(daddr, saddr ^ (iif << 5), dst_port, src_port, tos);
#else
	hash = rt_hash_code(daddr, saddr ^ (iif << 5), tos);
#endif

	rcu_read_lock();
	for (rth = rcu_dereference(rt_hash_table[hash].chain); rth;
	     rth = rcu_dereference(rth->u.rt_next)) {
		if (rth->fl.fl4_dst == daddr &&
		    rth->fl.fl4_src == saddr &&
		    rth->fl.iif == iif &&
		    rth->fl.oif == 0 &&
#ifdef CONFIG_NK_PROTO_BINDING
		    /*2006/11/1 trenchen : protocol binding*/
		    protocol_type == rth->fl.fl4_protocol_type &&
		    /*2007/5/4 trenchen : bug fix, error compare*/
		    dst_port == rth->fl.fl4_dst_port &&
		    src_port == rth->fl.fl4_src_port &&
		    /**********************************/
#endif
#ifdef CONFIG_IP_ROUTE_FWMARK
		    rth->fl.fl4_fwmark == skb->nfmark &&
#endif
		    rth->fl.fl4_tos == tos) {
			rth->u.dst.lastuse = jiffies;
			dst_hold(&rth->u.dst);
			rth->u.dst.__use++;
			RT_CACHE_STAT_INC(in_hit);
			rcu_read_unlock();
			skb->dst = (struct dst_entry*)rth;
			return 0;
		}
		RT_CACHE_STAT_INC(in_hlist_search);
	}
	rcu_read_unlock();

	/* Multicast recognition logic is moved from route cache to here.
	   The problem was that too many Ethernet cards have broken/missing
	   hardware multicast filters :-( As result the host on multicasting
	   network acquires a lot of useless route cache entries, sort of
	   SDR messages from all the world. Now we try to get rid of them.
	   Really, provided software IP multicast filter is organized
	   reasonably (at least, hashed), it does not result in a slowdown
	   comparing with route cache reject entries.
	   Note, that multicast routers are not affected, because
	   route cache entry is created eventually.
	 */
	if (MULTICAST(daddr)) {
		struct in_device *in_dev;

		rcu_read_lock();
		if ((in_dev = __in_dev_get_rcu(dev)) != NULL) {
			int our = ip_check_mc(in_dev, daddr, saddr,
				skb->nh.iph->protocol);
			if (our
#ifdef CONFIG_IP_MROUTE
			    || (!LOCAL_MCAST(daddr) && IN_DEV_MFORWARD(in_dev))
#endif
			    ) {
				rcu_read_unlock();
#ifdef ROUTING_HASH_IMPROVE
//Michael Lu add
				return ip_route_input_mc(skb, daddr, saddr, dst_port, src_port,
							 tos, dev, our);
#else
				return ip_route_input_mc(skb, daddr, saddr,
							 tos, dev, our);
#endif
			}
		}
		rcu_read_unlock();
		return -EINVAL;
	}
	return ip_route_input_slow(skb, daddr, saddr, tos, dev);
}

static inline int __mkroute_output(struct rtable **result,
				   struct fib_result* res, 
				   const struct flowi *fl,
				   const struct flowi *oldflp, 
				   struct net_device *dev_out, 
				   unsigned flags) 
{
	struct rtable *rth;
	struct in_device *in_dev;
	u32 tos = RT_FL_TOS(oldflp);
	int err = 0;

	if (LOOPBACK(fl->fl4_src) && !(dev_out->flags&IFF_LOOPBACK))
		return -EINVAL;

	if (fl->fl4_dst == 0xFFFFFFFF)
		res->type = RTN_BROADCAST;
	else if (MULTICAST(fl->fl4_dst))
		res->type = RTN_MULTICAST;
	else if (BADCLASS(fl->fl4_dst) || ZERONET(fl->fl4_dst))
		return -EINVAL;

	if (dev_out->flags & IFF_LOOPBACK)
		flags |= RTCF_LOCAL;

	/* get work reference to inet device */
	in_dev = in_dev_get(dev_out);
	if (!in_dev)
		return -EINVAL;

	if (res->type == RTN_BROADCAST) {
		flags |= RTCF_BROADCAST | RTCF_LOCAL;
		if (res->fi) {
			fib_info_put(res->fi);
			res->fi = NULL;
		}
	} else if (res->type == RTN_MULTICAST) {
		flags |= RTCF_MULTICAST|RTCF_LOCAL;
		if (!ip_check_mc(in_dev, oldflp->fl4_dst, oldflp->fl4_src, 
				 oldflp->proto))
			flags &= ~RTCF_LOCAL;
		/* If multicast route do not exist use
		   default one, but do not gateway in this case.
		   Yes, it is hack.
		 */
		if (res->fi && res->prefixlen < 4) {
			fib_info_put(res->fi);
			res->fi = NULL;
		}
	}


	rth = dst_alloc(&ipv4_dst_ops);
	if (!rth) {
		err = -ENOBUFS;
		goto cleanup;
	}		

	atomic_set(&rth->u.dst.__refcnt, 1);
	rth->u.dst.flags= DST_HOST;
#ifdef CONFIG_IP_ROUTE_MULTIPATH_CACHED
	if (res->fi) {
		rth->rt_multipath_alg = res->fi->fib_mp_alg;
		if (res->fi->fib_nhs > 1)
			rth->u.dst.flags |= DST_BALANCED;
	}
#endif
	if (in_dev->cnf.no_xfrm)
		rth->u.dst.flags |= DST_NOXFRM;
	if (in_dev->cnf.no_policy)
		rth->u.dst.flags |= DST_NOPOLICY;

	rth->fl.fl4_dst	= oldflp->fl4_dst;
	rth->fl.fl4_tos	= tos;
	rth->fl.fl4_src	= oldflp->fl4_src;
	rth->fl.oif	= oldflp->oif;
#ifdef CONFIG_IP_ROUTE_FWMARK
	rth->fl.fl4_fwmark= oldflp->fl4_fwmark;
#endif
	rth->rt_dst	= fl->fl4_dst;
	rth->rt_src	= fl->fl4_src;
	rth->rt_iif	= oldflp->oif ? : dev_out->ifindex;
	/* get references to the devices that are to be hold by the routing 
	   cache entry */
	rth->u.dst.dev	= dev_out;
	dev_hold(dev_out);
	rth->idev	= in_dev_get(dev_out);
	rth->rt_gateway = fl->fl4_dst;
	rth->rt_spec_dst= fl->fl4_src;

	rth->u.dst.output=ip_output;

	RT_CACHE_STAT_INC(out_slow_tot);

	if (flags & RTCF_LOCAL) {
		rth->u.dst.input = ip_local_deliver;
		rth->rt_spec_dst = fl->fl4_dst;
	}
	if (flags & (RTCF_BROADCAST | RTCF_MULTICAST)) {
		rth->rt_spec_dst = fl->fl4_src;
		if (flags & RTCF_LOCAL && 
		    !(dev_out->flags & IFF_LOOPBACK)) {
			rth->u.dst.output = ip_mc_output;
			RT_CACHE_STAT_INC(out_slow_mc);
		}
#ifdef CONFIG_IP_MROUTE
		if (res->type == RTN_MULTICAST) {
			if (IN_DEV_MFORWARD(in_dev) &&
			    !LOCAL_MCAST(oldflp->fl4_dst)) {
				rth->u.dst.input = ip_mr_input;
				rth->u.dst.output = ip_mc_output;
			}
		}
#endif
	}

	rt_set_nexthop(rth, res, 0);

	rth->rt_flags = flags;

	*result = rth;
 cleanup:
	/* release work reference to inet device */
	in_dev_put(in_dev);

	return err;
}

static inline int ip_mkroute_output_def(struct rtable **rp,
					struct fib_result* res,
					const struct flowi *fl,
					const struct flowi *oldflp,
					struct net_device *dev_out,
					unsigned flags)
{
	struct rtable *rth = NULL;
	int err = __mkroute_output(&rth, res, fl, oldflp, dev_out, flags);
	unsigned hash;
	if (err == 0) {
		u32 tos = RT_FL_TOS(oldflp);

#ifdef ROUTING_HASH_IMPROVE
//Michael Lu add
                hash = rt_hash_code(oldflp->fl4_dst, oldflp->fl4_src ^ (oldflp->oif << 5), fl->fl4_dst_port, fl->fl4_src_port, tos);
#else
		hash = rt_hash_code(oldflp->fl4_dst, 
				    oldflp->fl4_src ^ (oldflp->oif << 5), tos);
#endif
		err = rt_intern_hash(hash, rth, rp);
	}
	
	return err;
}

static inline int ip_mkroute_output(struct rtable** rp,
				    struct fib_result* res,
				    const struct flowi *fl,
				    const struct flowi *oldflp,
				    struct net_device *dev_out,
				    unsigned flags)
{
#ifdef CONFIG_IP_ROUTE_MULTIPATH_CACHED
	u32 tos = RT_FL_TOS(oldflp);
	unsigned char hop;
	unsigned hash;
	int err = -EINVAL;
	struct rtable *rth = NULL;

	if (res->fi && res->fi->fib_nhs > 1) {
		unsigned char hopcount = res->fi->fib_nhs;

		for (hop = 0; hop < hopcount; hop++) {
			struct net_device *dev2nexthop;

			res->nh_sel = hop;

			/* hold a work reference to the output device */
			dev2nexthop = FIB_RES_DEV(*res);
			dev_hold(dev2nexthop);

			/* put reference to previous result */
			if (hop)
				ip_rt_put(*rp);

			err = __mkroute_output(&rth, res, fl, oldflp,
					       dev2nexthop, flags);

			if (err != 0)
				goto cleanup;

#ifdef ROUTING_HASH_IMPROVE
//Michael Lu add
                hash = rt_hash_code(oldflp->fl4_dst, oldflp->fl4_src ^ (oldflp->oif << 5), fl->fl4_dst_port, fl->fl4_src_port, tos);
#else
			hash = rt_hash_code(oldflp->fl4_dst, 
					    oldflp->fl4_src ^
					    (oldflp->oif << 5), tos);
#endif
			err = rt_intern_hash(hash, rth, rp);

			/* forward hop information to multipath impl. */
			multipath_set_nhinfo(rth,
					     FIB_RES_NETWORK(*res),
					     FIB_RES_NETMASK(*res),
					     res->prefixlen,
					     &FIB_RES_NH(*res));
		cleanup:
			/* release work reference to output device */
			dev_put(dev2nexthop);

			if (err != 0)
				return err;
		}
		return err;
	} else {
		return ip_mkroute_output_def(rp, res, fl, oldflp, dev_out,
					     flags);
	}
#else /* CONFIG_IP_ROUTE_MULTIPATH_CACHED */
	return ip_mkroute_output_def(rp, res, fl, oldflp, dev_out, flags);
#endif
}

/*
 * Major route resolver routine.
 */

static int ip_route_output_slow(struct rtable **rp, const struct flowi *oldflp)
{
	u32 tos	= RT_FL_TOS(oldflp);
	struct flowi fl = { .nl_u = { .ip4_u =
				      { .daddr = oldflp->fl4_dst,
					.saddr = oldflp->fl4_src,
					.tos = tos & IPTOS_RT_MASK,
					.scope = ((tos & RTO_ONLINK) ?
						  RT_SCOPE_LINK :
						  RT_SCOPE_UNIVERSE),
#ifdef CONFIG_IP_ROUTE_FWMARK
					.fwmark = oldflp->fl4_fwmark
#endif
				      } },
			    .iif = loopback_dev.ifindex,
			    .oif = oldflp->oif, 
			    .wan2lan = 0};/*2007/12/26 trenchen : bug fix, protocol binding src ip 192.168.0~0->0.0.0.0~255.255.255.255 the range may cover packet from want to lan make packet be binded to wan error*/
	struct fib_result res;
	unsigned flags = 0;
	struct net_device *dev_out = NULL;
	int free_res = 0;
	int err;
	struct net_device *temp_device = NULL;	/*2006/11/2 trenchen : modify for specify out device*/
	struct net_device *out_device = NULL;	/*2006/11/2 trenchen : modify for specify out device*/
	u32 fsrc;

#ifdef CONFIG_NK_PROTO_BINDING
	//printk("proto[%u] srcport[%u] dport[%d]\n",oldflp->proto,oldflp->fl_ip_sport,oldflp->fl_ip_dport);
	//if( oldflp->proto==IPPROTO_TCP || oldflp->proto==IPPROTO_UDP ){
	if( oldflp->proto==IPPROTO_TCP || oldflp->proto==IPPROTO_UDP || oldflp->proto==IPPROTO_ICMP ){/* support ICMP -- incifer 2008/10 */
		fl.fl4_protocol_type = oldflp->proto;
		fl.fl4_src_port = oldflp->fl_ip_sport;
		fl.fl4_dst_port = oldflp->fl_ip_dport;
	} else{
		fl.fl4_protocol_type = NULL;
		fl.fl4_src_port = NULL;
		fl.fl4_dst_port = NULL;
	}
#endif

	res.fi		= NULL;
#ifdef CONFIG_IP_MULTIPLE_TABLES
	res.r		= NULL;
#endif

	if (oldflp->fl4_src) {
		err = -EINVAL;
		if (MULTICAST(oldflp->fl4_src) ||
		    BADCLASS(oldflp->fl4_src) ||
		    ZERONET(oldflp->fl4_src))
			goto out;

		/* It is equivalent to inet_addr_type(saddr) == RTN_LOCAL */
		dev_out = ip_dev_find(oldflp->fl4_src);
		if (dev_out == NULL)
			goto out;

		/*2006/11/2 trenchen : support source route*/
		if (dev_out && oldflp->fl4_dst != oldflp->fl4_src)
			temp_device = dev_out;

		/* I removed check for oif == dev_out->oif here.
		   It was wrong for two reasons:
		   1. ip_dev_find(saddr) can return wrong iface, if saddr is
		      assigned to multiple interfaces.
		   2. Moreover, we are allowed to send packets with saddr
		      of another iface. --ANK
		 */

		if (oldflp->oif == 0
		    && (MULTICAST(oldflp->fl4_dst) || oldflp->fl4_dst == 0xFFFFFFFF)) {
			/* Special hack: user can direct multicasts
			   and limited broadcast via necessary interface
			   without fiddling with IP_MULTICAST_IF or IP_PKTINFO.
			   This hack is not just for fun, it allows
			   vic,vat and friends to work.
			   They bind socket to loopback, set ttl to zero
			   and expect that it will work.
			   From the viewpoint of routing cache they are broken,
			   because we are not allowed to build multicast path
			   with loopback source addr (look, routing cache
			   cannot know, that ttl is zero, so that packet
			   will not leave this host and route is valid).
			   Luckily, this hack is good workaround.
			 */

			fl.oif = dev_out->ifindex;
			goto make_route;
		}
		if (dev_out)
			dev_put(dev_out);
		dev_out = NULL;
	}


	if (oldflp->oif) {
		dev_out = dev_get_by_index(oldflp->oif);
		err = -ENODEV;
		if (dev_out == NULL)
			goto out;

		/* RACE: Check return value of inet_select_addr instead. */
		if (__in_dev_get_rtnl(dev_out) == NULL) {
			dev_put(dev_out);
			goto out;	/* Wrong error code */
		}

		if (LOCAL_MCAST(oldflp->fl4_dst) || oldflp->fl4_dst == 0xFFFFFFFF) {
			if (!fl.fl4_src)
				fl.fl4_src = inet_select_addr(dev_out, 0,
							      RT_SCOPE_LINK);
			goto make_route;
		}
		if (!fl.fl4_src) {
			if (MULTICAST(oldflp->fl4_dst))
				fl.fl4_src = inet_select_addr(dev_out, 0,
							      fl.fl4_scope);
			else if (!oldflp->fl4_dst)
				fl.fl4_src = inet_select_addr(dev_out, 0,
							      RT_SCOPE_HOST);
		}
	}

	if (!fl.fl4_dst) {
		fl.fl4_dst = fl.fl4_src;
		if (!fl.fl4_dst)
			fl.fl4_dst = fl.fl4_src = htonl(INADDR_LOOPBACK);
		if (dev_out)
			dev_put(dev_out);
		dev_out = &loopback_dev;
		dev_hold(dev_out);
		fl.oif = loopback_dev.ifindex;
		res.type = RTN_LOCAL;
		flags |= RTCF_LOCAL;
		goto make_route;
	}

	/*2006/11/2 trenchen : support remote manage*/
#ifdef CONFIG_NK_PROTO_BINDING
	/*2008/03/05 trenchen : bug fix, when pc in lan side, using "http://wan ip" to remote manage, need port
           information to make caching search match*/
	out_device=nk_multipath_select(fl.fl4_dst, fl.fl4_src, fl.fl4_protocol_type, fl.fl4_dst_port, fl.fl4_src_port, tos);
#else
	out_device=nk_multipath_select(fl.fl4_dst, fl.fl4_src, 0, 0, 0, tos);
#endif
	/*2006/11/2 trenchen : support source route*/
	if ( !out_device && temp_device )
		out_device = temp_device;
	if(out_device && !strcmp(out_device->name,NK_LAN_IF))
		out_device = NULL;
	/******************************************/

#if 1
	if (nk_fib_lookup(&fl, &res, out_device)) {
#else
	if (fib_lookup(&fl, &res)) {
#endif
		res.fi = NULL;
		if (oldflp->oif) {
			/* Apparently, routing tables are wrong. Assume,
			   that the destination is on link.

			   WHY? DW.
			   Because we are allowed to send to iface
			   even if it has NO routes and NO assigned
			   addresses. When oif is specified, routing
			   tables are looked up with only one purpose:
			   to catch if destination is gatewayed, rather than
			   direct. Moreover, if MSG_DONTROUTE is set,
			   we send packet, ignoring both routing tables
			   and ifaddr state. --ANK


			   We could make it even if oif is unknown,
			   likely IPv6, but we do not.
			 */

			if (fl.fl4_src == 0)
				fl.fl4_src = inet_select_addr(dev_out, 0,
							      RT_SCOPE_LINK);
			res.type = RTN_UNICAST;
			goto make_route;
		}
		if (dev_out)
			dev_put(dev_out);
		err = -ENETUNREACH;
		goto out;
	}
	free_res = 1;

	if (res.type == RTN_LOCAL) {
		if (!fl.fl4_src)
			fl.fl4_src = fl.fl4_dst;
		if (dev_out)
			dev_put(dev_out);
		dev_out = &loopback_dev;
		dev_hold(dev_out);
		fl.oif = dev_out->ifindex;
		if (res.fi)
			fib_info_put(res.fi);
		res.fi = NULL;
		flags |= RTCF_LOCAL;
		goto make_route;
	}

#ifdef CONFIG_IP_ROUTE_MULTIPATH
	if (res.fi->fib_nhs > 1 && fl.oif == 0)
		fib_select_multipath(&fl, &res);
	//else
#endif
//	if (!res.prefixlen && res.type == RTN_UNICAST && !fl.oif)
//		fib_select_default(&fl, &res);

	//printk("after select fib_lookup result name[%s]\n",res.fi->fib_dev->name);

	fsrc = fl.fl4_src;
	if (!fl.fl4_src)
		fl.fl4_src = FIB_RES_PREFSRC(res);

	if (dev_out)
		dev_put(dev_out);
	dev_out = FIB_RES_DEV(res);
	dev_hold(dev_out);
	fl.oif = dev_out->ifindex;


make_route:
	err = ip_mkroute_output(rp, &res, &fl, oldflp, dev_out, flags);


	if (free_res)
		fib_res_put(&res);
	if (dev_out)
		dev_put(dev_out);
out:	return err;
}

int __ip_route_output_key(struct rtable **rp, const struct flowi *flp)
{
	unsigned hash;
	struct rtable *rth;

#ifdef ROUTING_HASH_IMPROVE
//Michael Lu add
        hash = rt_hash_code(flp->fl4_dst, flp->fl4_src ^ (flp->oif << 5), flp->fl4_dst_port, flp->fl4_src_port, flp->fl4_tos);
#else
	hash = rt_hash_code(flp->fl4_dst, flp->fl4_src ^ (flp->oif << 5), flp->fl4_tos);
#endif

	rcu_read_lock_bh();
	for (rth = rcu_dereference(rt_hash_table[hash].chain); rth;
		rth = rcu_dereference(rth->u.rt_next)) {
		if (rth->fl.fl4_dst == flp->fl4_dst &&
		    rth->fl.fl4_src == flp->fl4_src &&
		    rth->fl.iif == 0 &&
		    rth->fl.oif == flp->oif &&
#ifdef CONFIG_IP_ROUTE_FWMARK
		    rth->fl.fl4_fwmark == flp->fl4_fwmark &&
#endif
		    !((rth->fl.fl4_tos ^ flp->fl4_tos) &
			    (IPTOS_RT_MASK | RTO_ONLINK))) {

			/* check for multipath routes and choose one if
			 * necessary
			 */
			if (multipath_select_route(flp, rth, rp)) {
				dst_hold(&(*rp)->u.dst);
				RT_CACHE_STAT_INC(out_hit);
				rcu_read_unlock_bh();
				return 0;
			}

			rth->u.dst.lastuse = jiffies;
			dst_hold(&rth->u.dst);
			rth->u.dst.__use++;
			RT_CACHE_STAT_INC(out_hit);
			rcu_read_unlock_bh();
			*rp = rth;
			return 0;
		}
		RT_CACHE_STAT_INC(out_hlist_search);
	}
	rcu_read_unlock_bh();

	return ip_route_output_slow(rp, flp);
}

EXPORT_SYMBOL_GPL(__ip_route_output_key);

int ip_route_output_flow(struct rtable **rp, struct flowi *flp, struct sock *sk, int flags)
{
	int err;

	if ((err = __ip_route_output_key(rp, flp)) != 0)
		return err;

	if (flp->proto) {
		if (!flp->fl4_src)
			flp->fl4_src = (*rp)->rt_src;
		if (!flp->fl4_dst)
			flp->fl4_dst = (*rp)->rt_dst;
		return xfrm_lookup((struct dst_entry **)rp, flp, sk, flags);
	}

	return 0;
}

EXPORT_SYMBOL_GPL(ip_route_output_flow);

int ip_route_output_key(struct rtable **rp, struct flowi *flp)
{
	return ip_route_output_flow(rp, flp, NULL, 0);
}

static int rt_fill_info(struct sk_buff *skb, u32 pid, u32 seq, int event,
			int nowait, unsigned int flags)
{
	struct rtable *rt = (struct rtable*)skb->dst;
	struct rtmsg *r;
	struct nlmsghdr  *nlh;
	unsigned char	 *b = skb->tail;
	struct rta_cacheinfo ci;
#ifdef CONFIG_IP_MROUTE
	struct rtattr *eptr;
#endif
	nlh = NLMSG_NEW(skb, pid, seq, event, sizeof(*r), flags);
	r = NLMSG_DATA(nlh);
	r->rtm_family	 = AF_INET;
	r->rtm_dst_len	= 32;
	r->rtm_src_len	= 0;
	r->rtm_tos	= rt->fl.fl4_tos;
	r->rtm_table	= RT_TABLE_MAIN;
	r->rtm_type	= rt->rt_type;
	r->rtm_scope	= RT_SCOPE_UNIVERSE;
	r->rtm_protocol = RTPROT_UNSPEC;
	r->rtm_flags	= (rt->rt_flags & ~0xFFFF) | RTM_F_CLONED;
	if (rt->rt_flags & RTCF_NOTIFY)
		r->rtm_flags |= RTM_F_NOTIFY;
	RTA_PUT(skb, RTA_DST, 4, &rt->rt_dst);
	if (rt->fl.fl4_src) {
		r->rtm_src_len = 32;
		RTA_PUT(skb, RTA_SRC, 4, &rt->fl.fl4_src);
	}

	/*2006/11/2 trenchen : let ip r s c show protocol type and port info*/
	if (rt->fl.fl4_protocol_type) {//add by trenchen
		RTA_PUT(skb, RTA_PROTOCOL_TYPE, 4, &rt->fl.fl4_protocol_type);
	}
	if (rt->fl.fl4_src_port) {
		RTA_PUT(skb, RTA_START_SRC_PORT, 4, &rt->fl.fl4_src_port);
	}
	if (rt->fl.fl4_dst_port) {
		RTA_PUT(skb, RTA_START_DST_PORT, 4, &rt->fl.fl4_dst_port);
	}
	/*****************************************************************/

	if (rt->u.dst.dev)
		RTA_PUT(skb, RTA_OIF, sizeof(int), &rt->u.dst.dev->ifindex);
#ifdef CONFIG_NET_CLS_ROUTE
	if (rt->u.dst.tclassid)
		RTA_PUT(skb, RTA_FLOW, 4, &rt->u.dst.tclassid);
#endif
#ifdef CONFIG_IP_ROUTE_MULTIPATH_CACHED
	if (rt->rt_multipath_alg != IP_MP_ALG_NONE) {
		__u32 alg = rt->rt_multipath_alg;

		RTA_PUT(skb, RTA_MP_ALGO, 4, &alg);
	}
#endif
	if (rt->fl.iif)
		RTA_PUT(skb, RTA_PREFSRC, 4, &rt->rt_spec_dst);
	else if (rt->rt_src != rt->fl.fl4_src)
		RTA_PUT(skb, RTA_PREFSRC, 4, &rt->rt_src);
	if (rt->rt_dst != rt->rt_gateway)
		RTA_PUT(skb, RTA_GATEWAY, 4, &rt->rt_gateway);
	if (rtnetlink_put_metrics(skb, rt->u.dst.metrics) < 0)
		goto rtattr_failure;
	ci.rta_lastuse	= jiffies_to_clock_t(jiffies - rt->u.dst.lastuse);
	ci.rta_used	= rt->u.dst.__use;
	ci.rta_clntref	= atomic_read(&rt->u.dst.__refcnt);
	if (rt->u.dst.expires)
		ci.rta_expires = jiffies_to_clock_t(rt->u.dst.expires - jiffies);
	else
		ci.rta_expires = 0;
	ci.rta_error	= rt->u.dst.error;
	ci.rta_id	= ci.rta_ts = ci.rta_tsage = 0;
	if (rt->peer) {
		ci.rta_id = rt->peer->ip_id_count;
		if (rt->peer->tcp_ts_stamp) {
			ci.rta_ts = rt->peer->tcp_ts;
			ci.rta_tsage = xtime.tv_sec - rt->peer->tcp_ts_stamp;
		}
	}
#ifdef CONFIG_IP_MROUTE
	eptr = (struct rtattr*)skb->tail;
#endif
	RTA_PUT(skb, RTA_CACHEINFO, sizeof(ci), &ci);
	if (rt->fl.iif) {
#ifdef CONFIG_IP_MROUTE
		u32 dst = rt->rt_dst;

		if (MULTICAST(dst) && !LOCAL_MCAST(dst) &&
		    ipv4_devconf.mc_forwarding) {
			int err = ipmr_get_route(skb, r, nowait);
			if (err <= 0) {
				if (!nowait) {
					if (err == 0)
						return 0;
					goto nlmsg_failure;
				} else {
					if (err == -EMSGSIZE)
						goto nlmsg_failure;
					((struct rta_cacheinfo*)RTA_DATA(eptr))->rta_error = err;
				}
			}
		} else
#endif
			RTA_PUT(skb, RTA_IIF, sizeof(int), &rt->fl.iif);
	}

	nlh->nlmsg_len = skb->tail - b;
	return skb->len;

nlmsg_failure:
rtattr_failure:
	skb_trim(skb, b - skb->data);
	return -1;
}

int inet_rtm_getroute(struct sk_buff *in_skb, struct nlmsghdr* nlh, void *arg)
{
	struct rtattr **rta = arg;
	struct rtmsg *rtm = NLMSG_DATA(nlh);
	struct rtable *rt = NULL;
	u32 dst = 0;
	u32 src = 0;
	int iif = 0;
	int err = -ENOBUFS;
	struct sk_buff *skb;

	skb = alloc_skb(NLMSG_GOODSIZE, GFP_KERNEL);
	if (!skb)
		goto out;

	/* Reserve room for dummy headers, this skb can pass
	   through good chunk of routing engine.
	 */
	skb->mac.raw = skb->nh.raw = skb->data;

	/* Bugfix: need to give ip_route_input enough of an IP header to not gag. */
	skb->nh.iph->protocol = IPPROTO_ICMP;
	skb_reserve(skb, MAX_HEADER + sizeof(struct iphdr));

	if (rta[RTA_SRC - 1])
		memcpy(&src, RTA_DATA(rta[RTA_SRC - 1]), 4);
	if (rta[RTA_DST - 1])
		memcpy(&dst, RTA_DATA(rta[RTA_DST - 1]), 4);
	if (rta[RTA_IIF - 1])
		memcpy(&iif, RTA_DATA(rta[RTA_IIF - 1]), sizeof(int));

	if (iif) {
		struct net_device *dev = __dev_get_by_index(iif);
		err = -ENODEV;
		if (!dev)
			goto out_free;
		skb->protocol	= htons(ETH_P_IP);
		skb->dev	= dev;
		local_bh_disable();
		err = ip_route_input(skb, dst, src, rtm->rtm_tos, dev);
		local_bh_enable();
		rt = (struct rtable*)skb->dst;
		if (!err && rt->u.dst.error)
			err = -rt->u.dst.error;
	} else {
		struct flowi fl = { .nl_u = { .ip4_u = { .daddr = dst,
							 .saddr = src,
							 .tos = rtm->rtm_tos } } };
		int oif = 0;
		if (rta[RTA_OIF - 1])
			memcpy(&oif, RTA_DATA(rta[RTA_OIF - 1]), sizeof(int));
		fl.oif = oif;
		err = ip_route_output_key(&rt, &fl);
	}
	if (err)
		goto out_free;

	skb->dst = &rt->u.dst;
	if (rtm->rtm_flags & RTM_F_NOTIFY)
		rt->rt_flags |= RTCF_NOTIFY;

	NETLINK_CB(skb).dst_pid = NETLINK_CB(in_skb).pid;

	err = rt_fill_info(skb, NETLINK_CB(in_skb).pid, nlh->nlmsg_seq,
				RTM_NEWROUTE, 0, 0);
	if (!err)
		goto out_free;
	if (err < 0) {
		err = -EMSGSIZE;
		goto out_free;
	}

	err = netlink_unicast(rtnl, skb, NETLINK_CB(in_skb).pid, MSG_DONTWAIT);
	if (err > 0)
		err = 0;
out:	return err;

out_free:
	kfree_skb(skb);
	goto out;
}

int ip_rt_dump(struct sk_buff *skb,  struct netlink_callback *cb)
{
	struct rtable *rt;
	int h, s_h;
	int idx, s_idx;

	s_h = cb->args[0];
	s_idx = idx = cb->args[1];
	for (h = 0; h <= rt_hash_mask; h++) {
		if (h < s_h) continue;
		if (h > s_h)
			s_idx = 0;
		rcu_read_lock_bh();
		for (rt = rcu_dereference(rt_hash_table[h].chain), idx = 0; rt;
		     rt = rcu_dereference(rt->u.rt_next), idx++) {
			if (idx < s_idx)
				continue;
			skb->dst = dst_clone(&rt->u.dst);
			if (rt_fill_info(skb, NETLINK_CB(cb->skb).pid,
					 cb->nlh->nlmsg_seq, RTM_NEWROUTE, 
					 1, NLM_F_MULTI) <= 0) {
				dst_release(xchg(&skb->dst, NULL));
				rcu_read_unlock_bh();
				goto done;
			}
			dst_release(xchg(&skb->dst, NULL));
		}
		rcu_read_unlock_bh();
	}

done:
	cb->args[0] = h;
	cb->args[1] = idx;
	return skb->len;
}

void ip_rt_multicast_event(struct in_device *in_dev)
{
#ifndef CONFIG_NK_ROUTE_CACHE_MODIFY
	rt_cache_flush(0);
#else
	rt_cache_flush(0,in_dev->dev);
#endif
}

#ifdef CONFIG_SYSCTL
static int flush_delay;

#ifdef CONFIG_NK_ROUTE_CACHE_MODIFY
static char flush_dev[20]="";

static int ipv4_sysctl_rtcache_flush_dev(ctl_table *ctl, int write,
					struct file *filp, void __user *buffer,
					size_t *lenp, loff_t *ppos)
{
	struct net_device *dev;
	if (write) {
		proc_dointvec(ctl, write, filp, buffer, lenp, ppos);
		//printk("ipvr dev[%s]\n",buffer);
		dev = dev_get_by_name(buffer);
		if(dev)
			dev_put(dev);
		//printk("ipv4_sysctl_rtcache_flush_dev[%s]\n",dev->name);
		rt_cache_flush(0,dev);
		return 0;
	}

	return -EINVAL;
}
#endif


static int ipv4_sysctl_rtcache_flush(ctl_table *ctl, int write,
					struct file *filp, void __user *buffer,
					size_t *lenp, loff_t *ppos)
{
	if (write) {
		proc_dointvec(ctl, write, filp, buffer, lenp, ppos);
#ifndef CONFIG_NK_ROUTE_CACHE_MODIFY
		rt_cache_flush(flush_delay);
#else
		rt_cache_flush(flush_delay,0);
#endif
		return 0;
	} 

	return -EINVAL;
}

static int ipv4_sysctl_rtcache_flush_strategy(ctl_table *table,
						int __user *name,
						int nlen,
						void __user *oldval,
						size_t __user *oldlenp,
						void __user *newval,
						size_t newlen,
						void **context)
{
	int delay;
	if (newlen != sizeof(int))
		return -EINVAL;
	if (get_user(delay, (int __user *)newval))
		return -EFAULT; 
#ifndef CONFIG_NK_ROUTE_CACHE_MODIFY
	rt_cache_flush(delay); 
#else
	rt_cache_flush(delay,0);
#endif
	return 0;
}

ctl_table ipv4_route_table[] = {
        {
		.ctl_name 	= NET_IPV4_ROUTE_FLUSH,
		.procname	= "flush",
		.data		= &flush_delay,
		.maxlen		= sizeof(int),
		.mode		= 0200,
		.proc_handler	= &ipv4_sysctl_rtcache_flush,
		.strategy	= &ipv4_sysctl_rtcache_flush_strategy,
	},
	{
		.ctl_name	= NET_IPV4_ROUTE_MIN_DELAY,
		.procname	= "min_delay",
		.data		= &ip_rt_min_delay,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= &proc_dointvec_jiffies,
		.strategy	= &sysctl_jiffies,
	},
	{
		.ctl_name	= NET_IPV4_ROUTE_MAX_DELAY,
		.procname	= "max_delay",
		.data		= &ip_rt_max_delay,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= &proc_dointvec_jiffies,
		.strategy	= &sysctl_jiffies,
	},
	{
		.ctl_name	= NET_IPV4_ROUTE_GC_THRESH,
		.procname	= "gc_thresh",
		.data		= &ipv4_dst_ops.gc_thresh,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= &proc_dointvec,
	},
	{
		.ctl_name	= NET_IPV4_ROUTE_MAX_SIZE,
		.procname	= "max_size",
		.data		= &ip_rt_max_size,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= &proc_dointvec,
	},
	{
		/*  Deprecated. Use gc_min_interval_ms */
 
		.ctl_name	= NET_IPV4_ROUTE_GC_MIN_INTERVAL,
		.procname	= "gc_min_interval",
		.data		= &ip_rt_gc_min_interval,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= &proc_dointvec_jiffies,
		.strategy	= &sysctl_jiffies,
	},
	{
		.ctl_name	= NET_IPV4_ROUTE_GC_MIN_INTERVAL_MS,
		.procname	= "gc_min_interval_ms",
		.data		= &ip_rt_gc_min_interval,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= &proc_dointvec_ms_jiffies,
		.strategy	= &sysctl_ms_jiffies,
	},
	{
		.ctl_name	= NET_IPV4_ROUTE_GC_TIMEOUT,
		.procname	= "gc_timeout",
		.data		= &ip_rt_gc_timeout,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= &proc_dointvec_jiffies,
		.strategy	= &sysctl_jiffies,
	},
	{
		.ctl_name	= NET_IPV4_ROUTE_GC_INTERVAL,
		.procname	= "gc_interval",
		.data		= &ip_rt_gc_interval,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= &proc_dointvec_jiffies,
		.strategy	= &sysctl_jiffies,
	},
	{
		.ctl_name	= NET_IPV4_ROUTE_REDIRECT_LOAD,
		.procname	= "redirect_load",
		.data		= &ip_rt_redirect_load,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= &proc_dointvec,
	},
	{
		.ctl_name	= NET_IPV4_ROUTE_REDIRECT_NUMBER,
		.procname	= "redirect_number",
		.data		= &ip_rt_redirect_number,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= &proc_dointvec,
	},
	{
		.ctl_name	= NET_IPV4_ROUTE_REDIRECT_SILENCE,
		.procname	= "redirect_silence",
		.data		= &ip_rt_redirect_silence,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= &proc_dointvec,
	},
	{
		.ctl_name	= NET_IPV4_ROUTE_ERROR_COST,
		.procname	= "error_cost",
		.data		= &ip_rt_error_cost,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= &proc_dointvec,
	},
	{
		.ctl_name	= NET_IPV4_ROUTE_ERROR_BURST,
		.procname	= "error_burst",
		.data		= &ip_rt_error_burst,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= &proc_dointvec,
	},
	{
		.ctl_name	= NET_IPV4_ROUTE_GC_ELASTICITY,
		.procname	= "gc_elasticity",
		.data		= &ip_rt_gc_elasticity,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= &proc_dointvec,
	},
	{
		.ctl_name	= NET_IPV4_ROUTE_MTU_EXPIRES,
		.procname	= "mtu_expires",
		.data		= &ip_rt_mtu_expires,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= &proc_dointvec_jiffies,
		.strategy	= &sysctl_jiffies,
	},
	{
		.ctl_name	= NET_IPV4_ROUTE_MIN_PMTU,
		.procname	= "min_pmtu",
		.data		= &ip_rt_min_pmtu,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= &proc_dointvec,
	},
	{
		.ctl_name	= NET_IPV4_ROUTE_MIN_ADVMSS,
		.procname	= "min_adv_mss",
		.data		= &ip_rt_min_advmss,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= &proc_dointvec,
	},
	{
		.ctl_name	= NET_IPV4_ROUTE_SECRET_INTERVAL,
		.procname	= "secret_interval",
		.data		= &ip_rt_secret_interval,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= &proc_dointvec_jiffies,
		.strategy	= &sysctl_jiffies,
	},
#ifdef CONFIG_NK_IP_BASE
	{       /******************add by trenchen 2005/2/16******************/
		.ctl_name	= NET_IPV4_ROUTE_IP_BALANCE,
		.procname	= "ip_base",
		.data		= &ip_base_flag,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= &proc_dointvec,
	},      /**************************************************************/
#endif
#ifdef CONFIG_NK_ROUTE_CACHE_MODIFY
	{
		.ctl_name	= NET_IPV4_ROUTE_PC_FLUSH_DEV,
		.procname	= "flush_dev",
		.data		= &flush_dev,
		.maxlen		= 20,
		.mode		= 0644,
		.proc_handler	= &ipv4_sysctl_rtcache_flush_dev,
	},
#endif
//-->add by michael lu 2008/10
#ifdef CONFIG_NK_SB_ENHANCEMENT
	{
		.ctl_name	= NET_IPV4_ROUTE_SESSION_BALANCE,
		.procname	= "session_base",
		.data		= &session_base_flag,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= &proc_dointvec,
	},
	{
		.ctl_name	= NET_IPV4_ROUTE_SESSION_FLUSH,
		.procname	= "session_flush",
		.data		= &session_base_flush,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= &proc_dointvec,
	},
	{
		.ctl_name	= NET_IPV4_ROUTE_SESSION_TABLE_LIMIT,
		.procname	= "session_table_limit",
		.data		= &SessionNumData,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= &proc_dointvec,
	},
	{
		.ctl_name	= NET_IPV4_ROUTE_SESSION_TABLE_DENOMINATOR,
		.procname	= "session_table_denominator",
		.data		= &SessionNumDeno,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= &proc_dointvec,
	},
	{
		.ctl_name	= NET_IPV4_ROUTE_SESSION_TABLE_DENO_FLAG,
		.procname	= "session_table_deno_flag",
		.data		= &SessionDenoFlag,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= &proc_dointvec,
	},
#endif
/* support IpBalance Struct Aging -- incifer 2009/01 */
#ifdef CONFIG_IPBALANCE_STRUCT_AGING
	{
		.ctl_name	= NET_IPV4_ROUTE_IP_BALANCE_STRUCT_AGING_START_TIME,
		.procname	= "proc_ipbalance_struct_aging_start_time",
		.data		= &proc_ipbalance_struct_aging_start_time,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= &proc_dointvec,
	},
	{
		.ctl_name	= NET_IPV4_ROUTE_IP_BALANCE_STRUCT_AGING_PERIOD_TIME,
		.procname	= "proc_ipbalance_struct_aging_period_time",
		.data		= &proc_ipbalance_struct_aging_period_time,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= &proc_dointvec,
	},
#endif
//<--add by michael lu 2008/10
	{ .ctl_name = 0 }
};
#endif

#ifdef CONFIG_NET_CLS_ROUTE
struct ip_rt_acct *ip_rt_acct;

/* This code sucks.  But you should have seen it before! --RR */

/* IP route accounting ptr for this logical cpu number. */
#define IP_RT_ACCT_CPU(i) (ip_rt_acct + i * 256)

#ifdef CONFIG_PROC_FS
static int ip_rt_acct_read(char *buffer, char **start, off_t offset,
			   int length, int *eof, void *data)
{
	unsigned int i;

	if ((offset & 3) || (length & 3))
		return -EIO;

	if (offset >= sizeof(struct ip_rt_acct) * 256) {
		*eof = 1;
		return 0;
	}

	if (offset + length >= sizeof(struct ip_rt_acct) * 256) {
		length = sizeof(struct ip_rt_acct) * 256 - offset;
		*eof = 1;
	}

	offset /= sizeof(u32);

	if (length > 0) {
		u32 *src = ((u32 *) IP_RT_ACCT_CPU(0)) + offset;
		u32 *dst = (u32 *) buffer;

		/* Copy first cpu. */
		*start = buffer;
		memcpy(dst, src, length);

		/* Add the other cpus in, one int at a time */
		for_each_cpu(i) {
			unsigned int j;

			src = ((u32 *) IP_RT_ACCT_CPU(i)) + offset;

			for (j = 0; j < length/4; j++)
				dst[j] += src[j];
		}
	}
	return length;
}
#endif /* CONFIG_PROC_FS */
#endif /* CONFIG_NET_CLS_ROUTE */

static __initdata unsigned long rhash_entries;
static int __init set_rhash_entries(char *str)
{
	if (!str)
		return 0;
	rhash_entries = simple_strtoul(str, &str, 0);
	return 1;
}
__setup("rhash_entries=", set_rhash_entries);

int __init ip_rt_init(void)
{
	int rc = 0;

	rt_hash_rnd = (int) ((num_physpages ^ (num_physpages>>8)) ^
			     (jiffies ^ (jiffies >> 7)));

#ifdef SB_USE_DST_ARRAY
        SessionSrcInfoInit();
#endif
#ifdef CONFIG_NET_CLS_ROUTE
	{
	int order;
	for (order = 0;
	     (PAGE_SIZE << order) < 256 * sizeof(struct ip_rt_acct) * NR_CPUS; order++)
		/* NOTHING */;
	ip_rt_acct = (struct ip_rt_acct *)__get_free_pages(GFP_KERNEL, order);
	if (!ip_rt_acct)
		panic("IP: failed to allocate ip_rt_acct\n");
	memset(ip_rt_acct, 0, PAGE_SIZE << order);
	}
#endif

	ipv4_dst_ops.kmem_cachep = kmem_cache_create("ip_dst_cache",
						     sizeof(struct rtable),
						     0, SLAB_HWCACHE_ALIGN,
						     NULL, NULL);

	if (!ipv4_dst_ops.kmem_cachep)
		panic("IP: failed to allocate ip_dst_cache\n");

	rt_hash_table = (struct rt_hash_bucket *)
		alloc_large_system_hash("IP route cache",
					sizeof(struct rt_hash_bucket),
					rhash_entries,
					(num_physpages >= 128 * 1024) ?
					15 : 17,
					HASH_HIGHMEM,
					&rt_hash_log,
					&rt_hash_mask,
					0);
	memset(rt_hash_table, 0, (rt_hash_mask + 1) * sizeof(struct rt_hash_bucket));
	rt_hash_lock_init();

	ipv4_dst_ops.gc_thresh = (rt_hash_mask + 1);
	ip_rt_max_size = (rt_hash_mask + 1) * 16;

	devinet_init();
	ip_fib_init();

	init_timer(&rt_flush_timer);
	rt_flush_timer.function = rt_run_flush;
	init_timer(&rt_periodic_timer);
	rt_periodic_timer.function = rt_check_expire;
#ifndef CONFIG_NK_ROUTE_CACHE_MODIFY
	init_timer(&rt_secret_timer);
	rt_secret_timer.function = rt_secret_rebuild;
#endif
	/* All the timers, started at system startup tend
	   to synchronize. Perturb it a bit.
	 */
	rt_periodic_timer.expires = jiffies + net_random() % ip_rt_gc_interval +
					ip_rt_gc_interval;
	add_timer(&rt_periodic_timer);

#ifndef CONFIG_NK_ROUTE_CACHE_MODIFY
	rt_secret_timer.expires = jiffies + net_random() % ip_rt_secret_interval +
		ip_rt_secret_interval;
	add_timer(&rt_secret_timer);
#endif

#ifdef CONFIG_PROC_FS
	{
	struct proc_dir_entry *rtstat_pde = NULL; /* keep gcc happy */
	if (!proc_net_fops_create("rt_cache", S_IRUGO, &rt_cache_seq_fops) ||
	    !(rtstat_pde = create_proc_entry("rt_cache", S_IRUGO, 
			    		     proc_net_stat))) {
		return -ENOMEM;
	}
	rtstat_pde->proc_fops = &rt_cpu_seq_fops;
	}
#ifdef CONFIG_NET_CLS_ROUTE
	create_proc_read_entry("rt_acct", 0, proc_net, ip_rt_acct_read, NULL);
#endif
#endif
#ifdef CONFIG_XFRM
	xfrm_init();
	xfrm4_init();
#endif

#ifdef CONFIG_NK_PROTO_BINDING
	init_timer(&fire_spec_timer);
	fire_spec_timer.function = fire_spec_aging;
	fire_spec_timer.expires = jiffies + 10*HZ;
	add_timer(&fire_spec_timer);
#endif

	return rc;
}

/*2006/11/1 trenchen : support remote manage*/
struct net_device * nk_multipath_select(u32 daddr , u32 saddr , u32 proto , u32 dport , u32 sport , u8 tos)
{
	struct net_device *device=NULL;
	unsigned	hash_tmp;
	struct rtable *rth;

	read_lock_bh(&dev_base_lock);
	//printk("multipath_select look for: daddr[%x]-p[%d] saddr[%x]-p[%d] \n",daddr,dport,saddr,sport);
	for (device = dev_base ; device != NULL ; device = device->next)
	{
#ifdef ROUTING_HASH_IMPROVE
//Michael Lu add
		hash_tmp = rt_hash_code(saddr, daddr ^ ((device->ifindex) << 5), dport , sport, tos);
#else
		hash_tmp = rt_hash_code(saddr, daddr ^ ((device->ifindex) << 5), tos);
#endif

		rcu_read_lock();
		for (rth = rcu_dereference(rt_hash_table[hash_tmp].chain); rth; rth = rcu_dereference(rth->u.rt_next)) {
		if (rth->fl.fl4_dst == saddr &&
		    rth->fl.fl4_src == daddr &&
		    rth->fl.iif == device->ifindex &&
		    rth->fl.oif == 0 &&
#ifdef CONFIG_NK_PROTO_BINDING
		    /*2006/11/1 trenchen : protocol binding*/
		    proto == rth->fl.fl4_protocol_type &&
		    sport == rth->fl.fl4_dst_port &&
		    dport == rth->fl.fl4_src_port 
		    /**********************************/
#endif
			) {
				//printk("multipath_select result: daddr[%x] saddr[%x] dev[%s]\n",daddr,saddr,device->name);
				rcu_read_unlock();
				read_unlock_bh(&dev_base_lock);
				return device;
			}
		}
		rcu_read_unlock();
	}
	read_unlock_bh(&dev_base_lock);

	return 0;
}
/***********************************************/


EXPORT_SYMBOL(__ip_select_ident);
EXPORT_SYMBOL(ip_route_input);
EXPORT_SYMBOL(ip_route_output_key);
