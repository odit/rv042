/*
 * dhcpcd - DHCP client daemon -
 * Copyright (C) 1996 - 1997 Yoichi Hariguchi <yoichi@fore.com>
 * Copyright (C) January, 1998 Sergei Viznyuk <sv@phystech.com>
 * 
 * dhcpcd is an RFC2131 and RFC1541 compliant DHCP client daemon.
 *
 * This is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <time.h>
#include <sys/utsname.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <net/if_packet.h>
#include <net/route.h>
#include <netdb.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>
#include <errno.h>
#include <setjmp.h>
#include "client.h"
#include "buildmsg.h"
#include "udpipgen.h"
#include "pathnames.h"

/* update sysconfig and inform webBoot when dhcpConfig done. --> Ryoko 2005/07/04 */
#include "nkutil.h"
#include "nkdef.h"
void nk_dhcp_dbconfig(int flag);
void nk_dhcp_dbconfig_clear(void);
// <--
//Ryoko 2005/08/26 Save InterFace
extern  char		iFace[20];
//<--
#ifndef SOCK_PACKET
#define SOCK_PACKET       10
#endif

extern	char		*ProgramName,**ProgramEnviron,*Cfilename;
extern	char		*IfName;
extern	int		IfName_len;
extern	char		*HostName;
extern	unsigned char	*ClassID;
extern	int		ClassID_len;
extern  unsigned char	*ClientID;
extern  int		ClientID_len;
extern	int		DebugFlag;
extern	int		BeRFC1541;
extern	unsigned	LeaseTime;
extern	int		ReplResolvConf;
extern	int		SetDomainName;
extern	int		SetHostName;
extern	unsigned short	ip_id;
extern  void		*(*currState)();
extern  time_t          TimeOut;
extern  unsigned        nleaseTime;
extern  struct in_addr  inform_ipaddr;
extern	int		DoCheckSum;
extern	int		TestCase;

#ifdef ARPCHECK
int arpCheck();
#endif
int arpRelease();
int arpInform();

int		dhcpSocket;
int             prev_ip_addr;
time_t		ReqSentTime;
dhcpOptions	DhcpOptions;
dhcpInterface   DhcpIface;
udpipMessage	UdpIpMsgSend,UdpIpMsgRecv;
jmp_buf		env;
unsigned char	ClientHwAddr[ETH_ALEN];

const struct ip *ipSend=(struct ip *)((struct udpiphdr *)UdpIpMsgSend.udpipmsg)->ip;
const struct udphdr *udpSend=(struct udphdr *)((struct udpiphdr *)UdpIpMsgSend.udpipmsg)->udp;
const struct ip *ipRecv=(struct ip *)((struct udpiphdr *)UdpIpMsgRecv.udpipmsg)->ip;
const struct udphdr *udpRecv=(struct udphdr *)((struct udpiphdr *)UdpIpMsgRecv.udpipmsg)->udp;
const dhcpMessage *DhcpMsgSend = (dhcpMessage *)&UdpIpMsgSend.udpipmsg[sizeof(udpiphdr)];
      dhcpMessage *DhcpMsgRecv = (dhcpMessage *)&UdpIpMsgRecv.udpipmsg[sizeof(udpiphdr)];
