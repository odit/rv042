/*****************************************************************************
 * Copyright (c) 2005, 2006 Cavium Networks. All rights reserved.
 *
 * This Software is the property of Cavium Networks. The Software and all
 * accompanying documentation are copyrighted. The Software made available here
 * constitutes the proprietary information of Cavium Networks. You agree to
 * take reasonable steps to prevent the disclosure, unauthorized use or
 * unauthorized distribution of the Software. You shall use this Software
 * solely with Cavium hardware.
 *
 * Except as expressly permitted in a separate Software License Agreement
 * between You and Cavium Networks, you shall not modify, decompile,
 * disassemble, extract, or otherwise reverse engineer this Software. You shall
 * not make any copy of the Software or its accompanying documentation, except
 * for copying incident to the ordinary and intended use of the Software and
 * the Underlying Program and except for the making of a single archival copy.
 *
 * This Software, including technical data, may be subject to U.S. export
 * control laws, including the U.S. Export Administration Act and its
 * associated regulations, and may be subject to export or import regulations
 * in other countries. You warrant that You will comply strictly in all
 * respects with all such regulations and acknowledge that you have the
 * responsibility to obtain licenses to export, re-export or import the
 * Software.
 *
 * TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS" AND
 * WITH ALL FAULTS AND CAVIUM MAKES NO PROMISES, REPRESENTATIONS OR WARRANTIES,
 * EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO THE
 * SOFTWARE, INCLUDING ITS CONDITION, ITS CONFORMITY TO ANY REPRESENTATION OR
 * DESCRIPTION, OR THE EXISTENCE OF ANY LATENT OR PATENT DEFECTS, AND CAVIUM
 * SPECIFICALLY DISCLAIMS ALL IMPLIED (IF ANY) WARRANTIES OF TITLE,
 * MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF
 * VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. THE ENTIRE RISK ARISING OUT OF USE OR
 * PERFORMANCE OF THE SOFTWARE LIES WITH YOU.
 *
 ****************************************************************************/

#include <stdio.h>

#include "cvmx-config.h"
#include "cvmx.h"
#include "cvmx-asm.h"
#include "cvmx-l2c.h"
#include "cvmx-spinlock.h"


/* This spinlock is used internally to ensure that only one core is performing
** certain L2 operations at a time.
**
** NOTE: This only protects calls from within a single application - if multiple applications
** or operating systems are running, then it is up to the user program to coordinate between them.
*/
CVMX_SHARED cvmx_spinlock_t l2c_spinlock;


static inline int l2_size_half()
{
    uint64_t val = cvmx_read_csr(CVMX_L2D_FUS3);
    return !!(val & (1ull << 34));
}
int cvmx_l2c_get_core_way_partition(uint32_t core)
{
    uint32_t    field;

    /* Validate the core number */
    if (core >= cvmx_octeon_num_cores())
        return -1;

    /* Use the lower two bits of the coreNumber to determine the bit offset
     * of the UMSK[] field in the L2C_SPAR register.
     */
    field = (core & 0x3) * 8;

    /* Return the UMSK[] field from the appropriate L2C_SPAR register based
     * on the coreNumber.
     */
    switch (core & 0xC)
    {
        case 0x0:
            return((cvmx_read_csr(CVMX_L2C_SPAR0) & (0xFF << field)) >> field);
        case 0x4:
            if (OCTEON_IS_MODEL(OCTEON_CN38XX))
                return((cvmx_read_csr(CVMX_L2C_SPAR1) & (0xFF << field)) >> field);
            else
                return 0;
        case 0x8:
            if (OCTEON_IS_MODEL(OCTEON_CN38XX))
                return((cvmx_read_csr(CVMX_L2C_SPAR2) & (0xFF << field)) >> field);
            else
                return 0;
        case 0xC:
            if (OCTEON_IS_MODEL(OCTEON_CN38XX))
                return((cvmx_read_csr(CVMX_L2C_SPAR3) & (0xFF << field)) >> field);
            else
                return 0;
    }
    return(0);
}

