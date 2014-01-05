/* tftp-hpa: $Id: tftpd.c 2233 2009-05-12 02:48:54Z jerry $ */

/* $OpenBSD: tftpd.c,v 1.13 1999/06/23 17:01:36 deraadt Exp $	*/

/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef lint
const char *copyright = \
"@(#) Copyright (c) 1983 Regents of the University of California.\n\
 All rights reserved.\n";
const char *rcsid = "tftp-hpa $Id: tftpd.c 2233 2009-05-12 02:48:54Z jerry $";
#endif /* not lint */

/*
 * Trivial file transfer protocol server.
 *
 * This version includes many modifications by Jim Guyton <guyton@rand-unix>
 */

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/mount.h>
#include <signal.h>
#include <fcntl.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/tftp.h>
#include <netdb.h>

#include <setjmp.h>
#include <syslog.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#define __USE_GNU		/* Necessary for basename() on glibc systems */
#include <string.h>
#include <stdlib.h>
#include <pwd.h>
#include <unistd.h>
#include <limits.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include "config.h"
#include "tftpsubs.h"
#include "recvfrom.h"
#include <nkdef.h>
/*#include "../web/webconfig/util.h"*/

#ifdef HAVE_TCPWRAPPERS
#include <tcpd.h>

int deny_severity	= LOG_WARNING;
int allow_severity	= -1;	/* Don't log at all */

struct request_info wrap_request;
#endif

void bsd_signal(int, void (*)(int));
void signal_handler(int signum);

#define KSFLASH_IOC_ERASE		_IOW('K', 101, unsigned long)
#define KSFLASH_IOC_GETSIZE		_IOR('K', 102, unsigned long)
#define KSFLASH_IOC_WRITEDB		_IOW('K', 103, unsigned long)
#define KSFLASH_IOC_ERASE_BOOT	_IOW('K', 104, unsigned long)
#define KSFLASH_IOC_ERASE_CODE	_IOW('K', 105, unsigned long)
#define KSFLASH_IOC_WRITE		_IOW('K', 106, unsigned long)

#ifndef HAVE_SIGSETJMP
#define sigsetjmp(x,y)  setjmp(x)
#define siglongjmp(x,y) longjmp(x,y)
#define sigjmp_buf jmp_buf
#endif

#define	TIMEOUT 5		/* Default timeout (seconds) */
#define TRIES   4		/* Number of attempts to send each packet */
#define TIMEOUT_LIMIT (TRIES*(TRIES+1)/2)

#ifndef OACK
#define OACK	6
#endif
#ifndef EOPTNEG
#define EOPTNEG	8
#endif

extern	char *__progname;
int	peer;
int	timeout    = TIMEOUT;
int	rexmtval   = TIMEOUT;
int	maxtimeout = TIMEOUT_LIMIT*TIMEOUT;

#define	PKTSIZE	MAX_SEGSIZE+4
char	buf[PKTSIZE];
char	ackbuf[PKTSIZE];
struct	sockaddr_in from;
int		fromlen;
off_t	tsize;
int     tsize_ok;

int		ndirs;
char	**dirs;

int		secure = 0;
int		cancreate = 0;

struct	formats;
// --> 2005/09/12 Ryan : added to support loader/firmware/mac address upgrade via Mass Production program
char	upgrade_filename[128];
unsigned int ByteSwap32(unsigned int );
unsigned int calchecksum(unsigned char * , unsigned int );
//TT int get_mac_address(int );
char	msgbuf[PKTSIZE];
// <--

extern int kd_BurnFW(char *FileName, unsigned int len);
extern int kd_BurnLoader(char *FileName, unsigned int len);
extern int kd_BurnFactory(char *FileName, unsigned int len);
//--> Jerry 2009/04/13 modified: to handle error
//extern void nk_strip_header(char *FileName);
extern int nk_strip_header(char *FileName);
//<--

int tftp(struct tftphdr *, int);
void nak(int);
void timer(int);
void justquit(int);
void do_opt(char *, char *, char **);

int set_blksize(char *, char **);
int set_blksize2(char *, char **);
int set_tsize(char *, char **);
int set_timeout(char *, char **);

struct options {
        char    *o_opt;
        int     (*o_fnc)(char *, char **);
} options[] = {
        { "blksize",    set_blksize  },
        { "blksize2",   set_blksize2  },
        { "tsize",      set_tsize },
	{ "timeout",	set_timeout  },
        { NULL,         NULL }
};



static void
usage(void)
{
	syslog(LOG_ERR, "Usage: %s [-c] [-u user] [-t timeout] [-r option...] [-s] [directory ...]",
	       __progname);
	exit(1);
}

/*
 * program starting, what need to initialize
 */
static void
doInit(void)
{
}

void signal_handler(int signum)
{
  //  printf("signum=%d\n",signum);
	if(signum == SIGCHLD)
	{
		while(waitpid(-1,NULL,WNOHANG) > 0); /* clean up child processes */
	}
	// don't comment out this line, other wise program will exit
	else if(signum == SIGHUP)
		doInit();
	else
	{
		while(waitpid(-1,NULL,WNOHANG) > 0);
		exit(0);
	}
}

