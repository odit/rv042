/***************************************************************************
 *  v4_handler.cc : This file is part of 'ataga'
 *  created on: Tue Jun  7 22:44:17 CDT 2005
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
#include <unistd.h>
#include <netdb.h>
//#include <pcap-bpf.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/if_ether.h>
#include "v4_handler.h"
#include "v6_handler.h"
#include "ataga.h"
#include "dns_helper.h"
#include "alg.h"
#include "ndisc.h"

listen_interface 	*v4_interfaces;
pending_packet		*v4_pending_packets = 0;

void *v4_handler_init(void *p)
{
	/* init stuff */
	v4_interfaces = (listen_interface*)p;
	listen_interface *i = v4_interfaces;
	
	while (i) {
		i->fd = initialise_network_int(i->name, ETH_P_ALL);
		get_int_hardware_address(i);
		get_int_mtu(i);
		i = i->next;
	}
	
	bool get_v4_addresses_from_interfaces(false);
	pSettings->Get(get_v4_addresses_from_interfaces, get_v4_from_outside_int);
	
	if (get_v4_addresses_from_interfaces) {
		i = v4_interfaces;
		//listen_interface *new_interfaces = 0;
		while (i) {
			in_addr addr = get_interface_v4_addr(i);
			pNat->Add_Pool(addr, addr, 1050, 65000);
			
			/*
			bpf_insn filter_template[] = {
				BPF_STMT(BPF_LD+BPF_W+BPF_ABS, 0),
				BPF_JUMP(BPF_JMP+BPF_JEQ+BPF_K, 0, 0, 9),
				BPF_STMT(BPF_LD+BPF_H+BPF_ABS, 4),
				BPF_JUMP(BPF_JMP+BPF_JEQ+BPF_K, 0, 0, 7),
				BPF_STMT(BPF_LD+BPF_H+BPF_ABS, 12),
				BPF_JUMP(BPF_JMP+BPF_JEQ+BPF_K, ETHERTYPE_IP, 0, 5),
				BPF_STMT(BPF_LD+BPF_W+BPF_ABS, 30), // have checksums been updated?
				BPF_JUMP(BPF_JMP+BPF_JEQ+BPF_K, 0, 2, 0),
				BPF_STMT(BPF_LD+BPF_W+BPF_ABS, 30),
				BPF_JUMP(BPF_JMP+BPF_JEQ+BPF_K, 0, 0, 1),
				BPF_STMT(BPF_RET+BPF_K, (u32)-1),
				BPF_STMT(BPF_RET+BPF_K, 0)
			};
			
			bpf_insn *filter_progIP4 = (bpf_insn*) new char[sizeof(filter_template)];
			memcpy(filter_progIP4, filter_template, sizeof(filter_template));

			bpf_program filterIP4 = { sizeof(filter_template) / sizeof(filter_template[0]),
				filter_progIP4 };
				
			u32 *pLong = (u32*) &i->hw_addr[0];
			u16 *pShort = (u16*) &i->hw_addr[4];
				
			filter_progIP4[1].k = (bpf_u_int32) htonl(*pLong);
			filter_progIP4[3].k = (bpf_u_int32) htons(*pShort);

			pLong = (u32*)  &addr;
			filter_progIP4[7].k = (bpf_u_int32) htonl(*pLong);
			filter_progIP4[7].jf = filter_progIP4[7].jt + 1;
			
			listen_interface *new_int = new listen_interface;
			new_int->name = i->name;
			new_int->fd = initialise_network_int(new_int->name, ETH_P_ALL);	
			new_int->next = 0;
			new_int->filter = filter_progIP4;
			get_int_hardware_address(new_int);
			get_int_mtu(new_int);

			if (!new_interfaces)
				new_interfaces = new_int;
			else {
				listen_interface *iter = new_interfaces;
				while (iter->next)
					iter = iter->next;
				
				iter->next = new_int;
			}
			
			if (setsockopt(i->fd, SOL_SOCKET, SO_ATTACH_FILTER, 
				&filterIP4, sizeof(bpf_program)) == -1)
				pLog->write("Error attaching filter to interface %s: %s", 
					new_int->name, strerror(errno));
			*/
			i = i->next;
		}
		/*
		if (new_interfaces) {
			listen_interface *iter = new_interfaces;
			while (iter->next)
				iter = iter->next;
			
			iter->next = v4_interfaces;
			v4_interfaces = new_interfaces;
			
			iter = v4_interfaces;
			while (iter) {
				pLog->write("int name %s fd %i filter 0x%x", iter->name, iter->fd, 
					iter->filter);
				iter = iter->next;
			}
		}
		*/
	}
	
	/* end of init stuff */
	
	/* main loop */
	bool working = true;
	NAT_Manager::v6_addr v6_addr;
	NAT_Manager::addr_mapping *mapping;
	char read_buffer[packet_size];
	char *packet_buffer;
	int bytes_read;
	
	while (working) {
		/* read pending packets */
		bytes_read = Process_Pending_Packets(&v4_pending_packets, &packet_buffer);
		
		if (!bytes_read) {
			bytes_read = Get_Data_From_v4_Sockets(read_buffer);
			packet_buffer = read_buffer;
		}
		
		if (!bytes_read) /* I wonder how this happened... */
			continue;
		
		iphdr *ip = (iphdr*)(packet_buffer + sizeof(ether_header));
		
		if (ip->protocol == IPPROTO_TCP) {
			tcphdr *tcp = (tcphdr*)(packet_buffer + sizeof(ether_header) + ip->ihl * 4);
			
			if (!tcp->source || !tcp->dest) {
#ifdef _DEBUG_
				printf("alarm!\n");
				printf("strange port on v4 side!\n");
				FILE *hFile = fopen("bad.packets.log", "a+");
				if (!hFile)
					hFile = fopen("bad.packets.log", "x+");
				if (hFile) {
					fwrite(packet_buffer, bytes_read, 1, hFile);
					fprintf(hFile, "\n\n");
					fclose(hFile);
				}
#endif // _DEBUG_
				continue;
			}
			
			if (tcp->syn && !tcp->ack) { /* new connection */
				mapping = pNat->Get_v4_to_v6_Translation((in_addr*)&ip->daddr, 
					tcp->dest, &v6_addr, NAT_Manager::TCP);
				
				/* Don't have it, don't bother */
				if (!mapping)
					continue;
				
				/* 	mapping == 0x01
					we have the IP in the Priv Pool, but need a Dynamic resolution through DNS */
				
				if (mapping == (NAT_Manager::addr_mapping*)0x01)
					Request_Dynamic_DNS_mapping((in_addr*)&ip->daddr, NAT_Manager::TCP,
						packet_buffer, bytes_read);
						
				else 
					Translate_v4_Send_v6_Packet(mapping, packet_buffer, bytes_read, 
						&v6_addr.addr, 
						(!(mapping->flags & NAT_Manager::addr_mapping::static_m) 
						? v6_addr.port : tcp->dest));
						
			} else if (tcp->fin || tcp->rst) {
				mapping = pNat->Get_v4_to_v6_Translation((in_addr*)&ip->daddr, 
					tcp->dest, &v6_addr, NAT_Manager::TCP); 
				
				/* Don't have it, or we do but this is not the first packet of the stream */
				/* in either case don't bother */
				if (!mapping || mapping == (NAT_Manager::addr_mapping*)0x01)
					continue;
				
				Translate_v4_Send_v6_Packet(mapping, packet_buffer, bytes_read,
					&v6_addr.addr, (!(mapping->flags & NAT_Manager::addr_mapping::static_m) 
					? v6_addr.port : tcp->dest));
				
				/* prepare to remove connection */
				pNat->Prep_Remove_Mapping(mapping);
			} else {
				mapping = pNat->Get_v4_to_v6_Translation((in_addr*)&ip->daddr, 
					tcp->dest, &v6_addr, NAT_Manager::TCP);
				
				/* Don't have it, or we do but this is not the first packet of the stream */
				/* in either case don't bother */
				if (!mapping || mapping == (NAT_Manager::addr_mapping*)0x01)
					continue;
				
				Translate_v4_Send_v6_Packet(mapping, packet_buffer, bytes_read,
					&v6_addr.addr, (!(mapping->flags & NAT_Manager::addr_mapping::static_m) 
					? v6_addr.port : tcp->dest));
			}
					
		} else if (ip->protocol == IPPROTO_UDP) {
			udphdr *udp = (udphdr*)(packet_buffer + sizeof(ether_header) + ip->ihl * 4);
			
			mapping = pNat->Get_v4_to_v6_Translation((in_addr*)&ip->daddr, 
				udp->dest, &v6_addr, NAT_Manager::UDP);
				
			/* Don't have it, don't bother */
			if (!mapping)
				continue;
				
			/* 	mapping == 0x01
				we have the IP in the Priv Pool, but need a Dynamic resolution through DNS */
				
			if (mapping == (NAT_Manager::addr_mapping*)0x01)
				Request_Dynamic_DNS_mapping((in_addr*)&ip->daddr, NAT_Manager::UDP,
					packet_buffer, bytes_read);
						
			else 
				Translate_v4_Send_v6_Packet(mapping, packet_buffer, bytes_read, 
					&v6_addr.addr, (!(mapping->flags & NAT_Manager::addr_mapping::static_m) 
					? v6_addr.port : udp->dest));
				
		} else if (ip->protocol == IPPROTO_ICMP) {
			icmphdr *icmp = (icmphdr*)(packet_buffer + sizeof(ether_header) + ip->ihl * 4);

			mapping = pNat->Get_v4_to_v6_Translation((in_addr*)&ip->daddr, 
				icmp->un.echo.sequence, &v6_addr, NAT_Manager::ICMP);
			
			/* Don't have it, don't bother */
			if (!mapping)
				continue;
				
			/* 	mapping == 0x01
				we have the IP in the Priv Pool, but need a Dynamic resolution through DNS */
				
			if (mapping == (NAT_Manager::addr_mapping*)0x01)
				Request_Dynamic_DNS_mapping((in_addr*)&ip->daddr, NAT_Manager::ICMP,
					packet_buffer, bytes_read);
						
			else 
				Translate_v4_Send_v6_Packet(mapping, packet_buffer, bytes_read, &v6_addr.addr, 
					(!(mapping->flags & NAT_Manager::addr_mapping::static_m) 
					? v6_addr.port : icmp->un.echo.sequence));
				
		} else {
			/* don't know this one */
			continue;
		}
	}
	
	/* clean_up */
	
	clean_network_int(v4_interfaces);
	Clear_Pending_Packets(v4_pending_packets);
	
	return 0;
}

