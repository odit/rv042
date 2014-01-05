
#include <features.h>
#include <pwd.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/sysinfo.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <errno.h>
#include <ctype.h>

#include <netinet/if_ether.h>
#include <netinet/ip.h>

#include "sysconfig.h"
#include "nkutil.h"
//#include "nkdef.h"
#include "cvmx.h"	/* for using cvmx_get_cycle() */

#ifndef DBG
//#define DBG
#endif

//2008/12/18 trenchen : send arp API------->
int GetInfIpHaddr( char *inf, struct sockaddr_in *myip, u_char *myhwaddr)
{
	struct ifreq ifr;
	int sock;

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if( sock < 0 ){
		printf("GetInfIpHaddr: open sock error\n");
		return -1;
	}
	
	strcpy( ifr.ifr_name, inf);
	
	if( ioctl(sock, SIOCGIFADDR, &ifr) < 0 ) {
		printf("GetInfIpHaddr: get ip addr error\n");
		return -2;
	}
	myip->sin_addr = ((struct sockaddr_in *)(&ifr.ifr_addr))->sin_addr;
	printf("get inf[%s] ip[%s]\n",inf,inet_ntoa(myip->sin_addr));

	if( ioctl(sock, SIOCGIFHWADDR, &ifr) < 0 ) {
		printf("GetInfIpHaddr: get hw addr error\n");
		return -3;
	}
	memcpy(myhwaddr, ifr.ifr_hwaddr.sa_data, 6);
	printf("get inf[%s] hw[%02x:%02x:%02x:%02x:%02x:%02x]\n",inf,myhwaddr[0],myhwaddr[1],myhwaddr[2],myhwaddr[3],myhwaddr[4],myhwaddr[5]);

	close(sock);
	return 0;
}

int send_arp( char *inf, int dip)
{
	struct sockaddr_in myip;
	u_char myhwaddr[6];
	int ret;
	int sock;
	struct ethhdr *eth_hdr;
	struct ether_arp *arp;
	struct sockaddr sa;
	unsigned int pack_len;
	u_char *arp_packet;
	int count = 0;
	
	ret = GetInfIpHaddr( inf, &myip, myhwaddr);
	if( ret < 0 ){
		printf("send_arp: get ip hw addr error\n");
		return -1;
	}

	sock = socket(AF_INET, SOCK_PACKET, htons(0x0003));
	if( sock < 0 ){
		printf("send_arp: open sock error\n");
		return -2;
	}


	pack_len = sizeof(struct ethhdr)+sizeof(struct ether_arp)+18;
	arp_packet =(u_char *) malloc(pack_len);
	eth_hdr = (struct ethhdr *)arp_packet;
	arp = (struct ether_arp *)(arp_packet+sizeof(struct ethhdr));

	memset(eth_hdr->h_dest, 0xff,sizeof(eth_hdr->h_dest));
	memcpy(eth_hdr->h_source, myhwaddr,sizeof(eth_hdr->h_source));
	eth_hdr->h_proto = htons(ETHERTYPE_ARP);

	arp->arp_hrd = htons(ARPHRD_ETHER);
	arp->arp_pro = htons(0x0800);
	arp->arp_hln = 6;
	arp->arp_pln = 4;
	arp->arp_op = htons(ARPOP_REQUEST);

	memcpy(arp->arp_sha, myhwaddr,sizeof(arp->arp_sha));
	memcpy(arp->arp_spa,(u_char *) (&myip.sin_addr), 4);

	memset(arp->arp_tha, 0, sizeof(arp->arp_tha));
	memcpy(arp->arp_tpa, (u_char *)&dip, 4);

	//strcpy(sa.sa_data, inf);
	for(count=0; count < 14; count++){
		if( inf[count] == ':' || inf[count] == 0 ){
			sa.sa_data[count] = 0;
			break;
		}
		sa.sa_data[count] = inf[count];
	}

	sendto(sock,arp_packet,pack_len,0,&sa,sizeof(sa));

	free(arp_packet);
	close(sock);
	return 0;

}
//<--------

static int scriptSock;
//static struct sockaddr_in scriptc_addr;

// When system time change, the lease table will error, so we use system uptime
time_t	// long int
get_time(time_t *t){
#ifdef USE_UPTIME
        struct sysinfo info;

        sysinfo(&info);

        return info.uptime;
#else
        return time(0);
#endif
}

