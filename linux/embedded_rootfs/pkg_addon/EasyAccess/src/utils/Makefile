include ../make-include.mk

BIN=
ifdef STATISTICS
BIN+=EasyAccessStats
endif

all: $(BIN)

CFLAGS+=-I$(SYS_LIB) -I$(AUTH_LIB)

ifdef STATISTICS
EasyAccessStats: EasyAccessStats.c ${SYS_LIB}/libSys.so
	${CC} $(CFLAGS) $(LDFLAGS) -o $@ EasyAccessStats.c -L$(SYS_LIB) -lSys -lcrypt -lssl
endif

cleanbin: 
	rm $(BIN)

clean:
	rm -f *.o
	rm -f $(BIN)

install:

un-install:
