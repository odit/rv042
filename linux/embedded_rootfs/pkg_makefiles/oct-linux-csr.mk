DIR=oct-linux-csr

.PHONY: all
all: install
#all: ${DIR}/oct-linux-csr install

${DIR}/oct-linux-csr: ../.config ${SOURCE_DIR}/oct-linux-csr.c
	mkdir -p ${DIR}
	${CC} ${CFLAGS} -W -Wall -DUSE_RUNTIME_MODEL_CHECKS=1 -I${OCTEON_ROOT}/target/include/open-license -I${OCTEON_ROOT}/target/include -I${OCTEON_ROOT}/host/pci -o $@ ${SOURCE_DIR}/oct-linux-csr.c

.PHONY: install
install:
	mkdir -p ${ROOT}/usr/bin
	${STRIP} -o ${ROOT}/usr/bin/oct-linux-csr ${DIR}/oct-linux-csr