int
main(int argc, char **argv)
{
	struct tftphdr *tp;
	struct passwd *pw;
	struct options *opt;
	struct sockaddr_in myaddr;
	struct sockaddr_in servaddr;
	int n = 0;
	int on = 1;
	int fd = 0;
	int pid, xx;
	int c;
	int setrv;
	int timeout = 900;	/* Default timeout */
	//	char *user = "nobody";	/* Default user */
	char *user = "root";	/* Default user */

	__progname = basename(argv[0]);

	openlog(__progname, LOG_PID | LOG_NDELAY, LOG_DAEMON);

	while ((c = getopt(argc, argv, "csu:r:t:")) != -1)
		switch (c) {
		case 'c':
			cancreate = 1;
			break;
		case 's':
			secure = 1;
			break;
		case 't':
		  timeout = atoi(optarg);
		  break;
		case 'u':
		  user = optarg;
		  break;
		case 'r':
		  for ( opt = options ; opt->o_opt ; opt++ ) {
		    if ( !strcasecmp(optarg, opt->o_opt) ) {
		      opt->o_opt = ""; /* Don't support this option */
		      break;
		    }
		  }
		  if ( !opt->o_opt ) {
		        syslog(LOG_ERR, "Unknown option: %s", optarg);
			exit(1);
		  }
		  break;

		default:
			usage();
			break;
		}

	for (; optind != argc; optind++) {
		if (dirs)
			dirs = realloc(dirs, (ndirs+2) * sizeof (char *));
		else
			dirs = calloc(ndirs+2, sizeof(char *));
		if (dirs == NULL) {
			syslog(LOG_ERR, "malloc: %m");
			exit(1);
		}			
		dirs[n++] = argv[optind];
		dirs[n] = NULL;
		ndirs++;
	}

	if (secure) {
		if (ndirs == 0) {
			syslog(LOG_ERR, "no -s directory");
			exit(1);
		}
		if (ndirs > 1) {
			syslog(LOG_ERR, "too many -s directories");
			exit(1);
		}
		if (chdir(dirs[0])) {
			syslog(LOG_ERR, "%s: %m", dirs[0]);
			exit(1);
		}
	}

	pw = getpwnam(user);
	if (!pw) {
		syslog(LOG_ERR, "no user %s: %m", user);
		exit(1);
	}

#if KAM_LATER   // not needed if not by inetd
	if (ioctl(fd, FIONBIO, &on) < 0) {
		syslog(LOG_ERR, "ioctl(FIONBIO): %m");
		exit(1);
	}
#else
	// bind to default tftp port
	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		exit(1);
	}
	//printf("fd=%d\n",fd);

        on = 1;
        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (void*)&on, sizeof(on));
	                                      
	bzero(&servaddr, sizeof(servaddr));     /* zero the struct */
	servaddr.sin_family = AF_INET;         /* host byte order */
	servaddr.sin_port = htons(IPPORT_TFTP);/* short, network byte order */
	servaddr.sin_addr.s_addr = INADDR_ANY; /* auto-fill with my IP */

	if (bind(fd, (struct sockaddr *)&servaddr, sizeof(struct sockaddr)) == -1) 
	{
	  	close(fd);
		exit(1);
	}
#endif
	/* This means we don't want to wait() for children */
	bsd_signal(SIGCHLD, SIG_IGN);

	
	// Set up signal handlers so we can clear up our child processes
//	signal(SIGTERM, signal_handler);
	signal(SIGHUP, signal_handler);
	signal(SIGINT, signal_handler);
	signal(SIGCHLD, signal_handler);
	signal(SIGKILL, signal_handler);

	xx = fork();

	if(xx==-1)
	{
		syslog(LOG_CRIT, "Could not fork() tftpd into background\n");
		closelog();
		close(fd);
		exit(1);
	}
	if(xx != 0) {	// Parent...
	  printf("tftp in background pid=%d\n",xx);
		exit(0);
	}

	do {
	  fromlen = sizeof (from);
	  //	  printf("recvfrom %d: ",fd);
#if KAM_LATER	  
	  n = myrecvfrom(fd, buf, sizeof (buf), 0,
			 (struct sockaddr *)&from, &fromlen,
			 &myaddr);
#else
	  bzero(&myaddr, sizeof(struct sockaddr_in));
	  myaddr.sin_family = AF_INET;
	  myaddr.sin_port   = htons(IPPORT_TFTP);
	  n = recvfrom(fd, buf, sizeof (buf), 0,
		       (struct sockaddr *)&from, &fromlen);
#endif
	  
	  if (n < 0) {
	    syslog(LOG_ERR, "recvfrom: %m");
	    exit(1);
	  }

	  //printf("%d bytes ",n);
#if KAM_LATER
//#ifdef HAVE_TCPWRAPPERS
	/* Verify if this was a legal request for us.  This has to be
	   done before the chroot, while /etc is still accessible. */
	  request_init(&wrap_request,
		       RQ_DAEMON, __progname,
		       RQ_FILE, fd,
		       RQ_CLIENT_SIN, &from,
		       RQ_SERVER_SIN, &myaddr,
		       0);
	  sock_methods(&wrap_request);
	  if ( hosts_access(&wrap_request) == 0 ) {
	    if ( deny_severity != -1 )
	      syslog(deny_severity, "connection refused from %s",
		     inet_ntoa(from.sin_addr));
	    exit(1);		/* Access denied */
	  } else if ( allow_severity != -1 ) {
	    syslog(allow_severity, "connect from %s",
		   inet_ntoa(from.sin_addr));
	  }
#endif
	/*
	 * Now that we have read the message out of the UDP
	 * socket, we fork and go back to listening to the
	 * socket.
	 */
	  pid = fork();
	  if (pid < 0) {
	    syslog(LOG_ERR, "fork: %m");
	    exit(1);		/* Return to inetd, just in case */
	  }
	} while ( pid > 0 );	/* Parent process continues... */

	/* Child process: handle the actual request here */

	/* Chroot and drop privileges */

	if (secure && chroot(".")) {
		syslog(LOG_ERR, "chroot: %m");
		exit(1);
	}

#ifdef HAVE_SETREGID
	setrv = setregid(pw->pw_gid, pw->pw_gid);
#else
	setrv = setegid(pw->pw_gid) || setgid(pw->pw_gid);
#endif

#ifdef HAVE_SETREUID
	setrv = setrv || setreuid(pw->pw_uid, pw->pw_uid);
#else
	/* Important: setuid() must come first */
	setrv = setrv || setuid(pw->pw_uid) ||
	  (geteuid() != pw->pw_uid && seteuid(pw->pw_uid));
#endif

	if ( setrv ) {
	  syslog(LOG_ERR, "cannot drop privileges: %m");
	  exit(1);
	}

	/* Close file descriptors we don't need */

	from.sin_family = AF_INET;
	alarm(0);
#if KAM_LATER
	close(fd);
	close(1);
