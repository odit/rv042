
PKG:=atheros_802.11n_multiband-func
VERSION:=20070302
DIR:=${PKG}-${VERSION}
                                                                                
.PHONY: install
install: ${DIR}
	mkdir -p ${ROOT}/wireless/atheros
	cp -R ${DIR}/AP ${DIR}/STA ${ROOT}/wireless/atheros/

${DIR}:
	mkdir ${DIR}; cd ${DIR}; tar -zxf ${STORAGE}/${PKG}-${VERSION}.tgz

