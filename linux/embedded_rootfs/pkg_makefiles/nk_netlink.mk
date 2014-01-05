
.PHONY: all
all: install

.PHONY: build
build:
	${MAKE} -C ${KERNEL_DIR}/../nk_netlink

.PHONY: install
install:
	mkdir -p ${ROOT}/lib/modules
	cp ${KERNEL_DIR}/../nk_netlink/nk_netlink.ko ${ROOT}/lib/modules/

