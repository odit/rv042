

.PHONY: all
all: build install

.PHONY: build
build:
	cd ${OCTEON_ROOT}/components/crypto-api/core/cryptolinux && ${MAKE}

.PHONY: install
install:
	mkdir -p ${ROOT}/lib/modules
	cp ${OCTEON_ROOT}/components/crypto-api/core/cryptolinux/cavmodexp.ko ${ROOT}/lib/modules/

