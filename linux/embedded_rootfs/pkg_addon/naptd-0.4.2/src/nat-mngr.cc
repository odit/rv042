/***************************************************************************
 *  nat-mngr.cc : This file is part of 'ataga'
 *  created on: Mon Jun  6 14:47:52 CDT 2005
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

#include <pthread.h>
#include "ataga.h"
#include "nat-mngr.h"

#ifdef _DEBUG_

#include <iostream>
using namespace std;

#endif // _DEBUG_

//NAT_Manager::pool_mini

NAT_Manager::NAT_Manager() : total_tcp_mappings(0), free_tcp_mappings(0), 
total_udp_mappings(0), free_udp_mappings(0), total_icmp_mappings(0), 
free_icmp_mappings(0), translation_time_tcp(86400), translation_time_udp(3600), 
translation_time_icmp(30)
{
	tcp_pool = 0;
	udp_pool = 0;
	icmp_pool = 0;
	first_priv_ip = 0;
	first_tcp_mapping = 0;
	first_udp_mapping = 0;
	first_icmp_mapping = 0;
	first_frag = 0;
	tcp_range = 0;
	udp_range = 0;
	icmp_range = 0;
	pthread_mutex_init(&mapping_mutex, 0);
	pthread_mutex_init(&v4_lookup_mutex, 0);
	pthread_mutex_init(&v6_lookup_mutex, 0);
}

NAT_Manager::~NAT_Manager()
{
	Drop_Pool();
}

bool NAT_Manager::Get_Translation_Times(u32 *tcp, u32 *udp, u32 *icmp)
{
	*tcp = translation_time_tcp;
	*udp = translation_time_udp;
	*icmp = translation_time_icmp;
	
	return true;
}

bool NAT_Manager::Set_Translation_Times(u32 tcp, u32 udp, u32 icmp)
{
	translation_time_tcp = tcp;
	translation_time_udp = udp;
	translation_time_icmp = icmp;
	
	return true;
}

u32 NAT_Manager::Get_Pool_Size() 
{
	return (total_tcp_mappings + total_udp_mappings + total_icmp_mappings);
}

u32 NAT_Manager::Get_Remaining_Pool_Size()
{
	return (free_tcp_mappings + free_udp_mappings + free_icmp_mappings);
}

NAT_Manager::addr_mapping *NAT_Manager::Get_v4_to_v6_Translation(in_addr *addr, 
	u16 port, v6_addr *out, protocol_type prot)
{
	addr_mapping *base;
	if (prot == TCP)
		base = first_tcp_mapping;
	else if (prot == UDP)
		base = first_udp_mapping;
	else 
		base = first_icmp_mapping;
	
	pthread_mutex_lock(&v4_lookup_mutex);
	addr_mapping *i = base;
	in_addr cmp_addr;
	cmp_addr.s_addr = ntohl(addr->s_addr);
	port = htons(port);
	while (i) {
		if (i->v4_addr->addr->addr.s_addr == cmp_addr.s_addr && 
				(i->v4_addr->port->port == port || !i->v4_addr->port->port)) {
			if (!(i->flags & addr_mapping::ready_to_remove))
				i->last_active = time(0);					

			memcpy(&out->addr, &i->v6_addr, sizeof(out->addr));
			out->port = i->port;
			pthread_mutex_unlock(&v4_lookup_mutex);
			
			return i;
		}
		i = i->next;
	}
	pthread_mutex_unlock(&v4_lookup_mutex);
	
	/* implemented privileaged IP range below */
	
	bool found(false);
	
	pool_ip *ip = first_priv_ip;
	//addr->s_addr = htonl(addr->s_addr);
	while (ip) {
		if (ip->addr.s_addr == addr->s_addr) {
			found = true;
			break;
		}
		ip = ip->next_free_ip;
	}
	
	return (NAT_Manager::addr_mapping*)found;
}