int cvmx_l2c_set_core_way_partition(uint32_t core, uint32_t mask)
{
    uint32_t    field;
    uint32_t valid_mask;
    if (OCTEON_IS_MODEL(OCTEON_CN38XX))
    {
        valid_mask = 0xff;
        if (l2_size_half())
            valid_mask = 0xf;
    }
    else if (l2_size_half())
        valid_mask = 0x3;

    valid_mask = 0xf;

    mask &= valid_mask;

    /* A UMSK setting which blocks all L2C Ways is an error. */
    if (mask == valid_mask)
        return -1;

    /* Validate the core number */
    if (core >= cvmx_octeon_num_cores())
        return -1;

    /* Check to make sure current mask & new mask don't block all ways */
    if (((mask | cvmx_l2c_get_core_way_partition(core)) & valid_mask) == valid_mask)
        return -1;


    /* Use the lower two bits of core to determine the bit offset of the
     * UMSK[] field in the L2C_SPAR register.
     */
    field = (core & 0x3) * 8;

    /* Assign the new mask setting to the UMSK[] field in the appropriate
     * L2C_SPAR register based on the core_num.
     *
     */
    switch (core & 0xC)
    {
        case 0x0:
            cvmx_write_csr(CVMX_L2C_SPAR0,
                           (cvmx_read_csr(CVMX_L2C_SPAR0) & ~(0xFF << field)) |
                           mask << field);
            break;
        case 0x4:
            if (OCTEON_IS_MODEL(OCTEON_CN38XX))
                cvmx_write_csr(CVMX_L2C_SPAR1,
                               (cvmx_read_csr(CVMX_L2C_SPAR1) & ~(0xFF << field)) |
                               mask << field);
            break;
        case 0x8:
            if (OCTEON_IS_MODEL(OCTEON_CN38XX))
                cvmx_write_csr(CVMX_L2C_SPAR2,
                               (cvmx_read_csr(CVMX_L2C_SPAR2) & ~(0xFF << field)) |
                               mask << field);
            break;
        case 0xC:
            if (OCTEON_IS_MODEL(OCTEON_CN38XX))
                cvmx_write_csr(CVMX_L2C_SPAR3,
                               (cvmx_read_csr(CVMX_L2C_SPAR3) & ~(0xFF << field)) |
                               mask << field);
            break;
    }
    return 0;
}


int cvmx_l2c_set_hw_way_partition(uint32_t mask)
{
    uint32_t valid_mask;

    if (OCTEON_IS_MODEL(OCTEON_CN38XX))
    {
        valid_mask = 0xff;
        if (l2_size_half())
            valid_mask = 0xf;
    }
    else if (l2_size_half())
        valid_mask = 0x3;

    valid_mask = 0xf;

    mask &= valid_mask;

    /* A UMSK setting which blocks all L2C Ways is an error. */
    if (mask == valid_mask)
        return -1;
    /* Check to make sure current mask & new mask don't block all ways */
    if (((mask | cvmx_l2c_get_hw_way_partition()) & valid_mask) == valid_mask)
        return -1;

    cvmx_write_csr(CVMX_L2C_SPAR4, (cvmx_read_csr(CVMX_L2C_SPAR4) & ~0xFF) | mask);
    return 0;
}

int cvmx_l2c_get_hw_way_partition(void)
{
    return(cvmx_read_csr(CVMX_L2C_SPAR4) & (0xFF));
}