bool CheckFragmentation(iphdr *ip)
{
	if (ip->frag_off & IP_MF) {
		if (!(ip->frag_off & IP_OFFMASK)) {
			pNat->AddFrag(ip->id, *((u8*)ip + ip->ihl * 4), *((u8*)ip + ip->ihl * 4 + 2));
			return true;
		}
	}
	
	return false;
}

int Get_Data_From_v4_Sockets(char *read_buffer)
{
	fd_set read_set;
	int highest_fd = create_fd_set(v4_interfaces, &read_set);
	while (!select(highest_fd + 1, &read_set, 0, 0, 0)) {
		pLog->write("Error while listening on IPv4 interfaces: %s", strerror(errno));
		++global_stats.v4_packets_input_errors;
	
		continue;
	}
	
	int bytes_read(0);
	listen_interface *i = v4_interfaces;
	while (i) {
		if (FD_ISSET(i->fd, &read_set)) {
			/* something interesting ;) */
				
			char *packet_buffer = read_buffer;
			bytes_read = read(i->fd, packet_buffer, packet_size);
				
	    	if (bytes_read == -1) {
				pLog->write("Error while reading IPv4 packet on %s interface: %s", i->name, strerror(errno));
				i = i->next;
				bytes_read = 0;
				++global_stats.v4_packets_input_errors;
				
				continue;
			}
			ether_header *eth = (ether_header*) packet_buffer;

			if (memcmp(&eth->ether_dhost, i->hw_addr, ETH_ALEN)) {
				i = i->next;
				continue;
			}
		
			if (eth->ether_type != htons(ETHERTYPE_IP)) {
				i = i->next;
				continue;
			}
			
			++global_stats.v4_packets_input_packets;
			
			return bytes_read;	
		}
		i = i->next;
	}
	
	return 0;
}

