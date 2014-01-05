
LIBS = -lnku
DETECT_WAN_SESSION_OBJ = detect_wan_session.o


CFLAGS += -Wall -Wpointer-arith -Wcast-align -Wcast-qual -Wshadow \
 -Waggregate-return -Wnested-externs -Winline -Wwrite-strings \
 -Wstrict-prototypes 

CFLAGS += -O

detect_wan_session: $(DETECT_WAN_SESSION_OBJ)
	$(CC) $(CFLAGS) -o $@ $(DETECT_WAN_SESSION_OBJ) $(LIBS)

install: 
	$(CP) detect_wan_session $(DESTUSR)/sbin/detect_wan_session

clean:
	rm -f detect_wan_session *.o
