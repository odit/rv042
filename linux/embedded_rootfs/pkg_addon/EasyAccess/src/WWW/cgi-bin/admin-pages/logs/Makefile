BIN=logconfig eventlog
HTML=logconfig?.html eventlog*.html

all: $(BIN)

include ../../cgi-include.mk

logconfig: logconfig.c
	${CC} $^  $(CFLAGS) -o $@
	chmod ugo+s logconfig

eventlog: eventlog.c
	${CC} $^  $(CFLAGS) $(LDFLAGS) -o $@
	chmod ugo+s eventlog
