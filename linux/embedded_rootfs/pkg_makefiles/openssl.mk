include ../.config

ifdef TOOLCHAIN_ABI_N32
   SSL_TARGET=linux-octeon32
endif
ifdef TOOLCHAIN_ABI_64
   SSL_TARGET=linux-octeon64
endif
ifdef TOOLCHAIN_UCLIBC
   SSL_TARGET=linux-octeon-uclibc32
endif

PKG:=openssl
VERSION:=0.9.8d
DIR:=${PKG}-${VERSION}

.PHONY: all
all: build install

.PHONY: build
build: ${DIR} ${DIR}/Makefile.bak
	${MAKE} -C ${DIR} AR="${AR} -r"

${DIR}/Makefile.bak:
	cd ${DIR} && ./Configure shared -DOPENSSL_NO_SSL2 --openssldir=/usr ${SSL_TARGET}

.PHONY: install
install: ${DIR}
	${MAKE} -C ${DIR} INSTALL_PREFIX=${ROOT} install_sw

${DIR}:
	tar -zxf ${STORAGE}/${PKG}-${VERSION}.tar.gz
	cd ${DIR} && patch -p0 < ${STORAGE}/openssl.patch
	
