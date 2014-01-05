
PKG:=mtd
VERSION:=20050122.orig
DIR:=${PKG}-${VERSION}

LIBSEARCH=-L ${shell pwd}/zlib-1.2.3

BINARIES = ftl_format flash_erase flash_eraseall nanddump doc_loadbios \
	mkfs.jffs ftl_check mkfs.jffs2 flash_lock flash_unlock \
	flash_info mtd_debug flashcp nandwrite jffs2dump jffs3dump \
	nftldump nftl_format docfdisk \
	sumtool #jffs2reader

.PHONY: all
all: build install

.PHONY: build
build: ${DIR}
	${MAKE} -C ${DIR}/util CROSS=${CROSS}- CFLAGS="${CFLAGS} -I ../include -I ${ROOT}/usr/include" LDFLAGS="${CFLAGS} ${LIBSEARCH}"

.PHONY: install
install: ${BINARIES:%=%-install}

.PHONY: ${BINARIES:%=%-install}
${BINARIES:%=%-install}:
	${STRIP} -o ${ROOT}/usr/bin/${@:%-install=%} ${DIR}/util/${@:%-install=%}

${DIR}:
	tar -zxf ${STORAGE}/${PKG}_${VERSION}.tar.gz
	sed -i "s/static int target_endian/int target_endian/g" ${DIR}/util/*.c
	
