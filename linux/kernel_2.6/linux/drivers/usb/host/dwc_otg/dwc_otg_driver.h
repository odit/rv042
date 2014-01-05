/* ==========================================================================
 * $File: //dwh/usb_iip/dev/software/otg_ipmate/linux/drivers/dwc_otg_driver.h $
 * $Revision: 3106 $
 * $Date: 2009-12-18 15:40:38 +0800 (Fri, 18 Dec 2009) $
 * $Change: 510275 $
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

#if !defined(__DWC_OTG_DRIVER_H__)
#define __DWC_OTG_DRIVER_H__

/** @file
 * This file contains the interface to the Linux driver.
 */
#include "dwc_otg_cil.h"

/* Type declarations */
struct dwc_otg_pcd;
struct dwc_otg_hcd;

/**
 * This structure is a wrapper that encapsulates the driver components used to
 * manage a single DWC_otg controller.
 */
typedef struct dwc_otg_device
{
        /** Base address returned from ioremap() */
        void *base;

        /** Pointer to the core interface structure. */
        dwc_otg_core_if_t *core_if;

        /** Register offset for Diagnostic API.*/
        uint32_t reg_offset;
        
        /** Pointer to the PCD structure. */
        struct dwc_otg_pcd *pcd;

        /** Pointer to the HCD structure. */
        struct dwc_otg_hcd *hcd;

	/** Flag to indicate whether the common IRQ handler is installed. */
	uint8_t common_irq_installed;

} dwc_otg_device_t;

