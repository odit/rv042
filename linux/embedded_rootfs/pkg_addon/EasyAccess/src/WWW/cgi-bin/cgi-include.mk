include /home/lenny/Project/openwrt/cisco_RV0XX-v4.0.2.08-tm-GPL_CI005-ipv6/linux/embedded_rootfs/pkg_addon/EasyAccess/src/make-include-Octeon.mk
CFLAGS +=-DOPENSSL_NO_KRB5  -I$(SYS_LIB) -I$(GCGI_LIB) -I$(SSLCERT_LIB) -L$(SYS_LIB) -lSys -L$(GCGI_LIB) -lgcgi -L/usr/kerberos/lib -lcrypto -lssl -lm -L$(SSLCERT_LIB) -lsslcert
AAA_LIB=-lAuth -lldap -lcrypt -lssl -lsmbclient -lnsl -ldl -lgssapi_krb5 -lkrb5 -lcrypto -lk5crypto -lcom_err -lkrb5 -lnsl -ldl -lresolv
install: bin-install content-install

bin-install:
	if [ -n '$(BIN)' ]; then $(STRIP) $(BIN) ; fi
	if [ -n '$(CERT-BIN)' ]; then $(STRIP) $(CERT-BIN) ; fi
	if [ -n '$(BIN)' ]; then cp $(BIN) $(DEST_CGI_BIN); fi
	if [ -n '$(CERT-BIN)' ]; then cp $(CERT-BIN) $(DEST_CERT_BIN); fi

content-install:
	if [ -n '$(HTML)' ]; then cd html; cp $(HTML) $(DEST_CGI_BIN); fi
	if [ -n '$(OTHER)' ]; then cd html; cp -rf $(OTHER) $(DEST_HTDOCS); fi

un-install: bin-un-install content-un-install

bin-un-install:
	cd $(DEST_CGI_BIN); rm -rf $(BIN)
	if [ -n '$(CERT-BIN)' ]; then cd $(DEST_CERT_BIN); rm -rf $(CERT_BIN); fi

content-un-install:
	cd $(DEST_CGI_BIN); rm -rf $(HTML)
	if [ -n '$(OTHER)' ]; then cd $(DEST_HTDOCS); rm -rf $(OTHER); fi

clean:
	rm -f $(BIN) $(CERT-BIN) *.o
