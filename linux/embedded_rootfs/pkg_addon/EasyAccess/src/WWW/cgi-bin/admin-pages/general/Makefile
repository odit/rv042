all:
	$(MAKE) -C status
	$(MAKE) -C settings
	$(MAKE) -C certificates
	$(MAKE) -C date

clean:
	$(MAKE) -C status clean
	$(MAKE) -C settings clean
	$(MAKE) -C certificates clean
	$(MAKE) -C date clean

install: bin-install content-install

bin-install:
	$(MAKE) -C status  bin-install
	$(MAKE) -C settings bin-install
	$(MAKE) -C certificates bin-install
	$(MAKE) -C date bin-install

content-install:
	$(MAKE) -C status  content-install
	$(MAKE) -C settings content-install
	$(MAKE) -C certificates content-install
	$(MAKE) -C date content-install

bin-un-install:
	$(MAKE) -C status  bin-un-install
	$(MAKE) -C settings  bin-un-install
	$(MAKE) -C certificates bin-un-install
	$(MAKE) -C date bin-un-install

content-un-install:
	$(MAKE) -C status  content-un-install
	$(MAKE) -C settings  content-un-install
	$(MAKE) -C certificates content-un-install
	$(MAKE) -C date content-un-install