/*****************************************************************************/
int parseDhcpMsgRecv() /* this routine parses dhcp message received */
{
#ifdef DEBUG
  int i,j;
#endif
  register u_char *p = DhcpMsgRecv->options+4;
  unsigned char *end = DhcpMsgRecv->options+sizeof(DhcpMsgRecv->options);
  while ( p < end )
    switch ( *p )
      {
        case endOption: goto swend;
       	case padOption: p++; break;
       	default:
	  if ( p[1] )
	    {
	      if ( DhcpOptions.len[*p] == p[1] )
	        memcpy(DhcpOptions.val[*p],p+2,p[1]);
	      else
	        {
		  DhcpOptions.len[*p] = p[1];
	          if ( DhcpOptions.val[*p] )
	            free(DhcpOptions.val[*p]);
	      	  else
		    DhcpOptions.num++;
	      	  DhcpOptions.val[*p] = malloc(p[1]+1);
		  memset(DhcpOptions.val[*p],0,p[1]+1);
	  	  memcpy(DhcpOptions.val[*p],p+2,p[1]);
	        }
	    }
	  p+=p[1]+2;
      }
swend:
#ifdef DEBUG
  fprintf(stderr,"parseDhcpMsgRecv: %d options received:\n",DhcpOptions.num);
  for (i=1;i<255;i++)
    if ( DhcpOptions.val[i] )
      switch ( i )
        {
	  case 1: /* subnet mask */
	  case 3: /* routers on subnet */
	  case 4: /* time servers */
	  case 5: /* name servers */
	  case 6: /* dns servers */
	  case 28:/* broadcast addr */
	  case 33:/* staticRoute */
	  case 41:/* NIS servers */
	  case 42:/* NTP servers */
	  case 50:/* dhcpRequestdIPaddr */
	  case 54:/* dhcpServerIdentifier */
	    for (j=0;j<DhcpOptions.len[i];j+=4)
	      fprintf(stderr,"i=%-2d  len=%-2d  option = %u.%u.%u.%u\n",
		i,DhcpOptions.len[i],
		((unsigned char *)DhcpOptions.val[i])[0+j],
		((unsigned char *)DhcpOptions.val[i])[1+j],
		((unsigned char *)DhcpOptions.val[i])[2+j],
		((unsigned char *)DhcpOptions.val[i])[3+j]);
	    break;
	  case 2: /* time offset */
	  case 51:/* dhcpAddrLeaseTime */
	  case 57:/* dhcpMaxMsgSize */
	  case 58:/* dhcpT1value */
	  case 59:/* dhcpT2value */
	    fprintf(stderr,"i=%-2d  len=%-2d  option = %d\n",
		i,DhcpOptions.len[i],
		    ntohl(*(int *)DhcpOptions.val[i]));
	    break;
	  case 23:/* defaultIPTTL */
	  case 29:/* performMaskdiscovery */
	  case 31:/* performRouterdiscovery */
	  case 53:/* dhcpMessageType */
	    fprintf(stderr,"i=%-2d  len=%-2d  option = %u\n",
		i,DhcpOptions.len[i],*(unsigned char *)DhcpOptions.val[i]);
	    break;
	  default:
	    fprintf(stderr,"i=%-2d  len=%-2d  option = \"%s\"\n",
		i,DhcpOptions.len[i],(char *)DhcpOptions.val[i]);
	}
fprintf(stderr,"\
DhcpMsgRecv->yiaddr  = %u.%u.%u.%u\n\
DhcpMsgRecv->siaddr  = %u.%u.%u.%u\n\
DhcpMsgRecv->giaddr  = %u.%u.%u.%u\n\
DhcpMsgRecv->sname   = \"%s\"\n\
ServerHardwareAddr   = %02X.%02X.%02X.%02X.%02X.%02X\n",
((unsigned char *)&DhcpMsgRecv->yiaddr)[0],
((unsigned char *)&DhcpMsgRecv->yiaddr)[1],
((unsigned char *)&DhcpMsgRecv->yiaddr)[2],
((unsigned char *)&DhcpMsgRecv->yiaddr)[3],
((unsigned char *)&DhcpMsgRecv->siaddr)[0],
((unsigned char *)&DhcpMsgRecv->siaddr)[1],
((unsigned char *)&DhcpMsgRecv->siaddr)[2],
((unsigned char *)&DhcpMsgRecv->siaddr)[3],
((unsigned char *)&DhcpMsgRecv->giaddr)[0],
((unsigned char *)&DhcpMsgRecv->giaddr)[1],
((unsigned char *)&DhcpMsgRecv->giaddr)[2],
((unsigned char *)&DhcpMsgRecv->giaddr)[3],
DhcpMsgRecv->sname,
UdpIpMsgRecv.ethhdr.ether_shost[0],
UdpIpMsgRecv.ethhdr.ether_shost[1],
UdpIpMsgRecv.ethhdr.ether_shost[2],
UdpIpMsgRecv.ethhdr.ether_shost[3],
UdpIpMsgRecv.ethhdr.ether_shost[4],
UdpIpMsgRecv.ethhdr.ether_shost[5]);
#endif
  if ( ! DhcpMsgRecv->yiaddr ) DhcpMsgRecv->yiaddr=DhcpMsgSend->ciaddr;
  if ( ! DhcpOptions.val[dhcpServerIdentifier] ) /* did not get dhcpServerIdentifier */
    {	/* make it the same as IP address of the sender */
      DhcpOptions.val[dhcpServerIdentifier] = malloc(4);
      memcpy(DhcpOptions.val[dhcpServerIdentifier],&ipRecv->ip_src.s_addr,4);
      DhcpOptions.len[dhcpServerIdentifier] = 4;
      DhcpOptions.num++;
      if ( DebugFlag )
	syslog(LOG_DEBUG,
	"dhcpServerIdentifier option is missing in DHCP server response. Assuming %u.%u.%u.%u\n",
	((unsigned char *)DhcpOptions.val[dhcpServerIdentifier])[0],
	((unsigned char *)DhcpOptions.val[dhcpServerIdentifier])[1],
	((unsigned char *)DhcpOptions.val[dhcpServerIdentifier])[2],
	((unsigned char *)DhcpOptions.val[dhcpServerIdentifier])[3]);
    }
  if ( ! DhcpOptions.val[dns] ) /* did not get DNS */
    {	/* make it the same as dhcpServerIdentifier */
      DhcpOptions.val[dns] = malloc(4);
      memcpy(DhcpOptions.val[dns],DhcpOptions.val[dhcpServerIdentifier],4);
      DhcpOptions.len[dns] = 4;
      DhcpOptions.num++;
      if ( DebugFlag )
	syslog(LOG_DEBUG,
	"dns option is missing in DHCP server response. Assuming %u.%u.%u.%u\n",
	((unsigned char *)DhcpOptions.val[dns])[0],
	((unsigned char *)DhcpOptions.val[dns])[1],
	((unsigned char *)DhcpOptions.val[dns])[2],
	((unsigned char *)DhcpOptions.val[dns])[3]);
    }
  if ( ! DhcpOptions.val[subnetMask] ) /* did not get subnetMask */
    {
      DhcpOptions.val[subnetMask] = malloc(4);
      ((unsigned char *)DhcpOptions.val[subnetMask])[0] = 255;
      if ( ((unsigned char *)&DhcpMsgRecv->yiaddr)[0] < 128 )
        ((unsigned char *)DhcpOptions.val[subnetMask])[1] = 0; /* class A */
      else
	{
          ((unsigned char *)DhcpOptions.val[subnetMask])[1] = 255;
	  if ( ((unsigned char *)&DhcpMsgRecv->yiaddr)[0] < 192 )
	    ((unsigned char *)DhcpOptions.val[subnetMask])[2] = 0;/* class B */
	  else
	    ((unsigned char *)DhcpOptions.val[subnetMask])[2] = 255;/* class C */
	}
      ((unsigned char *)DhcpOptions.val[subnetMask])[3] = 0;
      DhcpOptions.len[subnetMask] = 4;
      DhcpOptions.num++;
      if ( DebugFlag )
	syslog(LOG_DEBUG,
	"subnetMask option is missing in DHCP server response. Assuming %u.%u.%u.%u\n",
	((unsigned char *)DhcpOptions.val[subnetMask])[0],
	((unsigned char *)DhcpOptions.val[subnetMask])[1],
	((unsigned char *)DhcpOptions.val[subnetMask])[2],
	((unsigned char *)DhcpOptions.val[subnetMask])[3]);
    }
  if ( ! DhcpOptions.val[broadcastAddr] ) /* did not get broadcastAddr */
    {
      int br = DhcpMsgRecv->yiaddr | ~*((int *)DhcpOptions.val[subnetMask]);
      DhcpOptions.val[broadcastAddr] = malloc(4);
      memcpy(DhcpOptions.val[broadcastAddr],&br,4);
      DhcpOptions.len[broadcastAddr] = 4;
      DhcpOptions.num++;
      if ( DebugFlag )
	syslog(LOG_DEBUG,
	"broadcastAddr option is missing in DHCP server response. Assuming %u.%u.%u.%u\n",
	((unsigned char *)DhcpOptions.val[broadcastAddr])[0],
	((unsigned char *)DhcpOptions.val[broadcastAddr])[1],
	((unsigned char *)DhcpOptions.val[broadcastAddr])[2],
	((unsigned char *)DhcpOptions.val[broadcastAddr])[3]);
    }
  if ( ! DhcpOptions.val[routersOnSubnet] )
    {
      DhcpOptions.val[routersOnSubnet] = malloc(4);
      if ( DhcpMsgRecv->giaddr )
      	memcpy(DhcpOptions.val[routersOnSubnet],&DhcpMsgRecv->giaddr,4);
      else
	memcpy(DhcpOptions.val[routersOnSubnet],DhcpOptions.val[dhcpServerIdentifier],4);
      DhcpOptions.len[routersOnSubnet] = 4;
      DhcpOptions.num++;
      if ( DebugFlag )
	syslog(LOG_DEBUG,
	"routersOnSubnet option is missing in DHCP server response. Assuming %u.%u.%u.%u\n",
	((unsigned char *)DhcpOptions.val[routersOnSubnet])[0],
	((unsigned char *)DhcpOptions.val[routersOnSubnet])[1],
	((unsigned char *)DhcpOptions.val[routersOnSubnet])[2],
	((unsigned char *)DhcpOptions.val[routersOnSubnet])[3]);
    }
  if ( DhcpOptions.val[dhcpIPaddrLeaseTime] )
    {
      if ( *(unsigned int *)DhcpOptions.val[dhcpIPaddrLeaseTime] == 0 )
	{
          memcpy(DhcpOptions.val[dhcpIPaddrLeaseTime],&nleaseTime,4);
	  if ( DebugFlag )
	    syslog(LOG_DEBUG,
	    "dhcpIPaddrLeaseTime=0 in DHCP server response. Assuming %u sec\n",
	    LeaseTime);
	}
    }
  else /* did not get dhcpIPaddrLeaseTime */
    {
      DhcpOptions.val[dhcpIPaddrLeaseTime] = malloc(4);
      memcpy(DhcpOptions.val[dhcpIPaddrLeaseTime],&nleaseTime,4);
      DhcpOptions.len[dhcpIPaddrLeaseTime] = 4;
      DhcpOptions.num++;
      if ( DebugFlag )
	syslog(LOG_DEBUG,"dhcpIPaddrLeaseTime option is missing in DHCP server response. Assuming %u sec\n",LeaseTime);
    }
  if ( ! DhcpOptions.val[dhcpT1value] ) /* did not get T1 */
    {
      int t1 = htonl((unsigned )(0.5*ntohl(*(unsigned int *)DhcpOptions.val[dhcpIPaddrLeaseTime])));
      DhcpOptions.val[dhcpT1value] = malloc(4);
      memcpy(DhcpOptions.val[dhcpT1value],&t1,4);
      DhcpOptions.len[dhcpT1value] = 4;
      DhcpOptions.num++;
    }
  if ( ! DhcpOptions.val[dhcpT2value] ) /* did not get T2 */
    {
      int t2 =  htonl((unsigned )(0.875*ntohl(*(unsigned int *)DhcpOptions.val[dhcpIPaddrLeaseTime])));
      DhcpOptions.val[dhcpT2value] = malloc(4);
      memcpy(DhcpOptions.val[dhcpT2value],&t2,4);
      DhcpOptions.len[dhcpT2value] = 4;
      DhcpOptions.num++;
    }
  if ( DhcpOptions.val[dhcpMessageType] )
    return *(unsigned char *)DhcpOptions.val[dhcpMessageType];
  return 0;
}
/*****************************************************************************/
void classIDsetup()
{
  struct utsname sname;
  if ( uname(&sname) ) syslog(LOG_INFO,"classIDsetup: uname: %m\n");
  DhcpIface.class_len=sprintf(DhcpIface.class_id,
  "%s %s %s",sname.sysname,sname.release,sname.machine);
}
/*****************************************************************************/
void clientIDsetup()
{
  unsigned char *c = DhcpIface.client_id;
  *c++ = dhcpClientIdentifier;
  if ( ClientID )
    {
      *c++ = ClientID_len + 1;	/* 1 for the field below */
      *c++ = 0;			/* type: string */
      memcpy(c,ClientID,ClientID_len);
      DhcpIface.client_len = ClientID_len + 3;
      return;
    }
  *c++ = ETH_ALEN + 1;	        /* length: 6 (MAC Addr) + 1 (# field) */
  *c++ = ARPHRD_ETHER;		/* type: Ethernet address */
  memcpy(c,ClientHwAddr,ETH_ALEN);
  DhcpIface.client_len = ETH_ALEN + 3;
}
/*****************************************************************************/
void releaseDhcpOptions()
{
  register int i;
  for (i=1;i<256;i++)
    if ( DhcpOptions.val[i] ) free(DhcpOptions.val[i]);
  memset(&DhcpOptions,0,sizeof(dhcpOptions));
}
/*****************************************************************************/
/* Subtract the `struct timeval' values X and Y,
   storing the result in RESULT.
   Return 1 if the difference is negative, otherwise 0.  */
static int
timeval_subtract (result, x, y)
     struct timeval *result, *x, *y;
{
  /* Perform the carry for the later subtraction by updating Y. */
  if (x->tv_usec < y->tv_usec) {
    int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
    y->tv_usec -= 1000000 * nsec;
    y->tv_sec += nsec;
  }
  if (x->tv_usec - y->tv_usec > 1000000) {
    int nsec = (x->tv_usec - y->tv_usec) / 1000000;
    y->tv_usec += 1000000 * nsec;
    y->tv_sec -= nsec;
  }

  /* Compute the time remaining to wait.
     `tv_usec' is certainly positive. */
  result->tv_sec = x->tv_sec - y->tv_sec;
  result->tv_usec = x->tv_usec - y->tv_usec;

