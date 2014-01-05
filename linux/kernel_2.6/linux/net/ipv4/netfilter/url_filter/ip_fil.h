#ifndef	__IP_FIL_H__
#define	__IP_FIL_H__
/** Packet in filtering information */
typedef	struct	fr_info	{
    void *fin_dp;		/**< start of data past IP header */
    u16	fin_dlen;		/**< length of data portion of packet */
//    struct sk_buff **fin_mp;	/**< pointer to pointer to mbuf */
    struct iphdr *fin_ip;
    struct in_addr url_filter_src;
    void *apps_data;
} fr_info_t;
#endif

