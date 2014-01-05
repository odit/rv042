#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <syslog.h>
#include <signal.h>
#include <errno.h>
#include <getopt.h>
#include <time.h>

#define REMAINING 0
#define ABSOLUTE 1

struct lease_t {
	unsigned char chaddr[16];
	u_int32_t yiaddr;
	u_int32_t expires;
	char		hostname[64];	/* client hostname --> Ryoko 2005/07/04 */
	u_int32_t	is_static;		/* static lease --> Ryoko 2005/07/04*/
};

int main (int argc, char *argv[]) {
	FILE *fp;
	int i, c, mode = REMAINING;
	u_int32_t expires;
	char file[255] = "/etc/udhcpd.leases";
	struct lease_t lease;
	struct in_addr addr;
	
	static struct option options[] = {
		{"absolute", 0, 0, 'a'},
		{"remaining", 0, 0, 'r'},
		{"file", 1, 0, 'f'},
		{"help", 0, 0, 'h'},
		{0, 0, 0, 0}
	};

	while (1) {
		int option_index = 0;
		c = getopt_long(argc, argv, "arf:h", options, &option_index);
		if (c == -1) break;
		
		switch (c) {
		case 'a': mode = ABSOLUTE; break;
		case 'r': mode = REMAINING; break;
		case 'f':
			strncpy(file, optarg, 255);
			file[254] = '\0';
			break;
		case 'h':
			printf("Usage: dumpleases -f <file> -[ra]\n\n");
			printf("  -f, --file=FILENAME             Leases file to load\n");
			printf("  -r, --remaining                 Interepret lease times as time reemaing\n");
			printf("  -a, --absolute                  Interepret lease times as expire time\n");
			break;
		}
	}
			
	if (!(fp = fopen(file, "r"))) {
		perror("could not open input file");
		return 0;
	}

	printf("Mac Address       IP-Address      static Expires %s 		   Hostname\n", mode == REMAINING ? "in" : "at");  
	/*     "00:00:00:00:00:00 255.255.255.255 0	 Wed Jun 30 21:49:08 1993  localhost" */
	while (fread(&lease, sizeof(lease), 1, fp)) {

		for (i = 0; i < 6; i++) {
			printf("%02x", lease.chaddr[i]);
			if (i != 5) printf(":");
		}
		addr.s_addr = lease.yiaddr;
		printf(" %-15s", inet_ntoa(addr));
		printf(" %-6d", lease.is_static);
		expires = ntohl(lease.expires);
		printf(" ");
		if (mode == REMAINING) {
//			if (!expires) printf("expired\n");
			if (!expires) printf("%-30s", "expired");
			else {
				if (expires > 60*60*24) {
					printf("%ld day, ", expires / (60*60*24));
					expires %= 60*60*24;
				}
				if (expires > 60*60) {
					printf("%2ld hr, ", expires / (60*60));
					expires %= 60*60;
				}
				if (expires > 60) {
					printf("%2ld min, ", expires / 60);
					expires %= 60;
				}
//				printf("%ld sec\n", expires);
				printf("%2ld sec", expires);
			}
		} else printf("%s", ctime(&expires));

                if (lease.hostname)
		printf(" %-64s\n", lease.hostname);
		else
		printf(" NULL");
	}
	fclose(fp);
	
	return 0;
}
