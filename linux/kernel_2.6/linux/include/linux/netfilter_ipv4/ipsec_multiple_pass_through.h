#ifndef _IPSEC_MULTIPLE_PASS_THROUGH
#define _IPSEC_MULTIPLE_PASS_THROUGH

/**
	include file
**/
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4/ip_conntrack.h>


/**
	define content
**/
#define ISAKMP_PORT	(500)	 	/* UDP */
#define ISAKMP_TIMEOUT	3600	/* seconds */


/**
	declare structure
**/
/* Internet Security Association and Key Management Protocol(ISAKMP Header) */
struct isakmp_hdr
{
	unsigned int init_cookie[2];
	unsigned int resp_cookie[2];
	unsigned int next_payload :8,
	maj_ver :4,
	min_ver   :4,
	exchang_type:8,
	flag:8;
	unsigned int msg_id;
	unsigned int len;
};

struct isakmp_trigger
{
	struct list_head list;		/* Trigger list */
	struct timer_list timeout;	/* Timer for list destroying */
	u_int32_t srcip;			/* Outgoing source address */
	u_int32_t dstip;			/* Outgoing destination address */
	u_int32_t init_cookie[2]; 	/*ex:[1]0x31D69A44[2]D1A6EB87 */
	u_int32_t time;
	u_int16_t next_payload;
	u_int8_t reply;				/* Confirm a reply connection */
};


/**
	declare function
**/
/* isakmp helper */
int isakmp_help(struct ip_conntrack *ct, enum ip_conntrack_info ctinfo, unsigned int hooknum, struct sk_buff **pskb);

#endif
