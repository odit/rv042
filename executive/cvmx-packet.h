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
 * Packet buffer defines.
 *
 * $Id: cvmx-packet.h 2 2007-04-05 08:51:12Z tt $ $Name$
 *
 *
 */

#ifndef __CVMX_PACKET_H__
#define __CVMX_PACKET_H__

#ifdef	__cplusplus
extern "C" {
#endif

/**
 * This structure defines a buffer pointer on Octeon
 */
typedef union
{
    void*           ptr;
    uint64_t        u64;
    struct
    {
        uint64_t    i    : 1; /**< if set, invert the "free" pick of the overall packet. HW always sets this bit to 0 on inbound packet */
        uint64_t    back : 4; /**< Indicates the amount to back up to get to the buffer start in cache lines. In most cases
                                this is less than one complete cache line, so the value is zero */
        uint64_t    pool : 3; /**< The pool that the buffer came from / goes to */
        uint64_t    size :16; /**< The size of the segment pointed to by addr (in bytes) */
        uint64_t    addr :40; /**< Pointer to the first byte of the data, NOT buffer */
    } s;
} cvmx_buf_ptr_t;

#ifdef	__cplusplus
}
#endif

#endif /*  __CVMX_PACKET_H__ */

