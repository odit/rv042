/***************************************************************************
 *  v6_handler.cc : This file is part of 'ataga'
 *  created on: Tue Jun  7 22:45:04 CDT 2005
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
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/if_ether.h>
#include <sys/socket.h>
#include "v6_handler.h"
#include "alg.h"
#include "arp_handler.h"
#include "v4_handler.h"

listen_interface 	*v6_interfaces;
pending_packet		*v6_pending_packets = 0;
//int				outbound_v4_socket(0);

void *v6_handler_init(void *p)
{
	/* init stuff */
	v6_interfaces = (listen_interface *)p;
	listen_interface *i = v6_interfaces;
	
	//if (!initialise_outbound_interface(&outbound_v4_socket))
	//	clean_up(-1);
	
	while (i) {
		i->fd = initialise_network_int(i->name, ETH_P_ALL);
		get_int_hardware_address(i);
		get_int_mtu(i);
		
		i = i->next;
	}
	
	/* end of init stuff */
	
	/* main loop */
	bool working = true;
	NAT_Manager::v4_addr v4_addr;
	NAT_Manager::addr_mapping *mapping;
	char read_buffer[packet_size];
	char *packet_buffer;
	int bytes_read;
	
	while (working) {
		/* read pending packets */
		bytes_read = Process_Pending_Packets(&v6_pending_packets, &packet_buffer);
		
		if (!bytes_read) {
			bytes_read = Get_Data_From_v6_Sockets(read_buffer);
			packet_buffer = read_buffer;
		}
		
		if (!bytes_read) /* I wonder how this happened... */
			continue;
		
		ip6_hdr *ip6 = (ip6_hdr*)(packet_buffer + sizeof(ether_header));
	//	u8 nxt_hdr= Get_Next_Header(ip6);
		
		if (ip6->ip6_nxt == IPPROTO_TCP) {
			tcphdr *tcp = (tcphdr*)(packet_buffer + sizeof(ether_header) + sizeof(ip6_hdr));

#ifdef _DEBUG_
			if (!tcp->source || !tcp->dest) {
				printf("alarm!\n");
				printf("strange port on v6 side!\n");
				FILE *hFile = fopen("bad.packets.log", "a+");
				if (!hFile)
					hFile = fopen("bad.packets.log", "x+");
				if (hFile) {
					fwrite(packet_buffer, bytes_read, 1, hFile);
					fprintf(hFile, "\n\n");
					fclose(hFile);
				}
			}
#endif // _DEBUG_
			
			if (tcp->syn && !tcp->ack) { /* new connection */
				mapping = pNat->Get_v6_to_v4_Translation((in6_addr*)&ip6->ip6_src, 
					tcp->source, &v4_addr, true, NAT_Manager::TCP);
				
				/* Don't have it, don't bother */
				if (!mapping)
					continue;
				
				Translate_v6_Send_v4_Packet(mapping, packet_buffer, bytes_read, &v4_addr.addr, 
					(v4_addr.port ? htons(v4_addr.port) : tcp->source));
						
			} else if (tcp->fin || tcp->rst) {
				mapping = pNat->Get_v6_to_v4_Translation((in6_addr*)&ip6->ip6_src, 
					tcp->source, &v4_addr, false, NAT_Manager::TCP);
				
				/* Don't have it, don't bother */
				if (!mapping)
					continue;
				
				Translate_v6_Send_v4_Packet(mapping, packet_buffer, bytes_read, &v4_addr.addr, 
					(v4_addr.port ? htons(v4_addr.port) : tcp->source));
				
				/* prepare to remove connection */
				pNat->Prep_Remove_Mapping(mapping);
			} else {
				mapping = pNat->Get_v6_to_v4_Translation((in6_addr*)&ip6->ip6_src, 
					tcp->source, &v4_addr, false, NAT_Manager::TCP);
				
				/* Don't have it, don't bother */
				if (!mapping)
					continue;
				
				Translate_v6_Send_v4_Packet(mapping, packet_buffer, bytes_read, &v4_addr.addr, 
					(v4_addr.port ? htons(v4_addr.port) : tcp->source));
			}
		} else if (ip6->ip6_nxt == IPPROTO_UDP) {
			udphdr *udp = (udphdr*)(packet_buffer + sizeof(ether_header) + sizeof(ip6_hdr));
			
			mapping = pNat->Get_v6_to_v4_Translation((in6_addr*)&ip6->ip6_src, 
				udp->source, &v4_addr, true, NAT_Manager::UDP);
			
			/* Don't have it, don't bother */
			if (!mapping)
				continue;
			
			Translate_v6_Send_v4_Packet(mapping, packet_buffer, bytes_read, &v4_addr.addr, 
				(v4_addr.port ? htons(v4_addr.port) : udp->source));
		} else if (ip6->ip6_nxt == IPPROTO_ICMPV6) {
			icmp6_hdr *icmp = (icmp6_hdr*)(packet_buffer + sizeof(ether_header) + sizeof(ip6_hdr));
			
			if (!icmp->icmp6_seq)
				continue;
	
			mapping = pNat->Get_v6_to_v4_Translation((in6_addr*)&ip6->ip6_src, 
				icmp->icmp6_seq, &v4_addr, true, NAT_Manager::ICMP);
			
			/* Don't have it, don't bother */
			if (!mapping)
				continue;
			
			Translate_v6_Send_v4_Packet(mapping, packet_buffer, bytes_read, &v4_addr.addr, 
				(v4_addr.port ? htons(v4_addr.port) : icmp->icmp6_seq));
		} else {
			/* don't know this one */
			continue;
		}
	}
	
	/* clean_up */
	/* close outbound_v4_socket */
	
	//close(outbound_v4_socket);
	clean_network_int(v6_interfaces);
	Clear_Pending_Packets(v6_pending_packets);
	
	return 0;
}

