/* ==========================================================================
 * $File: //dwh/usb_iip/dev/software/otg_ipmate/linux/drivers/dwc_otg_regs.h $
 * $Revision: 2 $
 * $Date: 2007-04-05 16:51:12 +0800 (Thu, 05 Apr 2007) $
 * $Change: 525573 $
 *
 * Synopsys HS OTG Linux Software Driver and documentation (hereinafter,
 * "Software") is an Unsupported proprietary work of Synopsys, Inc. unless
 * otherwise expressly agreed to in writing between Synopsys and you.
 *
 * The Software IS NOT an item of Licensed Software or Licensed Product under
 * any End User Software License Agreement or Agreement for Licensed Product
 * with Synopsys or any supplement thereto. You are permitted to use and
 * redistribute this Software in source and binary forms, with or without
 * modification, provided that redistributions of source code must retain this
 * notice. You may not view, use, disclose, copy or distribute this file or
 * any information contained herein except pursuant to this license grant from
 * Synopsys. If you do not agree with this notice, including the disclaimer
 * below, then you are not authorized to use the Software.
 *
 * THIS SOFTWARE IS BEING DISTRIBUTED BY SYNOPSYS SOLELY ON AN "AS IS" BASIS
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE HEREBY DISCLAIMED. IN NO EVENT SHALL SYNOPSYS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 * ========================================================================== */

#ifndef __DWC_OTG_REGS_H__
#define __DWC_OTG_REGS_H__

/**
 * @file
 *
 * This file contains the data structures for accessing the DWC_otg core registers.
 *
 * The application interfaces with the HS OTG core by reading from and
 * writing to the Control and Status Register (CSR) space through the
 * AHB Slave interface. These registers are 32 bits wide, and the
 * addresses are 32-bit-block aligned.
 * CSRs are classified as follows:
 * - Core Global Registers
 * - Device Mode Registers
 * - Device Global Registers
 * - Device Endpoint Specific Registers
 * - Host Mode Registers
 * - Host Global Registers
 * - Host Port CSRs
 * - Host Channel Specific Registers
 *
 * Only the Core Global registers can be accessed in both Device and
 * Host modes. When the HS OTG core is operating in one mode, either
 * Device or Host, the application must not access registers from the
 * other mode. When the core switches from one mode to another, the
 * registers in the new mode of operation must be reprogrammed as they
 * would be after a power-on reset.
 */

/****************************************************************************/
/** DWC_otg Core registers .
 * The dwc_otg_core_global_regs structure defines the size
 * and relative field offsets for the Core Global registers.
 */
typedef struct dwc_otg_core_global_regs
{
    /** OTG Control and Status Register.  <i>Offset: 000h</i> */
    volatile uint32_t gotgctl;
    /** OTG Interrupt Register.  <i>Offset: 004h</i> */
    volatile uint32_t gotgint;
    /**Core AHB Configuration Register.  <i>Offset: 008h</i> */
    volatile uint32_t gahbcfg;
#define DWC_GLBINTRMASK 	0x0001
#define DWC_DMAENABLE   	0x0020
#define DWC_NPTXEMPTYLVL_EMPTY 	0x0080
#define DWC_NPTXEMPTYLVL_HALFEMPTY 	0x0000
#define DWC_PTXEMPTYLVL_EMPTY 	0x0100
#define DWC_PTXEMPTYLVL_HALFEMPTY 	0x0000


    /**Core USB Configuration Register.  <i>Offset: 00Ch</i> */
    volatile uint32_t gusbcfg;
    /**Core Reset Register.  <i>Offset: 010h</i> */
    volatile uint32_t grstctl;
    /**Core Interrupt Register.  <i>Offset: 014h</i> */
    volatile uint32_t gintsts;
    /**Core Interrupt Mask Register.  <i>Offset: 018h</i> */
    volatile uint32_t gintmsk;
    /**Receive Status Queue Read Register (Read Only).  <i>Offset: 01Ch</i> */
    volatile uint32_t grxstsr;
    /**Receive Status Queue Read & POP Register (Read Only).  <i>Offset: 020h</i>*/
    volatile uint32_t grxstsp;
    /**Receive FIFO Size Register.  <i>Offset: 024h</i> */
    volatile uint32_t grxfsiz;
    /**Non Periodic Transmit FIFO Size Register.  <i>Offset: 028h</i> */
    volatile uint32_t gnptxfsiz;
    /**Non Periodic Transmit FIFO/Queue Status Register (Read
     * Only). <i>Offset: 02Ch</i> */
    volatile uint32_t gnptxsts;
    /**I2C Access Register.  <i>Offset: 030h</i> */
    volatile uint32_t gi2cctl;
    /**PHY Vendor Control Register.  <i>Offset: 034h</i> */
    volatile uint32_t gpvndctl;
    /**General Purpose Input/Output Register.  <i>Offset: 038h</i> */
    volatile uint32_t ggpio;
    /**User ID Register.  <i>Offset: 03Ch</i> */
    volatile uint32_t guid;
    /**Synopsys ID Register (Read Only).  <i>Offset: 040h</i> */
    volatile uint32_t gsnpsid;
    /**User HW Config1 Register (Read Only).  <i>Offset: 044h</i> */
    volatile uint32_t ghwcfg1;
    /**User HW Config2 Register (Read Only).  <i>Offset: 048h</i> */
    volatile uint32_t ghwcfg2;
#define DWC_SLAVE_ONLY_ARCH 0
#define DWC_EXT_DMA_ARCH 1
#define DWC_INT_DMA_ARCH 2

#define DWC_MODE_HNP_SRP_CAPABLE 	0
#define DWC_MODE_SRP_ONLY_CAPABLE 	1
#define DWC_MODE_NO_HNP_SRP_CAPABLE 	2
#define DWC_MODE_SRP_CAPABLE_DEVICE 	3
#define DWC_MODE_NO_SRP_CAPABLE_DEVICE  4
#define DWC_MODE_SRP_CAPABLE_HOST 	5
#define DWC_MODE_NO_SRP_CAPABLE_HOST  	6

    /**User HW Config3 Register (Read Only).  <i>Offset: 04Ch</i> */
    volatile uint32_t ghwcfg3;
    /**User HW Config4 Register (Read Only).  <i>Offset: 050h</i>*/
    volatile uint32_t ghwcfg4;
    /** Reserved  <i>Offset: 054h-0FFh</i> */
    uint32_t reserved[43];
    /** Host Periodic Transmit FIFO Size Register. <i>Offset: 100h</i> */
    volatile uint32_t hptxfsiz;
    /** Device Periodic Transmit FIFO#n Register.
     * <i>Offset: 104h + (FIFO_Number-1)*04h, 1 <= FIFO Number <= 15 (1<=n<=15).</i> */
    volatile uint32_t dptxfsiz[15];
} dwc_otg_core_global_regs_t;

/**
 * This union represents the bit fields of the Core OTG Control
 * and Status Register (GOTGCTL).  Set the bits using the bit
 * fields then write the <i>d32</i> value to the register.
 */
typedef union gotgctl_data
{
    /** raw register data */
    uint32_t d32;
    /** register bits */
    struct
    {
#ifdef __BIG_ENDIAN_BITFIELD
        unsigned reserved21_31 : 11;
        unsigned currmod : 1;
        unsigned bsesvld : 1;
        unsigned asesvld : 1;
        unsigned reserved17 : 1;
        unsigned conidsts : 1;
        unsigned reserved12_15 : 4;
        unsigned devhnpen : 1;
        unsigned hstsethnpen : 1;
        unsigned hnpreq : 1;
        unsigned hstnegscs : 1;
        unsigned reserved2_7 : 6;
        unsigned sesreq : 1;
        unsigned sesreqscs : 1;
#else
        unsigned sesreqscs : 1;
        unsigned sesreq : 1;
        unsigned reserved2_7 : 6;
        unsigned hstnegscs : 1;
        unsigned hnpreq : 1;
        unsigned hstsethnpen : 1;
        unsigned devhnpen : 1;
        unsigned reserved12_15 : 4;
        unsigned conidsts : 1;
        unsigned reserved17 : 1;
        unsigned asesvld : 1;
        unsigned bsesvld : 1;
        unsigned currmod : 1;
        unsigned reserved21_31 : 11;
#endif
    } b;
} gotgctl_data_t;

/**
 * This union represents the bit fields of the Core OTG Interrupt Register
 * (GOTGINT).  Set/clear the bits using the bit fields then write the <i>d32</i>
 * value to the register.
 */
typedef union gotgint_data
{
    /** raw register data */
    uint32_t d32;
    /** register bits */
    struct
    {
#ifdef __BIG_ENDIAN_BITFIELD
        unsigned reserved31_20 : 12;
        unsigned debdone : 1;
        unsigned adevtoutchng : 1;
        unsigned hstnegdet : 1;
        unsigned reserver10_16 : 7;
        unsigned hstnegsucstschng : 1;
        unsigned sesreqsucstschng : 1;
        unsigned reserved3_7 : 5;
        unsigned sesenddet : 1;
        unsigned reserved0_1 : 2;
#else

        /** Current Mode */
        unsigned reserved0_1 : 2;

        /** Session End Detected */
        unsigned sesenddet : 1;

        unsigned reserved3_7 : 5;

        /** Session Request Success Status Change */
        unsigned sesreqsucstschng : 1;
        /** Host Negotiation Success Status Change */
        unsigned hstnegsucstschng : 1;

        unsigned reserver10_16 : 7;

        /** Host Negotiation Detected */
        unsigned hstnegdet : 1;
        /** A-Device Timeout Change */
        unsigned adevtoutchng : 1;
        /** Debounce Done */
        unsigned debdone : 1;

        unsigned reserved31_20 : 12;
#endif
    } b;
} gotgint_data_t;


/**
 * This union represents the bit fields of the Core AHB Configuration
 * Register (GAHBCFG).  Set/clear the bits using the bit fields then
 * write the <i>d32</i> value to the register.
 */
