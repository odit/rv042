/* serverpacket.c
 *
 * Constuct and send DHCP server packets
 *
 * Russ Dill <Russ.Dill@asu.edu> July 2001
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

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>

#include "packet.h"
#include "debug.h"
#include "dhcpd.h"
#include "options.h"
#include "leases.h"

/* Ryoko 2005/007/04: include this header file for syscall */
//#include "nkutil.h"
//#include "nkdef.h"

#if 0
static int nk_is_wan_conneciton_up(void)
{
	char	buf[16],cmdBuf[50];
	int	i;

	// ISP WAN
	memset(buf, 0, sizeof(buf));
	for(i=1;i<=CONFIG_NK_NUM_WAN;i++)
	{
		sprintf(cmdBuf,"ISP%d WAN",i);
		kd_doCommand(cmdBuf, CMD_PRINT, ASH_DO_NOTHING, (char *)buf);
		if ( strcmp(buf, "0.0.0.0") )
			return 1;
	}

		return 0;
} /* nk_is_wan_conneciton_up() */
// <--
#endif

#ifdef NK_CONFIG_SUPPORT_MULTI_DHCP_SUBNET
static int is_usable_ip_in_dhcp_pool(struct rg_dhcp_pool_list_t* dhcp_pool)
{
	u_int32_t	last_ip;

	for (last_ip = dhcp_pool->start_ip.s_addr; last_ip <=dhcp_pool->end_ip.s_addr; last_ip++)
	{	
//LOG(LOG_INFO, "is_usable_ip_in_dhcp_pool last_ip=%x", last_ip);
		if (last_ip == dhcp_pool->server_ip.s_addr)
			continue;
		if (find_lease_by_yiaddr(last_ip) == NULL)
			return 1;
	}

	return 0;
}

struct rg_dhcp_pool_list_t* dhcp_pools_select(struct dhcpMessage* packet, unsigned int use_match)
{
	struct rg_dhcp_pool_list_t *dhcp_pools = NULL;
	struct rg_mac_match_list_t *mac_list = NULL;
	struct rg_vlan_match_list_t *vlan_list = NULL;
	unsigned int match_criteria;

	dhcp_pools = server_config.dhcp_pools;
	while (dhcp_pools)
	{
		if (!is_usable_ip_in_dhcp_pool(dhcp_pools))
			goto NEXT_ONE;
		
		if (!use_match) return dhcp_pools;
		
		match_criteria = dhcp_pools->match_criteria;
		if (match_criteria == DHCP_MAC_MATCH) {
			mac_list = dhcp_pools->mac_list;
			while (mac_list) {
				if (!memcmp(packet->chaddr,mac_list->mac,6)) {
					return dhcp_pools;
				}
				mac_list = mac_list->next;
			}
/* not support yet, packet info need vlanid */
#if 0
		} else if (match_criteria == DHCP_VLAN_MATCH) {
			vlan_list = dhcp_pools->vlan_list;
			while (vlan_list) {
				if (packet->vlanid == vlan_list->vlan_id) {
					return dhcp_pools;
				}
				vlan_list = vlan_list->next;
			}
#endif
		}
NEXT_ONE:
		dhcp_pools = dhcp_pools->next;
	}

	return NULL;
}
#endif

/* send a packet to giaddr using the kernel ip stack */
static int send_packet_to_relay(struct dhcpMessage *payload)
{
	DEBUG(LOG_INFO, "Forwarding packet to relay");

	return kernel_packet(payload, server_config.server, SERVER_PORT,
			payload->giaddr, SERVER_PORT);
}


/* send a packet to a specific arp address and ip address by creating our own ip packet */
static int send_packet_to_client(struct dhcpMessage *payload, int force_broadcast)
{
	unsigned char *chaddr;
	u_int32_t ciaddr;
	
	if (force_broadcast) {
		DEBUG(LOG_INFO, "broadcasting packet to client (NAK)");
		ciaddr = INADDR_BROADCAST;
		chaddr = MAC_BCAST_ADDR;
	} else if (payload->ciaddr) {
		DEBUG(LOG_INFO, "unicasting packet to client ciaddr");
		ciaddr = payload->ciaddr;
		chaddr = payload->chaddr;
	} else if (ntohs(payload->flags) & BROADCAST_FLAG) {
		DEBUG(LOG_INFO, "broadcasting packet to client (requested)");
		ciaddr = INADDR_BROADCAST;
		chaddr = MAC_BCAST_ADDR;
	} else {
		DEBUG(LOG_INFO, "unicasting packet to client yiaddr");
		ciaddr = payload->yiaddr;
		chaddr = payload->chaddr;
	}
	return raw_packet(payload, server_config.server, SERVER_PORT, 
			ciaddr, CLIENT_PORT, chaddr, server_config.ifindex);
}


