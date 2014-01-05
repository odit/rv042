
PKG:=pppd_small
DIR:=${PKG}

.PHONY: all
all: build install

.PHONY: build
build: ${DIR} 
	${MAKE} -C ${DIR} CC="${CC} ${CFLAGS}" -j4

.PHONY: install
install: ${DIR}
	mkdir -p ${ROOT}/etc/ppp
	cp ${DIR}/etc.ppp/options ${ROOT}/etc/ppp
	cp ${DIR}/pppd/pppd ${ROOT}/sbin/

${DIR}:
	tar -xzf ${STORAGE}/${PKG}.tar.gz && patch -p0 < ${STORAGE}/pppd-makefile.patch


