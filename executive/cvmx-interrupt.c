/****************************************************************
 * Copyright (c) 2003-2005, Cavium Networks. All rights reserved.
 *
 * This Software is the property of Cavium Networks.  The Software and all
 * accompanying documentation are copyrighted.  The Software made available
 * here constitutes the proprietary information of Cavium Networks.  You
 * agree to take reasonable steps to prevent the disclosure, unauthorized use
 * or unauthorized distribution of the Software.  You shall use this Software
 * solely with Cavium hardware.
 *
 * Except as expressly permitted in a separate Software License Agreement
 * between You and Cavium Networks, you shall not modify, decompile,
 * disassemble, extract, or otherwise reverse engineer this Software.  You
 * shall not make any copy of the Software or its accompanying documentation,
 * except for copying incident to the ordinary and intended use of the
 * Software and the Underlying Program and except for the making of a single
 * archival copy.
 *
 * This Software, including technical data, may be subject to U.S.  export
 * control laws, including the U.S.  Export Administration Act and its
 * associated regulations, and may be subject to export or import regulations
 * in other countries.  You warrant that You will comply strictly in all
 * respects with all such regulations and acknowledge that you have the
 * responsibility to obtain licenses to export, re-export or import the
 * Software.
 *
 * TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND CAVIUM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 * TO THE SOFTWARE, INCLUDING ITS CONDITION, ITS CONFORMITY TO ANY
 * REPRESENTATION OR DESCRIPTION, OR THE EXISTENCE OF ANY LATENT OR PATENT
 * DEFECTS, AND CAVIUM SPECIFICALLY DISCLAIMS ALL IMPLIED (IF ANY) WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR
 * PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET
 * POSSESSION OR CORRESPONDENCE TO DESCRIPTION.  THE ENTIRE RISK ARISING OUT
 * OF USE OR PERFORMANCE OF THE SOFTWARE LIES WITH YOU.
 *
 **************************************************************************/

/**
 * @file
 *
 * Interface to the Mips interrupts.
 *
 * $Id: cvmx-interrupt.c 2 2007-04-05 08:51:12Z tt $ $Name$
 */
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "cvmx-config.h"
#include "cvmx.h"
#include "cvmx-interrupt.h"
#include "cvmx-sysinfo.h"
#include "cvmx-uart.h"
#include "cvmx-pow.h"
#include "cvmx-ebt3000.h"
#include "cvmx-coremask.h"
#include "cvmx-spinlock.h"

EXTERN_ASM void cvmx_interrupt_stage1();

/**
 * Internal status the interrupt registration
 */
typedef struct
{
    cvmx_interrupt_func_t handlers[256];  /**< One function to call per interrupt */
    void *                data[256];      /**< User data per interrupt */
    cvmx_interrupt_exception_t exception_handler;
} cvmx_interrupt_state_t;

/**
 * Internal state the interrupt registration
 */
static CVMX_SHARED cvmx_interrupt_state_t cvmx_interrupt_state;
static CVMX_SHARED cvmx_spinlock_t cvmx_interrupt_default_lock;

#define COP0_CAUSE      "$13,0"
#define COP0_STATUS     "$12,0"
#define COP0_BADVADDR   "$8,0"
#define COP0_EPC        "$14,0"
#define READ_COP0(dest, R) asm volatile ("dmfc0 %[rt]," R : [rt] "=r" (dest))


/**
 * version of printf that works better in exception context.
 *
 * @param format
 */
static void safe_printf(const char *format, ...)
{
    static char buffer[256];
    va_list args;
    va_start(args, format);
    int count = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    char *ptr = buffer;
    while (count-- > 0)
    {
        cvmx_uart_lsr_t lsrval;

        /* Spin until there is room */
        do
        {
            lsrval.u64 = cvmx_read_csr(CVMX_MIO_UARTX_LSR(0));
            if (lsrval.s.temt == 0)
                cvmx_wait(10000);   /* Just to reduce the load on the system */
        }
        while (lsrval.s.temt == 0);

        if (*ptr == '\n')
            cvmx_write_csr(CVMX_MIO_UARTX_THR(0), '\r');
        cvmx_write_csr(CVMX_MIO_UARTX_THR(0), *ptr++);
    }
}


