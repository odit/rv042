BIN=cifs cifs-upload
HTML=popup.html addcifsbookmark.html

all: $(BIN)
include ../../cgi-include.mk

CFLAGS += -L/usr/local/samba/lib -lsmbclient -ldl -lnsl -I /usr/local/samba/include -DCGI_DEBUG -UEXTRA_CHAR

cifs: cifs.c cifsHtml.c cifsSamba.c SmbLib.h cifs.h
	${CC} cifs.c  $(CFLAGS) -o $@

cifs-upload: cifs-upload.c cifs.h
	${CC} cifs-upload.c $(CFLAGS) -o $@
