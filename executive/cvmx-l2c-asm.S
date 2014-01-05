#include "regdef.h"

#define CVMX_L2C_DBG             0x8001180080000030
#define DBG_L2T                  0x1				
#define DBG_PPNUM_SHIFT          6
#define DBG_SET_SHIFT            3
#define DBG_SET_MASK             0x7

	.file	1 "cvmx-l2c.S"
	.section .mdebug.eabi64
	.previous
	.text
	.align	3
	.globl	cvmx_get_l2c_tag_asm
	.ent	cvmx_get_l2c_tag_asm

cvmx_get_l2c_tag_asm:			// (assoc, line)
	.set	noreorder
#define assoc a0		// 0...7
#define line a1			// XKPHYS | (0...1023<<7) to avoid TLBMiss
	
	sync			// Force the write through first

// Set us up as the debug core and select the assoc

	rdhwr	t1, $0		// My core number
	sll	t1, t1, DBG_PPNUM_SHIFT
	ori	t1, t1, DBG_L2T
	andi	assoc, assoc, DBG_SET_MASK
	sll	assoc, assoc, DBG_SET_SHIFT
	or	t1, t1, assoc

	dli	t3, CVMX_L2C_DBG
	ld	t0, 0(t3)	// save the DBG reg for later

	sync
	cache	1, 0(zero)	// Invalidate dCache to get miss on next ld

	sd	t1, 0(t3)	// Put L2C into debug mode
	ld	t2, 0(t3)	// Get it out to the L2C before

	ld	v0, 32(line)	// We get the tag

	sd	t0, 0(t3)	// restore DBG
	ld	t2, 0(t3)	// and force it out

	sync
	cache	1, 0(zero)	// Clean up our mess

	j	ra
	nop

	.set	reorder
	.end	cvmx_get_l2c_tag_asm
