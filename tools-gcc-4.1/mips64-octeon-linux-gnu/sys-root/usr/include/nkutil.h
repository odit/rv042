/****************************************************************************
 *  Copyright (c) 2005 ODM Tech. Inc. All Rights Reserved.
 * 
 *  nkutil.c
 *
 *  Developed by ODM Tech. Inc. 
 *  R&D Software Dept.
 *
 *  This file is part of the ODM Software and may not be distributed,
 *  sold, reproduced or copied in any way.
 *
 *  This copyright notice should not be removed
 *
 */
#ifndef UTIL_H
#define UTIL_H

#include <sys/types.h>
#include "nkdef.h"
#include "sysconfig.h"
//===========richie
#include <syslog.h>
//===========

#define CMDBUF_SIZE    160
#define PPTP_FILE_SIZE    300
//#define ARGV_SIZE    384

#define DATABASE_PATH 		"/tmp/"
#define RESOLV_CONF			"/etc/resolv.conf"     // needed by udhcpd and system
#define DHCPD_CONF			"/etc/udhcpd.conf"     // needed by udhcpd and system
#define SYS_FILE			"/tmp/nk_sysconfig" // TT "/tmp/sysconfig"
#define BACKUP_SYS_FILE		"/tmp/nk_sysconfig1" // TT "/tmp/sysconfig"
#define DDNS_CONF			"/etc/ez-ipupdate.conf" // DDNS
#define DHCPS_STATIC_LEASE	"/tmp/dhcp_static.leases" // DHCP static lease

#define SCRIPT_PORT   0x5648
#define SCRIPT_C_PORT 0x5649

#define CMD_WRITE           1
#define CMD_DELETE          2
#define CMD_PRINT           3
#define CMD_NEW             4
#define CMD_BACKUP          5
#define CMD_TASK_DB         6
#define CMD_MODIFY          7
#define CMD_PRINT_NO_FLAG          8

// set for each html page
#define DATA_INDEX         1
#define DATA_FILTER        2
#define DATA_PFORWARD      3
#define DATA_DHCP          4
#define DATA_DMZ           5
#define DATA_PASSWD        6
#define DATA_HCLONE        7
#define DATA_SWITCH        8
#define DATA_SWITCHA       9
#define DATA_SWITCHI       10
#define DATA_MIBC          11

//RLQ, for UPNP and WLAN
#define DATA_UPNP		   12
#define DATA_WLAN		   13

// for ASH
#define ASH_DO_NOTHING     0
#define ASH_DO_SCAN        1
#define ASH_DO_FIRE        2
#define ASH_DO_DHCP        3
#define ASH_DO_UPNP	   4
#define ASH_DO_WLAN	   5
#define ASH_DO_RESTART_DHCP  6
#define ASH_DO_START_DHCP    7
#define ASH_DO_STOP_DHCP     8
#define ASH_DO_CHG_MAC_ADDR  9

// TT HTML PAGE
#define ASH_PAGE_SETUP		10
#define ASH_PAGE_PASSWORD	11
#define ASH_PAGE_DHCP		12
#define ASH_PAGE_FIREWALL	13
#define ASH_PAGE_DMZHOST	14
#define ASH_PAGE_FILTER		15
#define ASH_PAGE_MACCLONE	16
#define ASH_PAGE_FORWARD	17
#define ASH_PAGE_UPNP		18
#define ASH_PAGE_WLAN		19
#define ASH_PAGE_TIME		20
#define ASH_PAGE_DDNS		21
#define ASH_PAGE_SCHEDULER	22
#define ASH_PAGE_ADMINISTRATION	23
#define ASH_PAGE_QOS	        24
#define ASH_PAGE_URL	        25
#define ASH_PAGE_AP		        26
#define ASH_PAGE_BLOCK	        27
#define ASH_PAGE_DUALWAN        28
#define ASH_PAGE_ROUTING		29
#define ASH_PAGE_1TO1NAT		30
//#define ASH_PAGE_SNMP			31
#define ASH_PAGE_ACCESSRULE		32
#define ASH_PAGE_CONTENRFILTER	33
#define ASH_PAGE_G2GIPSEC		34
#define ASH_PAGE_PASSTHRU		35
#define ASH_PAGE_QVM			36
#define ASH_PAGE_LOG			37
#define ASH_PAGE_ROUTE		38	//add by trenchen for support static route
#define ASH_PAGE_EMAIL			39	//log_setting.htm email log by incifer 2006/02/23
#define ASH_PAGE_SNMP		40      //add by trenchen 2006/3/7
#define ASH_PAGE_PPTP			41
#define ASH_PAGE_VOICE		42


