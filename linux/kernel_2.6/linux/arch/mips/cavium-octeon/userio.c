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

#include <asm/reboot.h>
#include <asm/io.h>
#include <asm/time.h>
#include <asm/processor.h>
#include <asm/reboot.h>
#include <asm/system.h>
#include <asm/irq_cpu.h>
#include <asm/mipsregs.h>
#include <asm/gdb-stub.h>
#include <asm/bootinfo.h>

#include "hal.h"


/**
 *
 * @return
 */
void octeon_user_io_init(void)
{
    octeon_cvmemctl_t cvmmemctl;
    octeon_iob_fau_timeout_t fau_timeout;
    octeon_pow_nw_tim_t nm_tim;

    /* Get the current settings for CP0_CVMMEMCTL_REG */
    cvmmemctl.u64 = __read_64bit_c0_register($11, 7);

    cvmmemctl.s.dismarkwblongto = 0;        /**< R/W If set, marked write-buffer entries time out the same as
                                                as other entries; if clear, marked write-buffer entries use the
                                                maximum timeout. */
    cvmmemctl.s.dismrgclrwbto = 0;          /**< R/W If set, a merged store does not clear the write-buffer entry
                                                timeout state. */
    cvmmemctl.s.iobdmascrmsb = 0;           /**< R/W Two bits that are the MSBs of the resultant CVMSEG LM word
                                                location for an IOBDMA. The other 8 bits come from the SCRADDR
                                                field of the IOBDMA. */
    cvmmemctl.s.syncwsmarked = 0;           /**< R/W If set, SYNCWS and SYNCS only order marked stores; if clear,
                                                SYNCWS and SYNCS only order unmarked stores. SYNCWSMARKED has no
                                                effect when DISSYNCWS is set. */
    cvmmemctl.s.dissyncws = 0;              /**< R/W If set, SYNCWS acts as SYNCW and SYNCS acts as SYNC. */
    if (octeon_is_pass1())
        cvmmemctl.s.diswbfst = 0;           /**< R/W If set, no stall happens on write buffer full. */
    else
        cvmmemctl.s.diswbfst = 1;           /**< R/W If set, no stall happens on write buffer full. */
    cvmmemctl.s.xkmemenas = 0;              /**< R/W If set (and SX set), supervisor-level loads/stores can use
                                                XKPHYS addresses with VA<48>==0 */
#ifdef CONFIG_CAVIUM_OCTEON_USER_MEM
    cvmmemctl.s.xkmemenau = 1;              /**< R/W If set (and UX set), user-level loads/stores can use XKPHYS
                                                addresses with VA<48>==0 */
#else
    cvmmemctl.s.xkmemenau = 0;
#endif
    cvmmemctl.s.xkioenas = 0;               /**< R/W If set (and SX set), supervisor-level loads/stores can use
                                                XKPHYS addresses with VA<48>==1 */
    cvmmemctl.s.xkioenau = 1;               /**< R/W If set (and UX set), user-level loads/stores can use XKPHYS
                                                addresses with VA<48>==1 */
    cvmmemctl.s.allsyncw = 0;               /**< R/W If set, all stores act as SYNCW (NOMERGE must be set when
                                                this is set) RW, reset to 0. */
    cvmmemctl.s.nomerge = 0;                /**< R/W If set, no stores merge, and all stores reach the coherent
                                                bus in order. */
    cvmmemctl.s.didtto = 0;                 /**< R/W Selects the bit in the counter used for DID time-outs
                                                0 = 231, 1 = 230, 2 = 229, 3 = 214. Actual time-out is between
                                                1× and 2× this interval. For example, with DIDTTO=3, expiration
                                                interval is between 16K and 32K. */
    cvmmemctl.s.csrckalwys = 0;             /**< R/W If set, the (mem) CSR clock never turns off. */
    cvmmemctl.s.mclkalwys = 0;              /**< R/W If set, mclk never turns off. */
    cvmmemctl.s.wbfltime = 0;               /**< R/W Selects the bit in the counter used for write buffer flush
                                                time-outs (WBFLT+11) is the bit position in an internal counter
                                                used to determine expiration. The write buffer expires between
                                                1× and 2× this interval. For example, with WBFLT = 0, a write
                                                buffer expires between 2K and 4K cycles after the write buffer
                                                entry is allocated. */
    cvmmemctl.s.istrnol2 = 0;               /**< R/W If set, do not put Istream in the L2 cache. */
    cvmmemctl.s.wbthresh = 10;              /**< R/W The write buffer threshold. */
    cvmmemctl.s.cvmsegenak = 1;             /**< R/W If set, CVMSEG is available for loads/stores in kernel/debug mode. */
    cvmmemctl.s.cvmsegenas = 0;             /**< R/W If set, CVMSEG is available for loads/stores in supervisor mode. */
    cvmmemctl.s.cvmsegenau = 0;             /**< R/W If set, CVMSEG is available for loads/stores in user mode. */
    cvmmemctl.s.lmemsz = CONFIG_CAVIUM_OCTEON_CVMSEG_SIZE; /**< R/W Size of local memory in cache blocks, 54 (6912 bytes) is max legal value. */

    if (smp_processor_id() == 0)
        printk("CVMSEG size: %d cache lines (%d bytes)\n",
               CONFIG_CAVIUM_OCTEON_CVMSEG_SIZE, CONFIG_CAVIUM_OCTEON_CVMSEG_SIZE * 128);

    __write_64bit_c0_register($11, 7, cvmmemctl.u64);

    /* Set a default for the hardware timeouts */
    fau_timeout.u64 = 0;
    fau_timeout.s.tout_enb = 1;
    fau_timeout.s.tout_val = 16; /* 4096 cycles */
    octeon_write_csr(OCTEON_IOB_FAU_TIMEOUT, fau_timeout.u64);

    nm_tim.u64 = 0;
    nm_tim.s.nw_tim = 3; /* 4096 cycles */
    octeon_write_csr(OCTEON_POW_NW_TIM, nm_tim.u64);
}

