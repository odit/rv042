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

#ifndef ICMPIPGEN_H
#define ICMPIPGEN_H

#include <netinet/ip.h>
#include <net/ethernet.h>

#define IPPACKET_SIZE		1500

typedef struct icmphdr
{
  u_int8_t type;
  u_int8_t code;
  u_int16_t checksum;
  union {
     struct {
        u_int16_t id;
        u_int16_t sequence;
     } echo;
     u_int32_t gateway;
     struct {
        u_int16_t __unused;
        u_int16_t mtu;
     } frag;
  } un;
} __attribute__((packed)) icmphdr;

typedef struct icmpiphdr
{
  char ip[sizeof(struct ip)];
  char icmp[sizeof(struct icmphdr)];
} __attribute__((packed)) icmpiphdr;

void icmpipgen(icmpiphdr *icmpip,
	       unsigned int saddr,
	       unsigned int daddr,
	       unsigned char type,
	       unsigned char code,
	       unsigned short id,
	       unsigned short seq);
int icmpipchk(icmpiphdr *icmpip);

struct packed_ether_header {
  u_int8_t  ether_dhost[ETH_ALEN];      /* destination eth addr */
  u_int8_t  ether_shost[ETH_ALEN];      /* source ether addr    */
  u_int16_t ether_type;                 /* packet type ID field */
} __attribute__((packed));

typedef struct icmpipMessage
{
  struct packed_ether_header	ethhdr;
  char	 icmpipmsg[IPPACKET_SIZE];
} __attribute__((packed)) icmpipMessage;

#endif
