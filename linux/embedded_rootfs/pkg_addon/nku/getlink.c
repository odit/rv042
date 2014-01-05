#include <features.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "./getlink.h"
#include "nkutil.h"
#include <stdarg.h>
#if defined(CONFIG_NK_SWITCH_ARCH)
#include "../../../kernel_2.6/nk_switch/nk_switch_common.h"
#endif

#define _(x) x
#define _PATH_PROCNET_DEV               "/proc/net/dev"
#define  KdDoCmdPrint(a,b)	kd_doCommand(a, CMD_PRINT, ASH_DO_NOTHING, b)
#define ishex(x) (((x) >= '0' && (x) <= '9') || ((x) >= 'a' && (x) <= 'f') || \
		  ((x) >= 'A' && (x) <= 'F'))

static int skfd = -1;			/* generic raw socket desc.     */
static int procnetdev_vsn = 1;
static const char TRext[] = "\0\0k\0M";
static unsigned int WAN_CONN_STAT = 0xffffffff;

#ifdef CONFIG_NK_DYNAMIC_PORT_NUM
static unsigned int WAN_CONN_MAP[CONFIG_NK_NUM_WAN] = {0,1,2,3,4,5,6,7};
#else
static unsigned int WAN_CONN_MAP[CONFIG_NK_NUM_WAN] = {0,1,2,3};
#endif

/* Global Variable */
#ifdef CONFIG_NK_DYNAMIC_PORT_NUM
	unsigned int DYNAMIC_NUM_LAN;
	unsigned int DYNAMIC_NUM_WAN;
	unsigned int DYNAMIC_NUM_DMZ;
	unsigned int DYNAMIC_NUM_USB;
	unsigned int SWITCH_TYPE;
#endif

void nk_set_wan_conn_led(unsigned int if_num, int state);
void nk_set_led(unsigned int uLedNo, int state);

static void console_printf(char *str)
{
 FILE *fp;
 fp = fopen("/dev/console", "w");
 if (fp == NULL) {
   return;
 }
 fprintf(fp, "ssi.cgi: %s\r\n", str);
 fflush(fp);
 fclose(fp);
}

static void console_printf2(char *fmt, ...)
{
	char temp[1024];
	va_list ap;

	va_start(ap, fmt);
	vsprintf(temp, fmt, ap);
	va_end(ap);

	console_printf(temp);
}

/*
	print string to console
	Example 1:
		con_printf("This is a string.\r\n");
	Example 2:
		char buf[200];
		fprintf(buf, "My func is [%s].\r\n", __func__);
		con_printf(buf);
*/
void con_printf(char *str)
{
 FILE *fp;
 fp = fopen("/dev/console", "w");
 if (fp == NULL) {
   return;
 }
 fprintf(fp, "%s", str);
 fflush(fp);
 fclose(fp);
}

#ifdef CONFIG_NK_DYNAMIC_PORT_NUM
unsigned int Get_Switch_Type() {
    char cmdBuf[128];

    KdDoCmdPrint("VERSION SWITCH_TYPE", cmdBuf);

    return atoi(cmdBuf);
}

unsigned int Get_Num_Lan() {
    char cmdBuf[128];
    int tmp;

    KdDoCmdPrint("VERSION NUM_LAN", cmdBuf);

    tmp = atoi(cmdBuf);
    if ( !tmp )
        return 5;

    return tmp;
}

unsigned int Get_Num_Wan() {
    char cmdBuf[128];
    int tmp;

    KdDoCmdPrint("VERSION NUM_WAN", cmdBuf);

    tmp = atoi(cmdBuf);
    if ( !tmp )
        return 8;

    return tmp;
}

unsigned int Get_Num_Dmz() {
    char cmdBuf[128];

    KdDoCmdPrint("VERSION NUM_DMZ", cmdBuf);

    return atoi(cmdBuf);
}

unsigned int Get_Num_USB() {
    char cmdBuf[128];
    int tmp;

    KdDoCmdPrint("VERSION NUM_USB", cmdBuf);

    tmp = atoi(cmdBuf);
    if ( !tmp )
        return 0;

    return tmp;
}

unsigned int WAN_2_FRONT_3000[] = {4,3,2,1,8,7,6,5};
unsigned int WAN_2_FRONT_1450[] = {1,2,3,4,5,6,7,8};
unsigned int WAN_2_FRONT_650[] = {5,4,3,2,1};
unsigned int WAN_2_FRONT_363[] = {2,1};
unsigned int WAN_2_FRONT_005[] = {4,3,2,1};
unsigned int WAN_2_FRONT_016[] = {7,6,5,4,3,2,1,8};
unsigned int WAN_2_FRONT_042[] = {1,2};
unsigned int WAN_2_FRONT_082[] = {2,1};
unsigned int WAN_2_FRONT_DEF[] = {4,3,2,1,5};

unsigned int wanportmap(unsigned int wanport, unsigned int switch_type) {
	if ( switch_type == 5 )
		return WAN_2_FRONT_005[wanport - 1];
	else if ( switch_type == 363 )
		return WAN_2_FRONT_363[wanport - 1];
	else if ( switch_type == 1450 )
		return WAN_2_FRONT_1450[wanport - 1];
	else if ( switch_type == 650 || switch_type == 1550)
		return WAN_2_FRONT_650[wanport - 1];
	else if ( switch_type == 3000 )
		return WAN_2_FRONT_3000[wanport - 1];
	else if ( switch_type == 16 )
		return WAN_2_FRONT_016[wanport - 1];
	else if ( switch_type == 42 )
		return WAN_2_FRONT_042[wanport - 1];
	else if ( switch_type == 82 )
		return WAN_2_FRONT_082[wanport - 1];
	else
		return WAN_2_FRONT_DEF[wanport - 1];
}

uint32_t LAN_2_FRONT_016[] = { 1, 2, 3, 4, 5, 6, 7, 8 };
uint32_t LAN_2_FRONT_042[] = { 1, 2, 3, 4 };
uint32_t LAN_2_FRONT_082[] = { 1, 2, 3, 4, 5, 6, 7, 8 };
uint32_t lanportmap ( uint32_t lanidx, unsigned int switch_type ) {

	switch ( switch_type ) {
		case 16:
			return LAN_2_FRONT_016[lanidx - 1];
		case 42:
			return LAN_2_FRONT_042[lanidx - 1];
		case 82:
			return LAN_2_FRONT_082[lanidx - 1];
		default:
			return 0;
	}
}
#else
unsigned int WAN_2_FRONT[4+1]={4,3,2,1,5};
unsigned int wanportmap(unsigned int wanport)
{
	return WAN_2_FRONT[wanport-1];
}
#endif

static char *strncpyz(char *dest, char const *src, size_t size)
{
    if (!size--)
	return dest;
    strncpy(dest, src, size);
    dest[size] = 0; /* Make sure the string is null terminated */
    return dest;
}

static int
htoi(s)
	unsigned char	*s;
{
	int	value;
	char	c;

	c = s[0];
	if (isupper(c))
		c = tolower(c);
	value = (c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10) * 16;

	c = s[1];
	if (isupper(c))
		c = tolower(c);
	value += c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10;

	return (value);
}

static void
url_unescape(str)
	unsigned char	*str;
{
	unsigned char	*dest = str;

	while (str[0])
	{
		if (str[0] == '+')
			dest[0] = ' ';
		else if (str[0] == '%' && ishex(str[1]) && ishex(str[2]))
		{
			dest[0] = (unsigned char) htoi(str + 1);
			str += 2;
		}
		else
			dest[0] = str[0];

		str++;
		dest++;
	}

	dest[0] = '\0';
}
/*purpose     : 0012882 author : Jason.Huang date : 2010-07-23*/
/*description : support more special character              */

static int find_special_word(char *offset, int len, int i)
{
f_again:
    if (((offset[i+len+1] == '\"')&&(offset[i+len+2] == '&'))||((offset[i+len+1] == '\"')&&(offset[i+len+2] == '\"')))
    {
	i++;
	for (i; (((offset[i+len+1] == '\"')&&(offset[i+len+2] == '&'))||((offset[i+len+1] == '\"')&&(offset[i+len+2] == '\"'))); i++);
	goto f_again;
    }
    return i;
}
/*purpose     : 0012882 author : michael lu date : 2010-07-21*/
/*description : support more special character           */
/*purpose     : 0012882 author : michael.lu date : 2010-07-23*/
/*description : remove unwork code              */
static char* name_get_value(char *string, char *varname, char *retval, int buf_len, char *offset)
{
	int i, len=strlen(varname), break_loop=0, ValueEmpty=0;
	char searchName[257];
	char checkValueEnd[257];
	
	strcpy(searchName, varname);
	strcpy(checkValueEnd, varname);
	strcat(checkValueEnd, "=\"\"");
	strcat(searchName,"=\"");
	len+=2;
	
	if (!string)
	{
		retval[0] = '\0';
		return NULL;
	}
	offset = offset ? offset : string;
	
	char *p=NULL,*q=NULL;
	if(p=strstr(offset, checkValueEnd))
	{
		q=strstr( offset , searchName );
		if(p == q)
			ValueEmpty=1;
	}
	if (((offset=strstr(offset, searchName)) != 0) && (ValueEmpty==0))
	{
		for (i=0; ((offset+i+len)<(string+strlen(string)) && (break_loop==0)); i++)
		{
			if(((offset[i+len+1] == '\"')&&(offset[i+len+2] == '&'))||((offset[i+len+1] == '\"')&&(offset[i+len+2] == '\"')))
			{
				break_loop=1;
			}
		}
		
		i = find_special_word(offset, len, i);
		if (buf_len >= (i+1))
		{
			strncpyz(retval, (offset+len), i+1);
		}
		else
		{
			console_printf("value buffer too small !");
		}
	
		return (offset+len+2+(i+1));
	}
	retval[0] = '\0';
	return NULL;
}