void Request_ND(in6_addr *addr, in_addr *v4_addr, const char *buffer, u32 buffer_size)
{
	pending_packet *packet = AllocatePacket(buffer, buffer_size);
	packet->status = awaiting_ND_request;
	
	in6_addr nd_request_source;
	memcpy(&nd_request_source, &v6_prefix, 12);
	memcpy(((char*)&nd_request_source) + 12, v4_addr, 4);
	Add_ND_Request(addr, &nd_request_source, packet);
	Add_Pending_Packet(&v4_pending_packets, packet);
}

void Request_Dynamic_DNS_mapping(in_addr *addr, NAT_Manager::protocol_type prot, 
	const char *buffer, u32 buffer_size)
{
	pending_packet *packet = AllocatePacket(buffer, buffer_size);
	packet->status = awaiting_DNS_request;
	
	Add_DNS_Request(addr, prot, packet);
	Add_Pending_Packet(&v4_pending_packets, packet);
}

bool GetHardwareAddr_v6(const listen_interface **send, in6_addr *dst, in_addr *src, char *packet, u32 size)
{
	in6_addr gw, *res;
	if (FindRoute_v6(send, &gw, dst, packet))
		res = &gw;
	else
		res = dst;
	
	if (!GetNeighbor(send, res, packet)) {
		if (size)
			Request_ND(res, src, packet, size);
		
		return false;
	}
	return true;
}