/*
 * for MAC address, 0 need to display 00, 1 need to display 01
 */
static void
setString(char * string, char * mac)
{
  char buf[3];
  int i;

  string[0] = '\0';

  for (i=0; i<6; i++) {
    if ((mac[i] >= '0') && (mac[i] <= 0x0F)) {
         buf[0] = '0';
         sprintf(&buf[1],"%1X:",mac[i]&0xFF);
    } else {
             sprintf(buf,"%02X:",mac[i]&0xFF);
    }
    strcat(string, buf);
  }
  string[strlen(string)-1] = '\0';
}

/*
 * mac0 is supposed to be WAN MAC
 */
void kd_getMACs(int skfd, char * mac0, char * mac1, int doSetstring)
{
    struct ifreq ifr;
    int i;
    char mac[7];
    int skfdcreate;

    skfdcreate = 0;
    if (skfd == 0) {
      skfd = socket(AF_INET, SOCK_DGRAM, 0);
      if (skfd <= 0) {
	kd_Log("getMACs create socket fail = %d",errno);
	return ;
      }
      skfdcreate = 1;
    }

    if (mac0 != (char *) NULL) {
      strcpy(ifr.ifr_name, "eth0");
      //Log("getMACs:skfd=%d",skfd);
      if ((i=ioctl(skfd, SIOCGIFHWADDR, &ifr)) >= 0) {
	if (doSetstring) {
	  memcpy((void *) mac, (void *) ifr.ifr_hwaddr.sa_data, 6);
	  setString(mac0, mac);
	} else
	  memcpy((void *) mac0, (void *) ifr.ifr_hwaddr.sa_data, 6);
	//Log("doMACs:e0Mac=%s",eth0);
      }
    }

    if (mac1 != (char *) NULL) {
      //Log("getMACs:skfd=%d i=%d errno=%d",skfd,i,errno);
      strcpy(ifr.ifr_name, "eth1");
      if (ioctl(skfd, SIOCGIFHWADDR, &ifr) >= 0) {
	if (doSetstring) {
	  memcpy((void *) mac, (void *) ifr.ifr_hwaddr.sa_data, 6);
	  setString(mac1, mac);
	} else
	  memcpy((void *) mac1, (void *) ifr.ifr_hwaddr.sa_data, 6);
	//Log("doMACs:e1Mac=%s",eth1);
      }
    }

    if (skfdcreate)
      close(skfd);
}

static
int hex2Int(char hex)
{
  int val;

  if (isdigit(hex))
    return (int) (hex - '0');
  val = tolower(hex);
  return val - 'a' + 10;
}

/*
 * buf comes with a hex string
 */
int kd_hexString2Int(char * buf)
{
  if (strlen(buf) == 1) 
    return hex2Int(*buf);
  return hex2Int(*buf) * 16 + hex2Int(*(buf+1));
}

/*
 * mac0 is supposed to be WAN MAC
 */
int kd_setMACs(char * mac, char * ethr)
{
    struct ifreq ifr;
    int i, skfd;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (skfd <= 0) {
      kd_Log("setMACs create socket fail = %d",errno);
      return -1;
    }

    strcpy(ifr.ifr_name, ethr);
    memcpy((void *) ifr.ifr_hwaddr.sa_data, (void *) mac, 6);
	ifr.ifr_hwaddr.sa_family = ARPHRD_ETHER;
    i = ioctl(skfd, SIOCSIFHWADDR, &ifr);
    close(skfd);
    kd_Log("%s mac ioctl=%d %d",ethr, i, errno);
    return i;
}

int kd_setMTUs(int mtu1, char * ethr)
{
    struct ifreq ifr;
    int i, skfd;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (skfd <= 0) {
      kd_Log("kd_setMTU create socket fail = %d",errno);
      return -1;
    }
		
    strcpy(ifr.ifr_name, ethr);
	ifr.ifr_mtu =  mtu1;
    i = ioctl(skfd, SIOCSIFMTU, &ifr);
    close(skfd);
    kd_Log("%s mtu ioctl=%d %d",ethr, i, errno);
    return i;
}

/*
 * some page like DHCP, FILTER, PForward, Dmz
 * need to wait for the last command in order 
 * exec ASH i.e. goFire and goDhcp
 */