#endif

	/* Process the request... */

	peer = socket(AF_INET, SOCK_DGRAM, 0);
	if (peer < 0) {
		syslog(LOG_ERR, "socket: %m");
		exit(1);
	}
	myaddr.sin_port = htons(0); /* We want a new local port */
	if (bind(peer, (struct sockaddr *)&myaddr, sizeof (myaddr)) < 0) {
		syslog(LOG_ERR, "bind: %m");
		exit(1);
	}
	if (connect(peer, (struct sockaddr *)&from, sizeof(from)) < 0) {
		syslog(LOG_ERR, "connect: %m");
		exit(1);
	}
	tp = (struct tftphdr *)buf;
	tp->th_opcode = ntohs(tp->th_opcode);
	if (tp->th_opcode == RRQ || tp->th_opcode == WRQ) {
	  tftp(tp, n);
	}
	exit(0);
}

int	validate_access(char *, int, struct formats *);
void	sendfile(struct formats *, struct tftphdr *, int);
void	recvfile(struct formats *, struct tftphdr *, int);

struct formats {
	char	*f_mode;
	int	(*f_validate)(char *, int, struct formats *);
	void	(*f_send)(struct formats *, struct tftphdr *, int);
	void	(*f_recv)(struct formats *, struct tftphdr *, int);
	int	f_convert;
        char    filename[128];
} formats[] = {
	{ "netascii",	validate_access,	sendfile,	recvfile, 1, "\0" },
	{ "octet",	validate_access,	sendfile,	recvfile, 0, "\0" },
	{ NULL, NULL, NULL, NULL, 0, "\0" }
};

/*
 * Handle initial connection protocol.
 */
int
tftp(struct tftphdr *tp, int size)
{
	char *cp;
	int argn, ecode;
	struct formats *pf = NULL;
	char *filename, *mode = NULL;

        char *val = NULL, *opt = NULL;
        char *ap = ackbuf + 2;

        ((struct tftphdr *)ackbuf)->th_opcode = ntohs(OACK);

	filename = cp = tp->th_stuff;
	// --> 2005/09/26 Ryan : using tmpfs for firmware upgrade to save run time memory
	printf("filename[%s]\n",filename);
	if (!strncmp(filename+5,"tmpfs",5))
	{
		mkdir("/tmp/tmpfs", 0777);
//TT		mount("tmpfs", "/tmp/tmpfs", "tmpfs", MS_MGC_VAL, NULL);
	}
	// <--

	argn = 0;

	while ( cp < buf + size && *cp ) {
	     do {
		  cp++;
	     } while (cp < buf + size && *cp);

	     if ( *cp ) {
		  nak(EBADOP);	/* Corrupt packet - no final NULL */
		  exit(0);
	     }

	     argn++;
	     if (argn == 1) {
		  mode = ++cp;
	     } else if (argn == 2) {
		  for (cp = mode; *cp; cp++)
			    *cp = tolower(*cp);
		  for (pf = formats; pf->f_mode; pf++) {
		       if (!strcmp(pf->f_mode, mode))
			    break;
		  }
		  if (!pf->f_mode) {
		       nak(EBADOP);
		       exit(0);
		  }
		  ecode = (*pf->f_validate)(filename, tp->th_opcode, pf);
		  if (ecode) {
		       nak(ecode);
		       exit(0);
		  }
		  opt = ++cp;
	     } else if ( argn & 1 ) {
		  val = ++cp;
	     } else {
		  do_opt(opt, val, &ap);
		  opt = ++cp;
	     }
	}

	if (!pf) {
	     nak(EBADOP);
	     exit(0);
	}

	if (strlen(filename) >= 128) {
	     nak(EBADOP);
	     exit(0);
	}
        strcpy(pf->filename, filename);

	//printf("tftp filename=%s\n",filename);
	if ( ap != (ackbuf+2) ) {
	     // doesn't come here
	     if ( tp->th_opcode == WRQ )
		  (*pf->f_recv)(pf, (struct tftphdr *)ackbuf, ap-ackbuf);
	     else
		  (*pf->f_send)(pf, (struct tftphdr *)ackbuf, ap-ackbuf);
	} else {
	     if (tp->th_opcode == WRQ)
		  (*pf->f_recv)(pf, NULL, 0);
	     else
		  (*pf->f_send)(pf, NULL, 0);
	}
	exit(1);
}

static int blksize_set;

/*
 * Set a non-standard block size (c.f. RFC2348)
 */
int
set_blksize(char *val, char **ret)
{
  	static char b_ret[6];
        unsigned int sz = atoi(val);

	if ( blksize_set )
	  return 0;

        if (sz < 8)
                return(0);
        else if (sz > MAX_SEGSIZE)
                sz = MAX_SEGSIZE;

        segsize = sz;
        sprintf(*ret = b_ret, "%u", sz);

	blksize_set = 1;

        return(1);
}

/*
 * Set a power-of-two block size (nonstandard)
 */
int
set_blksize2(char *val, char **ret)
{
  	static char b_ret[6];
        unsigned int sz = atoi(val);

	if ( blksize_set )
	  return 0;

        if (sz < 8)
                return(0);
        else if (sz > MAX_SEGSIZE)
	        sz = MAX_SEGSIZE;

	/* Convert to a power of two */
	if ( sz & (sz-1) ) {
	  unsigned int sz1 = 1;
	  /* Not a power of two - need to convert */
	  while ( sz >>= 1 )
	    sz1 <<= 1;
	  sz = sz1;
	}

        segsize = sz;
        sprintf(*ret = b_ret, "%u", sz);

	blksize_set = 1;

        return(1);
}

/*
 * Return a file size (c.f. RFC2349)
 * For netascii mode, we don't know the size ahead of time;
 * so reject the option.
 */
int
set_tsize(char *val, char **ret)
{
        static char b_ret[sizeof(off_t)*CHAR_BIT/3+2];
        off_t sz = atol(val);

	if ( !tsize_ok )
	  return 0;

        if (sz == 0)
                sz = tsize;
        sprintf(*ret = b_ret, "%lu", sz);
        return(1);
}

/*
 * Set the timeout (c.f. RFC2349)
 */
