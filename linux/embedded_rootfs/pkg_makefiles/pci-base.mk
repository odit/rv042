
PKG:=pci-base
VERSION:=0.9.3-36
DIR:=${PKG}-${VERSION}

.PHONY: all
all: build 

.PHONY: build
build:	${DIR}
	cd ${OCTEON_ROOT}/components/driver/host/api && ${MAKE} clean && ${MAKE}


${DIR}:
	cd ${OCTEON_ROOT} && tar -zxf ${STORAGE}/${PKG}-${VERSION}.tar.gz 