typedef union gahbcfg_data
{
    /** raw register data */
    uint32_t d32;
    /** register bits */
    struct
    {
#define DWC_GAHBCFG_TXFEMPTYLVL_HALFEMPTY	0
#define DWC_GAHBCFG_TXFEMPTYLVL_EMPTY 		1
#define DWC_GAHBCFG_DMAENABLE   		1
#define DWC_GAHBCFG_INT_DMA_BURST_INCR16 	7
#define DWC_GAHBCFG_INT_DMA_BURST_INCR8 	5
#define DWC_GAHBCFG_INT_DMA_BURST_INCR4 	3
#define DWC_GAHBCFG_INT_DMA_BURST_INCR 		1
#define DWC_GAHBCFG_INT_DMA_BURST_SINGLE 	0
#define DWC_GAHBCFG_GLBINT_ENABLE 		1

#ifdef __BIG_ENDIAN_BITFIELD
        unsigned reserved9_31 : 23;
        unsigned ptxfemplvl : 1;
        unsigned nptxfemplvl : 1;
        unsigned reserved : 1;
        unsigned dmaenable : 1;
        unsigned hburstlen : 4;
        unsigned glblintrmsk : 1;
#else
        unsigned glblintrmsk : 1;
        unsigned hburstlen : 4;
        unsigned dmaenable : 1;
        unsigned reserved : 1;
        unsigned nptxfemplvl : 1;
        unsigned ptxfemplvl : 1;
        unsigned reserved9_31 : 23;
#endif
    } b;
} gahbcfg_data_t;

/**
 * This union represents the bit fields of the Core USB Configuration
 * Register (GUSBCFG).  Set the bits using the bit fields then write
 * the <i>d32</i> value to the register.
 */
typedef union gusbcfg_data
{
    /** raw register data */
    uint32_t d32;
    /** register bits */
    struct
    {
#ifdef __BIG_ENDIAN_BITFIELD
        unsigned reserved : 9;
        unsigned term_sel_dl_pulse : 1;
        unsigned ulpi_int_vbus_indicator : 1;
        unsigned ulpi_ext_vbus_drv : 1;
        unsigned ulpi_clk_sus_m : 1;
        unsigned ulpi_auto_res : 1;
        unsigned ulpi_fsls : 1;
        unsigned otgutmifssel : 1;
        unsigned phylpwrclksel : 1;
        unsigned nptxfrwnden : 1;
        unsigned usbtrdtim : 4;
        unsigned hnpcap : 1;
        unsigned srpcap : 1;
        unsigned ddrsel : 1;
        unsigned physel : 1;
        unsigned fsintf : 1;
        unsigned ulpi_utmi_sel : 1;
        unsigned phyif : 1;
        unsigned toutcal : 3;
#else
        unsigned toutcal : 3;
        unsigned phyif : 1;
        unsigned ulpi_utmi_sel : 1;
        unsigned fsintf : 1;
        unsigned physel : 1;
        unsigned ddrsel : 1;
        unsigned srpcap : 1;
        unsigned hnpcap : 1;
        unsigned usbtrdtim : 4;
        unsigned nptxfrwnden : 1;
        unsigned phylpwrclksel : 1;
        unsigned otgutmifssel : 1;
        unsigned ulpi_fsls : 1;
        unsigned ulpi_auto_res : 1;
        unsigned ulpi_clk_sus_m : 1;
        unsigned ulpi_ext_vbus_drv : 1;
        unsigned ulpi_int_vbus_indicator : 1;
        unsigned term_sel_dl_pulse : 1;
        unsigned reserved : 9;
#endif
    } b;
} gusbcfg_data_t;

/**
 * This union represents the bit fields of the Core Reset Register
 * (GRSTCTL).  Set/clear the bits using the bit fields then write the
 * <i>d32</i> value to the register.
 */
typedef union grstctl_data
{
    /** raw register data */
    uint32_t d32;
    /** register bits */
    struct
    {
#ifdef __BIG_ENDIAN_BITFIELD
        unsigned ahbidle : 1;
        unsigned dmareq : 1;
        unsigned reserved11_29 : 19;
        unsigned txfnum : 5;
        unsigned txfflsh : 1;
        unsigned rxfflsh : 1;
        unsigned intknqflsh : 1;
        unsigned hstfrm : 1;
        unsigned hsftrst : 1;
        unsigned csftrst : 1;
#else

        /** Core Soft Reset (CSftRst) (Device and Host)
         *
         * The application can flush the control logic in the
         * entire core using this bit. This bit resets the
         * pipelines in the AHB Clock domain as well as the
         * PHY Clock domain.
         *
         * The state machines are reset to an IDLE state, the
         * control bits in the CSRs are cleared, all the
         * transmit FIFOs and the receive FIFO are flushed.
         *
         * The status mask bits that control the generation of
         * the interrupt, are cleared, to clear the
         * interrupt. The interrupt status bits are not
         * cleared, so the application can get the status of
         * any events that occurred in the core after it has
         * set this bit.
         *
         * Any transactions on the AHB are terminated as soon
         * as possible following the protocol. Any
         * transactions on the USB are terminated immediately.
         *
         * The configuration settings in the CSRs are
         * unchanged, so the software doesn't have to
         * reprogram these registers (Device
         * Configuration/Host Configuration/Core System
         * Configuration/Core PHY Configuration).
         *
         * The application can write to this bit, any time it
         * wants to reset the core. This is a self clearing
         * bit and the core clears this bit after all the
         * necessary logic is reset in the core, which may
         * take several clocks, depending on the current state
         * of the core.
         */
        unsigned csftrst : 1;
        /** Hclk Soft Reset
 *
 * The application uses this bit to reset the control logic in
 * the AHB clock domain. Only AHB clock domain pipelines are
 * reset.
 */
        unsigned hsftrst : 1;
        /** Host Frame Counter Reset (Host Only)<br>
         *
         * The application can reset the (micro)frame number
         * counter inside the core, using this bit. When the
         * (micro)frame counter is reset, the subsequent SOF
         * sent out by the core, will have a (micro)frame
         * number of 0.
         */
        unsigned hstfrm : 1;
        /** In Token Sequence Learning Queue Flush
         * (INTknQFlsh) (Device Only)
         */
        unsigned intknqflsh : 1;
        /** RxFIFO Flush (RxFFlsh) (Device and Host)
         *
         * The application can flush the entire Receive FIFO
         * using this bit.  <p>The application must first
         * ensure that the core is not in the middle of a
         * transaction.  <p>The application should write into
         * this bit, only after making sure that neither the
         * DMA engine is reading from the RxFIFO nor the MAC
         * is writing the data in to the FIFO.  <p>The
         * application should wait until the bit is cleared
         * before performing any other operations. This bit
         * will takes 8 clocks (slowest of PHY or AHB clock)
         * to clear.
         */
        unsigned rxfflsh : 1;
        /** TxFIFO Flush (TxFFlsh) (Device and Host).
         *
         * This bit is used to selectively flush a single or
         * all transmit FIFOs.  The application must first
         * ensure that the core is not in the middle of a
         * transaction.  <p>The application should write into
         * this bit, only after making sure that neither the
         * DMA engine is writing into the TxFIFO nor the MAC
         * is reading the data out of the FIFO.  <p>The
         * application should wait until the core clears this
         * bit, before performing any operations. This bit
         * will takes 8 clocks (slowest of PHY or AHB clock)
         * to clear.
         */
        unsigned txfflsh : 1;

        /** TxFIFO Number (TxFNum) (Device and Host).
         *
         * This is the FIFO number which needs to be flushed,
         * using the TxFIFO Flush bit. This field should not
         * be changed until the TxFIFO Flush bit is cleared by
         * the core.
         *   - 0x0 : Non Periodic TxFIFO Flush
         *   - 0x1 : Periodic TxFIFO #1 Flush in device mode
         *     or Periodic TxFIFO in host mode
         *   - 0x2 : Periodic TxFIFO #2 Flush in device mode.
         *   - ...
         *   - 0xF : Periodic TxFIFO #15 Flush in device mode
         *   - 0x10: Flush all the Transmit NonPeriodic and
         *     Transmit Periodic FIFOs in the core
         */
        unsigned txfnum : 5;
        /** Reserved */
        unsigned reserved11_29 : 19;
        /** DMA Request Signal.  Indicated DMA request is in
         * probress.  Used for debug purpose. */
        unsigned dmareq : 1;
        /** AHB Master Idle.  Indicates the AHB Master State
         * Machine is in IDLE condition. */
        unsigned ahbidle : 1;
#endif
    } b;
} grstctl_t;


/**
 * This union represents the bit fields of the Core Interrupt Mask
 * Register (GINTMSK).  Set/clear the bits using the bit fields then
 * write the <i>d32</i> value to the register.
 */
typedef union gintmsk_data
{
    /** raw register data */
    uint32_t d32;
    /** register bits */
    struct
    {
#ifdef __BIG_ENDIAN_BITFIELD
        unsigned wkupintr : 1;
        unsigned sessreqintr : 1;
        unsigned disconnect : 1;
        unsigned conidstschng : 1;
        unsigned reserved27 : 1;
        unsigned ptxfempty : 1;
        unsigned hcintr : 1;
        unsigned portintr : 1;
        unsigned reserved22_23 : 2;
        unsigned incomplisoout : 1;
        unsigned incomplisoin : 1;
        unsigned outepintr : 1;
        unsigned inepintr : 1;
        unsigned epmismatch : 1;
        unsigned reserved16 : 1;
        unsigned eopframe : 1;
        unsigned isooutdrop : 1;
        unsigned enumdone : 1;
        unsigned usbreset : 1;
        unsigned usbsuspend : 1;
        unsigned erlysuspend : 1;
        unsigned i2cintr : 1;
        unsigned reserved8 : 1;
        unsigned goutnakeff : 1;
        unsigned ginnakeff : 1;
        unsigned nptxfempty : 1;
        unsigned rxstsqlvl : 1;
        unsigned sofintr : 1;
        unsigned otgintr : 1;
        unsigned modemismatch : 1;
        unsigned reserved0 : 1;
#else
        unsigned reserved0 : 1;
        unsigned modemismatch : 1;
        unsigned otgintr : 1;
        unsigned sofintr : 1;
        unsigned rxstsqlvl : 1;
        unsigned nptxfempty : 1;
        unsigned ginnakeff : 1;
        unsigned goutnakeff : 1;
        unsigned reserved8 : 1;
        unsigned i2cintr : 1;
        unsigned erlysuspend : 1;
        unsigned usbsuspend : 1;
        unsigned usbreset : 1;
        unsigned enumdone : 1;
        unsigned isooutdrop : 1;
        unsigned eopframe : 1;
        unsigned reserved16 : 1;
        unsigned epmismatch : 1;
        unsigned inepintr : 1;
        unsigned outepintr : 1;
        unsigned incomplisoin : 1;
        unsigned incomplisoout : 1;
        unsigned reserved22_23 : 2;
        unsigned portintr : 1;
        unsigned hcintr : 1;
        unsigned ptxfempty : 1;
        unsigned reserved27 : 1;
        unsigned conidstschng : 1;
        unsigned disconnect : 1;
        unsigned sessreqintr : 1;
        unsigned wkupintr : 1;
#endif
    } b;
} gintmsk_data_t;
/**
 * This union represents the bit fields of the Core Interrupt Register
 * (GINTSTS).  Set/clear the bits using the bit fields then write the
 * <i>d32</i> value to the register.
 */
