/***************************************************************************
 *  ndisc.h : This file is part of 'ataga'
 *  created on: Thu Jun 16 16:40:56 CDT 2005
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

#ifndef __NDISC_H_
#define __NDISC_H_

#include <netinet/ip6.h>
#include <netinet/icmp6.h>
#include "ataga.h"

struct nd_cache_entry {
	in6_addr addr;
	char hw_addr[6];
	const listen_interface *i;
	u32 last_update;
	nd_cache_entry *next;
};

struct v6_route_entry {
	in6_addr dst;
	in6_addr gw;
	u8 mask;
	v6_route_entry *next;
};

void *ndisc_handler_init(void *p);
bool UpdateRouteCache_v6();
bool ClearRoutes_v6();
bool AddRoute_v6(v6_route_entry *route);
bool FindRoute_v6(const listen_interface **send, in6_addr*, in6_addr *v6_addr, char *packet);
bool FindCacheEntry_v6(const listen_interface **send, in6_addr *v6_addr, char *packet);
bool Add_ND_Request(in6_addr*, in6_addr*, pending_packet *packet);
bool Add_ND_Request(ND_find*);
bool Remove_ND_Request(ND_find*);
bool FindNeighbor(const listen_interface*, ND_find*);
bool GetNeighbor(const listen_interface **, in6_addr*, char*);
bool GetCacheEntry(FILE *hFile, in6_addr *out);
bool CreateNeighborSolicitation(in6_addr*, in6_addr*, const char*, char*);
bool FindCacheEntry(in6_addr *v6_addr, char *packet);
bool AddNeighborCacheEntry(in6_addr*, char*, const listen_interface*);
bool ClearNeighborCache(nd_cache_entry *nd);
bool RemoveNeighborCacheEntry(nd_cache_entry *nd, nd_cache_entry *prev);
bool FindCacheEntry(const listen_interface **send, in6_addr *v6_addr, char *packet);
bool UpdateNDCacheEntry(in6_addr*, char*, const listen_interface*);
bool Get_Data_From_ND_Sockets();
bool Convertv6Addr(const char *src, u8 *dst);

const u32 nd_datagram_size = 86; /* options */
const u32 solicit_timer = 1;
const u32 max_solicit_sends = 3;

extern listen_interface 	*ndisc_interfaces;
extern nd_cache_entry 		*nd_cache;
extern v6_route_entry		*v6_routes;

#endif // __NDISC_H_
