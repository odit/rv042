/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2005 Cavium Networks, Inc.
 */
#include <linux/config.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/bitops.h>

#include <asm/bcache.h>
#include <asm/bootinfo.h>
#include <asm/cacheops.h>
#include <asm/cpu.h>
#include <asm/cpu-features.h>
#include <asm/io.h>
#include <asm/page.h>
#include <asm/pgtable.h>
#include <asm/r4kcache.h>
#include <asm/system.h>
#include <asm/mmu_context.h>
#include <asm/war.h>
#include "../cavium-octeon/hal.h"

unsigned long long cache_err_dcache[NR_CPUS];

/**
 * Octeon automatically flushes the dcache on tlb changes, so
 * from Linux's viewpoint it acts much like a physically
 * tagged cache. No flushing is needed
 *
 * @param addr
 */
static void octeon_flush_data_cache_page(unsigned long addr)
{
    /* Nothing to do */
}


/**
 * Flush caches as necessary for all cores affected by a
 * vma. If no vma is supplied, all cores are flushed.
 *
 * @param vma    VMA to flush or NULL to flush all icaches.
 */
static void octeon_flush_icache_all_cores(struct vm_area_struct *vma)
{
#ifdef CONFIG_SMP
    int i;
    int cpu;
#endif
    preempt_disable();
#ifdef CONFIG_SMP
    cpu = smp_processor_id();
#endif
    mb();

    /* If we have a vma structure, we only need to worry about cores it
        has been used on */
    if (vma)
    {
        /* Octeon dcache does enough magic we don't need to worry about it.
            Only flush the icache if this vma was executable */
        if (vma->vm_flags & (VM_EXEC|VM_MAYEXEC|VM_EXECUTABLE))
        {
#ifdef CONFIG_SMP
            for (i = 0; i < NR_CPUS; i++)
                if (cpu_isset(i, vma->vm_mm->cpu_vm_mask) && i != cpu)
                    core_send_ipi(i, SMP_ICACHE_FLUSH);
#endif
            asm volatile ("synci 0($0)\n");
        }
    }
    else
    {
        /* No extra info available. Flush the icache on all cores that
            are online */
#ifdef CONFIG_SMP
        for (i = 0; i < NR_CPUS; i++)
            if (cpu_online(i) && i != cpu)
                core_send_ipi(i, SMP_ICACHE_FLUSH);
#endif
        asm volatile ("synci 0($0)\n");
    }
    preempt_enable();
}


/**
 * Called to flush the icache on all cores
 */
static void octeon_flush_icache_all(void)
{
	octeon_flush_icache_all_cores(NULL);
}


/**
 * Called to flush all memory associated with a memory
 * context. Will flush all cores that the memory context
 * was used on.
 *
 * @param mm     Memory context to flush
 */
static void octeon_flush_cache_mm(struct mm_struct *mm)
{
#ifdef CONFIG_SMP
    int i;
    int cpu;
    mb();
    preempt_disable();
    cpu = smp_processor_id();
    for (i = 0; i < NR_CPUS; i++)
        if (cpu_online(i) && i != cpu)
            core_send_ipi(i, SMP_ICACHE_FLUSH);
    preempt_enable();
#else
    mb();
#endif
    asm volatile ("synci 0($0)\n");
}


/**
 * Flush a range of kernel addresses out of the icache
 *
 * @param start
 * @param end
 */
static void octeon_flush_icache_range(unsigned long start, unsigned long end)
{
	octeon_flush_icache_all_cores(NULL);
}


/**
 * Flush a specific page of a vma
 *
 * @param vma    VMA to flush page for
 * @param page   Page to flush
 */
static void octeon_flush_icache_page(struct vm_area_struct *vma, struct page *page)
{
    octeon_flush_icache_all_cores(vma);
}


/**
 * Flush the icache for a trampoline. These are used for interrupt
 * and exception hooking.
 *
 * @param addr   Address to flush
 */
static void octeon_flush_cache_sigtramp(unsigned long addr)
{
    /* Only flush trampolines on the current core. Assume trampoline fits
        in a cache line. */
    mb();
    asm volatile ("synci 0(%0)\n":: "r" (addr));
}


/**
 * Flush a range out of a vma
 *
 * @param vma    VMA to flush
 * @param start
 * @param end
 */
static void octeon_flush_cache_range(struct vm_area_struct *vma, unsigned long start, unsigned long end)
{
    octeon_flush_icache_all_cores(vma);
}


/**
 * Flush a specific page of a vma
 *
 * @param vma    VMA to flush page for
 * @param page   Page to flush
 * @param pfn
 */
static void octeon_flush_cache_page(struct vm_area_struct *vma, unsigned long page, unsigned long pfn)
{
    octeon_flush_icache_all_cores(vma);
}