NAT_Manager::addr_mapping *NAT_Manager::Get_v6_to_v4_Translation(in6_addr *addr, 
	u16 port, v4_addr *out, bool init_new, protocol_type prot)
{
	addr_mapping **base;
	if (prot == TCP)
		base = &first_tcp_mapping;
	else if (prot == UDP)
		base = &first_udp_mapping;
	else 
		base = &first_icmp_mapping;
	
	pthread_mutex_lock(&v6_lookup_mutex);
	addr_mapping *i = *base;
	while (i) {
		if (!memcmp(&i->v6_addr, addr, sizeof(in6_addr))) {
			if (i->port == port || !i->port) {
				if (!(i->flags & addr_mapping::ready_to_remove))
					i->last_active = time(0);
				
				out->addr.s_addr = htonl(i->v4_addr->addr->addr.s_addr);
				out->port = i->v4_addr->port->port;
				pthread_mutex_unlock(&v6_lookup_mutex);
				
				return i;
			}
		}
		i = i->next;
	}
	pthread_mutex_unlock(&v6_lookup_mutex);
	
	if (!init_new)
		return 0;

	if (!port)
		return 0;
	
	if (prot == TCP) {
		if (!free_tcp_mappings)
			Increment_Pool(TCP);
	} else if (prot == UDP) {
		if (!free_udp_mappings)
			Increment_Pool(UDP);
	} else {
		if (!free_icmp_mappings)
			Increment_Pool(ICMP);
	}
	
	pool_pair p = Get_Free_Pair(prot);
	
	if (!p.addr)
		return 0;
	
	out->addr.s_addr = htonl(p.addr->addr.s_addr);
	out->port = p.port->port;
	
	pool_pair *pair = new pool_pair;
	memcpy(pair, &p, sizeof(pool_pair));
	addr_mapping *mapping = new addr_mapping;
	mapping->v6_addr = *addr;
	mapping->port = port;
	mapping->last_active = time(0);
	mapping->v4_addr = pair;
	
	Insert_Mapping(mapping, base, prot);
	
	return mapping;
}

void NAT_Manager::Insert_Mapping(addr_mapping *mapping, addr_mapping **base, protocol_type prot)
{
	if (prot == TCP) {
		--free_tcp_mappings;
		++global_stats.tcp_translations_created;
	} else if (prot == UDP) {
		--free_udp_mappings;
		++global_stats.udp_translations_created;
	} else {
		--free_icmp_mappings;
		++global_stats.icmp_translations_created;
	}
	
	pthread_mutex_lock(&mapping_mutex);
	pthread_mutex_lock(&v4_lookup_mutex);
	pthread_mutex_lock(&v6_lookup_mutex);
	
	mapping->prev = 0;
	mapping->next = *base;
	
	if (*base)
		(*base)->prev = mapping;
	
	*base = mapping;
	
	pthread_mutex_unlock(&v6_lookup_mutex);
	pthread_mutex_unlock(&v4_lookup_mutex);
	pthread_mutex_unlock(&mapping_mutex);
}

void NAT_Manager::Remove_Mapping(addr_mapping *mapping, addr_mapping **base, protocol_type prot)
{	
	if (!(mapping->flags & addr_mapping::static_m)) {
		if (prot == TCP)
			++free_tcp_mappings;
		else if (prot == UDP)
			++free_udp_mappings;
		else 
			++free_icmp_mappings;
	}
	
	pthread_mutex_lock(&mapping_mutex);
	pthread_mutex_lock(&v4_lookup_mutex);
	pthread_mutex_lock(&v6_lookup_mutex);

	if (mapping->prev)
		mapping->prev->next = mapping->next;
	
	if (mapping->next) {
		mapping->next->prev = mapping->prev;
		if (!mapping->prev)
			*base = mapping->next;
	} else if (!mapping->prev) {
		*base = 0;
	}
	
	pool_ip **pool_base;
	if (prot == TCP)
		pool_base = &tcp_pool;
	else if (prot == UDP)
		pool_base = &udp_pool;
	else 
		pool_base = &icmp_pool;
	
	if (mapping->flags & addr_mapping::static_m) {
		delete mapping->v4_addr->addr;
		delete mapping->v4_addr->port;
		delete mapping->v4_addr;
	} else
		Free_Pair(mapping->v4_addr, pool_base);
	
	delete mapping;
	
	pthread_mutex_unlock(&v6_lookup_mutex);	
	pthread_mutex_unlock(&v4_lookup_mutex);	
	pthread_mutex_unlock(&mapping_mutex);	
}

