
LIBS = -lnku
NETLINKCTL_OBJ = netlinkctl.o


CFLAGS += -Wall -Wpointer-arith -Wcast-align -Wcast-qual -Wshadow \
 -Waggregate-return -Wnested-externs -Winline -Wwrite-strings \
 -Wstrict-prototypes 

CFLAGS += -O

netlinkctl: $(NETLINKCTL_OBJ)
	$(CC) $(CFLAGS) -o $@ $(NETLINKCTL_OBJ) $(LIBS)

install:
	$(CP) netlinkctl $(DESTUSR)/sbin/netlinkctl

clean:
	rm -f netlinkctl *.o
