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
 * Support library for the hardware Packet Output unit.
 *
 * File version info: $Id: cvmx-pko.c 2 2007-04-05 08:51:12Z tt $ $Name$
 */

#if defined(__KERNEL__) && defined(__linux__)
    #include <linux/kernel.h>
    #include <linux/string.h>
    #define printf printk
#else
    #include <stdio.h>
    #include <string.h>
#endif
#include "executive-config.h"
#include "cvmx-config.h"
#include "cvmx.h"
#include "cvmx-fau.h"
#include "cvmx-pko.h"

/**
 * Internal state of packet output
 */

#ifdef CVMX_ENABLE_PKO_FUNCTIONS


/**
 * Call before any other calls to initialize the packet
 * output system.  This does chip global config, and should only be
 * done by one core.
 */

void cvmx_pko_initialize_global(void)
{
    int i;
    uint64_t priority = 8;
    cvmx_pko_pool_cfg_t config;

    /* Set the size of the PKO command buffers to an odd number of 64bit
        words. This allows the normal two word send to stay aligned and never
        span a comamnd word buffer. */
    config.u64 = 0;
    config.s.pool = CVMX_FPA_OUTPUT_BUFFER_POOL;
    config.s.size = CVMX_FPA_OUTPUT_BUFFER_POOL_SIZE / 8 - 1;

    cvmx_write_csr(CVMX_PKO_REG_CMD_BUF, config.u64);

    for (i=0; i<CVMX_PKO_MAX_OUTPUT_QUEUES; i++)
        cvmx_pko_config_port(CVMX_PKO_MEM_QUEUE_PTRS_ILLEGAL_PID, i, 1, &priority);
}

/**
 * This function does per-core initialization required by the PKO routines.
 * This must be called on all cores that will do packet output, and must
 * be called after the FPA has been initialized and filled with pages.
 *
 * @return 0 on success
 *         !0 on failure
 */
int cvmx_pko_initialize_local(void)
{
    /* Preallocate a command buffer into the designated scratch memory location */
    cvmx_fpa_async_alloc(CVMX_SCR_OQ_BUF_PRE_ALLOC, CVMX_FPA_OUTPUT_BUFFER_POOL);
    CVMX_SYNCIOBDMA;
    return(!cvmx_scratch_read64(CVMX_SCR_OQ_BUF_PRE_ALLOC));

}
#endif

/**
 * Enables the packet output hardware. It must already be
 * configured.
 */
void cvmx_pko_enable(void)
{
    cvmx_pko_reg_flags_t flags;

    flags.u64 = cvmx_read_csr(CVMX_PKO_REG_FLAGS);
    if (flags.s.ena_pko)
        cvmx_dprintf("Warning: Enabling PKO when PKO already enabled.\n");

    flags.s.ena_dwb = 1;
    flags.s.ena_pko = 1;
    cvmx_write_csr(CVMX_PKO_REG_FLAGS, flags.u64);
}


/**
 * Disables the packet output. Does not affect any configuration.
 */
void cvmx_pko_disable(void)
{
    cvmx_write_csr(CVMX_PKO_REG_FLAGS, 0);
}


/**
 * Shutdown and free resources required by packet output.
 */
#ifdef CVMX_ENABLE_PKO_FUNCTIONS
static uint8_t cvmx_pko_queue_map[CVMX_PKO_MAX_OUTPUT_QUEUES_STATIC];  /* Used to track queue usage for shutdown */
void cvmx_pko_shutdown(void)
{
    cvmx_pko_queue_cfg_t config;
    int queue;
    uint64_t buf_addr;

    cvmx_pko_disable();

    for (queue=0; queue<CVMX_PKO_MAX_OUTPUT_QUEUES; queue++)
    {
        config.u64          = 0;
        config.s.tail       = 1;
        config.s.index      = 0;
        config.s.port       = CVMX_PKO_MEM_QUEUE_PTRS_ILLEGAL_PID;
        config.s.queue      = queue;
        config.s.qos_mask   = 0;
        config.s.buf_ptr    = 0;
        cvmx_write_csr(CVMX_PKO_MEM_QUEUE_PTRS, config.u64);

        if(cvmx_pko_queue_map[queue])
        {
            buf_addr = cvmx_fau_fetch_and_add64((cvmx_fau_reg_64_t)(CVMX_FAU_REG_OQ_ADDR_INDEX + 8 * queue), 0) >> CVMX_PKO_INDEX_BITS;
            if (buf_addr)
            {
                cvmx_fpa_free(cvmx_phys_to_ptr(buf_addr), CVMX_FPA_OUTPUT_BUFFER_POOL, 0);
            }
        }
    }
}


/**
 * Configure a output port and the associated queues for use.
 *
 * @param port       Port to configure.
 * @param base_queue First queue number to associate with this port.
 * @param num_queues Number of queues t oassociate with this port
 * @param priority   Array of priority levels for each queue. Values are
 *                   allowed to be 0-8. A value of 8 get 8 times the traffic
 *                   of a value of 1.  A value of 0 indicates that no rounds
 *                   will be participated in. These priorities can be changed
 *                   on the fly while the pko is enabled. A priority of 9
 *                   indicates that static priority should be used.  If static
 *                   priority is used all queues with static priority must be
 *                   contiguous starting at the base_queue, and lower numbered
 *                   queues have higher priority than higher numbered queues.
 *                   There must be num_queues elements in the array.
 */
