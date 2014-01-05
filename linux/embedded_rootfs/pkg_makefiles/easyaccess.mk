
PKG:=EasyAccess
DIR:=${PKG}
#CFLAGS:="${TOOLCHAIN_ABI}"
LDFLAGS="${TOOLCHAIN_ABI}" 

.PHONY: all
all: build install

.PHONY: build
build: ${DIR}
	cd ${DIR} && CONFIG_EPS="n" STATISTICS="n" ./RUN.Octeon
	${MAKE} -C ${DIR} PREFIX=${ROOT}/usr/local/EasyAccess -j4

.PHONY: install
install: ${DIR}
	cd ${DIR}/Octeon && cp embedded_rootfs.EA/etc-files/rc.easyaccess ${ROOT}/sbin/
	cd ${DIR}/Octeon && cp embedded_rootfs.EA/etc-files/rc.firewall ${ROOT}/etc/
	cd ${DIR}/Octeon && cp embedded_rootfs.EA/etc-files/mime.types 	${ROOT}/etc/
	cd ${DIR}/Octeon && cp embedded_rootfs.EA/etc-files/syslog.conf ${ROOT}/etc/
	cd ${DIR}/Octeon && cp embedded_rootfs.EA/etc-files/logrotate.conf ${ROOT}/etc/
	cd ${DIR}/Octeon && echo "rc.easyaccess" >> ${ROOT}/sbin/rc
	cd ${DIR}/Octeon && cat embedded_rootfs.EA/etc-files/fstab.EA >> ${ROOT}/etc/fstab
	cd ${DIR}/Octeon && cat embedded_rootfs.EA/etc-files/services.EA >> ${ROOT}/etc/services
	${MAKE} -C ${DIR} PREFIX=${ROOT}/usr/local/EasyAccess  install

${DIR}:
	tar -zxf ${STORAGE}/${PKG}-Config.tar.gz
	tar -zxf ${STORAGE}/${PKG}.tar.gz
