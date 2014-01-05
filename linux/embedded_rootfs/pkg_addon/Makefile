include nkuserland.config

BUILD_DIR-y += nku
CLEAN_DIR += nku

BUILD_DIR-y += tool
CLEAN_DIR += tool

BUILD_DIR-y += webconfig
CLEAN_DIR += webconfig

ifeq "$(NK_CONFIG_EASYACCESS)" "y"
CLEAN_DIR += EasyAccess
endif

SCRT_BUILD_DIR-y += msmtp-1.4.21
CLEAN_DIR += msmtp-1.4.21

BUILD_DIR-${CONFIG_PPP2_4_3} += ppp-2.4.3
CLEAN_DIR += ppp-2.4.3

BUILD_DIR-${CONFIG_PPTP_CLIENT} += pptp-1.7.0
CLEAN_DIR += pptp-1.7.0

BUILD_DIR-${CONFIG_IPROUTE2} += iproute2
CLEAN_DIR += iproute2

BUILD_DIR-${CONFIG_NK_NSD} += nsd
CLEAN_DIR += nsd

BUILD_DIR-${NK_CONFIG_DHCPCD} += dhcpcd
CLEAN_DIR += dhcpcd

BUILD_DIR-${NK_CONFIG_UDHCP} += udhcp
CLEAN_DIR += udhcp

BUILD_DIR-${NK_CONFIG_DNSMASQ} += dnsmasq
CLEAN_DIR += dnsmasq

BUILD_DIR-${NK_CONFIG_DDNS} += ez-ipupdate
CLEAN_DIR += ez-ipupdate

BUILD_DIR-${CONFIG_NK_DDNS_QDNS} += ddns_update
CLEAN_DIR += ddns_update

BUILD_DIR-${NK_CONFIG_SYSKLOGD} += sysklogd-1.4.1
CLEAN_DIR += sysklogd-1.4.1

BUILD_DIR-${NK_CONFIG_SNMP} += net-snmp-5.4.1.2
CLEAN_DIR += net-snmp-5.4.1.2

ifeq "$(CONFIG_RV0XX)" "y"
BUILD_DIR-y += nkfwmd5sum
CLEAN_DIR += nkfwmd5sum
endif

BUILD_DIR-${NK_CONFIG_SNMP} += bind9

ifeq "$(NK_CONFIG_SNMP)" "y"
BUILD_DIR-${NK_CONFIG_SNMP} += sysstat-8.1.5
CLEAN_DIR += sysstat-8.1.5
else
BUILD_DIR-${CONFIG_COMMON_UI} += sysstat-8.1.5
CLEAN_DIR += sysstat-8.1.5
endif

BUILD_DIR-${NK_CONFIG_SNMP} += sysstat-8.1.5
CLEAN_DIR += sysstat-8.1.5

BUILD_DIR-${CONFIG_NK_BONJOUR} += mDNSResponder-107.6/mDNSPosix
CLEAN_DIR += mDNSResponder-107.6/mDNSPosix

BUILD_DIR-${NK_CONFIG_TFTP} += tftp
CLEAN_DIR += tftp

BUILD_DIR-${CONFIG_NK_RIP} += zebra
CLEAN_DIR += zebra

BUILD_DIR-${NK_CONFIG_PPTPD} += pptpd-1.3.4
CLEAN_DIR += pptpd-1.3.4

BUILD_DIR-y += iptables-1.3.7
CLEAN_DIR += iptables-1.3.7

BUILD_DIR-y += netlinkctl
CLEAN_DIR += netlinkctl

BUILD_DIR-${NK_CONFIG_NTPCLIENT} += ntpclient
CLEAN_DIR += ntpclient

BUILD_DIR-${CONFIG_NK_STRONGSWAN} += strongswan-4.0.4
CLEAN_DIR += strongswan-4.0.4

BUILD_DIR-${CONFIG_NK_IPSEC} += ipsec
CLEAN_DIR += ipsec

BUILD_DIR-y += lan_up_daemon
CLEAN_DIR += lan_up_daemon

BUILD_DIR-y += detect_wan_session
CLEAN_DIR += detect_wan_session

ifeq "$(CONFIG_RV0XX)" "y"
BUILD_DIR-y += certificate_management
CLEAN_DIR += certificate_management
endif

BUILD_DIR-${CONFIG_NK_DHCP_RELAY} += dhcp-helper-1.0
CLEAN_DIR += dhcp-helper-1.0

BUILD_DIR-y += check_ui_lock
CLEAN_DIR += check_ui_lock

