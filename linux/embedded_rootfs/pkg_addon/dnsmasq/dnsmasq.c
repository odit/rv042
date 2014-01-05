/* dnsmasq is Copyright (c) 2000 Simon Kelley

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 dated June, 1991.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
*/

/* See RFC1035 for details of the protocol this code talks. */

/* Author's email: simon@thekelleys.org.uk */

#include "dnsmasq.h"
#include <nkutil.h>

#ifdef CONFIG_NK_LOCAL_DNS_DATABASE
extern void load_local_dns_db(void);
#endif

static int sighup, sigusr1;

#if defined(vxworks)
#define SHUTDOWN_MESSAGE_LEN	8
#define SHUTDOWN_UDPPORT	0x8888
static int shutdownfd = -1;
#endif

static void sig_handler(int sig)
{
  if (sig == SIGHUP)
    sighup = 1;
  else if (sig == SIGUSR1)
    sigusr1 = 1;
#ifdef vxworks
  else if (sig == SIGTERM) {
	if (shutdownfd != -1)
	{
		struct sockaddr_in sockaddr;
		char message[SHUTDOWN_MESSAGE_LEN];
		memset(&sockaddr, 0, sizeof(sockaddr));
		sockaddr.sin_len = sizeof(sockaddr);
		sockaddr.sin_family=AF_INET;
		sockaddr.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
		sockaddr.sin_port=htons(SHUTDOWN_UDPPORT);
		sendto(shutdownfd, message, sizeof(message), 0, &sockaddr, sizeof(sockaddr));
	}
  }
#endif
}

