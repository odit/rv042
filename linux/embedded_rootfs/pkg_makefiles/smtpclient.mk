
PKG:=smtpclient
VERSION:=1.0.0
DIR:=${PKG}-${VERSION}

.PHONY: all
all: build install

.PHONY: build
build: ${DIR} ${DIR}/Makefile
	${MAKE} -C ${DIR} -j4

${DIR}/Makefile:
	cd ${DIR} && ./configure --prefix=${ROOT}/usr --host=${CROSS}

.PHONY: install
install: ${DIR}
	${STRIP} -s ${DIR}/smtpclient
	cp ${DIR}/smtpclient ${ROOT}/bin/

${DIR}:
	tar -zxf ${STORAGE}/${PKG}-${VERSION}.tar.gz
	cd ${DIR} && cp -rf ${STORAGE}/smtpclient-1.0.0_Makefile.in.patch ./Makefile.in
	cd ${DIR} && patch -p0 b64.c ${STORAGE}/smtpclient-1.0.0_b64.c.patch
	cd ${DIR} && patch -p0 b64.h ${STORAGE}/smtpclient-1.0.0_b64.h.patch
	cd ${DIR} && patch -p0 client.c ${STORAGE}/smtpclient-1.0.0_client.c.patch
	cd ${DIR} && patch -p0 client.h ${STORAGE}/smtpclient-1.0.0_client.h.patch
	cd ${DIR} && patch -p0 < ${STORAGE}/smtpclient_config.patch
