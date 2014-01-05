
#define MAJOR_NUM		237
#define MINOR_NUM		1
#define VOICE_IC_CONTROL		_IOWR(MAJOR_NUM, 0x14, struct NK_VOICE)
#define CPLD_IC_CONTROL			_IOWR(MAJOR_NUM, 0x15, struct NK_CPLD)
#define VOICE_IC_CONTROL1		_IOWR(MAJOR_NUM, 0x17, struct voice_message)
#define HIGH_PRI	9
#define LOW_PRI		1
#define DATAPIN_HIGH	0
#define DATAPIN_LOW	1
#define INTERVAL	120

#define QNOBOOTUP	"006501"
#define WANDOWN		"000"
#define WANUP		"001"
#define WANCHANGEIP	"005"
#define WANDOS		"004"
#define LANDOS		"0061"
#define LOGIN		"0063"
#define LOGINFAIL	"0064"
#define SESSION_LIMIT	"70"
#define MAC_ADDRESS	"71"
#define IP_ADDRESS	"72"
#define NEW_HOST	"6E"
#define WRONG_IP	"6F"
#define WANUPJAM	"003"
#define WANDOWNJAM	"002"
#define ARPATT		"0062"
#define SHOCKWAVE	"0060"
#define LAN		"56"
#define WAN		"6B"
#define ATTACK		"55"
#define	DOS		"11"
#define LANCHANGEIP	"6D"
#define LANADDPC	"6C"
//#define


typedef struct alarm_msg
{
	struct alarm_msg *next;
	struct alarm_msg **pnext;
	char msg[200];
	int mtimer;
	int interval;
} alarm_msg_t;


typedef struct voice_setting
{
	int enable;
	int wan_down_check;
	int wan_down_time;
	int wan_up_check;
	int wan_up_time;
	int upstream_jam_check;
	int upstream_jam_time;
	int downstream_jam_check;
	int downstream_jam_time;
	int wan_dos_check;
	int wan_dos_time;
	int wan_dos_period;
	int lan_dos_check;
	int lan_dos_time;
	int lan_dos_period;
	int arp_check;
	int arp_time;
	int arp_period;
	int block_wrong_ip_check;
	int block_wrong_ip_time;
	int block_wrong_ip_period;
	int block_wrong_ip_add_mac;
	int block_wrong_ip_add_ip;
	int block_wrong_ip_mac;
	int block_wrong_ip_ip;
	int block_not_on_list_check;
	int block_not_on_list_time;
	int block_not_on_list_period;
	int session_limit_check;
	int session_limit_time;
	int session_limit_period;
	int shockwave_check;
	int shockwave_time;
	int shockwave_period;
} voice_setting_t;
