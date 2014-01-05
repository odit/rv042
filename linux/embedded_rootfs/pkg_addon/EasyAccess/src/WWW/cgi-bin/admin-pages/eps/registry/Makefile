BIN=epsregistry addregistrylist addepsregistry editepsregistry editregistrylist
HTML=epsregistry?.html addregistrylist?.html addepsregistry?.html editepsregistry?.html editregistrylist?.html

all: $(BIN)

include ../../../cgi-include.mk

epsregistry: epsregistry.c
	${CC} $^  $(CFLAGS) -o $@
	chmod ugo+s epsregistry

addregistrylist: addregistrylist.c
	${CC} $^  $(CFLAGS) -o $@
	chmod ugo+s addregistrylist

addepsregistry: addepsregistry.c
	${CC} $^  $(CFLAGS) -o $@
	chmod ugo+s addepsregistry

editepsregistry: editepsregistry.c
	${CC} $^  $(CFLAGS) -o $@
	chmod ugo+s editepsregistry

editregistrylist: editregistrylist.c
	${CC} $^  $(CFLAGS) -o $@
	chmod ugo+s editregistrylist
