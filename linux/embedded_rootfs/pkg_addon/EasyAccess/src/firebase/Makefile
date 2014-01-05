HEADERS=Firebase.h

SOURCES=Firebase.o\
        FirebaseMain.o\
        Utils.o\
	Options.o

BIN=firebase
all: $(BIN)
include ../make-include.mk

CFLAGS+=-I$(SYS_LIB)

firebase: $(SOURCES) $(SYS_LIB)/libSys.so $(HEADERS)
	${CC} $(CFLAGS) $(LDFLAGS) -o $@ $(SOURCES) -L$(SYS_LIB) -lSys -lcrypt -lssl -lnsl -ldl -lresolv -lm

cleanbin:
	rm $(BIN)
	if [ -n '$(LIB)' ]; then rm -rf $(LIB); fi

clean:
	rm -f *.o
	rm -f $(BIN)
	if [ -n '$(LIB)' ]; then rm -rf $(LIB); fi

install:
	if [ -n '$(BIN)' ]; then $(STRIP) $(BIN); cp -rf $(BIN) $(ACCESS_POINT_BIN); fi
	@cp firebase.default $(TOP_DIR)/var/conf/firebase.default
	@cp firebase.default $(TOP_DIR)/var/conf/firebase.conf
	@chmod 777 $(TOP_DIR)/var/conf/firebase.conf
	@chmod 777 $(TOP_DIR)/var/conf/firebase.default

un-install:
	if [ -n '$(BIN)' ]; then cd $(ACCESS_POINT_BIN); rm -rf $(BIN); fi