  /* Return 1 if result is negative. */
  return x->tv_sec < y->tv_sec;
}
/*****************************************************************************/
int dhcpSendAndRecv(xid,msg,buildUdpIpMsg)
unsigned xid,msg;
void (*buildUdpIpMsg)(unsigned);
{
  struct sockaddr addr;
  struct timeval begin, current, diff;
  int i,len;
  int j=DHCP_INITIAL_RTO/2;
  int timeout;
  do
    {
      do
    	{
	  j+=j;
	  if (j > DHCP_MAX_RTO) j = DHCP_MAX_RTO;
      	  memset(&addr,0,sizeof(struct sockaddr));
      	  memcpy(addr.sa_data,IfName,IfName_len);
	  buildUdpIpMsg(xid);
      	  if ( sendto(dhcpSocket,&UdpIpMsgSend,
		      sizeof(struct packed_ether_header)+
		      sizeof(udpiphdr)+sizeof(dhcpMessage),0,
		      &addr,sizeof(struct sockaddr)) == -1 )
	    {
	      syslog(LOG_INFO,"sendto: %m\n");
	      return -1;
	    }
	  gettimeofday(&begin, NULL);
      	  i=random();
    	}
      while ( peekfd(dhcpSocket,j+i%200000) );
      do
	{
	  memset(&UdpIpMsgRecv,0,sizeof(udpipMessage));
      	  i=sizeof(struct sockaddr);
      	  len=recvfrom(dhcpSocket,&UdpIpMsgRecv,sizeof(udpipMessage),0,
		     (struct sockaddr *)&addr,&i);
	  if ( len == -1 )
    	    {
      	      syslog(LOG_INFO,"recvfrom: %m\n");
      	      return -1;
    	    }
	  gettimeofday(&current, NULL);
	  timeval_subtract(&diff, &current, &begin);
	  timeout = j - diff.tv_sec*1000000 - diff.tv_usec + random()%200000;
	  if ( UdpIpMsgRecv.ethhdr.ether_type != htons(ETHERTYPE_IP) )
	    continue;
	  if ( ipRecv->ip_p != IPPROTO_UDP ) continue;
	  len-=sizeof(struct packed_ether_header);
	  i=(int )ntohs(ipRecv->ip_len);
	  if ( len < i )
	    {
	      if ( DebugFlag ) syslog(LOG_DEBUG,
		"corrupted IP packet of size=%d and ip_len=%d discarded\n",
		len,i);
	      continue;
	    }
	  len=i-(ipRecv->ip_hl<<2);
	  i=(int )ntohs(udpRecv->uh_ulen);
	  if ( len < i )
	    {
	      if ( DebugFlag ) syslog(LOG_DEBUG,
		"corrupted UDP msg of size=%d and uh_ulen=%d discarded\n",
		len,i);
	      continue;
	    }
	  if ( DoCheckSum )
	    {
	      len=udpipchk((udpiphdr *)UdpIpMsgRecv.udpipmsg);
	      if ( len )
		{
		  if ( DebugFlag )
		    switch ( len )
		      {
			case -1: syslog(LOG_DEBUG,
			  "corrupted IP packet with ip_len=%d discarded\n",
			  (int )ntohs(ipRecv->ip_len));
			  break;
			case -2: syslog(LOG_DEBUG,
			  "corrupted UDP msg with uh_ulen=%d discarded\n",
			  (int )ntohs(udpRecv->uh_ulen));
			break;
		      }
		  continue;
		}
	    }
	  DhcpMsgRecv = (dhcpMessage *)&UdpIpMsgRecv.udpipmsg[(ipRecv->ip_hl<<2)+sizeof(struct udphdr)];
	  if ( DhcpMsgRecv->htype != ARPHRD_ETHER ) continue;
	  if ( DhcpMsgRecv->xid != xid ) continue;
	  if ( DhcpMsgRecv->op != DHCP_BOOTREPLY ) continue;
	  if ( parseDhcpMsgRecv() == msg ) return 0;
	  if ( *(unsigned char *)DhcpOptions.val[dhcpMessageType] == DHCP_NAK )
	    {
	      if ( DhcpOptions.val[dhcpMsg] )
		syslog(LOG_INFO,
		"DHCP_NAK server response received: %s\n",
		(char *)DhcpOptions.val[dhcpMsg]);
	      else
		syslog(LOG_INFO,
		"DHCP_NAK server response received\n");
	      return 1;
	    }
    	}
      while ( timeout > 0 && peekfd(dhcpSocket, timeout) == 0 );
    }
  while ( 1 );
  return 1;
}
/*****************************************************************************/
int dhcpConfig()
{
  int i;
  FILE *f;
  char	cache_file[48];
  struct ifreq		ifr;
  struct rtentry	rtent;
  struct sockaddr_in	*p = (struct sockaddr_in *)&(ifr.ifr_addr);
  struct hostent *hp=NULL;
  char *dname=NULL;
  int dname_len=0;

  if ( TestCase ) return 0;
  memset(&ifr,0,sizeof(struct ifreq));
  memcpy(ifr.ifr_name,IfName,IfName_len);
  p->sin_family = AF_INET;
  p->sin_addr.s_addr = DhcpIface.ciaddr;
  if ( ioctl(dhcpSocket,SIOCSIFADDR,&ifr) == -1 )  /* setting IP address */
    {
      syslog(LOG_INFO,"dhcpConfig: ioctl SIOCSIFADDR: %m\n");
      return -1;
    }
  memcpy(&p->sin_addr.s_addr,DhcpOptions.val[subnetMask],4);
  if ( ioctl(dhcpSocket,SIOCSIFNETMASK,&ifr) == -1 )  /* setting netmask */
    {
      syslog(LOG_INFO,"dhcpConfig: ioctl SIOCSIFNETMASK: %m\n");
      return -1;
    }
  memcpy(&p->sin_addr.s_addr,DhcpOptions.val[broadcastAddr],4);
  if ( ioctl(dhcpSocket,SIOCSIFBRDADDR,&ifr) == -1 ) /* setting broadcast address */
    syslog(LOG_INFO,"dhcpConfig: ioctl SIOCSIFBRDADDR: %m\n");

  /* setting local route - not needed on later kernels  */
#ifdef OLD_LINUX_VERSION
  memset(&rtent,0,sizeof(struct rtentry));
  p			=	(struct sockaddr_in *)&rtent.rt_dst;
  p->sin_family		=	AF_INET;
  memcpy(&p->sin_addr.s_addr,DhcpOptions.val[subnetMask],4);
  p->sin_addr.s_addr	&=	DhcpIface.ciaddr;
  p			=	(struct sockaddr_in *)&rtent.rt_gateway;
  p->sin_family		=	AF_INET;
  p->sin_addr.s_addr	=	0;
  p			=	(struct sockaddr_in *)&rtent.rt_genmask;
  p->sin_family		=	AF_INET;
  memcpy(&p->sin_addr.s_addr, DhcpOptions.val[subnetMask], 4);
  rtent.rt_dev	=	IfName;
  rtent.rt_metric     =	1;
  rtent.rt_flags      =	RTF_UP;
  if ( ioctl(dhcpSocket,SIOCADDRT,&rtent) )
    syslog(LOG_INFO,"dhcpConfig: ioctl SIOCADDRT: %m\n");
#endif

  for (i=0;i<DhcpOptions.len[staticRoute];i+=8)
    {  /* setting static routes */
      memset(&rtent,0,sizeof(struct rtentry));
      p                   =   (struct sockaddr_in *)&rtent.rt_dst;
      p->sin_family	  =	  AF_INET;
      memcpy(&p->sin_addr.s_addr,
      ((char *)DhcpOptions.val[staticRoute])+i,4);
      p		          =	  (struct sockaddr_in *)&rtent.rt_gateway;
      p->sin_family	  =	  AF_INET;
      memcpy(&p->sin_addr.s_addr,
      ((char *)DhcpOptions.val[staticRoute])+i+4,4);
      p		          =	  (struct sockaddr_in *)&rtent.rt_genmask;
      p->sin_family	  =       AF_INET;
      p->sin_addr.s_addr  =	  0xffffffff;
      rtent.rt_dev	      =	  IfName;
      rtent.rt_metric     =	  1;
      rtent.rt_flags      =	  RTF_UP|RTF_HOST;
      if ( ioctl(dhcpSocket,SIOCADDRT,&rtent) )
	syslog(LOG_INFO,"dhcpConfig: ioctl SIOCADDRT: %m\n");
    }
  for (i=0;i<DhcpOptions.len[routersOnSubnet];i+=4)
    {  /* setting default routes */
      memset(&rtent,0,sizeof(struct rtentry));
      p			=	(struct sockaddr_in *)&rtent.rt_dst;
      p->sin_family		=	AF_INET;
      p->sin_addr.s_addr	=	0;
      p			=	(struct sockaddr_in *)&rtent.rt_gateway;
      p->sin_family		=	AF_INET;
      memcpy(&p->sin_addr.s_addr,
      ((char *)DhcpOptions.val[routersOnSubnet])+i,4);
      p			=	(struct sockaddr_in *)&rtent.rt_genmask;
      p->sin_family		=	AF_INET;
      p->sin_addr.s_addr	=	0;
      rtent.rt_dev		=	IfName;
	/* setting default routes metric . --> Ryoko 2005/09/02 */
      rtent.rt_metric	        =	NK_DEFAULT_METRIC;
//      rtent.rt_metric	        =	1;
	//<--
      rtent.rt_flags	        =	RTF_UP|RTF_GATEWAY;
      if ( ioctl(dhcpSocket,SIOCADDRT,&rtent) == -1 )
	{
	  if ( errno == ENETUNREACH )    /* possibly gateway is over the bridge */
	    {                            /* try adding a route to gateway first */
	      memset(&rtent,0,sizeof(struct rtentry));
	      p                   =   (struct sockaddr_in *)&rtent.rt_dst;
	      p->sin_family	      =	  AF_INET;
	      memcpy(&p->sin_addr.s_addr,
	      ((char *)DhcpOptions.val[routersOnSubnet])+i,4);
	      p		      =	  (struct sockaddr_in *)&rtent.rt_gateway;
	      p->sin_family	      =	  AF_INET;
	      p->sin_addr.s_addr  =   0;
	      p		      =	  (struct sockaddr_in *)&rtent.rt_genmask;
	      p->sin_family	      =   AF_INET;
	      p->sin_addr.s_addr  =	  0xffffffff;
	      rtent.rt_dev	      =	  IfName;
	      rtent.rt_metric     =	  0;
	      rtent.rt_flags      =	  RTF_UP|RTF_HOST;
	      if ( ioctl(dhcpSocket,SIOCADDRT,&rtent) == 0 )
		{
		  memset(&rtent,0,sizeof(struct rtentry));
		  p    	             =	(struct sockaddr_in *)&rtent.rt_dst;
		  p->sin_family	     =	AF_INET;
		  p->sin_addr.s_addr =	0;
		  p		     =	(struct sockaddr_in *)&rtent.rt_gateway;
		  p->sin_family	     =	AF_INET;
		  memcpy(&p->sin_addr.s_addr,
		  ((char *)DhcpOptions.val[routersOnSubnet])+i,4);
		  p		     =	(struct sockaddr_in *)&rtent.rt_genmask;
		  p->sin_family	     =	AF_INET;
		  p->sin_addr.s_addr =	0;
		  rtent.rt_dev	     =	IfName;
		  rtent.rt_metric    =	1;
	          rtent.rt_flags     =	RTF_UP|RTF_GATEWAY;
	          if ( ioctl(dhcpSocket,SIOCADDRT,&rtent) == -1 )
		    syslog(LOG_INFO,"dhcpConfig: ioctl SIOCADDRT: %m\n");
		}
	    }
	  else
	    syslog(LOG_INFO,"dhcpConfig: ioctl SIOCADDRT: %m\n");
	}
    }
  arpInform();
  if ( DebugFlag )
    fprintf(stdout,"dhcpcd: your IP address = %u.%u.%u.%u\n",
    ((unsigned char *)&DhcpIface.ciaddr)[0],
    ((unsigned char *)&DhcpIface.ciaddr)[1],
    ((unsigned char *)&DhcpIface.ciaddr)[2],
    ((unsigned char *)&DhcpIface.ciaddr)[3]);
  if ( SetHostName )
    {
      if ( ! DhcpOptions.len[hostName] )
	{
	  hp=gethostbyaddr((char *)&DhcpIface.ciaddr,
	  sizeof(DhcpIface.ciaddr),AF_INET);
	  if ( hp )
	    {
	      dname=hp->h_name;
	      while ( *dname > 32 )
		if ( *dname == '.' )
		  break;
		else
		  dname++;
	      dname_len=dname-hp->h_name;
	      DhcpOptions.val[hostName]=(char *)malloc(dname_len+1);
	      DhcpOptions.len[hostName]=dname_len;
	      memcpy((char *)DhcpOptions.val[hostName],
	      hp->h_name,dname_len);
	      ((char *)DhcpOptions.val[hostName])[dname_len]=0;
	      DhcpOptions.num++;
	    }
	}
      if ( DhcpOptions.len[hostName] )
        {
          sethostname(DhcpOptions.val[hostName],DhcpOptions.len[hostName]);
	  if ( DebugFlag )
	    fprintf(stdout,"dhcpcd: your hostname = %s\n",
	    (char *)DhcpOptions.val[hostName]);
	}
    }
  if ( SetDomainName )
    {
      if ( DhcpOptions.len[nisDomainName] )
        {
          setdomainname(DhcpOptions.val[nisDomainName],
		      DhcpOptions.len[nisDomainName]);
	  if ( DebugFlag )
	    fprintf(stdout,"dhcpcd: your domainname = %s\n",
		(char *)DhcpOptions.val[nisDomainName]);
        }
      else
        {
	  if ( ! DhcpOptions.len[domainName] )
	    {
	      if ( ! hp )
		hp=gethostbyaddr((char *)&DhcpIface.ciaddr,
		sizeof(DhcpIface.ciaddr),AF_INET);
	      if ( hp )
		{
		  dname=hp->h_name;
		  while ( *dname > 32 )
		    if ( *dname == '.' )
		      {
			dname++;
		        break;
		      }
		    else
		      dname++;
		  dname_len=strlen(dname);
		  if ( dname_len )
		    {
		      DhcpOptions.val[domainName]=(char *)malloc(dname_len+1);
		      DhcpOptions.len[domainName]=dname_len;
		      memcpy((char *)DhcpOptions.val[domainName],
		      dname,dname_len);
		      ((char *)DhcpOptions.val[domainName])[dname_len]=0;
		      DhcpOptions.num++;
		    }
		}
	    }
          if ( DhcpOptions.len[domainName] )
            {
              setdomainname(DhcpOptions.val[domainName],
			DhcpOptions.len[domainName]);
	      if ( DebugFlag )
		fprintf(stdout,"dhcpcd: your domainname = %s\n",
		(char *)DhcpOptions.val[domainName]);
	    }
	}
    }
  sprintf(cache_file,DHCP_CACHE_FILE,IfName);
  i=open(cache_file,O_WRONLY|O_CREAT|O_TRUNC,S_IRUSR+S_IWUSR);
 // if ( i == -1 || write(i,(char *)&DhcpIface,sizeof(dhcpInterface)) == -1 || close(i) == -1 )
//    syslog(LOG_INFO,"dhcpConfig: open/write/close: %m\n");
  sprintf(cache_file,DHCP_HOSTINFO,IfName);
  f=fopen(cache_file,"w");
  if ( f )
    {
      int b,c;
      memcpy(&b,DhcpOptions.val[subnetMask],4);
      c = DhcpIface.ciaddr & b;
      fprintf(f,"\
IPADDR=%u.%u.%u.%u\n\
NETMASK=%u.%u.%u.%u\n\
NETWORK=%u.%u.%u.%u\n\
BROADCAST=%u.%u.%u.%u\n\
GATEWAY=%u.%u.%u.%u",
((unsigned char *)&DhcpIface.ciaddr)[0],
((unsigned char *)&DhcpIface.ciaddr)[1],
((unsigned char *)&DhcpIface.ciaddr)[2],
((unsigned char *)&DhcpIface.ciaddr)[3],
((unsigned char *)DhcpOptions.val[subnetMask])[0],
((unsigned char *)DhcpOptions.val[subnetMask])[1],
((unsigned char *)DhcpOptions.val[subnetMask])[2],
((unsigned char *)DhcpOptions.val[subnetMask])[3],
((unsigned char *)&c)[0],
((unsigned char *)&c)[1],
((unsigned char *)&c)[2],
((unsigned char *)&c)[3],
((unsigned char *)DhcpOptions.val[broadcastAddr])[0],
((unsigned char *)DhcpOptions.val[broadcastAddr])[1],
((unsigned char *)DhcpOptions.val[broadcastAddr])[2],
((unsigned char *)DhcpOptions.val[broadcastAddr])[3],
((unsigned char *)DhcpOptions.val[routersOnSubnet])[0],
((unsigned char *)DhcpOptions.val[routersOnSubnet])[1],
((unsigned char *)DhcpOptions.val[routersOnSubnet])[2],
((unsigned char *)DhcpOptions.val[routersOnSubnet])[3]);
for (i=4;i<DhcpOptions.len[routersOnSubnet];i+=4)
  fprintf(f,",%u.%u.%u.%u",
  ((unsigned char *)DhcpOptions.val[routersOnSubnet])[i],
  ((unsigned char *)DhcpOptions.val[routersOnSubnet])[1+i],
  ((unsigned char *)DhcpOptions.val[routersOnSubnet])[2+i],
  ((unsigned char *)DhcpOptions.val[routersOnSubnet])[3+i]);
if ( DhcpOptions.len[staticRoute] )
  {
    fprintf(f,"\nROUTE=%u.%u.%u.%u,%u.%u.%u.%u",
    ((unsigned char *)DhcpOptions.val[staticRoute])[0],
    ((unsigned char *)DhcpOptions.val[staticRoute])[1],
    ((unsigned char *)DhcpOptions.val[staticRoute])[2],
    ((unsigned char *)DhcpOptions.val[staticRoute])[3],
    ((unsigned char *)DhcpOptions.val[staticRoute])[4],
    ((unsigned char *)DhcpOptions.val[staticRoute])[5],
    ((unsigned char *)DhcpOptions.val[staticRoute])[6],
    ((unsigned char *)DhcpOptions.val[staticRoute])[7]);
    for (i=8;i<DhcpOptions.len[staticRoute];i+=8)
    fprintf(f,",%u.%u.%u.%u,%u.%u.%u.%u",
    ((unsigned char *)DhcpOptions.val[staticRoute])[i],
    ((unsigned char *)DhcpOptions.val[staticRoute])[1+i],
    ((unsigned char *)DhcpOptions.val[staticRoute])[2+i],
    ((unsigned char *)DhcpOptions.val[staticRoute])[3+i],
    ((unsigned char *)DhcpOptions.val[staticRoute])[4+i],
    ((unsigned char *)DhcpOptions.val[staticRoute])[5+i],
    ((unsigned char *)DhcpOptions.val[staticRoute])[6+i],
    ((unsigned char *)DhcpOptions.val[staticRoute])[7+i]);
  }
if ( DhcpOptions.len[hostName] )
  fprintf(f,"\nHOSTNAME=%s",(char *)DhcpOptions.val[hostName]);
if ( DhcpOptions.len[domainName] )
  fprintf(f,"\nDOMAIN=%s",(char *)DhcpOptions.val[domainName]);
if ( DhcpOptions.len[nisDomainName] )
  fprintf(f,"\nNISDOMAIN=%s",(char *)DhcpOptions.val[nisDomainName]);
fprintf(f,"\n\
DNS=%u.%u.%u.%u",
((unsigned char *)DhcpOptions.val[dns])[0],
((unsigned char *)DhcpOptions.val[dns])[1],
((unsigned char *)DhcpOptions.val[dns])[2],
((unsigned char *)DhcpOptions.val[dns])[3]);
for (i=4;i<DhcpOptions.len[dns];i+=4)
  fprintf(f,",%u.%u.%u.%u",
  ((unsigned char *)DhcpOptions.val[dns])[i],
  ((unsigned char *)DhcpOptions.val[dns])[1+i],
  ((unsigned char *)DhcpOptions.val[dns])[2+i],
  ((unsigned char *)DhcpOptions.val[dns])[3+i]);
fprintf(f,"\n\
DHCPSID=%u.%u.%u.%u\n\
DHCPGIADDR=%u.%u.%u.%u\n\
DHCPSIADDR=%u.%u.%u.%u\n\
DHCPCHADDR=%02X:%02X:%02X:%02X:%02X:%02X\n\
DHCPSHADDR=%02X:%02X:%02X:%02X:%02X:%02X\n\
DHCPSNAME=%s\n\
LEASETIME=%u\n\
RENEWALTIME=%u\n\
REBINDTIME=%u\n",
((unsigned char *)DhcpOptions.val[dhcpServerIdentifier])[0],
((unsigned char *)DhcpOptions.val[dhcpServerIdentifier])[1],
((unsigned char *)DhcpOptions.val[dhcpServerIdentifier])[2],
((unsigned char *)DhcpOptions.val[dhcpServerIdentifier])[3],
((unsigned char *)&DhcpMsgRecv->giaddr)[0],
((unsigned char *)&DhcpMsgRecv->giaddr)[1],
((unsigned char *)&DhcpMsgRecv->giaddr)[2],
((unsigned char *)&DhcpMsgRecv->giaddr)[3],
((unsigned char *)&DhcpMsgRecv->siaddr)[0],
((unsigned char *)&DhcpMsgRecv->siaddr)[1],
((unsigned char *)&DhcpMsgRecv->siaddr)[2],
((unsigned char *)&DhcpMsgRecv->siaddr)[3],
ClientHwAddr[0],
ClientHwAddr[1],
ClientHwAddr[2],
ClientHwAddr[3],
ClientHwAddr[4],
ClientHwAddr[5],
DhcpIface.shaddr[0],
DhcpIface.shaddr[1],
DhcpIface.shaddr[2],
DhcpIface.shaddr[3],
DhcpIface.shaddr[4],
DhcpIface.shaddr[5],
DhcpMsgRecv->sname,
ntohl(*(unsigned int *)DhcpOptions.val[dhcpIPaddrLeaseTime]),
ntohl(*(unsigned int *)DhcpOptions.val[dhcpT1value]),
ntohl(*(unsigned int *)DhcpOptions.val[dhcpT2value]));
      fclose(f);
    }
 // else
//    syslog(LOG_INFO,"dhcpConfig: fopen: %m\n");
  if ( ReplResolvConf )
    {
      mode_t oldumask;
      rename(RESOLV_CONF,""RESOLV_CONF".sv");
      oldumask=umask(022);
      f=fopen(RESOLV_CONF,"w");
      umask(oldumask);
      if ( f )
	{
	  int d_i;
	  if ( DhcpOptions.len[nisDomainName] )
	    fprintf(f,"domain %s\n",(char *)DhcpOptions.val[nisDomainName]);
	  else
	    if ( DhcpOptions.len[domainName] )
	      fprintf(f,"domain %s\n",(char *)DhcpOptions.val[domainName]);
	  for (d_i=0;d_i<DhcpOptions.len[dns];d_i+=4)
	    fprintf(f,"nameserver %u.%u.%u.%u\n",
	    ((unsigned char *)DhcpOptions.val[dns])[d_i],
	    ((unsigned char *)DhcpOptions.val[dns])[d_i+1],
	    ((unsigned char *)DhcpOptions.val[dns])[d_i+2],
	    ((unsigned char *)DhcpOptions.val[dns])[d_i+3]);
	  if ( DhcpOptions.len[nisDomainName] )
	    fprintf(f,"search %s\n",(char *)DhcpOptions.val[nisDomainName]);
	  if ( DhcpOptions.len[domainName] )
	    fprintf(f,"search %s\n",(char *)DhcpOptions.val[domainName]);
	  fclose(f);
	}
 //     else
//	syslog(LOG_INFO,"dhcpConfig: fopen: %m\n");
    }
  if ( Cfilename )
    if ( fork() == 0 )
      {
	char *argc[2];
	argc[0]=Cfilename;
	argc[1]=NULL;
	if ( execve(Cfilename,argc,ProgramEnviron) )
	  syslog(LOG_INFO,"error executing \"%s\": %m\n",
	  Cfilename);
	exit(0);
      }
  if ( *(unsigned int *)DhcpOptions.val[dhcpIPaddrLeaseTime] == 0xffffffff )
    {
      syslog(LOG_INFO,"infinite IP address lease time. Exiting\n");
      exit(0);
    }
  return 0;
}
/*****************************************************************************/
int readDhcpCache()
{
  int i,o;
  char cache_file[48];
  sprintf(cache_file,DHCP_CACHE_FILE,IfName);
  i=open(cache_file,O_RDONLY);
  if ( i == -1 ) return -1;
  o=read(i,(char *)&DhcpIface,sizeof(dhcpInterface));
  close(i);
  if ( o != sizeof(dhcpInterface) ) return -1;
  prev_ip_addr = DhcpIface.ciaddr;
  return 0;
}
/*****************************************************************************/
void deleteDhcpCache()
{
  char cache_file[48];
  sprintf(cache_file,DHCP_CACHE_FILE,IfName);
  unlink(cache_file);
}
/*****************************************************************************/
void *dhcpStart()
{
  int o = 1;
  struct ifreq	ifr;
  struct sockaddr_pkt sap;
  memset(&ifr,0,sizeof(struct ifreq));
  memcpy(ifr.ifr_name,IfName,IfName_len);
#ifdef OLD_LINUX_VERSION
  dhcpSocket = socket(AF_INET,SOCK_PACKET,htons(ETH_P_ALL));
#else
  dhcpSocket = socket(AF_PACKET,SOCK_PACKET,htons(ETH_P_ALL));
#endif
  if ( dhcpSocket == -1 )
    {
      syslog(LOG_INFO,"dhcpStart: socket: %m\n");
      exit(1);
    }

  if ( ioctl(dhcpSocket,SIOCGIFHWADDR,&ifr) )
    {
      syslog(LOG_INFO,"dhcpStart: ioctl SIOCGIFHWADDR: %m\n");
      exit(1);
    }
  if ( ifr.ifr_hwaddr.sa_family != ARPHRD_ETHER )
    {
      syslog(LOG_INFO,"dhcpStart: interface %s is not Ethernet\n",ifr.ifr_name);
      exit(1);
    }
  if ( setsockopt(dhcpSocket,SOL_SOCKET,SO_BROADCAST,&o,sizeof(o)) == -1 )
    {
      syslog(LOG_INFO,"dhcpStart: setsockopt: %m\n");
      exit(1);
    }
  ifr.ifr_flags = IFF_UP | IFF_BROADCAST | IFF_NOTRAILERS | IFF_RUNNING;
  if ( ioctl(dhcpSocket,SIOCSIFFLAGS,&ifr) )
    {
      syslog(LOG_INFO,"dhcpStart: ioctl SIOCSIFFLAGS: %m\n");
      exit(1);
    }
  memset(&sap,0,sizeof(sap));
#ifdef OLD_LINUX_VERSION
  sap.spkt_family = AF_INET;
#else
  sap.spkt_family = AF_PACKET;
#endif
  sap.spkt_protocol = htons(ETH_P_ALL);
  memcpy(sap.spkt_device,IfName,IfName_len);
  if ( bind(dhcpSocket,(void*)&sap,sizeof(struct sockaddr)) == -1 )
    syslog(LOG_INFO,"dhcpStart: bind: %m\n");

  memcpy(ClientHwAddr,ifr.ifr_hwaddr.sa_data,ETH_ALEN);
  ip_id=get_time(NULL)&0xffff;
  srandom(ip_id);
  return &dhcpInit;
}
/*****************************************************************************/
void *dhcpReboot()
{
  dhcpStart();
  memset(&DhcpOptions,0,sizeof(DhcpOptions));
  memset(&DhcpIface,0,sizeof(dhcpInterface));
  if ( readDhcpCache() )
    {
      struct ifreq	ifr;
      struct sockaddr_in *p = (struct sockaddr_in *)&(ifr.ifr_addr);
      memset(&DhcpIface,0,sizeof(dhcpInterface));
      memset(&ifr,0,sizeof(struct ifreq));
      memcpy(ifr.ifr_name,IfName,IfName_len);
      p->sin_family = AF_INET;
      if ( ioctl(dhcpSocket,SIOCGIFADDR,&ifr) == 0 )
        DhcpIface.ciaddr=p->sin_addr.s_addr;
      if ( ClassID )
	{
    	  memcpy(DhcpIface.class_id,ClassID,ClassID_len);
	  DhcpIface.class_len=ClassID_len;
	}
      else
	classIDsetup();
      clientIDsetup();
      return &dhcpInit;
    }
  if ( sigsetjmp(env,0xffff) )
    {
      if ( DebugFlag )
	syslog(LOG_DEBUG,"timed out waiting for DHCP_ACK response\n");
      alarm(TimeOut);
      return &dhcpInit;
    }
#if 1
{
int wanidx=0;
sscanf(iFace,"eth%d", &wanidx);
printf("iFace=%s, wanidx=%d\n", iFace, wanidx);
  return dhcpRequest(random()+wanidx,&buildDhcpReboot);
}
#else
  return dhcpRequest(random(),&buildDhcpReboot);
#endif
}
/*****************************************************************************/
void *dhcpInit()
{
  releaseDhcpOptions();

#ifdef DEBUG
fprintf(stderr,"ClassID  = \"%s\"\n\
ClientID = \"%u.%u.%u.%02X.%02X.%02X.%02X.%02X.%02X\"\n",
DhcpIface.class_id,
DhcpIface.client_id[0],DhcpIface.client_id[1],DhcpIface.client_id[2],
DhcpIface.client_id[3],DhcpIface.client_id[4],DhcpIface.client_id[5],
DhcpIface.client_id[6],DhcpIface.client_id[7],DhcpIface.client_id[8]);
#endif

  if ( DebugFlag ) syslog(LOG_DEBUG,"broadcasting DHCP_DISCOVER start\n");
#if 1
{
int wanidx=0;
sscanf(iFace,"eth%d", &wanidx);
printf("iFace=%s, wanidx=%d\n", iFace, wanidx);
  if ( dhcpSendAndRecv(random()+wanidx,DHCP_OFFER,&buildDhcpDiscover) )
    {
      dhcpStop();
      return 0;
    }
}
#else
  if ( dhcpSendAndRecv(random(),DHCP_OFFER,&buildDhcpDiscover) )
    {
      dhcpStop();
      return 0;
    }
#endif

  if ( DebugFlag ) syslog(LOG_DEBUG,"broadcasting second DHCP_DISCOVER\n");
//2005/10/21 Ryoko don't send 2th DHCP_DISCOVER
//  dhcpSendAndRecv(DhcpMsgRecv->xid,DHCP_OFFER,&buildDhcpDiscover);
  prev_ip_addr = DhcpIface.ciaddr;
  DhcpIface.ciaddr = DhcpMsgRecv->yiaddr;
  memcpy(&DhcpIface.siaddr,DhcpOptions.val[dhcpServerIdentifier],4);
  memcpy(DhcpIface.shaddr,UdpIpMsgRecv.ethhdr.ether_shost,ETH_ALEN);
  DhcpIface.xid = DhcpMsgRecv->xid;
/* DHCP_OFFER received */
  if ( DebugFlag )
    syslog(LOG_DEBUG,"DHCP_OFFER received from %s (%u.%u.%u.%u)\n",
    DhcpMsgRecv->sname,
    ((unsigned char *)DhcpOptions.val[dhcpServerIdentifier])[0],
    ((unsigned char *)DhcpOptions.val[dhcpServerIdentifier])[1],
    ((unsigned char *)DhcpOptions.val[dhcpServerIdentifier])[2],
    ((unsigned char *)DhcpOptions.val[dhcpServerIdentifier])[3]);

  return dhcpRequest(DhcpIface.xid,&buildDhcpRequest);
}
/*****************************************************************************/
void *dhcpRequest(xid,buildDhcpMsg)
unsigned xid;
void (*buildDhcpMsg)(unsigned);
{
 char cmdBuf[30],nk_prev_ip_addr[30];
/* send the message and read and parse replies into DhcpOptions */
  if ( DebugFlag )
    syslog(LOG_DEBUG,"broadcasting DHCP_REQUEST for %u.%u.%u.%u\n",
	   ((unsigned char *)&DhcpIface.ciaddr)[0],
	   ((unsigned char *)&DhcpIface.ciaddr)[1],
	   ((unsigned char *)&DhcpIface.ciaddr)[2],
	   ((unsigned char *)&DhcpIface.ciaddr)[3]);
  if ( dhcpSendAndRecv(xid,DHCP_ACK,buildDhcpMsg) ) return &dhcpInit;
  ReqSentTime=get_time(NULL);
  if ( DebugFlag ) syslog(LOG_DEBUG,
    "DHCP_ACK received from %s (%u.%u.%u.%u)\n",DhcpMsgRecv->sname,
    ((unsigned char *)DhcpOptions.val[dhcpServerIdentifier])[0],
    ((unsigned char *)DhcpOptions.val[dhcpServerIdentifier])[1],
    ((unsigned char *)DhcpOptions.val[dhcpServerIdentifier])[2],
    ((unsigned char *)DhcpOptions.val[dhcpServerIdentifier])[3]);
#ifdef ARPCHECK
/* check if the offered IP address already in use */
  if ( arpCheck() )
    {
      if ( DebugFlag ) syslog(LOG_DEBUG,
	"requested %u.%u.%u.%u address is in use\n",
	((unsigned char *)&DhcpIface.ciaddr)[0],
	((unsigned char *)&DhcpIface.ciaddr)[1],
	((unsigned char *)&DhcpIface.ciaddr)[2],
	((unsigned char *)&DhcpIface.ciaddr)[3]);
      dhcpDecline();
      DhcpIface.ciaddr = 0;
      return &dhcpInit;
    }
  if ( DebugFlag ) syslog(LOG_DEBUG,
    "verified %u.%u.%u.%u address is not in use\n",
    ((unsigned char *)&DhcpIface.ciaddr)[0],
    ((unsigned char *)&DhcpIface.ciaddr)[1],
    ((unsigned char *)&DhcpIface.ciaddr)[2],
    ((unsigned char *)&DhcpIface.ciaddr)[3]);
#endif
  if ( dhcpConfig() )
    {
      dhcpStop();
      return 0;
    }
//-->check ISP IP
	sprintf(cmdBuf,"ISP%d WAN",nk_dhcp_search());
	kd_doCommand(cmdBuf, CMD_PRINT, ASH_DO_NOTHING, (char *) nk_prev_ip_addr);
//<--	

//  if ( DhcpIface.ciaddr == prev_ip_addr )
  if ( DhcpIface.ciaddr != inet_addr(nk_prev_ip_addr) )
    {
	/* update sysconfig and inform webBoot when dhcpConfig done. --> Ryoko 2005/07/04 */
	nk_dhcp_dbconfig(1);
      if ( fork() == 0 )
	{
	  char *argc[4],filename[64],ipaddrstr[16];
	  sprintf(ipaddrstr,"%u.%u.%u.%u",
	  ((unsigned char *)&DhcpIface.ciaddr)[0],
	  ((unsigned char *)&DhcpIface.ciaddr)[1],
	  ((unsigned char *)&DhcpIface.ciaddr)[2],
	  ((unsigned char *)&DhcpIface.ciaddr)[3]);
	  sprintf(filename,EXEC_ON_IP_CHANGE,IfName);
	  argc[0]=PROGRAM_NAME;
	  argc[1]=ipaddrstr;
	  if ( DebugFlag )
	    argc[2]="-d";
	  else
	    argc[2]=NULL;
	  argc[3]=NULL;
	  if ( execve(filename,argc,ProgramEnviron) && errno != ENOENT )
	    syslog(LOG_INFO,"error executing \"%s %s\": %m\n",
	    filename,ipaddrstr);
	  exit(0);
	}
      prev_ip_addr=DhcpIface.ciaddr;
    }
    else
    {
	/* update sysconfig and inform webBoot when dhcpConfig done. --> Ryoko 2005/07/04 */
	nk_dhcp_dbconfig(0);	//TT: only update [ISP], doesn't send message to webBoot
    }

  /* Successfull ACK: Use the fields obtained for future requests */
  memcpy(&DhcpIface.siaddr,DhcpOptions.val[dhcpServerIdentifier],4);
  memcpy(DhcpIface.shaddr,UdpIpMsgRecv.ethhdr.ether_shost,ETH_ALEN);

  return &dhcpBound;
}
/*****************************************************************************/
void *dhcpBound()
{
  int i;
  if ( sigsetjmp(env,0xffff) ) return &dhcpRenew;
  i=ReqSentTime+ntohl(*(unsigned int *)DhcpOptions.val[dhcpT1value])-get_time(NULL);
  if ( i > 0 )
    alarm(i);
  else
    return &dhcpRenew;

  sleep(ntohl(*(u_int *)DhcpOptions.val[dhcpT1value]));
  return &dhcpRenew;
}
/*****************************************************************************/
void *dhcpRenew()
{
  int i;
  if ( sigsetjmp(env,0xffff) ) return &dhcpRebind;
  i = ReqSentTime+ntohl(*(unsigned int *)DhcpOptions.val[dhcpT2value])-get_time(NULL);
  if ( i > 0 )
    alarm(i);
  else
    return &dhcpRebind;

  if ( DebugFlag )
    syslog(LOG_DEBUG,"sending DHCP_REQUEST for %u.%u.%u.%u to %u.%u.%u.%u\n",
	   ((unsigned char *)&DhcpIface.ciaddr)[0],
	   ((unsigned char *)&DhcpIface.ciaddr)[1],
	   ((unsigned char *)&DhcpIface.ciaddr)[2],
	   ((unsigned char *)&DhcpIface.ciaddr)[3],
	   ((unsigned char *)&DhcpIface.siaddr)[0],
	   ((unsigned char *)&DhcpIface.siaddr)[1],
	   ((unsigned char *)&DhcpIface.siaddr)[2],
	   ((unsigned char *)&DhcpIface.siaddr)[3]);
#if 1
{
int wanidx=0;
sscanf(iFace,"eth%d", &wanidx);
printf("iFace=%s, wanidx=%d\n", iFace, wanidx);
  if ( dhcpSendAndRecv(random()+wanidx,DHCP_ACK,&buildDhcpRenew) ) return &dhcpRebind;
}
#else
  if ( dhcpSendAndRecv(random(),DHCP_ACK,&buildDhcpRenew) ) return &dhcpRebind;
#endif
  ReqSentTime=get_time(NULL);
  if ( DebugFlag ) syslog(LOG_DEBUG,
    "DHCP_ACK received from %s (%u.%u.%u.%u)\n",DhcpMsgRecv->sname,
    ((unsigned char *)DhcpOptions.val[dhcpServerIdentifier])[0],
    ((unsigned char *)DhcpOptions.val[dhcpServerIdentifier])[1],
    ((unsigned char *)DhcpOptions.val[dhcpServerIdentifier])[2],
    ((unsigned char *)DhcpOptions.val[dhcpServerIdentifier])[3]);
  return &dhcpBound;
}
/*****************************************************************************/
void *dhcpRebind()
{
  char cmd[CMDBUF_SIZE],buf[20];
  int i,iface;
  if ( sigsetjmp(env,0xffff) )
  {
	/* Daniel Cheng 2010.4.29 DHCP client bug fix 12401 */
	if((iface=nk_dhcp_search())!=-1)
	{
		sprintf(cmd, "ISP%d WAN=0.0.0.0",iface);
		kd_doCommand(cmd, CMD_WRITE, ASH_DO_NOTHING, (char *)NULL);

		sprintf(cmd, "ISP%d WANMASK=0.0.0.0",iface);
		kd_doCommand(cmd, CMD_WRITE, ASH_DO_NOTHING, (char *)NULL);

		sprintf(cmd, "ISP%d GATEWAY=0.0.0.0",iface);
		kd_doCommand(cmd, CMD_WRITE, ASH_DO_NOTHING, (char *)NULL);
	}
	return &dhcpStop;
  }
  i = ReqSentTime+ntohl(*(unsigned int *)DhcpOptions.val[dhcpIPaddrLeaseTime])-get_time(NULL);
  if ( i > 0 )
    alarm(i);
  else
  {
	/* Daniel Cheng 2010.4.29 DHCP client bug fix 12401 */
	if((iface=nk_dhcp_search())!=-1)
	{
		sprintf(cmd, "ISP%d WAN=0.0.0.0",iface);
		kd_doCommand(cmd, CMD_WRITE, ASH_DO_NOTHING, (char *)NULL);

		sprintf(cmd, "ISP%d WANMASK=0.0.0.0",iface);
		kd_doCommand(cmd, CMD_WRITE, ASH_DO_NOTHING, (char *)NULL);

		sprintf(cmd, "ISP%d GATEWAY=0.0.0.0",iface);
		kd_doCommand(cmd, CMD_WRITE, ASH_DO_NOTHING, (char *)NULL);
	}
	return &dhcpStop;
  }

  if ( DebugFlag )
    syslog(LOG_DEBUG,"broadcasting DHCP_REQUEST for %u.%u.%u.%u\n",
	   ((unsigned char *)&DhcpIface.ciaddr)[0],
	   ((unsigned char *)&DhcpIface.ciaddr)[1],
	   ((unsigned char *)&DhcpIface.ciaddr)[2],
	   ((unsigned char *)&DhcpIface.ciaddr)[3]);
#if 1
{
int wanidx=0;
sscanf(iFace,"eth%d", &wanidx);
printf("iFace=%s, wanidx=%d\n", iFace, wanidx);
  if ( dhcpSendAndRecv(random()+wanidx,DHCP_ACK,&buildDhcpRebind) )
  {
	  /* Daniel Cheng 2010.4.29 DHCP client bug fix 12401 */
	/* purpose: 12401
	  * author: Daniel
	  * date: 2010-05-27
	  * description: When WAN DHCP renew IP, clear the ISP DB.
	*/
	  if((iface=nk_dhcp_search())!=-1)
	  {
		  sprintf(cmd, "ISP%d WAN=0.0.0.0",iface);
		  kd_doCommand(cmd, CMD_WRITE, ASH_DO_NOTHING, (char *)NULL);

		  sprintf(cmd, "ISP%d WANMASK=0.0.0.0",iface);
		  kd_doCommand(cmd, CMD_WRITE, ASH_DO_NOTHING, (char *)NULL);

		  sprintf(cmd, "ISP%d GATEWAY=0.0.0.0",iface);
		  kd_doCommand(cmd, CMD_WRITE, ASH_DO_NOTHING, (char *)NULL);
	  }
	  return &dhcpStop;
  }
}
#else
  if ( dhcpSendAndRecv(random(),DHCP_ACK,&buildDhcpRebind) ) return &dhcpStop;
#endif
  ReqSentTime=get_time(NULL);
  if ( DebugFlag ) syslog(LOG_DEBUG,
    "DHCP_ACK received from %s (%u.%u.%u.%u)\n",DhcpMsgRecv->sname,
    ((unsigned char *)DhcpOptions.val[dhcpServerIdentifier])[0],
    ((unsigned char *)DhcpOptions.val[dhcpServerIdentifier])[1],
    ((unsigned char *)DhcpOptions.val[dhcpServerIdentifier])[2],
    ((unsigned char *)DhcpOptions.val[dhcpServerIdentifier])[3]);

  /* Successfull ACK: Use the fields obtained for future requests */
  memcpy(&DhcpIface.siaddr,DhcpOptions.val[dhcpServerIdentifier],4);
  memcpy(DhcpIface.shaddr,UdpIpMsgRecv.ethhdr.ether_shost,ETH_ALEN);

  return &dhcpBound;
}
/*****************************************************************************/
void *dhcpRelease()
{
  struct sockaddr addr;
  deleteDhcpCache();
  if ( DhcpIface.ciaddr == 0 ) return &dhcpInit;

#if 1
{
int wanidx=0;
sscanf(iFace,"eth%d", &wanidx);
printf("iFace=%s, wanidx=%d\n", iFace, wanidx);
  buildDhcpRelease(random()+wanidx);
}
#else
  buildDhcpRelease(random());
#endif

  memset(&addr,0,sizeof(struct sockaddr));
  memcpy(addr.sa_data,IfName,IfName_len);
  if ( DebugFlag )
    syslog(LOG_DEBUG,"sending DHCP_RELEASE for %u.%u.%u.%u to %u.%u.%u.%u\n",
	   ((unsigned char *)&DhcpIface.ciaddr)[0],
	   ((unsigned char *)&DhcpIface.ciaddr)[1],
	   ((unsigned char *)&DhcpIface.ciaddr)[2],
	   ((unsigned char *)&DhcpIface.ciaddr)[3],
	   ((unsigned char *)&DhcpIface.siaddr)[0],
	   ((unsigned char *)&DhcpIface.siaddr)[1],
	   ((unsigned char *)&DhcpIface.siaddr)[2],
	   ((unsigned char *)&DhcpIface.siaddr)[3]);
  if ( sendto(dhcpSocket,&UdpIpMsgSend,sizeof(struct packed_ether_header)+
	      sizeof(udpiphdr)+sizeof(dhcpMessage),0,
	      &addr,sizeof(struct sockaddr)) == -1 )
    syslog(LOG_INFO,"dhcpRelease: sendto: %m\n");
  arpRelease(); /* clear ARP cache entries for client IP addr */
  return &dhcpInit;
}
/*****************************************************************************/
void *dhcpStop()
{
  struct ifreq ifr;
  struct sockaddr_in	*p = (struct sockaddr_in *)&(ifr.ifr_addr);

  releaseDhcpOptions();
  if ( TestCase ) return &dhcpStart;
  memset(&ifr,0,sizeof(struct ifreq));
  memcpy(ifr.ifr_name,IfName,IfName_len);
  p->sin_family = AF_INET;
  p->sin_addr.s_addr = 0;
/*  if ( ioctl(s,SIOCSIFADDR,&ifr) == -1 )
    syslog(LOG_INFO,"dhcpStop: ioctl SIOCSIFADDR: %m\n");*/
  if ( ioctl(dhcpSocket,SIOCSIFFLAGS,&ifr) )
    syslog(LOG_INFO,"dhcpStop: ioctl SIOCSIFFLAGS: %m\n");
  close(dhcpSocket);
  if ( ReplResolvConf )
    rename(""RESOLV_CONF".sv",RESOLV_CONF);

  /* Ryoko 2005/07/04:clear ISP information in sysconfig, when iface shutdown */
  nk_dhcp_dbconfig_clear();
  // <--

  return &dhcpStart;
}
/*****************************************************************************/
#ifdef ARPCHECK
void *dhcpDecline()
{
  struct sockaddr addr;
  memset(&UdpIpMsgSend,0,sizeof(udpipMessage));
  memcpy(UdpIpMsgSend.ethhdr.ether_dhost,MAC_BCAST_ADDR,ETH_ALEN);
  memcpy(UdpIpMsgSend.ethhdr.ether_shost,ClientHwAddr,ETH_ALEN);
  UdpIpMsgSend.ethhdr.ether_type = htons(ETHERTYPE_IP);
#if 1
{
int wanidx=0;
sscanf(iFace,"eth%d", &wanidx);
printf("iFace=%s, wanidx=%d\n", iFace, wanidx);
  buildDhcpDecline(random()+wanidx);
}
#else
  buildDhcpDecline(random());
#endif
  udpipgen((udpiphdr *)&UdpIpMsgSend.udpipmsg,0,INADDR_BROADCAST,
  htons(DHCP_CLIENT_PORT),htons(DHCP_SERVER_PORT),sizeof(dhcpMessage));
  memset(&addr,0,sizeof(struct sockaddr));
  memcpy(addr.sa_data,IfName,IfName_len);
  if ( DebugFlag ) syslog(LOG_DEBUG,"broadcasting DHCP_DECLINE\n");
  if ( sendto(dhcpSocket,&UdpIpMsgSend,sizeof(struct packed_ether_header)+
	      sizeof(udpiphdr)+sizeof(dhcpMessage),0,
	      &addr,sizeof(struct sockaddr)) == -1 )
    syslog(LOG_INFO,"dhcpDecline: sendto: %m\n");
  return &dhcpInit;
}
#endif
/*****************************************************************************/
void checkIfAlreadyRunning()
{
  int o;
  char pidfile[48];
  sprintf(pidfile,PID_FILE_PATH,IfName);
  o=open(pidfile,O_RDONLY);
  if ( o == -1 ) return;
  close(o);
  fprintf(stderr,"\
****  %s: already running\n\
****  %s: if not then delete %s file\n",ProgramName,ProgramName,pidfile);
  exit(1);
}
/*****************************************************************************/
void *dhcpInform()
{
  dhcpStart();
  memset(&DhcpOptions,0,sizeof(DhcpOptions));
  memset(&DhcpIface,0,sizeof(dhcpInterface));
  if ( ! inform_ipaddr.s_addr )
    {
      struct ifreq ifr;
      struct sockaddr_in *p = (struct sockaddr_in *)&(ifr.ifr_addr);
      memset(&ifr,0,sizeof(struct ifreq));
      memcpy(ifr.ifr_name,IfName,IfName_len);
      p->sin_family = AF_INET;
      if ( ioctl(dhcpSocket,SIOCGIFADDR,&ifr) == 0 )
        inform_ipaddr.s_addr=p->sin_addr.s_addr;
      if ( ! inform_ipaddr.s_addr )
	{
	  if ( readDhcpCache() )
	    {
	      syslog(LOG_INFO,"dhcpInform: no IP address given\n");
	      return 0;
	    }
          else
	    inform_ipaddr.s_addr=DhcpIface.ciaddr;
	}
    }
  DhcpIface.ciaddr=inform_ipaddr.s_addr;
  if ( ! DhcpIface.class_len )
    { 
      if ( ClassID )
        {
    	  memcpy(DhcpIface.class_id,ClassID,ClassID_len);
	  DhcpIface.class_len=ClassID_len;
        }
      else
        classIDsetup();
    }
  if ( ! DhcpIface.client_len ) clientIDsetup();
  if ( sigsetjmp(env,0xffff) )
    {
      if ( DebugFlag )
	syslog(LOG_DEBUG,"timed out waiting for DHCP_ACK response\n");
      return 0;
    }
  if ( DebugFlag )
    syslog(LOG_DEBUG,"broadcasting DHCP_INFORM for %u.%u.%u.%u\n",
	   ((unsigned char *)&DhcpIface.ciaddr)[0],
	   ((unsigned char *)&DhcpIface.ciaddr)[1],
	   ((unsigned char *)&DhcpIface.ciaddr)[2],
	   ((unsigned char *)&DhcpIface.ciaddr)[3]);
#if 1
{
int wanidx=0;
sscanf(iFace,"eth%d", &wanidx);
printf("iFace=%s, wanidx=%d\n", iFace, wanidx);
  if ( dhcpSendAndRecv(random()+wanidx,DHCP_ACK,buildDhcpInform) ) return 0;
}
#else
  if ( dhcpSendAndRecv(random(),DHCP_ACK,buildDhcpInform) ) return 0;
#endif
  if ( DebugFlag ) syslog(LOG_DEBUG,
    "DHCP_ACK received from %s (%u.%u.%u.%u)\n",DhcpMsgRecv->sname,
    ((unsigned char *)DhcpOptions.val[dhcpServerIdentifier])[0],
    ((unsigned char *)DhcpOptions.val[dhcpServerIdentifier])[1],
    ((unsigned char *)DhcpOptions.val[dhcpServerIdentifier])[2],
    ((unsigned char *)DhcpOptions.val[dhcpServerIdentifier])[3]);
#ifdef ARPCHECK
/* check if the offered IP address already in use */
  if ( arpCheck() )
    {
      if ( DebugFlag ) syslog(LOG_DEBUG,
	"requested %u.%u.%u.%u address is in use\n",
	((unsigned char *)&DhcpIface.ciaddr)[0],
	((unsigned char *)&DhcpIface.ciaddr)[1],
	((unsigned char *)&DhcpIface.ciaddr)[2],
	((unsigned char *)&DhcpIface.ciaddr)[3]);
      dhcpDecline();
      return 0;
    }
  if ( DebugFlag ) syslog(LOG_DEBUG,
    "verified %u.%u.%u.%u address is not in use\n",
    ((unsigned char *)&DhcpIface.ciaddr)[0],
    ((unsigned char *)&DhcpIface.ciaddr)[1],
    ((unsigned char *)&DhcpIface.ciaddr)[2],
    ((unsigned char *)&DhcpIface.ciaddr)[3]);
#endif
  if ( dhcpConfig() ) return 0;
	/* update sysconfig and inform webBoot when dhcpConfig done. --> Ryoko 2005/07/04 */
	nk_dhcp_dbconfig(1);
	// <--

  exit(0);
}

