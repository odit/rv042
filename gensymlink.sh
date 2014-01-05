
mkdir host/include
mkdir target/include
mkdir target/include/open-license
mkdir target/lib

################### NK-modify start ###################
# purpose:   Build Machine  # author:    Chihmou
# date:      2010-07-25   # description: Update the link of toolchain

#ln -s /usr/local/cavium/tools-gcc-4.1 tools
[ -d $PWD/cavium ] && \
ln -s $PWD/cavium/tools-gcc-4.1 tools || \
ln -s /usr/local/cavium/tools-gcc-4.1 tools

###################  NK-modify end  ###################

ln -s ../pci/octeon-pci.h host/include/octeon-pci.h
ln -s ../pci/octeon-pci-debug.h host/include/octeon-pci-debug.h

ln -s ../pci/oct-pci-boot host/bin/oct-pci-boot
ln -s ../../simulator/bin/oct-uart-io host/bin/oct-uart-io
ln -s ../pci/oct-pci-profile host/bin/oct-pci-profile
ln -s ../../simulator/bin/perfzilla host/bin/perfzilla
#ln -s ../../simulator/bin/pp64 host/bin/pp64
ln -s ../../simulator/bin/oct-profile host/bin/oct-profile
ln -s ../../simulator/bin/vz-cut host/bin/vz-cut
ln -s ../../simulator/bin/oct-debug host/bin/oct-debug
ln -s ../../simulator/bin/cn30xx-simulator host/bin/cn30xx-simulator
ln -s ../pci/oct-pci-reset host/bin/oct-pci-reset
ln -s ../../simulator/bin/cn30xx-simulator64 host/bin/cn30xx-simulator64
ln -s ../pci/oct-pci-bootcmd host/bin/oct-pci-bootcmd
ln -s ../pci/oct-pci-csr host/bin/oct-pci-csr
ln -s ../../simulator/utils/oct-packet-io/oct-packet-io host/bin/oct-packet-io
ln -s ../pci/oct-pci-load host/bin/oct-pci-load
#ln -s ../../simulator/bin/pp host/bin/pp
ln -s ../../simulator/bin/dwarfdump host/bin/dwarfdump
ln -s ../../simulator/bin/oct-debuginfo host/bin/oct-debuginfo
ln -s ../../simulator/bin/viewzilla host/bin/viewzilla
ln -s ../../simulator/bin/cvmx-config host/bin/cvmx-config
ln -s ../../simulator/bin/cn31xx-simulator host/bin/cn31xx-simulator
ln -s ../../simulator/bin/cn31xx-simulator64 host/bin/cn31xx-simulator64
ln -s ../../simulator/bin/cn31xx-simulator host/bin/cn38xx-simulator
ln -s ../../simulator/bin/cn31xx-simulator64 host/bin/cn38xx-simulator64
ln -s ../../simulator/bin/cn31xx-simulator host/bin/cn56xx-simulator
ln -s ../../simulator/bin/cn31xx-simulator64 host/bin/cn56xx-simulator64
ln -s ../../simulator/bin/cn31xx-simulator host/bin/cn58xx-simulator
ln -s ../../simulator/bin/cn31xx-simulator64 host/bin/cn58xx-simulator64
ln -s ../pci/oct-pci-memory host/bin/oct-pci-memory
ln -s ../pci/oct-pci-ddr host/bin/oct-pci-ddr
ln -s ../../simulator/bin/oct-sim host/bin/oct-sim

ln -s ../../bootloader/u-boot/lib_mips/lib_octeon_shared.c host/pci/lib_octeon_shared.c

ln -s component_not_installed.html docs/html/zlib_main.html
ln -s component_not_installed.html docs/html/crypto_main.html
ln -s component_not_installed.html docs/html/l2ip_main.html
ln -s component_not_installed.html docs/html/ccsm_main.html
ln -s component_not_installed.html docs/html/pci_driver_linux_main.html
ln -s component_not_installed.html docs/html/ipsec_main.html
ln -s component_not_installed.html docs/html/ssl_main.html
ln -s component_not_installed.html docs/html/dfa_main.html
ln -s component_not_installed.html docs/html/tcpip_main.html

ln -s ../../../executive/octeon-model.h target/include/open-license/octeon-model.h
ln -s ../../../executive/cvmx-bootmem-shared.c target/include/open-license/cvmx-bootmem-shared.c
ln -s ../../../executive/cvmx-bootmem-shared.h target/include/open-license/cvmx-bootmem-shared.h
ln -s ../../../executive/cvmx-app-init.h target/include/open-license/cvmx-app-init.h

