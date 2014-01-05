
PKG:=flex
VERSION:=2.5.4a
DIR:=${PKG}-2.5.4

.PHONY: all
all: build install

.PHONY: build
build: ${DIR} ${DIR}/Makefile
	${MAKE} -C ${DIR}

${DIR}/Makefile:
	cd ${DIR} && ./configure --prefix=${ROOT}/usr --host=${CROSS}

.PHONY: install
install:
	${MAKE} -C ${DIR} install
	rm ${ROOT}/usr/bin/flex

${DIR}:
	tar -zxf ${STORAGE}/${PKG}-${VERSION}.tar.gz
	
