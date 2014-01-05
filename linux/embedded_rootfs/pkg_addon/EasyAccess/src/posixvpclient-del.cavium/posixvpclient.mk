# General Macros

MAKEFILE_LINUX=Makefile.linux
MAKEFILE_MACOSX=Makefile.macosx
MAKEFILE_MACUNIV=Makefile.macuniv
PACKAGE=com/code
JAVA=java
VPDIALER=vpdialer
PPPD=pppd
LINUX_CONFIG_SCRIPT=linuxconfig.sh
MAC_CONFIG_SCRIPT=macconfig.sh
UNINSTALL_SCRIPT=uninstaller.sh
RM_DIR=rm -rf
# ~General Macros

# Macros for java
MANIFEST_FILE=java/version.info
ifdef IPV6
DIALER_JAR=release/SignedVPClient6.jar
MAC_DIALER_JAR=release/SignedMacVPClient6.jar
INSTALLER_JAR=release/SignedInstaller6.jar
else
DIALER_JAR=release/SignedVPClient.jar
MAC_DIALER_JAR=release/SignedMacVPClient.jar
INSTALLER_JAR=release/SignedInstaller.jar
endif
# ~Macros for java

# Macros for vpdialer

INTEL=intel
PPC=ppc
JNILIB=$(DIR)libdialer.so
PPP_WRAP=$(DIR)pppwrap
PPP_STOP=pppstop
ROUTE_WRAP=$(DIR)routewrap
ROUTE=$(DIR)route
IFCONF=$(DIR)ifconf
DNSCOMMAND=$(DIR)dnscommand
DNS_SCRIPT=dnscommand.sh
IP-UP=ip-up
IP-DOWN=ip-down
VPDIALER_INCLUDE_LINUX = -I/OpenSSL/include \
				-I/OpenSSL/include/openssl \
				   -I/usr/java/j2sdk1.4.2_12/include \
				   -I/usr/java/j2sdk1.4.2_12/include/linux

MAC_SSL_LIBS = -lssl -lcrypto
LINUX_SSL_LIBS = ../openssl/linux_vp_ssl.a ../openssl/linux_vp_crypto.a 
SYMBOLS = -K SSL_library_init
# ~Macros for vpdialer

# Macros for pppd

CUSTOM_PPPD=pppd.vp

# ~Macros for pppd

# Macros for signing the jar

ALIAS=qno
KEYSTORE=mycert.p12
STORETYPE=pkcs12
# ~Macros for signing the jar