/**
 * Dump all useful registers to the console
 *
 * @param registers CPU register to dump
 */
static void cvmx_interrupt_dump_registers(uint64_t registers[32])
{
    static const char *name[32] = {"r0","at","v0","v1","a0","a1","a2","a3",
        "t0","t1","t2","t3","t4","t5","t6","t7","s0","s1","s2","s3","s4","s5",
        "s6","s7", "t8","t9", "k0","k1","gp","sp","s8","ra"};
    uint64_t reg;
    for (reg=0; reg<16; reg++)
    {
        safe_printf("%3s ($%02ld): 0x%016lx \t %3s ($%02ld): 0x%016lx\n",
               name[reg], reg, registers[reg], name[reg+16], reg+16, registers[reg+16]);
    }
    READ_COP0(reg, COP0_CAUSE);
    safe_printf("%16s: 0x%016lx\n", "COP0_CAUSE", reg);
    READ_COP0(reg, COP0_STATUS);
    safe_printf("%16s: 0x%016lx\n", "COP0_STATUS", reg);
    READ_COP0(reg, COP0_BADVADDR);
    safe_printf("%16s: 0x%016lx\n", "COP0_BADVADDR", reg);
    READ_COP0(reg, COP0_EPC);
    safe_printf("%16s: 0x%016lx\n", "COP0_EPC", reg);
}


/**
 * Default exception handler. Prints out the exception
 * cause decode and all relevant registers.
 *
 * @param registers Registers at time of the exception
 */
static void cvmx_interrupt_default_exception_handler(uint64_t registers[32])
{
    uint64_t trap_print_cause;

    ebt3000_str_write("Trap");
    cvmx_spinlock_lock(&cvmx_interrupt_default_lock);
    safe_printf("******************************************************************\n");
    safe_printf("Core %lu: Unhandled Exception. Cause register decodes to:\n", cvmx_get_core_num());
    READ_COP0(trap_print_cause, COP0_CAUSE);
    switch ((trap_print_cause >> 2) & 0x1f)
    {
        case 0x0:
            safe_printf("Interrupt\n");
            break;
        case 0x1:
            safe_printf("TLB Mod\n");
            break;
        case 0x2:
            safe_printf("tlb load/fetch\n");
            break;
        case 0x3:
            safe_printf("tlb store\n");
            break;
        case 0x4:
            safe_printf("address exc, load/fetch\n");
            break;
        case 0x5:
            safe_printf("address exc, store\n");
            break;
        case 0x6:
            safe_printf("bus error, inst. fetch\n");
            break;
        case 0x7:
            safe_printf("bus error, load/store\n");
            break;
        case 0x8:
            safe_printf("syscall\n");
            break;
        case 0x9:
            safe_printf("breakpoint \n");
            break;
        case 0xa:
            safe_printf("reserved instruction\n");
            break;
        case 0xb:
            safe_printf("cop unusable\n");
            break;
        case 0xc:
            safe_printf("arithmetic overflow\n");
            break;
        case 0xd:
            safe_printf("trap\n");
            break;
        case 0xf:
            safe_printf("floating point exc\n");
            break;
        case 0x12:
            safe_printf("cop2 exception\n");
            break;
        case 0x16:
            safe_printf("mdmx unusable\n");
            break;
        case 0x17:
            safe_printf("watch\n");
            break;
        case 0x18:
            safe_printf("machine check\n");
            break;
        case 0x1e:
            safe_printf("cache error\n");
            break;
        default:
            safe_printf("Reserved exception cause.\n");
            break;

    }

    safe_printf("******************************************************************\n");
    cvmx_interrupt_dump_registers(registers);
    safe_printf("******************************************************************\n");
    cvmx_spinlock_unlock(&cvmx_interrupt_default_lock);

    while (1)
    {
 	if (cvmx_sysinfo_get()->board_type == CVMX_BOARD_TYPE_SIM)
            asm volatile ("break 1");
        else
            asm volatile ("wait");
    }
}


