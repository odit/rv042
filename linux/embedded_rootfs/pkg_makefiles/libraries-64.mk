
.PHONY: install
install:
	mkdir -p ${ROOT}/lib64
	cd ${OCTEON_ROOT}/tools/${CROSS}/sys-root/lib64; for shared in `find . -name "*.so*"`; do cp -d --parents $$shared ${ROOT}/lib64/; done
	mkdir -p ${ROOT}/usr/lib64
	cd ${OCTEON_ROOT}/tools/${CROSS}/sys-root/usr/lib64; for shared in `find . -name "*.so*"`; do cp -d --parents $$shared ${ROOT}/usr/lib64/; done