ln -s ../../executive/cvmx-pip.h target/include/cvmx-pip.h
ln -s ../../executive/cvmx-rwlock.h target/include/cvmx-rwlock.h
ln -s ../../executive/cvmx-cn3010-evb-hs5.h target/include/cvmx-cn3010-evb-hs5.h
ln -s ../../executive/cvmx-rng.h target/include/cvmx-rng.h
ln -s ../../executive/cvmx-pko.h target/include/cvmx-pko.h
ln -s ../../executive/cvmx-cvmmem.h target/include/cvmx-cvmmem.h
ln -s ../../executive/cvmx-wqe.h target/include/cvmx-wqe.h
ln -s ../../executive/cvmx-spi.h target/include/cvmx-spi.h
ln -s ../../executive/cvmip.h target/include/cvmip.h
ln -s ../../executive/cvmx-dfa.h target/include/cvmx-dfa.h
ln -s ../../executive/cvmx-atomic.h target/include/cvmx-atomic.h
ln -s ../../executive/cvmx-twsi.h target/include/cvmx-twsi.h
ln -s ../../executive/cvmx-coremask.h target/include/cvmx-coremask.h
ln -s ../../executive/cvmx-l2c.h target/include/cvmx-l2c.h
ln -s ../../executive/cvmx-ciu.h target/include/cvmx-ciu.h
ln -s ../../executive/cvmx-helper.h target/include/cvmx-helper.h
ln -s ../../executive/cvmx-malloc.h target/include/cvmx-malloc.h
ln -s ../../executive/cvmx-gmx.h target/include/cvmx-gmx.h
ln -s ../../executive/cvmx-iob.h target/include/cvmx-iob.h
ln -s ../../executive/cvmx-tra.h target/include/cvmx-tra.h
ln -s ../../executive/cvmx-fau.h target/include/cvmx-fau.h
ln -s ../../executive/cvmx-packet.h target/include/cvmx-packet.h
ln -s ../../executive/cvmx-llm.h target/include/cvmx-llm.h
ln -s ../../executive/cvmx-log.h target/include/cvmx-log.h
ln -s ../../executive/cvmx-mio.h target/include/cvmx-mio.h
ln -s ../../executive/cvmx-pow.h target/include/cvmx-pow.h
ln -s ../../executive/cvmx-tim.h target/include/cvmx-tim.h
ln -s ../../executive/cvmx-gpio.h target/include/cvmx-gpio.h
ln -s ../../executive/cvmx-flash.h target/include/cvmx-flash.h
ln -s ../../executive/cvmx.h target/include/cvmx.h
ln -s ../../executive/cvmx-abi.h target/include/cvmx-abi.h
ln -s ../../executive/cvmx-zip.h target/include/cvmx-zip.h
ln -s ../../executive/cvmx-core.h target/include/cvmx-core.h
ln -s ../../executive/cvmx-uart.h target/include/cvmx-uart.h
ln -s ../../executive/cvmx-interrupt.h target/include/cvmx-interrupt.h
ln -s ../../executive/cvmx-ebt3000.h target/include/cvmx-ebt3000.h
ln -s ../../executive/cvmx-rtc.h target/include/cvmx-rtc.h
ln -s ../../executive/cvmx-ipd.h target/include/cvmx-ipd.h
ln -s ../../executive/cvmx-key.h target/include/cvmx-key.h
ln -s ../../executive/cvmx-fpa.h target/include/cvmx-fpa.h
ln -s ../../executive/cvmx-resources.config target/include/cvmx-resources.config
ln -s ../../executive/cvmx-pci.h target/include/cvmx-pci.h
ln -s ../../executive/cvmx-scratch.h target/include/cvmx-scratch.h
ln -s ../../executive/cvmx-csr.h target/include/cvmx-csr.h
ln -s ../../executive/cvmx-spinlock.h target/include/cvmx-spinlock.h
ln -s ../../executive/cvmx-sysinfo.h target/include/cvmx-sysinfo.h
ln -s ../../executive/cvmx-asm.h target/include/cvmx-asm.h
ln -s ../../executive/cvmx-npi.h target/include/cvmx-npi.h
ln -s ../../executive/cvmx-thunder.h target/include/cvmx-thunder.h
ln -s ../../executive/cvmx-asx.h target/include/cvmx-asx.h
ln -s ../../executive/cvmx-lmc.h target/include/cvmx-lmc.h
ln -s ../../executive/cvmx-bootmem.h target/include/cvmx-bootmem.h
ln -s ../../executive/cvmx-csr-addresses.h target/include/cvmx-csr-addresses.h
ln -s ../../executive/cvmx-csr-enums.h target/include/cvmx-csr-enums.h
ln -s ../../executive/cvmx-csr-typedefs.h target/include/cvmx-csr-typedefs.h
ln -s ../../executive/cvmx-version.h target/include/cvmx-version.h

ln -s ../../simulator/boot-reset/boot_reset target/bin/boot_reset

ln -s ../../executive/cvmx-shared-linux-o32.ld target/lib/cvmx-shared-linux-o32.ld
ln -s ../../executive/cvmx-shared-linux.ld target/lib/cvmx-shared-linux.ld
ln -s ../../executive/cvmx-shared-linux-n32.ld target/lib/cvmx-shared-linux-n32.ld