int
kd_doCommand(char * parm2, int cmdType, int doASH, char * printBuf)
{
	char argv[3][ARGV_SIZE];
	char dataBuf[ARGV_SIZE];
	int rtn, i;
	char * pbuf;
	struct sockaddr_in peer_addr; /* connector's address information */
	ScriptDo scriptParm;
//	struct sockaddr_in their_addr; /* connector's address information */
//	int sin_size;

	if ((parm2 != (char *) NULL) && (cmdType != (int) NULL))
	{
		dataBuf[0] = '\0';
		pbuf = dataBuf;
		switch (cmdType) 
		{
			case CMD_WRITE:
				sprintf(argv[1], "-w");
				break;

			case CMD_DELETE:
				sprintf(argv[1], "-d");
				break;

			case CMD_PRINT:
				sprintf(argv[1], "-p");
				pbuf = printBuf;
			break;

			case CMD_NEW:
				sprintf(argv[1], "-n");
				pbuf = printBuf;
				break;

			case CMD_BACKUP:
				sprintf(argv[1], "-b");
				pbuf = printBuf;
				break;

			case CMD_TASK_DB:
				sprintf(argv[1], "-t");
				pbuf = printBuf;
				break;

			case CMD_MODIFY:
				sprintf(argv[1], "-m");
				pbuf = printBuf;
				break;

			case CMD_PRINT_NO_FLAG:
				sprintf(argv[1], "-a");
				pbuf = printBuf;
				break;
			
			default:
				return 1;
		}

		if (strlen(parm2) >= ARGV_SIZE) 
		{
			kd_Log("doCommand:Too Big:%s",parm2);
			return 0;
		}

//    strcpy(argv[0], "sysconfig");//TT to know program name in sysconfig()
		strcpy(argv[2], parm2);
#ifdef DBG
		if (cmdType != CMD_PRINT)
			kd_Log("sysconfig %s %s",argv[1],argv[2]);
#endif
		rtn = nk_sysconfig(3, argv, pbuf);
#if 0
		if (cmdType == CMD_PRINT)
			kd_Log("rtn=%d buf=%s",rtn,pbuf);
#endif

		// no need to run scrip if only CMD_DELETE
		if (cmdType != CMD_WRITE)
			return rtn;
	}

	if (doASH == ASH_DO_NOTHING)
		return 0;

//	printf("doCommand doASH=%d \n",doASH);
//TT	if (scriptSock == 0) 
//only act as client (mark server part), & close socket each time finished
//due to fork problem, like DHCPC use kd_docommand send message to webBoot,
//it will result socket bind error 
	{
		if ((scriptSock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) 
		{
			kd_Log("scriptSock: socket err");
			return -1;
		}
		i = 1;
		setsockopt(scriptSock, SOL_SOCKET, SO_REUSEADDR, (void*)&i, i);

#if 0 //TT (mark server part)
		scriptc_addr.sin_family = AF_INET;         /* host byte order */
		scriptc_addr.sin_port = htons(SCRIPT_C_PORT);     /* short, network byte order */
	    //    scriptc_addr.sin_addr.s_addr = INADDR_ANY; /* auto-fill with my IP */
		scriptc_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); /* auto-fill with my IP */
		bzero(&(scriptc_addr.sin_zero), 8);        /* zero the rest of the struct */

		if (bind(scriptSock, (struct sockaddr *)&scriptc_addr, sizeof(struct sockaddr)) == -1) 
		{
			kd_Log("scriptc:bind err");
			return -1;
		}
#endif 
	}
	
	peer_addr.sin_family = AF_INET;         /* host byte order */
	peer_addr.sin_port = htons(SCRIPT_PORT);     /* short, network byte order */
	peer_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
	bzero(&(peer_addr.sin_zero), 8);        /* zero the rest of the struct */

	scriptParm.doASH = doASH;
	if (parm2 != (char *) NULL) 
	{
		scriptParm.cmd =  *(parm2+1);
	    strcpy (scriptParm.mesg, parm2);
	}
	else
	{
		scriptParm.cmd = 'x';
		strcpy (scriptParm.mesg, "EMPTY");
//		scriptParm.mesg = NULL;
	}

	kd_Log("doCommand doASH=%d parm=%c %s ",scriptParm.doASH,scriptParm.cmd, scriptParm.mesg);
	
	i = sendto(scriptSock, (char *) &scriptParm, sizeof(ScriptDo), 0,
		(struct sockaddr *) &peer_addr, sizeof(struct sockaddr));
	if (i < 0)
		kd_Log("doCommand sendto %d %d",i, errno);
	

#if 0	//TT (mark server part)
	//  kd_Log("doCommand: recvfrom");
	i = recvfrom(scriptSock, &scriptParm, sizeof(ScriptDo), 0,
		(struct sockaddr *)&their_addr, &sin_size);
	if (i <= 0) 
	{
		kd_Log("doCommand recvfrom fail %d",errno);
		return i;
	}
#endif
	//  kd_Log("doCommand recvfrom status=%d",scriptParm.status);
	close (scriptSock); //TT close socket each time finished
//	return scriptParm.status;
	return 0;  // edit by johnli, if we mark server part, do not return status
}