int NAT_Manager::Clean_Pool()
{
	/* clean TCP mappings, default timeout: 24 hours		*/
	addr_mapping *i = first_tcp_mapping;
	u32 remove_time(time(0) - translation_time_tcp);
	while (i) {
		if (!(i->flags & addr_mapping::static_m) && i->last_active < remove_time) {
			Remove_Mapping(i, &first_tcp_mapping, TCP);
			i = first_tcp_mapping;
			continue;
		}
		i = i->next;
	}
	
	/* clean UDP mappings, default timeout: 1 hour			*/
	i = first_udp_mapping;
	remove_time = time(0) - translation_time_udp;
	while (i) {
		if (!(i->flags & addr_mapping::static_m) && i->last_active < remove_time) {
			Remove_Mapping(i, &first_udp_mapping, UDP);
			i = first_udp_mapping;
			continue;
		}
		i = i->next;
	}
	
	/* clean ICMP mappings, default timeout: 30 seconds		*/
	i = first_icmp_mapping;
	remove_time = time(0) - translation_time_icmp;
	while (i) {
		if (!(i->flags & addr_mapping::static_m) && i->last_active < remove_time) {
			Remove_Mapping(i, &first_icmp_mapping, ICMP);
			i = first_icmp_mapping;
			continue;
		}
		i = i->next;
	}
	
	return 0;
}

/*																				*/
/*	int NAT_Manager::Prep_Remove_Mapping(addr_mapping *mapping)					*/
/*																				*/	
/* 	We don't really remove the connection just set its timer to expire in one 	*/
/*	minute. 																	*/
/* 	This is only done for TCP sessions as there is really no way of telling 	*/
/*	when an UDNP session has ended.												*/
/* 	This way the TCP session will be automatically removed when clearing 		*/
/*	expired sessions. 															*/
/*	This is done this way, because there is no guarantee that the FIN packet 	*/
/*	that triggered this function call was really the last packet in the stream.	*/
/* 	Ultimately, every TCP session will end with a RST, FIN or will simply 		*/
/*	timeout after 24 hours.														*/
/* 	The last situation is the worst possible scenario because it will waste one */
/*	mapping that could otherwise be used.										*/
/*																				*/
/*	by default:																	*/
/*		TCP mappings are removed after 24 hours of inactivity 					*/
/* 		UDP mappings are removed after 1 hour of inactivity 					*/
/* 		ICMP mappings are removed after 30s of inactivity 						*/

int NAT_Manager::Prep_Remove_Mapping(addr_mapping *mapping)
{
	if (mapping->flags & addr_mapping::static_m)
		return 0;
	
	mapping->last_active = time(0) - (translation_time_tcp - 300);
	mapping->flags |= addr_mapping::ready_to_remove;
	// this is for TCP, as more packets maybe necessary to close a stream
	// UDP and ICMP don't require anything like this
	
	return 0;
}

