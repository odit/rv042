.EXPORT_ALL_VARIABLES:


all:
	cd chat; $(MAKE) $(MFLAGS) all
	cd pppd/plugins; $(MAKE) $(MFLAGS) all
	${MAKE} -C chat
	${MAKE} -C pppd
clean:
	cd chat; $(MAKE) clean
	cd pppd/plugins; $(MAKE) clean
	${MAKE} clean -C chat
	${MAKE} clean -C pppd

install:
#	cd pppd/plugins; $(MAKE) $(MFLAGS) install
	${MAKE} install -C pppd
	$(CP) chat/chat $(ROOT)/sbin/chat
