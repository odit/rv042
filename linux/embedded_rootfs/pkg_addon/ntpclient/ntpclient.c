/*
 * ntpclient.c - NTP client
 *
 * Copyright 1997, 1999, 2000  Larry Doolittle  <larry@doolittle.boa.org>
 * Last hack: 2 December, 2000
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License (Version 2,
 *  June 1991) as published by the Free Software Foundation.  At the
 *  time of writing, that license was published by the FSF with the URL
 *  http://www.gnu.org/copyleft/gpl.html, and is incorporated herein by
 *  reference.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  Possible future improvements:
 *      - Double check that the originate timestamp in the received packet
 *        corresponds to what we sent.
 *      - Verify that the return packet came from the host we think
 *        we're talking to.  Not necessarily useful since UDP packets
 *        are so easy to forge.
 *      - Complete phase locking code.
 *      - Write more documentation  :-(
 *
 *  Compile with -D_PRECISION_SIOCGSTAMP if your machine really has it.
 *  There are patches floating around to add this to Linux, but
 *  usually you only get an answer to the nearest jiffy.
 *  Hint for Linux hacker wannabes: look at the usage of get_fast_time()
 *  in net/core/dev.c, and its definition in kernel/time.c .
 *
 *  If the compile gives you any flak, check below in the section
 *  labelled "XXXX fixme - non-automatic build configuration".
 */
#include "nkdef.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/utsname.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#ifdef _PRECISION_SIOCGSTAMP
#include <sys/ioctl.h>
#endif


// --> Ryoko 2005/07/20
//#include "nkdef.h"
#include "nkutil.h"
//#include "time_zones.h"

#define NTP_M_TIMER		60*60*1		// 1hr (update UTC time erery hour)
//#define NTP_M_TIMER		86400		// 24 hours
#define NTP_RETRY_TIMER		300		// 5 mins
#define NTP_PORT_RANGE		10000
// <-- Ryoko 2005/07/20
#ifndef SOCK_DGRAM
#define SOCK_DGRAM       1
#endif

#define ENABLE_DEBUG

extern char *optarg;

/* XXXX fixme - non-automatic build configuration */
#ifdef linux
#include <asm/types.h>
#include <sys/timex.h>
#else
extern int h_errno;
#define herror(hostname) \
	fprintf(stderr,"Error %d looking up hostname %s\n", h_errno,hostname)
typedef uint32_t __u32;
#endif

#define JAN_1970        0x83aa7e80      /* 2208988800 1970 - 1900 in seconds */
#define NTP_PORT (123)

/* How to multiply by 4294.967296 quickly (and not quite exactly)
 * without using floating point or greater than 32-bit integers.
 * If you want to fix the last 12 microseconds of error, add in
 * (2911*(x))>>28)
 */
#define NTPFRAC(x) ( 4294*(x) + ( (1981*(x))>>11 ) )

/* The reverse of the above, needed if we want to set our microsecond
 * clock (via settimeofday) based on the incoming time in NTP format.
 * Basically exact.
 */
#define USEC(x) ( ( (x) >> 12 ) - 759 * ( ( ( (x) >> 10 ) + 32768 ) >> 16 ) )

/* Converts NTP delay and dispersion, apparently in seconds scaled
 * by 65536, to microseconds.  RFC1305 states this time is in seconds,
 * doesn't mention the scaling.
 * Should somehow be the same as 1000000 * x / 65536
 */
#define sec2u(x) ( (x) * 15.2587890625 )

struct ntptime {
	unsigned int coarse;
	unsigned int fine;
};

void send_packet(int usd);
void rfc1305print(char *data, struct ntptime *arrival);
void udp_handle(int usd, char *data, int data_len, struct sockaddr *sa_source, int sa_len);

/* global variables (I know, bad form, but this is a short program) */
char incoming[1500];
struct timeval time_of_send;
int live=0;
int set_clock=0;   /* non-zero presumably needs root privs */

#ifdef ENABLE_DEBUG
int debug=0;
#define DEBUG_OPTION "d"
#else
#define debug 0
#define DEBUG_OPTION
#endif

int contemplate_data(unsigned int absolute, double skew, double errorbar, int freq);

