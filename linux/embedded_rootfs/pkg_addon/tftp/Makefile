LIBS = -lnku
TFTP_OBJ = tftpd.o tftpsubs.o 


CFLAGS += -Wall -Wpointer-arith -Wcast-align -Wcast-qual -Wshadow \
 -Waggregate-return -Wnested-externs -Winline -Wwrite-strings \
 -Wstrict-prototypes 

CFLAGS += -O

tftpd: $(TFTP_OBJ)
	$(CC) $(CFLAGS) -o $@ $(TFTP_OBJ) $(LIBS)

install: tftpd
	$(CP) tftpd $(DESTUSR)/sbin/tftpd

clean:
	rm -f tftpd *.o
