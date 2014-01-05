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
 * Interface to the hardware Packet Order / Work unit.
 *
 * File version info: $Id: cvmx-pow.h 2 2007-04-05 08:51:12Z tt $ $Name$
 */

#ifndef __CVMX_POW_H__
#define __CVMX_POW_H__

#ifndef CONFIG_OCTEON_U_BOOT
#if defined(__KERNEL__) && defined(__linux__)
    #include <linux/kernel.h>
    #define printf printk
#else
    #include <stdio.h>
#endif
#endif

#include "cvmx-scratch.h"
#include "cvmx-wqe.h"

#ifdef	__cplusplus
extern "C" {
#endif

/**
 * Wait flag values for pow functions.
 */
typedef enum
{
    CVMX_POW_WAIT = 1,
    CVMX_POW_NO_WAIT = 0,
} cvmx_pow_wait_t;



/**
 *  POW tag operations.  These are used in the data stored to the POW.
 */
typedef enum
{
    CVMX_POW_TAG_OP_SWTAG = 0L,         /**< switch the tag (only) for this PP
                                            - the previous tag should be non-NULL in this case
                                            - tag switch response required
                                            - fields used: op, type, tag */
    CVMX_POW_TAG_OP_SWTAG_FULL = 1L,    /**< switch the tag for this PP, with full information
                                            - this should be used when the previous tag is NULL
                                            - tag switch response required
                                            - fields used: address, op, grp, type, tag */
    CVMX_POW_TAG_OP_SWTAG_DESCH = 2L,   /**< switch the tag (and/or group) for this PP and de-schedule
                                            - OK to keep the tag the same and only change the group
                                            - fields used: op, no_sched, grp, type, tag */
    CVMX_POW_TAG_OP_DESCH = 3L,         /**< just de-schedule
                                            - fields used: op, no_sched */
    CVMX_POW_TAG_OP_ADDWQ = 4L,         /**< create an entirely new work queue entry
                                            - fields used: address, op, qos, grp, type, tag */
    CVMX_POW_TAG_OP_UPDATE_WQP_GRP = 5L,/**< just update the work queue pointer and grp for this PP
                                            - fields used: address, op, grp */
    CVMX_POW_TAG_OP_SET_NSCHED = 6L,    /**< set the no_sched bit on the de-schedule list
                                            - does nothing if the selected entry is not on the de-schedule list
                                            - does nothing if the stored work queue pointer does not match the address field
                                            - fields used: address, index, op
                                            Before issuing a *_NSCHED operation, SW must guarantee that all
                                            prior deschedules and set/clr NSCHED operations are complete and all
                                            prior switches are complete. The hardware provides the opsdone bit
                                            and swdone bit for SW polling. After issuing a *_NSCHED operation,
                                            SW must guarantee that the set/clr NSCHED is complete before
                                            any subsequent operations. */
    CVMX_POW_TAG_OP_CLR_NSCHED = 7L,    /**< clears the no_sched bit on the de-schedule list
                                            - does nothing if the selected entry is not on the de-schedule list
                                            - does nothing if the stored work queue pointer does not match the address field
                                            - fields used: address, index, op
                                            Before issuing a *_NSCHED operation, SW must guarantee that all
                                            prior deschedules and set/clr NSCHED operations are complete and all
                                            prior switches are complete. The hardware provides the opsdone bit
                                            and swdone bit for SW polling. After issuing a *_NSCHED operation,
                                            SW must guarantee that the set/clr NSCHED is complete before
                                            any subsequent operations. */
    CVMX_POW_TAG_OP_NOP = 15L           /**< do nothing */
} cvmx_pow_tag_op_t;

/**
 * This structure defines the store data on a store to POW
 */
typedef union
{
    uint64_t u64;
    struct
    {
        uint64_t              no_sched  : 1; /**< don't reschedule this entry. no_sched is used for CVMX_POW_TAG_OP_SWTAG_DESCH and CVMX_POW_TAG_OP_DESCH */
        uint64_t                unused  : 2;
        uint64_t                 index  :13; /**< contains index of entry for a CVMX_POW_TAG_OP_*_NSCHED */
        cvmx_pow_tag_op_t          op   : 4; /**< the operation to perform */
        uint64_t                unused2 : 2;
        uint64_t                   qos  : 3; /**< the QOS level for the packet. qos is only used for CVMX_POW_TAG_OP_ADDWQ */
        uint64_t                   grp  : 4; /**< the group that the work queue entry will be scheduled to grp is used for CVMX_POW_TAG_OP_ADDWQ, CVMX_POW_TAG_OP_SWTAG_FULL, CVMX_POW_TAG_OP_SWTAG_DESCH, and CVMX_POW_TAG_OP_UPDATE_WQP_GRP */
        cvmx_pow_tag_type_t        type : 3; /**< the type of the tag. type is used for everything except CVMX_POW_TAG_OP_DESCH, CVMX_POW_TAG_OP_UPDATE_WQP_GRP, and CVMX_POW_TAG_OP_*_NSCHED */
        uint64_t                   tag  :32; /**< the actual tag. tag is used for everything except CVMX_POW_TAG_OP_DESCH, CVMX_POW_TAG_OP_UPDATE_WQP_GRP, and CVMX_POW_TAG_OP_*_NSCHED */
    } s;


} cvmx_pow_tag_req_t;

/**
 * This structure describes the address to load stuff from POW
 */
typedef union
{
    uint64_t u64;

    // address for new work request loads (did<2:0> == 0)
    struct
    {
        uint64_t                mem_region  :2;
        uint64_t                mbz  :13;
        uint64_t                is_io  : 1;    // must be one
        uint64_t                did    : 8;    // the ID of POW -- did<2:0> == 0 in this case
        uint64_t                unaddr : 4;
        uint64_t                unused :32;
        uint64_t                wait   : 1;    // if set, don't return load response until work is available
        uint64_t                mbzl   : 3;    // must be zero
    } swork; // physical address


    // address for NULL_RD request (did<2:0> == 4)
    // when this is read, HW attempts to change the state to NULL if it is NULL_NULL
    // (the hardware cannot switch from NULL_NULL to NULL if a POW entry is not available -
    // software may need to recover by finishing another piece of work before a POW
    // entry can ever become available.)
    struct
    {
        uint64_t                mem_region  :2;
        uint64_t                mbz  :13;
        uint64_t                is_io  : 1;    // must be one
        uint64_t                did    : 8;    // the ID of POW -- did<2:0> == 4 in this case
        uint64_t                unaddr : 4;
        uint64_t                unused :33;
        uint64_t                mbzl   : 3;    // must be zero
    } snull_rd; // physical address

    // address for CSR accesses
    struct
    {
        uint64_t                mem_region  :2;
        uint64_t                mbz  :13;
        uint64_t                is_io  : 1;    // must be one
        uint64_t                did    : 8;    // the ID of POW -- did<2:0> == 7 in this case
        uint64_t                unaddr : 4;
        uint64_t                csraddr:36;    // only 36 bits in O1, addr<2:0> must be zero
    } stagcsr; // physical address

} cvmx_pow_load_addr_t;


/**
 * This structure defines the response to a load/SENDSINGLE to POW (except CSR reads)
 */
typedef union
{
    uint64_t         u64;

    cvmx_wqe_t *wqp;

    // response to new work request loads
    struct
    {
        uint64_t       no_work : 1;   // set when no new work queue entry was returned
        // If there was de-scheduled work, the HW will definitely
        // return it. When this bit is set, it could mean
        // either mean:
        //   - There was no work, or
        //   - There was no work that the HW could find. This
        //     case can happen, regardless of the wait bit value
        //     in the original request, when there is work
        //     in the IQ's that is too deep down the list.
        uint64_t       unused  : 23;
        uint64_t       addr    : 40;  // 36 in O1 -- the work queue pointer
    } s_work;

    // response to NULL_RD request loads
    struct
    {
        uint64_t       unused  : 62;
        uint64_t       state    : 2;  // of type cvmx_pow_tag_type_t
        // state is one of the following:
        //       CVMX_POW_TAG_TYPE_ORDERED
        //       CVMX_POW_TAG_TYPE_ATOMIC
        //       CVMX_POW_TAG_TYPE_NULL
        //       CVMX_POW_TAG_TYPE_NULL_NULL
    } s_null_rd;

} cvmx_pow_tag_load_resp_t;





/**
 * This structure describes the address used for stores to the POW.
 *  The store address is meaningful on stores to the POW.  The hardware assumes that an aligned
 *  64-bit store was used for all these stores.
 *  Note the assumption that the work queue entry is aligned on an 8-byte
 *  boundary (since the low-order 3 address bits must be zero).
 *  Note that not all fields are used by all operations.
 *
 *  NOTE: The following is the behavior of the pending switch bit at the PP
 *       for POW stores (i.e. when did<7:3> == 0xc)
 *     - did<2:0> == 0      => pending switch bit is set
 *     - did<2:0> == 1      => no affect on the pending switch bit
 *     - did<2:0> == 3      => pending switch bit is cleared
 *     - did<2:0> == 7      => no affect on the pending switch bit
 *     - did<2:0> == others => must not be used
 *     - No other loads/stores have an affect on the pending switch bit
 *     - The switch bus from POW can clear the pending switch bit
 *
 *  NOTE: did<2:0> == 2 is used by the HW for a special single-cycle ADDWQ command
 *  that only contains the pointer). SW must never use did<2:0> == 2.
 */
typedef union
{
    /**
     * Unsigned 64 bit integer representation of store address
     */
    uint64_t u64;

    struct
    {

        /**
         * Memory region.  Should be CVMX_IO_SEG in most cases
         */
        uint64_t                mem_reg  :2;
        /**
         * must be zero
         */
        uint64_t                mbz  :13;
        /**
         * Selects I/O space, must be 1
         */
        uint64_t                is_io : 1;
        /**
         * Device ID of POW.  Note that different sub-dids are used.
         */
        uint64_t                did   : 8;
        /**
         * unused address bits, must be zero
         */
        uint64_t                unaddr:4;
        /**
         * Address field. addr<2:0> must be zero
         */
        uint64_t                addr  :36;
    } stag;
} cvmx_pow_tag_store_addr_t;



/**
 * decode of the store data when an IOBDMA SENDSINGLE is sent to POW
 */
typedef union
{
    uint64_t         u64;

    struct
    {
        uint64_t                scraddr : 8;    /**< the (64-bit word) location in scratchpad to write to (if len != 0) */
        uint64_t                len     : 8;    /**< the number of words in the response (0 => no response) */
        uint64_t                did     : 8;    /**< the ID of the device on the non-coherent bus */
        uint64_t                unused  :36;
        uint64_t                wait    : 1;    /**< if set, don't return load response until work is available */
        uint64_t                unused2 : 3;
    } s;

} cvmx_pow_iobdma_store_t;


/* CSR typedefs have been moved to cvmx-csr-*.h */


/**
 * Waits for a tag switch to complete by polling the completion bit.
 * Note that switches to NULL complete immediately and do not need
 * to be waited for.
 */
static inline void cvmx_pow_tag_sw_wait(void)
{
    uint64_t switch_complete;
    do
    {
        CVMX_MF_CHORD(switch_complete);
    } while (cvmx_likely(!switch_complete));
}


/**
 * Synchronous work request.  Requests work from the POW.
 * This function does NOT wait for previous tag switches to complete,
 * so the caller must ensure that there is not a pending tag switch.
 *
 * @param wait   When set, call stalls until work becomes avaiable, or times out.
 *               If not set, returns immediately.
 *
 * @return Returns the WQE pointer from POW. Returns NULL if no work was available.
 */
static inline cvmx_wqe_t * cvmx_pow_work_request_sync_nocheck(cvmx_pow_wait_t wait)
{
    cvmx_pow_load_addr_t ptr;
    cvmx_pow_tag_load_resp_t result;

    ptr.u64 = 0;
    ptr.swork.mem_region = CVMX_IO_SEG;
    ptr.swork.is_io = 1;
    ptr.swork.did = CVMX_OCT_DID_TAG_SWTAG;
    ptr.swork.wait = wait;

    result.u64 = cvmx_read_csr(ptr.u64);

    if (result.s_work.no_work)
        return NULL;
    else
        return (cvmx_wqe_t*)cvmx_phys_to_ptr(result.s_work.addr);
}
/**
 * Synchronous work request.  Requests work from the POW.
 * This function waits for any previous tag switch to complete before
 * requesting the new work.
 *
 * @param wait   When set, call stalls until work becomes avaiable, or times out.
 *               If not set, returns immediately.
 *
 * @return Returns the WQE pointer from POW. Returns NULL if no work was available.
 */
static inline cvmx_wqe_t * cvmx_pow_work_request_sync(cvmx_pow_wait_t wait)
{
    /* Must not have a switch pending when requesting work */
    cvmx_pow_tag_sw_wait();
    return(cvmx_pow_work_request_sync_nocheck(wait));

}

/**
 * Synchronous null_rd request.  Requests a switch out of NULL_NULL POW state.
 * This function waits for any previous tag switch to complete before
 * requesting the null_rd.
 *
 * @return Returns the POW state of type cvmx_pow_tag_type_t.
 */
static inline cvmx_pow_tag_type_t cvmx_pow_work_request_null_rd(void)
{
    cvmx_pow_load_addr_t ptr;
    cvmx_pow_tag_load_resp_t result;

    /* Must not have a switch pending when requesting work */
    cvmx_pow_tag_sw_wait();

    ptr.u64 = 0;
    ptr.snull_rd.mem_region = CVMX_IO_SEG;
    ptr.snull_rd.is_io = 1;
    ptr.snull_rd.did = CVMX_OCT_DID_TAG_NULL_RD;

    result.u64 = cvmx_read_csr(ptr.u64);

    return (cvmx_pow_tag_type_t)result.s_null_rd.state;
}

/**
 * Asynchronous work request.  Work is requested from the POW unit, and should later
 * be checked with function cvmx_pow_work_response_async.
 * This function does NOT wait for previous tag switches to complete,
 * so the caller must ensure that there is not a pending tag switch.
 *
 * @param scr_addr Scratch memory address that response will be returned to,
 *                  which is either a valid WQE, or a response with the invalid bit set.
 *                  Byte address, must be 8 byte aligned.
 * @param wait      1 to cause response to wait for work to become available (or timeout)
 *                  0 to cause response to return immediately
 */
static inline void cvmx_pow_work_request_async_nocheck(int scr_addr, cvmx_pow_wait_t wait)
{
    cvmx_pow_iobdma_store_t data;

    /* scr_addr must be 8 byte aligned */
    data.s.scraddr = scr_addr >> 3;
    data.s.len = 1;
    data.s.did = CVMX_OCT_DID_TAG_SWTAG;
    data.s.wait = wait;
    cvmx_send_single(data.u64);
}
/**
 * Asynchronous work request.  Work is requested from the POW unit, and should later
 * be checked with function cvmx_pow_work_response_async.
 * This function waits for any previous tag switch to complete before
 * requesting the new work.
 *
 * @param scr_addr Scratch memory address that response will be returned to,
 *                  which is either a valid WQE, or a response with the invalid bit set.
 *                  Byte address, must be 8 byte aligned.
 * @param wait      1 to cause response to wait for work to become available (or timeout)
 *                  0 to cause response to return immediately
 */
static inline void cvmx_pow_work_request_async(int scr_addr, cvmx_pow_wait_t wait)
{
    /* Must not have a switch pending when requesting work */
    cvmx_pow_tag_sw_wait();
    cvmx_pow_work_request_async_nocheck(scr_addr, wait);
}

/**
 * Gets result of asynchronous work request.  Performs a IOBDMA sync
 * to wait for the response.
 *
 * @param scr_addr Scratch memory address to get result from
 *                  Byte address, must be 8 byte aligned.
 * @return Returns the WQE from the scratch register, or NULL if no work was available.
 */
static inline cvmx_wqe_t * cvmx_pow_work_response_async(int scr_addr)
{
    cvmx_pow_tag_load_resp_t result;

    CVMX_SYNCIOBDMA;
    result.u64 = cvmx_scratch_read64(scr_addr);

    if (result.s_work.no_work)
        return NULL;
    else
        return (cvmx_wqe_t*)cvmx_phys_to_ptr(result.s_work.addr);
}


/**
 * Checks if a work queue entry pointer returned by a work
 * request is valid.  It may be invalid due to no work
 * being available or due to a timeout.
 *
 * @param wqe_ptr pointer to a work queue entry returned by the POW
 *
 * @return 0 if pointer is valid
 *         1 if invalid (no work was returned)
 */
static inline uint64_t cvmx_pow_work_invalid(cvmx_wqe_t *wqe_ptr)
{
    return (wqe_ptr == NULL);
}



/**
 * Starts a tag switch to the provided tag value and tag type.  Completion for
 * the tag switch must be checked for separately.
 * This function does NOT update the
 * work queue entry in dram to match tag value and type, so the application must
 * keep track of these if they are important to the application.
 * This tag switch command must not be used for switches to NULL, as the tag
 * switch pending bit will be set by the switch request, but never cleared by the
 * hardware.
 *
 * NOTE: This should not be used when switching from a NULL tag.  Use
 * cvmx_pow_tag_sw_full() instead.
 *
 * This function does no checks, so the caller must ensure that any previous tag
 * switch has completed.
 *
 * @param tag      new tag value
 * @param tag_type new tag type (ordered or atomic)
 */
static inline void cvmx_pow_tag_sw_nocheck(uint32_t tag, cvmx_pow_tag_type_t tag_type)
{
    cvmx_addr_t ptr;
    cvmx_pow_tag_req_t tag_req;

    /* Note that WQE in DRAM is not updated here, as the POW does not read from DRAM
    ** once the WQE is in flight.  See hardware manual for complete details.
    ** It is the application's responsibility to keep track of the current tag
    ** value if that is important.
    */


    tag_req.u64 = 0;
    tag_req.s.op = CVMX_POW_TAG_OP_SWTAG;
    tag_req.s.tag = tag;
    tag_req.s.type = tag_type;

    ptr.u64 = 0;
    ptr.sio.mem_region = CVMX_IO_SEG;
    ptr.sio.is_io = 1;
    ptr.sio.did = CVMX_OCT_DID_TAG_SWTAG;

    /* once this store arrives at POW, it will attempt the switch
       software must wait for the switch to complete separately */
    cvmx_write_io(ptr.u64, tag_req.u64);
}

/**
 * Starts a tag switch to the provided tag value and tag type.  Completion for
 * the tag switch must be checked for separately.
 * This function does NOT update the
 * work queue entry in dram to match tag value and type, so the application must
 * keep track of these if they are important to the application.
 * This tag switch command must not be used for switches to NULL, as the tag
 * switch pending bit will be set by the switch request, but never cleared by the
 * hardware.
 *
 * NOTE: This should not be used when switching from a NULL tag.  Use
 * cvmx_pow_tag_sw_full() instead.
 *
 * This function waits for any previous tag switch to complete, and also
 * displays an error on tag switches to NULL.
 *
 * @param tag      new tag value
 * @param tag_type new tag type (ordered or atomic)
 */
static inline void cvmx_pow_tag_sw(uint32_t tag, cvmx_pow_tag_type_t tag_type)
{

    /* Note that WQE in DRAM is not updated here, as the POW does not read from DRAM
    ** once the WQE is in flight.  See hardware manual for complete details.
    ** It is the application's responsibility to keep track of the current tag
    ** value if that is important.
    */


    /* This tag switch command must not be used for switches to null, as the tag
    ** switch pending bit will be set by the switch request, but never cleared by the
    ** hardware.
    ** cvmx_pow_tag_sw_null() should be used for switching to null tags.
    */
    if (tag_type == CVMX_POW_TAG_TYPE_NULL)
    {
        cvmx_dprintf("cvmx_pow_tag_sw: tag switch to NULL requested. Use cvmx_pow_tag_sw_null instead\n");
        CVMX_SYNC;
    }

    /* Ensure that there is not a pending tag switch, as a tag switch cannot be started
    ** if a previous switch is still pending.  */
    cvmx_pow_tag_sw_wait();

    cvmx_pow_tag_sw_nocheck(tag, tag_type);

}


/**
 * Starts a tag switch to the provided tag value and tag type.  Completion for
 * the tag switch must be checked for separately.
 * This function does NOT update the
 * work queue entry in dram to match tag value and type, so the application must
 * keep track of these if they are important to the application.
 * This tag switch command must not be used for switches to NULL, as the tag
 * switch pending bit will be set by the switch request, but never cleared by the
 * hardware.
 *
 * This function must be used for tag switches from NULL.
 *
 * This function does no checks, so the caller must ensure that any previous tag
 * switch has completed.
 *
 * @param wqp      pointer to work queue entry to submit.  This entry is updated to match the other parameters
 * @param tag      tag value to be assigned to work queue entry
 * @param tag_type type of tag
 * @param group      group value for the work queue entry.
 */
static inline void cvmx_pow_tag_sw_full_nocheck(cvmx_wqe_t *wqp, uint32_t tag, cvmx_pow_tag_type_t tag_type, uint64_t group)
{
    cvmx_addr_t ptr;
    cvmx_pow_tag_req_t tag_req;

    /* Note that WQE in DRAM is not updated here, as the POW does not read from DRAM
    ** once the WQE is in flight.  See hardware manual for complete details.
    ** It is the application's responsibility to keep track of the current tag
    ** value if that is important.
    */

    tag_req.u64 = 0;
    tag_req.s.op = CVMX_POW_TAG_OP_SWTAG_FULL;
    tag_req.s.tag = tag;
    tag_req.s.type = tag_type;
    tag_req.s.grp = group;

    ptr.u64 = 0;
    ptr.sio.mem_region = CVMX_IO_SEG;
    ptr.sio.is_io = 1;
    ptr.sio.did = CVMX_OCT_DID_TAG_SWTAG;
    ptr.sio.offset = CAST64(wqp);

    /* once this store arrives at POW, it will attempt the switch
       software must wait for the switch to complete separately */
    cvmx_write_io(ptr.u64, tag_req.u64);
}

/**
 * Starts a tag switch to the provided tag value and tag type.  Completion for
 * the tag switch must be checked for separately.
 * This function does NOT update the
 * work queue entry in dram to match tag value and type, so the application must
 * keep track of these if they are important to the application.
 * This tag switch command must not be used for switches to NULL, as the tag
 * switch pending bit will be set by the switch request, but never cleared by the
 * hardware.
 *
 * This function must be used for tag switches from NULL.
 *
 * This function waits for any pending tag switches to complete
 * before requesting the tag switch.
 *
 * @param wqp      pointer to work queue entry to submit.  This entry is updated to match the other parameters
 * @param tag      tag value to be assigned to work queue entry
 * @param tag_type type of tag
 * @param group      group value for the work queue entry.
 */
static inline void cvmx_pow_tag_sw_full(cvmx_wqe_t *wqp, uint32_t tag, cvmx_pow_tag_type_t tag_type, uint64_t group)
{
    /* Ensure that there is not a pending tag switch, as a tag switch cannot be started
    ** if a previous switch is still pending.  */
    cvmx_pow_tag_sw_wait();
    cvmx_pow_tag_sw_full_nocheck(wqp, tag, tag_type, group);
}







/**
 * Switch to a NULL tag, which ends any ordering or
 * synchronization provided by the POW for the current
 * work queue entry.  This operation completes immediatly,
 * so completetion should not be waited for.
 * This function does NOT wait for previous tag switches to complete,
 * so the caller must ensure that any previous tag switches have completed.
 */
static inline void cvmx_pow_tag_sw_null_nocheck(void)
{
    cvmx_addr_t ptr;
    cvmx_pow_tag_req_t tag_req;

    tag_req.u64 = 0;
    tag_req.s.op = CVMX_POW_TAG_OP_SWTAG;
    tag_req.s.type = CVMX_POW_TAG_TYPE_NULL;


    ptr.u64 = 0;
    ptr.sio.mem_region = CVMX_IO_SEG;
    ptr.sio.is_io = 1;
    ptr.sio.did = CVMX_OCT_DID_TAG_TAG1;


    cvmx_write_io(ptr.u64, tag_req.u64);

    /* switch to NULL completes immediately */
}

/**
 * Switch to a NULL tag, which ends any ordering or
 * synchronization provided by the POW for the current
 * work queue entry.  This operation completes immediatly,
 * so completetion should not be waited for.
 * This function waits for any pending tag switches to complete
 * before requesting the switch to NULL.
 */
static inline void cvmx_pow_tag_sw_null(void)
{
    /* Ensure that there is not a pending tag switch, as a tag switch cannot be started
    ** if a previous switch is still pending.  */
    cvmx_pow_tag_sw_wait();
    cvmx_pow_tag_sw_null_nocheck();

    /* switch to NULL completes immediately */
}



/**
 * Submits work to an input queue.  This function updates the work queue entry in DRAM to match
 * the arguments given.
 * Note that the tag provided is for the work queue entry submitted, and is unrelated to the tag that
 * the core currently holds.
 *
 * @param wqp      pointer to work queue entry to submit.  This entry is updated to match the other parameters
 * @param tag      tag value to be assigned to work queue entry
 * @param tag_type type of tag
 * @param qos      Input queue to add to.
 * @param grp      group value for the work queue entry.
 */
static inline void cvmx_pow_work_submit(cvmx_wqe_t *wqp, uint32_t tag, cvmx_pow_tag_type_t tag_type, uint64_t qos, uint64_t grp)
{
    cvmx_addr_t ptr;
    cvmx_pow_tag_req_t tag_req;

    wqp->qos = qos;
    wqp->tag = tag;
    wqp->tag_type = tag_type;
    wqp->grp = grp;

    tag_req.u64 = 0;
    tag_req.s.op = CVMX_POW_TAG_OP_ADDWQ;
    tag_req.s.type = tag_type;
    tag_req.s.tag = tag;
    tag_req.s.qos = qos;
    tag_req.s.grp = grp;


    ptr.u64 = 0;
    ptr.sio.mem_region = CVMX_IO_SEG;
    ptr.sio.is_io = 1;
    ptr.sio.did = CVMX_OCT_DID_TAG_TAG1;
    ptr.sio.offset = cvmx_ptr_to_phys(wqp);

    /* SYNC write to memory before the work submit.  This is necessary
    ** as POW may read values from DRAM at this time */
    CVMX_SYNCWS;
    cvmx_write_io(ptr.u64, tag_req.u64);
}



/**
 * This function sets the group mask for a core.  The group mask
 * indicates which groups each core will accept work from. There are
 * 16 groups.
 *
 * @param core_num   core to apply mask to
 * @param mask   Group mask. There are 16 groups, so only bits 0-15 are valid,
 *               representing groups 0-15.
 *               Each 1 bit in the mask enables the core to accept work from
 *               the corresponding group.
 */
static inline void cvmx_pow_set_group_mask(uint64_t core_num, uint64_t mask)
{
    cvmx_addr_t ptr;

    ptr.u64 = 0;
    ptr.sio.mem_region = CVMX_IO_SEG;
    ptr.sio.is_io = 1;
    ptr.sio.did = CVMX_OCT_DID_TAG_CSR;

    /* grp_msk happens to be at address 0 and different PP's are offset by 8 bytes */
    cvmx_write_io(ptr.u64 + core_num * 8, mask);
}



/**
 * Performs a tag switch and then an immediate deschedule. This completes
 * immediatly, so completion must not be waited for.  This function does NOT
 * update the wqe in DRAM to match arguments.
 *
 * This function does NOT wait for any prior tag switches to complete, so the
 * calling code must do this.
 *
 * Note the following CAVEAT of the Octeon HW behavior when
 * re-scheduling DE-SCHEDULEd items whose (next) state is
 * ORDERED:
 *   - If there are no switches pending at the time that the
 *     HW executes the de-schedule, the HW will only re-schedule
 *     the head of the FIFO associated with the given tag. This
 *     means that in many respects, the HW treats this ORDERED
 *     tag as an ATOMIC tag. Note that in the SWTAG_DESCH
 *     case (to an ORDERED tag), the HW will do the switch
 *     before the deschedule whenever it is possible to do
 *     the switch immediately, so it may often look like
 *     this case.
 *   - If there is a pending switch to ORDERED at the time
 *     the HW executes the de-schedule, the HW will perform
 *     the switch at the time it re-schedules, and will be
 *     able to reschedule any/all of the entries with the
 *     same tag.
 * Due to this behavior, the RECOMMENDATION to software is
 * that they have a (next) state of ATOMIC when they
 * DE-SCHEDULE. If an ORDERED tag is what was really desired,
 * SW can choose to immediately switch to an ORDERED tag
 * after the work (that has an ATOMIC tag) is re-scheduled.
 * Note that since there are never any tag switches pending
 * when the HW re-schedules, this switch can be IMMEDIATE upon
 * the reception of the pointer during the re-schedule.
 *
 * @param tag      New tag value
 * @param tag_type New tag type
 * @param group    New group value
 * @param no_sched Control whether this work queue entry will be rescheduled.
 *                 - 1 : don't schedule this work
 *                 - 0 : allow this work to be scheduled.
 */
static inline void cvmx_pow_tag_sw_desched_nocheck(uint32_t tag, cvmx_pow_tag_type_t tag_type, uint64_t group, uint64_t no_sched)
{
    cvmx_addr_t ptr;
    cvmx_pow_tag_req_t tag_req;

    tag_req.u64 = 0;
    tag_req.s.op = CVMX_POW_TAG_OP_SWTAG_DESCH;
    tag_req.s.tag = tag;
    tag_req.s.type = tag_type;
    tag_req.s.grp = group;
    tag_req.s.no_sched = no_sched;

    ptr.u64 = 0;
    ptr.sio.mem_region = CVMX_IO_SEG;
    ptr.sio.is_io = 1;
    ptr.sio.did = CVMX_OCT_DID_TAG_TAG3;

    cvmx_write_io(ptr.u64, tag_req.u64); // since TAG3 is used, this store will clear the local pending switch bit
}
/**
 * Performs a tag switch and then an immediate deschedule. This completes
 * immediatly, so completion must not be waited for.  This function does NOT
 * update the wqe in DRAM to match arguments.
 *
 * This function waits for any prior tag switches to complete, so the
 * calling code may call this function with a pending tag switch.
 *
 * Note the following CAVEAT of the Octeon HW behavior when
 * re-scheduling DE-SCHEDULEd items whose (next) state is
 * ORDERED:
 *   - If there are no switches pending at the time that the
 *     HW executes the de-schedule, the HW will only re-schedule
 *     the head of the FIFO associated with the given tag. This
 *     means that in many respects, the HW treats this ORDERED
 *     tag as an ATOMIC tag. Note that in the SWTAG_DESCH
 *     case (to an ORDERED tag), the HW will do the switch
 *     before the deschedule whenever it is possible to do
 *     the switch immediately, so it may often look like
 *     this case.
 *   - If there is a pending switch to ORDERED at the time
 *     the HW executes the de-schedule, the HW will perform
 *     the switch at the time it re-schedules, and will be
 *     able to reschedule any/all of the entries with the
 *     same tag.
 * Due to this behavior, the RECOMMENDATION to software is
 * that they have a (next) state of ATOMIC when they
 * DE-SCHEDULE. If an ORDERED tag is what was really desired,
 * SW can choose to immediately switch to an ORDERED tag
 * after the work (that has an ATOMIC tag) is re-scheduled.
 * Note that since there are never any tag switches pending
 * when the HW re-schedules, this switch can be IMMEDIATE upon
 * the reception of the pointer during the re-schedule.
 *
 * @param tag      New tag value
 * @param tag_type New tag type
 * @param group    New group value
 * @param no_sched Control whether this work queue entry will be rescheduled.
 *                 - 1 : don't schedule this work
 *                 - 0 : allow this work to be scheduled.
 */
static inline void cvmx_pow_tag_sw_desched(uint32_t tag, cvmx_pow_tag_type_t tag_type, uint64_t group, uint64_t no_sched)
{
    /* Need to make sure any writes to the work queue entry are complete */
    CVMX_SYNCWS;
    /* Ensure that there is not a pending tag switch, as a tag switch cannot be started
    ** if a previous switch is still pending.  */
    cvmx_pow_tag_sw_wait();
    cvmx_pow_tag_sw_desched_nocheck(tag, tag_type, group, no_sched);
}





/**
 * Descchedules the current work queue entry.
 *
 * @param no_sched no schedule flag value to be set on the work queue entry.  If this is set
 *                 the entry will not be rescheduled.
 */
static inline void cvmx_pow_desched(uint64_t no_sched)
{
    cvmx_addr_t ptr;
    cvmx_pow_tag_req_t tag_req;

    /* Need to make sure any writes to the work queue entry are complete */
    CVMX_SYNCWS;

    tag_req.u64 = 0;
    tag_req.s.op = CVMX_POW_TAG_OP_DESCH;
    tag_req.s.no_sched = no_sched;

    ptr.u64 = 0;
    ptr.sio.mem_region = CVMX_IO_SEG;
    ptr.sio.is_io = 1;
    ptr.sio.did = CVMX_OCT_DID_TAG_TAG3;

    cvmx_write_io(ptr.u64, tag_req.u64); // since TAG3 is used, this store will clear the local pending switch bit
}







/***********************************************************************************************
** Define usage of bits within the 32 bit tag values.
***********************************************************************************************/

/*
 * Number of bits of the tag used by software.  The SW bits
 * are always a contiguous block of the high starting at bit 31.
 * The hardware bits are always the low bits.  By default, the top 8 bits
 * of the tag are reserved for software, and the low 24 are set by the IPD unit.
 */
#define CVMX_TAG_SW_BITS    (8)
#define CVMX_TAG_SW_SHIFT   (32 - CVMX_TAG_SW_BITS)

/* Below is the list of values for the top 8 bits of the tag. */
#define CVMX_TAG_SW_BITS_INTERNAL  0x1  /* Tag values with top byte of this value are reserved for internal executive uses */
/* The executive divides the remaining 24 bits as follows:
**  * the upper 8 bits (bits 23 - 16 of the tag) define a subgroup
**  * the lower 16 bits (bits 15 - 0 of the tag) define are the value with the subgroup
** Note that this section describes the format of tags generated by software - refer to the
** hardware documentation for a description of the tags values generated by the packet input
** hardware.
** Subgroups are defined here */
#define CVMX_TAG_SUBGROUP_MASK  0xFFFF /* Mask for the value portion of the tag */
#define CVMX_TAG_SUBGROUP_SHIFT 16
#define CVMX_TAG_SUBGROUP_PKO  0x1


/* End of executive tag subgroup definitions */

/* The remaining values software bit values 0x2 - 0xff are available for application use */



/**
 * This function creates a 32 bit tag value from the two values provided.
 *
 * @param sw_bits The upper bits (number depends on configuration) are set to this value.  The remainder of
 *                bits are set by the hw_bits parameter.
 * @param hw_bits The lower bits (number depends on configuration) are set to this value.  The remainder of
 *                bits are set by the sw_bits parameter.
 *
 * @return 32 bit value of the combined hw and sw bits.
 */
static inline uint32_t cvmx_pow_tag_compose(uint64_t sw_bits, uint64_t hw_bits)
{
    return((((sw_bits & cvmx_build_mask(CVMX_TAG_SW_BITS)) << CVMX_TAG_SW_SHIFT) | (hw_bits & cvmx_build_mask(32 - CVMX_TAG_SW_BITS))));
}
/**
 * Extracts the bits allocated for software use from the tag
 *
 * @param tag    32 bit tag value
 *
 * @return N bit software tag value, where N is configurable with the CVMX_TAG_SW_BITS define
 */
static inline uint32_t cvmx_pow_tag_get_sw_bits(uint64_t tag)
{
    return((tag >> (32 - CVMX_TAG_SW_BITS)) & cvmx_build_mask(CVMX_TAG_SW_BITS));
}
/**
 *
 * Extracts the bits allocated for hardware use from the tag
 *
 * @param tag    32 bit tag value
 *
 * @return (32 - N) bit software tag value, where N is configurable with the CVMX_TAG_SW_BITS define
 */
static inline uint32_t cvmx_pow_tag_get_hw_bits(uint64_t tag)
{
    return(tag & cvmx_build_mask(32 - CVMX_TAG_SW_BITS));
}

#ifdef	__cplusplus
}
#endif

#endif  // __CVMX_POW_H__
