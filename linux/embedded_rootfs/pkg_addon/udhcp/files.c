/* 
 * files.c -- DHCP server file manipulation *
 * Rewrite by Russ Dill <Russ.Dill@asu.edu> July 2001
 */
 
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <netdb.h>

#include "debug.h"
#include "dhcpd.h"
#include "files.h"
#include "options.h"
#include "leases.h"
#include "nkutil.h"
#include "nkdef.h"

/* on these functions, make sure you datatype matches */
static int read_ip(char *line, void *arg)
{
	struct in_addr *addr = arg;
	struct hostent *host;
	int retval = 1;

	if (!inet_aton(line, addr)) {
		if ((host = gethostbyname(line))) 
			addr->s_addr = *((unsigned long *) host->h_addr_list[0]);
		else retval = 0;
	}
	return retval;

}


static int read_str(char *line, void *arg)
{
	char **dest = arg;
	int i;
	
	if (*dest) free(*dest);
	*dest = strdup(line);
	
	/* elimate trailing whitespace */
	for (i = strlen(*dest) - 1; i > 0 && isspace((*dest)[i]); i--);
	(*dest)[i > 0 ? i + 1 : 0] = '\0';
	return 1;
}


static int read_u32(char *line, void *arg)
{
	u_int32_t *dest = arg;
	*dest = strtoul(line, NULL, 0);
	return 1;
}


static int read_yn(char *line, void *arg)
{
	char *dest = arg;
	if (!strcasecmp("yes", line) || !strcmp("1", line) || !strcasecmp("true", line))
		*dest = 1;
	else if (!strcasecmp("no", line) || !strcmp("0", line) || !strcasecmp("false", line))
		*dest = 0;
	else return 0;
	
	return 1;
}


/* read a dhcp option and add it to opt_list */
static int read_opt(char *line, void *arg)
{
	struct option_set **opt_list = arg;
	char *opt, *val;
	char fail;
	struct dhcp_option *option = NULL;
	int length = 0;
	char buffer[255];
	u_int16_t result_u16;
	int16_t result_s16;
	u_int32_t result_u32;
	int32_t result_s32;
	
	int i;
	
	if (!(opt = strtok(line, " \t="))) return 0;
	
	for (i = 0; options[i].code; i++)
		if (!strcmp(options[i].name, opt)) {
			option = &(options[i]);
			break;
		}
		
	if (!option) return 0;
	
	do {
		val = strtok(NULL, ", \t");
		if (val) {
			fail = 0;
			length = 0;
			switch (option->flags & TYPE_MASK) {
			case OPTION_IP:
				read_ip(val, buffer);
				break;
			case OPTION_IP_PAIR:
				read_ip(val, buffer);
				if ((val = strtok(NULL, ", \t/-")))
					read_ip(val, buffer + 4);
				else fail = 1;
				break;
			case OPTION_STRING:
				length = strlen(val);
				if (length > 254) length = 254;
				memcpy(buffer, val, length);
				break;
			case OPTION_BOOLEAN:
				if (!read_yn(val, buffer)) fail = 1;
				break;
			case OPTION_U8:
				buffer[0] = strtoul(val, NULL, 0);
				break;
			case OPTION_U16:
				result_u16 = htons(strtoul(val, NULL, 0));
				memcpy(buffer, &result_u16, 2);
				break;
			case OPTION_S16:
				result_s16 = htons(strtol(val, NULL, 0));
				memcpy(buffer, &result_s16, 2);
				break;
			case OPTION_U32:
				result_u32 = htonl(strtoul(val, NULL, 0));
				memcpy(buffer, &result_u32, 4);
				break;
			case OPTION_S32:
				result_s32 = htonl(strtol(val, NULL, 0));	
				memcpy(buffer, &result_s32, 4);
				break;
			default:
				break;
			}
			length += option_lengths[option->flags & TYPE_MASK];
			if (!fail)
				attach_option(opt_list, option, buffer, length);
		} else fail = 1;
	} while (!fail && option->flags & OPTION_LIST);
	return 1;
}


