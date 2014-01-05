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
 * Configuration and status register (CSR) address and type definitions for
 * Octoen.
 *
 * $Id: cvmx-csr.h 2 2007-04-05 08:51:12Z tt $ $Name$
 *
 */
#ifndef __CVMX_CSR_H__
#define __CVMX_CSR_H__

#include "cvmx-csr-enums.h"
#include "cvmx-csr-addresses.h"
#include "cvmx-csr-typedefs.h"

/* These addresses are the same as in cvmx-csr-addresses.h, just with an index
    to allow easier access */
#define CVMX_IPD_QOSX_RED_MARKS(offset) (CVMX_IPD_QOS0_RED_MARKS+(offset)*8)
#define CVMX_IPD_RED_QUEX_PARAM(offset) (CVMX_IPD_RED_QUE0_PARAM+(offset)*8)

/* Map the HW names to the SDK historical names */
typedef cvmx_ciu_intx_en1_t             cvmx_ciu_int1_t;
typedef cvmx_ciu_intx_sum0_t            cvmx_ciu_intx0_t;
typedef cvmx_ciu_mbox_setx_t            cvmx_ciu_mbox_t;
typedef cvmx_fpa_fpfx_marks_t           cvmx_fpa_fpf_marks_t;
typedef cvmx_fpa_quex_page_index_t      cvmx_fpa_que0_page_index_t;
typedef cvmx_fpa_quex_page_index_t      cvmx_fpa_que1_page_index_t;
typedef cvmx_fpa_quex_page_index_t      cvmx_fpa_que2_page_index_t;
typedef cvmx_fpa_quex_page_index_t      cvmx_fpa_que3_page_index_t;
typedef cvmx_fpa_quex_page_index_t      cvmx_fpa_que4_page_index_t;
typedef cvmx_fpa_quex_page_index_t      cvmx_fpa_que5_page_index_t;
typedef cvmx_fpa_quex_page_index_t      cvmx_fpa_que6_page_index_t;
typedef cvmx_fpa_quex_page_index_t      cvmx_fpa_que7_page_index_t;
typedef cvmx_ipd_1st_mbuff_skip_t       cvmx_ipd_mbuff_first_skip_t;
typedef cvmx_ipd_1st_next_ptr_back_t    cvmx_ipd_first_next_ptr_back_t;
typedef cvmx_ipd_packet_mbuff_size_t    cvmx_ipd_mbuff_size_t;
typedef cvmx_ipd_qosx_red_marks_t       cvmx_ipd_qos_red_marks_t;
typedef cvmx_ipd_wqe_fpa_queue_t        cvmx_ipd_wqe_fpa_pool_t;
typedef cvmx_l2c_pfcx_t                 cvmx_l2c_pfc0_t;
typedef cvmx_l2c_pfcx_t                 cvmx_l2c_pfc1_t;
typedef cvmx_l2c_pfcx_t                 cvmx_l2c_pfc2_t;
typedef cvmx_l2c_pfcx_t                 cvmx_l2c_pfc3_t;
typedef cvmx_lmc_wodt_ctl_t             cvmx_lmc_odt_ctl_t;
typedef cvmx_npi_base_addr_inputx_t     cvmx_npi_base_addr_input_t;
typedef cvmx_npi_base_addr_outputx_t    cvmx_npi_base_addr_output_t;
typedef cvmx_npi_buff_size_outputx_t    cvmx_npi_buff_size_output_t;
typedef cvmx_npi_dma_highp_counts_t     cvmx_npi_dma_counts_t;
typedef cvmx_npi_dma_highp_naddr_t      cvmx_npi_dma_naddr_t;
typedef cvmx_npi_highp_dbell_t          cvmx_npi_dbell_t;
typedef cvmx_npi_highp_ibuff_saddr_t    cvmx_npi_dma_ibuff_saddr_t;
typedef cvmx_npi_mem_access_subidx_t    cvmx_npi_mem_access_subid_t;
typedef cvmx_npi_num_desc_outputx_t     cvmx_npi_num_desc_output_t;
typedef cvmx_npi_px_dbpair_addr_t       cvmx_npi_dbpair_addr_t;
typedef cvmx_npi_px_instr_addr_t        cvmx_npi_instr_addr_t;
typedef cvmx_npi_px_instr_cnts_t        cvmx_npi_instr_cnts_t;
typedef cvmx_npi_px_pair_cnts_t         cvmx_npi_pair_cnts_t;
typedef cvmx_npi_size_inputx_t          cvmx_npi_size_input_t;
typedef cvmx_pci_dbellx_t               cvmx_pci_dbell_t;
typedef cvmx_pci_dma_cntx_t             cvmx_pci_dma_cnt_t;
typedef cvmx_pci_dma_int_levx_t         cvmx_pci_dma_int_lev_t;
typedef cvmx_pci_dma_timex_t            cvmx_pci_dma_time_t;
typedef cvmx_pci_instr_countx_t         cvmx_pci_instr_count_t;
typedef cvmx_pci_pkt_creditsx_t         cvmx_pci_pkt_credits_t;
typedef cvmx_pci_pkts_sent_int_levx_t   cvmx_pci_pkts_sent_int_lev_t;
typedef cvmx_pci_pkts_sent_timex_t      cvmx_pci_pkts_sent_time_t;
typedef cvmx_pci_pkts_sentx_t           cvmx_pci_pkts_sent_t;
typedef cvmx_pip_prt_cfgx_t             cvmx_pip_port_cfg_t;
typedef cvmx_pip_prt_tagx_t             cvmx_pip_port_tag_cfg_t;
typedef cvmx_pip_qos_watchx_t           cvmx_pip_port_watcher_cfg_t;
typedef cvmx_pko_mem_queue_ptrs_t       cvmx_pko_queue_cfg_t;
typedef cvmx_pko_reg_cmd_buf_t          cvmx_pko_pool_cfg_t;
typedef cvmx_tim_reg_flags_t            cvmx_tim_control_t;

#endif /* __CVMX_CSR_H__ */

