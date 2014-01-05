
#COPTS	= -O2
#CFLAGS	= $(COPTS) -I.. -I../../include -fPIC
LDFLAGS	= -shared




#MANDIR = $(DESTDIR)/share/man/man8

SUBDIRS := rp-pppoe
#SUBDIRS := rp-pppoe pppoatm radius
# Uncomment the next line to include the radius authentication plugin
# SUBDIRS += radius
#PLUGINS := minconn.so passprompt.so passwordfd.so winbind.so
PLUGINS :=

# include dependencies if present
ifeq (.depend,$(wildcard .depend))
include .depend
endif

all:	$(PLUGINS)
	for d in $(SUBDIRS); do $(MAKE) $(MFLAGS) -C $$d all; done

%.so: %.c
	$(CC) -o $@ $(LDFLAGS) $(CFLAGS) $^

VERSION = $(shell awk -F '"' '/VERSION/ { print $$2; }' ../patchlevel.h)

install: $(PLUGINS)
#	$(INSTALL) -d $(LIBDIR)
	for d in $(SUBDIRS); do $(MAKE) $(MFLAGS) -C $$d install; done

clean:
	rm -f *.o *.so *.a
	for d in $(SUBDIRS); do $(MAKE) $(MFLAGS) -C $$d clean; done

#depend:
#	$(CPP) -M $(CFLAGS) *.c >.depend
#	for d in $(SUBDIRS); do $(MAKE) $(MFLAGS) -C $$d depend; done
