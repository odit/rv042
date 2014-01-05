/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2005 Cavium Networks, Inc
 */
#ifndef __ASM_MACH_GENERIC_KERNEL_ENTRY_H
#define __ASM_MACH_GENERIC_KERNEL_ENTRY_H


#define CP0_CYCLE_COUNTER $9,6
#define CP0_CVMCTL_REG $9,7
#define CP0_PRID_REG $15,0
#define CP0_PRID_OCTEON_PASS1 0x000d0000
#define CP0_PRID_OCTEON_CN30XX 0x000d0200

.macro  kernel_entry_setup
    # Registers set by bootloader:
    # (only 32 bits set by bootloader, all addresses are physical addresses, and need
    #   to have the appropriate memory region set by the kernel
    # a0 = argc
    # a1 = argv (kseg0 compat addr )
    # a2 = 1 if init core, zero otherwise
    # a3 = address of boot descriptor block

    dmfc0   v0, CP0_CVMCTL_REG          # Read the cavium control register
#ifdef CONFIG_CAVIUM_OCTEON_HW_FIX_UNALIGNED
    or  v0, v0, 0x5001                  # Disable unaligned load/store support but leave HW fixup enabled
    xor v0, v0, 0x1001
#else
    or  v0, v0, 0x5001                  # Disable unaligned load/store and HW fixup support
    xor v0, v0, 0x5001
#endif
    mfc0 v1, CP0_PRID_REG               # Read the processor ID register
    or  v0, v0, 0x2000                  # Disable instruction prefetching (Octeon Pass1 errata)
    beq v1, CP0_PRID_OCTEON_PASS1,skip  # Skip reenable of prefetching for Octeon Pass1
     nop
    xor v0, v0, 0x2000                  # Reenable instruction prefetching, not on Pass1
    srl v1, 8                           # Strip off pass number off of processor id
    sll v1, 8
    bne v1, CP0_PRID_OCTEON_CN30XX,skip # CN30XX needs some extra stuff turned off for better performance
     nop
    or  v0, v0, 0x400                   # CN30XX Use random Icache replacement
    or  v0, v0, 0x2000                  # CN30XX Disable instruction prefetching
skip:
    dmtc0   v0, CP0_CVMCTL_REG          # Write the cavium control register
    sync
    cache   9, 0($0)                    # Flush dcache after config change

    PTR_LA  t2, octeon_boot_desc_ptr    # Store the boot descriptor pointer
    LONG_S  a3, (t2)

#ifdef CONFIG_SMP
    rdhwr   v0, $0                      # Get my core id

    bne     a2, zero, octeon_main_processor # Jump the master to kernel_entry
    nop
    #
    # All cores other than the master need to wait here for SMP bootstrap
    # to begin
    #
    PTR_LA  t0, octeon_processor_boot   # This is the variable where the next core to boot os stored
octeon_spin_wait_boot:
    LONG_L  t1, (t0)                    # Get the core id of the next to be booted
    bne t1, v0, octeon_spin_wait_boot   # Keep looping if it isn't me
    nop
    PTR_LA  t0, octeon_processor_cycle  # Synchronize the cycle counters
    LONG_L  t0, (t0)
    daddu   t0, 122                     # Aproximately how many cycles we will be off
    dmtc0   t0, CP0_CYCLE_COUNTER
    PTR_LA  t0, octeon_processor_gp     # Get my GP from the global variable
    LONG_L  gp, (t0)
    PTR_LA  t0, octeon_processor_sp     # Get my SP from the global variable
    LONG_L  sp, (t0)
    LONG_S  zero, (t0)                  # Set the SP global variable to zero so the master knows we've started
#ifdef __OCTEON__
    syncw
#else
    sync
#endif
    b   smp_bootstrap                   # Jump to the normal Linux SMP entry point
    nop
octeon_main_processor:
#endif
.endm

/*
 * Do SMP slave processor setup necessary before we can savely execute C code.
 */
    .macro  smp_slave_setup
    .endm


#endif /* __ASM_MACH_GENERIC_KERNEL_ENTRY_H */
