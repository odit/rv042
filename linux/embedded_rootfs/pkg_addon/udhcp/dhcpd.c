/* dhcpd.c
 *
 * Moreton Bay DHCP Server
 * Copyright (C) 1999 Matthew Ramsay <matthewr@moreton.com.au>
 *			Chris Trew <ctrew@moreton.com.au>
 *
 * Rewrite by Russ Dill <Russ.Dill@asu.edu> July 2001
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <time.h>
#include <sys/time.h>

#include "debug.h"
#include "dhcpd.h"
#include "arpping.h"
#include "socket.h"
#include "options.h"
#include "files.h"
#include "leases.h"
#include "packet.h"
#include "serverpacket.h"
#include "pidfile.h"
#include "nkutil.h"
#include "nkdef.h"

#ifndef SOCK_RAW
#define SOCK_RAW       3
#endif


/* globals */
struct dhcpOfferedAddr *leases;
struct server_config_t server_config;


/* Exit and cleanup */
static void exit_server(int retval)
{
#ifdef NK_CONFIG_SUPPORT_MULTI_DHCP_SUBNET
	//LOG(LOG_INFO, "udhcp server rg_dhcp_pool_list_free");
	rg_dhcp_pool_list_free(&server_config.dhcp_pools);
#endif

	pidfile_delete(server_config.pidfile);
	CLOSE_LOG();
	exit(retval);
}


/* SIGTERM handler */
static void udhcpd_killed(int sig)
{
	sig = 0;
	LOG(LOG_INFO, "Received SIGTERM");
	exit_server(0);
}

/* add this lease to syscoonfig --> Kide 2005/03/28 */
void nk_add_lease_to_sysconf(struct dhcpMessage *packet, struct dhcpOfferedAddr *lease)
{
	char	buf[20], usr[72], cmd[CMDBUF_SIZE], *hostname=NULL;
	struct in_addr	ip;
#ifdef CONFIG_NK_DHCP_CLIENT_USER
	// this entry has already existed in sysconfig
	if (lease->is_static)
		return;
#endif
	/* update sysconfig database */
	if ( (hostname = get_option(packet, DHCP_HOST_NAME)) )
	{
		int	bytes = hostname[-1];
		if (bytes >= (int) sizeof(lease->hostname))
			bytes = sizeof(lease->hostname) - 1;
		strncpy(lease->hostname, hostname, bytes);
		lease->hostname[bytes] = '\0';
		sprintf(usr, "%s", lease->hostname);
	}
	else
	{
		strncpy(lease->hostname, lease->chaddr, 16);
		lease->hostname[16] = '\0';
		sprintf(usr, "%02X%02X%02X%02X%02X%02X", lease->chaddr[0], lease->chaddr[1], lease->chaddr[2],
												 lease->chaddr[3], lease->chaddr[4], lease->chaddr[5]);
	}
#ifdef CONFIG_NK_DHCP_CLIENT_USER
	// add to user list
	sprintf(cmd, "HOST_LIST ID=USER_%s", usr);
	kd_doCommand(cmd, CMD_WRITE, ASH_DO_NOTHING, (char *)NULL);

	// user name
	sprintf(cmd, "USER_%s NAME=%s", usr, usr);
	kd_doCommand(cmd, CMD_NEW, ASH_DO_NOTHING, (char *)NULL);

	// ip
	ip.s_addr = lease->yiaddr;
	sprintf(cmd, "USER_%s IP=%s", usr, inet_ntoa(ip));
	kd_doCommand(cmd, CMD_NEW, ASH_DO_NOTHING, (char *)NULL);

	// mac
	sprintf(cmd, "USER_%s MAC=%02X:%02X:%02X:%02X:%02X:%02X", usr, lease->chaddr[0], lease->chaddr[1],
																   lease->chaddr[2], lease->chaddr[3],
																   lease->chaddr[4], lease->chaddr[5]);
	kd_doCommand(cmd, CMD_NEW, ASH_DO_NOTHING, (char *)NULL);
	
	// min rate
	sprintf(cmd,"USER_%s MINRATE=0", usr);
	kd_doCommand(cmd, CMD_NEW, ASH_DO_NOTHING, (char *)NULL);

	// max rate
	kd_doCommand("SYSTEM DOWNSTREAMBW", CMD_PRINT, ASH_DO_NOTHING, (char *)buf);
	sprintf(cmd,"USER_%s MAXRATE=%s", usr, buf);
	kd_doCommand(cmd, CMD_NEW, ASH_DO_NOTHING, (char *)NULL);
	
	// current rate
	sprintf(cmd,"USER_%s CURRATE=0", usr);
	kd_doCommand(cmd, CMD_NEW, ASH_DO_NOTHING, (char *)NULL);

	// application NUMBER
	sprintf(cmd,"USER_%s NUMBER=0", usr);
	kd_doCommand(cmd, CMD_NEW, ASH_DO_NOTHING, (char *)NULL);

	// record with static flag
	lease->is_static = 1;

	// update to flash
	kd_updateFlash(SYSTEM_CHANGE_DB);

	// inform webBoot that new host added
	sprintf(cmd, "USER_%s", usr);
	kd_doCommand(cmd, NULL, ASH_DO_DHCP_NEW_HOST, (char *)NULL);
#endif
	return;
} /* nk_add_lease_to_sysconf() */


