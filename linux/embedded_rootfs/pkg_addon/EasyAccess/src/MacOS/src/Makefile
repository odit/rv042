PLAT=-DMACOSX -I/System/Library/Frameworks/JavaVM.framework/Headers -g
DEST_PATH=/Library/Java/Extensions/com/menlo
PACKAGE=Package_Root/Package_contents/
COM=com/menlo
JAVAC=/System/Library/Frameworks/JavaVM.framework/Versions/1.3.1/Commands/javac -deprecation
JAVAH=/System/Library/Frameworks/JavaVM.framework/Versions/1.3.1/Commands/javah

CFLAGS+=      -Wall -Waggregate-return -Wcast-align -Wcast-qual -Wcomment \
	      -Wformat -Wimplicit-int  -Wimplicit-function-declaration -Winline \
	      -Wlong-long -Wmain -Wmissing-prototypes -Wmissing-declarations \
	      -Wnested-externs -Wno-import  -Wparentheses -Wpointer-arith -Wredundant-decls \
	      -Wreturn-type -Wswitch -Wunused  -Wunused-function -Wunused-label \
	      -Wunused-variable -Wunused-value -Werror $(PLAT)


all: pppwrap pppstop routewrap dnscommand ${COM}/MenloDialer.class ${COM}/MenloMgr.class ${COM}/MenloSSLApplet.class MenloDialer.o libMenloDialer.jnilib

pppwrap: PPPWrap.c
	gcc ${CFLAGS} PPPWrap.c -o pppwrap
	sudo -u root chmod ugo+s pppwrap
	sudo -u root chown root pppwrap
	sudo -u root chgrp wheel pppwrap

pppstop: PPPStop.c
	gcc ${CFLAGS} PPPStop.c -o pppstop
	sudo -u root chmod ugo+s pppstop
	sudo -u root chown root pppstop
	sudo -u root chgrp wheel pppstop

routewrap: RouteWrap.c
	gcc ${CFLAGS} RouteWrap.c -o routewrap
	sudo -u root chmod ugo+s routewrap
	sudo -u root chown root routewrap
	sudo -u root chgrp wheel routewrap

dnscommand: DnsCommand.c
	gcc ${CFLAGS} DnsCommand.c -o dnscommand
	sudo -u root chmod ugo+s dnscommand
	sudo -u root chown root dnscommand
	sudo -u root chgrp wheel dnscommand

${COM}/MenloDialer.class: ${COM}/MenloDialer.java
	${JAVAC} ${COM}/MenloDialer.java

${COM}/MenloMgr.class: ${COM}/MenloMgr.java
	${JAVAC} ${COM}/MenloMgr.java

${COM}/MenloSSLApplet.class: ${COM}/MenloSSLApplet.java
	${JAVAC} ${COM}/MenloSSLApplet.java

com_menlo_MenloDialer.h: ${COM}/MenloDialer.java
	${JAVAH} -jni com.menlo.MenloDialer

com_menlo_MenloMgr.h: ${COM}/MenloMgr.java
	${JAVAH} -jni com.menlo.MenloMgr

MenloDialer.o: MenloDialer.c com_menlo_MenloDialer.h Common.h
	gcc -c ${CFLAGS} -o MenloDialer.o MenloDialer.c -I/usr/include/openssl

libMenloDialer.jnilib: MenloDialer.o MenloMgr.o
	gcc -dynamiclib MenloDialer.o MenloMgr.o -lssl -lcrypto -o libMenloDialer.jnilib -framework JavaVM
 
clean:
	sudo -u root rm -f pppwrap pppstop routewrap dnscommand ${COM}/MenloDialer.class ${COM}/MenloMgr.class MenloDialer.o libMenloDialer.jnilib

install:
	sudo -u root rm -rf ${DEST_PATH}
	sudo -u root mkdir -p ${DEST_PATH}
	sudo -u root cp -rf pppwrap pppstop routewrap dnscommand ${COM}/*.class ${DEST_PATH}
	sudo -u root cp libMenloDialer.jnilib /System/Library/Frameworks/JavaVM.framework/Libraries//
	sudo -u root chmod ugo+s ${DEST_PATH}/pppwrap
	sudo -u root chown root ${DEST_PATH}/pppwrap
	sudo -u root chgrp wheel ${DEST_PATH}/pppwrap
	sudo -u root chmod ugo+s ${DEST_PATH}/pppstop
	sudo -u root chown root ${DEST_PATH}/pppstop
	sudo -u root chgrp wheel ${DEST_PATH}/pppstop
	sudo -u root chmod ugo+s ${DEST_PATH}/routewrap
	sudo -u root chown root ${DEST_PATH}/routewrap
	sudo -u root chgrp wheel ${DEST_PATH}/routewrap
	sudo -u root chmod ugo+s ${DEST_PATH}/dnscommand
	sudo -u root chown root ${DEST_PATH}/dnscommand
	sudo -u root chgrp wheel ${DEST_PATH}/dnscommand

pkg-install:
	sudo -u root rm -rf ${PACKAGE}/com/menlo
	sudo -u root mkdir -p ${PACKAGE}/com/menlo
	sudo -u root cp -rf pppwrap pppstop routewrap dnscommand libMenloDialer.jnilib ${COM}/*.class ${PACKAGE}/com/menlo
	sudo -u root chown -R root ${PACKAGE}
	sudo -u root chgrp -R wheel ${PACKAGE}
	sudo -u root chmod ugo+s ${PACKAGE}/com/menlo/pppwrap
	sudo -u root chmod ugo+s ${PACKAGE}/com/menlo/pppstop
	sudo -u root chmod ugo+s ${PACKAGE}/com/menlo/routewrap
	sudo -u root chmod ugo+s ${PACKAGE}/com/menlo/dnscommand
	sudo -u root rm -rf MenloLogic.pkg
	sudo -u root /Developer/Applications/Utilities/PackageMaker.app/Contents/MacOS/PackageMaker MLVirtualPassage_Package.pmsp


pkg-clean:
	sudo -u root rm -rf ${PACKAGE}/com/menlo

un-install:
	sudo -u root rm -rf ${DEST_PATH}
	sudo -u root rm -f /System/Library/Frameworks/JavaVM.framework/Libraries/libMenloDialer.jnilib
