/***************************************************************************
 *  alg.cc : This file is part of 'ataga'
 *  created on: Sun Jun 12 12:47:53 CDT 2005
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

#include "alg.h"
#include "ataga.h"

// alg_entity *alg_base[alg_list_num];

/*******************************************************************************
	#----------------------------------#
	# memo: next header implementation #
	#----------------------------------#
	
	It is imperative that the protocol translation mechanism is redesigned, to
	better incorporate the existance of IPv6 extension headers, as for the date
	being only the base header is translated in any reasonable fashion. It will
	become important to create fragmentation information based on the IPv4 
	incoming packets and vice versa. The search for true packet payloads (layer
	4 protocols, specifically TCP and UDP) must be performed without assuming
	that the IPv6 doesn't contain any extension headers.
	
*******************************************************************************/

using namespace std;

alg_manager::alg_manager(const char *base) : basedir(base)
{
	/* clear ALG tables */
	memset(alg_base, 0, sizeof(alg_entity*) * alg_list_num);
	
	load_plugins();
}

alg_manager::~alg_manager()
{
	unload_plugins();
}

bool alg_manager::load_plugins()
{
	DIR *plugin_dir = opendir(basedir.c_str());
	
	if (!plugin_dir)
		return false;
	
	struct dirent *d;
		
	while ((d = readdir(plugin_dir))) {
		if (!strcmp(d->d_name, ".") || !strcmp(d->d_name, "..")
				|| strcmp(".so", (d->d_name + strlen(d->d_name) - 3)))
			continue;
		
		if (!load_entity(d->d_name))
			pLog->write("Runtime error loading dynamic plugin: %s", d->d_name);
	}
	
	closedir(plugin_dir);
	
	return true;
}

bool alg_manager::load_entity(const char *str)
{
	string plugin_path = basedir + "/" + str;
	void *p = dlopen(plugin_path.c_str(), RTLD_LAZY);
	if (!p) {
		pLog->write("Error loading library %s", dlerror());
		return false;
	} 
	
	bool (*v4_main)(NAT_Manager::addr_mapping *, char*, u32*) = (bool (*)(NAT_Manager::addr_mapping *, char*, u32*)) dlsym(p, "_Z7v4_mainPN11NAT_Manager12addr_mappingEPcPj");
	if (!v4_main) {
		pLog->write("Error getting v4_main %s", dlerror());
		return false;
	} 
	
	bool (*v6_main)(NAT_Manager::addr_mapping *, char*, u32*) = (bool (*)(NAT_Manager::addr_mapping *, char*, u32*)) dlsym(p, "_Z7v6_mainPN11NAT_Manager12addr_mappingEPcPj");
	if (!v6_main) {
		pLog->write("Error getting v6_main %s", dlerror());
		return false;
	} 
	
	const char* (*name_function)(int) = (const char*(*)(int))dlsym(p, "_Z9name_funci");
	if (!name_function) {
		pLog->write("Error getting name_function %s", dlerror());
		return false;
	}
	
	u16 (*port_function)(int) = (u16(*)(int))dlsym(p, "_Z9port_funci");
	if (!port_function) {
		pLog->write("Error getting port_function %s", dlerror());
		return false;
	}
	
	bool (*init_plugin)(bool (*)(NAT_Manager::addr_mapping*, int*, int*, u16, seq_action));
	init_plugin = (bool (*)(bool (*)(NAT_Manager::addr_mapping*, int*, int*, u16, seq_action)))dlsym(p, "_Z11init_pluginPFbPN11NAT_Manager12addr_mappingEPiS2_t10seq_actionE");
	if (!init_plugin) {
		pLog->write("Error getting init_plugin %s", dlerror());
		return false;
	}
	(*init_plugin)(Seq_Number);
	
	alg_entity *alg_en = new alg_entity;
	alg_en->name = (*name_function)((int_cast)&v6_prefix);
	alg_en->port = (*port_function)(0);
	alg_en->alg_func = v4_main;
	alg_en->prot = IPPROTO_IP;
	alg_en->handle = p;
	add_plugin(alg_en);
	
	alg_en = new alg_entity;
	alg_en->name = (*name_function)(0);
	alg_en->port = (*port_function)(0);
	alg_en->alg_func = v6_main;
	alg_en->prot = IPPROTO_IPV6;
	alg_en->handle = 0;
	add_plugin(alg_en);
	
	pLog->write("Loaded %s plugin.", alg_en->name);
	
	return true;
}

