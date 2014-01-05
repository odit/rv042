BIN=status activeusers activeconnections
HTML=status?.html activeusers?.html activeconnections?.html

all: $(BIN)
include ../../../cgi-include.mk

status: status.c
	${CC} $^  $(CFLAGS) -o $@
	chmod ugo+s status

activeusers: activeusers.c
	${CC} $^  $(CFLAGS) -o $@

activeconnections: activeconnections.c
	${CC} $^  $(CFLAGS) -o $@
