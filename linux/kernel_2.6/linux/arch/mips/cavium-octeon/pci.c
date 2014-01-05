/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2005, 2006 Cavium Networks
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/interrupt.h>
#include <asm/time.h>
#include <asm/delay.h>
#include "hal.h"
#include "pci_bar_setup.h"

#define USE_OCTEON_INTERNAL_ARBITER

/* Octeon's PCI controller uses did=3, subdid=2 for PCI IO addresses.
    Use PCI endian swapping 1 so no address swapping is necessary. The
    Linux io routines will endian swap the data */
#define OCTEON_PCI_IOSPACE_BASE     0x80011a0400000000ull
#define OCTEON_PCI_IOSPACE_SIZE     (1ull<<32)

/* Octeon't PCI controller uses did=3, subdid=3 for PCI memory. */
#define OCTEON_PCI_MEMSPACE_OFFSET  (0x00011b0000000000ull)

/**
 * This is the bit decoding used for the Octeon PCI controller addresses
 */
typedef union
{
    uint64_t    u64;
    uint64_t *  u64_ptr;
    uint32_t *  u32_ptr;
    uint16_t *  u16_ptr;
    uint8_t *   u8_ptr;
    struct
    {
        uint64_t    upper       : 2;
        uint64_t    reserved    : 13;
        uint64_t    io          : 1;
        uint64_t    did         : 5;
        uint64_t    subdid      : 3;
        uint64_t    reserved2   : 4;
        uint64_t    endian_swap : 2;
        uint64_t    reserved3   : 10;
        uint64_t    bus         : 8;
        uint64_t    dev         : 5;
        uint64_t    func        : 3;
        uint64_t    reg         : 8;
    } s;
} octeon_pci_address_t;


/**
 * Map a PCI device to the appropriate interrupt line
 *
 * @param dev    The Linux PCI device structure for the device to map
 * @param slot   The slot number for this device on __BUS 0__. Linux
 *               enumerates through all the bridges and figures out the
 *               slot on Bus 0 where this device eventually hooks to.
 * @param pin    The PCI interrupt pin read from the device, then swizzled
 *               as it goes through each bridge.
 * @return Interrupt number for the device
 */
int __init pcibios_map_irq(struct pci_dev *dev, u8 slot, u8 pin)
{
    extern void octeon_i8259_setup(int irq_line);
    extern void octeon_pci_chip_init(struct pci_dev *dev);
    int irq_num;
    const char *interrupts;
    int dev_num;

    octeon_pci_chip_init(dev);

    /* Get the board specific interrupt mapping */
    interrupts = octeon_get_pci_interrupts();

    dev_num = dev->devfn>>3;
    if (dev_num < strlen(interrupts))
        irq_num = ((interrupts[dev_num] - 'A' + pin - 1) & 3) + 44;   /* 44 is the irq number of PCI INT-A */
    else
        irq_num = ((slot + pin - 3) & 3) + 44;   /* 44 is the irq number of PCI INT-A */

    if (dev->vendor == 0x1106) /* Via chip */
    {
        /* Devices in the Via southbridge don't use PCI interrupts. They are
            routed through the two 8259s */
        switch (dev->device)
        {
            case 0x0686: /* VT82C686B Super South South Bridge */
                octeon_i8259_setup(octeon_get_southbridge_interrupt());
                irq_num = 80+10;
                break;
            case 0x3038: /* VT82C686B USB ports */
                irq_num = 80+13;
                break;
            case 0x0571: /* VT82C686B Bus Master IDE */
                irq_num = 80+14;
                break;
        }
    }

    printk("PCI dev %2d.%d slot %2d pin %d: %04x:%04x using irq %d\n",
           dev->devfn>>3, dev->devfn&0x7, slot, pin, dev->vendor, dev->device, irq_num);

    return irq_num;
}


/**
 * Called to perform platform specific PCI setup
 *
 * @param dev
 * @return
 */
