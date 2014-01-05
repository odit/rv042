/*
 * dnsr_vx.h - DNS relay vxWorks porting specific module. It includes 
 * vxWorks missing interfaces that are found in unix/linux environment.
 *
 * Copyright 2003, Broadcom Corporation
 * All Rights Reserved.                
 *                                     
 *
 * $Id: dnsr_vx.h 13 2007-04-09 06:18:00Z tt $
 */

#ifndef __dnsr_vx_h__
#define __dnsr_vx_h__

/* logging utilities */
#include <logLib.h>

#define LOG_DEBUG	0
#define LOG_INFO	1
#define LOG_WARNING	2
#define LOG_ERR	3
#define LOG_CRIT	4

#define openlog(ident, option, facility)
int syslog(int level, char *format, ...);
#define closelog()

/* inet utilities */
#define INADDRSZ	sizeof(struct in_addr)
#define INET_ADDRSTRLEN	16
#define IF_NAMESIZE IFNAMSIZ
#define socklen_t int

const char* inet_ntop(int af, const void *src, char *dst, size_t size);
int inet_pton(int af, const char *src, void *dst);

/* time utilities */
#include "sys/times.h"

/* defined in src/vxWorks/target/config/bcm47xx/router/unix.c */
int gettimeofday(struct timeval *tv, struct timezone *tz);
int settimeofday(const struct timeval *tv , const struct timezone *tz);

/* task utilities */
#include <taskLib.h>

#define getpid taskIdSelf
#define die dnsr_die

#endif	/* #ifndef __log_vx_h__ */