typedef union gintsts_data
{
    /** raw register data */
    uint32_t d32;
#define DWC_SOF_INTR_MASK 0x0008

    /** register bits */
    struct
    {
#define DWC_HOST_MODE 1
#ifdef __BIG_ENDIAN_BITFIELD
        unsigned wkupintr : 1;
        unsigned sessreqintr : 1;
        unsigned disconnect : 1;
        unsigned conidstschng : 1;
        unsigned reserved27 : 1;
        unsigned ptxfempty : 1;
        unsigned hcintr : 1;
        unsigned portintr : 1;
        unsigned reserved22_23 : 2;
        unsigned incomplisoout : 1;
        unsigned incomplisoin : 1;
        unsigned outepintr : 1;
        unsigned inepint: 1;
        unsigned epmismatch : 1;
        unsigned intokenrx : 1;
        unsigned eopframe : 1;
        unsigned isooutdrop : 1;
        unsigned enumdone : 1;
        unsigned usbreset : 1;
        unsigned usbsuspend : 1;
        unsigned erlysuspend : 1;
        unsigned i2cintr : 1;
        unsigned reserved8 : 1;
        unsigned goutnakeff : 1;
        unsigned ginnakeff : 1;
        unsigned nptxfempty : 1;
        unsigned rxstsqlvl : 1;
        unsigned sofintr : 1;
        unsigned otgintr : 1;
        unsigned modemismatch : 1;
        unsigned curmode : 1;
#else
        unsigned curmode : 1;
        unsigned modemismatch : 1;
        unsigned otgintr : 1;
        unsigned sofintr : 1;
        unsigned rxstsqlvl : 1;
        unsigned nptxfempty : 1;
        unsigned ginnakeff : 1;
        unsigned goutnakeff : 1;
        unsigned reserved8 : 1;
        unsigned i2cintr : 1;
        unsigned erlysuspend : 1;
        unsigned usbsuspend : 1;
        unsigned usbreset : 1;
        unsigned enumdone : 1;
        unsigned isooutdrop : 1;
        unsigned eopframe : 1;
        unsigned intokenrx : 1;
        unsigned epmismatch : 1;
        unsigned inepint: 1;
        unsigned outepintr : 1;
        unsigned incomplisoin : 1;
        unsigned incomplisoout : 1;
        unsigned reserved22_23 : 2;
        unsigned portintr : 1;
        unsigned hcintr : 1;
        unsigned ptxfempty : 1;
        unsigned reserved27 : 1;
        unsigned conidstschng : 1;
        unsigned disconnect : 1;
        unsigned sessreqintr : 1;
        unsigned wkupintr : 1;
#endif
    } b;
} gintsts_data_t;


/**
 * This union represents the bit fields in the Device Receive Status Read and
 * Pop Registers (GRXSTSR, GRXSTSP) Read the register into the <i>d32</i>
 * element then read out the bits using the <i>b</i>it elements.
 */
typedef union device_grxsts_data
{
    /** raw register data */
    uint32_t d32;
    /** register bits */
    struct
    {
#define DWC_DSTS_SETUP_UPDT	0x6               // SETUP Packet
#define DWC_DSTS_SETUP_COMP 	0x4               // Setup Phase Complete
#define DWC_DSTS_GOUT_NAK   	0x1               // Global OUT NAK
#define DWC_STS_XFER_COMP   	0x3               // OUT Data Transfer Complete
#define DWC_STS_DATA_UPDT   	0x2               // OUT Data Packet

#ifdef __BIG_ENDIAN_BITFIELD
        unsigned reserved : 7;
        unsigned fn : 4;
        unsigned pktsts : 4;
        unsigned dpid : 2;
        unsigned bcnt : 11;
        unsigned epnum : 4;
#else
        unsigned epnum : 4;
        unsigned bcnt : 11;
        unsigned dpid : 2;
        unsigned pktsts : 4;
        unsigned fn : 4;
        unsigned reserved : 7;
#endif
    } b;
} device_grxsts_data_t;

/**
 * This union represents the bit fields in the Host Receive Status Read and
 * Pop Registers (GRXSTSR, GRXSTSP) Read the register into the <i>d32</i>
 * element then read out the bits using the <i>b</i>it elements.
 */
typedef union host_grxsts_data
{
    /** raw register data */
    uint32_t d32;
    /** register bits */
    struct
    {
#define DWC_GRXSTS_PKTSTS_CH_HALTED       0x7
#define DWC_GRXSTS_PKTSTS_DATA_TOGGLE_ERR 0x5
#define DWC_GRXSTS_PKTSTS_IN_XFER_COMP    0x3
#define DWC_GRXSTS_PKTSTS_IN              0x2

#ifdef __BIG_ENDIAN_BITFIELD
        unsigned reserved : 11;
        unsigned pktsts : 4;
        unsigned dpid : 2;
        unsigned bcnt : 11;
        unsigned chnum : 4;
#else
        unsigned chnum : 4;
        unsigned bcnt : 11;
        unsigned dpid : 2;
        unsigned pktsts : 4;
        unsigned reserved : 11;
#endif
    } b;
} host_grxsts_data_t;

/**
 * This union represents the bit fields in the FIFO Size Registers (HPTXFSIZ,
 * GNPTXFSIZ, DPTXFSIZn). Read the register into the <i>d32</i> element then
 * read out the bits using the <i>b</i>it elements.
 */
typedef union fifosize_data
{
    /** raw register data */
    uint32_t d32;
    /** register bits */
    struct
    {
#ifdef __BIG_ENDIAN_BITFIELD
        unsigned depth : 16;
        unsigned startaddr : 16;
#else
        unsigned startaddr : 16;
        unsigned depth : 16;
#endif
    } b;
} fifosize_data_t;

/**
 * This union represents the bit fields in the Non-Periodic Transmit
 * FIFO/Queue Status Register (GNPTXSTS). Read the register into the
 * <i>d32</i> element then read out the bits using the <i>b</i>it
 * elements.
 */
typedef union gnptxsts_data
{
    /** raw register data */
    uint32_t d32;
    /** register bits */
    struct
    {
#ifdef __BIG_ENDIAN_BITFIELD
        unsigned reserved : 1;
        unsigned nptxqtop_chnep : 4;
        unsigned nptxqtop_token : 2;
        unsigned nptxqtop_terminate : 1;
        unsigned nptxqspcavail : 8;
        unsigned nptxfspcavail : 16;
#else
        unsigned nptxfspcavail : 16;
        unsigned nptxqspcavail : 8;
        /** Top of the Non-Periodic Transmit Request Queue
         *  - bit 24 - Terminate (Last entry for the selected
         *    channel/EP)
         *  - bits 26:25 - Token Type
         *    - 2'b00 - IN/OUT
         *    - 2'b01 - Zero Length OUT
         *    - 2'b10 - PING/Complete Split
         *    - 2'b11 - Channel Halt
         *  - bits 30:27 - Channel/EP Number
         */
        unsigned nptxqtop_terminate : 1;
        unsigned nptxqtop_token : 2;
        unsigned nptxqtop_chnep : 4;
        unsigned reserved : 1;
#endif
    } b;
} gnptxsts_data_t;

/**
 * This union represents the bit fields in the I2C Control Register
 * (I2CCTL). Read the register into the <i>d32</i> element then read out the
 * bits using the <i>b</i>it elements.
 */
typedef union gi2cctl_data
{
    /** raw register data */
    uint32_t d32;
    /** register bits */
    struct
    {
#ifdef __BIG_ENDIAN_BITFIELD
        unsigned bsydne : 1;
        unsigned rw : 1;
        unsigned reserved : 2;
        unsigned i2cdevaddr : 2;
        unsigned i2csuspctl : 1;
        unsigned ack : 1;
        unsigned i2cen : 1;
        unsigned addr : 7;
        unsigned regaddr : 8;
        unsigned rwdata : 8;
#else
        unsigned rwdata : 8;
        unsigned regaddr : 8;
        unsigned addr : 7;
        unsigned i2cen : 1;
        unsigned ack : 1;
        unsigned i2csuspctl : 1;
        unsigned i2cdevaddr : 2;
        unsigned reserved : 2;
        unsigned rw : 1;
        unsigned bsydne : 1;
#endif
    } b;
} gi2cctl_data_t;

/**
 * This union represents the bit fields in the User HW Config1
 * Register.  Read the register into the <i>d32</i> element then read
 * out the bits using the <i>b</i>it elements.
 */
