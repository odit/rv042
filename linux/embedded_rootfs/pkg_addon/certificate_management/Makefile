
LIBS = -lnku
CERTIFICATE_MANAGEMENT_OBJ = certificate_management.o


CFLAGS += -Wall -Wpointer-arith -Wcast-align -Wcast-qual -Wshadow \
 -Waggregate-return -Wnested-externs -Winline -Wwrite-strings \
 -Wstrict-prototypes 

CFLAGS += -O

certificate_management: $(CERTIFICATE_MANAGEMENT_OBJ)
	$(CC) $(CFLAGS) -o $@ $(CERTIFICATE_MANAGEMENT_OBJ) $(LIBS)

install:
	$(CP) certificate_management $(DESTUSR)/sbin/certificate_management

clean:
	rm -f certificate_management *.o
