all:
	$(MAKE) -C users-groups
	$(MAKE) -C domains
	$(MAKE) -C networkresources
	$(MAKE) -C ui-management

clean:
	$(MAKE) -C users-groups clean
	$(MAKE) -C domains clean
	$(MAKE) -C networkresources clean
	$(MAKE) -C ui-management clean

bin-install:
	$(MAKE) -C users-groups bin-install
	$(MAKE) -C domains bin-install
	$(MAKE) -C networkresources bin-install
	$(MAKE) -C ui-management bin-install

content-install:
	$(MAKE) -C users-groups content-install
	$(MAKE) -C domains content-install
	$(MAKE) -C networkresources content-install
	$(MAKE) -C ui-management content-install

bin-un-install:
	$(MAKE) -C users-groups bin-un-install
	$(MAKE) -C domains bin-un-install
	$(MAKE) -C networkresources bin-un-install
	$(MAKE) -C ui-management bin-un-install

content-un-install:
	$(MAKE) -C users-groups content-un-install
	$(MAKE) -C domains content-un-install
	$(MAKE) -C networkresources content-un-install
	$(MAKE) -C ui-management content-un-install
