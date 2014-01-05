# Copyright (C) 2004-2008  Internet Systems Consortium, Inc. ("ISC")
# Copyright (C) 1998-2002  Internet Software Consortium.
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH
# REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
# AND FITNESS.  IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT,
# INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
# LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
# OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
# PERFORMANCE OF THIS SOFTWARE.

# $Id: Makefile.in,v 1.96.130.4 2008/03/31 05:06:47 marka Exp $

srcdir =	.

top_srcdir =	../..

VERSION=9.5.0-P2

# Copyright (C) 2004, 2005, 2007  Internet Systems Consortium, Inc. ("ISC")
# Copyright (C) 1999-2001  Internet Software Consortium.
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH
# REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
# AND FITNESS.  IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT,
# INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
# LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
# OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
# PERFORMANCE OF THIS SOFTWARE.

# $Id: includes.in,v 1.21 2007/06/19 23:47:24 tbox Exp $

# Search for machine-generated header files in the build tree,
# and for normal headers in the source tree (${top_srcdir}).
# We only need to look in OS-specific subdirectories for the
# latter case, because there are no machine-generated OS-specific
# headers.

ISC_INCLUDES = -I/home/gavin/CA2/linux/embedded_rootfs/pkg_addon/bind-9.5.0-P2/lib/isc/include \
	-I${top_srcdir}/lib/isc \
	-I${top_srcdir}/lib/isc/include \
	-I${top_srcdir}/lib/isc/unix/include \
	-I${top_srcdir}/lib/isc/nothreads/include \
	-I${top_srcdir}/lib/isc/mips/include

ISCCC_INCLUDES = -I/home/gavin/CA2/linux/embedded_rootfs/pkg_addon/bind-9.5.0-P2/lib/isccc/include \
       -I${top_srcdir}/lib/isccc/include

ISCCFG_INCLUDES = -I/home/gavin/CA2/linux/embedded_rootfs/pkg_addon/bind-9.5.0-P2/lib/isccfg/include \
       -I${top_srcdir}/lib/isccfg/include

DNS_INCLUDES = -I/home/gavin/CA2/linux/embedded_rootfs/pkg_addon/bind-9.5.0-P2/lib/dns/include \
	-I${top_srcdir}/lib/dns/include

LWRES_INCLUDES = -I/home/gavin/CA2/linux/embedded_rootfs/pkg_addon/bind-9.5.0-P2/lib/lwres/include \
	-I${top_srcdir}/lib/lwres/unix/include \
	-I${top_srcdir}/lib/lwres/include

BIND9_INCLUDES = -I/home/gavin/CA2/linux/embedded_rootfs/pkg_addon/bind-9.5.0-P2/lib/bind9/include \
	-I${top_srcdir}/lib/bind9/include

TEST_INCLUDES = \
	-I${top_srcdir}/lib/tests/include

#
# Add database drivers here.
#
DBDRIVER_OBJS =
DBDRIVER_SRCS =
DBDRIVER_INCLUDES =
DBDRIVER_LIBS =

DLZ_DRIVER_DIR =	${top_srcdir}/contrib/dlz/drivers

DLZDRIVER_OBJS =	
DLZDRIVER_SRCS =	
DLZDRIVER_INCLUDES =	
DLZDRIVER_LIBS =	

CINCLUDES =	-I${srcdir}/include -I${srcdir}/unix/include -I. \
		${LWRES_INCLUDES} ${DNS_INCLUDES} ${BIND9_INCLUDES} \
		${ISCCFG_INCLUDES} ${ISCCC_INCLUDES} ${ISC_INCLUDES} \
		${DLZDRIVER_INCLUDES} ${DBDRIVER_INCLUDES}

CDEFINES =      

CWARNINGS =

DNSLIBS =	../../lib/dns/libdns.a 
ISCCFGLIBS =	../../lib/isccfg/libisccfg.a
ISCCCLIBS =	../../lib/isccc/libisccc.a
ISCLIBS =	../../lib/isc/libisc.a
LWRESLIBS =	../../lib/lwres/liblwres.a
BIND9LIBS =	../../lib/bind9/libbind9.a

DNSDEPLIBS =	../../lib/dns/libdns.a
ISCCFGDEPLIBS =	../../lib/isccfg/libisccfg.a
ISCCCDEPLIBS =	../../lib/isccc/libisccc.a
ISCDEPLIBS =	../../lib/isc/libisc.a
LWRESDEPLIBS =	../../lib/lwres/liblwres.a
BIND9DEPLIBS =	../../lib/bind9/libbind9.a

DEPLIBS =	${LWRESDEPLIBS} ${DNSDEPLIBS} ${BIND9DEPLIBS} \
		${ISCCFGDEPLIBS} ${ISCCCDEPLIBS} ${ISCDEPLIBS}

