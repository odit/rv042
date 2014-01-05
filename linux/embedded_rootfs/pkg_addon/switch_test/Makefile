
LIBS = -lnku
SWITCH_TEST_OBJ = switch_test.o


CFLAGS += -Wall -Wpointer-arith -Wcast-align -Wcast-qual -Wshadow \
 -Waggregate-return -Wnested-externs -Winline -Wwrite-strings \
 -Wstrict-prototypes 

CFLAGS += -O

switch_test: $(SWITCH_TEST_OBJ)
	$(CC) $(CFLAGS) -o $@ $(SWITCH_TEST_OBJ) $(LIBS)

install:
	$(CP) switch_test $(DESTUSR)/sbin/st

clean:
	rm -f st *.o
