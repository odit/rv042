/****************************************************************
 * Copyright (c) 2005, Cavium Networks. All rights reserved.
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
 * Support library for the SPI4000 card
 *
 * File version info: $Id: cvmx-spi4000.c 2 2007-04-05 08:51:12Z tt $ $Name$
 */

#if defined(__KERNEL__) && defined(__linux__)
    #include <linux/kernel.h>
    #include <linux/string.h>
    #define printf printk
#else
    #include <stdio.h>
    #include <string.h>
#endif
#include "cvmx-config.h"
#include "cvmx.h"
#include "cvmx-mio.h"
#include "cvmx-pko.h"
#include "cvmx-spi.h"

/* If someone is using an old config, make the SPI4000 act like RGMII for backpressure */
#ifndef CVMX_HELPER_DISABLE_SPI4000_BACKPRESSURE
#ifndef CVMX_HELPER_DISABLE_RGMII_BACKPRESSURE
#define CVMX_HELPER_DISABLE_RGMII_BACKPRESSURE 0
#endif
#define CVMX_HELPER_DISABLE_SPI4000_BACKPRESSURE CVMX_HELPER_DISABLE_RGMII_BACKPRESSURE
#endif

#define SPI4000_BASE(interface)             (0x8000660000000000ull + ((interface)*0x0000010000000000ull))
#define SPI4000_READ_ADDRESS(interface)     (SPI4000_BASE(interface) | 0xf0)
#define SPI4000_WRITE_ADDRESS(interface)    (SPI4000_BASE(interface) | 0xf2)
#define SPI4000_WRITE_DATA(interface)       (SPI4000_BASE(interface) | 0xf8)
#define SPI4000_DO_READ(inteface)           (SPI4000_BASE(interface) | 0xfc)
#define SPI4000_DO_WRITE(interface)         (SPI4000_BASE(interface) | 0xfe)
#define SPI4000_READ_DATA(interface)        (SPI4000_BASE(interface) | 0x030000f400000000ull)
#define SPI4000_READ_DATA_CONT(interface)   (SPI4000_BASE(interface) | 0x0100000000000000ull)
#define SPI4000_GET_READ_STATUS(interface)  (SPI4000_BASE(interface) | 0x030000fd00000000ull)
#define SPI4000_GET_WRITE_STATUS(interface) (SPI4000_BASE(interface) | 0x030000ff00000000ull)
#ifndef MS
#define MS                          (400000ull) /*  Not exactly a millisecond, but close enough for our delays */
#endif

/* MDI Single Command (register 0x680) */
typedef union
{
    uint32_t u32;
    struct
    {
        uint32_t    reserved_21_31  : 11;
        uint32_t    mdi_command     : 1; /**< Performs an MDIO access. When set, this bit
                                            self clears upon completion of the access. */
        uint32_t    reserved_18_19  : 2;
        uint32_t    op_code         : 2; /**< MDIO Op Code
                                            00 = Reserved
                                            01 = Write Access
                                            10 = Read Access
                                            11 = Reserved */
        uint32_t    reserved_13_15  : 3;
        uint32_t    phy_address     : 5; /**< Address of external PHY device */
        uint32_t    reserved_5_7    : 3;
        uint32_t    reg_address     : 5; /**< Address of register within external PHY */
    } s;
} mdio_single_command_t;


static CVMX_SHARED int interface_is_spi4000[2] = {0,0};

/**
 * Read a value from the twsi interface
 *
 * @return Value after the read completes
 */
static inline uint64_t cvmx_twsi_read(void)
{
    cvmx_mio_tws_sw_twsi_t result;
    do
    {
        result.u64 = cvmx_read_csr(CVMX_MIO_TWS_SW_TWSI);
    } while (result.s.v);

    if (result.s.r == 0)
    {
#ifdef DEBUG
        cvmx_dprintf("SPI4000: TWSI Read failed\n");
#endif
        result.s.d = 0;
    }
    return result.u64;
}


/**
 * Write a command to the twsi interface
 *
 * @param value  Value to write
 */
static inline int cvmx_twsi_write(uint64_t value)
{
    cvmx_mio_tws_sw_twsi_t result;
    cvmx_write_csr(CVMX_MIO_TWS_SW_TWSI, value);

    result.u64 = cvmx_twsi_read();
    if (result.s.r == 0)
        return -1;
    else
        return 0;
}


