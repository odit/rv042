LIBS= -lnku
DHCP_HELPER_OBJ = dhcp-helper.o

CFLAGS += -Wall -Wpointer-arith -Wcast-align -Wcast-qual -Wshadow \
 -Waggregate-return -Wnested-externs -Winline -Wwrite-strings \
 -Wstrict-prototypes 

CFLAGS += -O

dhcp-helper: $(DHCP_HELPER_OBJ)
	$(CC) $(CFLAGS) -o $@ $(DHCP_HELPER_OBJ) ${LIBS}

install: dhcp-helper
	$(CP) dhcp-helper $(DESTUSR)/sbin/dhcp-helper

clean:
	rm -f *~ *.o core dhcp-helper
