#ifndef _IPT_IPRANGE_H
#define _IPT_IPRANGE_H

#define IPRANGE_SRC		0x01	/* Match source IP address */
#define IPRANGE_DST		0x02	/* Match destination IP address */
#define IPRANGE_SRC_INV		0x10	/* Negate the condition */
#define IPRANGE_DST_INV		0x20	/* Negate the condition */
/* purpose : multicast pass through   author : selena.peng date : 2010-07-12
 * description : max queue length related to the num we support multicast sessions at the same time */
#define IPRANGE_MC_MAX_QUEUE_LEN	128
#define IPRANGE_MC_HASH_SIZE		(IPRANGE_MC_MAX_QUEUE_LEN>>2)
#define IPRANGE_MC_CLEAR_NUM		5

struct ipt_iprange {
	/* Inclusive: network order. */
	u_int32_t min_ip, max_ip;
};

struct ipt_iprange_info
{
	struct ipt_iprange src;
	struct ipt_iprange dst;

	/* Flags from above */
	u_int8_t flags;
};

#endif /* _IPT_IPRANGE_H */