int get_current_freq()
{
	/* OS dependent routine to get the current value of clock frequency.
	 */
/* #ifdef linux */
#if 0
	struct timex txc;
	txc.modes=0;
	if (__adjtimex(&txc) < 0) {
		perror("adjtimex"); exit(1);
	}
	return txc.freq;
#else
	return 0;
#endif
}

int set_freq(int new_freq)
{
	/* OS dependent routine to set a new value of clock frequency.
	 */
/* #ifdef linux */
#if 0
	struct timex txc;
	txc.modes = ADJ_FREQUENCY;
	txc.freq = new_freq;
	if (__adjtimex(&txc) < 0) {
		perror("adjtimex"); exit(1);
	}
	return txc.freq;
#else
	return 0;
#endif
}

void send_packet(int usd)
{
	__u32 data[12];
	struct timeval now;
#define LI 0
#define VN 3
#define MODE 3
#define STRATUM 0
#define POLL 4 
#define PREC -6

	if (debug) fprintf(stderr,"Sending ...\n");
	if (sizeof(data) != 48) {
		fprintf(stderr,"size error\n");
		return;
	}
	bzero(data,sizeof(data));
	data[0] = htonl (
		( LI << 30 ) | ( VN << 27 ) | ( MODE << 24 ) |
		( STRATUM << 16) | ( POLL << 8 ) | ( PREC & 0xff ) );
	data[1] = htonl(1<<16);  /* Root Delay (seconds) */
	data[2] = htonl(1<<16);  /* Root Dispersion (seconds) */
	gettimeofday(&now,NULL);
	data[10] = htonl(now.tv_sec + JAN_1970); /* Transmit Timestamp coarse */
	data[11] = htonl(NTPFRAC(now.tv_usec));  /* Transmit Timestamp fine   */
	send(usd,data,48,0);
	time_of_send=now;
}


void udp_handle(int usd, char *data, int data_len, struct sockaddr *sa_source, int sa_len)
{
	struct timeval udp_arrival;
	struct ntptime udp_arrival_ntp;

#ifdef _PRECISION_SIOCGSTAMP
	if ( ioctl(usd, SIOCGSTAMP, &udp_arrival) < 0 ) {
		perror("ioctl-SIOCGSTAMP");
		gettimeofday(&udp_arrival,NULL);
	}
#else
	gettimeofday(&udp_arrival,NULL);
#endif
	udp_arrival_ntp.coarse = udp_arrival.tv_sec + JAN_1970;
	udp_arrival_ntp.fine   = NTPFRAC(udp_arrival.tv_usec);

	if (debug) {
		struct sockaddr_in *sa_in=(struct sockaddr_in *)sa_source;
		printf("packet of length %d received\n",data_len);
		if (sa_source->sa_family==AF_INET) {
			printf("Source: INET Port %d host %s\n",
				ntohs(sa_in->sin_port),inet_ntoa(sa_in->sin_addr));
		} else {
			printf("Source: Address family %d\n",sa_source->sa_family);
		}
	}
	rfc1305print(data,&udp_arrival_ntp);
}

double ntpdiff( struct ntptime *start, struct ntptime *stop)
{
	int a;
	unsigned int b;
	a = stop->coarse - start->coarse;
	if (stop->fine >= start->fine) {
		b = stop->fine - start->fine;
	} else {
		b = start->fine - stop->fine;
		b = ~b;
		a -= 1;
	}
	
	return a*1.e6 + b * (1.e6/4294967296.0);
}