LIBS =		${LWRESLIBS} ${DNSLIBS} ${BIND9LIBS} \
		${ISCCFGLIBS} ${ISCCCLIBS} ${ISCLIBS} \
		${DLZDRIVER_LIBS} ${DBDRIVER_LIBS} -lnsl 

SUBDIRS =	unix

TARGETS =	named lwresd

OBJS =		builtin.o client.o config.o control.o \
		controlconf.o interfacemgr.o \
		listenlist.o log.o logconf.o main.o notify.o \
		query.o server.o sortlist.o statschannel.o \
		tkeyconf.o tsigconf.o update.o xfrout.o \
		zoneconf.o \
		lwaddr.o lwresd.o lwdclient.o lwderror.o lwdgabn.o \
		lwdgnba.o lwdgrbn.o lwdnoop.o lwsearch.o \
		${DLZDRIVER_OBJS} ${DBDRIVER_OBJS}

UOBJS =		unix/os.o

SRCS =		builtin.c client.c config.c control.c \
		controlconf.c interfacemgr.c \
		listenlist.c log.c logconf.c main.c notify.c \
		query.c server.c sortlist.c statschannel.c \
		tkeyconf.c tsigconf.c update.c xfrout.c \
		zoneconf.c \
		lwaddr.c lwresd.c lwdclient.c lwderror.c lwdgabn.c \
		lwdgnba.c lwdgrbn.c lwdnoop.c lwsearch.c \
		${DLZDRIVER_SRCS} ${DBDRIVER_SRCS}

MANPAGES =	named.8 lwresd.8 named.conf.5

HTMLPAGES =	named.html lwresd.html named.conf.html

MANOBJS =	${MANPAGES} ${HTMLPAGES}

# Copyright (C) 2004-2008  Internet Systems Consortium, Inc. ("ISC")
# Copyright (C) 1998-2003  Internet Software Consortium.
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH
# REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
# AND FITNESS.  IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT,
# INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
# LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
# OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
# PERFORMANCE OF THIS SOFTWARE.

# $Id: rules.in,v 1.62.128.2 2008/02/18 23:46:30 tbox Exp $

###
### Common Makefile rules for BIND 9.
###

###
### Paths
###
### Note: paths that vary by Makefile MUST NOT be listed
### here, or they won't get expanded correctly.

prefix =	/tmp/root-rootfs
exec_prefix =	${prefix}
bindir =	${exec_prefix}/bin
sbindir =	${exec_prefix}/sbin
includedir =	/home/gavin/CA2/linux/embedded_rootfs/pkg_addon/bind-9.5.0-P2/include-tmp
libdir =	/home/gavin/CA2/linux/embedded_rootfs/pkg_addon/bind-9.5.0-P2/lib-tmp
sysconfdir =	/etc
localstatedir =	/var
mandir =	/home/gavin/CA2/linux/embedded_rootfs/pkg_addon/bind-9.5.0-P2/man-tmp

DESTDIR =



top_builddir =	/home/gavin/CA2/linux/embedded_rootfs/pkg_addon/bind-9.5.0-P2

###
### All
###
### Makefile may define:
###	TARGETS

all: subdirs ${TARGETS}

###
### Subdirectories
###
### Makefile may define:
###	SUBDIRS

ALL_SUBDIRS = ${SUBDIRS} nulldir

#
# We use a single-colon rule so that additional dependencies of
# subdirectories can be specified after the inclusion of this file.
# The "depend" target is treated the same way.
#
subdirs:
	@for i in ${ALL_SUBDIRS}; do \
		if [ "$$i" != "nulldir" -a -d $$i ]; then \
			echo "making all in `pwd`/$$i"; \
			(cd $$i; ${MAKE} ${MAKEDEFS} DESTDIR="${DESTDIR}" all) || exit 1; \
		fi; \
	done

install:: all

install clean distclean maintainer-clean doc docclean man manclean::
	@for i in ${ALL_SUBDIRS}; do \
		if [ "$$i" != "nulldir" -a -d $$i ]; then \
			echo "making $@ in `pwd`/$$i"; \
			(cd $$i; ${MAKE} ${MAKEDEFS} DESTDIR="${DESTDIR}" $@) || exit 1; \
		fi; \
	done

###
### C Programs
###
### Makefile must define
###	CC
### Makefile may define
###	CFLAGS
###	LDFLAGS
###	CINCLUDES
###	CDEFINES
###	CWARNINGS
### User may define externally
###     EXT_CFLAGS

CC = 		mips64-octeon-linux-gnu-gcc
CFLAGS =	-s -O2
LDFLAGS =	
STD_CINCLUDES =	
STD_CDEFINES =	
STD_CWARNINGS =	 -W -Wall -Wmissing-prototypes -Wcast-qual -Wwrite-strings -Wformat -Wpointer-arith -fno-strict-aliasing

