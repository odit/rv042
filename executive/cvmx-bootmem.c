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
 * Simple allocate only memory allocator.  Used to allocate memory at application
 * start time.
 *
 * $Id: cvmx-bootmem.c 2 2007-04-05 08:51:12Z tt $
 *
 */


#if defined(__linux__)
    #if defined(__KERNEL__)
        #include <linux/kernel.h>
        #define printf printk
    #else
        #include <stdio.h>
        #include <fcntl.h>
        #include <unistd.h>
        #include <sys/stat.h>
        #include <sys/types.h>
        #include <sys/mman.h>
    #endif
#else
    #include <stdio.h>
#endif

#include "cvmx-config.h"
#include "cvmx.h"
#include "cvmx-abi.h"
#include "cvmx-spinlock.h"
#include "cvmx-bootmem.h"
#include "cvmx-bootmem-shared.h"


/* Offsets of data elements in bootmem list, must match cvmx_bootmem_block_header_t */
#define NEXT_OFFSET 0
#define SIZE_OFFSET 8


#undef	MAX
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))

#undef	MIN
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))

static cvmx_bootmem_desc_t *cvmx_bootmem_desc = NULL;

/* See header file for descriptions of functions */

/* Some wrapper functions to provide the the same names as used
** in the bootloader
*/
void octeon_write64(uint64_t addr, uint64_t val)
{
    cvmx_write64(octeon_xkphys(addr), val);
}
uint64_t octeon_read64(uint64_t addr)
{
    return cvmx_read64(octeon_xkphys(addr));
}

void octeon_phy_mem_set_size(uint64_t addr, uint64_t size)
{
    octeon_write64(addr + SIZE_OFFSET, size);
}
void octeon_phy_mem_set_next(uint64_t addr, uint64_t next)
{
    octeon_write64(addr + NEXT_OFFSET, next);
}
uint64_t octeon_phy_mem_get_size(uint64_t addr)
{
    return(octeon_read64(addr + SIZE_OFFSET));
}
uint64_t octeon_phy_mem_get_next(uint64_t addr)
{
    return(octeon_read64(addr + NEXT_OFFSET));
}

void *cvmx_bootmem_alloc_range(uint64_t size, uint64_t alignment, uint64_t min_addr, uint64_t max_addr)
{
    uint64_t address;
    octeon_lock(CAST64(&(cvmx_bootmem_desc->lock)));
    address = octeon_phy_mem_block_alloc(cvmx_bootmem_desc, size, min_addr, max_addr, alignment);
    octeon_unlock(CAST64(&(cvmx_bootmem_desc->lock)));

    if (address)
        return cvmx_phys_to_ptr(address);
    else
        return NULL;
}

void *cvmx_bootmem_alloc_address(uint64_t size, uint64_t address, uint64_t alignment)
{
    return cvmx_bootmem_alloc_range(size, alignment, address, address + size);
}


void *cvmx_bootmem_alloc(uint64_t size, uint64_t alignment)
{
    return cvmx_bootmem_alloc_range(size, alignment, 0, 0);
}

void *cvmx_bootmem_alloc_named_range(uint64_t size, uint64_t min_addr, uint64_t max_addr, uint64_t align, char *name)
{
    uint64_t addr;

    addr = octeon_phy_mem_named_block_alloc(cvmx_bootmem_desc, size, min_addr, max_addr, align, name);
    if (addr)
        return cvmx_phys_to_ptr(addr);
    else
        return NULL;

}
void *cvmx_bootmem_alloc_named_address(uint64_t size, uint64_t address, char *name)
{
    return(cvmx_bootmem_alloc_named_range(size, address, address + size, 0, name));
}
void *cvmx_bootmem_alloc_named(uint64_t size, uint64_t alignment, char *name)
{
    return(cvmx_bootmem_alloc_named_range(size, 0, 0, alignment, name));
}

