
#ifndef _nk_MIB_h_
#define _nk_MIB_h_

/*
 * include important headers 
 */
#include <net-snmp/net-snmp-config.h>
#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif

/*
 * needed by util_funcs.h 
 */
#if TIME_WITH_SYS_TIME
#  include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#if HAVE_WINSOCK_H
#include <winsock.h>
#endif
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>
/*
#include "../../../../nku/util.c"
#include "../../../../nku/getlink.c"
#include "../../../../nku/kd_sys.c"
#include "../../../../nku/flash.c"
#include "../../../../../../../environment/nkutil.h"
*/
#include <nkdef.h>
#include <nkutil.h>
#include "../../../../tool/nkuserlandconf.h"

#include "common.h"
#include "basicMgt.h"
#include "advanceMgt.h"
#include "wanMgt.h"

//#define SNMP_Debug
#ifdef SNMP_Debug
	#define dprintf(arg...);	printf(arg);
#else
	#define dprintf(arg...);
#endif

//int nk_awk(char *src, int pos, char *dst);
int nk_parse_tmp_file(char *filename, int pos, char *dst);


#define	SNMP_TMP_FILE	"/tmp/snmp_tmp.txt"
#define	SNMP_TMP_FILE1	"/tmp/snmp_tmp1.txt"

#define NK_SNMP_STR_LEN	256
#define NK_STR64_LEN 64


/* purpose   	:  SNMP author :  Gavin.Lin  date :  2010-05-27            */	
/* description  :  Customer requirements, add odm OID for netklass       */
/* #define VENDOR_OID	1, 3, 6, 1, 4, 1, 9, 6, 1, 103 */
/* purpose   	:  SNMP_OID_CHANGE author :  Gavin.Lin  date :  2010-07-19 */	
/* description  :  Customer requirements                                   */
#define VENDOR_OID	1, 3, 6, 1, 4, 1, 9, 6, 1, 105



#endif



