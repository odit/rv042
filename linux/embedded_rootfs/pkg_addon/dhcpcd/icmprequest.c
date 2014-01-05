/*
 * icmprequest
 * Copyright (C) September, 1999 Sergei Viznyuk <sv@phystech.com>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <net/ethernet.h>
#include "icmpipgen.h"

#define PROGRAM_VERSION      "0.1"
#define DEFAULT_IFNAME	     "eth0"
#define DEFAULT_IFNAME_LEN    4
#define DEFAULT_INTERVAL      1000000
#define MAC_BCAST_ADDR	      "\xff\xff\xff\xff\xff\xff"
#ifndef AF_PACKET
#define AF_PACKET	      17
#endif


unsigned char	ClientHwAddr[ETH_ALEN];
icmpipMessage	IcmpIpMsgSend,IcmpIpMsgRecv;
unsigned short  ip_id;
int peekfd(int s,int tv_usec);
unsigned short  icmp_seq        =       0;
char		*IfName		=	DEFAULT_IFNAME;
int		IfName_len	=	DEFAULT_IFNAME_LEN;
int             Sock            =       0;
int             ipaddr          =       0;
int             bcastaddr       =       0xffffffff;
const struct ip *ipSend=(struct ip *)((struct icmpiphdr *)IcmpIpMsgSend.icmpipmsg)->ip;
const struct icmphdr *icmpSend=(struct icmphdr *)((struct icmpiphdr *)IcmpIpMsgSend.icmpipmsg)->icmp;
const struct ip *ipRecv=(struct ip *)((struct icmpiphdr *)IcmpIpMsgRecv.icmpipmsg)->ip;
const struct icmphdr *icmpRecv=(struct icmphdr *)((struct icmpiphdr *)IcmpIpMsgRecv.icmpipmsg)->icmp;
/*****************************************************************************/
void print_version()
{
  fprintf(stderr,"icmprequest v."PROGRAM_VERSION"\n\
Copyright (C) September, 1999 Sergei Viznyuk <sv@phystech.com>\n");
}
/*****************************************************************************/
void start_interface()
{
  int o = 1;
  struct ifreq	ifr;
  struct sockaddr_in	*p = (struct sockaddr_in *)&(ifr.ifr_addr);

  memset(&ifr,0,sizeof(struct ifreq));
  memcpy(ifr.ifr_name,IfName,IfName_len);
  Sock = socket(AF_PACKET,SOCK_PACKET,htons(ETH_P_ALL));
  if ( Sock == -1 )
    {
      fprintf(stderr,"icmprequest: socket: %s\n",sys_errlist[errno]);
      exit(1);
    }
  if ( ioctl(Sock,SIOCGIFHWADDR,&ifr) )
    {
      fprintf(stderr,"icmprequest: ioctl SIOCGIFHWADDR: %s\n",
	      sys_errlist[errno]);
      exit(1);
    }
  if ( ifr.ifr_hwaddr.sa_family != ARPHRD_ETHER )
    {
      fprintf(stderr,"icmprequest: interface %s is not Ethernet\n",
	      ifr.ifr_name);
      exit(1);
    }
  if ( setsockopt(Sock,SOL_SOCKET,SO_BROADCAST,&o,sizeof(o)) == -1 )
    {
      fprintf(stderr,"icmprequest: setsockopt: %s\n",sys_errlist[errno]);
      exit(1);
    }
  ifr.ifr_flags = IFF_UP | IFF_BROADCAST | IFF_NOTRAILERS | IFF_RUNNING;
  if ( ioctl(Sock,SIOCSIFFLAGS,&ifr) )
    {
      fprintf(stderr,"icmprequest: ioctl SIOCSIFFLAGS: %s\n",
	      sys_errlist[errno]);
      exit(1);
    }
  memcpy(ClientHwAddr,ifr.ifr_hwaddr.sa_data,ETH_ALEN);
  if ( ioctl(Sock,SIOCGIFADDR,&ifr) == 0 )
    ipaddr = p->sin_addr.s_addr;
//  if ( ioctl(Sock,SIOCGIFBRDADDR,&ifr) == 0 )
//    bcastaddr = p->sin_addr.s_addr;
   bcastaddr = inet_addr("128.146.37.255");
  ip_id=time(NULL)&0xffff;
  srandom(ip_id);
}
/*****************************************************************************/
void buildIcmpSubnetMaskRequest(id)
unsigned short id;
{
  memset(&IcmpIpMsgSend,0,sizeof(icmpipMessage));
  memcpy(IcmpIpMsgSend.ethhdr.ether_dhost,MAC_BCAST_ADDR,ETH_ALEN);
  memcpy(IcmpIpMsgSend.ethhdr.ether_shost,ClientHwAddr,ETH_ALEN);
  IcmpIpMsgSend.ethhdr.ether_type = htons(ETHERTYPE_IP);

  icmpipgen((icmpiphdr *)IcmpIpMsgSend.icmpipmsg,ipaddr,bcastaddr,
  17,0,id,icmp_seq++);
}
/*****************************************************************************/
int icmpSendAndRecv(id,buildIcmpIpMsg)
unsigned short id;
void (*buildIcmpIpMsg)(unsigned short);
{
  struct sockaddr addr;
  int i,len;
  do
    	{
      	  memset(&addr,0,sizeof(struct sockaddr));
      	  memcpy(addr.sa_data,IfName,IfName_len);
	  buildIcmpIpMsg(id);
      	  if ( sendto(Sock,&IcmpIpMsgSend,
		      sizeof(struct packed_ether_header)+sizeof(icmpiphdr),0,
		      &addr,sizeof(struct sockaddr)) == -1 )
	    {
	      fprintf(stderr,"icmprequest: sendto: %s\n",sys_errlist[errno]);
	      exit(1);
	    }
    	}
  while ( peekfd(Sock,DEFAULT_INTERVAL) );
  do
	{
	  memset(&IcmpIpMsgRecv,0,sizeof(icmpipMessage));
      	  i=sizeof(struct sockaddr);
      	  len=recvfrom(Sock,&IcmpIpMsgRecv,sizeof(icmpipMessage),0,
		     (struct sockaddr *)&addr,&i);
	  if ( len == -1 )
    	    {
      	      fprintf(stderr,"icmprequest: recvfrom: %s\n",sys_errlist[errno]);
      	      exit(1);
    	    }
	  if ( IcmpIpMsgRecv.ethhdr.ether_type != htons(ETHERTYPE_IP) )
	    continue;
	  if ( ipRecv->ip_p != IPPROTO_ICMP ) continue;
	  len-=sizeof(struct packed_ether_header);
	  i=(int )ntohs(ipRecv->ip_len);
	  if ( len < i )
	    {
	      fprintf(stderr,
	      "corrupted IP packet of size=%d and ip_len=%d discarded\n",
	      len,i);
	      continue;
	    }
	   fprintf(stderr,"passed 1\n");
	  len=icmpipchk((icmpiphdr *)IcmpIpMsgRecv.icmpipmsg);
	  switch ( len )
	    {
	      case 0: break;
	      case -1: fprintf(stderr,
		"corrupted IP packet with ip_len=%d discarded\n",i);
	      	continue;
	      default: fprintf(stderr,
		"corrupted ICMP msg with message size=%d discarded\n",
		 i-(ipRecv->ip_hl<<2));
	      	continue;
	    }
	  icmpRecv = (icmphdr *)&IcmpIpMsgRecv.icmpipmsg[(ipRecv->ip_hl<<2)];
	  if ( icmpRecv->un.echo.id == id ) return 1;
    	}
  while ( peekfd(Sock,DEFAULT_INTERVAL) == 0 );
  return 0;
}
/*****************************************************************************/
int main(argn,argc)
int argn;
char *argc[];
{
  int versionFlag	=	0;
  int subnetmaskFlag	=	0;
  int routersFlag	=	0;
  int s			=	1;
  int k			=	1;
  int i			=	1;

  while ( argc[i] )
    if ( argc[i][0]=='-' )
prgs: switch ( argc[i][s] )
	{
	  case 0:
	    i++;
	    s=1;
	    break;
	  case 'm':
	    s++;
	    subnetmaskFlag=1;
	    goto prgs;
	  case 'r':
	    s++;
	    routersFlag=1;
	    goto prgs;
	  case 'V':
	    s++;
	    versionFlag=1;
	    goto prgs;
          default:
usage:	    print_version();
fprintf(stderr,"Usage: icmprequest [-V] <-m|-r> [interface]\n");
	    exit(1);
	}
    else
      argc[k++]=argc[i++];
  if ( k > 1 )
    {
      if ( (IfName_len=strlen(argc[1])) > IFNAMSIZ )
        goto usage;
      else
        IfName=argc[1];
    }
//  memset(&ipaddr,0,sizeof(ipaddr));
//  if ( (k > 1) && (! inet_aton(argc[1],&ipaddr)) ) goto usage;
  if ( subnetmaskFlag+routersFlag != 1 ) goto usage;
  if ( versionFlag ) print_version();
  start_interface();
  fprintf(stdout,"ipaddr=%u.%u.%u.%u\nbcastaddr=%u.%u.%u.%u\n",
	  ((unsigned char *)&ipaddr)[0],
	  ((unsigned char *)&ipaddr)[1],
	  ((unsigned char *)&ipaddr)[2],
	  ((unsigned char *)&ipaddr)[3],
	  ((unsigned char *)&bcastaddr)[0],
	  ((unsigned char *)&bcastaddr)[1],
	  ((unsigned char *)&bcastaddr)[2],
	  ((unsigned char *)&bcastaddr)[3]);
  if ( icmpSendAndRecv(getpid(),&buildIcmpSubnetMaskRequest) )
    fprintf(stdout,"received mask = %u.%u.%u.%u  from %u.%u.%u.%u\n",
    ((char *)icmpRecv)[sizeof(icmphdr)],
    ((char *)icmpRecv)[sizeof(icmphdr)+1],
    ((char *)icmpRecv)[sizeof(icmphdr)+2],
    ((char *)icmpRecv)[sizeof(icmphdr)+3],
    ((char *)&ipRecv->ip_src.s_addr)[0],
    ((char *)&ipRecv->ip_src.s_addr)[1],
    ((char *)&ipRecv->ip_src.s_addr)[2],
    ((char *)&ipRecv->ip_src.s_addr)[3]);
    fprintf(stdout,"received mask = %u.%u.%u.%u  from %u.%u.%u.%u\n",
    ((char *)icmpRecv)[sizeof(icmphdr)],
    ((char *)icmpRecv)[sizeof(icmphdr)+1],
    ((char *)icmpRecv)[sizeof(icmphdr)+2],
    ((char *)icmpRecv)[sizeof(icmphdr)+3],
    ((char *)&ipRecv->ip_src.s_addr)[0],
    ((char *)&ipRecv->ip_src.s_addr)[1],
    ((char *)&ipRecv->ip_src.s_addr)[2],
    ((char *)&ipRecv->ip_src.s_addr)[3]);
  exit(0);
}
