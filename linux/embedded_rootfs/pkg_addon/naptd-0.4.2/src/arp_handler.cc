/***************************************************************************
 *  arp_handler.cc : This file is part of 'ataga'
 *  created on: Tue Jun  7 22:46:29 CDT 2005
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
#include <net/if_arp.h>
#include <net/route.h>
#include "arp_handler.h"
#include "v4_handler.h"

listen_interface 	*arp_help_interfaces;
listen_interface 	*arp_handler_interfaces;
ARP_find 			*arp_request_list = 0;
arp_cache_entry		*arp_cache = 0;
v4_route_entry		*v4_routes = 0;
pthread_mutex_t 	v4_routes_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t 		arp_helper_thread_wait;

void *arp_helper_init(void *p)
{
	/* init stuff */
	arp_help_interfaces = (listen_interface*)p;
	listen_interface *i = arp_help_interfaces;
	
	while (i) {
		i->fd = initialise_network_int(i->name, ETH_P_ALL);
		get_int_hardware_address(i);
		get_int_mtu(i);
		i = i->next;
	}
	/* end of init stuff */
	
	/* main loop */
	while (true) {
		Get_Data_From_ARP_Sockets();
		
		ARP_find *i = arp_request_list;
		
		while (i) {
			if (i->packet->status != awaiting_ARP_request) {
				Remove_ARP_Request(i);
				i = arp_request_list;
			} else {
				if (i->packet->status == awaiting_ARP_request)
					if (FindARP(arp_help_interfaces, i))
						i->packet->status = ready_for_processing;
					else
						i->packet->status = ready_for_discarding;
				i = i->next;
			}
		}
		
		pthread_cond_init(&arp_helper_thread_wait, 0);
		pthread_mutex_t wait_mutex = PTHREAD_MUTEX_INITIALIZER;
		pthread_mutex_lock(&wait_mutex);
		pthread_cond_wait(&arp_helper_thread_wait, &wait_mutex);
		pthread_mutex_unlock(&wait_mutex);
	}
	
	clean_network_int(arp_help_interfaces);
	ClearARPCache(arp_cache);
	
	return 0;
}

bool Get_Data_From_ARP_Sockets()
{
	fd_set read_set;
	int highest_fd = create_fd_set(arp_help_interfaces, &read_set);
	timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	
	if (!select(highest_fd + 1, &read_set, 0, 0, &tv))
		return false;
	
	int bytes_read(0);
	listen_interface *i = arp_help_interfaces;
	while (i) {
		if (FD_ISSET(i->fd, &read_set)) {
			/* something interesting ;) */
				
			char packet_buffer[packet_size];
			bytes_read = read(i->fd, packet_buffer, packet_size);
				
	    	if (bytes_read == -1) {
				pLog->write("Error while reading ARP packet on %s interface: %s", i->name, strerror(errno));
				i = i->next;
				bytes_read = 0;
				continue;
			}
	
			ether_header *eth = (ether_header*) packet_buffer;
		
			if (eth->ether_type != htons(ETHERTYPE_ARP)) {
				i = i->next;
				continue;
			}
			
			arphdr *arp = (arphdr*)(packet_buffer + sizeof(ether_header));
			
			if (arp->ar_hrd != htons(ARPHRD_ETHER)) {
				i = i->next;
				continue;
			}
		
			if (arp->ar_pro != htons(ETHERTYPE_IP)) {
				i = i->next;
				continue;
			}
			
			if (arp->ar_op != htons(ARPOP_REPLY)) {
				i = i->next;
				continue;
			}
			
			UpdateARPCacheEntry((in_addr*)(packet_buffer + 22), packet_buffer + 6, i);
		}
		i = i->next;
	}
	
	return true;
}

bool AddARPCacheEntry(in_addr *v4_addr, char *hw_addr, const listen_interface *i)
{
	arp_cache_entry *arp = new arp_cache_entry;
	
	memcpy(&arp->addr, v4_addr, sizeof(in_addr));
	memcpy(&arp->hw_addr, hw_addr, 6);
	arp->last_update = time(0);
	arp->i = i;
	
	if (arp_cache)
		arp->next = arp_cache;
	else
		arp->next = 0;
	
	arp_cache = arp;
	
	return true;
}

bool UpdateARPCacheEntry(in_addr *v4_addr, char *hw_addr, const listen_interface *inbound)
{
	arp_cache_entry *i = arp_cache, *save = 0;
	while (i) {
		if (!memcmp(&i->addr, v4_addr, sizeof(in_addr)))
			break;
		
		save = i;
		i = i->next;
	}
	if (i) {
		if (!memcpy(&i->hw_addr, hw_addr, 6)) {
			i->last_update = time(0);
			return true;
		} else {
			RemoveARPCacheEntry(i, save);
			i = arp_cache;
			return false;
		}
	} else
		return AddARPCacheEntry(v4_addr, hw_addr, inbound);
}