#ifdef CONFIG_NK_LICENSE_COUNT_DOWN
int
kd_doCommand_factory(char * parm2, int cmdType, int doASH, char * printBuf)
{
	char argv[3][ARGV_SIZE];
	char dataBuf[ARGV_SIZE];
	int rtn, i;
	char * pbuf;
	struct sockaddr_in peer_addr; /* connector's address information */
	ScriptDo scriptParm;
	if ((parm2 != (char *) NULL) && (cmdType != (int) NULL))
	{
		dataBuf[0] = '\0';
		pbuf = dataBuf;
		switch (cmdType) 
		{
			case CMD_WRITE:
				sprintf(argv[1], "-w");
				break;

			case CMD_DELETE:
				sprintf(argv[1], "-d");
				break;

			case CMD_PRINT:
				sprintf(argv[1], "-p");
				pbuf = printBuf;
				break;

			case CMD_NEW:
				sprintf(argv[1], "-n");
				pbuf = printBuf;
				break;

			case CMD_BACKUP:
				sprintf(argv[1], "-b");
				pbuf = printBuf;
				break;

			case CMD_TASK_DB:
				sprintf(argv[1], "-t");
				pbuf = printBuf;
				break;

			case CMD_MODIFY:
				sprintf(argv[1], "-m");
				pbuf = printBuf;
				break;

			case CMD_PRINT_NO_FLAG:
				sprintf(argv[1], "-a");
				pbuf = printBuf;
				break;
			
			default:
				return 1;
		}

		if (strlen(parm2) >= ARGV_SIZE) 
		{
			kd_Log("doCommand_factory:Too Big:%s",parm2);
			return 0;
		}

//    strcpy(argv[0], "sysconfig");//TT to know program name in sysconfig()
		strcpy(argv[2], parm2);
#ifdef DBG
		if (cmdType != CMD_PRINT)
			kd_Log("sysconfig %s %s",argv[1],argv[2]);
#endif
		rtn = nk_sysconfig_factory(3, argv, pbuf);
#if 0
		if (cmdType == CMD_PRINT)
			kd_Log("rtn=%d buf=%s",rtn,pbuf);
#endif

		// no need to run scrip if only CMD_DELETE
		if (cmdType != CMD_WRITE)
			return rtn;
	}

	if (doASH == ASH_DO_NOTHING)
		return 0;

//	printf("doCommand doASH=%d \n",doASH);
//TT	if (scriptSock == 0) 
//only act as client (mark server part), & close socket each time finished
//due to fork problem, like DHCPC use kd_docommand send message to webBoot,
//it will result socket bind error 
	{
		if ((scriptSock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) 
		{
			kd_Log("scriptSock: socket err");
			return -1;
		}
		i = 1;
		setsockopt(scriptSock, SOL_SOCKET, SO_REUSEADDR, (void*)&i, i);

#if 0 //TT (mark server part)
		scriptc_addr.sin_family = AF_INET;         /* host byte order */
		scriptc_addr.sin_port = htons(SCRIPT_C_PORT);     /* short, network byte order */
	    //    scriptc_addr.sin_addr.s_addr = INADDR_ANY; /* auto-fill with my IP */
		scriptc_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); /* auto-fill with my IP */
		bzero(&(scriptc_addr.sin_zero), 8);        /* zero the rest of the struct */

		if (bind(scriptSock, (struct sockaddr *)&scriptc_addr, sizeof(struct sockaddr)) == -1) 
		{
			kd_Log("scriptc:bind err");
			return -1;
		}
#endif 
	}
	
	peer_addr.sin_family = AF_INET;         /* host byte order */
	peer_addr.sin_port = htons(SCRIPT_PORT);     /* short, network byte order */
	peer_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
	bzero(&(peer_addr.sin_zero), 8);        /* zero the rest of the struct */

	scriptParm.doASH = doASH;
	if (parm2 != (char *) NULL) 
	{
		scriptParm.cmd =  *(parm2+1);
	    strcpy (scriptParm.mesg, parm2);
	}
	else
	{
		scriptParm.cmd = 'x';
		strcpy (scriptParm.mesg, "EMPTY");
//		scriptParm.mesg = NULL;
	}

	kd_Log("doCommand_factory doASH=%d parm=%c %s ",scriptParm.doASH,scriptParm.cmd, scriptParm.mesg);
	
	i = sendto(scriptSock, (char *) &scriptParm, sizeof(ScriptDo), 0,
		(struct sockaddr *) &peer_addr, sizeof(struct sockaddr));
	if (i < 0)
		kd_Log("doCommand_factory sendto %d %d",i, errno);
	

#if 0	//TT (mark server part)
	//  kd_Log("doCommand: recvfrom");
	i = recvfrom(scriptSock, &scriptParm, sizeof(ScriptDo), 0,
		(struct sockaddr *)&their_addr, &sin_size);
	if (i <= 0) 
	{
		kd_Log("doCommand recvfrom fail %d",errno);
		return i;
	}
#endif
	//  kd_Log("doCommand recvfrom status=%d",scriptParm.status);
	close (scriptSock); //TT close socket each time finished
//	return scriptParm.status;
	return 0;  // edit by johnli, if we mark server part, do not return status
}
#endif