int pcibios_plat_dev_init(struct pci_dev *dev)
{
    /* Force the Cache line setting to 64 bytes. The standard Linux bus scan
        doesn't seem to set it. Octeon really has 128 byte lines, but Intel
        bridges get really upset if you try and set values above 64 bytes.
        Value is specified in 32bit words */
    pci_write_config_byte(dev, PCI_CACHE_LINE_SIZE, 64/4);
    /* Set latency timers for all devices */
    pci_write_config_byte(dev, PCI_LATENCY_TIMER, 48);
    if (dev->subordinate)
    {
        uint16_t config;

        /* Set latency timers on sub bridges */
        pci_write_config_byte(dev, PCI_SEC_LATENCY_TIMER, 48);
        /* Enable parity checking and error reporting */
        pci_read_config_word(dev, PCI_COMMAND, &config);
        config |= PCI_COMMAND_PARITY | PCI_COMMAND_SERR;
        pci_write_config_word(dev, PCI_COMMAND, config);
        /* More bridge error detection */
        pci_read_config_word(dev, PCI_BRIDGE_CONTROL, &config);
        config |= PCI_BRIDGE_CTL_PARITY | PCI_BRIDGE_CTL_SERR;
        /* Reporting master aborts also causes SERR. Normally it creates too
            much noise, but it might be useful in the future */
        //config |= PCI_BRIDGE_CTL_MASTER_ABORT;
        pci_write_config_word(dev, PCI_BRIDGE_CONTROL, config);
    }
    return 0;
}


/**
 * Read a value from configuration space
 *
 * @param bus
 * @param devfn
 * @param reg
 * @param size
 * @param val
 * @return
 */
static int octeon_read_config(struct pci_bus *bus, unsigned int devfn, int reg, int size, u32 *val)
{
    static int do_once = 0;
    octeon_pci_address_t pci_addr;

    pci_addr.u64 = 0;
    pci_addr.s.upper = 2;
    pci_addr.s.io = 1;
    pci_addr.s.did = 3;
    pci_addr.s.subdid = 1;
    pci_addr.s.endian_swap = 1;
    pci_addr.s.bus = bus->number;
    pci_addr.s.dev = devfn>>3;
    pci_addr.s.func = devfn & 0x7;
    pci_addr.s.reg = reg;

    switch (size)
    {
        case 4:
            *val = le32_to_cpup(pci_addr.u32_ptr);
            if (unlikely(!do_once && (reg==0) && (*val == 0x06861106)))
            {
                /* VT82C686B Super South South Bridge */
                pci_addr.s.reg = 0x48;
                if (*pci_addr.u8_ptr & 0x2)
                {
                    printk("Force enabling the Via IDE (bus=%d, dev=%d)\n", bus->number, devfn>>3);
                    *pci_addr.u8_ptr ^= 2;
                }
                do_once = 1;
            }
            return PCIBIOS_SUCCESSFUL;
        case 2:
            *val = le16_to_cpup(pci_addr.u16_ptr);
            return PCIBIOS_SUCCESSFUL;
        case 1:
            *val = *pci_addr.u8_ptr;
            return PCIBIOS_SUCCESSFUL;
    }
    return PCIBIOS_FUNC_NOT_SUPPORTED;
}


/**
 * Write a value to PCI configuration space
 *
 * @param bus
 * @param devfn
 * @param reg
 * @param size
 * @param val
 * @return
 */
static int octeon_write_config(struct pci_bus *bus, unsigned int devfn, int reg, int size, u32 val)
{
    octeon_pci_address_t pci_addr;

    pci_addr.u64 = 0;
    pci_addr.s.upper = 2;
    pci_addr.s.io = 1;
    pci_addr.s.did = 3;
    pci_addr.s.subdid = 1;
    pci_addr.s.endian_swap = 1;
    pci_addr.s.bus = bus->number;
    pci_addr.s.dev = devfn>>3;
    pci_addr.s.func = devfn & 0x7;
    pci_addr.s.reg = reg;

    switch (size)
    {
        case 4:
            *pci_addr.u32_ptr = cpu_to_le32(val);
            return PCIBIOS_SUCCESSFUL;
        case 2:
            *pci_addr.u16_ptr = cpu_to_le16(val);
            return PCIBIOS_SUCCESSFUL;
        case 1:
            *pci_addr.u8_ptr = val;
            return PCIBIOS_SUCCESSFUL;
    }
    return PCIBIOS_FUNC_NOT_SUPPORTED;
}


