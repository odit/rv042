all:
	echo "No make for this folder"

include ../make-include.mk

ifdef CONFIG_IPV6

COMMAND = cp Release/XTunnel6.cab $(DEST_HTDOCS)/XTunnel.cab ;\
	  cp Release/WindowsVPDialer6.jar $(DEST_HTDOCS)/WindowsVPDialer.jar ;\
	  cp Release/WindowsVPInstaller.jar $(DEST_HTDOCS) \
	  cp Release/VPInstall.cab $(DEST_HTDOCS)

else

COMMAND = cp Release/XTunnel.cab Release/WindowsVPDialer.jar Release/WindowsVPInstaller.jar Release/VPInstall.cab $(DEST_HTDOCS)

endif

install:
	$(COMMAND)
un-install:
	rm -rf $(DEST_HTDOCS)/XTunnel.cab $(DEST_HTDOCS)/WindowsVPDialer.jar $(DEST_HTDOCS)/WindowsVPInstaller.jar $(DEST_HTDOCS)/VPInstall.cab
