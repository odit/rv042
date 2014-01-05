/*
 * net/sched/cls_u32.c	Ugly (or Universal) 32bit key Packet Classifier.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Alexey Kuznetsov, <kuznet@ms2.inr.ac.ru>
 *
 *	The filters are packed to hash tables of key nodes
 *	with a set of 32bit key/mask pairs at every node.
 *	Nodes reference next level hash tables etc.
 *
 *	This scheme is the best universal classifier I managed to
 *	invent; it is not super-fast, but it is not slow (provided you
 *	program it correctly), and general enough.  And its relative
 *	speed grows as the number of rules becomes larger.
 *
 *	It seems that it represents the best middle point between
 *	speed and manageability both by human and by machine.
 *
 *	It is especially useful for link sharing combined with QoS;
 *	pure RSVP doesn't need such a general approach and can use
 *	much simpler (and faster) schemes, sort of cls_rsvp.c.
 *
 *	JHS: We should remove the CONFIG_NET_CLS_IND from here
 *	eventually when the meta match extension is made available
 *
 *	nfmark match added by Catalin(ux aka Dino) BOIE <catab at umbrella.ro>
 */


#include <asm/uaccess.h>
#include <asm/system.h>
#include <linux/bitops.h>
#include <linux/config.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/socket.h>
#include <linux/sockios.h>
#include <linux/in.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/if_ether.h>
#include <linux/inet.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/notifier.h>
#include <linux/rtnetlink.h>
#include <net/ip.h>
#include <net/route.h>
#include <linux/skbuff.h>
#include <net/sock.h>
#include <net/act_api.h>
#include <net/pkt_cls.h>
//#include <net/pkt_sched.h>
//richie add for debug u32 filter
//#define QOS_DEBUG 1
#undef QOS_DEBUG

extern unsigned int device_ip;//richie add 060213: add to get device lan ip


struct tc_u_knode
{
	struct tc_u_knode	*next;
	u32			handle;
	struct tc_u_hnode	*ht_up;
	struct tcf_exts		exts;
#ifdef CONFIG_NET_CLS_IND
	char                     indev[IFNAMSIZ];
#endif
	u8			fshift;
	struct tcf_result	res;
	struct tc_u_hnode	*ht_down;
#ifdef CONFIG_CLS_U32_PERF
	struct tc_u32_pcnt	*pf;
#endif
#ifdef CONFIG_CLS_U32_MARK
	struct tc_u32_mark	mark;
#endif
	struct tc_u32_sel	sel;
};

struct tc_u_hnode
{
	struct tc_u_hnode	*next;
	u32			handle;
	u32			prio;
	struct tc_u_common	*tp_c;
	int			refcnt;
	unsigned int		divisor;
	struct tc_u_knode	*ht[1];
};

struct tc_u_common
{
	struct tc_u_common	*next;
	struct tc_u_hnode	*hlist;
	struct Qdisc		*q;
	int			refcnt;
	u32			hgenerator;
};

static struct tcf_ext_map u32_ext_map = {
	.action = TCA_U32_ACT,
	.police = TCA_U32_POLICE
};

static struct tc_u_common *u32_list;

static __inline__ unsigned u32_hash_fold(u32 key, struct tc_u32_sel *sel, u8 fshift)
{
	unsigned h = (key & sel->hmask)>>fshift;
//printk(KERN_EMERG "!!!!h[%d], key[%x], sel->hmask[%d], fshift[%d]!!!!!!\n", h, key, sel->hmask, fshift);
	return h;
}



