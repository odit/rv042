/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2004, 2005 Cavium Networks
 */
#include <linux/config.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/serial.h>
#include <linux/types.h>
#include <linux/string.h>	/* for memset */
#include <linux/console.h>
#include <linux/serial.h>
#include <linux/tty.h>
#include <asm/time.h>
#include <linux/serial_core.h>
#include <linux/string.h>
#include <linux/mtd/physmap.h>

#include <asm/reboot.h>
#include <asm/io.h>
#include <asm/time.h>
#include <asm/processor.h>
#include <asm/reboot.h>
#include <asm/system.h>
#include <asm/irq_cpu.h>
#include <asm/mipsregs.h>
#include <asm/bootinfo.h>
#include <asm/gdb-stub.h>
#include "hal.h"
#include "cvmx-app-init.h"

#ifdef CONFIG_CAVIUM_OCTEON_SIMULATOR
static uint64_t MAX_MEMORY = 64ull << 20;
#else
static uint64_t MAX_MEMORY = 512ull << 20;
#endif

/* The following define is used to limit the maximum physical address Linux
    will allocate. It leaves out the 2nd 256MB region at 0x410000000. This
    region causes the Linux page tables to grow by 300MB in order to map all
    the addresses below it. Once we have NUMA disjoint memory support, this
    mask should be changed */
#define MAX_MEMORY_ADDRESS  0x3ffffffffull

extern void octeon_user_io_init(void);
extern asmlinkage void octeon_handle_irq(void);
static uint64_t CYCLES_PER_JIFFY;
static int ECC_REPORT_SINGLE_BIT_ERRORS = 0;

/**
 * Return the version string. Generated from CVS tags. Turns
 * NAME_V_V_V_build_B into "V.V.V, build B". If the tag isn't of
 * this format then the tag is returned. If there isn't a tag
 * then the build date is returned as "Internal __DATE__".
 *
 * @return Version string
 */
static inline const char *get_version(void)
{
    static char version[80];
    const char *cvs_tag = "$Name$";

    if (cvs_tag[7] == ' ')
    {
        snprintf(version, sizeof(version), "Internal %s", __DATE__);
    }
    else
    {
        char *major = NULL;
        char *minor1 = NULL;
        char *minor2 = NULL;
        char *build = NULL;
        char *buildnum = NULL;
        char *end = NULL;
        char buf[80];

        strncpy(buf, cvs_tag, sizeof(buf));
        buf[sizeof(buf)-1] = 0;

        major = strchr(buf, '_');
        if (major)
        {
            major++;
            minor1 = strchr(major, '_');
            if (minor1)
            {
                *minor1 = 0;
                minor1++;
                minor2 = strchr(minor1, '_');
                if (minor2)
                {
                    *minor2 = 0;
                    minor2++;
                    build = strchr(minor2, '_');
                    if (build)
                    {
                        *build = 0;
                        build++;
                        buildnum = strchr(build, '_');
                        if (buildnum)
                        {
                            *buildnum = 0;
                            buildnum++;
                            end = strchr(buildnum, ' ');
                            if (end)
                                *end = 0;
                        }
                    }
                }
            }
        }

        if (major && minor1 && minor2 && build && buildnum && (strcmp(build, "build") == 0))
            snprintf(version, sizeof(version), "%s.%s.%s, build %s", major, minor1, minor2, buildnum);
        else
            snprintf(version, sizeof(version), "%s", cvs_tag);
    }

    return version;
}


/**
 * Reboot Octeon
 *
 * @param command Command to pass to the bootloader. Currently ignored.
 */
static void octeon_restart(char *command)
{
    mb();
#if 0   // Disabled BIST on reset due to a chip errata G-200
    /* Note:  BIST should always be enabled when doing a soft reset.
    ** L2 Cache locking for instance is not cleared unless BIST is enabled */
    octeon_write_csr(OCTEON_CIU_SOFT_BIST, 1);
    /* Read it back to make sure it is set before we reset the chip. */
    octeon_read_csr(OCTEON_CIU_SOFT_BIST);
#endif

    while (1)
        octeon_write_csr(OCTEON_CIU_SOFT_RST, 1);
}


/**
 * Permanently stop a core.
 *
 * @param arg
 */
static void octeon_kill_core(void *arg)
{
    mb();
    #ifdef CONFIG_CAVIUM_OCTEON_SIMULATOR
        /* A break instruction causes the simulator stop a core */
        asm volatile ("break");
    #endif
}


/**
 * Halt the system
 */
static void octeon_halt(void)
{
    smp_call_function (octeon_kill_core, NULL, 0, 0);
    octeon_write_lcd("PowerOff");
    octeon_kill_core(NULL);
}


/**
 * This variable is used to correct for a offset on the clock
 * when Linux boots.
 */
static int64_t octeon_hpt_correction = 0;


/**
 * Read the Octeon high performance counter
 *
 * @return The counter value. For some brain dead reason, the kernel
 *         uses a 32bit number here.
 */
static unsigned int octeon_hpt_read(void)
{
    int64_t cycles;
    asm ("rdhwr %0,$31" : "=r" (cycles));
    return cycles - octeon_hpt_correction;
}


/**
 * Initialize the high performance counter.
 *
 * @param count  Offset to apply to the counter
 */
static void octeon_hpt_init(unsigned int count)
{
    octeon_hpt_correction += count;
}


/**
 * Acknowledge a timer tick. We don't use the standard Mips
 * one because it confuses the timer ticks and the HPT clock.
 */
static void octeon_timer_ack(void)
{
    uint32_t count;
    uint32_t next_compare = read_c0_compare() + CYCLES_PER_JIFFY;
    write_c0_compare(next_compare);
    count = read_c0_count();
    if ((count - next_compare) < 0x7fffffff)
    {
        next_compare = count + CYCLES_PER_JIFFY;
        write_c0_compare(next_compare);
    }
}


