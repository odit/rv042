#include <net/if.h>
#include <nkuserlandconf.h>


struct interface *nk_if_statistic_get(char *target);
/* matrix: not port base vlan, just using port's PVID as vlan id 
 * and each port can join in one vlan group only.
 */
//#define PORT_VLAN_GROUP 1

struct user_net_device_stats {
    unsigned long long rx_packets;	/* total packets received       */
    unsigned long long tx_packets;	/* total packets transmitted    */
    unsigned long long rx_bytes;	/* total bytes received         */
    unsigned long long tx_bytes;	/* total bytes transmitted      */
    unsigned long rx_errors;	/* bad packets received         */
    unsigned long tx_errors;	/* packet transmit problems     */
    unsigned long rx_dropped;	/* no space in linux buffers    */
    unsigned long tx_dropped;	/* no space available in linux  */
    unsigned long rx_multicast;	/* multicast packets received   */
    unsigned long rx_compressed;
    unsigned long tx_compressed;
    unsigned long collisions;

    /* detailed rx_errors: */
    unsigned long rx_length_errors;
    unsigned long rx_over_errors;	/* receiver ring buff overflow  */
    unsigned long rx_crc_errors;	/* recved pkt with crc error    */
    unsigned long rx_frame_errors;	/* recv'd frame alignment error */
    unsigned long rx_fifo_errors;	/* recv'r fifo overrun          */
    unsigned long rx_missed_errors;	/* receiver missed packet     */
    /* detailed tx_errors */
    unsigned long tx_aborted_errors;
    unsigned long tx_carrier_errors;
    unsigned long tx_fifo_errors;
    unsigned long tx_heartbeat_errors;
    unsigned long tx_window_errors;
};

struct interface {
    struct interface *next, *prev; 
    char name[IFNAMSIZ];	/* interface name        */
    short type;			/* if type               */
    short flags;		/* various flags         */
    int metric;			/* routing metric        */
    int mtu;			/* MTU value             */
    int tx_queue_len;		/* transmit queue length */
    struct ifmap map;		/* hardware setup        */
    struct sockaddr addr;	/* IP address            */
    struct sockaddr dstaddr;	/* P-P IP address        */
    struct sockaddr broadaddr;	/* IP broadcast address  */
    struct sockaddr netmask;	/* IP network mask       */
    struct sockaddr ipxaddr_bb;	/* IPX network address   */
    struct sockaddr ipxaddr_sn;	/* IPX network address   */
    struct sockaddr ipxaddr_e3;	/* IPX network address   */
    struct sockaddr ipxaddr_e2;	/* IPX network address   */
    struct sockaddr ddpaddr;	/* Appletalk DDP address */
    struct sockaddr ecaddr;	/* Econet address        */
    int has_ip;
    int has_ipx_bb;
    int has_ipx_sn;
    int has_ipx_e3;
    int has_ipx_e2;
    int has_ax25;
    int has_ddp;
    int has_econet;
    char hwaddr[32];		/* HW address            */
    int statistics_valid;
    struct user_net_device_stats stats;		/* statistics            */
    int keepalive;		/* keepalive value for SLIP */
    int outfill;		/* outfill value for SLIP */
};

struct VlanGroup{
	unsigned int filter_id;
	unsigned int vlanid;
	unsigned int member_port;
	unsigned int tag_member;
	unsigned int filter_number;
};

struct TOS_VLAN_PRIORITY_T{
	unsigned int queue_id;
	unsigned int queue_priority;
	unsigned int queue_weight;
};

struct DSCP_PRIORITY_T{
	unsigned int class_id;
	char queue_priority[8];
};

struct  PortStatusNStatistics{
	/* Basic Port Config */
	unsigned int device_type;			// 0/1 LAN MII / WAN MII
	unsigned int port;				// port number which should be configure or read counter from
	unsigned int pvid;				// vlan id
	unsigned int enabled;				// enable/disable
	unsigned int link;				// link up/down
	unsigned int speed;				// 100/10
	unsigned int duplex;				// half/full
	unsigned int flow_control;			// enable/disable
	unsigned int auto_negotiation;			// enable/disable
	unsigned int priority;				// 0/1/2/3 -> low(Queue0) / low_medium(Queue1) / highmedium(Queue2) / high(Queue3)
	/* matrix: RVL200 support 802.1p / 802.1q */
	unsigned int output_tag;			// 0/1 output no tag / output tagging
	unsigned int trust_mode;			// 0/1/2 -> port base / cos(vlan) / dscp(tos)
	unsigned int support_8021p;
	unsigned int support_8021q;

