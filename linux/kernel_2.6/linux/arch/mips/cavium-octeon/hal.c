/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2004, 2005 Cavium Networks
 */
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/module.h>
#include <asm/time.h>
#include <octeon-app-init.h>

#ifndef CONFIG_CAVIUM_RESERVE32
#define CONFIG_CAVIUM_RESERVE32 0
#endif

#include "hal.h"

#define CAST64(v) ((long long)(long)(v))
#define CASTPTR(type, v) ((type *)(long)(v))
#define CVMX_MIPS32_SPACE_KSEG0 1l
#define CVMX_ADD_SEG32(segment, add)          (((int32_t)segment << 31) | (int32_t)(add))

#include "cvmx-bootmem-shared.h"
#include "cvmx-app-init.h"

/* Set to non-zero, so it is not in .bss section and is not zeroed */
volatile octeon_boot_descriptor_t *octeon_boot_desc_ptr = (void *)0xEADBEEFULL;
cvmx_bootinfo_t *octeon_bootinfo;

/* This must not be static since inline functions access it */
spinlock_t octeon_led_lock;

#if CONFIG_CAVIUM_RESERVE32
uint64_t octeon_reserve32_memory = 0;
#endif

void octeon_write_lcd(const char *s)
{
    if (octeon_bootinfo->led_display_base_addr)
    {
        volatile char *lcd_address = (volatile char*)((1ull << 63) | octeon_bootinfo->led_display_base_addr);
        int i;
        for (i=0; i<8; i++)
        {
            if (*s)
                lcd_address[i] = *s++;
            else
                lcd_address[i] = ' ';
        }
    }
}

void octeon_check_cpu_bist(void)
{
    const int coreid = octeon_get_core_num();
    uint64_t mask;
    uint64_t bist_val;

    /* Check BIST results for COP0 registers */
    mask     = 0x1f00000000ull;
    bist_val = __read_64bit_c0_register($27,0);
    if (bist_val & mask)
        printk("Core%d BIST Failure: CacheErr(icache) = 0x%lx\n", coreid, bist_val);

    bist_val = __read_64bit_c0_register($27,1);
    if (bist_val & 1)
        printk("Core%d L1 Dcache parity error: CacheErr(dcache) = 0x%lx\n", coreid, bist_val);

    mask     = 0xfc00000000000000ull;
    bist_val = __read_64bit_c0_register($11,7);
    if (bist_val & mask)
        printk("Core%d BIST Failure: COP0_CVM_MEM_CTL = 0x%lx\n", coreid, bist_val);

    __write_64bit_c0_register($27,1,0);

    mask     = 0x18ull;
    bist_val = octeon_read_csr(OCTEON_L2D_ERR);
#ifdef CONFIG_NK_SUPPORT_CN5010
    octeon_write_csr(OCTEON_L2D_ERR, bist_val); /* Clear error bits */
#else
    octeon_write_csr(OCTEON_L2D_ERR, mask); /* Clear error bits */
#endif
    if (bist_val & mask)
        printk("Core%d L2 Parity error: L2D_ERR = 0x%lx\n", coreid, bist_val);
}


/**
 * Return true if Octeon is in PCI Host mode. This means
 * Linux can control the PCI bus.
 *
 * @return Non zero if Octeon in host mode
 */
int octeon_is_pci_host(void)
{
    return (octeon_bootinfo->config_flags & CVMX_BOOTINFO_CFG_FLAG_PCI_HOST);
}


/**
 * Get the clock rate of Octeon
 *
 * @return Clock rate in HZ
 */
uint64_t octeon_get_clock_rate(void)
{

#ifdef CONFIG_CAVIUM_OCTEON_SIMULATOR
    octeon_bootinfo->eclock_hz = 6000000;
#else
    if ((octeon_bootinfo->eclock_hz < 300000000) ||
        (octeon_bootinfo->eclock_hz > 800000000))
    {
        printk("Clock speed from bootloader (%dMhz) is out of range. Assuming 500Mhz\n",
               octeon_bootinfo->eclock_hz/1000000);
        octeon_bootinfo->eclock_hz = 500000000;
    }
#endif

    return octeon_bootinfo->eclock_hz;
}


