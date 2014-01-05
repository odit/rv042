/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2004 Cavium Networks
 */
#ifndef __ASM_MACH_CAVIUM_OCTEON_PARAM_OVERRIDES_H
#define __ASM_MACH_CAVIUM_OCTEON_PARAM_OVERRIDES_H

#ifdef CONFIG_CAVIUM_OCTEON_SIMULATOR
#define HZ 100
#else
#define HZ 1000
#endif

#define RTC_IRQ 88

#endif
