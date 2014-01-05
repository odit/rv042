BIN=changepassword
HTML=changepassword.html

all: $(BIN)
include ../../cgi-include.mk

changepassword: changepassword.c
	${CC} $^  $(CFLAGS) -o $@