BUILD_DIR-y += switch_test
CLEAN_DIR += switch_test

BUILD_DIR-y += libupnp
CLEAN_DIR += libupnp

BUILD_DIR-y += libupnp/upnp/sample/linuxigd
CLEAN_DIR += libupnp/upnp/sample/linuxigd

SCRT_BUILD_DIR-${CONFIG_NK_LIBGMP} += gmp-4.2.1
CLEAN_DIR += gmp-4.2.1

SCRT_BUILD_DIR-${NK_CONFIG_IPV6} += dhcp-4.1.1-P1
CLEAN_DIR += dhcp-4.1.1-P1

SCRT_BUILD_DIR-${NK_CONFIG_IPV6} += radvd-1.6
CLEAN_DIR += radvd-1.6

BUILD_DIR-${NK_CONFIG_IPV6} += naptd-0.4.2
CLEAN_DIR += naptd-0.4.2

ifeq "$(CONFIG_NK_URL_TMUFE_FILTER)" "y"
BUILD_DIR-y += webconfig/stunnel
CLEAN_DIR += webconfig/stunnel
endif

MAN_USER = root
MAN_GROUP = root

.PHONY: all
all:
	rm -rf vendor/web_ui_Base/root/sbin/*
	cp -r vendor/web_ui_Data/Model_sbin/RV0XX_CRAMFS/. vendor/web_ui_Base/root/sbin

ifeq "$(NK_CONFIG_IPV6)" "y"
	cp -r dhcp-4.1.1-P1/client/scripts/linux ${ROOT}/etc/dhclient-script
	chmod 755 ${ROOT}/etc/dhclient-script
	touch ${ROOT}/etc/dhclient.conf
	chmod 755 ${ROOT}/etc/dhclient.conf
endif

	for s in ${SCRT_BUILD_DIR-y}; do \
		cd $$s;\
		./RUN; \
		cd -; \
	done

	for i in ${BUILD_DIR-y}; do \
		if ! ${MAKE} -C $$i ; then exit 1; fi; \
		if ! ${MAKE} -C $$i install; \
		then \
			echo "@@@@Error found! ...when doing [${MAKE} -C $$i install]"; \
		fi; \
	done	

	for u in ${HTDOCS_DIR-y}; do \
		cp -r $$u/root/. ${ROOT}/; \
	done

ifeq "$(NK_CONFIG_EASYACCESS)" "y"
	cd EasyAccess && CONFIG_RV0XX="$(CONFIG_RV0XX)" CONFIG_NK_CRAMFS="$(CONFIG_NK_CRAMFS)" CONFIG_COMMON_UI="$(CONFIG_COMMON_UI)" CONFIG_EPS="y" CONFIG_NK_LICENSE_COUNT_DOWN="$(CONFIG_NK_LICENSE_COUNT_DOWN)" CONFIG_NK_HTTPS_CONTROL="$(CONFIG_NK_HTTPS_CONTROL)" CONFIG_IPV6="$(NK_CONFIG_IPV6)" STATISTICS="n" ./RUN.Octeon
	${MAKE} -C EasyAccess PREFIX=${ROOT}/usr/local/EasyAccess  install
endif

ifeq "$(CONFIG_SSL_UI)" "y"
ifeq "$(CONFIG_COMMON_UI)" "y"
	cp -r vendor/web_ui_CI005/root/base_html/. ${ROOT}/usr/local/EasyAccess/www/htdocs
	cp -r vendor/web_ui_CI005/root/login.html ${ROOT}/usr/local/EasyAccess/www/cgi-bin/
	cp -rf vendor/web_ui_CI005/root/etc/. ${ROOT}/etc
	cp -rf vendor/web_ui_Base/root/etc/l7-protocols/. ${ROOT}/etc/l7-protocols
	cp -rf vendor/web_ui_Base/root/sbin/. ${ROOT}/sbin
endif
endif

	find ${ROOT} -name '.svn' | xargs rm -rf \;
	rm ${ROOT}/usr/local/EasyAccess/www/cgi-bin/singlecifs
	rm ${ROOT}/usr/local/EasyAccess/www/cgi-bin/edittscapplication.cgi
	
.PHONY: clean
clean:
	for j in ${CLEAN_DIR}; do \
		${MAKE} -C $$j clean; \
	done

ifeq "$(NK_CONFIG_EASYACCESS)" "y"
	cd EasyAccess ./RUN.CLEAN
	rm -rf /tmp/nklib/
endif
