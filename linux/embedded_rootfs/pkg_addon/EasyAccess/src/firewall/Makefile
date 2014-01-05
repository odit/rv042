PPP_BIN=ip-up.local ip-down.local

include ../make-include.mk

ifdef CONFIG_IPV6
PPP_BIN+= ip6-up.local ip6-down.local
endif
BIN=iptables_manage $(PPP_BIN)
IPT_SCRIPT=ip-down.local.sh ip6-down.local.sh

all: $(BIN)

CFLAGS+=-I$(SYS_LIB) -I$(AUTH_LIB)

ip-up.local: ip-up.c IpTablesUpdate.c $(SYS_LIB)/libSys.so
	${CC} $(CFLAGS) $(LDFLAGS) -o $@ ip-up.c IpTablesUpdate.c -L$(SYS_LIB) -lSys -lcrypt -lssl

ip-down.local: ip-down.c $(SYS_LIB)/libSys.so
	${CC} $(CFLAGS) $(LDFLAGS) -o $@ ip-down.c -L$(SYS_LIB) -lSys -lcrypt -lssl

ifdef CONFIG_IPV6
ip6-up.local: ip6-up.c IpTablesUpdate.c $(SYS_LIB)/libSys.so
	${CC} $(CFLAGS) $(LDFLAGS) -o $@ ip6-up.c IpTablesUpdate.c -L$(SYS_LIB) -lSys -lcrypt -lssl

ip6-down.local: ip6-down.c $(SYS_LIB)/libSys.so
	${CC} $(CFLAGS) $(LDFLAGS) -o $@ ip6-down.c -L$(SYS_LIB) -lSys -lcrypt -lssl
endif

iptables_manage: IptablesManage.c ${SYS_LIB}/libSys.so
	${CC} $(CFLAGS) $(LDFLAGS) -o $@ IptablesManage.c -L$(SYS_LIB) -lSys -lcrypt -lssl

cleanbin: 
	rm $(BIN)
	if [ -n '$(LIB)' ]; then rm -rf $(LIB); fi

clean:
	rm -f *.o
	rm -f $(BIN)
	if [ -n '$(LIB)' ]; then rm -rf $(LIB); fi

install:
	if [ -n '$(BIN)' ]; then $(STRIP) $(BIN); cp -rf $(PPP_BIN) $(IPT_SCRIPT) /etc/ppp/; cp -rf iptables_manage ${ACCESS_POINT_BIN}/; cp -rf rc.firewall /etc/rc.d; fi

un-install:
	if [ -n '$(BIN)' ]; then cd /etc/ppp/; rm -rf $(PPP_BIN) $(IPT_SCRIPT); cd ${ACCESS_POINT_BIN}; rm -rf iptables_manage; cd /etc/rc.d; rm -rf rc.firewall; fi