/* 
 * Concatenates NULL-terminated list of arguments into a single
 * commmand and executes it
 * @param	argv	argument list
 * @param	timeout	seconds to wait before timing out or 0 for no timeout
 * @param	ppid	NULL to wait for child termination or pointer to pid
 * @return	return value of executed command or errno
 */
int
eval(char *const argv[], int timeout, int *ppid)
{ 
	pid_t pid;
	int status;
	char buf[254]="";
	int i;

	switch (pid = fork()) {
	case -1:	/* error */
		return -1;
	case 0:		/* child */
		/* execute command */
//		for(i=0 ; argv[i] ; i++) 
//			snprintf(buf+strlen(buf), sizeof(buf), "%s ", argv[i]);
//		setenv("PATH", "/sbin:/bin:/usr/sbin:/usr/bin", 1);
		alarm(timeout);
		execvp(argv[0], argv);
		exit(1);
	default:	/* parent */
		if (ppid) 
		{
			*ppid = pid;
			return 0;
		} else {
			waitpid(pid, &status, 0);
			if (WIFEXITED(status))
				return WEXITSTATUS(status);
			else
				return status;
		}
	}
}



void kd_Log(char *format, ...)
{
  FILE *logfile;
  time_t t;
  struct tm *tm;
  char temp[1500], logfilename[80];
  char datetime[] = "[%d.%m.%Y] [%H:%M.%S]";
  char datetime_final[128];
  va_list ap;
  struct stat sbuf;

  va_start(ap, format);		// format it all into temp
  vsprintf(temp, format, ap);
  va_end(ap);

  strcat(temp, "\r\n");
  time (&t);
  tm = localtime(&t);
  memset(datetime_final, 0, 128);
  strftime(datetime_final, 127, datetime, tm);
	
  // format it all so we have date/time/loginfo
  //  sprintf(temp2, "%s - %s\r\n", datetime_final, temp);
  sprintf(logfilename, "%s/web.log", DATABASE_PATH);

  
  if ((stat(logfilename, &sbuf) == 0) && (sbuf.st_size > 0x80000))
      unlink(logfilename);
  if((logfile = fopen(logfilename, "a"))==NULL)
    return;

  fputs(datetime_final, logfile);
  fputs(" ", logfile);
  fputs(temp, logfile);		// Save to the file
		
  fclose(logfile);		// Close file
}

