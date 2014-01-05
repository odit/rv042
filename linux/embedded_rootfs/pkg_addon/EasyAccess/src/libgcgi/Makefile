SOURCES= code.o\
	common.o\
       	crlfstringbuf.o\
       	gcgi.o\
       	mime.o\
       	parse.o

LIB=libgcgi.so

all: $(LIB)
include ../make-include.mk

CFLAGS+=-DHAVE_CONFIG_H -I .
#CFLAGS=-DHAVE_CONFIG_H -I . -O2

$(LIB): $(SOURCES)
	$(CC) -shared $(CFLAGS) -o libgcgi.so $(SOURCES)

install:
	if [ -n '$(LIB)' ]; then $(STRIP) $(LIB); cp -rf $(LIB) $(ACCESS_POINT_BIN); fi

un-install:
	if [ -n '$(LIB)' ]; then rm -rf $(ACCESS_POINT_BIN)/$(LIB); fi

clean:
	rm -f $(LIB)
	rm -f *.o