// Ryoko 2005/08/26 search WAN number by InterFace(iFace)
int nk_dhcp_search(void)
{
	int i=1;
	char buf[80],cmdBuf[80];
#ifdef CONFIG_NK_DYNAMIC_PORT_NUM
	unsigned int DYNAMIC_NUM_WAN;
#endif

#ifdef CONFIG_NK_DYNAMIC_PORT_NUM
	DYNAMIC_NUM_WAN = Get_Num_Wan();
#endif

#ifdef CONFIG_NK_DYNAMIC_PORT_NUM
	for( i = 1; i < ( DYNAMIC_NUM_WAN + 1 ); i++ )
#else
	for(i=1;i<CONFIG_NK_NUM_WAN+1;i++)
#endif
	{
		sprintf(cmdBuf,"WAN%d WANINTERFACE",i);
		kd_doCommand(cmdBuf, CMD_PRINT, ASH_DO_NOTHING, buf);	
		if(!strcmp(iFace,buf))	
			return i;
	}
	return -1;
}/* nk_dhcp_search() */


/* update sysconfig and inform webBoot when dhcpconfig done. Kide 2005/03/15 */
void nk_dhcp_dbconfig(int flag)
{
	char	cmd[CMDBUF_SIZE],buf[20];
	int		i,iface;
//	printf("Ryoko start interface:%s WAN%d flag=%d\n",iFace,nk_dhcp_search(),flag);
	if((iface=nk_dhcp_search())==-1)
		return;
	/* update sysconfig database */
	// wan ip
	sprintf(cmd, "ISP%d WAN=%u.%u.%u.%u",iface,
					     ((unsigned char *)&DhcpIface.ciaddr)[0],
					     ((unsigned char *)&DhcpIface.ciaddr)[1],
					     ((unsigned char *)&DhcpIface.ciaddr)[2],
					     ((unsigned char *)&DhcpIface.ciaddr)[3]);
	kd_doCommand(cmd, CMD_WRITE, ASH_DO_NOTHING, (char *)NULL);

	// wan mask
	sprintf(cmd, "ISP%d WANMASK=%u.%u.%u.%u",iface,
						((unsigned char *)DhcpOptions.val[subnetMask])[0],
						((unsigned char *)DhcpOptions.val[subnetMask])[1],
						((unsigned char *)DhcpOptions.val[subnetMask])[2],
						((unsigned char *)DhcpOptions.val[subnetMask])[3]);
	kd_doCommand(cmd, CMD_WRITE, ASH_DO_NOTHING, (char *)NULL);

	// gateway
	sprintf(cmd, "ISP%d GATEWAY=%u.%u.%u.%u",iface,
						((unsigned char *)DhcpOptions.val[routersOnSubnet])[0],
						((unsigned char *)DhcpOptions.val[routersOnSubnet])[1],
						((unsigned char *)DhcpOptions.val[routersOnSubnet])[2],
						((unsigned char *)DhcpOptions.val[routersOnSubnet])[3]);
	kd_doCommand(cmd, CMD_WRITE, ASH_DO_NOTHING, (char *)NULL);

	// interface
	sprintf(cmd, "ISP%d INTERFACE=%s",iface,iFace);
	kd_doCommand(cmd, CMD_WRITE, ASH_DO_NOTHING, (char *)NULL);

	if (flag == 0)
		return;
	// dns servers
	for (i=0; i < 3; i++)
	{
		// delete old entry by order first
//		sprintf(cmd, "ISP DNS");
//		kd_doCommand(cmd, CMD_DELETE, ASH_DO_NOTHING, (char *)NULL);

		if ( (i*4) < DhcpOptions.len[dns] )
		{
			sprintf(cmd, "ISP%d DNS%d=%u.%u.%u.%u",iface, i+1,
								((unsigned char *)DhcpOptions.val[dns])[i*4],
								((unsigned char *)DhcpOptions.val[dns])[i*4+1],
								((unsigned char *)DhcpOptions.val[dns])[i*4+2],
								((unsigned char *)DhcpOptions.val[dns])[i*4+3]);
		}
		else
			sprintf(cmd, "ISP%d DNS%d=0.0.0.0",iface, i+1);
		kd_doCommand(cmd, CMD_WRITE, ASH_DO_NOTHING, (char *)NULL);
	} // for loop

	// inform webBoot that the wan interface is ready
	sprintf(buf,"WAN%d",iface);
	kd_doCommand(buf, NULL, ASH_DO_WAN_CONN_UP, (char *)NULL);
} /* nk_dhcp_dbconfig() */