/**
 * Return the board name as a constant string
 *
 * @return board name
 */
const char *octeon_board_type_string(void)
{
    return cvmx_board_type_to_string(octeon_bootinfo->board_type);
}


/**
 * Return the mapping of PCI device number to IRQ line. Each
 * character in the return string represents the interrupt
 * line for the device at that position. Device 1 maps to the
 * first character, etc. The characters A-D are used for PCI
 * interrupts.
 *
 * @return PCI interrupt mapping
 */
const char *octeon_get_pci_interrupts(void)
{
    /* Returning an empty string causes the interrupts to be routed based
        on the PCI specification. From the PCI spec:

       INTA# of Device Number 0 is connected to IRQW on the system board.
       (Device Number has no significance regarding being located on the system
       board or in a connector.) INTA# of Device Number 1 is connected to IRQX
       on the system board. INTA# of Device Number 2 is connected to IRQY on
       the system board. INTA# of Device Number 3 is connected to IRQZ on the
       system board. The table below describes how each agent's INTx# lines are
       connected to the system board interrupt lines. The following equation
       can be used to determine to which INTx# signal on the system board a
       given device's INTx# line(s) is connected.

        MB = (D + I) MOD 4
        MB = System board Interrupt (IRQW = 0, IRQX = 1, IRQY = 2, and IRQZ = 3)
        D = Device Number
        I = Interrupt Number (INTA# = 0, INTB# = 1, INTC# = 2, and INTD# = 3)
    */
    switch (octeon_bootinfo->board_type)     /* Device ID 1111111111222222222233 */
    {                                        /* 01234567890123456789012345678901 */
        //case CVMX_BOARD_TYPE_NAO38:     return "AAAAAADBAAAAAAAAAAAAAAAAAAAAAAAA";
        case CVMX_BOARD_TYPE_NAO38:     return "AAAAADABAAAAAAAAAAAAAAAAAAAAAAAA"; /* This is really the NAC38 */
        case CVMX_BOARD_TYPE_THUNDER:   return "";
        case CVMX_BOARD_TYPE_EBH3000:   return "";
        case CVMX_BOARD_TYPE_EBH3100:
        case CVMX_BOARD_TYPE_CN3010_EVB_HS5:
        case CVMX_BOARD_TYPE_CN3005_EVB_HS5:
                                        return "AAABAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
        default:                        return "";
    }
}


/**
 * Return the interrupt line for the i8259 in the southbridge
 *
 * @return
 */
int octeon_get_southbridge_interrupt(void)
{
    switch (octeon_bootinfo->board_type)
    {
        case CVMX_BOARD_TYPE_EBH3000:
            return 47; /* PCI INDD */
        default:
            return 39; /* GPIO 15 */
    }
}


/**
 * Get the coremask Linux was booted on.
 *
 * @return Core mask
 */
int octeon_get_boot_coremask(void)
{
    return octeon_boot_desc_ptr->core_mask;
}


/**
 * Return the number of arguments we got from the bootloader
 *
 * @return argc
 */
int octeon_get_boot_num_arguments(void)
{
    return octeon_boot_desc_ptr->argc;
}


/**
 * Return the console uart passed by the bootloader
 *
 * @return uart   (0 or 1)
 */
int octeon_get_boot_uart(void)
{
#if OCTEON_APP_INIT_H_VERSION >= 1  /* The UART1 flag is new */
    return !!(octeon_boot_desc_ptr->flags & OCTEON_BL_FLAG_CONSOLE_UART1) ;
#else
    return 0;
#endif
}
/**
 * Get an argument from the bootloader
 *
 * @param arg    argument to get
 * @return argument
 */
const char *octeon_get_boot_argument(int arg)
{
    return octeon_phys_to_ptr(octeon_boot_desc_ptr->argv[arg]);
}


