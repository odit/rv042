HTTPD_HEADERS=Httpd.h\
	HttpdErrors.h\
        HttpdRewrite.h \
	Config.h

HTTPD_SOURCES= HttpdAlias.o\
	HttpdBuffer.o\
	Httpd.o\
	HttpdCgi.o\
	HttpdConfig.o\
	HttpdGet.o\
	HttpdLog.o\
	HttpdMain.o\
	HttpdRead.o\
	HttpdResponse.o\
	HttpdSSL.o\
	HttpdTunnel.o\
        HttpdRewrite.o\
	HttpdUtils.o

BIN=httpd

all: $(BIN)

include ../make-include.mk

CFLAGS+=-I$(TOP_DIR)/src

CFLAGS+=-DHTTPD_DEBUG -UHTTPD_ASSERT -DUSE_FORK -UHTTPD_USE_BLOCKING_TUNNEL_FD -UHTTPD_READ_CONFIG_FILE \
        -UHTTPD_ALTER_SOCKBUFSIZE -UHTTPD_READ_CONFIG_FILE -DOPENSSL_NO_KRB5 -DHTTPD_URL_REWRITE -DHTTPD_ENABLE_CLIENT_CERTS

ifdef CONFIG_IPV6
CFLAGS+=-DHTTPD_IPV6
endif

$(BIN): $(HTTPD_SOURCES) $(HTTPD_HEADERS)
	${CC} $(CFLAGS) $(LDFLAGS) -o $@ $(HTTPD_SOURCES) -L$(SYS_LIB) -lSys -lcrypt -lssl -lutil

cleanbin:
	rm $(BIN)
	if [ -n '$(LIB)' ]; then rm -rf $(LIB); fi

clean:
	rm -f *.o
	rm -f $(BIN)
	if [ -n '$(LIB)' ]; then rm -rf $(LIB); fi

install:
	if [ -n '$(BIN)' ]; then $(STRIP) $(BIN); cp -rf $(BIN) $(ACCESS_POINT_BIN); fi

un-install:
	if [ -n '$(BIN)' ]; then cd $(ACCESS_POINT_BIN); rm -rf $(BIN); fi