int
set_timeout(char *val, char **ret)
{
	static char b_ret[4];
	unsigned long to = atol(val);

	if ( to < 1 || to > 255 )
	  return 0;

	timeout    = to;
	rexmtval   = to;
	maxtimeout = TIMEOUT_LIMIT*to;

	sprintf(*ret = b_ret, "%lu", to);
	return(1);
}

/*
 * Parse RFC2347 style options
 */
void
do_opt(char *opt, char *val, char **ap)
{
     struct options *po;
     char *ret;

     /* Global option-parsing variables initialization */
     blksize_set = 0;
     
     if ( !*opt )
	  return;

     for (po = options; po->o_opt; po++)
	  if (!strcasecmp(po->o_opt, opt)) {
	       if (po->o_fnc(val, &ret)) {
		    if (*ap + strlen(opt) + strlen(ret) + 2 >=
			ackbuf + sizeof(ackbuf)) {
			 nak(ENOSPACE);	/* EOPTNEG? */
			 exit(0);
		    }
		    *ap = strrchr(strcpy(strrchr(strcpy(*ap, opt),'\0') + 1,
					 ret),'\0') + 1;
	       } else {
		    nak(EOPTNEG);
		    exit(0);
	       }
	       break;
	  }
     return;
}


FILE *file;

/*
 * Validate file access.  Since we
 * have no uid or gid, for now require
 * file to exist and be publicly
 * readable/writable.
 * If we were invoked with arguments
 * from inetd then the file must also be
 * in one of the given directory prefixes.
 * Note also, full path name must be
 * given as we have no login directory.
 *
 * for soho router, strict rule applied,
 * only certain files in certain directories 
 * can be created, read, written
 */
int
validate_access(char *filename, int mode, struct formats *pf)
{
	struct stat stbuf;
	int	fd, wmode;
#if KAM_LATER
	char *cp, **dirp;
#endif

	tsize_ok = 0;

	//printf("validate_access:\n");
#if KAM_LATER
	if (!secure) {
		if (*filename != '/')
			return (EACCESS);
		/*
		 * prevent tricksters from getting around the directory
		 * restrictions
		 */
		for (cp = filename + 1; *cp; cp++)
			if(*cp == '.' && strncmp(cp-1, "/../", 4) == 0)
				return(EACCESS);
		for (dirp = dirs; *dirp; dirp++)
			if (strncmp(filename, *dirp, strlen(*dirp)) == 0)
				break;
		if (*dirp==0 && dirp!=dirs)
			return (EACCESS);
	}
#endif
	/*
	 * We use different a different permissions scheme if `cancreate' is
	 * set.
	 */
	wmode = O_TRUNC;
	if (stat(filename, &stbuf) < 0) {
		if (!cancreate)
			return (errno == ENOENT ? ENOTFOUND : EACCESS);
		else {
			if ((errno == ENOENT) && (mode != RRQ))
				wmode |= O_CREAT;
			else
				return(EACCESS);
		}
	} else {
		if (mode == RRQ) {
			if ((stbuf.st_mode&(S_IREAD >> 6)) == 0)
				return (EACCESS);
			tsize = stbuf.st_size;
			/* We don't know the tsize if conversion is needed */
			tsize_ok = !pf->f_convert;
		} else {
			if ((stbuf.st_mode&(S_IWRITE >> 6)) == 0)
				return (EACCESS);
			tsize = 0;
			tsize_ok = 1;
		}
	}
	fd = open(filename, mode == RRQ ? O_RDONLY : (O_WRONLY|wmode), 0666);
	//printf("validate_access: fd=%d filename=%s\n",fd, filename);
	if (fd < 0)
		return (errno + 100);
	/*
	 * If the file was created, set default permissions.
	 */
	if ((wmode & O_CREAT) && fchmod(fd, 0666) < 0) {
		int serrno = errno;

		close(fd);
		unlink(filename);

		return (serrno + 100);
	}
	file = fdopen(fd, (mode == RRQ)? "r":"w");
	if (file == NULL) 
		return (errno + 100);

	//printf("validate_access: fdopen OK\n");
	return (0);
}

int	timeout;
sigjmp_buf	timeoutbuf;

void
timer(int sig)
{
        sig = timeout; //to supress warning
	timeout += rexmtval;
	if (timeout >= maxtimeout)
		exit(0);
	siglongjmp(timeoutbuf, 1);
}

/*
 * Send the requested file.
 */
void
sendfile(struct formats *pf, struct tftphdr *oap, int oacklen)
{
	struct tftphdr *dp;
	struct tftphdr *ap;    /* ack packet */
	int block = 1, size, n;

	ap = (struct tftphdr *)ackbuf;

        if (oap) {
	     timeout = 0;
	     (void)sigsetjmp(timeoutbuf,1);
oack:
	     if (send(peer, oap, oacklen, 0) != oacklen) {
		  syslog(LOG_ERR, "tftpd: oack: %m\n");
		  goto abort;
	     }
	     for ( ; ; ) {
	          bsd_signal(SIGALRM, timer);
		  alarm(rexmtval);
		  n = recv(peer, ackbuf, sizeof(ackbuf), 0);
		  alarm(0);
		  if (n < 0) {
		       syslog(LOG_ERR, "tftpd: read: %m\n");
		       goto abort;
		  }
		  ap->th_opcode = ntohs((u_short)ap->th_opcode);
		  ap->th_block = ntohs((u_short)ap->th_block);
		  
		  if (ap->th_opcode == ERROR) {
		       syslog(LOG_ERR, "tftp: client does not accept "
			      "options\n");
		       goto abort;
		  }
		  if (ap->th_opcode == ACK) {
		       if (ap->th_block == 0)
			    break;
		       /* Resynchronize with the other side */
		       (void)synchnet(peer);
		       goto oack;
		  }
	     }
        }

	dp = r_init();
	do {
		size = readit(file, &dp, pf->f_convert);
		if (size < 0) {
			nak(errno + 100);
			goto abort;
		}
		dp->th_opcode = htons((u_short)DATA);
		dp->th_block = htons((u_short)block);
		timeout = 0;
		(void) sigsetjmp(timeoutbuf,1);

send_data:
		if (send(peer, dp, size + 4, 0) != size + 4) {
			syslog(LOG_ERR, "tftpd: write: %m");
			goto abort;
		}
		read_ahead(file, pf->f_convert);
		for ( ; ; ) {
		        bsd_signal(SIGALRM, timer);
			alarm(rexmtval);	/* read the ack */
			n = recv(peer, ackbuf, sizeof (ackbuf), 0);
			alarm(0);
			if (n < 0) {
				syslog(LOG_ERR, "tftpd: read(ack): %m");
				goto abort;
			}
			ap->th_opcode = ntohs((u_short)ap->th_opcode);
			ap->th_block = ntohs((u_short)ap->th_block);

			if (ap->th_opcode == ERROR)
				goto abort;
			
			if (ap->th_opcode == ACK) {
				//if (ap->th_block == block) {
				if (ap->th_block == (u_short)block) {
					break;
				}
				/* Re-synchronize with the other side */
				(void) synchnet(peer);
				//if (ap->th_block == (block -1)) {
				if (ap->th_block == ((u_short)block -1)) {
					goto send_data;
				}
			}

		}
		block++;
	} while (size == segsize);
abort:
	(void) fclose(file);
}

