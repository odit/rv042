/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2006 Cavium Networks
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/interrupt.h>
#include "hal.h"

#define SLAVE           (0xa0 - 0x20)

/* Initialization Command Word 1 (ICW1 address 0x20 or 0xa0) */
/* 7:5 Interrupt Vector Addresses for MCS-80/85 Mode. */
#define ICW1_ADDRESS    0x20
#define ICW1            0x10    // 4   Must be set to 1 for ICW1
#define ICW1_LEVEL_TRIG (1<<3)  // 3   1 Level Triggered Interrupts, 0 Edge Triggered Interrupts
#define ICW1_INTERVAL4  (1<<2)  // 2   1 Call Address Interval of 4, 0 Call Address Interval of 8
#define ICW1_SINGLE_PIC (1<<1)  // 1   1 Single PIC, 0 Cascaded PICs
#define ICW1_NEED_ICW4  (1<<0)  // 0   1 Will be Sending ICW4, 0 Don't need ICW4

/* Initialization Command Word 2 (ICW2 address 0x21 or 0xa1) */
#define ICW2_ADDRESS    0x21
/* Bit 8086/8080 Mode  MCS 80/85 Mode
    7   I7              A15
    6   I6              A14
    5   I5              A13
    4   I4              A12
    3   I3              A11
    2   -               A10
    1   -               A9
    0   -               A8 */

/* Initialization Command Word 3 (ICW3 address 0x21 or 0xa1)
    For the master, this is a bitfield saying which line is hooked to a slave.
    For a slave, this is the slave's ID, the line it is hooked to */
#define ICW3_ADDRESS    0x21

/* Initialization Command Word 4 (ICW4 address 0x21 or 0xa1) */
/* Bits 7-5 are reserved */
#define ICW4_ADDRESS        0x21
#define ICW4_FULLY_NESTED   (1<<4)  // 4   1 Special Fully Nested Mode, 0 Not Special Fully Nested Mode
#define ICW4_BUFFERED       (3<<2)  // 3   1 Buffered Mode, 0 Unbuffered
#define ICW4_MASTER         (2<<2)  // 2   1 Master, 0 Slave
#define ICW4_AUTO_EOI       (1<<1)  // 1   1 Auto EOI, 0 Normal EOI
#define ICW4_8086           (1<<0)  // 0   1 8086/8080 Mode, 0 MCS-80/85

/* Operation Control Word 1 (OCW1 address 0x21 or 0xa1)
    This is a bitmask for each interrupt */
#define OCW1_ADDRESS    0x21

/* Operation Control Word 2 (OCW2 address 0x20 or 0xa0) */
#define OCW2_ADDRESS    0x20
#define OCW2            0x00    // Bits 4:3 must be zero
#define OCW2_ROTATE_AUTO_EOI_CLEAR  (0<<5)  // 7:5     000 Rotate in Auto EOI Mode (Clear)
#define OCW2_NON_SPECIFIC_EOI       (1<<5)  //         001 Non Specific EOI
#define OCW2_NOP                    (2<<5)  //         010 NOP
#define OCW2_SPECIFIC_EOI           (3<<5)  //         011 Specific EOI
#define OCW2_ROTATE_AUTO_EOI_SET    (4<<5)  //         100 Rotate in Auto EOI Mode (Set)
#define OCW2_ROTATE_NON_SPECIFIC_EOI (5<<5) //         101 Rotate on Non-Specific EOI
#define OCW2_SET_PRIORITY           (6<<5)  //         110 Set Priority Command (Use Bits 2:0)
#define OCW2_ROTATE_SPECIFIC_EOI    (7<<5)  //         111 Rotate on Specific EOI (Use Bits 2:0)

/* Operation Control Word 3 (OCW3 address 0x20 or 0xa0) */
/* Bit 7 Must be set to 0 */
#define OCW3_ADDRESS    0x20
#define OCW3            0x08            // 4:3     Must be set to 01
#define OCW3_RESET_SPECIAL_MASK (2<<5)  // 6:5     00 Reserved, 01 Reserved, 10 Reset Special Mask
#define OCW3_SET_SPECIAL_MASK   (3<<5)  //         11 Set Special Mask
#define OCW3_POLL               (1<<2)  // 2       1 Poll Command, 0 No Poll Command
#define OCW3_READ_IRR           (2<<0)  // 1:0     00 Reserved, 01 Reserved, 10 Next Read Returns Interrupt Request Register
#define OCW3_READ_ISR           (3<<0)  //         11 Next Read Returns In-Service Register


