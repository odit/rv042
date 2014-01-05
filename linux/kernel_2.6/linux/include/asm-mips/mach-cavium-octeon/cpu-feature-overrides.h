/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2004 Cavium Networks
 */
#ifndef __ASM_MACH_CAVIUM_OCTEON_CPU_FEATURE_OVERRIDES_H
#define __ASM_MACH_CAVIUM_OCTEON_CPU_FEATURE_OVERRIDES_H

/*
 * Cavium Octeons are MIPS64v2 processors
 */
#define cpu_dcache_line_size()	128
#define cpu_icache_line_size()	128

#ifdef CONFIG_SMP
#define cpu_has_llsc		1
#else
/* Disable LL/SC on non SMP systems. It is faster to disable interrupts
    for atomic access than a LL/SC */
#define cpu_has_llsc		0
#endif
#define cpu_has_prefetch  	1
#define cpu_has_dc_aliases  0
#define cpu_has_fpu         0
#define cpu_has_octeon_cache 1
/* We actually use two syncw instructions in a row when we need a write memory
    barrier. This is because the CN3XXX series of Octeons have errata
    Core-401. This can cause a single syncw to not enforce ordering under very
    rare conditions. Even if it is rare, better safe than sorry */
#define OCTEON_SYNCW_STR ".set push\n.set arch=octeon\nsyncw\nsyncw\n.set pop\n"

#define ARCH_HAS_READ_CURRENT_TIMER 1
#define ARCH_HAS_IRQ_PER_CPU 1

static inline unsigned long read_current_timer(unsigned long *result)
{
    asm volatile ("rdhwr %0,$31" : "=r" (*result));
    return *result;
}

static inline int octeon_is_pass1(void)
{
    uint32_t id;
    asm volatile ("mfc0 %0, $15,0" : "=r" (id));
    return (id == 0x000d0000);
}

#endif