static int u32_classify(struct sk_buff *skb, struct tcf_proto *tp, struct tcf_result *res)
{
	struct {
		struct tc_u_knode *knode;
		u8		  *ptr;
	} stack[TC_U32_MAXDEPTH];

	struct tc_u_hnode *ht = (struct tc_u_hnode*)tp->root;
	u8 *ptr = skb->nh.raw;
	struct tc_u_knode *n;
	int sdepth = 0;
	int off2 = 0;
	int sel = 0;
#ifdef CONFIG_CLS_U32_PERF
	int j;
#endif
	int i, r;
	int interface_number;
	unsigned int src_port,dst_port;
	unsigned int src_ip,dst_ip;

#ifdef QOS_DEBUG	
	printk(KERN_EMERG "==============u32_classify!!!!!!!!!!!!!=================\n");
#endif


	skb->qos_need = 0; //richie modify for upstream default rule
	//skb->qos_need = 1; 

next_ht:
	n = ht->ht[sel];

next_knode:
	if (n) {
		struct tc_u32_key *key = n->sel.keys;

#ifdef QOS_DEBUG
	printk(KERN_EMERG "0000000key->off[%d] key->offmask[%x] key->val[%d] key->mask[%x]\n",key->off,key->offmask,key->val,key->mask);
#endif

#ifdef CONFIG_CLS_U32_PERF
		n->pf->rcnt +=1;
		j = 0;
#endif
	
#ifdef CONFIG_CLS_U32_MARK
		if ((skb->nfmark & n->mark.mask) != n->mark.val) {
			n = n->next;
			goto next_knode;
		} else {
			n->mark.success++;
		}
#endif

		for (i = n->sel.nkeys; i>0; i--, key++) {

#ifdef QOS_DEBUG
	printk(KERN_EMERG "11111111key->off[%d] key->offmask[%x] key->val[%d] key->mask[%x]\n",key->off,key->offmask,key->val,key->mask);
#endif

#if 0
			if ((*(u32*)(ptr+key->off+(off2&key->offmask))^key->val)&key->mask) 
			{
#ifdef QOS_DEBUG
	printk(KERN_EMERG "key->off[%d] key->offmask[%x] key->val[%d] key->mask[%x]\n",key->off,key->offmask,key->val,key->mask);
#endif
				n = n->next;
				goto next_knode;
			}else
			{
			#ifdef QOS_DEBUG
				printk(KERN_EMERG "rule match\n");
			#endif
			}
#else
			//printk(KERN_EMERG "i[%d] orig_ip[%x] orig_port[%d]\n",i,skb->orig_ip,skb->orig_port);
			//--------printk("\n\n");
			//printk(KERN_EMERG "key->off[%d] key->offmask[%x] key->val[%x] key->mask[%x]\n",key->off,key->offmask,key->val,key->mask);
			//printk("skb->protool = %x\n", *(u32*)(ptr+8));
			//printk("skb->protool = %d\n", skb->nh.iph->protocol);
			//printk("key->off == %d",key->off);



			//printk("key->ssport[%d] key->esport[%d] key->sdport[%d] key->edport[%d]\n",key->ssport,key->esport,key->sdport,key->edport);
			if ((key->off == 12) || (key->off == 20))
			{
				
				//printk(KERN_EMERG "key->val[%08x] key->mask[%x] key->off = %x\n",key->val,key->mask, key->off);
				//printk("skb->orig_ip = %x\n", skb->orig_ip);
				//printk("key->ssrc_ip = %x\n", key->ssrc_ip);
				//printk("key->esrc_ip = %x\n", key->esrc_ip);
				

				//printk("key->off = %x\n", key->off);
				//printk("key->val[%08x] key->mask[%x]\n",key->val,key->mask);
				/*printk("skb->orig_src_ip = %x, skb->orig_src_port = %d, skb->orig_dst_port=%d\n", skb->orig_src_ip, skb->orig_src_port, skb->orig_dst_port);
				printk("key->ssrc_ip = %x, key->esrc_ip = %x\n", key->ssrc_ip, key->esrc_ip);
				printk("key->sdport = %d, key->edport = %d\n\n", key->sdport, key->edport);*/
				//printk("\nkey->sdport = %d, key->edport = %d\n", key->sdport, key->edport);
				//printk("key->ssport = %d, key->esport = %d\n", key->ssport, key->esport);
				//printk("skb->orig_src_port = %d, skb->orig_dst_port = %d\n", skb->orig_src_port, skb->orig_dst_port);



				if (key->off == 12)//src_ip
				{
					src_ip = (*(u32*)(ptr+key->off)) & 0xffffffff;//richie 1122
				#ifdef QOS_DEBUG
					printk(KERN_EMERG "src_ip[%x], device_ip[%x]\n",src_ip, device_ip);
					printk(KERN_EMERG "***********check src IP*************\n");
					printk(KERN_EMERG "src_ip = %x\n", src_ip);
					printk(KERN_EMERG "skb->orig_src_ip = %x\n", skb->orig_src_ip);
					printk(KERN_EMERG "key->ssrc_ip = %x, key->esrc_ip = %x\n", key->ssrc_ip, key->esrc_ip);
				#endif
					//printk("key->ssrc_ip = %x, key->esrc_ip = %x\n", key->ssrc_ip, key->esrc_ip);
	
					//if ((!skb->orig_ip || ((skb->orig_ip^key->val)&key->mask)))
					if(device_ip == src_ip)//for UI
					{
						if (src_ip < key->ssrc_ip || src_ip > key->esrc_ip)
						{
						#ifdef QOS_DEBUG
							printk(KERN_EMERG "UI NOT match \n");
							printk(KERN_EMERG "************************\n");
						#endif
							n = n->next;
							goto next_knode;
						}else
						{
						#ifdef QOS_DEBUG
							printk(KERN_EMERG "UI match \n");
							printk(KERN_EMERG "************************\n");
						#endif
						}
					}
					else
					{
						if (!skb->orig_src_ip || (skb->orig_src_ip < key->ssrc_ip) || (skb->orig_src_ip > key->esrc_ip))
						{
						#ifdef QOS_DEBUG
							printk(KERN_EMERG "src ip NOT match \n");
							printk(KERN_EMERG "************************\n");
						#endif
							n = n->next;
							goto next_knode;
						}
						else
						{
						#ifdef QOS_DEBUG
							printk(KERN_EMERG "src ip match \n");
							printk(KERN_EMERG "************************\n");
						#endif
						}
					}
				}
				else if ((key->off == 20) && (key->mask == 0xffff0000))//src_port
					//else if ((key->off == 20) && (key->mask == 0xffff))//src_port : for little endian
				{
				#ifdef QOS_DEBUG
					printk(KERN_EMERG "********check source port********\n");
				#endif
					src_port = (*(u32*)(ptr+key->off)) & 0xffff0000;
				#ifdef QOS_DEBUG
					printk(KERN_EMERG "src_port[%d]\n",src_port);
					printk(KERN_EMERG "key->ssport = %x, key->esport = %x\n", key->ssport, key->esport);
				#endif
						//if ((src_port < key->ssport) || (src_port > key->esport))
						//--------printk("key->ssport = %x, key->esport = %x\n", key->ssport, key->esport);
						//--------printk("skb->orig_src_port = %x\n", skb->orig_src_port);
						//printk("\n************************\n");
						//printk("src_port[%x]\n",src_port);
						//printk("key->ssport = %x, key->esport = %x\n", key->ssport, key->esport);
				#ifdef QOS_DEBUG
					printk(KERN_EMERG "1src_port[%x]\n",ntohl(src_port));
				#endif
					src_port = ntohl(src_port);
					src_port >>=16;
				#ifdef QOS_DEBUG
					printk(KERN_EMERG "2src_port[%x]\n",src_port);
				#endif
					//printk(KERN_EMERG "3src_port[%x]\n",ntoh(src_port));
	
					if ((src_port < key->ssport) || (src_port > key->esport))
					//if (!skb->orig_src_port || (skb->orig_src_port < key->ssport) || skb->orig_src_port > key->esport))
					{
					#ifdef QOS_DEBUG
						printk(KERN_EMERG "source port NOT match \n");
						printk(KERN_EMERG "************************\n");
					#endif
						n = n->next;
						goto next_knode;
					}
					else
					{
					#ifdef QOS_DEBUG
						printk(KERN_EMERG "source port match \n");
						printk(KERN_EMERG "************************\n");
					#endif
					}
				}
				else if ((key->off == 20) && (key->mask == 0xffff))//dst_port
					//else if ((key->off == 20) && (key->mask == 0xffff0000))//dst_port : for little endian
				{
				#ifdef QOS_DEBUG
					printk(KERN_EMERG "**********check dest port********\n");
				#endif
					//int a = skb->h.uh->dest;
					//if(skb->h.th->dest)
					//	printk("skb->h.th->dest=%u, source=%u\n", ntohs(skb->h.th->dest), ntohs(skb->h.th->source));
					//printk("key->sdport = %d, key->edport = %d\n", key->sdport, key->edport);
					/*if (skb->orig_dst_port)
					{
						printk("skb->orig_dst_port = %u\n", ntohs(skb->orig_dst_port));
					}*/
					//printk("\n************************\n");
					/*dst_port = (*(u32*)(ptr+key->off)) & 0xffff;
					printk("dst_port1[%x]\n",dst_port);
					dst_port >>=16;
					printk("dst_port2[%x]\n",dst_port);
					dst_port = ntohl(dst_port);
					printk("dst_port3[%x]\n",dst_port);
					dst_port >>=16;
					printk("dst_port4[%x]\n",dst_port);*/
	
					/*dst_port = (*(u32*)(ptr+key->off)) & 0xffff0000;
					printk("dst_port5[%x]\n",dst_port);
					dst_port >>=16;
					printk("dst_port6[%d]\n",dst_port);*/
						
					dst_port = (*(u32*)(ptr+key->off)) & 0xffff;
				#ifdef QOS_DEBUG
					printk(KERN_EMERG "1dst_port[%x]\n",dst_port);
				#endif
	
					dst_port = ntohl(dst_port);
				#ifdef QOS_DEBUG
					printk(KERN_EMERG "2dst_port[%x]\n",dst_port);
					printk(KERN_EMERG "key->sdport = %x, key->edport = %x\n", key->sdport, key->edport);
				#endif
					/*printk("dst_port7[%d]\n",dst_port);
	
					printk("skb->orig_dst_port = %u\n", ntohs(skb->orig_dst_port));
					printk("key->sdport = %x, key->edport = %x\n", key->sdport, key->edport);*/
					//--------printk("dst_port[%x]\n",dst_port);
					//dst_port = ntohl(dst_port);
					//printk("dst_port=%d\n", dst_port);
					//--------printk("dst_port[%x]\n",dst_port);
	
					if ((dst_port < key->sdport) || (dst_port > key->edport))
					//if (!skb->h.th->dest || (skb->h.th->dest < key->sdport) || (skb->h.th->dest > key->edport))
					//if (!skb->orig_dst_port || (ntohs(skb->orig_dst_port) < key->sdport) || (ntohs(skb->orig_dst_port) > key->edport))
					{
					#ifdef QOS_DEBUG
						printk(KERN_EMERG "dest port NOT match \n");
						printk(KERN_EMERG "************************\n");
					#endif	
						n = n->next;
						goto next_knode;
					}
					else
					{
					#ifdef QOS_DEBUG
						printk(KERN_EMERG "dest port match \n");
						printk(KERN_EMERG "************************\n");
					#endif
					}
				}
			}
			else if(key->off == 16)//dst ip
			{
				dst_ip = (*(u32*)(ptr+key->off)) & 0xffffffff;
			#ifdef QOS_DEBUG	
				printk(KERN_EMERG "********check dst ip**************\n");
				printk(KERN_EMERG "dst_ip[%x]\n",dst_ip);
				printk(KERN_EMERG "key->sdst_ip = %x, key->edst_ip = %x\n", key->sdst_ip, key->edst_ip);
			#endif
	
				if (dst_ip < key->sdst_ip || dst_ip > key->edst_ip)
				{
				#ifdef QOS_DEBUG
					printk(KERN_EMERG "dst ip NOT match \n");
					printk(KERN_EMERG "************************\n");
				#endif
					n = n->next;
					goto next_knode;
				}
				else
				{
				#ifdef QOS_DEBUG
					printk(KERN_EMERG "dst ip match \n");
					printk(KERN_EMERG "************************\n");
				#endif
				}
			}
			else if(key->off == 100)//richie add to support inbound qos by interface
			{
			#ifdef QOS_DEBUG
				printk(KERN_EMERG "************check wan if ************\n");
				printk(KERN_EMERG "skb->src_interface_num = %d\n", skb->src_interface_num);
				printk(KERN_EMERG "key->wanif = %d\n", key->wanif);
			#endif
	
				if (!skb->src_interface_num || skb->src_interface_num != key->wanif)
				{
				#ifdef QOS_DEBUG
					printk(KERN_EMERG "wan interface NOT match \n");
					printk(KERN_EMERG "************************\n");
				#endif
					n = n->next;
					goto next_knode;
				}
				else
				{
				#ifdef QOS_DEBUG
					printk(KERN_EMERG "wan interface match \n");
					printk(KERN_EMERG "************************\n");
				#endif
				}
			}
			else if(key->off == 120)//richie add to support inbound qos by interface
			{
				//printk("skb->ap_type = %d\n", skb->ap_type);
				//printk("key->ap_type=%d\n", key->ap_type);
				if (!skb->ap_type || skb->ap_type != key->ap_type)
				{
					n = n->next;
					goto next_knode;
				}
				else
				{
				#ifdef QOS_DEBUG
					printk(KERN_EMERG "ap_type match \n");
				#endif
				}
			}
			else
			{
				//printk("come here!! \n");
			#ifdef QOS_DEBUG
				printk(KERN_EMERG "********check protocol*******\n");
			#endif
				if ((*(u32*)(ptr+key->off+(off2&key->offmask))^key->val)&key->mask)
				{
				#ifdef QOS_DEBUG
					printk(KERN_EMERG "*******protocol NOT match********\n");
				#endif
					n = n->next;
					goto next_knode;
				}
				else
				{
				#ifdef QOS_DEBUG
					printk(KERN_EMERG "*******protocol match********\n");
				#endif
				}
			}
#endif

#ifdef CONFIG_CLS_U32_PERF
			n->pf->kcnts[j] +=1;
			j++;
#endif
		}
		if (n->ht_down == NULL) {
check_terminal:
			if (n->sel.flags&TC_U32_TERMINAL) {

				*res = n->res;
#ifdef CONFIG_NET_CLS_IND
				if (!tcf_match_indev(skb, n->indev)) 
				{
					//skb->qos_need = 0;//richie modify for upstream default rule
					skb->qos_need = 1;
				#ifdef QOS_DEBUG
					printk(KERN_EMERG "==>rule not match\n");
				#endif
					n = n->next;
					goto next_knode;
				}else
				{
					skb->qos_need = 1;
				#ifdef QOS_DEBUG
					printk(KERN_EMERG "==>rule match\n");
				#endif
				}
				//printk(KERN_EMERG "cls_u32: skb->qos_need[%d]\n", skb->qos_need);	
#endif
#ifdef CONFIG_CLS_U32_PERF
				n->pf->rhit +=1;
#endif
				r = tcf_exts_exec(skb, &n->exts, res);
				if (r < 0) {
					//skb->qos_need = 0;//richie modify for upstream default rule
					skb->qos_need = 1;
				#ifdef QOS_DEBUG
					printk(KERN_EMERG "rule not match\n");
				#endif
					n = n->next;
					goto next_knode;
				}else
				{
					skb->qos_need = 1;
				#ifdef QOS_DEBUG
					printk(KERN_EMERG "rule match\n");
				#endif
				}	

				return r;
			}
			n = n->next;
		#ifdef QOS_DEBUG
			//skb->qos_need = 0;//richie modify for upstream default rule
			skb->qos_need = 1;
			printk(KERN_EMERG "99999goto next_knode;\n");
		#endif
			goto next_knode;
		}

		/* PUSH */
		if (sdepth >= TC_U32_MAXDEPTH)
			goto deadloop;
		stack[sdepth].knode = n;
		stack[sdepth].ptr = ptr;
		sdepth++;
	
		ht = n->ht_down;
		sel = 0;
		if (ht->divisor)
		{
			if(n->sel.hoff == 12)//upstream hash
			{
				//sel = ht->divisor&u32_hash_fold(*(u32*)(ptr+n->sel.hoff), &n->sel,n->fshift);
				sel = ht->divisor&u32_hash_fold(skb->orig_src_ip, &n->sel,n->fshift);
#ifdef QOS_DEBUG	
				printk(KERN_EMERG "!!!!n->fshift[%d], n->sel.hoff[%d], sel[%d]!!!!!!\n", n->fshift, n->sel.hoff, sel);
#endif
			}else
			{
				sel = ht->divisor&u32_hash_fold(*(u32*)(ptr+n->sel.hoff), &n->sel,n->fshift);
#ifdef QOS_DEBUG	
				printk(KERN_EMERG "!!!!n->fshift[%d], n->sel.hoff[%d], sel[%d]!!!!!!\n", n->fshift, n->sel.hoff, sel);
#endif
			}
		}
	
		if (!(n->sel.flags&(TC_U32_VAROFFSET|TC_U32_OFFSET|TC_U32_EAT)))
		{
			//skb->qos_need = 0;//richie modify for upstream default rule
			skb->qos_need = 1;
		#ifdef QOS_DEBUG
			printk(KERN_EMERG "11111goto next_ht;\n");
		#endif
			goto next_ht;
		}

		if (n->sel.flags&(TC_U32_OFFSET|TC_U32_VAROFFSET)) {
			off2 = n->sel.off + 3;
			if (n->sel.flags&TC_U32_VAROFFSET)
				off2 += ntohs(n->sel.offmask & *(u16*)(ptr+n->sel.offoff)) >>n->sel.offshift;
				off2 &= ~3;
		}
		if (n->sel.flags&TC_U32_EAT) {
			ptr += off2;
			off2 = 0;
		}

		if (ptr < skb->tail)
		{
			//skb->qos_need = 0;//richie modify for upstream default rule
			skb->qos_need = 1;
		#ifdef QOS_DEBUG
			printk(KERN_EMERG "22222goto next_ht;\n");
		#endif
			goto next_ht;
		}
	}
	
	/* POP */
	if (sdepth--) {
		n = stack[sdepth].knode;
		ht = n->ht_up;
		ptr = stack[sdepth].ptr;
		goto check_terminal;
	}
	return -1;
	
deadloop:
	if (net_ratelimit())
	{
#ifdef QOS_DEBUG
		printk("cls_u32: dead loop\n");
#endif
	}
	return -1;
}



