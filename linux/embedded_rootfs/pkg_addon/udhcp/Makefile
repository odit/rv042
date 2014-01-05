# udhcp makefile

# Uncomment this to get a shared binary. Call as udhcpd for the server,
# and udhcpc for the client
# COMBINED_BINARY=1

VER := 0.9.6

# Uncomment this to output messages to stdout.
# CFLAGS += -DSYSLOG

# Uncomment this to output debug messages to stdout.
# CFLAGS += -DDEBUG

CFLAGS += -DVERSION='"$(VER)"'

OBJS_SHARED = options.o socket.o packet.o pidfile.o
DHCPD_OBJS = dhcpd.o arpping.o files.o leases.o serverpacket.o
DHCPC_OBJS = dhcpc.o clientpacket.o script.o

EXEC1 = udhcpd
ifdef COMBINED_BINARY
OBJS1 = $(DHCPD_OBJS) $(DHCPC_OBJS) $(OBJS_SHARED) frontend.o
CFLAGS += -DCOMBINED_BINARY
else
OBJS1 = $(DHCPD_OBJS) $(OBJS_SHARED)

EXEC2 = udhcpc
OBJS2 = $(DHCPC_OBJS) $(OBJS_SHARED)
endif

EXEC3 = dumpleases
OBJS3 = dumpleases.o

LDADD += -lnku

# 2007/04/25 jane: Uncomment this to support multiple dhcp subnet
CFLAGS += -DNK_CONFIG_SUPPORT_MULTI_DHCP_SUBNET

all: $(EXEC1) $(EXEC2) $(EXEC3)

$(OBJS1) $(OBJS2) $(OBJS3): *.h Makefile
$(EXEC1) $(EXEC2) $(EXEC3): Makefile

$(EXEC1): $(OBJS1)
	$(CC) $(CFLAGS) -o $(EXEC1) ${LIBS} $(OBJS1) ${LDADD}

$(EXEC2): $(OBJS2)
	$(CC) $(CFLAGS) -o $(EXEC2) ${LIBS} $(OBJS2) ${LDADD}

$(EXEC3): $(OBJS3)
	$(CC) $(CFLAGS) -o $(EXEC3) ${LIBS} $(OBJS3) ${LDADD}

install: all
	${CP} $(EXEC1) $(DESTSBIN)
ifdef COMBINED_BINARY
	#cd ${DESTSBIN}
	#ln -sf /sbin/$(EXEC1) udhcpc
	(cd ${DESTSBIN}; ${SU} "ln -sf /sbin/$(EXEC1) udhcpc")
else
	${CP} $(EXEC2) $(DESTSBIN)
endif
	${CP} $(EXEC3) $(DESTSBIN)

clean:
	rm -f ${EXEC1} ${EXEC2} ${EXEC3} *.o core

