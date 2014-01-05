
include ../.config

PKG:=libpcap
VERSION:=0.9.4
DIR:=${PKG}-${VERSION}

ifdef CFG_ENABLE_IPV6	
	IPV6=--enable-ipv6
else
	IPV6=--disable-ipv6
endif

.PHONY: all
all: build install

.PHONY: build
build: ${DIR} ${DIR}/Makefile
	${MAKE} -C ${DIR}

${DIR}/Makefile:
	cd ${DIR} && ac_cv_linux_vers=2 ./configure --prefix=${ROOT}/usr --host=${CROSS} CFLAGS="${CFLAGS}" LDFLAGS=${TOOLCHAIN_ABI} --with-pcap=linux ${IPV6}

.PHONY: install
install: ${DIR}
	${MAKE} -C ${DIR} install

${DIR}:
	tar -zxf ${STORAGE}/${PKG}-${VERSION}.tar.gz
	
