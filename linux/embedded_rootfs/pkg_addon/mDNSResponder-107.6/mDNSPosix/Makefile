
LIBVERS = 1

COREDIR = ../mDNSCore
SHAREDDIR = ../mDNSShared
JDK = /usr/jdk

#CC = @cc
#LD = ld -shared
LD += -shared
CP = cp
RM = rm
LN = ln -s -f
CFLAGS_COMMON = -I. -I$(COREDIR) -I$(SHAREDDIR) -W -Wall -DPID_FILE=\"/var/run/mdnsd.pid\" -DMDNS_UDS_SERVERPATH=\"/var/run/mdnsd\"
CFLAGS_PTHREAD =
LINKOPTS =
LINKOPTS_PTHREAD = -lpthread
LDSUFFIX = so
JAVACFLAGS_OS = -fPIC -shared -ldns_sd

# Set up diverging paths for debug vs. prod builds
DEBUG=0
ifeq ($(DEBUG),1)
CFLAGS_DEBUG = -g -DMDNS_DEBUGMSGS=2 
OBJDIR = objects/debug
BUILDDIR = build/debug
STRIP = echo 
else
# We use -Os for two reasons:
# 1. We want to make small binaries, suitable for putting into hardware devices
# 2. Some of the code analysis warnings only work when some form of optimization is enabled
#CFLAGS_DEBUG = -Os -DMDNS_DEBUGMSGS=0 
OBJDIR = objects/prod
BUILDDIR = build/prod
#STRIP = strip -S 
endif

# Configure per-OS peculiarities
CFLAGS_OS = -DNOT_HAVE_SA_LEN -DUSES_NETLINK -DHAVE_LINUX
JAVACFLAGS_OS += -I$(JDK)/include/linux
OPTIONALTARG = nss_mdns
OPTINSTALL   = InstalledNSS

NSSLIBNAME  := libnss_mdns
NSSVERSION  := 0.2
NSSLIBFILE  := $(NSSLIBNAME)-$(NSSVERSION).so
NSSLINKNAME := $(NSSLIBNAME).so.2
NSSINSTPATH := /lib

# If not otherwise defined, we install into /usr/lib and /usr/include
# and our startup script is called mdns (e.g. /etc/init.d/mdns)
INSTBASE?=${ROOT}/usr

ifeq ($(HAVE_IPV6),1)
CFLAGS_OS += -DHAVE_IPV6=1
else
ifeq ($(HAVE_IPV6),0)
CFLAGS_OS += -DHAVE_IPV6=0
endif
endif

# If directory /usr/share/man exists, then we install man pages into that, else /usr/man
ifeq ($(wildcard /usr/share/man), /usr/share/man)
MANPATH := /usr/share/man
else
MANPATH := /usr/man
endif

CFLAGS += $(CFLAGS_COMMON) $(CFLAGS_OS) $(CFLAGS_DEBUG)

#############################################################################

#all: setup Daemon libdns_sd Clients SAClient SAResponder SAProxyResponder Identify NetMonitor dnsextd $(OPTIONALTARG)
all: setup Daemon libdns_sd Clients

#install: setup InstalledDaemon InstalledLib InstalledClients $(OPTINSTALL)
install: setup InstalledDaemon InstalledLib InstalledClients

# 'setup' sets up the build directory structure the way we want
setup:
	@if test ! -d objects     ; then mkdir objects     ; fi
	@if test ! -d build       ; then mkdir build       ; fi
	@if test ! -d $(OBJDIR)   ; then mkdir $(OBJDIR)   ; fi
	@if test ! -d $(BUILDDIR) ; then mkdir $(BUILDDIR) ; fi

# clean removes targets and objects
clean:
	@if test -d $(OBJDIR)   ; then rm -r $(OBJDIR)   ; fi
	@if test -d $(BUILDDIR) ; then rm -r $(BUILDDIR) ; fi
	@$(MAKE) -C ../Clients clean

#############################################################################

