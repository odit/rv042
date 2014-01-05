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
 * $Id: cvmx-interrupt.h 2 2007-04-05 08:51:12Z tt $ $Name$
 */
#ifndef __CVMX_INTERRUPT_H__
#define __CVMX_INTERRUPT_H__

#ifdef	__cplusplus
extern "C" {
#endif

/**
 * Enumeration of Interrupt numbers
 */
typedef enum
{
    CVMX_IRQ_SW0        = 0,
    CVMX_IRQ_SW1        = 1,
    CVMX_IRQ_CIU0       = 2,
    CVMX_IRQ_CIU1       = 3,
    CVMX_IRQ_4          = 4,
    CVMX_IRQ_5          = 5,
    CVMX_IRQ_6          = 6,
    CVMX_IRQ_7          = 7,
    CVMX_IRQ_WORKQ0     = 8,
    CVMX_IRQ_WORKQ1     = 9,
    CVMX_IRQ_WORKQ2     = 10,
    CVMX_IRQ_WORKQ3     = 11,
    CVMX_IRQ_WORKQ4     = 12,
    CVMX_IRQ_WORKQ5     = 13,
    CVMX_IRQ_WORKQ6     = 14,
    CVMX_IRQ_WORKQ7     = 15,
    CVMX_IRQ_WORKQ8     = 16,
    CVMX_IRQ_WORKQ9     = 17,
    CVMX_IRQ_WORKQ10    = 18,
    CVMX_IRQ_WORKQ11    = 19,
    CVMX_IRQ_WORKQ12    = 20,
    CVMX_IRQ_WORKQ13    = 21,
    CVMX_IRQ_WORKQ14    = 22,
    CVMX_IRQ_WORKQ15    = 23,
    CVMX_IRQ_GPIO0      = 24,
    CVMX_IRQ_GPIO1      = 25,
    CVMX_IRQ_GPIO2      = 26,
    CVMX_IRQ_GPIO3      = 27,
    CVMX_IRQ_GPIO4      = 28,
    CVMX_IRQ_GPIO5      = 29,
    CVMX_IRQ_GPIO6      = 30,
    CVMX_IRQ_GPIO7      = 31,
    CVMX_IRQ_GPIO8      = 32,
    CVMX_IRQ_GPIO9      = 33,
    CVMX_IRQ_GPIO10     = 34,
    CVMX_IRQ_GPIO11     = 35,
    CVMX_IRQ_GPIO12     = 36,
    CVMX_IRQ_GPIO13     = 37,
    CVMX_IRQ_GPIO14     = 38,
    CVMX_IRQ_GPIO15     = 39,
    CVMX_IRQ_MBOX0      = 40,
    CVMX_IRQ_MBOX1      = 41,
    CVMX_IRQ_UART0      = 42,
    CVMX_IRQ_UART1      = 43,
    CVMX_IRQ_PCI_INT0   = 44,
    CVMX_IRQ_PCI_INT1   = 45,
    CVMX_IRQ_PCI_INT2   = 46,
    CVMX_IRQ_PCI_INT3   = 47,
    CVMX_IRQ_PCI_MSI0   = 48,
    CVMX_IRQ_PCI_MSI1   = 49,
    CVMX_IRQ_PCI_MSI2   = 50,
    CVMX_IRQ_PCI_MSI3   = 51,
    CVMX_IRQ_RESERVED44 = 52,
    CVMX_IRQ_TWSI       = 53,
    CVMX_IRQ_RML        = 54,
    CVMX_IRQ_TRACE      = 55,
    CVMX_IRQ_GMX_DRP0   = 56,
    CVMX_IRQ_GMX_DRP1   = 57,
    CVMX_IRQ_IPD_DRP    = 58,
    CVMX_IRQ_KEY_ZERO   = 59,
    CVMX_IRQ_TIMER0     = 60,
    CVMX_IRQ_TIMER1     = 61,
    CVMX_IRQ_TIMER2     = 62,
    CVMX_IRQ_TIMER3     = 63,
    CVMX_IRQ_RESERVED56 = 64,
    CVMX_IRQ_RESERVED57 = 65,
    CVMX_IRQ_RESERVED58 = 66,
    CVMX_IRQ_RESERVED59 = 67,
    CVMX_IRQ_RESERVED60 = 68,
    CVMX_IRQ_RESERVED61 = 69,
    CVMX_IRQ_RESERVED62 = 70,
    CVMX_IRQ_RESERVED63 = 71,
} cvmx_irq_t;

/**
 * Function prototype for the exception handler
 */
typedef void (*cvmx_interrupt_exception_t)(uint64_t registers[32]);

/**
 * Function prototype for interrupt handlers
 */
typedef void (*cvmx_interrupt_func_t)(int irq_number, uint64_t registers[32], void *user_arg);

/**
 * Register an interrupt handler for the specified interrupt number.
 *
 * @param irq_number Interrupt number to register for (0-135)
 * @param func       Function to call on interrupt.
 * @param user_arg   User data to pass to the interrupt handler
 */
void cvmx_interrupt_register(int irq_number, cvmx_interrupt_func_t func, void *user_arg);

/**
 * Set the exception handler for all non interrupt sources.
 *
 * @param handler New exception handler
 * @return Old exception handler
 */
cvmx_interrupt_exception_t cvmx_interrupt_set_exception(cvmx_interrupt_exception_t handler);

/**
 * Masks a given interrupt number
 *
 * @param irq_number interrupt number to mask (0-135)
 */
static inline void cvmx_interrupt_mask_irq(int irq_number)
{
    if (irq_number<8)
    {
        uint32_t mask;
        asm volatile ("mfc0 %0,$12,0" : "=r" (mask));
        mask &= ~(1<< (8 + irq_number));
        asm volatile ("mtc0 %0,$12,0" : : "r" (mask));
    }
    else
    {
        int ciu_bit = (irq_number - 8) & 63;
        int ciu_offset = cvmx_get_core_num() * 2 + (irq_number - 8) / 64;
        uint64_t mask = cvmx_read_csr(CVMX_CIU_INTX_EN0(ciu_offset));
        mask &= ~(1ull << ciu_bit);
        cvmx_write_csr(CVMX_CIU_INTX_EN0(ciu_offset), mask);
    }
}


/**
 * Unmasks a given interrupt number
 *
 * @param irq_number interrupt number to unmask (0-135)
 */
static inline void cvmx_interrupt_unmask_irq(int irq_number)
{
    if (irq_number<8)
    {
        uint32_t mask;
        asm volatile ("mfc0 %0,$12,0" : "=r" (mask));
        mask |= (1<< (8 + irq_number));
        asm volatile ("mtc0 %0,$12,0" : : "r" (mask));
    }
    else
    {
        int ciu_bit = (irq_number - 8) & 63;
        int ciu_offset = cvmx_get_core_num() * 2 + (irq_number - 8) / 64;
        uint64_t mask = cvmx_read_csr(CVMX_CIU_INTX_EN0(ciu_offset));
        mask |= (1ull << ciu_bit);
        cvmx_write_csr(CVMX_CIU_INTX_EN0(ciu_offset), mask);
    }
}

#ifdef	__cplusplus
}
#endif

#endif