#if 0
//==========================>
static int u32_classify(struct sk_buff *skb, struct tcf_proto *tp, struct tcf_result *res)
{
	struct {
		struct tc_u_knode *knode;
		u8		  *ptr;
	} stack[TC_U32_MAXDEPTH];

	struct tc_u_hnode *ht = (struct tc_u_hnode*)tp->root;
	u8 *ptr = skb->nh.raw;
	struct tc_u_knode *n;
	int sdepth = 0;
	int off2 = 0;
	int sel = 0;
	int i;
	int interface_number;
	unsigned int src_port,dst_port;
	unsigned int src_ip,dst_ip;

	//unsigned int tmp1 , tmp2, tmp3, tmp4;

	//--->richie : get ixp0 ip
	struct net_device *device=NULL;
	struct in_device *in_dev = NULL;
	struct in_ifaddr *ifa_list = NULL;
	//unsigned int device_ip;


	//printk("=====================u32_classify\n");

	/*for (device = dev_base ; device != NULL ; device = device->next)
	{
		//printk("here1\n");
		if(!strcmp(device->name,"eth1"))
		{
			//printk("here2\n");
			if (device)
			{
				//printk("here3\n");
				in_dev = device->ip_ptr;
			}
			if (in_dev)
			{
				//printk("here4\n");
				ifa_list = in_dev->ifa_list;
			}
			if (ifa_list)
			{
				//printk("here5\n");
				device_ip = ifa_list->ifa_address;
				//printk("========device_ip[%x]========\n",device_ip);
			}
		}
	}//<---*/

#if !defined(__i386__) && !defined(__mc68000__)
	if ((unsigned long)ptr & 3)
		return -1;
#endif

next_ht:
	n = ht->ht[sel];
next_knode:
	if (n) {
		struct tc_u32_key *key = n->sel.keys;
		for (i = n->sel.nkeys; i>0; i--, key++) 
		{
			// -->
			// 2004/04/13 Ryan : using original src ip & original src port in sk_buff
			//					 to classify packet for u32 filter
#if 0
			if ((*(u32*)(ptr+key->off+(off2&key->offmask))^key->val)&key->mask) {
				n = n->next;
				goto next_knode;
			}
#else
			//printk("i[%d] orig_ip[%x] orig_port[%d]\n",i,skb->orig_ip,skb->orig_port);
			//--------printk("\n\n");
			//--------printk("key->off[%d] key->offmask[%x] key->val[%x] key->mask[%x]\n",key->off,key->offmask,key->val,key->mask);
			//printk("skb->protool = %x\n", *(u32*)(ptr+8));
			//printk("skb->protool = %d\n", skb->nh.iph->protocol);
			//printk("key->off == %d",key->off);



			//printk("key->ssport[%d] key->esport[%d] key->sdport[%d] key->edport[%d]\n",key->ssport,key->esport,key->sdport,key->edport);
			if ((key->off == 12) || (key->off == 20))
			{
				
#ifdef QOS_DEBUG	
				printk(KERN_EMERG "key->val[%08x] key->mask[%x]\n",key->val,key->mask);
				//printk(KERN_EMERG "skb->orig_ip = %x\n", skb->orig_ip);
				printk(KERN_EMERG "key->ssrc_ip = %x\n", key->ssrc_ip);
				printk(KERN_EMERG "key->esrc_ip = %x\n", key->esrc_ip);
				

				printk(KERN_EMERG "key->off = %x\n", key->off);
#endif
				//printk("key->val[%08x] key->mask[%x]\n",key->val,key->mask);
				/*printk("skb->orig_src_ip = %x, skb->orig_src_port = %d, skb->orig_dst_port=%d\n", skb->orig_src_ip, skb->orig_src_port, skb->orig_dst_port);
				printk("key->ssrc_ip = %x, key->esrc_ip = %x\n", key->ssrc_ip, key->esrc_ip);
				printk("key->sdport = %d, key->edport = %d\n\n", key->sdport, key->edport);*/
				//printk("\nkey->sdport = %d, key->edport = %d\n", key->sdport, key->edport);
				//printk("key->ssport = %d, key->esport = %d\n", key->ssport, key->esport);
				//printk("skb->orig_src_port = %d, skb->orig_dst_port = %d\n", skb->orig_src_port, skb->orig_dst_port);



				if (key->off == 12)//src_ip
				{
					src_ip = (*(u32*)(ptr+key->off)) & 0xffffffff;//richie 1122
					//printk("src_ip[%x], device_ip[%x]\n",src_ip, device_ip);

					
					printk("\n************************\n");
					printk("src_ip = %x\n", src_ip);
					//printk("skb->orig_src_ip = %x\n", skb->orig_src_ip);
					//printk("key->ssrc_ip = %x, key->esrc_ip = %x\n", key->ssrc_ip, key->esrc_ip);

					//if ((!skb->orig_ip || ((skb->orig_ip^key->val)&key->mask)))
					if(device_ip == src_ip)//for UI
					{
						if (src_ip < key->ssrc_ip || src_ip > key->esrc_ip)
						{
							//printk("goto next_knode\n");
							n = n->next;
							goto next_knode;
						}else
						{
							/*printk("UI match \n");
							printk("************************\n");*/
						}
					}
					else
					{
						if (!skb->orig_src_ip || (skb->orig_src_ip < key->ssrc_ip) || (skb->orig_src_ip > key->esrc_ip))
						{
							//printk("goto next_knode\n");
							n = n->next;
							goto next_knode;
						}
						else
						{
							/*printk("src ip match \n");
							printk("************************\n");*/
						}
					}
				}
				//else if ((key->off == 20) && (key->mask == 0xffff0000))//src_port
				else if ((key->off == 20) && (key->mask == 0xffff))//src_port : for little endian
				{
					//printk("check source port\n");
					src_port = (*(u32*)(ptr+key->off)) & 0xffff;
					//printk("src_port[%d]\n",src_port);
					//if ((src_port < key->ssport) || (src_port > key->esport))
					//--------printk("key->ssport = %x, key->esport = %x\n", key->ssport, key->esport);
					//--------printk("skb->orig_src_port = %x\n", skb->orig_src_port);
					//printk("\n************************\n");
					//printk("src_port[%x]\n",src_port);
					//printk("key->ssport = %x, key->esport = %x\n", key->ssport, key->esport);
					//--------printk("src_port[%x]\n",ntohl(src_port));
					src_port = ntohl(src_port);
					src_port >>=16;
					//--------printk("src_port[%x]\n",src_port);

					//printk("src_port[%x]\n",ntoh(src_port));


					if ((src_port < key->ssport) || (src_port > key->esport))
					//if (!skb->orig_src_port || (skb->orig_src_port < key->ssport) || (skb->orig_src_port > key->esport))
					{
							//printk("goto next_knode\n");
							n = n->next;
							goto next_knode;
					}
					else
					{
					 	/*printk("source port match \n");
						printk("************************\n");*/
					}
				}
				//else if ((key->off == 20) && (key->mask == 0xffff))//dst_port
				else if ((key->off == 20) && (key->mask == 0xffff0000))//dst_port : for little endian
				{
					//printk("check dest port\n");
					//int a = skb->h.uh->dest;
					//if(skb->h.th->dest)
					//	printk("skb->h.th->dest=%u, source=%u\n", ntohs(skb->h.th->dest), ntohs(skb->h.th->source));
					//printk("key->sdport = %d, key->edport = %d\n", key->sdport, key->edport);
					/*if (skb->orig_dst_port)
					{
						printk("skb->orig_dst_port = %u\n", ntohs(skb->orig_dst_port));
					}*/
					//printk("\n************************\n");

					/*dst_port = (*(u32*)(ptr+key->off)) & 0xffff;
					printk("dst_port1[%x]\n",dst_port);
					dst_port >>=16;
					printk("dst_port2[%x]\n",dst_port);
					dst_port = ntohl(dst_port);
					printk("dst_port3[%x]\n",dst_port);
					dst_port >>=16;
					printk("dst_port4[%x]\n",dst_port);*/

					/*dst_port = (*(u32*)(ptr+key->off)) & 0xffff0000;
					printk("dst_port5[%x]\n",dst_port);
					dst_port >>=16;
					printk("dst_port6[%d]\n",dst_port);*/
					dst_port = (*(u32*)(ptr+key->off)) & 0xffff0000;
					dst_port = ntohl(dst_port);
					/*printk("dst_port7[%d]\n",dst_port);

					printk("skb->orig_dst_port = %u\n", ntohs(skb->orig_dst_port));
					printk("key->sdport = %x, key->edport = %x\n", key->sdport, key->edport);*/
					//--------printk("dst_port[%x]\n",dst_port);
					//dst_port = ntohl(dst_port);
					//dst_port >>=16;
					//printk("dst_port=%d\n", dst_port);
					//--------printk("dst_port[%x]\n",dst_port);

					if ((dst_port < key->sdport) || (dst_port > key->edport))
					//if (!skb->h.th->dest || (skb->h.th->dest < key->sdport) || (skb->h.th->dest > key->edport))
					//if (!skb->orig_dst_port || (ntohs(skb->orig_dst_port) < key->sdport) || (ntohs(skb->orig_dst_port) > key->edport))
					{
							//printk("goto next_knode\n");
							n = n->next;
							goto next_knode;
					}
					else
					{
					 	/*printk("dest port match \n");
						printk("************************\n");*/
					}
				}
			}
			else if(key->off == 16)//dst ip
			{
				dst_ip = (*(u32*)(ptr+key->off)) & 0xffffffff;
				
				/*printk("\n************************\n");
				printk("dst_ip[%x]\n",dst_ip);
				printk("key->sdst_ip = %x, key->edst_ip = %x\n", key->sdst_ip, key->edst_ip);*/


				if (dst_ip < key->sdst_ip || dst_ip > key->edst_ip)
				{
					//printk("goto next_knode\n");
					n = n->next;
					goto next_knode;
				}
				else
				{
				 	/*printk("dst ip match \n");
					printk("************************\n");*/
				}
			}
			else if(key->off == 100)//richie add to support inbound qos by interface
			{
				/*printk("\n************************\n");
				printk("skb->src_interface_num = %d\n", skb->src_interface_num);
				printk("key->wanif = %d\n", key->wanif);*/

				if (!skb->src_interface_num || skb->src_interface_num != key->wanif)
				{
					//printk("goto next_knode\n");
					n = n->next;
					goto next_knode;
				}
				else
				{
				 	/*printk("wan interface match \n");*/
				}
				
			}
			else if(key->off == 120)//richie add to support inbound qos by interface
			{

				//printk("skb->ap_type = %d\n", skb->ap_type);
				//printk("key->ap_type=%d\n", key->ap_type);
				if (!skb->ap_type || skb->ap_type != key->ap_type)
				{
					n = n->next;
					goto next_knode;
				}
				/*else
				{
				 	printk("ap_type match \n");
				}*/
			}
			else
			{
				//printk("come here!! \n");
				if ((*(u32*)(ptr+key->off+(off2&key->offmask))^key->val)&key->mask)
				{
					//printk("nothing match \n");
					n = n->next;
					goto next_knode;
				}
				else
				{
					/*printk("protocol match \n");*/
				}
			}
#endif
			// <--
		}
		if (n->ht_down == NULL) {
check_terminal:
			if (n->sel.flags&TC_U32_TERMINAL) {
				*res = n->res;
#ifdef CONFIG_NET_CLS_POLICE
				if (n->police) {
					int pol_res = tcf_police(skb, n->police);
					if (pol_res >= 0)
						return pol_res;
				} else
#endif
					return 0;
			}
			n = n->next;
			goto next_knode;
		}

		/* PUSH */
		if (sdepth >= TC_U32_MAXDEPTH)
			goto deadloop;
		stack[sdepth].knode = n;
		stack[sdepth].ptr = ptr;
		sdepth++;

		ht = n->ht_down;
		sel = 0;
		if (ht->divisor)
			sel = ht->divisor&u32_hash_fold(*(u32*)(ptr+n->sel.hoff), &n->sel);

		if (!(n->sel.flags&(TC_U32_VAROFFSET|TC_U32_OFFSET|TC_U32_EAT)))
			goto next_ht;

		if (n->sel.flags&(TC_U32_OFFSET|TC_U32_VAROFFSET)) {
			off2 = n->sel.off + 3;
			if (n->sel.flags&TC_U32_VAROFFSET)
				off2 += ntohs(n->sel.offmask & *(u16*)(ptr+n->sel.offoff)) >>n->sel.offshift;
			off2 &= ~3;
		}
		if (n->sel.flags&TC_U32_EAT) {
			ptr += off2;
			off2 = 0;
		}

		if (ptr < skb->tail)
			goto next_ht;
	}

	/* POP */
	if (sdepth--) {
		n = stack[sdepth].knode;
		ht = n->ht_up;
		ptr = stack[sdepth].ptr;
		goto check_terminal;
	}
	return -1;

deadloop:
	if (net_ratelimit())
		printk("cls_u32: dead loop\n");
	return -1;
}
#endif


