/*For url hit rate*/
#define URL_MAJOR       241   /* This exists in the experimental range */
#define URL_NAME        "url"
//#define URL_GET_STATS  0xFEED0004
#define	URL_GET_STATS _IOWR('r', 100, url_ranking_t *)
#define	SIOCGETIPSERDOWNLOAD _IOWR('r', 106, ip_service_sort_t *)
#define	SIOCGETWANTOTAL _IOWR('r', 107, u_int32_t)
#define	SET_TIMER _IOWR('r', 108, NULL)
#define	SIOCGETIPTOTAL _IOWR('r', 109, u_int32_t)
#define	SIOCGETSESSIONUPSORT _IOWR('r', 110, u_int32_t)
#define	SIOCGETSESSIONDOWNSORT _IOWR('r', 111, u_int32_t)
#define SIOCGETIPDOWNSORT _IOWR('r', 112, u_int32_t)
#define SIOCGETWANSESSION _IOWR('r', 113, u_int32_t)
#define SIOCGETIPUPSORT _IOWR('r', 114, u_int32_t)
#define SIOCGETSERVDOWNSORT _IOWR('r', 115, u_int32_t)
#define SIOCGETSERVUPSORT _IOWR('r', 116, service_sort_t)
#define	SIOCGETIPSESSION _IOWR('r', 117, ip_session_t *)
#define	SIOCCLRIPSESSION _IOWR('r', 118, ip_session_t *)
#define SIOCGETSESSIONNUMBER _IOWR('r', 119, int)
#define	SIOCSENDWANNAME _IOWR('r', 120, struct interface_session *)
#define SIOCGETIPMACLEARN _IOWR('r', 121, struct ip_mac_bind_entry *)
#define	SIOCSENDARPTOGW _IOWR('r', 122, nk_send_arp_t *)
#define SIOGETWANNEWSESSION _IOWR('r', 123, int)
#define SIOCCLEANIPCONN _IOWR('r', 130, u_int32_t)
#define SIOCCLEANIPCONN2 _IOWR('r', 131, struct port_forwarding *)
#define SIOCCLEARBADSESSION _IOWR('r', 132, u_int32_t)
#define SIOCCLEARIPSESSION _IOWR('r', 133, u_int32_t)
#define SIOCCLEARUDPSESSION _IOWR('r', 134, u_int32_t)
#define SIOCDELETEDIPSESSION _IOWR('r', 135, u_int32_t)
/* arp spoof protect incifer 2006/06/28 */
#ifdef CONFIG_SUPPORT_ARP_SPOOF_PROTECT
	#define ARPSPOOFPROTECT _IOWR('r', 140, int)
	#define SIOCSETSTATICARP _IOWR('r', 141, int)
	#define SIOCCLRSTATICARP _IO('r', 142)
#endif
#define SIOCAGINGTIME _IOWR('r', 143, u_int32_t)
#define SIOCCLRAGINGTIME _IOWR('r', 144, u_int32_t)
//-->
/* Arp Spoof Protect:send ip mac bind rule to kernel incifer 2006/08/17 */
//#ifdef CONFIG_SUPPORT_ARP_SPOOF_PROTECT
#if 1
	#define SIOCADDIPMACBIND _IOWR('r', 145, int)
	#define SIOCCLRIPMACBIND _IOWR('r', 146, int)
#endif

//-->
/*2007/12/14 trenchen : change compiler flag*/
/*2006/07/05 Ryoko add support netbios*/
//#ifdef CONFIG_IPSEC_NETBIOS_BC
#ifdef CONFIG_NK_IPSEC_NETBIOS_BC
#define SIOCADIPSCEBC _IOWR('r', 124, struct nat_ipsec_bc_t *)
#define SIOCRMIPSCEBC _IOWR('r', 125, struct nat_ipsec_bc_t *)
#endif
/* support set fw_options incifer 1128 */
#ifdef CONFIG_SUPPORT_IP_MAC_BINDING
#define SIOCFWOPTS _IOWR('r', 150, nk_fw_opt_t)
#endif

/*Support session limit 2007/03/12*/
#ifdef CONFIG_NK_SESSION_LIMIT
#define SIOCSESSIONLIMITSETUP _IOWR('r', 151, struct session_limit_setting_t *)
#define SIOCSESSIONLIMITENHANCEINIT _IOWR('r', 152, struct session_limit_enhance_list_t *)
#define SIOCSESSIONLIMITENHANCE _IOWR('r', 153, struct session_limit_enhance_list_t *)
#define SIOCGETSESSIONLIMITBLOCKIP  _IOWR('r', 154, struct session_limit_enhance_list_t *)
#endif

