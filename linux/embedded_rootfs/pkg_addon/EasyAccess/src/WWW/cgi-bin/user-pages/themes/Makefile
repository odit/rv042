BIN=themes
HTML=themes?.html
OTHER=

all: $(BIN)
include ../../cgi-include.mk

themes: themes.c
	${CC} $^  $(CFLAGS) -o $@
