BIN=services ssh telnet servicesBookmarkDelete http https ftp-addnew ftp-upload ftp-download ftp-logout ftp-rename ftp-frame addservicesbookmark  showBookmark editBookmark
HTML=services?.html  ftp*.html  addservicesbookmark.html userBookmark?.html
OTHER=

all: all-bin
include ../../cgi-include.mk

all-bin: $(BIN)

services: services.c
	${CC} $^  $(CFLAGS) -o $@

http: http.c
	${CC} $^  $(CFLAGS) -o $@

https: https.c
	${CC} $^  $(CFLAGS)  -o $@

ssh: ssh.c
	${CC} $^  $(CFLAGS) -o $@

telnet: telnet.c
	${CC} $^  $(CFLAGS) -o $@

servicesBookmarkDelete: servicesBookmarkDelete.c
	${CC} $^  $(CFLAGS) -o $@

ftp-addnew: ftp-addnew.c
	${CC} $^  $(CFLAGS) -o $@

ftp-upload: ftp-upload.c Tty.c
	${CC} $^  $(CFLAGS) -lutil -o $@ 

ftp-rename: ftp-rename.c
	${CC} $^  $(CFLAGS) -o $@

ftp-download: ftp-download.c Tty.c
	${CC} $^  $(CFLAGS) -lutil -o $@

ftp-logout: ftp-logout.c
	${CC} $^  $(CFLAGS) -o $@

ftp-frame: ftp-frame.c
	${CC} $^  $(CFLAGS) -o $@

addservicesbookmark: addservicesbookmark.c
	${CC} $^  $(CFLAGS) -o $@

showBookmark: showBookmark.c
	${CC} $^  $(CFLAGS) -o $@

editBookmark: editBookmark.c
	${CC} $^  $(CFLAGS) -o $@
