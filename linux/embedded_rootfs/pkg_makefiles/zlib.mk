
include ../.config

PKG:=zlib
VERSION:=1.2.3
DIR:=${PKG}-${VERSION}

ifdef TOOLCHAIN_ABI_N32
  export LIBDIR=${ROOT}/usr/lib32
endif

ifdef TOOLCHAIN_ABI_64
  export LIBDIR=${ROOT}/usr/lib64
endif

.PHONY: all
all: build install

.PHONY: build
build: ${DIR}
	export -n CFLAGS && cd ${DIR} && ./configure --shared --prefix=${ROOT}/usr
	${MAKE} -C ${DIR} CC=${CC} CFLAGS="${CFLAGS}" AR="${AR} rc" LDSHARED="${CC} ${CFLAGS} -shared -Wl,-soname,libz.so.1"


.PHONY: install
install: ${DIR}
	${MAKE} -C ${DIR} install libdir=${LIBDIR}

${DIR}:
	tar -jxf ${STORAGE}/${PKG}-${VERSION}.tar.bz2
	
