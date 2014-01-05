#ifndef __SWITCH_H__
#define __SWITCH_H__


#if defined(CONFIG_NK_SWITCH_ARCH)

#include "nk_switch_common.h"

enum NK_SWITCH_CMD {
    NK_SWITCH_CMD_READ,
    NK_SWITCH_CMD_WRITE,
    NK_SWITCH_CMD_PREAD,
    NK_SWITCH_CMD_PWRITE,
    NK_SWITCH_CMD_GPIO_SET_STATE,
    NK_SWITCH_CMD_GPIO_SET_VALUE,
    NK_SWITCH_CMD_GPIO_GET_VALUE,
    NK_SWITCH_CMD_GPIO_GET_STATE,
    NK_SWITCH_CMD_GPIO_SHOW,
    NK_SWITCH_CMD_PORT_GET_PORT_STATUS,
    NK_SWITCH_CMD_PORT_SET_PORT_STATUS,
    NK_SWITCH_CMD_LED,
    NK_SWITCH_CMD_PRINT_VTABLE,
    NK_SWITCH_CMD_PRINT_PQOS,
};

typedef struct {
    uint32_t dir;
    uint32_t page;
    uint32_t addr;
    uint64_t value;
    uint32_t reserve;
} NK_Switch_RW_t;

typedef struct {
    uint32_t pin;
    uint32_t value;
} NK_Switch_GPIO_RW_t;

typedef struct {
    uint32_t type;
    uint32_t state;
} NK_Switch_LED_RW_t;

typedef struct {
    int cmd;
    NK_Switch_RW_t rw;
    NK_Switch_GPIO_RW_t gpio;
    NK_Switch_LED_RW_t led;
    nk_switch_port_status_t pstatus;
} NK_Switch_Util_t;

#endif

/**
 *	port status structure
 */
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
struct NK_VOICE{
	unsigned int type;
	unsigned char len[200];
} ;
struct NK_CPLD{
	unsigned char len[200];
	unsigned int return_number[200];
	int length;
} ;
typedef	struct voice_message {
    char message[200];
    int pri;	//priority, high=9, low=1
    int interval;
} voice_message_t;
/**
 *	Constent define
 *	@param: SDEBUG: Advance debug message
 */
#define SDEBUG		1
#if SDEBUG
	#define printd(arg...)	printk(KERN_EMERG arg)
#endif

#define MAJOR_NUM		237
#define MINOR_NUM		1

typedef struct phy_control
{
	int flag; /* read/write flag, read: 1, write: 0 */
	int is_lan; /* LAN: 1, WAN: 0 */
	int port; /* 0~8, or all port: 99 */
	unsigned char addr; /* register address/offset(0~3E) */
	unsigned short data;
}phy_control_t;

typedef struct {
	uint32_t dir;
	uint32_t phyad;
	uint32_t regad;
	uint32_t npage;
	uint64_t value;
}RTL8306_Control_t;

typedef struct {
	uint32_t gpio;
	uint32_t state;
	uint32_t value;
}GPIO_Control_t;

typedef struct {
	uint32_t dir;
	uint32_t reg;
	uint32_t addr;
	uint32_t reserve;
	uint64_t value;
}Switch_Control_t;

/**
	ARL Table Data Index Structure -- Incifer 2009/02
	Sync File: getlink.h, switch.h, sw_template_sw_control.h
**/
struct ARL_Data{
	uint64_t mac_index;
	uint32_t vid_index;
	uint32_t data_value;
	int is_lan;
};

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

/**
 *	ioctl define
 */
#define SWITCH_IOCTEST			_IOWR(MAJOR_NUM, 1, int)
#define SWITCH_DRIVERTEST		_IOWR(MAJOR_NUM, 2, int)
#define SWITCH_MDIO_READ_TEST		_IOWR(MAJOR_NUM, 3, uint64_t)
#define SWITCH_MDIO_WRITE_TEST		_IOWR(MAJOR_NUM, 4, uint64_t)
#define SWITCH_TO_LAN			_IO(MAJOR_NUM, 5)
#define SWITCH_TO_WAN			_IO(MAJOR_NUM, 6)
#define SWITCH_GPIO_TEST		_IOWR(MAJOR_NUM, 7, int)
#define SWITCH_SET_VLAN_GROUP		_IOWR(MAJOR_NUM, 8, vlan_table_entry_t)
#define SWITCH_GET_VLAN_GROUP		_IOWR(MAJOR_NUM, 9, uint16_t)

#define SWITCH_SET_LAN_PORT_STATUS	_IOWR(MAJOR_NUM, 0x10, struct PortStatusNStatistics)
#define SWITCH_GET_LAN_PORT_STATUS	_IOWR(MAJOR_NUM, 0x11, struct PortStatusNStatistics)
#define SWITCH_SET_WAN_PORT_STATUS	_IOWR(MAJOR_NUM, 0x12, struct PortStatusNStatistics)
#define SWITCH_GET_WAN_PORT_STATUS	_IOWR(MAJOR_NUM, 0x13, struct PortStatusNStatistics)

#define VOICE_IC_CONTROL		_IOWR(MAJOR_NUM, 0x14, struct NK_VOICE)
#define VOICE_IC_CONTROL1		_IOWR(MAJOR_NUM, 0x17, struct voice_message)
#define VOICE_SETTING		_IOWR(MAJOR_NUM, 0x16, struct voice_setting)
#define CPLD_IC_CONTROL			_IOWR(MAJOR_NUM, 0x15, struct NK_CPLD)

