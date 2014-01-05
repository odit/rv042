SOURCES= RadiusLib.o\
	SmbAuthUtil.o\
        LDAPAuthUtil.o\
        ADAuthUtil.o

LIB=libAuth.so

all: $(LIB)
include ../make-include.mk

#Included /usr/include/et for Fedora, Redhat has it in /usr/kerberos/include
CFLAGS+=-DLDAP_DEPRECATED=1 -I$(SYS_LIB) -I/usr/kerberos/include -L/usr/kerberos/lib \
        -I /usr/include/et -I/usr/local/samba/include


$(LIB): $(SOURCES)
	$(CC) -shared $(CFLAGS) -o libAuth.so $(SOURCES)

cleanbin:
	rm $(BIN)
	if [ -n '$(LIB)' ]; then rm -rf $(LIB); fi

clean:
	rm -f *.o
	rm -f $(BIN)
	if [ -n '$(LIB)' ]; then rm -rf $(LIB); fi

install:
	if [ -n '$(LIB)' ]; then $(STRIP) $(LIB); cp -rf $(LIB) $(ACCESS_POINT_BIN); fi

un-install:
	if [ -n '$(LIB)' ]; then rm -rf $(ACCESS_POINT_BIN)/$(LIB); fi
