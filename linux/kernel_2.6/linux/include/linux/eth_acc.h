
/*For url hit rate*/
#define ETH_ACC_MAJOR       238   /* This exists in the experimental range */
#define ETH_ACC_NAME        "eth_acc"

#define	SIOCFLUSH_CACHE_BY_IP _IOWR(ETH_ACC_MAJOR, 101, u_int32_t *)
#define SIOCFLUSH_CACHE_BY_WAN _IOWR(ETH_ACC_MAJOR, 102, char *)
#define	SIOCFLUSH_CACHE_BY_IP_PORT _IOWR(ETH_ACC_MAJOR, 103, struct port_forwarding *)
#define	SIOCFLUSH_CACHE_BY_FIP _IOWR(ETH_ACC_MAJOR, 104, u_int32_t *)

#define	MAC_ADDR_FILITER _IOWR(ETH_ACC_MAJOR, 113, u_int32_t *)
typedef struct mac_filiter_enabled {
    unsigned short enabled;
    unsigned short wan_num;
} mac_filiter_enabled_t;

#ifdef CONFIG_NK_DOS_ENHANCEMENT
typedef struct dos_setting {
    unsigned int total_threshold_wan;
    unsigned int total_threshold_lan;
    unsigned int perip_threshold_wan;
    unsigned int perip_threshold_lan;
    unsigned int per_dst_ip_threshold_lan;
    unsigned short block_time_wan;
    unsigned short block_time_lan;
    unsigned short total_wan_cnt;
    unsigned short total_lan_cnt;
} dos_setting_t;

typedef struct dos_setup {
    unsigned char dos_tcp_syn_enabled;
    unsigned char dos_udp_enabled;
    unsigned char dos_icmp_enabled;
    dos_setting_t dos_setting[3];
    unsigned char except_src_ip_enabled;
    unsigned char except_dst_ip_enabled;
    unsigned int except_dst_ip[5];
    unsigned int internal_except_dst_ip[1];
} dos_setup_t;

typedef struct dos_ip {
    unsigned int packet_cnt[3];
    unsigned short block_timer;
    unsigned int ip;
/**************need remove when commit local cgi*********/
    struct dos_block_ip *block_p;
    struct dos_ip *next;
/****************************************************************/
} dos_ip_t;

typedef struct dos_to_ui_ip {
    //unsigned int packet_cnt[3];
    unsigned short block_timer;
    unsigned int ip;
    //struct dos_block_ip *block_p;
    struct dos_to_ui_ip *next;
} dos_to_ui_ip_t;

typedef struct dos_except_src_ip {
    unsigned int s_ip;
    unsigned int e_ip;
    struct dos_except_src_ip *next;
} dos_except_src_ip_t;

/*typedef struct dos_block_ip {
    unsigned int ip;
    unsigned int rx_block_cnt;
    struct dos_block_ip *next;
    struct dos_block_ip **pnext;
} dos_block_ip_t;*/

#define SIOCDOSSETTING _IOWR(ETH_ACC_MAJOR, 105, dos_setup_t *)
#define SIOCDOSEXCEPTSRCIP _IOWR(ETH_ACC_MAJOR, 106, dos_except_src_ip_t *)
#define SIOCDOSINIT _IO(ETH_ACC_MAJOR, 107)
#define SIOCDOSSTART _IO(ETH_ACC_MAJOR, 108)
#define GET_DOS_BLOCK_IP _IOR(ETH_ACC_MAJOR, 109, dos_to_ui_ip_t *)
#define CLR_DOS_BLOCK_IP _IO(ETH_ACC_MAJOR, 110)
#define GET_DOS_BLOCK_DST_IP _IOR(ETH_ACC_MAJOR, 111, dos_to_ui_ip_t *)
#define CLR_DOS_BLOCK_DST_IP _IO(ETH_ACC_MAJOR, 112)

#endif
#define GET_WAN5_TYPE _IO(ETH_ACC_MAJOR, 114)

/* support IP balance enhancement: flush cache after 40s -- incifer 2008/11 */
#ifdef CONFIG_IPBALANCE_ENHANCE
struct wan_down_time_t{
	int inf;/* interface */
	int flag;/* 1: record wan down time; 0: reset wan down time */
};
#define	CAPTURE_WAN_DOWN_TIME _IOWR(ETH_ACC_MAJOR, 115, struct wan_down_time_t *)
#endif

/* Support Show DOS Block IP on Console -- incifer 2009/02 */
#define SHOW_DOS_BLOCK_SRC_IP 				_IO(ETH_ACC_MAJOR, 116)
#define SHOW_DOS_BLOCK_DST_IP 				_IO(ETH_ACC_MAJOR, 117)

/**
		When Ethernet Recieve Src IP==LAN IP, Show it -- incifer 2009/02
		Sync File: getlink.h, eth_acc.h
**/
#define SHOW_RECIEVE_LAN 							_IOWR(ETH_ACC_MAJOR, 118, u_int32_t *)
#define SHOW_RECIEVE_LAN_DISABLE 		_IO(ETH_ACC_MAJOR, 119)
