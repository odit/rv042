LIBS = -lnku
CHECK_UI_LOCK_OBJ = check_ui_lock.o
GET_ANTI_SPAM_OBJ = nk_qrtg.o

CFLAGS += -Wall -Wpointer-arith -Wcast-align -Wcast-qual -Wshadow \
 -Waggregate-return -Wnested-externs -Winline -Wwrite-strings \
 -Wstrict-prototypes 

CFLAGS += -O
allexec = check_ui_lock nk_qrtg

all:
#all: $(allexec)

check_ui_lock: $(CHECK_UI_LOCK_OBJ)
	$(CC) $(CFLAGS) -o $@ $(CHECK_UI_LOCK_OBJ) $(LIBS)
	$(STRIP) $@
nk_qrtg: $(GET_ANTI_SPAM_OBJ)
	$(CC) $(CFLAGS) -o $@ $(GET_ANTI_SPAM_OBJ) $(LIBS)
	$(STRIP) $@

install:
	$(CP) -f check_ui_lock $(DESTUSR)/sbin/check_ui_lock
	$(CP) -f test.sh $(DESTUSR)/sbin/test.sh
	$(CP) -f get_db.sh $(DESTUSR)/sbin/get_db.sh
ifeq "$(CONFIG_QTM3000)" "y"
	$(CP) -f nk_qrtg $(DESTUSR)/sbin/nk_qrtg
	$(CP) -f anti_spam.sh $(DESTUSR)/sbin/anti_spam.sh
endif	

clean:
	rm -f check_ui_lock *.o
ifeq "$(CONFIG_QTM3000)" "y"
	rm -f nk_qrtg *.o
endif
