#
# Makefile for DWC_otg Highspeed USB controller driver
#

EXTRA_CFLAGS	+= -Dlinux

# Use one of the following flags to compile the software in host-only or
# device-only mode.
#EXTRA_CFLAGS   += -DDWC_HOST_ONLY
#EXTRA_CFLAGS	+= -DDWC_DEVICE_ONLY

obj-$(CONFIG_USB_DWC_OTG)	:= dwc_otg.o
dwc_otg-objs	:= dwc_otg_driver.o dwc_otg_attr.o
dwc_otg-objs	+= dwc_otg_cil.o dwc_otg_cil_intr.o
dwc_otg-objs	+= dwc_otg_pcd.o dwc_otg_pcd_intr.o
dwc_otg-objs	+= dwc_otg_hcd.o dwc_otg_hcd_intr.o dwc_otg_hcd_queue.o

clean:
	rm -f *.o *.ko
