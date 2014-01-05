
.PHONY: install
install:
	mkdir -p ${ROOT}/lib/modules
	cp ${KERNEL_DIR}/drivers/char/n_hdlc.ko ${ROOT}/lib/modules/

