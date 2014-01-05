/***************************************************************************
 *  v4_handler.h : This file is part of 'ataga'
 *  created on: Wed Jun  8 10:59:23 CDT 2005
 *
 *  (c) 2005 by Lukasz Tomicki <lucas.tomicki@gmail.com>
 * 
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __V4_HANDLER_H__
#define __V4_HANDLER_H__

#include <netinet/ip.h>
#include "ataga.h"

#ifndef ICMP6_DST_UNREACH_BEYONDSCOPE
#define ICMP6_DST_UNREACH_BEYONDSCOPE	2   /* beyond scope of source address */
#endif

void *v4_handler_init(void*);
in_addr get_interface_v4_addr(listen_interface *i);
void Request_Dynamic_DNS_mapping(in_addr*, NAT_Manager::protocol_type, const char*, u32);
void Create_IPv6_Header(char*, const char*, in6_addr*);
bool Translate_v4_Send_v6_Packet(NAT_Manager::addr_mapping *mapping, char*, u32, in6_addr*, u16);
int Get_Data_From_v4_Sockets(char *read_buffer);
void Request_ND(in6_addr *addr, in_addr*, const char *buffer, u32 buffer_size);
bool GetHardwareAddr_v6(const listen_interface **send, in6_addr *dst, in_addr *src, char *packet, u32 size);
bool Send_v6_Packet(const listen_interface *send, const char *packet, u32 size);
int Translate_v4_Packet(NAT_Manager::addr_mapping *mapping, char *packet_v6, char *packet_v4, in6_addr*, u16);
bool Translate_ICMPv4_code(u8 *type, u8 *code);
void SendICMP_MsgTooBig_v6(iphdr *src, const listen_interface *i);
bool CheckMTU_v6(const char *packet_v4, u32 total_length, const listen_interface *i);

extern pending_packet		*v4_pending_packets;
extern listen_interface 	*v4_interfaces;

#endif // __V4_HANDLER_H__
