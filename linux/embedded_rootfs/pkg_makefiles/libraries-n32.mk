
.PHONY: install
install:
	mkdir -p ${ROOT}/lib32
	cd ${OCTEON_ROOT}/tools/${CROSS}/sys-root/lib32; for shared in `find . -name "*.so*"`; do cp -d --parents $$shared ${ROOT}/lib32/; done
	mkdir -p ${ROOT}/usr/lib32
	cd ${OCTEON_ROOT}/tools/${CROSS}/sys-root/usr/lib32; for shared in `find . -name "*.so*"`; do cp -d --parents $$shared ${ROOT}/usr/lib32/; done

