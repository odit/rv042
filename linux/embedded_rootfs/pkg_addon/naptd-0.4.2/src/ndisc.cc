/***************************************************************************
 *  ndisc.cc : This file is part of 'ataga'
 *  created on: Thu Jun 16 16:40:25 CDT 2005
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

#include <errno.h>
#include <netinet/if_ether.h>
#include <net/route.h>
#include "ndisc.h"
#include "alg.h"
#include "v6_handler.h"

listen_interface 	*ndisc_interfaces;
ND_find 			*nd_request_list = 0;
nd_cache_entry		*nd_cache = 0;
v6_route_entry		*v6_routes = 0;
pthread_mutex_t 	v6_routes_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t 		ndisc_helper_thread_wait;

void *ndisc_handler_init(void *p)
{
	/* init stuff */
	ndisc_interfaces = (listen_interface*)p;
	listen_interface *n = ndisc_interfaces;
	
	while (n) {
		n->fd = initialise_network_int(n->name, ETH_P_ALL);
		get_int_hardware_address(n);
		get_int_mtu(n);
		n = n->next;
	}
	/* end of init stuff */
	
	/* main loop */
	while (true) {
		Get_Data_From_ND_Sockets();
		
		ND_find *i = nd_request_list;
		
		while (i) {
			if (i->packet->status != awaiting_ND_request) {
				Remove_ND_Request(i);
				i = nd_request_list;
			} else {
				if (i->packet->status == awaiting_ND_request)
					if (FindNeighbor(ndisc_interfaces, i))
						i->packet->status = ready_for_processing;
					else
						i->packet->status = ready_for_discarding;
				i = i->next;
			}
		}
		
		pthread_cond_init(&ndisc_helper_thread_wait, 0);
		pthread_mutex_t wait_mutex = PTHREAD_MUTEX_INITIALIZER;
		pthread_mutex_lock(&wait_mutex);
		pthread_cond_wait(&ndisc_helper_thread_wait, &wait_mutex);
		pthread_mutex_unlock(&wait_mutex);
	}
	
	clean_network_int(ndisc_interfaces);
	ClearNeighborCache(nd_cache);
	
	return 0;
}

bool Get_Data_From_ND_Sockets()
{
	fd_set read_set;
	int highest_fd = create_fd_set(ndisc_interfaces, &read_set);
	timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	
	if (!select(highest_fd + 1, &read_set, 0, 0, &tv))
		return false;
	
	int bytes_read(0);
	listen_interface *i = ndisc_interfaces;
	while (i) {
		if (FD_ISSET(i->fd, &read_set)) {
			/* something interesting ;) */
				
			char packet_buffer[packet_size];
			bytes_read = read(i->fd, packet_buffer, packet_size);
				
	    	if (bytes_read == -1) {
				pLog->write("Error while reading ND packet on %s interface: %s", i->name, strerror(errno));
				i = i->next;
				bytes_read = 0;
				continue;
			}
	
			ether_header *eth = (ether_header*)packet_buffer;
			
			if (memcmp(&eth->ether_dhost, &i->hw_addr, 6)) {
				i = i->next;
				continue;
			}
			
			if (eth->ether_type != htons(ETHERTYPE_IPV6)) {
				i = i->next;
				continue;
			}
			
			UpdateNDCacheEntry((in6_addr*)(packet_buffer + 22), packet_buffer + 6, i);
		}
		i = i->next;
	}
	
	return true;
}

bool Remove_ND_Request(ND_find *request)
{
	nd_request_list = request->next;
	delete request;
	
	return true;
}

bool Add_ND_Request(ND_find *request)
{
	if (!nd_request_list)
		nd_request_list = request;
	else {
		request->next = nd_request_list;
		nd_request_list = request;
	}
	
	pthread_cond_signal(&ndisc_helper_thread_wait);
	
	return true;
}

bool Add_ND_Request(in6_addr* v6_addr_lookup, in6_addr *src, pending_packet *packet)
{
	ND_find *nd_request = new ND_find;
	memcpy(&nd_request->lookup, v6_addr_lookup, sizeof(in6_addr));
	memcpy(&nd_request->source, src, sizeof(in6_addr));
	nd_request->packet = packet;
	nd_request->next = 0;
	
	return Add_ND_Request(nd_request);
}