#if defined(vxworks)
/*
* default parameters - for debugging only
*/
static char *dnsr_default_argv[] =
{
	"dnsr_main",
	"-q",
	"-d",
	"-i",
	"mirror0",
	"-a",
	"192.168.1.1",
	"-S",
	"10.19.13.12",
	"-S",
	"10.19.13.13",
	"-s",
	"sj.broadcom.com",
};
static int dnsr_default_argc = sizeof(dnsr_default_argv)/sizeof(dnsr_default_argv[0]);
/*
* tDNSR task main entry.
*/
int dnsr_main (int argc, char **argv)
#else
int main (int argc, char **argv)
#endif
{
  int i;
  int cachesize = CACHESIZ;
  int port = NAMESERVER_PORT;
  unsigned int options;
  int first_loop = 1;
#ifdef HAVE_FILE_SYSTEM
  int logged_resolv = 0, logged_lease = 0; 
  char *resolv = RESOLVFILE;
  char *runfile = RUNFILE;
  time_t resolv_changed = 0;
  char *lease_file = NULL;
  off_t lease_file_size = (off_t)0;
  ino_t lease_file_inode = (ino_t)0;
#endif
  struct irec *iface;
  int peerfd, peerfd6;
  struct irec *interfaces = NULL;
  char *mxname = NULL;
  char *mxtarget = NULL;
  char *domain_suffix = NULL;
  char *username = CHUSER;
  struct iname *if_names = NULL;
  struct iname *if_addrs = NULL;
  struct server *serv_addrs = NULL;
  char *dnamebuff, *packet;
  struct server *servers, *last_server;
 
  sighup = 1; /* init cache the first time through */
  sigusr1 = 0; /* but don't dump */
  signal(SIGUSR1, sig_handler);
  signal(SIGHUP, sig_handler);
#ifdef vxworks
  signal(SIGTERM, sig_handler);
#endif
#ifdef CONFIG_NK_LOCAL_DNS_DATABASE
  signal(SIGUSR2, load_local_dns_db);
#endif

  /* These get allocated here to avoid overflowing the small stack
     on embedded systems. dnamebuff is big enough to hold one
     maximal sixed domain name and gets passed into all the processing
     code. We manage to get away with one buffer. */
  dnamebuff = safe_malloc(MAXDNAME);
  /* Size: we check after adding each record, so there must be 
     memory for the largest packet, and the largest record */
  packet = safe_malloc(PACKETSZ+MAXDNAME+RRFIXEDSZ);

#if defined(vxworks)
	if (argc == 0)
	{
		argc = dnsr_default_argc;
		argv = dnsr_default_argv;
	}
#endif
#ifdef HAVE_FILE_SYSTEM
  options = read_opts(argc, argv, dnamebuff, &resolv, &mxname, &mxtarget, &lease_file,
		      &username, &domain_suffix, &runfile, &if_names, &if_addrs, 
		      &serv_addrs, &cachesize, &port) ;
#else
  options = read_opts(argc, argv, dnamebuff, NULL, &mxname, &mxtarget, NULL,
		      &username, &domain_suffix, NULL, &if_names, &if_addrs, 
		      &serv_addrs, &cachesize, &port) ;
#endif
  
  /* peerfd is not bound to a low port
     so that we can send queries out on it without them getting
     blocked at firewalls */
  
  if ((peerfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1 && 
      errno != EAFNOSUPPORT &&
      errno != EINVAL)
    die("dnsmasq: cannot create socket: %s", NULL);
  
#ifdef HAVE_IPV6
  if ((peerfd6 = socket(AF_INET6, SOCK_DGRAM, 0)) == -1 && 
      errno != EAFNOSUPPORT &&
      errno != EINVAL)
    die("dnsmasq: cannot create IPv6 socket: %s", NULL);
#else
  peerfd6 = -1;
#endif
  
  if (peerfd == -1 && peerfd6 == -1)
    die("dnsmasq: no kernel support for IPv4 _or_ IPv6.", NULL);
#if defined(vxworks)
  /*
  * Create a socket to listen to shutdown message. This is a must
  * do on vxworks since posix kill(SIGINT) does not work if a task
  * is in PEND mode (select puts task in this mode).
  */
  else
  {
	struct sockaddr_in sockaddr;
	memset(&sockaddr, 0, sizeof(sockaddr));
	sockaddr.sin_len=sizeof(sockaddr);
	sockaddr.sin_family=AF_INET;
	sockaddr.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
	sockaddr.sin_port=htons(SHUTDOWN_UDPPORT);
	if ((shutdownfd = socket(sockaddr.sin_family, SOCK_DGRAM, 0)) == -1)
		die("dnsmasq: cannot create socket: %s", NULL);
	else if (bind(shutdownfd, &sockaddr, sa_len(&sockaddr)) == -1)
		die("dnsmasq: cannot bind socket: %s", NULL);
  }
#endif

  interfaces = find_all_interfaces(if_names, if_addrs, port);
  
  /* open a socket bound to NS port on each local interface.
     this is necessary to ensure that our replies originate from
     the address they were sent to. See Stevens page 531 */
  for (iface = interfaces; iface; iface = iface->next)
    {
      if ((iface->fd = socket(iface->addr.sa.sa_family, SOCK_DGRAM, 0)) == -1)
	die("cannot create socket: %s", NULL);
	      
      if (bind(iface->fd, &iface->addr.sa, sa_len(&iface->addr)))
	die("bind failed: %s", NULL);
    }

#ifdef CONFIG_NK_LOCAL_DNS_DATABASE
  load_local_dns_db();
#endif

  forward_init(1);

  cache_init(cachesize, options & OPT_LOG);
  
  setbuf(stdout, NULL);

#ifdef HAVE_FILE_SYSTEM
  if (!(options & OPT_DEBUG))
    {
      FILE *pidfile;
      struct passwd *ent_pw;
        
      /* The following code "daemonizes" the process. 
	 See Stevens section 12.4 */

#ifdef HAVE_FORK
      if (fork() != 0 )
	exit(0);
      
      setsid();
      
      if (fork() != 0)
	exit(0);
#endif
      
      chdir("/");
      umask(022); /* make pidfile 0644 */
      
      /* write pidfile _after_ forking ! */
      if (runfile && (pidfile = fopen(runfile, "w")))
      	{
	  fprintf(pidfile, "%d\n", (int) getpid());
	  fclose(pidfile);
	}
      
      umask(0);

      for (i=0; i<64; i++)
	{
	  if (i == peerfd || i == peerfd6)
	    continue;
	  for (iface = interfaces; iface; iface = iface->next)
	    if (iface->fd == i)
	      break;
	  if (!iface)
	    close(i);
	}

      /* Change uid and gid for security */
      if (username && (ent_pw = getpwnam(username)))
	{
	  gid_t dummy;
	  struct group *gp;
	  /* remove all supplimentary groups */
	  setgroups(0, &dummy);
	  /* change group to "dip" if it exists, for /etc/ppp/resolv.conf 
	     otherwise get the group for "nobody" */
	  if ((gp = getgrnam("dip")) || (gp = getgrgid(ent_pw->pw_gid)))
	    setgid(gp->gr_gid); 
	  /* finally drop root */
	  setuid(ent_pw->pw_uid);
	}
    }
#else
#if defined(vxworks)
#endif
#endif

  /* In debug mode, log to stderr too and cut the prefix crap. */
  openlog("dnsmasq", options & OPT_DEBUG ? LOG_PERROR : LOG_PID, LOG_DAEMON);
  
  if (cachesize)
    syslog(LOG_INFO, "started, version %s cachesize %d", VERSION, cachesize);
  else
    syslog(LOG_INFO, "started, version %s cache disabled", VERSION);
  
  if (mxname)
    syslog(LOG_INFO, "serving MX record for mailhost %s target %s", 
	   mxname, mxtarget);
  
#if defined(vxworks) 
#else
  if (getuid() == 0 || geteuid() == 0)
    syslog(LOG_WARNING, "failed to drop root privs");
#endif
  
  serv_addrs = servers = last_server = check_servers(serv_addrs, interfaces, peerfd, peerfd6);
  
  while (1)
    {
      int ready, maxfd = peerfd > peerfd6 ? peerfd : peerfd6;
      fd_set rset;
      HEADER *header;
#ifdef HAVE_FILE_SYSTEM
      struct stat statbuf;
#endif
   
      if (first_loop)
	/* do init stuff only first time round. */
	{
	  first_loop = 0;
	  ready = 0;
	}
      else
	{
	  FD_ZERO(&rset);

	  if (peerfd != -1)
	    FD_SET(peerfd, &rset);
	  if (peerfd6 != -1)
	    FD_SET(peerfd6, &rset);
#if defined(vxworks)
	  if (shutdownfd != -1)
	  {
		FD_SET(shutdownfd, &rset);
		if (shutdownfd > maxfd)
		  maxfd = shutdownfd;
	  }
#endif
	    	  
	  for (iface = interfaces; iface; iface = iface->next)
	    {
	      FD_SET(iface->fd, &rset);
	      if (iface->fd > maxfd)
		maxfd = iface->fd;
	    }
	  
	  ready = select(maxfd+1, &rset, NULL, NULL, NULL);
	  
	  if (ready == -1)
	    {
	      if (errno == EINTR)
		ready = 0; /* do signal handlers */
	      else
		continue;
	    }
	}

#ifdef HAVE_FILE_SYSTEM
      if (sighup)
	{
	  signal(SIGHUP, SIG_IGN);
	  cache_reload(options & OPT_NO_HOSTS, dnamebuff);
	  if (resolv && (options & OPT_NO_POLL))
	    servers = last_server = 
	      check_servers(reload_servers(resolv, dnamebuff, servers), 
			    interfaces, peerfd, peerfd6);
	  sighup = 0;
	  signal(SIGHUP, sig_handler);
	}

      if (sigusr1)
	{
	  signal(SIGUSR1, SIG_IGN);
	  dump_cache(options & (OPT_DEBUG | OPT_LOG), cachesize);
	  sigusr1 = 0;
	  signal(SIGUSR1, sig_handler);
	}

      if (resolv && !(options & OPT_NO_POLL))
	{
	  if (stat(resolv, &statbuf) == -1)
	    {
	      if (!logged_resolv)
		syslog(LOG_WARNING, "failed to access %s: %m", resolv);
	      logged_resolv = 1;
	    }
	  else
	    {
	      logged_resolv = 0;
	      if ((statbuf.st_mtime > resolv_changed) &&
		  (statbuf.st_mtime < time(NULL) || resolv_changed == 0))
		{
		  resolv_changed = statbuf.st_mtime;
		  servers = last_server = 
		    check_servers(reload_servers(resolv, dnamebuff, servers),
				  interfaces, peerfd, peerfd6);
		}
	    }
	}
#else
#if defined(vxworks)
      if (sighup)
	{
	  cache_reload(options & OPT_NO_HOSTS, dnamebuff);
 	  sighup = 0;
 	}
#endif
#endif

#ifdef HAVE_FILE_SYSTEM
      if (lease_file)
	{
	  if (stat(lease_file, &statbuf) == -1)
	    {
	      if (!logged_lease)
		syslog(LOG_WARNING, "failed to access %s: %m", lease_file);
	      logged_lease = 1;
	    }
	  else
	    { 
	      logged_lease = 0;
	      if ((lease_file_size == (off_t)0) ||
		  (statbuf.st_size > lease_file_size) ||
		  (statbuf.st_ino != lease_file_inode))
		{
		  lease_file_size = statbuf.st_size;
		  lease_file_inode = statbuf.st_ino;
		  load_dhcp(lease_file, domain_suffix, time(NULL), dnamebuff);
		}
	    }
	}
#else
#if defined(vxworks)
      load_dhcp(NULL, domain_suffix, time(NULL), dnamebuff);
#endif
#endif

      if (ready == 0)
	continue; /* no sockets ready */
      
      if (peerfd != -1 && FD_ISSET(peerfd, &rset))
	last_server = reply_query(peerfd, packet, dnamebuff, last_server);
      if (peerfd6 != -1 && FD_ISSET(peerfd6, &rset))
	last_server = reply_query(peerfd6, packet, dnamebuff, last_server);
#if defined(vxworks)
	if (shutdownfd != -1 && FD_ISSET(shutdownfd, &rset))
	{
		for (; interfaces != NULL; )
		{
			struct irec *next = interfaces->next;
			close(interfaces->fd);
			safe_free(interfaces);
			interfaces = next;
		}
		if (mxname != NULL)
		{
			safe_free(mxname);
		}
		mxname = NULL;
		if (mxtarget != NULL)
		{
			safe_free(mxtarget);
		}
		mxtarget = NULL;
		if (domain_suffix != NULL)
		{
			safe_free(domain_suffix);
		}
		domain_suffix = NULL;
		for (; if_names != NULL; )
		{
			struct iname *next = if_names->next;
			safe_free(if_names->name);
			safe_free(if_names);
			if_names = next;
		}
		if_names = NULL;
		for (; if_addrs != NULL; )
		{
			struct iname *next = if_addrs->next;
			safe_free(if_addrs);
			if_addrs = next;
		}
		if_addrs = NULL;
		for (; serv_addrs != NULL; )
		{
			struct server *next = serv_addrs->next;
			if (serv_addrs->domain != NULL)
			{
				safe_free(serv_addrs->domain);
			}
			safe_free(serv_addrs);
			serv_addrs = next;
		}
		serv_addrs = NULL;
		if (dnamebuff != NULL)
		{
			safe_free(dnamebuff);
		}
		dnamebuff = NULL;
		if (packet != NULL)
		{
			safe_free(packet);
		}
		packet = NULL;
		forward_clean();
		cache_clean();
		if (peerfd != -1)
		{
			close(peerfd);
		}
		peerfd = -1;
#ifdef HAVE_IPV6
		if (peerfd6 != -1)
		{
			close(peerfd6);
		}
		peerfd6 = -1;
#endif
		if (shutdownfd != -1)
		{
			close(shutdownfd);
		}
		shutdownfd = -1;
		break;
	}
#endif

      for (iface = interfaces; iface; iface = iface->next)
	{
	  if (FD_ISSET(iface->fd, &rset))
	    {
	      /* request packet, deal with query */
	      union mysockaddr udpaddr;
	      socklen_t udplen = sizeof(udpaddr);
	      int m, n = recvfrom(iface->fd, packet, PACKETSZ, 0, &udpaddr.sa, &udplen); 
#if defined(vxworks)
#ifdef HAVE_SOCKADDR_SA_LEN
	      udpaddr.sa.sa_len = sizeof(udpaddr.sa);
#endif
#endif
	      udpaddr.sa.sa_family = iface->addr.sa.sa_family;
#ifdef HAVE_IPV6
	      if (udpaddr.sa.sa_family == AF_INET6)
		udpaddr.in6.sin6_flowinfo = htonl(0);
#endif	      
	      header = (HEADER *)packet;
	      if (n >= (int)sizeof(HEADER) && !header->qr)
		{
		  m = answer_request (header, ((char *) header) + PACKETSZ, (unsigned int)n, 
				      mxname, mxtarget, options, dnamebuff);
		  if (m >= 1)
		    {
		      /* answered from cache, send reply */
		      sendto(iface->fd, (char *)header, m, 0, 
			     &udpaddr.sa, sa_len(&udpaddr));
		    }
		  else 
		    {
		      /* cannot answer from cache, send on to real nameserver */
		      last_server = forward_query(iface->fd, peerfd, peerfd6, &udpaddr, header, n, 
						  options && OPT_ORDER, dnamebuff,
						  servers, last_server);
		    }
		}
	      
	    }
	}
    }
  
  return 0;
}
