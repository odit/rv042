#Edit by Robert 2008/04
#CC	= gcc
DESTDIR = ${ROOT}
COPTS	= -O2 -g
CFLAGS	= $(COPTS) -I.. -I../../include -fPIC
LDFLAGS	= -shared
LDADD   = -lutil
INSTALL	= install -o root

PLUGINS = pptpd-logwtmp.so

# include dependencies if present
ifeq (.depend,$(wildcard .depend))
include .depend
endif

all:	$(PLUGINS)

%.so: %.c
	$(CC) -o $@ $(LDFLAGS) $(CFLAGS) $^ $(LDADD)

install: $(PLUGINS)
	$(INSTALL) -d $(USRLIBDIR)
	$(INSTALL) $? $(USRLIBDIR)

uninstall:
	rm -f $(USRLIBDIR)$(PLUGINS)

clean:
	rm -f *.o *.so *.a

depend:
	$(CPP) -M $(CFLAGS) *.c >.depend