#ifdef CONFIG_NK_SUPPORT_5010_USB
typedef union
{
    uint64_t u64;
    struct cvmx_usbnx_clk_ctl_s
    {
#ifdef __BIG_ENDIAN_BITFIELD
        uint64_t reserved_20_63          : 44;
        uint64_t divide2                 : 2;       /**< The 'hclk' used by the USB subsystem is derived
                                                         from the eclk.
                                                         Also see the field DIVIDE. DIVIDE2<1> must currently
                                                         be zero because it is not implemented, so the maximum
                                                         ratio of eclk/hclk is currently 16.
                                                         The actual divide number for hclk is:
                                                         (DIVIDE2 + 1) * (DIVIDE + 1) */
        uint64_t hclk_rst                : 1;       /**< When this field is '0' the HCLK-DIVIDER used to
                                                         generate the hclk in the USB Subsystem is held
                                                         in reset. This bit must be set to '0' before
                                                         changing the value os DIVIDE in this register.
                                                         The reset to the HCLK_DIVIDERis also asserted
                                                         when core reset is asserted. */
        uint64_t p_x_on                  : 1;       /**< Force USB-PHY on during suspend.
                                                         '1' USB-PHY XO block is powered-down during
                                                             suspend.
                                                         '0' USB-PHY XO block is powered-up during
                                                             suspend.
                                                         The value of this field must be set while POR is
                                                         active. */
        uint64_t reserved_14_15          : 2;
        uint64_t p_com_on                : 1;       /**< '0' Force USB-PHY XO Bias, Bandgap and PLL to
                                                             remain powered in Suspend Mode.
                                                         '1' The USB-PHY XO Bias, Bandgap and PLL are
                                                             powered down in suspend mode.
                                                         The value of this field must be set while POR is
                                                         active. */
        uint64_t p_c_sel                 : 2;       /**< Phy clock speed select.
                                                         Selects the reference clock / crystal frequency.
                                                         '11': Reserved
                                                         '10': 48 MHz (reserved when a crystal is used)
                                                         '01': 24 MHz (reserved when a crystal is used)
                                                         '00': 12 MHz
                                                         The value of this field must be set while POR is
                                                         active.
                                                         NOTE: if a crystal is used as a reference clock,
                                                         this field must be set to 12 MHz. */
        uint64_t cdiv_byp                : 1;       /**< Used to enable the bypass input to the USB_CLK_DIV. */
        uint64_t sd_mode                 : 2;       /**< Scaledown mode for the USBC. Control timing events
                                                         in the USBC, for normal operation this must be '0'. */
        uint64_t s_bist                  : 1;       /**< Starts bist on the hclk memories, during the '0'
                                                         to '1' transition. */
        uint64_t por                     : 1;       /**< Power On Reset for the PHY.
                                                         Resets all the PHYS registers and state machines. */
        uint64_t enable                  : 1;       /**< When '1' allows the generation of the hclk. When
                                                         '0' the hclk will not be generated. SEE DIVIDE
                                                         field of this register. */
        uint64_t prst                    : 1;       /**< When this field is '0' the reset associated with
                                                         the phy_clk functionality in the USB Subsystem is
                                                         help in reset. This bit should not be set to '1'
                                                         until the time it takes 6 clocks (hclk or phy_clk,
                                                         whichever is slower) has passed. Under normal
                                                         operation once this bit is set to '1' it should not
                                                         be set to '0'. */
        uint64_t hrst                    : 1;       /**< When this field is '0' the reset associated with
                                                         the hclk functioanlity in the USB Subsystem is
                                                         held in reset.This bit should not be set to '1'
                                                         until 12ms after phy_clk is stable. Under normal
                                                         operation, once this bit is set to '1' it should
                                                         not be set to '0'. */
        uint64_t divide                  : 3;       /**< The frequency of 'hclk' used by the USB subsystem
                                                         is the eclk frequency divided by the value of
                                                         (DIVIDE2 + 1) * (DIVIDE + 1), also see the field
                                                         DIVIDE2 of this register.
                                                         The hclk frequency should be less than 125Mhz.
                                                         After writing a value to this field the SW should
                                                         read the field for the value written.
                                                         The ENABLE field of this register should not be set
                                                         until AFTER this field is set and then read. */
#else
        uint64_t divide                  : 3;
        uint64_t hrst                    : 1;
        uint64_t prst                    : 1;
        uint64_t enable                  : 1;
        uint64_t por                     : 1;
        uint64_t s_bist                  : 1;
        uint64_t sd_mode                 : 2;
        uint64_t cdiv_byp                : 1;
        uint64_t p_c_sel                 : 2;
        uint64_t p_com_on                : 1;
        uint64_t reserved_14_15          : 2;
        uint64_t p_x_on                  : 1;
        uint64_t hclk_rst                : 1;
        uint64_t divide2                 : 2;
        uint64_t reserved_20_63          : 44;
#endif
    } s;
    struct cvmx_usbnx_clk_ctl_cn30xx
    {
#ifdef __BIG_ENDIAN_BITFIELD
        uint64_t reserved_18_63          : 46;
        uint64_t hclk_rst                : 1;       /**< When this field is '0' the HCLK-DIVIDER used to
                                                         generate the hclk in the USB Subsystem is held
                                                         in reset. This bit must be set to '0' before
                                                         changing the value os DIVIDE in this register.
                                                         The reset to the HCLK_DIVIDERis also asserted
                                                         when core reset is asserted. */
        uint64_t p_x_on                  : 1;       /**< Force USB-PHY on during suspend.
                                                         '1' USB-PHY XO block is powered-down during
                                                             suspend.
                                                         '0' USB-PHY XO block is powered-up during
                                                             suspend.
                                                         The value of this field must be set while POR is
                                                         active. */
        uint64_t p_rclk                  : 1;       /**< Phy refrence clock enable.
                                                         '1' The PHY PLL uses the XO block output as a
                                                         reference.
                                                         '0' Reserved. */
        uint64_t p_xenbn                 : 1;       /**< Phy external clock enable.
                                                         '1' The XO block uses the clock from a crystal.
                                                         '0' The XO block uses an external clock supplied
                                                             on the XO pin. USB_XI should be tied to
                                                             ground for this usage. */
        uint64_t p_com_on                : 1;       /**< '0' Force USB-PHY XO Bias, Bandgap and PLL to
                                                             remain powered in Suspend Mode.
                                                         '1' The USB-PHY XO Bias, Bandgap and PLL are
                                                             powered down in suspend mode.
                                                         The value of this field must be set while POR is
                                                         active. */
        uint64_t p_c_sel                 : 2;       /**< Phy clock speed select.
                                                         Selects the reference clock / crystal frequency.
                                                         '11': Reserved
                                                         '10': 48 MHz
                                                         '01': 24 MHz
                                                         '00': 12 MHz
                                                         The value of this field must be set while POR is
                                                         active. */
        uint64_t cdiv_byp                : 1;       /**< Used to enable the bypass input to the USB_CLK_DIV. */
        uint64_t sd_mode                 : 2;       /**< Scaledown mode for the USBC. Control timing events
                                                         in the USBC, for normal operation this must be '0'. */
        uint64_t s_bist                  : 1;       /**< Starts bist on the hclk memories, during the '0'
                                                         to '1' transition. */
        uint64_t por                     : 1;       /**< Power On Reset for the PHY.
                                                         Resets all the PHYS registers and state machines. */
        uint64_t enable                  : 1;       /**< When '1' allows the generation of the hclk. When
                                                         '0' the hclk will not be generated. */
        uint64_t prst                    : 1;       /**< When this field is '0' the reset associated with
                                                         the phy_clk functionality in the USB Subsystem is
                                                         help in reset. This bit should not be set to '1'
                                                         until the time it takes 6 clocks (hclk or phy_clk,
                                                         whichever is slower) has passed. Under normal
                                                         operation once this bit is set to '1' it should not
                                                         be set to '0'. */
        uint64_t hrst                    : 1;       /**< When this field is '0' the reset associated with
                                                         the hclk functioanlity in the USB Subsystem is
                                                         held in reset.This bit should not be set to '1'
                                                         until 12ms after phy_clk is stable. Under normal
                                                         operation, once this bit is set to '1' it should
                                                         not be set to '0'. */
        uint64_t divide                  : 3;       /**< The 'hclk' used by the USB subsystem is derived
                                                         from the eclk. The eclk will be divided by the
                                                         value of this field +1 to determine the hclk
                                                         frequency. (Also see HRST of this register).
                                                         The hclk frequency must be less than 125 MHz. */
#else
        uint64_t divide                  : 3;
        uint64_t hrst                    : 1;
        uint64_t prst                    : 1;
        uint64_t enable                  : 1;
        uint64_t por                     : 1;
        uint64_t s_bist                  : 1;
        uint64_t sd_mode                 : 2;
        uint64_t cdiv_byp                : 1;
        uint64_t p_c_sel                 : 2;
        uint64_t p_com_on                : 1;
        uint64_t p_xenbn                 : 1;
        uint64_t p_rclk                  : 1;
        uint64_t p_x_on                  : 1;
        uint64_t hclk_rst                : 1;
        uint64_t reserved_18_63          : 46;
#endif
    } cn30xx;
    struct cvmx_usbnx_clk_ctl_cn30xx     cn31xx;
    struct cvmx_usbnx_clk_ctl_cn50xx
    {
#ifdef __BIG_ENDIAN_BITFIELD
        uint64_t reserved_20_63          : 44;
        uint64_t divide2                 : 2;       /**< The 'hclk' used by the USB subsystem is derived
                                                         from the eclk.
                                                         Also see the field DIVIDE. DIVIDE2<1> must currently
                                                         be zero because it is not implemented, so the maximum
                                                         ratio of eclk/hclk is currently 16.
                                                         The actual divide number for hclk is:
                                                         (DIVIDE2 + 1) * (DIVIDE + 1) */
        uint64_t hclk_rst                : 1;       /**< When this field is '0' the HCLK-DIVIDER used to
                                                         generate the hclk in the USB Subsystem is held
                                                         in reset. This bit must be set to '0' before
                                                         changing the value os DIVIDE in this register.
                                                         The reset to the HCLK_DIVIDERis also asserted
                                                         when core reset is asserted. */
        uint64_t reserved_16_16          : 1;
        uint64_t p_rtype                 : 2;       /**< PHY reference clock type
                                                         '0' The USB-PHY uses a 12MHz crystal as a clock
                                                             source at the USB_XO and USB_XI pins
                                                         '1' Reserved
                                                         '2' The USB_PHY uses 12/24/48MHz 2.5V board clock
                                                             at the USB_XO pin. USB_XI should be tied to
                                                             ground in this case.
                                                         '3' Reserved
                                                         (bit 14 was P_XENBN on 3xxx)
                                                         (bit 15 was P_RCLK on 3xxx) */
        uint64_t p_com_on                : 1;       /**< '0' Force USB-PHY XO Bias, Bandgap and PLL to
                                                             remain powered in Suspend Mode.
                                                         '1' The USB-PHY XO Bias, Bandgap and PLL are
                                                             powered down in suspend mode.
                                                         The value of this field must be set while POR is
                                                         active. */
        uint64_t p_c_sel                 : 2;       /**< Phy clock speed select.
                                                         Selects the reference clock / crystal frequency.
                                                         '11': Reserved
                                                         '10': 48 MHz (reserved when a crystal is used)
                                                         '01': 24 MHz (reserved when a crystal is used)
                                                         '00': 12 MHz
                                                         The value of this field must be set while POR is
                                                         active.
                                                         NOTE: if a crystal is used as a reference clock,
                                                         this field must be set to 12 MHz. */
        uint64_t cdiv_byp                : 1;       /**< Used to enable the bypass input to the USB_CLK_DIV. */
        uint64_t sd_mode                 : 2;       /**< Scaledown mode for the USBC. Control timing events
                                                         in the USBC, for normal operation this must be '0'. */
        uint64_t s_bist                  : 1;       /**< Starts bist on the hclk memories, during the '0'
                                                         to '1' transition. */
        uint64_t por                     : 1;       /**< Power On Reset for the PHY.
                                                         Resets all the PHYS registers and state machines. */
        uint64_t enable                  : 1;       /**< When '1' allows the generation of the hclk. When
                                                         '0' the hclk will not be generated. SEE DIVIDE
                                                         field of this register. */
        uint64_t prst                    : 1;       /**< When this field is '0' the reset associated with
                                                         the phy_clk functionality in the USB Subsystem is
                                                         help in reset. This bit should not be set to '1'
                                                         until the time it takes 6 clocks (hclk or phy_clk,
                                                         whichever is slower) has passed. Under normal
                                                         operation once this bit is set to '1' it should not
                                                         be set to '0'. */
        uint64_t hrst                    : 1;       /**< When this field is '0' the reset associated with
                                                         the hclk functioanlity in the USB Subsystem is
                                                         held in reset.This bit should not be set to '1'
                                                         until 12ms after phy_clk is stable. Under normal
                                                         operation, once this bit is set to '1' it should
                                                         not be set to '0'. */
        uint64_t divide                  : 3;       /**< The frequency of 'hclk' used by the USB subsystem
                                                         is the eclk frequency divided by the value of
                                                         (DIVIDE2 + 1) * (DIVIDE + 1), also see the field
                                                         DIVIDE2 of this register.
                                                         The hclk frequency should be less than 125Mhz.
                                                         After writing a value to this field the SW should
                                                         read the field for the value written.
                                                         The ENABLE field of this register should not be set
                                                         until AFTER this field is set and then read. */
#else
        uint64_t divide                  : 3;
        uint64_t hrst                    : 1;
        uint64_t prst                    : 1;
        uint64_t enable                  : 1;
        uint64_t por                     : 1;
        uint64_t s_bist                  : 1;
        uint64_t sd_mode                 : 2;
        uint64_t cdiv_byp                : 1;
        uint64_t p_c_sel                 : 2;
        uint64_t p_com_on                : 1;
        uint64_t p_rtype                 : 2;
        uint64_t reserved_16_16          : 1;
        uint64_t hclk_rst                : 1;
        uint64_t divide2                 : 2;
        uint64_t reserved_20_63          : 44;
#endif
    } cn50xx;
    struct cvmx_usbnx_clk_ctl_cn50xx     cn52xx;
    struct cvmx_usbnx_clk_ctl_cn50xx     cn52xxp1;
    struct cvmx_usbnx_clk_ctl_cn50xx     cn56xx;
    struct cvmx_usbnx_clk_ctl_cn50xx     cn56xxp1;
} cvmx_usbnx_clk_ctl_t;
#endif

#endif
