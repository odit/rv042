/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 1998, 1999, 2003 by Ralf Baechle
 */
#ifndef _ASM_TIMEX_H
#define _ASM_TIMEX_H

#include <asm/mipsregs.h>

/*
 * This is the frequency of the timer used for Linux's timer interrupt.
 * The value should be defined as accurate as possible or under certain
 * circumstances Linux timekeeping might become inaccurate or fail.
 *
 * For many system the exact clockrate of the timer isn't known but due to
 * the way this value is used we can get away with a wrong value as long
 * as this value is:
 *
 *  - a multiple of HZ
 *  - a divisor of the actual rate
 *
 * 500000 is a good such cheat value.
 *
 * The obscure number 1193182 is the same as used by the original i8254
 * time in legacy PC hardware; the chip unfortunately also found in a
 * bunch of MIPS systems.  The last remaining user of the i8254 for the
 * timer interrupt is the RM200; it's a very standard system so there is
 * no reason to make this a separate architecture.
 */

#include <timex.h>

/*
 * Standard way to access the cycle counter.
 * Currently only used on SMP for scheduling.
 *
 * Only the low 32 bits are available as a continuously counting entity.
 * But this only means we'll force a reschedule every 8 seconds or so,
 * which isn't an evil thing.
 *
 * We know that all SMP capable CPUs have cycle counters.
 */

#ifdef CONFIG_CPU_CAVIUM_OCTEON
/* Since the Octeon supports a 64 bit cycle counter we might as well use it */
typedef unsigned long cycles_t;
#else
typedef unsigned int cycles_t;
#endif

static inline cycles_t get_cycles (void)
{
#ifdef CONFIG_CPU_CAVIUM_OCTEON
    cycles_t result;
    asm volatile ("rdhwr %0,$31" : "=r" (result));
    return result;
#else
	return read_c0_count();
#endif
}

#endif /*  _ASM_TIMEX_H */