/**
 * Interrupt entry point for timer ticks
 *
 * @param irq
 * @param dev_id
 * @param regs
 * @return
 */
static irqreturn_t octeon_main_timer_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{
    if (read_c0_count() - read_c0_compare() >= 0)
    {
        if (smp_processor_id() == 0)
            timer_interrupt(irq, NULL, regs);
        else
        {
            octeon_timer_ack();
            local_timer_interrupt(irq, NULL, regs);
        }
    }
    return IRQ_HANDLED;
}


/**
 * Setup the first cores timer interrupt
 *
 * @param irq
 * @return
 */
static void __init octeon_timer_setup(struct irqaction *irq)
{
    irq->handler = octeon_main_timer_interrupt;
    irq->flags |= SA_SHIRQ;
    setup_irq(7, irq);

    write_c0_compare(read_c0_count() + CYCLES_PER_JIFFY);
}


/**
 * Handle all the error condition interrupts that might occur.
 *
 * @param cpl
 * @param dev_id
 * @param regs
 * @return
 */
static irqreturn_t octeon_rlm_interrupt(int cpl, void *dev_id, struct pt_regs *regs)
{
    irqreturn_t result = IRQ_NONE;
    uint64_t err;
    uint64_t rsl_blocks;

    rsl_blocks = octeon_read_csr(OCTEON_NPI_RSL_INT_BLOCKS);
    //printk("OCTEON_NPI_RSL_INT_BLOCKS=0x%lx\n", rsl_blocks);

    if (rsl_blocks & (1<<30)) /* IOB_INT_SUM */
    {
        /* Check for IOB SOP and EOP errors */
        err = octeon_read_csr(OCTEON_IOB_INT_SUM);
        if (err)
        {
            uint64_t port = octeon_read_csr(OCTEON_IOB_PKT_ERR);
            octeon_write_csr(OCTEON_IOB_INT_SUM, err);
            if (err & 0x1) printk("\nIOB: Port %lu SOP error\n", port);
            if (err & 0x2) printk("\nIOB: Port %lu EOP error\n", port);
            if (err & 0x4) printk("\nIOB: Passthrough Port %lu SOP error\n", port);
            if (err & 0x8) printk("\nIOB: Passthrough Port %lu EOP error\n", port);
            result = IRQ_HANDLED;
        }
    }

    if (rsl_blocks & (1<<23)) /* ASX1_INT_REG */
    {
        err = octeon_read_csr(OCTEON_ASXX_INT_REG(1));
        if (err)
        {
            int port;
            octeon_write_csr(OCTEON_ASXX_INT_REG(1), err);
            for (port=0; port<4; port++)
            {
                if (err&(0x001<<port)) printk("ASX1: RX FIFO overflow on RMGII port %d\n", port+16);
                if (err&(0x010<<port)) printk("ASX1: TX FIFO underflow on RMGII port %d\n", port+16);
                if (err&(0x100<<port)) printk("ASX1: TX FIFO overflow on RMGII port %d\n", port+16);
            }
            result = IRQ_HANDLED;
        }
    }

    if (rsl_blocks & (1<<22)) /* ASX0_INT_REG */
    {
        err = octeon_read_csr(OCTEON_ASXX_INT_REG(0));
        if (err)
        {
            int port;
            octeon_write_csr(OCTEON_ASXX_INT_REG(0), err);
            for (port=0; port<4; port++)
            {
                if (err&(0x001<<port)) printk("ASX0: RX FIFO overflow on RMGII port %d\n", port);
                if (err&(0x010<<port)) printk("ASX0: TX FIFO underflow on RMGII port %d\n", port);
                if (err&(0x100<<port)) printk("ASX0: TX FIFO overflow on RMGII port %d\n", port);
            }
            result = IRQ_HANDLED;
        }
    }

    if (rsl_blocks & (1<<20)) /* PIP_INT_REG. */
    {
        err = octeon_read_csr(OCTEON_PIP_INT_REG);
        if (err)
        {
            octeon_write_csr(OCTEON_PIP_INT_REG, err);
            err &= octeon_read_csr(OCTEON_PIP_INT_EN);
            if (err&0x100) printk("PIP: Parity Error in back end memory\n");
            if (err&0x080) printk("PIP: Parity Error in front end memory\n");
            if (err&0x040) printk("PIP: Todo list overflow (see PIP_BCK_PRS[HIWATER])\n");
            if (err&0x020) printk("PIP: Packet was engulfed by skipper\n");
            if (err&0x010) printk("PIP: A bad tag was sent from IPD\n");
            if (err&0x008) printk("PIP: Non-existent port\n");
            if (err&0x004) printk("PIP: PIP asserted backpressure\n");
            if (err&0x002) printk("PIP: PIP calculated bad CRC\n");
            if (err&0x001) printk("PIP: Packet Dropped due to QOS\n");
            result = IRQ_HANDLED;
        }
    }

    if (rsl_blocks & (1<<19)) /* SPX1_INT_REG & STX1_INT_REG */
    {
        /* Currently not checked. These should be checked by the ethernet driver */
    }

    if (rsl_blocks & (1<<18)) /* SPX0_INT_REG & STX0_INT_REG */
    {
        /* Currently not checked. These should be checked by the ethernet driver */
    }

    if (rsl_blocks & (1<<17)) /* LMC_MEM_CFG0 */
    {
        static uint64_t single_bit_errors = 0;
        static uint64_t double_bit_errors = 0;

        /* Get the ECC status from LMC config zero */
        uint64_t mem_cfg0 = octeon_read_csr(OCTEON_LMC_MEM_CFG0);
        /* Write out the same value to clear the ECC error bits */
        octeon_write_csr(OCTEON_LMC_MEM_CFG0, mem_cfg0);

        mem_cfg0 = (mem_cfg0 >> 21) & 0xff;
        if (mem_cfg0 & 0x1) single_bit_errors++;
        if (mem_cfg0 & 0x2) single_bit_errors++;
        if (mem_cfg0 & 0x4) single_bit_errors++;
        if (mem_cfg0 & 0x8) single_bit_errors++;
        if (mem_cfg0 & 0x10) double_bit_errors++;
        if (mem_cfg0 & 0x20) double_bit_errors++;
        if (mem_cfg0 & 0x40) double_bit_errors++;
        if (mem_cfg0 & 0x80) double_bit_errors++;
        if (mem_cfg0)
            result = IRQ_HANDLED;

        if ((mem_cfg0 & 0xf0) || (mem_cfg0 && ECC_REPORT_SINGLE_BIT_ERRORS))
        {
            octeon_lmc_fadr_t fadr;
            fadr.u64 = octeon_read_csr(OCTEON_LMC_FADR);
            printk("\nECC: %lu Single bit errors, %lu Double bit errors\n"
                   "ECC:\tFailing dimm:   %u\n"
                   "ECC:\tFailing rank:   %u\n"
                   "ECC:\tFailing bank:   %u\n"
                   "ECC:\tFailing row:    0x%x\n"
                   "ECC:\tFailing column: 0x%x\n",
                   single_bit_errors, double_bit_errors, fadr.s.fdimm,
                   fadr.s.fbunk, fadr.s.fbank, fadr.s.frow, fadr.s.fcol);
        }
    }

    if (rsl_blocks & (1<<16)) /* L2T_ERR & L2D_ERR */
    {
        static uint64_t l2t_single_bit_errors = 0;
        static uint64_t l2t_double_bit_errors = 0;
        static uint64_t l2d_single_bit_errors = 0;
        static uint64_t l2d_double_bit_errors = 0;

        /* Get the ECC status from L2T_ERR */
        err = octeon_read_csr(OCTEON_L2T_ERR);
        /* Write out the same value to clear the ECC error bits */
        octeon_write_csr(OCTEON_L2T_ERR, err);

        err = (err >> 3) & 0x3;
        if (err & 0x1) l2t_single_bit_errors++;
        if (err & 0x2) l2t_double_bit_errors++;
        if (err)
            result = IRQ_HANDLED;

        if ((err & 0x2) || (err && ECC_REPORT_SINGLE_BIT_ERRORS))
        {
            printk("\nECC: L2T %lu Single bit errors, %lu Double bit errors\n",
                   l2t_single_bit_errors, l2t_double_bit_errors);
        }

        /* Get the ECC status from L2D_ERR */
        err = octeon_read_csr(OCTEON_L2D_ERR);
        /* Write out the same value to clear the ECC error bits */
        octeon_write_csr(OCTEON_L2D_ERR, err);

        err = (err >> 3) & 0x3;
        if (err & 0x1) l2d_single_bit_errors++;
        if (err & 0x2) l2d_double_bit_errors++;
        if (err)
            result = IRQ_HANDLED;

        if ((err & 0x2) || (err && ECC_REPORT_SINGLE_BIT_ERRORS))
        {
            printk("\nECC: L2D %lu Single bit errors, %lu Double bit errors\n",
                   l2d_single_bit_errors, l2d_double_bit_errors);
        }
    }

    if (rsl_blocks & (1<<12)) /* POW_ECC_ERR */
    {
        static uint64_t pow_single_bit_errors = 0;
        static uint64_t pow_double_bit_errors = 0;

        /* Get the ECC status from POW */
        err = octeon_read_csr(OCTEON_POW_ECC_ERR);
        /* Write out the same value to clear the ECC error bits */
        octeon_write_csr(OCTEON_POW_ECC_ERR, err);

        err = err & 0x3;
        if (err & 0x1) pow_single_bit_errors++;
        if (err & 0x2) pow_double_bit_errors++;
        if (err)
            result = IRQ_HANDLED;

        if ((err & 0x2) || (err && ECC_REPORT_SINGLE_BIT_ERRORS))
        {
            printk("\nECC: POW %lu Single bit errors, %lu Double bit errors\n",
                   pow_single_bit_errors, pow_double_bit_errors);
        }
    }

    if (rsl_blocks & (1<<11)) /* TIM_REG_ERROR */
    {
        err = octeon_read_csr(OCTEON_TIM_REG_ERROR);
        if (err)
        {
            int i;
            octeon_write_csr(OCTEON_TIM_REG_ERROR, err);
            for (i=0;i<16; i++)
                if (err & (1<<i))
                    printk("TIM: Timer wheel %d error\n", i);
            result = IRQ_HANDLED;
        }
    }

    if (rsl_blocks & (1<<10)) /* PKO_REG_ERROR */
    {
        err = octeon_read_csr(OCTEON_PKO_REG_ERROR);
        if (err)
        {
            octeon_write_csr(OCTEON_PKO_REG_ERROR, err);
            if (err & 0x2) printk("PKO: A doorbell count has overflowed\n");
            if (err & 0x1) printk("PKO: A read-parity error has occurred in the port data buffer\n");
            result = IRQ_HANDLED;
        }
    }

    if (rsl_blocks & (1<<9)) /* IPD_INT_SUM */
    {
        err = octeon_read_csr(OCTEON_IPD_INT_SUM);
        if (err)
        {
            octeon_write_csr(OCTEON_IPD_INT_SUM, err);
            if (err & 0x1) printk("\nIPD: PBM memory parity error [31:0]\n");
            if (err & 0x2) printk("\nIPD: PBM memory parity error [63:32]\n");
            if (err & 0x4) printk("\nIPD: PBM memory parity error [95:64]\n");
            if (err & 0x8) printk("\nIPD: PBM memory parity error [127:96]\n");
            if (err & 0x10) printk("\nIPD: Backpressure subtract with an illegal value\n");
            result = IRQ_HANDLED;
        }
    }

    if (rsl_blocks & (1<<7)) /* ZIP_ERROR */
    {
        if ((current_cpu_data.cputype != CPU_CAVIUM_CN30XX) && (current_cpu_data.cputype != CPU_CAVIUM_CN50XX))
        {
            err = octeon_read_csr(OCTEON_ZIP_ERROR);
            if (err)
            {
                octeon_write_csr(OCTEON_ZIP_ERROR, err);
                printk("\nZIP: Doorbell overflow\n");
                result = IRQ_HANDLED;
            }
        }
    }

    if (rsl_blocks & (1<<6)) /* DFA_ERR */
    {
        err = octeon_read_csr(OCTEON_DFA_ERR);
        octeon_write_csr(OCTEON_DFA_ERR, err);
        if (err &(1ul<<31)) printk("DFA: Doorbell Overflow detected\n");
        if (err &(1ul<<29)) printk("DFA: PP-CP2 Parity Error Detected\n");
        if (err &(1ul<<26)) printk("DFA: DTE Parity Error Detected\n");
        if (err &(1ul<<15)) printk("DFA: DTE 29b Double Bit Error Detected\n");
        if (ECC_REPORT_SINGLE_BIT_ERRORS && (err & (1ul<<14))) printk("DFA: DTE 29b Single Bit Error Corrected\n");
        if (err &(1ul<<2)) printk("DFA: PP-CP2 Double Bit Error Detected\n");
        if (ECC_REPORT_SINGLE_BIT_ERRORS && (err & (1ul<<1))) printk("DFA: PP-CP2 Single Bit Error Corrected\n");
    }

    if (rsl_blocks & (1<<5)) /* FPA_INT_SUM */
    {
        err = octeon_read_csr(OCTEON_FPA_INT_SUM);
        if (err)
        {
            /* The Queue X stack end tag check is disabled due to Octeon Pass
                2 errata FPA-100. This error condition can be set erroneously. */
            octeon_write_csr(OCTEON_FPA_INT_SUM, err);
            err &= octeon_read_csr(OCTEON_FPA_INT_ENB); /* Don't report disabled errors */
            if (err & 0x8000000) printk("FPA: Queue 7 pointer read from the stack in the L2C does not have the FPA owner ship bit set.\n");
            //if (err & 0x4000000) printk("FPA: Queue 7 stack end tag is present and the count available is greater than than pointers present in the FPA.\n");
            if (err & 0x2000000) printk("FPA: Queue 7 page count available goes negative.\n");
            if (err & 0x1000000) printk("FPA: Queue 6 pointer read from the stack in the L2C does not have the FPA owner ship bit set.\n");
            //if (err & 0x800000) printk("FPA: Queue 6 stack end tag is present and the count available is greater than than pointers present in the FPA.\n");
            if (err & 0x400000) printk("FPA: Queue 6 page count available goes negative.\n");
            if (err & 0x200000) printk("FPA: Queue 5 pointer read from the stack in the L2C does not have the FPA owner ship bit set.\n");
            //if (err & 0x100000) printk("FPA: Queue 5 stack end tag is present and the count available is greater than than pointers present in the FPA.\n");
            if (err & 0x80000) printk("FPA: Queue 5 page count available goes negative.\n");
            if (err & 0x40000) printk("FPA: Queue 4 pointer read from the stack in the L2C does not have the FPA owner ship bit set.\n");
            //if (err & 0x20000) printk("FPA: Queue 4 stack end tag is present and the count available is greater than than pointers present in the FPA.\n");
            if (err & 0x10000) printk("FPA: Queue 4 page count available goes negative.\n");
            if (err & 0x8000) printk("FPA: Queue 3 pointer read from the stack in the L2C does not have the FPA owner ship bit set.\n");
            //if (err & 0x4000) printk("FPA: Queue 3 stack end tag is present and the count available is greater than than pointers present in the FPA.\n");
            if (err & 0x2000) printk("FPA: Queue 3 page count available goes negative.\n");
            if (err & 0x1000) printk("FPA: Queue 2 pointer read from the stack in the L2C does not have the FPA owner ship bit set.\n");
            //if (err & 0x800) printk("FPA: Queue 2 stack end tag is present and the count available is greater than than pointers present in the FPA.\n");
            if (err & 0x400) printk("FPA: Queue 2 page count available goes negative.\n");
            if (err & 0x200) printk("FPA: Queue 1 pointer read from the stack in the L2C does not have the FPA owner ship bit set.\n");
            //if (err & 0x100) printk("FPA: Queue 1 stack end tag is present and the count available is greater than than pointers present in the FPA.\n");
            if (err & 0x80) printk("FPA: Queue 1 page count available goes negative.\n");
            if (err & 0x40) printk("FPA: Queue 0 pointer read from the stack in the L2C does not have the FPA owner ship bit set.\n");
            //if (err & 0x20) printk("FPA: Queue 0 stack end tag is present and the count available is greater than than pointers present in the FPA.\n");
            if (err & 0x10) printk("FPA: Queue 0 page count available goes negative.\n");
            if (err & 0x8) printk("FPA: Double Bit Error detected in FPF1.\n");
            if (ECC_REPORT_SINGLE_BIT_ERRORS && (err & 0x4)) printk("FPA: Single Bit Error detected in FPF1.\n");
            if (err & 0x2) printk("FPA: Double Bit Error detected in FPF0.\n");
            if (ECC_REPORT_SINGLE_BIT_ERRORS && (err & 0x1)) printk("FPA: Single Bit Error detected in FPF0.\n");
            result = IRQ_HANDLED;
        }
    }

    if (rsl_blocks & (1<<4)) /* KEY_INT_SUM */
    {
        err = octeon_read_csr(OCTEON_KEY_INT_SUM);
        if (err)
        {
            octeon_write_csr(OCTEON_KEY_INT_SUM, err);
            err &= octeon_read_csr(OCTEON_KEY_INT_ENB); /* Don't report disabled errors */
            if (err & 0x8) printk("KEY: KED1 double-bit error.\n");
            if (ECC_REPORT_SINGLE_BIT_ERRORS && (err & 0x4)) printk("KEY: KED1 single-bit error.\n");
            if (err & 0x2) printk("KEY: KED0 double-bit error.\n");
            if (ECC_REPORT_SINGLE_BIT_ERRORS && (err & 0x1)) printk("KEY: KED0 single-bit error.\n");
            result = IRQ_HANDLED;
        }
    }

    if (rsl_blocks & (1<<3)) /* NPI_INT_SUM */
    {
        err = octeon_read_csr(OCTEON_NPI_PCI_INT_SUM2);
        if (err)
        {
            octeon_write_csr(OCTEON_NPI_PCI_INT_SUM2, err);
            err &= octeon_read_csr(OCTEON_NPI_PCI_INT_ENB2); /* Don't report disabled errors */

            if (err & (1ull<<33)) printk("PCI ERROR[ILL_RD]: A read to a disabled area of BAR1 or BAR2, when the mem area is disabled.\n");
            if (err & (1ull<<32)) printk("PCI ERROR[ILL_WR]: A write to a disabled area of BAR1 or BAR2, when the mem area is disabled.\n");
            if (err & (1ull<<31)) printk("PCI ERROR[WIN_WR]: A write to the disabled window write-data register took place.\n");
            if (err & (1ull<<30)) printk("PCI ERROR[DMA1_FI]: A DMA operation operation finished that was required to set the FORCE-INT bit for counter 1.\n");
            if (err & (1ull<<29)) printk("PCI ERROR[DMA0_FI]: A DMA operation operation finished that was required to set the FORCE-INT bit for counter 0.\n");
            if (err & (1ull<<28)) printk("PCI ERROR[DTIME1]: When the value in the PCI_DMA_CNT1 register is not 0 the DMA_CNT1\n"
                                         "                  timer counts. When the DMA1_CNT timer has a value greater than the\n"
                                         "                  PCI_DMA_TIME1 register this bit is set. The timer is reset when the bit\n"
                                         "                  is written with a one.\n");
            if (err & (1ull<<27)) printk("PCI ERROR[DTIME0]: When the value in the PCI_DMA_CNT0 register is not 0 the DMA_CNT0\n"
                                         "                  timer counts. When the DMA0_CNT timer has a value greater than the\n"
                                         "                  PCI_DMA_TIME0 register this bit is set. The timer is reset when the bit\n"
                                         "                  is written with a one.\n");
            if (err & (1ull<<26)) printk("PCI ERROR[DCNT1]: This bit indicates that PCI_DMA_CNT1 value is greater than the value in\n"
                                         "                  the PCI_DMA_INT_LEV1 register.\n");
            if (err & (1ull<<25)) printk("PCI ERROR[DCNT0]: This bit indicates that PCI_DMA_CNT0 value is greater than the value in\n"
                                         "                  the PCI_DMA_INT_LEV0 register.\n");
            if (err & (1ull<<24)) printk("PCI ERROR[PTIME3]: When the value in the PCI_PKTS_SENT3 register is not 0 the Sent3\n"
                                         "                  timer counts. When the Sent3 timer has a value greater than the\n"
                                         "                  PCI_PKTS_SENT_TIME3 register this bit is set. The timer is reset when\n"
                                         "                  the bit is written with a 1.\n");
            if (err & (1ull<<23)) printk("PCI ERROR[PTIME2]: When the value in the PCI_PKTS_SENT(2) register is not 0 the Sent2 timer\n"
                                         "                  counts. When the Sent2 timer has a value greater than the\n"
                                         "                  PCI_PKTS_SENT2 register this bit is set. The timer is reset when the bit\n"
                                         "                  is written with a 1.\n");
            if (err & (1ull<<22)) printk("PCI ERROR[PTIME1]: When the value in the PCI_PKTS_SENT(1) register is not 0 the Sent1 timer\n"
                                         "                  counts. When the Sent1 timer has a value greater than the\n"
                                         "                  PCI_PKTS_SENT1 register this bit is set. The timer is reset when the bit\n"
                                         "                  is written with a 1.\n");
            if (err & (1ull<<21)) printk("PCI ERROR[PTIME0]: When the value in the PCI_PKTS_SENT0 register is not 0 the Sent0 timer\n"
                                         "                  counts. When the Sent0 timer has a value greater than the\n"
                                         "                  PCI_PKTS_SENT0 register this bit is set. The timer is reset when the bit\n"
                                         "                  is written with a 1.\n");
            if (err & (1ull<<20)) printk("PCI ERROR[PCNT3]: This bit indicates that PCI_PKTS_SENT3 value is greater than the value in\n"
                                         "                  the PCI_PKTS_SENT_INT_LEV3 register.\n");
            if (err & (1ull<<19)) printk("PCI ERROR[PCNT2]: This bit indicates that PCI_PKTS_SENT2 value is greater than the value in\n"
                                         "                  the PCI_PKTS_SENT_INT_LEV2 register.\n");
            if (err & (1ull<<18)) printk("PCI ERROR[PCNT1]: This bit indicates that PCI_PKTS_SENT1 value is greater than the value in\n"
                                         "                  the PCI_PKTS_SENT_INT_LEV1 register.\n");
            if (err & (1ull<<17)) printk("PCI ERROR[PCNT0]: This bit indicates that PCI_PKTS_SENT0 value is greater than the value in\n"
                                         "                  the PCI_PKTS_SENT_INT_LEV0 register.\n");
            if (err & (1ull<<16)) printk("PCI ERROR[RSL_INT]: This bit is set when the RSL Chain has generated an interrupt.\n");
            if (err & (1ull<<15)) printk("PCI ERROR[ILL_RRD]: A read to the disabled PCI registers took place.\n");
            if (err & (1ull<<14)) printk("PCI ERROR[ILL_RWR]: A write to the disabled PCI registers took place.\n");
            if (err & (1ull<<13)) printk("PCI ERROR[DPERR]: Data parity error detected by PCX core\n");
            if (err & (1ull<<12)) printk("PCI ERROR[APERR]: Address parity error detected by PCX core\n");
            if (err & (1ull<<11)) printk("PCI ERROR[SERR]: SERR# detected by PCX core\n");
            if (err & (1ull<<10)) printk("PCI ERROR[TSR_ABT]: Target split-read abort detected\n");
            if (err & (1ull<<9)) printk("PCI ERROR[MSC_MSG]: Master split completion message detected\n");
            if (err & (1ull<<8)) printk("PCI ERROR[MSI_MABT]: PCI MSI master abort.\n");
            if (err & (1ull<<7)) printk("PCI ERROR[MSI_TABT]: PCI MSI target abort.\n");
            if (err & (1ull<<6)) printk("PCI ERROR[MSI_PER]: PCI MSI parity error.\n");
            // if (err & (1ull<<5)) printk("PCI ERROR[MR_TTO]: PCI master retry time-out on read.\n");
            if (err & (1ull<<4)) printk("PCI ERROR[MR_ABT]: PCI master abort on read.\n");
            if (err & (1ull<<3)) printk("PCI ERROR[TR_ABT]: PCI target abort on read.\n");
            if (err & (1ull<<2)) printk("PCI ERROR[MR_WTTO]: PCI master retry time-out on write.\n");
            if (err & (1ull<<1)) printk("PCI ERROR[MR_WABT]: PCI master abort detected on write.\n");
            if (err & (1ull<<0)) printk("PCI ERROR[TR_WABT]: PCI target abort detected on write.\n");
            result = IRQ_HANDLED;
        }
    }

    if (rsl_blocks & (1<<2)) /* GMX1_RX*_INT_REG & GMX1_TX_INT_REG */
    {
        /* Currently not checked. These should be checked by the ethernet driver */
    }

    if (rsl_blocks & (1<<1)) /* GMX0_RX*_INT_REG & GMX0_TX_INT_REG */
    {
        /* Currently not checked. These should be checked by the ethernet driver */
    }

    if (rsl_blocks & (1<<0)) /* MIO_BOOT_ERR */
    {
        err = octeon_read_csr(OCTEON_MIO_BOOT_ERR);
        if (err)
        {
            octeon_write_csr(OCTEON_MIO_BOOT_ERR, err);
            err &= octeon_read_csr(OCTEON_MIO_BOOT_INT); /* Don't report disabled errors */
            if (err & 2) printk("BOOT BUS: Wait mode error\n");
            if (err & 1) printk("BOOT BUS: Address decode error\n");
            result = IRQ_HANDLED;
        }
    }

    return result;
}


