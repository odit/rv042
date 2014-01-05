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
 * Interface to the hardware Input Packet Data unit.
 *
 * File version info: $Id: cvmx-ipd.h 2 2007-04-05 08:51:12Z tt $ $Name$
 */


#ifndef __CVMX_IPD_H__
#define __CVMX_IPD_H__

#ifdef	__cplusplus
extern "C" {
#endif

#ifndef CVMX_ENABLE_LEN_M8_FIX
#define CVMX_ENABLE_LEN_M8_FIX 0
#endif

/* CSR typedefs have been moved to cvmx-csr-*.h */

typedef cvmx_ipd_mbuff_first_skip_t cvmx_ipd_mbuff_not_first_skip_t;
typedef cvmx_ipd_first_next_ptr_back_t cvmx_ipd_second_next_ptr_back_t;


/**
 * Configure IPD
 *
 * @param mbuff_size Packets buffer size in 8 byte words
 * @param first_mbuff_skip
 *                   Number of 8 byte words to skip in the first buffer
 * @param not_first_mbuff_skip
 *                   Number of 8 byte words to skip in each following buffer
 * @param first_back Must be same as first_mbuff_skip / 128
 * @param second_back
 *                   Must be same as not_first_mbuff_skip / 128
 * @param wqe_fpa_pool
 *                   FPA pool to get work entries from
 * @param cache_mode
 * @param back_pres_enable_flag
 *                   Enable or disable port back pressure
 */
static inline void cvmx_ipd_config(uint64_t mbuff_size,
                                   uint64_t first_mbuff_skip,
                                   uint64_t not_first_mbuff_skip,
                                   uint64_t first_back,
                                   uint64_t second_back,
                                   uint64_t wqe_fpa_pool,
                                   cvmx_ipd_mode_t cache_mode,
                                   uint64_t back_pres_enable_flag
                                  )
{
    cvmx_ipd_mbuff_first_skip_t first_skip;
    cvmx_ipd_mbuff_not_first_skip_t not_first_skip;
    cvmx_ipd_mbuff_size_t size;
    cvmx_ipd_first_next_ptr_back_t first_back_struct;
    cvmx_ipd_second_next_ptr_back_t second_back_struct;
    cvmx_ipd_wqe_fpa_pool_t wqe_pool;
    cvmx_ipd_ctl_status_t ipd_ctl_reg;

    first_skip.u64 = 0;
    first_skip.s.skip_sz = first_mbuff_skip;
    cvmx_write_csr(CVMX_IPD_1ST_MBUFF_SKIP, first_skip.u64);

    not_first_skip.u64 = 0;
    not_first_skip.s.skip_sz = not_first_mbuff_skip;
    cvmx_write_csr(CVMX_IPD_NOT_1ST_MBUFF_SKIP, not_first_skip.u64);

    size.u64 = 0;
    size.s.mb_size = mbuff_size;
    cvmx_write_csr(CVMX_IPD_PACKET_MBUFF_SIZE, size.u64);

    first_back_struct.u64 = 0;
    first_back_struct.s.back = first_back;
    cvmx_write_csr(CVMX_IPD_1st_NEXT_PTR_BACK, first_back_struct.u64);

    second_back_struct.u64 = 0;
    second_back_struct.s.back = second_back;
    cvmx_write_csr(CVMX_IPD_2nd_NEXT_PTR_BACK,second_back_struct.u64);

    wqe_pool.u64 = 0;
    wqe_pool.s.wqe_pool = wqe_fpa_pool;
    cvmx_write_csr(CVMX_IPD_WQE_FPA_QUEUE, wqe_pool.u64);

    ipd_ctl_reg.u64 = cvmx_read_csr(CVMX_IPD_CTL_STATUS);
    ipd_ctl_reg.s.opc_mode = cache_mode;
    ipd_ctl_reg.s.pbp_en = back_pres_enable_flag;
    cvmx_write_csr(CVMX_IPD_CTL_STATUS, ipd_ctl_reg.u64);

    /* Note: the example RED code that used to be here has been moved to
        cvmx_helper_setup_red */
}


/**
 * Enable IPD
 */
static inline void cvmx_ipd_enable(void)
{
    cvmx_ipd_ctl_status_t ipd_reg;
    ipd_reg.u64 = cvmx_read_csr(CVMX_IPD_CTL_STATUS);
    if (ipd_reg.s.ipd_en)
    {
        cvmx_dprintf("Warning: Enabling IPD when IPD already enabled.\n");
    }
    ipd_reg.s.ipd_en = TRUE;
    #if  CVMX_ENABLE_LEN_M8_FIX
    if(cvmx_get_proc_id() != OCTEON_CN38XX_PASS2) {
        ipd_reg.s.len_m8 = TRUE;
    }
    #endif
    cvmx_write_csr(CVMX_IPD_CTL_STATUS, ipd_reg.u64);
}


/**
 * Disable IPD
 */
static inline void cvmx_ipd_disable(void)
{
    cvmx_ipd_ctl_status_t ipd_reg;
    ipd_reg.u64 = cvmx_read_csr(CVMX_IPD_CTL_STATUS);
    ipd_reg.s.ipd_en = FALSE;
    cvmx_write_csr(CVMX_IPD_CTL_STATUS, ipd_reg.u64);
}

#ifdef	__cplusplus
}
#endif

#endif  /*  __CVMX_IPD_H__ */