BUILD_CC = gcc
BUILD_CFLAGS = 
BUILD_CPPFLAGS = 
BUILD_LDFLAGS = 
BUILD_LIBS = 

.SUFFIXES:
.SUFFIXES: .c .o

ALWAYS_INCLUDES = -I${top_builddir}
ALWAYS_DEFINES = 
ALWAYS_WARNINGS =

ALL_CPPFLAGS = \
	${ALWAYS_INCLUDES} ${CINCLUDES} ${STD_CINCLUDES} \
	${ALWAYS_DEFINES} ${CDEFINES} ${STD_CDEFINES}

ALL_CFLAGS = ${EXT_CFLAGS} ${ALL_CPPFLAGS} ${CFLAGS} \
	${ALWAYS_WARNINGS} ${STD_CWARNINGS} ${CWARNINGS}

.c.o:
	${LIBTOOL_MODE_COMPILE} ${CC} ${ALL_CFLAGS} -c $<

SHELL = /bin/sh
LIBTOOL = 
LIBTOOL_MODE_COMPILE = ${LIBTOOL} 
LIBTOOL_MODE_INSTALL = ${LIBTOOL} 
LIBTOOL_MODE_LINK = ${LIBTOOL} 
PURIFY = 

MKDEP = ${SHELL} ${top_builddir}/make/mkdep

cleandir: distclean
superclean: maintainer-clean

clean distclean maintainer-clean::
	rm -f *.o *.o *.lo *.la core *.core .depend
	rm -rf .libs

distclean maintainer-clean::
	rm -f Makefile

depend:
	@for i in ${ALL_SUBDIRS}; do \
		if [ "$$i" != "nulldir" -a -d $$i ]; then \
			echo "making depend in `pwd`/$$i"; \
			(cd $$i; ${MAKE} ${MAKEDEFS} DESTDIR="${DESTDIR}" $@) || exit 1; \
		fi; \
	done
	@if [ X"${VPATH}" != X ] ; then \
		if [ X"${SRCS}" != X -a X"${PSRCS}" != X ] ; then \
			echo ${MKDEP} -vpath ${VPATH} ${ALL_CPPFLAGS} ${ALL_CFLAGS} ${SRCS}; \
			${MKDEP} -vpath ${VPATH} ${ALL_CPPFLAGS} ${ALL_CFLAGS} ${SRCS}; \
			echo ${MKDEP} -vpath ${VPATH} -ap ${ALL_CPPFLAGS} ${ALL_CFLAGS} ${PSRCS}; \
			${MKDEP} -vpath ${VPATH} -ap ${ALL_CPPFLAGS} ${ALL_CFLAGS} ${PSRCS}; \
			${DEPENDEXTRA} \
		elif [ X"${SRCS}" != X ] ; then \
			echo ${MKDEP} -vpath ${VPATH} ${ALL_CPPFLAGS} ${ALL_CFLAGS} ${SRCS}; \
			${MKDEP} -vpath ${VPATH} ${ALL_CPPFLAGS} ${ALL_CFLAGS} ${SRCS}; \
			${DEPENDEXTRA} \
		elif [ X"${PSRCS}" != X ] ; then \
			echo ${MKDEP} -vpath ${VPATH} ${ALL_CPPFLAGS} ${ALL_CFLAGS} ${PSRCS}; \
			${MKDEP} -vpath ${VPATH} -p ${ALL_CPPFLAGS} ${ALL_CFLAGS} ${PSRCS}; \
			${DEPENDEXTRA} \
		fi \
	else \
		if [ X"${SRCS}" != X -a X"${PSRCS}" != X ] ; then \
			echo ${MKDEP} ${ALL_CPPFLAGS} ${ALL_CFLAGS} ${SRCS}; \
			${MKDEP} ${ALL_CPPFLAGS} ${ALL_CFLAGS} ${SRCS}; \
			echo ${MKDEP} -ap ${ALL_CPPFLAGS} ${ALL_CFLAGS} ${PSRCS}; \
			${MKDEP} -ap ${ALL_CPPFLAGS} ${ALL_CFLAGS} ${PSRCS}; \
			${DEPENDEXTRA} \
		elif [ X"${SRCS}" != X ] ; then \
			echo ${MKDEP} ${ALL_CPPFLAGS} ${ALL_CFLAGS} ${SRCS}; \
			${MKDEP} ${ALL_CPPFLAGS} ${ALL_CFLAGS} ${SRCS}; \
			${DEPENDEXTRA} \
		elif [ X"${PSRCS}" != X ] ; then \
			echo ${MKDEP} ${ALL_CPPFLAGS} ${ALL_CFLAGS} ${PSRCS}; \
			${MKDEP} -p ${ALL_CPPFLAGS} ${ALL_CFLAGS} ${PSRCS}; \
			${DEPENDEXTRA} \
		fi \
	fi

