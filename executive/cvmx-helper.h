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
 * Helper functions for common, but complicated tasks.
 *
 * $Id: cvmx-helper.h 2 2007-04-05 08:51:12Z tt $ $Name$
 */

#ifndef __CVMX_HELPER_H__
#define __CVMX_HELPER_H__

#include "executive-config.h"
#if defined(CVMX_ENABLE_PKO_FUNCTIONS) || defined(CVMX_ENABLE_DFA_FUNCTIONS) || defined(CVMX_ENABLE_TIMER_FUNCTIONS) || defined(CVMX_ENABLE_HELPER_FUNCTIONS)
#include "cvmx-config.h"
#include "cvmx-pko.h"
#include "cvmx-pow.h"
#endif
#include "cvmx-wqe.h"

#ifdef	__cplusplus
extern "C" {
#endif

/**
 * Allocate memory and initialize the FPA pools using memory
 * from cvmx-bootmem. Specifying zero for the number of
 * buffers will cause that FPA pool to not be setup. This is
 * useful if you aren't using some of the hardware and want
 * to save memory. Use cvmx_helper_initialize_fpa instead of
 * this function directly.
 *
 * @param pip_pool Should always be CVMX_FPA_PACKET_POOL
 * @param pip_size Should always be CVMX_FPA_PACKET_POOL_SIZE
 * @param pip_buffers
 *                 Number of packet buffers.
 * @param wqe_pool Should always be CVMX_FPA_WQE_POOL
 * @param wqe_size Should always be CVMX_FPA_WQE_POOL_SIZE
 * @param wqe_entries
 *                 Number of work queue entries
 * @param pko_pool Should always be CVMX_FPA_OUTPUT_BUFFER_POOL
 * @param pko_size Should always be CVMX_FPA_OUTPUT_BUFFER_POOL_SIZE
 * @param pko_buffers
 *                 PKO Command buffers. You should at minimum have two per
 *                 each PKO queue.
 * @param tim_pool Should always be CVMX_FPA_TIMER_POOL
 * @param tim_size Should always be CVMX_FPA_TIMER_POOL_SIZE
 * @param tim_buffers
 *                 TIM ring buffer command queues. At least two per timer bucket
 *                 is recommened.
 * @param dfa_pool Should always be CVMX_FPA_DFA_POOL
 * @param dfa_size Should always be CVMX_FPA_DFA_POOL_SIZE
 * @param dfa_buffers
 *                 DFA command buffer. A relatively small (32 for example)
 *                 number should work.
 * @return Zero on success, non-zero if out of memory
 */
extern int cvmx_helper_initialize_fpa_internal(int pip_pool, int pip_size, int pip_buffers,
                                               int wqe_pool, int wqe_size, int wqe_entries,
                                               int pko_pool, int pko_size, int pko_buffers,
                                               int tim_pool, int tim_size, int tim_buffers,
                                               int dfa_pool, int dfa_size, int dfa_buffers);

/**
 * Initialize the PIP, IPD, and PKO hardware to support
 * simple priority based queues for the ethernet ports. Each
 * port is configured with a number of priority queues based
 * on CVMX_PKO_QUEUES_PER_PORT_* where each queue is lower
 * priority than the previous. Use cvmx_helper_initialize_packet_io_global
 * instead of this one directly.
 *
 * @return Zero on success, non-zero on failure
 */
extern int cvmx_helper_initialize_packet_io_internal_global(void);


/**
 * Configure the RGMII port to match the PHY auto negotiated
 * speed and duplex
 *
 * @param port   Port to configure (0-3,16-19)
 */
extern void cvmx_helper_rgmii_speed(int port);



/**
 * Configure the RGMII port to internal loopback mode
 *
 * @param port   Port to configure (0-3,16-19)
 */
void cvmx_helper_rgmii_internal_loopback(int port);
/**
 * This function needs to be called on all Octeon chips with
 * errata PKI-100.
 *
 * The Size field is 8 too large in WQE and next pointers
 *
 *  The Size field generated by IPD is 8 larger than it should
 *  be. The Size field is <55:40> of both:
 *      - WORD3 in the work queue entry, and
 *      - the next buffer pointer (which precedes the packet data
 *        in each buffer).
 *
 * @param work   Work queue entry to fix
 * @return Zero on success. Negative on failure
 */
extern int cvmx_helper_fix_ipd_packet_chain(cvmx_wqe_t *work);


/**
 * Debug routine to dump the packet structure to the console
 *
 * @param work   Work queue entry containing the packet to dump
 * @return
 */
extern int cvmx_helper_dump_packet(cvmx_wqe_t *work);


/**
 * Setup Random Early Drop on a specific input queue
 *
 * @param queue  Input queue to setup RED on (0-7)
 * @param pass_thresh
 *               Packets will begin slowly dropping when there are less than
 *               this many packet buffers free in FPA 0.
 * @param drop_thresh
 *               All incomming packets will be dropped when there are less
 *               than this many free packet buffers in FPA 0.
 * @return Zero on success. Negative on failure
 */
extern int cvmx_helper_setup_red_queue(int queue, int pass_thresh, int drop_thresh);


/**
 * Setup Random Early Drop to automatically begin dropping packets.
 *
 * @param pass_thresh
 *               Packets will begin slowly dropping when there are less than
 *               this many packet buffers free in FPA 0.
 * @param drop_thresh
 *               All incomming packets will be dropped when there are less
 *               than this many free packet buffers in FPA 0.
 * @return Zero on success. Negative on failure
 */
extern int cvmx_helper_setup_red(int pass_thresh, int drop_thresh);


/**
 * This function enables the IPD and also enables the packet interfaces.
 * The packet interfaces (RGMII and SPI) must be enabled after the
 * IPD.  This should be called by the user program after any additional
 * IPD configuration changes are made if CVMX_HELPER_ENABLE_IPD
 * is not set in the executive-config.h file.
 * 
 * @return 0 on success
 *         -1 on failure
 */
extern int cvmx_helper_ipd_and_packet_input_enable(void);


/**
 * Convert a CVS Name tag into a version string.
 *
 * @param cvs_name_str
 *               CVS Name tag to parse
 * @return Version string. Note this buffer is allocated statically
 *         and will be shared by all callers.
 */
extern const char *cvmx_helper_parse_version(const char *cvs_name_str);


/**
 * Get the version of the CVMX libraries.
 *
 * @return Version string. Note this buffer is allocated statically
 *         and will be shared by all callers.
 */
extern const char *cvmx_helper_get_version(void);


/**
 * Allocate memory and initialize the FPA pools using memory
 * from cvmx-bootmem. Sizes of each element in the pools is
 * controlled by the cvmx-config.h header file. Specifying
 * zero for any parameter will cause that FPA pool to not be
 * setup. This is useful if you aren't using some of the
 * hardware and want to save memory.
 *
 * @param packet_buffers
 *               Number of packet buffers to allocate
 * @param work_queue_entries
 *               Number of work queue entries
 * @param pko_buffers
 *               PKO Command buffers. You should at minimum have two per
 *               each PKO queue.
 * @param tim_buffers
 *               TIM ring buffer command queues. At least two per timer bucket
 *               is recommened.
 * @param dfa_buffers
 *               DFA command buffer. A relatively small (32 for example)
 *               number should work.
 * @return Zero on success, non-zero if out of memory
 */
static inline int cvmx_helper_initialize_fpa(int packet_buffers, int work_queue_entries, int pko_buffers, int tim_buffers, int dfa_buffers)
{
    return cvmx_helper_initialize_fpa_internal(
#if defined(CVMX_ENABLE_PKO_FUNCTIONS) || defined(CVMX_ENABLE_HELPER_FUNCTIONS)
        CVMX_FPA_PACKET_POOL,        CVMX_FPA_PACKET_POOL_SIZE,          packet_buffers,
#else
        -1, 0,  0,
#endif
#ifdef CVMX_ENABLE_HELPER_FUNCTIONS
        CVMX_FPA_WQE_POOL,           CVMX_FPA_WQE_POOL_SIZE,             work_queue_entries,
#else
        -1, 0,  0,
#endif
#ifdef CVMX_ENABLE_PKO_FUNCTIONS
        CVMX_FPA_OUTPUT_BUFFER_POOL, CVMX_FPA_OUTPUT_BUFFER_POOL_SIZE,   pko_buffers,
#else
        -1, 0,  0,
#endif
#ifdef CVMX_ENABLE_TIMER_FUNCTIONS
        CVMX_FPA_TIMER_POOL,         CVMX_FPA_TIMER_POOL_SIZE,           tim_buffers,
#else
        -1, 0,  0,
#endif
#ifdef CVMX_ENABLE_DFA_FUNCTIONS
        CVMX_FPA_DFA_POOL,           CVMX_FPA_DFA_POOL_SIZE,             dfa_buffers);
#else
        -1, 0,  0);
#endif
}


