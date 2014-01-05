
.PHONY: all
all: build install

.PHONY: build
build:
	${MAKE} -C ${KERNEL_DIR} modules
	
.PHONY: install
install:
	mkdir -p ${ROOT}/lib/modules
	if [ "`find ${KERNEL_DIR} -name "*.ko"`" != "" ]; then \
	cd ${KERNEL_DIR} && find . -name "*.ko" | xargs cp -f --parents --target-directory=${ROOT}/lib/modules; fi