/* send a dhcp packet, if force broadcast is set, the packet will be broadcast to the client */
static int send_packet(struct dhcpMessage *payload, int force_broadcast)
{
	int ret;

	if (payload->giaddr)
		ret = send_packet_to_relay(payload);
	else ret = send_packet_to_client(payload, force_broadcast);
	return ret;
}


static void init_packet(struct dhcpMessage *packet, struct dhcpMessage *oldpacket, char type)
{
	memset(packet, 0, sizeof(struct dhcpMessage));
	
	packet->op = BOOTREPLY;
	packet->htype = ETH_10MB;
	packet->hlen = ETH_10MB_LEN;
	packet->xid = oldpacket->xid;
	memcpy(packet->chaddr, oldpacket->chaddr, 16);
	packet->cookie = htonl(DHCP_MAGIC);
	packet->options[0] = DHCP_END;
	packet->flags = oldpacket->flags;
	packet->giaddr = oldpacket->giaddr;
	packet->ciaddr = oldpacket->ciaddr;
	add_simple_option(packet->options, DHCP_MESSAGE_TYPE, type);
//	add_simple_option(packet->options, DHCP_SERVER_ID, ntohl(server_config.server)); /* expects host order */
	add_simple_option(packet->options, DHCP_SERVER_ID, server_config.server);
}


/* add in the bootp options */
static void add_bootp_options(struct dhcpMessage *packet)
{
	packet->siaddr = server_config.siaddr;
	if (server_config.sname)
		strncpy(packet->sname, server_config.sname, sizeof(packet->sname) - 1);
	if (server_config.boot_file)
		strncpy(packet->file, server_config.boot_file, sizeof(packet->file) - 1);
}
	