//<==========================
static __inline__ struct tc_u_hnode *
u32_lookup_ht(struct tc_u_common *tp_c, u32 handle)
{
	struct tc_u_hnode *ht;

	for (ht = tp_c->hlist; ht; ht = ht->next)
		if (ht->handle == handle)
			break;

	return ht;
}

static __inline__ struct tc_u_knode *
u32_lookup_key(struct tc_u_hnode *ht, u32 handle)
{
	unsigned sel;
	struct tc_u_knode *n = NULL;

	sel = TC_U32_HASH(handle);
	if (sel > ht->divisor)
		goto out;

	for (n = ht->ht[sel]; n; n = n->next)
		if (n->handle == handle)
			break;
out:
	return n;
}


static unsigned long u32_get(struct tcf_proto *tp, u32 handle)
{
	struct tc_u_hnode *ht;
	struct tc_u_common *tp_c = tp->data;

	if (TC_U32_HTID(handle) == TC_U32_ROOT)
		ht = tp->root;
	else
		ht = u32_lookup_ht(tp_c, TC_U32_HTID(handle));

	if (!ht)
		return 0;

	if (TC_U32_KEY(handle) == 0)
		return (unsigned long)ht;

	return (unsigned long)u32_lookup_key(ht, handle);
}

static void u32_put(struct tcf_proto *tp, unsigned long f)
{
}