void cvmx_l2c_config_perf(uint32_t counter, cvmx_l2c_event_t event,
                          uint32_t clear_on_read)
{   cvmx_l2c_pfctl_t pfctl;

    pfctl.u64 = cvmx_read_csr(CVMX_L2C_PFCTL);

    switch (counter)
    {
        case 0:
            pfctl.s.cnt0sel = event;
            pfctl.s.cnt0ena = 1;
            if (!cvmx_octeon_is_pass1())
                pfctl.s.cnt0rdclr = clear_on_read;
            break;
        case 1:
            pfctl.s.cnt1sel = event;
            pfctl.s.cnt1ena = 1;
            if (!cvmx_octeon_is_pass1())
                pfctl.s.cnt1rdclr = clear_on_read;
            break;
        case 2:
            pfctl.s.cnt2sel = event;
            pfctl.s.cnt2ena = 1;
            if (!cvmx_octeon_is_pass1())
                pfctl.s.cnt2rdclr = clear_on_read;
            break;
        case 3:
        default:
            pfctl.s.cnt3sel = event;
            pfctl.s.cnt3ena = 1;
            if (!cvmx_octeon_is_pass1())
                pfctl.s.cnt3rdclr = clear_on_read;
            break;
    }

    cvmx_write_csr(CVMX_L2C_PFCTL, pfctl.u64);
}

uint64_t cvmx_l2c_read_perf(uint32_t counter)
{
    switch (counter)
    {
        case 0:
            return(cvmx_read_csr(CVMX_L2C_PFC0));
        case 1:
            return(cvmx_read_csr(CVMX_L2C_PFC1));
        case 2:
            return(cvmx_read_csr(CVMX_L2C_PFC2));
        case 3:
        default:
            return(cvmx_read_csr(CVMX_L2C_PFC3));
    }
}



/**
 * Helper function use to fault in cache lines for L2 cache locking
 *
 * @param addr   Address of base of memory region to read into L2 cache
 * @param len    Length (in bytes) of region to fault in
 */
static void fault_in(uint64_t addr, int len)
{
    /* Adjust addr and length so we get all cache lines even for
    ** small ranges spanning two cache lines */
    len += addr & CVMX_CACHE_LINE_MASK;
    addr &= ~CVMX_CACHE_LINE_MASK;
    volatile char *ptr = cvmx_phys_to_ptr(addr);
    volatile char dummy;
    CVMX_DCACHE_INVALIDATE;  /* Invalidate L1 cache to make sure all loads result in data being in L2 */
    while (len > 0)
    {
        dummy += *ptr;
        len -= CVMX_CACHE_LINE_SIZE;
        ptr += CVMX_CACHE_LINE_SIZE;
    }
}

int cvmx_l2c_lock_line(uint64_t addr)
{
    int retval = 0;
    cvmx_l2c_dbg_t l2cdbg;
    l2cdbg.u64 = 0;
    cvmx_l2c_lckbase_t lckbase;
    lckbase.u64 = 0;
    cvmx_l2c_lckoff_t lckoff;
    lckoff.u64 = 0;

    cvmx_spinlock_lock(&l2c_spinlock);

    /* Clear l2t error bits if set */
    cvmx_l2t_err_t l2t_err;
    l2t_err.u64 = cvmx_read_csr(CVMX_L2T_ERR);
    l2t_err.s.lckerr = 1;
    l2t_err.s.lckerr2 = 1;
    cvmx_write_csr(CVMX_L2T_ERR, l2t_err.u64);

    addr &= ~CVMX_CACHE_LINE_MASK;

    /* Set this core as debug core */
    l2cdbg.s.ppnum = cvmx_get_core_num();
    CVMX_SYNC;
    cvmx_write_csr(CVMX_L2C_DBG, l2cdbg.u64);
    cvmx_read_csr(CVMX_L2C_DBG);

    lckoff.s.lck_offset = 0; /* Only lock 1 line at a time */
    cvmx_write_csr(CVMX_L2C_LCKOFF, lckoff.u64);
    cvmx_read_csr(CVMX_L2C_LCKOFF);

    if (((cvmx_l2c_cfg_t)(cvmx_read_csr(CVMX_L2C_CFG))).s.idxalias)
    {
        int alias_shift = CVMX_L2C_IDX_ADDR_SHIFT + 2 * CVMX_L2_SET_BITS - 1;
        uint64_t addr_tmp = addr ^ (addr & ((1 << alias_shift) - 1)) >> CVMX_L2_SET_BITS;
        lckbase.s.lck_base = addr_tmp >> 7;
    }
    else
    {
        lckbase.s.lck_base = addr >> 7;
    }

    lckbase.s.lck_ena = 1;
    cvmx_write_csr(CVMX_L2C_LCKBASE, lckbase.u64);
    cvmx_read_csr(CVMX_L2C_LCKBASE);    // Make sure it gets there

    fault_in(addr, CVMX_CACHE_LINE_SIZE);

    lckbase.s.lck_ena = 0;
    cvmx_write_csr(CVMX_L2C_LCKBASE, lckbase.u64);
    cvmx_read_csr(CVMX_L2C_LCKBASE);    // Make sure it gets there

    /* Stop being debug core */
    cvmx_write_csr(CVMX_L2C_DBG, 0);
    cvmx_read_csr(CVMX_L2C_DBG);

    l2t_err.u64 = cvmx_read_csr(CVMX_L2T_ERR);
    if (l2t_err.s.lckerr || l2t_err.s.lckerr2)
        retval = 1;  /* We were unable to lock the line */

    cvmx_spinlock_unlock(&l2c_spinlock);

    return(retval);
}


