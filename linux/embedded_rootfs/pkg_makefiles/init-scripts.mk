
include ../pkg_addon/nkuserland.config

.PHONY: install
install:
	cp ${ETC_FILES}/inittab 	${ROOT}/etc/
	cp ${ETC_FILES}/fstab 		${ROOT}/etc/
	cp ${ETC_FILES}/rc    		${ROOT}/sbin/
	cp ${ETC_FILES}/passwd		${ROOT}/etc/
	cp ${ETC_FILES}/group		${ROOT}/etc/
	cp ${ETC_FILES}/shadow 		${ROOT}/etc/
ifeq "$(CONFIG_RV0XX)" "y"
	cp ${ETC_FILES}/shadow-cisco 		${ROOT}/etc/shadow
endif
	cp ${ETC_FILES}/rc.local 	${ROOT}/sbin/
	cp ${ETC_FILES}/rc.firewall 	${ROOT}/etc/
	cp ${ETC_FILES}/services 	${ROOT}/etc/
	cp ${ETC_FILES}/logrotate.conf 	${ROOT}/etc/
	cp ${ETC_FILES}/syslog.conf 	${ROOT}/etc/
	ln -s /proc/mounts ${ROOT}/etc/mtab
	mkdir -p ${ROOT}/usr/share/udhcpc/
	cp ${ETC_FILES}/udhcpc.script   ${ROOT}/usr/share/udhcpc/
	cp ${ETC_FILES}/udhcpc.renew    ${ROOT}/usr/share/udhcpc/
	cp ${ETC_FILES}/udhcpc.nak      ${ROOT}/usr/share/udhcpc/
	cp ${ETC_FILES}/udhcpc.deconfig ${ROOT}/usr/share/udhcpc/
	cp ${ETC_FILES}/udhcpc.bound    ${ROOT}/usr/share/udhcpc/
	ln -s udhcpc.script		${ROOT}/usr/share/udhcpc/default.script
	mkdir -p ${ROOT}/var/run
	cp ${ETC_FILES}/udhcpd.conf     ${ROOT}/etc/
	mkdir -p ${ROOT}/var/lib/misc; touch ${ROOT}/var/lib/misc/udhcpd.leases