int NAT_Manager::Add_Static_v4_to_v6_Mapping(in_addr v4, in6_addr v6)
{
	pool_pair *pair;
	addr_mapping *mapping;
	
	mapping = new addr_mapping(addr_mapping::static_m);
	pair = new pool_pair;
	pair->addr = new pool_ip;
	pair->addr->addr.s_addr = htonl(v4.s_addr);
	pair->addr->next_free_ip = 0;
	pair->addr->next_free_port = 0;
	pair->port = new pool_port;
	pair->port->port = 0;
	
	mapping->v4_addr = pair;
	mapping->port = 0;
	
	memcpy(&mapping->v6_addr, &v6, sizeof(mapping->v6_addr));
	++free_tcp_mappings;
	Insert_Mapping(mapping, &first_tcp_mapping, TCP);
	
	mapping = new addr_mapping(addr_mapping::static_m);
	pair = new pool_pair;
	pair->addr = new pool_ip;
	pair->addr->addr.s_addr = htonl(v4.s_addr);
	pair->addr->next_free_ip = 0;
	pair->addr->next_free_port = 0;
	pair->port = new pool_port;
	pair->port->port = 0;
	
	mapping->v4_addr = pair;
	mapping->port = 0;
	
	memcpy(&mapping->v6_addr, &v6, sizeof(mapping->v6_addr));
	++free_udp_mappings;
	Insert_Mapping(mapping, &first_udp_mapping, UDP);
	
	mapping = new addr_mapping(addr_mapping::static_m);
	pair = new pool_pair;
	pair->addr = new pool_ip;
	pair->addr->addr.s_addr = htonl(v4.s_addr);
	pair->addr->next_free_ip = 0;
	pair->addr->next_free_port = 0;
	pair->port = new pool_port;
	pair->port->port = 0;
	
	mapping->v4_addr = pair;
	mapping->port = 0;
	
	memcpy(&mapping->v6_addr, &v6, sizeof(mapping->v6_addr));
	++free_icmp_mappings;
	Insert_Mapping(mapping, &first_icmp_mapping, ICMP);
	
	return 0;
}

bool NAT_Manager::In_Pool(in_addr *addr)
{
	addr_mapping *ip = first_tcp_mapping;
	while (ip) {
		if (ip->v4_addr->addr->addr.s_addr == htonl(addr->s_addr))
			return true;
		
		ip = ip->next;
	}
	
	ip = first_udp_mapping;
	while (ip) {
		if (ip->v4_addr->addr->addr.s_addr == htonl(addr->s_addr))
			return true;
		
		ip = ip->next;
	}
	
	ip = first_icmp_mapping;
	while (ip) {
		if (ip->v4_addr->addr->addr.s_addr == htonl(addr->s_addr))
			return true;
		
		ip = ip->next;
	}
	
	pool_ip *priv = first_priv_ip;
	while (priv) {
		if (priv->addr.s_addr == addr->s_addr)
			return true;
		
		priv = priv->next_free_ip;
	}
	
	return false;
}

int NAT_Manager::Add_Priv_Pool(in_addr start_ip, in_addr end_ip)
{
	pool_ip *save = new pool_ip;
	pool_ip *next = save;
	pool_ip *ip;
	end_ip.s_addr = ntohl(end_ip.s_addr);
	for (u32 i = ntohl(start_ip.s_addr); i <= end_ip.s_addr; ++i) {
		if (i != end_ip.s_addr)
			next->next_free_ip = new pool_ip;
		else
			next->next_free_ip = 0;
		
		next->addr.s_addr = htonl(i);
		
		if (i != end_ip.s_addr) {
			ip = next->next_free_ip;
			next = ip;
		}
	}
	if (!first_priv_ip)
		first_priv_ip = save;
	else {
		next->next_free_ip = first_priv_ip;
		first_priv_ip = save;
	}
	
	return 0;
}

void NAT_Manager::Add_Pool_Range(pool_range *range, pool_range **base)
{
	if (*base)
		range->next = *base;
	else
		range->next = 0;
	
	*base = range;
}

int NAT_Manager::Add_Pool(in_addr start_ip, in_addr end_ip, u16 start_port, u16 end_port)
{
	pool_range *new_tcp_range = new pool_range;
	pool_range *new_udp_range = new pool_range;
	pool_range *new_icmp_range = new pool_range;
	
	memcpy(&new_tcp_range->begin_range, &start_ip, sizeof(in_addr));
	memcpy(&new_tcp_range->end_range, &end_ip, sizeof(in_addr));
	new_tcp_range->begin_port = start_port;
	new_tcp_range->end_port = end_port;
	new_tcp_range->next = 0;
	
	memcpy(new_udp_range, new_tcp_range, sizeof(pool_range));
	memcpy(new_icmp_range, new_tcp_range, sizeof(pool_range));
	
	Add_Pool_Range(new_tcp_range, &tcp_range);
	Add_Pool_Range(new_udp_range, &udp_range);
	Add_Pool_Range(new_icmp_range, &icmp_range);
	
	return 0;
}