typedef union hwcfg1_data
{
    /** raw register data */
    uint32_t d32;
    /** register bits */
    struct
    {
#ifdef __BIG_ENDIAN_BITFIELD
        unsigned ep_dir15 : 2;
        unsigned ep_dir14 : 2;
        unsigned ep_dir13 : 2;
        unsigned ep_dir12 : 2;
        unsigned ep_dir11 : 2;
        unsigned ep_dir10 : 2;
        unsigned ep_dir9 : 2;
        unsigned ep_dir8 : 2;
        unsigned ep_dir7 : 2;
        unsigned ep_dir6 : 2;
        unsigned ep_dir5 : 2;
        unsigned ep_dir4 : 2;
        unsigned ep_dir3 : 2;
        unsigned ep_dir2 : 2;
        unsigned ep_dir1 : 2;
        unsigned ep_dir0 : 2;
#else
        unsigned ep_dir0 : 2;
        unsigned ep_dir1 : 2;
        unsigned ep_dir2 : 2;
        unsigned ep_dir3 : 2;
        unsigned ep_dir4 : 2;
        unsigned ep_dir5 : 2;
        unsigned ep_dir6 : 2;
        unsigned ep_dir7 : 2;
        unsigned ep_dir8 : 2;
        unsigned ep_dir9 : 2;
        unsigned ep_dir10 : 2;
        unsigned ep_dir11 : 2;
        unsigned ep_dir12 : 2;
        unsigned ep_dir13 : 2;
        unsigned ep_dir14 : 2;
        unsigned ep_dir15 : 2;
#endif
    } b;
} hwcfg1_data_t;

/**
 * This union represents the bit fields in the User HW Config2
 * Register.  Read the register into the <i>d32</i> element then read
 * out the bits using the <i>b</i>it elements.
 */
typedef union hwcfg2_data
{
    /** raw register data */
    uint32_t d32;
    /** register bits */
    struct
    {
#define DWC_HWCFG2_HS_PHY_TYPE_UTMI_ULPI 3
#define DWC_HWCFG2_HS_PHY_TYPE_ULPI 2
#define DWC_HWCFG2_HS_PHY_TYPE_UTMI 1
#define DWC_HWCFG2_HS_PHY_TYPE_NOT_SUPPORTED 0
#define DWC_HWCFG2_OP_MODE_NO_SRP_CAPABLE_HOST 6
#define DWC_HWCFG2_OP_MODE_SRP_CAPABLE_HOST 5
#define DWC_HWCFG2_OP_MODE_NO_SRP_CAPABLE_DEVICE 4
#define DWC_HWCFG2_OP_MODE_SRP_CAPABLE_DEVICE 3
#define DWC_HWCFG2_OP_MODE_NO_HNP_SRP_CAPABLE_OTG 2
#define DWC_HWCFG2_OP_MODE_SRP_ONLY_CAPABLE_OTG 1
#define DWC_HWCFG2_OP_MODE_HNP_SRP_CAPABLE_OTG 0

#ifdef __BIG_ENDIAN_BITFIELD
        unsigned reserved31 : 1;
        unsigned dev_token_q_depth : 5;
        unsigned host_perio_tx_q_depth : 2;
        unsigned nonperio_tx_q_depth : 2;
        unsigned rx_status_q_depth : 2;
        unsigned dynamic_fifo : 1;
        unsigned perio_ep_supported : 1;
        unsigned num_host_chan : 4;
        unsigned num_dev_ep : 4;
        unsigned fs_phy_type : 2;
        unsigned hs_phy_type : 2;
        unsigned point2point : 1;
        unsigned architecture : 2;
        unsigned op_mode : 3;
#else
        unsigned op_mode : 3;
        unsigned architecture : 2;
        unsigned point2point : 1;
        unsigned hs_phy_type : 2;
        unsigned fs_phy_type : 2;
        unsigned num_dev_ep : 4;
        unsigned num_host_chan : 4;
        unsigned perio_ep_supported : 1;
        unsigned dynamic_fifo : 1;
        unsigned rx_status_q_depth : 2;
        unsigned nonperio_tx_q_depth : 2;
        unsigned host_perio_tx_q_depth : 2;
        unsigned dev_token_q_depth : 5;
        unsigned reserved31 : 1;
#endif
    } b;
} hwcfg2_data_t;

/**
 * This union represents the bit fields in the User HW Config3
 * Register.  Read the register into the <i>d32</i> element then read
 * out the bits using the <i>b</i>it elements.
 */
typedef union hwcfg3_data
{
    /** raw register data */
    uint32_t d32;
    /** register bits */
    struct
    {
        /* GHWCFG3 */
#ifdef __BIG_ENDIAN_BITFIELD
        unsigned dfifo_depth : 16;
        unsigned reserved15_13 : 3;
        unsigned ahb_phy_clock_synch : 1;
        unsigned synch_reset_type : 1;
        unsigned optional_features : 1;
        unsigned vendor_ctrl_if : 1;
        unsigned i2c : 1;
        unsigned otg_func : 1;
        unsigned packet_size_cntr_width : 3;
        unsigned xfer_size_cntr_width : 4;
#else
        unsigned xfer_size_cntr_width : 4;
        unsigned packet_size_cntr_width : 3;
        unsigned otg_func : 1;
        unsigned i2c : 1;
        unsigned vendor_ctrl_if : 1;
        unsigned optional_features : 1;
        unsigned synch_reset_type : 1;
        unsigned ahb_phy_clock_synch : 1;
        unsigned reserved15_13 : 3;
        unsigned dfifo_depth : 16;
#endif
    } b;
} hwcfg3_data_t;

/**
 * This union represents the bit fields in the User HW Config4
 * Register.  Read the register into the <i>d32</i> element then read
 * out the bits using the <i>b</i>it elements.
 */
typedef union hwcfg4_data
{
    /** raw register data */
    uint32_t d32;
    /** register bits */
    struct
    {
#ifdef __BIG_ENDIAN_BITFIELD
        unsigned reserved31_25 : 7;
        unsigned session_end_filt_en : 1;
        unsigned b_valid_filt_en : 1;
        unsigned a_valid_filt_en : 1;
        unsigned vbus_valid_filt_en : 1;
        unsigned iddig_filt_en : 1;
        unsigned num_dev_mode_ctrl_ep : 4;
        unsigned utmi_phy_data_width : 2;
        unsigned min_ahb_freq : 9;
        unsigned power_optimiz : 1;
        unsigned num_dev_perio_in_ep : 4;
#else
        unsigned num_dev_perio_in_ep : 4;
        unsigned power_optimiz : 1;
        unsigned min_ahb_freq : 9;
        unsigned utmi_phy_data_width : 2;
        unsigned num_dev_mode_ctrl_ep : 4;
        unsigned iddig_filt_en : 1;
        unsigned vbus_valid_filt_en : 1;
        unsigned a_valid_filt_en : 1;
        unsigned b_valid_filt_en : 1;
        unsigned session_end_filt_en : 1;
        unsigned reserved31_25 : 7;
#endif
    } b;
} hwcfg4_data_t;

////////////////////////////////////////////
// Device Registers
/**
 * Device Global Registers. <i>Offsets 800h-BFFh</i>
 *
 * The following structures define the size and relative field offsets
 * for the Device Mode Registers.
 *
 * <i>These registers are visible only in Device mode and must not be
 * accessed in Host mode, as the results are unknown.</i>
 */
typedef struct dwc_otg_dev_global_regs
{
    /** Device Configuration Register. <i>Offset 800h</i> */
    volatile uint32_t dcfg;
    /** Device Control Register. <i>Offset: 804h</i> */
    volatile uint32_t dctl;
    /** Device Status Register (Read Only). <i>Offset: 808h</i> */
    volatile uint32_t dsts;
    /** Reserved. <i>Offset: 80Ch</i> */
    uint32_t unused;
    /** Device IN Endpoint Common Interrupt Mask
     * Register. <i>Offset: 810h</i> */
    volatile uint32_t diepmsk;
    /** Device OUT Endpoint Common Interrupt Mask
     * Register. <i>Offset: 814h</i> */
    volatile uint32_t doepmsk;
    /** Device All Endpoints Interrupt Register.  <i>Offset: 818h</i> */
    volatile uint32_t daint;
    /** Device All Endpoints Interrupt Mask Register.  <i>Offset:
     * 81Ch</i> */
    volatile uint32_t daintmsk;
    /** Device IN Token Queue Read Register-1 (Read Only).
     * <i>Offset: 820h</i> */
    volatile uint32_t dtknqr1;
    /** Device IN Token Queue Read Register-2 (Read Only).
     * <i>Offset: 824h</i> */
    volatile uint32_t dtknqr2;
    /** Device VBUS  discharge Register.  <i>Offset: 828h</i> */
    volatile uint32_t dvbusdis;
    /** Device VBUS Pulse Register.  <i>Offset: 82Ch</i> */
    volatile uint32_t dvbuspulse;
    /** Device IN Token Queue Read Register-3 (Read Only).
     * <i>Offset: 830h</i> */
    volatile uint32_t dtknqr3;
    /** Device IN Token Queue Read Register-4 (Read Only).
     * <i>Offset: 834h</i> */
    volatile uint32_t dtknqr4;
} dwc_otg_device_global_regs_t;

/**
 * This union represents the bit fields in the Device Configuration
 * Register.  Read the register into the <i>d32</i> member then
 * set/clear the bits using the <i>b</i>it elements.  Write the
 * <i>d32</i> member to the dcfg register.
 */
typedef union dcfg_data
{
    /** raw register data */
    uint32_t d32;
    /** register bits */
    struct
    {
#define DWC_DCFG_FRAME_INTERVAL_95 3
#define DWC_DCFG_FRAME_INTERVAL_90 2
#define DWC_DCFG_FRAME_INTERVAL_85 1
#define DWC_DCFG_FRAME_INTERVAL_80 0
#define DWC_DCFG_SEND_STALL 1

#ifdef __BIG_ENDIAN_BITFIELD
        unsigned reserved9 : 10;
        unsigned epmscnt : 4;
        unsigned reserved13_17 : 5;
        unsigned perfrint : 2;
        unsigned devaddr : 7;
        unsigned reserved3 : 1;
        unsigned nzstsouthshk : 1;
        unsigned devspd : 2;
#else

        /** Device Speed */
        unsigned devspd : 2;
        /** Non Zero Length Status OUT Handshake */
        unsigned nzstsouthshk : 1;
        unsigned reserved3 : 1;
        /** Device Addresses */
        unsigned devaddr : 7;
        /** Periodic Frame Interval */
        unsigned perfrint : 2;
        unsigned reserved13_17 : 5;
        /** In Endpoint Mis-match count */
        unsigned epmscnt : 4;
        unsigned reserved9 : 10;
#endif
    } b;
} dcfg_data_t;

