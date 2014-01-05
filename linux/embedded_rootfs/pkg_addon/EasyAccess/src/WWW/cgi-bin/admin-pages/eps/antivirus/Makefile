BIN=epsantivirus addantiviruslist addepsantivirus editepsantivirus editantiviruslist
HTML=epsantivirus?.html addantiviruslist?.html addepsantivirus?.html editepsantivirus?.html editantiviruslist?.html

all: $(BIN)

include ../../../cgi-include.mk

epsantivirus: epsantivirus.c
	${CC} $^  $(CFLAGS) -o $@
	chmod ugo+s epsantivirus

addantiviruslist: addantiviruslist.c
	${CC} $^  $(CFLAGS) -o $@
	chmod ugo+s addantiviruslist

addepsantivirus: addepsantivirus.c
	${CC} $^  $(CFLAGS) -o $@
	chmod ugo+s addepsantivirus

editepsantivirus: editepsantivirus.c
	${CC} $^  $(CFLAGS) -o $@
	chmod ugo+s editepsantivirus

editantiviruslist: editantiviruslist.c
	${CC} $^  $(CFLAGS) -o $@
	chmod ugo+s editantiviruslist