/**
 * Default interrupt handler if the user doesn't register one.
 *
 * @param irq_number IRQ that caused this interrupt
 * @param registers  Register at the time of the interrupt
 * @param user_arg   Unused optional user data
 */
static void cvmx_interrupt_default(int irq_number, uint64_t registers[32], void *user_arg)
{
    safe_printf("cvmx_interrupt_default: Received interrupt %d\n", irq_number);
    cvmx_interrupt_dump_registers(registers);
}


/**
 * Handler for interrupt lines 2 and 3. These are directly tied
 * to the CIU. The handler queres the status of the CIU and
 * calls the secondary handler for the CIU interrupt that
 * occurred. Interrupts coming in on line 2 are mapped to 8-71.
 * Interrupts on line 3 are mapped to 72 - 135.
 *
 * @param irq_number Interrupt number that fired (2 or 3)
 * @param registers  Registers at the time of the interrupt
 * @param user_arg   Unused user argument
 */
static void cvmx_interrupt_ciu(int irq_number, uint64_t registers[32], void *user_arg)
{
    int ciu_offset = cvmx_get_core_num() * 2 + irq_number - 2;
    uint64_t irq_mask = cvmx_read_csr(CVMX_CIU_INTX_SUM0(ciu_offset)) & cvmx_read_csr(CVMX_CIU_INTX_EN0(ciu_offset));
    int irq = 8 + (irq_number - 2) * 64;

    while (irq_mask)
    {
        if (irq_mask&1)
        {
            cvmx_interrupt_state.handlers[irq](irq, registers, cvmx_interrupt_state.data[irq]);
            return;
        }
        irq_mask = irq_mask >> 1;
        irq++;
    }
}


/**
 * Called for all RML interrupts. This is usually an ECC error
 *
 * @param irq_number Interrupt number that we're being called for
 * @param registers  Registers at the time of the interrupt
 * @param user_arg   Unused user argument
 */
static void cvmx_interrupt_ecc(int irq_number, uint64_t registers[32], void *user_arg)
{
    /* Get the ECC status from LMC config zero */
    uint64_t mem_cfg0 = cvmx_read_csr(CVMX_LMC_MEM_CFG0);
    /* Write out the same value to clear the ECC error bits */
    cvmx_write_csr(CVMX_LMC_MEM_CFG0, mem_cfg0);

    mem_cfg0 = (mem_cfg0 >> 21) & 0xff;
    if (mem_cfg0 & 0xf0)
    {
        typedef union
        {
            uint64_t u64;
            struct
            {
                uint64_t reserved                : 32;
                uint64_t fdimm                   : 2;
                uint64_t fbunk                   : 1;
                uint64_t fbank                   : 3;
                uint64_t frow                    : 14;
                uint64_t fcol                    : 12;
            } s;
        } cvmx_lmc_fadr_t;
        cvmx_lmc_fadr_t fadr;
        fadr.u64 = cvmx_read_csr(CVMX_LMC_FADR);
        safe_printf("\nECC: Double bit error\n"
               "\tFailing dimm:   %u\n"
               "\tFailing rank:   %u\n"
               "\tFailing bank:   %u\n"
               "\tFailing row:    0x%x\n"
               "\tFailing column: 0x%x\n",
               fadr.s.fdimm, fadr.s.fbunk, fadr.s.fbank, fadr.s.frow, fadr.s.fcol);
    }

    /* Get the ECC status from L2T_ERR */
    uint64_t l2t_err = cvmx_read_csr(CVMX_L2T_ERR);
    /* Write out the same value to clear the ECC error bits */
    cvmx_write_csr(CVMX_L2T_ERR, l2t_err);

    l2t_err = (l2t_err >> 3) & 0x3;
    if (l2t_err & 0x2)
        safe_printf("\nECC: L2T Double bit error\n");

    /* Get the ECC status from L2D_ERR */
    uint64_t l2d_err = cvmx_read_csr(CVMX_L2D_ERR);
    /* Write out the same value to clear the ECC error bits */
    cvmx_write_csr(CVMX_L2D_ERR, l2d_err);

    l2d_err = (l2d_err >> 3) & 0x3;
    if (l2d_err & 0x2)
        safe_printf("\nECC: L2D Double bit error\n");

    /* Get the ECC status from POW */
    cvmx_pow_ecc_err_t pow_ecc;
    pow_ecc.u64 = cvmx_read_csr(CVMX_POW_ECC_ERR);
    if (pow_ecc.s.dbe)
        safe_printf("\nECC: POW Double bit error\n");
}


