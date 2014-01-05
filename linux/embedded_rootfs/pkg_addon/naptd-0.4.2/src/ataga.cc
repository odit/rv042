/***************************************************************************
 *  ataga.cc : This file is part of 'ataga'
 *  created on: Sun Jun  5 10:18:59 CDT 2005
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
#include <signal.h>
#include <linux/if_packet.h>
#include "ataga.h"
#include "arp_handler.h"
#include "v4_handler.h"
#include "v6_handler.h"
#include "ndisc.h"
#include "dns_helper.h"
//#include "mysql.h"
#include "alg.h"

NAT_Manager 	*pNat;
CLog			*pLog;
CSettings		*pSettings;
alg_manager		*alg_mngr;
pthread_cond_t 	main_thread_wait;
in6_addr 		v6_prefix;
statistics		global_stats;
char 			pid_file[1024];
char 			config_file[1024];
	
int main(int argc, char** argv)
{
	/* check input arguments */
	/* display print_usage() is necessary */
	
	strcpy(pid_file, NAPT_PID_FILE);	
	strcpy(config_file, NAPT_CONF_FILE);	
	
	char o;
	bool debug(false);
	while ((o = getopt(argc, argv, "hgp:c:")) != -1) {
		switch (o) {
			case 'h':
				print_usage();
				exit(0);
			break;
			
			case 'g':
				debug = true;
			break;
			
			case 'p':
				strncpy(pid_file, optarg, 1024);
			break;
			
			case 'c':
				strncpy(config_file, optarg, 1024);
			break;
			
			default:
				continue;
		}
	}
	
	/* check and init pid file - we want only one copy of Ataga running */
	CheckPID(pid_file);
	
	/* init global stats */
	StatsInit();
	
	pSettings = new CSettings(config_file);
	
	/* read config file and prepare necessary data structures */
	
	int n(0);
	const char *name = (const char*)pSettings->GetPointerData(n++);
	
	if (!name) {
		printf("configuration file not found, please run naptd-confmaker");
		clean_up(0);
	}
	
	pLog = new CLog(debug);
	
	pLog->write("Starting NAT-PT.");
	
	if (!debug) {
		/* go daemon */
		printf("Going daemon. Check syslog messages for runtime information.\n");
	
		if (fork()) {
			delete pSettings;
			exit(0);
		}
	}
	
	/* main init */
	srand(time(0));
	pNat = new NAT_Manager;
	
	read_NAT_config(&n);
	int save_n(n);
	listen_interface *v6_int = read_int_config(&save_n, number_of_inside_interfaces);
	listen_interface *nd_int = read_int_config(&n, number_of_inside_interfaces);
	save_n = n;
	listen_interface *v4_int = read_int_config(&save_n, number_of_outside_interfaces);
	save_n = n;
	listen_interface *arp_help_int = read_int_config(&save_n, number_of_outside_interfaces);
	listen_interface *arp_handle_int = read_int_config(&n, number_of_outside_interfaces);
	
	u32 tcp_translation_time(0), udp_translation_time(0), icmp_translation_time(0);
	pSettings->Get(tcp_translation_time, translation_time_tcp);
	pSettings->Get(udp_translation_time, translation_time_udp);
	pSettings->Get(icmp_translation_time, translation_time_icmp);
	
	pNat->Set_Translation_Times(tcp_translation_time, udp_translation_time, icmp_translation_time);
	
	pSettings->Get(v6_prefix, ipv6_prefix);
	
	signal(SIGINT, clean_up);
	signal(SIGTERM, clean_up);
	
	/* Init Application Level Gateways */
	pLog->write("Initializing Application Level Gateways.");
	alg_mngr = new alg_manager(NAPT_PLUGIN_DIR);
	
	/* dispatch threads */
	pLog->write("Dispatching threads.");
	
	pthread_t pthread;
	
	if (pthread_create(&pthread, 0, v4_handler_init, v4_int)) {
		pLog->write("unable to create thread for handling IPv4 packets.");
		clean_up(-1);
	}
	
	if (pthread_create(&pthread, 0, v6_handler_init, v6_int)) {
		pLog->write("unable to create thread for handling IPv6 packets.");
		clean_up(-1);
	}
	
	if (pthread_create(&pthread, 0, arp_handler_init, arp_handle_int)) {
		pLog->write("unable to create thread for handling ARP replies.");
		clean_up(-1);
	}
	
	if (pthread_create(&pthread, 0, arp_helper_init, arp_help_int)) {
		pLog->write("unable to create thread for handling ARP requests.");
		clean_up(-1);
	}
	
	if (pthread_create(&pthread, 0, ndisc_handler_init, nd_int)) {
		pLog->write("unable to create thread for handling Neighbor Discovery requests.");
		clean_up(-1);
	}
	
	if (pthread_create(&pthread, 0, dns_helper_init, 0)) {
		pLog->write("unable to create thread for handling dynamic DNS address mapping.");
		clean_up(-1);
	}
	
	/* installing regural NAT mapping pool cleaning */
	signal(SIGALRM, periodic_clean);
	alarm(NAT_Manager::NAT_pool_clean_interval);
	
	sleep(1);
	
	/* Find IPv4 Routes */
	pLog->write("Loading IPv4 routes into cache.");
	UpdateRouteCache_v4();
	
	/* Find IPv6 Routes */
	pLog->write("Loading IPv6 routes into cache.");
	UpdateRouteCache_v6();
	
