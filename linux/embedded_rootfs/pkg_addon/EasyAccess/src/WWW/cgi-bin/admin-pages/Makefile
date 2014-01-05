include ../../../make-include.mk
all:
	$(MAKE) -C access
	$(MAKE) -C general
	$(MAKE) -C network
	$(MAKE) -C logs
	$(MAKE) -C sslvpnclient

clean:
	$(MAKE) -C access clean
	$(MAKE) -C general clean
	$(MAKE) -C network clean
	$(MAKE) -C logs clean
	$(MAKE) -C sslvpnclient clean

install: bin-install content-install

bin-install:
	$(MAKE) -C access bin-install
	$(MAKE) -C general bin-install
	$(MAKE) -C network bin-install
	$(MAKE) -C logs bin-install
	$(MAKE) -C sslvpnclient bin-install

content-install:
	$(MAKE) -C access content-install
	$(MAKE) -C general content-install
	$(MAKE) -C network content-install
	$(MAKE) -C logs content-install
	$(MAKE) -C sslvpnclient content-install

un-install: bin-un-install content-un-install

bin-un-install:
	$(MAKE) -C access bin-un-install
	$(MAKE) -C general bin-un-install
	$(MAKE) -C network bin-un-install
	$(MAKE) -C logs bin-un-install
	$(MAKE) -C sslvpnclient bin-un-install

content-un-install:
	$(MAKE) -C access content-un-install
	$(MAKE) -C general content-un-install
	$(MAKE) -C network content-un-install
	$(MAKE) -C logs content-un-install
	$(MAKE) -C sslvpnclient content-un-install