/**
 * Write data to the specified SPI4000 address
 *
 * @param address Address to write to
 * @param data    Data to write
 */
static void cvmx_spi4000_write(int interface, int address, uint32_t data)
{
    uint8_t    addressh, addressl;
    uint8_t    data0, data1, data2, data3;
    uint64_t   start_cycle;

    // Set the write address
    addressh = (uint8_t) ((address >> 8) & 0xFF);
    addressl = (uint8_t) (address & 0xFF);

    cvmx_twsi_write(SPI4000_WRITE_ADDRESS(interface));
    cvmx_twsi_write(SPI4000_BASE(interface) | addressh);
    cvmx_twsi_write(SPI4000_BASE(interface) | addressl);

    // Set write data
    data0 = (uint8_t) ((data >> 24) & 0xFF);
    data1 = (uint8_t) ((data >> 16) & 0xFF);
    data2 = (uint8_t) ((data >>  8) & 0xFF);
    data3 = (uint8_t) ((data >>  0) & 0xFF);

    cvmx_twsi_write(SPI4000_WRITE_DATA(interface));
    cvmx_twsi_write(SPI4000_BASE(interface) | data0);
    cvmx_twsi_write(SPI4000_BASE(interface) | data1);
    cvmx_twsi_write(SPI4000_BASE(interface) | data2);
    cvmx_twsi_write(SPI4000_BASE(interface) | data3);

    // Do the write
    cvmx_twsi_write(SPI4000_DO_WRITE(interface));

    // check the status
    start_cycle = cvmx_get_cycle();
    do
    {
        if (cvmx_get_cycle() - start_cycle > 1000*MS)
            break;
        cvmx_twsi_write(SPI4000_GET_WRITE_STATUS(interface));
    } while ((cvmx_twsi_read() & 0xFF) != 4);
}


/**
 * Read data from the SPI4000.
 *
 * @param address Address to read from
 * @return Value at the specified address
 */
static uint32_t cvmx_spi4000_read(int interface, int address)
{
    uint8_t    addressh, addressl;
    uint8_t    data0, data1, data2, data3;
    uint64_t   result;

    addressh = (uint8_t) ((address >> 8) & 0xFF);
    addressl = (uint8_t) (address & 0xFF);

    cvmx_twsi_write(SPI4000_READ_ADDRESS(interface));
    cvmx_twsi_write(SPI4000_BASE(interface) | addressh);
    cvmx_twsi_write(SPI4000_BASE(interface) | addressl);

    // Do the read
    cvmx_twsi_write(SPI4000_DO_READ(interface));
    result = cvmx_twsi_read();
    while (result & 0xFF)
    {
        // check the status
        cvmx_twsi_write(SPI4000_GET_READ_STATUS(interface));
        result = cvmx_twsi_read();
    }

    cvmx_twsi_write(SPI4000_READ_DATA(interface));
    data0 = (uint8_t) (cvmx_twsi_read() & 0xFF);

    cvmx_twsi_write(SPI4000_READ_DATA_CONT(interface));
    data1 = (uint8_t) (cvmx_twsi_read() & 0xFF);

    cvmx_twsi_write(SPI4000_READ_DATA_CONT(interface));
    data2 = (uint8_t) (cvmx_twsi_read() & 0xFF);

    cvmx_twsi_write(SPI4000_READ_DATA_CONT(interface));
    data3 = (uint8_t) (cvmx_twsi_read() & 0xFF);

    return((data0<<24) | (data1<<16) | (data2<<8) | data3);
}


/**
 * Write to a PHY using MDIO on the SPI4000
 *
 * @param interface Interface the SPI4000 is on. (0 or 1)
 * @param port      SPI4000 RGMII port to write to. (0-9)
 * @param location  MDIO register to write
 * @param val       Value to write
 */
