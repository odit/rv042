/*
 * iptables module to match IP address ranges
 *
 * (C) 2003 Jozsef Kadlecsik <kadlec@blackhole.kfki.hu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_ipv4/ipt_iprange.h>
#include <linux/inetdevice.h>
#include <linux/igmp.h>
#include <linux/netfilter_ipv4/ioctl.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jozsef Kadlecsik <kadlec@blackhole.kfki.hu>");
MODULE_DESCRIPTION("iptables arbitrary IP range match module");

#if 0
#define DEBUGP printk
#else
#define DEBUGP(format, args...)
#endif

extern struct firewall_setting fw_setting;
#define MAX_WAN_NUM (CONFIG_NK_NUM_WAN+CONFIG_NK_NUM_DMZ)
#ifdef CONFIG_NK_DYNAMIC_PORT_NUM
extern interface_session_t wan_interface_session[CONFIG_NK_NUM_MAX_WAN];
#else
extern interface_session_t wan_interface_session[MAX_WAN_NUM];
#endif

typedef struct mc_node{
	__u32	gpsrc;
	__u64	jiffy;
	struct mc_node *next;
}mc_node_t;

/* purpose : multicast pass through   author : selena.peng date : 2010-07-12 */
/* description : use hash instead of link-list */
mc_node_t *mc_queue[IPRANGE_MC_HASH_SIZE];
int mc_session_num;
mc_node_t clist[IPRANGE_MC_CLEAR_NUM];
mc_node_t *ptr_used;
mc_node_t *ptr_queue;

void print_clear_list(void)
{
	mc_node_t *a = NULL;
	a = ptr_used;

	DEBUGP("\n==clear list===");
	while(a)
	{
		DEBUGP("%d==",a->gpsrc);
		DEBUGP("%lX",a->jiffy);
		a = a->next;
	}
}

void print_hash_table(void)
{
	int i;
	mc_node_t *a = NULL;

	DEBUGP("\n==hash table===");
	for(i=0;i<IPRANGE_MC_HASH_SIZE;i++)
	{
		a = mc_queue[i];
		while(a)
		{
			DEBUGP("\n==%d==",i);
			DEBUGP("%d==",mc_queue[i]->gpsrc);
			DEBUGP("%lX",mc_queue[i]->jiffy);
			a = a->next;
		}
	}
}

int getIndex(__u32 add)
{
	return (add & (IPRANGE_MC_HASH_SIZE-1));
}

void modify_clear_list(mc_node_t * new, mc_node_t * mid)
{
	mc_node_t* temp;

	if(!ptr_queue) /* full clear list */
	{
		if(ptr_used == new)
		{
			ptr_used->gpsrc = mid->gpsrc;
			ptr_used->jiffy = mid->jiffy;
		}
		else
		{
			ptr_used->gpsrc = mid->gpsrc;
			ptr_used->jiffy = mid->jiffy;
			temp = ptr_used->next;
			ptr_used->next = new->next;
			new->next = ptr_used;
			ptr_used = temp;
		}
	}
	else
	{
		temp = ptr_queue->next;
		ptr_queue->gpsrc = mid->gpsrc;
		ptr_queue->jiffy = mid->jiffy;
		if(!new) /* the newest */
		{
			ptr_queue->next = ptr_used;
			ptr_used = ptr_queue;
		}
		else
		{
			ptr_queue->next = new->next;
			new->next = ptr_queue;
		}
		ptr_queue = temp;
	}
	/* print_clear_list(); */
}

void remove_node(__u32	gpSrc)
{
	mc_node_t *a = NULL, *b = NULL;
	int index = getIndex(gpSrc);
	a = mc_queue[index];

	DEBUGP("\n==remove%d===",gpSrc);
	if(a)
	{
		do
		{
			if(a->gpsrc == gpSrc)
			{
				if(b)
				{
					b->next = a->next;
				}
				else
				{
					mc_queue[index] = NULL;
				}
				kfree(a);
				mc_session_num-- ;
				break;
			}
			else
			{
				b = a;
				a = b->next;
			}
		}
		while(a);
	}
	DEBUGP("\nmc_session_num=%d",mc_session_num);
}