// --> 2005/09/12 Ryan : added to support loader/firmware/mac address upgrade via Mass Production program
unsigned int ByteSwap32(unsigned int x)
{
	return ((((x) & 0xFF000000 ) >> 24)|(((x) &0x00FF0000)>>8)|(((x) & 0x0000FF00) << 8)|(((x) & 0x000000FF) << 24));
}

unsigned int calchecksum(unsigned char *data_buffer , unsigned int size)
{
	unsigned int i , checksum = 0x0;

	for (i = 0 ; i < size ; i++)
	{
		checksum = checksum + (*data_buffer & 0xFF);
		data_buffer ++;
	}
	return checksum;
}
// <--

void
justquit(int sig)
{
	// --> 2005/09/12 Ryan : added to support loader/firmware/mac address upgrade via Mass Production program
	int flash_fd , file_fd , cnt , active_mac, i;
	char buf[512],header_buf[512], comp_buf[32];
	struct image_header *imagehdr;
	unsigned int checksum,tmp_checksum,mac;
	struct tftphdr *msg_ap;
	char msg_buf[512];
	char mac_serial[MAC_SERIAL_LEN];
	int length;
	int upgrade_status;
	// <--
	struct stat statinfo;

	sig = 0; //supress warning
	upgrade_status = 0;

	// --> 2005/09/12 Ryan : added to support loader/firmware/mac address upgrade via Mass Production program
	printf("receive file name[%s]\n",upgrade_filename);
	msg_ap = (struct tftphdr *)msgbuf;
	file_fd = open(upgrade_filename, O_RDONLY);
	if (file_fd != -1)
	{
		// read image header and check the signature first
		if (read(file_fd, header_buf, sizeof(struct image_header)) > 0)
		{
			imagehdr = (struct image_header *)header_buf;
			printf("VENDOR_CODE[%s] PLATFORM_CODE[%s] imagehdr->signature[%s] imagehdr->fwver[%s] imagehdr->fwtype[%s] fwcrc[%x]\n",VENDOR_CODE,PLATFORM_CODE,imagehdr->signature,&imagehdr->fwver[4],imagehdr->fwtype,(imagehdr->fwcrc));
			kd_Log("VENDOR_CODE[%s] PLATFORM_CODE[%s] imagehdr->signature[%s] imagehdr->fwver[%s] imagehdr->fwtype[%s] fwcrc[%x]\n",VENDOR_CODE,PLATFORM_CODE,imagehdr->signature,&imagehdr->fwver[4],imagehdr->fwtype,(imagehdr->fwcrc));
			//printf("kernel base[%x]--size[%d],fs base[%x]--size[%d],fw[%d]\r\n",ByteSwap32(imagehdr->knaddr),ByteSwap32(imagehdr->knsize),ByteSwap32(imagehdr->fsaddr),ByteSwap32(imagehdr->fssize),ByteSwap32(imagehdr->fwsize));
			if (strncmp(imagehdr->signature,PLATFORM_CODE,sizeof(PLATFORM_CODE)))
			{
				printf("unknow signature in header %s\n",imagehdr->signature);
				kd_Log("unknow signature in header %s\n",imagehdr->signature);
				strcpy(msg_buf,"Unknow Signature !!");
				close(file_fd);
				goto send_msg;
			}
			else
			{
				// calculate checksum before trying to burn into flash
				checksum = 0;
				lseek(file_fd, sizeof(struct image_header), SEEK_SET);
				do
				{
					tmp_checksum = 0;
					cnt = read(file_fd, buf, sizeof(buf));
					tmp_checksum = calchecksum(buf,cnt);
					checksum = checksum + tmp_checksum ;
					//printf("cnt[%d] checksum[%x] tmp_checksum[%x]\n",cnt,checksum,tmp_checksum);
				} while (cnt > 0);

				if (checksum == (imagehdr->fwcrc))
				{
					// burn this image to flash
					if (!strncmp(imagehdr->fwtype,FIRMWARE_PATTERN,sizeof(FIRMWARE_PATTERN)) &&
					(!strncmp(&(imagehdr->fwver[4]),VENDOR_CODE,sizeof(VENDOR_CODE)) ||
					 !strncmp(VENDOR_CODE,NONBRAND_VENDOR_CODE,sizeof(NONBRAND_VENDOR_CODE))))
					{
//--> Jerry 2009/05/12 added. For security reason, only the image file with the path "/tmp/tmpfs/" can be written to flash.
					    /* only /tmp/tmpfs/xxx file can be written to flash */
					    if (!strstr(upgrade_filename, "/tmp/tmpfs/"))
					    {
						con_printf("Invaild Path! Stop burning Image to flash.\r\n");
						strcpy(msg_buf,"Invalid Path !!");
						close(file_fd);
						goto send_msg;
					    }
					    else
					    {
						con_printf("TFTP: Burning Image to flash...(It may take several minutes)\r\n");
//<--
						stat(upgrade_filename, &statinfo);
    						kd_BurnFW(upgrade_filename, statinfo.st_size);
						printf("IMG done\n");
						kd_Log("IMG done\n");
//--> Jerry 2009/05/12
					    }
//<--
					}
					else if ((!strncmp(imagehdr->fwtype,BOOT_PATTERN,sizeof(BOOT_PATTERN)) ||
					  !strncmp(imagehdr->fwtype,UN_BOOT_PATTERN,sizeof(UN_BOOT_PATTERN))) &&
					(!strncmp(&(imagehdr->fwver[4]),VENDOR_CODE,sizeof(VENDOR_CODE)) ||
					 !strncmp(VENDOR_CODE,NONBRAND_VENDOR_CODE,sizeof(NONBRAND_VENDOR_CODE))))
					{
						//---> Jerry 2009/4/13 modified. Add error handler
#if 0
						//TT 20070519 : need strip header before burn
						nk_strip_header(upgrade_filename);
						stat(upgrade_filename, &statinfo);
	    					kd_BurnLoader(upgrade_filename, statinfo.st_size);
						printf("BOOT done\n");
						kd_Log("BOOT done\n");
#endif
						if (nk_strip_header(upgrade_filename))
						{
							con_printf("Error: Failed to strip header! Stop burning BOOT.\r\n");
							strcpy(msg_buf,"Invalid Path !!");
							close(file_fd);
							goto send_msg;
						}
						else
						{
							stat(upgrade_filename, &statinfo);
	    						kd_BurnLoader(upgrade_filename, statinfo.st_size);
							printf("BOOT done\n");
							kd_Log("BOOT done\n");
							con_printf("Burning BOOT is done!\r\n");
						}
						//<---
					}
					else if (!strncmp(imagehdr->fwtype,MAC_PATTERN,sizeof(MAC_PATTERN)))
					{
						//---> Jerry 2009/4/13 modified. Add error handler
#if 0
						//TT 20070519 : need strip header before burn
						nk_strip_header(upgrade_filename);
						stat(upgrade_filename, &statinfo);
	    					kd_BurnFactory(upgrade_filename, statinfo.st_size);
						printf("FACTORY done\n");
						kd_Log("FACTORY done\n");
#endif
						if (nk_strip_header(upgrade_filename))
						{
							con_printf("Error: Failed to strip header! Stop burning FACTORY.\r\n");
							strcpy(msg_buf,"Invalid Path !!");
							close(file_fd);
							goto send_msg;
						}
						else
						{
							stat(upgrade_filename, &statinfo);
	    						kd_BurnFactory(upgrade_filename, statinfo.st_size);
							printf("FACTORY done\n");
							kd_Log("FACTORY done\n");
							con_printf("Burning FACTORY is done!\r\n");
						}
						//<---
					}
					else
					{
						strcpy(msg_buf,"Unknow Signature !!");
						close(file_fd);
						//close(flash_fd);	//Jerry 2009/4/14 removed: not define flash_fd. must be a bug!
						goto send_msg;
					}
					//close(flash_fd);		//Jerry 2009/4/14 removed: not define flash_fd. must be a bug!
				}
				else
				{
					printf("checksum error : from file %s[%x] from header[%x]\n",upgrade_filename,checksum,(imagehdr->fwcrc));
					kd_Log("checksum error : from file %s[%x] from header[%x]\n",upgrade_filename,checksum,(imagehdr->fwcrc));
					strcpy(msg_buf,"Checksum error !!");
					close(file_fd);
					goto send_msg;
				}
			}
		}
		else
		{
			printf("fail to read loader/firmware/mac header from %s\n",upgrade_filename);
			kd_Log("fail to read loader/firmware/mac header from %s\n",upgrade_filename);
		}
		close(file_fd);
	}
	else
		printf("fail to open %s\n",upgrade_filename);

	upgrade_status = 1;
	strcpy(msg_buf,"upgrade successful !!");
send_msg:
	strcpy(msg_ap->th_msg,msg_buf);
	length = strlen(msg_buf);
	msg_ap->th_msg[length] = '\0';
	length += 5;
	(void) send(peer, msgbuf, length, 0);
	//TT 20060403 auto reboot when upgrade code successful
	if ((upgrade_status == 1) && 
	(!strncmp(imagehdr->fwtype,FIRMWARE_PATTERN,sizeof(FIRMWARE_PATTERN))))
	{ 
		sleep(1);
		kill(1, SIGTERM);
	}
	//--> Jerry 2009/4/13 added: For convenient to manufacture, remove DIMM 
	//     if destination path is /tmp/tmpfs/FaCtOrY 
	if ((upgrade_status == 1) && 
	    (!strncmp(imagehdr->fwtype,MAC_PATTERN,sizeof(MAC_PATTERN))) &&
	    (!strncmp(upgrade_filename,"/tmp/tmpfs/FaCtOrY", 18))
	)
	{
		con_printf("Clear memory settings ...\r\n");
		system("rm /etc/flash/etc/DIMM\n");
	}
	//<--

	// --> 2005/09/26 Ryan : using tmpfs for firmware upgrade to save run time memory
	if (!strncmp(upgrade_filename+5,"tmpfs",5))
	{
		printf("remove\n");
		remove(upgrade_filename);
//TT		umount("/tmp/tmpfs");
		rmdir("/tmp/tmpfs");
	}
	// <--
	

	// <--
    exit(0);
}


