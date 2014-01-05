/****************************************************************
 * Copyright (c) 2006, Cavium Networks. All rights reserved.
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
 * Interface to the TWSI / I2C bus
 *
 * $Id: cvmx-twsi.c 2 2007-04-05 08:51:12Z tt $ $Name$
 *
 */

#include "cvmx-config.h"
#include "cvmx.h"
#include "cvmx-twsi.h"


/** 
 * Read 8-bit from a device on the TWSI / I2C bus
 *
 * @param dev_addr       I2C device address (7 bit)
 * @param internal_addr  Internal device address
 *
 * @return 8-bit data or < 0 in case of error
 */
int cvmx_twsi_read8(uint8_t dev_addr, uint8_t internal_addr)
{
    /* 8 bit internal address */
    cvmx_mio_tws_sw_twsi_t sw_twsi_val;

    sw_twsi_val.u64 = 0;
    sw_twsi_val.s.v = 1;
    sw_twsi_val.s.a = dev_addr;
    sw_twsi_val.s.d = internal_addr;
    cvmx_write_csr(CVMX_MIO_TWS_SW_TWSI, sw_twsi_val.u64);
    while (((cvmx_mio_tws_sw_twsi_t)(sw_twsi_val.u64 = cvmx_read_csr(CVMX_MIO_TWS_SW_TWSI))).s.v)
        ;
    if (!sw_twsi_val.s.r)
        return -1;

    return cvmx_twsi_read8_cur_addr(dev_addr);
}

/** 
 * Read 8-bit from a device on the TWSI / I2C bus
 * 
 * Uses current internal address
 *
 * @param dev_addr       I2C device address (7 bit)
 *
 * @return 8-bit value or < 0 in case of error
 */
int cvmx_twsi_read8_cur_addr(uint8_t dev_addr)
{
    cvmx_mio_tws_sw_twsi_t sw_twsi_val;

    sw_twsi_val.u64 = 0;
    sw_twsi_val.s.v = 1;
    sw_twsi_val.s.a = dev_addr;
    sw_twsi_val.s.r = 1;

    cvmx_write_csr(CVMX_MIO_TWS_SW_TWSI, sw_twsi_val.u64);
    while (((cvmx_mio_tws_sw_twsi_t)(sw_twsi_val.u64 = cvmx_read_csr(CVMX_MIO_TWS_SW_TWSI))).s.v)
        ;
    if (!sw_twsi_val.s.r)
        return -1;
    else
        return(sw_twsi_val.s.d & 0xFF);
}

/**
 * Write 8-bit to a device on the TWSI / I2C bus
 * 
 * @param dev_addr       I2C device address (7 bit)
 * @param internal_addr  Internal device address
 * @param data           Data to be written
 *
 * @return 0 on success and < 0 in case of error
 */
int cvmx_twsi_write8(uint8_t dev_addr, uint8_t internal_addr, uint8_t data)
{
    /* 8 bit internal address */
    cvmx_mio_tws_sw_twsi_t sw_twsi_val;

    sw_twsi_val.u64 = 0;
    sw_twsi_val.s.v = 1;
    sw_twsi_val.s.op = 1;
    sw_twsi_val.s.a = dev_addr;
    sw_twsi_val.s.ia = internal_addr >> 3;
    sw_twsi_val.s.eop_ia = internal_addr & 0x7;
    sw_twsi_val.s.d = data;

    cvmx_write_csr(CVMX_MIO_TWS_SW_TWSI, sw_twsi_val.u64);
    while (((cvmx_mio_tws_sw_twsi_t)(sw_twsi_val.u64 = cvmx_read_csr(CVMX_MIO_TWS_SW_TWSI))).s.v)
        ;
    if (!sw_twsi_val.s.r)
        return -1;

    return 0;
}