NAT_Manager::pool_range *NAT_Manager::Get_Pool_Range(protocol_type prot)
{
	if (prot == TCP) {
		return tcp_range;
	} else if (prot == UDP) {
		return udp_range;
	} else {
		return icmp_range;
	}
}

void NAT_Manager::Free_Pool_Range(pool_range *range, protocol_type prot)
{
	pool_range **base;
	if (prot == TCP) {
		base = &tcp_range;
	} else if (prot == UDP) {
		base = &udp_range;
	} else {
		base = &icmp_range;
	}
	assert(range == *base); // this should always be true as we get pool ranges
							// with Get_Pool_Range(...)
	*base = range->next;
	
	delete range;
}

/* 																			*/
/*	int NAT_Manager::Increment_Pool(protocol_type prot);					*/
/*																			*/
/*	return value: number of available mappings added to the pool			*/
/*																			*/

int NAT_Manager::Increment_Pool(protocol_type prot)
{
	pool_range *range = Get_Pool_Range(prot);
	
	pool_ip **base;
	u32	*total_mappings;
	u32	*free_mappings;
	if (prot == TCP) {
		base = &tcp_pool;
		total_mappings = &total_tcp_mappings;
		free_mappings = &free_tcp_mappings;
	} else if (prot == UDP) {
		base = &udp_pool;
		total_mappings = &total_udp_mappings;
		free_mappings = &free_udp_mappings;
	} else {
		base = &icmp_pool;
		total_mappings = &total_icmp_mappings;
		free_mappings = &free_icmp_mappings;
	}
	
	if (!range) {
		pLog->write("Unable to increment pool for protocol %i translations. %i free translations remain.\n", 
			prot, *free_mappings);
		return 0;
	}

	pool_ip *save = new pool_ip;
	pool_ip *next = save;
	in_addr end_ip;
	end_ip.s_addr = ntohl(range->end_range.s_addr);
	
	u32 added_to_pool(0);
	
	for (u32 i = ntohl(range->begin_range.s_addr); i <= end_ip.s_addr; ++i) {
		if (i != end_ip.s_addr && 
				(*free_mappings + (range->end_port - range->begin_port) + 1 < pool_minimun))
			next->next_free_ip = new pool_ip;
		
		next->addr.s_addr = i;
		
		pool_port *save_port = new pool_port;
		pool_port *next_port = save_port;
		pool_port *port;
		
		++(*total_mappings);
		++(*free_mappings);
		++added_to_pool;
		
		for (u16 p = range->begin_port; p <= range->end_port; ++p) {
			if (p != range->end_port) {
				next_port->next = new pool_port;
				
				++(*total_mappings);
				++(*free_mappings);
				++added_to_pool;
			}
			
			next_port->port = p;
			
			if (p != range->end_port) {
				port = next_port->next;
				next_port = port;
			}
		}
		
		next_port->next = save_port;
		next->next_free_port = save_port;
		
		if (i == end_ip.s_addr) {
			Free_Pool_Range(range, prot);
		} else if (*free_mappings >= pool_minimun) {
			++i;
			range->begin_range.s_addr = ntohl(i);
			break;
		}
		
		if (i != end_ip.s_addr) {
			pool_ip *ip = next->next_free_ip;
			next = ip;
		}
	}
	
	if (!(*base)) {
		next->next_free_ip = save;
		*base = save;
	} else {
		next->next_free_ip = (*base)->next_free_ip;
		(*base)->next_free_ip = save;
	}	
	
	return added_to_pool;
}

bool NAT_Manager::FindFrag(u16 id, u16 *src_port, u16 *dst_port) 
{
	frag_info *frag = first_frag;
	
	while (frag) {
		if (frag->id == id) {
			*src_port = frag->src_port;
			*dst_port = frag->dst_port;
			frag->time = time(0);
			
			return true;
		}
		frag = frag->next;
	}
	
	return false;
}