#ifndef _DEBUG_
	
	pLog->write("Dropping root privileges.");
	setuid(getpid());

#endif // _DEBUG_
 
	/* create PID */
	if (!CreatePID(pid_file)) {
		pLog->write("Error creating PID file: %s", strerror(errno));
		clean_up(0);
	}
	
	/* log program initialization */
	pLog->write("NAT-PT initialized.");
	
	pthread_cond_init(&main_thread_wait, 0);
	pthread_mutex_t wait_mutex = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_lock(&wait_mutex);
	pthread_cond_wait(&main_thread_wait, &wait_mutex);
	pthread_mutex_unlock(&wait_mutex);
	
	clean_up(0);
	
	return 0;
}

void periodic_clean(int code)
{
	UpdateRouteCache_v4();
	UpdateRouteCache_v6();
	pNat->Clean_Pool();
	alarm(NAT_Manager::NAT_pool_clean_interval);
}

void clean_up(int code)
{
	static bool entered(0);
	
	if (entered)
		return;
	
	entered = 1;
	
	signal(SIGALRM, SIG_IGN);
	
	pthread_cond_signal(&main_thread_wait);
	
	/* clear pid file */
	RemovePID(pid_file);
	
	/* wait for every other thread to close */
	sleep(1);
	
	pLog->write("Freeing memory for IPv4 interfaces.");
	clean_network_int(v4_interfaces);
	pLog->write("Freeing memory for IPv6 interfaces.");
	clean_network_int(v6_interfaces);
	
	pLog->write("Freeing memory for pending IPv4 packets.");
	Clear_Pending_Packets(v4_pending_packets);
	pLog->write("Freeing memory for pending IPv6 packets.");
	Clear_Pending_Packets(v6_pending_packets);
	
	pLog->write("Clearing IPv4 ARP cache.");
	clean_network_int(arp_help_interfaces);
	ClearARPCache(arp_cache);
	
	pLog->write("Clearing IPv6 neighbor cache.");
	clean_network_int(ndisc_interfaces);
	ClearNeighborCache(nd_cache);
	
	pLog->write("Clearing IPv4 route cache.");
	ClearRoutes_v4();
	pLog->write("Clearing IPv6 route cache.");
	ClearRoutes_v6();
	
	/* Dump Stats */
	DumpStats();
	
	/* Delete all plugins */
	delete alg_mngr;
	
	/* Delete all mappings */
	pLog->write("Releasing address mappings.");
	delete pNat;
	
	/* Close config */
	pLog->write("Closing configuration file.");
	delete pSettings;
	
	/* log program shutdown */
	pLog->write("NAT-PT shutdown complete.");
	pLog->write("Exiting with code %i.", code);
	delete pLog;
	
	/* exit (hope everything went ok;) */
	exit(code);
}

void DumpStats()
{
	pLog->write("Program Statistics");
	
	u32 days, hours, minutes, seconds, total_time(time(0) - global_stats.startup_time);
	days = total_time / 86400;
	total_time = total_time % 86400;
	hours = total_time / 3600;
	total_time = total_time % 3600;
	minutes = total_time / 60;
	seconds = total_time % 60;
	
	pLog->write("NAT-PT was up for %u days, %u hours, %u minutes and %u seconds", 
		days, hours, minutes, seconds);
	
	pLog->write("Total Translations Performed");
	pLog->write("TCP: %u", global_stats.tcp_translations_created);
	pLog->write("UDP: %u", global_stats.udp_translations_created);
	pLog->write("ICMP: %u", global_stats.icmp_translations_created);
	
	pLog->write("IPv6 Interface Statistics");
	pLog->write("Input packets: %u", global_stats.v6_packets_input_packets);
	pLog->write("Output packets: %u", global_stats.v6_packets_output_packets);
	pLog->write("Input errors: %u", global_stats.v6_packets_input_errors);
	pLog->write("Output errors: %u", global_stats.v6_packets_output_errors);

	pLog->write("IPv4 Interface Statistics");
	pLog->write("Input packets: %u", global_stats.v4_packets_input_packets);
	pLog->write("Output packets: %u", global_stats.v4_packets_output_packets);
	pLog->write("Input errors: %u", global_stats.v4_packets_input_errors);
	pLog->write("Output errors: %u", global_stats.v4_packets_output_errors);
}

