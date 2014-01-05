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
 * This file defines macros for use in determining the current calling ABI.
 *
 * File version info: $Id: cvmx-abi.h 2 2007-04-05 08:51:12Z tt $ $Name$
*/

#ifndef __CVMX_ABI_H__
#define __CVMX_ABI_H__

#ifdef	__cplusplus
extern "C" {
#endif

/* Check for N32 ABI, defined for 32-bit Simple Exec applications
   and Linux N32 ABI.*/
#if (defined _ABIN32 && _MIPS_SIM == _ABIN32)
#define CVMX_ABI_N32
/* Check for N64 ABI, defined for 64-bit Linux toolchain. */
#elif (defined _ABI64 && _MIPS_SIM == _ABI64)
#define CVMX_ABI_N64
/* Check for O32 ABI, defined for Linux 032 ABI, not supported yet. */
#elif (defined _ABIO32 && _MIPS_SIM == _ABIO32)
#define CVMX_ABI_O32
/* Check for EABI ABI, defined for 64-bit Simple Exec applications. */
#else
#define CVMX_ABI_EABI
#endif

#ifndef __BYTE_ORDER
    #if defined(__BIG_ENDIAN) && !defined(__LITTLE_ENDIAN)
        #define __BYTE_ORDER __BIG_ENDIAN
    #elif !defined(__BIG_ENDIAN) && defined(__LITTLE_ENDIAN)
        #define __BYTE_ORDER __LITTLE_ENDIAN
    #elif !defined(__BIG_ENDIAN) && !defined(__LITTLE_ENDIAN)
        #define __BIG_ENDIAN 4321
        #define __BYTE_ORDER __BIG_ENDIAN
    #else
        #error Unable to determine Endian mode
    #endif
#endif

#ifdef	__cplusplus
}
#endif

#endif /* __CVMX_ABI_H__ */
