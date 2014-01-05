
PKG:=logrotate
DIR:=${PKG}

.PHONY: all
all: build install

.PHONY: build
build: ${DIR}
	${MAKE} -C ${DIR} CC="${CC} ${CFLAGS}"

.PHONY: install
install: ${DIR}
	mkdir -p ${ROOT}/sbin
	${MAKE} -C ${DIR} CP=cp DESTDIR=${ROOT} install 

${DIR}:
	tar -xvzf ${STORAGE}/${PKG}.tar.gz
