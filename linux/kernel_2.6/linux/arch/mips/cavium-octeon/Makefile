#
# Makefile for the Cavium Octeon specific kernel interface routines
# under Linux.
#

obj-y := setup.o serial.o int_handler.o hal.o perf_counters.o octeon_info.o
obj-y += dma-octeon.o userio.o

obj-$(CONFIG_SMP)                     += smp.o
obj-$(CONFIG_CAVIUM_OCTEON_BOOTBUS_COMPACT_FLASH) += ebt3000_cf.o
obj-$(CONFIG_PCI)                     += pci.o pci_chips.o i8259.o
#obj-$(CONFIG_CAVIUM_OCTEON_SIMULATOR) += simulator.o
obj-$(CONFIG_CAVIUM_OCTEON_POW_ONLY_ETHERNET) += ethernet-pow.o

EXTRA_CFLAGS := -I$(OCTEON_ROOT)/target/include -I$(OCTEON_ROOT)/executive

clean:
	rm -f *.o

