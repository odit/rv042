/****************************************************************
 * Copyright (c) 2003-2004, Cavium Networks. All rights reserved.
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

#include <machine/asm.h>
#include <machine/regdef.h>

.set noreorder
.set noat

LEAF(cvmx_interrupt_stage1)
	dla     k0, cvmx_interrupt_stage2
	jalr 	k1, k0   // Save our address in k1, so we can tell which
                         // vector we are coming from.
	nop
END(cvmx_interrupt_stage1)

#define STACK_SIZE  (35*8)
LEAF(cvmx_interrupt_stage2)
	dsubu	sp, sp, STACK_SIZE
	sd	zero, 0(sp)	// Just a place holder
	sd	$1, 8(sp)	// start saving registers	
	sd	$2, 16(sp)
	sd 	$3, 24(sp)
	sd 	$4, 32(sp)
	sd	$5, 40(sp)
	sd	$6, 48(sp)
	sd	$7, 56(sp)
	sd	$8, 64(sp)
	sd	$9, 72(sp)
	sd	$10, 80(sp)
	sd	$11, 88(sp)
	sd	$12, 96(sp)
	sd	$13, 104(sp)
	sd	$14, 112(sp)
	sd	$15, 120(sp)
	sd	$16, 128(sp)
	sd	$17, 136(sp)
	sd	$18, 144(sp)
	sd	$19, 152(sp)
	sd	$20, 160(sp)
	sd	$21, 168(sp)
	sd	$22, 176(sp)
	sd	$23, 184(sp)
	sd	$24, 192(sp)	
	sd	$25, 200(sp)
	sd	$26, 208(sp)
	sd	$27, 216(sp)
	mfhi	k0		// Reading lo and high takes multiple cycles
	mflo	k1		// Do it here so it completes by the time we need it
	sd	$28, 224(sp)
	sd	$29, 232(sp)
	sd	$30, 240(sp)
	sd	$31, 248(sp)	// saved all general purpose registers
	sd	k0, 256(sp)	// save hi
	sd	k1, 264(sp)	// save lo
        /* Save DCACHE error register early, since any non-errored DCACHE accesses will clear
        ** error bit */
        dmfc0   k0, $27, 1
        sd      k0, 272(sp)  

	dla     k0, cvmx_interrupt_do_irq
	jal 	k0
	dadd	a0, sp, 0	// First argument is array of registers
	
	ld	k0, 256(sp)	// read hi
	ld	k1, 264(sp)	// read lo
	mthi	k0		// restore hi
	mtlo	k1		// restore lo
	
	ld	$1, 8(sp)	// start restoring registers	
	ld	$2, 16(sp)
	ld 	$3, 24(sp)
	ld 	$4, 32(sp)
	ld	$5, 40(sp)
	ld	$6, 48(sp)
	ld	$7, 56(sp)
	ld	$8, 64(sp)
	ld	$9, 72(sp)
	ld	$10, 80(sp)
	ld	$11, 88(sp)
	ld	$12, 96(sp)
	ld	$13, 104(sp)
	ld	$14, 112(sp)
	ld	$15, 120(sp)
	ld	$16, 128(sp)
	ld	$17, 136(sp)
	ld	$18, 144(sp)
	ld	$19, 152(sp)
	ld	$20, 160(sp)
	ld	$21, 168(sp)
	ld	$22, 176(sp)
	ld	$23, 184(sp)
	ld	$24, 192(sp)	
	ld	$25, 200(sp)
	ld	$26, 208(sp)
	ld	$28, 224(sp)
	ld	$29, 232(sp)
	ld	$30, 240(sp)
	ld	$31, 248(sp)	// restored all general purpose registers
	daddu	sp, sp, STACK_SIZE
	eret
	nop
END(cvmx_interrupt_stage2)

