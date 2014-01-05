
PKG:=ntp
VERSION:=4.2.2
DIR:=${PKG}-${VERSION}

################### NK-modify start ###################
# purpose:   Build Machine  # author:    Chihmou
# date:      2010-07-25   # description: Change PREFIX_DIR path
#PREFIX_DIR:=/tmp/cavium/ntp
PREFIX_DIR:=${ROOT}/../ntp
###################  NK-modify end  ###################

.PHONY: all
all: build install

.PHONY: build
build: ${DIR} ${DIR}/Makefile
	${MAKE} -C ${DIR} CFLAGS="${CFLAGS}" -j4

${DIR}/Makefile:
	cd ${DIR} && ./configure --prefix=${PREFIX_DIR} --sysconfdir=${ROOT}/etc --host=${CROSS} CFLAGS="${CFLAGS}" LDFLAGS=${TOOLCHAIN_ABI}

.PHONY: install
install: ${DIR}
	${MAKE} -C ${DIR} install
	mkdir -p ${ROOT}/usr/bin/
	cp -f ${PREFIX_DIR}/bin/ntpd ${ROOT}/bin
	cp -f ${PREFIX_DIR}/bin/ntpdate ${ROOT}/bin
	rm -rf ${PREFIX_DIR}

${DIR}:
	tar -zxf ${STORAGE}/${PKG}-${VERSION}.tar.gz
