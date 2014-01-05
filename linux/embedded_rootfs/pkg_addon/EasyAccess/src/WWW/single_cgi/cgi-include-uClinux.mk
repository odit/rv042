AAA_LIB=
install:
	if [ -n '$(BIN)' ]; then $(STRIP) $(BIN) ; fi
	if [ -n '$(CERT-BIN)' ]; then $(STRIP) $(CERT-BIN) ; fi
	if [ -n '$(BIN)' ]; then cp $(BIN) $(DEST_CGI_BIN); fi
	if [ -n '$(CERT-BIN)' ]; then cp $(CERT-BIN) $(DEST_CERT_BIN); fi
	if [ -n '$(HTML)' ]; then cd html; cp $(HTML) $(DEST_CGI_BIN); fi
	if [ -n '$(OTHER)' ]; then cd html; cp -rf $(OTHER) $(DEST_HTDOCS); fi

un-install:
	cd $(DEST_CGI_BIN); rm -rf $(BIN)
	cd $(DEST_CGI_BIN); rm -rf $(HTML)
	if [ -n '$(CERT-BIN)' ]; then cd $(DEST_CERT_BIN); rm -rf $(CERT_BIN); fi
	if [ -n '$(OTHER)' ]; then cd $(DEST_HTDOCS); rm -rf $(OTHER); fi

clean:
	rm -f $(BIN) $(CERT-BIN) *.o
