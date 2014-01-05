/****************************************************************************
 *  Copyright (c) 2005 Netklass Tech. Inc. All Rights Reserved.
 * 
 *  nkdef.h
 *
 *  Developed by Netklass Tech. Inc. 
 *  R&D Software Dept.
 *
 *  This file is part of the Netklass Software and may not be distributed,
 *  sold, reproduced or copied in any way.
 *
 *  This copyright notice should not be removed
 *
 */
/*
 *
 * nkdef.h
 *
 * definition
 *
 *
 */
#ifndef NKDEF_H
#define NKDEF_H

// for model definition
#include "nkuserlandconf.h"

#define NK_NO					0
#define NK_YES					1

#define NK_FAIL					-1
#define NK_OK					0

#define NK_DISABLE				0
#define NK_ENABLE				1

//STATE
#define NK_DOWN					0
#define NK_UP					1

//WANTYPE
#define NK_WAN_TYPE_WAN			0
#define NK_WAN_TYPE_DMZ			1
#define NK_WAN_TYPE_LAN			2

//DMZTYPE
#define NK_DMZ_TYPE_SUBNET		0
#define NK_DMZ_TYPE_RANGE		1

//DMZ RAGNE INFO
#define NK_DMZ_RANGE_IP			"1.1.1.1"
#define NK_DMZ_RANGE_MASK		"255.255.255.0"
#define NK_DMZ_RANGE_GW			"0.0.0.0"

//MULTIWANMODE
#define NK_MULTIWAN_TYPE_SLB	0
#define NK_MULTIWAN_TYPE_LB		1
#define NK_MULTIWAN_TYPE_IPGROUP	2
#define NK_MULTIWAN_TYPE_STROUTE	3

//WANCONNECTION
#define NK_WAN_CONN_DHCP		0
#define NK_WAN_CONN_STATIC		1
#define NK_WAN_CONN_PPPOE		2
#define NK_WAN_CONN_PPTP		3
#define NK_WAN_CONN_BRIDGE		8
#define NK_WAN_CONN_ROUTER	16

//WANCONNECTION
#define NK_WAN_CONN_DHCP_C		"0"
#define NK_WAN_CONN_STATIC_C	"1"
#define NK_WAN_CONN_PPPOE_C		"2"
#define NK_WAN_CONN_PPTP_C	"3"
#define NK_WAN_CONN_BRIDGE_C	"8"
#define NK_WAN_CONN_ROUTER_C	16

//MTU
#define NK_MAX_MTU				1500
#define NK_MIN_MTU				64
#define NK_MAX_MTU_PPPOE		1492
#define NK_MAX_MTU_PPTP		1460

//QoS
#define NK_USER_MAX_RATE		512
#define NK_USER_MIN_RATE		0
#define NK_WAN_UPSTREAM			512
#define NK_WAN_DOWNSTREAM		512


// IPSec
//#define NK_IPSEC_COD
//#define NK_IPSEC_SUPPORT_MANUAL

//INTERFACE
#define NK_LAN_IF				"eth0"
#define NK_WAN1_IF				"eth1"
#define NK_WAN2_IF				"eth2"
#define NK_WAN3_IF				"eth3"
#define NK_WAN4_IF				"eth4"
#define NK_WAN5_IF				"eth5"

//METRIC
#define NK_DEFAULT_METRIC		41
#define NK_EQUALIZER_METRIC		15

//interface ID for link status ... between drvier & UI
#define NK_LAN_ID				0
#define NK_LAN1_ID				1
#define NK_LAN2_ID				2
#define NK_LAN3_ID				3
#define NK_LAN4_ID				4
#define NK_WAN1_ID				21
#define NK_WAN2_ID				22

#if defined(CONFIG_MODEL_GQF1100) || defined(CONFIG_MODEL_QVM2000)|| defined(CONFIG_MODEL_QTM3000)
#define WAN_LINK_STATUS					23
#define LAN_PORT1_PORT3_LINK_STATUS		23
#define LAN_PORT2_PORT4_LINK_STATUS		7
#else
#define WAN_LINK_STATUS					23
#define LAN_PORT1_PORT3_LINK_STATUS		23
#define LAN_PORT2_PORT4_LINK_STATUS		7
#endif

//system description
#if defined CONFIG_MODEL_QR50
	#define SYSTEM_NAME				"QR50"
	#define SYSTEM_DESCRIPTION		"QoS Router"
	#define SYSTEM_CONTACT			"support@netklass.com"