bool GetCacheEntry(FILE *hFile, in6_addr *v6_addr, char *out)
{
	char xx[9];
	char entry[512];
	char hdw_addr[6];
	in6_addr v6_entry;
	
	while (fgets(entry, sizeof(entry), hFile)) {
		for (int i(0); i < 4; ++i) {
			strncpy(xx, entry + i * 8, 8);
			*((unsigned long *)&v6_entry + i * 4) = htonl(strtoul(xx, 0, 16));
		}
		memset(hdw_addr, 0, 6);
		strncpy(xx, entry + 96, 8);
		*((unsigned long *)hdw_addr) = htonl(strtoul(xx, 0, 16));
		strncpy(xx, entry + 104, 4);
		*((unsigned long *)hdw_addr + 4) = htons(strtoul(xx, 0, 16));
		
		if (IN6_ARE_ADDR_EQUAL(&v6_entry, v6_addr)) {
			memcpy(out, hdw_addr, 6);
			return true;
		}
	}
	return false;
}

bool GetNeighbor(const listen_interface **send, in6_addr *v6_addr, char *packet)
{
	/*
	FILE *hFile = fopen("/proc/net/ndisc", "r");
	
	if (!hFile) {
		pLog->write("Error opening ND cache (/proc/net/ndisc): %s", strerror(errno));
		return false;
	}
	
	ether_header *eth = (ether_header*) packet;
	bool return_value = GetCacheEntry(hFile, v6_addr, (char*)eth->ether_dhost);

	fclose(hFile);
	*/
	
	return FindCacheEntry_v6(send, v6_addr, packet);
}

/* bool Convertv6Addr(const char *src, char *dst) */
/* dst must be at least 16 bytes long */

bool Convertv6Addr(const char *src, u8 *dst) 
{
	char txt[3] = { 0, 0, 0 };
	
	for (int i(0); i < 32; i += 2) {
		memcpy(txt, src + i, 2);
		sscanf(txt, "%x", (u32*)dst);
	//	*dst = htons(*dst);
		++dst;
	}
	
	return true;
}

bool UpdateRouteCache_v6()
{
	ClearRoutes_v6();
	AddRoute_v6(0);
	FILE *hFile = fopen("/proc/net/ipv6_route", "r");
	if (!hFile)
		return false;
	
	char buff[4096], dst[33], src[32], gw[32], iface[16];
	int prefix_len, slen, metric, use, refcnt, iflags;
	
	while (fgets(buff, 4096, hFile)) {
		sscanf(buff, "%32s %02x %32s %02x %32s %08x %08x %08x %08x %s\n", 
			dst, &prefix_len, src, &slen, gw, &iflags, &use, &refcnt, &metric, iface);
		memcpy(dst, buff, 32);
		
		*(dst + 32) = 0;
		
		if (!strcmp(iface, "lo"))
			continue;
	
		if (prefix_len > 64)
			continue;
		
		listen_interface *i = v6_interfaces;
		while (i) {
			if (!strcmp(iface, i->name)) {
				v6_route_entry *entry = new v6_route_entry;
				Convertv6Addr(dst, (u8*)&entry->dst);
				
				if (strcmp("00000000000000000000000000000000", gw))
					Convertv6Addr(gw, (u8*)&entry->gw);
				else
					memset(&entry->gw, 0, sizeof(in6_addr));
				
				entry->mask = prefix_len;
				AddRoute_v6(entry);
				
				break;
			}
			i = i->next;
		}
	}
	
	fclose(hFile);
	
	return true;
}

bool ClearRoutes_v6()
{
	pthread_mutex_lock(&v6_routes_mutex);
	v6_route_entry *entry = v6_routes;
	while (entry) {
		v6_route_entry *save = entry;
		entry = save->next;
		
		delete save;
	}
	
	v6_routes = 0;
	pthread_mutex_unlock(&v6_routes_mutex);
	
	return true;
}

bool AddRoute_v6(v6_route_entry *entry)
{
	static v6_route_entry *current = 0;
	
	pthread_mutex_lock(&v6_routes_mutex);

	if (!entry) {
		current = 0;
		
		pthread_mutex_unlock(&v6_routes_mutex);
		return false;
	}
	
	entry->next = 0;
	
	if (!current) {
		v6_routes = entry;
		current = entry;
		
		pthread_mutex_unlock(&v6_routes_mutex);
		return false;
	}
	
	current->next = entry;
	current = entry;
	
	pthread_mutex_unlock(&v6_routes_mutex);
	
	return false;
}

