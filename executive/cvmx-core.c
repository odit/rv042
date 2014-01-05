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
 * $Id: cvmx-core.c 2 2007-04-05 08:51:12Z tt $ $Name$
 *
 */

#include <stdio.h>
#include "cvmx-config.h"
#include "cvmx.h"
#include "cvmx-asm.h"
#include "cvmx-core.h"


/**
 * Adds a wired TLB entry, and returns the index of the entry added.
 * Parameters are written to TLB registers without further processing.
 *
 * @param hi     HI register value
 * @param lo0    lo0 register value
 * @param lo1    lo1 register value
 * @param page_mask   pagemask register value
 *
 * @return Success: TLB index used (0-31)
 *         Failure: -1
 */
int cvmx_core_add_wired_tlb_entry(uint64_t hi, uint64_t lo0, uint64_t lo1, cvmx_tlb_pagemask_t page_mask)
{
    uint32_t index;

    CVMX_MF_TLB_WIRED(index);
    if (index >= 31)
    {
        return(-1);
    }
    CVMX_MT_ENTRY_HIGH(hi);
    CVMX_MT_ENTRY_LO_0(lo0);
    CVMX_MT_ENTRY_LO_1(lo1);
    CVMX_MT_PAGEMASK(page_mask);
    CVMX_MT_TLB_INDEX(index);
    CVMX_MT_TLB_WIRED(index + 1);
    CVMX_EHB;
    CVMX_TLBWI;
    CVMX_EHB;
    return(index);
}



/**
 * Adds a fixed (wired) TLB mapping.  Returns TLB index used or -1 on error.
 * This is a wrapper around cvmx_core_add_wired_tlb_entry()
 *
 * @param vaddr      Virtual address to map
 * @param page0_addr page 0 physical address, with low 3 bits representing the DIRTY, VALID, and GLOBAL bits
 * @param page1_addr page1 physical address, with low 3 bits representing the DIRTY, VALID, and GLOBAL bits
 * @param page_mask  page mask.
 *
 * @return Success: TLB index used (0-31)
 *         Failure: -1
 */
int cvmx_core_add_fixed_tlb_mapping_bits(uint64_t vaddr, uint64_t page0_addr, uint64_t page1_addr, cvmx_tlb_pagemask_t page_mask)
{

    if ((vaddr & (page_mask | 0x7fff))
        || ((page0_addr & ~0x7ULL) & ((page_mask | 0x7fff) >> 1))
        || ((page1_addr & ~0x7ULL) & ((page_mask | 0x7fff) >> 1)))
    {
        cvmx_dprintf("Error adding tlb mapping: invalid address alignment at vaddr: 0x%llx\n", (unsigned long long)vaddr);
        return(-1);
    }


    return(cvmx_core_add_wired_tlb_entry(vaddr,
                                         (page0_addr >> 6) | (page0_addr & 0x7),
                                         (page1_addr >> 6) | (page1_addr & 0x7),
                                         page_mask));

}
/**
 * Adds a fixed (wired) TLB mapping.  Returns TLB index used or -1 on error.
 * Assumes both pages are valid.  Use cvmx_core_add_fixed_tlb_mapping_bits for more control.
 * This is a wrapper around cvmx_core_add_wired_tlb_entry()
 *
 * @param vaddr      Virtual address to map
 * @param page0_addr page 0 physical address
 * @param page1_addr page1 physical address
 * @param page_mask  page mask.
 *
 * @return Success: TLB index used (0-31)
 *         Failure: -1
 */
int cvmx_core_add_fixed_tlb_mapping(uint64_t vaddr, uint64_t page0_addr, uint64_t page1_addr, cvmx_tlb_pagemask_t page_mask)
{

    return(cvmx_core_add_fixed_tlb_mapping_bits(vaddr, page0_addr | TLB_DIRTY | TLB_VALID | TLB_GLOBAL, page1_addr | TLB_DIRTY | TLB_VALID | TLB_GLOBAL, page_mask));

}