void print_usage(void)
{
	puts("Ataga Network Address and Protocol Translator v0.4.2");
	puts("(c) 2005-2010 by Lukasz Tomicki <lucas.tomicki@gmail.com>");
	puts(" -h display this help");
	puts(" -g debug mode, doesn't fork and prints all messages to console");
	printf(" -p [%s] specify non-default PID file\n", NAPT_PID_FILE);
	printf(" -c [%s] specify non-default configuration file\n", NAPT_CONF_FILE);
}

bool CheckPID(char *pid_file)
{
	FILE *hFile = fopen(pid_file, "r");
	
	if (hFile) {
		char pid_number[16];
		fgets(pid_number, 16, hFile);
		kill(atoi(pid_number), SIGTERM);
		RemovePID(pid_file);
		fclose(hFile);
	}
	
	return true;
}

bool CreatePID(char *pid_file)
{
	FILE *hFile = fopen(pid_file, "w+");
	
	if (!hFile)
		return false;
	
	fprintf(hFile, "%u", getpid());
	fclose(hFile);
	
	return true;
}

bool RemovePID(char *pid_file)
{
	unlink(pid_file);
	
	return true;
}

void read_NAT_config(int *n)
{
	int size;
	pSettings->Get(size, number_of_public_ip_blocks);
	
	for (int i = 0; i < size; ++i) {
		const char *start_ip = (const char*)pSettings->GetPointerData((*n)++);
		const char *end_ip = (const char*)pSettings->GetPointerData((*n)++);
		u16 start_port;
		memcpy(&start_port, pSettings->GetPointerData((*n)++), sizeof(u16));
		u16 end_port;
		memcpy(&end_port, pSettings->GetPointerData((*n)++), sizeof(u16));
		in_addr start_addr;
		inet_pton(AF_INET, start_ip, &start_addr);
		in_addr end_addr;
		inet_pton(AF_INET, end_ip, &end_addr);
		
		pNat->Add_Pool(start_addr, end_addr, start_port, end_port);
	}
	
	pSettings->Get(size, number_of_priv_blocks);
	
	for (int i = 0; i < size; ++i) {
		const char *start_ip = (const char*)pSettings->GetPointerData((*n)++);
		const char *end_ip = (const char*)pSettings->GetPointerData((*n)++);
		in_addr start_addr;
		inet_pton(AF_INET, start_ip, &start_addr);
		in_addr end_addr;
		inet_pton(AF_INET, end_ip, &end_addr);
		
		pNat->Add_Priv_Pool(start_addr, end_addr);
	}
	
	pSettings->Get(size, number_of_static_mappings);
	
	for (int i = 0; i < size; ++i) {
		const char *start_ip = (const char*)pSettings->GetPointerData((*n)++);
		const char *end_ip = (const char*)pSettings->GetPointerData((*n)++);
		in_addr start_addr;
		inet_pton(AF_INET, start_ip, &start_addr);
		in6_addr end_addr;
		inet_pton(AF_INET6, end_ip, &end_addr);
		
		pNat->Add_Static_v4_to_v6_Mapping(start_addr, end_addr);
	}
}

listen_interface *read_int_config(int *n, int side)
{
	int size;
	pSettings->Get(size, side);
	if (!size)
		return 0;
	
	listen_interface *int_list = new listen_interface, *org_int;
	org_int = int_list;
	
	for (int i = 0; i < size; ++i) {
		if (i + 1 < size)
			int_list->next = new listen_interface;
		else
			int_list->next = 0;
		
		int_list->name = (const char*)pSettings->GetPointerData((*n)++);
		int_list = int_list->next;
	}
	
	return org_int;
}