bool alg_manager::add_plugin(alg_entity *alg)
{
	alg_entity **base;
	if (alg->prot == IPPROTO_IP)
		base = &alg_base[prot_v4];
	else
		base = &alg_base[prot_v6];
	
	if (!*base)
		*base = alg;
	else
		(*base)->next = alg;
	
	return true;
}

bool alg_manager::unload_plugins()
{
	for (int i(prot_v4); i < alg_list_num; ++i) {
		alg_entity *p = alg_base[i];
		while (p) {
			alg_entity *save = p;
			p = save->next;
			
			if (save->handle) /* error checking ? */ {
				pLog->write("Releasing %s plugin.", save->name);
				dlclose(save->handle);
			}
			
			delete save;
		}
	}
	return true;
}

/*	bool Process_ALG_v4(char *packet, u32 *size)							*/
/*	*packet - pointer to the beginning of the IPv6 packet, 					*/
/*				not the ethernet header										*/
/*																			*/
/*	return value: 															*/
/*		true - the according ALG was found and/or processing was successful */
/*		false - something went wrong										*/
/*		in either case the packet checksums will be updated					*/
/*																			*/

bool alg_manager::Process_ALG_v4(NAT_Manager::addr_mapping *mapping, char *packet, u32 *size)
{
	alg_entity *i = alg_base[prot_v4];
 	iphdr *ip = (iphdr*)packet;
 	tcphdr *tcp = (tcphdr*)(packet + (ip->ihl * 4));
 	u16 src_port(ntohs(tcp->source));
 	u16 dst_port(ntohs(tcp->dest));
 	bool processing_result(true);
 
 	while (i) {
 		if (i->port == src_port || i->port == dst_port) {
 			processing_result = (*i->alg_func)(mapping, packet, size);
 			break;
 		}
 		i = i->next;
 	}
	
 	if (!processing_result) // did everything go alright ?
 		return processing_result;

 	/* TCP or UDP ? (ICMPv6 should never get here) */
 	if (ip->protocol == IPPROTO_TCP) {
 		tcp->check = compute_checksum_TCP_v4(ip, tcp);
 	} else {
 		assert(ip->protocol == IPPROTO_UDP); 
 		// if anything else got here then 
 		// we have a lot of fubar'ed somewhere in the code calling this 
 		// function and need to investigate
 		
 		udphdr *udp = (udphdr*)(packet + (ip->ihl * 4));
 		udp->check = compute_checksum_UDP_v4(ip, udp);
 	}
 	
 	/* checksums updated*/
 	
 	return true;
}

/*	bool Process_ALG_v6(char *packet, u32 *size)							*/
/*	*packet - pointer to the beginning of the IPv6 packet, 					*/
/*				not the ethernet header										*/
/*																			*/
/*	return value: 															*/
/*		true - the according ALG was found and/or processing was successful */
/*		false - something went wrong										*/
/*		in either case the packet checksums will be updated					*/
/*																			*/

