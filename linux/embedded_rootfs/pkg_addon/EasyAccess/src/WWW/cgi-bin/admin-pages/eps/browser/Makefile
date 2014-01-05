BIN=epsbrowser addbrowserlist addepsbrowser editbrowserlist
HTML=epsbrowser?.html addbrowserlist?.html addepsbrowser?.html editbrowserlist?.html

all: $(BIN)

include ../../../cgi-include.mk

epsbrowser: epsbrowser.c
	${CC} $^  $(CFLAGS) -o $@
	chmod ugo+s epsbrowser

addbrowserlist: addbrowserlist.c
	${CC} $^  $(CFLAGS) -o $@
	chmod ugo+s addbrowserlist

addepsbrowser: addepsbrowser.c
	${CC} $^  $(CFLAGS) -o $@
	chmod ugo+s addepsbrowser

editbrowserlist: editbrowserlist.c
	${CC} $^  $(CFLAGS) -o $@
	chmod ugo+s editbrowserlist
