
.PHONY: all
all: install

.PHONY: build
build:
	${MAKE} -C ${KERNEL_DIR}/../nk_switch

.PHONY: install
install:
	mkdir -p ${ROOT}/lib/modules
	cp ${KERNEL_DIR}/../nk_switch/nk_switch.ko ${ROOT}/lib/modules/

