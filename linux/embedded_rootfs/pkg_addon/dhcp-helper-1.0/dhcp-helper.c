/* dhcp-helper is Copyright (c) 2004,2008 Simon Kelley

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 dated June, 1991, or
   (at your option) version 3 dated 29 June, 2007.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/* Author's email: simon@thekelleys.org.uk */

#define VERSION "1.0"

#define COPYRIGHT "Copyright (C) 2004-2008 Simon Kelley" 

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <limits.h>
#include <net/if.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <pwd.h>
#include <grp.h>
#include <netdb.h>
#include <linux/types.h>
#include <linux/capability.h>
/* There doesn't seem to be a universally-available 
   userpace header for this. */
extern int capset(cap_user_header_t header, cap_user_data_t data);
extern int capget(cap_user_header_t header, cap_user_data_t data);
#define LINUX_CAPABILITY_VERSION_1  0x19980330
#define LINUX_CAPABILITY_VERSION_2  0x20071026
#define LINUX_CAPABILITY_VERSION_3  0x20080522

#include <sys/prctl.h>
#include <net/if_arp.h>
#include <nkutil.h>

#define PIDFILE "/var/run/dhcp-helper.pid"
#define USER "nobody"

#define DHCP_CHADDR_MAX  16
#define DHCP_SERVER_PORT 67
#define DHCP_CLIENT_PORT 68
#define DHCP_SERVER_ALTPORT 1067
#define DHCP_CLIENT_ALTPORT 1068
#define BOOTREQUEST      1
#define BOOTREPLY        2

#ifndef NIPQUAD
	#define NIPQUAD(addr) \
		((unsigned char *)&addr)[0], \
		((unsigned char *)&addr)[1], \
		((unsigned char *)&addr)[2], \
		((unsigned char *)&addr)[3]
#endif

#define paul_printf(args...) \
{ \
FILE *fp; \
fp = fopen("/dev/console","w"); \
if(fp==NULL) return; \
fprintf(fp, args); \
fflush(fp); \
fclose(fp); \
}

struct namelist {
  char name[IF_NAMESIZE];
  struct in_addr addr;
  struct namelist *next;
};

struct interface {
  int index;
  struct in_addr addr;
  struct interface *next;
};

struct dhcp_packet_with_opts{
  struct dhcp_packet {
    unsigned char op, htype, hlen, hops;
    unsigned int xid;
    unsigned short secs, flags;
    struct in_addr ciaddr, yiaddr, siaddr, giaddr;
    unsigned char chaddr[DHCP_CHADDR_MAX], sname[64], file[128];
  } header;
  unsigned char options[312];
};