static void cvmx_spi4000_mdio_write(int interface, int port, int location, int val)
{
    static int last_value=-1;
    mdio_single_command_t mdio;

    mdio.u32 = 0;
    mdio.s.mdi_command = 1;
    mdio.s.op_code = 1;
    mdio.s.phy_address = port;
    mdio.s.reg_address = location;

    /* Since the TWSI accesses are very slow, don't update the write value
        if it is the same as the last value */
    if (val != last_value)
    {
        last_value = val;
        cvmx_spi4000_write(interface, 0x0681, val);
    }

    cvmx_spi4000_write(interface, 0x0680, mdio.u32);
}


/**
 * Read from a PHY using MDIO on the SPI4000
 *
 * @param interface Interface the SPI4000 is on. (0 or 1)
 * @param port      SPI4000 RGMII port to read from. (0-9)
 * @param location  MDIO register to read
 * @return The MDI read result
 */
static int cvmx_spi4000_mdio_read(int interface, int port, int location)
{
    mdio_single_command_t mdio;

    mdio.u32 = 0;
    mdio.s.mdi_command = 1;
    mdio.s.op_code = 2;
    mdio.s.phy_address = port;
    mdio.s.reg_address = location;
    cvmx_spi4000_write(interface, 0x0680, mdio.u32);

    do
    {
        mdio.u32 = cvmx_spi4000_read(interface, 0x0680);
    } while (mdio.s.mdi_command);

    return cvmx_spi4000_read(interface, 0x0681) >> 16;
}


/**
 * Configure the SPI4000 MACs
 */
static void cvmx_spi4000_configure_mac(int interface)
{
    int port;
    // IXF1010 configuration
    // ---------------------
    //
    // Step 1: Apply soft reset to TxFIFO and MAC
    //         MAC soft reset register. address=0x505
    //         TxFIFO soft reset. address=0x620
    cvmx_spi4000_write(interface, 0x0505, 0x3ff);  // reset all the MACs
    cvmx_spi4000_write(interface, 0x0620, 0x3ff);  // reset the TX FIFOs

    //         Global address and Configuration Register. address=0x500
    //
    // Step 2: Apply soft reset to RxFIFO and SPI.
    cvmx_spi4000_write(interface, 0x059e, 0x3ff);  // reset the RX FIFOs

    // Step 3a: Take the MAC out of softreset
    //          MAC soft reset register. address=0x505
    cvmx_spi4000_write(interface, 0x0505, 0x0);    // reset all the MACs

    // Step 3b: De-assert port enables.
    //          Global address and Configuration Register. address=0x500
    cvmx_spi4000_write(interface, 0x0500, 0x0);    // disable all ports

    // Step 4: Assert Clock mode change En.
    //         Clock and interface mode Change En. address=Serdes base + 0x14
    //         Serdes (Serializer/de-serializer). address=0x780
    //         [Can't find this one]

    for (port=0; port < 10; port++)
    {
        int port_offset = port << 7;

        // Step 5: Set MAC interface mode GMII speed.
        //         MAC interface mode and RGMII speed register.
        //             address=port_index+0x10
        //
        //         OUT port_index+0x10, 0x07     //RGMII 1000 Mbps operation.
        cvmx_spi4000_write(interface, port_offset | 0x0010, 0x3);

        // Step 6: Change Interface to Copper mode
        //         Interface mode register. address=0x501
        //         [Can't find this]

        // Step 7: MAC configuration
        //         Station address configuration.
        //         Source MAC address low register. Source MAC address 31-0.
        //             address=port_index+0x00
        //         Source MAC address high register. Source MAC address 47-32.
        //             address=port_index+0x01
        //         where Port index is 0x0 to 0x5.
        //         This address is inserted in the source address filed when
        //         transmitting pause frames, and is also used to compare against
        //         unicast pause frames at the receiving side.
        //
        //         OUT port_index+0x00, source MAC address low.
        cvmx_spi4000_write(interface, port_offset | 0x0000, 0x0000);
        //         OUT port_index+0x01, source MAC address high.
        cvmx_spi4000_write(interface, port_offset | 0x0001, 0x0000);

        // Step 8: Set desired duplex mode
        //         Desired duplex register. address=port_index+0x02
        //         [Reserved]

        // Step 9: Other configuration.
        //         FC Enable Register.             address=port_index+0x12
        //         Discard Unknown Control Frame.  address=port_index+0x15
        //         Diverse config write register.  address=port_index+0x18
        //         RX Packet Filter register.      address=port_index+0x19
        //
        // Step 9a: Tx FD FC Enabled / Rx FD FC Enabled
        if (CVMX_HELPER_DISABLE_SPI4000_BACKPRESSURE)
            cvmx_spi4000_write(interface, port_offset | 0x0012, 0);
        else
            cvmx_spi4000_write(interface, port_offset | 0x0012, 0x7);

        // Step 9b: Discard unknown control frames
        cvmx_spi4000_write(interface, port_offset | 0x0015, 0x1);

        // Step 9c: Enable auto-CRC and auto-padding
        cvmx_spi4000_write(interface, port_offset | 0x0018, 0x11cd); //??

        // Step 9d: Drop bad CRC / Drop Pause / No DAF
        cvmx_spi4000_write(interface, port_offset | 0x0019, 0x00);
    }

    // Step 9d: Drop frames
    cvmx_spi4000_write(interface, 0x059f, 0x03ff);

    for (port=0; port < 10; port++)
    {
        // Step 9e: Set the TX FIFO marks
        cvmx_spi4000_write(interface, port + 0x0600, 0x0900); // TXFIFO High watermark
        cvmx_spi4000_write(interface, port + 0x060a, 0x0800); // TXFIFO Low watermark
        cvmx_spi4000_write(interface, port + 0x0614, 0x0380); // TXFIFO threshold
    }

    // Step 12: De-assert RxFIFO and SPI Rx/Tx reset
    cvmx_spi4000_write(interface, 0x059e, 0x0);    // reset the RX FIFOs

    // Step 13: De-assert TxFIFO and MAC reset
    cvmx_spi4000_write(interface, 0x0620, 0x0);    // reset the TX FIFOs

    // Step 14: Assert port enable
    //          Global address and Configuration Register. address=0x500
    cvmx_spi4000_write(interface, 0x0500, 0x03ff); // enable all ports

    // Step 15: Disable loopback
    //          [Can't find this one]
}