bool FindRoute_v6(const listen_interface **send, in6_addr *gw, in6_addr *v6_addr, char *packet)
{
	pthread_mutex_lock(&v6_routes_mutex);
	v6_route_entry *entry = v6_routes;
	while (entry) {
		u8 rem_mask = (entry->mask & 0x07);
		u8 size = (entry->mask >> 3);
		
		if (!memcmp(v6_addr, &entry->dst, size) || !size) {
			if ((rem_mask & *((u8*)v6_addr + size)) == (rem_mask & *((u8*)&entry->dst + size))) {
				if (entry->gw.s6_addr[0] == 0) {
					pthread_mutex_unlock(&v6_routes_mutex);
					return false;
				}
					
				memcpy(gw, &entry->gw, sizeof(in6_addr));
				pthread_mutex_unlock(&v6_routes_mutex);
				return true;
			}
		}
		entry = entry->next;
	}
	pthread_mutex_unlock(&v6_routes_mutex);
	
	return false;
}

bool FindCacheEntry_v6(const listen_interface **send, in6_addr *v6_addr, char *packet)
{
	nd_cache_entry *i = nd_cache, *save = 0;
	u32 remove_time = time(0) - 600;
	while (i) {
		if (i->last_update < remove_time) {
			RemoveNeighborCacheEntry(i, save);
			i = nd_cache;
			save = 0;
			continue;
		}
		
		if (!memcmp(&i->addr, v6_addr, sizeof(in6_addr)))
			break;
		
		save = i;
		i = i->next;
	}
	
	if (!i)
		return false;
	
	i->last_update = time(0);
	memcpy(packet, i->hw_addr, 6);
	memcpy(packet + 6, i->i->hw_addr, 6);
	*(packet + 12) = 0x86;
	*(packet + 13) = 0xdd;
	*send = i->i;
	
	return true;
}

bool ClearNeighborCache(nd_cache_entry *nd)
{
	nd_cache_entry *i = nd;
	
	while (i) {
		nd_cache_entry *save = i;
		i = i->next;
		
		delete save;
	}
	nd_cache = 0;
	
	return true;
}

bool RemoveNeighborCacheEntry(nd_cache_entry *nd, nd_cache_entry *prev)
{
	if (prev)
		prev->next = nd->next;
	else
		nd_cache = nd->next;
	
	delete nd;
	
	return true;
}

bool UpdateNDCacheEntry(in6_addr *v6_addr, char *hw_addr, const listen_interface *inbound)
{
	nd_cache_entry *i = nd_cache, *save = 0;
	while (i) {
		if (!memcmp(&i->addr, v6_addr, sizeof(in6_addr)))
			break;
		
		save = i;
		i = i->next;
	}
	if (i) {
		if (!memcpy(&i->hw_addr, hw_addr, 6)) {
			i->last_update = time(0);
			return true;
		} else {
			RemoveNeighborCacheEntry(i, save);
			i = nd_cache;
			return false;
		}
	} else
		return AddNeighborCacheEntry(v6_addr, hw_addr, inbound);
}

bool AddNeighborCacheEntry(in6_addr *v6_addr, char *hw_addr, const listen_interface *i)
{
	nd_cache_entry *nd = new nd_cache_entry;
	
	memcpy(&nd->addr, v6_addr, sizeof(in6_addr));
	memcpy(&nd->hw_addr, hw_addr, 6);
	nd->last_update = time(0);
	nd->i = i;
	
	if (nd_cache)
		nd->next = nd_cache;
	else
		nd->next = 0;
	
	nd_cache = nd;
	
	return true;
}