int nk_led_on(unsigned int uLedNo)
{
	int i,fd;
	char tmp[50];

#ifdef CONFIG_NK_DYNAMIC_PORT_NUM
	DYNAMIC_NUM_LAN = Get_Num_Lan();
	DYNAMIC_NUM_WAN = Get_Num_Wan();
	DYNAMIC_NUM_DMZ = Get_Num_Dmz();
	SWITCH_TYPE = Get_Switch_Type();
#endif

#ifdef CONFIG_NK_DYNAMIC_PORT_NUM
	if ( SWITCH_TYPE == 1450 ) {
		if ( (uLedNo >= NK_WAN_CONN_LED + 0) && (uLedNo <= NK_WAN_CONN_LED + (DYNAMIC_NUM_WAN-1) ) ) // WAN1~8
			nk_set_wan_conn_led((uLedNo-NK_WAN_CONN_LED+1), LED_ON);
		else if(uLedNo == NK_DMZ_CONN_LED) // DMZ
			nk_set_wan_conn_led((NK_WAN_CONN_LED + DYNAMIC_NUM_WAN - 1), LED_ON);
		else if(uLedNo == NK_DIAG_LED) // DIAG
			nk_set_led(NK_DIAG_LED, LED_ON);
		else
		{
			sprintf(tmp, "nk_led_on: error uLedNo[%d]\n", uLedNo);
			console_printf(tmp);
		}
	}
	else {
		if((uLedNo >= NK_WAN_CONN_LED + 0) && (uLedNo <= NK_WAN_CONN_LED + (DYNAMIC_NUM_WAN+DYNAMIC_NUM_DMZ-1)))
		{
// 			/* if DMZ has self GPIO pin, then use func(nk_set_led), else use func(nk_set_wan_conn_led) */
// 			if( ( uLedNo == NK_WAN_CONN_LED + DYNAMIC_NUM_WAN + DYNAMIC_NUM_DMZ- 1) && ( SWITCH_TYPE != 5 ) )
// 				nk_set_led(NK_DMZ_CONN_LED, LED_ON);
// 			else
				nk_set_wan_conn_led((uLedNo-NK_WAN_CONN_LED+1), LED_ON);
		}
		else if(uLedNo == NK_DMZ_CONN_LED)
		{
			nk_set_led(NK_DMZ_CONN_LED, LED_ON);
		}
		else if(uLedNo == NK_DIAG_LED)
		{
			nk_set_led(NK_DIAG_LED, LED_ON);
		}
		else
		{
			sprintf(tmp, "nk_led_on: error uLedNo[%d]\n", uLedNo);
			console_printf(tmp);
		}
	}
#else
	if((uLedNo >= NK_WAN_CONN_LED + 0) && (uLedNo <= NK_WAN_CONN_LED + (CONFIG_NK_NUM_WAN-1)))
	{
		/* incifer 2008/08 */
		if(uLedNo == NK_WAN_CONN_LED + CONFIG_NK_NUM_WAN - 1)
			nk_set_led(NK_DMZ_CONN_LED, LED_ON);
		else
			nk_set_wan_conn_led((uLedNo-NK_WAN_CONN_LED+1), LED_ON);
	}
	else if(uLedNo == NK_DMZ_CONN_LED)
	{
		nk_set_led(NK_DMZ_CONN_LED, LED_ON);
	}
	else if(uLedNo == NK_DIAG_LED)
	{
		nk_set_led(NK_DIAG_LED, LED_ON);
	}
	else
	{
		sprintf(tmp, "nk_led_on: error uLedNo[%d]\n", uLedNo);
		console_printf(tmp);
	}
#endif

	return 0;
}

int nk_led_off(unsigned int uLedNo)
{
	int i,fd;
	char tmp[50];

#ifdef CONFIG_NK_DYNAMIC_PORT_NUM
	DYNAMIC_NUM_LAN = Get_Num_Lan();
	DYNAMIC_NUM_WAN = Get_Num_Wan();
	DYNAMIC_NUM_DMZ = Get_Num_Dmz();
	SWITCH_TYPE = Get_Switch_Type();
#endif

#ifdef CONFIG_NK_DYNAMIC_PORT_NUM
	if ( SWITCH_TYPE == 1450 ) {
		if((uLedNo >= NK_WAN_CONN_LED + 0) && (uLedNo <= NK_WAN_CONN_LED + (DYNAMIC_NUM_WAN-1)))
			nk_set_wan_conn_led((uLedNo-NK_WAN_CONN_LED+1), LED_OFF);
		else if(uLedNo == NK_DMZ_CONN_LED)
			nk_set_wan_conn_led((NK_WAN_CONN_LED + DYNAMIC_NUM_WAN - 1), LED_OFF);
		else if(uLedNo == NK_DIAG_LED)
			nk_set_led(NK_DIAG_LED, LED_OFF);
		else
		{
			sprintf(tmp, "nk_led_off: error uLedNo[%d]\n", uLedNo);
			console_printf(tmp);
		}
	}
	else {
		if((uLedNo >= NK_WAN_CONN_LED + 0) && (uLedNo <= NK_WAN_CONN_LED + (DYNAMIC_NUM_WAN+DYNAMIC_NUM_DMZ-1)))
		{
// 			/* if DMZ has self GPIO pin, then use func(nk_set_led), else use func(nk_set_wan_conn_led) */
// 			if( ( uLedNo == NK_WAN_CONN_LED + DYNAMIC_NUM_WAN + DYNAMIC_NUM_DMZ - 1) && ( SWITCH_TYPE != 5 ) )
// 				nk_set_led(NK_DMZ_CONN_LED, LED_OFF);
// 			else
				nk_set_wan_conn_led((uLedNo-NK_WAN_CONN_LED+1), LED_OFF);
		}
		else if(uLedNo == NK_DMZ_CONN_LED)
		{
			nk_set_led(NK_DMZ_CONN_LED, LED_OFF);
		}
		else if(uLedNo == NK_DIAG_LED)
		{
			nk_set_led(NK_DIAG_LED, LED_OFF);
		}
		else
		{
			sprintf(tmp, "nk_led_off: error uLedNo[%d]\n", uLedNo);
			console_printf(tmp);
		}
	}
#else
	if((uLedNo >= NK_WAN_CONN_LED + 0) && (uLedNo <= NK_WAN_CONN_LED + (CONFIG_NK_NUM_WAN-1)))
	{
		/* incifer 2008/08 */
		if(uLedNo == NK_WAN_CONN_LED + CONFIG_NK_NUM_WAN - 1)
			nk_set_led(NK_DMZ_CONN_LED, LED_OFF);
		else
			nk_set_wan_conn_led((uLedNo-NK_WAN_CONN_LED+1), LED_OFF);
	}
	else if(uLedNo == NK_DMZ_CONN_LED)
	{
		nk_set_led(NK_DMZ_CONN_LED, LED_OFF);
	}
	else if(uLedNo == NK_DIAG_LED)
	{
		nk_set_led(NK_DIAG_LED, LED_OFF);
	}
	else
	{
		sprintf(tmp, "nk_led_off: error uLedNo[%d]\n", uLedNo);
		console_printf(tmp);
	}
#endif

	return 0;
}

/**
 *	@param inf: range: 1 ~ CONFIG_NK_NUM_WAN
**/
unsigned int inf2led(unsigned int inf)
{
	return WAN_CONN_MAP[inf-1];
}

