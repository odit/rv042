GRAPH_HEADERS=Graph.h

GRAPH_SOURCES=Graph.o

BIN=graphd

all: $(BIN)

include ../make-include.mk

CFLAGS+=-I$(SYS_LIB)

graphd: $(GRAPH_SOURCES) $(SYS_LIB)/libSys.so $(GRAPH_HEADERS)
	${CC} $(CFLAGS) $(LDFLAGS) -o $@ $(GRAPH_SOURCES) -L$(SYS_LIB) -lSys -lcrypt -lssl

cleanbin:
	rm $(BIN)

clean:
	rm -f *.o
	rm -f $(BIN)

install:
	if [ -n '$(BIN)' ]; then $(STRIP) $(BIN); cp -rf $(BIN) $(ACCESS_POINT_BIN); fi

un-install:
	if [ -n '$(BIN)' ]; then cd $(ACCESS_POINT_BIN); rm -rf $(BIN); fi
