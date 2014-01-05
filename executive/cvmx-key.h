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
 * Interface to the on chip key memory. Key memory is
 * 8k on chip that is inaccessible from off chip. It can
 * also be cleared using an external hardware pin.
 *
 * $Id: cvmx-key.h 2 2007-04-05 08:51:12Z tt $
 *
 */

#ifndef __CVMX_KEY_H__
#define __CVMX_KEY_H__

#ifdef	__cplusplus
extern "C" {
#endif

#define CVMX_KEY_MEM_SIZE 8192  /* Size in bytes */


/**
 * Read from KEY memory
 *
 * @param address Address (byte) in key memory to read
 *                0 <= address < CVMX_KEY_MEM_SIZE
 * @return Value from key memory
 */
static inline uint64_t cvmx_key_read(uint64_t address)
{
    cvmx_addr_t ptr;

    ptr.u64 = 0;
    ptr.sio.mem_region  = CVMX_IO_SEG;
    ptr.sio.is_io       = 1;
    ptr.sio.did         = CVMX_OCT_DID_KEY_RW;
    ptr.sio.offset      = address;

    return cvmx_read_csr(ptr.u64);
}


/**
 * Write to KEY memory
 *
 * @param address Address (byte) in key memory to write
 *                0 <= address < CVMX_KEY_MEM_SIZE
 * @param value   Value to write to key memory
 */
static inline void cvmx_key_write(uint64_t address, uint64_t value)
{
    cvmx_addr_t ptr;

    ptr.u64 = 0;
    ptr.sio.mem_region  = CVMX_IO_SEG;
    ptr.sio.is_io       = 1;
    ptr.sio.did         = CVMX_OCT_DID_KEY_RW;
    ptr.sio.offset      = address;

    cvmx_write_csr(ptr.u64, value);
}


/* CSR typedefs have been moved to cvmx-csr-*.h */

#ifdef	__cplusplus
}
#endif

#endif /*  __CVMX_KEY_H__ */