static u32 gen_new_htid(struct tc_u_common *tp_c)
{
	/*int i = 0x800;

	do {
		if (++tp_c->hgenerator == 0x7FF)
			tp_c->hgenerator = 1;
	} while (--i>0 && u32_lookup_ht(tp_c, (tp_c->hgenerator|0x800)<<20));

	return i > 0 ? (tp_c->hgenerator|0x800)<<20 : 0;*/
	/*int i = 0x1000;

	//printk("33gen_new_htid:  tp_c->hgenerator[%x]\n", tp_c->hgenerator);

	do {
		if (++tp_c->hgenerator == 0xFFFF)
			tp_c->hgenerator = 1;
	}	while (--i>0 && u32_lookup_ht(tp_c, (tp_c->hgenerator|0x800)<<16));

	//printk("44i[%d]:  tp_c->hgenerator[%x], (tp_c->hgenerator|0x800)<<16)=[%x]\n", i, tp_c->hgenerator, (tp_c->hgenerator|0x800)<<16);

	return i > 0 ? (tp_c->hgenerator|0x800)<<16 : 0;*/
	int i = 0x1000;//richie1124

	printk("gen_new_htid\n");

	do {
		if (++tp_c->hgenerator == 0x7FF)
			tp_c->hgenerator = 1;
	}	while (--i>0 && u32_lookup_ht(tp_c, (tp_c->hgenerator|0x800)<<20));

	printk("44i[%d]:  tp_c->hgenerator[%x], (tp_c->hgenerator|0x800)<<16)=[%x]\n", i, tp_c->hgenerator, (tp_c->hgenerator|0x800)<<20);

	return i > 0 ? (tp_c->hgenerator|0x800)<<20 : 0;
}