bool FindARP(const listen_interface *i, ARP_find *arp_request)
{
	while (i) {
		char arp_datagram_buffer[arp_datagram_size];
		
		/* create ARP request */
		CreateARPRequest(&arp_request->lookup, &arp_request->source, 
			i->hw_addr, arp_datagram_buffer);
		
		/* send */
		for (u8 n(0); n < max_arp_sends; ++n) {
			SendARPRequest(i, arp_datagram_buffer, arp_datagram_size);
			if (GetARPReply(i, arp_timer, arp_datagram_buffer, 
					arp_datagram_size, arp_request->packet->packet_start))
				return true;
		}
		
		/* wait for reply */
		i = i->next;
	}
	return false;
}

bool SendARPRequest(const listen_interface *i, const char *buffer, u32 size)
{
	if (write(i->fd, buffer, size) == -1) {
		pLog->write("Error while writing ARP request: %s", strerror(errno));
		return false;
	}
	return true;
}

bool CreateARPRequest(in_addr *lookup, in_addr *src, const char *hdw_src, char *buffer)
{
	memset(buffer, 0, arp_datagram_size);
	
	ether_header *eth = (ether_header*)buffer;
	arphdr *arp = (arphdr*)(buffer + sizeof(ether_header));
	
	char eth_broadcast[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
	
	memcpy(&eth->ether_dhost, eth_broadcast, 6);
	memcpy(&eth->ether_shost, hdw_src, 6);
	memcpy(((char*)(arp)) + 8, hdw_src, 6);
	memcpy(((char*)(arp)) + 14, src, 4);
	memcpy(((char*)(arp)) + 24, lookup, 4);
		    
	eth->ether_type = htons(ETHERTYPE_ARP);
	arp->ar_hrd = htons(ARPHRD_ETHER);
	arp->ar_pro = htons(ETHERTYPE_IP);
	arp->ar_hln = 0x06;	
	arp->ar_pln = 0x04;		
	arp->ar_op = htons(ARPOP_REQUEST);
	
	return true;
}

bool GetARPReply(const listen_interface *i, u32 timeout, 
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
		pLog->write("Error while listening on ARP interface %s: %s", i->name, strerror(errno));
		return false;
	}
	
	if (!FD_ISSET(i->fd, &read_set))
		return false;
	
	char read_buffer[arp_datagram_size];
	int bytes_read = read(i->fd, read_buffer, arp_datagram_size);
	
	if (bytes_read == -1) {
		pLog->write("Error while reading on ARP interface %s: %s", i->name, strerror(errno));
		return false;
	}
	
	if ((u32)bytes_read != arp_datagram_size)
		goto select_data;
	
	ether_header *eth = (ether_header*)read_buffer;
	
	if (eth->ether_type != htons(ETHERTYPE_ARP))
		goto select_data;
	
	arphdr *arp = (arphdr*)(read_buffer + sizeof(ether_header));
			
	if (arp->ar_hrd != htons(ARPHRD_ETHER))
		goto select_data;
		
	if (arp->ar_pro != htons(ETHERTYPE_IP)) 
		goto select_data;
			
	if (arp->ar_op != htons(ARPOP_REPLY))
		goto select_data;
	
	AddARPCacheEntry((in_addr*)(read_buffer + 28), read_buffer + 22, i);
	
	return true;
}

bool AddRoute_v4(v4_route_entry *entry)
{
	static v4_route_entry *current = 0;
	
	pthread_mutex_lock(&v4_routes_mutex);
	
	if (!entry) {
		current = 0;
		
		pthread_mutex_unlock(&v4_routes_mutex);
		return false;
	}
	
	entry->next = 0;
	
	if (!current) {
		v4_routes = entry;
		current = entry;
		
		pthread_mutex_unlock(&v4_routes_mutex);
		return false;
	}
	
	current->next = entry;
	current = entry;
	
	pthread_mutex_unlock(&v4_routes_mutex);
	
//	char buffer1[INET_ADDRSTRLEN], buffer2[INET_ADDRSTRLEN];
//	pLog->write("loading route %s/%u via %s", inet_ntop(AF_INET, &entry->dst, buffer1, INET_ADDRSTRLEN), 
//		entry->mask, entry->gw.s_addr ? inet_ntop(AF_INET, &entry->gw, buffer2, INET_ADDRSTRLEN) : "null");
	
	return false;
}