#elif defined CONFIG_MODEL_GQF1100
#define SYSTEM_NAME			"GQF1100"
#define SYSTEM_DESCRIPTION		"Broadband Router"
#define SYSTEM_CONTACT			"support@qno.com"
#define SYSTEM_CPU_TYPE			"Cavium CN3110"
#define SYSTEM_RAM_SIZE			"512MB"
#define SYSTEM_FLASH_SIZE		"32MB"

#elif defined CONFIG_MODEL_QVM2100
#define SYSTEM_NAME			"SSL"
#define SYSTEM_DESCRIPTION		"SSL VPN Router"
#define SYSTEM_CONTACT			"support@qno.com"
#define SYSTEM_CPU_TYPE			"Cavium CN3110"
#define SYSTEM_RAM_SIZE			"1024MB"
#define SYSTEM_FLASH_SIZE		"32MB"

#elif defined CONFIG_MODEL_RV0XX
#define SYSTEM_NAME			"cisco"
#define SYSTEM_DESCRIPTION		"Broadband Router"
#define SYSTEM_CONTACT			"support@cisco.com"
#define SYSTEM_CPU_TYPE			"Cavium CN5020"
#define SYSTEM_RAM_SIZE			"256MB"
#define SYSTEM_FLASH_SIZE		"32MB"

#elif defined CONFIG_MODEL_QVM2000
#define SYSTEM_NAME			"SSL"
#define SYSTEM_DESCRIPTION		"SSL VPN Router"
#define SYSTEM_CONTACT			"support@qno.com"
#define SYSTEM_CPU_TYPE			"Cavium CN3110"
#define SYSTEM_RAM_SIZE			"512MB"
#define SYSTEM_FLASH_SIZE		"32MB"

#elif defined CONFIG_MODEL_QTM3000
#define SYSTEM_NAME			"UTM"
#define SYSTEM_DESCRIPTION		"UTM Router"
#define SYSTEM_CONTACT			"support@qno.com"
#define SYSTEM_CPU_TYPE			"Cavium CN3110"
#define SYSTEM_RAM_SIZE			"2048MB"
#define SYSTEM_FLASH_SIZE		"64MB"
#endif

// H/W GPIO/LED
#define LED_ON					1
#define LED_OFF					2
#if defined CONFIG_MODEL_QR50
	#define GPIO_PIN_1			1
	#define GPIO_PIN_5			5
	#define NK_QLED				1
	#define NK_DIAG_LED			5
	#define NK_WAN_CONN_LED		8
	#define NK_DMZ_CONN_LED		9
	#define NK_DIAGLED			NK_DIAG_LED
	#define SOFTWARE_RESET		1 << GPIO_PIN_1
	#define DIAG_LED			1 << GPIO_PIN_5
#elif defined(CONFIG_MODEL_GQF1100) || defined(CONFIG_MODEL_QVM2000)|| defined(CONFIG_MODEL_QTM3000)
	#define NK_RESET_BUTTON		6
	#define NK_DIAG_LED			7
	#define NK_DMZ_CONN_LED		8
	#define NK_WAN_CONN_LED		9
	#define NK_DIAGLED			NK_DIAG_LED
	#define SOFTWARE_RESET		1 << NK_RESET_BUTTON
	#define DIAG_LED			1 << NK_DIAG_LED
#endif

	#define DB_VERSION 				34

// image version
#if defined CONFIG_MODEL_QR50
	#define FIRMWARE_VERSION 			"v1.0.0"
	#define FIRMWARE_RC_VERSION 			"RC4"
	#define BOOT_VERSION				"v1.0.0"
	#define HARDWARE_VERSION 			"v1.0"
	#define AREA_CODE 				"nb"

	#define PRODUCT_NAME				"QR50"
	#define FIRMWARE_PATTERN			"QR50"
	#define BOOT_PATTERN				"BT50"
	#define MAC_PATTERN				"-MAC-!"

#elif defined CONFIG_MODEL_RV0XX
	#define FIRMWARE_VERSION            		"v4.0.2"
	#define FIRMWARE_RC_VERSION        		".08-tm"
	#define BOOT_VERSION                		"v1.0.0"
	#define HARDWARE_VERSION            		"V03"
	#define PRODUCT_NAME                		"RV0XX"
	#define VENDOR_CODE                 		"ls"
	#define PLATFORM_CODE               		"RV0XX"

