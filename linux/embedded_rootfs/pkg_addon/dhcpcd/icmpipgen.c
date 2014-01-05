/*
 * dhcpcd - DHCP client daemon -
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

#include <string.h>
#include "udpipgen.h"
#include "icmpipgen.h"

extern unsigned short ip_id;
unsigned short in_cksum(unsigned short *addr,int len);
/*****************************************************************************/
void icmpipgen(icmpip,saddr,daddr,type,code,id,seq)
icmpiphdr *icmpip;
unsigned int saddr,daddr;
unsigned char type,code;
unsigned short id,seq;
{
  struct ip *ip=(struct ip *)icmpip->ip;
  struct icmphdr *icmp=(struct icmphdr *)icmpip->icmp;

  icmp->type=type;
  icmp->code=code;
  icmp->un.echo.id = id;
  icmp->un.echo.sequence = seq;
  icmp->checksum=0;
  icmp->checksum=in_cksum((unsigned short *)icmp,sizeof(struct icmphdr));
  if ( icmp->checksum == 0 ) icmp->checksum = 0xffff;
  ip->ip_hl = 5;
  ip->ip_v = IPVERSION;
  ip->ip_tos = 0;
  ip->ip_len = htons(sizeof(icmpiphdr));
  ip->ip_id = htons(ip_id++);
  ip->ip_off = 0;
  ip->ip_ttl = IPDEFTTL;
  ip->ip_p = IPPROTO_ICMP;
  ip->ip_src.s_addr = saddr;
  ip->ip_dst.s_addr = daddr;
  ip->ip_sum = 0;
  ip->ip_sum = in_cksum((unsigned short *)icmpip,sizeof(struct ip));
}
/*****************************************************************************/
int icmpipchk(icmpip)
icmpiphdr *icmpip;
{
  int hl;
  struct ip *ip=(struct ip *)icmpip->ip;
  struct icmphdr *icmp=(struct icmphdr *)icmpip->icmp;

  hl = ip->ip_hl<<2;
  if ( in_cksum((unsigned short *)icmpip,hl) ) return -1;
  hl -= sizeof(struct ip);
  if ( hl )
    icmp=(struct icmphdr *)((char *)icmpip->icmp+hl);
  if ( icmp->checksum == 0 ) return 0;
  if ( in_cksum((unsigned short *)icmp,ntohs(ip->ip_len)-hl-sizeof(struct ip)) )
     return -2;
  return 0;
}
