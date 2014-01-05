/***************************************************************************
 *  ataga.h : This file is part of 'ataga'
 *  created on: Mon Jun  6 15:49:18 CDT 2005
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

#ifndef __ATAGA_H_
#define __ATAGA_H_

#include "settings.h"
#include "nat-mngr.h"
#include "log.h"

extern NAT_Manager 		*pNat;
extern CLog				*pLog;
extern CSettings		*pSettings;
extern pthread_cond_t 	main_thread_wait;
extern pthread_cond_t 	ndisc_helper_thread_wait;
extern pthread_cond_t 	dns_helper_thread_wait;
extern in6_addr 		v6_prefix;

typedef enum {
	in_processing,
	awaiting_DNS_request,
	awaiting_ND_request,
	awaiting_ARP_request,
	ready_for_processing,
	ready_for_discarding
} packet_status;
	
struct statistics {
	u32 v6_packets_input_packets;
	u32 v6_packets_input_errors;
	u32 v6_packets_output_packets;
	u32 v6_packets_output_errors;
	u32 v4_packets_input_packets;
	u32 v4_packets_output_packets;
	u32 v4_packets_input_errors;
	u32 v4_packets_output_errors;
	u32 tcp_translations_created;
	u32 udp_translations_created;
	u32 icmp_translations_created;
	u32 startup_time;
};

//#include <pcap-bpf.h>

struct listen_interface {
	const char 			*name;
	int 				fd;
	char 				hw_addr[6];
	u16					link_mtu;
	//bpf_insn 			*filter;
	listen_interface 	*next;
};

struct IPv6_pseudo_hdr {
	struct in6_addr src_addr;
	struct in6_addr dst_addr;
	u_long dgram_length;
	u_long next_hdr;
};

struct pending_packet {
	char *packet_start;
	u32 packet_size;
	packet_status status;
	pending_packet *next;
};
	
struct DNS_find {
	in_addr	addr;
	u16 port;
	NAT_Manager::protocol_type prot;
	pending_packet *packet;
	DNS_find *next;
};

struct ND_find {
	in6_addr lookup;
	in6_addr source;
	pending_packet *packet;
	ND_find *next;
};

struct ARP_find {
	in_addr lookup;
	in_addr source;
	pending_packet *packet;
	ARP_find *next;
};

void periodic_clean(int code);
void clean_up(int code);
void print_usage(void);
void read_NAT_config(int*);
listen_interface *read_int_config(int*, int);
int initialise_network_int(const char*, int);
void get_int_hardware_address(listen_interface*);
void clean_network_int(listen_interface*);
int create_fd_set(listen_interface *i, fd_set *set);
void get_int_mtu(listen_interface*);
void StatsInit();
void DumpStats();
bool CheckPID(char *pid_file);
bool CreatePID(char *pid_file);
bool RemovePID(char *pid_file);
pending_packet *AllocatePacket(const char *buffer, u32 buffer_size);
void Add_Pending_Packet(pending_packet **base, pending_packet *packet);
pending_packet *Remove_Pending_Packet(pending_packet *packet);
void Clear_Pending_Packets(pending_packet *base);
int Process_Pending_Packets(pending_packet **base, char **read_buffer);

const u32 packet_size = 1600; 	// this maybe a problem and maybe need to be 
								// adjusted in some dynamic form in the future
								// as the use of jumbograms may become common
								// for now it works perfectly as it is
								
extern statistics		global_stats;

#ifndef NAPT_PLUGIN_DIR
#define NAPT_PLUGIN_DIR "/usr/lib/naptd/plugins"
#endif // NAPT_PLUGIN_DIR

#ifndef NAPT_PID_FILE
#define NAPT_PID_FILE "/tmp/naptd.pid"
#endif // NAPT_PID_FILE

#ifndef NAPT_CONF_FILE
#define NAPT_CONF_FILE "/etc/naptd.conf"
#endif // NAPT_CONF_FILE

#ifndef ETHERTYPE_IP
#define ETHERTYPE_IP 0x0800
#endif // ETHERTYPE_IP

#ifndef ETHERTYPE_IPV6
#define ETHERTYPE_IPV6 0x86dd
#endif // ETHERTYPE_IPV6

#include "enum-settings.h"

#endif // __ATAGA_H_