/* add by chihmou, support pptp trunking 2008/06/11 --> */
#ifdef CONFIG_NK_PPTP_TRUNKING
#define ASH_PPTP_TRUNKING_CONNECT	43
#define ASH_PPTP_TRUNKING_DISCONNECT	44
#define ASH_PPTP_TRUNKING_OPERATE_SET	45
#define ASH_PPTP_TRUNKING_OPERATE_DEL	46
#endif
/* <-- add by chihmou, support pptp trunking */

#define ASH_PAGE_IDS			47

#define ASH_PAGE_ANTI_SPAM		48

#define ASH_BUTTON_DHCP_RELEASE		60
#define ASH_BUTTON_DHCP_RENEW		61
#define ASH_BUTTON_PPPOE_DISCONNECT	62
#define ASH_BUTTON_PPPOE_CONNECT	63
#define ASH_BUTTON_REMOVE_USER		64
#define ASH_BUTTON_WRITE_LEASE		65
#define ASH_BUTTON_QVM_CONNECT		66
#define ASH_BUTTON_REMOVE_LOG		67
#define ASH_BUTTON_EMAIL_LOG		68

//2008/12/22:trenchen:support HA
#define ASH_DO_HA_INIT	94
#define ASH_DO_CDETECT2NORMAL	95
#define ASH_DO_CNORMAL2BACKUP	96
#define ASH_DO_CBACKUP2NORMAL	97
#define ASH_PAGE_HA	98

// TT special action need other tasks follow
#define ASH_DO_START_WAN_CONN	101
#define ASH_DO_WAN_CONN_DOWN	102
#define ASH_DO_WAN_CONN_UP		103
#define ASH_DO_INIT				104
#define ASH_DO_DHCP_NEW_HOST	105
#define ASH_DO_TIMER 			106
// add by johnli, handle other tasks for physical link up/down
#define ASH_PHYLINK_UP 			107
#define ASH_PHYLINK_DOWN		108
#define ASH_QOS_BUTTON_PRESSED	109 
#define ASH_DO_INIT_WAN_CONN	110
#define ASH_NSD_OK	 			111
#define ASH_NSD_FAIL			112
#define ASH_SLB_DEL_LINK		113
#define ASH_SLB_ADD_LINK		114
#define ASH_PPP_COD				115
#define ASH_PAGE_ACCESS_RULE	116
#define ASH_PAGE_QVM_UP			117
#define ASH_PAGE_QVM_DOWN		118
#define ASH_SYSTEM_REBOOT		119
//add by trenchen for write time to rtc
#define ASH_DO_RTC		120
//add by yami for content_filter and 121nat
#define ASH_PAGE_CONTENT_FILTER		121
#define ASH_PAGE_121NAT			122
#define ASH_PAGE_DYN_ROUTE		123 //for dynamic route
#define ASH_PAGE_VLAN			124
#define ASH_PAGE_VLAN_MEMBERSHIP	125
#define ASH_PAGE_SESSION_LIMIT		126
#define ASH_DEBUG_SESSION_LIMIT		127
#define ASH_PAGE_PPTP_RESTART		132
#define ASH_UPDATE_FIREWALL_SETTING	133
#define ASH_BUTTON_MAC_ADDR_FILITER		134
#define ASH_PAGE_ANTI_SPAM_STOP		135
#define ASH_PAGE_ANTI_SPAM_START		136
#define ASH_PAGE_ANTI_SPAM_SAVE_LEARN_SPAM_COUNTER	137
#define ASH_PAGE_ANTI_SPAM_CONFIG	138
#define ASH_PAGE_ANTI_SPAM_IMPORT_DB 139
#define ASH_PAGE_ANTI_SPAM_LEARN_SPAM 140
#define ASH_PAGE_ANTI_SPAM_LEARN_HAM 141
#define ASH_EXCEPTION_QQ_NUMBER		142
#define ASH_PAGE_ANTI_SPAM_SAVE_DB_TO_FLASH		143
#define ASH_PAGE_ANTI_SPAM_ERASE_DB_TO_FLASH		144
/* support line dropped */
#define ASH_ACTIVE_LINEDROP		145
#define ASH_INACTIVE_LINEDROP		146
#define ASH_SESSION_NUMBER		147