//#ifdef CONFIG_NK_QOS_SCHED
#if 1
    #define SIOCQOSIPSORT _IOWR('r',161,smart_qos_ip_t *)
    #define SIOCCLRQOSIPSORT _IOWR('r',162,smart_qos_ip_t *)
    #define SIOCQOSIPSORTUP _IOWR('r',163,smart_qos_ip_t *)
    #define SIOCQOSIPSORTDOWN _IOWR('r',164,smart_qos_ip_t *)
    #define SIOCQOSIPCOPY _IOWR('r',165,smart_qos_ip_t *)
#endif

#if 1
#define SIOCSENDFWSET _IOWR('r',170,firewall_setting_t *)
#endif

#ifdef CONFIG_NK_TRANSPARENT_BRIDGE
	#define SENDARPTOTRANSPARENTBRIDGE _IOWR('r', 171, nk_send_arp_t *)
#endif

/* support IpBalance Struct Aging -- incifer 2009/01 */
#ifdef CONFIG_IPBALANCE_STRUCT_AGING
	#define IPBALANCE_STRUCT_AGING _IOWR('r', 172, int *)
#endif


#ifdef CONFIG_NK_RESTRICT_APP
#define SIOCRESTRICTAPP _IOWR('r', 175, struct restrict_app_t *)
#define SIOCEXCEPTIONQQCLEARLIST _IOWR('r', 173, int)
#define SIOCEXCEPTIONQQADDLIST _IOWR('r', 174, struct exception_qq_t *)
#endif

/* add by chihmou, support pptp trunking 2008/06/11 */
#ifdef CONFIG_NK_PPTP_TRUNKING
	#define SIOCSETPPTPTRUNKINGTABLE _IOW('r', 175, struct PortTableHead *)
	#define SIOCDISPPTPTRUNKINGTABLE _IO('r', 176)
#endif

//Rain add for Anti-Virus
#define SIOCSENDANTIVIRUSSET _IOWR('r',177,anti_virus_setting_t *)

#define SIOCWEBSERVICEQUERY   _IOWR('r', 178, struct web_service_port_t *)

#ifdef CONFIG_NK_SUPPORT_USB_3G
	#define USB_LED_ON _IOWR('r', 179, int *)
	#define USB_LED_OFF _IOWR('r', 180, int *)	
#endif

//20100104 trenchen : support split dns
#define SIOCADDNSSPL _IOWR('r', 181, struct ipsec_split_dns_t *)
#define SIOCRMDNSSPL _IOWR('r', 182, struct ipsec_split_dns_t *)

#if CONFIG_NK_URL_TMUFE_FILTER
#define SIOSENDURLFILTERSETTING _IOWR('r', 183, u_int32_t)
#define SIORECVURLFILTERSETTING _IOWR('r', 184, u_int32_t)
#define SIOSENDURLFILTERSTATICS _IOWR('r', 185, u_int32_t)
#define SIORECVURLFILTERSTATICS _IOWR('r', 186, u_int32_t)
#define SIOSENDURLFILTERLIC     _IOWR('r', 187, u_int32_t)
#define SIORECVURLFILTERLIC     _IOWR('r', 188, u_int32_t)
#define SIOURLFILTERINIT        _IOWR('r', 189, u_int32_t)
#define SIOURLFILTERDEBUG       _IOWR('r', 190, u_int32_t)
#endif


//-->
/* This macro is used to prevent dereferencing of NULL pointers. If
 * a pointer argument is NULL, this will return -EINVAL */
#define NULL_CHECK(ptr)    \
   if ((ptr) == NULL)  return -EINVAL

#ifndef _IOCTL_H
#define _IOCTL_H
/* support set fw_options incifer 1128 */
#ifdef CONFIG_SUPPORT_IP_MAC_BINDING
typedef struct nk_fw_opt{
	int nk_fw_block_not_on_list;
	int nk_fw_block_wrong_ip;
}nk_fw_opt_t;
#endif
//-->

#ifdef CONFIG_NK_RESTRICT_APP
typedef struct restrict_app
{
	u_int32_t  except_s_ip[100];
	u_int32_t  except_e_ip[100];
	char block_qq;
}restrict_app_t;

typedef struct exception_qq
{
	u_int64_t qq_Num;
	struct exception_qq *next;
}exception_qq_t;
#endif


typedef	struct ip_session {
    u_int32_t	src_ip;
    u_int32_t	dst_ip;
    u_int16_t proto;
    u_int16_t wan_num;
    u_int16_t	src_port;
    u_int16_t	dst_port;
    u_int32_t up_bytes;
    u_int32_t down_bytes;
    u_int32_t wan_ip;
    struct ip_session *next;
} ip_session_t;

typedef struct RouterModeInfo1 {
	u_int32_t rgw;
	u_int32_t rip1;
	u_int32_t rip2;
	u_int32_t rip3;
	u_int32_t rip4;
} RouterModeInfo1;

