/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2005, 2006 Cavium Networks
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/interrupt.h>
#include <asm/delay.h>
#include "hal.h"

static uint32_t chip_read_config(struct pci_dev *dev, int where, int size)
{
    uint32_t val;
    dev->bus->ops->read(dev->bus, dev->devfn, where, size, &val);
    return val;
}

static void chip_write_config(struct pci_dev *dev, int where, int size, uint32_t val)
{
    dev->bus->ops->write(dev->bus, dev->devfn, where, size, val);
}

static void chip_bridge_setup(struct pci_dev *dev)
{
    uint16_t val;
    chip_write_config(dev, 0x04, 2, 0x0026);
    val = chip_read_config(dev, 0x3e, 2);
    val &= ~(1<<2);                         /* Disable the ISA port filtering */
    val |= 1<<3;                            /* Enable VGA */
    val |= 1<<4;                            /* Enable VGA Alias Filter */
    chip_write_config(dev, 0x3e, 2, val);
    chip_write_config(dev, 0x1c, 1, 0x0);   /* Force IO port range to start at 0 for Via 686 ISA devices */
    chip_write_config(dev, 0x30, 1, 0x0);
    chip_write_config(dev, 0x04, 2, 0x0027);
}

static void chip_vt82c686b_setup(struct pci_dev *dev)
{
#ifdef CONFIG_PCI_NAMES
    printk("%s(bus=%d, dev=%d, func=%d, name=%s)\n", __FUNCTION__, dev->bus->number, dev->devfn>>3, dev->devfn&7, dev->pretty_name);
#endif
    chip_write_config(dev, 0x40, 1, 0x08);	// I/O Recovery Time Enable
    chip_write_config(dev, 0x41, 1, 0x41);	// I/O Recovery Time=8BCLKs, ISA Refresh
    chip_write_config(dev, 0x45, 1, 0x80);	// ISA Master /DMA to PCI Line Buffer Enable
    chip_write_config(dev, 0x46, 1, 0x60);	// GATE INTR Enable, Flush Line Buffer for Int or DMA IOR Cycle Enable
    chip_write_config(dev, 0x47, 1, 0xa0);	// CPU Reset source: INIT, EISA 4d0 /4d1 port enable
/*
Offset 48 - Miscellaneous Control 3 ................................ RW
  7-4 Reserved ........................................ always reads 0
   3    Extra RTC Port 74/75 Enable
            0 Disable ...................................................default
            1 Enable
   2    Integrated USB Controller Disable
            0 Enable ....................................................default
            1 Disable
   1    Integrated IDE Controller Disable
            0 Enable ....................................................default
            1 Disable
   0    512K PCI Memory Decode
            0 Use Rx4E[15-12] to select top of PCI memory
            1 Use contents of Rx4E[15-12] plus 512K as top
               of PCI memory.......................................default
*/
    chip_write_config(dev, 0x48, 1, 0x09);

/*
Offset 4A - IDE Interrupt Routing..................................RW
   7    Wait for PGNT Before Grant to ISA Master /
        DMA
           0 Disable ...................................................default
           1 Enable
   6    Bus Select for Access to I/O Devices Below 100h
           0 Access ports 00-FFh via XD bus ...........default
           1 Access ports 00-FFh via SD bus (applies to
              external devices only; internal devices such as
              the mouse controller are not effected)
  5-4 Reserved (do not program)..................... default = 0
  3-2 IDE Second Channel IRQ Routing
          00 IRQ14
          01 IRQ15.....................................................default
          10 IRQ10
          11 IRQ11
  1-0 IDE Primary Channel IRQ Routing
          00 IRQ14.....................................................default
          01 IRQ15
          10 IRQ10
          11 IRQ11
*/
    chip_write_config(dev, 0x4a, 1, 0x04);

/*
Offset 4F-4E - ISA DMA/Master Mem Access Ctrl 3 ... RW
 15-12 Top of PCI Memory for ISA DMA/Master accesses
         0000 1M .................................................... default
         0001 2M
          ... ...
         1111 16M
Note: All ISA DMA / Masters that access addresses higher
        than the top of PCI memory will not be directed to
        the PCI bus.
   11 Forward E0000-EFFFF Accesses to PCI ....... def=0
   10 Forward A0000-BFFFF Accesses to PCI ....... def=0
    9   Forward 80000-9FFFF Accesses to PCI ........def=1
    8   Forward 00000-7FFFF Accesses to PCI ........def=1
    7   Forward DC000-DFFFF Accesses to PCI ...... def=0
    6   Forward D8000-DBFFF Accesses to PCI ......def=0
    5   Forward D4000-D7FFF Accesses to PCI ......def=0
    4   Forward D0000-D3FFF Accesses to PCI ......def=0
    3   Forward CC000-CFFFF Accesses to PCI .....def=0
    2   Forward C8000-CBFFF Accesses to PCI ......def=0
    1   Forward C4000-C7FFF Accesses to PCI ......def=0
    0   Forward C0000-C3FFF Accesses to PCI ......def=0
*/
    chip_write_config(dev, 0x4e, 2, 0xf000);

/*
Offset 51 - PNP IRQ Routing 1........................................RW
  7-4 PnP Routing for Parallel Port IRQ (see PnP IRQ
        routing table)
  3-0 PnP Routing for Floppy IRQ (see PnP IRQ routing
        table)
*/
    chip_write_config(dev, 0x51, 1, 0x76);

/*
Offset 52 - PNP IRQ Routing 2........................................RW
  7-4 PnP Routing for Serial Port 2 IRQ (see PnP IRQ
        routing table)
  3-0 PnP Routing for Serial Port 1 IRQ (see PnP IRQ
        routing table)
*/
    chip_write_config(dev, 0x52, 1, 0x34);

/*
Offset 5A ­ KBC / RTC Control......................................RW
Bits 7-4 of this register are latched from pins SD7-4 at power-
up but are read/write accessible so may be changed after
power-up to change the default strap setting:
    7    Keyboard RP16........................... latched from SD7
    6    Keyboard RP15 .......................... latched from SD6
    5    Keyboard RP14 .......................... latched from SD5
    4    Keyboard RP13 .......................... latched from SD4
    3    Reserved ........................................ always reads 0
    2    Internal RTC Enable
            0 Disable
            1 Enable ...................................................default
    1    Internal PS2 Mouse Enable
            0 Disable ..................................................default
            1 Enable
    0    Internal KBC Enable
            0 Disable ..................................................default
            1 Enable
*/
    chip_write_config(dev, 0x5a, 1, chip_read_config(dev, 0x5a, 1) | 0x7);

/*
Offset 61-60 - Distributed DMA Ch 0 Base / Enable ..... RW
 15-4 Channel 0 Base Address Bits 15-4.......... default = 0
   3    Channel 0 Enable
           0 Disable ...................................................default
           1 Enable
  2-0 Reserved ........................................ always reads 0
*/
    chip_write_config(dev, 0x60, 2, 0x4);

/*
Offset 63-62 - Distributed DMA Ch 1 Base / Enable ..... RW
 15-4 Channel 1 Base Address Bits 15-4.......... default = 0
   3    Channel 1 Enable
           0 Disable ...................................................default
           1 Enable
  2-0 Reserved ........................................ always reads 0
*/
    chip_write_config(dev, 0x62, 2, 0x4);

/*
Offset 65-64 - Distributed DMA Ch 2 Base / Enable ..... RW
 15-4 Channel 2 Base Address Bits 15-4.......... default = 0
   3    Channel 2 Enable
           0 Disable ...................................................default
           1 Enable
  2-0 Reserved ........................................ always reads 0
*/
    chip_write_config(dev, 0x64, 2, 0x4);

/*
Offset 67-66 - Distributed DMA Ch 3 Base / Enable ..... RW
 15-4 Channel 3 Base Address Bits 15-4.......... default = 0
   3    Channel 3 Enable
           0 Disable ...................................................default
           1 Enable
  2-0 Reserved ........................................ always reads 0
*/
    chip_write_config(dev, 0x66, 2, 0x4);

/*
Offset 77 ­ GPIO Control 4 Control (10h)..................... RW
   7    DRQ / DACK# Pins are GPI / GPO
           0 Disable................................................... default
           1 Enable
   6    Game Port XY Pins are GPI / GPO
           0 Disable................................................... default
           1 Enable
   5    Reserved    ........................................always reads 0
   4    Internal APIC Enable
           0 Disable
           1 Enable (U10 = WSC#, V9 = APICD0, T10 =
              APICD1)................................................ default
   3    IRQ0 Output
           0 Disable................................................... default
           1 Enable IRQ0 output to GPIOC
   2    RTC Rx32 Write Protect
           0 Disable................................................... default
           1 Enable
   1    RTC Rx0D Write Protect
           0 Disable................................................... default
           1 Enable
   0    GPO13 Enable (Pin U5)
           0 Pin defined as SOE#.............................. default
           1 Pin defined as GPO13
*/
    chip_write_config(dev, 0x77, 1, 0x40);	// Enable GPI22

/*
Offset 81 ­ ISA Positive Decoding Control 1..................RW
 7  On-Board I/O Port Positive Decoding
      0 Disable ...................................................default
      1 Enable
 6  Microsoft-Sound System I/O Port Positive
    Decoding
      0 Disable ...................................................default
      1 Enable
5-4 Microsoft-Sound System I/O Decode Range
     00 0530h-0537h ..........................................default
     01 0604h-060Bh
     10 0E80-0E87h
     11 0F40h-0F47h
 3  APIC Positive Decoding
      0 Disable ...................................................default
      1 Enable
 2  BIOS ROM Positive Decoding
      0 Disable ...................................................default
      1 Enable
 1  Reserved ........................................ always reads 0
 0  PCS0 Positive Decoding
      0 Disable ...................................................default
      1 Enable
*/
    chip_write_config(dev, 0x81, 1, 0xc0);

/*
Offset 82 ­ ISA Positive Decoding Control 2..................RW
   7    FDC Positive Decoding
            0 Disable ...................................................default
            1 Enable
   6    LPT Positive Decoding
            0 Disable ...................................................default
            1 Enable
  5-4 LPT Decode Range
           00 3BCh-3BFh, 7BCh-7BEh ......................default
           01 378h-37Fh, 778h-77Ah
           10 278h-27Fh, 678h-67Ah
           11 -reserved-
   3    Game Port Positive Decoding
            0 Disable ...................................................default
            1 Enable
   2    MIDI Positive Decoding
            0 Disable ...................................................default
            1 Enable
  1-0 MIDI Decode Range
           00 300h-303h ..............................................default
           01 310h-313h
           10 320h-323h
           11 330h-333h
*/
    chip_write_config(dev, 0x82, 1, 0xdc);

/*
Offset 83 ­ ISA Positive Decoding Control 3 ................. RW
   7    COM Port B Positive Decoding
           0 Disable................................................... default
           1 Enable
  6-4 COM-Port B Decode Range
          000 3F8h-3FFh (COM1)............................ default
          001 2F8h-2FFh (COM2)
          010 220h-227h
          011 228h-22Fh
          100 238h-23Fh
          101 2E8h-2EFh (COM4)
          110 338h-33Fh
          111 3E8h-3EFh (COM3)
   3    COM Port A Positive Decoding
           0 Disable................................................... default
           1 Enable
  2-0 COM-Port A Decode Range
          000 3F8h-3FFh (COM1)............................ default
          001 2F8h-2FFh (COM2)
          010 220h-227h
          011 228h-22Fh
          100 238h-23Fh
          101 2E8h-2EFh (COM4)
          110 338h-33Fh
          111 3E8h-3EFh (COM3)
*/
    chip_write_config(dev, 0x83, 1, 0x98);

/*
Offset 84 ­ ISA Positive Decoding Control 4 ................. RW
  7-5 Reserved        ........................................always reads 0
   4    CD: Reserved.....................................always reads 0
        CE: Port CF9 Positive Decoding
            0 Disable
            1 Enable................................................... default
   3    FDC Decoding Range
            0 Primary .................................................. default
            1 Secondary
   2    Sound Blaster Positive Decoding
            0 Disable................................................... default
            1 Enable
  1-0 Sound Blaster Decode Range
           00 220h-22Fh, 230h-233h .......................... default
           01 240h-24Fh, 250h-253h
           10 260h-26Fh, 270h-273h
           11 280h-28Fh, 290h-293h
*/
    chip_write_config(dev, 0x84, 1, 0x04);

/*
Offset 85 ­ Extended Function Enable............................RW
  7-6 PCI Master Grant Timeout Select
           00 Disable ...................................................default
           01 32 PCI Clocks
           10 64 PCI Clocks
           11 96 PCI Clocks
   5    Keyboard Controller Configuration
            0 Disable ...................................................default
            1 Enable
   4    Function 3 USB Ports 2-3
            0 Enable ....................................................default
            1 Disable
   3    Function 6 Modem / Audio
            0 Enable ....................................................default
            1 Disable
   2    Function 5 Audio
            0 Enable ....................................................default
            1 Disable
   1    Super-I/O Configuration
            0 Disable ...................................................default
            1 Enable
   0    Super-I/O
            0 Disable ...................................................default
            1 Enable
*/
    chip_write_config(dev, 0x85, 1, 0x23);

/*
Index E0 ­ Super-I/O Device ID (3Ch) ............................ RO
  7-0 Super-I/O ID ........................................ default = 3Ch
*/
    outb(0xe0, 0x3f0);
    if (inb(0x3f1) != 0x3c)
        printk("    ERROR: Super-I/O Device ID not found (read 0x%x, expected 0x3c)\n", inb(0x3f1));

/*
Index E2 ­ Super-I/O Function Select (03h)...................RW
  7-5 Reserved ........................................ always reads 0
   4   Floppy Controller Enable
           0 Disable ...................................................default
           1 Enable
   3   Serial Port 2 Enable
           0 Disable ...................................................default
           1 Enable
   2   Serial Port 1 Enable
           0 Disable ...................................................default
           1 Enable
  1-0 Parallel Port Mode / Enable
          00 Unidirectional mode
          01 ECP
          10 EPP
          11 Parallel Port Disable ..............................default
*/
    outb(0xe2, 0x3f0);
    outb(0x1d, 0x3f1);

    /* Set the floppy controller address */
    outb(0xe3, 0x3f0);
    outb(0x3f0>>2, 0x3f1); /* PC Legacy default is 0x3f0 */

    /* Set the LTP port address */
    outb(0xe6, 0x3f0);
    outb(0x378>>2, 0x3f1); /* PC Legacy default is 0x378 */

    /* Set the Serail Port 1 port address */
    outb(0xe7, 0x3f0);
    outb(0x3f8>>2, 0x3f1); /* PC Legacy default is 0x3f8 */

    /* Set the Serail Port 2 port address */
    outb(0xe8, 0x3f0);
    outb(0x2f8>>2, 0x3f1); /* PC Legacy default is 0x2f8 */

/*
Index F6 ­ Floppy Controller Configuration................. RW
  7-6 Reserved         ........................................always reads 0
   5    Floppy Drive On Parallel Port
           0 Parallel Port (SPP) Mode ...................... default
           1 FDC Mode
   4    3-Mode FDD
           0 Disable................................................... default
           1 Enable
   3    Reserved       ........................................always reads 0
   2    Four Floppy Drive Option
           0 Internal 2-Drive Decoder....................... default
           1 External 4-Drive Decoder
   1    FDC DMA Non-Burst
           0 Burst .................................................... default
           1 Non-Burst
   0    FDC Swap
           0 Disable................................................... default
           1 Enable
*/
    outb(0xf6, 0x3f0);
    outb(0x20, 0x3f1);

    printk("    Disabling   [Super-I/O Configuration]\n");
    chip_write_config(dev, 0x85, 1, 0x21);
}