	struct TOS_VLAN_PRIORITY_T tos_vlan_priority_t[8];  // priority 0-7 mapping to 4 priority queue
	struct DSCP_PRIORITY_T dscp_priority_t[8];

	/* Port Statistic */
	unsigned long long recv_packet_cnt;
	unsigned long long recv_byte_cnt;
	unsigned long long tran_packet_cnt;
	unsigned long long tran_byte_cnt;
	unsigned int collision_cnt;
	unsigned int error_cnt;

	/* VLAN Group */
	struct VlanGroup vlangroup;
	/* not use in RVL200 */
#if 0
	unsigned int dmz_led_status;
	unsigned int active_wan_num;
	unsigned int wan_con_led_status;
	unsigned int wan_link_led_status; //QVM1000
	unsigned int vlan;
	unsigned int mac_clone;
	unsigned int q_button_led_status;
	unsigned int tag_vlan; 			  //ADM6926 for tag_base/port_base vlan
	unsigned int switch_reinit;
#endif
};

#define MAX_VLAN_FILTER_RULE_NUMBER			16
#define LAN_MII 					0x0
#define WAN_MII 					0x1
#define PORT_TYPE_LAN					0x1
#define PORT_TYPE_WAN					0x2

#define SET_ADM69XX_1_PORT_STATUS			0x0
#define GET_ADM69XX_1_PORT_STATUS			0x1
#define SET_ADM69XX_2_PORT_STATUS			0x2
#define GET_ADM69XX_2_PORT_STATUS			0x3
#define SET_TOS_VLAN_PRIORITY				0x4
#define SET_PRIORITY_QUEUE_WEIGHT			0x5
#define SET_ADM69XX_VLAN_GROUP				0x6
#define SET_AC101_PHY_STATUS				0x8950 /* SET PHY STATUS */
#define GET_AC101_PHY_STATUS				0x8951 /* GET PHY STATUS */
#define SET_NETDEVICE_VID				0x8952 /* SET NETDEVICE VID*/

#define _1000Mbs					0x2
#define _100Mbs						0x1
#define _10Mbs						0x0
#define _FULL						0x1
#define _HALF						0x0
#define _ENABLE						0x1
#define _DISABLE					0x0
#define _UP						0x1
#define _DOWN						0x0
/* Port Priority Define, we just using _HIGH(High) & _LOW(Normal) Priority */
#define _HIGH						0x3
#define	_HIGH_MEDIUM					0x2
#define _LOW_MEDIUM					0x1
#define _LOW						0x0
#define _SUPPORT_DSCP_PRIORITY				0x2
#define _SUPPORT_VLAN_PRIORITY				0x1
#define _SUPPORT_PORT_PRIORITY				0x0
/* 802.1Q VLAN Port Include VLAN@Define */
#define _PORT_EXCLUDE_VLAN				"0"
#define _PORT_UNTAG_VLAN				"1"
#define _PORT_TAG_VLAN					"2"

/**
 *	ioctl define
 */
#define MAJOR_NUM		237
#define MINOR_NUM		1

#define SWITCH_SET_LAN_PORT_STATUS	_IOWR(MAJOR_NUM, 0x10, struct PortStatusNStatistics)
#define SWITCH_GET_LAN_PORT_STATUS	_IOWR(MAJOR_NUM, 0x11, struct PortStatusNStatistics)
#define SWITCH_SET_WAN_PORT_STATUS	_IOWR(MAJOR_NUM, 0x12, struct PortStatusNStatistics)
#define SWITCH_GET_WAN_PORT_STATUS	_IOWR(MAJOR_NUM, 0x13, struct PortStatusNStatistics)
#define SWITCH_SET_LAN_VLAN		_IOWR(MAJOR_NUM, 0x20, struct PortStatusNStatistics)
#define SWITCH_SET_MIRROR_PORT		_IOWR(MAJOR_NUM, 0x21, int)
#define SWITCH_SET_WAN_CONN_LED		_IOWR(MAJOR_NUM, 0x22, unsigned int)
#define SWITCH_SET_LED_ON		_IOWR(MAJOR_NUM, 0x23, unsigned int)
#define SWITCH_SET_LED_OFF		_IOWR(MAJOR_NUM, 0x24, unsigned int)
#define LAN_MII_CONTROL			_IOR(MAJOR_NUM, 0X28, int)
#define WAN_MII_CONTROL			_IOR(MAJOR_NUM, 0X29, int)