#elif defined CONFIG_MODEL_GQF1100
	#define FIRMWARE_VERSION 			"v1.2.01"
	#define FIRMWARE_RC_VERSION 			".26S"
	#define BOOT_VERSION				"v1.0.0"
	#define HARDWARE_VERSION 			"v1.0"
	#define PRODUCT_NAME				"GQF1100"
	#define VENDOR_CODE					"ls"
	#define PLATFORM_CODE				"GQF1100"
	
#elif defined CONFIG_MODEL_GVF363g
	#define FIRMWARE_VERSION 			"v1.0.9"
	#define FIRMWARE_RC_VERSION 			".03"
	#define BOOT_VERSION				"v1.0.0"
	#define HARDWARE_VERSION 			"v1.0"
	#define PRODUCT_NAME				"GVF363g"
	#define VENDOR_CODE				"ls"
	#define PLATFORM_CODE				"GVF363g"

#elif defined CONFIG_MODEL_QVM2100
	#define FIRMWARE_VERSION 			"v1.0.8"
	#define FIRMWARE_RC_VERSION 			".08"
	#define BOOT_VERSION				"v1.0.0"
	#define HARDWARE_VERSION 			"v1.0"
	#define PRODUCT_NAME				"QVM2100"
	#define VENDOR_CODE				"ls"
	#define PLATFORM_CODE				"QVM2100"

#elif defined CONFIG_MODEL_QVM2000
	#define FIRMWARE_VERSION 			"v1.0.7"
	#define FIRMWARE_RC_VERSION 			".01"
	#define BOOT_VERSION				"v1.0.0"
	#define HARDWARE_VERSION 			"v1.0"
	#define PRODUCT_NAME				"QVM2000"
	#define VENDOR_CODE				"ls"
	#define PLATFORM_CODE				"QVM2000"

#elif defined CONFIG_MODEL_QTM3000
	#define FIRMWARE_VERSION 			"v1.0.0"
	#define FIRMWARE_RC_VERSION 			".03"
	#define BOOT_VERSION				"v1.0.0"
	#define HARDWARE_VERSION 			"v1.0"
	#define PRODUCT_NAME				"QTM3000"
	#define VENDOR_CODE				"ls"
	#define PLATFORM_CODE				"QTM3000"

#endif

#define FLASH0_SIZE					0x800000
#define FLASH1_SIZE					0x800000
#define FLASH0_START_ADDRESS		0xBF800000
#define FLASH1_START_ADDRESS		0xBF000000
	
	// --> 2005/11/10 Ryan : if the flash sector size is change , please update this value
#define FLASH_BLOCK_SIZE			0x20000
#define NUM_BLOCK_SIZE				1
#define PROTECTED_AREA_SIZE			(FLASH_BLOCK_SIZE * NUM_BLOCK_SIZE)
	// <--

#if defined CONFIG_MODEL_RV0XX

#define USER_CHANGE_DB 1   //oolong 03/09
#define SYSTEM_CHANGE_DB 2 //oolong 03/09

	#if defined CONFIG_NK_CRAMFS
		#define nk_LOADER_MTD		"/dev/mtd0"
		#define nk_LOADER_MTD_OFFSET	"0x0"
		#define nk_LOADER_MTD_LEN	"0x80000"

		#define nk_IMAGE_MTD		"/dev/mtd1"
		#define nk_IMAGE_MTD_OFFSET	"0x0"
		#define nk_IMAGE_MTD_LEN	"0x500000"

		#define nk_FileSystem_MTD		"/dev/mtd2"
		#define nk_FileSystem_MTD_OFFSET	"0x0"
		#define nk_FileSystem_MTD_LEN		"0x1900000"

		#define nk_DataBase_MTD		"/dev/mtd3"
		#define nk_DataBase_MTD_OFFSET	"0x0"
		#define nk_DataBase_MTD_LEN	"0x100000"

		#define nk_FACTORY_MTD		"/dev/mtd4"
		#define nk_FACTORY_MTD_OFFSET	"0x0"
		#define nk_FACTORY_MTD_LEN	"0x40000"

		#define nk_OTHER_MTD		"/dev/mtd5"
		#define nk_OTHER_MTD_OFFSET	"0x0"
		#define nk_OTHER_MTD_LEN	"0x40000"
	#else
		#define nk_LOADER_MTD		"/dev/mtd0"
		#define nk_LOADER_MTD_OFFSET	"0x0"
		#define nk_LOADER_MTD_LEN	"0x80000"

		#define nk_IMAGE_MTD		"/dev/mtd1"
		#define nk_IMAGE_MTD_OFFSET	"0x0"
		#define nk_IMAGE_MTD_LEN	"0x1E00000"

		#define nk_DataBase_MTD		"/dev/mtd2"
		#define nk_DataBase_MTD_OFFSET	"0x0"
		#define nk_DataBase_MTD_LEN	"0x100000"

		#define nk_FACTORY_MTD		"/dev/mtd3"
		#define nk_FACTORY_MTD_OFFSET	"0x0"
		#define nk_FACTORY_MTD_LEN	"0x40000"

		#define nk_OTHER_MTD		"/dev/mtd4"
		#define nk_OTHER_MTD_OFFSET	"0x0"
		#define nk_OTHER_MTD_LEN	"0x40000"
	#endif