bool alg_manager::Process_ALG_v6(NAT_Manager::addr_mapping *mapping, char *packet, u32 *size)
{
	alg_entity *i = alg_base[prot_v6];
	ip6_hdr *ip = (ip6_hdr*)packet;
	tcphdr *tcp = (tcphdr*)(packet + sizeof(ip6_hdr));
	u16 src_port(ntohs(tcp->source));
	u16 dst_port(ntohs(tcp->dest));
	bool processing_result(true);
	
	while (i) {
		if (i->port == src_port || i->port == dst_port) {
			processing_result = (*i->alg_func)(mapping, packet, size);
			break;
		}
		i = i->next;
	}
	
	if (!processing_result) // have checksums been updated?
		return processing_result;
	
	/* TCP or UDP ? (ICMPv6 should never get here) */
	if (ip->ip6_nxt == IPPROTO_TCP) {
		tcp->check = compute_checksum_TCP_v6(ip, tcp);
	} else {
		assert(ip->ip6_nxt == IPPROTO_UDP); 
		// if anything else got here then 
		// we have a lot of foobars somewhere in the code calling this 
		// function and need to investigate
		
		udphdr *udp = (udphdr*)(packet + sizeof(ip6_hdr));
		udp->check = compute_checksum_UDP_v6(ip, udp);
	}
	
	/* checksums updated*/
	
	return true;
}


// void ALG_Init()
// {
// 	/* clear ALG tables */
// 	memset(alg_base, 0, sizeof(alg_entity*) * alg_list_num);
// 	/* add all available ALGs - later this will be done through a configuration file */
// 	
// 	Add_ALG("ftp", 21, IP, alg_ftp_v4_main);
// 	Add_ALG("ftp", 21, IP6, alg_ftp_v6_main);
// 	
// 	Add_ALG("dns", 53, IP, alg_dns_v4_main);
// 	Add_ALG("dns", 53, IP6, alg_dns_v6_main);
// }

// bool Add_ALG(const char *name, u32 port, network_protocol prot, alg_main_func func)
// {
// 	alg_entity *alg = new alg_entity;
// 	alg->name = name;
// 	alg->port = port;
// 	alg->alg_func = func;
// 	alg->prot = prot;
// 	alg->next = 0;
// 
// 	return Add_ALG(alg);
// }

// bool Add_ALG(alg_entity *alg)
// {
// 	alg_entity **base;
// 	if (alg->prot == IP)
// 		base = &alg_base[prot_v4];
// 	else
// 		base = &alg_base[prot_v6];
// 	
// 	if (!*base)
// 		*base = alg;
// 	else
// 		(*base)->next = alg;
// 	
// 	return true;
// }

// bool Clean_ALG()
// {
// 	for (int i(prot_v4); i < alg_list_num; ++i) {
// 		alg_entity *p = alg_base[i];
// 		while (p) {
// 			alg_entity *save = p;
// 			p = p->next;
// 			delete save;
// 		}
// 	}
// 	return true;
// }

/*	bool Process_ALG_v6(char *packet, u32 *size)							*/
/*	*packet - pointer to the beginning of the IPv6 packet, 					*/
/*				not the ethernet header										*/
/*																			*/
/*	return value: 															*/
/*		true - the according ALG was found and/or processing was successful */
/*		false - something went wrong										*/
/*		in either case the packet checksums will be updated					*/
/*																			*/

// bool Process_ALG_v6(char *packet, u32 *size)
// {
// 	alg_entity *i = alg_base[prot_v6];
// 	ip6_hdr *ip = (ip6_hdr*)packet;
// 	tcphdr *tcp = (tcphdr*)(packet + sizeof(ip6_hdr));
// 	u16 src_port(ntohs(tcp->source));
// 	u16 dst_port(ntohs(tcp->source));
// 	bool processing_result(true);
// 	
// 	while (i) {
// 		if (i->port == src_port || i->port == dst_port) {
// 			processing_result = (*i->alg_func)(packet, size);
// 			break;
// 		}
// 		i = i->next;
// 	}
// 	
// 	if (!processing_result)
// 		return processing_result;
// 	
// 	/* TCP or UDP ? (ICMPv6 should never get here) */
// 	if (ip->ip6_nxt == IPPROTO_TCP) {
// 		tcp->check = compute_checksum_TCP_v6(ip, tcp);
// 	} else {
// 		assert(ip->ip6_nxt == IPPROTO_UDP); 
// 		// if anything else got here then 
// 		// we have a lot of foobars somewhere in the code calling this 
// 		// function and need to investigate
// 		
// 		udphdr *udp = (udphdr*)(packet + sizeof(ip6_hdr));
// 		udp->check = compute_checksum_UDP_v6(ip, udp);
// 	}
// 	
// 	/* checksums updated*/
// 	
// 	return true;
// }