/**
 *	@param if_num: range: 1 ~ CONFIG_NK_NUM_WAN
 *	@param state: LED_ON/LED_OFF
**/
void nk_set_wan_conn_led(unsigned int if_num, int state)
{
	int fd;
	char tmp[50];

#ifdef CONFIG_NK_DYNAMIC_PORT_NUM
	DYNAMIC_NUM_LAN = Get_Num_Lan();
	DYNAMIC_NUM_WAN = Get_Num_Wan();
	DYNAMIC_NUM_DMZ = Get_Num_Dmz();
	SWITCH_TYPE = Get_Switch_Type();
#endif

#ifdef CONFIG_NK_DYNAMIC_PORT_NUM
	if(if_num > DYNAMIC_NUM_WAN || if_num < 1)
#else
	if(if_num > CONFIG_NK_NUM_WAN || if_num < 1)
#endif
		return;

	/* Led off set high, Led on set low */
	if(state == LED_OFF)
	{
	#ifdef CONFIG_NK_DYNAMIC_PORT_NUM
		if ( SWITCH_TYPE == 1450 )
			WAN_CONN_STAT |= ( 1 << ( wanportmap (if_num, SWITCH_TYPE) - 1 ) );
		else
			WAN_CONN_STAT |= (1 << inf2led(if_num));
	#else
		WAN_CONN_STAT |= (1 << inf2led(if_num));
	#endif
	}
	else if(state == LED_ON)
	{
	#ifdef CONFIG_NK_DYNAMIC_PORT_NUM
		if ( SWITCH_TYPE == 1450 )
			WAN_CONN_STAT &= ~( 1 << ( wanportmap (if_num, SWITCH_TYPE) - 1 ) );
		else
			WAN_CONN_STAT &= ~(1 << inf2led(if_num));
	#else
		WAN_CONN_STAT &= ~(1 << inf2led(if_num));
	#endif
	}

	if ((fd = open("/dev/nk_switch", O_RDONLY)) < 0)
	{
		console_printf("nk_set_wan_conn_led: failed open /dev/nk_switch");
		return;
	}

	if (ioctl(fd, SWITCH_SET_WAN_CONN_LED, &WAN_CONN_STAT) != 0)
		console_printf("nk_set_wan_conn_led: ioctl error");

	close(fd);
}

void nk_set_led(unsigned int uLedNo, int state)
{
	int fd;
	char tmp[50];

	/* check vaild of uLedNo/state */
	if(uLedNo != NK_DMZ_CONN_LED && uLedNo != NK_DIAG_LED)
	{
		sprintf(tmp, "nk_set_led: Unknow uLedNo[%d]\n", uLedNo);
		console_printf(tmp);
		return;
	}
	if(state != LED_ON && state != LED_OFF)
	{
		sprintf(tmp, "nk_set_led: Unknow state[%d]\n", state);
		console_printf(tmp);
		return;
	}

	if ((fd = open("/dev/nk_switch", O_RDONLY)) < 0)
	{
		console_printf("nk_set_led: failed open /dev/nk_switch");
		return;
	}

	if(state == LED_ON)
	{
		if (ioctl(fd, SWITCH_SET_LED_ON, &uLedNo) != 0)
				console_printf("nk_set_led: ioctl error");	
	}
	else if(state == LED_OFF)
	{
		if (ioctl(fd, SWITCH_SET_LED_OFF, &uLedNo) != 0)
				console_printf("nk_set_led: ioctl error");
	}

	close(fd);
}

static int get_dev_fields(char *bp, struct interface *ife)
{
	char temp[300];
    switch (procnetdev_vsn) {
    case 3:
	sscanf(bp,
	"%Lu %Lu %lu %lu %lu %lu %lu %lu %Lu %Lu %lu %lu %lu %lu %lu %lu",
	       &ife->stats.rx_bytes,
	       &ife->stats.rx_packets,
	       &ife->stats.rx_errors,
	       &ife->stats.rx_dropped,
	       &ife->stats.rx_fifo_errors,
	       &ife->stats.rx_frame_errors,
	       &ife->stats.rx_compressed,
	       &ife->stats.rx_multicast,

	       &ife->stats.tx_bytes,
	       &ife->stats.tx_packets,
	       &ife->stats.tx_errors,
	       &ife->stats.tx_dropped,
	       &ife->stats.tx_fifo_errors,
	       &ife->stats.collisions,
	       &ife->stats.tx_carrier_errors,
	       &ife->stats.tx_compressed);

	break;
    case 2:
	sscanf(bp, "%Lu %Lu %lu %lu %lu %lu %Lu %Lu %lu %lu %lu %lu %lu",
	       &ife->stats.rx_bytes,
	       &ife->stats.rx_packets,
	       &ife->stats.rx_errors,
	       &ife->stats.rx_dropped,
	       &ife->stats.rx_fifo_errors,
	       &ife->stats.rx_frame_errors,

	       &ife->stats.tx_bytes,
	       &ife->stats.tx_packets,
	       &ife->stats.tx_errors,
	       &ife->stats.tx_dropped,
	       &ife->stats.tx_fifo_errors,
	       &ife->stats.collisions,
	       &ife->stats.tx_carrier_errors);
	ife->stats.rx_multicast = 0;
	break;
    case 1:
	sscanf(bp, "%Lu %lu %lu %lu %lu %Lu %lu %lu %lu %lu %lu",
	       &ife->stats.rx_packets,
	       &ife->stats.rx_errors,
	       &ife->stats.rx_dropped,
	       &ife->stats.rx_fifo_errors,
	       &ife->stats.rx_frame_errors,

	       &ife->stats.tx_packets,
	       &ife->stats.tx_errors,
	       &ife->stats.tx_dropped,
	       &ife->stats.tx_fifo_errors,
	       &ife->stats.collisions,
	       &ife->stats.tx_carrier_errors);
	ife->stats.rx_bytes = 0;
	ife->stats.tx_bytes = 0;
	ife->stats.rx_multicast = 0;
	break;
    }
    return 0;
}

static inline int procnetdev_version(char *buf)
{
    if (strstr(buf, "compressed"))
	return 3;
    if (strstr(buf, "bytes"))
	return 2;
    return 1;
}

static char *get_name(char *name, char *p)
{
    while (isspace(*p))
	p++;
    while (*p) {
	if (isspace(*p))
	    break;
	if (*p == ':') {	/* could be an alias */
	    char *dot = p, *dotname = name;
	    *name++ = *p++;
	    while (isdigit(*p))
		*name++ = *p++;
	    if (*p != ':') {	/* it wasn't, backup */
		p = dot;
		name = dotname;
	    }
	    if (*p == '\0')
		return NULL;
	    p++;
	    break;
	}
	*name++ = *p++;
    }
    *name++ = '\0';
    return p;
}

static void print_bytes_scaled(unsigned long long ull, const char *end)
{
	unsigned long long int_part;
	unsigned long frac_part;
	const char *ext;
	int i;

	frac_part = 0;
	ext = TRext;
	int_part = ull;
	for (i=0 ; i<2 ; i++) {
		if (int_part >= 1024) {
			frac_part = ((int_part % 1024) * 10) / 1024;
			int_part /= 1024;
			ext += 2;			/* Kb, Mb */
		}
	}

	printf("X bytes:%Lu (%Lu.%lu %sb)%s", ull, int_part, frac_part, ext, end);
}



/*2007/6/4 trenchen : add for check io. replace nk_if_statistic_get() which has bug*/
int nk_if_statistic_get_safe(char *target, struct interface *ife)
{
    static int proc_read;
    FILE *fh;
    char buf[512];
//    struct interface *ife;
    int err;
//console_printf("fnk_if_statistic_get_safe");
    if (proc_read)
	    return 0;
    if (!target)
	    proc_read = 1;

    fh = fopen(_PATH_PROCNET_DEV, "r");
    if (!fh) {
		fprintf(stderr, _("Warning: cannot open %s (%s). Limited output.\n"),
			_PATH_PROCNET_DEV, strerror(errno));
		//return if_readconf();
		return 0;
	}
    fgets(buf, sizeof buf, fh);	/* eat line */
    fgets(buf, sizeof buf, fh);

    procnetdev_vsn = procnetdev_version(buf);

    err = 0;
    while (fgets(buf, sizeof buf, fh)) {
	char *s, name[IFNAMSIZ];
	s = get_name(name, buf);
	//ife = add_interface(name);
//	ife = malloc(sizeof(struct interface));
	get_dev_fields(s, ife);
	ife->statistics_valid = 1;
	if (target && !strcmp(target,name))
		break;
    }
    if (ferror(fh)) {
	perror(_PATH_PROCNET_DEV);
	err = -1;
	proc_read = 0;
    }

    fclose(fh);

	if(ife)
	    return 1;//ife;
	else
		return 0;//NULL;
}





struct interface *nk_if_statistic_get(char *target)
{
	static int proc_read;
	FILE *fh;
	char buf[512], temp[300];
	struct interface *ife;
	int err;
	
	if (proc_read)
		return 0;
	if (!target)
		proc_read = 1;
	
	fh = fopen(_PATH_PROCNET_DEV, "r");
	if (!fh) {
			fprintf(stderr, _("Warning: cannot open %s (%s). Limited output.\n"),
				_PATH_PROCNET_DEV, strerror(errno));
			//return if_readconf();
			return 0;
		}
	fgets(buf, sizeof buf, fh);	/* eat line */
	fgets(buf, sizeof buf, fh);
	
	procnetdev_vsn = procnetdev_version(buf);
	
	err = 0;
	while (fgets(buf, sizeof buf, fh)) {
		char *s, name[IFNAMSIZ];
		s = get_name(name, buf);
		//ife = add_interface(name);
		ife = malloc(sizeof(struct interface));
		get_dev_fields(s, ife);
		ife->statistics_valid = 1;
	
		if (target && !strcmp(target,name))
			break;
	}
	if (ferror(fh)) {
		perror(_PATH_PROCNET_DEV);
		err = -1;
		proc_read = 0;
	}
	
	fclose(fh);

