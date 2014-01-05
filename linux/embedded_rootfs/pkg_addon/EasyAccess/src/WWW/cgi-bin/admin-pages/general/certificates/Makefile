BIN=newcsr sslcert viewcert delcert gencsr importcert
HTML=newcsr.html sslcert?.html viewcert?.html

all: $(BIN)

include ../../../cgi-include.mk

importcert: importcert.c
	${CC} $^  $(CFLAGS) -o $@
	chmod +xs importcert

gencsr: gencsr.c
	${CC} $^  $(CFLAGS) -o $@
	chmod +xs $@

newcsr: newcsr.c
	${CC} $^  $(CFLAGS) -o $@
	chmod +xs $@

sslcert: sslcert.c certLib.c
	${CC} $^  $(CFLAGS) -o $@
	chmod +xs sslcert

delcert: delcert.c
	${CC} $^  $(CFLAGS) -o $@
	chmod +xs $@

viewcert: viewcert.c certLib.c
	${CC} $^  $(CFLAGS) -o $@
	chmod +xs $@

test: test.c certLib.c
	${CC} $^  $(CFLAGS) -o $@
