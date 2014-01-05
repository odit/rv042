BIN=clientaddresses clientroutes transportapps
HTML=clientaddresses?.html clientroutes?.html transportapps?.html

all: $(BIN)
include ../../cgi-include.mk

clientaddresses: clientaddresses.c
	${CC} $^  $(CFLAGS) -o $@

clientroutes: clientroutes.c
	${CC} $^  $(CFLAGS) -o $@

transportapps: transportapps.c
	${CC} $^  $(CFLAGS) -o $@
