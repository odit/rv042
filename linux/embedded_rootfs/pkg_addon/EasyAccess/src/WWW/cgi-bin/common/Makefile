include ../../../make-include.mk

BIN=userLogin userLogout welcome addUserBookmark portal accesspoint ppp_launch
HTML=login?.html mainwindow?.html error.html logout.html browser_error.html portal?.html accesspoint.html loginbanner?.html missingportal.html climiterror.html
OTHER=images index.html js missing.html rebooting.html restarting.html blank.html dashboard.swf
#  WebHelp 
CERT-BIN=certVerifyLogin

ifeq "$(CONFIG_EPS)" "y"
BIN+=epsverify epslogin
HTML+=accesspoint_eps.html
endif

all: $(BIN) $(CERT-BIN)
include ../cgi-include.mk

install: bin-install content-install

bin-install:
	$(STRIP) $(BIN)
	$(STRIP) $(CERT-BIN)
	cp $(BIN) $(DEST_CGI_BIN)
	cp $(CERT-BIN) $(DEST_CERT_BIN)

content-install:
	cd html; cp $(HTML) $(DEST_CGI_BIN)
	cd html; cp -rf $(OTHER) $(DEST_HTDOCS)
	install -d $(DEST_HTDOCS)/themes
	cd html/themes && for i in blueblackgrey brown creamred green greyandblue \
		greyredorange khakiblue orangeandblue orangepurple redwhiteandblue seafoam \
		whitebluegrey yellow yellowredblack ; do \
		cp -f $$i/*.js $(DEST_HTDOCS)/themes; \
		cp -f $$i/*.css $(DEST_HTDOCS)/themes; \
		cp -f $$i/*.png $(DEST_HTDOCS)/images; \
		cp -f $$i/*.gif $(DEST_HTDOCS)/images || true; \
	done
	cd html/themes && cp -f *.gif $(DEST_HTDOCS)/themes
	cd html/themes && cp -f whitebluegrey/*.gif $(DEST_HTDOCS)/images
	cd html/themes && cp -f green/*.gif $(DEST_HTDOCS)/images
	cd html/themes && cp -f brown/*.gif $(DEST_HTDOCS)/images
	cd html/themes && cp -f blueblackgrey/*.gif $(DEST_HTDOCS)/images
	cd html/themes && cp -f orangepurple/*.gif $(DEST_HTDOCS)/images
	cd html/themes && cp -f greyredorange/*.gif $(DEST_HTDOCS)/images
	cd html/themes && cp -f creamred/*.gif $(DEST_HTDOCS)/images
	cd html/themes && cp -f yellow/*.gif $(DEST_HTDOCS)/images

ifdef CONFIG_IPV6
	cd html/js6 && cp -f formval.js $(DEST_HTDOCS)/js
# 	cd html; cp -rf ConfigHelp6 $(DEST_HTDOCS)/ConfigHelp
# else
# 	cd html; cp -rf ConfigHelp $(DEST_HTDOCS)
endif 

CFLAGS += -L/usr/local/samba/lib -DACCESSPOINT_AUTH_LIB

test: test.c
	${CC} $^  $(CFLAGS) $(CFLAGS) -I$(AUTH_LIB) -L$(AUTH_LIB) $(AAA_LIB) -o $@

welcome: welcome.c
	${CC} $^  $(CFLAGS) -o $@

userLogin: userLogin.c
	${CC} $^  $(CFLAGS) -I$(AUTH_LIB) -L$(AUTH_LIB) $(AAA_LIB) -o $@

certVerifyLogin: certVerifyLogin.c
	${CC} $^  $(CFLAGS) -I$(AUTH_LIB) -L$(AUTH_LIB) $(AAA_LIB) -o $@

userLogout: userLogout.c
	${CC} $^  $(CFLAGS) -o $@

addUserBookmark: addUserBookmark.c
	${CC} $^  $(CFLAGS) -o $@

portal: portal.c
	${CC} $^  $(CFLAGS) -o $@

accesspoint: accesspoint.c
	${CC} $^  $(CFLAGS) -o $@

ppp_launch: ppp_launch.c
	${CC} $^  $(CFLAGS) -o $@

ifdef CONFIG_EPS
epsverify: epsverify.c
	${CC} $^  $(CFLAGS) -o $@

epslogin: epslogin.c
	${CC} $^  $(CFLAGS) -o $@
endif