void SendICMP_MsgTooBig_v4(iphdr *src, const listen_interface *i)
{
	int packet_size = sizeof(ether_header) + sizeof(iphdr) + sizeof(icmphdr) + sizeof(iphdr) + 8;
	char packet[packet_size];
	
	const listen_interface *sending_int;
	if (!GetHardwareAddr_v4(&sending_int, (in_addr*)&src->saddr, (in_addr*)&src->daddr, packet, 0))
		return;
				  
	iphdr *ip = (iphdr*)(packet + sizeof(ether_header));
	icmphdr *icmp = (icmphdr*) (packet + sizeof(ether_header) + sizeof(iphdr));

	memset(packet, 0, packet_size);
	memcpy(packet + sizeof(ether_header) + sizeof(iphdr) + sizeof(icmphdr), src, sizeof(iphdr) + 8);

	ip->version = 4;
	ip->ihl = 5;
	ip->tot_len = htons (packet_size);
	ip->id = rand();
	ip->ttl = 255;
	ip->protocol = IPPROTO_ICMP;
	ip->saddr = src->daddr;
	ip->daddr = src->saddr;
	ip->check = in_cksum ((u16*)ip, sizeof (iphdr));

  	icmp->type = ICMP_DEST_UNREACH;
	icmp->code = ICMP_FRAG_NEEDED;
	icmp->un.frag.mtu = htons(i->link_mtu - (sizeof(ip6_hdr) - sizeof(iphdr)));
  	icmp->checksum = in_cksum((u16*)icmp, sizeof(icmphdr) + sizeof(iphdr) + 8);
		
	int bytes_written = write(sending_int->fd, packet, packet_size); 
	// = sendto(outbound_v4_socket, packet, packet_size, 0, (sockaddr*)&servaddr, sizeof(sockaddr_in));
	
	if (bytes_written == -1) {
		pLog->write("Error while sending Message Too Big packet to IPv4 host: %s",
				strerror(errno));
		++global_stats.v4_packets_output_errors;
				
		return;
	}
	
	++global_stats.v4_packets_output_packets;
	
	return;
}

/*	bool Translate_v4_Send_v6_Packet(char *packet_v4, u32 size, in6_addr *v6_addr, u16 port) 	*/
/*																								*/
/*	if port == 0 then the packet is ICMP and should be translated accordingly 					*/
/*																								*/

bool Translate_v4_Send_v6_Packet(NAT_Manager::addr_mapping *mapping, char *packet_v4, u32 size, in6_addr *v6_addr, u16 port)
{
	/*	memo: this code does not yet support fragmentation 					*/
	/*	at some point it will become necessary to create IPv6 fragmentation */
	/*	headers based on the fragmentation information from IPv4 datagrams	*/
	/*	it will also become necessary to fragment packets that exceed the 	*/
	/*	MTU of the outbound interface to enable oversized packets to be 	*/
	/*	delivered.															*/

	/* get hardware address to send to */
	const listen_interface *sending_int; 	/* here we hold a pointer to the   	*/
											/* outbound IPv6 interface that we 	*/
											/* will use for sending 			*/
	
	if (!GetHardwareAddr_v6(&sending_int, v6_addr, (in_addr*)((u8*)packet_v4 + sizeof(ether_header) + 12), packet_v4, size))
		return false;
	
	char packet_v6[packet_size];
	int total_length = Translate_v4_Packet(mapping, packet_v6, packet_v4, v6_addr, port);
	
	/* error checking */
	if (total_length == -1)
		return false;
	
	/* check MTU size */
	if (!CheckMTU_v6(packet_v4, total_length, sending_int))
		return false;
	
	/* send */
	Send_v6_Packet(sending_int, packet_v6, total_length);
	
	return true;
}