static void chip_vt82c686b_usb_setup(struct pci_dev *dev)
{
    printk("Routing VT82C686B USB interrupt to legacy irq 13\n");
    chip_write_config(dev, 0x3c, 1, 80 + 13);
}

static void chip_vt82c686b_ide_setup(struct pci_dev *dev)
{
    printk("Enabling VT82C686B IDE channels\n");
    chip_write_config(dev, 0x40, 1, chip_read_config(dev, 0x40, 1) | 3);
    chip_write_config(dev, 0x04, 1, 0x87);	// Enable Bus Master I/O space,Memory space
    chip_write_config(dev, 0x0d, 1, 0x40);	// Latency Timer
    chip_write_config(dev, 0x3c, 1, 0x0e); 	// Interrupt Line
    chip_write_config(dev, 0x41, 1, chip_read_config(dev, 0x41, 1) | 0xc0);  // enable read prefectch/post write buffer
    chip_write_config(dev, 0x43, 1, 0x05); 	// FIFO configuration 1/2
    chip_write_config(dev, 0x44, 1, 0x1c); 	// Miscellaneous Control
    chip_write_config(dev, 0x45, 1, 0x00); 	// Miscellaneous Control

    chip_write_config(dev, 0x46, 1, 0xc0); 	// Disable DMA FIFO flush
    chip_write_config(dev, 0x48, 4, 0xa8a8a8a8);	// Drive Timing Control
    chip_write_config(dev, 0x4c, 1, 0xff);		// Address Setup Time

    chip_write_config(dev, 0x50, 4, 0x07070707);	// UltraDMA Extended Timing Control
    chip_write_config(dev, 0x54, 1, 0x04);		// UltraDMA FIFO control

    chip_write_config(dev, 0x04, 2, 0x87); 	// Eanable I/O space, Bus Master
}

