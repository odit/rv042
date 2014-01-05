
include ../.config

PKG:=busybox
VERSION:=1.2.1
DIR:=${PKG}-${VERSION}
BUSYBOX_LINKS=`xargs < ${DIR}/busybox.links`
TESTSUITE=${ROOT}/examples/busybox-testsuite

.PHONY: all
all: build install

.PHONY: build
build: ${DIR}
	${MAKE} -C ${DIR} EXTRA_CFLAGS="${TOOLCHAIN_ABI}" LDFLAGS=""

.PHONY: install
install: ${DIR}
	mkdir -p ${ROOT}/bin
	mkdir -p ${ROOT}/sbin
	mkdir -p ${ROOT}/usr/bin
	mkdir -p ${ROOT}/usr/sbin
	mkdir -p ${ROOT}/proc
	mkdir -p ${ROOT}/sys
	mkdir -p ${ROOT}/mnt
	mkdir -p ${ROOT}/tmp
	mkdir -p ${ROOT}/var
	mkdir -p ${ROOT}/etc
	mkdir -p ${ROOT}/root
	mkdir -p ${ROOT}/lib/modules
	mkdir -p ${ROOT}/home
	${STRIP} -o ${ROOT}/bin/busybox ${DIR}/busybox
	for link in ${BUSYBOX_LINKS}; do ln -s /bin/busybox ${ROOT}$$link; done
	# The following link is needed for initramfs based boots
	ln -s /sbin/init ${ROOT}/init
	cp ${SOURCE_DIR}/cpu-load ${ROOT}/usr/bin/
ifdef CFG_BUSYBOX_TESTSUITE
	mkdir -p ${ROOT}/examples
	cp ${DIR}/.config ${ROOT}/examples/
	cp -R ${DIR}/testsuite ${TESTSUITE}
	cp ${SOURCE_DIR}/busybox-runtest ${TESTSUITE}/runtest
	rm -r ${TESTSUITE}/bunzip2				# No bzip2 available
	rm -r ${TESTSUITE}/du					# Doesn't test anything
	rm -r ${TESTSUITE}/wget					# Networking may not be up
	rm ${TESTSUITE}/cp/cp-a-files-to-dir 			# Unsupported touch option
	rm ${TESTSUITE}/cp/cp-does-not-copy-unreadable-file 	# Doesn't work because root can read
	rm ${TESTSUITE}/mv/mv-files-to-dir			# Unsupported touch option
	rm ${TESTSUITE}/mv/mv-refuses-mv-dir-to-subdir		# Unsupported touch option
	rm ${TESTSUITE}/touch/touch-touches-files-after-non-existent-file			# touch -t doesn't work
	sed -i "s:../../busybox:/bin/busybox:g" ${TESTSUITE}/strings/strings-works-like-GNU	# Fix busybox path
	sed -i "s:BUSYBOX=.*:BUSYBOX=/bin/busybox:" ${TESTSUITE}/which/which-uses-default-path	# Fix busybox path
endif	

${DIR}:
	tar -jxf ${STORAGE}/${PKG}-${VERSION}.tar.bz2
	cd ${DIR} && patch -p0 < ${STORAGE}/busybox-udhcpcd-64bit.patch
	cp ${STORAGE}/busybox.config ${DIR}/.config
ifdef CFG_ENABLE_IPV6	
	sed -i "s/# CONFIG_FEATURE_IPV6 is not set/CONFIG_FEATURE_IPV6=y/" ${DIR}/.config
	sed -i "s/# CONFIG_PING6 is not set/CONFIG_PING6=y/" ${DIR}/.config
	sed -i "s/# CONFIG_FEATURE_FANCY_PING6 is not set/CONFIG_FEATURE_FANCY_PING6=y/" ${DIR}/.config
endif
	${MAKE} -C ${DIR} oldconfig > /dev/null
	
