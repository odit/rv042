BIN=homepage
HTML=homepage?.html
OTHER=

all: $(BIN)
include ../../cgi-include.mk

homepage: homepage.c
	${CC} $^  $(CFLAGS) -o $@
