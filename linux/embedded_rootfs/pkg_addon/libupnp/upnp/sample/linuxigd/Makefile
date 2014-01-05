export PATH := $(PATH):/usr/local/arm/bin
PREFIX=/usr
#PREFIX=/bm/cdc/CA2/linux/embedded_rootfs/pkg_addon/libupnp
#LIBUPNP_PREFIX=/usr
#LIBUPNP_PREFIX=/bm/cdc/CA2/linux/embedded_rootfs/pkg_addon/libupnp
LIBUPNP_PREFIX=../../../lib/lib64
#LIBIPTC_PREFIX=/usr

#CC=gcc
CC=mips64-octeon-linux-gnu-gcc

INCLUDES= -I$(LIBUPNP_PREFIX)/include -I../include -I../../../include -I../../../include/upnp
# LIBS= -lpthread -lnku -lupnp -lixml -lthreadutil -L$(LIBUPNP_PREFIX) -L../libs
LIBS= -lpthread -lnku $(LIBUPNP_PREFIX)/libupnp.a $(LIBUPNP_PREFIX)/libixml.a $(LIBUPNP_PREFIX)/libthreadutil.a -L$(LIBUPNP_PREFIX) -L../libs
FILES= main.o gatedevice.o pmlist.o util.o config.o

CFLAGS += -Wall -g -O2

ifdef HAVE_LIBIPTC
ifdef LIBIPTC_PREFIX
LIBS += -L$(LIBIPTC_PREFIX)/lib64
INCLUDES += -I$(LIBIPTC_PREFIX)/include
endif

LIBS += -liptc
INCLUDES += -DHAVE_LIBIPTC
FILES += iptc.o
endif

all: upnpd
	#ln -s /tmp/root-rootfs/lib/* /usr/lib
	#ln -s /tmp/root-rootfs/include/* /usr/include

upnpd: $(FILES)
	$(CC) $(CFLAGS) $(FILES) $(LIBS) -o $@ 
	@echo "make $@ finished on `date`"

%.o:	%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

clean:
	rm -f *.o upnpd

install: upnpd
	#install -d /etc/linuxigd
	#install etc/gatedesc.xml /etc/linuxigd
	#kefile:56: *** missing separator (did you mean TAB instead of 8 spaces?).  Stoinstall etc/gateconnSCPD.xml  /etc/linuxigd
	#install etc/gateicfgSCPD.xml /etc/linuxigd
	#install etc/dummy.xml /etc/linuxigd
	#install upnpd $(PREFIX)/sbin
	#install upnpd.8 $(PREFIX)/share/man/man8
	#if [ ! -f /etc/upnpd.conf ]; then install etc/upnpd.conf /etc; fi
	
	rm -rf $(DESTETC)/linuxigd
	@install -d $(DESTETC)/linuxigd
	@install etc/gatedesc.xml $(DESTETC)/linuxigd
	@install etc/gateconnSCPD.xml $(DESTETC)/linuxigd
	@install etc/gateicfgSCPD.xml $(DESTETC)/linuxigd
	@install etc/dummy.xml $(DESTETC)/linuxigd
	@install upnpd $(DESTBIN)
	@install etc/upnpd.conf $(DESTETC)
	@install etc/ligd.gif $(DESTETC)