static int u32_init(struct tcf_proto *tp)
{
	struct tc_u_hnode *root_ht;
	struct tc_u_common *tp_c;

	for (tp_c = u32_list; tp_c; tp_c = tp_c->next)
		if (tp_c->q == tp->q)
			break;

	root_ht = kmalloc(sizeof(*root_ht), GFP_KERNEL);
	if (root_ht == NULL)
		return -ENOBUFS;

	memset(root_ht, 0, sizeof(*root_ht));
	root_ht->divisor = 0;
	root_ht->refcnt++;
	//root_ht->handle = tp_c ? gen_new_htid(tp_c) : 0x80000000;
	//root_ht->handle = tp_c ? gen_new_htid(tp_c) : 0x08000000;
	root_ht->handle = tp_c ? gen_new_htid(tp_c) : 0x80000000;//richie1124

	root_ht->prio = tp->prio;

	if (tp_c == NULL) {
		tp_c = kmalloc(sizeof(*tp_c), GFP_KERNEL);
		if (tp_c == NULL) {
			kfree(root_ht);
			return -ENOBUFS;
		}
		memset(tp_c, 0, sizeof(*tp_c));
		tp_c->q = tp->q;
		tp_c->next = u32_list;
		u32_list = tp_c;
	}

	tp_c->refcnt++;
	root_ht->next = tp_c->hlist;
	tp_c->hlist = root_ht;
	root_ht->tp_c = tp_c;

	tp->root = root_ht;
	tp->data = tp_c;
	return 0;
}

static int u32_destroy_key(struct tcf_proto *tp, struct tc_u_knode *n)
{
	tcf_unbind_filter(tp, &n->res);
	tcf_exts_destroy(tp, &n->exts);
	if (n->ht_down)
		n->ht_down->refcnt--;
#ifdef CONFIG_CLS_U32_PERF
	if (n)
		kfree(n->pf);
#endif
	kfree(n);
	return 0;
}

static int u32_delete_key(struct tcf_proto *tp, struct tc_u_knode* key)
{
	struct tc_u_knode **kp;
	struct tc_u_hnode *ht = key->ht_up;

	if (ht) {
		for (kp = &ht->ht[TC_U32_HASH(key->handle)]; *kp; kp = &(*kp)->next) {
			if (*kp == key) {
				tcf_tree_lock(tp);
				*kp = key->next;
				tcf_tree_unlock(tp);

				u32_destroy_key(tp, key);
				return 0;
			}
		}
	}
	BUG_TRAP(0);
	return 0;
}

static void u32_clear_hnode(struct tcf_proto *tp, struct tc_u_hnode *ht)
{
	struct tc_u_knode *n;
	unsigned h;

	for (h=0; h<=ht->divisor; h++) {
		while ((n = ht->ht[h]) != NULL) {
			ht->ht[h] = n->next;

			u32_destroy_key(tp, n);
		}
	}
}

static int u32_destroy_hnode(struct tcf_proto *tp, struct tc_u_hnode *ht)
{
	struct tc_u_common *tp_c = tp->data;
	struct tc_u_hnode **hn;

	BUG_TRAP(!ht->refcnt);

	u32_clear_hnode(tp, ht);

	for (hn = &tp_c->hlist; *hn; hn = &(*hn)->next) {
		if (*hn == ht) {
			*hn = ht->next;
			kfree(ht);
			return 0;
		}
	}

	BUG_TRAP(0);
	return -ENOENT;
}