#define ASH_EXCEPTION_QQ_NUMBER		150

/* support Protect Link */
#define ASH_PAGE_TM			151
#define ASH_URLFILTER_LICENSECHECK	152

//Rain add for Anti-Virus
#define ASH_PAGE_ANTI_VIRUS		166
#define ASH_PAGE_ANTI_VIRUS_MANUAL_UPLOAD		167

#ifdef NK_CONFIG_IPV6
#define ASH_PAGE_RADVD			168
#define ASH_PAGE_6TO4			169
#define ASH_DO_DNSMASQ			170
#define ASH_PAGE_ACCESS_RULE_V6		171
#define ASH_DO_INIT_WAN_CONN_IPV6 172
#endif

// Charles : VPN Utility Part
#define ASH_VPN_ADD				200
#define ASH_VPN_DEL				201
#define ASH_VPN_CONNECTED			202
#define ASH_VPN_DISCONNECTED			203

// Daniel Cheng Quick VPN -->
#ifdef CONFIG_NK_IPSEC_RW_CONN
#define ASH_QUICK_VPN_INIT			204
#endif
// Daniel Cheng <--

// RVxx Certificate Management
#ifdef NK_CONFIG_EASYACCESS
#define ASH_RV_IMPORT_CERT			205
#define ASH_RV_IMPORT_CERT_2		206
#endif

// support diagnostic incifer 2006/06/26
#define ASH_BUTTON_NSLOOKUP		221
#define ASH_BUTTON_PING			222

#define ASH_PAGE_VIRTUAL_PASSAGE	231

#define ASH_PAGE_ADV_DOS		232

#define ASH_PAGE_MLAN			233

#define ASH_KMALLOC_DEBUG_START		250
#define ASH_KMALLOC_DEBUG_SHOW		251

#define ASH_PAGE_SSL_ADV		252
#define ASH_RESTART_EA_HTTPD		253

//session base enhancement by michael lu 2008/11
#define ASH_PAGE_SB_ENHANCEMENT		254

/* support IP balance enhancement -- incifer 2008/11 */
#define IPBALANCE_ENHANCE_CHECK_WAN		255

#define ASH_DIMM_MODULE_CHECK		256

#define ASH_PAGE_INBOUNDLB		257
#define ASH_IMPORT_CERT		258

#define ASH_BCL_ACL_UPDATE	259

#define ASH_BONJOUR	300

/*purpose     : 13292 author : jason.huang date : 2010-10-07*/
/*description : RV042 supports MAC clone on one WAN only. */
#define ASH_PAGE_HW_MACCLONE	310

#define ASH_PHYLINK_USB_UP	420		//	ODM USB Device physical link up event number //Robert 2009/10/17
#define ASH_PHYLINK_USB_DOWN	421		//	ODM USB Device physical link down event number //Robert 2009/10/17
#define ASH_3G_PPP_ON		422
#define ASH_3G_PPP_OFF		423
#define ASK_CHECK_3G_BACKUP 424
#define ASH_ACCESSRULE_DEFAULT 425

#define USE_UPTIME 1
#if 0
//richie add for syslog=================================================
# define NK_LOG_SYS(level, str, args...) 	do { 	openlog("System Log", 0, LOG_LOCAL0); \
						syslog(level, str, ## args); \
						openlog("System Log", LOG_PID, LOG_LOCAL0); } while(0)

# define NK_LOG_VPN(level, str, args...)	do { 	openlog("VPN Log", 0, LOG_LOCAL1); \
						syslog(level, str, ## args); \
						openlog("VPN Log:", LOG_PID, LOG_LOCAL0); } while(0)

