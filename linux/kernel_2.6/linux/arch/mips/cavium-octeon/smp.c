/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2004-2005 Cavium Networks
 */
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/smp.h>
#include <linux/kernel_stat.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <asm/mmu_context.h>
#include <asm/time.h>
#include <asm/system.h>

#include "hal.h"

extern void octeon_user_io_init(void);

volatile unsigned long octeon_processor_boot = 0xff;
volatile unsigned long octeon_processor_cycle;
volatile unsigned long octeon_processor_sp;
volatile unsigned long octeon_processor_gp;

/**
 * Mailbox interrupt handler
 *
 * @param irq
 * @param regs
 */
void mailbox_interrupt(int irq, struct pt_regs *regs)
{
    const uint64_t  coreid = octeon_get_core_num();
	uint64_t        action;

    /* Count the IRQs manually since we skip the normal process */
	kstat_this_cpu.irqs[irq]++;

	/* Load the mailbox register to figure out what we're supposed to do */
	action = octeon_read_csr(OCTEON_CIU_MBOX_CLRX(coreid));

    if (action)
    {
        //octeon_led_set(1, 31-coreid);

        /* Clear the mailbox to clear the interrupt */
        octeon_write_csr(OCTEON_CIU_MBOX_CLRX(coreid), action);

        //printk("SMP: Mailbox interrupt cpu=%u, coreid=%lu action=%lu\n", cpu_number_map(coreid), coreid, action);

        if (action & SMP_CALL_FUNCTION) {
            smp_call_function_interrupt();
        }

        /* Check if we've been told to flush the icache */
        if (action & SMP_ICACHE_FLUSH) {
            asm volatile ("synci 0($0)\n");
        }
        //octeon_led_clear(1, 31-coreid);
    }
}


/**
 * Cause the function described by call_data to be executed on the passed
 * cpu.  When the function has finished, increment the finished field of
 * call_data.
 *
 * @param cpu
 * @param action
 */
void core_send_ipi(int cpu, unsigned int action)
{
    int coreid = cpu_logical_map(cpu);
    //printk("SMP: Mailbox send cpu=%d, coreid=%d, action=%u\n", cpu, coreid, action);
    octeon_write_csr(OCTEON_CIU_MBOX_SETX(coreid), action);
}


/**
 * Detect available CPUs, populate phys_cpu_present_map
 */
void plat_smp_setup(void)
{
    const uint64_t coreid = octeon_get_core_num();
    int cpus;
    int id;

    uint64_t core_mask = octeon_get_boot_coremask();

    cpus_clear(phys_cpu_present_map);
    __cpu_number_map[coreid] = 0;
    __cpu_logical_map[0] = coreid;
    cpu_set(0, phys_cpu_present_map);

    cpus = 1;
    for (id=0; id<16; id++)
    {
        if ((id != coreid) && (core_mask & (1<<id)))
        {
            cpu_set(cpus, phys_cpu_present_map);
            __cpu_number_map[id] = cpus;
            __cpu_logical_map[cpus] = id;
            cpus++;
        }
    }
}


/**
 * Firmware CPU startup hook
 *
 * @param cpu
 * @param idle
 */
void prom_boot_secondary(int cpu, struct task_struct *idle)
{
    uint64_t count;

    printk("SMP: Booting CPU%02d (CoreId %2d)...", cpu, cpu_logical_map(cpu));

    octeon_processor_sp = __KSTK_TOS(idle);
    octeon_processor_gp = (unsigned long)idle->thread_info;
    __sync(); /* Use sync so all ops are done. This makes the cycle counter propagate in a more bounded amount of time */
    octeon_processor_cycle = get_cycles();
    octeon_processor_boot = cpu_logical_map(cpu);
    mb();

    count = 10000;
    while (octeon_processor_sp && count)
    {
        /* Waiting for processor to get the SP and GP */
        udelay(1);
        count--;
    }
    if (count == 0)
        printk("Timeout\n");
}


/**
 * After we've done initial boot, this function is called to allow the
 * board code to clean up state, if needed
 */
void prom_init_secondary(void)
{
    const uint64_t      coreid = octeon_get_core_num();
    octeon_ciu_intx0_t    interrupt_enable;

    octeon_check_cpu_bist();

    //printk("SMP: CPU%d (CoreId %lu) started\n", cpu, coreid);

    /* Enable Mailbox interrupts to this core. These are the only interrupts
        allowed on line 3 */
    octeon_write_csr(OCTEON_CIU_MBOX_CLRX(coreid), 0xffffffff);
    interrupt_enable.u64 = 0;
    interrupt_enable.s.mbox = 0x3;
    octeon_write_csr(OCTEON_CIU_INTX_EN0((coreid * 2)), 0);
    octeon_write_csr(OCTEON_CIU_INTX_EN0((coreid * 2 + 1)), interrupt_enable.u64);
    set_c0_status(0xff01); /* Enable core interrupt processing */
}


/**
 * Callout to firmware before smp_init
 *
 * @param max_cpus
 */
void plat_prepare_cpus(unsigned int max_cpus)
{
    const uint64_t      coreid = octeon_get_core_num();
    octeon_ciu_intx0_t    interrupt_enable;

    /* This irq register is just a placeholder. For speed, the low level
        interrupt handler calls mailbox_interrupt directly. This just
        lets the normal interrupt handling stuff know */
    request_irq(3, (irqreturn_t (*)(int, void *, struct pt_regs *))mailbox_interrupt, SA_SHIRQ, "IPC", mailbox_interrupt);

    /* Enable Mailbox interrupts to this core. These are the only interrupts
        allowed on line 3 */
    octeon_write_csr(OCTEON_CIU_MBOX_CLRX(coreid), 0xffffffff);
    interrupt_enable.u64 = 0;
    interrupt_enable.s.mbox = 0x3;
    octeon_write_csr(OCTEON_CIU_INTX_EN0((coreid * 2 + 1)), interrupt_enable.u64);
}


/**
 * Last chance for the board code to finish SMP initialization before
 * the CPU is "online".
 */
void prom_smp_finish(void)
{
    #ifdef CONFIG_CAVIUM_GDB
        register uint64_t tmp;
        /* Pulse MCD0 signal on Ctrl-C to stop all the cores. Also
            set the MCD0 to be not masked by this core so we know
            the signal is received by someone */
        asm volatile (
            "dmfc0 %0, $22\n"
            "ori   %0, %0, 0x9100\n"
            "dmtc0 %0, $22\n"
            : "=r" (tmp));
    #endif

    #ifdef CONFIG_CAVIUM_OCTEON_USER_MEM
        octeon_user_io_init();
    #endif

    /* to generate the first CPU timer interrupt */
	write_c0_compare(read_c0_count() + mips_hpt_frequency / HZ);
}


/**
 * Hook for after all CPUs are online
 */
void prom_cpus_done(void)
{
    #ifdef CONFIG_CAVIUM_GDB
        register uint64_t tmp;
        /* Pulse MCD0 signal on Ctrl-C to stop all the cores. Also
            set the MCD0 to be not masked by this core so we know
            the signal is received by someone */
        asm volatile (
            "dmfc0 %0, $22\n"
            "ori   %0, %0, 0x9100\n"
            "dmtc0 %0, $22\n"
            : "=r" (tmp));
    #endif
}

EXPORT_SYMBOL(__cpu_logical_map);

