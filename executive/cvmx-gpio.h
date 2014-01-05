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
 * General Purpose IO interface.
 *
 * $Id: cvmx-gpio.h 2 2007-04-05 08:51:12Z tt $ $Name$
 */
#include "cvmx-csr.h"

#ifndef __CVMX_GPIO_H__
#define __CVMX_GPIO_H__

#ifdef	__cplusplus
extern "C" {
#endif

/* CSR typedefs have been moved to cvmx-csr-*.h */

/**
 * Clear the interrupt rising edge detector for the supplied pins in the mask
 *
 * @param clear_mask Mask of pins to clear
 */
static inline void cvmx_gpio_interrupt_clear(uint16_t clear_mask)
{
    cvmx_write_csr(CVMX_GPIO_INT_CLR, clear_mask);
}


/**
 * GPIO Read Data
 *
 * @return Status of the GPIO pins
 */
static inline uint16_t cvmx_gpio_read(void)
{
    return cvmx_read_csr(CVMX_GPIO_RX_DAT);
}


/**
 * GPIO Clear pin
 *
 * @param clear_mask Bit mask to indicate which bits to drive to '0'.
 */
static inline void cvmx_gpio_clear(uint16_t clear_mask)
{
    cvmx_write_csr(CVMX_GPIO_TX_CLR, clear_mask);
}


/**
 * GPIO Set pin
 *
 * @param set_mask Bit mask to indicate which bits to drive to '1'.
 */
static inline void cvmx_gpio_set(uint16_t set_mask)
{
    cvmx_write_csr(CVMX_GPIO_TX_SET, set_mask);
}

#ifdef	__cplusplus
}
#endif

#endif
