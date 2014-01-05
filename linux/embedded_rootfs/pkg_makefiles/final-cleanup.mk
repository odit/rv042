
include ../.config

################### NK-modify start ###################
# purpose:   Build Machine  # author:    Chihmou
# date:      2010-07-25   # description: Change nklib path
#NKLIB_DIR = /tmp/nklib/usr/lib
NKLIB_DIR = ${ROOT}/../nklib/usr/lib
###################  NK-modify end  ###################

.PHONY: install
install:
	mkdir -p ${NKLIB_DIR}
	chmod 777 ${NKLIB_DIR}
	for u in ${ROOT}/usr/; do \
		cp -r $$u/lib/. ${NKLIB_DIR}; \
	done

	- mv ${ROOT}/usr/lib/* ${ROOT}/${LIBDIR}/ &> /dev/null
	rm -rf ${ROOT}/usr/include
	rm -rf ${ROOT}/usr/lib32/gconv
	rm -rf ${ROOT}/usr/lib64/gconv
	- find ${ROOT} -name doc | xargs rm -fr
	- find ${ROOT} -name man | xargs rm -fr
	- find ${ROOT} -name info | xargs rm -fr
	- find ${ROOT} -name "*.a" | xargs rm -fr
	for f in `find ${ROOT}/bin ${ROOT}/sbin ${ROOT}/usr/bin ${ROOT}/usr/sbin -not -type l -and -not -type d`; \
	do if sh -c "file $$f | grep -q ELF"; then ${STRIP} $$f; fi; done
	find ${ROOT} -name "*.so*" -and -not -type d -and -size +10 | xargs chmod +w
	find ${ROOT} -name "*.so*" -and -not -type d -and -size +10 | xargs ${STRIP}
	find ${ROOT} -name "*.so*" -and -not -type d -and -size +10 | xargs chmod -w
ifneq (${CFG_EXTRA_FILES_DIR},"")
	cp -R ${CFG_EXTRA_FILES_DIR}/. ${ROOT}
else	
	if [ -d ${SOURCE_DIR}/../user-include ]; then cp -R ${SOURCE_DIR}/../user-include/. ${ROOT}; fi
endif
	sudo chown -R -h root ${ROOT}
	sudo chgrp -R -h root ${ROOT}

