#include <sys/param.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/signal.h>
#include <sys/types.h>

#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

//#define nsd_debug
#ifdef nsd_debug
  #define trenchen_printf(x...) printf(x)
#else
  #define trenchen_printf(x...)
#endif

/* It turns out that libc5 doesn't have proper icmp support
 * built into it header files, so we have to supplement it */
#if __GNU_LIBRARY__ < 5
static const int ICMP_MINLEN = 8;				/* abs minimum */

struct icmp_ra_addr
{
  u_int32_t ira_addr;
  u_int32_t ira_preference;
};


struct icmp
{
  u_int8_t  icmp_type;	/* type of message, see below */
  u_int8_t  icmp_code;	/* type sub code */
  u_int16_t icmp_cksum;	/* ones complement checksum of struct */
  union
  {
    u_char ih_pptr;		/* ICMP_PARAMPROB */
    struct in_addr ih_gwaddr;	/* gateway address */
    struct ih_idseq		/* echo datagram */
    {
      u_int16_t icd_id;
      u_int16_t icd_seq;
    } ih_idseq;
    u_int32_t ih_void;

    /* ICMP_UNREACH_NEEDFRAG -- Path MTU Discovery (RFC1191) */
    struct ih_pmtu
    {
      u_int16_t ipm_void;
      u_int16_t ipm_nextmtu;
    } ih_pmtu;

    struct ih_rtradv
    {
      u_int8_t irt_num_addrs;
      u_int8_t irt_wpa;
      u_int16_t irt_lifetime;
    } ih_rtradv;
  } icmp_hun;
#define	icmp_pptr	icmp_hun.ih_pptr
#define	icmp_gwaddr	icmp_hun.ih_gwaddr
#define	icmp_id		icmp_hun.ih_idseq.icd_id
#define	icmp_seq	icmp_hun.ih_idseq.icd_seq
#define	icmp_void	icmp_hun.ih_void
#define	icmp_pmvoid	icmp_hun.ih_pmtu.ipm_void
#define	icmp_nextmtu	icmp_hun.ih_pmtu.ipm_nextmtu
#define	icmp_num_addrs	icmp_hun.ih_rtradv.irt_num_addrs
#define	icmp_wpa	icmp_hun.ih_rtradv.irt_wpa
#define	icmp_lifetime	icmp_hun.ih_rtradv.irt_lifetime
  union
  {
    struct
    {
      u_int32_t its_otime;
      u_int32_t its_rtime;
      u_int32_t its_ttime;
    } id_ts;
    struct
    {
      struct ip idi_ip;
      /* options and then 64 bits of data */
    } id_ip;
    struct icmp_ra_addr id_radv;
    u_int32_t   id_mask;
    u_int8_t    id_data[1];
  } icmp_dun;
#define	icmp_otime	icmp_dun.id_ts.its_otime
#define	icmp_rtime	icmp_dun.id_ts.its_rtime
#define	icmp_ttime	icmp_dun.id_ts.its_ttime
#define	icmp_ip		icmp_dun.id_ip.idi_ip
#define	icmp_radv	icmp_dun.id_radv
#define	icmp_mask	icmp_dun.id_mask
#define	icmp_data	icmp_dun.id_data
};
#endif

static const int DEFDATALEN = 16;
static const int MAXIPLEN = 60;
static const int MAXICMPLEN = 76;
static const int MAXPACKET = 65468;
#define	MAX_DUP_CHK	(8 * 128)
static const int MAXWAIT = 10;
static const int PINGINTERVAL = 1;		/* second */

#define O_QUIET         (1 << 0)

#define	A(bit)		rcvd_tbl[(bit)>>3]	/* identify byte in array */
#define	B(bit)		(1 << ((bit) & 0x07))	/* identify bit in byte */
#define	SET(bit)	(A(bit) |= B(bit))
#define	CLR(bit)	(A(bit) &= (~B(bit)))
#define	TST(bit)	(A(bit) & B(bit))


int create_icmp_socket(void)
{
	struct protoent *proto;
	int sock;

	proto = getprotobyname("icmp");
	/* if getprotobyname failed, just silently force
	 * proto->p_proto to have the correct value for "icmp" */
	if ((sock = socket(AF_INET, SOCK_RAW,
			(proto ? proto->p_proto : 1))) < 0) {        /* 1 == ICMP */
		printf("NSD Ping: open socket error\n");
	}

	/* drop root privs if running setuid */
	setuid(getuid());

	return sock;
}

/* common routines */
static int in_cksum(unsigned short *buf, int sz)
{
	int nleft = sz;
	int sum = 0;
	unsigned short *w = buf;
	unsigned short ans = 0;

	while (nleft > 1) {
		sum += *w++;
		nleft -= 2;
	}

	if (nleft == 1) {
		*(unsigned char *) (&ans) = *(unsigned char *) w;
		sum += ans;
	}

	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	ans = ~sum;
	return (ans);
}

