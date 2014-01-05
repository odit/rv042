UCLINUX_DIR=/build2/uClinux/uC-src
INSTALL=/build2/uClinux/brecis/holes/gzipelf/gzipelf.sh
.PHONY=all clean cleanbin install un-install

ifdef CONFIG_REAL_TURBOSSL
SSL_DIR=$(UCLINUX_DIR)/real/TurboSSL-0.9.7d
else
SSL_DIR=$(UCLINUX_DIR)/real/openssl-0.9.7c
endif

KRB_DIR=$(UCLINUX_DIR)/real/krb5-1.3.1
KRB_INC_DIR=$(KRB_DIR)/src/include
KRB_LIB_DIR=$(KRB_DIR)/src/lib

SAMBA_DIR=$(UCLINUX_DIR)/real/samba-3.0.8
SAMBA_INC_DIR=$(SAMBA_DIR)/source/include
SAMBA_LIB_DIR=$(SAMBA_DIR)/source/bin

LDAP_DIR=$(UCLINUX_DIR)/real/openldap-2.1.25
LDAP_INC_DIR=$(LDAP_DIR)/include
LDAP_LIB_DIR=$(LDAP_DIR)/libraries/libldap/.libs
LDAP_BER_LIB_DIR=$(LDAP_DIR)/libraries/liblber/.libs

ifdef CONFIG_REAL_EASYACCESS
TOP_DIR=$(UCLINUX_DIR)/real/EasyAccess
INSTALL_TOP_DIR=$(UCLINUX_DIR)/usr/local/src/EasyAccess
else
TOP_DIR=$(UCLINUX_DIR)/real/AccessPoint
INSTALL_TOP_DIR=$(TOP_DIR)
endif

CFLAGS= -Wall -Os\
        -I$(SSL_DIR)/include -I$(UCLINUX_DIR)/include -DSINGLE_BINARY \
        -DMAX_MSG_LEN=32768 \
        -DPRODUCT_ID=\"EasyAccess\" -DVENDOR_ID="\"Cavium Networks\"" \
        -DPRODUCT_STRING="\"Cavium Networks, EasyAccess SSL VPN\""

STRIP=mips-strip -d -x -R .note -R .comment


SYS_LIB=$(TOP_DIR)/src/lib
AUTH_LIB=$(TOP_DIR)/src/libauth
GCGI_LIB=$(TOP_DIR)/src/libgcgi
SSLCERT_LIB=$(TOP_DIR)/src/libsslcert

WWW_ROOT=$(INSTALL_TOP_DIR)/www
ACCESS_POINT_BIN=$(INSTALL_TOP_DIR)/bin

DEST_CERT_BIN=$(WWW_ROOT)/cert-bin
DEST_CGI_BIN=$(WWW_ROOT)/cgi-bin
DEST_HTDOCS=$(WWW_ROOT)/htdocs

SSL_LIBS=$(SSL_DIR)/libssl.a $(SSL_DIR)/libcrypto.a

