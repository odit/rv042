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
 * Simple executive application initialization for Linux user space. This
 * file should be used instead of cvmx-app-init.c for running simple executive
 * applications under Linux in userspace. The following are some of the key
 * points to remember when writing applications to run both under the
 * standalone simple executive and userspace under Linux.
 *
 * -# Application main must be called "appmain" under Linux. Use and ifdef
 *      based on __linux__ to determine the proper name.
 * -# Be careful to use cvmx_ptr_to_phys() and cvmx_phys_to_ptr. The simple
 *      executive 1-1 TLB mappings allow you to be sloppy and interchange
 *      hardware addresses with virtual address. This isn't true under Linux.
 * -# If you're talking directly to hardware, be careful. The normal Linux
 *      protections are circumvented. If you do something bad, Linux won't
 *      save you.
 * -# Most hardware can only be initialized once. Unless you're very careful,
 *      this also means you Linux application can only run once.
 *
 * $Id: cvmx-app-init-linux.c 2 2007-04-05 08:51:12Z tt $
 *
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <signal.h>
#include <sys/statfs.h>
#define __USE_GNU
#include <sched.h>
#include <octeon-app-init.h>

#include "cvmx-config.h"
#include "cvmx.h"
#include "cvmx-atomic.h"
#include "cvmx-sysinfo.h"
#include "cvmx-coremask.h"
#include "cvmx-spinlock.h"
#include "cvmx-bootmem.h"
#include "octeon-model.h"

int octeon_model_version_check(uint32_t chip_id);

#define OCTEON_ECLOCK_MULT_INPUT_X16    ((int)(33.4*16))

/* Applications using the simple executive libraries under Linux userspace must
    rename their "main" function to match the prototype below. This allows the
    simple executive to perform needed memory initialization and process
    creation before the application runs. */
extern int appmain(int argc, const char *argv[]);

/* These two external addresses provide the beginning and end markers for the
    CVMX_SHARED section. These are defined by the cvmx-shared.ld linker script.
    If they aren't defined, you probably forgot to link using this script. */
extern void __cvmx_shared_start;
extern void __cvmx_shared_end;

/* Global variables that define the min/max of the memory region set up for 32 bit userspace access */
uint64_t linux_mem32_min = 0;
uint64_t linux_mem32_max = 0;
uint64_t linux_mem32_offset = 0;

/* Global variable with the processor ID since we can't read it directly */
uint32_t cvmx_app_init_processor_id;

/**
 * This function performs some default initialization of the Octeon executive.  It initializes
 * the cvmx_bootmem memory allocator with the list of physical memory shared by the bootloader.
 * This function should be called on all cores that will use the bootmem allocator.
 * Applications which require a different configuration can replace this function with a suitable application
 * specific one.
 *
 * @return 0 on success
 *         -1 on failure
 */
int cvmx_user_app_init(void)
{
    return cvmx_bootmem_init(cvmx_sysinfo_get()->phy_mem_desc_ptr);
}


/**
 * Simulator magic is not supported in user mode under Linux.
 * This version of simprintf simply calls the underlying C
 * library printf for output. It also makes sure that two
 * calls to simprintf provide atomic output.
 *
 * @param fmt    Format string in the same format as printf.
 */
void simprintf(const char *fmt, ...)
{
    CVMX_SHARED static cvmx_spinlock_t simprintf_lock = CVMX_SPINLOCK_UNLOCKED_INITIALIZER;
    va_list ap;

    cvmx_spinlock_lock(&simprintf_lock);
    printf("SIMPRINTF(%d): ", (int)cvmx_get_core_num());
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
    cvmx_spinlock_unlock(&simprintf_lock);
}


/**
 * Setup the CVMX_SHARED data section to be shared across
 * all processors running this application. A memory mapped
 * region is allocated using shm_open and mmap. The current
 * contents of the CVMX_SHARED section are copied into the
 * region. Then the new region is remapped to replace the
 * existing CVMX_SHARED data.
 *
 * This function will display a message and abort the
 * application under any error conditions. The Linux tmpfs
 * filesystem must be mounted under /dev/shm.
 */