void clear_mc_hash_table(void)
{
	mc_node_t *a = NULL, *b=NULL, *c=NULL;
	int i;
	int session_count = 0;

	/* init clear table*/
	memset(clist,0,sizeof(clist));
	ptr_used = NULL;
	ptr_queue = clist;
	for(i=0;i<(IPRANGE_MC_CLEAR_NUM-1);i++)
	{
		a = (clist+i);
		a->next = (clist+i+1);
	}
	a = (clist+(IPRANGE_MC_CLEAR_NUM-1));
	a->next = NULL;

 	/* get top 5 oldest sessions */
 	for(i=0;i<IPRANGE_MC_HASH_SIZE;i++)
 	{
 		a =  mc_queue[i];
 		while(a)
 		{
			session_count++;
 			b = ptr_used;
 			c = NULL;
 			do
 			{
 				if(b)
 				{
 					if(a->jiffy < b->jiffy) /* older */
 					{
 						c = b;
						if(!b->next)
						{
							modify_clear_list(c,a);
							break;
						}
					}
					else /* newer */
					{
						if(c)
						{
							modify_clear_list(c,a);
							break;
						}
						else if(ptr_queue)
						{
							modify_clear_list(NULL,a);
							break;
						}
						else
						{
							break;
						}
					}
					b = b->next;
				}
				else /* 1st, ptr_queue!=null */
				{
					modify_clear_list(NULL,a);
					break;
				}
			}while(b);
			a = a->next;	
		}
	}
	a = ptr_used;
	/* print_hash_table(); */
	printk("\n[Warning] MulticastPassThrough - The system does not support multicast group more than %d!\n",IPRANGE_MC_MAX_QUEUE_LEN);
 	while(a)
 	{
		printk("=>Remove %d ",a->gpsrc);
		remove_node(a->gpsrc);
		session_count--;
		a = a->next;
	}
	mc_session_num = session_count;
	/* print_hash_table(); */
}

mc_node_t * search_node(__u32 mcip)
{
	mc_node_t *a = NULL;
	a = mc_queue[getIndex(mcip)];

	if(a)
	{
		do
		{
			if(a->gpsrc == mcip)
			{
				a->jiffy = jiffies;
				return a;
			}
			else
			{
				a = a->next;
			}
		}
		while(a);
	}
	return NULL;
}

void add_node(__u32 gpSrc)
{
	mc_node_t *a = NULL, *b = NULL;
	int index = 0;

	if(search_node(gpSrc))
	{
		return;
	}

	DEBUGP("\n==add%d",gpSrc);

	if(mc_session_num>IPRANGE_MC_MAX_QUEUE_LEN)
	{
		clear_mc_hash_table();
	}
	a = (mc_node_t *)kmalloc(sizeof(mc_node_t), GFP_ATOMIC);
	if(!a)
	{
		return;
	}
	a->gpsrc = gpSrc;
	a->jiffy = jiffies;
	a->next = NULL;
	index = getIndex(gpSrc);
	b = mc_queue[index];

	if(b)
	{
		a->next = b;
	}
	mc_queue[index] = a;
	mc_session_num++ ;
	DEBUGP("\nmc_session_num=%d",mc_session_num);
}

struct net_device *ipt_iprange_search_dev(char *name)
{
	struct net_device *device=NULL;

	read_lock_bh(&dev_base_lock);	
	for (device = dev_base ; device != NULL ; device = device->next)
	{
		if(strcmp(name,device->name)==0)
			break;
	}
	read_unlock_bh(&dev_base_lock);

	return device;
}

void ipt_iprange_xmit_pkt(const struct sk_buff *skb, char *dev_name)
{
	struct net_device *device = ipt_iprange_search_dev(dev_name);
	struct in_device *ipptr = NULL;
	struct in_ifaddr *a = NULL;
	struct sk_buff *mc = NULL;
	struct iphdr *new_iph = NULL;
	struct ethhdr *new_eth = NULL;
	int i;

	if(device == NULL)
	{
		return;
	}
	if(device->state == __LINK_STATE_NOCARRIER)
	{
		return;
	}
  	ipptr = __in_dev_get_rcu(device);
	if(ipptr == NULL)
	{
		return;
	}
	a = ipptr->ifa_list;
	if(a == NULL)
	{
		return;
	}
	/* mc = skb_copy(skb,GFP_ATOMIC); */
	mc = skb_clone((struct sk_buff *)skb,GFP_ATOMIC);

	/* start to modify skb content */
	mc->pkt_type = PACKET_HOST;
	mc->csum=0;

	new_iph = (mc)->nh.iph;
	new_eth = (struct ethhdr *)(mc)->mac.raw;
	for(i=0;i<ETH_ALEN;i++)
	{
		new_eth->h_source[i] = device->dev_addr[i];
	}
	mc->dev = device;

	new_iph->saddr = a->ifa_address ;
	new_iph->ttl = 127;
	new_iph->check =0;
	new_iph->check = ip_fast_csum((void *)new_iph, new_iph->ihl);

	skb_push(mc,ETH_HLEN);
	/* skb_push(mc,mc->data-mc->mac.raw); */
	dev_queue_xmit(mc);
}

