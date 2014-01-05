SOURCES=libsslcert.o	\
	lscrsa.o	\
	lscrand.o	\
	lscCertReq.o	\
	lscx509.o

LIB=libsslcert.a

include ../make-include.mk

CFLAGS+=	-I$(SYS_LIB)		\
		-DLSC_DEBUG_ASSERT	\
		-DLSC_DEBUG_LOG		\
		-DLSC_OPENSSL_DEBUG	\
		-DLSC_LOG_TARGET=stderr
rm=/bin/rm -f
AR= ar cq
RANLIB= /usr/bin/ranlib

all: version $(LIB)

$(LIB): $(SOURCES) 
	$(rm) $@
	$(AR) $@ $(SOURCES)
	$(RANLIB) $@


version: 
	./version.sh

cleanbin: 
	rm $(BIN)
	if [ -n '$(LIB)' ]; then rm -rf $(LIB); fi

clean:
	rm -f *.o
	rm -f $(BIN)
	if [ -n '$(LIB)' ]; then rm -rf $(LIB); fi
	rm -f *~

install:

un-install:
