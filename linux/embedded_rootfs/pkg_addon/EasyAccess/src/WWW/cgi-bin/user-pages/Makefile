all:
	$(MAKE) -C services
	$(MAKE) -C desktop
	$(MAKE) -C client
	$(MAKE) -C themes
	$(MAKE) -C tscapplications
	$(MAKE) -C homepage
	$(MAKE) -C cifs
	$(MAKE) -C userprofile

clean:
	$(MAKE) -C services clean
	$(MAKE) -C desktop clean
	$(MAKE) -C client clean
	$(MAKE) -C themes clean
	$(MAKE) -C tscapplications clean
	$(MAKE) -C homepage clean
	$(MAKE) -C cifs clean
	$(MAKE) -C userprofile clean

install: bin-install content-install

bin-install:
	$(MAKE) -C services bin-install 
	$(MAKE) -C desktop bin-install 
	$(MAKE) -C client bin-install 
	$(MAKE) -C themes bin-install 
	$(MAKE) -C tscapplications bin-install 
	$(MAKE) -C homepage bin-install 
	$(MAKE) -C cifs bin-install 
	$(MAKE) -C userprofile bin-install

content-install:
	$(MAKE) -C services content-install 
	$(MAKE) -C desktop content-install 
	$(MAKE) -C client content-install 
	$(MAKE) -C themes content-install 
	$(MAKE) -C tscapplications content-install 
	$(MAKE) -C homepage content-install 
	$(MAKE) -C cifs content-install 
	$(MAKE) -C userprofile content-install

un-install: bin-un-install content-un-install

bin-un-install:
	$(MAKE) -C services bin-un-install 
	$(MAKE) -C desktop bin-un-install 
	$(MAKE) -C client bin-un-install 
	$(MAKE) -C themes bin-un-install 
	$(MAKE) -C tscapplications bin-un-install 
	$(MAKE) -C homepage bin-un-install 
	$(MAKE) -C cifs bin-un-install 
	$(MAKE) -C userprofile bin-un-install

content-un-install:
	$(MAKE) -C services content-un-install 
	$(MAKE) -C desktop content-un-install 
	$(MAKE) -C client content-un-install 
	$(MAKE) -C themes content-un-install 
	$(MAKE) -C tscapplications content-un-install 
	$(MAKE) -C homepage content-un-install 
	$(MAKE) -C cifs content-un-install 
	$(MAKE) -C userprofile content-un-install
