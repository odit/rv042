/****************************************************************
 * Copyright (c) 2005, Cavium Networks. All rights reserved.
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
 * Function and structure definitions for random number generator hardware
 *
 * $Id: cvmx-rng.h 2 2007-04-05 08:51:12Z tt $ $Name$
 */


#ifndef __CMVX_RNG_H__
#define __CMVX_RNG_H__

#ifdef	__cplusplus
extern "C" {
#endif

#define CVMX_RNG_LOAD_ADDRESS   CVMX_ADD_IO_SEG(cvmx_build_io_address(CVMX_OCT_DID_RNG, 0))

/**
 * Structure describing the data format used for IOBDMA stores to the RNG.
 */
typedef union
{
    uint64_t        u64;
    struct {
        uint64_t    scraddr : 8;    /**< the (64-bit word) location in scratchpad to write to (if len != 0) */
        uint64_t    len     : 8;    /**< the number of words in the response (0 => no response) */
        uint64_t    did     : 5;    /**< the ID of the device on the non-coherent bus */
        uint64_t    subdid  : 3;    /**< the sub ID of the device on the non-coherent bus */
        uint64_t    addr    :40;    /**< the address that will appear in the first tick on the NCB bus */
    } s;
} cvmx_rng_iobdma_data_t;

/**
 * Enables the random number generator. Must be called before RNG is used
 */
static inline void cvmx_rng_enable(void)
{
    cvmx_write_csr(CVMX_RNM_CTL_STATUS, 0x3ull);
}
/**
 * Reads 8 bits of random data from Random number generator
 *
 * @return random data
 */
static inline uint8_t cvmx_rng_get_random8(void)
{
    return cvmx_read64_uint8(CVMX_RNG_LOAD_ADDRESS);
}

/**
 * Reads 16 bits of random data from Random number generator
 *
 * @return random data
 */
static inline uint16_t cvmx_rng_get_random16(void)
{
    return cvmx_read64_uint16(CVMX_RNG_LOAD_ADDRESS);
}

/**
 * Reads 32 bits of random data from Random number generator
 *
 * @return random data
 */
static inline uint32_t cvmx_rng_get_random32(void)
{
    return cvmx_read64_uint32(CVMX_RNG_LOAD_ADDRESS);
}

/**
 * Reads 64 bits of random data from Random number generator
 *
 * @return random data
 */
static inline uint64_t cvmx_rng_get_random64(void)
{
    return cvmx_read64_uint64(CVMX_RNG_LOAD_ADDRESS);
}

/**
 * Requests random data from the RNG block asynchronously using and IOBDMA operation.
 * The random data will be written into the cores
 * local memory at the specified address.  A SYNCIOBDMA
 * operation should be issued to stall for completion of the write.
 *
 * @param scr_addr  Address in scratch memory to put the result
 *                  MUST be a multiple of 8 bytes
 * @param num_bytes Number of bytes of random data to write at
 *                  scr_addr
 *                  MUST be a multiple of 8 bytes
 *
 * @return 0 on success
 *         1 on error
 */
static inline int cvmx_rng_request_random_async(uint64_t scr_addr, uint64_t num_bytes)
{
    if (num_bytes & 0x7 || scr_addr & 0x7)
        return(1);

    cvmx_rng_iobdma_data_t data;

    /* scr_addr must be 8 byte aligned */
    data.s.scraddr = scr_addr >> 3;
    data.s.len = num_bytes >> 3;
    data.s.did = CVMX_OCT_DID_RNG;
    cvmx_send_single(data.u64);
    return(0);
}

#ifdef	__cplusplus
}
#endif

#endif /*  __CMVX_RNG_H__  */
