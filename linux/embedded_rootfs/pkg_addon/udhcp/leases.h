/* leases.h */
#ifndef _LEASES_H
#define _LEASES_H


#ifdef NK_CONFIG_SUPPORT_MULTI_DHCP_SUBNET
typedef u_int8_t ether_mac_t[6];

typedef enum {
    DHCP_MAC_MATCH = 0,
    DHCP_VLAN_MATCH = 1,
} match_criteria_t;

typedef struct rg_mac_match_list_t {
    struct rg_mac_match_list_t *next;
    unsigned char mac[6];
} rg_mac_match_list_t;

typedef struct rg_vlan_match_list_t {
    struct rg_vlan_match_list_t *next;
    unsigned char vlan_id;
} rg_vlan_match_list_t;

typedef struct rg_dhcp_pool_list_t {
	struct rg_dhcp_pool_list_t *next;
	struct rg_mac_match_list_t *mac_list;
	struct rg_vlan_match_list_t *vlan_list;
	unsigned int match_criteria;
	struct in_addr server_ip;
	struct in_addr start_ip;
	struct in_addr end_ip;
	struct in_addr netmask;
} rg_dhcp_pool_list_t;
#endif /* NK_CONFIG_SUPPORT_MULTI_DHCP_SUBNET */

struct dhcpOfferedAddr {
	u_int8_t chaddr[16];
	u_int32_t yiaddr;	/* network order */
	u_int32_t expires;	/* host order */
	char		hostname[64];	/* client hostname --> Ryoko 2005/07/04 */
	u_int32_t	is_static;		/* static lease --> Ryoko 2005/07/04*/
};


void clear_lease(u_int8_t *chaddr, u_int32_t yiaddr);
/* 2007/07/30 jane: bug fix dhcp status do not show client's host name */
#if 1
struct dhcpOfferedAddr *add_lease(u_int8_t *chaddr, u_int32_t yiaddr, unsigned long lease, char *hostname);
#else
struct dhcpOfferedAddr *add_lease(u_int8_t *chaddr, u_int32_t yiaddr, unsigned long lease);
#endif
int lease_expired(struct dhcpOfferedAddr *lease);
struct dhcpOfferedAddr *oldest_expired_lease(void);
struct dhcpOfferedAddr *find_lease_by_chaddr(u_int8_t *chaddr);
struct dhcpOfferedAddr *find_lease_by_yiaddr(u_int32_t yiaddr);
#ifdef NK_CONFIG_SUPPORT_MULTI_DHCP_SUBNET
u_int32_t find_address(int check_expired, struct rg_dhcp_pool_list_t *selected_dhcp_pool); 
int is_ip_in_lease_range_and_not_server_ip(u_int32_t addr);
#else
u_int32_t find_address(int check_expired);
#endif
int check_ip(u_int32_t addr);


#endif