	if(ife)
	{
		//console_printf("test 1");
		return ife;
	}
	else
	{
		//console_printf("test 2");	
		return NULL;
	}
}

void print_if_statistic(struct interface *ife)
{
	char temp [300];
//console_printf("print_if_statistic");
#if 1

    if (ife->statistics_valid) {
	/* XXX: statistics are currently only printed for the primary address,
	 *      not for the aliases, although strictly speaking they're shared
	 *      by all addresses.
	 */
	printf("          ");

	printf(_("RX packets:%Lu errors:%lu dropped:%lu overruns:%lu frame:%lu\n"),
	       ife->stats.rx_packets, ife->stats.rx_errors,
	       ife->stats.rx_dropped, ife->stats.rx_fifo_errors,
	       ife->stats.rx_frame_errors);
	//if (can_compress)
	    //printf(_("             compressed:%lu\n"), ptr->stats.rx_compressed);
	printf("          ");
	printf(_("TX packets:%Lu errors:%lu dropped:%lu overruns:%lu carrier:%lu\n"),
	       ife->stats.tx_packets, ife->stats.tx_errors,
	       ife->stats.tx_dropped, ife->stats.tx_fifo_errors,
	       ife->stats.tx_carrier_errors);
	//printf(_("          collisions:%lu "), ptr->stats.collisions);
	//if (can_compress)
	    //printf(_("compressed:%lu "), ptr->stats.tx_compressed);
	//if (ptr->tx_queue_len != -1)
	    //printf(_("txqueuelen:%d "), ptr->tx_queue_len);
	printf("\n          R");
	print_bytes_scaled(ife->stats.rx_bytes, "  T");
	print_bytes_scaled(ife->stats.tx_bytes, "\n");

    }
#endif	
}

int NK_Config_PHY_Reg(char *if_name)
{
	struct ifreq ifr;
	int sockfd;
	char data[16];
	int ret;

	strncpy(ifr.ifr_name, if_name, IFNAMSIZ);
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		return 1;
	}

	//if (ioctl(sockfd, 0x101, &ifr) < 0)
	ret = 0;
	//ret = ioctl(sockfd, SIOCGIFFLAGS, &ifr);
	//if ((ret = ioctl(sockfd, SIOCGIFFLAGS, &ifr)) < 0)
	ret = ioctl(sockfd, 0x101, &ifr);
	if(ret < 0)
		return ret;
	else
		return 7;
		//return 2;
}

#if defined(CONFIG_NK_SWITCH_ARCH)
int fport_map_wan_016[] = { 7, 6, 5, 4, 3, 2, 1, 8 };
int fport_map_wan_042[] = { 1, 2 };
int fport_map_wan_082[] = { 2, 1 };
int NK_SET_PORT_STATUS(void) {

	int i, fd, dir, idx;
	uint32_t enabled, auto_negotiation, speed, duplex, priority;
	char path[32], tmpbuf[32];
	nk_switch_port_status_t new_status;

	DYNAMIC_NUM_LAN = Get_Num_Lan();
	DYNAMIC_NUM_WAN = Get_Num_Wan();
	DYNAMIC_NUM_DMZ = Get_Num_Dmz();
	SWITCH_TYPE = Get_Switch_Type();

	memset ( &new_status, 0, sizeof ( nk_switch_port_status_t ) );

	if ((fd = open("/dev/nk_switch", O_RDONLY)) < 0)
		return -1;

	for ( i = 1; i <= ( DYNAMIC_NUM_LAN + DYNAMIC_NUM_WAN + DYNAMIC_NUM_DMZ ); i++ ) {

		if ( i <= DYNAMIC_NUM_LAN ) {
			new_status.dir = 0;
			new_status.fport = lanportmap ( i, SWITCH_TYPE );
			dir = PORT_TYPE_LAN;
			idx = i;
		}
		else {
			new_status.dir = 1;
			new_status.fport = wanportmap ( i - DYNAMIC_NUM_LAN, SWITCH_TYPE );
			dir = PORT_TYPE_WAN;
			idx = i - DYNAMIC_NUM_LAN;
		}

		sprintf ( path, "PORT_%d%02d ENABLED", dir, idx );
		KdDoCmdPrint ( path, tmpbuf );
		sscanf ( tmpbuf, "%d", &enabled );

		sprintf ( path, "PORT_%d%02d AUTO", dir, idx );
		KdDoCmdPrint ( path, tmpbuf );
		sscanf ( tmpbuf, "%d", &auto_negotiation );

		sprintf ( path, "PORT_%d%02d SPEED", dir, idx );
		KdDoCmdPrint ( path, tmpbuf );
		sscanf ( tmpbuf, "%d", &speed );

		sprintf ( path, "PORT_%d%02d DUPLEX", dir, idx );
		KdDoCmdPrint ( path, tmpbuf );
		sscanf ( tmpbuf, "%d", &duplex );

		sprintf ( path, "PORT_%d%02d PRIORITY", dir, idx );
		KdDoCmdPrint ( path, tmpbuf );
		sscanf ( tmpbuf, "%d", &priority );

		new_status.enable = enabled;
		new_status.an = auto_negotiation;
		new_status.speed = speed;
		new_status.duplex = duplex;
		new_status.priority = priority;

		if ( ioctl ( fd, NK_SWITCH_SET_PORT_STATUS, &new_status ) != 0 );
	}

	close ( fd );

	return 0;
}
#else
//int NK_SET_PORT_STATUS(int port, int enabled, int auto_negotiation, int speed, int duplex)
int NK_SET_PORT_STATUS(void)
{
	int fd, i;
	int enabled, auto_negotiation, speed, duplex,priority;
	int support_8021p, trust_mode;
	int support_8021q, output_tag, pvid;
	char    tmp[20];
	char	path[30];
	struct PortStatusNStatistics state;

#ifdef CONFIG_NK_DYNAMIC_PORT_NUM
	DYNAMIC_NUM_LAN = Get_Num_Lan();
	DYNAMIC_NUM_WAN = Get_Num_Wan();
	DYNAMIC_NUM_DMZ = Get_Num_Dmz();
	SWITCH_TYPE = Get_Switch_Type();
#endif

	if ((fd = open("/dev/nk_switch", O_RDONLY)) < 0)
		return -1;

#ifdef CONFIG_NK_DYNAMIC_PORT_NUM
	for ( i = 1; i <= DYNAMIC_NUM_LAN; i++)
#else
	for(i=1; i<=CONFIG_NK_NUM_LAN; i++)
#endif
	{
		sprintf(path,"PORT_%d%02d ENABLED",PORT_TYPE_LAN, i);
		KdDoCmdPrint(path, tmp);
		sscanf(tmp,"%d",&enabled);

		sprintf(path,"PORT_%d%02d AUTO",PORT_TYPE_LAN, i);
		KdDoCmdPrint(path, tmp);
		sscanf(tmp,"%d",&auto_negotiation);

		sprintf(path,"PORT_%d%02d SPEED",PORT_TYPE_LAN, i);
		KdDoCmdPrint(path, tmp);
		sscanf(tmp,"%d",&speed);

		sprintf(path,"PORT_%d%02d DUPLEX",PORT_TYPE_LAN, i);
		KdDoCmdPrint(path, tmp);
		sscanf(tmp,"%d",&duplex);

		/* Support 802.1p */
		KdDoCmdPrint("8021P ENABLED", tmp);
		sscanf(tmp,"%d",&support_8021p);

		sprintf(path,"PORT_%d%02d TRUST_MODE",PORT_TYPE_LAN, i);
		KdDoCmdPrint(path, tmp);
		sscanf(tmp,"%d",&trust_mode);
		
		sprintf(path,"PORT_%d%02d PRIORITY",PORT_TYPE_LAN, i);
		KdDoCmdPrint(path, tmp);
		sscanf(tmp,"%d",&priority);

		/* Support 802.1q */
		KdDoCmdPrint("8021Q ENABLED", tmp);
		sscanf(tmp,"%d",&support_8021q);

#if defined(PORT_VLAN_GROUP)
		sprintf(path,"PORT_%d%02d OUTPUT_TAGGING",PORT_TYPE_LAN, i);
		KdDoCmdPrint(path, tmp);
		sscanf(tmp,"%d",&output_tag);
#else
		/* we won't using output tagging for all lan port ever */
	#if 0
		sprintf(path,"8021Q OUTPUT_TAGGING");
		KdDoCmdPrint(path, tmp);
		sscanf(tmp,"%d",&output_tag);
	#endif
#endif

		sprintf(path,"PORT_%d%02d PRIVATE_VID",PORT_TYPE_LAN, i);
		KdDoCmdPrint(path, tmp);
		sscanf(tmp,"%d",&pvid);

		/* Basic Port Control */
		state.device_type = LAN_MII;
		state.port = i;
		state.enabled = enabled;
		state.auto_negotiation = auto_negotiation;
		state.speed = speed;
		state.duplex = duplex;

		/* 802.1p */
		state.support_8021p = support_8021p;
		state.trust_mode = trust_mode;
		state.priority = priority;

		/* 802.1q */
		state.support_8021q = support_8021q;
#if defined(PORT_VLAN_GROUP)
		state.output_tag = output_tag;
#endif
		state.pvid = pvid;

		if (ioctl(fd, SWITCH_SET_LAN_PORT_STATUS, &state) != 0) ;
			//return -1;
	}
	close(fd);
	/* Set Phy Status */
	if ((fd = open("/dev/nk_switch", O_RDONLY)) < 0)
		return -1;
#ifdef CONFIG_NK_DYNAMIC_PORT_NUM
		for( i = 1; i <= ( DYNAMIC_NUM_WAN + DYNAMIC_NUM_DMZ ); i++ )
#else
        for(i=1; i<=CONFIG_NK_NUM_WAN+CONFIG_NK_NUM_DMZ; i++)
#endif
	{
		sprintf(path,"PORT_%d%02d ENABLED",PORT_TYPE_WAN, i);
		KdDoCmdPrint(path, tmp);
		sscanf(tmp,"%d",&enabled);

		sprintf(path,"PORT_%d%02d AUTO",PORT_TYPE_WAN, i);
		KdDoCmdPrint(path, tmp);
		sscanf(tmp,"%d",&auto_negotiation);

		sprintf(path,"PORT_%d%02d SPEED",PORT_TYPE_WAN, i);
		KdDoCmdPrint(path, tmp);
		sscanf(tmp,"%d",&speed);

		sprintf(path,"PORT_%d%02d DUPLEX",PORT_TYPE_WAN, i);
		KdDoCmdPrint(path, tmp);
		sscanf(tmp,"%d",&duplex);

		memset(&state, 0, sizeof(struct PortStatusNStatistics));
		state.device_type = WAN_MII;
	#ifdef CONFIG_NK_DYNAMIC_PORT_NUM
		state.port = wanportmap(i, SWITCH_TYPE);
	#else
		state.port = wanportmap(i);
	#endif
		state.enabled = enabled;
		state.auto_negotiation = auto_negotiation;
		state.speed = speed;
		state.duplex = duplex;
/*
		fd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
		if (fd < 0)
			return -1;
*/
		if (ioctl(fd, SWITCH_SET_WAN_PORT_STATUS, &state) != 0) ;
		
	}
        close(fd);
	return 0;
}
#endif

