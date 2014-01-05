/* 
 * leases.c -- tools to manage DHCP leases 
 * Russ Dill <Russ.Dill@asu.edu> July 2001
 */

#include <time.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "debug.h"
#include "dhcpd.h"
#include "files.h"
#include "options.h"
#include "leases.h"
#include "arpping.h"


/* clear every lease out that chaddr OR yiaddr matches and is nonzero */
/*
void clear_lease(u_int8_t *chaddr, u_int32_t yiaddr)
{
	unsigned int i, blank_chaddr = 0, blank_yiaddr = 0;
	
	for (i = 0; i < 16 && !chaddr[i]; i++);
	if (i == 16) blank_chaddr = 1;
	blank_yiaddr = (yiaddr == 0);
	
	for (i = 0; i < server_config.max_leases; i++)
		if ((!blank_chaddr && !memcmp(leases[i].chaddr, chaddr, 16)) ||
		    (!blank_yiaddr && leases[i].yiaddr == yiaddr)) {
			memset(&(leases[i]), 0, sizeof(struct dhcpOfferedAddr));
		}
}
*/
void clear_lease(u_int8_t *chaddr, u_int32_t yiaddr)
{
	unsigned int i, j;
	
	for (j = 0; j < 16 && !chaddr[j]; j++);
	
	for (i = 0; i < server_config.max_leases; i++)
		if ((j != 16 && !memcmp(leases[i].chaddr, chaddr, 16)) ||
		    (yiaddr && leases[i].yiaddr == yiaddr)) {
			memset(&(leases[i]), 0, sizeof(struct dhcpOfferedAddr));
		}
}

/* 2007/07/30 jane: bug fix dhcp status do not show client's host name */
#if 1
/* add a lease into the table, clearing out any old ones */
struct dhcpOfferedAddr *add_lease(u_int8_t *chaddr, u_int32_t yiaddr, unsigned long lease, char *hostname)
#else
/* add a lease into the table, clearing out any old ones */
struct dhcpOfferedAddr *add_lease(u_int8_t *chaddr, u_int32_t yiaddr, unsigned long lease)
#endif
{
	struct dhcpOfferedAddr *oldest;
#if 1
	/* statitc lease --> Kide 2005/03/20 */
	if ( (oldest = find_lease_by_yiaddr(yiaddr)) && oldest->is_static)
		goto STATIC_LEASE;

	// <--
#endif
	/* clean out any old ones */
	clear_lease(chaddr, yiaddr);

	oldest = oldest_expired_lease();

	if (oldest) {
#ifdef CONFIG_NK_DHCP_CLIENT_USER
		/* if lease is zero, mean it's static --> Kide 2005/03/20 */
		oldest->is_static = (lease == 0) ? 1 : 0;
		lease = (lease == 0) ? server_config.lease : lease;
#endif
STATIC_LEASE:
		// <--

		if(!(oldest->is_static))
		{
			memcpy(oldest->chaddr, chaddr, 16);
			oldest->yiaddr = yiaddr;
			oldest->expires = get_time(0) + lease;
		}
		else
		{
			oldest->expires = get_time(0) + lease;
		}
/* 2007/07/30 jane: bug fix dhcp status do not show client's host name */
#if 1
      		if (hostname)
		{
			memset(oldest->hostname, '\0', sizeof(oldest->hostname));
			strcpy(oldest->hostname, hostname);
		}
#endif
	}

	return oldest;
}



/* true if a lease has expired */
int lease_expired(struct dhcpOfferedAddr *lease)
{
#ifdef CONFIG_NK_DHCP_CLIENT_USER
	/* static lease never expired --> Kide 2005/03/30 */
	if (lease->is_static)
		lease->expires = server_config.lease + get_time(0);
	//<--
#endif
	return (lease->expires < (u_int32_t) get_time(0));
}	


/* Find the oldest expired lease, NULL if there are no expired leases */
struct dhcpOfferedAddr *oldest_expired_lease(void)
{
	struct dhcpOfferedAddr *oldest = NULL;
	unsigned long oldest_lease = get_time(0);
	unsigned int i;

	
	for (i = 0; i < server_config.max_leases; i++)
	{
#if 1
		/* skip static lease --> Kide 2005/03/30 */
		if (leases[i].is_static)
			continue;
		// <--
#endif
		if (oldest_lease > leases[i].expires) {
			oldest_lease = leases[i].expires;
			oldest = &(leases[i]);
		}
	}
	return oldest;
		
}


