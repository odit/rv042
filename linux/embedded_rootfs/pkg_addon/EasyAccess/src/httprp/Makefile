BIN = nph-httprp
HTML = eablank.html
XML_DIR = ../../3rdParty/libxml2
INCLUDES=	-I.				\
		-I$(XML_DIR)			\
		-I$(XML_DIR)/include		\
                -I$(SYS_LIB)

LIBS=	$(XML_DIR)/.libs/libxml2.a		\
       -L$(SYS_LIB) -lSys			\
	-lssl					\
	-lcrypto				\
        -lz

#	-lgdbm

 
DEFINES=	$(INCLUDES)			\
		-DSYS_UNIX=1			\
                -DOPENSSL_NO_KRB5		\
		-DHTTPRP_URL_WITH_DOT_PATCH	\
		-UHTTPRP_ENABLE_SESSION_CACHE   \
		-DHTTPRP_DEBUG_ASSERT		\


SRCS =	httprp.c	\
	httprp_cgi.c	\
	httprp_html.c	\
	httprp_buf.c	\
	httprp_nc.c	\
	httprp_enc.c

OBJS =	httprp.o	\
	httprp_cgi.o	\
	httprp_html.o	\
	httprp_buf.o	\
	httprp_nc.o	\
	httprp_enc.o

.c.o:
	$(RM) $@
	$(CC) $(CFLAGS) -c $*.c

all:  $(BIN)

include ../make-include.mk

CFLAGS= -Os $(DEFINES) \
	-Wall -Waggregate-return -Wcast-qual -Wcast-align -Wcomment \
        -Wformat -Wimplicit-int -Wimplicit-function-declaration -Winline \
	-Wlong-long -Wmain -Wmissing-prototypes -Wmissing-declarations \
	-Wnested-externs -Wno-import -Wparentheses -Wpointer-arith \
	-Wredundant-decls -Wreturn-type -Wswitch -Wunused -Wunused-function \
	-Wunused-label -Wunused-parameter -Wunused-variable -Wunused-value \
	-fPIC

$(XML_DIR)/.libs/libxml2.a:
	$(MAKE) -C $(XML_DIR) libxml2.la

$(BIN) : $(XML_DIR)/.libs/libxml2.a $(OBJS)
	$(CC) $(CFLAGS) -o $(BIN) $(OBJS) $(LIBS)

cleanbin:
	$(RM) $(BIN)
	if [ -n '$(LIB)' ]; then rm -rf $(LIB); fi

module-clean:
	$(RM) *.o *~
	$(RM) $(BIN)
	if [ -n '$(LIB)' ]; then $(RM) $(LIB); fi

clean:
	$(RM) *.o *~
	$(RM) $(BIN)
	$(MAKE) -C $(XML_DIR) clean
	if [ -n '$(LIB)' ]; then $(RM) $(LIB); fi

install:
	if [ -n '$(BIN)' ]; then $(STRIP) $(BIN); cp -rf $(BIN) $(DEST_CGI_BIN); fi
	if [ -n '$(HTML)' ]; then cp $(HTML) $(DEST_HTDOCS); fi

un-install:
	if [ -n '$(BIN)' ]; then cd $(DEST_CGI_BIN); rm -rf $(BIN); fi