cvmx_pko_status_t cvmx_pko_config_port(uint64_t port, uint64_t base_queue, uint64_t num_queues, const uint64_t priority[])
{
    cvmx_pko_status_t   result_code;
    uint64_t            queue;
    cvmx_pko_queue_cfg_t config;
    int static_priority_base = -1;
    int static_priority_end = -1;

    if ((port >= CVMX_PKO_NUM_OUTPUT_PORTS) && (port != CVMX_PKO_MEM_QUEUE_PTRS_ILLEGAL_PID))
    {
        cvmx_dprintf("ERROR: cvmx_pko_config_port: Invalid port %llu\n", (unsigned long long)port);
        return CVMX_PKO_INVALID_PORT;
    }

    if (base_queue + num_queues > CVMX_PKO_MAX_OUTPUT_QUEUES)
    {
        cvmx_dprintf("ERROR: cvmx_pko_config_port: Invalid queue range\n");
        return CVMX_PKO_INVALID_QUEUE;
    }

    if (port != CVMX_PKO_MEM_QUEUE_PTRS_ILLEGAL_PID)
    {
        /* Validate the static queue priority setup and set static_priority_base and static_priority_end
        ** accordingly. */
        for (queue = 0; queue < num_queues; queue++)
        {
            /* Find first queue of static priority */
            if (static_priority_base == -1 && priority[queue] == CVMX_PKO_QUEUE_STATIC_PRIORITY)
                static_priority_base = queue;
            /* Find last queue of static priority */
            if (static_priority_base != -1 && static_priority_end == -1 && priority[queue] != CVMX_PKO_QUEUE_STATIC_PRIORITY && queue)
                static_priority_end = queue - 1;
            else if (static_priority_base != -1 && static_priority_end == -1 && queue == num_queues - 1)
                static_priority_end = queue;  /* all queues are static priority */
            /* Check to make sure all static priority queues are contiguous.  Also catches some cases of
            ** static priorites not starting at queue 0. */
            if (static_priority_end != -1 && (int)queue > static_priority_end && priority[queue] == CVMX_PKO_QUEUE_STATIC_PRIORITY)
            {
                cvmx_dprintf("ERROR: cvmx_pko_config_port: Static priority queues aren't contiguous or don't start at base queue. q: %d, eq: %d\n", (int)queue, static_priority_end);
                return CVMX_PKO_INVALID_PRIORITY;
            }
        }
        if (static_priority_base > 0)
        {
            cvmx_dprintf("ERROR: cvmx_pko_config_port: Static priority queues don't start at base queue. sq: %d\n", static_priority_base);
            return CVMX_PKO_INVALID_PRIORITY;
        }
#if 0
        cvmx_dprintf("Port %d: Static priority queue base: %d, end: %d\n", port, static_priority_base, static_priority_end);
#endif
    }
    /* At this point, static_priority_base and static_priority_end are either both -1,
    ** or are valid start/end queue numbers */

    result_code = CVMX_PKO_SUCCESS;

    for (queue = 0; queue < num_queues; queue++)
    {
        uint64_t  *buf_ptr = NULL;
        config.u64          = 0;
        config.s.tail       = queue == (num_queues - 1);
        config.s.index      = queue;
        config.s.port       = port;
        config.s.queue      = base_queue + queue;

        if (!cvmx_octeon_is_pass1())
        {
            config.s.static_p   = static_priority_base >= 0;
            config.s.static_q   = (int)queue <= static_priority_end;
            config.s.s_tail     = (int)queue == static_priority_end;
        }
        /* Convert the priority into an enable bit field. Try to space the bits
            out evenly so the packet don't get grouped up */
        switch ((int)priority[queue])
        {
            case 0: config.s.qos_mask = 0x00; break;
            case 1: config.s.qos_mask = 0x01; break;
            case 2: config.s.qos_mask = 0x11; break;
            case 3: config.s.qos_mask = 0x49; break;
            case 4: config.s.qos_mask = 0x55; break;
            case 5: config.s.qos_mask = 0x57; break;
            case 6: config.s.qos_mask = 0x77; break;
            case 7: config.s.qos_mask = 0x7f; break;
            case 8: config.s.qos_mask = 0xff; break;
            case CVMX_PKO_QUEUE_STATIC_PRIORITY:
                if (!cvmx_octeon_is_pass1()) /* Pass 1 will fall through to the error case */
                {
                    config.s.qos_mask = 0xff;
                    break;
                }
            default:
                cvmx_dprintf("ERROR: cvmx_pko_config_port: Invalid priority %llu\n", (unsigned long long)priority[queue]);
                config.s.qos_mask = 0xff;
                result_code = CVMX_PKO_INVALID_PRIORITY;
                break;
        }

        if (port != CVMX_PKO_MEM_QUEUE_PTRS_ILLEGAL_PID)
        {
            buf_ptr = (uint64_t*)cvmx_fpa_alloc(CVMX_FPA_OUTPUT_BUFFER_POOL);
            if (!buf_ptr)
            {
                cvmx_dprintf("ERROR: cvmx_pko_config_port: Unable to allocate output buffer.\n");
                return(CVMX_PKO_NO_MEMORY);
            }

            /* Set initial command buffer address and index in FAU register for queue */
            cvmx_fau_atomic_write64((cvmx_fau_reg_64_t)(CVMX_FAU_REG_OQ_ADDR_INDEX + 8 * (base_queue + queue)), cvmx_ptr_to_phys(buf_ptr) << CVMX_PKO_INDEX_BITS);
            cvmx_pko_queue_map[base_queue + queue] = 1;

        }
        config.s.buf_ptr = cvmx_ptr_to_phys(buf_ptr);
        CVMX_SYNCWS;
        cvmx_write_csr(CVMX_PKO_MEM_QUEUE_PTRS, config.u64);
    }

    return result_code;
}
#endif /* CVMX_ENABLE_PKO_FUNCTIONS */