#elif defined CONFIG_MODEL_QVM2000
	#define nk_LOADER_MTD		"/dev/mtd0"
	#define nk_LOADER_MTD_OFFSET	"0x0"
	#define nk_LOADER_MTD_LEN	"0x80000"

	#define nk_IMAGE_MTD		"/dev/mtd1"
	#define nk_IMAGE_MTD_OFFSET	"0x0"
	#define nk_IMAGE_MTD_LEN	"0x1E00000"

	#define nk_DataBase_MTD		"/dev/mtd2"
	#define nk_DataBase_MTD_OFFSET	"0x0"
	#define nk_DataBase_MTD_LEN	"0x100000"

	#define nk_FACTORY_MTD		"/dev/mtd3"
	#define nk_FACTORY_MTD_OFFSET	"0x0"
	#define nk_FACTORY_MTD_LEN	"0x40000"

	#define nk_OTHER_MTD		"/dev/mtd4"
	#define nk_OTHER_MTD_OFFSET	"0x0"
	#define nk_OTHER_MTD_LEN	"0x40000"

#elif defined CONFIG_MODEL_QVM2000
	#define nk_LOADER_MTD		"/dev/mtd0"
	#define nk_LOADER_MTD_OFFSET	"0x0"
	#define nk_LOADER_MTD_LEN	"0x80000"

	#define nk_IMAGE_MTD		"/dev/mtd1"
	#define nk_IMAGE_MTD_OFFSET	"0x0"
	#define nk_IMAGE_MTD_LEN	"0x1E00000"

	#define nk_DataBase_MTD		"/dev/mtd2"
	#define nk_DataBase_MTD_OFFSET	"0x0"
	#define nk_DataBase_MTD_LEN	"0x100000"

	#define nk_FACTORY_MTD		"/dev/mtd3"
	#define nk_FACTORY_MTD_OFFSET	"0x0"
	#define nk_FACTORY_MTD_LEN	"0x40000"

	#define nk_OTHER_MTD		"/dev/mtd4"
	#define nk_OTHER_MTD_OFFSET	"0x0"
	#define nk_OTHER_MTD_LEN	"0x40000"

#elif defined CONFIG_MODEL_GQF650_DUAL_CORE
	#define nk_LOADER_MTD		"/dev/mtd0"
	#define nk_LOADER_MTD_OFFSET	"0x0"
	#define nk_LOADER_MTD_LEN	"0x80000"

	#define nk_IMAGE_MTD		"/dev/mtd1"
	#define nk_IMAGE_MTD_OFFSET	"0x0"
	#define nk_IMAGE_MTD_LEN	"0x1D00000"

	#define nk_DataBase_MTD		"/dev/mtd2"
	#define nk_DataBase_MTD_OFFSET	"0x0"
	#define nk_DataBase_MTD_LEN	"0x100000"

	#define nk_DataBase1_MTD	"/dev/mtd3"
	#define nk_DataBase1_MTD_OFFSET	"0x0"
	#define nk_DataBase1_MTD_LEN	"0x100000"

	#define nk_FACTORY_MTD		"/dev/mtd4"
	#define nk_FACTORY_MTD_OFFSET	"0x0"
	#define nk_FACTORY_MTD_LEN	"0x40000"