# daemon target builds the daemon
DAEMONOBJS = $(OBJDIR)/PosixDaemon.c.o $(OBJDIR)/mDNSPosix.c.o $(OBJDIR)/mDNSUNP.c.o $(OBJDIR)/mDNS.c.o \
             $(OBJDIR)/DNSDigest.c.o $(OBJDIR)/uDNS.c.o $(OBJDIR)/DNSCommon.c.o $(OBJDIR)/uds_daemon.c.o \
             $(OBJDIR)/mDNSDebug.c.o $(OBJDIR)/dnssd_ipc.c.o $(OBJDIR)/GenLinkedList.c.o $(OBJDIR)/PlatformCommon.c.o 

Daemon: setup $(BUILDDIR)/mdnsd
	@echo "Responder daemon done"

$(BUILDDIR)/mdnsd: $(DAEMONOBJS)
	$(CC) -o $@ $+ $(LINKOPTS)
	@$(STRIP) $@

# libdns_sd target builds the client library
libdns_sd: setup $(BUILDDIR)/libdns_sd.$(LDSUFFIX)
	@echo "Client library done"

CLIENTLIBOBJS = $(OBJDIR)/dnssd_clientlib.c.so.o $(OBJDIR)/dnssd_clientstub.c.so.o $(OBJDIR)/dnssd_ipc.c.so.o

$(BUILDDIR)/libdns_sd.$(LDSUFFIX): $(CLIENTLIBOBJS)
	@$(LD) $(LINKOPTS) -o $@ $+
	@$(STRIP) $@

Clients: setup libdns_sd ../Clients/build/dns-sd
	@echo "Clients done"

../Clients/build/dns-sd:
	@$(MAKE) -C ../Clients

# nss_mdns target builds the Name Service Switch module
nss_mdns: setup $(BUILDDIR)/$(NSSLIBFILE)
	@echo "Name Service Switch module done"

$(BUILDDIR)/$(NSSLIBFILE): $(CLIENTLIBOBJS) $(OBJDIR)/nss_mdns.c.so.o
	@$(LD) $(LINKOPTS) -o $@ $+
	@$(STRIP) $@

#############################################################################

# The Install targets place built stuff in their proper places
InstalledDaemon: $(INSTBASE)/sbin/mdnsd
	@echo $+ " installed"

InstalledLib: $(INSTBASE)/lib64/libdns_sd.$(LDSUFFIX).$(LIBVERS)
	@echo $+ " installed"

InstalledManPages: $(MANPATH)/man8/mdnsd.8
	@echo $+ " installed"

InstalledClients: $(INSTBASE)/bin/dns-sd
	@echo $+ " installed"

InstalledNSS: $(NSSINSTPATH)/$(NSSLINKNAME) /etc/nss_mdns.conf $(MANPATH)/man5/nss_mdns.conf.5 $(MANPATH)/man8/libnss_mdns.8
	@echo $+ " installed"

# Note: If daemon already installed, we make sure it's stopped before overwriting it
$(INSTBASE)/sbin/mdnsd: $(BUILDDIR)/mdnsd
	$(CP) $< $@

$(INSTBASE)/lib64/libdns_sd.$(LDSUFFIX).$(LIBVERS): $(BUILDDIR)/libdns_sd.$(LDSUFFIX)
	$(CP) $< $@
	$(LN) libdns_sd.$(LDSUFFIX).$(LIBVERS) $(INSTBASE)/lib64/libdns_sd.$(LDSUFFIX)
#	$(LN) $@ $(INSTBASE)/lib64/libdns_sd.$(LDSUFFIX)
ifdef LDCONFIG
    # -m means 'merge into existing database', -R means 'rescan directories'
	$(LDCONFIG) -mR
endif

$(INSTBASE)/bin/dns-sd: ../Clients/build/dns-sd
	$(CP) $< $@

$(NSSINSTPATH)/$(NSSLINKNAME): $(NSSINSTPATH)/$(NSSLIBFILE)
	$(LN) $< $@
	ldconfig

$(NSSINSTPATH)/$(NSSLIBFILE): $(BUILDDIR)/$(NSSLIBFILE)
	$(CP) $< $@
	chmod 444 $@

