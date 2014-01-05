
LIBS = -lnku
LAN_UP_DAEMON_OBJ = lan_up_daemon.o


CFLAGS += -Wall -Wpointer-arith -Wcast-align -Wcast-qual -Wshadow \
 -Waggregate-return -Wnested-externs -Winline -Wwrite-strings \
 -Wstrict-prototypes 

CFLAGS += -O

lan_up_daemon: $(LAN_UP_DAEMON_OBJ)
	$(CC) $(CFLAGS) -o $@ $(LAN_UP_DAEMON_OBJ) $(LIBS)

install:
	$(CP) lan_up_daemon $(DESTUSR)/sbin/lan_up_daemon

clean:
	rm -f lan_up_daemon *.o
