/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2004-2005 Cavium Networks
 */
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/pci.h>

#include <asm/irq_cpu.h>
#include <asm/mipsregs.h>
#include <asm/system.h>

#include "../cavium-octeon/hal.h"

static void octeon_unmask_irq(unsigned int irq)
{
    unsigned long flags;
    local_irq_save(flags);
    if (irq < 8)
    {
        /* Core local interrupts, irq 0-7 */
        clear_c0_cause(0x100 << irq);
        set_c0_status(0x100 << irq);
    }
    else if (irq<72)
    {
        /* Interrupts from the CIU, irq 8-71 */
        const uint64_t coreid = octeon_get_core_num();
        uint64_t bit = (irq - 8) & 0x3f;    /* Bit 0-63 of EN0 */
        uint64_t en0 = octeon_read_csr(OCTEON_CIU_INTX_EN0(coreid*2));
        en0 |= 1ull<<bit;
        octeon_write_csr(OCTEON_CIU_INTX_EN0(coreid*2), en0);
    }
    else if (irq<88)
    {
        /* Interrupts from the master 8259, irq 80-87 */
        outb(inb(0x21) & ~(1<<(irq-80)), 0x21);
    }
    else if (irq<96)
    {
        /* Interrupts from the slave 8259, irq 88-95 */
        outb(inb(0xa1) & ~(1<<(irq-88)), 0xa1);
    }
    local_irq_restore(flags);
}

static void octeon_mask_irq(unsigned int irq)
{
    unsigned long flags;
    local_irq_save(flags);
    if (irq < 8)
    {
        /* Core local interrupts, irq 0-7 */
        clear_c0_status(0x100 << irq);
    }
    else if (irq<72)
    {
        /* Interrupts from the CIU, irq 8-71 */
        const uint64_t coreid = octeon_get_core_num();
        uint64_t bit = (irq - 8) & 0x3f;    /* Bit 0-63 of EN0 */
        uint64_t en0 = octeon_read_csr(OCTEON_CIU_INTX_EN0(coreid*2));
        en0 &= ~(1ull<<bit);
        octeon_write_csr(OCTEON_CIU_INTX_EN0(coreid*2), en0);
        octeon_read_csr(OCTEON_CIU_INTX_EN0(coreid*2));
    }
    else if (irq<88)
    {
        /* Interrupts from the master 8259, irq 80-87 */
        outb(inb(0x21) | (1<<(irq-80)), 0x21);
    }
    else if (irq<96)
    {
        /* Interrupts from the slave 8259, irq 88-95 */
        outb(inb(0xa1) | (1<<(irq-88)), 0xa1);
    }
    local_irq_restore(flags);
}

static void octeon_mask_irq_all(unsigned int irq)
{
    unsigned long flags;
    local_irq_save(flags);
    if (irq < 8)
    {
        /* Core local interrupts, irq 0-7 */
        clear_c0_status(0x100 << irq);
    }
    else if (irq<72)
    {
        /* Interrupts from the CIU, irq 8-71 */
        uint64_t bit = (irq - 8) & 0x3f;    /* Bit 0-63 of EN0 */

#ifdef CONFIG_SMP
        int cpu;
        for (cpu=0; cpu<NR_CPUS; cpu++)
        {
            if (cpu_present(cpu))
            {
                uint64_t coreid = cpu_logical_map(cpu);
                uint64_t en0 = octeon_read_csr(OCTEON_CIU_INTX_EN0(coreid*2));
                en0 &= ~(1ull<<bit);
                octeon_write_csr(OCTEON_CIU_INTX_EN0(coreid*2), en0);
            }
        }
        /* We need to do a read after the last update to make sure all of
            them are done */
        octeon_read_csr(OCTEON_CIU_INTX_EN0(octeon_get_core_num()*2));
#else
        const uint64_t coreid = octeon_get_core_num();
        uint64_t en0 = octeon_read_csr(OCTEON_CIU_INTX_EN0(coreid*2));
        en0 &= ~(1ull<<bit);
        octeon_write_csr(OCTEON_CIU_INTX_EN0(coreid*2), en0);
        octeon_read_csr(OCTEON_CIU_INTX_EN0(coreid*2));
#endif
    }
    else if (irq<88)
    {
        /* Interrupts from the master 8259, irq 80-87 */
        outb(inb(0x21) & ~(1<<(irq-80)), 0x21);
    }
    else if (irq<96)
    {
        /* Interrupts from the slave 8259, irq 88-95 */
        outb(inb(0xa1) & ~(1<<(irq-88)), 0xa1);
    }
    local_irq_restore(flags);
}


static unsigned int octeon_irq_startup(unsigned int irq)
{
    octeon_unmask_irq(irq);
    return 0;
}

static void octeon_irq_set_affinity(unsigned int irq, cpumask_t dest)
{
#ifdef CONFIG_SMP

    /* Interrupts from the CIU, irq 8-71 */
    if ((irq > 8) && (irq < 72))
    {
        int             cpu;
        unsigned long   flags;
        irq_desc_t *    desc = irq_desc + irq;
        uint64_t        bit = (irq - 8) & 0x3f;    /* Bit 0-63 of EN0 */
        spin_lock_irqsave(&desc->lock, flags);
        for (cpu=0; cpu<NR_CPUS; cpu++)
        {
            if (cpu_present(cpu))
            {
                uint64_t coreid = cpu_logical_map(cpu);
                uint64_t en0 = octeon_read_csr(OCTEON_CIU_INTX_EN0(coreid*2));
                if (cpu_isset(cpu, dest))
                    en0 |= 1ull<<bit;
                else
                    en0 &= ~(1ull<<bit);
                octeon_write_csr(OCTEON_CIU_INTX_EN0(coreid*2), en0);
            }
        }
        /* We need to do a read after the last update to make sure all of
            them are done */
        octeon_read_csr(OCTEON_CIU_INTX_EN0(octeon_get_core_num()*2));
        spin_unlock_irqrestore(&desc->lock, flags);
    }
#endif
}

static hw_irq_controller octeon_irq_controller = {
    "Octeon",
    octeon_irq_startup,     /* startup */
    octeon_mask_irq_all,    /* shutdown */
    octeon_unmask_irq,      /* enable */
    octeon_mask_irq_all,    /* disable */
    octeon_mask_irq,        /* ack */
    octeon_unmask_irq,      /* end */
    octeon_irq_set_affinity
};

void __init octeon_irq_init(void)
{
    int i;

    /* Interrupt 0-7 map to the Mips internal interrupts. Cascaded off of
        interrupt 2, the CIU uses irq numbers 8-71. 72-79 are not used, they
        exist only to align the i8259 interrupts on a multiple of 16. This is
        so the Via southbridges don't get confused when they and off the lower
        bits for internal use. 80-87 is the master 8259, 88-95 is the slave. */

    if (NR_IRQS < 8 + 64 + 8 + 16)
        printk("octeon_irq_init: NR_IRQS is set too low\n");

    for (i = 0; i < NR_IRQS; i++)
    {
        irq_desc[i].status = IRQ_DISABLED | IRQ_PER_CPU;
        irq_desc[i].action = NULL;
        irq_desc[i].depth = 1;
        irq_desc[i].handler = &octeon_irq_controller;
    }
    set_c0_status(0x100 << 2);
}