int initialise_network_int(const char *name, int protocol)
{
	int sockfd = socket(PF_PACKET, SOCK_RAW, htons(protocol));
    
	if (sockfd == -1) {
		pLog->write("Unable to create socket: %s", strerror(errno));
		clean_up(-1);
		
		return sockfd;
	}
	
	ifreq ifr;
		
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, name, strlen(name));
	
	if (ioctl(sockfd, SIOCGIFINDEX, &ifr) == -1) {
		pLog->write("Unable to get interface index: %s", strerror(errno));
		clean_up(-1);
		
		return -1;
	}

    sockaddr_ll sll;
		
	memset(&sll, 0, sizeof(sll));
    sll.sll_family = AF_PACKET;
    sll.sll_ifindex = ifr.ifr_ifindex;
    sll.sll_protocol = htons(protocol);
	
	if (bind(sockfd, (sockaddr*) &sll, sizeof(sll)) == -1) {
		pLog->write("Unable to bind socket: %s", strerror(errno));
		clean_up(-1);
		
		return -1;
	}
	
	if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) == -1) {
		pLog->write("Unable to check promiscuous mode on socket: %s", strerror(errno));
		clean_up(-1);
		
		return -1;
	}
	
	
	if (!(ifr.ifr_flags & IFF_PROMISC)) {
		ifr.ifr_flags |= IFF_PROMISC;
	    
		if (ioctl(sockfd, SIOCSIFFLAGS, &ifr) == -1) {
			pLog->write("Unable to set promiscuous mode on socket: %s", strerror(errno));
			clean_up(-1);
			
			return -1;
		}
	}
	
	return sockfd;
}

void get_int_hardware_address(listen_interface *i)
{
	ifreq ifr;
		
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, i->name, strlen(i->name));
	
	if (ioctl(i->fd, SIOCGIFHWADDR, &ifr) == -1) {
		pLog->write("Unable to get hardware address on interface %s: %s", i->name, 
			strerror(errno));
		clean_up(-1);
			
		return;
	}
	
	sockaddr *addr = (sockaddr*) &ifr.ifr_addr;
    
	memcpy(i->hw_addr, &addr->sa_data, 6);
}

void get_int_mtu(listen_interface *i)
{
	ifreq ifr;
		
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, i->name, strlen(i->name));
	
	if (ioctl(i->fd, SIOCGIFMTU, &ifr) == -1) {
		pLog->write("Unable to get MTU size on interface %s: %s", i->name, 
			strerror(errno));
		clean_up(-1);
			
		return;
	}
	
	i->link_mtu = ifr.ifr_mtu;
}

void clean_network_int(listen_interface *i)
{
	while (i) {
		listen_interface *save = i;
		close(i->fd);
		//if (i->filter)
		//	delete i->filter;
		
		i = save->next;
		
		delete save;
	}
}

int create_fd_set(listen_interface *i, fd_set *set)
{
	int max = i->fd;
	FD_ZERO(set);
	while (i) {
		FD_SET(i->fd, set);
		max = i->fd > max ? i->fd : max; 	// this suppossedly shouldn't be done ;)
											// as it's BAD to read and write into 
											// the same memory space in one statement
		i = i->next;
	}
	
	return max;
}

int Process_Pending_Packets(pending_packet **base, char **read_buffer)
{
	int bytes_read(0);
	pending_packet *save = 0, *i = *base;
	
	while (i) {
		if (i->status == ready_for_processing) {
			i->status = ready_for_discarding;
			*read_buffer = i->packet_start;
			bytes_read = i->packet_size;
			break;
		} else if (i->status == ready_for_discarding) {
			pending_packet *next = Remove_Pending_Packet(i);
			if (save)
				save->next = next;
			else
				*base = next;
			
			i = *base;
			save = 0;
			continue;
		}
		save = i;
		i = i->next;
	}
	return bytes_read;
}

void Add_Pending_Packet(pending_packet **base, pending_packet *packet)
{
	if (!*base)
		*base = packet;
	else {
		packet->next = *base;
		*base = packet;
	}
}

pending_packet *AllocatePacket(const char *buffer, u32 buffer_size)
{
	char *packet_buffer = new char[buffer_size];
	memcpy(packet_buffer, buffer, buffer_size);
	
	pending_packet *packet = new pending_packet;
	packet->packet_start = packet_buffer;
	packet->packet_size = buffer_size;
	packet->next = 0;
	
	return packet;
}

pending_packet *Remove_Pending_Packet(pending_packet *packet)
{
	pending_packet *save = packet->next;
	
	if (packet->packet_start)
		delete [] packet->packet_start;
	
	delete packet;
	
	return save;
}

void Clear_Pending_Packets(pending_packet *base)
{
	while (base) {
		base = Remove_Pending_Packet(base);
	}
}

void StatsInit()
{
	memset(&global_stats, 0, sizeof(statistics));
	global_stats.startup_time = time(0);
}
