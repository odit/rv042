BIN=desktop vnc tscbookmark deskBookmarkDelete adddesktopbookmark
HTML=desktop?.html  adddesktopbookmark.html
OTHER=

all: $(BIN)
include ../../cgi-include.mk


desktop: desktop.c
	${CC} $^  $(CFLAGS) -o $@

vnc: vnc.c
	${CC} $^  $(CFLAGS) -o $@

tscbookmark: tscbookmark.c
	${CC} $^  $(CFLAGS) -o $@

deskBookmarkDelete: deskBookmarkDelete.c
	${CC} $^  $(CFLAGS) -o $@

adddesktopbookmark: adddesktopbookmark.c
	${CC} $^  $(CFLAGS) -o $@