#define SWITCH_SET_DIMM_DIAG_LED_BLINK	_IOWR(MAJOR_NUM, 0x31, unsigned int)
#define SWITCH_SET_DIMM_DIAG_LED_OFF	_IOWR(MAJOR_NUM, 0x32, unsigned int)
#define SWITCH_SET_DIMM_DIAG_LED_ON	_IOWR(MAJOR_NUM, 0x33, unsigned int)
#define SWITCH_SET_DIMM_DIP_CHECK	_IOWR(MAJOR_NUM, 0x34, unsigned int)

#define ROUTE_TRANS_DST_TABLE		_IOWR(MAJOR_NUM, 0x35, struct SessionDstHead)
#define ROUTE_TRANS_LOCK_TABLE		_IOWR(MAJOR_NUM, 0x41, struct SessionDstHead)

/**
	ARL Table Data Index Structure -- Incifer 2009/02
	Sync File: getlink.h, switch.h, sw_template_sw_control.h
**/
#include <stdint.h>
struct ARL_Data{
	uint64_t mac_index;
	uint32_t vid_index;
	uint32_t data_value;
	int is_lan;
};
/**
		Set LAN MAC to Static MAC -- incifer 2009/02
		Sync File: getlink.h, switch.h
**/
#define SWITCH_READ_ARL_TEST						_IOWR(MAJOR_NUM, 0x36, struct ARL_Data)
#define SWITCH_WRITE_ARL_TEST						_IOWR(MAJOR_NUM, 0x37, struct ARL_Data)
#define SWITCH_READ_ARL									_IOWR(MAJOR_NUM, 0x38, struct ARL_Data)
#define SWITCH_WRITE_ARL								_IOWR(MAJOR_NUM, 0x39, struct ARL_Data)

/**
	sync: getlink.h, sw_template_bcm_acl.h switch.h
**/
typedef struct {
	uint32_t dir;/* LAN:0 or WAN:1 */
	int type;/* L3 Type(IPv4:0, NONIP:2) */
	uint32_t ip_proto;/* IP Protocol(ICMP:0x1, TCP:0x6, UDP:0x11, None:0x0) */
	/* UDF */
	int mac_type;/* MAC Type(SRC MAC:1, DST MAC:2, None:0) */
	uint64_t src_mac;
	uint64_t src_mac_mask;
	uint64_t dst_mac;
	uint64_t dst_mac_mask;

	int ip_type;/* IP Type(SRC IP:1, DST IP:2, None:0) */
	uint64_t src_ip;
	uint64_t src_ip_mask;
	uint64_t dst_ip;
	uint64_t dst_ip_mask;

	int srcport_type, dstport_type;
	uint64_t src_port;/* ip protocol must be TCP or UDP */
	uint64_t dst_port;/* ip protocol must be TCP or UDP */

	uint32_t eth_type;/* Ether Type(ARP:0x0806, MAC Control:0x8808) */

	int act_type;/* Action Type(Rate Control:1, Block:2, Change Priority:3, Statistic:4, None:0 ) */
	uint32_t rate;
	uint32_t prio;
}_BCM_ACL_Control_t;

/** BCM ACL Rule Only **/
#define BCM_ACL_TEST				_IOWR(MAJOR_NUM, 0x60, _BCM_ACL_Control_t)
#define BCM_ACL_RESET				_IOWR(MAJOR_NUM, 0x61, _BCM_ACL_Control_t)
#if defined(CONFIG_NK_SWITCH_ARCH)
#define NK_SWITCH_GET_LINK_STATUS   _IOWR(MAJOR_NUM, 0x71, nk_switch_port_status_t )
#define NK_SWITCH_GET_PORT_STATUS   _IOWR(MAJOR_NUM, 0x72, nk_switch_port_status_t )
#define NK_SWITCH_SET_PORT_STATUS   _IOWR(MAJOR_NUM, 0x73, nk_switch_port_status_t )
#endif

#define NK_SWITCH_SET_LED_WANLAN    _IOWR(MAJOR_NUM, 0x80, unsigned int )

#if defined(CONFIG_NK_SWITCH_ARCH)
#define NK_SWITCH_HW_MACCLONE       _IOWR(MAJOR_NUM, 0x81, nk_switch_hw_macclone_t )
#endif
/**
		When Ethernet Recieve Src IP==LAN IP, Show it -- incifer 2009/02
		Sync File: getlink.h, eth_acc.h
**/
#define ETH_ACC_MAJOR       238   /* This exists in the experimental range */
#define SHOW_RECIEVE_LAN 							_IOWR(ETH_ACC_MAJOR, 118, u_int32_t *)
#define SHOW_RECIEVE_LAN_DISABLE 		_IO(ETH_ACC_MAJOR, 119)
