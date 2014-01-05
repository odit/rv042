/* config.h.  Generated automatically by configure.  */
/* $Id: config.h 265 2007-05-07 14:28:57Z tt $ */
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
 * config.h.in
 *
 * Pattern file for configurations
 */

#define HAVE_SIGSETJMP 1                  // in tftpd.c, tftp.c main.c
#define HAVE_MSGHDR_MSG_CONTROL 1         // in recvfrom.c only
#define HAVE_RECVMSG 1                    // in recvfrom.c only
//#define HAVE_TCPWRAPPERS 1                // in tftpd.c only
#define HAVE_STRUCT_IN_PKTINFO 1          // in recvfrom.c only
#define HAVE_SETREUID 1                   // in tftpd.c only
#define HAVE_SETREGID 1                   // in tftpd.c only