#if 0
/* set local time with timezone and daylight if needed --> Ryoko 2005/07/20*/
void nk_set_localtime(void)
{
	struct tm		tm;
	struct timeval	tv;
	struct timezone	tz;
	char	cmd[CMDBUF_SIZE], zone_name[100];
	int		i;

	// init vars
	memset(cmd, 0, sizeof(cmd));
	memset(zone_name, 0, sizeof(zone_name));

	// read time zone from sysconfig
	sprintf(cmd, "SYSTEM TIMEZONE");
	kd_doCommand(cmd, CMD_PRINT, ASH_DO_NOTHING, (char *)zone_name);
	for (i=0; time_zones[i].name; i++)
	{
		if ( strlen(zone_name) && !strncmp(zone_name, time_zones[i].name, strlen(time_zones[i].name)) )
			break;
	} // for loop

	if (!time_zones[i].name)
	{
		printf("the timezone name wasn't found on the time zone list !\n");
		return;
	}

	gettimeofday(&tv, &tz);
	tv.tv_sec = tv.tv_sec + time_zones[i].gmt_offset * 3600;
	settimeofday(&tv, &tz);

	// print current time
	gettimeofday(&tv, &tz);
	memcpy(&tm, localtime(&tv.tv_sec), sizeof(struct tm));
	printf("[GMT+(%d)%s]%d %02d %02d %02d:%02d:%02d\n", time_zones[i].gmt_offset, time_zones[i].name,
											tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	// daylight saving
	if (time_zones[i].dst_idx)
	{
		unsigned char	sday;
		unsigned char	eday;
		unsigned char	smonth;
		unsigned char	emonth;
		unsigned char	stime;
		unsigned char	etime;
		int	k;

		gettimeofday(&tv, &tz);
		memcpy(&tm, localtime(&tv.tv_sec), sizeof(struct tm));

		k = tm.tm_year + 1900 - 2005;
		smonth = dstEntry[time_zones[i].dst_idx].start_month;
		emonth =	dstEntry[time_zones[i].dst_idx].end_month;
		sday =	dstEntry[time_zones[i].dst_idx].start_day[k];
		eday =	dstEntry[time_zones[i].dst_idx].end_day[k];
		stime =	dstEntry[time_zones[i].dst_idx].start_time;
		etime =	dstEntry[time_zones[i].dst_idx].end_time;

		if ( (tm.tm_mon+1 == smonth && tm.tm_mday == sday && tm.tm_hour >= stime) ||
			 (tm.tm_mon+1 == smonth && tm.tm_mday > sday) ||
			 (tm.tm_mon+1 == emonth && tm.tm_mday == eday && tm.tm_hour < etime) ||
			 (tm.tm_mon+1 == emonth && tm.tm_mday < eday) ||
			 (emonth > smonth && tm.tm_mon+1 > smonth && tm.tm_mon+1 < emonth) ||
			 (emonth < smonth && ((tm.tm_mon+1 > smonth && tm.tm_mon+1 > emonth) || (tm.tm_mon+1 < smonth && tm.tm_mon+1 < emonth))) )
		{
			tv.tv_sec = tv.tv_sec + 3600;
			settimeofday(&tv, &tz);
		}

		gettimeofday(&tv, &tz);
		memcpy(&tm, localtime(&tv.tv_sec), sizeof(struct tm));
		printf("(DST)%d %02d %02d %02d:%02d:%02d\n", tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	} /* daylight */

	    kd_doCommand(NULL, CMD_WRITE, ASH_DO_RTC, NULL);
	return;
} /* nk_set_localtime() */
#endif

//<-- Ryoko 2005/07/20
/* Does more than print, so this name is bogus.
 * It also makes time adjustments, both sudden (-s)
 * and phase-locking (-l).  */
void rfc1305print(char *data, struct ntptime *arrival)
{
/* straight out of RFC-1305 Appendix A */
	int li, vn, mode, stratum, poll, prec;
	int delay, disp, refid;
	struct ntptime reftime, orgtime, rectime, xmttime;
	double etime,r_stime,skew1,skew2;
	int freq;

#define Data(i) ntohl(((unsigned int *)data)[i])
	li      = Data(0) >> 30 & 0x03;
	vn      = Data(0) >> 27 & 0x07;
	mode    = Data(0) >> 24 & 0x07;
	stratum = Data(0) >> 16 & 0xff;
	poll    = Data(0) >>  8 & 0xff;
	prec    = Data(0)       & 0xff;
	if (prec & 0x80) prec|=0xffffff00;
	delay   = Data(1);
	disp    = Data(2);
	refid   = Data(3);
	reftime.coarse = Data(4);
	reftime.fine   = Data(5);
	orgtime.coarse = Data(6);
	orgtime.fine   = Data(7);
	rectime.coarse = Data(8);
	rectime.fine   = Data(9);
	xmttime.coarse = Data(10);
	xmttime.fine   = Data(11);
#undef Data

	if (set_clock) {   /* you'd better be root, or ntpclient will crash! */
		struct timeval tv_set;
		/* it would be even better to subtract half the slop */
		tv_set.tv_sec  = xmttime.coarse - JAN_1970;
		/* divide xmttime.fine by 4294.967296 */
		tv_set.tv_usec = USEC(xmttime.fine);
		if (settimeofday(&tv_set,NULL)<0) {
			perror("settimeofday");
			exit(1);
		}
		if (debug) {
			printf("set time to %lu.%.6lu\n", tv_set.tv_sec, tv_set.tv_usec);
		}
		/* set local time with timezone and daylight if needed --> Ryoko 2005/07/20 */
	    //kd_doCommand(NULL, CMD_WRITE, ASH_DO_RTC, NULL);		
//		nk_set_localtime();
		// <--
	}

	if (debug) {
	printf("LI=%d  VN=%d  Mode=%d  Stratum=%d  Poll=%d  Precision=%d\n",
		li, vn, mode, stratum, poll, prec);
	printf("Delay=%.1f  Dispersion=%.1f  Refid=%u.%u.%u.%u\n",
		sec2u(delay),sec2u(disp),
		refid>>24&0xff, refid>>16&0xff, refid>>8&0xff, refid&0xff);
	printf("Reference %u.%.10u\n", reftime.coarse, reftime.fine);
	printf("Originate %u.%.10u\n", orgtime.coarse, orgtime.fine);
	printf("Receive   %u.%.10u\n", rectime.coarse, rectime.fine);
	printf("Transmit  %u.%.10u\n", xmttime.coarse, xmttime.fine);
	printf("Our recv  %u.%.10u\n", arrival->coarse, arrival->fine);
	}
	etime=ntpdiff(&orgtime,arrival);
	r_stime=ntpdiff(&rectime,&xmttime);
	skew1=ntpdiff(&orgtime,&rectime);
	skew2=ntpdiff(&xmttime,arrival);
	freq=get_current_freq();
	if (debug) {
	printf("Total elapsed: %9.2f\n"
	       "Server stall:  %9.2f\n"
	       "Slop:          %9.2f\n",
		etime, r_stime, etime-r_stime);
	printf("Skew:          %9.2f\n"
	       "Frequency:     %9d\n"
	       " day   second     elapsed    stall     skew  dispersion  freq\n",
		(skew1-skew2)/2, freq);
	}
	/* Not the ideal order for printing, but we want to be sure
	 * to do all the time-sensitive thinking (and time setting)
	 * before we start the output, especially fflush() (which
	 * could be slow).  Of course, if debug is turned on, speed
	 * has gone down the drain anyway. */
	if (live) {
		int new_freq;
		new_freq = contemplate_data(arrival->coarse, (skew1-skew2)/2,
			etime+sec2u(disp), freq);
		if (!debug && new_freq != freq) set_freq(new_freq);
	}
	printf("%d %5d.%.3d  %8.1f %8.1f  %8.1f %8.1f %9d\n",
		arrival->coarse/86400+15020, arrival->coarse%86400,
		arrival->fine/4294967, etime, r_stime,
		(skew1-skew2)/2, sec2u(disp), freq);
	fflush(stdout);
}

/* Ryoko 2005/07/20: Don't quit the process even resolve failed */
int stuff_net_addr(struct in_addr *p, char *hostname)
{
#ifdef CONFIG_NK_CHANGE_SRC_PORT
	struct in_addr resolved_addr;
	//ntpserver=gethostbyname_new(hostname);
	if (!gethostbyname_new(hostname, &resolved_addr)) {
//		herror(hostname);
		//exit(1);
	}
	memcpy(&(p->s_addr), &(resolved_addr.s_addr) ,4);
	return 0;
#else
	struct hostent *ntpserver;
	
	ntpserver=gethostbyname(hostname);
	if (ntpserver == NULL) {
//		herror(hostname);
		//exit(1);
	}
	if (ntpserver->h_length != 4) {
//		fprintf(stderr,"oops %d\n",ntpserver->h_length);
		//exit(1);
	}
	memcpy(&(p->s_addr),ntpserver->h_addr_list[0],4);
	return 0;
#endif
}

void setup_receive(int usd, unsigned int interface, short port)
{
	struct sockaddr_in sa_rcvr;
	struct timeval today;
	int SrcPort=0;

	bzero((char *) &sa_rcvr, sizeof(sa_rcvr));
	sa_rcvr.sin_family=AF_INET;
	sa_rcvr.sin_addr.s_addr=htonl(interface);

#ifdef CONFIG_NK_CHANGE_SRC_PORT
	gettimeofday(&today, NULL);
	
	srand(today.tv_usec);
	SrcPort=((rand()%NTP_PORT_RANGE)+NTP_PORT_RANGE)+1;
	sa_rcvr.sin_port=htons(SrcPort);
#else
	sa_rcvr.sin_port=htons(port);
#endif
	if(bind(usd,(struct sockaddr *) &sa_rcvr,sizeof(sa_rcvr)) == -1) {
#ifdef CONFIG_NK_CHANGE_SRC_PORT
		fprintf(stderr,"could not bind to udp port %d\n",SrcPort);
#else
		fprintf(stderr,"could not bind to udp port %d\n",port);
#endif
		perror("bind");
		exit(1);
	}
	listen(usd,3);
}
void setup_transmit(int usd, char *host, short port)
{
	struct sockaddr_in sa_dest;

	bzero((char *) &sa_dest, sizeof(sa_dest));
	sa_dest.sin_family=AF_INET;
	for ( ; ; )
	{
		if (stuff_net_addr(&(sa_dest.sin_addr),host) != 0)
			goto DO_AGAIN;

		sa_dest.sin_port=htons(port);
		if (connect(usd,(struct sockaddr *)&sa_dest,sizeof(sa_dest))==-1)
			goto DO_AGAIN;

		// go here, everything is ok

		break;

DO_AGAIN:
		/* Kide 2005/04/15/: wait 5min and then try again when failed */
		sleep(NTP_RETRY_TIMER);
	} // for loop
}
void primary_loop(int usd, int num_probes, int interval)
{
	fd_set fds;
	struct sockaddr sa_xmit;
	int i, pack_len, sa_xmit_len, probes_sent;
	struct timeval to;
/* infinitely loop --> Ryoko 2005/07/20 */
	int		tsleep;
	static char cmdBuf[4];
DO_AGAIN:
	tsleep = NTP_RETRY_TIMER;
// <--

	if (debug) printf("Listening...\n");

	probes_sent=0;
	sa_xmit_len=sizeof(sa_xmit);
	to.tv_sec=0;
	to.tv_usec=0;
	for (;;) {
		FD_ZERO(&fds);
		FD_SET(usd,&fds);
		i=select(usd+1,&fds,NULL,NULL,&to);  /* Wait on read or error */
		if ((i!=1)||(!FD_ISSET(usd,&fds))) {
			if (i==EINTR) continue;
			if (i<0) perror("select");
			if (to.tv_sec == 0) {
				if (probes_sent >= num_probes &&
					num_probes != 0) break;
				send_packet(usd);
				++probes_sent;
				to.tv_sec=interval;
				to.tv_usec=0;
			}	
			continue;
		}
		pack_len=recvfrom(usd,incoming,sizeof(incoming),0,
		                  &sa_xmit,&sa_xmit_len);
		if (pack_len<0) {
			perror("recvfrom");
		} else if (pack_len>0 && pack_len<sizeof(incoming)){
			tsleep = NTP_M_TIMER;		// --> Ryoko 2005/07/20   ?????
			udp_handle(usd,incoming,pack_len,&sa_xmit,sa_xmit_len);
			
			kd_doCommand("VERSION RTC",CMD_PRINT, ASH_DO_NOTHING, cmdBuf);
			if(!strcmp(cmdBuf, "1"))
				system("hwclock -w"); //RTC by Robert 10/03/24
		} else {
			printf("Ooops.  pack_len=%d\n",pack_len);
			fflush(stdout);
		}
		if (probes_sent >= num_probes && num_probes != 0) break;
	}
	// sleep some time and then do agin --> Ryoko 2005/07/20
	sleep(tsleep);
	goto DO_AGAIN;
	// <--
}

void do_replay(void)
{
	char line[100];
	int n, day, freq, absolute;
	float sec, etime, r_stime, disp;
	double skew, errorbar;
	int simulated_freq = 0;
	unsigned int last_fake_time = 0;
	double fake_delta_time = 0.0;

	while (fgets(line,sizeof(line),stdin)) {
		n=sscanf(line,"%d %f %f %f %lf %f %d",
			&day, &sec, &etime, &r_stime, &skew, &disp, &freq);
		if (n==7) {
			fputs(line,stdout);
			absolute=(day-15020)*86400+(int)sec;
			errorbar=etime+disp;
			if (debug) printf("contemplate %u %.1f %.1f %d\n",
				absolute,skew,errorbar,freq);
			if (last_fake_time==0) simulated_freq=freq;
			fake_delta_time += (absolute-last_fake_time)*((double)(freq-simulated_freq))/65536;
			if (debug) printf("fake %f %d \n", fake_delta_time, simulated_freq);
			skew += fake_delta_time;
			freq = simulated_freq;
			last_fake_time=absolute;
			simulated_freq = contemplate_data(absolute, skew, errorbar, freq);
		} else {
			fprintf(stderr,"Replay input error\n");
			exit(2);
		}
	}
}

void usage(char *argv0)
{
	fprintf(stderr,
	"Usage: %s \n"
	"\t [-c count]     stop after count time measurements (default 0 means go forever)\n"
	"\t [-d]           print diagnostics (feature can be disabled at compile time)\n"
	"\t -h hostname    (mandatory) NTP server host, against which to measure system time\n"
	"\t [-i interval]  check time every interval seconds (default 600)\n"
	"\t [-l]           attempt to lock local clock to server using adjtimex(2)\n"
	"\t [-p port]      local NTP client UDP port (default 0 means \"any available\")\n"
	"\t [-r]           replay analysis code based on stdin\n"
	"\t [-s]           simple clock set (implies -c 1)\n",
	argv0);
}

int main(int argc, char *argv[]) {
	int usd;  /* socket */
	int c;
	/* These parameters are settable from the command line
	   the initializations here provide default behavior */
	short int udp_local_port=0;   /* default of 0 means kernel chooses */
	int cycle_time=600;           /* seconds */
	int probe_count=0;            /* default of 0 means loop forever */
	/* int debug=0; is a global above */
	char *hostname=NULL;          /* must be set */
	int replay=0;                 /* replay mode overrides everything */

	for (;;) {
		c = getopt( argc, argv, "c:" DEBUG_OPTION "h:i:lp:rs");
		if (c == EOF) break;
		switch (c) {
			case 'c':
				probe_count = atoi(optarg);
				break;
#ifdef ENABLE_DEBUG
			case 'd':
				++debug;
				break;
#endif
			case 'h':
				hostname = optarg;
				break;
			case 'i':
				cycle_time = atoi(optarg);
				break;
			case 'l':
				live++;
				break;
			case 'p':
				udp_local_port = atoi(optarg);
				break;
			case 'r':
				replay++;
				break;
			case 's':
				set_clock++;
				probe_count = 1;
				break;
			default:
				usage(argv[0]);
				exit(1);
		}
	}
	if (replay) {
		do_replay();
		exit(0);
	}
	if (hostname == NULL) {
		usage(argv[0]);
		exit(1);
	}
	if (debug) {
		printf("Configuration:\n"
		"  -c probe_count %d\n"
		"  -d (debug)     %d\n"
		"  -h hostname    %s\n"
		"  -i interval    %d\n"
		"  -l live        %d\n"
		"  -p local_port  %d\n"
		"  -s set_clock   %d\n",
		probe_count, debug, hostname, cycle_time,
		live, udp_local_port, set_clock );
	}

	/* Startup sequence */
	if ((usd=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP))==-1)
		{perror ("socket");exit(1);}

	setup_receive(usd, INADDR_ANY, udp_local_port);

	setup_transmit(usd, hostname, NTP_PORT);

	primary_loop(usd, probe_count, cycle_time);

	close(usd);
	return 0;
}