static struct pci_ops octeon_pci_ops = {
    octeon_read_config,
    octeon_write_config,
};

static struct resource octeon_pci_mem_resource = {
    "Octeon PCI MEM", 0, 0, IORESOURCE_MEM,
};

/* PCI ports must be above 16KB so the ISA bus filtering in the PCI-X to PCI bridge */
static struct resource octeon_pci_io_resource = {
    "Octeon PCI IO", 0x4000, OCTEON_PCI_IOSPACE_SIZE - 1, IORESOURCE_IO,
};

static struct pci_controller octeon_pci_controller = {
	.pci_ops        = &octeon_pci_ops,
	.mem_resource   = &octeon_pci_mem_resource,
	.mem_offset     = OCTEON_PCI_MEMSPACE_OFFSET,
	.io_resource    = &octeon_pci_io_resource,
	.io_offset      = 0
};


/**
 * Low level initialize the Octeon PCI controller
 *
 * @return
 */
static void octeon_pci_initialize(void)
{
    octeon_pci_cfg01_t cfg01;
    octeon_npi_ctl_status_t ctl_status;
    octeon_pci_ctl_status_2_t ctl_status_2;
    octeon_pci_cfg19_t cfg19;
    octeon_pci_cfg16_t cfg16;
    octeon_pci_cfg22_t cfg22;
    octeon_pci_cfg56_t cfg56;

    /* Reset the PCI Bus */
    octeon_write_csr(OCTEON_CIU_SOFT_PRST, 0x1);
    octeon_read_csr(OCTEON_CIU_SOFT_PRST);

    udelay (2000);	   /* Hold  PCI reset for 2 ms */

    ctl_status.u64 = 0;//octeon_read_csr(OCTEON_NPI_CTL_STATUS);
    ctl_status.s.max_word = 1;
    ctl_status.s.timer = 1;
    octeon_write_csr(OCTEON_NPI_CTL_STATUS, ctl_status.u64);

    /* Deassert PCI reset and advertize PCX Host Mode Device Capability (64b) */
    octeon_write_csr(OCTEON_CIU_SOFT_PRST, 0x4);
    octeon_read_csr(OCTEON_CIU_SOFT_PRST);

    udelay (2000);	   /* Wait 2 ms after deasserting PCI reset */

    ctl_status_2.u32 = 0;
    ctl_status_2.s.tsr_hwm = 1;	/* Initializes to 0.  Must be set before any PCI reads. */
    ctl_status_2.s.bar2pres = 1; /* Enable BAR2 */
    ctl_status_2.s.bar2_enb = 1;
    ctl_status_2.s.bar2_cax = 0;
    ctl_status_2.s.bar2_esx = 1;
    ctl_status_2.s.pmo_amod = 1; /* Round robin priority */
    if (octeon_dma_use_big_bar)
    {
        ctl_status_2.s.bb1_hole = OCTEON_PCI_BAR1_HOLE_BITS;/* BAR1 hole */
        ctl_status_2.s.bb1_siz = 1; /* BAR1 is 2GB */
        ctl_status_2.s.bb_ca = 0;   /* Use L2 with big bars */
        ctl_status_2.s.bb_es = 1;   /* Big bar in byte swap mode */
        ctl_status_2.s.bb1 = 1;     /* BAR1 is big */
        ctl_status_2.s.bb0 = 1;     /* BAR0 is big */
    }

    octeon_npi_write32(OCTEON_NPI_PCI_CTL_STATUS_2, ctl_status_2.u32);
    udelay (2000);	   /* Wait 2 ms before doing PCI reads */

    ctl_status_2.u32 = octeon_npi_read32(OCTEON_NPI_PCI_CTL_STATUS_2);
    printk("PCI Status: %s %s-bit\n",
	   ctl_status_2.s.ap_pcix ? "PCI-X" : "PCI",
	   ctl_status_2.s.ap_64ad ? "64" : "32" );

    /*
    ** TDOMC must be set to one in PCI mode. TDOMC should be set to 4
    ** in PCI-X mode to allow four oustanding splits. Otherwise,
    ** should not change from its reset value. Don't write PCI_CFG19
    ** in PCI mode (0x82000001 reset value), write it to 0x82000004
    ** after PCI-X mode is known. MRBCI,MDWE,MDRE -> must be zero.
    ** MRBCM -> must be one.
    */
    if (ctl_status_2.s.ap_pcix) {
      cfg19.u32 = 0;
      cfg19.s.tdomc = 4;	/* Target Delayed/Split request
                                   outstanding maximum count. [1..31]
                                   and 0=32.  NOTE: If the user
                                   programs these bits beyond the
                                   Designed Maximum outstanding count,
                                   then the designed maximum table
                                   depth will be used instead.  No
                                   additional Deferred/Split
                                   transactions will be accepted if
                                   this outstanding maximum count is
                                   reached. Furthermore, no additional
                                   deferred/split transactions will be
                                   accepted if the I/O delay/ I/O
                                   Split Request outstanding maximum
                                   is reached. */
      cfg19.s.mdrrmc = 2;	/* Master Deferred Read Request Outstanding Max
                                   Count (PCI only).
                                   CR4C[26:24]  Max SAC cycles   MAX DAC cycles
                                    000              8                4
                                    001              1                0
                                    010              2                1
                                    011              3                1
                                    100              4                2
                                    101              5                2
                                    110              6                3
                                    111              7                3
                                   For example, if these bits are programmed to
                                   100, the core can support 2 DAC cycles, 4 SAC
                                   cycles or a combination of 1 DAC and 2 SAC cycles.
                                   NOTE: For the PCI-X maximum outstanding split
                                   transactions, refer to CRE0[22:20]  */

      cfg19.s.mrbcm = 1;	/* Master Request (Memory Read) Byte Count/Byte
                                   Enable select.
                                    0 = Byte Enables valid. In PCI mode, a burst
                                        transaction cannot be performed using
                                        Memory Read command=4?h6.
                                    1 = DWORD Byte Count valid (default). In PCI
                                        Mode, the memory read byte enables are
                                        automatically generated by the core.
                                   Note: N3 Master Request transaction sizes are
                                   always determined through the
                                   am_attr[<35:32>|<7:0>] field.  */
      octeon_npi_write32(OCTEON_NPI_PCI_CFG19, cfg19.u32);
    }


    cfg01.u32 = 0;
    cfg01.s.msae = 1;		/* Memory Space Access Enable */
    cfg01.s.me = 1;		    /* Master Enable */
    cfg01.s.pee = 1;		/* PERR# Enable */
    cfg01.s.see = 1;		/* System Error Enable */
    cfg01.s.fbbe = 1;		/* Fast Back to Back Transaction Enable */

    octeon_npi_write32(OCTEON_NPI_PCI_CFG01, cfg01.u32);
    octeon_npi_read32(OCTEON_NPI_PCI_CFG01);

#ifdef USE_OCTEON_INTERNAL_ARBITER
    /*
    ** When OCTEON is a PCI host, most systems will use OCTEON's
    ** internal arbiter, so must enable it before any PCI/PCI-X
    ** traffic can occur.
    */
    {
      octeon_npi_pci_int_arb_cfg_t pci_int_arb_cfg;

      pci_int_arb_cfg.u64 = 0;
      pci_int_arb_cfg.s.en = 1;	/* Internal arbiter enable */
      octeon_write_csr(OCTEON_NPI_PCI_INT_ARB_CFG, pci_int_arb_cfg.u64);
    }
#endif /* USE_OCTEON_INTERNAL_ARBITER */

    /*
    ** Preferrably written to 1 to set MLTD. [RDSATI,TRTAE,
    ** TWTAE,TMAE,DPPMR -> must be zero. TILT -> must not be set to
    ** 1..7.
    */
    cfg16.u32 = 0;
    cfg16.s.mltd = 1;		/* Master Latency Timer Disable */
    octeon_npi_write32(OCTEON_NPI_PCI_CFG16, cfg16.u32);

    /*
    ** Should be written to 0x4ff00. MTTV -> must be zero.
    ** FLUSH -> must be 1. MRV -> should be 0xFF.
    */
    cfg22.u32 = 0;
    cfg22.s.mrv = 0xff;		/* Master Retry Value [1..255] and 0=infinite */
    cfg22.s.flush = 1;		/* AM_DO_FLUSH_I control NOTE: This
				   bit MUST BE ONE for proper N3K
				   operation */
    octeon_npi_write32(OCTEON_NPI_PCI_CFG22, cfg22.u32);

    /*
    ** MOST Indicates the maximum number of outstanding splits (in -1
    ** notation) when OCTEON is in PCI-X mode.  PCI-X performance is
    ** affected by the MOST selection.  Should generally be written
    ** with one of 0x3be807, 0x2be807, 0x1be807, or 0x0be807,
    ** depending on the desired MOST of 3, 2, 1, or 0, respectively.
    */
    cfg56.u32 = 0;
    cfg56.s.pxcid = 7;		/* RO - PCI-X Capability ID */
    cfg56.s.ncp = 0xe8;		/* RO - Next Capability Pointer */
    cfg56.s.dpere = 1;		/* Data Parity Error Recovery Enable */
    cfg56.s.roe = 1;		/* Relaxed Ordering Enable */
    cfg56.s.mmbc = 1;		/* Maximum Memory Byte Count [0=512B,1=1024B,2=2048B,3=4096B] */
    cfg56.s.most = 3;		/* Maximum outstanding Split transactions [0=1 .. 7=32] */

    octeon_npi_write32(OCTEON_NPI_PCI_CFG56, cfg56.u32);

    /*
    ** Affects PCI performance when OCTEON services reads to its
    ** BAR1/BAR2. Refer to Section 10.6.1.  The recommended values are
    ** 0x22, 0x33, and 0x33 for PCI_READ_CMD_6, PCI_READ_CMD_C, and
    ** PCI_READ_CMD_E, respectively. Note that these values differ
    ** from their reset values.
    */
    octeon_npi_write32(OCTEON_NPI_PCI_READ_CMD_6, 0x22);
    octeon_npi_write32(OCTEON_NPI_PCI_READ_CMD_C, 0x33);
    octeon_npi_write32(OCTEON_NPI_PCI_READ_CMD_E, 0x33);
}


