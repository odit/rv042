/***************************************************************************
 *  alg.h : This file is part of 'ataga'
 *  created on: Sun Jun 12 12:48:57 CDT 2005
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


/* 	those who know me should know why I named the program Ataga
	for eveyone else there's a hint: Nimda	*/
	
#ifndef __ALG__H_
#define __ALG__H_

#include <string>
#include <dlfcn.h>
#include <dirent.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>
#include <netinet/icmp6.h>
#include "nat-mngr.h"

typedef unsigned int u32;
typedef unsigned short int u16;
typedef unsigned char u8;

class alg_manager {
public:
	alg_manager(const char *base);
	~alg_manager();

	bool Process_ALG_v6(NAT_Manager::addr_mapping *mapping, char *packet, u32 *size);
	bool Process_ALG_v4(NAT_Manager::addr_mapping *mapping, char *packet, u32 *size);
	
protected:
	bool load_plugins();
	bool unload_plugins();

private:
	enum {
		prot_v4 = 0,
		prot_v6,
		alg_list_num
	};
	
	typedef bool (*alg_main_func)(NAT_Manager::addr_mapping *, char*, u32*);
	
	struct alg_entity {
		const char *name;
		u16 port;
		alg_main_func alg_func;
		u16 prot;
		void *handle;
		alg_entity *next;
	};
	
	bool add_plugin(alg_entity *alg);
	bool load_entity(const char *str);

	std::string basedir;
	alg_entity *alg_base[alg_list_num];
};

enum {
	TCPv6,
	TCPv4,
	UDPv6,
	UDPv4
};

typedef enum {
	seq_get,
	seq_set
} seq_action;

void ALG_Init();
//bool Add_ALG(const char*, u32 port, network_protocol prot, alg_main_func);
//bool Add_ALG(alg_entity*);
bool Clean_ALG();
u16 compute_checksum_pseudo(u8*, u32, u8*, u32);
u16 compute_checksum_TCP_v6(ip6_hdr *ip6, tcphdr *tcp);
u16 compute_checksum_TCP_v4(iphdr *ip, tcphdr *tcp);
u16 compute_checksum_UDP_v6(ip6_hdr *ip6, udphdr *udp);
u16 compute_checksum_UDP_v4(iphdr *ip, udphdr *udp);
u16 compute_checksum_ICMP_v6(ip6_hdr *ip6, icmp6_hdr *icmp);
u16 compute_checksum_ICMP_v4(iphdr *ip, icmphdr *icmp);
unsigned short in_cksum(unsigned short *ptr, int nbytes);
bool Seq_Number(NAT_Manager::addr_mapping*, int*, int*, u16, seq_action);
bool Seq_Number_Get(NAT_Manager::addr_mapping*, int*, int*, u16);
bool Seq_Number_Set(NAT_Manager::addr_mapping*, int*, u16);

extern alg_manager		*alg_mngr;

#ifdef __LP64__
#define int_cast long int
#else
#define int_cast int
#endif


#endif // __ALG__H_
