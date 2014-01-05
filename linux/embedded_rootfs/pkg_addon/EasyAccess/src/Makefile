include make-include.mk
all:
	$(MAKE) -C lib
	$(MAKE) -C libgcgi
	$(MAKE) -C libauth
	$(MAKE) -C libsslcert
	$(MAKE) -C smm
	$(MAKE) -C firebase
	$(MAKE) -C ftp
	$(MAKE) -C lcmgr
	$(MAKE) -C WWW
	$(MAKE) -C graphs
	$(MAKE) -C firewall
	$(MAKE) -C httprp

ifeq "$(CONFIG_USE_HTTPD)" "y"
	$(MAKE) -C httpd
endif

clean:
	$(MAKE) -C lib clean
	$(MAKE) -C libgcgi clean
	$(MAKE) -C libauth clean
	$(MAKE) -C libsslcert clean
	$(MAKE) -C smm clean
	$(MAKE) -C firebase clean
	$(MAKE) -C ftp clean
	$(MAKE) -C lcmgr clean
	$(MAKE) -C WWW clean
	$(MAKE) -C graphs clean
	$(MAKE) -C firewall clean
	$(MAKE) -C httprp clean

ifeq "$(CONFIG_USE_HTTPD)" "y"
	$(MAKE) -C httpd clean
endif

install:
	$(MAKE) -C smm install
	$(MAKE) -C firebase install
	$(MAKE) -C WWW install
	$(MAKE) -C ftp install
	$(MAKE) -C lib install
	$(MAKE) -C libgcgi install
	$(MAKE) -C libauth install
	$(MAKE) -C libsslcert install
	$(MAKE) -C graphs install
	$(MAKE) -C firewall install
	$(MAKE) -C AccessPointDial install
	$(MAKE) -C VirtualTransport install
	$(MAKE) -C COMWebCacheCleaner install
	$(MAKE) -C TSAC install
	$(MAKE) -C MacOS install
	$(MAKE) -C posixvpclient install
	$(MAKE) -C httprp install

ifeq "$(CONFIG_USE_HTTPD)" "y"
	$(MAKE) -C httpd install
endif

ifeq "$(CONFIG_EPS)" "y"
	$(MAKE) -C EPS install 
endif

un-install:
	$(MAKE) -C smm un-install
	$(MAKE) -C firebase un-install
	$(MAKE) -C WWW un-install
	$(MAKE) -C ftp un-install
	$(MAKE) -C lib un-install
	$(MAKE) -C libgcgi un-install
	$(MAKE) -C libauth un-install
	$(MAKE) -C libsslcert un-install
	$(MAKE) -C graphs un-install
	$(MAKE) -C firewall un-install
	$(MAKE) -C AccessPointDial un-install
	$(MAKE) -C VirtualTransport un-install
	$(MAKE) -C COMWebCacheCleaner un-install
	$(MAKE) -C TSAC un-install
	$(MAKE) -C MacOS un-install
	$(MAKE) -C posixvpclient un-install
	$(MAKE) -C httprp un-install

ifeq "$(CONFIG_USE_HTTPD)" "y"
	$(MAKE) -C httpd un-install
endif

ifeq "$(CONFIG_EPS)" "y"
	$(MAKE) -C EPS un-install
endif