/**
 * Initialize the Octeon PCI controller
 *
 * @return
 */
static int __init octeon_pci_setup(void)
{
    octeon_npi_mem_access_subid_t mem_access;
    int index;

    /* Only use the big bars on chips that support it */
    octeon_dma_use_big_bar = (current_cpu_data.cputype == CPU_CAVIUM_CN30XX) || (current_cpu_data.cputype == CPU_CAVIUM_CN50XX);

    /* PCI I/O and PCI MEM values */
    set_io_port_base(OCTEON_PCI_IOSPACE_BASE);
    ioport_resource.start = 0;
    ioport_resource.end   = OCTEON_PCI_IOSPACE_SIZE - 1;
    iomem_resource.start  = (1ull<<48);
    iomem_resource.end    = (2ull<<48)-1;

    if (!octeon_is_pci_host())
    {
        printk("Not in host mode, PCI Controller not initialized\n");
        return 0;
    }

    octeon_pci_initialize();

    mem_access.u64 = 0;
    mem_access.s.esr = 1;   /**< Endian-Swap on read. */
    mem_access.s.esw = 1;   /**< Endian-Swap on write. */
    mem_access.s.nsr = 0;   /**< No-Snoop on read. */
    mem_access.s.nsw = 0;   /**< No-Snoop on write. */
    mem_access.s.ror = 0;   /**< Relax Read on read. */
    mem_access.s.row = 0;   /**< Relax Order on write. */
    mem_access.s.ba = 0;    /**< PCI Address bits [63:36]. */
    octeon_write_csr(OCTEON_NPI_MEM_ACCESS_SUBID3, mem_access.u64);

    /* Remap the Octeon BAR 2 above all 32 bit devices (0x8000000000ul). This
        is done here so it is remapped before the readl()'s below. We don't
        want BAR2 overlapping with BAR0/BAR1 during these reads */
    octeon_npi_write32(OCTEON_NPI_PCI_CFG08, 0);
    octeon_npi_write32(OCTEON_NPI_PCI_CFG09, 0x80);

    /* Disable the BAR1 movable mappings */
    for (index=0; index<32; index++)
        octeon_npi_write32(OCTEON_NPI_PCI_BAR1_INDEXX(index), 0);

    if (octeon_dma_use_big_bar)
    {
        /* Remap the Octeon BAR 0 to 0-2GB */
        octeon_npi_write32(OCTEON_NPI_PCI_CFG04, 0);
        octeon_npi_write32(OCTEON_NPI_PCI_CFG05, 0);

        /* Remap the Octeon BAR 1 to map 2GB-4GB (minus the BAR 1 hole) */
        octeon_npi_write32(OCTEON_NPI_PCI_CFG06, 2ul<<30);
        octeon_npi_write32(OCTEON_NPI_PCI_CFG07, 0);

        /* Devices go after BAR1 */
        octeon_pci_mem_resource.start = OCTEON_PCI_MEMSPACE_OFFSET + (4ul<<30) - (OCTEON_PCI_BAR1_HOLE_SIZE<<20);
        octeon_pci_mem_resource.end = octeon_pci_mem_resource.start + (1ul<<30);

        /* Read a 32bit word from memory through the BAR0 register. This is a
            workaround for Errata PCI-302. Some flops used to generate the upper
            address bits are not being reset which can result in the wrong address
            being used for only the first PCI access.  A read to any BAR will
            initialize these flops. */
        readl((void*)XKPHYS + OCTEON_PCI_MEMSPACE_OFFSET);
    }
    else
    {
        /* Remap the Octeon BAR 0 to map 128MB-(128MB+4KB) */
        octeon_npi_write32(OCTEON_NPI_PCI_CFG04, 128ul<<20);
        octeon_npi_write32(OCTEON_NPI_PCI_CFG05, 0);

        /* Remap the Octeon BAR 1 to map 0-128MB */
        octeon_npi_write32(OCTEON_NPI_PCI_CFG06, 0);
        octeon_npi_write32(OCTEON_NPI_PCI_CFG07, 0);

        /* Devices go after BAR0 */
        octeon_pci_mem_resource.start = OCTEON_PCI_MEMSPACE_OFFSET + (128ul<<20) + (4ul<<10);
        octeon_pci_mem_resource.end = octeon_pci_mem_resource.start + (1ul<<30);

        /* Read a 32bit word from memory through the BAR0 register. This is a
            workaround for Errata PCI-302. Some flops used to generate the upper
            address bits are not being reset which can result in the wrong address
            being used for only the first PCI access.  A read to any BAR will
            initialize these flops. Technically, this isn't needed on chips
            without big bar support. It is here in case someone decides not to
            use big bars on a chip that supports them. */
        readl((void*)XKPHYS + OCTEON_PCI_MEMSPACE_OFFSET + (128<<20));
    }

    register_pci_controller(&octeon_pci_controller);

    /* Clear any errors that might be pending from before the bus was
        setup properly */
    octeon_write_csr(OCTEON_NPI_PCI_INT_SUM2, -1);
    return 0;
}

arch_initcall(octeon_pci_setup);
