BIN=epspolicy addepspolicy epsfwpolicy epsavpolicy epsregistrypolicy epsbrowserpolicy epspatchpolicy 
HTML=epspolicy?.html addepspolicy?.html epsfwpolicy?.html epsavpolicy?.html epsregistrypolicy?.html epsbrowserpolicy?.html epspatchpolicy?.html

all: $(BIN)

include ../../../cgi-include.mk

epspolicy: epspolicy.c
	${CC} $^  $(CFLAGS) -o $@
	chmod ugo+s epspolicy

addepspolicy: addepspolicy.c
	${CC} $^  $(CFLAGS) -o $@
	chmod ugo+s addepspolicy

epsfwpolicy: epsfwpolicy.c
	${CC} $^  $(CFLAGS) -o $@
	chmod ugo+s epsfwpolicy

epsavpolicy: epsavpolicy.c
	${CC} $^  $(CFLAGS) -o $@
	chmod ugo+s epsavpolicy

epsregistrypolicy: epsregistrypolicy.c
	${CC} $^  $(CFLAGS) -o $@
	chmod ugo+s epsregistrypolicy

epsbrowserpolicy: epsbrowserpolicy.c
	${CC} $^  $(CFLAGS) -o $@
	chmod ugo+s epsbrowserpolicy

epspatchpolicy: epspatchpolicy.c
	${CC} $^  $(CFLAGS) -o $@
	chmod ugo+s epspatchpolicy


