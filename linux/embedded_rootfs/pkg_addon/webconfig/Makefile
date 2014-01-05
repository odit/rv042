include ../nkuserland.config

#IPSEC_TASK = ../super_freeswan/ipsec_task

SHELL=/bin/bash
ifeq ($(CONFIG_NK_URL_TMUFE_FILTER),y)
SHLIBDIR   = ${ROOT}/lib
LIBNAME1   = ./lib/libTrpsLm.so
LIBNAME2   = ./lib/libtmufeng.so
endif

PROG1=	webBoot
PROG2=	doAsh 
PROG3=	sysconfig 
PROG4=	factoryDefault 
PROG5=	sysInfo
PROG6=	initppp
#PROG7=	qos
PROG7=	session_limit_setting
PROG8=	voice
PROG9=	qos_rule_set 
PROG90=	gofire
PROG10=	nk_send_arp
ifeq ($(CONFIG_NK_LINE_DROP),y)
PROG11=	linedropped
endif
ifeq ($(CONFIG_NK_IDS),y)
PROG12=	snortconfig
endif
ifeq ($(CONFIG_NK_LICENSE_COUNT_DOWN),y)
PROG13=	qnosniff_trial
endif
PROG14= nk_send_setting
ifeq ($(CONFIG_ANTI_SPAM),y)
PROG15=	anti_spam_client_mail
endif
ifeq ($(CONFIG_NK_URL_TMUFE_FILTER),y)
PROG16=	licensecheck
PROG17=	urlfilter
endif
ifeq ($(NK_CONFIG_SNMP),y)
PROG18= snmp_startup_trap
endif
ifeq ($(CONFIG_NK_BONJOUR),y)
PROG19 = goBonjour
endif

ifeq ($(NK_CONFIG_IPV6),y)
PROG91=	gofire6
endif

DAEMONS= ${PROG1} ${PROG2} ${PROG3} ${PROG4} ${PROG5} ${PROG6} ${PROG7} ${PROG8} ${PROG9} ${PROG90} ${PROG10} ${PROG11} ${PROG12} ${PROG14} ${PROG15}
ifeq ($(CONFIG_NK_LICENSE_COUNT_DOWN),y)
DAEMONS+= ${PROG13}
endif
ifeq ($(CONFIG_NK_URL_TMUFE_FILTER),y)
DAEMONS+= ${PROG16}
DAEMONS+= ${PROG17}
endif
ifeq ($(NK_CONFIG_SNMP),y)
DAEMONS+= ${PROG18}
endif
ifeq ($(NK_CONFIG_IPV6),y)
DAEMONS+= ${PROG91}
endif
ifeq ($(CONFIG_NK_BONJOUR),y)
DAEMONS+= ${PROG19}
endif

#DAEMONS= ${PROG1} ${PROG2} ${PROG3} ${PROG4} ${PROG5} ${PROG6} ${PROG7}


SRCS1=	webBoot.c network.c webFlash.c config.c firewall.c webconfig_util.c dhcpc.c dhcps.c pppoe-operate.c pptpd.c pptp-operate.c ddns.c sys_diag.c sys_diag_util.c syslog.c proc-set.c static_route_operate.c rip-operate.c ripng-operate.c cpld.c qos_setting.c qos_command.c wan_mac_filiter.c route_util.c anti_spam.c anti_virus.c inboundLB.c url_filter/url_filter_mod.c
SRCS2=	doAsh.c
SRCS3=	sysconfig.c
SRCS4=	factoryDefault.c
SRCS5=	sysInfo.c
SRCS6=	initppp.c
#SRCS7=	qos_setting.c qos_command.c webconfig_util.c
SRCS7=	session_limit_setting.c webconfig_util.c 
SRCS8=	voice.c
SRCS9=	qos_rule_set.c
SRCS90=	gofire.c webconfig_util.c
SRCS10=	nk_send_arp.c webconfig_util.c
ifeq ($(CONFIG_NK_LINE_DROP),y)
SRCS11=	linedropped.c webconfig_util.c
endif
SRCS14=	nk_send_setting.c webconfig_util.c 

ifeq ($(CONFIG_NK_IDS),y)
SRCS12=	snortconfig.c
endif

ifeq ($(CONFIG_NK_LICENSE_COUNT_DOWN),y)
SRCS13=	qnosniff_trial.c webconfig_util.c
endif

ifeq ($(CONFIG_ANTI_SPAM),y)
SRCS15=	anti_spam_client_mail.c
endif

ifeq ($(CONFIG_NK_URL_TMUFE_FILTER),y)
SRCS16=	licensecheck.c webconfig_util.o
SRCS17=	url_filter/urlfilterd.c webconfig_util.o
endif

ifeq ($(NK_CONFIG_SNMP),y)
SRCS18=	snmp_startup_trap.c 
endif

ifeq ($(NK_CONFIG_IPV6),y)
SRCS91=	gofire6.c webconfig_util.c
endif
ifeq ($(CONFIG_NK_BONJOUR),y)
SRCS19= goBonjour.c
endif