/**
 * This union represents the bit fields in the Device Control
 * Register.  Read the register into the <i>d32</i> member then
 * set/clear the bits using the <i>b</i>it elements.
 */
typedef union dctl_data
{
    /** raw register data */
    uint32_t d32;
    /** register bits */
    struct
    {
#ifdef __BIG_ENDIAN_BITFIELD
        unsigned reserved : 21;
        unsigned cgoutnak : 1;
        unsigned sgoutnak : 1;
        unsigned cgnpinnak : 1;
        unsigned sgnpinnak : 1;
        unsigned tstctl : 3;
        unsigned goutnaksts : 1;
        unsigned gnpinnaksts : 1;
        unsigned sftdiscon : 1;
        unsigned rmtwkupsig : 1;
#else

        /** Remote Wakeup */
        unsigned rmtwkupsig : 1;
        /** Soft Disconnect */
        unsigned sftdiscon : 1;
        /** Global Non-Periodic IN NAK Status */
        unsigned gnpinnaksts : 1;
        /** Global OUT NAK Status */
        unsigned goutnaksts : 1;
        /** Test Control */
        unsigned tstctl : 3;
        /** Set Global Non-Periodic IN NAK */
        unsigned sgnpinnak : 1;
        /** Clear Global Non-Periodic IN NAK */
        unsigned cgnpinnak : 1;
        /** Set Global OUT NAK */
        unsigned sgoutnak : 1;
        /** Clear Global OUT NAK */
        unsigned cgoutnak : 1;

        unsigned reserved : 21;
#endif
    } b;
} dctl_data_t;

/**
 * This union represents the bit fields in the Device Status
 * Register.  Read the register into the <i>d32</i> member then
 * set/clear the bits using the <i>b</i>it elements.
 */
typedef union dsts_data
{
    /** raw register data */
    uint32_t d32;
    /** register bits */
    struct
    {
#define DWC_DSTS_ENUMSPD_FS_PHY_48MHZ          3
#define DWC_DSTS_ENUMSPD_LS_PHY_6MHZ           2
#define DWC_DSTS_ENUMSPD_FS_PHY_30MHZ_OR_60MHZ 1
#define DWC_DSTS_ENUMSPD_HS_PHY_30MHZ_OR_60MHZ 0

#ifdef __BIG_ENDIAN_BITFIELD
        unsigned reserved22_31 : 10;
        unsigned soffn : 14;
        unsigned reserved4_7: 4;
        unsigned errticerr : 1;
        unsigned enumspd : 2;
        unsigned suspsts : 1;
#else

        /** Suspend Status */
        unsigned suspsts : 1;
        /** Enumerated Speed */
        unsigned enumspd : 2;
        /** Erratic Error */
        unsigned errticerr : 1;
        unsigned reserved4_7: 4;
        /** Frame or Microframe Number of the received SOF */
        unsigned soffn : 14;
        unsigned reserved22_31 : 10;
#endif
    } b;
} dsts_data_t;


/**
 * This union represents the bit fields in the Device IN EP Interrupt
 * Register and the Device IN EP Common Mask Register.
 *
 * - Read the register into the <i>d32</i> member then set/clear the
 *   bits using the <i>b</i>it elements.
 */
typedef union diepint_data
{
    /** raw register data */
    uint32_t d32;
    /** register bits */
    struct
    {
#ifdef __BIG_ENDIAN_BITFIELD
        unsigned reserved07_31 : 25;
        unsigned inepnakeff : 1;
        unsigned intknepmis : 1;
        unsigned intktxfemp : 1;
        unsigned timeout : 1;
        unsigned ahberr : 1;
        unsigned epdisabled : 1;
        unsigned xfercompl : 1;
#else

        /** Transfer complete mask */
        unsigned xfercompl : 1;
        /** Endpoint disable mask */
        unsigned epdisabled : 1;
        /** AHB Error mask */
        unsigned ahberr : 1;
        /** TimeOUT Handshake mask (non-ISOC EPs) */
        unsigned timeout : 1;
        /** IN Token received with TxF Empty mask */
        unsigned intktxfemp : 1;
        /** IN Token Received with EP mismatch mask */
        unsigned intknepmis : 1;
        /** IN Endpoint HAK Effective mask */
        unsigned inepnakeff : 1;
        unsigned reserved07_31 : 25;
#endif
    } b;
} diepint_data_t;
/**
 * This union represents the bit fields in the Device IN EP Common
 * Interrupt Mask Register.
 */
typedef union diepint_data diepmsk_data_t;

/**
 * This union represents the bit fields in the Device OUT EP Interrupt
 * Registerand Device OUT EP Common Interrupt Mask Register.
 *
 * - Read the register into the <i>d32</i> member then set/clear the
 *   bits using the <i>b</i>it elements.
 */
typedef union doepint_data
{
    /** raw register data */
    uint32_t d32;
    /** register bits */
    struct
    {
#ifdef __BIG_ENDIAN_BITFIELD
        unsigned reserved04_31 : 28;
        unsigned setup : 1;
        unsigned ahberr : 1;
        unsigned epdisabled : 1;
        unsigned xfercompl : 1;
#else

        /** Transfer complete */
        unsigned xfercompl : 1;
        /** Endpoint disable  */
        unsigned epdisabled : 1;
        /** AHB Error */
        unsigned ahberr : 1;
        /** Setup Phase Done (contorl EPs) */
        unsigned setup : 1;
        unsigned reserved04_31 : 28;
#endif
    } b;
} doepint_data_t;
/**
 * This union represents the bit fields in the Device OUT EP Common
 * Interrupt Mask Register.
 */
typedef union doepint_data doepmsk_data_t;


/**
 * This union represents the bit fields in the Device All EP Interrupt
 * and Mask Registers.
 * - Read the register into the <i>d32</i> member then set/clear the
 *   bits using the <i>b</i>it elements.
 */
typedef union daint_data
{
    /** raw register data */
    uint32_t d32;
    /** register bits */
    struct
    {
#ifdef __BIG_ENDIAN_BITFIELD
        unsigned out : 16;
        unsigned in : 16;
#else

        /** IN Endpoint bits */
        unsigned in : 16;
        /** OUT Endpoint bits */
        unsigned out : 16;
#endif
    } ep;
    struct
    {
#ifdef __BIG_ENDIAN_BITFIELD
        unsigned outep15 : 1;
        unsigned outep14 : 1;
        unsigned outep13 : 1;
        unsigned outep12 : 1;
        unsigned outep11 : 1;
        unsigned outep10 : 1;
        unsigned outep9  : 1;
        unsigned outep8  : 1;
        unsigned outep7  : 1;
        unsigned outep6  : 1;
        unsigned outep5  : 1;
        unsigned outep4  : 1;
        unsigned outep3  : 1;
        unsigned outep2  : 1;
        unsigned outep1  : 1;
        unsigned outep0  : 1;
        unsigned inep15 : 1;
        unsigned inep14 : 1;
        unsigned inep13 : 1;
        unsigned inep12 : 1;
        unsigned inep11 : 1;
        unsigned inep10 : 1;
        unsigned inep9  : 1;
        unsigned inep8  : 1;
        unsigned inep7  : 1;
        unsigned inep6  : 1;
        unsigned inep5  : 1;
        unsigned inep4  : 1;
        unsigned inep3  : 1;
        unsigned inep2  : 1;
        unsigned inep1  : 1;
        unsigned inep0  : 1;
#else

        /** IN Endpoint bits */
        unsigned inep0  : 1;
        unsigned inep1  : 1;
        unsigned inep2  : 1;
        unsigned inep3  : 1;
        unsigned inep4  : 1;
        unsigned inep5  : 1;
        unsigned inep6  : 1;
        unsigned inep7  : 1;
        unsigned inep8  : 1;
        unsigned inep9  : 1;
        unsigned inep10 : 1;
        unsigned inep11 : 1;
        unsigned inep12 : 1;
        unsigned inep13 : 1;
        unsigned inep14 : 1;
        unsigned inep15 : 1;
        /** OUT Endpoint bits */
        unsigned outep0  : 1;
        unsigned outep1  : 1;
        unsigned outep2  : 1;
        unsigned outep3  : 1;
        unsigned outep4  : 1;
        unsigned outep5  : 1;
        unsigned outep6  : 1;
        unsigned outep7  : 1;
        unsigned outep8  : 1;
        unsigned outep9  : 1;
        unsigned outep10 : 1;
        unsigned outep11 : 1;
        unsigned outep12 : 1;
        unsigned outep13 : 1;
        unsigned outep14 : 1;
        unsigned outep15 : 1;
#endif
    } b;
} daint_data_t;

/**
 * This union represents the bit fields in the Device IN Token Queue
 * Read Registers.
 * - Read the register into the <i>d32</i> member.
 * - READ-ONLY Register
 */
typedef union dtknq1_data
{
    /** raw register data */
    uint32_t d32;
    /** register bits */
    struct
    {
#ifdef __BIG_ENDIAN_BITFIELD
        unsigned epnums0_5 : 24;
        unsigned wrap_bit : 1;
        unsigned reserved05_06 : 2;
        unsigned intknwptr : 5;
#else

        /** In Token Queue Write Pointer */
        unsigned intknwptr : 5;
        /** Reserved */
        unsigned reserved05_06 : 2;
        /** write pointer has wrapped. */
        unsigned wrap_bit : 1;
        /** EP Numbers of IN Tokens 0 ... 4 */
        unsigned epnums0_5 : 24;
#endif
    }b;
} dtknq1_data_t;

/**
 * Device Logical IN Endpoint-Specific Registers. <i>Offsets
 * 900h-AFCh</i>
 *
 * There will be one set of endpoint registers per logical endpoint
 * implemented.
 *
 * <i>These registers are visible only in Device mode and must not be
 * accessed in Host mode, as the results are unknown.</i>
 */
