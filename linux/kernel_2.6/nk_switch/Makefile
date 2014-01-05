#
# Makefile for the BCM5397/5398 switch driver
#

#CAVIUM_INCLUDES := -I ${SUBDIRS}/config -I ${OCTEON_ROOT}/target/include -I ${OCTEON_ROOT}/executive

# RTL8367_INCLUDES: RTL8367_API to Include Files
RTL8367_INCLUDES := -I ${OCTEON_ROOT}/linux/kernel_2.6/nk_switch/ -I ${OCTEON_ROOT}/linux/kernel_2.6/nk_switch/RTL8370_RTL8367_API
# RTL8367_FLAGS: RTL8367_API to Use MDC/MDIO Interface
RTL8367_FLAGS := -DMDC_MDIO_OPERATION
CAVIUM_INCLUDES :=  -I ${OCTEON_ROOT}/target/include -I ${OCTEON_ROOT}/executive ${RTL8367_INCLUDES} ${RTL8367_FLAGS}

kernel_source := ../linux

# Common flags to be passed for driver compilation
EXTRA_CFLAGS += -Winline -Wall ${CAVIUM_INCLUDES}

default: all
obj-m         :=  nk_switch.o
nk_switch-objs := switch.o Rtl8306_AsicDrv.o Rtl8306_Driver_s.o Rtl8306_Driver_sd.o \
		nk_switch_drv.o nk_gpio.o nk_mdio.o nk_rtl_serial_format.o nk_bcm_virtual_phy.o nk_rtl8309_drv.o nk_rtl8326_drv.o nk_switch_util.o nk_rtl8306_drv.o nk_rtl8329_drv.o
# 		./RTL8370_RTL8367_API/smi.o ./RTL8370_RTL8367_API/rtl8370_asicdrv.o ./RTL8370_RTL8367_API/rtl8370_asicdrv_phy.o \
# 		./RTL8370_RTL8367_API/rtk_api.o ./RTL8370_RTL8367_API/rtl8370_asicdrv_lut.o ./RTL8370_RTL8367_API/rtl8370_asicdrv_port.o\

nk_switch.o: $(nk_switch-objs)
	$(LD) -r $^ -o $@

#config/cvmx-config.h: config/executive-config.h ${OCTEON_ROOT}/executive/cvmx-resources.config
#	cvmx-config config/executive-config.h

all:
	$(MAKE) -C $(kernel_source) SUBDIRS=`pwd`  modules;

clean:
	rm -f *.mod.c *.o *.ko
