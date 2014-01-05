BIN=adddomain adddomainradius domains adddomainntdomain adddomainactivedirectory adddomainldap editdomain
HTML=adddomain?.html adddomainradius.html adddomainradius_chap.html adddomainradius_mschap.html adddomainradius_mschapv2.html domains?.html adddomainldap.html adddomainntdomain.html adddomainactivedirectory.html adddomainkerberos.html editdomain.html adddomainldap2.html

all: $(BIN)
include ../../../cgi-include.mk

domains: domains.c
	${CC} $^ $(CFLAGS) -o $@

adddomain: adddomain.c
	${CC} $^ $(CFLAGS) -o $@

adddomainradius: adddomainradius.c
	${CC} $^ $(CFLAGS) -o $@

adddomainldap: adddomainldap.c
	${CC} $^ $(CFLAGS) -o $@

adddomainntdomain: adddomainntdomain.c
	${CC} $^ $(CFLAGS) -o $@

adddomainactivedirectory: adddomainactivedirectory.c
	${CC} $^ $(CFLAGS) -o $@

editdomain: editdomain.c
	${CC} $^ $(CFLAGS) -o $@
