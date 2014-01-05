BIN=dnssettings etchosts ifacesettings networkinterface staticroutes 
HTML=dnssettings?.html etchosts?.html ifacesettings?.html networkinterface?.html staticroutes?.html

all: $(BIN)
include ../../cgi-include.mk

dnssettings: dnssettings.c
	${CC} $^  $(CFLAGS) -o $@

etchosts: etchosts.c
	${CC} $^  $(CFLAGS) -o $@

ifacesettings: ifacesettings.c
	${CC} $^  $(CFLAGS) -o $@
	chmod  +s ifacesettings

networkinterface: networkinterface.c
	${CC} $^  $(CFLAGS) -o $@
	chmod  +s networkinterface

staticroutes: staticroutes.c
	${CC} $^  $(CFLAGS) -o $@