/*
 * Receive a file.
 */
void
recvfile(struct formats *pf, struct tftphdr *oap, int oacklen)
{
	struct tftphdr *dp = NULL;
	struct tftphdr *ap = NULL;    /* ack buffer */
	struct tftphdr *msg_ap = NULL;	/* message buffer */
	static int block = 0, acksize, n, size = 0;
	int written=0;
//--> Jerry 2009/05/12 added. Print messages to console in order to know the upgrade status.
	char tmp_buf[512];
	int cnt;
//<--

	dp = w_init();
//--> Jerry 2009/05/12 added. Print messages to console in order to know the upgrade status.
	cnt=0;
	sprintf(tmp_buf, "\n\rTFTP: Start to receive file [%s]\r\n",pf->filename);
	con_printf(tmp_buf);
//<--
	do {
		timeout = 0;
		ap = (struct tftphdr *)ackbuf;
		if (!block && oap){
		     acksize = oacklen;
		}else {
		     ap->th_opcode = htons((u_short)ACK);
		     ap->th_block = htons((u_short)block);
		     acksize = 4;
		}
		block++;
		(void) sigsetjmp(timeoutbuf,1);
		bsd_signal(SIGALRM, timer);
send_ack:
		if (send(peer, ackbuf, acksize, 0) != acksize) {
			syslog(LOG_ERR, "tftpd: write(ack): %m");
			goto abort;
		}
		written = write_behind(file, pf->f_convert);
		if (written < 0) {
			if (written == -1) nak(errno + 100);
			else nak(ENOSPACE);
			fclose(file);
			unlink(pf->filename);
			goto abort;
		}

		for ( ; ; ) {
		        bsd_signal(SIGALRM, timer);
		  	alarm(rexmtval);
			n = recv(peer, dp, PKTSIZE, 0);
//--> Jerry 2009/05/12 added. Print messages, dot per 1000-block (512KB) transmission, to console in order to know the upgrade status.
			cnt++;
			if( !(cnt%1000) )
			    con_printf(".");
//<--
			alarm(0);
			if (n < 0) {		/* really? */
			        nak(EUNDEF);
				fclose(file);
				unlink(pf->filename);
				syslog(LOG_ERR, "tftpd: read: %m");
				printf("tftpd: read: %m");
				goto abort;
			}
			//printf("recv %d\n",n);
			dp->th_opcode = ntohs((u_short)dp->th_opcode);
			dp->th_block = ntohs((u_short)dp->th_block);
			if (dp->th_opcode == ERROR) {
				goto abort;
			}
			if (dp->th_opcode == DATA) {
				//if (dp->th_block == block) {
				if (dp->th_block == (u_short)block) {
					break;   /* normal */
				}
				/* Re-synchronize with the other side */
				(void) synchnet(peer);
				//if (dp->th_block == (block-1))
				if (dp->th_block == ((u_short)block-1))
					goto send_ack;		/* rexmit */
			}
		}
		/*  size = write(file, dp->th_data, n - 4); */
		size = writeit(file, &dp, n - 4, pf->f_convert);
		if (size != (n-4)) {			/* ahem */
			if (size < 0) nak(errno + 100);
			else nak(ENOSPACE);
			goto abort;
		}
	} while (size == segsize);
//--> Jerry 2009/05/12 added. Print messages to console in order to know the upgrade status.
	con_printf("\n\rTFTP: Receiving file successfully!\r\n");
//<--
	written = write_behind(file, pf->f_convert);
	(void) fclose(file);		/* close data file */

	ap->th_opcode = htons((u_short)ACK);    /* send the "final" ack */
	ap->th_block = htons((u_short)(block));
	(void) send(peer, ackbuf, 4, 0);

	// --> 2005/09/12 Ryan : added to support loader/firmware/mac address upgrade via Mass Production program
	memset((char *)upgrade_filename,0,sizeof(upgrade_filename));
	memcpy((char *)upgrade_filename,pf->filename,sizeof(pf->filename));
	msg_ap = (struct tftphdr *)msgbuf;
	msg_ap->th_opcode = htons((u_short)ACK);
	msg_ap->th_block = htons((u_short)(block));
	// <--
	bsd_signal(SIGALRM, justquit);      /* just quit on timeout */
	// --> 2005/09/12 Ryan : added to support loader/firmware/mac address upgrade via Mass Production program
	//alarm(rexmtval);
	alarm(1);
	// <--
	n = recv(peer, buf, sizeof (buf), 0); /* normally times out and quits */
	alarm(0);
	if (n >= 4 &&			/* if read some data */
	    dp->th_opcode == DATA &&    /* and got a data block */
	    //block == dp->th_block) {	/* then my last ack was lost */
	    (u_short)block == dp->th_block) {	/* then my last ack was lost */
		(void) send(peer, ackbuf, 4, 0);     /* resend final ack */
	}
abort:
	return;
}

