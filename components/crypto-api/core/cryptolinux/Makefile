#
# Makefile for the Cavium Octeon specific kernel interface routines
# under Linux.
#

KERNELSRC := $(OCTEON_ROOT)/linux/kernel_2.6/linux
CRYPTOSRC := $(OCTEON_ROOT)/components/crypto-api/core/crypto
#CRYPTOSRC := /root/octsw/sdk/components/crypto-api/core/crypto

EXTRA_CFLAGS += -I$(OCTEON_ROOT)/target/include -I$(OCTEON_ROOT)/executive 
#EXTRA_CFLAGS += -DCRYPTO_DEBUG

obj-m := cavmodexp.o
cavmodexp-objs += crypto.o modexp.o mul_lin.o


modules:
	ln -sf $(CRYPTOSRC)/cryptolinux.h
	$(MAKE) -C $(KERNELSRC) M=$(PWD) modules

clean:
	rm -rf *.ko *.o *.mod.c *.mod.o .*.o.cmd .*.ko.cmd .tmp_versions 
	rm -f cryptolinux.h
