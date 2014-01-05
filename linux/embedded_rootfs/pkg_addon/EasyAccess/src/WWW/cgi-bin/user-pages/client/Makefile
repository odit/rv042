BIN=sslvpnclient virtualtransport clientlinux vpdesktopclient
HTML=sslvpnclient?.html sslvpnclientmac?.html virtualtransport.html sslvpnclientlinux?.html vpdesktopclient?.html

all: $(BIN)
include ../../cgi-include.mk

sslvpnclient: sslvpnclient.c
	${CC} $^  $(CFLAGS) -o $@

virtualtransport: virtualtransport.c
	${CC} $^  $(CFLAGS) -o $@

clientlinux: clientlinux.c
	${CC} $^  $(CFLAGS) -o $@

vpdesktopclient: vpdesktopclient.c
	${CC} $^  $(CFLAGS) -o $@