#if defined(CONFIG_NK_SWITCH_ARCH)
int NK_GET_PORT_STATUS(struct PortStatusNStatistics *status) {

    int fd, wanidx=0;
    char dev_name[8];
    struct interface ife;
    nk_switch_port_status_t new_status;

    SWITCH_TYPE = Get_Switch_Type();

    memset ( &new_status, 0, sizeof ( nk_switch_port_status_t ) );
    if ( status->device_type == LAN_MII ) {
        strcpy ( dev_name, "eth0" );
        memset ( &ife, 0, sizeof ( struct interface ) );
        nk_if_statistic_get_safe ( dev_name, &ife );
        new_status.dir = NK_SWITCH_LAN;
    }
    else {
        if ( SWITCH_TYPE == 16 )
            wanidx = fport_map_wan_016 [ status->port - 1 ];
        else if ( SWITCH_TYPE == 42 )
            wanidx = fport_map_wan_042 [ status->port - 1 ];
        else if ( SWITCH_TYPE == 82 )
            wanidx = fport_map_wan_082 [ status->port - 1 ];
        sprintf ( dev_name, "eth%d", wanidx );
        nk_if_statistic_get_safe ( dev_name, &ife );
        new_status.dir = NK_SWITCH_WAN;
    }

    new_status.fport = status->port;

    if ( ( fd = open("/dev/nk_switch", O_RDONLY ) ) < 0 ) {
        console_printf ( "failed open /dev/nk_switch" );
        return -1;
    }

    if ( ioctl ( fd, NK_SWITCH_GET_PORT_STATUS, &new_status ) != 0 )
        console_printf("ioctl error");
    close(fd);

    status->enabled = new_status.enable;
    status->link = new_status.link;
    status->auto_negotiation = new_status.an;
    status->speed = new_status.speed;
    status->duplex = new_status.duplex;
//     status->flow_control = new_status.;
//     status->priority = new_status.;
    status->recv_packet_cnt = ife.stats.rx_packets;
    status->recv_byte_cnt = ife.stats.rx_bytes;
    status->tran_packet_cnt = ife.stats.tx_packets;
    status->tran_byte_cnt = ife.stats.tx_bytes;
    status->collision_cnt = ife.stats.collisions;
    status->error_cnt = ife.stats.rx_errors + ife.stats.tx_errors;

    return 0;
}

#else
int NK_GET_PORT_STATUS(struct PortStatusNStatistics *status)
{
	int fd;
	int auto_negotiation, speed, duplex;
	char    tmp[20];
	char	path[20];

	if(status->device_type == LAN_MII)
	{
		if ((fd = open("/dev/nk_switch", O_RDONLY)) < 0)
		{
			console_printf("failed open /dev/nk_switch");
			return -1;
		}

		if (ioctl(fd, SWITCH_GET_LAN_PORT_STATUS, status) != 0) 
			console_printf("ioctl error");
	}
	else
	{
		if ((fd = open("/dev/nk_switch", O_RDONLY)) < 0)
		{
			console_printf("failed open /dev/nk_switch");
			return -1;
		}

		if (ioctl(fd, SWITCH_GET_WAN_PORT_STATUS, status) != 0) 
			console_printf("ioctl error");
	}
	close(fd);
	return 0;
}
#endif

int NK_SET_TOS_VLAN_PRIORITY(void)
{
	int fd, i;
	int support_8021p, ip_vlan_priority_type, queue_id, queue_priority;
	char    tmp[20];
	char	path[20];
	struct PortStatusNStatistics state;


	if ((fd = open("/dev/adm69xx", O_RDONLY)) < 0)
		return -1;

	/* matrix: we use IP_VLAN_PRIORITY_TYPE to different the Port Priority / CoS(VLAN Priority) / DSCP.
	 * TOS Priority has merge in DSCP, so we won't change in TOS Priority mode ever.
	 */

	/* Allow user setting the priority of CoS/DSCP map to Queue, but the configuration work or not depend on 8021P_ENABLED */
#if 0
	sprintf(path,"8021P ENABLED");
	KdDoCmdPrint(path, tmp);
	sscanf(tmp,"%d",&support_8021p);
	state.support_8021p = support_8021p;

	if(!support_8021p)
	{
		close(fd);
		return -1;
	}
#endif

	sprintf(path,"8021P IP_VLAN_PRIORITY_TYPE");
	KdDoCmdPrint(path, tmp);
	sscanf(tmp,"%d",&ip_vlan_priority_type);
#if 0
	if(ip_vlan_priority_type == _SUPPORT_PORT_PRIORITY)
	{
		close(fd);
		return -1;
	}
	else
		state.trust_mode = ip_vlan_priority_type;
#endif

	if(ip_vlan_priority_type == _SUPPORT_DSCP_PRIORITY)
	{
		for(i = 0; i <= 7; i++)
		{
			sprintf(path,"8021P DSCP_PRIORITY_%d_MAPPING",i);
			KdDoCmdPrint(path, tmp);

			state.dscp_priority_t[i].class_id = i;
			memcpy(state.dscp_priority_t[i].queue_priority, tmp, sizeof(state.dscp_priority_t[i].queue_priority));
		}
		state.trust_mode = _SUPPORT_DSCP_PRIORITY;
	}
	//else if(ip_vlan_priority_type == _SUPPORT_VLAN_PRIORITY)
	else
	{
		for(i = 0; i <= 7; i++)
		{
			sprintf(path,"8021P PRIORITY_%d_QUEUE",i);
			KdDoCmdPrint(path, tmp);
			sscanf(tmp,"%d",&queue_id);

			state.tos_vlan_priority_t[i].queue_id = queue_id;
			state.tos_vlan_priority_t[i].queue_priority = i;
		}
		state.trust_mode = _SUPPORT_VLAN_PRIORITY;
	}


	if (ioctl(fd, SET_TOS_VLAN_PRIORITY, &state) != 0) ;
	close(fd);
	return 0;
}

int NK_SET_PRIORITY_QUEUE_WEIGHT(void)
{
	int fd, i;
	int queue_weight, ip_vlan_priority_type;
	char    tmp[20];
	char	path[20];
	struct PortStatusNStatistics state;


	if ((fd = open("/dev/adm69xx", O_RDONLY)) < 0)
		return -1;

	/* Port Base / Cos(VLAN) / DSCP using the same Queue Weight setting */
#if 0
	sprintf(path,"8021P IP_VLAN_PRIORITY_TYPE");
	KdDoCmdPrint(path, tmp);
	sscanf(tmp,"%d",&ip_vlan_priority_type);
	if(ip_vlan_priority_type == _SUPPORT_PORT_PRIORITY)
	{
		close(fd);
		return -1;
	}
	else
		state.ip_vlan_priority_type = ip_vlan_priority_type;
#endif

	for(i = 1; i < 4; i++)
	{
		sprintf(path,"8021P QUEUE_%d_WEIGHT",i);
		KdDoCmdPrint(path, tmp);
		sscanf(tmp,"%d",&queue_weight);

		state.tos_vlan_priority_t[i].queue_id = i;
		state.tos_vlan_priority_t[i].queue_weight = queue_weight;
	}
	state.device_type = LAN_MII;

	if (ioctl(fd, SET_PRIORITY_QUEUE_WEIGHT, &state) != 0) ;
	close(fd);
	return 0;
}