static void setup_cvmx_shared(void)
{
    const char *SHM_NAME = "cvmx_shared";
    unsigned long shared_size = &__cvmx_shared_end - &__cvmx_shared_start;
    int fd;

    /* If there isn't and shared data we can skip all this */
    if (shared_size)
    {
        printf("CVMX_SHARED: %p-%p\n", &__cvmx_shared_start, &__cvmx_shared_end);

#ifdef __UCLIBC__
	const char *defaultdir = "/dev/shm/";
	struct statfs f;
	char shm_name[30];
	int pid;
	/* The canonical place is /dev/shm. */ 
	if (statfs (defaultdir, &f) == 0)
	{
	    pid = getpid();
	    sprintf (shm_name, "%s%s-%d", defaultdir, SHM_NAME, pid);
	}
	else
	{
	    perror("/dev/shm is not mounted");
	    exit(-1);
	}

	/* shm_open(), shm_unlink() are not implemented in uClibc. Do the 
	   same thing using open() and close() system calls.  */
	fd = open (shm_name, O_RDWR | O_CREAT | O_TRUNC, 0);

	if (fd < 0)
	{
	    perror("Failed to open CVMX_SHARED(shm_name)");
	    exit(errno);
	}
	
	unlink (shm_name);
#else
        /* Open a new shared memory region for use as CVMX_SHARED */
        fd = shm_open(SHM_NAME, O_RDWR | O_CREAT | O_TRUNC, 0);
        if (fd <0)
        {
            perror("Failed to setup CVMX_SHARED(shm_open)");
            exit(errno);
        }

        /* We don't want the file on the filesystem. Immediately unlink it so
            another application can create its own shared region */
        shm_unlink(SHM_NAME);
#endif

        /* Resize the region to match the size of CVMX_SHARED */
        ftruncate(fd, shared_size);

        /* Map the region into some random location temporarily so we can
            copy the shared data to it */
        void *ptr = mmap(NULL, shared_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (ptr == NULL)
        {
            perror("Failed to setup CVMX_SHARED(mmap copy)");
            exit(errno);
        }

        /* Copy CVMX_SHARED to the new shared region so we don't lose
            initializers */
        memcpy(ptr, &__cvmx_shared_start, shared_size);
        munmap(ptr, shared_size);

        /* Remap the shared region to replace the old CVMX_SHARED region */
        ptr = mmap(&__cvmx_shared_start, shared_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED, fd, 0);
        if (ptr == NULL)
        {
            perror("Failed to setup CVMX_SHARED(mmap final)");
            exit(errno);
        }

        /* Once mappings are setup, the file handle isn't needed anymore */
        close(fd);
    }
}


/**
 * Shutdown and free the shared CVMX_SHARED region setup by
 * setup_cvmx_shared.
 */
static void shutdown_cvmx_shared(void)
{
    unsigned long shared_size = &__cvmx_shared_end - &__cvmx_shared_start;
    if (shared_size)
        munmap(&__cvmx_shared_start, shared_size);
}


/**
 * Setup access to the CONFIG_CAVIUM_RESERVE32 memory section
 * created by the kernel. This memory is used for shared
 * hardware buffers with 32 bit userspace applications.
 */
static void setup_reserve32(void)
{
    if (linux_mem32_min && linux_mem32_max)
    {
        int fd = open("/dev/mem", O_RDWR);
        if (fd < 0)
        {
            perror("ERROR opening /dev/mem");
            exit(-1);
        }

        void *linux_mem32_base_ptr = mmap(NULL,
                                          linux_mem32_max - linux_mem32_min,
                                          PROT_READ | PROT_WRITE,
                                          MAP_SHARED,
                                          fd,
                                          linux_mem32_min);

        close(fd);

        if (MAP_FAILED == linux_mem32_base_ptr)
        {
            perror("Error mapping reserve32");
            exit(-1);
        }

        linux_mem32_offset = CAST64(linux_mem32_base_ptr) - linux_mem32_min;
    }
}


static void setup_system_info(void)
{
    cvmx_sysinfo_t *system_info = cvmx_sysinfo_get();
    memset(system_info, 0, sizeof(cvmx_sysinfo_t));

    system_info->core_mask = 0;
    system_info->init_core = cvmx_get_core_num();

    FILE *infile = fopen("/proc/octeon_info", "r");
    if (infile == NULL)
    {
        perror("Error opening /proc/octeon_info");
        exit(-1);
    }

    while (!feof(infile))
    {
        char buffer[80];
        if (fgets(buffer, sizeof(buffer), infile))
        {
            const char *field = strtok(buffer, " ");
            const char *valueS = strtok(NULL, " ");
            if (field == NULL)
                continue;
            if (valueS == NULL)
                continue;
            unsigned long long value;
            sscanf(valueS, "%lli", &value);

            if (strcmp(field, "dram_size:") == 0)
                system_info->system_dram_size = value;
            else if (strcmp(field, "phy_mem_desc_addr:") == 0)
                system_info->phy_mem_desc_ptr = cvmx_phys_to_ptr(value);
            else if (strcmp(field, "eclock_hz:") == 0)
                system_info->cpu_clock_hz = value;
            else if (strcmp(field, "dclock_hz:") == 0)
                system_info->dram_data_rate_hz = value * 2;
            else if (strcmp(field, "spi_clock_hz:") == 0)
                system_info->spi_clock_hz = value;
            else if (strcmp(field, "board_type:") == 0)
                system_info->board_type = value;
            else if (strcmp(field, "board_rev_major:") == 0)
                system_info->board_rev_major = value;
            else if (strcmp(field, "board_rev_minor:") == 0)
                system_info->board_rev_minor = value;
            else if (strcmp(field, "chip_type:") == 0)
                system_info->chip_type = value;
            else if (strcmp(field, "chip_rev_major:") == 0)
                system_info->chip_rev_major = value;
            else if (strcmp(field, "chip_rev_minor:") == 0)
                system_info->chip_rev_minor = value;
            else if (strcmp(field, "board_serial_number:") == 0)
                strncpy(system_info->board_serial_number, valueS, sizeof(system_info->board_serial_number));
            else if (strcmp(field, "mac_addr_base:") == 0)
            {
                int i;
                int m[6];
                sscanf(valueS, "%02x:%02x:%02x:%02x:%02x:%02x", m+0, m+1, m+2, m+3, m+4, m+5);
                for (i=0; i<6; i++)
                    system_info->mac_addr_base[i] = m[i];
            }
            else if (strcmp(field, "mac_addr_count:") == 0)
                system_info->mac_addr_count = value;
            else if (strcmp(field, "32bit_shared_mem_base:") == 0)
                linux_mem32_min = value;
            else if (strcmp(field, "32bit_shared_mem_size:") == 0)
                linux_mem32_max = linux_mem32_min + value - 1;
            else if (strcmp(field, "processor_id:") == 0)
                cvmx_app_init_processor_id = value;
        }
    }


#if 0
    cvmx_dprintf("system_dram_size:       %llu\n", (unsigned long long)system_info->system_dram_size);
    cvmx_dprintf("phy_mem_desc_ptr:       %p\n", system_info->phy_mem_desc_ptr);
    cvmx_dprintf("init_core:              %u\n", system_info->init_core);
    cvmx_dprintf("cpu_clock_hz:           %u\n", system_info->cpu_clock_hz);
    cvmx_dprintf("dram_data_rate_hz:      %u\n", system_info->dram_data_rate_hz);
    cvmx_dprintf("spi_clock_hz:           %u\n", system_info->spi_clock_hz);
    cvmx_dprintf("board_type:             %u\n", system_info->board_type);
    cvmx_dprintf("board_rev_major:        %u\n", system_info->board_rev_major);
    cvmx_dprintf("board_rev_minor:        %u\n", system_info->board_rev_minor);
    cvmx_dprintf("chip_type:              %u\n", system_info->chip_type);
    cvmx_dprintf("chip_rev_major:         %u\n", system_info->chip_rev_major);
    cvmx_dprintf("chip_rev_minor:         %u\n", system_info->chip_rev_minor);
    cvmx_dprintf("mac_addr_base:          %02x:%02x:%02x:%02x:%02x:%02x\n",
               (int)system_info->mac_addr_base[0],
               (int)system_info->mac_addr_base[1],
               (int)system_info->mac_addr_base[2],
               (int)system_info->mac_addr_base[3],
               (int)system_info->mac_addr_base[4],
               (int)system_info->mac_addr_base[5]);
    cvmx_dprintf("mac_addr_count:         %u\n", system_info->mac_addr_count);
    cvmx_dprintf("board_serial_number:    %s\n", system_info->board_serial_number);
#endif
}


/**
 * Main entrypoint of the application. Here we setup shared
 * memory and fork processes for each cpu. This simulates the
 * normal simple executive environment of one process per
 * cpu core.
 *
 * @param argc   Number of command line arguments
 * @param argv   The command line arguments
 * @return Return value for the process
 */
int main(int argc, const char *argv[])
{
    CVMX_SHARED static cvmx_spinlock_t mask_lock = CVMX_SPINLOCK_UNLOCKED_INITIALIZER;
    CVMX_SHARED static int32_t pending_fork;
    unsigned long cpumask;
    unsigned long cpu;

    setup_system_info();

    if (sizeof(void*) == 4)
    {
        if (linux_mem32_min)
            setup_reserve32();
        else
        {
            printf("\nFailed to access 32bit shared memory region. Most likely the Kernel\n"
                   "has not been configured for 32bit shared memory access. Check the\n"
                   "kernel configuration.\n"
                   "Aborting...\n\n");
            exit(-1);
        }
    }

    setup_cvmx_shared();

    /* Check to make sure the Chip version matches the configured version */
    octeon_model_version_check(cvmx_app_init_processor_id);

    /* Get the list of logical cpus we should run on */
    if (sched_getaffinity(0, sizeof(cpumask), (cpu_set_t*)&cpumask))
    {
        perror("sched_getaffinity failed");
        exit(errno);
    }

    cvmx_sysinfo_t *system_info = cvmx_sysinfo_get();

    cvmx_atomic_set32(&pending_fork, 1);
    for (cpu=0; cpu<16; cpu++)
    {
        if (cpumask & (1<<cpu))
        {
            /* Turn off the bit for this CPU number. We've counted him */
            cpumask ^= (1<<cpu);
            /* If this is the last CPU to run on, use this process instead of forking another one */
            if (cpumask == 0)
                break;
            /* Increment the number of CPUs running this app */
            cvmx_atomic_add32(&pending_fork, 1);
            /* Fork a process for the new CPU */
            int pid = fork();
            if (pid == 0)
            {
                break;
            }
            else if (pid == -1)
            {
                perror("Fork failed");
                exit(errno);
            }
        }
    }

    /* Set affinity to lock me to the correct CPU */
    cpumask = (1<<cpu);
    if (sched_setaffinity(0, sizeof(cpumask), (cpu_set_t*)&cpumask))
    {
        perror("sched_setaffinity failed");
        exit(errno);
    }

    cvmx_spinlock_lock(&mask_lock);
    system_info->core_mask |= 1<<cvmx_get_core_num();
    cvmx_atomic_add32(&pending_fork, -1);
    if (cvmx_atomic_get32(&pending_fork) == 0)
        cvmx_dprintf("Active coremask = 0x%x\n", system_info->core_mask);
    cvmx_spinlock_unlock(&mask_lock);

    /* Spinning waiting for forks to complete */
    while (cvmx_atomic_get32(&pending_fork)) {}

    cvmx_coremask_barrier_sync(system_info->core_mask);

    int result = appmain(argc, argv);

    shutdown_cvmx_shared();

    return result;
}