bool UpdateRouteCache_v4()
{
	ClearRoutes_v4();
	AddRoute_v4(0);
	FILE *hFile = fopen("/proc/net/route", "r");
	if (!hFile)
		return false;
	
	char buff[4096], iface[16];
	in_addr dst, gw;
	int mask, metric, use, refcnt, iflags;
	fgets(buff, 4096, hFile);
	while (fgets(buff, 4096, hFile)) {
		sscanf(buff, "%8s %16x %16x %08x %08x %08x %08x %08x\n", 
			iface, (u32*)&dst, (u32*)&gw, &iflags, &refcnt, &use, &metric, &mask);
		
		if (!strcmp(iface, "lo"))
			continue;
		
		if (!(iflags & RTF_UP))
	  		continue;
		
		listen_interface *i = v4_interfaces;
		while (i) {
			if (!strcmp(iface, i->name)) {
				v4_route_entry *entry = new v4_route_entry;
				memcpy(&entry->dst, &dst, sizeof(in_addr));
				
				if (iflags & RTF_GATEWAY)
					memcpy(&entry->gw, &gw, sizeof(in_addr));
				else
					entry->gw.s_addr = 0;
				
				entry->mask = mask;
				AddRoute_v4(entry);
				
				break;
			}
			i = i->next;
		}
	}
	
	fclose(hFile);
	
	return true;
}

bool ClearRoutes_v4()
{
	pthread_mutex_lock(&v4_routes_mutex);
	v4_route_entry *entry = v4_routes;
	while (entry) {
		v4_route_entry *save = entry;
		entry = save->next;
		
		delete save;
	}
	
	v4_routes = 0;
	
	pthread_mutex_unlock(&v4_routes_mutex);
	
	return true;
}

bool ClearARPCache(arp_cache_entry *arp)
{
	arp_cache_entry *i = arp;
	
	while (i) {
		arp_cache_entry *save = i;
		i = i->next;
		
		delete save;
	}
	arp_cache = 0;
	
	return true;
}
	
bool Remove_ARP_Request(ARP_find *request)
{
	arp_request_list = request->next;
	delete request;
	
	return true;
}

bool Add_ARP_Request(ARP_find *request)
{
	if (!arp_request_list)
		arp_request_list = request;
	else {
		request->next = arp_request_list;
		arp_request_list = request;
	}
	
	pthread_cond_signal(&arp_helper_thread_wait);
	
	return true;
}

bool Add_ARP_Request(in_addr* v4_addr_lookup, in_addr *src, pending_packet *packet)
{
	ARP_find *arp_request = new ARP_find;
	memcpy(&arp_request->lookup, v4_addr_lookup, sizeof(in_addr));
	memcpy(&arp_request->source, src, sizeof(in_addr));
	arp_request->packet = packet;
	arp_request->next = 0;
	
	return Add_ARP_Request(arp_request);
}

bool GetARP(const listen_interface **send, in_addr *v4_addr, char *packet)
{
	return FindCacheEntry_v4(send, v4_addr, packet);
}

bool FindRoute_v4(const listen_interface **send, in_addr *gw, in_addr *v4_addr, char *packet)
{
	pthread_mutex_lock(&v4_routes_mutex);
	v4_route_entry *entry = v4_routes;
	while (entry) {
//		char buffer1[INET_ADDRSTRLEN], buffer2[INET_ADDRSTRLEN];
//		in_addr a1, a2;
//		a1.s_addr = (v4_addr->s_addr & entry->mask);
//		a2.s_addr = (entry->dst.s_addr & entry->mask);
		
//		pLog->write("looking for route for %s", inet_ntop(AF_INET, &v4_addr->s_addr, buffer1, INET_ADDRSTRLEN));
//		pLog->write("comp %s %s", inet_ntop(AF_INET, &a1, buffer1, INET_ADDRSTRLEN), inet_ntop(AF_INET, &a2, buffer2, INET_ADDRSTRLEN));
	
		if ((v4_addr->s_addr & entry->mask) == (entry->dst.s_addr & entry->mask)) {
			if (!entry->gw.s_addr) {
				pthread_mutex_unlock(&v4_routes_mutex);
				return false;
			}
			
			memcpy(gw, &entry->gw, sizeof(in_addr));
			pthread_mutex_unlock(&v4_routes_mutex);
			return true;
		}
		
		entry = entry->next;
	}
	pthread_mutex_unlock(&v4_routes_mutex);
	
	return false;
}

bool FindCacheEntry_v4(const listen_interface **send, in_addr *v4_addr, char *packet)
{
	arp_cache_entry *i = arp_cache, *save = 0;
	u32 remove_time = time(0) - 600;
	while (i) {
		if (i->last_update < remove_time) {
			RemoveARPCacheEntry(i, save);
			i = arp_cache;
			save = 0;
			continue;
		}
		
		if (!memcmp(&i->addr, v4_addr, sizeof(in_addr)))
			break;
		
		save = i;
		i = i->next;
	}
	
	if (!i)
		return false;
	
	i->last_update = time(0);
	memcpy(packet, i->hw_addr, 6);
	memcpy(packet + 6, i->i->hw_addr, 6);
	*(packet + 12) = 0x08;
	*(packet + 13) = 0x00;
	*send = i->i;
	
	return true;
}