#if 0
int VLAN_GROUP[4] = {0x0, 0x0, 0x0, 0x0};
int NK_SET_VLAN_GROUP(void)
{
	int fd, i, port_id;
	int support_8021q, output_tag, pvid, tmp_pvid;
	int filter_id, filter_number, vlan_group;
	char    tmp[20];
	char	path[30];

	char buf[256];
	char cmdBuf[30];
	char vlan_id[5];
	struct PortStatusNStatistics state;
	char included_in[2];

	if ((fd = open("/dev/adm69xx", O_RDONLY)) < 0)
		return -1;

#if defined(PORT_VLAN_GROUP)
	for(port_id=1; port_id<=4; port_id++)
	{
		memset(&state, 0, sizeof(struct PortStatusNStatistics));
		sprintf(path,"8021Q ENABLED");
		KdDoCmdPrint(path, tmp);
		sscanf(tmp,"%d",&support_8021q);

		state.support_8021q = support_8021q;

		if(support_8021q)
		{

			sprintf(path,"PORT_%d%02d PRIVATE_VID",PORT_TYPE_LAN, port_id);
			KdDoCmdPrint(path, tmp);
			sscanf(tmp,"%d",&pvid);

			for(i=1; i<=4; i++)
			{
				sprintf(path,"PORT_%d%02d PRIVATE_VID",PORT_TYPE_LAN, i);
				KdDoCmdPrint(path, tmp);
				sscanf(tmp,"%d",&tmp_pvid);

				if(pvid == tmp_pvid)
				{
				 	if(i >= port_id)
						VLAN_GROUP[port_id-1] |= (1 << i);
						//VLAN_GROUP[port_id-1] |= (1 << (i - 1));
					else
					{
						/* we find the pvid has been configed in some VLAN, need clean this port's VLAN member */
						VLAN_GROUP[port_id-1] = 0x0;
						break;
					}
				}
			}

			state.device_type = LAN_MII;
			state.port = port_id;
			state.pvid = pvid;
			state.vlangroup.vlanid = pvid;
			state.vlangroup.member_port = VLAN_GROUP[port_id-1];

			if (ioctl(fd, SET_ADM69XX_VLAN_GROUP, &state) != 0) ;
		}
	}
#else

	KdDoCmdPrint("8021Q ENABLED", tmp);
	sscanf(tmp,"%d",&support_8021q);

	KdDoCmdPrint("8021Q NUMBER", tmp);
	sscanf(tmp,"%d",&filter_number);

	for(filter_id=1; filter_id<=MAX_VLAN_FILTER_RULE_NUMBER; filter_id++)
	{
		memset(&state, 0, sizeof(struct PortStatusNStatistics));

		/* If VLAN Enabled we using configuration to setting all vlan filter rules, 
		 * else we need to clean up the setting in switch reg of vlan reules.
		 */
		if(support_8021q)
		{
			if(filter_id<=filter_number)
			{
				sprintf(cmdBuf,"8021Q ID %d", filter_id);
				KdDoCmdPrint(cmdBuf, buf);
				name_get_value(buf, "VLANID", vlan_id, sizeof(vlan_id), NULL);
				sscanf(vlan_id, "%d", &pvid);

				for(port_id=1; port_id<=4; port_id++)
				{
					sprintf(path,"PORT%d", port_id);
					/* Setting VLAN member */
					name_get_value(buf, path, included_in, sizeof(included_in), NULL);
					if(strcmp(included_in, _PORT_EXCLUDE_VLAN))
						state.vlangroup.member_port |= (1 << port_id);
					else
						state.vlangroup.member_port &= ~(1 << port_id);
					/* Setting Tag member */
					if(strcmp(included_in, _PORT_UNTAG_VLAN) && strcmp(included_in, _PORT_EXCLUDE_VLAN))
						state.vlangroup.tag_member |= (1 << port_id);
					else
						state.vlangroup.tag_member &= ~(1 << port_id);
				}

				state.vlangroup.vlanid = pvid;
			}
		}

		state.support_8021q = support_8021q;
		state.device_type = LAN_MII;
		state.vlangroup.filter_id = filter_id;

		if (ioctl(fd, SET_ADM69XX_VLAN_GROUP, &state) != 0) ;
	}
#endif
	close(fd);

	/* matrix: Set VLAN ID in dev->vlan_id */
	KdDoCmdPrint("8021Q NUMBER", tmp);
	sscanf(tmp,"%d",&filter_number);

	fd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
	if (fd < 0)
		return -1;

	for(filter_id=1; filter_id<=MAX_VLAN_FILTER_RULE_NUMBER; filter_id++)
	{
		memset(&state, 0, sizeof(struct PortStatusNStatistics));
		if(support_8021q)
		{
			if(filter_id<=filter_number)
			{
				sprintf(cmdBuf,"8021Q ID %d", filter_id);
				KdDoCmdPrint(cmdBuf, buf);
				name_get_value(buf, "VLANID", vlan_id, sizeof(vlan_id), NULL);
				sscanf(vlan_id, "%d", &pvid);
				state.vlangroup.vlanid = pvid;
			}
			else
				state.vlangroup.vlanid = 0x1;
		}
		else
			state.vlangroup.vlanid = 0x1;

		state.device_type = WAN_MII;
		state.support_8021q = support_8021q;
		state.vlangroup.filter_id = filter_id;
		state.vlangroup.filter_number = filter_number;
		if (ioctl(fd, SET_NETDEVICE_VID, &state) < 0) ;
	}
	close(fd);
	return 0;
}
#endif

#if defined(CONFIG_NK_SWITCH_ARCH)
#define VLAN_ALL_VID    1
uint32_t nk_get_port_vgroup ( uint32_t dir, int port_id ) {

    int vgroup=0;
    uint32_t vid;
    char path[32], tmpbuf[32];

    sprintf ( path, "PORT_%d%02d VLANGROUP", dir, port_id );
    KdDoCmdPrint ( path, tmpbuf );
    sscanf ( tmpbuf, "%d", &vgroup );

    return vgroup;
}

/**
 * Function Name:
 *      nk_get_port_vid
 * Author:
 *      Incifer 2010/00
 * Description:
 *      Read VLAN Group to Decide VID
 * Input:
 *      dir: LAN or WAN
 *      port_id: LAN: [1:NUM_LAN], WAN: [1:NUM_WAN]
 * Output:
 *      N/A
 * Return:
 *      The VID of The [dir, port_id]
 * Note:
 *      1. VLAN Group 77, VID is 1,
 *         The Other VID is VLAN Group + ( 1 + NUM_WAN + NUM_DMZ ).
**/
uint32_t nk_get_port_vid ( uint32_t dir, int port_id ) {

    int vgroup=0;
    uint32_t vid;
    char path[32], tmpbuf[32];

    DYNAMIC_NUM_WAN = Get_Num_Wan();
    DYNAMIC_NUM_DMZ = Get_Num_Dmz();

    sprintf ( path, "PORT_%d%02d VLANGROUP", dir, port_id );
    KdDoCmdPrint ( path, tmpbuf );
    sscanf ( tmpbuf, "%d", &vgroup );

    if ( vgroup == 77 )
        vid = 1;
    else
        vid = vgroup + 1 + DYNAMIC_NUM_WAN + DYNAMIC_NUM_DMZ;
    return vid;
}

/**
 * Function Name:
 *      nk_is_lan
 * Author:
 *      Incifer 2010/04
 * Description:
 *      Read WANTYPE and Return The WANTYPE is LAN or Not.
 * Input:
 *      wanidx: WAN Index, Range: 1 : NUM_WAN + NUM_DMZ, ex: 016[1:8]
 * Output:
 *      N/A
 * Return:
 *      1: WANn is LAN
 *      0: WANn is not LAN
 * Note:
 *      1. DB: WANn WANTYPE
**/
int nk_is_lan ( int wanidx ) {

    char path[32], tmpbuf[32];

    sprintf ( path, "WAN%d WANTYPE", wanidx );
    KdDoCmdPrint ( path, tmpbuf );
    if ( !strcmp ( tmpbuf, "LAN" ) )
        return 1;
    else
        return 0;
}

