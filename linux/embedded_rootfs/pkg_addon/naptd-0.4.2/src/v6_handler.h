/***************************************************************************
 *  v6_handler.h : This file is part of 'ataga'
 *  created on: Wed Jun  8 11:00:06 CDT 2005
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
 
#ifndef __V6_HANDLER_H__
#define __V6_HANDLER_H__

#include <netinet/ip6.h>
#include "ataga.h"

void *v6_handler_init(void*);
int Get_Data_From_v6_Sockets(char *read_buffer);
bool Translate_v6_Send_v4_Packet(NAT_Manager::addr_mapping *, char*, u32, in_addr*, u16);
bool initialise_outbound_interface(int *sock);
bool Send_v4_Packet(const listen_interface *send, const char *packet, u32 size);
void Create_IPv4_Header(char*, const char*, in_addr *v4_addr);
int Translate_v6_Packet(NAT_Manager::addr_mapping *, char *packet_v4, char *packet_v6, in_addr*, u16);
bool Translate_ICMPv6_code(u8 *type, u8 *code);
void Request_ARP(in_addr *addr, in_addr *v4_addr, const char *buffer, u32 buffer_size);
bool GetHardwareAddr_v4(const listen_interface **send, in_addr *dst, in_addr *src, char *packet, u32 size);
bool CheckMTU_v4(const char *packet_v4, u32 total_length, const listen_interface *i);
void SendICMP_MsgTooBig_v6(ip6_hdr *src, const listen_interface *i);

extern listen_interface 	*v6_interfaces;
extern pending_packet		*v6_pending_packets;
extern int					outbound_v4_socket;

#endif // __V4_HANDLER_H__