static void chip_vt82c686b_pmio_setup(struct pci_dev *dev)
{
    uint32_t base;
    u8 val;

    chip_write_config(dev, 0x041, 2, 0x80);	// Eanble PM/IO base
    chip_write_config(dev, 0x048, 2, 0x500);	// Set PM/IO base to 0x500

    base = chip_read_config(dev, 0x48, 4);	// read PM/IO base
    base = base & 0xff00;
    val = inb(base+0x4a);			// bit 6 is IDE cable type 0:80w, 1:40w
    outb(0x10, 0x70);				// write to CMOS offset=0x10
    outb(val,  0x71);
    outb(0x10, 0x70);
    val = inb(0x71);
    printk("Detect VIA IDE Cable Type: %s\n", (val & 0x40) ? "40w" : "80w");	// IDE Cable type GPI22, Offset 0x4a, bit 6
}

void octeon_pci_chip_init(struct pci_dev *dev)
{
    uint32_t pciid = (dev->vendor<<16) | dev->device;
    switch (pciid)
    {
        case 0x8086537c: /* Intel 31154 133MHz PCI Bridge */
        case 0x10b56520: /* PLX Bridge */
            chip_bridge_setup(dev);
            break;
        case 0x10b56540: /* PLX 6540 Bridge - with smart prefetching */
            {
                uint8_t buffer_control;
                printk("Enabling smart prefetch and divided fifos for PLX 6540 bridge\n");
                chip_write_config(dev, 0x48, 1, (2<<1) /* PCIX 4 cache lines */ | (5<<3) /* PCI 20 dwords */); /* Primary bus prefetch */
                chip_write_config(dev, 0x4a, 1, (8<<2) /* 32 dwords */); /* Primary Incremental Prefetch Count */
                chip_write_config(dev, 0x4c, 1, 96 /* max dwords prefetch */); /* Primary Maximum Prefetch Count */
                buffer_control = chip_read_config(dev, 0x4f, 1);
                buffer_control |= 1<<1; /* Enable smart prefetch */
                buffer_control |= 1<<2; /* Divide fifos into four */
                chip_write_config(dev, 0x4f, 1, buffer_control);
            }
            break;
        case 0x11060686: /* VT82C686B Super South South Bridge */
            chip_vt82c686b_setup(dev);
            break;
        case 0x11063038: /* VT82C686B USB ports */
            chip_vt82c686b_usb_setup(dev);
            break;
        case 0x11060571: /* VT82C686B Bus Master IDE */
            chip_vt82c686b_ide_setup(dev);
            break;
        case 0x11063057: /* VT82C686B Bus Master IDE */
            chip_vt82c686b_pmio_setup(dev);
            break;
        default:
            break;
    }
}
