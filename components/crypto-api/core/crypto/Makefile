include $(OCTEON_ROOT)/common.mk
TOP=		..
TOPDIR =$(PWD)/../

INCLUDE=	-I. -I$(TOP) -I../include 
MAKE=           make -f Makefile.ssl
CFLAGS= $(INCLUDE) $(CFLAGS_GLOBAL) -DOCTEON_OPENSSL -DOPENSSL_NO_LOCKING -O2 -fno-strict-aliasing
# Enable the flag for static memory .
#CFLAGS += -DOCTEON_OPENSSL_NO_DYNAMIC_MEMORY

ifeq (linux,$(findstring linux,$(OCTEON_TARGET)))
CFLAGS += -DTARGET_LINUX
endif
SDIRS= sha md5 des dsa rsa dh rc4 hmac aes evp err bn stack \
	buffer engine lhash x509 rand objects asn1 bio modexp \
	x509v3 asn1 pkcs7 md2 pem conf ocsp

LIB= $(TOP)/libcrypto.a

LIBSRC= cryptlib.c crypto-generic-api.c mem.c mem_clr.c mem_dbg.c ex_data.c o_time.c
LIBOBJ= cryptlib.o crypto-generic-api.o mem.o mem_dbg.o mem_clr.o ex_data.o o_time.o

SRC= $(LIBSRC)

EXHEADER= crypto.h tmdiff.h opensslv.h opensslconf.h ebcdic.h symhacks.h \
	ossl_typ.h
HEADER=	cryptlib.h buildinf.h md32_common.h o_time.h $(EXHEADER)

#RANLIB = mipsisa64-octeon-elf-ranlib
RANLIB = mips64-octeon-linux-gnu-ranlib

#AR = mips64-octeon-linux-gnu-ar r
export CFLAGS RANLIB CC AR

ifndef OCTEON_TARGET
all:
	@echo "Usage:"
	@echo "     make OCTEON_TARGET=cvmx_64" 
	@echo "                or               "
	@echo "     make OCTEON_TARGET=linux_n32" 
	@echo "                or               "
	@echo "     make OCTEON_TARGET=linux_64" 
else
ifeq (cvmx_n32, $(OCTEON_TARGET))
all:
	@echo "cvmx_n32 is not supported in this Crypto Release"
else
all:  link buildtemp lib subdirs cleantemp
endif
endif

link:	
	@ln -sf $(TOPDIR)/include/openssl $(OCTEON_ROOT)/target/include/ 

lib:	$(LIBOBJ)
	$(AR) r $(LIB) $(LIBOBJ)
	@touch lib

subdirs:
	@for i in $(SDIRS); \
	do \
		cd $$i && echo "making in crypto/$$i....." && \
		$(MAKE) || exit 1; \
		cd ..; \
	done;

buildtemp:
	@ln -sf $(TOPDIR)/include/openssl/e_os.h $(TOPDIR)/
	@ln -sf $(TOPDIR)/crypto/crypto-generic-api.h $(TOPDIR)/include/openssl
	@ln -sf $(TOPDIR)/crypto/cryptlib.h $(TOPDIR)/include/openssl

cleantemp:
	@if [ -f $(TOPDIR)/libcrypto.a ]; then \
		cp $(TOPDIR)/libcrypto.a $(OCTEON_ROOT)/target/lib/; \
	fi
	@rm -f $(TOPDIR)/e_os.h

clean:
	@rm -f *.o *.obj lib
	@rm -f $(OCTEON_ROOT)/target/lib/libcrypto.a
	@for i in $(SDIRS) ;\
	do \
	(cd $$i && echo "making clean in crypto/$$i..." && \
	$(MAKE) CC='$(CC)' CFLAG='${CFLAG}' INSTALLTOP='${INSTALLTOP}' PEX_LIBS='${PEX_LIBS}' EX_LIBS='${EX_LIBS}' clean ); \
	done;

mem.o: ../e_os.h ../include/openssl/bio.h ../include/openssl/buffer.h
mem.o: ../include/openssl/crypto.h ../include/openssl/e_os2.h
mem.o: ../include/openssl/err.h ../include/openssl/lhash.h
mem.o: ../include/openssl/opensslconf.h ../include/openssl/opensslv.h
mem.o: ../include/openssl/safestack.h ../include/openssl/stack.h
mem.o: ../include/openssl/symhacks.h cryptlib.h mem.c
mem_dbg.o: ../e_os.h ../include/openssl/bio.h ../include/openssl/buffer.h
mem_dbg.o: ../include/openssl/crypto.h ../include/openssl/e_os2.h
mem_dbg.o: ../include/openssl/err.h ../include/openssl/lhash.h
mem_dbg.o: ../include/openssl/opensslconf.h ../include/openssl/opensslv.h
mem_dbg.o: ../include/openssl/safestack.h ../include/openssl/stack.h
mem_dbg.o: ../include/openssl/symhacks.h cryptlib.h mem_dbg.c
mem_clr.o: ../include/openssl/crypto.h ../include/openssl/e_os2.h
mem_clr.o: ../include/openssl/opensslconf.h ../include/openssl/opensslv.h
mem_clr.o: ../include/openssl/safestack.h ../include/openssl/stack.h
mem_clr.o: ../include/openssl/symhacks.h mem_clr.c
