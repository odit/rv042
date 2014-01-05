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

/**
 * @file
 *
 * This file provides support for the processor local scratch memory.
 * Scratch memory is byte addressable - all addresses are byte addresses.
 *
 *
 * $Id: cvmx-scratch.h 2 2007-04-05 08:51:12Z tt $ $Name$
 *
 *
 */


#ifndef __CVMX_SCRATCH_H__
#define __CVMX_SCRATCH_H__

#ifdef	__cplusplus
extern "C" {
#endif

/* Note: This define must be a long, not a long long in order to compile
        without warnings for both 32bit and 64bit. */
#define CVMX_SCRATCH_BASE       (-32768l) /* 0xffffffffffff8000 */


/**
 * Reads an 8 bit value from the processor local scratchpad memory.
 *
 * @param address byte address to read from
 *
 * @return value read
 */
static inline uint8_t cvmx_scratch_read8(uint64_t address)
{
    return *CASTPTR(volatile uint8_t, CVMX_SCRATCH_BASE + address);
}
/**
 * Reads a 16 bit value from the processor local scratchpad memory.
 *
 * @param address byte address to read from
 *
 * @return value read
 */
static inline uint16_t cvmx_scratch_read16(uint64_t address)
{
    return *CASTPTR(volatile uint16_t, CVMX_SCRATCH_BASE + address);
}
/**
 * Reads a 32 bit value from the processor local scratchpad memory.
 *
 * @param address byte address to read from
 *
 * @return value read
 */
static inline uint32_t cvmx_scratch_read32(uint64_t address)
{
    return *CASTPTR(volatile uint32_t, CVMX_SCRATCH_BASE + address);
}
/**
 * Reads a 64 bit value from the processor local scratchpad memory.
 *
 * @param address byte address to read from
 *
 * @return value read
 */
static inline uint64_t cvmx_scratch_read64(uint64_t address)
{
    return *CASTPTR(volatile uint64_t, CVMX_SCRATCH_BASE + address);
}



/**
 * Writes an 8 bit value to the processor local scratchpad memory.
 *
 * @param address byte address to write to
 * @param value   value to write
 */
static inline void cvmx_scratch_write8(uint64_t address, uint64_t value)
{
    *CASTPTR(volatile uint8_t, CVMX_SCRATCH_BASE + address) = (uint8_t)value;
}
/**
 * Writes a 32 bit value to the processor local scratchpad memory.
 *
 * @param address byte address to write to
 * @param value   value to write
 */
static inline void cvmx_scratch_write16(uint64_t address, uint64_t value)
{
    *CASTPTR(volatile uint16_t, CVMX_SCRATCH_BASE + address) = (uint16_t)value;
}
/**
 * Writes a 16 bit value to the processor local scratchpad memory.
 *
 * @param address byte address to write to
 * @param value   value to write
 */
static inline void cvmx_scratch_write32(uint64_t address, uint64_t value)
{
    *CASTPTR(volatile uint32_t, CVMX_SCRATCH_BASE + address) = (uint32_t)value;
}
/**
 * Writes a 64 bit value to the processor local scratchpad memory.
 *
 * @param address byte address to write to
 * @param value   value to write
 */
static inline void cvmx_scratch_write64(uint64_t address, uint64_t value)
{
    *CASTPTR(volatile uint64_t, CVMX_SCRATCH_BASE + address) = value;
}

#ifdef	__cplusplus
}
#endif

#endif /* __CVMX_SCRATCH_H__ */