/**
 * Configure the SPI4000 PHYs
 */
static void cvmx_spi4000_configure_phy(int interface)
{
    int port;

    /* We use separate loops below since it allows us to save a write
        to the SPI4000 for each repeated value. This adds up to a couple
        of seconds */

    /* Update the link state before resets. It takes a while for the links to
        come back after the resets. Most likely they'll come back the same as
        they are now */
    for (port=0; port < 10; port++)
        cvmx_spi4000_check_speed(interface, port);
    /* Enable RGMII DELAYS for TX_CLK and RX_CLK (see spec) */
    for (port=0; port < 10; port++)
        cvmx_spi4000_mdio_write(interface, port, 0x14, 0x00e2);
    /* Advertise pause and 100 Full Duplex. Don't advertise half duplex or 10Mbpa */
    for (port=0; port < 10; port++)
        cvmx_spi4000_mdio_write(interface, port, 0x4, 0x0d01);
    /* Enable PHY reset */
    for (port=0; port < 10; port++)
        cvmx_spi4000_mdio_write(interface, port, 0x0, 0x9140);
}


/**
 * Poll all the SPI4000 port and check its speed
 *
 * @param interface Interface the SPI4000 is on
 * @param port      Port to poll (0-9)
 * @return Status of the port. 0=down. All other values the port is up.
 */
