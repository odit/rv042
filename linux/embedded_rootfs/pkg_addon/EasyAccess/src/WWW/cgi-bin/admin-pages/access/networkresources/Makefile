BIN=networkresource addresource editresource
HTML=networkresource?.html addresource?.html editresource?.html

all: $(BIN)
include ../../../cgi-include.mk

networkresource: networkresource.c
	${CC} $^  $(CFLAGS) -o $@

addresource: addresource.c
	${CC} $^  $(CFLAGS) -o $@

editresource: editresource.c
	${CC} $^  $(CFLAGS) -o $@

test: test.c
	${CC} $^  $(CFLAGS) -o $@