struct errmsg {
	int	e_code;
	char	*e_msg;
} errmsgs[] = {
	{ EUNDEF,	"Undefined error code" },
	{ ENOTFOUND,	"File not found" },
	{ EACCESS,	"Access violation" },
	{ ENOSPACE,	"Disk full or allocation exceeded" },
	{ EBADOP,	"Illegal TFTP operation" },
	{ EBADID,	"Unknown transfer ID" },
	{ EEXISTS,	"File already exists" },
	{ ENOUSER,	"No such user" },
	{ EOPTNEG,	"Failure to negotiate RFC2347 options" },
	{ -1,		0 }
};

/*
 * Send a nak packet (error message).
 * Error code passed in is one of the
 * standard TFTP codes, or a UNIX errno
 * offset by 100.
 */
void
nak(int error)
{
	struct tftphdr *tp;
	int length;
	struct errmsg *pe;

	tp = (struct tftphdr *)buf;
	tp->th_opcode = htons((u_short)ERROR);
	tp->th_code = htons((u_short)error);
	for (pe = errmsgs; pe->e_code >= 0; pe++)
		if (pe->e_code == error)
			break;
	if (pe->e_code < 0) {
		pe->e_msg = strerror(error - 100);
		tp->th_code = EUNDEF;   /* set 'undef' errorcode */
	}
	strcpy(tp->th_msg, pe->e_msg);
	length = strlen(pe->e_msg);
	tp->th_msg[length] = '\0';
	length += 5;
	if (send(peer, buf, length, 0) != length)
		syslog(LOG_ERR, "nak: %m");
}


