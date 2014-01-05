
PKG:=components-common
VERSION:=1.3.1-19
DIR:=${PKG}-${VERSION}

.PHONY: all
all: build 

.PHONY: build
build:	${DIR}

${DIR}:
	cd ${OCTEON_ROOT} && tar -zxf ${STORAGE}/${PKG}-${VERSION}.tar.gz 