static struct config_keyword keywords[] = {
	/* keyword	handler   variable address		default */
	{"start",	read_ip,  &(server_config.start),	"192.168.0.20"},
	{"end",		read_ip,  &(server_config.end),		"192.168.0.254"},
	{"interface",	read_str, &(server_config.interface),	"eth0"},
	{"option",	read_opt, &(server_config.options),	""},
	{"opt",		read_opt, &(server_config.options),	""},
#ifdef NK_CONFIG_SUPPORT_MULTI_DHCP_SUBNET
	{"max_leases",	read_u32, &(server_config.max_leases),	"1020"},
#else
	{"max_leases",	read_u32, &(server_config.max_leases),	"254"},
#endif
	{"remaining",	read_yn,  &(server_config.remaining),	"yes"},
	{"auto_time",	read_u32, &(server_config.auto_time),	"7200"},
	{"decline_time",read_u32, &(server_config.decline_time),"3600"},
	{"conflict_time",read_u32,&(server_config.conflict_time),"300"},
	{"offer_time",	read_u32, &(server_config.offer_time),	"60"},
	{"min_lease",	read_u32, &(server_config.min_lease),	"60"},
	{"lease_file",	read_str, &(server_config.lease_file),	"/etc/udhcpd.leases"},
	{"pidfile",	read_str, &(server_config.pidfile),	"/var/run/udhcpd.pid"},
	{"notify_file", read_str, &(server_config.notify_file),	""},
	{"siaddr",	read_ip,  &(server_config.siaddr),	"0.0.0.0"},
	{"sname",	read_str, &(server_config.sname),	""},
	{"boot_file",	read_str, &(server_config.boot_file),	""},
	{"",		NULL, 	  NULL,				""}
};


int read_config(char *file)
{
	FILE *in;
	char buffer[80], *token, *line;
	int i;

	for (i = 0; strlen(keywords[i].keyword); i++)
		if (strlen(keywords[i].def))
			keywords[i].handler(keywords[i].def, keywords[i].var);

	if (!(in = fopen(file, "r"))) {
		LOG(LOG_INFO, "unable to open config file: %s", file);
		return 0;
	}
	
	while (fgets(buffer, 80, in)) {
		if (strchr(buffer, '\n')) *(strchr(buffer, '\n')) = '\0';
		if (strchr(buffer, '#')) *(strchr(buffer, '#')) = '\0';
		token = buffer + strspn(buffer, " \t");
		if (*token == '\0') continue;
		line = token + strcspn(token, " \t=");
		if (*line == '\0') continue;
		*line = '\0';
		line++;
		line = line + strspn(line, " \t=");
		if (*line == '\0') continue;
		
		
		
		for (i = 0; strlen(keywords[i].keyword); i++)
			if (!strcasecmp(token, keywords[i].keyword))
				keywords[i].handler(line, keywords[i].var);
	}
	fclose(in);
	return 1;
}


/* the dummy var is here so this can be a signal handler */
void write_leases(int dummy)
{
	FILE *fp;
	unsigned int i;
	char buf[255];
	u_int32_t lease_time;
	time_t curr = get_time(0);
	
	dummy = 0;
	
	if (!(fp = fopen(server_config.lease_file, "w"))) {
		LOG(LOG_INFO, "Unable to open %s for writing", server_config.lease_file);
		return;
	}
	
	for (i = 0; i < server_config.max_leases; i++) {
		if (leases[i].yiaddr != 0) {
			if (server_config.remaining) {
				if (lease_expired(&(leases[i])))
					lease_time = 0;
				else lease_time = leases[i].expires - curr;
			} else lease_time = leases[i].expires;
			lease_time = htonl(lease_time);
			fwrite(leases[i].chaddr, 16, 1, fp);
			fwrite(&(leases[i].yiaddr), 4, 1, fp);
			fwrite(&lease_time, 4, 1, fp);
			fwrite(leases[i].hostname, 64, 1, fp);	// Ryoko 2005/07/04
			fwrite(&(leases[i].is_static), 4, 1, fp);	// Kide 2005/03/30
		}
	}
	fclose(fp);
	
	if (server_config.notify_file) {
		sprintf(buf, "%s %s", server_config.notify_file, server_config.lease_file);
		system(buf);
	}
}