bool NAT_Manager::AddFrag(u16 id, u16 src_port, u16 dst_port)
{
	frag_info *frag = new frag_info;
	
	frag->id = id;
	frag->time = time(0);
	frag->src_port = src_port;
	frag->dst_port = dst_port;
	
	frag->prev = 0;
	
	if (first_frag) {
		first_frag->prev = frag;
	}
	
	frag->next = first_frag;
	first_frag = frag;
	
	return true;
}

bool NAT_Manager::DelFrag(u16 id)
{
	frag_info *frag = first_frag;
	
	while (frag) {
		if (frag->id == id) {
			
			if (frag->prev)
				frag->prev->next = frag->next;
			else
				first_frag = frag->next;
			
			if (frag->next)
				frag->next->prev = frag->prev;
			
			return true;
		}
		frag = frag->next;
	}
	
	return false;
}

int NAT_Manager::Drop_Pool()
{
	addr_mapping *i = first_tcp_mapping;
	while (i) {
		addr_mapping *save = i;
		i = save->next;
		Remove_Mapping(save, &first_tcp_mapping, TCP);
		i = first_tcp_mapping;
	}
	
	i = first_udp_mapping;
	while (i) {
		addr_mapping *save = i;
		i = save->next;
		Remove_Mapping(save, &first_udp_mapping, UDP);
		i = first_udp_mapping;
	}
	
	i = first_icmp_mapping;
	while (i) {
		addr_mapping *save = i;
		i = save->next;
		Remove_Mapping(save, &first_icmp_mapping, ICMP);
		i = first_icmp_mapping;
	}
	
	pool_range *range_i = tcp_range;
	while (range_i) {
		pool_range *save = range_i;
		range_i = save->next;
		delete save;
	}
	
	range_i = udp_range;
	while (range_i) {
		pool_range *save = range_i;
		range_i = save->next;
		delete save;
	}
	
	range_i = icmp_range;
	while (range_i) {
		pool_range *save = range_i;
		range_i = save->next;
		delete save;
	}
	
	pool_ip *ip_i = tcp_pool;
	pool_ip *save_ip_i = ip_i;
	while (ip_i) {
		pool_port *port_i = ip_i->next_free_port;
		pool_port *save_i = port_i;
		while (port_i) {
			pool_port *save_port = port_i;
			port_i = port_i->next;
			if (port_i == save_i)
				port_i = 0;
			
			delete save_port;
		}
		pool_ip *save_ip = ip_i;
		ip_i = ip_i->next_free_ip;
		if (ip_i == save_ip_i)
			ip_i = 0;
		
		delete save_ip;
	}
	
	ip_i = udp_pool;
	save_ip_i = ip_i;
	while (ip_i) {
		pool_port *port_i = ip_i->next_free_port;
		pool_port *save_i = port_i;
		while (port_i) {
			pool_port *save_port = port_i;
			port_i = port_i->next;
			if (port_i == save_i)
				port_i = 0;
			
			delete save_port;
		}
		pool_ip *save_ip = ip_i;
		ip_i = ip_i->next_free_ip;
		if (ip_i == save_ip_i)
			ip_i = 0;
		
		delete save_ip;
	}
	
	ip_i = icmp_pool;
	save_ip_i = ip_i;
	while (ip_i) {
		pool_port *port_i = ip_i->next_free_port;
		pool_port *save_i = port_i;
		while (port_i) {
			pool_port *save_port = port_i;
			port_i = port_i->next;
			if (port_i == save_i)
				port_i = 0;
			
			delete save_port;
		}
		pool_ip *save_ip = ip_i;
		ip_i = ip_i->next_free_ip;
		if (ip_i == save_ip_i)
			ip_i = 0;
		
		delete save_ip;
	}
	
	frag_info *frag = first_frag;
	while (frag) {
		frag_info *save = frag;
		frag = frag->next;
		delete save;
	}
	
	first_frag = 0;
	
	first_tcp_mapping = 0;
	first_udp_mapping = 0;
	first_icmp_mapping = 0;
	
	total_tcp_mappings = 0;
	free_tcp_mappings = 0;
	total_udp_mappings = 0;
	free_udp_mappings = 0;
	total_icmp_mappings = 0;
	free_icmp_mappings = 0;
	
	ip_i = first_priv_ip;
	save_ip_i = ip_i;
	while (ip_i) {
		pool_ip *save_ip = ip_i;
		ip_i = ip_i->next_free_ip;
		if (ip_i == save_ip_i)
			ip_i = 0;
		
		delete save_ip;
	}
	
	return Get_Pool_Size();
}

