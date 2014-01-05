
include ../pkg_addon/nkuserland.config

.PHONY: install
install:
	cat ${STORAGE}/device-files.tgz | (cd ${ROOT}; sudo tar -zx)
	sudo mknod ${ROOT}/dev/mtd0   c 90 0
	sudo mknod ${ROOT}/dev/mtd0ro c 90 1
	sudo mknod ${ROOT}/dev/mtd1   c 90 2
	sudo mknod ${ROOT}/dev/mtd1ro c 90 3
	sudo mknod ${ROOT}/dev/mtd2   c 90 4
	sudo mknod ${ROOT}/dev/mtd2ro c 90 5
	sudo mknod ${ROOT}/dev/mtd3   c 90 6
	sudo mknod ${ROOT}/dev/mtd3ro c 90 7
	sudo mknod ${ROOT}/dev/mtd4   c 90 8
	sudo mknod ${ROOT}/dev/mtd4ro c 90 9
	sudo mknod ${ROOT}/dev/mtd5   c 90 10
	sudo mknod ${ROOT}/dev/mtd5ro c 90 11
	sudo mknod ${ROOT}/dev/mtd6   c 90 12
	sudo mknod ${ROOT}/dev/mtd6ro c 90 13
	sudo mknod ${ROOT}/dev/mtd7   c 90 14
	sudo mknod ${ROOT}/dev/mtd7ro c 90 15
	sudo mknod ${ROOT}/dev/mtdblock0 b 31 0
	sudo mknod ${ROOT}/dev/mtdblock1 b 31 1
	sudo mknod ${ROOT}/dev/mtdblock2 b 31 2
	sudo mknod ${ROOT}/dev/mtdblock3 b 31 3
	sudo mknod ${ROOT}/dev/mtdblock4 b 31 4
	sudo mknod ${ROOT}/dev/mtdblock5 b 31 5
	sudo mknod ${ROOT}/dev/mtdblock6 b 31 6
	sudo mknod ${ROOT}/dev/mtdblock7 b 31 7

ifeq "$(CONFIG_NK_CRAMFS)" "y"
	sudo mknod ${ROOT}/dev/nk_esp c 240 1
	sudo mknod ${ROOT}/dev/nk_switch c 237 1
	sudo mknod ${ROOT}/dev/eth_acc c 238 0
	sudo mknod ${ROOT}/dev/octcrypto c 125 0
	sudo mknod ${ROOT}/dev/url c 241 0
	sudo mknod ${ROOT}/dev/tmufd c 245 0
	ln -sf /var/run/log ${ROOT}/dev/log
endif