static void u32_destroy(struct tcf_proto *tp)
{
	struct tc_u_common *tp_c = tp->data;
	struct tc_u_hnode *root_ht = xchg(&tp->root, NULL);

	BUG_TRAP(root_ht != NULL);

	if (root_ht && --root_ht->refcnt == 0)
		u32_destroy_hnode(tp, root_ht);

	if (--tp_c->refcnt == 0) {
		struct tc_u_hnode *ht;
		struct tc_u_common **tp_cp;

		for (tp_cp = &u32_list; *tp_cp; tp_cp = &(*tp_cp)->next) {
			if (*tp_cp == tp_c) {
				*tp_cp = tp_c->next;
				break;
			}
		}

		for (ht=tp_c->hlist; ht; ht = ht->next)
			u32_clear_hnode(tp, ht);

		while ((ht = tp_c->hlist) != NULL) {
			tp_c->hlist = ht->next;

			BUG_TRAP(ht->refcnt == 0);

			kfree(ht);
		};

		kfree(tp_c);
	}

	tp->data = NULL;
}

static int u32_delete(struct tcf_proto *tp, unsigned long arg)
{
	struct tc_u_hnode *ht = (struct tc_u_hnode*)arg;

	if (ht == NULL)
		return 0;

	if (TC_U32_KEY(ht->handle))
		return u32_delete_key(tp, (struct tc_u_knode*)ht);

	if (tp->root == ht)
		return -EINVAL;

	if (--ht->refcnt == 0)
		u32_destroy_hnode(tp, ht);

	return 0;
}

static u32 gen_new_kid(struct tc_u_hnode *ht, u32 handle)
{
	struct tc_u_knode *n;
	//unsigned i = 0x7FF;
	unsigned i = 0x7F;

	for (n=ht->ht[TC_U32_HASH(handle)]; n; n = n->next)
		if (i < TC_U32_NODE(n->handle))
			i = TC_U32_NODE(n->handle);
	i++;

	//return handle|(i>0xFFF ? 0xFFF : i);
	return handle|(i>0xFF ? 0xFF : i);
}

static int u32_set_parms(struct tcf_proto *tp, unsigned long base,
			 struct tc_u_hnode *ht,
			 struct tc_u_knode *n, struct rtattr **tb,
			 struct rtattr *est)
{
	int err;
	struct tcf_exts e;

	err = tcf_exts_validate(tp, tb, est, &e, &u32_ext_map);
	if (err < 0)
		return err;

	err = -EINVAL;
	if (tb[TCA_U32_LINK-1]) {
		u32 handle = *(u32*)RTA_DATA(tb[TCA_U32_LINK-1]);
		struct tc_u_hnode *ht_down = NULL;

		if (TC_U32_KEY(handle))
			goto errout;

		if (handle) {
			ht_down = u32_lookup_ht(ht->tp_c, handle);

			if (ht_down == NULL)
				goto errout;
			ht_down->refcnt++;
		}

		tcf_tree_lock(tp);
		ht_down = xchg(&n->ht_down, ht_down);
		tcf_tree_unlock(tp);

		if (ht_down)
			ht_down->refcnt--;
	}
	if (tb[TCA_U32_CLASSID-1]) {
		n->res.classid = *(u32*)RTA_DATA(tb[TCA_U32_CLASSID-1]);
		tcf_bind_filter(tp, &n->res, base);
	}

#ifdef CONFIG_NET_CLS_IND
	if (tb[TCA_U32_INDEV-1]) {
		int err = tcf_change_indev(tp, n->indev, tb[TCA_U32_INDEV-1]);
		if (err < 0)
			goto errout;
	}
#endif
	tcf_exts_change(tp, &n->exts, &e);

	return 0;
errout:
	tcf_exts_destroy(tp, &e);
	return err;
}

static int u32_change(struct tcf_proto *tp, unsigned long base, u32 handle,
		      struct rtattr **tca,
		      unsigned long *arg)
{
	struct tc_u_common *tp_c = tp->data;
	struct tc_u_hnode *ht;
	struct tc_u_knode *n;
	struct tc_u32_sel *s;
	struct rtattr *opt = tca[TCA_OPTIONS-1];
	struct rtattr *tb[TCA_U32_MAX];
	u32 htid;
	int err;

	if (opt == NULL)
		return handle ? -EINVAL : 0;

	if (rtattr_parse_nested(tb, TCA_U32_MAX, opt) < 0)
		return -EINVAL;

	if ((n = (struct tc_u_knode*)*arg) != NULL) {
		if (TC_U32_KEY(n->handle) == 0)
			return -EINVAL;

		return u32_set_parms(tp, base, n->ht_up, n, tb, tca[TCA_RATE-1]);
	}

	if (tb[TCA_U32_DIVISOR-1]) {
		unsigned int divisor = *(unsigned int*)RTA_DATA(tb[TCA_U32_DIVISOR-1]);//richie1124
//printk(KERN_EMERG "u32_change:divisor[%x]\n", divisor);
		//if (--divisor > 0x100)//richie1124
		if (--divisor > 0x1000)
		{
//printk(KERN_EMERG "u32_change: error #1\n");
			return -EINVAL;
		}
//printk(KERN_EMERG "handle___1 [%x]\n", handle);
		if (TC_U32_KEY(handle))
			return -EINVAL;
		if (handle == 0) {
			handle = gen_new_htid(tp->data);
			if (handle == 0)
				return -ENOMEM;
		}
		ht = kmalloc(sizeof(*ht) + divisor*sizeof(void*), GFP_KERNEL);
		if (ht == NULL)
			return -ENOBUFS;
		memset(ht, 0, sizeof(*ht) + divisor*sizeof(void*));
		ht->tp_c = tp_c;
		ht->refcnt = 0;
		ht->divisor = divisor;
		ht->handle = handle;
		ht->prio = tp->prio;
		ht->next = tp_c->hlist;
		tp_c->hlist = ht;
		*arg = (unsigned long)ht;
		return 0;
	}

	if (tb[TCA_U32_HASH-1]) {
		htid = *(unsigned*)RTA_DATA(tb[TCA_U32_HASH-1]);
		if (TC_U32_HTID(htid) == TC_U32_ROOT) {
			ht = tp->root;
			htid = ht->handle;
		} else {
			ht = u32_lookup_ht(tp->data, TC_U32_HTID(htid));
			if (ht == NULL)
				return -EINVAL;
		}
	} else {
		ht = tp->root;
		htid = ht->handle;
	}

	if (ht->divisor < TC_U32_HASH(htid))
		return -EINVAL;

	if (handle) {
		if (TC_U32_HTID(handle) && TC_U32_HTID(handle^htid))
			return -EINVAL;
		handle = htid | TC_U32_NODE(handle);
	} else
		handle = gen_new_kid(ht, htid);

	if (tb[TCA_U32_SEL-1] == 0 ||
	    RTA_PAYLOAD(tb[TCA_U32_SEL-1]) < sizeof(struct tc_u32_sel))
		return -EINVAL;

	s = RTA_DATA(tb[TCA_U32_SEL-1]);

	n = kmalloc(sizeof(*n) + s->nkeys*sizeof(struct tc_u32_key), GFP_KERNEL);
	if (n == NULL)
		return -ENOBUFS;

	memset(n, 0, sizeof(*n) + s->nkeys*sizeof(struct tc_u32_key));
#ifdef CONFIG_CLS_U32_PERF
	n->pf = kmalloc(sizeof(struct tc_u32_pcnt) + s->nkeys*sizeof(u64), GFP_KERNEL);
	if (n->pf == NULL) {
		kfree(n);
		return -ENOBUFS;
	}
	memset(n->pf, 0, sizeof(struct tc_u32_pcnt) + s->nkeys*sizeof(u64));
#endif

	memcpy(&n->sel, s, sizeof(*s) + s->nkeys*sizeof(struct tc_u32_key));
	n->ht_up = ht;
	n->handle = handle;
{
	u8 i = 0;
	u32 mask = s->hmask;
	if (mask) {
		while (!(mask & 1)) {
			i++;
			mask>>=1;
		}
	}
	n->fshift = i;
}

#ifdef CONFIG_CLS_U32_MARK
	if (tb[TCA_U32_MARK-1]) {
		struct tc_u32_mark *mark;

		if (RTA_PAYLOAD(tb[TCA_U32_MARK-1]) < sizeof(struct tc_u32_mark)) {
#ifdef CONFIG_CLS_U32_PERF
			kfree(n->pf);
#endif
			kfree(n);
			return -EINVAL;
		}
		mark = RTA_DATA(tb[TCA_U32_MARK-1]);
		memcpy(&n->mark, mark, sizeof(struct tc_u32_mark));
		n->mark.success = 0;
	}
#endif

	err = u32_set_parms(tp, base, ht, n, tb, tca[TCA_RATE-1]);
	if (err == 0) {
		struct tc_u_knode **ins;
		for (ins = &ht->ht[TC_U32_HASH(handle)]; *ins; ins = &(*ins)->next)
			if (TC_U32_NODE(handle) < TC_U32_NODE((*ins)->handle))
				break;

		n->next = *ins;
		wmb();
		*ins = n;

		*arg = (unsigned long)n;
		return 0;
	}
#ifdef CONFIG_CLS_U32_PERF
	if (n)
		kfree(n->pf);
#endif
	kfree(n);
	return err;
}

