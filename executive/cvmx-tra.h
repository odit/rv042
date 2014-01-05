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
 * Interface to the Trace buffer hardware.
 *
 * WRITING THE TRACE BUFFER
 *
 * When the trace is enabled, commands are traced continuously (wrapping) or until the buffer is filled once
 * (no wrapping).  Additionally and independent of wrapping, tracing can be temporarily enabled and disabled
 * by the tracing triggers.  All XMC commands can be traced except for IDLE and IOBRSP.  The subset of XMC
 * commands that are traced is determined by the filter and the two triggers, each of which is comprised of
 * masks for command, sid, did, and address).  If triggers are disabled, then only those commands matching
 * the filter are traced.  If triggers are enabled, then only those commands matching the filter, the start
 * trigger, or the stop trigger are traced during the time between a start trigger and a stop trigger.
 *
 * For a given command, its XMC data is written immediately to the buffer.  If the command has XMD data,
 * then that data comes in-order at some later time.  The XMD data is accumulated across all valid
 * XMD cycles and written to the buffer or to a shallow fifo.  Data from the fifo is written to the buffer
 * as soon as it gets access to write the buffer (i.e. the buffer is not currently being written with XMC
 * data).  If the fifo overflows, it simply overwrites itself and the previous XMD data is lost.
 *
 *
 * READING THE TRACE BUFFER
 *
 * Each entry of the trace buffer is read by a CSR read command.  The trace buffer services each read in order,
 * as soon as it has access to the (single-ported) trace buffer.
 *
 *
 * OVERFLOW, UNDERFLOW AND THRESHOLD EVENTS
 *
 * The trace buffer maintains a write pointer and a read pointer and detects both the overflow and underflow
 * conditions.  Each time a new trace is enabled, both pointers are reset to entry 0.  Normally, each write
 * (traced event) increments the write pointer and each read increments the read pointer.  During the overflow
 * condition, writing (tracing) is disabled.  Tracing will continue as soon as the overflow condition is
 * resolved.  The first entry that is written immediately following the overflow condition may be marked to
 * indicate that a tracing discontinuity has occurred before this entry.  During the underflow condition,
 * reading does not increment the read pointer and the read data is marked to indicate that no read data is
 * available.
 *
 * The full threshold events are defined to signal an interrupt a certain levels of "fullness" (1/2, 3/4, 4/4).
 * "fullness" is defined as the relative distance between the write and read pointers (i.e. not defined as the
 * absolute distance between the write pointer and entry 0).  When enabled, the full threshold event occurs
 * every time the desired level of "fullness" is achieved.
 *
 *
 * Trace buffer entry format
 * @verbatim
 *       6                   5                   4                   3                   2                   1                   0
 * 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |sta|                          address[35:3]                          |       0       | src id  |   0   | DWB   | diff timestamp|
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |sta|                          address[35:3]                          |       0       | src id  |   0   | PL2   | diff timestamp|
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |sta|                          address[35:3]                          |       0       | src id  |   0   | PSL1  | diff timestamp|
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |sta|                          address[35:3]                          |       0       | src id  |   0   | LDD   | diff timestamp|
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |sta|                          address[35:3]                          |       0       | src id  |   0   | LDI   | diff timestamp|
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |sta|                          address[35:3]                          |       0       | src id  |   0   | LDT   | diff timestamp|
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |sta|                          address[35:3]                          | * or 16B mask | src id  |   0   | STC   | diff timestamp|
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |sta|                          address[35:3]                          | * or 16B mask | src id  |   0   | STF   | diff timestamp|
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |sta|                          address[35:3]                          | * or 16B mask | src id  |   0   | STP   | diff timestamp|
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |sta|                          address[35:3]                          | * or 16B mask | src id  |   0   | STT   | diff timestamp|
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |sta|                          address[35:0]                                |    0    | src id| dest id |IOBLD8 | diff timestamp|
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |sta|                          address[35:1]                              |     0     | src id| dest id |IOBLD16| diff timestamp|
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |sta|                          address[35:2]                            |      0      | src id| dest id |IOBLD32| diff timestamp|
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |sta|                          address[35:3]                          |       0       | src id| dest id |IOBLD64| diff timestamp|
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |sta|                          address[35:3]                          | * or 16B mask | src id| dest id |IOBST  | diff timestamp|
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |sta|                     * or address[35:3]                          | * or length   | src id| dest id |IOBDMA | diff timestamp|
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * notes:
 * - Fields marked as '*' are first filled with '0' at XMC time and may be filled with real data later at XMD time.  Note that the
 * XMD write may be dropped if the shallow FIFO overflows which leaves the '*' fields as '0'.
 * - 2 bits (sta) are used not to trace, but to return global state information with each read, encoded as follows:
 * 0x0-0x1=not valid
 * 0x2=valid, no discontinuity
 * 0x3=valid,    discontinuity
 * - commands are encoded as follows:
 * 0x0=DWB
 * 0x1=PL2
 * 0x2=PSL1
 * 0x3=LDD
 * 0x4=LDI
 * 0x5=LDT
 * 0x6=STC
 * 0x7=STF
 * 0x8=STP
 * 0x9=STT
 * 0xa=IOBLD8
 * 0xb=IOBLD16
 * 0xc=IOBLD32
 * 0xd=IOBLD64
 * 0xe=IOBST
 * 0xf=IOBDMA
 * - For non IOB* commands
 * - source id is encoded as follows:
 * 0x00-0x0f=PP[n]
 * 0x10=IOB(Packet)
 * 0x11=IOB(PKO)
 * 0x12=IOB(ReqLoad, ReqStore)
 * 0x13=IOB(DWB)
 * 0x14-0x1e=illegal
 * 0x1f=IOB(generic)
 * - dest   id is unused (can only be L2c)
 * - For IOB* commands
 * - source id is encoded as follows:
 * 0x00-0x0f = PP[n]
 * - dest   id is encoded as follows:
 * 0x00-0x0f=PP[n]
 * 0x10=IOB(Packet)
 * 0x11=IOB(PKO)
 * 0x12=IOB(ReqLoad, ReqStore)
 * 0x13=IOB(DWB)
 * 0x14-0x1e=illegal
 * 0x1f=IOB(generic)
 *
 * Source of data for each command
 * command source id    dest id      address                 length/mask
 * -------+------------+------------+-----------------------+----------------------------------------------
 * LDI     xmc_sid[8:3] x            xmc_adr[35:3]           x
 * LDT     xmc_sid[8:3] x            xmc_adr[35:3]           x
 * STF     xmc_sid[8:3] x            xmc_adr[35:3]           16B mask(xmd_[wrval,eow,adr[6:4],wrmsk[15:0]])
 * STC     xmc_sid[8:3] x            xmc_adr[35:3]           16B mask(xmd_[wrval,eow,adr[6:4],wrmsk[15:0]])
 * STP     xmc_sid[8:3] x            xmc_adr[35:3]           16B mask(xmd_[wrval,eow,adr[6:4],wrmsk[15:0]])
 * STT     xmc_sid[8:3] x            xmc_adr[35:3]           16B mask(xmd_[wrval,eow,adr[6:4],wrmsk[15:0]])
 * DWB     xmc_sid[8:3] x            xmc_adr[35:3]           x
 * PL2     xmc_sid[8:3] x            xmc_adr[35:3]           x
 * PSL1    xmc_sid[8:3] x            xmc_adr[35:3]           x
 * IOBLD8  xmc_sid[8:3] xmc_did[8:3] xmc_adr[35:0]           x
 * IOBLD16 xmc_sid[8:3] xmc_did[8:3] xmc_adr[35:1]           x
 * IOBLD32 xmc_sid[8:3] xmc_did[8:3] xmc_adr[35:2]           x
 * IOBLD64 xmc_sid[8:3] xmc_did[8:3] xmc_adr[35:3]           x
 * IOBST   xmc_sid[8:3] xmc_did[8:3] xmc_adr[35:3]           16B mask(xmd_[wrval,eow,adr[6:4],wrmsk[15:0]])
 * IOBDMA  xmc_sid[8:3] xmc_did[8:3] (xmd_[wrval,eow,dat[]]) length(xmd_[wrval,eow,dat[]])
 * IOBRSP  not traced, but monitored to keep XMC and XMD data in sync.
 * @endverbatim
 *
 * $Id: cvmx-tra.h 2 2007-04-05 08:51:12Z tt $ $Name$
 */
