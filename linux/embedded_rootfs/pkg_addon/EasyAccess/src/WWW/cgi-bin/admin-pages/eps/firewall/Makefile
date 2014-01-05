BIN=epsfirewall addepsfirewall addfirewalllist editepsfirewall editepsfirewalllist
HTML=epsfirewall?.html addepsfirewall?.html addfirewalllist?.html editepsfirewall?.html editepsfirewalllist?.html

all: $(BIN)

include ../../../cgi-include.mk

epsfirewall: epsfirewall.c
	${CC} $^  $(CFLAGS) -o $@
	chmod ugo+s epsfirewall

addepsfirewall: addepsfirewall.c
	${CC} $^  $(CFLAGS) -o $@
	chmod ugo+s addepsfirewall

addfirewalllist: addfirewalllist.c
	${CC} $^  $(CFLAGS) -o $@
	chmod ugo+s addfirewalllist

editepsfirewall: editepsfirewall.c
	${CC} $^  $(CFLAGS) -o $@
	chmod ugo+s editepsfirewall

editfirewalllist: editepsfirewalllist.c
	${CC} $^  $(CFLAGS) -o $@
	chmod ugo+s editepsfirewalllist