/**
 * Probe Octeon's caches
 *
 * @return
 */
static void __init probe_octeon(void)
{
    unsigned long icache_size;
    unsigned long dcache_size;
    unsigned int config1;
    struct cpuinfo_mips *c = &current_cpu_data;

    switch (c->cputype)
    {
        case CPU_CAVIUM_CN38XX:
        case CPU_CAVIUM_CN31XX:
        case CPU_CAVIUM_CN30XX:
        case CPU_CAVIUM_CN50XX:
        case CPU_CAVIUM_CN58XX:
        case CPU_CAVIUM_CN56XX:
            config1 = read_c0_config1();
            c->icache.linesz = 2 << ((config1 >> 19) & 7);
            c->icache.sets = 64 << ((config1 >> 22) & 7);
            c->icache.ways = 1 + ((config1 >> 16) & 7);
            c->icache.flags |= MIPS_CACHE_VTAG;
            icache_size = c->icache.sets * c->icache.ways * c->icache.linesz;
            c->icache.waybit = ffs(icache_size / c->icache.ways) - 1;
            c->dcache.linesz = 128;
            if ((c->cputype == CPU_CAVIUM_CN58XX) || (c->cputype == CPU_CAVIUM_CN56XX)) /* CN5XXX has two Dcache sets */
                c->dcache.sets = 2;
            else
                c->dcache.sets = 1;
            c->dcache.ways = 64;
            dcache_size = c->dcache.sets * c->dcache.ways * c->dcache.linesz;
            c->dcache.waybit = ffs(dcache_size / c->dcache.ways) - 1;
            c->options |= MIPS_CPU_PREFETCH;
            break;

        default:
            panic("Unsupported Cavium Networks CPU type\n");
            break;
    }

    /* compute a couple of other cache variables */
    c->icache.waysize = icache_size / c->icache.ways;
    c->dcache.waysize = dcache_size / c->dcache.ways;

    c->icache.sets = icache_size / (c->icache.linesz * c->icache.ways);
    c->dcache.sets = dcache_size / (c->dcache.linesz * c->dcache.ways);

    if (smp_processor_id() == 0)
    {
        printk("Primary instruction cache %ldkB, %s, %d way, %d sets, linesize %d bytes.\n",
               icache_size >> 10,
               cpu_has_vtag_icache ? "virtually tagged" : "physically tagged",
               c->icache.ways, c->icache.sets, c->icache.linesz);

        printk("Primary data cache %ldkB, %d-way, %d sets, linesize %d bytes.\n",
               dcache_size >> 10, c->dcache.ways, c->dcache.sets, c->dcache.linesz);
    }
}


/**
 * Setup the Octeon cache flush routines
 *
 * @return
 */
void __init octeon_cache_init(void)
{
    extern unsigned long ebase;
    extern char except_vec2_octeon;

    memcpy((void *)(ebase + 0x100), &except_vec2_octeon, 0x80);
    octeon_flush_cache_sigtramp(ebase + 0x100);

    probe_octeon();

    shm_align_mask = PAGE_SIZE - 1;

    flush_cache_all         = octeon_flush_icache_all;
    __flush_cache_all       = octeon_flush_icache_all;
    flush_cache_mm          = octeon_flush_cache_mm;
    flush_cache_page        = octeon_flush_cache_page;
    flush_icache_page       = octeon_flush_icache_page;
    flush_cache_range       = octeon_flush_cache_range;
    flush_cache_sigtramp    = octeon_flush_cache_sigtramp;
    flush_icache_all        = octeon_flush_icache_all;
    flush_data_cache_page   = octeon_flush_data_cache_page;
    flush_icache_range      = octeon_flush_icache_range;
}

/**
 * Handle a cache error exception
 */

static void  cache_parity_error_octeon(int non_recoverable)
{
    unsigned long coreid = octeon_get_core_num();

    printk("Cache error exception:\n");
    printk("cp0_errorepc == %lx\n", read_c0_errorepc());
    printk("CacheErr (Icache) == %llx CacheErr (Dcache) == %llx\n",
           read_c0_cacheerr_icache(), cache_err_dcache[coreid]);

    cache_err_dcache[coreid] = 0;

    if (non_recoverable)
        panic("Can't handle cache error: nested exception");
}

/**
 * Called when the the exception is not recoverable
 */

asmlinkage void cache_parity_error_octeon_recoverable(void)
{
    cache_parity_error_octeon(0);
}

/**
 * Called when the the exception is recoverable
 */

asmlinkage void cache_parity_error_octeon_non_recoverable(void)
{
    cache_parity_error_octeon(1);
}

