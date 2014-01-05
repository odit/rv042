
PKG:=sysklogd
VERSION:=1.4.1
DIR:=${PKG}-${VERSION}

.PHONY: all
all: build install

.PHONY: build
build: ${DIR}
	${MAKE} -C ${DIR} CC="${CC} ${CFLAGS}"

.PHONY: install
install: ${DIR}
	mkdir -p ${ROOT}/sbin
	cp -f ${DIR}/syslogd ${ROOT}/sbin/syslogd
	cp -f ${DIR}/klogd ${ROOT}/sbin/klogd

${DIR}:
	tar -xvzf ${STORAGE}/${PKG}-${VERSION}.tar.gz
	cd ${DIR}/.. && patch -p0 < ${STORAGE}/sysklogd-1.4.1-fixes-1.patch
	cd ${DIR} && patch -p0 < ${STORAGE}/syslog.patch