static irqreturn_t octeon_i8259_interrupt(int cpl, void *dev_id, struct pt_regs *regs)
{
    u8 master_isr;
    u8 slave_isr;

    outb(OCW3|OCW3_POLL, OCW3_ADDRESS);
    master_isr = inb(OCW3_ADDRESS);
    if (master_isr & 0x80)  /* Top bit is set if the master requested the interrupt */
    {
        if ((master_isr & 0x7) == 2)
        {
            outb(OCW3|OCW3_POLL, OCW3_ADDRESS + SLAVE);
            slave_isr = inb(OCW3_ADDRESS + SLAVE);
            if (slave_isr & 0x80)   /* Top bit is set if the slave requested the interrupt */
            {
                int irq = (slave_isr&7) + 8 + 80;

                //printk("8259: Interrupt %d from slave\n", irq);
                if (irq_desc[irq].action)
                    do_IRQ(irq, regs);

                /* Ack the slave */
                outb(OCW2 | OCW2_SPECIFIC_EOI | (slave_isr&7), OCW2_ADDRESS + SLAVE);
            }
            else
                printk("8259: Spurious interrupt from master for slave\n");
        }
        else
        {
            int irq = (master_isr&7) + 80;
            //printk("8259: Interrupt %d from master\n", irq);
            if (irq_desc[irq].action)
                do_IRQ(irq, regs);
        }

        /* Ack the master */
        outb(OCW2 | OCW2_SPECIFIC_EOI | (master_isr&7), OCW2_ADDRESS);

        return IRQ_HANDLED;
    }
    else
    {
        printk("8259: Spurious interrupt from master\n");
        return IRQ_NONE;
    }
}

void octeon_i8259_setup(int irq_line)
{
    printk("8259: Initializing\n");

    /* Setup the Master 8259 */
    outb(ICW1|ICW1_NEED_ICW4, ICW1_ADDRESS);                /* Begin the init sequence */
    outb(0, ICW2_ADDRESS);                                  /* Master base address is zero, interrupts 0-7 */
    outb(1<<2, ICW3_ADDRESS);                               /* Slave is connected to line 2 */
    outb(ICW4_FULLY_NESTED|ICW4_MASTER|ICW4_BUFFERED|ICW4_8086, ICW4_ADDRESS);        /* Set the mode to buffered with edge triggering */
    outb(OCW3|OCW3_READ_ISR, OCW3_ADDRESS);                 /* Read ISR */

    /* Setup the Slave 8259 */
    outb(ICW1|ICW1_NEED_ICW4, ICW1_ADDRESS + SLAVE);        /* Begin the init sequence */
    outb(8, ICW2_ADDRESS + SLAVE);                          /* Slave base address is 8, interrupts 8-15 */
    outb(2, ICW3_ADDRESS + SLAVE);                          /* Slave is connected to line 2 */
    outb(ICW4_BUFFERED|ICW4_8086, ICW4_ADDRESS+SLAVE);      /* Set the mode to buffered with edge triggering */
    outb(OCW3|OCW3_READ_ISR, OCW3_ADDRESS+SLAVE);           /* Read ISR */

    /* Set interrupt mask to disable all interrupts */
    outb(0xfb, OCW1_ADDRESS);
    outb(0xff, OCW1_ADDRESS + SLAVE);

    /* Setup the GPIO pin if the interrupt is hooked to it */
    if ((irq_line >= 24) && (irq_line <= 39))
    {
        printk("8259: Setting GPIO %d for the interrupt\n", irq_line - 24);
        octeon_write_csr(OCTEON_GPIO_BIT_CFGX(irq_line - 24), 0x114);
        request_irq(irq_line, octeon_i8259_interrupt, SA_SHIRQ, "8259", octeon_i8259_interrupt);
    }
    else if ((irq_line >= 44) && (irq_line <= 47))
    {
        printk("8259: Using PCI INT-%c\n", irq_line - 44 + 'A');
        request_irq(irq_line, octeon_i8259_interrupt, SA_SHIRQ, "8259", octeon_i8259_interrupt);
    }
    else
    {
        panic("8259: Don't know how to setup the interrupt IRQ %d\n", irq_line);
    }
}