/*	bool Process_ALG_v4(char *packet, u32 *size)							*/
/*	*packet - pointer to the beginning of the IPv6 packet, 					*/
/*				not the ethernet header										*/
/*																			*/
/*	return value: 															*/
/*		true - the according ALG was found and/or processing was successful */
/*		false - something went wrong										*/
/*		in either case the packet checksums will be updated					*/
/*																			*/

// bool Process_ALG_v4(char *packet, u32 *size)
// {
// 	alg_entity *i = alg_base[prot_v4];
// 	iphdr *ip = (iphdr*)packet;
// 	tcphdr *tcp = (tcphdr*)(packet + (ip->ihl * 4));
// 	u16 src_port(ntohs(tcp->source));
// 	u16 dst_port(ntohs(tcp->source));
// 	bool processing_result(true);
// 	
// 	while (i) {
// 		if (i->port == src_port || i->port == dst_port) {
// 			processing_result = (*i->alg_func)(packet, size);
// 			break;
// 		}
// 		i = i->next;
// 	}
// 	
// 	if (!processing_result)
// 		return processing_result;
// 	
// 	/* TCP or UDP ? (ICMPv6 should never get here) */
// 	if (ip->protocol == IPPROTO_TCP) {
// 		tcp->check = compute_checksum_TCP_v4(ip, tcp);
// 	} else {
// 		assert(ip->protocol == IPPROTO_UDP); 
// 		// if anything else got here then 
// 		// we have a lot of foobars somewhere in the code calling this 
// 		// function and need to investigate
// 		
// 		udphdr *udp = (udphdr*)(packet + (ip->ihl * 4));
// 		udp->check = compute_checksum_UDP_v4(ip, udp);
// 	}
// 	
// 	/* checksums updated*/
// 	
// 	return true;
// }

/*Send to checksum calculation functions (below)*/

unsigned short in_cksum (unsigned short *ptr, int nbytes)
{
	register long sum;
	u_short oddbyte;
	register u_short answer;

	sum = 0;
	while (nbytes > 1) {
		sum += *ptr++;
		nbytes -= 2;
	}

	if (nbytes == 1) {
		oddbyte = 0;
		*((u_char *) & oddbyte) = *(u_char *) ptr;
		sum += oddbyte;
	}

	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);
	answer = ~sum;

	return (answer);
}

u16 compute_checksum_pseudo(u8 *pPseudo_hdr, u32 pseudo_hdr_length, 
	u8 *pAddress, u32 length)
{
	register long sum = 0;
	register u16 *pAddr; 
	register u32 len = 0;

	if (pPseudo_hdr != 0 && pseudo_hdr_length > 0) {
		pAddr = (u16 *) pPseudo_hdr;
		len   = pseudo_hdr_length;
		while (len > 0) {
			sum += *pAddr++;
			len -= 2;
		}
	}

	pAddr = (u16*) pAddress;
	len = length;
	while (len > 1) {
		sum += *pAddr++;
		len -= 2;
	}

	if (len > 0) {
		u8 *pAddrByte = (u8*)pAddr;
		sum += (u8) *pAddrByte;
	}
	
	while (sum >> 16)
		sum = (sum & 0xffff) + (sum >> 16);
	
	return ((u16) ~sum);
}

u16 compute_checksum_TCP_v6(ip6_hdr *ip6, tcphdr *tcp)
{
	u8 pseudo_header[40];
	memset(pseudo_header, 0, 40);

	tcp->check = 0;
	memcpy(pseudo_header, &ip6->ip6_src, 32);
	u16 payload_size(ntohs(ip6->ip6_plen));

	pseudo_header[34] = payload_size >> 8;
	pseudo_header[35] = payload_size;
	pseudo_header[39] = IPPROTO_TCP;
	
	return compute_checksum_pseudo(pseudo_header, 40, (u8*)tcp, payload_size);
}

