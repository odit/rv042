BIN=date
HTML=date?.html

all: $(BIN)
include ../../../cgi-include.mk

date: date.c
	${CC} $^  $(CFLAGS) -o $@
	chmod +xs date
