/*
 *  This program is free software; you can distribute it and/or modify it
 *  under the terms of the GNU General Public License (Version 2) as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 *  for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place - Suite 330, Boston MA 02111-1307, USA.
 *
 * Further private data for which no space exists in mips_fpu_soft_struct.
 * This should be subsumed into the mips_fpu_soft_struct structure as
 * defined in processor.h as soon as the absurd wired absolute assembler
 * offsets become dynamic at compile time.
 *
 * Kevin D. Kissell, kevink@mips.com and Carsten Langgaard, carstenl@mips.com
 * Copyright (C) 2000 MIPS Technologies, Inc.  All rights reserved.
 */
#ifndef _ASM_FPU_EMULATOR_H
#define _ASM_FPU_EMULATOR_H

#ifdef CONFIG_CPU_CAVIUM_OCTEON
/* On SMP systems the FPU emulator statistics cause huge amounts of cross CPU
    memory traffic that isn't really needed. Nobody actually has any way of
    reading these statistics, so we disable them on Octeon. Other processors
    might want to do the same. */
#undef MIPS_FPU_EMULATOR_STATS_ENABLED
#else
#define MIPS_FPU_EMULATOR_STATS_ENABLED 1
#endif

#ifdef MIPS_FPU_EMULATOR_STATS_ENABLED
struct mips_fpu_emulator_stats {
	unsigned int emulated;
	unsigned int loads;
	unsigned int stores;
	unsigned int cp1ops;
	unsigned int cp1xops;
	unsigned int errors;
};

extern struct mips_fpu_emulator_stats fpuemustats;
#define FPUEMUSTATS_INC(field) fpuemustats.field++
#else
#define FPUEMUSTATS_INC(field) do {} while (0)
#endif

#endif /* _ASM_FPU_EMULATOR_H */