/**
 * Process an interrupt request
 *
 * @param registers Registers at time of interrupt / exception
 * Registers 0-31 are standard MIPS, others specific to this routine
 * @return
 */
EXTERN_ASM void cvmx_interrupt_do_irq(uint64_t registers[35]);
void cvmx_interrupt_do_irq(uint64_t registers[35])
{
    uint64_t        mask;
    uint64_t        cause;
    uint64_t        status;
    uint64_t        cache_err;
    int             i;
    uint32_t exc_vec;

    /* Determine the cause of the interrupt */
    asm volatile ("dmfc0 %0,$13,0" : "=r" (cause));
    asm volatile ("dmfc0 %0,$12,0" : "=r" (status));

    /* The assembly stub at each exception vector saves its address in k1 when
    ** it calls the stage 2 handler.  We use this to compute the exception vector
    ** that brought us here */
    exc_vec = (uint32_t)(registers[27] & 0x780);  /* Mask off bits we need to ignore */

    /* Check for cache errors.  The cache errors go to a separate exception vector,
    ** so we will only check these if we got here from a cache error exception, and
    ** the ERL (error level) bit is set. */
    if (exc_vec == 0x100 && (status & 0x4))
    {
        i = cvmx_get_core_num();
        CVMX_MF_CACHE_ERR(cache_err);

        /* Use copy of DCACHE_ERR register that early exception stub read */
        if (registers[34] & 0x1)
        {
            safe_printf("Dcache error detected: core: %d, set: %d, va 6:3: 0x%x\n", i, (cache_err >> 3) & 0x3, (cache_err >> 3) & 0xf);
            uint64_t dcache_err = 0;
            CVMX_MT_DCACHE_ERR(dcache_err);
        } 
        else if (cache_err & 0x1)
        {
            safe_printf("Icache error detected: core: %d, set: %d, way : %d\n", i, (cache_err >> 5) & 0x3f, (cache_err >> 7) & 0x3);
            cache_err = 0;
            CVMX_MT_CACHE_ERR(cache_err);
        }
        else
            safe_printf("Cache error exception: core %d\n", i);
    }

    if ((cause & 0x7c) != 0)
    {
        cvmx_interrupt_state.exception_handler(registers);
        return;
    }

    /* Convert the cause into an active mask */
    mask = ((cause & status) >> 8) & 0xff;
    if (mask == 0)
        return; /* Spurious interrupt */

    for (i=0; i<8; i++)
    {
        if (mask & (1<<i))
        {
            cvmx_interrupt_state.handlers[i](i, registers, cvmx_interrupt_state.data[i]);
            return;
        }
    }

    /* We should never get here */
    cvmx_interrupt_default_exception_handler(registers);
}


/**
 * Initialize the interrupt routine and copy the low level
 * stub into the correct interrupt vector. This is called
 * automatically during application startup.
 */
