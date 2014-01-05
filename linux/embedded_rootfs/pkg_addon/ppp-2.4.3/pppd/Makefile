

#DEFS = -DUSE_MS_DNS=1 -D_linux_=1 -DNO_SYSLOG -DHAVE_PATHS_H -DXDEBUGALL -D__linux__
CFLAGS += -mabi=64 -O2 -pipe -Wall
#CFLAGS += -O2 -pipe -Wall -g
#CFLAGS += -I../include

#OBJS =	main.o magic.o fsm.o lcp.o ipcp.o upap.o chap.o ccp.o \
#	auth.o options.o demand.o utils.o sys-linux.o ipxcp.o multilink.o \
#	tdb.o tty.o md4.o chap_ms.o
OBJS =	main.o magic.o fsm.o lcp.o ipcp.o upap.o chap-new.o md5.o ccp.o \
	chap-md5.o ecp.o auth.o options.o demand.o utils.o sys-linux.o ipxcp.o multilink.o \
	eap.o tdb.o tty.o cbcp.o chap_ms.o md4.o sha1.o pppcrypt.o spinlock.o plugins/rp-pppoe/plugin.o \
	plugins/rp-pppoe/libplugin.a syslog.o

# OBJS +=	md5.o

CHAPMS=y
# Enable plugins
PLUGIN=y
MPPE=y
CFLAGS   += -DHAVE_PATHS_H -DIPX_CHANGE -I../include -DCHAPMS=1 -DMPPE=1 -DHAVE_CRYPT_H=1 -DUSE_CRYPT=1 -DHAVE_MULTILINK -DUSE_TDB=1 -DPLUGIN -DMAXOCTETS
#CFLAGS   += -DUSE_CRYPT=1
#CFLAGS   += -DMPPE=1
# PPPDOBJS += md4.o chap_ms.o

ifdef PLUGIN
CFLAGS	+= -DPLUGIN
LDFLAGS	+= -Wl,-E
LIBS	+= -ldl
endif

LDADD := -lcrypt -lnku ${LDADD}

#CFLAGS += $(DEFS)
LDFLAGS	+= -Wl,-E

all: pppd

pppd: $(OBJS)
	${CC} $(LDFLAGS) -o pppd ${LIBS} ${OBJS} ${LDADD}

clean:
	rm -f *.[oa] *.elf *.gdb *~ core pppd

install: all
	cp pppd $(ROOT)/sbin

