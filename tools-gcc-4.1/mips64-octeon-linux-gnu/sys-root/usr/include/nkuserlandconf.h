/*
 *
 *
 * --> come from model-userlandconf.h
 */
#define USERLANDCONF_INCLUDED


/*
 * feature setup
*/
#define CONFIG_SIGNATURE_CHECK 1
#define CONFIG_NK_NUM_LAN 8
#define CONFIG_NK_MAX_NUM_WAN 8
/* purpose     : 0013121    author : David    date : 2010-08-04    	       */
/* description : Change CONFIG_NK_NUM_WAN from 4 to 7 to fix WAN 6~7 issue     */
#define CONFIG_NK_NUM_WAN 7
#define CONFIG_NK_NUM_USB 0
#define CONFIG_NK_NUM_DMZ 1
#define CONFIG_NK_NUM_DEF_WAN 4
#define CONFIG_NK_NUM_VPN 5

#define CONFIG_MODEL_QVM2100 1
#define CONFIG_MODEL_QVM2000 1
#define CONFIG_MODEL_RV0XX 1
#define CONFIG_NK_IPSEC
#define CONFIG_NK_IPSEC_RW_CONN 1
#if 0
/* for SSL function*/
#define CONFIG_NK_SSL
/*SSL upgrade*/
#define CONFIG_NK_SSL_UPGRADE
#define CONFIG_NK_USBKEY_SERVER 1
#define CONFIG_NK_QVM_SERVER 1
#endif
/*
* nk define
*/

#define CONFIG_NK_PADT_ALWAYS
#define CONFIG_NK_PADI_RETRY 
#define CONFIG_NK_PROTO_BINDING
#define CONFIG_NK_NSD
#define CONFIG_NK_IP_BASE
#define CONFIG_NK_RIP
#define CONFIG_NK_STATIC_ROUTE
#define CONFIG_NK_ROUTE_CACHE_MODIFY

/*# Ryoko : support CPLD*/
#define CONFIG_NK_CPLD 1

/*# Ryoko : support PPTP Server*/
#define CONFIG_SUPPORT_PPTPD 1
#define CONFIG_SUPPORT_PPTPD_READ_CONfIGFILE_BY_SIGNAL 1

/* ip & mac binding*/
#define CONFIG_SUPPORT_IP_MAC_BINDING 1

/*prevent arp attack*/
#define CONFIG_SUPPORT_ARP_SPOOF_PROTECT 1

/*session limit*/
/*#define CONFIG_NK_SESSION_LIMIT 1
#define CONFIG_NK_SESSION_LIMIT_ENHANCE 1*/

/* traffic statistics */
#define CONFIG_NK_IPFILTER_SUPPORT_SORTING 1
#define CONFIG_NK_PHYLINK_CHECK 1

/* transparent bridge */
#define CONFIG_NK_TRANSPARENT_BRIDGE
#define CONFIG_NK_TRANSPARENT_BRIDGE_RANGE_NUM 2

/* dos enhancement */
/*#define CONFIG_NK_DOS_ENHANCEMENT*/

/* multiple subnet */
#define CONFIG_NK_MLAN

/* Qno ddns */
/*#define CONFIG_NK_DDNS_QDNS 1*/

/* support WAN interface MAC address filter */
#define CONFIG_NK_BROADCAST_PREVENTION 1

#define CONFIG_NK_CHANGE_SRC_PORT 1

/* content filter*/
#define CONFIG_NK_CONTENT_FILTER 1

/* When disable remote management, let SSL portal UI and Qnokey page come in */
#define CONFIG_NK_SSL_PASS_HTML 1

/* IPSec Multiple Passthrough */
#define CONFIG_NK_IPSEC_MULTIPLE_PASS_THROUGH 1

/* restrict applications : msn, bt, QQ, skype */
#define CONFIG_NK_RESTRICT_APP 1

#define CONFIG_NK_SNMP 1
/*trenchen : support HA*/
/*#define CONFIG_NK_HA 1*/

#define CONFIG_NK_NAT_FIREWALL_RULE_BY_FILE 1

/* Share Architecture, Dynamic LAN/WAN Numbers, First for 1450 -- incifer 2009/04 */
#define CONFIG_NK_DYNAMIC_PORT_NUM 1
/* Some varibales are pre declared, you can use it to re declare it -- incifer 2009/06 */
#define CONFIG_NK_NUM_MAX_LAN 8
#define CONFIG_NK_NUM_MAX_WAN 8

#define CONFIG_NK_NAT_MODE 1

/*support Line Drop*/
/*#define CONFIG_NK_LINE_DROP 1*/

/* support IP balance enhancement -- incifer 2008/11 */
/*#define CONFIG_IPBALANCE_ENHANCE 1*/

/*#define CONFIG_NK_SB_ENHANCEMENT 1*/

#define CONFIG_NK_IMPORT_EXPORT_ADV 1

#define CONFIG_NK_SUPPORT_CN5010 1

/* support CN5020 dual-core. Hope it supports CN5230 multi-core eventually too */
#define CONFIG_NK_SUPPORT_MULTI_CORE 1
/*--> Rain 20091022, solve IPSec crash over dual wan with multi-core CPU */
#define CONFIG_NK_SOLVE_IPSEC_DUALWAN_MULTI_CORE 1
/* support CN5020 dual-core. SOLVE_MULTI_CORE_MULTI_WAN_CRASH */
#define CONFIG_NK_SOLVE_MULTI_CORE_MULTI_WAN_CRASH 1

/* QRTG log_cpu_info.htm log_wan_info.htm*/
/*#define CONFIG_NK_QRTG_SYS_INF 1*/

/*support Multiple MTU add by jason.huang 2010/01/18*/
#define CONFIG_NK_SUPPORT_MULTIPLE_MTU 1

/* selena : support HTTPS Control */
#define CONFIG_NK_HTTPS_CONTROL 1

/* selena : if support EesyAccess, originally defined in *.config */
#define NK_CONFIG_EASYACCESS 1

#define CONFIG_NK_DB_CHECKSUM 1

/* Paul : support port trigger */
#define CONFIG_NK_PORT_TRIGGER 1

/* Paul : support local DNS database */
#define CONFIG_NK_LOCAL_DNS_DATABASE 1

/* Paul : support DHCP Relay */
#define CONFIG_NK_DHCP_RELAY 1

/* selena : support HTTPS Control */
#define CONFIG_NK_HTTPS_CONTROL 1

/* selena : if support EesyAccess, originally defined in *.config */
#define NK_CONFIG_EASYACCESS 1

/**
 * New Switch Arch -- incifer 2010/01
 **/
#define CONFIG_NK_SWITCH_ARCH   1

#define CONFIG_NK_IPSEC_SPLITDNS 1

#define CONFIG_NK_IPSEC_NETBIOS_BC 1

#define CONFIG_NK_CRAMFS 1

#define CONFIG_NK_URL_TMUFE_FILTER 1

#define CONFIG_NK_NSD_NETROUTE_CHANGE 1

/*20100428 trenchen : support pptp server using mppe*/
#define CONFIG_NK_PPTPSERVER_MPPE 1

#define NK_CONFIG_IPV6 1

#define CONFIG_NK_IPV6_ADDON 1
