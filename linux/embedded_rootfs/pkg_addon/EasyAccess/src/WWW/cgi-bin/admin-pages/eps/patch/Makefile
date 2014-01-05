BIN=epspatch addpatchlist addepspatch editpatchlist
HTML=epspatch?.html addpatchlist?.html addepspatch?.html editpatchlist?.html

all: $(BIN)

include ../../../cgi-include.mk

epspatch: epspatch.c
	${CC} $^  $(CFLAGS) -o $@
	chmod ugo+s epspatch

addepspatch: addepspatch.c
	${CC} $^  $(CFLAGS) -o $@
	chmod ugo+s addepspatch

addpatchlist: addpatchlist.c
	${CC} $^  $(CFLAGS) -o $@
	chmod ugo+s addpatchlist

editpatchlist: editpatchlist.c
	${CC} $^  $(CFLAGS) -o $@
	chmod ugo+s editpatchlist
