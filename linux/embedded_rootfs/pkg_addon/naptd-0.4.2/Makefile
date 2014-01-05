MAKE = gmake
MFLAGS = -C
SUBDIRS = src

all:
	$(MAKE) $(MFLAGS) $(SUBDIRS)

install:
	$(MAKE) $(MFLAGS) $(SUBDIRS) install
#	cp scripts/naptd /etc/init.d/naptd
#	ln -fs /etc/init.d/naptd /etc/rc3.d/S95naptd
#	ln -fs /etc/init.d/naptd /etc/rc5.d/S95naptd
#	ln -fs /etc/init.d/naptd /etc/rc3.d/K95naptd
#	ln -fs /etc/init.d/naptd /etc/rc5.d/K95naptd

clean:
	$(MAKE) $(MFLAGS) $(SUBDIRS) clean

uninstall:
	$(MAKE) $(MFLAGS) $(SUBDIRS) uninstall

