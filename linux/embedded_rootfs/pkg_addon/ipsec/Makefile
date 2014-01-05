include ../nkuserland.config
 
top_srcdir = ../strongswan-4.0.4
INCLUDES = -I$(top_srcdir)/src/libfreeswan -I$(top_srcdir)/src/pluto

LIBS = -lnku
NKIPSEC_OBJ = nk_ipsec.o nk_whack.o ../webconfig/webconfig_util.o

NKUSBKEY_OBJ = nk_usbkey.o ../webconfig/webconfig_util.o

ifeq "$(CONFIG_NK_IPSEC_RW_CONN)" "y"
NKQUICKVPN_OBJ = nk_quickvpn.o ../webconfig/webconfig_util.o
NKQUICKVPNMONITOR_OBJ = rwConnectionMonitor.o ../webconfig/webconfig_util.o
QUICKVPN_DOS_PREVENT_OBJ = quickvpn_dos_prevent.o ../webconfig/webconfig_util.o
endif

CFLAGS +=  -lpthread -Wall -Wpointer-arith -Wcast-align -Wcast-qual -Wshadow \
 -Waggregate-return -Wnested-externs -Winline -Wwrite-strings \
 -Wstrict-prototypes 

CFLAGS += -O

#ifeq "$(CONFIG_NK_IPSEC_RW_CONN)" "y"
#all: _updown nk_ipsecd nk_usbkeyd nk_quickvpnd rwconnectionmonitord quickvpn_dos_prevent
#else
#all: _updown nk_ipsecd nk_usbkeyd
#endif
all:

_updown: _updown.o
	$(CC) $(CFLAGS) -o _updown _updown.o $(LIBS)

nk_ipsecd: $(NKIPSEC_OBJ)
	$(CC) $(CFLAGS) -o nk_ipsecd $(NKIPSEC_OBJ) $(LIBS)

nk_whack.o:nk_whack.c
	$(CC) $(CFLAGS) $(INCLUDES) -c nk_whack.c $(LIBS)

nk_usbkeyd: $(NKUSBKEY_OBJ)
	$(CC) $(CFLAGS) -o nk_usbkeyd $(NKUSBKEY_OBJ) $(LIBS)

ifeq "$(CONFIG_NK_IPSEC_RW_CONN)" "y"
nk_quickvpnd: $(NKQUICKVPN_OBJ)
	$(CC) $(CFLAGS) -o nk_quickvpnd $(NKQUICKVPN_OBJ) $(LIBS)
rwconnectionmonitord: $(NKQUICKVPNMONITOR_OBJ)
	$(CC) $(CFLAGS) -o rwconnectionmonitord $(NKQUICKVPNMONITOR_OBJ) $(LIBS)
quickvpn_dos_prevent: $(QUICKVPN_DOS_PREVENT_OBJ)
	$(CC) $(CFLAGS) -o quickvpn_dos_prevent $(QUICKVPN_DOS_PREVENT_OBJ) $(LIBS)
endif

install:
	$(CP) nk_ipsecd $(DESTUSR)/sbin/nk_ipsecd
	$(CP) _updown $(DESTUSR)/sbin/_updown
	$(CP) nk_usbkeyd $(DESTUSR)/sbin/nk_usbkeyd
ifeq "$(CONFIG_NK_IPSEC_RW_CONN)" "y"
	$(CP) nk_quickvpnd $(DESTUSR)/sbin/nk_quickvpnd
	$(CP) rwconnectionmonitord $(DESTUSR)/sbin/rwconnectionmonitord
	$(CP) quickvpn_dos_prevent $(DESTUSR)/sbin/quickvpn_dos_prevent
endif

clean:
ifeq "$(CONFIG_NK_IPSEC_RW_CONN)" "y"
	rm -f nk_ipsecd _updown nk_usbkeyd nk_quickvpnd  rwconnectionmonitord *.o quickvpn_dos_prevent *.o
else
	rm -f nk_ipsecd _updown nk_usbkeyd *.o
endif
