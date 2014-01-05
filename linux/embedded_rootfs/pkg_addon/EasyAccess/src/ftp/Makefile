FTP_HEADERS=FtpProtocol.h FtpSession.h

FTP_SOURCES=FtpSession.o\
	FtpProtocol.o\

BIN=ftpsession
all: $(BIN)
include ../make-include.mk

CFLAGS+=-I$(SYS_LIB)

ftpsession: $(FTP_SOURCES) $(SYS_LIB)/libSys.so $(FTP_HEADERS)
	${CC} $(CFLAGS) $(LDFLAGS) -o $@ $(FTP_SOURCES) -L$(SYS_LIB) -lSys -lssl -lcrypto

ftp_test: ftp_test.o FtpProtocol.o $(SYS_LIB)/libSys.so
	${CC} $(CFLAGS) -o $@ $^ -L$(SYS_LIB) -lSys -lssl -lcrypt

cleanbin:
	rm $(BIN)

clean:
	rm -f *.o
	rm -f $(BIN)

install:
	@$(STRIP) $(BIN)
	@cp -rf $(BIN) $(ACCESS_POINT_BIN)

un-install:
	if [ -n '$(BIN)' ]; then cd $(ACCESS_POINT_BIN); rm -rf $(BIN); fi
