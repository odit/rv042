include ../make-include.mk

all:
ifdef SINGLE_CGI
	$(MAKE) -C single_cgi
else
	$(MAKE) -C cgi-bin
endif
ifdef RECOMPILE_JAVA
	$(MAKE) -C Java 
endif

clean:
ifdef SINGLE_CGI
	$(MAKE) -C single_cgi clean
else
	$(MAKE) -C cgi-bin clean
endif
ifdef RECOMPILE_JAVA
	$(MAKE) -C Java clean
endif

install:
ifdef SINGLE_CGI
	$(MAKE) -C single_cgi install
	$(MAKE) -C cgi-bin content-install
else
	$(MAKE) -C cgi-bin install
endif
ifdef RECOMPILE_JAVA
	$(MAKE) -C Java install
else
	$(MAKE) -C Java copy
endif

un-install:
ifdef SINGLE_CGI
	$(MAKE) -C single_cgi un-install
	$(MAKE) -C cgi-bin content-un-install
else
	$(MAKE) -C cgi-bin un-install
endif
ifdef RECOMPILE_JAVA
	$(MAKE) -C Java un-install
endif