/* send a DHCP OFFER to a DHCP DISCOVER */
int sendOffer(struct dhcpMessage *oldpacket)
{
	struct dhcpMessage packet;
	struct dhcpOfferedAddr *lease = NULL;
	u_int32_t req_align, lease_time_align = server_config.lease;
	char *req, *lease_time;
	struct option_set *curr;
	struct in_addr addr;
#ifdef NK_CONFIG_SUPPORT_MULTI_DHCP_SUBNET
	struct rg_dhcp_pool_list_t *selected_dhcp_pool = NULL;
#endif
	


/* 2007/07/30 jane: bug fix dhcp status do not show client's host name */
#if 1
	char *hostname, usr[40];
	memset(usr, '\0', sizeof(usr));
	if ( (hostname = get_option(oldpacket, DHCP_HOST_NAME)) )
	{
		int bytes = hostname[-1];
		if (bytes >= (int) sizeof(usr))
			bytes = sizeof(usr) - 1;
		strncpy(usr, hostname, bytes);
	}
/*	else
		sprintf(usr, "%02X%02X%02X%02X%02X%02X", oldpacket->chaddr[0], oldpacket->chaddr[1], oldpacket->chaddr[2],
							 oldpacket->chaddr[3], oldpacket->chaddr[4], oldpacket->chaddr[5]);
*/
#endif

	init_packet(&packet, oldpacket, DHCPOFFER);
	
#if 0
	/* the client is in our lease/offered table */
	if ((lease = find_lease_by_chaddr(oldpacket->chaddr))) {
 		if(!check_ip(lease->yiaddr)){	//2005/09/28 Ryoko  Confirm the ip isn't used by someone
			if (!lease_expired(lease)) 
				lease_time_align = lease->expires - get_time(0);
			packet.yiaddr = lease->yiaddr;	
		}
		else
			goto find_ip;
#else
	selected_dhcp_pool = dhcp_pools_select(&packet, 1);
	/* the client is in our lease/offered table */
	if ((lease = find_lease_by_chaddr(oldpacket->chaddr))) {
	    if (lease->is_static || (!lease->is_static && selected_dhcp_pool==NULL)) {
		if (!check_ip(lease->yiaddr)) {	//2005/09/28 Ryoko  Confirm the ip isn't used by someone
			if (!lease_expired(lease)) 
				lease_time_align = lease->expires - get_time(0);
			packet.yiaddr = lease->yiaddr;	
		}
		else
			goto find_ip;
	    } else goto find_ip;
	} else if (selected_dhcp_pool != NULL) {
		goto find_ip;
#endif
	/* Or the client has a requested ip */
	} else if ((req = get_option(oldpacket, DHCP_REQUESTED_IP)) &&

		   /* Don't look here (ugly hackish thing to do) */
		   memcpy(&req_align, req, 4) &&

#ifdef NK_CONFIG_SUPPORT_MULTI_DHCP_SUBNET
		   /* and the ip is in the lease range and not server ip*/
		   is_ip_in_lease_range_and_not_server_ip(req_align) &&
#else
		   /* and the ip is in the lease range */
		   ntohl(req_align) >= ntohl(server_config.start) &&
		   ntohl(req_align) <= ntohl(server_config.end) &&
#endif		   
		   /* and its not already taken/offered */
		   ((!(lease = find_lease_by_yiaddr(req_align)) ||
		   
		   /* or its taken, but expired */
		   lease_expired(lease)))) {
			if(req_align == server_config.server)   //2005/09/28 Ryoko The request ip cann't be router ip
				goto find_ip;
			else if(!check_ip(req_align))		// 2005/09/28 Ryoko Confirm the ip isn't used by someone
				packet.yiaddr = req_align; /* FIXME: oh my, is there a host using this IP? */
			else
				goto find_ip;

	/* otherwise, find a free IP */
	} else {
find_ip:
#ifdef NK_CONFIG_SUPPORT_MULTI_DHCP_SUBNET
//		selected_dhcp_pool = dhcp_pools_select(&packet, 1);

/*		if (selected_dhcp_pool)
			LOG(LOG_INFO, "selected_dhcp_pool [%x-%x] %x", selected_dhcp_pool->start_ip.s_addr, selected_dhcp_pool->end_ip.s_addr, selected_dhcp_pool->server_ip.s_addr);
		else
			LOG(LOG_INFO, "selected_dhcp_pool==NULL find again");
*/
		if (!selected_dhcp_pool)
			selected_dhcp_pool = dhcp_pools_select(&packet, 0);
				
		if (selected_dhcp_pool)
			packet.yiaddr = find_address(0, selected_dhcp_pool);
		
/*		if (packet.yiaddr)
			LOG(LOG_INFO, "find_address packet.yiaddr=%x", packet.yiaddr);
*/

		/* try for an expired lease */
		if (!packet.yiaddr)
		{
			selected_dhcp_pool = server_config.dhcp_pools;
			while (selected_dhcp_pool) {
				packet.yiaddr = find_address(1, selected_dhcp_pool);
				if (packet.yiaddr) 
					break;
				selected_dhcp_pool = selected_dhcp_pool->next;
			}
		}
#else
		packet.yiaddr = find_address(0);
		
		/* try for an expired lease */
		if (!packet.yiaddr) packet.yiaddr = find_address(1);
#endif
	}
	
	if(!packet.yiaddr) {
		LOG(LOG_WARNING, "no IP addresses to give -- OFFER abandoned");
		return -1;
	}

	
/* 2007/07/30 jane: bug fix dhcp status do not show client's host name */
#if 1
	if (!add_lease(packet.chaddr, packet.yiaddr, server_config.offer_time, usr)) {
#else
	if (!add_lease(packet.chaddr, packet.yiaddr, server_config.offer_time)) {
#endif
		LOG(LOG_WARNING, "lease pool is full -- OFFER abandoned");
		return -1;
	}		

	if ((lease_time = get_option(oldpacket, DHCP_LEASE_TIME))) {
		memcpy(&lease_time_align, lease_time, 4);
		lease_time_align = ntohl(lease_time_align);
		if (lease_time_align > server_config.lease) 
			lease_time_align = server_config.lease;
	}

	/* Make sure we aren't just using the lease time from the previous offer */
	if (lease_time_align < server_config.min_lease) 
		lease_time_align = server_config.lease;
		
//2005/09/08 Ryoko set lease time to 5min when wan connection is down(by webBoot)
#if 0	
	/* Kide 2005/04/15: set lease time to 5min when wan connection is down */
	if ( !nk_is_wan_conneciton_up() )
		lease_time_align = server_config.min_lease * 5;
	// <--
#endif

	add_simple_option(packet.options, DHCP_LEASE_TIME, lease_time_align);

	curr = server_config.options;
	while (curr) {
		if (curr->data[OPT_CODE] != DHCP_LEASE_TIME)
			add_option_string(packet.options, curr->data);
		curr = curr->next;
	}

	add_bootp_options(&packet);
	
	addr.s_addr = packet.yiaddr;
	LOG(LOG_INFO, "sending OFFER of %s", inet_ntoa(addr));
	return send_packet(&packet, 0);
}


int sendNAK(struct dhcpMessage *oldpacket)
{
	struct dhcpMessage packet;

	init_packet(&packet, oldpacket, DHCPNAK);
	
	DEBUG(LOG_INFO, "sending NAK");
	return send_packet(&packet, 1);
}

int randtime = 0;

int sendACK(struct dhcpMessage *oldpacket, u_int32_t yiaddr)
{
	struct dhcpMessage packet;
	struct option_set *curr;
	char *lease_time;
	u_int32_t lease_time_align = server_config.lease;
	struct in_addr addr;
	
	char router_option[7];
	struct option_set *router;
	unsigned int divid = 1;
	char *ssp = 0;

/* 2007/07/30 jane: bug fix dhcp status do not show client's host name */
#if 1
	char *hostname, usr[40];
	memset(usr, '\0', sizeof(usr));
	if ( (hostname = get_option(oldpacket, DHCP_HOST_NAME)) )
	{
		int bytes = hostname[-1];
		if (bytes >= (int) sizeof(usr))
			bytes = sizeof(usr) - 1;
		strncpy(usr, hostname, bytes);
	}
/*	else
		sprintf(usr, "%02X%02X%02X%02X%02X%02X", oldpacket->chaddr[0], oldpacket->chaddr[1], oldpacket->chaddr[2],
							 oldpacket->chaddr[3], oldpacket->chaddr[4], oldpacket->chaddr[5]);
*/
#endif

	init_packet(&packet, oldpacket, DHCPACK);
	packet.yiaddr = yiaddr;
	
	if ((lease_time = get_option(oldpacket, DHCP_LEASE_TIME))) {
		memcpy(&lease_time_align, lease_time, 4);
		lease_time_align = ntohl(lease_time_align);
		if (lease_time_align > server_config.lease) 
			lease_time_align = server_config.lease;
		else if (lease_time_align < server_config.min_lease) 
			lease_time_align = server_config.lease;
	}
	
#if 0
	/* Ryoko 2005/07/04: set lease time to 5min when wan connection is down */
	if ( !nk_is_wan_conneciton_up() )
		lease_time_align = server_config.min_lease * 5;
	// <--
#endif

	add_simple_option(packet.options, DHCP_LEASE_TIME, lease_time_align);


	//2008/12/22 ternchen : support HA  --->
	memset(router_option, '\0', sizeof(router_option));
	router = find_option(server_config.options,DHCP_ROUTER);
	if( router ){
		randtime++;
		divid = router->data[OPT_LEN] / 4 ;
		randtime = randtime%divid;
		router_option[0] = 0x03;
		router_option[1] = 0x04;
		memcpy(router_option+2,router->data+2+randtime*4,4);
	}
	//<------
	
	curr = server_config.options;
	while (curr) {
		if( curr->data[OPT_CODE] == DHCP_ROUTER ){
			add_option_string(packet.options,router_option);
		}else
		if (curr->data[OPT_CODE] != DHCP_LEASE_TIME)
			add_option_string(packet.options, curr->data);
		curr = curr->next;
	}

	add_bootp_options(&packet);

	addr.s_addr = packet.yiaddr;

	/* matrix: using bootp's flags to decide the ack pkt as broadcast or unicast */ //<--matrix 2006/06/14
	//if(get_option(oldpacket, DHCP_SERVER_ID)){
	if (ntohs(oldpacket->flags) & BROADCAST_FLAG) { //-->2006/06/14
		LOG(LOG_INFO, "sending ACK to %s", "255.255.255.255");
		if (send_packet(&packet, 1) < 0) return -1;
	}
	else{

		LOG(LOG_INFO, "sending ACK to %s", inet_ntoa(addr));
		if (send_packet(&packet, 0) < 0) return -1;
	}

/* 2007/07/30 jane: bug fix dhcp status do not show client's host name */
#if 1
	add_lease(packet.chaddr, packet.yiaddr, lease_time_align, usr);
#else
	add_lease(packet.chaddr, packet.yiaddr, lease_time_align);
#endif

	return 0;
}


int send_inform(struct dhcpMessage *oldpacket)
{
	struct dhcpMessage packet;
	struct option_set *curr;

	init_packet(&packet, oldpacket, DHCPACK);
	
	curr = server_config.options;
	while (curr) {
		if (curr->data[OPT_CODE] != DHCP_LEASE_TIME)
			add_option_string(packet.options, curr->data);
		curr = curr->next;
	}

	add_bootp_options(&packet);

	return send_packet(&packet, 0);
}



