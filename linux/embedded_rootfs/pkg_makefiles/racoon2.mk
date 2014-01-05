include ../.config

PKG:=racoon2
VERSION:=20061228a
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
	cd ${DIR} && ac_cv_func_memcmp_clean=yes ac_cv_sizeof_long_long=8 CPPFLAGS="${CFLAGS}" LDFLAGS="${TOOLCHAIN_ABI}" AR="${AR}" ./configure --prefix=${ROOT}/usr --host=${CROSS} \
		--enable-shared \
		--disable-static \
		--disable-kinkd \
		--disable-pedant \
		--enable-natt \
		--disable-ikev1 \
		--with-openssl-libdir=${ROOT}/usr ${IPV6} \
		--with-kernel-build-dir=${KERNEL_DIR}
		

.PHONY: install
install:
	${MAKE} -C ${DIR} install

${DIR}:
	tar -zxf ${STORAGE}/${PKG}-${VERSION}.tgz
	cd ${DIR} && patch -p0 < ${STORAGE}/racoon2_64.patch