/**
 * Return a string representing the system type
 *
 * @return
 */
const char *get_system_type(void)
{
    return octeon_board_type_string();
}


/**
 * Initialize the interrupt sub system
 */
void arch_init_irq(void)
{
    extern void octeon_irq_init(void);

	set_except_vector(0, octeon_handle_irq);
    octeon_irq_init();
}


/**
 * Early entry point for arch setup
 */
void prom_init(void)
{
    const uint64_t coreid = octeon_get_core_num();
    int i;
    int argc;
    struct uart_port octeon_port;
    uint64_t mem_alloc_size;
    uint64_t total;
#if defined(CONFIG_NK_IDS_SNORTXL)
    uint64_t dram_size;
    extern cvmx_bootinfo_t *octeon_bootinfo;
#endif
#if defined(CONFIG_MTD_PHYSMAP)
    octeon_mio_boot_reg_cfgx_t region_cfg;
#endif
    int octeon_uart;

    octeon_hal_init();
    octeon_check_cpu_bist();

    /* Disable All CIU Interrupts. The ones we need will be enabled
        later. Read the SUM register so we know the write completed. */
    octeon_write_csr(OCTEON_CIU_INTX_EN0((coreid * 2)), 0);
    octeon_write_csr(OCTEON_CIU_INTX_EN0((coreid * 2 + 1)), 0);
    octeon_read_csr(OCTEON_CIU_INTX_SUM0((coreid * 2)));

    printk("Cavium Networks Version: %s\n", get_version());

#ifdef CONFIG_SMP
    octeon_write_lcd("LinuxSMP");
#else
    octeon_write_lcd("Linux");
#endif

#ifdef CONFIG_GDB_CONSOLE
    strcpy(arcs_cmdline, "console=gdb");
#else


    octeon_uart = octeon_get_boot_uart();
    if (octeon_uart == 1)
        strcpy(arcs_cmdline, "console=ttyS1,115200");
    else
        strcpy(arcs_cmdline, "console=ttyS0,115200");

#endif
#if 1
	printk("kernel cmdline=%s\n",CONFIG_CMDLINE);
        strcat(arcs_cmdline," ");
        strcat(arcs_cmdline,CONFIG_CMDLINE);
        strcat(arcs_cmdline," ");
#endif

#ifdef CONFIG_CAVIUM_OCTEON_SIMULATOR
    /* The simulator uses a mtdram device pre filled with the filesystem. Also
        specify the calibration delay to avoid calculating it every time */
    strcat(arcs_cmdline, " rw root=/dev/mtdblock0 lpj=60176");
#endif

    argc = octeon_get_boot_num_arguments();
    for (i=0; i<argc; i++)
    {
        const char *arg = octeon_get_boot_argument(i);
        if ((strncmp(arg, "MEM=", 4) == 0) || (strncmp(arg, "mem=", 4) == 0))
        {
            sscanf(arg+4, "%li", &MAX_MEMORY);
            MAX_MEMORY <<= 20;
            if (MAX_MEMORY == 0)
                MAX_MEMORY = 32ull<<30;
        }
        else if (strcmp(arg, "ecc_verbose") == 0)
        {
            ECC_REPORT_SINGLE_BIT_ERRORS = 1;
        }
        else if (strlen(arcs_cmdline) + strlen(arg) + 1 < sizeof(arcs_cmdline) - 1)
        {
            strcat(arcs_cmdline, " ");
            strcat(arcs_cmdline, arg);
        }
    }

    /* you should these macros defined in include/asm/bootinfo.h */
    mips_machgroup = MACH_GROUP_CAVIUM;
    mips_machtype = MACH_CAVIUM_OCTEON;

    board_timer_setup = octeon_timer_setup;
    mips_hpt_frequency = octeon_get_clock_rate();
    mips_hpt_read = octeon_hpt_read;
    mips_hpt_init = octeon_hpt_init;
    mips_timer_ack = octeon_timer_ack;
    CYCLES_PER_JIFFY = ((mips_hpt_frequency + HZ / 2) / HZ);

    _machine_restart = octeon_restart;
    _machine_halt = octeon_halt;

    memset(&octeon_port, 0, sizeof(octeon_port));
    octeon_port.flags           = ASYNC_SKIP_TEST | UPF_SHARE_IRQ;
    octeon_port.iotype          = UPIO_MEM;
    octeon_port.regshift        = 3;                  /* I/O addresses are every 8 bytes */
    octeon_port.uartclk         = mips_hpt_frequency; /* Clock rate of the chip */
    octeon_port.fifosize        = 64;
    octeon_port.mapbase = 0x0001180000000800ull + (1024 * octeon_uart);
    octeon_port.membase = octeon_phys_to_ptr(octeon_port.mapbase);
    octeon_port.line = octeon_uart;
    if ((current_cpu_data.cputype != CPU_CAVIUM_CN38XX) || /* Only CN38XXp{1,2} has errata with uart interrupt */
        ((current_cpu_data.processor_id & 0xff) > 1))
        octeon_port.irq = 42+octeon_uart;
    early_serial_setup(&octeon_port);

    /* The Mips memory init uses the first memory location for some memory
        vectors. When SPARSEMEM is in use, it doesn't verify that the size is
        big enough for the final vectors. Making the smallest chuck 4MB seems
        to be enough to consistantly work. This needs to be debugged more */
    mem_alloc_size = 4 << 20;
    total = 0;
    if (mem_alloc_size > MAX_MEMORY)
        mem_alloc_size = MAX_MEMORY;

#if defined(CONFIG_NK_IDS_SNORTXL)
    dram_size=(octeon_bootinfo->dram_size)*1024ull*1024ull;
#endif
    /* When allocating memory, we want incrementing addresses from bootmem_alloc
        so the code in add_memory_region can merge regions next to each other */
    while ((boot_mem_map.nr_map < BOOT_MEM_MAP_MAX) && (total < MAX_MEMORY))
    {
        uint64_t memory = octeon_ptr_to_phys(octeon_bootmem_alloc(mem_alloc_size, 0x10000));
        if (memory)
        {
            /* This function automatically merges address regions next to each
                other if they are received in incrementing order */
            add_memory_region(memory, mem_alloc_size, BOOT_MEM_RAM);
            total += mem_alloc_size;
        }
        else
            break;
#if defined(CONFIG_NK_IDS_SNORTXL)
        if (dram_size==0x40000000ull)
        {
            /* Reserve 448M memory for snortxl */
            if (total>=(dram_size-0x1c000000ull))
                break;
        }
        else if (dram_size==0x80000000ull)
        {
            /* Reserve 512M memory for snortxl */
            if (total>=(dram_size-0x20000000ull))
                break;
        }
#endif
    }

    if (total == 0)
        panic("Unable to allocate memory from octeon_bootmem_alloc\n");

    octeon_hal_setup_reserved32();
    octeon_user_io_init();

#if defined(CONFIG_MTD_PHYSMAP)
    /* Read the bootbus region 0 setup to determine where the base of flash is
        set for */
    region_cfg.u64 = octeon_read_csr(OCTEON_MIO_BOOT_REG_CFGX(0));
    if (region_cfg.s.en)
    {
        /* The bootloader always takes the flash and sets its address so the
            entire flash fits below 0x1fc00000. This way the flash aliases to
            0x1fc00000 for booting. Software can access the full flash at the
            true address, while core boot can access 4MB */
#ifndef CONFIG_TWO_FLASH_BANKS
        uint64_t base = region_cfg.s.base<<16;
#else
//--> TT 20081226, why is 0x1bc00000? because 0x1fc00000 - 64MB = 0x1bc00000
//actually, we need to check register CFG of CS0 & CS1, then get the base address of Flash
        uint64_t base = 0x1bc00000;
#endif
        uint64_t size = 0x1fc00000 - base;
        printk("Setting flash physical map for %ldMB flash at 0x%08lx\n", size>>20, base);
#ifdef CONFIG_MTD_MAP_BANK_WIDTH_2
        physmap_configure(base, size, 2, NULL);
#else
        physmap_configure(base, size, 1, NULL);
#endif
    }
#endif

    set_c0_status(0xff<<8); /* Enable core interrupt processing */

#ifdef CONFIG_KGDB
{
    extern void putDebugChar(char ch);
    const char *s = "\r\nConnect GDB to this port\r\n";
    while (*s)
        putDebugChar(*s++);
}
#endif
}