/* clear ISP information in sysconfig, when iface shutdown */
void nk_dhcp_dbconfig_clear()
{
	char	cmd[CMDBUF_SIZE],buf[20];
	int		i,iface;
	if((iface=nk_dhcp_search())==-1)
		return;
#if 0 //Ryoko clean ISP by webBoot
	// wan ip
	sprintf(cmd, "ISP%d WAN=0.0.0.0",iface);
	kd_doCommand(cmd, CMD_WRITE, ASH_DO_NOTHING, (char *)NULL);

	// wan mask
	sprintf(cmd, "ISP%d WANMASK=0.0.0.0",iface);
	kd_doCommand(cmd, CMD_WRITE, ASH_DO_NOTHING, (char *)NULL);

	// gateway
	sprintf(cmd, "ISP%d GATEWAY=0.0.0.0",iface);
	kd_doCommand(cmd, CMD_WRITE, ASH_DO_NOTHING, (char *)NULL);

	// interface
	sprintf(cmd, "ISP%d INTERFACE=%s",iface,iFace);
	kd_doCommand(cmd, CMD_WRITE, ASH_DO_NOTHING, (char *)NULL);
#endif
#if 0 //TT may not clean ISP DNS
	// dns servers
	for (i=0; i < 3; i++)
	{
		// delete old entry by order first
//		sprintf(cmd, "ISP DNS");
//		kd_doCommand(cmd, CMD_DELETE, ASH_DO_NOTHING, (char *)NULL);

		sprintf(cmd, "ISP DNS%d=0.0.0.0", i+1);
		kd_doCommand(cmd, CMD_WRITE, ASH_DO_NOTHING, (char *)NULL);
	} // for loop
#endif

	// inform webBoot that the wan interface is down
	sprintf(buf,"WAN%d",iface);
	kd_doCommand(buf, NULL, ASH_DO_WAN_CONN_DOWN, (char *)NULL);
} /* nk_dhcp_dbconfig_clear() */
// <-- Kide 2005/03/15

