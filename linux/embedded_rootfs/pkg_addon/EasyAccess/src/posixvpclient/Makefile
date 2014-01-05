include ../make-include.mk

all:
	#Do Nothing for all

ifdef CONFIG_IPV6

COMMAND = cp release/SignedVPClient6.jar $(DEST_HTDOCS)/SignedVPClient.jar ;\
	  cp release/SignedMacVPClient6.jar $(DEST_HTDOCS)/SignedMacVPClient.jar ;\
	  cp release/SignedInstaller6.jar $(DEST_HTDOCS)/SignedInstaller.jar

else

COMMAND = cp release/SignedVPClient.jar release/SignedMacVPClient.jar release/SignedInstaller.jar $(DEST_HTDOCS)

endif
	
install: 
	$(COMMAND)
	
un-install:
	rm -f $(DEST_HTDOCS)/SignedVPClient.jar $(DEST_HTDOCS)/SignedMacVPClient.jar $(DEST_HTDOCS)/SignedInstaller.jar
