MOUNT_POINT=/mnt/cf1
MOUNT_POINT2=/mnt/cf2
KERNEL_VERSION=${shell make -s -C kernel_2.6/linux kernelrelease}
TFTPBOOT=/tftpboot
TOOLDIR=./embedded_rootfs/pkg_addon/tool
TFTPDIR=./kernel_2.6/linux


######################################################################
# Tell the user what to do
######################################################################
.DEFAULT: help
.PHONY: help
.SILENT: help
help:
	echo "Supply the build target:"
	echo "    kernel	- Build the Linux kernel supporting all Cavium Octeon reference boards"
	echo "    kernel-deb	- Linux kernel without the rootfs"
	echo "    sim     	- Octeon simulation environment"
	echo "    flash   	- Copy kernel onto compact flash at mount ${MOUNT_POINT}"
	echo "    strip   	- Strip symbols out of the kernel image"
	echo "    tftp    	- Copy a stripped kernel to ${TFTPBOOT}"
	echo "    test    	- Test an existing simulator build"
	echo "    clean   	- Remove all generated files and the KERNEL CONFIG"
	echo

include ./embedded_rootfs/pkg_addon/nkuserland.config

######################################################################
# Kernel build targets
######################################################################
.PHONY: sim
sim: embedded_rootfs/.config
	if [ ! -e kernel_2.6/linux/.config ]; then cp kernel_2.6/sim.config kernel_2.6/linux/.config; fi
	- rm -f kernel_2.6/linux/usr/initramfs_data.cpio*
	${MAKE} -C kernel_2.6
	${MAKE} -C embedded_rootfs all ext3
	${MAKE} -C kernel_2.6

.PHONY: kernel
kernel: embedded_rootfs/.config
	if [ ! -e kernel_2.6/linux/.config ]; then cp kernel_2.6/kernel.config kernel_2.6/linux/.config; fi
	${MAKE} -C kernel_2.6
ifeq "$(CONFIG_NK_CRAMFS)" "y"
	${MAKE} -C embedded_rootfs all cramfs
else
	${MAKE} -C embedded_rootfs all initramfs
	${MAKE} -C kernel_2.6
endif

######################################################################
#--------------- build image ---------------
######################################################################

ifeq "$(CONFIG_NK_CRAMFS)" "y"
	mips64-octeon-linux-gnu-strip $(TFTPDIR)/vmlinux.64
	$(TOOLDIR)/buildimage -k $(TFTPDIR)/vmlinux.64 -r embedded_rootfs/rootfs.cramfs -i $(TFTPDIR)/code.bin
else
	$(TOOLDIR)/buildimage -k $(TFTPDIR)/vmlinux.64 -i $(TFTPDIR)/code.bin
endif
	$(TOOLDIR)/addchecksum -i $(TFTPDIR)/code.bin
	chmod 644 $(TFTPDIR)/code.bin


.PHONY: kernel-deb
kernel-deb:
	if [ ! -e kernel_2.6/linux/.config ]; then cp kernel_2.6/kernel.config kernel_2.6/linux/.config; fi
	- rm -f kernel_2.6/linux/usr/initramfs_data.cpio*
	${MAKE} -C kernel_2.6
	${MAKE} -C kernel_2.6/cavium-ethernet

.PHONY: debian
debian: kernel-deb

.PHONY: ebt3000
ebt3000: kernel


######################################################################
# If the embedded filesystem isn't configured, start menuconfig
######################################################################
embedded_rootfs/.config:
	cp embedded_rootfs/default.config embedded_rootfs/.config
	make -sC embedded_rootfs oldconfig
	
	

######################################################################
# Targets to package the final kernel
######################################################################
.PHONY: flash
flash:
	# This target assumes the compact flash is already setup. Use
	# debian/Makefile to get this setup the first time.
	- umount ${MOUNT_POINT} &> /dev/null
	mount ${MOUNT_POINT}
	mips64-octeon-linux-gnu-strip -o ${MOUNT_POINT}/vmlinux.64 kernel_2.6/linux/vmlinux.64
	umount ${MOUNT_POINT}
	- umount ${MOUNT_POINT2} &> /dev/null
	mount ${MOUNT_POINT2}
	sudo mkdir -p ${MOUNT_POINT2}/lib/modules/${KERNEL_VERSION}
	if [ -e kernel_2.6/cavium-ethernet/cavium-ethernet.ko ]; then sudo cp kernel_2.6/cavium-ethernet/cavium-ethernet.ko ${MOUNT_POINT2}/lib/modules/${KERNEL_VERSION}/; fi
	if [ -e kernel_2.6/intercept-example/intercept-example.ko ]; then sudo cp kernel_2.6/intercept-example/intercept-example.ko ${MOUNT_POINT2}/lib/modules/${KERNEL_VERSION}/; fi
	if [ "`find kernel_2.6/linux -name "*.ko"`" != "" ]; then cd kernel_2.6/linux && find . -name "*.ko" | xargs sudo cp --parents --target-directory=${MOUNT_POINT2}/lib/modules/${KERNEL_VERSION}; fi
	umount ${MOUNT_POINT2}

.PHONY: strip
strip:
	mips64-octeon-linux-gnu-strip kernel_2.6/linux/vmlinux.64

.PHONY: tftp
tftp:
	mips64-octeon-linux-gnu-strip -o ${TFTPBOOT}/vmlinux.64 kernel_2.6/linux/vmlinux.64

.PHONY: pci
pci:
	oct-pci-reset
	oct-pci-load 0x21000000 kernel_2.6/linux/vmlinux.64
	oct-pci-bootcmd "bootoctlinux 21000000 root=/dev/cfa2 numcores=16"


######################################################################
# Test the kernel on the simulator
######################################################################
.PHONY: test
test:
	${MAKE} -f Makefile.test
	

######################################################################
# Clean out everything
######################################################################
.PHONY: clean
clean:
#	${MAKE} -C embedded_rootfs clean
	${MAKE} -C kernel_2.6 clean
	${MAKE} -C kernel_2.6/cavium-ethernet clean
	${MAKE} -C kernel_2.6/intercept-example clean
	${MAKE} -f Makefile.test clean
	echo "Removing the kernel config!"
	${MAKE} -C kernel_2.6 distclean
	echo "Removing the Embedded Rootfs config!"
	${MAKE} -C embedded_rootfs distclean
	