void NAT_Manager::Free_Pair(pool_pair *pair, pool_ip **base)
{
	if (!(*base)) {	
		pair->addr->next_free_port = pair->port;
		pair->port->next = pair->port;
		pair->addr->next_free_ip = pair->addr;
		*base = pair->addr;
		
		delete pair;
		
		return;
	}
	
	if (!pair->addr->next_free_port) {
		pair->addr->next_free_ip = (*base)->next_free_ip;
		pair->port->next = pair->port;
		pair->addr->next_free_port = pair->port;
		(*base)->next_free_ip = pair->addr;
	} else {
		pair->port->next = pair->addr->next_free_port->next;
		pair->addr->next_free_port->next = pair->port;
	}
	
	delete pair;
	
	return;
}

NAT_Manager::pool_pair NAT_Manager::Get_Free_Pair(protocol_type prot)
{
	pool_pair pair;
	pool_ip **base;
	if (prot == TCP)
		base = &tcp_pool;
	else if (prot == UDP)
		base = &udp_pool;
	else 
		base = &icmp_pool;
	
	if (!(*base)) {
		pair.addr = 0;
		return pair;
	}
		
	pair.addr = (*base)->next_free_ip;

	pool_port *port_save = pair.addr->next_free_port;
	pool_port *port_next = port_save->next;
	pair.addr->next_free_port->next = port_next->next;
	pair.port = port_next;
	
	if (port_next == port_save) {
		pair.addr->next_free_port = 0;
		(*base)->next_free_ip = pair.addr->next_free_ip;
		if ((*base) == pair.addr)
			(*base) = 0;
	} else if ((*base))
		(*base) = (*base)->next_free_ip;
		
	return pair;
}


/* 

void NAT_Manager::Free_Pair(NAT_Manager::pool_pair pair)
{
	if (!current_free_ip)
		current_free_ip = pair.addr;
	
	if (pair.addr->next_free_port == 0) {
		pair.addr->next_free_port = pair.port;
		pool_ip *save = current_free_ip->next_free_ip;
		pair.addr->next_free_ip = save;
		current_free_ip->next_free_ip = pair.addr;
		pair.port->next = pair.port;
	} else {
		pool_port *next = pair.addr->next_free_port->next;
		pair.addr->next_free_port->next = pair.port;
		pair.port->next = next;
	}
	// housekeeping
	++total_free_mappings;
}

*/

void NAT_Manager::Add_Dynamic_Mapping(in_addr *v4_addr, protocol_type prot, 
	in6_addr *v6_addr)
{
	pool_pair *pair = new pool_pair;
	pair->addr = new pool_ip;
	pair->addr->addr.s_addr = ntohl(v4_addr->s_addr);
	pair->addr->next_free_ip = 0;
	pair->addr->next_free_port = 0;
	pair->port = new pool_port;
	pair->port->port = 0;
	pair->port->next = 0;
	
	addr_mapping *mapping = new addr_mapping;
	
	mapping->port = 0;
	mapping->v4_addr = pair;
	mapping->last_active = time(0);
	
	memcpy(&mapping->v6_addr, v6_addr, sizeof(in6_addr));
	
	addr_mapping **base;
	if (prot == TCP)
		base = &first_tcp_mapping;
	else if (prot == UDP)
		base = &first_udp_mapping;
	else 
		base = &first_icmp_mapping;
	
	Insert_Mapping(mapping, base, prot);
}

NAT_Manager::addr_mapping::addr_mapping(u8 _static) : flags(_static), 
last_active(0), v4_seq_offset(0), v6_seq_offset(0)
{
	
}

NAT_Manager::addr_mapping::~addr_mapping()
{

}