# define NK_LOG_SSL(level, str, args...)	do { 	openlog("SSL Log", 0, LOG_LOCAL6); \
						syslog(level, str, ## args); \
						openlog("SSL Log", LOG_PID, LOG_LOCAL0); } while(0)

# define NK_LOG_FW(level, str, args...)		do { 	openlog("Firewall Log", 0, LOG_LOCAL2); \
						syslog(level, str, ## args); \
						openlog("Firewall Log", 0, LOG_LOCAL0); } while(0)

//=====================================================================
#endif

# define NK_LOG_SYS(level, str, args...) 	do { 	openlog("System Log", 0, LOG_LOCAL0); \
						syslog(level, str, ## args); } while(0)

# define NK_LOG_VPN(level, str, args...)	do { 	openlog("VPN Log", 0, LOG_LOCAL1); \
						syslog(level, str, ## args); } while(0)

# define NK_LOG_SSL(level, str, args...)	do { 	openlog("SSL Log", 0, LOG_LOCAL6); \
						syslog(level, str, ## args); } while(0)

# define NK_LOG_FW(level, str, args...)		do { 	openlog("Firewall Log", 0, LOG_LOCAL2); \
						syslog(level, str, ## args); } while(0)

# define NK_LOG_AS(level, str, args...)		do { 	openlog("Anti Spam Log", 0, LOG_LOCAL5); \
						syslog(level, str, ## args); } while(0)

# define NK_LOG_AV(level, str, args...)		do { 	openlog("Anti Virus Log", 0, LOG_LOCAL7); \
						syslog(level, str, ## args); } while(0)

#define LENGTH_READ_SIZE  768
#define LENGTH_READ_BUF   772
#define LENGTH_READ_PATH  800

typedef struct {
  int doASH;
  char cmd;
  char mesg[80];
  int  status;
} ScriptDo;

typedef struct mtd_info_1 {
	unsigned int size;	 		// Total size of the MTD
	unsigned int erasesize;		// erase size for each flash erase command
	char name[50];				// name of the MTD partition
	unsigned int index;
	unsigned int block_num;		// total block number in the MTD partition
	struct mtd_info_1 *next;
} mtd_info_1;

//#ifdef CONFIG_NK_NAT_MODE
typedef struct RouterModeInfo {
	char rgw[20];
	char rip1[40];
	char rip2[40];
} RouterModeInfo;
//#endif

typedef struct nkWanInfo {
// --> wan settings
	int wantype;
	char wanip[20];
	char wanmask[20];
	char wammac[6];
	char waninterface[10];
	int automtu;
	int mtu;
	char gateway[20];
	int userspecialdns;
	char dns[3][20];
	char macclone[6];
	int wanconnection;
	int prev_wanconnection;
	char dmzhost[20];
	int upstreambw;
	int downstreambw;
// <--
// --> wan status 
	char isp_wanip[20];
	char isp_wanmask[20];
// --> wan status 
	char isp_wanip6[40];
	char prefix[8];
	
//	int isp_mtu;
	char isp_gateway[20];
	char isp_dns[3][20];
	char isp_interface[10];
	int nsdok;
	int linkup;
	int state;
//	add database & data structure for DMZ port 
// <--
// --> pppoe
	char pppoe_username[128];
	char pppoe_password[128];
	int pppoe_connectondem;
	int pppoe_keepalive;
	int pppoe_maxidle;
	int pppoe_redialperiod;
	int pppoe_echoperiod;   //2006/7/21 trenchen : pppoe echo time can be configured
	int pppoe_echotimes;    
// <-
	int pptp_connectondem;  //2005/10/18 trenchen : support pptp
	int wanchanged;		// wan setting changed
	int ispchanged;		// wan ip changed
	int nsdchanged;
	int linkchanged;	// phylink, nsd, the behavior will affect wan status

#ifdef CONFIG_NK_TRANSPARENT_BRIDGE
	#ifdef CONFIG_NK_TRANSPARENT_BRIDGE_RANGE_NUM
		char internal_lan_ip_start[CONFIG_NK_TRANSPARENT_BRIDGE_RANGE_NUM][20];
		char internal_lan_ip_end[CONFIG_NK_TRANSPARENT_BRIDGE_RANGE_NUM][20];
	#else
	char internal_lan_ip1_start[20];
	char internal_lan_ip1_end[20];
	char internal_lan_ip2_start[20];
	char internal_lan_ip2_end[20];
	#endif
#endif
/* DMZ range incifer 2007/11/01 */
	int dmztype;
	char range_start[40];
	char range_end[40];
	int interface;
//-->

//#ifdef CONFIG_NK_NAT_MODE
	struct RouterModeInfo routerinfo[3];
//#endif
} nkWanInfo;

typedef struct QvmInfo {
	int enabled;
	char account[36];
	char password[256];
	char remote_server[4][40];
	int autotry_enable;
	int autotry_timer;
	int qvm_ipsec_dev;
	int backup;
} QvmInfo;

/* purpose : IPV6  author : Yami date : 2010-10-22 */
/* description : Add lanip6 */
typedef struct nkSysInfo {
	char hostname[65];
	char domainname[65];
	char lanip[20];
#ifdef NK_CONFIG_IPV6
	char lanip6[40];
	int v6lanprefix;
	int ip_type;
#endif
	char lanmask[20];
	char lanmac[6];
	char laninterface[10];
	int  lanchanged;
	int  multiwanmode;
	char slbprimary[10];
#ifdef CONFIG_NK_DYNAMIC_PORT_NUM
	struct nkWanInfo wan[8];
#else
	struct nkWanInfo wan[CONFIG_NK_NUM_WAN+CONFIG_NK_NUM_DMZ+CONFIG_NK_NUM_USB];
#endif
	struct QvmInfo qvm;
}nkSysInfo;

/* Simple version of _eval() (no timeout and wait for child termination) */
#define evals(cmd, args...) ({ \
	char *argv[] = { cmd, ## args, NULL }; \
	eval(argv, ">/dev/console", 0, NULL); \
})

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
int kd_doCommand(char * , int , int , char *);
#ifdef __cplusplus
}
#endif // __cplusplus
void kd_Log(char *format, ...);
void kd_getMACs(int skfd, char * mac0, char * mac1, int doSetstring);
int kd_setMACs(char * mac, char * ethr);
int kd_setMTUs(int mtu, char * ethr);
int kd_hexString2Int(char * buf);
int kd_updateFlash(int flag);
int kd_readFlash(char * buf);
void kd_eraseFlash(void);
struct mtd_info_1 * kd_GetMtdImageInfo(char *);
unsigned int getmtderasesize(char *);
int getmtdbyblocknum(char * , unsigned int , struct mtd_info_1 ** , unsigned int *);
int kd_EraseFlashOneBlock(unsigned int );
int kd_BurnFlashOneBlock(unsigned int , unsigned char *, unsigned int );
int kd_EraseFlashSector(char * , unsigned int );
int kd_BurnFlash(char * , unsigned int , unsigned char *, unsigned int  , unsigned int );
int kd_ReadFlashSector(char * , unsigned int , unsigned int , unsigned char *);
int eval(char *const argv[], int timeout, int *ppid);
time_t get_time(time_t *t);

// kd_sys.c
//int nk_sysconfig(int argc, char argv[3][ARGV_SIZE], char *supBuf);
// interface.c
int ifconfig(char *name, int flags, char *addr, char *netmask);
int route_add(char *name, int metric, char *dst, char *gateway, char *genmask);
int route_del(char *name, int metric, char *dst, char *gateway, char *genmask);
// add by johnli, support LED on/off function
int nk_led_on(unsigned int uLedNo);
int nk_led_off(unsigned int uLedNo);
struct interface *nk_if_statistic_get(char *target);
// end johnli

// network.c
void init_mtu(int i);
void set_mtu(int i);
void goinit(void);
void gonetwork(void);
void gostatic(int i);
void godmzport(int i);
void killwanconn(int i);
void chgmacaddr(int i);
// webFlash.c
int webFlash(void);
// config.c
void nk_syncconfig(int flag, char *mesg);
int nk_compconfig(int flag, char *mesg);

//2008/12/22 trenchen:support HA
int send_arp( char *inf, int dip);

#endif


