BASE=${shell pwd}
BUILD=${BASE}/build
MOUNT_POINT=/mnt/${USER}/linux_dev

-include .config

######################################################################
# These variables are exported to package makes to make life easier
######################################################################

################### NK-modify start ###################
# purpose:   Build Machine  # author:    Chihmou
# date:      2010-07-25   # description: Change ROOT path
#export ROOT=/tmp/${USER}-rootfs
export ROOT=${shell test -d /usr/local/cavium && echo "/tmp/${USER}-rootfs" || echo "${BASE}/tmp/${USER}-rootfs"}
###################  NK-modify end  ###################
export STORAGE=${BASE}/storage
export SOURCE_DIR=${BASE}/source
export ETC_FILES=${BASE}/etc-files
export KERNEL_DIR=${BASE}/../kernel_2.6/linux
export CROSS=mips64-octeon-linux-gnu
export CC=${CROSS}-gcc
export CXX=${CROSS}-g++
export LD=${CROSS}-ld
export AR=${CROSS}-ar
export RANLIB=${CROSS}-ranlib
export STRIP=${CROSS}-strip
export CFLAGS=${TOOLCHAIN_ABI} -Os -fomit-frame-pointer
export CXXFLAGS=${TOOLCHAIN_ABI} -Os -fomit-frame-pointer

#---------------
export CP = cp
export DESTBIN = ${ROOT}/bin
export DESTETC = ${ROOT}/etc
export DESTINFO = ${ROOT}/info
export DESTLIBEXEC = ${ROOT}/libexec
export DESTSBIN = ${ROOT}/sbin
export DESTMAN = ${ROOT}/man
export DESTSHARE = ${ROOT}/share
export DESTMODULE = ${ROOT}/modules
export DESTVAR = ${ROOT}/var
export DESTWWW = ${ROOT}/www
export DESTMAN = ${ROOT}/man
export DESTETC = ${ROOT}/etc
export DESTLIB = ${ROOT}/lib
export DESTUSR = ${ROOT}/usr
export DESTOPT = ${ROOT}/opt
export DESTOPTMICROCODE = ${DESTOPT}/microcode
export DESTOPTSAMPLE = ${DESTOPT}/samples
export DESTHOME = ${ROOT}/home
export DESTHOMECAVIUM = ${DESTHOME}/cavium
export DESTOPTAIRGO = ${DESTOPT}/airgo
export DESTOPTATHEROS = ${DESTOPT}/atheros
#---------------

ifdef TOOLCHAIN_ABI_N32
  export TOOLCHAIN_ABI=-mabi=n32
  export LDFLAGS=-melf32btsmipn32
  export LIBDIR=/usr/lib32
endif

ifdef TOOLCHAIN_ABI_64
  export TOOLCHAIN_ABI=-mabi=64
  export LDFLAGS=-melf64btsmip
  export LIBDIR=/usr/lib64
endif

ifdef TOOLCHAIN_UCLIBC
  export TOOLCHAIN_ABI=-muclibc
  export LDFLAGS=-melf32btsmipn32
  export LIBDIR=/uclibc/usr/lib
endif

######################################################################
# Create a list of all possible packages from all the *.mk files
######################################################################

ALL_PACKAGES_MK=${shell find ${BASE}/pkg_makefiles -name "*.mk" -printf "%f "}
ALL_PACKAGES=${ALL_PACKAGES_MK:.mk=}

######################################################################
# Create a list of the configured packages from the .config file
######################################################################

PACKAGES=${shell if [ -e .config ]; then grep CONFIG_.*=y .config | sed "s/CONFIG_//" | sed "s/=y//";fi}

######################################################################
# Default target - Give the user some help
######################################################################

.DEFAULT: help
.SILENT: help
.PHONY: help
help:
	echo
	echo
	echo "menuconfig        - Configure the packages for the filesystem (GUI)"
	echo "config            - Configure the packages for the filesystem (Command line)"
	echo "oldconfig         - Check the existing config file"
	echo "all               - Build the filessytem"
	echo "squashfs          - Package the filesystem into Squashfs"
	echo "cramfs            - Package the filesystem into Cramfs"
	echo "ext3              - Package the filesystem into Ext3"
	echo "initramfs         - Use the CPIO archive as an initramfs"
	echo "clean             - Delete the builds and all filesystem files"
	echo "clean-root        - Delete the filesystem files leaving the builds"
	echo "distclean         - Delete all generated files, including the config"
	echo
	echo "Currently configured packages:"
	echo ${PACKAGES}
	echo

######################################################################
# The all target does the actual building of the packages and creation
# of the filesystem. The .config file must have already been
# generated. The list of packaged was created earlier by searching for
# CONFIG-*=y in the configuration file.
######################################################################

include ./pkg_addon/nkuserland.config

.PHONY: all
all: configured .root_complete real nk_switch cavium-ethernet nk_netlink