/** interface session information */
typedef struct interface_session {
	char name[8];
	u_int32_t all_session_cnt;
	int current_session_cnt;
	u_int32_t pre_session_cnt;
	int delta_session_cnt;
	u_int32_t wan_ip;
	u_int32_t wan_mask;
	char laninterface[8];
	u_int32_t lanip;
	u_int32_t lanmask;
	#ifdef CONFIG_NK_TRANSPARENT_BRIDGE_RANGE_NUM
	u_int32_t internallanip1[CONFIG_NK_TRANSPARENT_BRIDGE_RANGE_NUM];
		u_int32_t internallanip2[CONFIG_NK_TRANSPARENT_BRIDGE_RANGE_NUM];
	#else
	u_int32_t internallanip1;
	u_int32_t internallanip2;
	u_int32_t internallanip3;
	u_int32_t internallanip4;
	#endif
	u_int32_t msubnet[8];
	u_int32_t msubmask[8];
	u_int32_t dmz_start;
	u_int32_t dmz_end;
	u_int32_t dmz_ip;
	u_int32_t dmz_mask;
	#
	struct RouterModeInfo1 routerinfo[3];
} interface_session_t;

typedef struct ip_mac_bind_entry {
    struct ip_mac_bind_entry *next;
    u_int32_t ip;
    u_int8_t mac[6];
    int number;
} ip_mac_bind_entry_t;

typedef	struct nk_send_arp {
    u_int32_t	src_ip;
    u_int32_t	dst_ip;
    char dev_name[8];
    /* Arp Spoof Protect incifer 2006/08/17 */
#ifdef CONFIG_SUPPORT_ARP_SPOOF_PROTECT
    int		arp_num;
#endif
} nk_send_arp_t;

//#ifdef CONFIG_NK_QOS_SCHED
#if 1
typedef struct smart_qos_ip
{
    struct in_addr ip;
    u_int64_t up_bw;
    u_int64_t down_bw;
    u_int64_t session;
    struct smart_qos_ip *next;
}smart_qos_ip_t;
#endif

typedef struct port_forwarding {
    u_int32_t ip;
    int id;
    int enabled;
    int change;
    int port;
    int protocol;
    //struct port_forwarding *next;
} port_forwarding_t;

typedef struct dmzhost {
    u_int32_t ip;
} dmzhost_t;

//2007/12/14 trenchen : support netbios broadcast
#ifdef CONFIG_NK_IPSEC_NETBIOS_BC
typedef	struct nat_ipsec_bc {
    struct nat_ipsec_bc *next;
    unsigned int local_bc;		/* local broadcast IP */
    unsigned int remote_bc; 		/* remote broadcast IP */
    char tunnel_name[25];
} nat_ipsec_bc_t;
#endif

#ifdef AUTOCONF_INCLUDED
#ifndef SPLITSTRUCT
#define SPLITSTRUCT
//20100103 trenchen : support split dns
#define SPDNSMAXDNAME 4
#define SPDNSMAXIP 2
typedef struct ipsec_split_dns_t{
	struct ipsec_split_dns_t *next;
	//20100509 trenchen : bug fix, domain name buffer length increase to 64byte
	//char domain_name[SPDNSMAXDNAME][32];
	char domain_name[SPDNSMAXDNAME][64];
	u32 ip[SPDNSMAXIP];
	unsigned int NumDname;
	unsigned int NumIp;
	char tunnel_name[NAME_MAX];
} ipsec_split_dns_t;

struct ipsec_split_dns_head{
	struct ipsec_split_dns_t *start;
	spinlock_t SplitdnsLock;
};

typedef struct dns_split_hash{
	u32 dstip;
	u32 rdns_ip[SPDNSMAXIP];
	u16 dns_id;
	u8 dns_split_server;
	u8 dns_split_ignore;
	struct dns_split_hash *next;
} dns_split_hash_t;

#endif
#endif

#if 0
/*2006/07/05 Ryoko add support netbios*/
#ifdef CONFIG_IPSEC_NETBIOS_BC
typedef	struct nat_ipsec_bc {
    struct nat_ipsec_bc_cmd *next;
    unsigned int local_bc;		/* local broadcast IP */
    unsigned int remote_bc; 		/* remote broadcast IP */
    char tunnel_name[8];
} nat_ipsec_bc_t;
#endif
#endif

// Session Limit
//#ifdef CONFIG_NK_SESSION_LIMIT
#if CONFIG_NK_SESSION_LIMIT

typedef	struct session_limit {
    u_int32_t	ip;
    u_int32_t	session_cnt;
    u_int32_t	tcp_session_cnt; //20091125
    u_int32_t	udp_session_cnt;
    u_int32_t block_timer;
    struct session_limit *next;
    //struct session_limit **pnext;
} session_limit_t;