/**
 * Initialize the PIP, IPD, and PKO hardware to support
 * simple priority based queues for the ethernet ports. Each
 * port is configured with a number of priority queues based
 * on CVMX_PKO_QUEUES_PER_PORT_* where each queue is lower
 * priority than the previous.
 *
 * @return Zero on success, non-zero on failure
 */
#if defined(CVMX_ENABLE_HELPER_FUNCTIONS)
static inline int cvmx_helper_initialize_packet_io_global(void)
{
    cvmx_pko_initialize_global();
    return cvmx_helper_initialize_packet_io_internal_global();
}

/**
 * Does core local initialization for packet io
 *
 * @return Zero on success, non-zero on failure
 */
static inline int cvmx_helper_initialize_packet_io_local(void)
{
    return cvmx_pko_initialize_local();
}

/**
 * Free the packet buffers contained in a work queue entry.
 * The work queue entry is not freed.
 *
 * @param work   Work queue entry with packet to free
 */
static inline void cvmx_helper_free_packet_data(cvmx_wqe_t *work)
{
    uint64_t        number_buffers;
    cvmx_buf_ptr_t  buffer_ptr;
    cvmx_buf_ptr_t  next_buffer_ptr;
    uint64_t        start_of_buffer;

    buffer_ptr = work->packet_ptr;
    number_buffers = work->word2.s.bufs;

    while (number_buffers--)
    {
        /* Remember the back pointer is in cache lines, not 64bit words */
        start_of_buffer = ((buffer_ptr.s.addr >> 7) - buffer_ptr.s.back) << 7;
        /* Read pointer to next buffer before we free the current buffer. */
        next_buffer_ptr = *(cvmx_buf_ptr_t*)cvmx_phys_to_ptr(buffer_ptr.s.addr - 8);
        cvmx_fpa_free(cvmx_phys_to_ptr(start_of_buffer), buffer_ptr.s.pool, 0);
        buffer_ptr = next_buffer_ptr;
    }
}


/**
 * Returns the number of ports on the given interface.
 * The interface must be initialized before the port count
 * can be returned.
 * 
 * @param interface Which interface to return port count for.
 * 
 * @return Port count for interface
 *         -1 for uninitialized interface
 */
int cvmx_helper_ports_on_interface(int interface);
#endif  /* defined(CVMX_ENABLE_HELPER_FUNCTIONS) */

#ifdef	__cplusplus
}
#endif

#endif  /* __CVMX_HELPER_H__ */
