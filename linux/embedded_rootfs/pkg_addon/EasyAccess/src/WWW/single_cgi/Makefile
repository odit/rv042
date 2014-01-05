BIN=single_cgi
HTML=
OTHER=
CERT-BIN=

all: $(BIN) $(CERT-BIN)
include ../cgi-bin/cgi-include.mk

CFLAGS += -DACCESSPOINT_AUTH_LIB -DSINGLE_CGI=1 -I$(AUTH_LIB) \
	-I../cgi-bin/user-pages/services -I../cgi-bin/user-pages/cifs \
        -I ../cgi-bin/admin-pages/general/certificates -I${SSLCERT_LIB}

LDFLAGS += -L${AUTH_LIB} $(AAA_LIB) -lutil -L${SSLCERT_LIB} -lsslcert

single_cgi: single_cgi.c
	$(CC) ${CFLAGS} ${LDFLAGS} single_cgi.c -o single_cgi

install:
	if [ -n '$(BIN)' ]; then $(STRIP) $(BIN) ; fi
	if [ -n '$(BIN)' ]; then cp $(BIN) $(DEST_CGI_BIN); fi
	cd $(DEST_CGI_BIN) && for i in welcome portal userLogout accesspoint \
                    addUserBookmark ppp_launch themes userBookmarkEdit ftp-logout telnet editBookmark \
                    nph-httprp ssh showBookmark https ftp-upload http ftp-rename ftp-frame \
                    ftp-addnew services ftp__download servicesBookmarkDelete addservicesbookmark \
                    tsc tscapplications sslvpnclient vpdesktopclient clientlinux \
                    installcert homepage deskBookmarkDelete vnc tscbookmark desktop adddesktopbookmark \
                    eventlog logconfig clientroutes clientaddresses homeconfig sitecustomization edittscapplication \
                    portallist addpolicy adminBookmarkDelete users editPolicy policyresource \
                    addbookmarkadmin editgroups showPolicy adduser addgroup editglobal bookmarkadmin \
                    policyrange editusers userDelete policyall policyadmin policyDelete groupDelete adddomainldap \
                    adddomainradius adddomainntdomain editdomain adddomainactivedirectory adddomain domains \
                    addtunnel tcptunneling networkresource editresource addresource sslcert viewcert importcert \
                    gencsr delcert newcsr status activeusers restart settings importlogo restoredefault importlicense \
                    importconfig upgradefirmware saveoptions encryptoptions layout savenow exportConfigFile \
                    date networkinterface staticroutes dnssettings ifacesettings etchosts activeconnections ; do \
	        ln -sf single_cgi $$i; \
	done
	cd $(DEST_CERT_BIN) && for i in certVerifyLogin; do \
	       ln -sf ../cgi-bin/single_cgi $$i; \
	done
