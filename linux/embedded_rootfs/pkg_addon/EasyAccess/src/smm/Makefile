SMM_HEADERS=ConfMgr.h\
	SmmMain.h\

SMM_SOURCES=Smm.o\
	SmmMain.o\
	ConfMgr.o\
	UserMgr.o\
	Options.o \
        Utils.o \
        TimerLib.o \
	PortalMgr.o \
	IptablesMgr.o

BIN=smm

all: $(BIN)

include ../make-include.mk

CFLAGS+=-I$(SYS_LIB) -I$(AUTH_LIB)

smm: $(SMM_SOURCES) $(SYS_LIB)/libSys.so $(SMM_HEADERS)
	${CC} $(CFLAGS) $(LDFLAGS) -o $@ $(SMM_SOURCES) -L$(SYS_LIB) -lSys -lcrypt -lssl

cleanbin: 
	rm $(BIN)
	if [ -n '$(LIB)' ]; then rm -rf $(LIB); fi

clean:
	rm -f *.o
	rm -f $(BIN)
	if [ -n '$(LIB)' ]; then rm -rf $(LIB); fi

install:
	if [ -n '$(BIN)' ]; then $(STRIP) $(BIN); cp -rf $(BIN) $(ACCESS_POINT_BIN); fi
	@cp smm.default $(TOP_DIR)/var/conf/smm.default
	@cp smm.default $(TOP_DIR)/var/conf/smm.conf
	@chmod 777 $(TOP_DIR)/var/conf/smm.conf
	@chmod 777 $(TOP_DIR)/var/conf/smm.default

un-install:
	if [ -n '$(BIN)' ]; then cd $(ACCESS_POINT_BIN); rm -rf $(BIN); fi