/**
 * Function Name:
 *      NK_SET_VLAN_GROUP
 * Author:
 *      Incifer 2010/04
 * Description:
 *      To Establish VLAN Table by Database, and IOCTL VLAN Table to Switch Driver.
 *      And Switch Driver Will Set VLAN According the VLAN Table.
 * Input:
 *      N/A
 * Output:
 *      N/A
 * Return:
 *      -1: IOCTL Failed
 *       0: Success
 * Note:
 *      1. VLAN ALL Group is 77, Index is 0, VID is 1.
 *      2. Mirror Port Must be VLAN ALL, And It is at LAN Front Port 1.
 *      3. VLAN Group Range(Adjustable WAN Number) is 1 : ( NUM_LAN + NUM_WAN - 2 ), ex: 016[1:13],
 *         VLAN Group Range(Fixed WAN Number) is 1 : ( NUM_LAN ), ex: 082[1:8].
 *      4. VLAN Table Size, We Use the MAX Size.
 *      5. vlan_all_member saves Who is VLAN ALL,
 *         vtable[0].member saves Who is LAN.
 *      6. member MSB is WAN Member, LSB is LAN Member.
 *      7. vtable[0].port = 99, It Means Not to Set Port VID, Just Set VLAN1 Member
 *      8. If config_wan == 1, It is That LAN and WAN Share the Same Switch.
**/
int NK_SET_VLAN_GROUP(void) {

    int i, j, config_wan=0,fd, vlan_all_member=0, vtable_size=0;
    uint32_t mirror_port=1, vgroup=0, vid=0;
    char path[32], tmpbuf[32];
    nk_switch_vlan_table_t *vtable;
#ifdef CONFIG_NK_DYNAMIC_PORT_NUM
    DYNAMIC_NUM_LAN = Get_Num_Lan();
    DYNAMIC_NUM_WAN = Get_Num_Wan();
    DYNAMIC_NUM_DMZ = Get_Num_Dmz();
    SWITCH_TYPE = Get_Switch_Type();
#endif

    vtable_size = 1/* VLAN ALL */ +
                  DYNAMIC_NUM_WAN + DYNAMIC_NUM_DMZ/* WAN VLAN */ +
                  ( DYNAMIC_NUM_LAN + DYNAMIC_NUM_WAN - 2 )/* LAN VLAN Groups */;
    vtable = malloc ( sizeof ( nk_switch_vlan_table_t ) * vtable_size );
    memset ( vtable, 0, sizeof ( nk_switch_vlan_table_t ) * vtable_size );
    for ( i = 1; i <= vtable_size; i++ )
        vtable[i-1].vid = i;

    if ( SWITCH_TYPE == 16 )
        config_wan = 1;/* Adjustable WAN Number */

    sprintf ( path, "MIRRORP ENABLED" );
    KdDoCmdPrint ( path, tmpbuf );
    if ( !strcmp ( tmpbuf, "YES" ) ) {
        vlan_all_member |= 1 << lanportmap ( mirror_port, SWITCH_TYPE );
    }

    /* Get VLAN ALL Member */
    for ( i = 1; i <= DYNAMIC_NUM_LAN; i++ ) {
        vgroup = nk_get_port_vgroup ( PORT_TYPE_LAN, i );
        if ( vgroup == 77 )
            vlan_all_member |= 1 << lanportmap ( i, SWITCH_TYPE );
        vtable[0].member |= 1 << lanportmap ( i, SWITCH_TYPE );
    }
    if ( config_wan ) {
        vtable[0].port = 99;
        for ( i = 1; i <= ( DYNAMIC_NUM_WAN + DYNAMIC_NUM_DMZ ); i++ ) {
            if ( nk_is_lan ( i ) ) {
                vgroup = nk_get_port_vgroup ( PORT_TYPE_WAN, i );
                if ( vgroup == 77 )
                    vlan_all_member |= ( 1 << ( wanportmap ( i, SWITCH_TYPE ) + DYNAMIC_NUM_LAN ) );
                vtable[0].member |= ( 1 << ( wanportmap ( i, SWITCH_TYPE ) + DYNAMIC_NUM_LAN ) );
            }
        }
    }

    /* Search Group to Setup VLAN Table */
    for ( i = 1; i <= DYNAMIC_NUM_LAN; i++ ) {
        vid = nk_get_port_vid ( PORT_TYPE_LAN, i );
        vtable[vid-1].member |= ( 1 << lanportmap ( i, SWITCH_TYPE ) ) | vlan_all_member;
        vtable[i].port = lanportmap ( i, SWITCH_TYPE );
        vtable[i].index = vid - 1;
    }

    if ( config_wan ) {
        for ( i = 1; i <= ( DYNAMIC_NUM_WAN + DYNAMIC_NUM_DMZ ); i++ ) {
            if ( nk_is_lan ( i ) ) {
                vid = nk_get_port_vid ( PORT_TYPE_WAN, i );
                vtable[vid-1].member |= ( 1 << ( wanportmap ( i, SWITCH_TYPE ) + DYNAMIC_NUM_LAN ) ) | vlan_all_member;
            }
            else {
                vid = i + 1;
                vtable[vid-1].member |= ( 1 << ( wanportmap ( i, SWITCH_TYPE ) + DYNAMIC_NUM_LAN ) );
            }
            vtable[i + DYNAMIC_NUM_LAN ].port = wanportmap ( i, SWITCH_TYPE );
            vtable[i + DYNAMIC_NUM_LAN ].index = vid - 1;

        }
    }

    if ( ( fd = open ( "/dev/nk_switch", O_RDONLY ) ) < 0) {
        console_printf("NK_SET_VLAN_GROUP: failed open /dev/nk_switch");
        return -1;
    }

    if ( ioctl ( fd, SWITCH_SET_LAN_VLAN, vtable ) != 0 )
        console_printf("NK_SET_VLAN_GROUP: ioctl error");

    close ( fd );

    return 0;
}
#else
#define VLAN_ALL_VID	77
#define VLAN_GROUP_NUM	5
unsigned int VLAN_GROUP[VLAN_GROUP_NUM];
unsigned int VLAN_GROUP_ALL = 0X1fe;
int NK_SET_VLAN_GROUP(void)
{
	int port_id, pvid, tmp_pvid, i, fd;
	int mirror_enable=0;
	char path[30], tmp[20];
	struct PortStatusNStatistics state;

#ifdef CONFIG_NK_DYNAMIC_PORT_NUM
	DYNAMIC_NUM_LAN = Get_Num_Lan();
	DYNAMIC_NUM_WAN = Get_Num_Wan();
	DYNAMIC_NUM_DMZ = Get_Num_Dmz();
#endif
	if ((fd = open("/dev/nk_switch", O_RDONLY)) < 0)
	{
		console_printf("NK_SET_VLAN_GROUP: failed open /dev/nk_switch");
		return -1;
	}

	/* init VLAN_GROUP */
	for(i=0;i<VLAN_GROUP_NUM;i++)
		VLAN_GROUP[i] = 0x0;

	/* check mirror port whether enable */
	sprintf(path,"MIRRORP ENABLED");
	KdDoCmdPrint(path, tmp);
	if(!strcmp(tmp, "YES"))
		mirror_enable = 1;

	/* Set VLan of Lan */
#ifdef CONFIG_NK_DYNAMIC_PORT_NUM
	for( port_id = 1; port_id <= DYNAMIC_NUM_LAN; port_id++ )
#else
	for(port_id=1;port_id<=CONFIG_NK_NUM_LAN;port_id++)
#endif
	{
		memset(&state, 0, sizeof(struct PortStatusNStatistics));
		
		sprintf(path,"PORT_%d%02d VLANGROUP",PORT_TYPE_LAN, port_id);
		KdDoCmdPrint(path, tmp);
		sscanf(tmp,"%d",&pvid);

		
		if(mirror_enable == 1 && port_id == 1)
			pvid = VLAN_ALL_VID;

		if(pvid == VLAN_ALL_VID)
		{
			/* pvid(77) means vlan all, config vlan all */
			state.device_type = LAN_MII;
			state.port = port_id;
			state.pvid = VLAN_ALL_VID;
			state.vlangroup.vlanid = VLAN_ALL_VID;
			state.vlangroup.member_port = VLAN_GROUP_ALL;

			if (ioctl(fd, SWITCH_SET_LAN_VLAN, &state) != 0)
				console_printf("NK_SET_VLAN_GROUP: ioctl error");
 		}
		else if(pvid != VLAN_ALL_VID)
		{
		#ifdef CONFIG_NK_DYNAMIC_PORT_NUM
			for( i = 1; i <= DYNAMIC_NUM_LAN; i++ )
		#else
			for(i=1;i<=CONFIG_NK_NUM_LAN;i++)
		#endif
			{
				if(mirror_enable == 1 && i == 1)
					tmp_pvid = VLAN_ALL_VID;
				else
				{
					sprintf(path,"PORT_%d%02d VLANGROUP",PORT_TYPE_LAN, i);
					KdDoCmdPrint(path, tmp);
					sscanf(tmp,"%d",&tmp_pvid);
				}
	
				if(tmp_pvid == pvid || tmp_pvid == VLAN_ALL_VID)
				{
					VLAN_GROUP[pvid - 1] |= (1 << i);
					
				}
			}
	
			if(VLAN_GROUP[pvid - 1])
			{
				state.device_type = LAN_MII;
				state.port = port_id;
			#ifdef CONFIG_NK_DYNAMIC_PORT_NUM
				state.pvid = pvid + (DYNAMIC_NUM_LAN + DYNAMIC_NUM_WAN + CONFIG_NK_NUM_DMZ);
				state.vlangroup.vlanid = pvid + (DYNAMIC_NUM_LAN + DYNAMIC_NUM_WAN + CONFIG_NK_NUM_DMZ);
			#else
				state.pvid = pvid + (CONFIG_NK_NUM_LAN + CONFIG_NK_NUM_WAN + CONFIG_NK_NUM_DMZ);
				state.vlangroup.vlanid = pvid + (CONFIG_NK_NUM_LAN + CONFIG_NK_NUM_WAN + CONFIG_NK_NUM_DMZ);
			#endif
				state.vlangroup.member_port = VLAN_GROUP[pvid - 1];
	
				if (ioctl(fd, SWITCH_SET_LAN_VLAN, &state) != 0)
					console_printf("NK_SET_VLAN_GROUP: ioctl error");
			}
		}
	}
	close(fd);
}
#endif

