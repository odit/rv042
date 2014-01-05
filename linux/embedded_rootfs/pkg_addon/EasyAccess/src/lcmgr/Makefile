SOURCES=LcGen.o  LcFirst.o
        
BIN=lcgen lcfirst
all: $(BIN)
include ../make-include.mk

CFLAGS+=-I$(SYS_LIB)

lcgen: LcGen.o $(SYS_LIB)/libSys.so
	${CC} $(CFLAGS) $(LDFLAGS) LcGen.o -o $@ -L $(SYS_LIB) -lSys -lcrypt -lssl

lcfirst: LcFirst.o $(SYS_LIB)/libSys.so
	${CC} $(CFLAGS) $(LDFLAGS) LcFirst.o -o $@ -L $(SYS_LIB) -lSys -lcrypt -lssl

cleanbin: 
	rm $(BIN)

clean:
	rm -f *.o
	rm -f $(BIN)

install:
	if [ -n '$(BIN)' ]; then cp -rf $(BIN) $(ACCESS_POINT_BIN); fi

un-install:
	if [ -n '$(BIN)' ]; then cd $(ACCESS_POINT_BIN); rm -rf $(BIN); fi