static void u32_walk(struct tcf_proto *tp, struct tcf_walker *arg)
{
	struct tc_u_common *tp_c = tp->data;
	struct tc_u_hnode *ht;
	struct tc_u_knode *n;
	unsigned int h;

	if (arg->stop)
		return;

	for (ht = tp_c->hlist; ht; ht = ht->next) {
		if (ht->prio != tp->prio)
			continue;
		if (arg->count >= arg->skip) {
			if (arg->fn(tp, (unsigned long)ht, arg) < 0) {
				arg->stop = 1;
				return;
			}
		}
		arg->count++;
		for (h = 0; h <= ht->divisor; h++) {
			for (n = ht->ht[h]; n; n = n->next) {
				if (arg->count < arg->skip) {
					arg->count++;
					continue;
				}
				if (arg->fn(tp, (unsigned long)n, arg) < 0) {
					arg->stop = 1;
					return;
				}
				arg->count++;
			}
		}
	}
}

static int u32_dump(struct tcf_proto *tp, unsigned long fh,
		     struct sk_buff *skb, struct tcmsg *t)
{
	struct tc_u_knode *n = (struct tc_u_knode*)fh;
	unsigned char	 *b = skb->tail;
	struct rtattr *rta;

	if (n == NULL)
		return skb->len;

	t->tcm_handle = n->handle;

	rta = (struct rtattr*)b;
	RTA_PUT(skb, TCA_OPTIONS, 0, NULL);

	if (TC_U32_KEY(n->handle) == 0) {
		struct tc_u_hnode *ht = (struct tc_u_hnode*)fh;
		u32 divisor = ht->divisor+1;
		RTA_PUT(skb, TCA_U32_DIVISOR, 4, &divisor);
	} else {
		RTA_PUT(skb, TCA_U32_SEL,
			sizeof(n->sel) + n->sel.nkeys*sizeof(struct tc_u32_key),
			&n->sel);
		if (n->ht_up) {
			//u32 htid = n->handle & 0xFFFFF000;//richie1124
			u32 htid = n->handle & 0xFFFFFF00;
//printk(KERN_EMERG "htid[%x], \n", htid);
			RTA_PUT(skb, TCA_U32_HASH, 4, &htid);
		}
		if (n->res.classid)
			RTA_PUT(skb, TCA_U32_CLASSID, 4, &n->res.classid);
		if (n->ht_down)
			RTA_PUT(skb, TCA_U32_LINK, 4, &n->ht_down->handle);

#ifdef CONFIG_CLS_U32_MARK
		if (n->mark.val || n->mark.mask)
			RTA_PUT(skb, TCA_U32_MARK, sizeof(n->mark), &n->mark);
#endif

		if (tcf_exts_dump(skb, &n->exts, &u32_ext_map) < 0)
			goto rtattr_failure;

#ifdef CONFIG_NET_CLS_IND
		if(strlen(n->indev))
			RTA_PUT(skb, TCA_U32_INDEV, IFNAMSIZ, n->indev);
#endif
#ifdef CONFIG_CLS_U32_PERF
		RTA_PUT(skb, TCA_U32_PCNT, 
		sizeof(struct tc_u32_pcnt) + n->sel.nkeys*sizeof(u64),
			n->pf);
#endif
	}

	rta->rta_len = skb->tail - b;
	if (TC_U32_KEY(n->handle))
		if (tcf_exts_dump_stats(skb, &n->exts, &u32_ext_map) < 0)
			goto rtattr_failure;
	return skb->len;

rtattr_failure:
	skb_trim(skb, b - skb->data);
	return -1;
}

static struct tcf_proto_ops cls_u32_ops = {
	.next		=	NULL,
	.kind		=	"u32",
	.classify	=	u32_classify,
	.init		=	u32_init,
	.destroy	=	u32_destroy,
	.get		=	u32_get,
	.put		=	u32_put,
	.change		=	u32_change,
	.delete		=	u32_delete,
	.walk		=	u32_walk,
	.dump		=	u32_dump,
	.owner		=	THIS_MODULE,
};

static int __init init_u32(void)
{
#ifdef QOS_DEBUG
	printk("u32 classifier\n");
#endif
#ifdef CONFIG_CLS_U32_PERF
	printk("    Perfomance counters on\n");
#endif
#ifdef CONFIG_NET_CLS_POLICE
	printk("    OLD policer on \n");
#endif
#ifdef CONFIG_NET_CLS_IND
	printk("    input device check on \n");
#endif
#ifdef CONFIG_NET_CLS_ACT
	printk("    Actions configured \n");
#endif
	return register_tcf_proto_ops(&cls_u32_ops);
}

static void __exit exit_u32(void) 
{
	unregister_tcf_proto_ops(&cls_u32_ops);
}

module_init(init_u32)
module_exit(exit_u32)
MODULE_LICENSE("GPL");
