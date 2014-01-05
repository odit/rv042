EXTRA_DIST		=	\
	README			\
	@PACKAGE@-@VERSION@.lsm	\
	@PACKAGE@.spec.in

man_MANS		= @PACKAGE@.8

sbin_PROGRAMS		= dhcpcd icmprequest 

dhcpcd_SOURCES		=	\
	peekfd.c		\
	udpipgen.c		\
	buildmsg.c		\
	signals.c		\
	arp.c			\
	client.c		\
	dhcpcd.c		\
	signals.h		\
	buildmsg.h		\
	client.h		\
	dhcpcd.h		\
	pathnames.h		\
	signals.h		\
	udpipgen.h

icmprequest_SOURCES	=	\
	icmprequest.c		\
	icmpipgen.c		\
	udpipgen.c		\
	peekfd.c		\
	icmpipgen.h		\
	udpipgen.h
	
configdir		= ${sysconfdir}/dhcpcd

config_DATA		=	

#
# Rule to build RPM distribution package
#
rpm: $(PACKAGE)-$(VERSION).tar.gz
	cp $(PACKAGE)-$(VERSION).tar.gz /usr/src/packages/SOURCES
	rpm -ba @PACKAGE@.spec