bool CheckMTU_v6(const char *packet_v4, u32 total_length, const listen_interface *i)
{
	if (total_length - sizeof(ether_header) > i->link_mtu) {
		/* packet TOO big! */
		/* inform source about this */
		iphdr *ip = (iphdr*)(packet_v4 + sizeof(ether_header));
		SendICMP_MsgTooBig_v4(ip, i);
		
		return false;
	}
	
	return true;
}

int Translate_v4_Packet(NAT_Manager::addr_mapping *mapping, char *packet_v6, char *packet_v4, in6_addr *v6_addr, u16 port)
{
	/* copy the ethernet header as we have already written the correct values */
	/* to it in GetNeighbor(&sending_int, v6_addr, packet_v4)				  */
	memcpy(packet_v6, packet_v4, sizeof(ether_header));
	
	/* create basic IPv6 header */
	ip6_hdr *ip6 = (ip6_hdr*)(packet_v6 + sizeof(ether_header));
	Create_IPv6_Header(packet_v6 + sizeof(ether_header), packet_v4 + sizeof(ether_header), v6_addr);
	
	/* copy the payloads */
	iphdr *ip = (iphdr*)(packet_v4 + sizeof(ether_header));
	
	/* see if the TTL is above 1 */
	/* if not discard packet */
	if (ip->ttl <= 1)
		return -1;
	
	memcpy(packet_v6 + sizeof(ether_header) + sizeof(ip6_hdr), packet_v4 + 
		sizeof(ether_header) + ip->ihl * 4, ntohs(ip->tot_len) - (ip->ihl * 4));
	
	if (ip6->ip6_nxt != IPPROTO_ICMP) { /* TCP || UDP */
		
		/* set port */
		
		memcpy(packet_v6 + sizeof(ether_header) + sizeof(ip6_hdr) + 2, &port, sizeof(u16));
		
		/* do Application Level Gateway processing + checksum updating */
		u32 alg_input_size = htons(ip6->ip6_plen) + sizeof(ip6_hdr);
		if (!alg_mngr->Process_ALG_v6(mapping, (char*)ip6, &alg_input_size))
			return -1; /* something went terribly wrong */
		
	} else { /* ICMP */
		/* ICMP transtions must be expanded to incorporate other message types 	*/
		/* as well as other codes, also translations of encapsulated headers 	*/
		/* must be performed.													*/
		
		icmp6_hdr *icmp = (icmp6_hdr*)(packet_v6 + sizeof(ether_header) + sizeof(ip6_hdr));
		ip6->ip6_nxt = IPPROTO_ICMPV6;
		
		if (!Translate_ICMPv4_code(&icmp->icmp6_type, &icmp->icmp6_code))
			return -1;
		
		memcpy(packet_v6 + sizeof(ether_header) + sizeof(ip6_hdr) + 6, &port, sizeof(u16));
		
		icmp->icmp6_cksum = compute_checksum_ICMP_v6(ip6, icmp);
	}
	
	return (htons(ip6->ip6_plen) + sizeof(ether_header) + sizeof(ip6_hdr));
}

