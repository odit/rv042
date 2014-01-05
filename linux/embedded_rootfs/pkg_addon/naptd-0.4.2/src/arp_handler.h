/***************************************************************************
 *  arp_handler.h : This file is part of 'ataga'
 *  created on: Wed Jun  8 10:58:10 CDT 2005
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
 
#ifndef __ARP_HANDLER_H__
#define __ARP_HANDLER_H__

#include <netinet/if_ether.h>
#include "ataga.h"

struct arp_cache_entry {
	in_addr addr;
	char hw_addr[6];
	const listen_interface *i;
	u32 last_update;
	arp_cache_entry *next;
};

struct v4_route_entry {
	in_addr dst;
	in_addr gw;
	int mask;
	v4_route_entry *next;
};

void *arp_handler_init(void*);
void *arp_helper_init(void*);
bool AddRoute_v4(v4_route_entry *entry);
int Get_Data_From_ARP_Sockets(char *read_buffer, listen_interface **i);
bool Get_Data_From_ARP_Sockets();
bool Remove_ARP_Request(ARP_find *request);
bool Add_ARP_Request(ARP_find *request);
bool Add_ARP_Request(in_addr* v4_addr_lookup, in_addr *src, pending_packet *packet);
bool GetARP(const listen_interface **send, in_addr *v4_addr, char *packet);
bool FindCacheEntry_v4(const listen_interface **send, in_addr *v4_addr, char *packet);
bool RemoveARPCacheEntry(arp_cache_entry *arp, arp_cache_entry *prev);
bool FindARP(const listen_interface *i, ARP_find *arp_request);
bool ClearARPCache(arp_cache_entry *arp);
bool ClearRoutes_v4();
bool UpdateRouteCache_v4();
bool AddARPCacheEntry(in6_addr *v6_addr, char *hw_addr, const listen_interface *i);
bool FindRoute_v4(const listen_interface **send, in_addr*, in_addr *v4_addr, char *packet);
bool UpdateARPCacheEntry(in_addr *v4_addr, char *hw_addr, const listen_interface *inbound);
bool CreateARPRequest(in_addr *lookup, in_addr *src, const char *hdw_src, char *buffer);
bool SendARPRequest(const listen_interface *i, const char *buffer, u32 size);
bool GetARPReply(const listen_interface *i, u32 timeout, const char *solicit, u32 size, char *packet);
	
const u32 arp_datagram_size = sizeof(ether_header) + 28;
const u32 arp_timer = 1;
const u32 max_arp_sends = 3;

extern listen_interface 	*arp_help_interfaces;
extern arp_cache_entry		*arp_cache;
extern v4_route_entry		*v4_routes;

#endif // __ARP_HANDLER_H__