cvmx_gmxx_rxx_rx_inbnd_t cvmx_spi4000_check_speed(int interface, int port)
{
#if  !defined(__KERNEL__) && CVMX_ENABLE_DEBUG_PRINTS
    const char *SPEED[] = {"10Mbps", "100Mbps", "1Gbps", "Reserved"};
#endif
    static int phy_status[10] = {0,};
    cvmx_gmxx_rxx_rx_inbnd_t link;
    int read_status;

    link.u64 = 0;

    if (!interface_is_spi4000[interface])
        return link;
    if (port>=10)
        return link;

    /* Register 0x11: PHY Specific Status Register
         Register   Function         Setting                     Mode   HW Rst SW Rst Notes
                                                                 RO     00     Retain note
         17.15:14   Speed            11 = Reserved
                                                                                      17.a
                                     10 = 1000 Mbps
                                     01 = 100 Mbps
                                     00 = 10 Mbps
         17.13      Duplex           1 = Full-duplex             RO     0      Retain note
                                     0 = Half-duplex                                  17.a
         17.12      Page Received    1 = Page received           RO, LH 0      0
                                     0 = Page not received
                                     1 = Resolved                RO     0      0      note
         17.11      Speed and
                                     0 = Not resolved                                 17.a
                    Duplex
                    Resolved
         17.10      Link (real time) 1 = Link up                 RO     0      0
                                     0 = Link down
                                                                 RO     000    000    note
                                     000 = < 50m
         17.9:7     Cable Length
                                     001 = 50 - 80m                                   17.b
                    (100/1000
                                     010 = 80 - 110m
                    modes only)
                                     011 = 110 - 140m
                                     100 = >140m
         17.6       MDI Crossover    1 = MDIX                    RO     0      0      note
                    Status           0 = MDI                                          17.a
         17.5       Downshift Sta-   1 = Downshift               RO     0      0
                    tus              0 = No Downshift
         17.4       Energy Detect    1 = Sleep                   RO     0      0
                    Status           0 = Active
         17.3       Transmit Pause   1 = Transmit pause enabled  RO     0      0      note17.
                    Enabled          0 = Transmit pause disabled                      a, 17.c
         17.2       Receive Pause    1 = Receive pause enabled   RO     0      0      note17.
                    Enabled          0 = Receive pause disabled                       a, 17.c
         17.1       Polarity (real   1 = Reversed                RO     0      0
                    time)            0 = Normal
         17.0       Jabber (real     1 = Jabber                  RO     0      Retain
                    time)            0 = No jabber
    */
    read_status = cvmx_spi4000_mdio_read(interface, port, 0x11);
    if ((read_status & (1<<10)) == 0)
        read_status = 0; /* If the link is down, force zero */
    else
        read_status &= 0xe400; /* Strip off all the don't care bits */
    if (read_status != phy_status[port])
    {
        phy_status[port] = read_status;
        if (read_status & (1<<10))
        {
            /* If the link is up, we need to set the speed based on the PHY status */
            if (read_status & (1<<15))
                cvmx_spi4000_write(interface, (port<<7) | 0x0010, 0x3); /* 1Gbps */
            else
                cvmx_spi4000_write(interface, (port<<7) | 0x0010, 0x1); /* 100Mbps */
#ifndef __KERNEL__
            cvmx_dprintf("Port %d: Up %s %s duplex\n", port + 16*interface,
                   SPEED[((read_status>>14) & 0x3)],
                   (read_status & (1<<13)) ? "Full" : "Unsupported Half");
#endif
        }
        else
        {
            /* If the link is down, force 1Gbps so TX traffic dumps fast */
            cvmx_spi4000_write(interface, (port<<7) | 0x0010, 0x3); /* 1Gbps */
#ifndef __KERNEL__
            cvmx_dprintf("Port %d: Down\n", port + 16*interface);
#endif
        }
    }

    if (read_status & (1<<10))
    {
        link.s.status = 1; /* Link up */
        if (read_status & (1<<15))
            link.s.speed = 2;
        else
            link.s.speed = 1;
    }
    else
    {
        link.s.speed = 2; /* Use 1Gbps when down */
        link.s.status = 0; /* Link Down */
    }
    link.s.duplex = ((read_status & (1<<13)) != 0);

    return link;
}


/**
 * Initialize the SPI4000 for use
 *
 * @param interface SPI interface the SPI4000 is connected to
 */
int cvmx_spi4000_initialize(int interface)
{
    if (!(OCTEON_IS_MODEL(OCTEON_CN38XX) || OCTEON_IS_MODEL(OCTEON_CN58XX)))
        return -1;
    // Check for the presence of a SPI4000. If it isn't there,
    // these writes will timeout.
    if (cvmx_twsi_write(SPI4000_WRITE_ADDRESS(interface)))
        return -1;
    if (cvmx_twsi_write(SPI4000_BASE(interface)))
        return -1;
    if (cvmx_twsi_write(SPI4000_BASE(interface)))
        return -1;

    interface_is_spi4000[interface] = 1;
    cvmx_spi4000_configure_mac(interface);
    cvmx_spi4000_configure_phy(interface);
    return 0;
}

