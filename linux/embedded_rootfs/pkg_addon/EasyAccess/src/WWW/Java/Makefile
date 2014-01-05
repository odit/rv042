qall:
	$(MAKE) -C TelSsh
	$(MAKE) -C vnc_javasrc

clean:
	$(MAKE) -C TelSsh clean
	$(MAKE) -C vnc_javasrc clean

install:
	$(MAKE) -C TelSsh install 
	$(MAKE) -C vnc_javasrc install 

un-install:
	$(MAKE) -C TelSsh un-install 
	$(MAKE) -C vnc_javasrc un-install 

include ../../make-include.mk
copy:
	cp jar/*.jar ${DEST_HTDOCS}

vnc_copy:
	cp jar/vncviewer.jar $(DEST_HTDOCS)