#ifdef CONFIG_NK_SSL_UPGRADE
typedef unsigned long long u64_t;
u64_t u64_pow(u64_t x, int y)
{
	int i=0;
	u64_t tmp=x;
	
	for (i=2 ; i<=y ; ++i )
		tmp = tmp * x;
	
	return tmp;
}
u64_t u64_atoi(char *sn)
{
	int i=0;
	char tmp[2];
	
	u64_t j=1, total=0;
	
	for ( i=strlen(sn), j=1 ; i>0 ; --i, j=j*10 )
	{
		sprintf(tmp, "%c", sn[i-1]);
		total = total + (atoi(tmp)*j);
	}
	return total;
}
u64_t nk_hash(u64_t old_ssl, u64_t new_ssl, char *router_sn)
{
	u64_t hv=0;
	
	hv = u64_pow(old_ssl, 3)*(u64_t)3413759 + 
		u64_pow(new_ssl, 2)*(u64_t)5979131 + 
		u64_pow(u64_atoi(router_sn), 4)*(u64_t)5391771 + (u64_t)5197371;
	
	return hv;
}
// change hash value to serial number(16 digit)
char *nk_hv2sno(u64_t hv)
{
	//char sn[17];	//for seria number
	char *sn;
	char tmp[3];
	int i=0, k=0;
	
	sn = (char*)malloc(sizeof(char)*17);
	
	sn[0] = tmp[0] = '\0';
	
	for ( i=56 ; i>=0 ; i=i-8 )
	{
		k = (int)((hv>>i)&0xff);
		if ( k == 0 )
			sprintf(tmp, "%s", "00" );
		else
		{
			sprintf(tmp, "%x", k );
			if ( strlen(tmp) == 1 )
				sprintf(tmp, "0%x", k);
		}
		strcat(sn, tmp);
		tmp[0]='\0';
	}
	
	return sn;
	
}
#endif

/* ------> */

#define OCTEON__FREQ 500
#define MAX_CYCLE_CHANNEL	8		/* Support max 8 channels. You can change it if you need more */
unsigned long int cycle_start[MAX_CYCLE_CHANNEL];
unsigned long int cycle_end[MAX_CYCLE_CHANNEL];
unsigned long int cycle_delta[MAX_CYCLE_CHANNEL];
unsigned long int cycle_time_ms[MAX_CYCLE_CHANNEL];


/* start_get_cycle() -- get current CPU cycle and save it into global variable cycle_start[channel] 
   Input: channel	= 0..7	; you can select any one of 8 channels(counters)
   Return: 0 = success; -1 = invalid argument
   Note: You should use this function with stop_get_cycle() together. 
          See the usage example in stop_get_cycle().
*/
int start_get_cycle(char channel)
{
	if(channel >= MAX_CYCLE_CHANNEL)
		return -1;
	cycle_start[channel] = cvmx_get_cycle();
	return 0;
}
	
/* stop_get_cycle() -- get current CPU cycle,  save it into globol variable cycle_end[channel],
			calculate the delta and save it to cycle_delta[channel]
			print result to console if necessary.
   Input:
	channel	= 0..7		; you can select any one of 8 channels(counters). 
			        ; It should be the same as the one which the start_get_cycle() specified.
	print	= 1 		; print results to console
		= other 	; do not print result
   Return: 
	cycle_delta[channel]	; valid 
	-1 			; invalid argument
	The results are also saved to global arrays cycle_start[], cycle_end[], ...
	   

   Usage example:
	To measure and print the computing cycle of the functoin Original_func()

	extern	unsigned long int cycle_time_ms[];
	char tmp_buf[1024];

	start_get_cycle(0); 	-- get the 1st cycle, using channel 0
	...Original_func(); 	-- original code we want to measure it's computing cycle
	stop_get_cycle(0, 1); 	-- get the 2nd cycle, print the delta to console
	(below code is optional, it just teaches you how to get the results from global variables)
	sprintf(tmp_buf, "We got a global variable cycle_time_ms[0]=[%ld]\n", cycle_delta[0]);
	con_printf(tmp_buf);

*/
unsigned long int stop_get_cycle(char channel, int print)
{
	unsigned long int time_ms, time_us;
	if(channel >= MAX_CYCLE_CHANNEL)
		return -1;
	cycle_end[channel] = cvmx_get_cycle();
	cycle_delta[channel] = cycle_end[channel]-cycle_start[channel];
	time_us = ( (cycle_delta[channel]) / (OCTEON__FREQ) );
	time_ms = time_us / 1000;
	//time_ms = (( cycle_delta[channel]/1000) / (OCTEON__FREQ));
	cycle_time_ms[channel] = time_ms;
	if(print == 1)
	{
		printf("....(%d) It took [%ld]-[%ld]=[%ld] CPU cycles. (=[%ld.%03ld] ms)\r\n", channel, cycle_end[channel], cycle_start[channel], cycle_delta[channel], time_ms, time_us%1000);
	}
	return cycle_delta[channel];
}

/* <--- */

