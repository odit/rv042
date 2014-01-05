/* files.h */
#ifndef _FILES_H
#define _FILES_H

struct config_keyword {
	char keyword[20];
	int (*handler)(char *line, void *var);
	void *var;
	char def[40];
};


int read_config(char *file);
void write_leases(int dummy);
void delete_leases(int dummy);		// Ryoko 2005/07/04
void read_leases(char *file);

#ifdef NK_CONFIG_SUPPORT_MULTI_DHCP_SUBNET
int read_dhcp_subnet_config(char *file, rg_dhcp_pool_list_t **l);
int rg_dhcp_pool_list_add(rg_dhcp_pool_list_t **, struct in_addr , struct in_addr , struct in_addr , struct in_addr ,void *, match_criteria_t );
int rg_vlan_list_add(rg_vlan_match_list_t **, unsigned int );
int rg_mac_list_add(rg_mac_match_list_t **, unsigned char *);
void rg_dhcp_pool_list_free(rg_dhcp_pool_list_t **);
void rg_vlan_list_free(rg_vlan_match_list_t **);
void rg_mac_list_free(rg_mac_match_list_t **);
struct rg_dhcp_pool_list_t *rg_dhcp_pool_list_dup(struct rg_dhcp_pool_list_t *);
struct rg_vlan_match_list_t *rg_vlan_list_dup(struct rg_vlan_match_list_t *);
struct rg_mac_match_list_t *rg_mac_list_dup(struct rg_mac_match_list_t *);
#endif

#endif