void octeon_hal_init(void)
{
    /* Make sure we got the boot descriptor block */
    if ((octeon_boot_desc_ptr == (void *)0xEADBEEFULL))
        panic("Boot descriptor block wasn't passed properly\n");

    octeon_bootinfo = octeon_phys_to_ptr(octeon_boot_desc_ptr->cvmx_desc_vaddr);

    spin_lock_init(&octeon_led_lock);
#ifndef CONFIG_CAVIUM_OCTEON_SIMULATOR
    /* Only enable the LED controller if we're running on a CN38XX or CN58XX.
        The CN30XX and CN31XX don't have an LED controller */
    if ((current_cpu_data.cputype == CPU_CAVIUM_CN38XX) ||
        (current_cpu_data.cputype == CPU_CAVIUM_CN58XX))
    {
        octeon_write_csr(OCTEON_LED_EN, 0);
        octeon_write_csr(OCTEON_LED_PRT, 0);
        octeon_write_csr(OCTEON_LED_DBG, 0);
        octeon_write_csr(OCTEON_LED_PRT_FMT, 0);
        octeon_write_csr(OCTEON_LED_UDD_CNTX(0), 32);
        octeon_write_csr(OCTEON_LED_UDD_CNTX(1), 32);
        octeon_write_csr(OCTEON_LED_UDD_DATX(0), 0);
        octeon_write_csr(OCTEON_LED_UDD_DATX(1), 0);
        octeon_write_csr(OCTEON_LED_EN, 1);
    }
#endif

#if CONFIG_CAVIUM_RESERVE32
    {
        cvmx_bootmem_desc_t *bootmem_desc = octeon_phys_to_ptr(octeon_bootinfo->phy_mem_desc_addr);
        octeon_reserve32_memory = octeon_phy_mem_named_block_alloc(bootmem_desc, CONFIG_CAVIUM_RESERVE32<<20, 0, 0, 2<<20, "CAVIUM_RESERVE32");
        if (octeon_reserve32_memory == 0)
            printk("Failed to allocate CAVIUM_RESERVE32 memory area\n");
    }
#endif

#ifdef CONFIG_CAVIUM_OCTEON_LOCK_L2
    if (octeon_read_csr(OCTEON_L2D_FUS3) & (3ull<<34))
    {
        printk("Skipping L2 locking due to reduced L2 cache size\n");
    }
    else
    {
	extern asmlinkage void octeon_handle_irq(void);
        uint64_t ebase = read_c0_ebase() & 0x3ffff000;
#ifdef CONFIG_CAVIUM_OCTEON_LOCK_L2_TLB
        octeon_l2_lock_range(ebase, 0x100);        /* TLB refill */
#endif
#ifdef CONFIG_CAVIUM_OCTEON_LOCK_L2_EXCEPTION
        octeon_l2_lock_range(ebase + 0x180, 0x80);  /* General exception */
#endif
#ifdef CONFIG_CAVIUM_OCTEON_LOCK_L2_LOW_LEVEL_INTERRUPT
        octeon_l2_lock_range(ebase + 0x200, 0x80);  /* Interrupt handler */
#endif
#ifdef CONFIG_CAVIUM_OCTEON_LOCK_L2_INTERRUPT
        octeon_l2_lock_range((uint64_t)octeon_handle_irq, 0x280);
#endif
#ifdef CONFIG_CAVIUM_OCTEON_LOCK_L2_MEMCPY
        octeon_l2_lock_range((uint64_t)memcpy, 0x480);
#endif
    }
#endif
}


/**
 * Called after Linux allocates all of its memory. This sets
 * up the 32bit shared region. Note that this region was
 * allocated as a named block during HAL init. This made sure
 * that nobody used the memory for something else during
 * init. Now we'll free it so userspace apps can use this
 * memory region with bootmem_alloc.
 */
void octeon_hal_setup_reserved32(void)
{
#if CONFIG_CAVIUM_RESERVE32
    if (octeon_reserve32_memory)
    {
        cvmx_bootmem_desc_t *bootmem_desc = octeon_phys_to_ptr(octeon_bootinfo->phy_mem_desc_addr);
        octeon_phy_mem_named_block_free(bootmem_desc, "CAVIUM_RESERVE32");
    }
#endif
}