void read_leases(char *file)
{
	FILE *fp;
	unsigned int i = 0,StartIP[4],EndIP[4],tempIP[4],MaskIP[4];
	time_t curr = get_time(0);
	struct dhcpOfferedAddr lease, *oldest;
	struct in_addr temp;
	char LanIP[30],SubMask[30];
	
	
	if (!(fp = fopen(file, "r"))) {
		LOG(LOG_INFO, "Unable to open %s for reading", file);
		return;
	}

#ifndef NK_CONFIG_SUPPORT_MULTI_DHCP_SUBNET
	kd_doCommand("SYSTEM LAN", CMD_PRINT, ASH_DO_NOTHING, LanIP);
	kd_doCommand("SYSTEM LANMASK", CMD_PRINT, ASH_DO_NOTHING, SubMask);
	sscanf(LanIP,"%d.%d.%d.%d",&StartIP[0],&StartIP[1],&StartIP[2],&StartIP[3]);
	sscanf(SubMask,"%d.%d.%d.%d",&MaskIP[0],&MaskIP[1],&MaskIP[2],&MaskIP[3]);
#endif

	while (i < server_config.max_leases && (fread(&lease, sizeof lease, 1, fp) == 1)) {
#ifdef NK_CONFIG_SUPPORT_MULTI_DHCP_SUBNET
		if (lease.is_static || is_ip_in_lease_range_and_not_server_ip(lease.yiaddr)) {
#else
		/* ADDME: is it a static lease */
		temp.s_addr = lease.yiaddr;
		sscanf(inet_ntoa(temp),"%u.%u.%u.%u",&EndIP[0],&EndIP[1],&EndIP[2],&EndIP[3]);

		if ( (StartIP[0] & MaskIP[0])==(EndIP[0] & MaskIP[0]) &&
			(StartIP[1] & MaskIP[1])==(EndIP[1] & MaskIP[1]) &&
			(StartIP[2] & MaskIP[2])==(EndIP[2] & MaskIP[2]) &&
			(StartIP[3] & MaskIP[3])==(EndIP[3] & MaskIP[3]) ) {
			if ((ntohl(lease.yiaddr) >= ntohl(server_config.start) && ntohl(lease.yiaddr) <= ntohl(server_config.end))||lease.is_static) {
#endif
				lease.expires = ntohl(lease.expires);
				/* mark this line --> Kide 2005/03/30 */
				//if (server_config.remaining) lease.expires += curr;
				// <--
/* 2007/07/30 jane: bug fix dhcp status do not show client's host name */
#if 1
				if (!(oldest = add_lease(lease.chaddr, lease.yiaddr, lease.expires, lease.hostname))) {
#else
				if (!(oldest = add_lease(lease.chaddr, lease.yiaddr, lease.expires))) {
#endif
					LOG(LOG_WARNING, "Too many leases while loading %s\n", file);
					break;
				}

				//--> 2005/09/23 Ryoko DHCP start read file,need read hostname
				strncpy(oldest->hostname, lease.hostname, sizeof(oldest->hostname) - 1);
				oldest->hostname[sizeof(oldest->hostname) - 1] = '\0';
				oldest->is_static = lease.is_static ;
				//<--
				i++;
#ifndef NK_CONFIG_SUPPORT_MULTI_DHCP_SUBNET
			}
#endif
		}
	}
	DEBUG(LOG_INFO, "Read %d leases", i);
	fclose(fp);
}

		
	
/* delete leases from memory --> Kide 2005/04/12 */
void delete_leases(int dummy)
{
	FILE	*fp;
	char	line[100], hostname[64], ip[16];
	int		i, mac[6];
	struct in_addr in;

	dummy = 0;
	LOG(LOG_DEBUG,"Receive SIGUSR2 : delete_leases()");

	/* read "/tmp/.user_delete" which generated from web-cgi */
	if ( (fp = fopen("/tmp/.user_delete", "r")) == NULL )
	{
		LOG(LOG_INFO, "Unable to open /tmp/.user_delete for reading");
		return;
	}

	while ( fgets(line, sizeof(line), fp) != NULL )
	{
		LOG(LOG_DEBUG, "/tmp/.delete_leases : %s", line);
		sscanf(line, "%s %s %02X:%02X:%02X:%02X:%02X:%02X",
						hostname, ip, &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
		inet_aton(ip, &in);
		break;
	}
	fclose(fp);

	// search deleted entry
	for (i=0; i < server_config.max_leases; i++)
	{
		if (leases[i].yiaddr != 0)
		{
			if (leases[i].yiaddr == (u_int32_t)in.s_addr)
			{
				// clear this entry
				LOG(LOG_DEBUG,"match & delete [%s %x]", leases[i].hostname, htonl(leases[i].yiaddr));
				memset(&leases[i], 0, sizeof(struct dhcpOfferedAddr));
				break;
			}
		}
	} // for loop

	if ( i == (int)server_config.max_leases )
		LOG(LOG_DEBUG,"not match [%s %s]", hostname, ip);
	return;
} /* delete_leases() */
// <-- Kide 2005/04/12
		
			
#ifdef NK_CONFIG_SUPPORT_MULTI_DHCP_SUBNET
ether_mac_t *atomac(char *mac_str)
{
    static ether_mac_t mac;
    int mac_tmp[6];
    int i;

    memset(&mac,0,sizeof(mac));
    if (!mac_str)
    {
	LOG(LOG_WARNING, "invalid MAC address");
	return NULL;
    }
//    if (sscanf(mac_str, "%02X:%02X:%02X:%02X:%02X:%02X", &mac_tmp[0],
    if (sscanf(mac_str, "%02X%02X%02X%02X%02X%02X", &mac_tmp[0],
	&mac_tmp[1], &mac_tmp[2], &mac_tmp[3], &mac_tmp[4], &mac_tmp[5]) != 6)
    {
	LOG(LOG_WARNING, "error translating a string to MAC address: %s", mac_str);
	return NULL;
    }
 
    for (i=0; i<6; i++)
	mac[i] = (u_int8_t)mac_tmp[i];
    return &mac;
}


int rg_mac_list_add(rg_mac_match_list_t **phead, unsigned char *mac)
{
	rg_mac_match_list_t *pmac;
	rg_mac_match_list_t *nmac = (rg_mac_match_list_t *)malloc(sizeof(rg_mac_match_list_t));
	if (!nmac)
		return -1;
	memset(nmac,0,sizeof(rg_mac_match_list_t));
	memcpy(&nmac->mac, mac, 6);

	if (*phead == NULL)
		*phead = nmac;
	else {
		for (pmac=*phead; pmac->next != NULL; pmac=pmac->next);
		pmac->next = nmac;
	}

	return 0;
}

int rg_vlan_list_add(rg_vlan_match_list_t **phead, unsigned int vlan_id)
{
	rg_vlan_match_list_t *vlan;
	rg_vlan_match_list_t *nvlan = (rg_vlan_match_list_t *)malloc(sizeof(rg_vlan_match_list_t));
	if (!nvlan)
		return -1;
	memset(nvlan,0,sizeof(rg_vlan_match_list_t));
	nvlan->vlan_id = vlan_id;

	if (*phead == NULL)
		*phead = nvlan;
	else {
		for (vlan=*phead; vlan->next != NULL; vlan=vlan->next);
		vlan->next = nvlan;
	}

	return 0;
}

int rg_dhcp_pool_list_add(rg_dhcp_pool_list_t **phead, struct in_addr server_ip , struct in_addr start_ip , struct in_addr end_ip , struct in_addr netmask,void *match_list, match_criteria_t criteria)
{
	rg_dhcp_pool_list_t *dhcp_pool;
	rg_dhcp_pool_list_t *ndhcp_pool = (rg_dhcp_pool_list_t *)malloc(sizeof(rg_dhcp_pool_list_t));

	if (!ndhcp_pool)
		return -1;
	memset(ndhcp_pool,0,sizeof(rg_dhcp_pool_list_t));
	memcpy(&ndhcp_pool->server_ip, &server_ip, sizeof(struct in_addr));
	memcpy(&ndhcp_pool->start_ip, &start_ip, sizeof(struct in_addr));
	memcpy(&ndhcp_pool->end_ip, &end_ip, sizeof(struct in_addr));
	memcpy(&ndhcp_pool->netmask, &netmask, sizeof(struct in_addr));
	ndhcp_pool->match_criteria = criteria;
	switch (criteria)
	{
		case DHCP_VLAN_MATCH:
			ndhcp_pool->vlan_list = (rg_vlan_match_list_t *)match_list;
			break;
		case DHCP_MAC_MATCH:
			ndhcp_pool->mac_list = (rg_mac_match_list_t *)match_list;
			break;
	}

	if (*phead == NULL)
		*phead = ndhcp_pool;
	else {
		for (dhcp_pool=*phead; dhcp_pool->next != NULL; dhcp_pool=dhcp_pool->next);
		dhcp_pool->next = ndhcp_pool;
	}

	return 0;
}

void rg_vlan_list_free(rg_vlan_match_list_t **head)
{
	rg_vlan_match_list_t *vlan_list;
	while (*head)
	{
		vlan_list = *head;
		*head = vlan_list->next;
		free(vlan_list);
    }
}

void rg_mac_list_free(rg_mac_match_list_t **head)
{
	rg_mac_match_list_t *mac_list;
	while (*head)
	{
		mac_list = *head;
		*head = mac_list->next;
		free(mac_list);
    }
}

void rg_dhcp_pool_list_free(rg_dhcp_pool_list_t **head)
{
	rg_dhcp_pool_list_t *dhcp_pool;
	rg_vlan_match_list_t *vlan_list;
	rg_mac_match_list_t *mac_list;

	while (*head)
	{
		dhcp_pool = *head;
		vlan_list = dhcp_pool->vlan_list;
		mac_list = dhcp_pool->mac_list;
		if (vlan_list)
			rg_vlan_list_free(&vlan_list);
		if (mac_list)
			rg_mac_list_free(&mac_list);
		*head = dhcp_pool->next;
		free(dhcp_pool);
	}
}

int read_dhcp_subnet_config(char *file, rg_dhcp_pool_list_t **l)
{
	FILE *in;
	char buffer[81], *token, *line;
	int num;
	rg_mac_match_list_t *mac_list = NULL;
	rg_vlan_match_list_t *vlan_list = NULL;
	struct in_addr start_ip, end_ip, netmask, server_ip;
	match_criteria_t match_criteria;
	unsigned char mac_address[6];

	if (!(in = fopen(file, "r"))) {
		LOG(LOG_INFO, "unable to open config file: %s", file);
		return 0;
	}
	
//LOG(LOG_INFO, "START read_dhcp_subnet_config");

	while (fgets(buffer, 80, in)) {
		if (strchr(buffer, '\n')) *(strchr(buffer, '\n')) = '\0';
		if (strchr(buffer, '#')) *(strchr(buffer, '#')) = '\0';
		token = buffer + strspn(buffer, " \t");
		if (*token == '\0') continue;
		line = token + strcspn(token, " \t=");
		if (*line == '\0') continue;
		*line = '\0';
		line++;
		line = line + strspn(line, " \t=");
		if (*line == '\0') continue;
		
//LOG(LOG_INFO, "token=%s, line=%s", token, line);

		if (!strcasecmp(token, "serverip"))
			inet_aton(line, &server_ip);
		else if (!strcasecmp(token, "mask"))
			inet_aton(line, &netmask);
		else if (!strcasecmp(token, "start"))
			inet_aton(line, &start_ip);
		else if (!strcasecmp(token, "end"))
			inet_aton(line, &end_ip);
		else if (!strcasecmp(token, "match_criteria"))
			match_criteria = atoi(line);
		else if (!strcasecmp(token, "mac"))
			rg_mac_list_add(&mac_list, atomac(line));
		else if (!strcasecmp(token, "vlan"))
			rg_vlan_list_add(&vlan_list, atoi(line));
	}	
		
	if (match_criteria == DHCP_VLAN_MATCH)
		rg_dhcp_pool_list_add(l,server_ip,start_ip,end_ip,netmask,vlan_list,match_criteria);
	else if (match_criteria == DHCP_MAC_MATCH)
		rg_dhcp_pool_list_add(l,server_ip,start_ip,end_ip,netmask,mac_list,match_criteria);

	fclose(in);
	return 1;
}
#endif