u8 Get_Next_Header(ip6_hdr *ip6)
{
	u8 next = ip6->ip6_nxt;
	//u8 move = ip6->
	while (next != IPPROTO_TCP || next != IPPROTO_UDP || next != IPPROTO_ICMP) {
		
		next = ip6->ip6_nxt;
	} 
	return (next);
}

void Request_ARP(in_addr *addr, in_addr *v4_addr, const char *buffer, u32 buffer_size)
{
	pending_packet *packet = AllocatePacket(buffer, buffer_size);
	packet->status = awaiting_ARP_request;
	
	Add_ARP_Request(addr, v4_addr, packet);
	Add_Pending_Packet(&v6_pending_packets, packet);
}

bool GetHardwareAddr_v4(const listen_interface **send, in_addr *dst, in_addr *src, char *packet, u32 size)
{
	in_addr gw, *res;
	
	if (FindRoute_v4(send, &gw, dst, packet))
		res = &gw;
	else
		res = dst;
	
	if (!GetARP(send, res, packet)) {
		if (size)
			Request_ARP(res, src, packet, size);
		
		return false;
	}
	return true;
}

void SendICMP_MsgTooBig_v6(ip6_hdr *src, const listen_interface *i)
{
#ifdef _DEBUG_
	pLog->write("SendICMP_MsgTooBig_v6() called");
#endif
	
	int packet_size = sizeof(ether_header) + sizeof(ip6_hdr) + sizeof(icmp6_hdr) + sizeof(ip6_hdr) + 8;
	char packet[packet_size];
	
	const listen_interface *sending_int;
	if (!GetHardwareAddr_v6(&sending_int, (in6_addr*)&src->ip6_src, (in_addr*)((u8*)&src->ip6_dst + 12), packet, 0))
		return;
				  
	ip6_hdr *ip6 = (ip6_hdr*)(packet + sizeof(ether_header));
	icmp6_hdr *icmp6 = (icmp6_hdr*) (packet + sizeof(ether_header) + sizeof(ip6_hdr));

	memset(packet, 0, packet_size);
	memcpy(packet + sizeof(ether_header) + sizeof(ip6_hdr) + sizeof(icmp6_hdr), src, sizeof(ip6_hdr) + 8);

	*(u8*)ip6 = (u8)0x60;
	ip6->ip6_hlim = 255;
	ip6->ip6_nxt = IPPROTO_ICMPV6;
	ip6->ip6_plen = htons(sizeof(icmp6_hdr) + sizeof(ip6_hdr) + 8);
	memcpy(&ip6->ip6_src, &src->ip6_dst, sizeof(in6_addr));
	memcpy(&ip6->ip6_dst, &src->ip6_src, sizeof(in6_addr));

  	icmp6->icmp6_type = ICMP6_DST_UNREACH;
	icmp6->icmp6_code = 0;
	icmp6->icmp6_mtu = htons(i->link_mtu - (sizeof(iphdr) - sizeof(ip6_hdr)));
  	icmp6->icmp6_cksum = compute_checksum_ICMP_v6(ip6, icmp6);
	
	int bytes_written = write(sending_int->fd, packet, packet_size); 
	
	// = sendto(outbound_v4_socket, packet, packet_size, 0, (sockaddr*)&servaddr, sizeof(sockaddr_in));
	
	if (bytes_written == -1) {
		pLog->write("Error while sending Message Too Big packet to IPv6 host: %s",
				strerror(errno));
		++global_stats.v4_packets_output_errors;
				
		return;
	}
	
	++global_stats.v4_packets_output_packets;
	
	return;
}