void ipt_iprange_mc_pass_through(struct sk_buff *skb)
{
	struct iphdr *iph = (skb)->nh.iph;
	int i = 0;

	if(iph->protocol == 0x02) /* IGMP */
	{
		/* LAN->WAN pass and management */
		if(strcmp(skb->input_dev->name,wan_interface_session[0].laninterface) == 0) /* LAN */
		{
			struct igmphdr *igmph2 = (void *)iph + iph->ihl*4;
			struct igmpv3_report *igmph3 = (void *)iph + iph->ihl*4;
		
			switch(igmph2->type)
			{
				case IGMPV2_HOST_MEMBERSHIP_REPORT: /* 0x16 */
					add_node(igmph2->group);
					break;
				case IGMPV3_HOST_MEMBERSHIP_REPORT: /* 0x22 */
					{
						if( igmph3->grec->grec_type == IGMPV3_CHANGE_TO_EXCLUDE) /* 0x4 */
						{
							add_node(igmph3->grec->grec_mca);
						}
						else if( igmph3->grec->grec_type == IGMPV3_CHANGE_TO_INCLUDE) /* 0x3 */
						{
							remove_node(igmph3->grec->grec_mca);
						}
						/* else: do nothing */
					}
					break;
				case IGMP_HOST_LEAVE_MESSAGE: //0x17
					remove_node(igmph2->group);
					break;
				default:
					break;
			}

			for(i=0;i<MAX_WAN_NUM;i++)
			{
				ipt_iprange_xmit_pkt(skb,wan_interface_session[i].name); /* WAN */
			}
		}
		else /* WAN -> LAN */
		{
			ipt_iprange_xmit_pkt(skb,wan_interface_session[0].laninterface);
		}
	}
	else  /* none igmp pkt */
	{
		/* purpose : multicast pass through   author : selena.peng date : 2010-07-12 */
		/* description : all wans <-> lan pass through */
		if((mc_session_num>0) && (strcmp(skb->input_dev->name,wan_interface_session[0].laninterface) != 0)) /* WAN */
		{
			if(search_node(iph->daddr))
			{
				ipt_iprange_xmit_pkt(skb,wan_interface_session[0].laninterface); /* LAN */
			}
		}
	}
}

static int
match(const struct sk_buff *skb,
      const struct net_device *in,
      const struct net_device *out,
      const void *matchinfo,
      int offset, unsigned int protoff, int *hotdrop)
{
	const struct ipt_iprange_info *info = matchinfo;
	const struct iphdr *iph = skb->nh.iph;

	if (info->flags & IPRANGE_SRC) {
		if (((ntohl(iph->saddr) < ntohl(info->src.min_ip))
			  || (ntohl(iph->saddr) > ntohl(info->src.max_ip)))
			 ^ !!(info->flags & IPRANGE_SRC_INV)) {
			DEBUGP("src IP %u.%u.%u.%u NOT in range %s"
			       "%u.%u.%u.%u-%u.%u.%u.%u\n",
				NIPQUAD(iph->saddr),
			        info->flags & IPRANGE_SRC_INV ? "(INV) " : "",
				NIPQUAD(info->src.min_ip),
				NIPQUAD(info->src.max_ip));
			return 0;
		}
	}
	if (info->flags & IPRANGE_DST) {
		if (((ntohl(iph->daddr) < ntohl(info->dst.min_ip))
			  || (ntohl(iph->daddr) > ntohl(info->dst.max_ip)))
			 ^ !!(info->flags & IPRANGE_DST_INV)) {
			DEBUGP("dst IP %u.%u.%u.%u NOT in range %s"
			       "%u.%u.%u.%u-%u.%u.%u.%u\n",
				NIPQUAD(iph->daddr),
			        info->flags & IPRANGE_DST_INV ? "(INV) " : "",
				NIPQUAD(info->dst.min_ip),
				NIPQUAD(info->dst.max_ip));
			return 0;
		}

		/* purpose : 12496  author : selena.peng date : 2010-06-02
 		* description : surport multicast pass through without crash */
		if(fw_setting.multicast_enable == 1)
		{
			if( skb->input_dev != NULL)
			{
				if ((iph->daddr > 3758096384) && (iph->daddr < 4026531839))
				{
					ipt_iprange_mc_pass_through((struct sk_buff *)skb);
				}
			}
		}
	}
	return 1;
}

static int check(const char *tablename,
		 const void *inf,
		 void *matchinfo,
		 unsigned int matchsize,
		 unsigned int hook_mask)
{
	/* verify size */
	if (matchsize != IPT_ALIGN(sizeof(struct ipt_iprange_info)))
		return 0;

	return 1;
}

static struct ipt_match iprange_match = 
{ 
	.list = { NULL, NULL }, 
	.name = "iprange", 
	.match = &match, 
	.checkentry = &check, 
	.destroy = NULL, 
	.me = THIS_MODULE
};

static int __init init(void)
{
	/* purpose : multicast pass through   author : selena.peng date : 2010-07-12 */
	/* description : init hash queue */
	mc_session_num = 0;
	memset(mc_queue,0,sizeof(mc_queue));

	return ipt_register_match(&iprange_match);
}

static void __exit fini(void)
{
	ipt_unregister_match(&iprange_match);
}

module_init(init);
module_exit(fini);
