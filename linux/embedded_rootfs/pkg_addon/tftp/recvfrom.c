/* $Id: recvfrom.c 265 2007-05-07 14:28:57Z tt $ */
/* ----------------------------------------------------------------------- *
 *   
 *   Copyright 2001 H. Peter Anvin - All Rights Reserved
 *
 *   This program is free software available under the same license
 *   as the "OpenBSD" operating system, distributed at
 *   http://www.openbsd.org/.
 *
 * ----------------------------------------------------------------------- */

/*
 * recvfrom.c
 *
 * Emulate recvfrom() using recvmsg(), but try to capture the local address
 * since some TFTP clients consider it an error to get the reply from another
 * IP address than the request was sent to.
 *
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "../config.h"
#include "recvfrom.h"

// both flags are ON in config.h
#if defined(HAVE_RECVMSG) && defined(HAVE_MSGHDR_MSG_CONTROL)

#include <sys/uio.h>

#ifdef IP_PKTINFO
# ifndef HAVE_STRUCT_IN_PKTINFO
#  ifdef __linux__
/* Assume this version of glibc simply lacks the definition */
struct in_pktinfo {
  int ipi_ifindex;
  struct in_addr ipi_spec_dst;
  struct in_addr ipi_addr;
};
#  else
#   undef IP_PKTINFO		/* No definition, no way to get it */
#  endif
# endif
#endif

#ifndef CMSG_LEN
# define CMSG_LEN(size)	 (sizeof(struct cmsghdr) + (size))
#endif
#ifndef CMSG_SPACE
# define CMSG_SPACE(size) (sizeof(struct cmsghdr) + (size))
#endif

int
myrecvfrom(int s, void *buf, int len, unsigned int flags,
	   struct sockaddr *from, int *fromlen,
	   struct sockaddr_in *myaddr)
{
  struct msghdr msg;
  struct iovec iov[1];
  int n;
  struct cmsghdr *cmptr;
  union {
    struct cmsghdr cm;
#ifdef IP_PKTINFO
    char control[CMSG_SPACE(sizeof(struct in_addr)) +
		CMSG_SPACE(sizeof(struct in_pktinfo))];
#else
    char control[CMSG_SPACE(sizeof(struct in_addr))];
#endif
  } control_un;
  int on = 1;
#ifdef IP_PKTINFO
  struct in_pktinfo pktinfo;
#endif

  /* Try to enable getting the return address */
#ifdef IP_RECVDSTADDR
  setsockopt(s, IPPROTO_IP, IP_RECVDSTADDR, &on, sizeof(on));
#endif
#ifdef IP_PKTINFO
  setsockopt(s, IPPROTO_IP, IP_PKTINFO, &on, sizeof(on));
#endif

  msg.msg_control = control_un.control;
  msg.msg_controllen = sizeof(control_un.control);
  msg.msg_flags = 0;

  msg.msg_name = from;
  msg.msg_namelen = *fromlen;
  iov[0].iov_base = buf;
  iov[0].iov_len  = len;
  msg.msg_iov = iov;
  msg.msg_iovlen = 1;

  if ( (n = recvmsg(s, &msg, flags)) < 0 )
    return n;			/* Error */

  *fromlen = msg.msg_namelen;

  if ( myaddr ) {
    bzero(myaddr, sizeof(struct sockaddr_in));
    myaddr->sin_family = AF_INET;

    if ( msg.msg_controllen < sizeof(struct cmsghdr) ||
	 (msg.msg_flags & MSG_CTRUNC) )
      return n;			/* No information available */

    for ( cmptr = CMSG_FIRSTHDR(&msg) ; cmptr != NULL ;
	  cmptr = CMSG_NXTHDR(&msg, cmptr) ) {

#ifdef IP_RECVSTDADDR
      if ( cmptr->cmsg_level == IPPROTO_IP &&
	   cmptr->cmsg_type == IP_RECVDSTADDR ) {
	memcpy(&myaddr->sin_addr, CMSG_DATA(cmptr),
	       sizeof(struct in_addr));
      }
#endif

#ifdef IP_PKTINFO
      if ( cmptr->cmsg_level == IPPROTO_IP &&
	   cmptr->cmsg_type == IP_PKTINFO ) {
	memcpy(&pktinfo, CMSG_DATA(cmptr), sizeof(struct in_pktinfo));
	memcpy(&myaddr->sin_addr, &pktinfo.ipi_addr, sizeof(struct in_addr));
      }
#endif

    }
  }

  return n;
}

#else /* pointless... */

int
myrecvfrom(int s, void *buf, int len, unsigned int flags,
	   struct sockaddr *from, int *fromlen,
	   struct sockaddr_in *myaddr)
{
  /* There is no way we can get the local address, fudge it */

  bzero(myaddr, sizeof(struct sockaddr_in));
  myaddr->sin_family = AF_INET;

  myaddr->sin_port   = htons(IPPORT_TFTP);
  bzero(&myaddr->sin_addr, sizeof(myaddr->sin_addr));

  return recvfrom(s,buf,len,flags,from,fromlen);
}

#endif
