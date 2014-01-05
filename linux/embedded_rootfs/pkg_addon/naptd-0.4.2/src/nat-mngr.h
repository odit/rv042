/***************************************************************************
 *  nat-mngr.h : This file is part of 'ataga'
 *  created on: Mon Jun  6 14:24:01 CDT 2005
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
 
#ifndef _NAT_MANAGER__H_
#define _NAT_MANAGER__H_

#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <net/if.h>
 
typedef unsigned int u32;
typedef unsigned short int u16;
typedef unsigned char u8;

class NAT_Manager
{
public:
	NAT_Manager();
	~NAT_Manager();

	int Add_Pool(in_addr start_ip, in_addr end_ip, u16 start_port, u16 end_port);
	int Add_Priv_Pool(in_addr start_ip, in_addr end_ip);
	int Add_Static_v4_to_v6_Mapping(in_addr, in6_addr);
	int Drop_Pool();
	int Clean_Pool();
	u32 Get_Pool_Size();
	u32 Get_Remaining_Pool_Size();

#ifdef _DEBUG_

	void Print_Free_Pool();

#endif // _DEBUG_

	static const u32 NAT_pool_clean_interval = 10;

private:
	static const u32 pool_minimun = 25; // DECREPTED

	struct frag_info {
		u16 id;
		u32 time;
		u16 src_port;
		u16 dst_port;
		frag_info *next;
		frag_info *prev;
	};
	
	struct pool_range {
		in_addr begin_range;
		in_addr end_range; // inclusive
		u16 begin_port;
		u16 end_port;
		pool_range *next;
	};
	
	struct pool_port {
		u16 port;
		pool_port *next;
	};
	
	struct pool_ip {
		in_addr addr;
		pool_port *next_free_port;
		pool_ip *next_free_ip;
	};
	
	u32 total_tcp_mappings;
	u32 free_tcp_mappings;
	u32 total_udp_mappings;
	u32 free_udp_mappings;
	u32 total_icmp_mappings;
	u32 free_icmp_mappings;
	pool_ip *tcp_pool;
	pool_ip *udp_pool;
	pool_ip *icmp_pool;
	pool_ip *first_priv_ip;
	pool_range *tcp_range;
	pool_range *udp_range;
	pool_range *icmp_range;
	
public:
	struct pool_pair {
		pool_ip *addr;
		pool_port *port;
	} __attribute__ ((packed));
	
	
	class addr_mapping {
	public:
		enum {
			none = 0,
			static_m = 1,
			ready_to_remove = 2
		};
		
		addr_mapping(u8 = none);
		~addr_mapping();
	
		in6_addr v6_addr;
		u16 port;
		pool_pair *v4_addr;
		u8 flags;
		u32	last_active;
		int v4_seq_offset; // going to v4
		int v6_seq_offset; // going to v6

		addr_mapping *prev;
		addr_mapping *next;
	};

private:
	pthread_mutex_t mapping_mutex;
	pthread_mutex_t v4_lookup_mutex;
	pthread_mutex_t v6_lookup_mutex;
	addr_mapping *first_tcp_mapping;
	addr_mapping *first_udp_mapping;
	addr_mapping *first_icmp_mapping;
	frag_info *first_frag;
	u32 translation_time_tcp;
	u32 translation_time_udp;
	u32 translation_time_icmp;
	
public:
	struct v6_addr {
		in6_addr addr;
		u16 port;
	};
	
	struct v4_addr {
		in_addr addr;
		u16 port;
	};
	
	struct static_pair {
		v4_addr v4;
		v6_addr v6;
	};
	
	typedef enum {
		TCP = IPPROTO_TCP,
		UDP = IPPROTO_UDP,
		ICMP = IPPROTO_ICMP
	} protocol_type;
	
	bool FindFrag(u16 id, u16 *src_port, u16 *dst_port);
	bool AddFrag(u16 id, u16 src_port, u16 dst_port);
	bool DelFrag(u16 id);
	bool Get_Translation_Times(u32 *tcp, u32 *udp, u32 *icmp);
	bool Set_Translation_Times(u32 tcp, u32 udp, u32 icmp);
	NAT_Manager::addr_mapping *Get_v6_to_v4_Translation(in6_addr*, u16, v4_addr*, bool, protocol_type);
	NAT_Manager::addr_mapping *Get_v4_to_v6_Translation(in_addr*, u16, v6_addr*, protocol_type);
	bool In_Pool(in_addr *addr);
	void Add_Dynamic_Mapping(in_addr *v4_addr, protocol_type prot, in6_addr *v6_addr);
	int Prep_Remove_Mapping(addr_mapping *mapping);
	
protected:
	void Remove_Mapping(addr_mapping *mapping, addr_mapping **base, protocol_type);
	void Insert_Mapping(addr_mapping *mapping, addr_mapping **base, protocol_type);
	pool_pair Get_Free_Pair(protocol_type prot);
	void Free_Pair(pool_pair *pair, pool_ip **base);
	int Increment_Pool(protocol_type prot);
	void Add_Pool_Range(pool_range *range, pool_range **base);
	pool_range *Get_Pool_Range(protocol_type prot);
	void Free_Pool_Range(pool_range *range, protocol_type prot);
};
 
#endif // _NAT_MANAGER__H_