static int octeon_l2_lock_line(uint64_t addr)
{
    int                     retval = 0;
    octeon_l2c_dbg_t        l2cdbg = {0};
    octeon_l2c_lckbase_t    lckbase = {0};
    octeon_l2c_lckoff_t     lckoff = {0};
    octeon_l2t_err_t        l2t_err;

    addr &= 0x7fffffff;

    /* Clear l2t error bits if set */
    l2t_err.u64 = octeon_read_csr(OCTEON_L2T_ERR);
    l2t_err.s.lckerr = 1;
    l2t_err.s.lckerr2 = 1;
    octeon_write_csr(OCTEON_L2T_ERR, l2t_err.u64);

    addr &= ~(cpu_icache_line_size()-1);

    /* Set this core as debug core */
    l2cdbg.s.ppnum = octeon_get_core_num();
    mb();
    octeon_write_csr(OCTEON_L2C_DBG, l2cdbg.u64);
    octeon_read_csr(OCTEON_L2C_DBG);

    lckoff.s.lck_offset = 0; /* Only lock 1 line at a time */
    octeon_write_csr(OCTEON_L2C_LCKOFF, lckoff.u64);
    octeon_read_csr(OCTEON_L2C_LCKOFF);

    if (((octeon_l2c_cfg_t)(octeon_read_csr(OCTEON_L2C_CFG))).s.idxalias)
    {
        struct cpuinfo_mips *c = &current_cpu_data;
        int l2_set_bits;
        int alias_shift;
        uint64_t addr_tmp;

        switch (c->cputype)
        {
            case CPU_CAVIUM_CN56XX:
            case CPU_CAVIUM_CN58XX:
                l2_set_bits =  11; /* 2048 sets */
                break;
            case CPU_CAVIUM_CN38XX:
                l2_set_bits =  10; /* 1024 sets */
                break;
            case CPU_CAVIUM_CN31XX:
                l2_set_bits =  9; /* 512 sets */
                break;
            case CPU_CAVIUM_CN30XX:
                l2_set_bits =  8; /* 256 sets */
                break;
	    case CPU_CAVIUM_CN50XX:
		l2_set_bits =  7; /* 128 sets */
		break;
            default:
                panic("Unknown L2 cache\n");
                break;
        }

        alias_shift = 7 + 2 * l2_set_bits - 1;
        addr_tmp = addr ^ (addr & ((1 << alias_shift) - 1)) >> l2_set_bits;
        lckbase.s.lck_base = addr_tmp >> 7;
    }
    else
    {
        lckbase.s.lck_base = addr >> 7;
    }

    lckbase.s.lck_ena = 1;
    octeon_write_csr(OCTEON_L2C_LCKBASE, lckbase.u64);
    octeon_read_csr(OCTEON_L2C_LCKBASE);    // Make sure it gets there

    *(volatile uint64_t *)(addr | (1ull<<63));

    lckbase.s.lck_ena = 0;
    octeon_write_csr(OCTEON_L2C_LCKBASE, lckbase.u64);
    octeon_read_csr(OCTEON_L2C_LCKBASE);    // Make sure it gets there

    /* Stop being debug core */
    octeon_write_csr(OCTEON_L2C_DBG, 0);
    octeon_read_csr(OCTEON_L2C_DBG);

    l2t_err.u64 = octeon_read_csr(OCTEON_L2T_ERR);
    if (l2t_err.s.lckerr || l2t_err.s.lckerr2)
        retval = 1;  /* We were unable to lock the line */

    return(retval);
}

int octeon_l2_lock_range(uint64_t addr, uint64_t len)
{
    int result = 0;

    while (len > 0)
    {
        result |= octeon_l2_lock_line(addr);
        addr += cpu_icache_line_size();
        len -= cpu_icache_line_size();
    }
    return result;
}


void *octeon_bootmem_alloc(uint64_t size, uint64_t alignment)
{
    return octeon_bootmem_alloc_range(size, alignment, 0, 0);
}


void *octeon_bootmem_alloc_range(uint64_t size, uint64_t alignment, uint64_t min_addr, uint64_t max_addr)
{
    cvmx_bootmem_desc_t *bootmem_desc = octeon_phys_to_ptr(octeon_bootinfo->phy_mem_desc_addr);

    uint64_t address;
    octeon_lock(CAST64(&(bootmem_desc->lock)));
    address = octeon_phy_mem_block_alloc(bootmem_desc, size, min_addr, max_addr, alignment);
    octeon_unlock(CAST64(&(bootmem_desc->lock)));

    if (address)
        return octeon_phys_to_ptr(address);
    else
        return NULL;
}