int cvmx_l2c_lock_mem_region(uint64_t start, uint64_t len)
{
    int retval = 0;

    /* Round start/end to cache line boundaries */
    len += start & CVMX_CACHE_LINE_MASK;
    start &= ~CVMX_CACHE_LINE_MASK;
    len = (len + CVMX_CACHE_LINE_MASK) & ~CVMX_CACHE_LINE_MASK;

    while (len)
    {
        retval += cvmx_l2c_lock_line(start);
        start += CVMX_CACHE_LINE_SIZE;
        len -= CVMX_CACHE_LINE_SIZE;
    }

    return(retval);
}


void cvmx_l2c_flush(void)
{
    uint64_t assoc, set;
    cvmx_l2c_dbg_t l2cdbg;

    cvmx_spinlock_lock(&l2c_spinlock);

    l2cdbg.u64 = 0;
    if (!OCTEON_IS_MODEL(OCTEON_CN30XX))
        l2cdbg.s.ppnum = cvmx_get_core_num();
    l2cdbg.s.finv = 1;
    for(set=0; set < CVMX_L2_SETS; set++)
    {
        for(assoc = 0; assoc < CVMX_L2_ASSOC; assoc++)
        {
            l2cdbg.s.set = assoc;
            /* Enter debug mode, and make sure all other writes complete before we
            ** enter debug mode */
            CVMX_SYNCW;
            cvmx_write_csr(CVMX_L2C_DBG, l2cdbg.u64);
            cvmx_read_csr(CVMX_L2C_DBG);

            CVMX_PREPARE_FOR_STORE (CVMX_ADD_SEG(CVMX_MIPS_SPACE_XKPHYS, set*CVMX_CACHE_LINE_SIZE), 0);
            CVMX_SYNCW; /* Push STF out to L2 */
            /* Exit debug mode */
            CVMX_SYNC;
            cvmx_write_csr(CVMX_L2C_DBG, 0);
            cvmx_read_csr(CVMX_L2C_DBG);
        }
    }

    cvmx_spinlock_unlock(&l2c_spinlock);
}