/* Find the first lease that matches chaddr, NULL if no match */
struct dhcpOfferedAddr *find_lease_by_chaddr(u_int8_t *chaddr)
{
	unsigned int i;

	for (i = 0; i < server_config.max_leases; i++)
		if (!memcmp(leases[i].chaddr, chaddr, 16))		
			 return &(leases[i]);
	return NULL;
}


/* Find the first lease that matches yiaddr, NULL is no match */
struct dhcpOfferedAddr *find_lease_by_yiaddr(u_int32_t yiaddr)
{
	unsigned int i;

	for (i = 0; i < server_config.max_leases; i++)
		if (leases[i].yiaddr == yiaddr) return &(leases[i]);
	
	return NULL;
}


/* find an assignable address, it check_expired is true, we check all the expired leases as well.
 * Maybe this should try expired leases by age... */
#ifdef NK_CONFIG_SUPPORT_MULTI_DHCP_SUBNET
u_int32_t find_address(int check_expired, struct rg_dhcp_pool_list_t *selected_dhcp_pool) 
#else
u_int32_t find_address(int check_expired) 
#endif
{
	u_int32_t addr, ret = 0;
	struct dhcpOfferedAddr *lease = NULL;		

#ifdef NK_CONFIG_SUPPORT_MULTI_DHCP_SUBNET
	addr = ntohl(selected_dhcp_pool->start_ip.s_addr);
	for (;ntohl(addr) <= ntohl(selected_dhcp_pool->end_ip.s_addr) ;addr = htonl(ntohl(addr) + 1)) {

		if (addr == ntohl(selected_dhcp_pool->server_ip.s_addr)) continue;

		if (ret == selected_dhcp_pool->server_ip.s_addr) continue;
#else
	addr = ntohl(server_config.start);
	for (;ntohl(addr) <= ntohl(server_config.end) ;addr = htonl(ntohl(addr) + 1)) {

		if (addr == ntohl(server_config.server)) continue;

		if (ret == server_config.server) continue;
#endif
		/* ie, 192.168.55.0 */
		if (!(ntohl(addr) & 0xFF)) continue;

		/* ie, 192.168.55.255 */
		if ((ntohl(addr) & 0xFF) == 0xFF) continue;

		/* lease is not taken */
		if ((!(lease = find_lease_by_yiaddr(addr)) ||

		     /* or it expired and we are checking for expired leases */
		     (check_expired  && lease_expired(lease))) &&

		     /* and it isn't on the network */
	    	     !check_ip(addr)) {
			ret = addr;
			break;
		}
	}
	return ret;
}


/* check is an IP is taken, if it is, add it to the lease table */
int check_ip(u_int32_t addr)
{
	char blank_chaddr[] = {[0 ... 15] = 0};
	struct in_addr temp;
	int i;//fix if lease is static IP,don't check arp
	for (i = 0; i < server_config.max_leases; i++) {
		if (leases[i].yiaddr == addr && leases[i].is_static == 1) {
			return 0;
		}
	}	
	if (arpping(addr, server_config.server, server_config.arp, server_config.interface) == 0) {
		temp.s_addr = addr;
	 	LOG(LOG_INFO, "%s belongs to someone, reserving it for %ld seconds", 
	 		inet_ntoa(temp), server_config.conflict_time);
/* 2007/07/30 jane: bug fix dhcp status do not show client's host name */
#if 1
		add_lease(blank_chaddr, addr, server_config.conflict_time, NULL);
#else
		add_lease(blank_chaddr, addr, server_config.conflict_time);
#endif
		return 1;
	} else return 0;
}

#ifdef NK_CONFIG_SUPPORT_MULTI_DHCP_SUBNET
int is_ip_in_lease_range_and_not_server_ip(u_int32_t addr)
{
	struct rg_dhcp_pool_list_t *dhcp_pool = NULL;
	int ret = 0;

	dhcp_pool = server_config.dhcp_pools;
//LOG(LOG_INFO, "addr=%x", addr); 
	while (dhcp_pool) {
//LOG(LOG_INFO, "start=%x, end=%x, server=%x", dhcp_pool->start_ip.s_addr, dhcp_pool->end_ip.s_addr, dhcp_pool->server_ip.s_addr); 

		if (ntohl(addr) >= ntohl(dhcp_pool->start_ip.s_addr) &&
		    ntohl(addr) <= ntohl(dhcp_pool->end_ip.s_addr) &&
		    addr != dhcp_pool->server_ip.s_addr)
			return 1;
	
		dhcp_pool = dhcp_pool->next; 
	}

	return 0;
}
#endif