bool Translate_ICMPv4_code(u8 *type, u8 *code)
{
	u8 icmp_type(*type);
	u8 icmp_code(*code);
	
	switch (icmp_type) {
		case ICMP_ECHO:
			*type = ICMP6_ECHO_REQUEST;
			break;

		case ICMP_ECHOREPLY:
			*type = ICMP6_ECHO_REPLY;
			break;

		case ICMP_UNREACH:
			*type = ICMP6_DST_UNREACH;

			switch (icmp_code) {
				case ICMP_UNREACH_NET:
					*code = ICMP6_DST_UNREACH_NOROUTE;
					break;
				
				case ICMP_UNREACH_HOST:
					*code = ICMP6_DST_UNREACH_NOROUTE;
					break;
				
				case ICMP_UNREACH_PROTOCOL:
					*type = ICMP6_PARAM_PROB;
					*code = ICMP6_PARAMPROB_NEXTHEADER;
					break;
				
				case ICMP_UNREACH_PORT:
					*code = ICMP6_DST_UNREACH_NOPORT;
					break;
				
				case ICMP_UNREACH_NEEDFRAG:
					*type = ICMP6_PACKET_TOO_BIG;
					*code = ICMP6_DST_UNREACH_NOROUTE;
					break;
				
				case ICMP_UNREACH_SRCFAIL:
					*code = ICMP6_DST_UNREACH_BEYONDSCOPE;
					break;
				
				case ICMP_UNREACH_NET_UNKNOWN:
					*code = ICMP6_DST_UNREACH_NOROUTE;
					break;
				
				case ICMP_UNREACH_HOST_UNKNOWN:
					*code = ICMP6_DST_UNREACH_NOROUTE;
					break;
				
				case ICMP_UNREACH_ISOLATED:
					*code = ICMP6_DST_UNREACH_NOROUTE;
					break;
				
				case ICMP_UNREACH_NET_PROHIB:
					*code = ICMP6_DST_UNREACH_ADMIN;
					break;
				
				case ICMP_UNREACH_HOST_PROHIB:
					*code = ICMP6_DST_UNREACH_ADMIN;
					break;
				
				case ICMP_UNREACH_TOSNET:
					*code = ICMP6_DST_UNREACH_NOROUTE;
					break;
				
				case ICMP_UNREACH_TOSHOST:
					*code = ICMP6_DST_UNREACH_NOROUTE;
					break;
				
				case ICMP_UNREACH_FILTER_PROHIB:
					*code = ICMP6_DST_UNREACH_ADMIN;
					break;
				
				case ICMP_UNREACH_HOST_PRECEDENCE:
					*code = ICMP6_DST_UNREACH_ADMIN;
					break;
				
				case ICMP_UNREACH_PRECEDENCE_CUTOFF:
					*code = ICMP6_DST_UNREACH_ADMIN;
					break;
				
				default:
					return false;
			}
			break;

		case ICMP_TIMXCEED:
			*type = ICMP6_TIME_EXCEEDED;
			break;

		case ICMP_PARAMPROB:
			*type = ICMP6_PARAM_PROB;
			break;

		default:
			return false;
	}
	
	return true;
}

bool Send_v6_Packet(const listen_interface *send, const char *packet, u32 size)
{
	int bytes_written = write(send->fd, packet, size);
	if (bytes_written == -1) {
		pLog->write("Error writing IPv6 packet (size: %u) to %s interface: %s", 
			size, send->name, strerror(errno));
		++global_stats.v6_packets_output_errors;
		
		return false;
	}
	
	++global_stats.v6_packets_output_packets;
	
	return true;
}

void Create_IPv6_Header(char *packet_v6, const char *packet_v4, in6_addr *v6_addr)
{
	iphdr *ip = (iphdr*)packet_v4;
	ip6_hdr *ip6 = (ip6_hdr*)packet_v6;
	
	const u32 v6_packet_header_size = 40;
	memset(packet_v6, 0, v6_packet_header_size);

	u32 v4_packet_payload = ntohs(ip->tot_len) - (ip->ihl * 4);
	
	packet_v6[0]  = (u8) 0x60;
	ip6->ip6_plen = htons(v4_packet_payload);
	packet_v6[6] = ip->protocol;
	packet_v6[7] = ip->ttl - 1; /* decrement TTL */
	
	memcpy(packet_v6 + 8, &v6_prefix, 12);
	memcpy(packet_v6 + 20, &ip->saddr, 4);
	memcpy(packet_v6 + 24, v6_addr, 16);
}

in_addr get_interface_v4_addr(listen_interface *i)
{
	ifreq ifr;
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, i->name, strlen(i->name));
	
	if (ioctl(i->fd, SIOCGIFADDR, (ifreq*)&ifr) == -1) {
		pLog->write("unable to get IPv4 address from interface %s: ", i->name,
			strerror(errno));
		clean_up(-1);
	}

	sockaddr *addr = &ifr.ifr_addr;
	in_addr v4_addr;
	memcpy(&v4_addr, &addr->sa_data[2], 4);

	return v4_addr;
}