int cvmx_l2c_unlock_line(uint64_t address)
{
    uint64_t assoc;
    cvmx_l2c_tag_t tag;
    cvmx_l2c_dbg_t l2cdbg;

    uint32_t index = cvmx_l2c_address_to_index(address);

    cvmx_spinlock_lock(&l2c_spinlock);
    /* Compute portion of address that is stored in tag */
    uint32_t tag_addr = ((address >> CVMX_L2C_TAG_ADDR_ALIAS_SHIFT) & ((1 << CVMX_L2C_TAG_ADDR_ALIAS_SHIFT) - 1));
    for(assoc = 0; assoc < CVMX_L2_ASSOC; assoc++)
    {
        tag = cvmx_get_l2c_tag(assoc, index);

        if (tag.s.V && (tag.s.addr == tag_addr))
        {
            l2cdbg.u64 = 0;
            l2cdbg.s.ppnum = cvmx_get_core_num();
            l2cdbg.s.set = assoc;
            l2cdbg.s.finv = 1;

            CVMX_SYNC;
            cvmx_write_csr(CVMX_L2C_DBG, l2cdbg.u64); /* Enter debug mode */
            cvmx_read_csr(CVMX_L2C_DBG);

            CVMX_PREPARE_FOR_STORE (CVMX_ADD_SEG(CVMX_MIPS_SPACE_XKPHYS, address), 0);
            CVMX_SYNC;
            /* Exit debug mode */
            cvmx_write_csr(CVMX_L2C_DBG, 0);
            cvmx_read_csr(CVMX_L2C_DBG);
            cvmx_spinlock_unlock(&l2c_spinlock);
            return tag.s.L;
        }
    }
    cvmx_spinlock_unlock(&l2c_spinlock);
    return 0;
}

int cvmx_l2c_unlock_mem_region(uint64_t start, uint64_t len)
{
    int num_unlocked = 0;
    /* Round start/end to cache line boundaries */
    len += start & CVMX_CACHE_LINE_MASK;
    start &= ~CVMX_CACHE_LINE_MASK;
    len = (len + CVMX_CACHE_LINE_MASK) & ~CVMX_CACHE_LINE_MASK;
    while (len > 0)
    {
        num_unlocked += cvmx_l2c_unlock_line(start);
        start += CVMX_CACHE_LINE_SIZE;
        len -= CVMX_CACHE_LINE_SIZE;
    }

    return num_unlocked;
}


/* Internal l2c tag types.  These are converted to a generic structure
** that can be used on all chips */
typedef union
{
    uint64_t u64;
#if __BYTE_ORDER == __BIG_ENDIAN
    struct cvmx_l2c_tag_cn50xx
    {
	uint64_t reserved		: 40;
	uint64_t V			: 1;	// Line valid
	uint64_t D			: 1;	// Line dirty
	uint64_t L			: 1;	// Line locked
	uint64_t U			: 1;	// Use, LRU eviction
	uint64_t addr			: 20;	// Phys mem addr (33..14)
    } cn50xx;
    struct cvmx_l2c_tag_cn30xx
    {
	uint64_t reserved		: 41;
	uint64_t V			: 1;	// Line valid
	uint64_t D			: 1;	// Line dirty
	uint64_t L			: 1;	// Line locked
	uint64_t U			: 1;	// Use, LRU eviction
	uint64_t addr			: 19;	// Phys mem addr (33..15)
    } cn30xx;
    struct cvmx_l2c_tag_cn31xx
    {
	uint64_t reserved		: 42;
	uint64_t V			: 1;	// Line valid
	uint64_t D			: 1;	// Line dirty
	uint64_t L			: 1;	// Line locked
	uint64_t U			: 1;	// Use, LRU eviction
	uint64_t addr			: 18;	// Phys mem addr (33..16)
    } cn31xx;
    struct cvmx_l2c_tag_cn38xx
    {
	uint64_t reserved		: 43;
	uint64_t V			: 1;	// Line valid
	uint64_t D			: 1;	// Line dirty
	uint64_t L			: 1;	// Line locked
	uint64_t U			: 1;	// Use, LRU eviction
	uint64_t addr			: 17;	// Phys mem addr (33..17)
    } cn38xx;
#endif
} cvmx_internal_l2c_tag_t;


