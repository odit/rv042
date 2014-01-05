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
 * Interface to the hardware Packet Output unit.
 *
 * File version info: $Id: cvmx-pko.h 2503 2009-09-18 07:07:04Z richie $ $Name$
 */


#ifndef __CVMX_PKO_H__
#define __CVMX_PKO_H__

#include "cvmx-cvmmem.h"
#include "cvmx-fau.h"
#include "cvmx-fpa.h"
#include "cvmx-pow.h"

#include "executive-config.h"
#ifdef CVMX_ENABLE_PKO_FUNCTIONS
#include "cvmx-config.h"
#endif

/* Adjust the command buffer size by 1 word so that in the case of using only
** two word PKO commands no command words stradle buffers.  The useful values
** for this are 0 and 1. */
#define CVMX_PKO_COMMAND_BUFFER_SIZE_ADJUST (1)

#ifdef	__cplusplus
extern "C" {
#endif

#define CVMX_PKO_MAX_OUTPUT_QUEUES_STATIC 128
#define CVMX_PKO_MAX_OUTPUT_QUEUES      ((OCTEON_IS_MODEL(OCTEON_CN31XX) || OCTEON_IS_MODEL(OCTEON_CN3010) || OCTEON_IS_MODEL(OCTEON_CN3005) || OCTEON_IS_MODEL(OCTEON_CN50XX)) ? 32 : 128)
#define CVMX_PKO_NUM_OUTPUT_PORTS       36
#define CVMX_PKO_MEM_QUEUE_PTRS_ILLEGAL_PID 63 // use this for queues that are not used
#define CVMX_PKO_QUEUE_STATIC_PRIORITY  9

/* Defines how the PKO command buffer FAU register is used */
#define CVMX_PKO_INDEX_BITS     12
#define CVMX_PKO_INDEX_MASK     ((1ull << CVMX_PKO_INDEX_BITS) - 1)

typedef enum
{
    CVMX_PKO_SUCCESS,
    CVMX_PKO_INVALID_PORT,
    CVMX_PKO_INVALID_QUEUE,
    CVMX_PKO_INVALID_PRIORITY,
    CVMX_PKO_NO_MEMORY
} cvmx_pko_status_t;

typedef struct
{
    uint32_t    packets;
    uint64_t    octets;
  uint64_t doorbell;
} cvmx_pko_port_status_t;

/**
 * This structure defines the address to use on a packet enqueue
 */
typedef union
{
    uint64_t                u64;
    struct
    {
        cvmx_mips_space_t   mem_space   : 2;    /**< Must CVMX_IO_SEG */
        uint64_t            reserved    :13;    /**< Must be zero */
        uint64_t            is_io       : 1;    /**< Must be one */
        uint64_t            did         : 8;    /**< The ID of the device on the non-coherent bus */
        uint64_t            reserved2   : 4;    /**< Must be zero */
        uint64_t            reserved3   :18;    /**< Must be zero */
        uint64_t            port        : 6;    /**< The hardware likes to have the output port in addition to the output queue */
        uint64_t            queue       : 9;    /**< The output queue to send the packet to (0-127 are legal) */
        uint64_t            reserved4   : 3;    /**< Must be zero */
   } s;
} cvmx_pko_doorbell_address_t;

/**
 * Structure of the first packet output command word.
 */
typedef union
{
    uint64_t                u64;
    struct
    {
        cvmx_fau_op_size_t  size1       : 2; /**< The size of the reg1 operation - could be 8, 16, 32, or 64 bits */
        cvmx_fau_op_size_t  size0       : 2; /**< The size of the reg0 operation - could be 8, 16, 32, or 64 bits */
        uint64_t            subone1     : 1; /**< If set, subtract 1, if clear, subtract packet size */
        uint64_t            reg1        :11; /**< The register, subtract will be done if reg1 is non-zero */
        uint64_t            subone0     : 1; /**< If set, subtract 1, if clear, subtract packet size */
        uint64_t            reg0        :11; /**< The register, subtract will be done if reg0 is non-zero */
        uint64_t            le          : 1; /**< When set, interpret segment pointer and segment bytes in little endian order */
        uint64_t            n2          : 1; /**< When set, packet data not allocated in L2 cache by PKO */
        uint64_t            wqp         : 1; /**< If set and rsp is set, word3 contains a pointer to a work queue entry */
        uint64_t            rsp         : 1; /**< If set, the hardware will send a response when done */
        uint64_t            gather      : 1; /**< If set, the supplied pkt_ptr is really a pointer to a list of pkt_ptr's */
        uint64_t            ipoffp1     : 7; /**< If ipoffp1 is non zero, (ipoffp1-1) is the number of bytes to IP header,
                                                and the hardware will calculate and insert the  UDP/TCP checksum */
        uint64_t            ignore_i    : 1; /**< If set, ignore the I bit (force to zero) from all pointer structures */
        uint64_t            dontfree    : 1; /**< If clear, the hardware will attempt to free the buffers containing the packet */
        uint64_t            segs        : 6; /**< The total number of segs in the packet, if gather set, also gather list length */
        uint64_t            total_bytes :16; /**< Including L2, but no trailing CRC */
    } s;
} cvmx_pko_command_word0_t;

/* CSR typedefs have been moved to cvmx-csr-*.h */

/**
 * Definition of internal state for Packet output processing
 */
typedef struct
{
    uint64_t *      start_ptr;          /**< ptr to start of buffer, offset kept in FAU reg */
} cvmx_pko_state_elem_t;


#ifdef CVMX_ENABLE_PKO_FUNCTIONS
/**
 * Call before any other calls to initialize the packet
 * output system.
 */
extern void cvmx_pko_initialize_global(void);
extern int cvmx_pko_initialize_local(void);
#endif



/**
 * Enables the packet output hardware. It must already be
 * configured.
 */
extern void cvmx_pko_enable(void);


/**
 * Disables the packet output. Does not affect any configuration.
 */
extern void cvmx_pko_disable(void);


/**
 * Shutdown and free resources required by packet output.
 */

#ifdef CVMX_ENABLE_PKO_FUNCTIONS
extern void cvmx_pko_shutdown(void);
#endif

/**
 * Configure a output port and the associated queues for use.
 *
 * @param port       Port to configure.
 * @param base_queue First queue number to associate with this port.
 * @param num_queues Number of queues t oassociate with this port
 * @param priority   Array of priority levels for each queue. Values are
 *                   allowed to be 1-8. A value of 8 get 8 times the traffic
 *                   of a value of 1. There must be num_queues elements in the
 *                   array.
 */
extern cvmx_pko_status_t cvmx_pko_config_port(uint64_t port, uint64_t base_queue, uint64_t num_queues, const uint64_t priority[]);


/**
 * Ring the packet output doorbell. This tells the packet
 * output hardware that "len" command words have been added
 * to its pending list.  This command includes the required
 * CVMX_SYNCWS before the doorbell ring.
 *
 * @param port   Port the packet is for
 * @param queue  Queue the packet is for
 * @param len    Length of the command in 64 bit words
 */
static inline void cvmx_pko_doorbell(uint64_t port, uint64_t queue, uint64_t len)
{
   cvmx_pko_doorbell_address_t ptr;

   ptr.u64          = 0;
   ptr.s.mem_space  = CVMX_IO_SEG;
   ptr.s.did        = CVMX_OCT_DID_PKT_SEND;
   ptr.s.is_io      = 1;
   ptr.s.port       = port;
   ptr.s.queue      = queue;
   CVMX_SYNCWS;  /* Need to make sure output queue data is in DRAM before doorbell write */
   cvmx_write_io(ptr.u64, len);
}

/**
 * Prepare to send a packet.  This initiates the tag switch
 * to get exclusive access to the output queue structure, and
 * performs other prep work for the packet send operation.
 *
 * cvmx_pko_send_packet_finish() MUST be called after this function is called,
 * and must be called with the same port/queue/use_locking arguments.
 *
 * The use_locking flag allows these functions to optionally do any locking/synchronization on the output
 * queue structures.  In some applications this is not required (for instance if only
 * one core is outputing to a queue) and passing a 0 in the use_locking argument causes the
 * packet output functions to not do any locking on the output queue structures.
 * 
 * NOTE: If locking is used, the POW entry CANNOT be descheduled, as it does not contain a valid
 * WQE pointer.
 *
 * @param port   Port to send it on
 * @param queue  Queue to use
 * @param use_locking Flag indicating whether locking should be done on queue structures.
 *                    If 1, then the the send packet routines will perform locking on the output
 *                    queue structures.  If set to 0, the send packet routines do not do any locking,
 *                    and the application must take care of the required synchronization.
 *
 * @return
 */
#ifdef CVMX_ENABLE_PKO_FUNCTIONS
static inline void cvmx_pko_send_packet_prepare(uint64_t port, uint64_t queue, int use_locking)
{
    /* Switch tag to value used to protect output queue */

    if (use_locking)
    {
        /* Must do a full switch here to handle all cases.  We use a fake WQE pointer, as the POW does
        ** not access this memory.  The WQE pointer and group are only used if this work is descheduled,
        ** which is not supported by the cvmx_pko_send_packet_prepare/cvmx_pko_send_packet_finish combination.
        ** Note that this is a special case in which these fake values can be used - this is not a general technique.
        */
        uint32_t tag = CVMX_TAG_SW_BITS_INTERNAL << CVMX_TAG_SW_SHIFT | CVMX_TAG_SUBGROUP_PKO  << CVMX_TAG_SUBGROUP_SHIFT | (CVMX_TAG_SUBGROUP_MASK & queue);
        cvmx_pow_tag_sw_full((cvmx_wqe_t *)cvmx_phys_to_ptr(0x80), 
                             tag,
                             CVMX_POW_TAG_TYPE_ATOMIC,
                             0);
        /* start a tagwait FAU operation for command buffer offset */
        cvmx_fau_async_tagwait_fetch_and_add64(CVMX_SCR_SCRATCH, (cvmx_fau_reg_64_t)(CVMX_FAU_REG_OQ_ADDR_INDEX + 8 * queue), 2);
    }
    else
    {
        cvmx_fau_async_fetch_and_add64(CVMX_SCR_SCRATCH, (cvmx_fau_reg_64_t)(CVMX_FAU_REG_OQ_ADDR_INDEX + 8 * queue), 2);
    }

}

/**
 * Complete packet output. cvmx_pko_send_packet_prepare() must be called exactly once before this,
 * and the same parameters must be passed to both cvmx_pko_send_packet_prepare() and
 * cvmx_pko_send_packet_finish().
 * @param port    Port to send it on
 * @param queue   Queue to use
 * @param pko_command
 *                PKO HW command word
 * @param packet  Packet to send
 * @param use_locking Flag indicating whether locking should be done on queue structures.
 *                    If 1, then the the send packet routines will perform locking on the output
 *                    queue structures.  If set to 0, the send packet routines do not do any locking,
 *                    and the application must take care of the required synchronization.
 *
 * @return returns CVMX_PKO_SUCCESS on success, or error code on failure of output
 */
static inline cvmx_pko_status_t cvmx_pko_send_packet_finish(uint64_t port, uint64_t queue,
                                        cvmx_pko_command_word0_t pko_command,
                                        cvmx_buf_ptr_t packet, int use_locking)
{
    uint64_t *cur_oq_ptr;
    uint64_t oq_index;
    uint64_t hw_newbuf;
    uint64_t *p_newbuf;

    CVMX_SYNCWS;  /* flush any pending writes before entering critical region, avoid stalling on SYNCW inside critical region */

    /* Spin while waiting for tag to complete so core can be interrupted. */
    cvmx_pow_tag_sw_wait();
    CVMX_SYNCIOBDMA;  /* wait for index fetch&add and the tag switch response to be in scratch */

    /* Start of critical section.  The switch to atomic has completed.  The critical
    ** section ends when the main program either switches to another tag or
    ** gets the next piece of work.
    */

    /* FAU tagwait timeouts are disabled, and we wait for the tag completion before continuing, so
    ** we always get a valid FAU response.
    */
    oq_index = cvmx_scratch_read64(CVMX_SCR_SCRATCH);


    /* Buffer pointer is in upper 64 - CVMX_PKO_INDEX_BITS bits,
    ** index is in lower CVMX_PKO_INDEX_BITS bits.
    */
    cur_oq_ptr = (uint64_t*)cvmx_phys_to_ptr(oq_index >> CVMX_PKO_INDEX_BITS);
    oq_index &= CVMX_PKO_INDEX_MASK;
    cur_oq_ptr += oq_index;

    if (cvmx_likely(oq_index < ((CVMX_FPA_OUTPUT_BUFFER_POOL_SIZE/8) - 3 - CVMX_PKO_COMMAND_BUFFER_SIZE_ADJUST)))
    {
        cur_oq_ptr[0] = pko_command.u64;
        cur_oq_ptr[1] = packet.u64;

        cvmx_pko_doorbell(port, queue, 2);
        return CVMX_PKO_SUCCESS;

    }


    /* We need to allocate new buffer. Allocate new buffer from FPA */
    hw_newbuf = cvmx_scratch_read64(CVMX_SCR_OQ_BUF_PRE_ALLOC);
    p_newbuf = (uint64_t*)cvmx_phys_to_ptr(hw_newbuf);

    /* Request new oq buffer - SYNCIOBDMA for critical section will ensure completion before use */
    cvmx_fpa_async_alloc(CVMX_SCR_OQ_BUF_PRE_ALLOC, CVMX_FPA_OUTPUT_BUFFER_POOL);

    if (cvmx_unlikely(!hw_newbuf))
    {
        /* We don't have room, so decrement our index, and return error. */
        cvmx_fau_fetch_and_add64((cvmx_fau_reg_64_t)(CVMX_FAU_REG_OQ_ADDR_INDEX + 8 * queue), -2);
        return CVMX_PKO_NO_MEMORY;
    }


    /* we either have 2 or 3 words left */
    cur_oq_ptr[0] = pko_command.u64;

    /* Since we set the command size word to the pool size minus one, the
        buffer will always end with 3 words. This is faster than the split
        case. The "if" statement is still here in case someone begins using
        the three word command format. At that time this code will work, just
        be less optimal */
    if (cvmx_unlikely(oq_index >= (CVMX_FPA_OUTPUT_BUFFER_POOL_SIZE/8) - 3))
    {
        /* 2 words left, spills into new buf */
        p_newbuf[0] = packet.u64;
        cur_oq_ptr[1] = hw_newbuf;
        oq_index = 1;
    }
    else
    {
        /* 3 words left, fills current buffer exactly */
        cur_oq_ptr[1] = packet.u64;
        cur_oq_ptr[2] = hw_newbuf;
        oq_index = 0;
    }

    /* Set new buffer pointer and index */
    cvmx_fau_atomic_write64((cvmx_fau_reg_64_t)(CVMX_FAU_REG_OQ_ADDR_INDEX + 8 * queue), (hw_newbuf << CVMX_PKO_INDEX_BITS) | oq_index);
    /* Read it and wait for response to ensure that the FAU register is updated within critical section
    ** protected by atomic tag.
    */
    cvmx_fau_fetch_and_add64((cvmx_fau_reg_64_t)(CVMX_FAU_REG_OQ_ADDR_INDEX + 8 * queue), 0);


    /* Hardware will free the previous buffer when it is finished with it */
    cvmx_pko_doorbell(port, queue, 2);
    return CVMX_PKO_SUCCESS;

}


/**
 * For a given port number, return the base pko output queue
 * for the port.
 *
 * @param port   Port number
 * @return Base output queue
 */
static inline int cvmx_pko_get_base_queue(int port)
{
    if (port < 16)
        return port * CVMX_PKO_QUEUES_PER_PORT_INTERFACE0;
    else if (port<32)
        return 16 * CVMX_PKO_QUEUES_PER_PORT_INTERFACE0 +
                (port-16) * CVMX_PKO_QUEUES_PER_PORT_INTERFACE1;
    else
        return 16 * CVMX_PKO_QUEUES_PER_PORT_INTERFACE0 +
                16 * CVMX_PKO_QUEUES_PER_PORT_INTERFACE1 +
                (port-32) * CVMX_PKO_QUEUES_PER_PORT_PCI;
}


/**
 * For a given port number, return the number of pko output queues.
 *
 * @param port   Port number
 * @return Number of output queues
 */
static inline int cvmx_pko_get_num_queues(int port)
{
    if (port < 16)
        return CVMX_PKO_QUEUES_PER_PORT_INTERFACE0;
    else if (port<32)
        return CVMX_PKO_QUEUES_PER_PORT_INTERFACE1;
    else
        return CVMX_PKO_QUEUES_PER_PORT_PCI;
}

/**
 * Get the status counters for a port.
 *
 * @param port_num Port number to get statistics for.
 * @param clear    Set to 1 to clear the counters after they are read
 * @param status   Where to put the results.
 */
static inline void cvmx_pko_get_port_status(uint64_t port_num, uint64_t clear, cvmx_pko_port_status_t *status)
{
    cvmx_write_csr(CVMX_PKO_REG_READ_IDX, port_num);
    status->packets = cvmx_read_csr(CVMX_PKO_MEM_COUNT0);
    if (clear)
        cvmx_write_csr(CVMX_PKO_MEM_COUNT0, port_num);
    status->octets = cvmx_read_csr(CVMX_PKO_MEM_COUNT1);
    if (clear)
        cvmx_write_csr(CVMX_PKO_MEM_COUNT1, port_num);
    cvmx_write_csr(CVMX_PKO_REG_READ_IDX, cvmx_pko_get_base_queue(port_num));
    status->doorbell = (cvmx_read_csr(CVMX_PKO_MEM_DEBUG9) >> 8) & 0xfffff;
}

#endif /* CVMX_ENABLE_PKO_FUNCTIONS */

#ifdef	__cplusplus
}
#endif

#endif   /* __CVMX_PKO_H__ */
