BIN=sitecustomization portallist homeconfig edittscapplication
HTML=sitecustomization?.html portallist?.html homeconfig?.html edittscapplication?.html



all: $(BIN)
include ../../../cgi-include.mk

sitecustomization: sitecustomization.c
	${CC} $^ $(CFLAGS) -o $@

portallist: portallist.c
	${CC} $^ $(CFLAGS) -o $@

homeconfig: homeconfig.c
	${CC} $^ $(CFLAGS) -o $@