SRCS1 += ../dualwan/smartlink.c
SRCS1 += ../dualwan/binding.c
SRCS1 += ../dualwan/autobalance.c
SRCS1 += ../dualwan/ipgroup.c
SRCS1 += ../dualwan/stategyroute.c
SRCS1 += ../dualwan/sroute.c
SRCS1 += nsd-operate.c

ifeq ($(NK_CONFIG_IPV6),y)
SRCS1 += 6to4.c
endif
CFLAGS := -mabi=64
LDADD = -lnku

# ifeq ($(CONFIG_NK_BONJOUR),y)
# SRCS1 += nk_bonjour.c
# CFLAGS += -DCONFIG_NK_BONJOUR
# endif

ifeq ($(CONFIG_NK_PPTP_TRUNKING),y)
SRCS1 += pptp_trunking-operate.c
endif

ifeq ($(NK_CONFIG_SYSKLOGD),y)
SRCS1 += start_sysklogd.c
CFLAGS += -DCONFIG_NK_SYSKLOGD
endif

#Added by Gavin Lin, 2008/08/22
ifeq ($(NK_CONFIG_SNMP),y)
SRCS1 += snmp-operate.c
CFLAGS += -DCONFIG_NK_SNMP
endif

# 2007/04/30 jane: add to support dhcp multiple subnet (pool)
CFLAGS += -DNK_CONFIG_SUPPORT_MULTI_DHCP_SUBNET

ifeq ($(NK_CONFIG_NTPCLIENT),y)
SRCS1 += ntp.c
CFLAGS += -DCONFIG_NK_NTPCLIENT
endif

ifeq ($(CONFIG_NK_IDS),y)
SRCS1 += IDS.c
DAEMONS += ${PROG12}
CFLAGS += -DCONFIG_NK_IDS
endif

ifeq ($(CONFIG_NK_URL_TMUFE_FILTER),y)
CFLAGS += -lpthread
CFLAGS +=  -I../../build/openssl-0.9.8d/include
endif

ifeq ($(CONFIG_ANTI_SPAM),y)
DAEMONS += ${PROG15}
endif

ifeq ($(NK_CONFIG_IPV6),y)
SRCS1 += isc_dhcp-operate.c
SRCS1 += radvd-operate.c
SRCS1 += network_ipv6.c
CFLAGS += -DNK_CONFIG_IPV6
endif

#---------------
OBJS1	+= ${SRCS1:.c=.o}
OBJS2	+= ${SRCS2:.c=.o}
OBJS3	+= ${SRCS3:.c=.o}
OBJS4	+= ${SRCS4:.c=.o}
OBJS5	+= ${SRCS5:.c=.o}
OBJS6	+= ${SRCS6:.c=.o}
OBJS7	+= ${SRCS7:.c=.o}
OBJS8	+= ${SRCS8:.c=.o}
OBJS9	+= ${SRCS9:.c=.o}
OBJS90	+= ${SRCS90:.c=.o} license_key/fun_key.o
OBJS10	+= ${SRCS10:.c=.o}
OBJS11	+= ${SRCS11:.c=.o}
ifeq ($(CONFIG_NK_IDS),y)
OBJS12	+= ${SRCS12:.c=.o}
endif
ifeq ($(CONFIG_NK_LICENSE_COUNT_DOWN),y)
OBJS13	+= ${SRCS13:.c=.o} license_key/fun_key.o
endif
OBJS14	+= ${SRCS14:.c=.o}

ifeq ($(CONFIG_ANTI_SPAM),y)
OBJS15	+= ${SRCS15:.c=.o}
endif

ifeq ($(CONFIG_NK_URL_TMUFE_FILTER),y)
OBJS16	+= ${SRCS16:.c=.o} ../../build/openssl-0.9.8d/libssl.a ../../build/openssl-0.9.8d/libcrypto.a
OBJS17	+= ${SRCS17:.c=.o}
endif

ifeq ($(NK_CONFIG_SNMP),y)
OBJS18	+= ${SRCS18:.c=.o}
endif

ifeq ($(NK_CONFIG_IPV6),y)
OBJS91	+= ${SRCS91:.c=.o}
endif

ifeq ($(CONFIG_NK_BONJOUR),y)
OBJS19 = ${SRCS19:.c=.o}
CFLAGS += -DCONFIG_NK_BONJOUR
endif

#---------------
all:	
#---------------
${PROG1}: ${OBJS1}
	${CC} $(CFLAGS) -o ${PROG1} ${LIBS} ${OBJS1} ${LDADD}
#---------------
${PROG2}: ${OBJS2}
	${CC} $(CFLAGS) -o ${PROG2} ${LIBS} ${OBJS2} ${LDADD}
#---------------
${PROG3}: ${OBJS3}
	${CC} $(CFLAGS) -o ${PROG3} ${LIBS} ${OBJS3} ${LDADD}
#---------------
${PROG4}: ${OBJS4}
	${CC} $(CFLAGS) -o ${PROG4} ${LIBS} ${OBJS4} ${LDADD}