typedef struct dwc_otg_dev_in_ep_regs
{
    /** Device IN Endpoint Control Register. <i>Offset:900h +
     * (ep_num * 20h) + 00h</i> */
    volatile uint32_t diepctl;
    /** Reserved. <i>Offset:900h + (ep_num * 20h) + 04h</i> */
    uint32_t reserved04;
    /** Device IN Endpoint Interrupt Register. <i>Offset:900h +
     * (ep_num * 20h) + 08h</i> */
    volatile uint32_t diepint;
    /** Reserved. <i>Offset:900h + (ep_num * 20h) + 0Ch</i> */
    uint32_t reserved0C;
    /** Device IN Endpoint Transfer Size
     * Register. <i>Offset:900h + (ep_num * 20h) + 10h</i> */
    volatile uint32_t dieptsiz;
    /** Device IN Endpoint DMA Address Register. <i>Offset:900h +
     * (ep_num * 20h) + 14h</i> */
    volatile uint32_t diepdma;
    /** Reserved. <i>Offset:900h + (ep_num * 20h) + 18h - 900h +
     * (ep_num * 20h) + 1Ch</i>*/
    uint32_t reserved18[2];
} dwc_otg_dev_in_ep_regs_t;

/**
 * Device Logical OUT Endpoint-Specific Registers. <i>Offsets:
 * B00h-CFCh</i>
 *
 * There will be one set of endpoint registers per logical endpoint
 * implemented.
 *
 * <i>These registers are visible only in Device mode and must not be
 * accessed in Host mode, as the results are unknown.</i>
 */
typedef struct dwc_otg_dev_out_ep_regs
{
    /** Device OUT Endpoint Control Register. <i>Offset:B00h +
     * (ep_num * 20h) + 00h</i> */
    volatile uint32_t doepctl;
    /** Device OUT Endpoint Frame number Register.  <i>Offset:
     * B00h + (ep_num * 20h) + 04h</i> */
    volatile uint32_t doepfn;
    /** Device OUT Endpoint Interrupt Register. <i>Offset:B00h +
     * (ep_num * 20h) + 08h</i> */
    volatile uint32_t doepint;
    /** Reserved. <i>Offset:B00h + (ep_num * 20h) + 0Ch</i> */
    uint32_t reserved0C;
    /** Device OUT Endpoint Transfer Size Register. <i>Offset:
     * B00h + (ep_num * 20h) + 10h</i> */
    volatile uint32_t doeptsiz;
    /** Device OUT Endpoint DMA Address Register. <i>Offset:B00h
     * + (ep_num * 20h) + 14h</i> */
    volatile uint32_t doepdma;
    /** Reserved. <i>Offset:B00h + (ep_num * 20h) + 18h - B00h +
     * (ep_num * 20h) + 1Ch</i> */
    uint32_t unused[2];
} dwc_otg_dev_out_ep_regs_t;

/**
 * This union represents the bit fields in the Device EP Control
 * Register.  Read the register into the <i>d32</i> member then
 * set/clear the bits using the <i>b</i>it elements.
 */
typedef union depctl_data
{
    /** raw register data */
    uint32_t d32;
    /** register bits */
    struct
    {
#define DWC_DEP0CTL_MPS_64   0
#define DWC_DEP0CTL_MPS_32   1
#define DWC_DEP0CTL_MPS_16   2
#define DWC_DEP0CTL_MPS_8    3

#ifdef __BIG_ENDIAN_BITFIELD
        unsigned mps : 11;
        unsigned epena : 1;
        unsigned epdis : 1;
        unsigned setd1pid : 1;
        unsigned setd0pid : 1;
        unsigned snak : 1;
        unsigned cnak : 1;
        unsigned txfnum : 4;
        unsigned stall : 1;
        unsigned snp : 1;
        unsigned eptype : 2;
        unsigned naksts : 1;
        unsigned dpid : 1;
        unsigned usbactep : 1;
        unsigned nextep : 4;
#else

        /** Maximum Packet Size
         * IN/OUT EPn
         * IN/OUT EP0 - 2 bits
         *   2'b00: 64 Bytes
         *   2'b01: 32
         *   2'b10: 16
         *   2'b11: 8 */
        unsigned mps : 11;
        /** Next Endpoint
         * IN EPn/IN EP0
         * OUT EPn/OUT EP0 - reserved */
        unsigned nextep : 4;

        /** USB Active Endpoint */
        unsigned usbactep : 1;

        /** Endpoint DPID (INTR/Bulk IN and OUT endpoints)
                 * This field contains the PID of the packet going to
                 * be received or transmitted on this endpoint. The
                 * application should program the PID of the first
                 * packet going to be received or transmitted on this
                 * endpoint , after the endpoint is
                 * activated. Application use the SetD1PID and
                 * SetD0PID fields of this register to program either
                 * D0 or D1 PID.
                 *
                 * The encoding for this field is
                 *   - 0: D0
                 *   - 1: D1
                 */
        unsigned dpid : 1;

        /** NAK Status */
        unsigned naksts : 1;

        /** Endpoint Type
         *  2'b00: Control
         *  2'b01: Isochronous
         *  2'b10: Bulk
         *  2'b11: Interrupt */
        unsigned eptype : 2;

        /** Snoop Mode
         * OUT EPn/OUT EP0
         * IN EPn/IN EP0 - reserved */
        unsigned snp : 1;

        /** Stall Handshake */
        unsigned stall : 1;

        /** Tx Fifo Number
         * IN EPn/IN EP0
         * OUT EPn/OUT EP0 - reserved */
        unsigned txfnum : 4;

        /** Clear NAK */
        unsigned cnak : 1;
        /** Set NAK */
        unsigned snak : 1;
        /** Set DATA0 PID (INTR/Bulk IN and OUT endpoints)
         * Writing to this field sets the Endpoint DPID (DPID)
         * field in this register to DATA0. Set Even
         * (micro)frame (SetEvenFr) (ISO IN and OUT Endpoints)
         * Writing to this field sets the Even/Odd
         * (micro)frame (EO_FrNum) field to even (micro)
         * frame.
         */
        unsigned setd0pid : 1;
        /** Set DATA1 PID (INTR/Bulk IN and OUT endpoints)
         * Writing to this field sets the Endpoint DPID (DPID)
         * field in this register to DATA1 Set Odd
         * (micro)frame (SetOddFr) (ISO IN and OUT Endpoints)
         * Writing to this field sets the Even/Odd
         * (micro)frame (EO_FrNum) field to odd (micro) frame.
         */
        unsigned setd1pid : 1;

        /** Endpoint Disable */
        unsigned epdis : 1;
        /** Endpoint Enable */
        unsigned epena : 1;
#endif
    } b;
} depctl_data_t;

/**
 * This union represents the bit fields in the Device EP Transfer
 * Size Register.  Read the register into the <i>d32</i> member then
 * set/clear the bits using the <i>b</i>it elements.
 */
typedef union deptsiz_data
{
    /** raw register data */
    uint32_t d32;
    /** register bits */
    struct
    {
#ifdef __BIG_ENDIAN_BITFIELD
        unsigned reserved : 1;
        unsigned mc : 2;
        unsigned pktcnt : 10;
        unsigned xfersize : 19;
#else

        /** Transfer size */
        unsigned xfersize : 19;
        /** Packet Count */
        unsigned pktcnt : 10;
        /** Multi Count - Periodic IN endpoints */
        unsigned mc : 2;
        unsigned reserved : 1;
#endif
    } b;
} deptsiz_data_t;

/**
 * This union represents the bit fields in the Device EP 0 Transfer
 * Size Register.  Read the register into the <i>d32</i> member then
 * set/clear the bits using the <i>b</i>it elements.
 */
typedef union deptsiz0_data
{
    /** raw register data */
    uint32_t d32;
    /** register bits */
    struct
    {
#ifdef __BIG_ENDIAN_BITFIELD
        unsigned reserved31 : 1;
        unsigned supcnt : 2;
        unsigned reserved20_28 : 9;
        unsigned pktcnt : 1;
        unsigned reserved7_18 : 12;
        unsigned xfersize : 7;
#else

        /** Transfer size */
        unsigned xfersize : 7;
        /** Reserved */
        unsigned reserved7_18 : 12;
        /** Packet Count */
        unsigned pktcnt : 1;
        /** Reserved */
        unsigned reserved20_28 : 9;
        /**Setup Packet Count (DOEPTSIZ0 Only) */
        unsigned supcnt : 2;
        unsigned reserved31 : 1;
#endif
    } b;
} deptsiz0_data_t;


/** Maximum number of Periodic FIFOs */
#define MAX_PERIO_FIFOS 15

/** Maximum number of Endpoints/HostChannels */
#define MAX_EPS_CHANNELS 16

/**
 * The dwc_otg_dev_if structure contains information needed to manage
 * the DWC_otg controller acting in device mode. It represents the
 * programming view of the device-specific aspects of the controller.
 */
typedef struct dwc_otg_dev_if
{
    /** Pointer to device Global registers.
     * Device Global Registers starting at offset 800h
     */
    dwc_otg_device_global_regs_t *dev_global_regs;
#define DWC_DEV_GLOBAL_REG_OFFSET 0x800

    /**
     * Device Logical IN Endpoint-Specific Registers 900h-AFCh
     */
    dwc_otg_dev_in_ep_regs_t     *in_ep_regs[MAX_EPS_CHANNELS];
#define DWC_DEV_IN_EP_REG_OFFSET 0x900
#define DWC_EP_REG_OFFSET 0x20

    /** Device Logical OUT Endpoint-Specific Registers B00h-CFCh */
    dwc_otg_dev_out_ep_regs_t    *out_ep_regs[MAX_EPS_CHANNELS];
#define DWC_DEV_OUT_EP_REG_OFFSET 0xB00

    /* Device configuration information*/
    uint8_t  speed;              /**< Device Speed  0: Unknown, 1: LS, 2:FS, 3: HS */
    uint8_t  num_eps;            /**< Number of EPs  range: 1-16 (includes EP0) */
    uint8_t  num_perio_eps;      /**< # of Periodic EP range: 0-15 */

    /** Size of periodic FIFOs (Bytes) */
    uint16_t perio_tx_fifo_size[MAX_PERIO_FIFOS];

} dwc_otg_dev_if_t;




