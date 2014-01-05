
PKG:=broadcom_802.11n_multiband
VERSION:=20061106
DIR:=${PKG}-${VERSION}
                                                                                
.PHONY: install
install: ${DIR}
	mkdir -p ${ROOT}/wireless/broadcom
	cp ${DIR}/wl ${DIR}/cavium-n-ap.sh ${DIR}/cavium-n-sta.sh ${DIR}/cavium-n-post-script.sh ${ROOT}/wireless/broadcom
	cp ${DIR}/tcp_tuning.sh ${ROOT}/wireless
	mkdir -p ${ROOT}/lib/modules
	cp ${DIR}/wl_ap.ko ${DIR}/wl_sta.ko ${ROOT}/lib/modules
                                                                                
${DIR}:
	mkdir ${DIR}; cd ${DIR}; tar -zxf ${STORAGE}/${PKG}-${VERSION}.tgz

