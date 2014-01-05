
LIBS = -lnku
NSD_OBJ = nsd.o ping.o gethost.o check_io.o


CFLAGS += -Wall -Wpointer-arith -Wcast-align -Wcast-qual -Wshadow \
 -Waggregate-return -Wnested-externs -Winline -Wwrite-strings \
 -Wstrict-prototypes 

CFLAGS += -O

nsd: $(NSD_OBJ)
	$(CC) $(CFLAGS) -o $@ $(NSD_OBJ) $(LIBS)

install: nsd
	$(CP) nsd $(DESTUSR)/sbin/nsd

clean:
	rm -f nsd *.o
