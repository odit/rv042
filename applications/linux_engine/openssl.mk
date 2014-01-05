include ../.config

ifdef TOOLCHAIN_ABI_N32
   SSL_TARGET=linux-octeon32
endif
ifdef TOOLCHAIN_ABI_64
   SSL_TARGET=linux-octeon64
endif

PKG:=openssl
VERSION:=0.9.8b
DIR:=${PKG}-${VERSION}

.PHONY: all
all: build install

.PHONY: build
build: ${DIR} ${DIR}/Makefile.bak
	${MAKE} -C ${DIR}

${DIR}/Makefile.bak:
	cd ${DIR} && ./Configure shared --openssldir=/usr ${SSL_TARGET}

.PHONY: install
install: ${DIR}
	${MAKE} -C ${DIR} INSTALL_PREFIX=${ROOT} install_sw

${DIR}:
	tar -zxf ${STORAGE}/${PKG}-${VERSION}.tar.gz
	cd ${DIR} && patch -p0 < ${STORAGE}/openssl.patch
	
