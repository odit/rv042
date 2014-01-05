include ../.config

ifdef TOOLCHAIN_ABI_N32
  export LIBDIR=${ROOT}/usr/lib32
endif

ifdef TOOLCHAIN_ABI_64
  export LIBDIR=${ROOT}/usr/lib64
endif

PKG:=samba
VERSION:=3.0.22
DIR:=${PKG}-${VERSION}/source

################### NK-modify start ###################
# purpose:   Build Machine  # author:    Chihmou
# date:      2010-07-25   # description: Change PREFIX_DIR path
#PREFIX_DIR:= /tmp/cavium/samba
PREFIX_DIR:= ${ROOT}/../cavium/samba
###################  NK-modify end  ###################

.PHONY: all
all: build install

.PHONY: build
build: ${DIR} ${DIR}/Makefile
	${MAKE} -C ${DIR} CFLAGS="${CFLAGS}" proto
	${MAKE} -C ${DIR} CFLAGS="${CFLAGS}" -j4

${DIR}/Makefile:
	cd ${DIR} && ac_cv_prog_cc_cross=${CROSS} \
                     ac_cv_prog_RANLIB=${RANLIB} \
		     ac_cv_func_setvbuf_reversed=no \
		     ac_cv_search_crypt="-lcrypt -lc" \
		     samba_cv_HAVE_GETTIMEOFDAY_TZ=yes \
		     samba_cv_HAVE_IFACE_IFCONF=yes \
		     ac_cv_search_yp_get_default_domain=no \
		     ./configure --with-fhs --prefix=${PREFIX_DIR} \
		        --sysconfdir=/etc \
			--without-included-regex \
			--with-libsmbclient=yes  \
			--without-ldap \
		 	--without-kerberos \
		 	--without-smbd \
			--without-nmbd \
 			--disable-cups \
			--without-ad \
			--without-getline \
			--without-smbd \
			--without-nmbd \
			--without-printing \
			--with-winbind \
			--disable-man \
			--disable-swat \
		        --host=${CROSS} CFLAGS="${CFLAGS}" LDFLAGS=${TOOLCHAIN_ABI} #&& patch -p0 < ${STORAGE}/samba-3.0.22_build.patch

.PHONY: install
install: ${DIR}
	${MAKE} -C ${DIR} install
	mkdir -p ${ROOT}/usr/lib/
	cp -f ${PREFIX_DIR}/lib/samba/libsmbclient.so ${LIBDIR}
	cd ${LIBDIR} && ln -s -f libsmbclient.so libsmbclient.so.0
	rm -rf ${PREFIX_DIR}
${DIR}:
	tar -zxf ${STORAGE}/${PKG}-${VERSION}.tar.gz
	cd ${DIR} && patch -p0 < ${STORAGE}/samba-3.0.22_configure.patch
################### NK-modify start ###################
# purpose:   Build Machine  # author:    Chihmou
# date:      2010-07-25   # description: add NK Patch
	cd ${DIR} && patch -p0 < ${STORAGE}/samba-3.0.22_nk.patch
###################  NK-modify end  ###################