#if KAM_LATER    // inetd only
int
main(int argc, char **argv)
{
	struct tftphdr *tp;
	struct passwd *pw;
	struct options *opt;
	struct sockaddr_in myaddr;
	int n = 0;
	int on = 1;
	int fd = 0;
	int pid;
	int c;
	int setrv;
	int timeout = 900;	/* Default timeout */
	char *user = "nobody";	/* Default user */

	__progname = basename(argv[0]);

	openlog(__progname, LOG_PID | LOG_NDELAY, LOG_DAEMON);

	while ((c = getopt(argc, argv, "csu:r:t:")) != -1)
		switch (c) {
		case 'c':
			cancreate = 1;
			break;
		case 's':
			secure = 1;
			break;
		case 't':
		  timeout = atoi(optarg);
		  break;
		case 'u':
		  user = optarg;
		  break;
		case 'r':
		  for ( opt = options ; opt->o_opt ; opt++ ) {
		    if ( !strcasecmp(optarg, opt->o_opt) ) {
		      opt->o_opt = ""; /* Don't support this option */
		      break;
		    }
		  }
		  if ( !opt->o_opt ) {
		        syslog(LOG_ERR, "Unknown option: %s", optarg);
			exit(1);
		  }
		  break;

		default:
			usage();
			break;
		}

	for (; optind != argc; optind++) {
		if (dirs)
			dirs = realloc(dirs, (ndirs+2) * sizeof (char *));
		else
			dirs = calloc(ndirs+2, sizeof(char *));
		if (dirs == NULL) {
			syslog(LOG_ERR, "malloc: %m");
			exit(1);
		}			
		dirs[n++] = argv[optind];
		dirs[n] = NULL;
		ndirs++;
	}

	if (secure) {
		if (ndirs == 0) {
			syslog(LOG_ERR, "no -s directory");
			exit(1);
		}
		if (ndirs > 1) {
			syslog(LOG_ERR, "too many -s directories");
			exit(1);
		}
		if (chdir(dirs[0])) {
			syslog(LOG_ERR, "%s: %m", dirs[0]);
			exit(1);
		}
	}

	pw = getpwnam(user);
	if (!pw) {
		syslog(LOG_ERR, "no user %s: %m", user);
		exit(1);
	}

	if (ioctl(fd, FIONBIO, &on) < 0) {
		syslog(LOG_ERR, "ioctl(FIONBIO): %m");
		exit(1);
	}

	/* This means we don't want to wait() for children */
	bsd_signal(SIGCHLD, SIG_IGN);

	do {
	  fd_set readset;
	  struct timeval tv_timeout;
	  

	  FD_ZERO(&readset);
	  FD_SET(fd, &readset);
	  tv_timeout.tv_sec = timeout;
	  tv_timeout.tv_usec = 0;

	  /* when fired up by inetd or xinetd, descriptors 0, 1 & 2 are
	     sockets to the client */
	  if ( select(fd+1, &readset, NULL, NULL, &tv_timeout) == 0 )
	    exit(0);		/* Timeout, return to inetd */

	  fromlen = sizeof (from);
	  n = myrecvfrom(fd, buf, sizeof (buf), 0,
			 (struct sockaddr *)&from, &fromlen,
			 &myaddr);

	  if (n < 0) {
	    syslog(LOG_ERR, "recvfrom: %m");
	    exit(1);
	  }

#ifdef HAVE_TCPWRAPPERS
	/* Verify if this was a legal request for us.  This has to be
	   done before the chroot, while /etc is still accessible. */
	  request_init(&wrap_request,
		       RQ_DAEMON, __progname,
		       RQ_FILE, fd,
		       RQ_CLIENT_SIN, &from,
		       RQ_SERVER_SIN, &myaddr,
		       0);
	  sock_methods(&wrap_request);
	  if ( hosts_access(&wrap_request) == 0 ) {
	    if ( deny_severity != -1 )
	      syslog(deny_severity, "connection refused from %s",
		     inet_ntoa(from.sin_addr));
	    exit(1);		/* Access denied */
	  } else if ( allow_severity != -1 ) {
	    syslog(allow_severity, "connect from %s",
		   inet_ntoa(from.sin_addr));
	  }
#endif

	/*
	 * Now that we have read the message out of the UDP
	 * socket, we fork and go back to listening to the
	 * socket.
	 */
	  pid = fork();
	  if (pid < 0) {
	    syslog(LOG_ERR, "fork: %m");
	    exit(1);		/* Return to inetd, just in case */
	  }
	} while ( pid > 0 );	/* Parent process continues... */

	/* Child process: handle the actual request here */

	/* Chroot and drop privileges */

	if (secure && chroot(".")) {
		syslog(LOG_ERR, "chroot: %m");
		exit(1);
	}

#ifdef HAVE_SETREGID
	setrv = setregid(pw->pw_gid, pw->pw_gid);
#else
	setrv = setegid(pw->pw_gid) || setgid(pw->pw_gid);
#endif

#ifdef HAVE_SETREUID
	setrv = setrv || setreuid(pw->pw_uid, pw->pw_uid);
#else
	/* Important: setuid() must come first */
	setrv = setrv || setuid(pw->pw_uid) ||
	  (geteuid() != pw->pw_uid && seteuid(pw->pw_uid));
#endif

	if ( setrv ) {
	  syslog(LOG_ERR, "cannot drop privileges: %m");
	  exit(1);
	}

	/* Close file descriptors we don't need */

	from.sin_family = AF_INET;
	alarm(0);
	close(fd);
	close(1);

	/* Process the request... */

	peer = socket(AF_INET, SOCK_DGRAM, 0);
	if (peer < 0) {
		syslog(LOG_ERR, "socket: %m");
		exit(1);
	}
	myaddr.sin_port = htons(0); /* We want a new local port */
	if (bind(peer, (struct sockaddr *)&myaddr, sizeof (myaddr)) < 0) {
		syslog(LOG_ERR, "bind: %m");
		exit(1);
	}
	if (connect(peer, (struct sockaddr *)&from, sizeof(from)) < 0) {
		syslog(LOG_ERR, "connect: %m");
		exit(1);
	}
	tp = (struct tftphdr *)buf;
	tp->th_opcode = ntohs(tp->th_opcode);
	if (tp->th_opcode == RRQ || tp->th_opcode == WRQ)
		tftp(tp, n);
	exit(0);
}
#endif

