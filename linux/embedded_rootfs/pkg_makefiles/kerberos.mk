include ../.config

PKG:=krb5
VERSION:=1.3.1
DIR:=${PKG}-${VERSION}/src

ifdef TOOLCHAIN_ABI_N32
  export LIBDIR=${ROOT}/usr/lib32
endif

ifdef TOOLCHAIN_ABI_64
  export LIBDIR=${ROOT}/usr/lib64
endif

################### NK-modify start ###################
# purpose:   Build Machine  # author:    Chihmou
# date:      2010-07-25   # description: Change PREFIX_DIR path
#PREFIX_DIR:=/tmp/cavium/kerberos
PREFIX_DIR:=${ROOT}/../kerberos
###################  NK-modify end  ###################

.PHONY: all
all: build install

.PHONY: build
build: ${DIR} ${DIR}/Makefile
	${MAKE} -C ${DIR}

${DIR}/Makefile:
	 cd ${DIR} && ac_cv_prog_cc_cross=yes \
		ac_cv_file__etc_environment=no \
	        ac_cv_func_regcomp=no \
		ac_cv_file__etc_TIMEZONE=no \
		./configure \
		--prefix=${PREFIX_DIR} \
		--sysconfdir=/etc \
		--disable-ipv6 \
		--without-krb4 \
    		--without-tcl \
		--without-crypto \
		--without-des425 \
		--without-util \
		--host=${CROSS} \
		--build=x86-linux \
		CPPFLAGS="${CPPFLAGS} -DHAVE_STRUCT_IN6_ADDR -DHAVE_STRUCT_SOCKADDR_IN6" CFLAGS="${CFLAGS}" LDFLAGS=${TOOLCHAIN_ABI} && patch -p0 < ${STORAGE}/kerberos_build.patch
		
.PHONY: install
install: ${DIR}
	${MAKE} -C ${DIR} install
	mkdir -p ${ROOT}/usr/bin/
	cp -f ${PREFIX_DIR}/bin/kinit ${ROOT}/usr/bin/
	cp -f ${PREFIX_DIR}/bin/kdestroy ${ROOT}/usr/bin/
	cp -f ${PREFIX_DIR}/bin/kpasswd ${ROOT}/usr/bin/
	rm -rf ${PREFIX_DIR}

${DIR}:
	tar -zxf ${STORAGE}/${PKG}-${VERSION}.tar.gz
	cd ${DIR} && patch -p0 < ${STORAGE}/kerberos_source.patch