/etc/nss_mdns.conf: nss_mdns.conf
	$(CP) $< $@
	chmod 444 $@
	# Check the nsswitch.conf file.
	# If 'mdns' does not already appear on the "hosts:" line, then add it right before 'dns'
	cp -f /etc/nsswitch.conf /etc/nsswitch.conf.pre-mdns
	sed -e '/mdns/!s/^\(hosts:.*\)dns\(.*\)/\1mdns dns\2/' /etc/nsswitch.conf.pre-mdns > /etc/nsswitch.conf

#############################################################################

# The following targets build Java wrappers for the dns-sd.h API.

JAVAC = $(JDK)/bin/javac
JAVAH = $(JDK)/bin/javah
JAVADOC = $(JDK)/bin/javadoc
JAR = $(JDK)/bin/jar
JAVACFLAGS = $(CFLAGS) $(JAVACFLAGS_OS) -I$(JDK)/include

Java: setup $(BUILDDIR)/dns_sd.jar $(BUILDDIR)/libjdns_sd.$(LDSUFFIX)
	@echo "Java wrappers done"

JAVASRC	= $(SHAREDDIR)/Java
JARCONTENTS =	$(OBJDIR)/com/apple/dnssd/DNSSDService.class \
				$(OBJDIR)/com/apple/dnssd/DNSSDException.class \
				$(OBJDIR)/com/apple/dnssd/DNSRecord.class \
				$(OBJDIR)/com/apple/dnssd/TXTRecord.class \
				$(OBJDIR)/com/apple/dnssd/DNSSDRegistration.class \
				$(OBJDIR)/com/apple/dnssd/BaseListener.class \
				$(OBJDIR)/com/apple/dnssd/BrowseListener.class \
				$(OBJDIR)/com/apple/dnssd/ResolveListener.class \
				$(OBJDIR)/com/apple/dnssd/RegisterListener.class \
				$(OBJDIR)/com/apple/dnssd/QueryListener.class \
				$(OBJDIR)/com/apple/dnssd/DomainListener.class \
				$(OBJDIR)/com/apple/dnssd/RegisterRecordListener.class \
				$(OBJDIR)/com/apple/dnssd/DNSSDRecordRegistrar.class \
				$(OBJDIR)/com/apple/dnssd/DNSSD.class

$(BUILDDIR)/dns_sd.jar: $(JARCONTENTS) setup
	$(JAR) -cf $@ -C $(OBJDIR) com

$(BUILDDIR)/libjdns_sd.$(LDSUFFIX): $(JAVASRC)/JNISupport.c $(OBJDIR)/DNSSD.java.h setup libdns_sd
	$(CC) -o $@ $< $(JAVACFLAGS) -I$(OBJDIR) -L$(BUILDDIR)

$(OBJDIR)/com/apple/dnssd/%.class:	$(JAVASRC)/%.java
	$(JAVAC) -d $(OBJDIR) -classpath $(OBJDIR) $<

$(OBJDIR)/DNSSD.java.h: $(OBJDIR)/com/apple/dnssd/DNSSD.class
	$(JAVAH) -force -classpath $(OBJDIR) -o $@ \
		com.apple.dnssd.AppleDNSSD \
		com.apple.dnssd.AppleBrowser \
		com.apple.dnssd.AppleResolver \
		com.apple.dnssd.AppleRegistration \
		com.apple.dnssd.AppleQuery \
		com.apple.dnssd.AppleDomainEnum \
		com.apple.dnssd.AppleService 

#############################################################################

# Implicit rules
$(OBJDIR)/%.c.o:	%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/%.c.o:	$(COREDIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/%.c.o:	$(SHAREDDIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/%.c.threadsafe.o:	%.c
	$(CC) $(CFLAGS) $(CFLAGS_PTHREAD) -D_REENTRANT -c -o $@ $<

$(OBJDIR)/%.c.so.o:	%.c
	$(CC) $(CFLAGS) -c -fPIC -o $@ $<

$(OBJDIR)/%.c.so.o:	$(SHAREDDIR)/%.c
	$(CC) $(CFLAGS) -c -fPIC -o $@ $<