#elif defined CONFIG_MODEL_GQF1100
	#define nk_LOADER_MTD		"/dev/mtd0"
	#define nk_LOADER_MTD_OFFSET	"0x0"
	#define nk_LOADER_MTD_LEN	"0x80000"

	#define nk_IMAGE_MTD		"/dev/mtd1"
	#define nk_IMAGE_MTD_OFFSET	"0x0"
	#define nk_IMAGE_MTD_LEN	"0x1E00000"

	#define nk_DataBase_MTD		"/dev/mtd2"
	#define nk_DataBase_MTD_OFFSET	"0x0"
	#define nk_DataBase_MTD_LEN	"0x100000"

	#define nk_FACTORY_MTD		"/dev/mtd3"
	#define nk_FACTORY_MTD_OFFSET	"0x0"
	#define nk_FACTORY_MTD_LEN	"0x40000"

#elif defined CONFIG_MODEL_QTM3000
	#define nk_IMAGE2_MTD		"/dev/mtd0"
	#define nk_IMAGE2_MTD_OFFSET	"0x0"
	#define nk_IMAGE2_MTD_LEN	"0x800000"

	#define nk_DataBase2_MTD	"/dev/mtd1"
	#define nk_DataBase2_MTD_OFFSET	"0x0"
	#define nk_DataBase2_MTD_LEN	"0x1800000"

	#define nk_LOADER_MTD		"/dev/mtd2"
	#define nk_LOADER_MTD_OFFSET	"0x0"
	#define nk_LOADER_MTD_LEN	"0x80000"

	#define nk_IMAGE_MTD		"/dev/mtd3"
	#define nk_IMAGE_MTD_OFFSET	"0x0"
	#define nk_IMAGE_MTD_LEN	"0x1E00000"

	#define nk_DataBase_MTD		"/dev/mtd4"
	#define nk_DataBase_MTD_OFFSET	"0x0"
	#define nk_DataBase_MTD_LEN	"0x100000"

	#define nk_FACTORY_MTD		"/dev/mtd5"
	#define nk_FACTORY_MTD_OFFSET	"0x0"
	#define nk_FACTORY_MTD_LEN	"0x40000"
#endif

	/* here comes the flash0's layout */
#if defined CONFIG_NK_CRAMFS

#define PMON_START_ADDRESS		0xBDC00000
#define IMAGE1_START_ADDRESS		0xBDC80000
#define IMAGE2_START_ADDRESS		0xBE180000
#define USER_CONFIG_START_ADDRESS	0xBFA80000
#define USER_CONFIG_SIZE		0x100000

#define IMAGE_HDR_BASE			IMAGE1_START_ADDRESS
#define PMON_SIZE			(IMAGE1_START_ADDRESS - PMON_START_ADDRESS)
#define IMAGE1_SIZE			(IMAGE2_START_ADDRESS - IMAGE1_START_ADDRESS)
#define IMAGE2_SIZE			(USER_CONFIG_START_ADDRESS - IMAGE2_START_ADDRESS)

#else
	// -->
#if 0
#define PROTECTED1_START_ADDRESS	0xBF800000
#define IMAGE1_START_ADDRESS		0xBF810000
#define PMON_START_ADDRESS			0xBFC00000
#define IMAGE2_START_ADDRESS		0xBFC40000
#define USER_CONFIG_START_ADDRESS	0xBFF00000
#define USER_CONFIG_SIZE			0x100000 - 0x10000
#define PROTECTED2_START_ADDRESS	0xBFFF0000
#else
#define IMAGE1_START_ADDRESS		0xBFC40000
#define PMON_START_ADDRESS			0xBFC00000
#define IMAGE3_START_ADDRESS		0xBF800000
#define USER_CONFIG_START_ADDRESS	0xBFF00000
#define USER_CONFIG_SIZE		0x100000
#endif
	// <--

	/* here comes the flash1's layout */
	// -->
#if 0
	/* 0xBE000000 ~ 0xBE010000 --> protected area 3*/
#define PROTECTED3_START_ADDRESS	0xBE000000
#define IMAGE3_START_ADDRESS		0xBE010000
#define PROTECTED4_START_ADDRESS	0xBE7F0000
#else
	//#define IMAGE3_START_ADDRESS		0xBE000000
#define IMAGE2_START_ADDRESS		0xBF000000
#define PROTECTED_START_ADDRESS		0xBF800000
#endif
	// <--
	//#define IMAGE1_SIZE				(PMON_START_ADDRESS - IMAGE1_START_ADDRESS)
	//#define IMAGE2_SIZE				(USER_CONFIG_START_ADDRESS - IMAGE2_START_ADDRESS)
	//#define IMAGE3_SIZE				(PROTECTED4_START_ADDRESS - IMAGE3_START_ADDRESS)
	//#define PMON_SIZE					(IMAGE2_START_ADDRESS - PMON_START_ADDRESS)