/* handle SIGUSR2 signal --> Ryoko 2005/07/04 */
static int signal_pipe[2];
static void signal_handler(int sig)
{
	if (send(signal_pipe[1], &sig, sizeof(sig), MSG_DONTWAIT) < 0)
		LOG(LOG_INFO, "Could not send signal: %s", strerror(errno));
}
// <-- Kide
#ifdef COMBINED_BINARY	
int udhcpd(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{	
	fd_set rfds;
	struct timeval tv;
//	int server_socket;
	int server_socket = -1;  // edit by johnli
	int bytes, retval;
	struct dhcpMessage packet;
	unsigned char *state;
	char *server_id, *requested;
	u_int32_t server_id_align, requested_align;
	unsigned long timeout_end;
	struct option_set *option;
	struct dhcpOfferedAddr *lease;
	struct sockaddr_in *U_sin;
	int pid_fd;
	int max_sock;
	struct rg_dhcp_pool_list_t *dhcp_pool = NULL;
			
	/* server ip addr */
	int fd = -1;
	struct ifreq ifr;

	argc = argv[0][0]; /* get rid of some warnings */
	
	OPEN_LOG("udhcpd");
	LOG(LOG_INFO, "udhcp server (v%s) started", VERSION);
	
	//pid_fd = pidfile_acquire(server_config.pidfile);
	pid_fd = pidfile_acquire("/var/run/udhcpd.pid");
	pidfile_write_release(pid_fd);

	memset(&server_config, 0, sizeof(struct server_config_t));

	read_config(DHCPD_CONF_FILE);
	if ((option = find_option(server_config.options, DHCP_LEASE_TIME))) {
		memcpy(&server_config.lease, option->data + 2, 4);
		server_config.lease = ntohl(server_config.lease);
	}
	else server_config.lease = LEASE_TIME;

#ifdef NK_CONFIG_SUPPORT_MULTI_DHCP_SUBNET
	int idx;
	for (idx=1; idx<=4/*CONFIG_NK_NUM_DHCP_SUBNET*/; idx++)
	{
		char filename[32];
		sprintf(filename, "%s-%d", DHCPD_CONF_FILE, idx);
		if (access(filename, 0) == 0)
			read_dhcp_subnet_config(filename,  &server_config.dhcp_pools);
	}
#endif

	leases = malloc(sizeof(struct dhcpOfferedAddr) * server_config.max_leases);
	memset(leases, 0, sizeof(struct dhcpOfferedAddr) * server_config.max_leases);
#ifdef CONFIG_NK_DHCP_CLIENT_USER
	read_leases("/tmp/dhcp_static.leases");	// static leases file --> Kide 2005/03/28
#else
	read_leases(server_config.lease_file);	// DYN. leases file --> Ryoko 2005/09/13
#endif


	if((fd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) >= 0) {
		ifr.ifr_addr.sa_family = AF_INET;
		strcpy(ifr.ifr_name, server_config.interface);
		if (ioctl(fd, SIOCGIFADDR, &ifr) == 0) {
			U_sin = (struct sockaddr_in *) &ifr.ifr_addr;
			server_config.server = U_sin->sin_addr.s_addr;
			DEBUG(LOG_INFO, "%s (server_ip) = %s", ifr.ifr_name, inet_ntoa(U_sin->sin_addr));
		} else {
			LOG(LOG_INFO, "SIOCGIFADDR failed!");
			exit_server(1);
		}
		if (ioctl(fd, SIOCGIFINDEX, &ifr) == 0) {
			DEBUG(LOG_INFO, "adapter index %d", ifr.ifr_ifindex);
			server_config.ifindex = ifr.ifr_ifindex;
		} else {
			LOG(LOG_INFO, "SIOCGIFINDEX failed!");
			exit_server(1);
		}
		if (ioctl(fd, SIOCGIFHWADDR, &ifr) == 0) {
			memcpy(server_config.arp, ifr.ifr_hwaddr.sa_data, 6);
			DEBUG(LOG_INFO, "adapter hardware address %02x:%02x:%02x:%02x:%02x:%02x",
				server_config.arp[0], server_config.arp[1], server_config.arp[2], 
				server_config.arp[3], server_config.arp[4], server_config.arp[5]);
		} else {
			LOG(LOG_INFO, "SIOCGIFHWADDR failed!");
			exit_server(1);
		}
	} else {
		LOG(LOG_INFO, "socket failed!");
		exit_server(1);
	}
	close(fd);

#ifndef DEBUGGING
	pid_fd = pidfile_acquire(server_config.pidfile); /* hold lock during fork. */
	switch(fork()) {
	case -1:
		perror("fork");
		exit_server(1);
		/*NOTREACHED*/
	case 0:
		break;	/* child continues */
	default:
		exit(0);	/* parent exits */
		/*NOTREACHED*/
		}
	close(0);
	setsid();
	pidfile_write_release(pid_fd);
#endif


	signal(SIGUSR1, write_leases);
	signal(SIGTERM, udhcpd_killed);

	/* open socket pair to handle SIGUSR2 --> Ryoko 2005/07/04 */
	socketpair(AF_UNIX, SOCK_STREAM, 0, signal_pipe);
	signal(SIGUSR2, signal_handler);
	// <--

	timeout_end = get_time(0) + server_config.auto_time;
	while(1) { /* loop until universe collapses */
 // remove by johnli
//		server_socket = listen_socket(INADDR_ANY, SERVER_PORT, server_config.interface);
//		if(server_socket == -1) {
// end johnli
		if(server_socket < 0) {
			// edit by johnli
			if ((server_socket = listen_socket(INADDR_ANY, SERVER_PORT, server_config.interface)) < 0) {
				LOG(LOG_INFO, "couldn't create server socket -- au revoir");
				exit_server(0);
			}
			// end johnli
		}			

		FD_ZERO(&rfds);
		FD_SET(server_socket, &rfds);
		FD_SET(signal_pipe[0], &rfds);
		if (server_config.auto_time) {
			tv.tv_sec = timeout_end - get_time(0);
			if (tv.tv_sec <= 0) {
				tv.tv_sec = server_config.auto_time;
				timeout_end = get_time(0) + server_config.auto_time;
				write_leases(0);
			}
			tv.tv_usec = 0;
		}
		max_sock = server_socket > signal_pipe[0] ? server_socket : signal_pipe[0];
		retval = select(max_sock + 1, &rfds, NULL, NULL, server_config.auto_time ? &tv : NULL);
		if (retval == 0) {
			write_leases(0);
			timeout_end = get_time(0) + server_config.auto_time;
//			close(server_socket);  // remove by johnli
			continue;
		} else if (retval < 0) {
			DEBUG(LOG_INFO, "error on select");
//			close(server_socket);  // remove by johnli
			continue;
		}
		/* handle SIGUSR2 from signal_pipe --> Ryoko 2005/07/04 */
		if ( FD_ISSET(signal_pipe[0], &rfds) )
		{
			int sig;
			if (read(signal_pipe[0], &sig, sizeof(sig)) < 0)
				continue;

			if (sig == SIGUSR2)
				delete_leases(sig);
			continue;
		}
		// <-- Ryoko 2005/07/04
		
		bytes = get_packet(&packet, server_socket); /* this waits for a packet - idle */
//		close(server_socket);  // remove by johnli
		if(bytes < 0) {
			// add by johnli
			close(server_socket);
			server_socket = -1;
			// end johnli
			continue;
		}

		if((state = get_option(&packet, DHCP_MESSAGE_TYPE)) == NULL) {
			DEBUG(LOG_INFO, "couldnt get option from packet -- ignoring");
			continue;
		}
		
		lease = find_lease_by_chaddr(packet.chaddr);
		dhcp_pool = dhcp_pools_select(&packet, 1);
		switch (state[0]) {
		case DHCPDISCOVER:
			DEBUG(LOG_INFO,"received DISCOVER");
			
			if (sendOffer(&packet) < 0) {
				LOG(LOG_INFO, "send OFFER failed -- ignoring");
			}
			break;			
 		case DHCPREQUEST:
			DEBUG(LOG_INFO,"received REQUEST");

			requested = get_option(&packet, DHCP_REQUESTED_IP);
			server_id = get_option(&packet, DHCP_SERVER_ID);

			if (requested) memcpy(&requested_align, requested, 4);
			if (server_id) memcpy(&server_id_align, server_id, 4);
		
			if (lease) {
				if ( dhcp_pool &&
				     (lease->yiaddr & ntohl(dhcp_pool->netmask.s_addr)) 
				  != (ntohl(dhcp_pool->server_ip.s_addr) & ntohl(dhcp_pool->netmask.s_addr)) )
					sendNAK(&packet);

				if (server_id) {
					/* SELECTING State */
					DEBUG(LOG_INFO, "server_id = %08x", ntohl(server_id_align));
					if (server_id_align == server_config.server && requested && 
					    requested_align == lease->yiaddr) {
						sendACK(&packet, lease->yiaddr);
						// add to sysconf to be static --> Ryoko 2005/07/04
						//nk_add_lease_to_sysconf(&packet, lease);
						// <--
					}
				} else {
					if (requested) {
						/* INIT-REBOOT State */
						if (lease->yiaddr == requested_align)
						{
							sendACK(&packet, lease->yiaddr);
							// add to sysconf to be static --> Ryoko 2005/07/04
							//nk_add_lease_to_sysconf(&packet, lease);
							// <--
						}
						else sendNAK(&packet);
					} else {
						/* RENEWING or REBINDING State */
						if (lease->yiaddr == packet.ciaddr)
						{
							sendACK(&packet, lease->yiaddr);
							// add to sysconf to be static --> Ryoko 2005/07/04
							//nk_add_lease_to_sysconf(&packet, lease);
							// <--
						}
						else {
							sendACK(&packet, lease->yiaddr);
							// add to sysconf to be static --> Ryoko 2005/07/04
							//nk_add_lease_to_sysconf(&packet, lease);
							// <--
							/* don't know what to do!!!! */
							//sendNAK(&packet);
							LOG(LOG_WARNING, "RENEWING IP YIADDR != CIDDAR");
						}
					}						
				}
			} /* else remain silent */
			/* what to do if we have no record of the client */
			 else if (server_id) {
				/* SELECTING State */
				sendNAK(&packet);       
			} else if (requested) {
				if ( dhcp_pool &&
				     (requested_align & ntohl(dhcp_pool->netmask.s_addr)) 
				  != (ntohl(dhcp_pool->server_ip.s_addr) & ntohl(dhcp_pool->netmask.s_addr)) )
					sendNAK(&packet);

				/* INIT-REBOOT State */
				if ((lease = find_lease_by_yiaddr(requested_align))) {
					if (lease_expired(lease)) {
						/* probably best if we drop this lease */
						memset(lease->chaddr, 0, 16);
					/* make some contention for this address */
					} else sendNAK(&packet);
#ifdef NK_CONFIG_SUPPORT_MULTI_DHCP_SUBNET
				} else if (!is_ip_in_lease_range_and_not_server_ip(requested_align)) {
#else
				} else if (requested_align < server_config.start || 
					   requested_align > server_config.end) {
#endif
					sendNAK(&packet);
				} else {
					sendNAK(&packet);
				}

			} else if (packet.ciaddr) {
				/* RENEWING or REBINDING State */
				sendNAK(&packet);
			}
			// <--				
			break;
		case DHCPDECLINE:
			DEBUG(LOG_INFO,"received DECLINE");
			//ignored other PC set static IP.
			//window send arp detect other PC(set static ip)
			// if the same IP,send DECLINE
			//we ignored DECLINE if leases is is_static
			if (lease) {
				if(leases->is_static != 1)
				{
					memset(lease->chaddr, 0, 16);
					lease->expires = get_time(0) + server_config.decline_time;
				}
			}			
			break;
		case DHCPRELEASE:
			DEBUG(LOG_INFO,"received RELEASE");
			if (lease) lease->expires = get_time(0);
			break;
		case DHCPINFORM:
			DEBUG(LOG_INFO,"received INFORM");
			send_inform(&packet);
			break;	
		default:
			LOG(LOG_WARNING, "unsupported DHCP message (%02x) -- ignoring", state[0]);
		}
	}

#ifdef NK_CONFIG_SUPPORT_MULTI_DHCP_SUBNET
	//LOG(LOG_INFO, "udhcp server rg_dhcp_pool_list_free");
	rg_dhcp_pool_list_free(&server_config.dhcp_pools);
#endif

	return 0;
}