#define SWITCH_SET_LAN_VLAN		_IOWR(MAJOR_NUM, 0x20, struct PortStatusNStatistics)

#define SWITCH_SET_MIRROR_PORT		_IOWR(MAJOR_NUM, 0x21, int)

#define SWITCH_SET_WAN_CONN_LED		_IOWR(MAJOR_NUM, 0x22, unsigned int)
#define SWITCH_SET_LED_ON		_IOWR(MAJOR_NUM, 0x23, unsigned int)
#define SWITCH_SET_LED_OFF		_IOWR(MAJOR_NUM, 0x24, unsigned int)

#define MEMORY_LEAK_START 		_IOWR(MAJOR_NUM, 0x26, unsigned int)
#define MEMORY_DEBUG_SHOW 		_IOWR(MAJOR_NUM, 0x27, unsigned int)

#define LAN_MII_CONTROL			_IOR(MAJOR_NUM, 0X28, int)
#define WAN_MII_CONTROL			_IOR(MAJOR_NUM, 0X29, int)

#define SWITCH_PHY_CONTROL		_IOWR(MAJOR_NUM, 0x30, struct phy_control)

#define SWITCH_SET_DIMM_DIAG_LED_BLINK	_IOWR(MAJOR_NUM, 0x31, unsigned int)
#define SWITCH_SET_DIMM_DIAG_LED_OFF	_IOWR(MAJOR_NUM, 0x32, unsigned int)
#define SWITCH_SET_DIMM_DIAG_LED_ON	_IOWR(MAJOR_NUM, 0x33, unsigned int)
#define SWITCH_SET_DIMM_DIP_CHECK	_IOWR(MAJOR_NUM, 0x34, unsigned int)
#define ROUTE_TRANS_DST_TABLE		_IOWR(MAJOR_NUM, 0x35, struct SessionDstHead)
#define ROUTE_TRANS_LOCK_TABLE		_IOWR(MAJOR_NUM, 0x41, struct SessionLockHead)

/**
		Set LAN MAC to Static MAC -- incifer 2009/02
		Sync File: getlink.h, switch.h
**/
#define SWITCH_READ_ARL_TEST		_IOWR(MAJOR_NUM, 0x36, struct ARL_Data)
#define SWITCH_WRITE_ARL_TEST		_IOWR(MAJOR_NUM, 0x37, struct ARL_Data)
#define SWITCH_READ_ARL				_IOWR(MAJOR_NUM, 0x38, struct ARL_Data)
#define SWITCH_WRITE_ARL			_IOWR(MAJOR_NUM, 0x39, struct ARL_Data)

#define SIOC_GET_SWITCH_TYPE		_IOWR(MAJOR_NUM, 0x40, int)

/** Command Test Only **/
#define RTL8306_READ_TEST			_IOWR(MAJOR_NUM, 0x50, RTL8306_Control_t)
#define RTL8306_WRITE_TEST			_IOWR(MAJOR_NUM, 0x51, RTL8306_Control_t)
#define GPIO_STATUS					_IOWR(MAJOR_NUM, 0x52, GPIO_Control_t)
#define GPIO_VALUE_CONTROL			_IOWR(MAJOR_NUM, 0x53, GPIO_Control_t)
#define GPIO_STATE_CONTROL			_IOWR(MAJOR_NUM, 0x54, GPIO_Control_t)
#define SWITCH_READ_REG_TEST		_IOWR(MAJOR_NUM, 0x55, Switch_Control_t)
#define SWITCH_READ_TEST			_IOWR(MAJOR_NUM, 0x56, Switch_Control_t)
#define SWITCH_WRITE_TEST			_IOWR(MAJOR_NUM, 0x57, Switch_Control_t)

/** BCM ACL Rule Only **/
#define BCM_ACL_TEST				_IOWR(MAJOR_NUM, 0x60, _BCM_ACL_Control_t)
#define BCM_ACL_RESET				_IOWR(MAJOR_NUM, 0x61, _BCM_ACL_Control_t)

#if defined(CONFIG_NK_SWITCH_ARCH)
#define NK_SWITCH_IOCTL_UTIL        _IOWR(MAJOR_NUM, 0x70, NK_Switch_Util_t )
#define NK_SWITCH_GET_LINK_STATUS   _IOWR(MAJOR_NUM, 0x71, nk_switch_port_status_t )
#define NK_SWITCH_GET_PORT_STATUS   _IOWR(MAJOR_NUM, 0x72, nk_switch_port_status_t )
#define NK_SWITCH_SET_PORT_STATUS   _IOWR(MAJOR_NUM, 0x73, nk_switch_port_status_t )
#endif

#define NK_SWITCH_SET_LED_WANLAN    _IOWR(MAJOR_NUM, 0x80, unsigned int )

#if defined(CONFIG_NK_SWITCH_ARCH)
#define NK_SWITCH_HW_MACCLONE       _IOWR(MAJOR_NUM, 0x81, nk_switch_hw_macclone_t )
#endif


#define LAN_MII 					0x0
#define WAN_MII 					0x1

/**
 *	port status flag
 */
#define _1000Mbs					0x2
#define _100Mbs						0x1
#define _10Mbs						0x0
#define _FULL						0x1
#define _HALF						0x0
#define _ENABLE						0x1
#define _DISABLE					0x0
#define _UP							0x1
#define _DOWN						0x0

#endif