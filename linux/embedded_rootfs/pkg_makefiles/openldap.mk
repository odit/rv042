include ../.config

ifdef TOOLCHAIN_ABI_N32
  export LIBDIR=${ROOT}/usr/lib32
endif

ifdef TOOLCHAIN_ABI_64
  export LIBDIR=${ROOT}/usr/lib64
endif

PKG:=openldap
VERSION:=2.3.24
DIR:=${PKG}-${VERSION}

################### NK-modify start ###################
# purpose:   Build Machine  # author:    Chihmou
# date:      2010-07-25   # description: Change PREFIX_DIR path
#PREFIX_DIR:=/tmp/cavium/openldap
PREFIX_DIR:=${ROOT}/../openldap
###################  NK-modify end  ###################
unexport LDFLAGS

.PHONY: all
all: build install

.PHONY: build
build: ${DIR} ${DIR}/Makefile
	${MAKE} -C ${DIR} -j4

${DIR}/Makefile:
	cd ${DIR} && ac_cv_func_memcmp_working=yes ac_cv_func_setvbuf_reversed=no ./configure --prefix=${PREFIX_DIR} \
	--sysconfdir=${ROOT}/etc \
	--with-yielding_select=yes \
	--disable-bdb \
	--disable-hdb \
	--disable-slapd \
	--disable-slurpd \
	--disable-syslog \
        --without-threads \
	--disable-debug \
	--disable-ipv6 \
	--without-cyrus-sasl \
 	--without-tls \
	--host=${CROSS} CFLAGS="${CFLAGS}" LDFLAGS="${TOOLCHAIN_ABI}" STRIPFLAGS="${TOOLCHAIN_ABI} -Wl" && patch -p0 < ${STORAGE}/openldap_build.patch

.PHONY: install
install: ${DIR}
	${MAKE} -C ${DIR} install		
	mkdir -p ${ROOT}/usr/bin
	mkdir -p ${ROOT}/usr/lib
	cp -f ${PREFIX_DIR}/bin/ldapsearch ${ROOT}/usr/bin
	cp -f ${PREFIX_DIR}/lib/liblber-2.3.so.0.2.12 ${LIBDIR}
	cd ${LIBDIR} && ln -s -f liblber-2.3.so.0.2.12 liblber-2.3.so.0
	cd ${LIBDIR} && ln -s -f liblber-2.3.so.0.2.12 liblber.so
	cp -f ${PREFIX_DIR}/lib/liblber.la ${LIBDIR}
	cp -f ${PREFIX_DIR}/lib/libldap-2.3.so.0.2.12 ${LIBDIR}
	cd ${LIBDIR} && ln -s -f libldap-2.3.so.0.2.12 libldap-2.3.so.0
	cd ${LIBDIR} && ln -s -f libldap-2.3.so.0.2.12 libldap.so
	cp -f ${PREFIX_DIR}/lib/libldap.la ${LIBDIR}
	rm -rf ${PREFIX_DIR}
${DIR}:
	tar -zxf ${STORAGE}/${PKG}-${VERSION}.tar.gz
	cd ${DIR}/build && patch -p0 < ${STORAGE}/openldap_config.patch