u16 compute_checksum_TCP_v4(iphdr *ip, tcphdr *tcp)
{
	u8 pseudo_header[36];
	memset(pseudo_header, 0, 36);
	
	tcp->check = 0;
	memcpy(pseudo_header, &ip->saddr, 8);
	u16 payload_size(htons(ip->tot_len) - (ip->ihl * 4));
	
	pseudo_header[9] = IPPROTO_TCP;
	pseudo_header[10] = payload_size >> 8;
	pseudo_header[11] = payload_size;
	
	return compute_checksum_pseudo(pseudo_header, 20, (u8*)tcp, payload_size);
}

u16 compute_checksum_UDP_v6(ip6_hdr *ip6, udphdr *udp)
{
	u8 pseudo_header[40];
	memset(pseudo_header, 0, 40);
	
	udp->check = 0;
	memcpy(pseudo_header, &ip6->ip6_src, 32);
	u16 payload_size(ntohs(ip6->ip6_plen));
	
	pseudo_header[34] = payload_size >> 8;
	pseudo_header[35] = payload_size;
	pseudo_header[39] = IPPROTO_UDP;

#ifdef _DEBUG_
	pLog->write("Calculating checksum for UDP v6, payload size %u", payload_size);
#endif // _DEBUG_
	
	u16 checksum = compute_checksum_pseudo(pseudo_header, 40, (u8*)udp, payload_size);
	if (!checksum)
		return 65535;
	
	return checksum;
}

u16 compute_checksum_UDP_v4(iphdr *ip, udphdr *udp)
{
	u8 pseudo_header[12];
	memset(pseudo_header, 0, 12);
	
	udp->check = 0;
	memcpy(pseudo_header, &ip->saddr, 8);
	u16 payload_size(htons(ip->tot_len) - (ip->ihl * 4));
	
	pseudo_header[9] = IPPROTO_UDP;
	pseudo_header[10] = payload_size >> 8;
	pseudo_header[11] = payload_size;

	return compute_checksum_pseudo(pseudo_header, 12, (u8*)udp, payload_size);
}

u16 compute_checksum_ICMP_v6(ip6_hdr *ip6, icmp6_hdr *icmp)
{
	u8 pseudo_header[50];
	memset(pseudo_header, 0, 50);
	
	icmp->icmp6_cksum = 0;
	memcpy(pseudo_header, &ip6->ip6_src, 32);
	u16 payload_size(htons(ip6->ip6_plen));
	u32 pay(htonl(payload_size));

	memcpy(pseudo_header + 32, &pay, 4);
	pseudo_header[39] = IPPROTO_ICMPV6;
	
	return compute_checksum_pseudo(pseudo_header, 40, (u8*)icmp, payload_size);
}

u16 compute_checksum_ICMP_v4(iphdr *ip, icmphdr *icmp)
{
	return in_cksum((u16*)icmp, htons(ip->tot_len) - sizeof(iphdr));
}

bool Seq_Number_Get(NAT_Manager::addr_mapping *mapping, int *seq, int *ack_seq, u16 dir)
{
	if (dir == IPPROTO_IPV6) {
		*seq = mapping->v4_seq_offset;
		*ack_seq= mapping->v6_seq_offset;
	
		return true;
	} else {
		*seq = mapping->v6_seq_offset;
		*ack_seq= mapping->v4_seq_offset;
	
		return true;
	}
	
	return false;
}

bool Seq_Number_Set(NAT_Manager::addr_mapping *mapping, int *seq, u16 dir)
{
	if (dir == IPPROTO_IPV6) {
		mapping->v4_seq_offset += *seq;
	
		return true;
	} else {
		mapping->v6_seq_offset += *seq;
	
		return true;
	}
	
	return false;
}

bool Seq_Number(NAT_Manager::addr_mapping *i, int *seq, int *ack_seq, u16 direction, seq_action action)
{
	switch (action) {
		case seq_get:
			return Seq_Number_Get(i, seq, ack_seq, direction);
		
		case seq_set:
			return Seq_Number_Set(i, seq, direction);
		
		default:
			return false;
	}
}