int Get_Data_From_v6_Sockets(char *read_buffer)
{
	fd_set read_set;
	int highest_fd = create_fd_set(v6_interfaces, &read_set);
	while (!select(highest_fd + 1, &read_set, 0, 0, 0)) {
		pLog->write("Error while listening on IPv6 interfaces: %s", strerror(errno));
		++global_stats.v6_packets_input_errors;

		continue;
	}
	
	int bytes_read(0);
	listen_interface *i = v6_interfaces;
	while (i) {
		if (FD_ISSET(i->fd, &read_set)) {
			/* something interesting ;) */
				
			char *packet_buffer = read_buffer;
			bytes_read = read(i->fd, packet_buffer, packet_size);
				
	    	if (bytes_read == -1) {
				pLog->write("Error while reading IPv6 packet on %s interface: %s", i->name, strerror(errno));
				i = i->next;
				bytes_read = 0;
				++global_stats.v6_packets_input_errors;

				continue;
			}
			ether_header *eth = (ether_header*) packet_buffer;

			if (memcmp(&eth->ether_dhost, i->hw_addr, ETH_ALEN)) {
				i = i->next;
				continue;
			}
		
			if (eth->ether_type != htons(ETHERTYPE_IPV6)) {
				i = i->next;
				continue;
			}
			
			ip6_hdr *ip6 = (ip6_hdr*)(packet_buffer + sizeof(ether_header));
			
			if (memcmp(&ip6->ip6_dst, &v6_prefix, 12))
				continue;
			
			++global_stats.v6_packets_input_packets;
						
			return bytes_read;	
		}
		i = i->next;
	}
	
	return 0;
}

bool CheckMTU_v4(const char *packet_v6, u32 total_length, const listen_interface *i)
{
	if (total_length - sizeof(ether_header) > i->link_mtu) {
		/* packet TOO big! */
		/* inform source about this */
		ip6_hdr *ip6 = (ip6_hdr*)(packet_v6 + sizeof(ether_header));
		SendICMP_MsgTooBig_v6(ip6, i);
		
		return false;
	}
	
	return true;
}

bool Translate_v6_Send_v4_Packet(NAT_Manager::addr_mapping *mapping, char *packet_v6, u32 size, in_addr *v4_addr, u16 port)
{
	const listen_interface *sending_int; 	/* here we hold a pointer to the   	*/
											/* outbound IPv4 interface that we 	*/
											/* will use for sending 			*/
	
	if (!GetHardwareAddr_v4(&sending_int, (in_addr*)((u8*)packet_v6 + sizeof(ether_header) + 36), v4_addr, packet_v6, size))
		return false;
	
	char packet_v4[packet_size];
	
	int total_length = Translate_v6_Packet(mapping, packet_v4, packet_v6, v4_addr, port);
	
	/* error checking */
	if (total_length == -1)
		return false;
	
	/* check MTU size */
	if (!CheckMTU_v4(packet_v6, total_length, sending_int))
		return false;
	
	Send_v4_Packet(sending_int, packet_v4, total_length);
	
	return true;
}

int Translate_v6_Packet(NAT_Manager::addr_mapping *mapping, char *packet_v4, char *packet_v6, in_addr *v4_addr, u16 port)
{
	/* copy the ethernet header as we have already written the correct values */
	/* to it in GetNeighbor(&sending_int, v6_addr, packet_v4)				  */
	memcpy(packet_v4, packet_v6, sizeof(ether_header));
	
	/* create basic IPv4 header */
	iphdr *ip = (iphdr*)(packet_v4 + sizeof(ether_header));
	Create_IPv4_Header(packet_v4 + sizeof(ether_header), packet_v6 + sizeof(ether_header), v4_addr);
	
	/* copy the payloads */
	ip6_hdr *ip6 = (ip6_hdr*)(packet_v6 + sizeof(ether_header));
	
	/* check TTL (Hops), discard if the 1 or less */
	if (ip6->ip6_hops <= 1)
		return -1;
	
	memcpy(packet_v4 + sizeof(ether_header) + (ip->ihl * 4), packet_v6 + sizeof(ether_header) + 
		sizeof(ip6_hdr), htons(ip6->ip6_plen));
	
	if (ip->protocol != IPPROTO_ICMPV6) { /* TCP || UDP */
		/* set port */
		memcpy(packet_v4 + sizeof(ether_header) + sizeof(iphdr), &port, sizeof(u16));
		
		/* do Application Level Gateway processing + checksum updating */
		u32 alg_input_size = htons(ip->tot_len);
		if (!alg_mngr->Process_ALG_v4(mapping, (char*)ip, &alg_input_size))
			return -1; /* something went terribly wrong */
	} else { /* ICMP */
		icmphdr *icmp = (icmphdr*)(packet_v4 + sizeof(ether_header) + (ip->ihl * 4));
		ip->protocol = IPPROTO_ICMP;
		
		if (!Translate_ICMPv6_code(&icmp->type, &icmp->code))
			return -1;
		
		/* set sequence number */
		memcpy(packet_v4 + sizeof(ether_header) + (ip->ihl * 4)+ 6, &port, sizeof(u16));
		
		icmp->checksum = 0;
		icmp->checksum = compute_checksum_ICMP_v4(ip, icmp);
	}
	/* IP header require checksums */
	ip->check = in_cksum ((u16*)ip, ip->ihl * 4);
	
	return (htons(ip->tot_len) + sizeof(ether_header));
}

