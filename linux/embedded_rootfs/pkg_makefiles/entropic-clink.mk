
PKG:=entropic-clink
VERSION:=AC2.11_alpha_64_bit-20070119
DIR:=${shell pwd}/${PKG}-${VERSION}

.PHONY: install
install:
	mkdir ${DIR}; cd ${DIR}; tar -zxf ${STORAGE}/${PKG}-${VERSION}.tgz
	mkdir -p ${ROOT}/etc
	cp ${DIR}/clink-he.conf ${ROOT}/etc/
	cp ${DIR}/clink-cpe.conf ${ROOT}/etc/
	mkdir -p ${ROOT}/usr/bin
	cp ${DIR}/clinkd-he ${ROOT}/usr/bin/
	cp ${DIR}/clnkstat-he ${ROOT}/usr/bin/
	cp ${DIR}/acch-he ${ROOT}/usr/bin/
	cp ${DIR}/clinkd-cpe ${ROOT}/usr/bin/
	cp ${DIR}/clnkstat-cpe ${ROOT}/usr/bin/
	cp ${DIR}/acch-cpe ${ROOT}/usr/bin/
	mkdir -p ${ROOT}/lib/modules
	cp ${DIR}/clnkdvr-he.ko ${ROOT}/lib/modules/
	cp ${DIR}/clnkdvr-cpe.ko ${ROOT}/lib/modules/
