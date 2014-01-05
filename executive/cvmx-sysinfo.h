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
 * This module provides system/board information obtained by the bootloader.
 *
 * $Id: cvmx-sysinfo.h 2 2007-04-05 08:51:12Z tt $ $Name$
 *
 */


#ifndef __CVMX_SYSINFO_H__
#define __CVMX_SYSINFO_H__

#include "open-license/cvmx-app-init.h"

#ifdef	__cplusplus
extern "C" {
#endif

#define OCTEON_SERIAL_LEN 20
/**
 * Structure describing application specific information.
 * __cvmx_app_init() populates this from the cvmx boot descriptor.
 * This structure is private to simple executive applications, so 
 * no versioning is required.
 */
typedef struct {
    /* System wide variables */
    uint64_t system_dram_size;  /**< installed DRAM in system, in bytes */
    void *phy_mem_desc_ptr;  /**< ptr to memory descriptor block */

    /* Application image specific variables */
    uint64_t stack_top;  /**< stack top address (virtual) */
    uint64_t heap_base;  /**< heap base address (virtual) */
    uint32_t stack_size; /**< stack size in bytes */
    uint32_t heap_size;  /**< heap size in bytes */
    uint32_t core_mask;  /**< coremask defining cores running application */
    uint32_t init_core;  /**< Deprecated, use cvmx_coremask_first_core() to select init core */
    uint64_t exception_base_addr;  /**< exception base address, as set by bootloader */
    uint32_t cpu_clock_hz;     /**< cpu clock speed in hz */
    uint32_t dram_data_rate_hz;  /**< dram data rate in hz (data rate = 2 * clock rate */

    uint32_t spi_clock_hz;  /**< SPI4 clock in hz */
    uint16_t board_type;
    uint8_t  board_rev_major;
    uint8_t  board_rev_minor;
    uint16_t chip_type;
    uint8_t  chip_rev_major;
    uint8_t  chip_rev_minor;
    uint8_t  mac_addr_base[6];
    uint8_t  mac_addr_count;
    char     board_serial_number[OCTEON_SERIAL_LEN];
    /* Several boards support compact flash on the Octeon boot bus.  The CF
    ** memory spaces may be mapped to different addresses on different boards.
    ** These values will be 0 if CF is not present.  
    ** Note that these addresses are physical addresses, and it is up to the application
    ** to use the proper addressing mode (XKPHYS, KSEG0, etc.)*/
    uint64_t compact_flash_common_base_addr;
    uint64_t compact_flash_attribute_base_addr;
    /* Base address of the LED display (as on EBT3000 board)
    ** This will be 0 if LED display not present. 
    ** Note that this address is a physical address, and it is up to the application
    ** to use the proper addressing mode (XKPHYS, KSEG0, etc.)*/
    uint64_t led_display_base_addr;
    uint32_t dfa_ref_clock_hz;  /**< DFA reference clock in hz (if applicable)*/
    uint32_t bootloader_config_flags;  /**< configuration flags from bootloader */
} cvmx_sysinfo_t;


/**
 * This function returns the system/board information as obtained
 * by the bootloader.
 *
 *
 * @return  Pointer to the boot information structure
 *
 */

extern cvmx_sysinfo_t * cvmx_sysinfo_get(void);

#ifdef	__cplusplus
}
#endif

#endif /* __CVMX_SYSINFO_H__ */