/* define capture port, default is port 1 */
#define MIRROR_PORT 1
void NK_SET_MIRROR_PORT(void)
{
	int fd, mirror;
	int mirror_enable=0;
	int port;
	char path[30], tmp[20];

	/* check mirror port whether enable */
	sprintf(path,"MIRRORP ENABLED");
	KdDoCmdPrint(path, tmp);
	if(!strcmp(tmp, "YES"))
		mirror_enable = 1;

	if(mirror_enable == 1)
		port = MIRROR_PORT;
	else
		port = 0;

	if ((fd = open("/dev/nk_switch", O_RDONLY)) < 0)
	{
		console_printf("NK_SET_MIRROR_PORT: failed open /dev/nk_switch");
		return;
	}

	if (ioctl(fd, SWITCH_SET_MIRROR_PORT, &port) != 0)
		console_printf("nk_set_mirror_port: ioctl error");

	close(fd);
}

/******************************************
 * NKLinkStatus : To Get PHY LINK STATUS
 * Input : port number
 * Output : True(1/Link) / False(0/UnLink)
 ******************************************/
unsigned int NKLinkStatus(unsigned int port)
{
struct PortStatusNStatistics Port_status;

#ifdef CONFIG_NK_DYNAMIC_PORT_NUM
	SWITCH_TYPE = Get_Switch_Type();
#endif

/* incifer 2008/12 */
#ifdef CONFIG_MODEL_QTM3000
		/* unsigned int port  is interface(WAN1~X) */
		memset(&Port_status, 0, sizeof(struct PortStatusNStatistics));
		Port_status.device_type = WAN_MII;
		//Port_status.port = wanportmap(port);
		Port_status.port = wanportmap ( port, SWITCH_TYPE );

		NK_GET_PORT_STATUS(&Port_status);
		if (Port_status.link)
			return 1;
		else
			return 0;
#endif

#if defined(CONFIG_MODEL_GQF1100) || defined(CONFIG_MODEL_QVM2000)
		memset(&Port_status, 0, sizeof(struct PortStatusNStatistics));
		Port_status.device_type = WAN_MII;
		if (port != 5)
			Port_status.port = 5 - port; //WAN1 : 4, ...., WAN4 : 1
		else if (port > 5)
			Port_status.port = 4;
		else
			Port_status.port = 5;
		NK_GET_PORT_STATUS(&Port_status);
		if (Port_status.link)
			return 1;
		else
			return 0;
#endif
}

#ifdef CONFIG_NK_DYNAMIC_PORT_NUM
/******************************************
 * NKLinkStatus : To Get PHY LINK STATUS
 * Input : port number
 * Output : True(1/Link) / False(0/UnLink)
 ******************************************/
#if defined(CONFIG_NK_SWITCH_ARCH)
unsigned int NKLinkStatusNew(unsigned int wanidx, int switch_type) {

    int fd;
    nk_switch_port_status_t status;

    memset ( &status, 0, sizeof ( nk_switch_port_status_t ) );

    status.dir = NK_SWITCH_WAN;
    status.fport = wanportmap ( wanidx, switch_type);

    if ( ( fd = open("/dev/nk_switch", O_RDONLY ) ) < 0 ) {
        console_printf ( "failed open /dev/nk_switch" );
        return 0;
    }
    if ( ioctl ( fd, NK_SWITCH_GET_LINK_STATUS, &status ) != 0 )
        console_printf("ioctl error");
    close(fd);

    if ( status.link == 1 )
        return 1;
    else
        return 0;
}
#else
unsigned int NKLinkStatusNew(unsigned int port, int switch_type)
{
	struct PortStatusNStatistics Port_status;

	memset(&Port_status, 0, sizeof(struct PortStatusNStatistics));
	Port_status.device_type = WAN_MII;

	Port_status.port = wanportmap(port, switch_type);

	NK_GET_PORT_STATUS(&Port_status);
	if (Port_status.link)
		return 1;
	else
		return 0;
}
#endif
#endif

/**
		Set LAN MAC to Static MAC -- incifer 2009/02
**/
void Set_LAN_Static_MAC(void)
{
		struct ARL_Data arl_data;
		uint64_t tmp64=0;
		int fd;
		char tmp_str[32];

#ifdef CONFIG_NK_DYNAMIC_PORT_NUM
	SWITCH_TYPE = Get_Switch_Type();
#endif
		/* 1. Read LAN MAC */
		kd_doCommand("SYSTEM LANMAC", CMD_PRINT, ASH_DO_NOTHING, tmp_str);

		/* 2. Set struct ARL_Data */
		sscanf(tmp_str, "%llx", &(arl_data.mac_index));
		arl_data.vid_index = 0x1;
		if ( SWITCH_TYPE == 650 || SWITCH_TYPE == 1550 )
		/* GQF650 Use PORT 5, it must sync ethernet.c, getlink.c, sw_template_sw_control.c */
		#ifdef BCM53115_USE_PORT5
			arl_data.data_value = 0x1C005;
		#else
			arl_data.data_value = 0x10008;
		#endif
		else
			arl_data.data_value = 0xE008;
		arl_data.is_lan = 1;

		/* 3. Open IOCTL: SWITCH_WRITE_ARL */
		if((fd=open("dev/nk_switch", O_RDONLY))<0)
		{
			console_printf2("%s: Open dev/nk_switch Fail!!!\n", __func__);
			return;
		}
		if(ioctl(fd, SWITCH_WRITE_ARL, &arl_data)!=0)
			console_printf2("%s: IOCTL: SWITCH_WRITE_ARL Fail!!!\n", __func__);

		close(fd);
		return;
}

/**
	When Ethernet Recieve Src IP==LAN IP, Show it -- incifer 2009/02
**/
void Show_Recieve_Lan(void)
{
	int fd;
	char tmp_str[32];
	uint32_t ip;

	kd_doCommand("SYSTEM LAN", CMD_PRINT, ASH_DO_NOTHING, tmp_str);
	ip = inet_addr(tmp_str);

	if((fd=open("dev/eth_acc", O_RDONLY))<0)
	{
		console_printf2("%s: Open dev/eth_acc Fail!!!\n", __func__);
		return;
	}
	if(ioctl(fd, SHOW_RECIEVE_LAN, &ip)!=0)
		console_printf2("%s: IOCTL: SWITCH_WRITE_ARL Fail!!!\n", __func__);

	close(fd);
	return;
}

/**
	Disable: When Ethernet Recieve Src IP==LAN IP, Show it -- incifer 2009/02
**/
void Show_Recieve_Lan_Disable(void)
{
	int fd;

	if((fd=open("dev/eth_acc", O_RDONLY))<0)
	{
		console_printf2("%s: Open dev/eth_acc Fail!!!\n", __func__);
		return;
	}
	if(ioctl(fd, SHOW_RECIEVE_LAN_DISABLE)!=0)
		console_printf2("%s: IOCTL: SHOW_RECIEVE_LAN_DISABLE Fail!!!\n", __func__);

	close(fd);
	return;
}

void NK_SET_LED_WANLAN ( void ) {

    int fd, i;
    uint32_t status=0;
    char path[32], tmpbuf[32];

#ifdef CONFIG_NK_DYNAMIC_PORT_NUM
    DYNAMIC_NUM_WAN = Get_Num_Wan();
    DYNAMIC_NUM_DMZ = Get_Num_Dmz();
    SWITCH_TYPE = Get_Switch_Type();
#endif
    /* purpose     : switch_rtl8329_driver    author : incifer    date : 2010-05-14 */
    /* description : set wan<->lan led                                              */
    /*               - just support 016                                             */
    if ( SWITCH_TYPE != 16 )
        return;

    for ( i = 1 ; i <= ( DYNAMIC_NUM_WAN + DYNAMIC_NUM_DMZ ); i++ ) {
        sprintf ( path, "WAN%d WANTYPE", i );
        KdDoCmdPrint ( path, tmpbuf );
        if ( !strcmp ( tmpbuf, "LAN" ) )
            status |= 1 << ( i - 1 );
    }

    if ( ( fd = open ( "/dev/nk_switch", O_RDONLY ) ) < 0 )
        return;

    if ( ioctl ( fd, NK_SWITCH_SET_LED_WANLAN, &status ) != 0 )
        console_printf("NK_SWITCH_SET_LED_WANLAN: ioctl error");

    close(fd);
}
