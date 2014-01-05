
include ../.config

PKG:=ipsec-tools
VERSION:=0.6.5
DIR:=${PKG}-${VERSION}
unexport LDFLAGS

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
	cd ${DIR} && ./configure --prefix=/usr --host=${CROSS} \
		--enable-shared \
		--disable-static \
		--enable-frag \
		--enable-hybrid \
		--enable-xauth \
		--enable-dpd \
		--enable-adminport \
		--enable-natt \
		--with-openssl=${ROOT}/usr ${IPV6} \
		--with-kernel-headers=${KERNEL_DIR}/include \
		--with-flexlib=${ROOT}/usr/lib/libfl.a

.PHONY: install
install:
	${MAKE} -C ${DIR} install prefix=/${ROOT}/usr

${DIR}:
	tar -jxf ${STORAGE}/${PKG}-${VERSION}.tar.bz2
	cd ${DIR} && patch -p0 < ${STORAGE}/ipsec-tools-cofigure.patch
	
