BIN=tscapplications tsc
HTML=tscapplications?.html
OTHER=

all: $(BIN)
include ../../cgi-include.mk

tscapplications: tscapplications.c
	${CC} $^  $(CFLAGS) -o $@

tsc: tsc.c
	${CC} $^  $(CFLAGS) -o $@
