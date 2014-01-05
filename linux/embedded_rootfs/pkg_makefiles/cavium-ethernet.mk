
.PHONY: all
all: install

.PHONY: build
build:
	${MAKE} -C ${KERNEL_DIR}/../cavium-ethernet

.PHONY: install
install:
	mkdir -p ${ROOT}/lib/modules
	cp ${KERNEL_DIR}/../cavium-ethernet/cavium-ethernet.ko ${ROOT}/lib/modules/

