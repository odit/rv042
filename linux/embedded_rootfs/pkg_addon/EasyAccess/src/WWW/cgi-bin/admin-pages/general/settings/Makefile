BIN=settings exportConfigFile importconfig restoredefault restart savenow saveoptions encryptoptions upgradefirmware 
#BIN +=importlogo importlicense layout 

HTML=settings?.html  importconfig.html restart.html upgradefirmware.html 
#HTML+=importlicense.html layout?.html 

all: $(BIN)
include ../../../cgi-include.mk

settings: settings.c
	${CC} $^  $(CFLAGS) -o $@

savenow: savenow.c
	${CC} $^  $(CFLAGS) -o $@

saveoptions: saveoptions.c
	${CC} $^  $(CFLAGS) -o $@

encryptoptions: encryptoptions.c
	${CC} $^  $(CFLAGS) -o $@

exportConfigFile: exportConfigFile.c
	${CC} $^  $(CFLAGS) -o $@

importconfig: importconfig.c
	${CC} $^  $(CFLAGS)  -o $@
	chmod +xs importconfig

upgradefirmware: upgradefirmware.c
	${CC} $^  $(CFLAGS)  -o $@
	chmod +xs upgradefirmware

restoredefault: restoredefault.c
	${CC} $^  $(CFLAGS)  -o $@
	chmod  +xs $@

restart: restart.c
	${CC} $^  $(CFLAGS)  -o $@
	chmod  +xs restart

importlogo: importlogo.c
	${CC} $^  $(CFLAGS)  -o $@
	chmod  +xs importlogo

importlicense: importlicense.c
	${CC} $^  $(CFLAGS)  -o $@
	chmod  +xs $@

layout: layout.c
	${CC} $^  $(CFLAGS)  -o $@