static inline void octeon_phy_mem_set_size(uint64_t addr, uint64_t size)
{
    *(uint64_t *)octeon_xkphys(addr + 8) = size;
}

static inline void octeon_phy_mem_set_next(uint64_t addr, uint64_t next)
{
    *(uint64_t *)octeon_xkphys(addr) = next;
}

static inline uint64_t octeon_phy_mem_get_size(uint64_t addr)
{
    return *(uint64_t *)octeon_xkphys(addr + 8);
}

static inline uint64_t octeon_phy_mem_get_next(uint64_t addr)
{
    return *(uint64_t *)octeon_xkphys(addr);
}


#define printf printk
#define cvmx_phys_to_ptr octeon_phys_to_ptr
#include "cvmx-bootmem-shared.c"


/* These are aliases for the above function for easy use by CVMX based
    modules. They shouldn't be called by any statically linked code */
static void *cvmx_bootmem_alloc(uint64_t size, uint64_t alignment)
{
    return octeon_bootmem_alloc(size, alignment);
}

static void *cvmx_bootmem_alloc_range(uint64_t size, uint64_t alignment, uint64_t min_addr, uint64_t max_addr)
{
    return octeon_bootmem_alloc_range(size, alignment, min_addr, max_addr);
}

cvmx_bootmem_named_block_desc_t * cvmx_bootmem_find_named_block(char *name)
{
    cvmx_bootmem_desc_t *bootmem_desc = octeon_phys_to_ptr(octeon_bootinfo->phy_mem_desc_addr);
    return(octeon_phy_mem_named_block_find(bootmem_desc, name));
}

/**
 * Enable access to Octeon's COP2 crypto hardware for kernel use.
 * Wrap any crypto operations in calls to
 * octeon_crypto_enable/disable in order to make sure the state of
 * COP2 isn't corrupted if userspace is also performing hardware
 * crypto operations. Allocate the state parameter on the stack.
 *
 * @param state  State structure to store current COP2 state in
 *
 * @return Flags to be passed to octeon_crypto_disable()
 */
unsigned long octeon_crypto_enable(struct octeon_cop2_state *state)
{
	extern void octeon_cop2_save(struct octeon_cop2_state *);
	int status;
	unsigned long flags;

	local_irq_save(flags);
	status = read_c0_status();
	write_c0_status(status | ST0_CU2);
	if (KSTK_STATUS(current) & ST0_CU2)
	{
		octeon_cop2_save(&(current->thread.cp2));
		KSTK_STATUS(current) &= ~ST0_CU2;
		status &= ~ST0_CU2;
	} else if (status & ST0_CU2)
		octeon_cop2_save(state);
	local_irq_restore(flags);
	return status & ST0_CU2;
}
EXPORT_SYMBOL(octeon_crypto_enable);


/**
 * Disable access to Octeon's COP2 crypto hardware in the kernel.
 * This must be called after an octeon_crypto_enable() before any
 * context switch or return to userspace.
 *
 * @param state  COP2 state to restore
 * @param flags  Return value from octeon_crypto_enable()
 */
void octeon_crypto_disable(struct octeon_cop2_state *state, unsigned long crypto_flags)
{
	extern void octeon_cop2_restore(struct octeon_cop2_state *);
	unsigned long flags;

	local_irq_save(flags);
	if (crypto_flags & ST0_CU2)
		octeon_cop2_restore(state);
	else
                write_c0_status(read_c0_status() & ~ST0_CU2);
	local_irq_restore(flags);
}
EXPORT_SYMBOL(octeon_crypto_disable);


EXPORT_SYMBOL(cvmx_bootmem_alloc_range);
EXPORT_SYMBOL(cvmx_bootmem_find_named_block);
EXPORT_SYMBOL(cvmx_bootmem_alloc);
EXPORT_SYMBOL(octeon_bootinfo);
EXPORT_SYMBOL(mips_hpt_frequency);
#if CONFIG_CAVIUM_RESERVE32
EXPORT_SYMBOL(octeon_reserve32_memory);
#endif