#---------------
${PROG5}: ${OBJS5}
	${CC} $(CFLAGS) -o ${PROG5} ${LIBS} ${OBJS5} ${LDADD}	
#---------------
${PROG6}: ${OBJS6}
	${CC} $(CFLAGS) -o ${PROG6} ${LIBS} ${OBJS6} ${LDADD}
#---------------
${PROG7}: ${OBJS7}
	${CC} $(CFLAGS) -o ${PROG7} ${LIBS} ${OBJS7} ${LDADD}
#---------------
${PROG8}: ${OBJS8}
	${CC} $(CFLAGS) -o ${PROG8} ${LIBS} ${OBJS8} ${LDADD}
#---------------
${PROG9}: ${OBJS9}
	${CC} $(CFLAGS) -o ${PROG9} ${LIBS} ${OBJS9} ${LDADD}

${PROG90}: ${OBJS90}
	${CC} $(CFLAGS) -o ${PROG90} ${LIBS} ${OBJS90} ${LDADD}

${PROG10}: ${OBJS10}
	${CC} $(CFLAGS) -o ${PROG10} ${LIBS} ${OBJS10} ${LDADD}

${PROG11}: ${OBJS11}
	${CC} $(CFLAGS) -o ${PROG11} ${LIBS} ${OBJS11} ${LDADD}

ifeq ($(CONFIG_NK_IDS),y)
${PROG12}: ${OBJS12}
	${CC} $(CFLAGS) -o ${PROG12} ${LIBS} ${OBJS12} ${LDADD}
endif

ifeq ($(CONFIG_NK_LICENSE_COUNT_DOWN),y)
${PROG13}: ${OBJS13}
	${CC} $(CFLAGS) -o ${PROG13} ${LIBS} ${OBJS13} ${LDADD}
endif
${PROG14}: ${OBJS14}
	${CC} $(CFLAGS) -o ${PROG14} ${LIBS} ${OBJS14} ${LDADD}

ifeq ($(CONFIG_ANTI_SPAM),y)
${PROG15}: ${OBJS15}
	${CC} $(CFLAGS) -o ${PROG15} ${LIBS} ${OBJS15} ${LDADD}
endif

ifeq ($(CONFIG_NK_URL_TMUFE_FILTER),y)
${PROG16}: ${OBJS16}
	${CC} $(CFLAGS) -o ${PROG16} ${LIBS} $(LIBNAME1) ${OBJS16} ${LDADD}

${PROG17}: ${OBJS17}
	${CC} $(CFLAGS) -o ${PROG17} ${LIBS} $(LIBNAME2) ${OBJS17} ${LDADD} -ldl
endif

ifeq ($(NK_CONFIG_SNMP),y)
${PROG18}: ${OBJS18}
	${CC} $(CFLAGS) -o ${PROG18} ${LIBS} ${OBJS18} ${LDADD}
endif

ifeq ($(NK_CONFIG_IPV6),y)
${PROG91}: ${OBJS91}
	${CC} $(CFLAGS) -o ${PROG91} ${LIBS} ${OBJS91} ${LDADD}
endif

ifeq ($(CONFIG_NK_BONJOUR),y)
${PROG19}: ${OBJS19}
	${CC} $(CFLAGS) -o ${PROG19} ${LIBS} ${OBJS19} ${LDADD}
endif

clean:
	rm -f *.o url_filter/*.o ${DAEMONS} ${DAEMONS}.elf ${DAEMONS}.gdb
#/*purpose     : bts id 12723 author : trenchen date : 2010-06-19    */
#/*description : dualwan directory need make clean                   */
	rm -f ../dualwan/*.o
	 
#---------------
install:
	cp ${DAEMONS} ${ROOT}/bin
ifeq "$(CONFIG_NK_URL_TMUFE_FILTER)" "y"
	cp ${LIBNAME1} ${SHLIBDIR}
	cp ${LIBNAME2} ${SHLIBDIR}
endif
	(cd ${ROOT}/bin; ln -fs $(PROG4) saveSettings; ln -fs $(PROG2) initWANConn)
#---------------

../dualwan/binding.o : ../dualwan/binding.c ../dualwan/autobalance.c ../dualwan/ipgroup.c ../dualwan/stategyroute.c ../dualwan/sroute.c

../dualwan/stategyroute.o : ../dualwan/binding.c ../dualwan/autobalance.c ../dualwan/ipgroup.c ../dualwan/stategyroute.c ../dualwan/sroute.c

../dualwan/sroute.o : ../dualwan/binding.c ../dualwan/autobalance.c ../dualwan/ipgroup.c ../dualwan/stategyroute.c ../dualwan/sroute.c

../dualwan/ipgroup.o : ../dualwan/binding.c ../dualwan/autobalance.c ../dualwan/ipgroup.c ../dualwan/stategyroute.c ../dualwan/sroute.c

../dualwan/autobalance.o : ../dualwan/binding.c ../dualwan/autobalance.c ../dualwan/ipgroup.c ../dualwan/stategyroute.c ../dualwan/sroute.c

# End of Makefile