#define IMAGE3_SIZE					(PMON_START_ADDRESS - IMAGE3_START_ADDRESS)
#define IMAGE1_SIZE					(USER_CONFIG_START_ADDRESS - IMAGE1_START_ADDRESS)
#define IMAGE2_SIZE					(PROTECTED_START_ADDRESS - IMAGE2_START_ADDRESS)
#define PMON_SIZE					(IMAGE1_START_ADDRESS - PMON_START_ADDRESS)
#define IMAGE_HDR_BASE				IMAGE1_START_ADDRESS
#endif
	// -->
	/* this value should be match with uClinux/brecis/gunzip/gunzip.lk */
#define COPY_RAM_ADDRESS			0x81500000
	// <--

#define FIRMWARE_PATTERN				"IMG"
#define BOOT_PATTERN					"BOOT"
#define MAC_PATTERN						"-MAC-!"
#define UN_BOOT_PATTERN					"BTIMG!"
#define NONBRAND_VENDOR_CODE			"nb"

#define CFG_IMG_HEADER // for loader if need do signature check or not 

// MultiVLAN

//protocol binding
enum rule_flush_mode 
{
	BINDING=1
};

#ifdef CONFIG_NK_PPTP_TRUNKING
/* add by chihmou, support pptp trunking 2008/06/11 */
#define sPPTP_TRUNK_ROUTE_TABLE	50
#define ePPTP_TRUNK_ROUTE_TABLE	55

#define sSROUTESDS3 3000
#define eSROUTESDS3 4000
#endif

#define sBINDING 5000
#define eBINDING 6000
#define sSROUTESDS0 7000
#define eSROUTESDS0 8000
#define sSROUTESDS1 9000
#define eSROUTESDS1 10000
#define sSROUTESDS2 11000
#define eSROUTESDS2 12000


//LLDP
#define LOCAL_PORT_TOTAL  		5
#define MSG_TXINTERVAL			20
#define MSG_TXHOLDMULTIPLIER		4
#define REINITDELAY			2
#define TXDELAY				2
#define NOTIFICATION_INTERVAL		0
#define LEN_MANADDR 			32
#define SPECIAL_TAG_LEN			6
//#define NK_LLDP_MODULE		//Zero:Jul.13.2006  Remark this line, if you don't support lldp.

//#ifdef CONFIG_NK_NAT_MODE
	#define ROUTER_GROUP_NUMBER	3
//#endif

#ifndef BOOTLOADER
// image header
struct image_header{
	char signature[8];
	char fwtype[8];
	char fwver[12];
	char fwdate[4];
	//char res2[8];
	unsigned int res;
	unsigned int knaddr;
	unsigned int fsaddr;
	unsigned int knsize;
	unsigned int fssize;
	unsigned int fwsize;
	unsigned int ihcrc;
	unsigned int fwcrc;
} ;
#endif

// image header
struct nk_image_header{
	char signature[8];
	char fwtype[8];
	char fwver[12];
	char fwdate[4];
	//char res2[8];
	unsigned int res;
	unsigned int knaddr;
	unsigned int fsaddr;
	unsigned int knsize;
	unsigned int fssize;
	unsigned int fwsize;
	unsigned int ihcrc;
	unsigned int fwcrc;
} ;

#define MAC_ADDRESS_NUM				1
#define MAC_ADDRESS_LEN				6
#define SERIAL_NO_LEN				20
#define RESERVE_BANK				6
#define MAC_SERIAL_LEN				(MAC_ADDRESS_LEN + SERIAL_NO_LEN + RESERVE_BANK)
#endif


/*--------------Defined Function ID for License Key---------------------------------*/
/*----------001~099 only enable:2/disable:0/trial:1 depend on old/new value 0,1,2---*/
/*----------101~199 only Service Number depend on old/new value---------------------*/
#define FID_QnoSniff		  1
#define FID_Inbound_Load_Balance  2
#define FID_VPN_QOS	      	  3
#define FID_VPN_Load_Balance	  4
#define FID_EBoard_WebAuth	  5
#define FID_HA		          6
#define FID_L7_App_Filter	  7
#define FID_FWTrial	     98
#define FID_LicenseKey		     99

#define FID_SSL_Tunnel		    101
#define FID_SoftKey		    102
#define FID_Speed_up_Count_down	    199

/*------------------------End Define LicenseKye-----------------------------------*/