.root_complete: .config ${BASE}/pkg_kconfig/*.kconfig ${BASE}/pkg_makefiles/*.mk
	${MAKE} force-build

.PHONY: force-build
force-build: configured clean-root ${PACKAGES}
	touch .root_complete

.SILENT: configured
.PHONY: configured
configured:
	if [ -e .config ]; then true; else echo "Not configured. Run \"make menuconfig\"" && false; fi
ifeq "$(CONFIG_NK_CRAMFS)" "y"
	cp -f ${BASE}/pkg_makefiles/device-files.mk-cramfs ${BASE}/pkg_makefiles/device-files.mk
else
	cp -f ${BASE}/pkg_makefiles/device-files.mk-initramfs ${BASE}/pkg_makefiles/device-files.mk
endif

.PHONY: ${ALL_PACKAGES}
${ALL_PACKAGES}: ${ROOT} ${BUILD}
	@echo "**** $@"
	${MAKE} -C ${BUILD} -f ${BASE}/pkg_makefiles/$@.mk

test:
	${MAKE} -C pkg_addon/nku install
.PHONY: real
real:
	${MAKE} -C pkg_addon
.PHONY: nk_switch
nk_switch:
	@echo "**** nk_switch"
	${MAKE} -C ${BUILD} -f ${BASE}/pkg_makefiles/nk_switch.mk
.PHONY: cavium-ethernet
cavium-ethernet:
	@echo "**** cavium-ethernet"
	${MAKE} -C ${BUILD} -f ${BASE}/pkg_makefiles/cavium-ethernet.mk
.PHONY: nk_netlink
nk_netlink:
	@echo "**** nk_netlink"
	${MAKE} -C ${BUILD} -f ${BASE}/pkg_makefiles/nk_netlink.mk

clean-real:
	${MAKE} -C pkg_addon clean

######################################################################
# Targets for cleaning up
######################################################################

.PHONY: clean
clean: clean-root clean-real
	rm -rf ${BUILD} .kconfig_include .config.* .tmpconfig.h .config
	$(MAKE) CC=gcc CFLAGS=-Os LDFLAGS="" -C config clean

.PHONY: clean-root
clean-root:
	rm -f .root_complete rootfs.cpio rootfs.cpio.gz rootfs.ext3  rootfs.squashfs rootfs.cramfs
	sudo rm -rf ${ROOT}

.PHONY: distclean
distclean: clean
	rm -f .config

${BUILD} ${ROOT}:
	mkdir -p $@
	chmod 777 $@

######################################################################
# Targets for supporting menuconfig
######################################################################

config/conf config/mconf: .kconfig_include config/Makefile
	$(MAKE) CC=gcc CFLAGS=-Os LDFLAGS="" -C config ncurses conf mconf

.PHONY: menuconfig
menuconfig: config/mconf
	@./config/mconf embedded_rootfs.kconfig

.PHONY: config
config: config/conf
	@./config/conf embedded_rootfs.kconfig

.PHONY: oldconfig
oldconfig: config/conf
	@./config/conf -s embedded_rootfs.kconfig

.PHONY: .kconfig_include
.kconfig_include: ${BASE}/pkg_kconfig/*.kconfig
	find ${BASE}/pkg_kconfig -name "*.kconfig" -printf "source \"pkg_kconfig/%f\"\n" | sort > $@

######################################################################
# Targets for packaging the filesystem
######################################################################

${BASE}/rootfs.cpio:
	cd ${ROOT} && sudo find . | sudo cpio --format=newc -oF /tmp/${USER}-rootfs.cpio
	sudo chmod a+r /tmp/${USER}-rootfs.cpio
	cp /tmp/${USER}-rootfs.cpio ${BASE}/rootfs.cpio
	sudo /bin/rm -f /tmp/${USER}-rootfs.cpio

${BASE}/rootfs.cpio.gz: ${BASE}/rootfs.cpio
	gzip -9 < ${BASE}/rootfs.cpio > ${BASE}/rootfs.cpio.gz

cpio_creat:
	cd ${ROOT} && sudo find . | sudo cpio --format=newc -oF /tmp/${USER}-rootfs.cpio
	sudo chmod a+r /tmp/${USER}-rootfs.cpio
	cp /tmp/${USER}-rootfs.cpio ${BASE}/rootfs.cpio
	sudo /bin/rm -f /tmp/${USER}-rootfs.cpio

cpio_zip:
	gzip -9 < ${BASE}/rootfs.cpio > ${BASE}/rootfs.cpio.gz

.PHONY: squashfs
squashfs:
	mksquashfs ${ROOT} rootfs.squashfs -be -noappend
	chmod -x rootfs.squashfs

.PHONY: cramfs
cramfs:
	find ${ROOT} -name CVS | xargs rm -rf
	find ${ROOT} -name .svn | xargs rm -rf
	./pkg_addon/tool/cramfs-1.1/mkcramfs -b ${ROOT} rootfs.cramfs

.PHONY: ext3
ext3: ${BASE}/rootfs.cpio
	sudo mkdir -p ${MOUNT_POINT}
	- sudo umount ${MOUNT_POINT} &> /dev/null
	# Create the filesystem as the size of the CPIO plus 16MB
	dd if=/dev/zero of=/tmp/${USER}-rootfs.ext3 bs=1M count=$$(($$(stat -c %s ${BASE}/rootfs.cpio) / 1048576 + 16))
	/sbin/mke2fs -qjF /tmp/${USER}-rootfs.ext3
	sudo mount -o loop /tmp/${USER}-rootfs.ext3 ${MOUNT_POINT}
	sudo cp -R ${ROOT}/. ${MOUNT_POINT}/
	sudo umount ${MOUNT_POINT}
	mv /tmp/${USER}-rootfs.ext3 rootfs.ext3

.PHONY: initramfs	
initramfs: 
	find ${ROOT} -name CVS | xargs rm -rf
	${MAKE} cpio_creat
	${MAKE} cpio_zip
	cp ${BASE}/rootfs.cpio ${KERNEL_DIR}/usr/initramfs_data.cpio
	cp ${BASE}/rootfs.cpio.gz ${KERNEL_DIR}/usr/initramfs_data.cpio.gz