bool CreateNeighborSolicitation(in6_addr *lookup, in6_addr *src, const char *hdw_src, char *buffer)
{
	memset(buffer, 0, nd_datagram_size);
	
	ether_header *eth = (ether_header*)buffer;
	char eth_broadcast[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
    
	memcpy(&eth->ether_dhost, eth_broadcast, 6);
	memcpy(&eth->ether_shost, hdw_src, 6);
	eth->ether_type = htons(ETHERTYPE_IPV6);
    
	ip6_hdr *ip6 = (ip6_hdr*)(buffer + sizeof(ether_header));
    
	ip6->ip6_flow = 0x60;
    ip6->ip6_plen = htons(0x20u);
    ip6->ip6_nxt = IPPROTO_ICMPV6;
    ip6->ip6_hops = 0xff;
    
	char solic_addr[16] = { 0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	                        0x00, 0x00, 0x00, 0x01, 0xff, 0x00, 0x00, 0x01 };
	
	memcpy(&ip6->ip6_src, src, 16);
    memcpy(&ip6->ip6_dst, solic_addr, 13);
    memcpy(((char*)&ip6->ip6_dst) + 13, ((char*)lookup) + 13, 3);
   
	//if (inet_ntop(AF_INET6, , pBufferIPin, INET6_ADDRSTRLEN) != NULL)
							
	icmp6_hdr *icmp6 = (icmp6_hdr*)((char*)ip6 + sizeof(ip6_hdr));
    
	icmp6->icmp6_type = ND_NEIGHBOR_SOLICIT;
	icmp6->icmp6_code = 0x00;
	icmp6->icmp6_cksum = 0x00;
    icmp6->icmp6_data32[0] = 0x00;
    
	memcpy(((char*)(icmp6)) + 8, lookup, 16);
    *(u8*)(((char*)(icmp6)) + 24) = (u8)0x01;
	*(u8*)(((char*)(icmp6)) + 25) = (u8)0x01;
	memcpy(((char*)(icmp6)) + 26, hdw_src, 6);
    
	char pseudo_buffer[40];
	IPv6_pseudo_hdr *pseudo_header = (IPv6_pseudo_hdr*) pseudo_buffer;

	memcpy(&pseudo_header->src_addr, &ip6->ip6_src, 16);
	memcpy(&pseudo_header->dst_addr, &ip6->ip6_dst, 16);
	memcpy(&pseudo_header->dgram_length, ((char *) &ip6->ip6_plen) - 2, 4);
	pseudo_header->next_hdr = htonl(ip6->ip6_nxt);

	icmp6->icmp6_cksum = compute_checksum_pseudo((u8*)pseudo_header, 40, (u8*)icmp6, 32);
    
	return true;
}

bool SendNeighborDiscovery(const listen_interface *i, const char *buffer, u32 size)
{
	if (write(i->fd, buffer, size) == -1) {
		pLog->write("Error while writing ND: %s", strerror(errno));
		return false;
	}
	return true;
}

bool GetNeighborAdvertisement(const listen_interface *i, u32 timeout, 
	const char *solicit, u32 size, char *packet)
{
	fd_set read_set;
	FD_ZERO(&read_set);
	FD_SET(i->fd, &read_set);
	
	timeval tv;
	
	tv.tv_sec = timeout;
	tv.tv_usec = 0;

select_data:
	
	if (select(i->fd + 1, &read_set, 0, 0, &tv) == -1) {
		pLog->write("Error while listening on ND interface %s: %s", i->name, strerror(errno));
		return false;
	}
	
	if (!FD_ISSET(i->fd, &read_set))
		return false;
	
	char read_buffer[nd_datagram_size];
	int bytes_read = read(i->fd, read_buffer, nd_datagram_size);
	
	if (bytes_read == -1) {
		pLog->write("Error while reading on ND interface %s: %s", i->name, strerror(errno));
		return false;
	}
	
	if ((u32)bytes_read != nd_datagram_size)
		goto select_data;
	
	ether_header *eth = (ether_header*)read_buffer;
	
	if (memcmp(&eth->ether_dhost, &i->hw_addr, 6))
		goto select_data;
	
	ip6_hdr *ip6 = (ip6_hdr*)(read_buffer + sizeof(ether_header));
	
	if (memcmp(solicit + 22, read_buffer + 38, 16))
		goto select_data;
	
	if (memcmp(read_buffer + 22, solicit + 62, 16))
		goto select_data;
	
    if (ip6->ip6_nxt != IPPROTO_ICMPV6)
		goto select_data;
	
	icmp6_hdr *icmp6 = (icmp6_hdr*)(read_buffer + sizeof(ether_header) + sizeof(ip6_hdr));
	
	if (icmp6->icmp6_type != ND_NEIGHBOR_ADVERT)
		goto select_data;
	
	if (*(read_buffer + 78) != 0x02)
		goto select_data;
	
	AddNeighborCacheEntry((in6_addr*)(read_buffer + 22), read_buffer + 6, i);
	
	return true;
}

bool FindNeighbor(const listen_interface *i, ND_find *nd_request)
{
	while (i) {
		char nd_datagram_buffer[nd_datagram_size];
		
		/* create ND solicitation */
		CreateNeighborSolicitation(&nd_request->lookup, &nd_request->source, 
			i->hw_addr, nd_datagram_buffer);
		
		/* send */
		for (u8 n(0); n < max_solicit_sends; ++n) {
			SendNeighborDiscovery(i, nd_datagram_buffer, nd_datagram_size);
			if (GetNeighborAdvertisement(i, solicit_timer, nd_datagram_buffer, 
					nd_datagram_size, nd_request->packet->packet_start))
				return true;
		}
		
		/* wait for reply */
		i = i->next;
	}
	return false;
}