unsigned long prom_free_prom_memory(void)
{
    uint64_t csr;

    /* Add an interrupt handler for general failures. */
    request_irq(8 + 46, octeon_rlm_interrupt, SA_SHIRQ, "RML Error", octeon_rlm_interrupt);

    /* Enable ECC Interrupts for double bit errors from main memory */
    csr = octeon_read_csr(OCTEON_LMC_MEM_CFG0);
    csr |= 0x3 << 19;
    octeon_write_csr(OCTEON_LMC_MEM_CFG0, csr);

    /* Enable ECC Interrupts for double bit errors from L2C Tags */
    csr = octeon_read_csr(OCTEON_L2T_ERR);
    csr |= 0x7;
    octeon_write_csr(OCTEON_L2T_ERR, csr);

    /* Enable ECC Interrupts for double bit errors from L2D Errors */
    csr = octeon_read_csr(OCTEON_L2D_ERR);
    csr |= 0x7;
    octeon_write_csr(OCTEON_L2D_ERR, csr);

    /* Enable ECC Interrupts for double bit errors from the POW */
    csr = octeon_read_csr(OCTEON_POW_ECC_ERR);
    csr |= 0x3<<2;
    octeon_write_csr(OCTEON_POW_ECC_ERR, csr);

    if (!octeon_is_pass1())
    {
        /* Enable interrupt on IOB port SOP and EOP errors */
        octeon_write_csr(OCTEON_IOB_INT_ENB, 0xf);

        /* Enable interrupts on IPD errors */
        octeon_write_csr(OCTEON_IPD_INT_ENB, 0x1f);

        /* Enable zip interrupt on errors. Bit 25 of FUS_DAT3
            signals if the zip block is not present */
        if ((current_cpu_data.cputype != CPU_CAVIUM_CN30XX) &&
            (current_cpu_data.cputype != CPU_CAVIUM_CN50XX) &&
            ((octeon_read_csr(OCTEON_MIO_FUS_DAT3) & (1<<25)) == 0))
            octeon_write_csr(OCTEON_ZIP_INT_MASK, 1);

        /* Enable PKO interrupt on errors */
        octeon_write_csr(OCTEON_PKO_REG_INT_MASK, 0x2);

        /* Enable Timer interrupt on errors */
        octeon_write_csr(OCTEON_TIM_REG_INT_MASK, 0xff);

        /* Enable FPA interrupt on errors */
        /* The Queue X stack end tag check is disabled due to Octeon Pass
            2 errata FPA-100. This error condition can be set erroneously. */
        octeon_write_csr(OCTEON_FPA_INT_ENB, 0xb6db6dfull);

        /* Enable ASX interrupts on errors */
        if (current_cpu_data.cputype == CPU_CAVIUM_CN31XX ||
            current_cpu_data.cputype == CPU_CAVIUM_CN50XX ||
            current_cpu_data.cputype == CPU_CAVIUM_CN30XX) {

            octeon_write_csr(OCTEON_ASXX_INT_EN(0), 0x777);
        } else {
            octeon_write_csr(OCTEON_ASXX_INT_EN(1), 0xfff);
            octeon_write_csr(OCTEON_ASXX_INT_EN(0), 0xfff);
        }

        /* Enable PIP interrupts on errors */
        octeon_write_csr(OCTEON_PIP_INT_EN, 0x1f8);

        /* Enable PKO interrupts on errors */
        octeon_write_csr(OCTEON_PKO_REG_INT_MASK, 0x3);

        /* Enable DFA interrupts on errors. CN30XX and CN56XX don't
            have a DFA block. For others we have to read the fuse
            in CVMCTL to know if is there. */
        if ((current_cpu_data.cputype != CPU_CAVIUM_CN30XX) &&
            (current_cpu_data.cputype != CPU_CAVIUM_CN50XX) &&
            (current_cpu_data.cputype != CPU_CAVIUM_CN56XX) &&
            ((__read_64bit_c0_register($9, 7) & (1<<28)) == 0))
            octeon_write_csr(OCTEON_DFA_ERR, 0xffffffff);

        /* Enable Bootbus interupts on errors */
        octeon_write_csr(OCTEON_MIO_BOOT_INT, 0x3);

        /* Enable Key memory interupts on errors */
        if ((current_cpu_data.cputype == CPU_CAVIUM_CN38XX) ||
            (current_cpu_data.cputype == CPU_CAVIUM_CN58XX))
        {
            octeon_write_csr(OCTEON_KEY_INT_ENB, 0xf);
        }

        /* Enable reporting PCI bus errors */
        if (octeon_is_pci_host())
        {
            /* All chips currently don't use bits 63:34 */
            uint64_t pci_int_enb2 = (1ull<<34)-1;
            /* CN31XX and CN3020 don't use bits 24:23, 20:19 */
            if (current_cpu_data.cputype != CPU_CAVIUM_CN31XX && current_cpu_data.cputype != CPU_CAVIUM_CN50XX)
                pci_int_enb2 ^= (3ull<<23) | (3ull<<19);
            /* CN30XX doesn't use bits 24:22, 20:18 */
            if (current_cpu_data.cputype != CPU_CAVIUM_CN30XX)
                pci_int_enb2 ^= (7ull<<22) | (7ull<<18);
            /* Disable messages for normal PCI aborts */
            pci_int_enb2 ^= (1ull<<9) | (1ull<<4);
            octeon_write_csr(OCTEON_NPI_PCI_INT_ENB2, pci_int_enb2);
            octeon_write_csr(OCTEON_NPI_INT_ENB, 0x4);
        }
    }

    return 0;
}

/**
 * This is called from arch/mips/kernel/setup.c early
 * during startup.
 *
 * @return
 */
void __init plat_setup(void)
{
    /* Currently nothing to do here... */
}