/////////////////////////////////////////////////
// Host Mode Register Structures
//
/**
 * The Host Global Registers structure defines the size and relative
 * field offsets for the Host Mode Global Registers.  Host Global
 * Registers offsets 400h-7FFh.
*/
typedef struct dwc_otg_host_global_regs
{
    /** Host Configuration Register.   <i>Offset: 400h</i> */
    volatile uint32_t hcfg;
    /** Host Frame Interval Register.   <i>Offset: 404h</i> */
    volatile uint32_t hfir;
    /** Host Frame Number / Frame Remaining Register. <i>Offset: 408h</i> */
    volatile uint32_t hfnum;
    /** Reserved.   <i>Offset: 40Ch</i> */
    uint32_t reserved40C;
    /** Host Periodic Transmit FIFO/ Queue Status Register. <i>Offset: 410h</i> */
    volatile uint32_t hptxsts;
    /** Host All Channels Interrupt Register. <i>Offset: 414h</i> */
    volatile uint32_t haint;
    /** Host All Channels Interrupt Mask Register. <i>Offset: 418h</i> */
    volatile uint32_t haintmsk;
} dwc_otg_host_global_regs_t;

/**
 * This union represents the bit fields in the Host Configuration Register.
 * Read the register into the <i>d32</i> member then set/clear the bits using
 * the <i>b</i>it elements. Write the <i>d32</i> member to the hcfg register.
 */
typedef union hcfg_data
{
    /** raw register data */
    uint32_t d32;

    /** register bits */
    struct
    {
#define DWC_HCFG_6_MHZ     2
#define DWC_HCFG_48_MHZ    1
#define DWC_HCFG_30_60_MHZ 0

#ifdef __BIG_ENDIAN_BITFIELD
	unsigned reserved : 29;
        unsigned fslssupp : 1;
        unsigned fslspclksel : 2;
#else

        /** FS/LS Phy Clock Select */
        unsigned fslspclksel : 2;
        /** FS/LS Only Support */
        unsigned fslssupp : 1;
	unsigned reserved : 29;
#endif
    } b;
} hcfg_data_t;

/**
 * This union represents the bit fields in the Host Frame Remaing/Number
 * Register.
 */
typedef union hfir_data
{
    /** raw register data */
    uint32_t d32;

    /** register bits */
    struct
    {
#ifdef __BIG_ENDIAN_BITFIELD
        unsigned reserved : 16;
        unsigned frint : 16;
#else
        unsigned frint : 16;
        unsigned reserved : 16;
#endif
    } b;
} hfir_data_t;

/**
 * This union represents the bit fields in the Host Frame Remaing/Number
 * Register.
 */
typedef union hfnum_data
{
    /** raw register data */
    uint32_t d32;

    /** register bits */
    struct
    {
#define DWC_HFNUM_MAX_FRNUM 0x3FFF

#ifdef __BIG_ENDIAN_BITFIELD
        unsigned frrem : 16;
        unsigned frnum : 16;
#else
        unsigned frnum : 16;
        unsigned frrem : 16;
#endif
    } b;
} hfnum_data_t;

typedef union hptxsts_data
{
    /** raw register data */
    uint32_t d32;

    /** register bits */
    struct
    {
#ifdef __BIG_ENDIAN_BITFIELD
        unsigned ptxqtop_odd : 1;
        unsigned ptxqtop_chnum : 4;
        unsigned ptxqtop_token : 2;
        unsigned ptxqtop_terminate : 1;
        unsigned ptxqspcavail : 8;
        unsigned ptxfspcavail : 16;
#else
        unsigned ptxfspcavail : 16;
        unsigned ptxqspcavail : 8;
        /** Top of the Periodic Transmit Request Queue
         *  - bit 24 - Terminate (last entry for the selected channel)
         *  - bits 26:25 - Token Type
         *    - 2'b00 - Zero length
         *    - 2'b01 - Ping
         *    - 2'b10 - Disable
         *  - bits 30:27 - Channel Number
         *  - bit 31 - Odd/even microframe
         */
        unsigned ptxqtop_terminate : 1;
        unsigned ptxqtop_token : 2;
        unsigned ptxqtop_chnum : 4;
        unsigned ptxqtop_odd : 1;
#endif
    } b;
} hptxsts_data_t;

/**
 * This union represents the bit fields in the Host Port Control and Status
 * Register. Read the register into the <i>d32</i> member then set/clear the
 * bits using the <i>b</i>it elements. Write the <i>d32</i> member to the
 * hprt0 register.
 */
typedef union hprt0_data
{
    /** raw register data */
    uint32_t d32;
    /** register bits */
    struct
    {
#define DWC_HPRT0_PRTSPD_LOW_SPEED  2
#define DWC_HPRT0_PRTSPD_FULL_SPEED 1
#define DWC_HPRT0_PRTSPD_HIGH_SPEED 0

#ifdef __BIG_ENDIAN_BITFIELD
        unsigned reserved19_31 : 13;
        unsigned prtspd : 2;
        unsigned prttstctl : 4;
        unsigned prtpwr : 1;
        unsigned prtlnsts : 2;
        unsigned reserved9 : 1;
        unsigned prtrst : 1;
        unsigned prtsusp : 1;
        unsigned prtres : 1;
        unsigned prtovrcurrchng : 1;
        unsigned prtovrcurract : 1;
        unsigned prtenchng : 1;
        unsigned prtena : 1;
        unsigned prtconndet : 1;
        unsigned prtconnsts : 1;
#else
        unsigned prtconnsts : 1;
        unsigned prtconndet : 1;
        unsigned prtena : 1;
        unsigned prtenchng : 1;
        unsigned prtovrcurract : 1;
        unsigned prtovrcurrchng : 1;
        unsigned prtres : 1;
        unsigned prtsusp : 1;
        unsigned prtrst : 1;
        unsigned reserved9 : 1;
        unsigned prtlnsts : 2;
        unsigned prtpwr : 1;
        unsigned prttstctl : 4;
        unsigned prtspd : 2;
        unsigned reserved19_31 : 13;
#endif
    } b;
} hprt0_data_t;

/**
 * This union represents the bit fields in the Host All Interrupt
 * Register.
 */
typedef union haint_data
{
    /** raw register data */
    uint32_t d32;
    /** register bits */
    struct
    {
#ifdef __BIG_ENDIAN_BITFIELD
        unsigned reserved : 16;
        unsigned ch15 : 1;
        unsigned ch14 : 1;
        unsigned ch13 : 1;
        unsigned ch12 : 1;
        unsigned ch11 : 1;
        unsigned ch10 : 1;
        unsigned ch9 : 1;
        unsigned ch8 : 1;
        unsigned ch7 : 1;
        unsigned ch6 : 1;
        unsigned ch5 : 1;
        unsigned ch4 : 1;
        unsigned ch3 : 1;
        unsigned ch2 : 1;
        unsigned ch1 : 1;
        unsigned ch0 : 1;
#else
        unsigned ch0 : 1;
        unsigned ch1 : 1;
        unsigned ch2 : 1;
        unsigned ch3 : 1;
        unsigned ch4 : 1;
        unsigned ch5 : 1;
        unsigned ch6 : 1;
        unsigned ch7 : 1;
        unsigned ch8 : 1;
        unsigned ch9 : 1;
        unsigned ch10 : 1;
        unsigned ch11 : 1;
        unsigned ch12 : 1;
        unsigned ch13 : 1;
        unsigned ch14 : 1;
        unsigned ch15 : 1;
        unsigned reserved : 16;
#endif
    } b;
    struct
    {
#ifdef __BIG_ENDIAN_BITFIELD
        unsigned reserved : 16;
        unsigned chint : 16;
#else
        unsigned chint : 16;
        unsigned reserved : 16;
#endif
    } b2;
} haint_data_t;

/**
 * This union represents the bit fields in the Host All Interrupt
 * Register.
 */
typedef union haintmsk_data
{
    /** raw register data */
    uint32_t d32;
    /** register bits */
    struct
    {
#ifdef __BIG_ENDIAN_BITFIELD
        unsigned reserved : 16;
        unsigned ch15 : 1;
        unsigned ch14 : 1;
        unsigned ch13 : 1;
        unsigned ch12 : 1;
        unsigned ch11 : 1;
        unsigned ch10 : 1;
        unsigned ch9 : 1;
        unsigned ch8 : 1;
        unsigned ch7 : 1;
        unsigned ch6 : 1;
        unsigned ch5 : 1;
        unsigned ch4 : 1;
        unsigned ch3 : 1;
        unsigned ch2 : 1;
        unsigned ch1 : 1;
        unsigned ch0 : 1;
#else
        unsigned ch0 : 1;
        unsigned ch1 : 1;
        unsigned ch2 : 1;
        unsigned ch3 : 1;
        unsigned ch4 : 1;
        unsigned ch5 : 1;
        unsigned ch6 : 1;
        unsigned ch7 : 1;
        unsigned ch8 : 1;
        unsigned ch9 : 1;
        unsigned ch10 : 1;
        unsigned ch11 : 1;
        unsigned ch12 : 1;
        unsigned ch13 : 1;
        unsigned ch14 : 1;
        unsigned ch15 : 1;
        unsigned reserved : 16;
#endif
    } b;
    struct
    {
#ifdef __BIG_ENDIAN_BITFIELD
        unsigned reserved : 16;
        unsigned chint : 16;
#else
        unsigned chint : 16;
        unsigned reserved : 16;
#endif
    } b2;
} haintmsk_data_t;

/**
 * Host Channel Specific Registers. <i>500h-5FCh</i>
 */
