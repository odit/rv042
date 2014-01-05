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
 * Module to support operations on core such as TLB config, etc.
 *
 * $Id: cvmx-core.h 2 2007-04-05 08:51:12Z tt $ $Name$
 *
 */


#ifndef __CVMX_CORE_H__
#define __CVMX_CORE_H__

#ifdef	__cplusplus
extern "C" {
#endif

typedef enum {
    CVMX_TLB_PAGEMASK_4K   = 0x3     << 11,
    CVMX_TLB_PAGEMASK_16K  = 0xF     << 11,
    CVMX_TLB_PAGEMASK_64K  = 0x3F    << 11,
    CVMX_TLB_PAGEMASK_256K = 0xFF    << 11,
    CVMX_TLB_PAGEMASK_1M   = 0x3FF   << 11,
    CVMX_TLB_PAGEMASK_4M   = 0xFFF   << 11,
    CVMX_TLB_PAGEMASK_16M  = 0x3FFF  << 11,
    CVMX_TLB_PAGEMASK_64M  = 0xFFFF  << 11,
    CVMX_TLB_PAGEMASK_256M = 0x3FFFF << 11,
} cvmx_tlb_pagemask_t;


int cvmx_core_add_wired_tlb_entry(uint64_t hi, uint64_t lo0, uint64_t lo1, cvmx_tlb_pagemask_t page_mask);


int cvmx_core_add_fixed_tlb_mapping(uint64_t vaddr, uint64_t page0_addr, uint64_t page1_addr, cvmx_tlb_pagemask_t page_mask);
int cvmx_core_add_fixed_tlb_mapping_bits(uint64_t vaddr, uint64_t page0_addr, uint64_t page1_addr, cvmx_tlb_pagemask_t page_mask);

#ifdef	__cplusplus
}
#endif

#endif /* __CVMX_CORE_H__ */
