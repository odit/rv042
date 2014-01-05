SOURCES=AvlLib.o\
	LinkList.o\
       	LogMgr.o\
       	MemMgr.o\
       	MessageLib.o\
       	NetLib.o\
       	SysLib.o\
	MD5.o\
	MD4.o\
	SHA1.o\
	Des.o\
	clientApi.o\
        version.o\
	LicMgr.o\
        ErrString.o\

LIB=libSys.so

include ../make-include.mk

ifdef CONFIG_IPV6
SOURCES+=LibIpv6.o
endif

CFLAGS+=-I../libauth

all: version $(LIB)

$(LIB): $(SOURCES) 
	$(CC) -shared $(CFLAGS) -o libSys.so $(SOURCES)

version: 
	./version.sh

cleanbin: 
	rm $(BIN)
	if [ -n '$(LIB)' ]; then rm -rf $(LIB); fi

clean:
	rm -f *.o
	rm -f $(BIN)
	if [ -n '$(LIB)' ]; then rm -rf $(LIB); fi

install:
	if [ -n '$(LIB)' ]; then $(STRIP) $(LIB); cp -rf $(LIB) $(ACCESS_POINT_BIN); fi

un-install:
	if [ -n '$(LIB)' ]; then rm -rf $(ACCESS_POINT_BIN)/$(LIB); fi
