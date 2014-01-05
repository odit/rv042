#
# Makefile for the BCM5397/5398 switch driver
#

#CAVIUM_INCLUDES := -I ${SUBDIRS}/config -I ${OCTEON_ROOT}/target/include -I ${OCTEON_ROOT}/executive
CAVIUM_INCLUDES :=  -I ${OCTEON_ROOT}/target/include -I ${OCTEON_ROOT}/executive
kernel_source := ../linux

# Common flags to be passed for driver compilation
EXTRA_CFLAGS += -Winline -Wall ${CAVIUM_INCLUDES}

default: all
obj-m         :=  nk_netlink.o
nk_netlink-objs := netlink_k.o Rtl8306_AsicDrv.o Rtl8306_Driver_s.o Rtl8306_Driver_sd.o

nk_netlink.o: $(nk_netlink-objs)
	$(LD) -r $^ -o $@

#config/cvmx-config.h: config/executive-config.h ${OCTEON_ROOT}/executive/cvmx-resources.config
#	cvmx-config config/executive-config.h

all:
	$(MAKE) -C $(kernel_source) SUBDIRS=`pwd`  modules;

clean:
	rm -f *.mod.c *.o *.ko