FORCE:

###
### Libraries
###

AR =		/usr/bin/ar
ARFLAGS =	cruv
RANLIB =	mips64-octeon-linux-gnu-ranlib

###
### Installation
###

INSTALL =		/usr/bin/install -c
INSTALL_PROGRAM =	${INSTALL}
LINK_PROGRAM =		ln -s
INSTALL_SCRIPT =	${INSTALL}
INSTALL_DATA =		${INSTALL} -m 644

###
### Programs used when generating documentation.  It's ok for these
### not to exist when not generating documentation.
###

XSLTPROC =		/usr/bin/xsltproc --novalid --xinclude --nonet
PERL =			/usr/bin/perl
LATEX =			/usr/bin/latex
PDFLATEX =		/usr/bin/pdflatex
W3M =			/usr/bin/w3m

###
### DocBook -> HTML
### DocBook -> man page
###

.SUFFIXES: .docbook .html .1 .2 .3 .4 .5 .6 .7 .8

.docbook.html:
	${XSLTPROC} -o $@ ${top_srcdir}/doc/xsl/isc-docbook-html.xsl $<

.docbook.1:
	${XSLTPROC} -o $@ ${top_srcdir}/doc/xsl/isc-manpage.xsl $<

.docbook.2:
	${XSLTPROC} -o $@ ${top_srcdir}/doc/xsl/isc-manpage.xsl $<

.docbook.3:
	${XSLTPROC} -o $@ ${top_srcdir}/doc/xsl/isc-manpage.xsl $<

.docbook.4:
	${XSLTPROC} -o $@ ${top_srcdir}/doc/xsl/isc-manpage.xsl $<

.docbook.5:
	${XSLTPROC} -o $@ ${top_srcdir}/doc/xsl/isc-manpage.xsl $<

.docbook.6:
	${XSLTPROC} -o $@ ${top_srcdir}/doc/xsl/isc-manpage.xsl $<

.docbook.7:
	${XSLTPROC} -o $@ ${top_srcdir}/doc/xsl/isc-manpage.xsl $<

.docbook.8:
	${XSLTPROC} -o $@ ${top_srcdir}/doc/xsl/isc-manpage.xsl $<

main.o: main.c
	${LIBTOOL_MODE_COMPILE} ${CC} ${ALL_CFLAGS} \
		-DVERSION=\"${VERSION}\" \
		-DNS_LOCALSTATEDIR=\"${localstatedir}\" \
		-DNS_SYSCONFDIR=\"${sysconfdir}\" -c ${srcdir}/main.c

config.o: config.c
	${LIBTOOL_MODE_COMPILE} ${CC} ${ALL_CFLAGS} \
		-DVERSION=\"${VERSION}\" \
		-DNS_LOCALSTATEDIR=\"${localstatedir}\" \
		-c ${srcdir}/config.c

named: ${OBJS} ${UOBJS} ${DEPLIBS}
	${LIBTOOL_MODE_LINK} ${PURIFY} ${CC} ${CFLAGS} ${LDFLAGS} -o $@ \
	${OBJS} ${UOBJS} ${LIBS}

lwresd: named
	rm -f lwresd
	ln named lwresd

doc man:: ${MANOBJS}

docclean manclean maintainer-clean::
	rm -f ${MANOBJS}

clean distclean maintainer-clean::
	rm -f ${TARGETS} ${OBJS}

bind9.xsl.h: bind9.xsl convertxsl.pl
	${PERL} ${srcdir}/convertxsl.pl < ${srcdir}/bind9.xsl > bind9.xsl.h

depend: bind9.xsl.h
statschannel.o: bind9.xsl.h

installdirs:
	$(SHELL) ${top_srcdir}/mkinstalldirs ${DESTDIR}${sbindir}
	$(SHELL) ${top_srcdir}/mkinstalldirs ${DESTDIR}${mandir}/man5
	$(SHELL) ${top_srcdir}/mkinstalldirs ${DESTDIR}${mandir}/man8

install:: named lwresd installdirs
	${LIBTOOL_MODE_INSTALL} ${INSTALL_PROGRAM} named ${DESTDIR}${sbindir}
	(cd ${DESTDIR}${sbindir}; rm -f lwresd; ln named lwresd)
	${INSTALL_DATA} ${srcdir}/named.8 ${DESTDIR}${mandir}/man8
	${INSTALL_DATA} ${srcdir}/lwresd.8 ${DESTDIR}${mandir}/man8
	${INSTALL_DATA} ${srcdir}/named.conf.5 ${DESTDIR}${mandir}/man5