int main(int argc, char **argv)
{
	int fd = -1, oneopt = 1, mtuopt = IP_PMTUDISC_DONT;
	struct ifreq ifr;
	struct sockaddr_in saddr;
	size_t buf_size = sizeof(struct dhcp_packet_with_opts);
	struct dhcp_packet *packet;
	struct namelist *interfaces = NULL, *except = NULL;
	struct interface *ifaces = NULL;
	struct namelist *servers = NULL;
	char *runfile = PIDFILE;
	char *user = USER;
	int debug = 0, altports = 0;
	int outfd = -1, activefd = -1;
	int ret, maxfd;
	fd_set allset, readset;
	char buf[20];

	while (1)
	{
		int option = getopt(argc, argv, "b:e:i:s:u:r:dvp");

		if (option == -1)
			break;

		switch (option)
		{
			case 's': case 'b': case 'i': case 'e':
			{
				struct namelist *new = malloc(sizeof(struct namelist));

				if (!new)
				{
					fprintf(stderr, "dhcp-helper: cannot get memory\n");
					exit(1);
				}
				strncpy(new->name, optarg, IF_NAMESIZE);
				strncpy(ifr.ifr_name, optarg, IF_NAMESIZE);
				new->addr.s_addr = 0;

				if (option == 's')
				{
					struct hostent *e = gethostbyname(optarg);

					if (!e)
					{
						fprintf(stderr, "dhcp-helper: cannot resolve server name %s\n", optarg);
						exit(1);
					}
					new->addr = *((struct in_addr *)e->h_addr);
				}
				else if (strlen(optarg) > IF_NAMESIZE)
				{
					fprintf(stderr, "dhcp-helper: interface name too long: %s\n", optarg);
					exit(1);
				}
				else if ((fd == -1 && (fd = socket(PF_INET, SOCK_DGRAM, 0)) == -1) || ioctl (fd, SIOCGIFFLAGS, &ifr) == -1)
				{
					fprintf(stderr, "dhcp-helper: bad interface %s: %s\n", optarg, strerror(errno));
					exit (1);
				}
				else if (option == 'b' && !(ifr.ifr_flags & IFF_BROADCAST))
				{
					fprintf(stderr, "dhcp-helper: interface %s cannot broadcast\n", optarg);
					exit(1);
				}

				if (option == 'i')
				{
					new->next = interfaces;
					interfaces = new;
				}
				else if (option == 'e')
				{
					new->next = except;
					except = new;
				}
				else
				{
					new->next = servers;
					servers = new;
				}
			}
			break;

			case 'u':
				if ((user = malloc(strlen(optarg) + 1)))
					strcpy(user, optarg);
				break;

			case 'r':
				if ((runfile = malloc(strlen(optarg) + 1)))
					strcpy(runfile, optarg);
				break;

			case 'd':
				debug = 1;
				break;

			case 'p':
				altports = 1;
				break;

			case 'v':
				fprintf(stderr, "dhcp-helper version %s, %s\n", VERSION, COPYRIGHT);
				exit(0);

			default:
				fprintf(stderr,
					"Usage: dhcp-helper [OPTIONS]\n"
					"Options are:\n"
					"-s <server>      Forward DHCP requests to <server>\n"
					"-b <interface>   Forward DHCP requests as broadcasts via <interface>\n"
					"-i <interface>   Listen for DHCP requests on <interface>\n"
					"-e <interface>   Do not listen for DHCP requests on <interface>\n"
					"-u <user>        Change to user <user> (defaults to %s)\n"
					"-r <file>        Write daemon PID to this file (default %s)\n"
					"-p               Use alternative ports (1067/1068)\n"
					"-d               Debug mode\n"
					"-v               Give version and copyright info and then exit\n",
					USER, PIDFILE);
				exit(1);
		}
	}

	if (!servers)
	{
		fprintf(stderr, "dhcp-helper: no destination specifed; give at least -s or -b option.\n");
		exit(1);
	}

	if (!(packet = malloc(buf_size)))
	{
		perror("dhcp-helper: cannot allocate buffer");
		exit(1);
	}

	if (fd == -1 && (fd = socket(PF_INET, SOCK_DGRAM, 0)) == -1)
	{
		perror("dhcp-helper: cannot create socket");
		exit(1);
	}

	if (outfd == -1 && (outfd = socket(PF_INET, SOCK_DGRAM, 0)) == -1)
	{
		perror("dhcp-helper: cannot create socket");
		exit(1);
	}

	if (setsockopt(fd, SOL_IP, IP_PKTINFO, &oneopt, sizeof(oneopt)) == -1 ||
	    setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &oneopt, sizeof(oneopt)) == -1 ||
	    setsockopt(fd, SOL_IP, IP_MTU_DISCOVER, &mtuopt, sizeof(mtuopt)) == -1 ||
	    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &oneopt, sizeof(oneopt)) == -1)
	{
		perror("dhcp-helper: cannot set options on DHCP socket");
		exit(1);
	}

	if (setsockopt(outfd, SOL_IP, IP_PKTINFO, &oneopt, sizeof(oneopt)) == -1 ||
	    setsockopt(outfd, SOL_SOCKET, SO_BROADCAST, &oneopt, sizeof(oneopt)) == -1 ||
	    setsockopt(outfd, SOL_IP, IP_MTU_DISCOVER, &mtuopt, sizeof(mtuopt)) == -1 ||
	    setsockopt(outfd, SOL_SOCKET, SO_REUSEADDR, &oneopt, sizeof(oneopt)) == -1)
	{
		perror("dhcp-helper: cannot set options on DHCP socket");
		exit(1);
	}

	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(altports ? DHCP_SERVER_ALTPORT : DHCP_SERVER_PORT);
	saddr.sin_addr.s_addr = INADDR_ANY;
	if (bind(fd, (struct sockaddr *)&saddr, sizeof(struct sockaddr_in)))
	{
		perror("dhcp-helper: cannot bind DHCP server socket");
		exit(1);
	}

	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(altports ? DHCP_SERVER_ALTPORT : DHCP_SERVER_PORT);
	kd_doCommand("SYSTEM LAN", CMD_PRINT, ASH_DO_NOTHING, buf);
	saddr.sin_addr.s_addr = inet_addr(buf);
	//paul_printf("dhcp-helper start : fd=%d, sin_addr=%u.%u.%u.%u:%d\n", outfd, NIPQUAD(saddr.sin_addr.s_addr), saddr.sin_port);
	if (bind(outfd, (struct sockaddr *)&saddr, sizeof(struct sockaddr_in)))
	{
		perror("dhcp-helper: cannot bind DHCP server socket");
		exit(1);
	}

	if (!debug)
	{
		FILE *pidfile;
		int i;
		struct passwd *ent_pw = getpwnam(user);
		gid_t dummy;
		struct group *gp;
		cap_user_header_t hdr = malloc(sizeof(*hdr));
		cap_user_data_t data = NULL;

		if (getuid() == 0)
		{
			if (hdr)
			{
				int capsize = 1;

				/* find version supported by kernel */
				memset(hdr, 0, sizeof(*hdr));
				capget(hdr, NULL);

				if (hdr->version != LINUX_CAPABILITY_VERSION_1)
				{
					/* if unknown version, use largest supported version (3) */
					if (hdr->version != LINUX_CAPABILITY_VERSION_2)
						hdr->version = LINUX_CAPABILITY_VERSION_3;
					capsize = 2;
				}

				if ((data = malloc(sizeof(*data) * capsize)))
					memset(data, 0, sizeof(*data) * capsize);
			}

			if (!hdr || !data)
			{
				perror("dhcp-helper: cannot allocate memory");
				exit(1);
			}

			hdr->pid = 0; /* this process */
			data->effective = data->permitted = data->inheritable = (1 << CAP_NET_ADMIN) | (1 << CAP_SETGID) | (1 << CAP_SETUID);

			/* Tell kernel to not clear capabilities when dropping root */
			if (capset(hdr, data) == -1 || prctl(PR_SET_KEEPCAPS, 1) == -1)
			{
				perror("dhcp-helper: cannot set kernel capabilities");
				exit(1);
			}

			if (!ent_pw)
			{
				fprintf(stderr, "dhcp-helper: cannot find user %s\n", user);
				exit(1);
			};
		}

		/* The following code "daemonizes" the process. See Stevens section 12.4 */

		if (fork() != 0 )
			_exit(0);

		setsid();

		if (fork() != 0)
			_exit(0);

		chdir("/");
		umask(022); /* make pidfile 0644 */

		/* write pidfile _after_ forking ! */
		if ((pidfile = fopen(runfile, "w")))
		{
			fprintf(pidfile, "%d\n", (int) getpid());
			fclose(pidfile);
		}

		umask(0);

		for (i=0; i<64; i++)
			//if (i != fd)
			if ((i != fd) && (i != outfd)) // 2010.03.24 add by paul
				close(i);

		if (getuid() == 0)
		{
			setgroups(0, &dummy);

			if ((gp = getgrgid(ent_pw->pw_gid)))
				setgid(gp->gr_gid);
			setuid(ent_pw->pw_uid);

			data->effective = data->permitted = 1 << CAP_NET_ADMIN;
			data->inheritable = 0;

			/* lose the setuid and setgid capbilities */
			capset(hdr, data);
		}
	}

	FD_ZERO(&allset);
	FD_SET(fd, &allset);
	maxfd = fd;

	FD_SET(outfd, &allset);
	if(outfd > maxfd)
		maxfd = outfd;

	while (1) {
		int iface_index;
		struct in_addr iface_addr;
		struct interface *iface;
		ssize_t sz;
		struct msghdr msg;
		struct iovec iov;
		struct cmsghdr *cmptr;
		struct in_pktinfo *pkt;
		union {
			struct cmsghdr align; /* this ensures alignment */
			char control[CMSG_SPACE(sizeof(struct in_pktinfo))];
		} control_u;

		msg.msg_control = control_u.control;
		msg.msg_controllen = sizeof(control_u);
		msg.msg_name = &saddr;
		msg.msg_namelen = 0;
		msg.msg_iov = &iov;
		msg.msg_iovlen = 1;
		iov.iov_base = packet;
		iov.iov_len = buf_size;

		activefd = -1;
		readset = allset;
		ret = select(maxfd+1, &readset, NULL, NULL, NULL);

		if( ret > 0 && FD_ISSET(fd, &readset) )
			activefd = fd;
		else if( ret > 0 && FD_ISSET(outfd, &readset) )
			activefd = outfd;
		else
			continue;

		/*while (1) {
			struct dhcp_packet *newbuf;
			size_t newsz;

			msg.msg_flags = 0;

			while((sz = recvmsg(fd, &msg, MSG_PEEK)) == -1 && errno == EINTR);

			if (sz == -1 || !(msg.msg_flags & MSG_TRUNC) || !(newbuf = realloc(packet, (newsz = buf_size + 100))))
				break;

			iov.iov_base = packet = newbuf;
			iov.iov_len = buf_size = newsz;
		}*/

		//while ((sz = recvmsg(fd, &msg, 0)) == -1 && errno == EINTR);
		while ((sz = recvmsg(activefd, &msg, 0)) == -1 && errno == EINTR); // 2010.03.24 add by paul

		if ((msg.msg_flags & MSG_TRUNC) || sz < (ssize_t)(sizeof(struct dhcp_packet)) || msg.msg_controllen < sizeof(struct cmsghdr))
			continue;

		iface_index = 0;
		for (cmptr = CMSG_FIRSTHDR(&msg); cmptr; cmptr = CMSG_NXTHDR(&msg, cmptr))
			if (cmptr->cmsg_level == SOL_IP && cmptr->cmsg_type == IP_PKTINFO)
				iface_index = ((struct in_pktinfo *)CMSG_DATA(cmptr))->ipi_ifindex;

		//if (!(ifr.ifr_ifindex = iface_index) || ioctl(fd, SIOCGIFNAME, &ifr) == -1)
		if (!(ifr.ifr_ifindex = iface_index) || ioctl(activefd, SIOCGIFNAME, &ifr) == -1) // 2010.03.24 add by paul
			continue;

		ifr.ifr_addr.sa_family = AF_INET;
		//if (ioctl(fd, SIOCGIFADDR, &ifr) == -1)
		if (ioctl(activefd, SIOCGIFADDR, &ifr) == -1) // 2010.03.24 add by paul
			continue;
		else
			iface_addr = ((struct sockaddr_in *) &ifr.ifr_addr)->sin_addr;

		/* last ditch loop squashing. */
		if ((packet->hops++) > 20)
			continue;

		if (packet->hlen > DHCP_CHADDR_MAX)
			continue;

		if (packet->op == BOOTREQUEST)
		{
			/* message from client */
			struct namelist *tmp;

			/* packets from networks we are broadcasting _too_ are explicitly not allowed to be forwarded _from_ */
			for (tmp = servers; tmp; tmp = tmp->next)
				if (tmp->addr.s_addr == 0 && strncmp(tmp->name, ifr.ifr_name, IF_NAMESIZE) == 0)
					break;
			if (tmp)
				continue;

			/* check if it came from an allowed interface */
			for (tmp = except; tmp; tmp = tmp->next)
				if (strncmp(tmp->name, ifr.ifr_name, IF_NAMESIZE) == 0)
					break;
			if (tmp)
				continue;

			if (interfaces)
			{
				for (tmp = interfaces; tmp; tmp = tmp->next)
					if (strncmp(tmp->name, ifr.ifr_name, IF_NAMESIZE) == 0)
						break;
				if (!tmp)
					continue;
			}

			/* already gatewayed ? */
			if (packet->giaddr.s_addr)
			{
				/* if so check if by us, to stomp on loops. */
				for (iface = ifaces; iface; iface = iface->next)
					if (iface->addr.s_addr == packet->giaddr.s_addr)
						break;
				if (iface)
					continue;
			}
			else
			{
				/* plug in our address */
				packet->giaddr = iface_addr;
			}

			/* send to all configured servers. */
			for (tmp = servers; tmp; tmp = tmp->next)
			{
				/* Do this each time round to pick up address changes. */
				if (tmp->addr.s_addr == 0)
				{
					strncpy(ifr.ifr_name, tmp->name, IF_NAMESIZE);
					//if (ioctl(fd, SIOCGIFBRDADDR, &ifr) == -1)
					if (ioctl(activefd, SIOCGIFBRDADDR, &ifr) == -1) // 2010.03.24 add by paul
						continue;
					saddr.sin_addr = ((struct sockaddr_in *) &ifr.ifr_addr)->sin_addr;
				}
				else
					saddr.sin_addr = tmp->addr;

				saddr.sin_port = htons(altports ? DHCP_SERVER_ALTPORT : DHCP_SERVER_PORT);
				//paul_printf("dhcp-helper BOOTREQUEST, fd=%d, saddr=%u.%u.%u.%u:%d\n", outfd, NIPQUAD(saddr.sin_addr.s_addr), saddr.sin_port);
				//while(sendto(fd, packet, sz, 0, (struct sockaddr *)&saddr, sizeof(saddr)) == -1 && errno == EINTR);
				while(sendto(outfd, packet, sz, 0, (struct sockaddr *)&saddr, sizeof(saddr)) == -1 && errno == EINTR); // 2010.03.24 add by paul
			}

			/* build address->interface index table for returning answers */
			for (iface = ifaces; iface; iface = iface->next)
				if (iface->addr.s_addr == iface_addr.s_addr)
				{
					iface->index = iface_index;
					break;
				}

			/* not there, add a new entry */
			if (!iface && (iface = malloc(sizeof(struct interface))))
			{
				iface->next = ifaces;
				ifaces = iface;
				iface->addr = iface_addr;
				iface->index = iface_index;
			}
		}
		else if (packet->op == BOOTREPLY)
		{
			/* packet from server send back to client */	
			saddr.sin_port = htons(altports ? DHCP_CLIENT_ALTPORT : DHCP_CLIENT_PORT);
			msg.msg_control = NULL;
			msg.msg_controllen = 0;
			msg.msg_namelen = sizeof(saddr);
			iov.iov_len = sz;

			/* look up interface index in cache */
			for (iface = ifaces; iface; iface = iface->next)
				if (iface->addr.s_addr == packet->giaddr.s_addr)
					break;

			if (!iface)
				continue;

			if (packet->ciaddr.s_addr)
				saddr.sin_addr = packet->ciaddr;
			else if (ntohs(packet->flags) & 0x8000 || packet->hlen > 14)
			{
				/* broadcast to 255.255.255.255 */
				msg.msg_controllen = sizeof(control_u);
				msg.msg_control = control_u.control;
				cmptr = CMSG_FIRSTHDR(&msg);
				saddr.sin_addr.s_addr = INADDR_BROADCAST;
				pkt = (struct in_pktinfo *)CMSG_DATA(cmptr);
				pkt->ipi_ifindex = iface->index;
				pkt->ipi_spec_dst.s_addr = 0;
				msg.msg_controllen = cmptr->cmsg_len = CMSG_LEN(sizeof(struct in_pktinfo));
				cmptr->cmsg_level = SOL_IP;
				cmptr->cmsg_type = IP_PKTINFO;
			}
			else
			{
				/* client not configured and cannot reply to ARP. Insert arp entry direct.*/
				saddr.sin_addr = packet->yiaddr;
				ifr.ifr_ifindex = iface->index;
				//if (ioctl(fd, SIOCGIFNAME, &ifr) != -1)
				if (ioctl(activefd, SIOCGIFNAME, &ifr) != -1) // 2010.03.24 add by paul
				{
					struct arpreq req;
					*((struct sockaddr_in *)&req.arp_pa) = saddr;
					req.arp_ha.sa_family = packet->htype;
					memcpy(req.arp_ha.sa_data, packet->chaddr, packet->hlen);
					strncpy(req.arp_dev, ifr.ifr_name, 16);
					req.arp_flags = ATF_COM;
					//ioctl(fd, SIOCSARP, &req);
					ioctl(activefd, SIOCSARP, &req); // 2010.03.24 add by paul
				}
			}
			//paul_printf("dhcp-helper BOOTREPLY, fd=%d, saddr=%u.%u.%u.%u:%d\n", outfd, NIPQUAD(saddr.sin_addr.s_addr), saddr.sin_port);
			//while (sendmsg(fd, &msg, 0) == -1 && errno == EINTR);
			while (sendmsg(outfd, &msg, 0) == -1 && errno == EINTR); // 2010.03.24 add by paul
		}
	}
}