/* cvmx_get_l2c_tag_asm is intended for internal use only */
uint64_t cvmx_get_l2c_tag_asm(uint64_t association, uint64_t line);
cvmx_l2c_tag_t cvmx_get_l2c_tag(uint32_t association, uint32_t index)
{
    cvmx_internal_l2c_tag_t tmp_tag;
    cvmx_l2c_tag_t tag;

    tmp_tag.u64 = cvmx_get_l2c_tag_asm(association, (1ULL << 63) | (index << 7));

    tag.u64 = 0;
    /* Convert all tag structure types to generic version, as it can represent all models */
    if (OCTEON_IS_MODEL(OCTEON_CN38XX))
    {
        tag.s.V    = tmp_tag.cn38xx.V;
        tag.s.D    = tmp_tag.cn38xx.D;
        tag.s.L    = tmp_tag.cn38xx.L;
        tag.s.U    = tmp_tag.cn38xx.U;
        tag.s.addr = tmp_tag.cn38xx.addr;
    }
    else if (OCTEON_IS_MODEL(OCTEON_CN31XX))
    {
        tag.s.V    = tmp_tag.cn31xx.V;
        tag.s.D    = tmp_tag.cn31xx.D;
        tag.s.L    = tmp_tag.cn31xx.L;
        tag.s.U    = tmp_tag.cn31xx.U;
        tag.s.addr = tmp_tag.cn31xx.addr;
    }
    else if (OCTEON_IS_MODEL(OCTEON_CN30XX))
    {
        tag.s.V    = tmp_tag.cn30xx.V;
        tag.s.D    = tmp_tag.cn30xx.D;
        tag.s.L    = tmp_tag.cn30xx.L;
        tag.s.U    = tmp_tag.cn30xx.U;
        tag.s.addr = tmp_tag.cn30xx.addr;
    }
    else if (OCTEON_IS_MODEL(OCTEON_CN50XX))
    {
        tag.s.V    = tmp_tag.cn50xx.V;
        tag.s.D    = tmp_tag.cn50xx.D;
        tag.s.L    = tmp_tag.cn50xx.L;
        tag.s.U    = tmp_tag.cn50xx.U;
        tag.s.addr = tmp_tag.cn50xx.addr;
    }
    else
    {
        cvmx_dprintf("Unsupported OCTEON Model\n");
    }

    return tag;
}

uint32_t cvmx_l2c_address_to_index (uint64_t addr)
{
    uint64_t idx = addr >> CVMX_L2C_IDX_ADDR_SHIFT;
    cvmx_l2c_cfg_t l2c_cfg;
    l2c_cfg.u64 = cvmx_read_csr(CVMX_L2C_CFG);

    if (l2c_cfg.s.idxalias)
    {
        idx ^= ((addr & CVMX_L2C_ALIAS_MASK) >> CVMX_L2C_TAG_ADDR_ALIAS_SHIFT);
    }
    idx &= CVMX_L2C_IDX_MASK;
    return(idx);
}




int cvmx_l2c_get_cache_size_bytes(void)
{
    if (OCTEON_IS_MODEL(OCTEON_CN38XX))
    {
        if (cvmx_fuse_read(264))
            return(512 * 1024);
        else
            return(1024 * 1024);
    }
    else if (OCTEON_IS_MODEL(OCTEON_CN31XX))
        return(256 * 1024);
    else if (OCTEON_IS_MODEL(OCTEON_CN3020))
        return(128 * 1024);
    else if (OCTEON_IS_MODEL(OCTEON_CN3010))
        return(128 * 1024);
    else if (OCTEON_IS_MODEL(OCTEON_CN3005))
        return(64 * 1024);
    else if (OCTEON_IS_MODEL(OCTEON_CN58XX))
        return(2 * 1024 * 1024);
    else if (OCTEON_IS_MODEL(OCTEON_CN56XX))
        return(2 * 1024 * 1024);
    else if (OCTEON_IS_MODEL(OCTEON_CN50XX))
        return(128 * 1024);
    else
        return -1;

}
