
include ../.config

PKG:=iptables
VERSION:=1.3.5
DIR:=${PKG}-${VERSION}

.PHONY: all
all: build install

.PHONY: build
build: ${DIR}
	${MAKE} -C ${DIR} LD="${LD} ${LDFLAGS}" COPT_FLAGS="${CFLAGS}" DO_MULTI=1 \
		BINDIR=${ROOT}/usr/bin LIBDIR=${LIBDIR} MANDIR=${ROOT}/usr/man

.PHONY: install
install:
	${MAKE} -C ${DIR} LD="${LD} ${LDFLAGS}" COPT_FLAGS="${CFLAGS}" DO_MULTI=1 \
		BINDIR=${ROOT}/usr/bin LIBDIR=${ROOT}/${LIBDIR} MANDIR=${ROOT}/usr/man install

${DIR}:
	tar -jxf ${STORAGE}/${PKG}-${VERSION}.tar.bz2
ifdef CFG_ENABLE_IPV6	
	# Defaults to IPV6
else
	sed -i "s/DO_IPV6:=1/DO_IPV6:=0/" ${DIR}/Makefile
endif
	