/* simple version */
static char *hostname = NULL;

/*static void noresp(int ign)
{
	printf("No response from %s\n", hostname);
	return ;
}*/

int ping(const char *host, const char *SourceIp)
{
	//struct hostent *h;
	int count;
	fd_set set;
	struct timeval timeout;
	struct sockaddr_in from;
	struct sockaddr_in Source;
	size_t fromlen = sizeof(from);
	time_t tstart,tend;
	int res;
	int myid;

	struct sockaddr_in pingaddr;
	struct icmp *pkt;
	int pingsock, c;
	char packet[DEFDATALEN + MAXIPLEN + MAXICMPLEN];

	pingsock = create_icmp_socket();

	memset(&pingaddr, 0, sizeof(struct sockaddr_in));
	memset(&Source, 0, sizeof(struct sockaddr_in));
	
	Source.sin_family = AF_INET;
	if( (Source.sin_addr.s_addr = inet_addr(SourceIp)) == -1 ) {
		printf("NSD Ping: SourceIp inet_addr() transfer error\n");
		res=-1;
		goto exit;
	}
	if( bind(pingsock, (struct sockaddr *)&Source,sizeof(Source)) == -1) {
		printf("NSD Ping: bind error\n");
		res=-1;
		goto exit;
	}

	pingaddr.sin_family = AF_INET;
	if( (pingaddr.sin_addr.s_addr = inet_addr(host)) == -1) {
		printf("NSD Ping: pingaddr inet_addr() transfer error\n");
		res=-1;
		goto exit;
	}
	//h = xgethostbyname(host);
	//memcpy(&pingaddr.sin_addr, h->h_addr, sizeof(pingaddr.sin_addr));
	//hostname = h->h_name;
	hostname = host;

	pkt = (struct icmp *) packet;
	memset(pkt, 0, sizeof(packet));
	pkt->icmp_type = ICMP_ECHO;
	myid = getpid() & 0xFFFF;
	pkt->icmp_id = myid;
	pkt->icmp_cksum = in_cksum((unsigned short *) pkt, sizeof(packet));

	c = sendto(pingsock, packet, sizeof(packet), 0,
			   (struct sockaddr *) &pingaddr, sizeof(struct sockaddr_in));

	if (c < 0 || c != sizeof(packet)) {
		perror("ping sendto:");
		res = -1;
		goto exit;
	}

	FD_ZERO(&set);
	FD_SET(pingsock,&set);
	timeout.tv_sec = 5;
	timeout.tv_usec = 0;

	while(timeout.tv_sec > 0) {
		time(&tstart);
		if( (count = select(pingsock+1, &set,0,0,&timeout)) == 0 ) {
			trenchen_printf("No response from %s\n", hostname);
			res = -1;
			goto exit;
		} else if ( count < 0 ) {
			perror("ping select");
			res = -1;
			goto exit;
		}

		if ((c = recvfrom(pingsock, packet, sizeof(packet), 0,
				  (struct sockaddr *) &from, &fromlen)) < 0) {
				perror("ping recvfrom");
		}
		if (c >= 36) {			/* ip + icmp */
			struct iphdr *iphdr = (struct iphdr *) packet;
			pkt = (struct icmp *) (packet + (iphdr->ihl << 2));	/* skip ip hdr */
			if (pkt->icmp_type == ICMP_ECHOREPLY && pkt->icmp_id == myid) {
				trenchen_printf("%s is alive!\n", hostname);
				res = 0;
				goto exit;
			}
		}
		time(&tend);
		timeout.tv_sec =  timeout.tv_sec-(tend-tstart);
	}
#if 0
	signal(SIGALRM, noresp);
	alarm(5);					/* give the host 5000ms to respond */
	/* listen for replies */
	while (1) {
		struct sockaddr_in from;
		size_t fromlen = sizeof(from);

		printf("timeout[%d]\n",TimeOut);
		if ((c = recvfrom(pingsock, packet, sizeof(packet), 0,
						  (struct sockaddr *) &from, &fromlen)) < 0) {
			if (errno == EINTR)
				continue;
			printf("recvfrom\n");
			continue;
		}
		if (c >= 76) {			/* ip + icmp */
			struct iphdr *iphdr = (struct iphdr *) packet;

			pkt = (struct icmp *) (packet + (iphdr->ihl << 2));	/* skip ip hdr */
			if (pkt->icmp_type == ICMP_ECHOREPLY)
				break;
		}
	}
#endif

	trenchen_printf("receive error\n");
	res = -1;
exit :
	close(pingsock);
	return res;

}