typedef struct dwc_otg_hc_regs
{
    /** Host Channel 0 Characteristic Register. <i>Offset: 500h + (chan_num * 20h) + 00h</i> */
    volatile uint32_t hcchar;
    /** Host Channel 0 Split Control Register. <i>Offset: 500h + (chan_num * 20h) + 04h</i> */
    volatile uint32_t hcsplt;
    /** Host Channel 0 Interrupt Register. <i>Offset: 500h + (chan_num * 20h) + 08h</i> */
    volatile uint32_t hcint;
    /** Host Channel 0 Interrupt Mask Register. <i>Offset: 500h + (chan_num * 20h) + 0Ch</i> */
    volatile uint32_t hcintmsk;
    /** Host Channel 0 Transfer Size Register. <i>Offset: 500h + (chan_num * 20h) + 10h</i> */
    volatile uint32_t hctsiz;
    /** Host Channel 0 DMA Address Register. <i>Offset: 500h + (chan_num * 20h) + 14h</i> */
    volatile uint32_t hcdma;
    /** Reserved.  <i>Offset: 500h + (chan_num * 20h) + 18h - 500h + (chan_num * 20h) + 1Ch</i> */
    uint32_t reserved[2];
} dwc_otg_hc_regs_t;

/**
 * This union represents the bit fields in the Host Channel Characteristics
 * Register. Read the register into the <i>d32</i> member then set/clear the
 * bits using the <i>b</i>it elements. Write the <i>d32</i> member to the
 * hcchar register.
 */
typedef union hcchar_data
{
    /** raw register data */
    uint32_t d32;

    /** register bits */
    struct
    {
#ifdef __BIG_ENDIAN_BITFIELD
        unsigned chen : 1;
        unsigned chdis : 1;
        unsigned oddfrm : 1;
        unsigned devaddr : 7;
        unsigned multicnt : 2;
        unsigned eptype : 2;
        unsigned lspddev : 1;
        unsigned reserved : 1;
        unsigned epdir : 1;
        unsigned epnum : 4;
        unsigned mps : 11;
#else

        /** Maximum packet size in bytes */
        unsigned mps : 11;

        /** Endpoint number */
        unsigned epnum : 4;

        /** 0: OUT, 1: IN */
        unsigned epdir : 1;

        unsigned reserved : 1;

        /** 0: Full/high speed device, 1: Low speed device */
        unsigned lspddev : 1;

        /** 0: Control, 1: Isoc, 2: Bulk, 3: Intr */
        unsigned eptype : 2;

        /** Packets per frame for periodic transfers. 0 is reserved. */
        unsigned multicnt : 2;

        /** Device address */
        unsigned devaddr : 7;

        /**
         * Frame to transmit periodic transaction.
         * 0: even, 1: odd
         */
        unsigned oddfrm : 1;

        /** Channel disable */
        unsigned chdis : 1;

        /** Channel enable */
        unsigned chen : 1;
#endif
    } b;
} hcchar_data_t;

typedef union hcsplt_data
{
    /** raw register data */
    uint32_t d32;

    /** register bits */
    struct
    {
#define DWC_HCSPLIT_XACTPOS_ALL 3
#define DWC_HCSPLIT_XACTPOS_BEGIN 2
#define DWC_HCSPLIT_XACTPOS_END 1
#define DWC_HCSPLIT_XACTPOS_MID 0

#ifdef __BIG_ENDIAN_BITFIELD
        unsigned spltena : 1;
        unsigned reserved : 14;
        unsigned compsplt : 1;
        unsigned xactpos : 2;
        unsigned hubaddr : 7;
        unsigned prtaddr : 7;
#else

        /** Port Address */
        unsigned prtaddr : 7;

        /** Hub Address */
        unsigned hubaddr : 7;

        /** Transaction Position */
        unsigned xactpos : 2;

        /** Do Complete Split */
        unsigned compsplt : 1;

        /** Reserved */
        unsigned reserved : 14;

        /** Split Enble */
        unsigned spltena : 1;
#endif
    } b;
} hcsplt_data_t;


/**
 * This union represents the bit fields in the Host All Interrupt
 * Register.
 */
typedef union hcint_data
{
    /** raw register data */
    uint32_t d32;
    /** register bits */
    struct
    {
#ifdef __BIG_ENDIAN_BITFIELD
        unsigned reserved : 21;
        unsigned datatglerr : 1;
        unsigned frmovrun : 1;
        unsigned bblerr : 1;
        unsigned xacterr : 1;
        unsigned nyet : 1;
        unsigned ack : 1;
        unsigned nak : 1;
        unsigned stall : 1;
        unsigned ahberr : 1;
        unsigned chhltd : 1;
        unsigned xfercomp : 1;
#else

        /** Transfer Complete */
        unsigned xfercomp : 1;
        /** Channel Halted */
        unsigned chhltd : 1;
        /** AHB Error */
        unsigned ahberr : 1;
        /** STALL Response Received */
        unsigned stall : 1;
        /** NAK Response Received */
        unsigned nak : 1;
        /** ACK Response Received */
        unsigned ack : 1;
        /** NYET Response Received */
        unsigned nyet : 1;
        /** Transaction Err */
        unsigned xacterr : 1;
        /** Babble Error */
        unsigned bblerr : 1;
        /** Frame Overrun */
        unsigned frmovrun : 1;
        /** Data Toggle Error */
        unsigned datatglerr : 1;
        /** Reserved */
        unsigned reserved : 21;
#endif
    } b;
} hcint_data_t;

/**
 * This union represents the bit fields in the Host Channel Transfer Size
 * Register. Read the register into the <i>d32</i> member then set/clear the
 * bits using the <i>b</i>it elements. Write the <i>d32</i> member to the
 * hcchar register.
 */
typedef union hctsiz_data
{
    /** raw register data */
    uint32_t d32;

    /** register bits */
    struct
    {
#define DWC_HCTSIZ_SETUP 3		
#define DWC_HCTSIZ_MDATA 3
#define DWC_HCTSIZ_DATA2 1
#define DWC_HCTSIZ_DATA1 2
#define DWC_HCTSIZ_DATA0 0

#ifdef __BIG_ENDIAN_BITFIELD
        unsigned dopng : 1;
        unsigned pid : 2;
        unsigned pktcnt : 10;
        unsigned xfersize : 19;
#else

        /** Total transfer size in bytes */
        unsigned xfersize : 19;

        /** Data packets to transfer */
        unsigned pktcnt : 10;

        /**
         * Packet ID for next data packet
         * 0: DATA0
         * 1: DATA2
         * 2: DATA1
         * 3: MDATA (non-Control), SETUP (Control)
         */
        unsigned pid : 2;

        /** Do PING protocol when 1 */
        unsigned dopng : 1;
#endif
    } b;
} hctsiz_data_t;

/**
 * This union represents the bit fields in the Host Channel Interrupt Mask
 * Register. Read the register into the <i>d32</i> member then set/clear the
 * bits using the <i>b</i>it elements. Write the <i>d32</i> member to the
 * hcintmsk register.
 */
typedef union hcintmsk_data
{
    /** raw register data */
    uint32_t d32;

    /** register bits */
    struct
    {
#ifdef __BIG_ENDIAN_BITFIELD
        unsigned reserved : 21;
        unsigned datatglerr : 1;
        unsigned frmovrun : 1;
        unsigned bblerr : 1;
        unsigned xacterr : 1;
        unsigned nyet : 1;
        unsigned ack : 1;
        unsigned nak : 1;
        unsigned stall : 1;
        unsigned ahberr : 1;
        unsigned chhltd : 1;
        unsigned xfercompl : 1;
#else
        unsigned xfercompl : 1;
        unsigned chhltd : 1;
        unsigned ahberr : 1;
        unsigned stall : 1;
        unsigned nak : 1;
        unsigned ack : 1;
        unsigned nyet : 1;
        unsigned xacterr : 1;
        unsigned bblerr : 1;
        unsigned frmovrun : 1;
        unsigned datatglerr : 1;
        unsigned reserved : 21;
#endif
    } b;
} hcintmsk_data_t;

/** OTG Host Interface Structure.
 *
 * The OTG Host Interface Structure structure contains information
 * needed to manage the DWC_otg controller acting in host mode. It
 * represents the programming view of the host-specific aspects of the
 * controller.
 */
typedef struct dwc_otg_host_if
{
    /** Host Global Registers starting at offset 400h.*/
    dwc_otg_host_global_regs_t *host_global_regs;
#define DWC_OTG_HOST_GLOBAL_REG_OFFSET 0x400

    /** Host Port 0 Control and Status Register */
    volatile uint32_t *hprt0;
#define DWC_OTG_HOST_PORT_REGS_OFFSET 0x440


    /** Host Channel Specific Registers at offsets 500h-5FCh. */
    dwc_otg_hc_regs_t *hc_regs[MAX_EPS_CHANNELS];
#define DWC_OTG_HOST_CHAN_REGS_OFFSET 0x500
#define DWC_OTG_CHAN_REGS_OFFSET 0x20


    /* Host configuration information */
    /** Number of Host Channels (range: 1-16) */
    uint8_t  num_host_channels;
    /** Periodic EPs supported (0: no, 1: yes) */
    uint8_t  perio_eps_supported;
    /** Periodic Tx FIFO Size (Only 1 host periodic Tx FIFO) */
    uint16_t perio_tx_fifo_size;

} dwc_otg_host_if_t;


/**
 * This union represents the bit fields in the Power and Clock Gating Control
 * Register. Read the register into the <i>d32</i> member then set/clear the
 * bits using the <i>b</i>it elements.
 */
typedef union pcgcctl_data
{
    /** raw register data */
    uint32_t d32;

    /** register bits */
    struct
    {
#ifdef __BIG_ENDIAN_BITFIELD
        unsigned reserved : 27;
        unsigned physuspended : 1;
        unsigned rstpdwnmodule : 1;
        unsigned pwrclmp : 1;
        unsigned gatehclk : 1;
        unsigned stoppclk : 1;
#else

        /** Stop Pclk */
        unsigned stoppclk : 1;
        /** Gate Hclk */
        unsigned gatehclk : 1;
        /** Power Clamp */
        unsigned pwrclmp : 1;
        /** Reset Power Down Modules */
        unsigned rstpdwnmodule : 1;
        /** PHY Suspended */
        unsigned physuspended : 1;

        unsigned reserved : 27;
#endif
    } b;
} pcgcctl_data_t;


#endif
