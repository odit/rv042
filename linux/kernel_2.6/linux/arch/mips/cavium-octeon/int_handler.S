/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2004 Cavium Networks
 */
#include <linux/config.h>
#include <asm/asm.h>
#include <asm/mipsregs.h>
#include <asm/addrspace.h>
#include <asm/regdef.h>
#include <asm/stackframe.h>

	.text
	.set	noat
	.set 	noreorder
	.align  5
        NESTED(octeon_handle_irq, PT_SIZE, sp)
        SAVE_ALL
        CLI
        .set    at
        .set    noreorder
        mfc0    t0, CP0_CAUSE
        mfc0    t2, CP0_STATUS

	dla	ra, ret_from_irq	// Set the return address to ret_from_irq to save an extra jump later
        move   	a1, sp			// Put the registers in Argument 1
        and     t0, t2			// AND Cause and Status to get interrupt bitmask
#ifdef CONFIG_SMP	
	bbit1	t0, STATUSB_IP3, ll_cpu_ip3	// On SMP, treat STATUSF_IP3 special
#endif	
	 srl	t1, t0, 8		// Shift so interrupts are bit 7-0
	bbit1	t0, STATUSB_IP2, ll_ciu_interrupt_check	// CIU interrupt must be decoded
	 andi	t1, 0xff		// AND off only the interrupt bits
	clz	a0, t1			// Find first bit set, 32 means non set
	li	t0, 31			// Load 31 immediate for flipping a0 count
	bbit1	a0, 5, ll_spurious_interrupt 	// spurious if nothing set
	 sub	a0, t0, a0		// Convert a0 to bit number
        j       do_IRQ			// Call the C interrupt handler
	 nop

#ifdef CONFIG_SMP	
ll_cpu_ip3:
        j       mailbox_interrupt
         li      a0, 3
#endif
	
ll_ciu_interrupt_check:
	lui     t0,0x8001		// Upper part of OCTEON_CIU_INTX_SUM0	(t0=0x80010000)
	rdhwr	t1, $0			// Get the core id
	ori     t0,t0,0x700		// Next part of OCTEON_CIU_INTX_SUM0	(t0=0x80010700)
	dsll32  t0,t0,0x0		// OCTEON_CIU_INTX_SUM0			(t0=0x8001070000000000)
	ori	t2,t0,0x200		// OCTEON_CIU_INTX_EN0			(t2=0x8001070000000200)
	dins	t0, t1, 4, 4		// Insert core id * 2 * 8 into OCTEON_CIU_INTX_SUM0
	ld	t0, 0(t0)		// Load OCTEON_CIU_INTX_SUM0
	dins	t2, t1, 5, 4		// Insert (core id * 2 * 8)*2 into OCTEON_CIU_INTX_EN0
	ld	t2, 0(t2)		// Load OCTEON_CIU_INTX_EN0
	and	t0, t0, t2		// OCTEON_CIU_INTX_SUM0 & OCTEON_CIU_INTX_EN0
	
	dclz	a0, t0			// Find first bit set
	li	t0, 71
	bbit1	a0, 6, ll_spurious_interrupt	// spurious if nothing set
	 move 	a1, sp			// Second param is the array of registers
        j	do_IRQ			// Call the main C IRQ handler
	 sub	a0, t0, a0		// IRQ = 71 - leading zeros
	
ll_spurious_interrupt:	
        j       spurious_interrupt
	 nop

        END(octeon_handle_irq)

