/* $Id: recvfrom.h 265 2007-05-07 14:28:57Z tt $ */
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
 * recvfrom.h
 *
 * Header for recvfrom substitute
 *
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>

int
myrecvfrom(int s, void *buf, int len, unsigned int flags,
	   struct sockaddr *from, int *fromlen,
	   struct sockaddr_in *myaddr);