bool Translate_ICMPv6_code(u8 *type, u8 *code)
{
	u8 icmp_type(*type);
	u8 icmp_code(*code);
	
	switch (icmp_type) {
		case ICMP6_DST_UNREACH:
			*type = ICMP_UNREACH;

			switch (icmp_code) {
				case ICMP6_DST_UNREACH_NOROUTE:
					*code = ICMP_UNREACH_HOST;
					break;
				
				case ICMP6_DST_UNREACH_ADMIN:
					*code = ICMP_UNREACH_HOST_PROHIB;
					break;
				
				case ICMP6_DST_UNREACH_BEYONDSCOPE:
					*code = ICMP_UNREACH_SRCFAIL;
					break;
				
				case ICMP6_DST_UNREACH_ADDR:
					*code = ICMP_UNREACH_HOST;
					break;
				
				case ICMP6_DST_UNREACH_NOPORT:
					*code = ICMP_UNREACH_PORT;
					break;
				
				default:
					return false;
			}
			break;

		case ICMP6_PACKET_TOO_BIG:
			*type = ICMP_UNREACH;
			*code = ICMP_UNREACH_NEEDFRAG;
		break;

		case ICMP6_TIME_EXCEEDED:
			*type = ICMP_TIMXCEED;
		break;

		case ICMP6_PARAM_PROB:
			switch (icmp_code) {
				case ICMP6_PARAMPROB_NEXTHEADER:
					*type = ICMP_UNREACH;
					*code = ICMP_UNREACH_PROTOCOL;
					break;
				
				default:
					*type = ICMP_PARAMPROB;
					*code = 0;
			}
			break;

		case ICMP6_ECHO_REQUEST:
			*type = ICMP_ECHO;
			break;

		case ICMP6_ECHO_REPLY:
			*type = ICMP_ECHOREPLY;
			break;

		default:
			return false;
	}
	
	return true;
}

bool Send_v4_Packet(const listen_interface *send, const char *packet, u32 size)
{
	int bytes_written = write(send->fd, packet, size);

	if (bytes_written == -1) {
		pLog->write("Error writing IPv4 packet to global IPv4 outbound interface: %s.", 
			strerror(errno));
		++global_stats.v4_packets_output_errors;
		
		return false;
	}
	
	++global_stats.v4_packets_output_packets;
	
	return true;
}

void Create_IPv4_Header(char *packet_v4, const char *packet_v6, in_addr *v4_addr)
{
	iphdr *ip = (iphdr*)packet_v4;
	ip6_hdr *ip6 = (ip6_hdr*)packet_v6;
	
	const u32 v4_packet_header_size = 20;
	memset(packet_v4, 0, v4_packet_header_size);
	
	ip->tot_len = htons(v4_packet_header_size + ntohs(ip6->ip6_plen));

	packet_v4[0] = (u8) 0x45;
	packet_v4[6] = (u8) ((65535 & IP_DF) >> 8);

	packet_v4[8] = ip6->ip6_hops - 1; /* decrement Hops */
	packet_v4[9] = ip6->ip6_nxt;
	
	memcpy(packet_v4 + 12, v4_addr, 4);
	memcpy(packet_v4 + 16, ((u8*)&ip6->ip6_dst) + 12, 4);
}

bool initialise_outbound_interface(int *sock)
{
	*sock = socket(AF_INET, SOCK_RAW, ETH_P_ALL);
	if (*sock  < 0) {
		pLog->write("Error while creating global outbound IPv4 socket: %s", strerror(errno));
		return false;
	}
	
	int on(1);
	if (setsockopt (*sock , IPPROTO_IP, IP_HDRINCL, (const char*)&on, sizeof (on)) == -1) {
		pLog->write("Error while setting IP options on global outbound IPv4 socket: %s", strerror(errno));
		return false;
	}
	
	return true;
}
