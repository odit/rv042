include ../nkuserland.config

DIRS:=

#CFLAGS += -Wno-error

CSRC = util.c kd_sys.c flash.c interface.c getlink.c voice.c gethost.c ipv6.c
ifeq "$(CONFIG_NK_LICENSE_COUNT_DOWN)" "y"
CSRC += licenseKey.c
endif

COBJS=$(patsubst %.c,%.o, $(CSRC))

ifeq "$(CONFIG_NK_LICENSE_COUNT_DOWN)" "y"
OBJS = $(COBJS) ../web_mng/web_ui_Cgi/LicenseKey/fun_key.o
else
OBJS=$(COBJS)
endif

CFLAGS= -mabi=64

#---- added for using cvmx_get_cycle()
ifndef OCTEON_ROOT
OCTEON_ROOT = ../../../..
endif
CFLAGS += -I$(OCTEON_ROOT)/target/include -DOCTEON_MODEL=OCTEON_CN31XX
#----

all: $(OBJS)
	$(CC) -shared -o libnku.so $(OBJS)
#	$(AR) cru libnku.a $(OBJS)
#	$(RANLIB) libnku.a

$(COBJS): %.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@
	$(STRIP) -x -R .note -R .comment $*.o

.PHONY: dummy
dummy:

install:
	cp ./libnku.so ${OCTEON_ROOT}/tools/mips64-octeon-linux-gnu/sys-root/lib64/libnku.so
	cp ./libnku.so ${ROOT}/usr/lib64/libnku.so

clean:
	rm -f *.[oa] *~ core