bool RemoveARPCacheEntry(arp_cache_entry *arp, arp_cache_entry *prev)
{
	if (prev)
		prev->next = arp->next;
	else
		arp_cache = arp->next;
	
	delete arp;
	
	return true;
}

void *arp_handler_init(void *p)
{
	/* init stuff */
	arp_handler_interfaces = (listen_interface*)p;
	listen_interface *i = arp_handler_interfaces;
	
	while (i) {
		i->fd = initialise_network_int(i->name, ETH_P_ALL);
		get_int_hardware_address(i);
		//get_int_mtu(i);
		i = i->next;
	}
	/* end of init stuff */
	
	char read_buffer[packet_size];
	int bytes_read;
	
	bool working(true);
	
	while (working) {
		listen_interface *arp_incoming;
		memset(read_buffer, 0, packet_size);
		bytes_read = Get_Data_From_ARP_Sockets(read_buffer, &arp_incoming);
		
		if (!bytes_read)
			continue;
		
		ether_header *eth = (ether_header*)read_buffer;
		arphdr *arp = (arphdr*)(read_buffer + sizeof(ether_header));
		    
		char arp_reply[arp_datagram_size];
		memset(arp_reply, 0, arp_datagram_size);
		
		ether_header *eth_rep = (ether_header*)arp_reply;
		arphdr *arp_rep = (arphdr*)(arp_reply + sizeof(ether_header));
	
		memcpy(&eth_rep->ether_dhost, &eth->ether_shost, 6);
		memcpy(((char*)(arp_rep)) + 18, &eth->ether_shost, 6);
		memcpy(&eth_rep->ether_shost, &arp_incoming->hw_addr, 6);
		memcpy(((char*)(arp_rep)) + 8, &arp_incoming->hw_addr, 6);
		memcpy(((char*)(arp_rep)) + 14, ((char*)(arp)) + 24, 4);
		memcpy(((char*)(arp_rep)) + 24, ((char*)(arp)) + 14, 4);
		    
		eth_rep->ether_type = htons(ETHERTYPE_ARP);
		arp_rep->ar_hrd = htons(ARPHRD_ETHER);
		arp_rep->ar_pro = htons(ETHERTYPE_IP);
		arp_rep->ar_hln = 0x06;	
		arp_rep->ar_pln = 0x04;		
		arp_rep->ar_op = htons(ARPOP_REPLY);
		
		write(arp_incoming->fd, arp_reply, arp_datagram_size);
	}
	
	clean_network_int(arp_handler_interfaces);
	
	return 0;
}

int Get_Data_From_ARP_Sockets(char *read_buffer, listen_interface **sock)
{
	fd_set read_set;
	int highest_fd = create_fd_set(arp_handler_interfaces, &read_set);
	while (!select(highest_fd + 1, &read_set, 0, 0, 0)) {
		pLog->write("Error while listening on ARP interfaces: %s", strerror(errno));
		continue;
	}
	
	int bytes_read(0);
	listen_interface *i = arp_handler_interfaces;
	while (i) {
		if (FD_ISSET(i->fd, &read_set)) {
			/* something interesting ;) */
				
			char *packet_buffer = read_buffer;
			bytes_read = read(i->fd, packet_buffer, packet_size);
				
	    	if (bytes_read == -1) {
				pLog->write("Error while reading ARP packet on %s interface: %s", i->name, strerror(errno));
				i = i->next;
				bytes_read = 0;
				continue;
			}
			ether_header *eth = (ether_header*) packet_buffer;
			char eth_broadcast[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
			
			if (memcmp(&eth->ether_dhost, &eth_broadcast, ETH_ALEN)) {
				i = i->next;
				continue;
			}
		
			if (eth->ether_type != htons(ETHERTYPE_ARP)) {
				i = i->next;
				continue;
			}
			
			arphdr *arp = (arphdr*)(packet_buffer + sizeof(ether_header));
			
			if (arp->ar_hrd != htons(ARPHRD_ETHER)) {
				i = i->next;
				continue;
			}
		
			if (arp->ar_pro != htons(ETHERTYPE_IP)) {
				i = i->next;
				continue;
			}
			
			if (arp->ar_op != htons(ARPOP_REQUEST)) {
				i = i->next;
				continue;
			}

			if (!pNat->In_Pool((in_addr*)((char*)arp + 24))) {
				i = i->next;
				continue;
			}
			
			*sock = i;
	
			return bytes_read;
		}
		i = i->next;
	}
	
	return 0;
}