#include "cvmx-csr.h"

#ifndef __CVMX_TRA_H__
#define __CVMX_TRA_H__
                       
#ifdef	__cplusplus
extern "C" {
#endif                       


/* CSR typedefs have been moved to cvmx-csr-*.h */

/**
 * Enumeration of the data types stored in cvmx_tra_data_t
 */
typedef enum
{
    CVMX_TRA_DATA_DWB       = 0x0,
    CVMX_TRA_DATA_PL2       = 0x1,
    CVMX_TRA_DATA_PSL1      = 0x2,
    CVMX_TRA_DATA_LDD       = 0x3,
    CVMX_TRA_DATA_LDI       = 0x4,
    CVMX_TRA_DATA_LDT       = 0x5,
    CVMX_TRA_DATA_STC       = 0x6,
    CVMX_TRA_DATA_STF       = 0x7,
    CVMX_TRA_DATA_STP       = 0x8,
    CVMX_TRA_DATA_STT       = 0x9,
    CVMX_TRA_DATA_IOBLD8    = 0xa,
    CVMX_TRA_DATA_IOBLD16   = 0xb,
    CVMX_TRA_DATA_IOBLD32   = 0xc,
    CVMX_TRA_DATA_IOBLD64   = 0xd,
    CVMX_TRA_DATA_IOBST     = 0xe,
    CVMX_TRA_DATA_IOBDMA    = 0xf,
} cvmx_tra_data_type_t;

/**
 * TRA data format definition. Use the type field to
 * determine which union element to use.
 */
typedef union
{
    uint64_t u64;
    struct
    {
        uint64_t    valid       : 1;
        uint64_t    discontinuity:1;
        uint64_t    address     : 33;
        uint64_t    mask        : 8;
        uint64_t    source      : 5;
        uint64_t    reserved2   : 4;
        cvmx_tra_data_type_t type:4;
        uint64_t    timestamp   : 8;
    } cmn; /**< for DWB, PL2, PSL1, LDD, LDI, LDT, STC, STF, STP, STT */
    struct
    {
        uint64_t    valid       : 1;
        uint64_t    discontinuity:1;
        uint64_t    address     : 37;
        uint64_t    reserved    : 5;
        uint64_t    source      : 4;
        uint64_t    dest        : 5;
        cvmx_tra_data_type_t type:4;
        uint64_t    timestamp   : 8;
    } iobld; /**< for IOBLD8, IOBLD16, IOBLD32, IOBLD64 */
    struct
    {
        uint64_t    valid       : 1;
        uint64_t    discontinuity:1;
        uint64_t    address     : 33;
        uint64_t    mask        : 8;
        uint64_t    source      : 4;
        uint64_t    dest        : 5;
        cvmx_tra_data_type_t type:4;
        uint64_t    timestamp   : 8;
    } iobst; /**< for IOBST */
    struct
    {
        uint64_t    valid       : 1;
        uint64_t    discontinuity:1;
        uint64_t    address     : 33;
        uint64_t    length      : 8;
        uint64_t    source      : 4;
        uint64_t    dest        : 5;
        cvmx_tra_data_type_t type:4;
        uint64_t    timestamp   : 8;
    } iobdma; /**< for IOBDMA */
} cvmx_tra_data_t;


/**
 * Setup the TRA buffer for use
 *
 * @param control TRA control setup
 * @param filter  Which events to log
 * @param source_filter
 *                Source match
 * @param dest_filter
 *                Destination match
 * @param address Address compare
 * @param address_mask
 *                Address mask
 */
static inline void cvmx_tra_setup(cvmx_tra_ctl_t control, cvmx_tra_filt_cmd_t filter,
                                  cvmx_tra_filt_sid_t source_filter, uint64_t dest_filter,
                                  uint64_t address, uint64_t address_mask)
{
    cvmx_write_csr(CVMX_TRA_CTL,            control.u64);
    cvmx_write_csr(CVMX_TRA_FILT_CMD,       filter.u64);
    cvmx_write_csr(CVMX_TRA_FILT_SID,       source_filter.u64);
    cvmx_write_csr(CVMX_TRA_FILT_DID,       dest_filter);
    cvmx_write_csr(CVMX_TRA_FILT_ADR_ADR,   address);
    cvmx_write_csr(CVMX_TRA_FILT_ADR_MSK,   address_mask);
}


/**
 * Setup a TRA trigger. How the triggers are used should be
 * setup using cvmx_tra_setup.
 *
 * @param trigger Trigger to setup (0 or 1)
 * @param filter  Which types of events to trigger on
 * @param source_filter
 *                Source trigger match
 * @param dest_filter
 *                Destination trigger match
 * @param address Trigger address compare
 * @param address_mask
 *                Trigger address mask
 */
static inline void cvmx_tra_trig_setup(uint64_t trigger, cvmx_tra_filt_cmd_t filter,
                                        cvmx_tra_filt_sid_t source_filter, uint64_t dest_filter,
                                        uint64_t address, uint64_t address_mask)
{
    cvmx_write_csr(CVMX_TRA_TRIG0_CMD + trigger * 64,       filter.u64);
    cvmx_write_csr(CVMX_TRA_TRIG0_SID + trigger * 64,       source_filter.u64);
    cvmx_write_csr(CVMX_TRA_TRIG0_DID + trigger * 64,       dest_filter);
    cvmx_write_csr(CVMX_TRA_TRIG0_ADR_ADR + trigger * 64,   address);
    cvmx_write_csr(CVMX_TRA_TRIG0_ADR_MSK + trigger * 64,   address_mask);
}


/**
 * Global variable used to remember when the TRA buffer was
 * started. This is used as a base for timestamps.
 */
extern uint64_t cvmx_tra_start_cycle;


/**
 * Enable or disable the TRA hardware
 *
 * @param enable 1=enable, 0=disable
 */
static inline void cvmx_tra_enable(int enable)
{
    cvmx_tra_ctl_t control;
    control.u64 = cvmx_read_csr(CVMX_TRA_CTL);
    control.s.ena = enable;
    cvmx_write_csr(CVMX_TRA_CTL, control.u64);
    if (enable)
        cvmx_tra_start_cycle = cvmx_get_cycle();
}


/**
 * Read an entry from the TRA buffer
 *
 * @return Value return. High bit will be zero if there wasn't any data
 */
static inline cvmx_tra_data_t cvmx_tra_read(void)
{
    cvmx_tra_data_t result;
    result.u64 = cvmx_read_csr(CVMX_TRA_READ_DAT);
    return result;
}

#ifdef	__cplusplus
}
#endif

#endif