void cvmx_interrupt_initialize(void)
{
    void *low_level_loc;
    cvmx_sysinfo_t *sys_info_ptr = cvmx_sysinfo_get();
    int i;

    /* Disable all CIU interrupts by default */
    cvmx_write_csr(CVMX_CIU_INTX_EN0(cvmx_get_core_num()*2), 0);
    cvmx_write_csr(CVMX_CIU_INTX_EN0(cvmx_get_core_num()*2+1), 0);
    cvmx_write_csr(CVMX_CIU_INTX_EN1(cvmx_get_core_num()*2), 0);
    cvmx_write_csr(CVMX_CIU_INTX_EN1(cvmx_get_core_num()*2+1), 0);

    if (cvmx_coremask_first_core(sys_info_ptr->core_mask))
    {
        cvmx_interrupt_state.exception_handler = cvmx_interrupt_default_exception_handler;

        for (i=0; i<256; i++)
        {
            cvmx_interrupt_state.handlers[i] = cvmx_interrupt_default;
            cvmx_interrupt_state.data[i] = NULL;
        }

        low_level_loc = CASTPTR(void, CVMX_ADD_SEG32(CVMX_MIPS32_SPACE_KSEG0,sys_info_ptr->exception_base_addr));
        memcpy(low_level_loc + 0x80, (void*)cvmx_interrupt_stage1, 0x80);
        memcpy(low_level_loc + 0x100, (void*)cvmx_interrupt_stage1, 0x80);
        memcpy(low_level_loc + 0x180, (void*)cvmx_interrupt_stage1, 0x80);
        memcpy(low_level_loc + 0x200, (void*)cvmx_interrupt_stage1, 0x80);
        CVMX_SYNC;

        /* Add an interrupt handlers for chained CIU interrupts */
        cvmx_interrupt_register(CVMX_IRQ_CIU0, cvmx_interrupt_ciu, NULL);
        cvmx_interrupt_register(CVMX_IRQ_CIU1, cvmx_interrupt_ciu, NULL);

        /* Add an interrupt handler for ECC failures */
        cvmx_interrupt_register(CVMX_IRQ_RML, cvmx_interrupt_ecc, NULL);

        /* Enable ECC Interrupts for double bit errors from main memory */
        uint64_t mem_cfg0 = cvmx_read_csr(CVMX_LMC_MEM_CFG0);
        mem_cfg0 |= 0x2 << 19;
        cvmx_write_csr(CVMX_LMC_MEM_CFG0, mem_cfg0);

        /* Enable ECC Interrupts for double bit errors from L2C Tags */
        uint64_t l2t_err = cvmx_read_csr(CVMX_L2T_ERR);
        l2t_err |= 0x5;
        cvmx_write_csr(CVMX_L2T_ERR, l2t_err);

        /* Enable ECC Interrupts for double bit errors from L2D Errors */
        uint64_t l2d_err = cvmx_read_csr(CVMX_L2D_ERR);
        l2d_err |= 0x5;
        cvmx_write_csr(CVMX_L2D_ERR, l2d_err);

        /* Enable ECC Interrupts for double bit errors from the POW */
        cvmx_pow_ecc_err_t pow_ecc;
        pow_ecc.u64 = cvmx_read_csr(CVMX_POW_ECC_ERR);
        pow_ecc.s.dbe_ie = 1;
        cvmx_write_csr(CVMX_POW_ECC_ERR, pow_ecc.u64);
        cvmx_interrupt_unmask_irq(CVMX_IRQ_RML);
    }

    cvmx_interrupt_unmask_irq(CVMX_IRQ_CIU0);
    cvmx_interrupt_unmask_irq(CVMX_IRQ_CIU1);
    CVMX_ICACHE_INVALIDATE;

    /* Enable interrupts for each core (bit0 of COP0 Status) */
    uint32_t mask;
    asm volatile (
        "mfc0   %0,$12,0\n"
        "ori    %0, %0, 1\n"
        "mtc0   %0,$12,0\n"
        : "=r" (mask));
}


/**
 * Register an interrupt handler for the specified interrupt number.
 *
 * @param irq_number Interrupt number to register for (0-135)
 * @param func       Function to call on interrupt.
 * @param user_arg   User data to pass to the interrupt handler
 */
void cvmx_interrupt_register(int irq_number, cvmx_interrupt_func_t func, void *user_arg)
{
    cvmx_interrupt_state.handlers[irq_number] = func;
    cvmx_interrupt_state.data[irq_number] = user_arg;
    CVMX_SYNCWS;
}


/**
 * Set the exception handler for all non interrupt sources.
 *
 * @param handler New exception handler
 * @return Old exception handler
 */
cvmx_interrupt_exception_t cvmx_interrupt_set_exception(cvmx_interrupt_exception_t handler)
{
    cvmx_interrupt_exception_t result = cvmx_interrupt_state.exception_handler;
    cvmx_interrupt_state.exception_handler = handler;
    CVMX_SYNCWS;
    return result;
}