int cvmx_bootmem_free_named(char *name)
{
    return(octeon_phy_mem_named_block_free(cvmx_bootmem_desc, name));
}

cvmx_bootmem_named_block_desc_t * cvmx_bootmem_find_named_block(char *name)
{
    return(octeon_phy_mem_named_block_find(cvmx_bootmem_desc, name));
}

#if defined(__linux__) && defined(CVMX_ABI_N32)
cvmx_bootmem_named_block_desc_t *linux32_named_block_array_ptr;
#endif

int cvmx_bootmem_init(void *mem_desc_ptr)
{
    /* Here we set the global pointer to the bootmem descriptor block.  This pointer will
    ** be used directly, so we will set it up to be directly usable by the application.
    ** It is set up as follows for the various runtime/ABI combinations:
    ** Linux 64 bit: Set XKPHYS bit
    ** Linux 32 bit: use mmap to create mapping, use virtual address
    ** CVMX 64 bit:  use physical address directly
    ** CVMX 32 bit:  use physical address directly
    ** Note that the CVMX environment assumes the use of 1-1 TLB mappings so that the physical addresses
    ** can be used directly
    */
    if (!cvmx_bootmem_desc)
    {
#if defined(__linux__) && defined(CVMX_ABI_N32)
        /* For 32 bit, we need to use mmap to create a mapping for the bootmem descriptor */
        int dm_fd = open("/dev/mem", O_RDWR);
        if (dm_fd < 0)
        {
            cvmx_dprintf("ERROR opening /dev/mem for boot descriptor mapping\n");
            return(-1);
        }

        void *base_ptr = mmap(NULL, 
                              sizeof(cvmx_bootmem_desc_t) + sysconf(_SC_PAGESIZE), 
                              PROT_READ | PROT_WRITE,
                              MAP_SHARED, 
                              dm_fd, 
                              ((off_t)mem_desc_ptr) & ~(sysconf(_SC_PAGESIZE) - 1));

        if (MAP_FAILED == base_ptr)
        {
            cvmx_dprintf("Error mapping bootmem descriptor!\n");
            close(dm_fd);
            return(-1);
        }

        /* Adjust pointer to point to bootmem_descriptor, rather than start of page it is in */
        cvmx_bootmem_desc =  base_ptr + (((off_t)mem_desc_ptr) & (sysconf(_SC_PAGESIZE) - 1));

        /* Also setup mapping for named memory block desc. while we are at it.  Here we must keep another
        ** pointer around, as the value in the bootmem descriptor is shared with other applications. */
        base_ptr = mmap(NULL, 
                              sizeof(cvmx_bootmem_named_block_desc_t) * cvmx_bootmem_desc->named_block_num_blocks + sysconf(_SC_PAGESIZE), 
                              PROT_READ | PROT_WRITE,
                              MAP_SHARED, 
                              dm_fd, 
                              ((off_t)cvmx_bootmem_desc->named_block_array_addr) & ~(sysconf(_SC_PAGESIZE) - 1));


        close(dm_fd);

        if (MAP_FAILED == base_ptr)
        {
            cvmx_dprintf("Error mapping named block descriptor!\n");
            return(-1);
        }

        /* Adjust pointer to point to named block array, rather than start of page it is in */
        linux32_named_block_array_ptr =  base_ptr + (((off_t)cvmx_bootmem_desc->named_block_array_addr) & (sysconf(_SC_PAGESIZE) - 1));


#elif defined(__linux__) && defined(CVMX_ABI_64)
        /* Set XKPHYS bit */
        cvmx_bootmem_desc = cvmx_phys_to_ptr(CAST64(mem_desc_ptr));

#else
        /* Not linux, just copy pointer */
        cvmx_bootmem_desc = mem_desc_ptr;
#endif
    }


    return(0);
}


uint64_t cvmx_bootmem_available_mem(uint64_t min_block_size)
{
    return(octeon_phy_mem_list_available_mem(cvmx_bootmem_desc, min_block_size));
}