typedef	struct session_limit_setting {
  u_int32_t session_limit_status;
  u_int32_t session_limit_max_session1;
  u_int32_t session_limit_max_session2;
  u_int32_t max_tcp; //20091125 Yami, count tcp/udp sessions
  u_int32_t max_udp;
  u_int32_t session_limit_block_minute1;
  u_int32_t session_limit_block_minute2;
  unsigned int scheduler;	/*0=disabled, 1=enabled*/
  unsigned int days_match;	/* 1 bit per day (bit 0 = Sunday) */
  unsigned int time_start;	/* 0 < time_start < 24*60*60-1 = 86399 */
  unsigned int time_stop;	/* 0 < time_end   < 24*60*60-1 = 86399 */
} session_limit_setting_t;
#endif

#ifdef CONFIG_NK_SESSION_LIMIT_ENHANCE
typedef struct session_limit_enhance_list {/* Session Limit Enhance:save exception rule incifer 2006/09/03 */
	u_int32_t sip;
	u_int32_t eip;
	int typeName;
	int sport;
	int eport;
	int max_session;
	struct session_limit_enhance_list *next;
}session_limit_enhance_list_t;
typedef struct session_limit_enhance_node_list {/* Session Limit Enhance incifer 2006/09/03 */
    struct session_limit_enhance_list *source;
    struct session_limit_enhance_node_list *next;
}session_limit_enhance_node_list_t;
#endif

#if 1

#define bzero(a,c) memset(a,0,c)

typedef struct url_entry {
    struct url_entry *next;
    char *url;
    u_int32_t cnt;
} url_entry_t;

typedef struct ser_entry {
    struct ser_entry *next;
    u_int32_t proto;
    u_int32_t port;
    u_int32_t cnt;
} ser_entry_t;

typedef struct host_entry {
    struct host_entry *next;
    u_int32_t ip;
    url_entry_t *url_list[7];
    ser_entry_t *ser_list[8];
    u_int32_t total_bytes[8];
} host_entry_t;

#define URL_RANKING_CNT 10

typedef struct url_ranking {
    u_int32_t ip;
    u_int32_t date;
    char url[URL_RANKING_CNT][64];
    u_int32_t cnt[URL_RANKING_CNT];
} url_ranking_t;


#define IP_SERVICE_SORT_CNT 20

/** service sorting */
typedef	struct service_sort {
    //ipstate_t *ips;
    u_int64_t delta_byte_cnt;
    u_int32_t proto;
    u_int32_t port_num;
    struct service_sort *next;
} service_sort_t;


typedef	struct ip_service_sort {
    u_int32_t ip;
    u_int32_t date;
    u_int32_t total;
    service_sort_t service[IP_SERVICE_SORT_CNT];
} ip_service_sort_t;


/** ip sorting */
typedef	struct ip_sort {
    //ipstate_t *ips;
    u_int64_t delta_byte_cnt;
    u_int32_t ip;
    struct ip_sort *next;
} ip_sort_t;

typedef struct traffic {
	struct ip_conntrack *ct;
	struct traffic *next;
} traffic_t;

typedef struct del_session {
	struct ip_conntrack *ct;
	struct del_session *next;
} del_session_t;

//Rain add for Anti-Virus -->
typedef struct anti_virus_setting {
	int av_enabled;
	int lan_enabled;
	int wan_enabled;
	int dmz_enabled;
	int http_enabled;
	int http_action;
	int ftp_enabled;
	int ftp_action;	
} anti_virus_setting_t;
// <-- Rain

#if 1
typedef struct firewall_setting {
	int tr_enable;
	int qos;
	int smart_qos;
	int cal_cnt_period;
	int pingofdeath;
	int multicast_enable;
	int denypolicy;
} firewall_setting_t;
#endif
#if 0
typedef struct ser_ranking {
    u_int32_t ip;
    u_int32_t date;
    u_int32_t proto[IP_SERVICE_SORT_CNT];
    u_int32_t port[IP_SERVICE_SORT_CNT];
    u_int32_t cnt[IP_SERVICE_SORT_CNT];
} ser_ranking_t;
#endif

#endif

#ifndef NIPQUAD
    #define NIPQUAD(addr) \
        ((unsigned char *)&addr)[0], \
        ((unsigned char *)&addr)[1], \
        ((unsigned char *)&addr)[2], \
        ((unsigned char *)&addr)[3]
#endif

typedef struct web_service_port {
	unsigned short sport;
	unsigned short dport;
	unsigned int sip;
	int age;
	struct web_service_port *next;
} web_service_port_t;


typedef	struct nk_agingtime {
    u_int32_t	protocol;
    u_int32_t port;
    u_int32_t timeout;
    struct nk_agingtime *next;
    //struct session_limit **pnext;
} nk_agingtime_t;

#endif



