#include <net/if.h>
#include <nkuserlandconf.h>

//oolong
#include <sys/types.h>
#include <netinet/in.h>

#define NK_IPV4 0
#define NK_IPV6 1
#define NK_IPV6_LAN 0
#define NK_IPV6_WAN 1

#define SCOPE_LINK 0
#define SCOPE_GLOBAL 1
#define SCOPE_COMPAT 2

#define WAN_CONN_INIT 0
#define WAN_CONN_DOWN_DHCPCLIENT_STOP 1
#define WAN_DHCPCLIENT_RELEASE 2
#define WAN_DHCPCLIENT_START 3

struct NK_IPV6_DATA {
	struct in6_addr ipv6_address;
	char mac[6];
	int prefix_len;
	int scope_type;
};
