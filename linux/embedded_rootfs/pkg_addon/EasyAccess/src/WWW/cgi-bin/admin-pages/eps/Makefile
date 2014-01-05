
all:
	$(MAKE) -C policy
	$(MAKE) -C patch
	$(MAKE) -C browser
	$(MAKE) -C registry
	$(MAKE) -C antivirus
	$(MAKE) -C firewall

clean:
	$(MAKE) -C policy clean
	$(MAKE) -C patch clean
	$(MAKE) -C browser clean
	$(MAKE) -C registry clean
	$(MAKE) -C antivirus clean
	$(MAKE) -C firewall clean

bin-install:
	$(MAKE) -C policy bin-install
	$(MAKE) -C patch bin-install
	$(MAKE) -C browser bin-install
	$(MAKE) -C registry bin-install
	$(MAKE) -C antivirus bin-install
	$(MAKE) -C firewall bin-install

content-install:
	$(MAKE) -C policy content-install
	$(MAKE) -C patch content-install
	$(MAKE) -C browser content-install
	$(MAKE) -C registry content-install
	$(MAKE) -C antivirus content-install
	$(MAKE) -C firewall content-install

bin-un-install:
	$(MAKE) -C policy bin-un-install
	$(MAKE) -C patch bin-un-install
	$(MAKE) -C browser bin-un-install
	$(MAKE) -C registry bin-un-install
	$(MAKE) -C antivirus bin-un-install
	$(MAKE) -C firewall bin-un-install

content-un-install:
	$(MAKE) -C policy content-un-install
	$(MAKE) -C patch content-un-install
	$(MAKE) -C browser content-un-install
	$(MAKE) -C registry content-un-install
	$(MAKE) -C antivirus content-un-install
	$(MAKE) -C firewall content-un-install
