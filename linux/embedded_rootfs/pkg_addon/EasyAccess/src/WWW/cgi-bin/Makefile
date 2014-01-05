all:
	$(MAKE) -C user-pages
	$(MAKE) -C common
	$(MAKE) -C admin-pages
clean:
	$(MAKE) -C user-pages clean
	$(MAKE) -C common clean
	$(MAKE) -C admin-pages clean

install: bin-install content-install

bin-install:
	$(MAKE) -C user-pages bin-install
	$(MAKE) -C common bin-install
	$(MAKE) -C admin-pages bin-install

content-install:
	$(MAKE) -C user-pages content-install
	$(MAKE) -C common content-install
	$(MAKE) -C admin-pages content-install

un-install: bin-un-install content-un-install

bin-un-install:
	$(MAKE) -C user-pages bin-un-install
	$(MAKE) -C common bin-un-install
	$(MAKE) -C admin-pages bin-un-install

content-un-install:
	$(MAKE) -C user-pages content-un-install
	$(MAKE) -C common content-un-install
	$(MAKE) -C admin-pages content-un-install
