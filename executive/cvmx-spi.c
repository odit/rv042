/****************************************************************
 * Copyright (c) 2006, Cavium Networks. All rights reserved.
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
 * Support library for the SPI
 *
 * File version info: $Id: cvmx-spi.c 2 2007-04-05 08:51:12Z tt $ $Name$
 */

#if defined(__KERNEL__) && defined(__linux__)
    #include <linux/kernel.h>
    #include <linux/string.h>
    #undef cvmx_dprintf
    #define cvmx_dprintf printk
#else
    #include <stdio.h>
    #include <string.h>
#endif
#include "cvmx-config.h"
#include "cvmx.h"
#include "cvmx-mio.h"
#include "cvmx-pko.h"
#include "cvmx-spi.h"

#define MS (500000ull)    /* Not exactly a millisecond, but close enough */

/* SPI Calendar settings */
#define CAL_LEN (10)
#define CAL_REP (1)

#if CVMX_ENABLE_DEBUG_PRINTS
static const char *modes[] = {"UNKNOWN", "TX Halfplex", "Rx Halfplex", "Duplex"};
#endif

/**
 * Initialize and start the SPI interface.
 *
 * @param interface The identifier of the packet interface to configure and
 *                  use as a SPI interface.
 * @param mode      The operating mode for the SPI interface. The interface
 *                  can operate as a full duplex (both Tx and Rx data paths
 *                  active) or as a halfplex (either the Tx data path is
 *                  active or the Rx data path is active, but not both).
 * @param timeout   Timeout to wait for clock synchronization in seconds
 * @return Zero on success, negative of failure.
 */
int cvmx_spi_start_interface(int interface, cvmx_spi_mode_t mode, int timeout)
{
    uint64_t                     timeout_time = cvmx_get_cycle() + 1000ull * MS * timeout;
    cvmx_spxx_trn4_ctl_t         spxx_trn4_ctl;
    cvmx_spxx_clk_stat_t         stat;
    cvmx_stxx_com_ctl_t          stxx_com_ctl;
    cvmx_srxx_com_ctl_t          srxx_com_ctl;
    cvmx_stxx_spi4_dat_t         stxx_spi4_dat;
    uint64_t                     count;
    cvmx_pko_reg_gmx_port_mode_t pko_mode;


    if (!(OCTEON_IS_MODEL(OCTEON_CN38XX) || OCTEON_IS_MODEL(OCTEON_CN58XX)))
        return -1;

    cvmx_dprintf ("SPI%d: mode %s, cal_len: %d, cal_rep: %d\n",
            interface, modes[mode], CAL_LEN, CAL_REP);

      // Configure for 16 ports (PKO -> GMX FIFO partition setting)
      // ----------------------------------------------------------
    pko_mode.u64 = cvmx_read_csr(CVMX_PKO_REG_GMX_PORT_MODE);
    if (interface == 0) 
    {
        pko_mode.s.mode0 = 0;
    }
    else 
    {
        pko_mode.s.mode1 = 0;
    }
    cvmx_write_csr(CVMX_PKO_REG_GMX_PORT_MODE, pko_mode.u64);

      // Configure GMX
      // -------------------------------------------------
    cvmx_write_csr (CVMX_GMXX_TX_PRTS(interface), 0xA);
      // PRTS           [ 4: 0] ( 5b) = 10


      // Bringing up Spi4 Interface
      // -------------------------------------------------

      // Reset the Spi4 deskew logic
      // -------------------------------------------------
    cvmx_write_csr (CVMX_SPXX_DBG_DESKEW_CTL(interface), 0x00200000);
      // DLLDIS         [ 0: 0] ( 1b) = 0
      // DLLFRC         [ 1: 1] ( 1b) = 0
      // OFFDLY         [ 7: 2] ( 6b) = 0
      // BITSEL         [12: 8] ( 5b) = 0
      // OFFSET         [17:13] ( 5b) = 0
      // MUX            [18:18] ( 1b) = 0
      // INC            [19:19] ( 1b) = 0
      // DEC            [20:20] ( 1b) = 0
      // CLRDLY         [21:21] ( 1b) = 1    // Forces a reset
    cvmx_wait (100 * MS);

      // Setup the CLKDLY right in the middle
      // -------------------------------------------------
    cvmx_write_csr (CVMX_SPXX_CLK_CTL(interface), 0x00000830);
      // SRXDLCK        [ 0: 0] ( 1b) = 0
      // RCVTRN         [ 1: 1] ( 1b) = 0
      // DRPTRN         [ 2: 2] ( 1b) = 0
      // SNDTRN         [ 3: 3] ( 1b) = 0
      // STATRCV        [ 4: 4] ( 1b) = 1    // Enable status channel Rx
      // STATDRV        [ 5: 5] ( 1b) = 1    // Enable status channel Tx
      // RUNBIST        [ 6: 6] ( 1b) = 0
      // CLKDLY         [11: 7] ( 5b) = 10   // 16 is the middle of the range
      // SRXLCK         [12:12] ( 1b) = 0
      // STXLCK         [13:13] ( 1b) = 0
      // SEETRN         [14:14] ( 1b) = 0
    cvmx_wait (100 * MS);

      // Reset SRX0 DLL
      // -------------------------------------------------
    cvmx_write_csr (CVMX_SPXX_CLK_CTL(interface), 0x00000831);
      // SRXDLCK        [ 0: 0] ( 1b) = 1    // Restart the DLL
      // RCVTRN         [ 1: 1] ( 1b) = 0
      // DRPTRN         [ 2: 2] ( 1b) = 0
      // SNDTRN         [ 3: 3] ( 1b) = 0
      // STATRCV        [ 4: 4] ( 1b) = 1
      // STATDRV        [ 5: 5] ( 1b) = 1
      // RUNBIST        [ 6: 6] ( 1b) = 0
      // CLKDLY         [11: 7] ( 5b) = 10
      // SRXLCK         [12:12] ( 1b) = 0
      // STXLCK         [13:13] ( 1b) = 0
      // SEETRN         [14:14] ( 1b) = 0

      // Waiting for Inf0 Spi4 RX DLL to lock
      // -------------------------------------------------
    cvmx_wait (100 * MS);

      // Enable dynamic alignment
      // -------------------------------------------------
    spxx_trn4_ctl.u64 = 0;
    spxx_trn4_ctl.s.mux_en   = 1;
    spxx_trn4_ctl.s.macro_en = 1;
    spxx_trn4_ctl.s.maxdist  = 16;
    spxx_trn4_ctl.s.jitter   = 1;
    cvmx_write_csr (CVMX_SPXX_TRN4_CTL(interface), spxx_trn4_ctl.u64);
      // MUX_EN         [ 0: 0] ( 1b) = 1
      // MACRO_EN       [ 1: 1] ( 1b) = 1
      // MAXDIST        [ 6: 2] ( 5b) = 16
      // SET_BOOT       [ 7: 7] ( 1b) = 0
      // CLR_BOOT       [ 8: 8] ( 1b) = 1
      // JITTER         [11: 9] ( 3b) = 1
      // TRNTEST        [12:12] ( 1b) = 0
    cvmx_write_csr (CVMX_SPXX_DBG_DESKEW_CTL(interface), 0x0);
      // DLLDIS         [ 0: 0] ( 1b) = 0
      // DLLFRC         [ 1: 1] ( 1b) = 0
      // OFFDLY         [ 7: 2] ( 6b) = 0
      // BITSEL         [12: 8] ( 5b) = 0
      // OFFSET         [17:13] ( 5b) = 0
      // MUX            [18:18] ( 1b) = 0
      // INC            [19:19] ( 1b) = 0
      // DEC            [20:20] ( 1b) = 0
      // CLRDLY         [21:21] ( 1b) = 0

    if (mode & CVMX_SPI_MODE_RX_HALFPLEX) {
          // SRX0 Ports
          // -------------------------------------------------
        cvmx_write_csr (CVMX_SRXX_COM_CTL(interface), 0x00000090);
          // INF_EN         [ 0: 0] ( 1b) = 0
          // ST_EN          [ 3: 3] ( 1b) = 0
          // PRTS           [ 9: 4] ( 6b) = 9

          // SRX0 Calendar Table
          // -------------------------------------------------
        cvmx_write_csr (CVMX_SRXX_SPI4_CALX(0, interface), 0x00013210);
          // PRT0           [ 4: 0] ( 5b) = 0
          // PRT1           [ 9: 5] ( 5b) = 1
          // PRT2           [14:10] ( 5b) = 2
          // PRT3           [19:15] ( 5b) = 3
          // ODDPAR         [20:20] ( 1b) = 1
        cvmx_write_csr (CVMX_SRXX_SPI4_CALX(1, interface), 0x00017654);
          // PRT0           [ 4: 0] ( 5b) = 4
          // PRT1           [ 9: 5] ( 5b) = 5
          // PRT2           [14:10] ( 5b) = 6
          // PRT3           [19:15] ( 5b) = 7
          // ODDPAR         [20:20] ( 1b) = 1
        cvmx_write_csr (CVMX_SRXX_SPI4_CALX(2, interface), 0x00000098);
          // PRT0           [ 4: 0] ( 5b) = 8
          // PRT1           [ 9: 5] ( 5b) = 9
          // PRT2           [14:10] ( 5b) = 0
          // PRT3           [19:15] ( 5b) = 0
          // ODDPAR         [20:20] ( 1b) = 0
        cvmx_write_csr (CVMX_SRXX_SPI4_STAT(interface), (CAL_REP << 8) | CAL_LEN);
          // LEN            [ 7: 0] ( 8b) = a
          // M              [15: 8] ( 8b) = 1
    }

    if (mode & CVMX_SPI_MODE_TX_HALFPLEX) {
          // STX0 Config
          // -------------------------------------------------
        cvmx_write_csr (CVMX_STXX_ARB_CTL(interface), 0x0);
          // IGNTPA         [ 3: 3] ( 1b) = 0
          // MINTRN         [ 5: 5] ( 1b) = 0
        cvmx_write_csr (CVMX_GMXX_TX_SPI_MAX(interface), 0x0408);
          // MAX2           [15: 8] ( 8b) = 4
          // MAX1           [ 7: 0] ( 8b) = 8
        cvmx_write_csr (CVMX_GMXX_TX_SPI_THRESH(interface), 0x4);
          // THRESH         [ 5: 0] ( 6b) = 4
        cvmx_write_csr (CVMX_GMXX_TX_SPI_CTL(interface), 0x0);
          // ENFORCE        [ 2: 2] ( 1b) = 0
          // TPA_CLR        [ 1: 1] ( 1b) = 0
          // CONT_PKT       [ 0: 0] ( 1b) = 0

          // STX0 Training Control
          // -------------------------------------------------
        stxx_spi4_dat.u64 = 0;
	stxx_spi4_dat.s.alpha = 32;    /*Minimum needed by dynamic alignment*/
	stxx_spi4_dat.s.max_t = 0xFFFF;  /*Minimum interval is 0x20*/
        cvmx_write_csr (CVMX_STXX_SPI4_DAT(interface), stxx_spi4_dat.u64);
          // MAX_T          [15: 0] (16b) = 0
          // ALPHA          [31:16] (16b) = 0

          // STX0 Calendar Table
          // -------------------------------------------------
        cvmx_write_csr (CVMX_STXX_SPI4_CALX(0, interface), 0x00013210);
          // PRT0           [ 4: 0] ( 5b) = 0
          // PRT1           [ 9: 5] ( 5b) = 1
          // PRT2           [14:10] ( 5b) = 2
          // PRT3           [19:15] ( 5b) = 3
          // ODDPAR         [20:20] ( 1b) = 1
        cvmx_write_csr (CVMX_STXX_SPI4_CALX(1, interface), 0x00017654);
          // PRT0           [ 4: 0] ( 5b) = 4
          // PRT1           [ 9: 5] ( 5b) = 5
          // PRT2           [14:10] ( 5b) = 6
          // PRT3           [19:15] ( 5b) = 7
          // ODDPAR         [20:20] ( 1b) = 1
        cvmx_write_csr (CVMX_STXX_SPI4_CALX(2, interface), 0x00000098);
          // PRT0           [ 4: 0] ( 5b) = 8
          // PRT1           [ 9: 5] ( 5b) = 9
          // PRT2           [14:10] ( 5b) = 0
          // PRT3           [19:15] ( 5b) = 0
          // ODDPAR         [20:20] ( 1b) = 0
        cvmx_write_csr (CVMX_STXX_SPI4_STAT(interface), (CAL_REP << 8) | CAL_LEN);
          // LEN            [ 7: 0] ( 8b) = a
          // M              [15: 8] ( 8b) = 1
    }

      /* Regardless of operating mode, both Tx and Rx clocks must be present
       * for the SPI interface to operate.
       */
    cvmx_dprintf ("SPI%d: Waiting to see TsClk...\n", interface);
    count = 0;
    timeout_time = cvmx_get_cycle() + 1000ull * MS * timeout;
    do {  /* Do we see the TsClk transitioning? */
        stat.u64 = cvmx_read_csr (CVMX_SPXX_CLK_STAT(interface));
        count = count + 1;
#ifdef DEBUG
        if ((count % 5000000) == 10) {
	    cvmx_dprintf ("SPI%d: CLK_STAT 0x%016llX\n"
                    "  s4 (%d,%d) d4 (%d,%d)\n",
                    interface, (unsigned long long)stat.u64,
                    stat.s.s4clk0, stat.s.s4clk1,
                    stat.s.d4clk0, stat.s.d4clk1);
        }
#endif
        if (cvmx_get_cycle() > timeout_time)
        {
            cvmx_dprintf ("SPI%d: Timeout\n", interface);
            return -1;
        }
    } while (stat.s.s4clk0 == 0 || stat.s.s4clk1 == 0);

    cvmx_dprintf ("SPI%d: Waiting to see RsClk...\n", interface);
    timeout_time = cvmx_get_cycle() + 1000ull * MS * timeout;
    do {  /* Do we see the RsClk transitioning? */
        stat.u64 = cvmx_read_csr (CVMX_SPXX_CLK_STAT(interface));
        if (cvmx_get_cycle() > timeout_time)
        {
            cvmx_dprintf ("SPI%d: Timeout\n", interface);
            return -1;
        }
    } while (stat.s.d4clk0 == 0 || stat.s.d4clk1 == 0);


      // SRX0 & STX0 Inf0 Links are configured - begin training
      // -------------------------------------------------
    cvmx_write_csr (CVMX_SPXX_CLK_CTL(interface), 0x0000083f);
      // SRXDLCK        [ 0: 0] ( 1b) = 1
      // RCVTRN         [ 1: 1] ( 1b) = 1
      // DRPTRN         [ 2: 2] ( 1b) = 1    ...was 0
      // SNDTRN         [ 3: 3] ( 1b) = 1
      // STATRCV        [ 4: 4] ( 1b) = 1
      // STATDRV        [ 5: 5] ( 1b) = 1
      // RUNBIST        [ 6: 6] ( 1b) = 0
      // CLKDLY         [11: 7] ( 5b) = 10
      // SRXLCK         [12:12] ( 1b) = 0
      // STXLCK         [13:13] ( 1b) = 0
      // SEETRN         [14:14] ( 1b) = 0
    cvmx_wait (1000 * MS);

      // SRX0 clear the boot bit
      // -------------------------------------------------
    spxx_trn4_ctl.s.clr_boot = 1;
    cvmx_write_csr (CVMX_SPXX_TRN4_CTL(interface), spxx_trn4_ctl.u64);

      // Wait for the training sequence to complete
      // -------------------------------------------------
      // SPX0_CLK_STAT - SPX0_CLK_STAT[SRXTRN] should be 1 (bit8)
    cvmx_dprintf ("SPI%d: Waiting for training\n", interface);
    cvmx_wait (1000 * MS);
    timeout_time = cvmx_get_cycle() + 1000ull * MS * 600;  /* Wait a really long time here */
    do {
        stat.u64 = cvmx_read_csr (CVMX_SPXX_CLK_STAT(interface));
        if (cvmx_get_cycle() > timeout_time)
        {
            cvmx_dprintf ("SPI%d: Timeout\n", interface);
            return -1;
        }
    } while (stat.s.srxtrn == 0);

    if (mode & CVMX_SPI_MODE_RX_HALFPLEX) {
          // SRX0 interface should be good, send calendar data
          // -------------------------------------------------
        cvmx_dprintf ("SPI%d: Rx is synchronized, start sending calendar data\n", interface);
        srxx_com_ctl.u64 = 0;
        srxx_com_ctl.s.prts   = 9;
        srxx_com_ctl.s.inf_en = 1;
        srxx_com_ctl.s.st_en  = 1;
        cvmx_write_csr (CVMX_SRXX_COM_CTL(interface), srxx_com_ctl.u64);
    }

    if (mode & CVMX_SPI_MODE_TX_HALFPLEX) {
          // STX0 has achieved sync
          // The corespondant board should be sending calendar data
          // Enable the STX0 STAT receiver.
          // -------------------------------------------------
        stxx_com_ctl.u64 = 0;
        stxx_com_ctl.s.inf_en = 1;
        stxx_com_ctl.s.st_en = 1;
        cvmx_write_csr (CVMX_STXX_COM_CTL(interface), stxx_com_ctl.u64);

          // Waiting for calendar sync on STX0 STAT
          // -------------------------------------------------
          // SPX0_CLK_STAT - SPX0_CLK_STAT[STXCAL] should be 1 (bit10)
        cvmx_dprintf ("SPI%d: Waiting to sync on STX[%d] STAT\n", interface, interface);
        timeout_time = cvmx_get_cycle() + 1000ull * MS * timeout;
        do {
            stat.u64 = cvmx_read_csr (CVMX_SPXX_CLK_STAT (interface));
            if (cvmx_get_cycle() > timeout_time)
            {
                cvmx_dprintf ("SPI%d: Timeout\n", interface);
                return -1;
            }
        } while (stat.s.stxcal == 0);
    }

      // Inf0 is synched
      // -------------------------------------------------
      // SPX0 is up
    if (mode & CVMX_SPI_MODE_RX_HALFPLEX) {
        srxx_com_ctl.s.inf_en = 1;
        cvmx_write_csr (CVMX_SRXX_COM_CTL(interface), srxx_com_ctl.u64);
        cvmx_dprintf ("SPI%d: Rx is now up\n", interface);
    }

    if (mode & CVMX_SPI_MODE_TX_HALFPLEX) {
        stxx_com_ctl.s.inf_en = 1;
        cvmx_write_csr (CVMX_STXX_COM_CTL(interface), stxx_com_ctl.u64);
        cvmx_dprintf ("SPI%d: Tx is now up\n", interface);
    }

    cvmx_write_csr (CVMX_GMXX_RXX_FRM_MIN (0,interface), 40);
    cvmx_write_csr (CVMX_GMXX_RXX_FRM_MAX (0,interface), 64*1024 - 4);
    cvmx_write_csr (CVMX_GMXX_RXX_JABBER  (0,interface), 64*1024 - 4);

    return 0;
}


/**
 * This routine restarts the SPI interface after it has lost synchronization
 * with its corespondant system.
 *
 * @param interface The identifier of the packet interface to configure and
 *                  use as a SPI interface.
 * @param mode      The operating mode for the SPI interface. The interface
 *                  can operate as a full duplex (both Tx and Rx data paths
 *                  active) or as a halfplex (either the Tx data path is
 *                  active or the Rx data path is active, but not both).
 * @param timeout   Timeout to wait for clock synchronization in seconds
 * @return Zero on success, negative of failure.
 */
int cvmx_spi_restart_interface(int interface, cvmx_spi_mode_t mode, int timeout)
{
    uint64_t                     timeout_time = cvmx_get_cycle() + 1000ull * MS * timeout;
    cvmx_spxx_trn4_ctl_t         spxx_trn4_ctl;
    cvmx_spxx_clk_stat_t         stat;
    cvmx_stxx_com_ctl_t          stxx_com_ctl;
    cvmx_srxx_com_ctl_t          srxx_com_ctl;
    //cvmx_stxx_spi4_dat_t         stxx_spi4_dat;

    cvmx_dprintf ("SPI%d: Restart %s\n", interface, modes[mode]);

      // Reset the Spi4 deskew logic
      // -------------------------------------------------
    cvmx_write_csr (CVMX_SPXX_DBG_DESKEW_CTL(interface), 0x00200000);
    cvmx_wait (100 * MS);
      // Setup the CLKDLY right in the middle
      // -------------------------------------------------
    cvmx_write_csr (CVMX_SPXX_CLK_CTL(interface), 0x00000830);
    cvmx_wait (100 * MS);
      // Reset SRX0 DLL
      // -------------------------------------------------
    cvmx_write_csr (CVMX_SPXX_CLK_CTL(interface), 0x00000831);
    cvmx_wait (100 * MS);
      // Enable dynamic alignment
      // -------------------------------------------------
    spxx_trn4_ctl.u64 = 0;
    spxx_trn4_ctl.s.mux_en   = 1;
    spxx_trn4_ctl.s.macro_en = 1;
    spxx_trn4_ctl.s.maxdist  = 16;
    spxx_trn4_ctl.s.jitter   = 1;
    cvmx_write_csr (CVMX_SPXX_TRN4_CTL(interface), spxx_trn4_ctl.u64);
    cvmx_write_csr (CVMX_SPXX_DBG_DESKEW_CTL(interface), 0x0);

      /* Regardless of operating mode, both Tx and Rx clocks must be present
       * for the SPI interface to operate.
       */
    cvmx_dprintf ("SPI%d: Waiting to see TsClk...\n", interface);
    do {  /* Do we see the TsClk transitioning? */
        stat.u64 = cvmx_read_csr (CVMX_SPXX_CLK_STAT(interface));
        if (cvmx_get_cycle() > timeout_time)
        {
            cvmx_dprintf ("SPI%d: Timeout\n", interface);
            return -1;
        }
    } while (stat.s.s4clk0 == 0 || stat.s.s4clk1 == 0);

    cvmx_dprintf ("SPI%d: Waiting to see RsClk...\n", interface);
    do {  /* Do we see the RsClk transitioning? */
        stat.u64 = cvmx_read_csr (CVMX_SPXX_CLK_STAT(interface));
        if (cvmx_get_cycle() > timeout_time)
        {
            cvmx_dprintf ("SPI%d: Timeout\n", interface);
            return -1;
        }
    } while (stat.s.d4clk0 == 0 || stat.s.d4clk1 == 0);

      // SRX0 & STX0 Inf0 Links are configured - begin training
      // -------------------------------------------------
    cvmx_write_csr (CVMX_SPXX_CLK_CTL(interface), 0x0000083f);
    cvmx_wait (1000 * MS);
      // SRX0 clear the boot bit
      // -------------------------------------------------
    spxx_trn4_ctl.s.clr_boot = 1;
    cvmx_write_csr (CVMX_SPXX_TRN4_CTL(interface), spxx_trn4_ctl.u64);

      // Wait for the training sequence to complete
      // -------------------------------------------------
      // SPX0_CLK_STAT - SPX0_CLK_STAT[SRXTRN] should be 1 (bit8)
    cvmx_dprintf ("SPI%d: Waiting for training\n", interface);
    cvmx_wait (1000 * MS);
    do {
        stat.u64 = cvmx_read_csr (CVMX_SPXX_CLK_STAT(interface));
        if (cvmx_get_cycle() > timeout_time)
        {
            cvmx_dprintf ("SPI%d: Timeout\n", interface);
            return -1;
        }
    } while (stat.s.srxtrn == 0);

    if (mode & CVMX_SPI_MODE_RX_HALFPLEX) {
          // SRX0 interface should be good, send calendar data
          // -------------------------------------------------
        cvmx_dprintf ("SPI%d: Rx is synchronized, start sending calendar data\n", interface);
        srxx_com_ctl.u64 = 0;
        srxx_com_ctl.s.prts   = 9;
        srxx_com_ctl.s.inf_en = 1;
        srxx_com_ctl.s.st_en  = 1;
        cvmx_write_csr (CVMX_SRXX_COM_CTL(interface), srxx_com_ctl.u64);
    }

    if (mode & CVMX_SPI_MODE_TX_HALFPLEX) {
          // STX0 has achieved sync
          // The corespondant board should be sending calendar data
          // Enable the STX0 STAT receiver.
          // -------------------------------------------------
        stxx_com_ctl.u64 = 0;
        stxx_com_ctl.s.inf_en = 1;
        stxx_com_ctl.s.st_en = 1;
        cvmx_write_csr (CVMX_STXX_COM_CTL(interface), stxx_com_ctl.u64);

          // Waiting for calendar sync on STX0 STAT
          // -------------------------------------------------
          // SPX0_CLK_STAT - SPX0_CLK_STAT[STXCAL] should be 1 (bit10)
        cvmx_dprintf ("SPI%d: Waiting to sync on STX[%d] STAT\n", interface, interface);
        do {
            stat.u64 = cvmx_read_csr (CVMX_SPXX_CLK_STAT (interface));
            if (cvmx_get_cycle() > timeout_time)
            {
                cvmx_dprintf ("SPI%d: Timeout\n", interface);
                return -1;
            }
        } while (stat.s.stxcal == 0);
    }

      // Inf0 is synched
      // -------------------------------------------------
      // SPX0 is up
    if (mode & CVMX_SPI_MODE_RX_HALFPLEX) {
        srxx_com_ctl.s.inf_en = 1;
        cvmx_write_csr (CVMX_SRXX_COM_CTL(interface), srxx_com_ctl.u64);
        cvmx_dprintf ("SPI%d: Rx is now up\n", interface);
    }

    if (mode & CVMX_SPI_MODE_TX_HALFPLEX) {
        stxx_com_ctl.s.inf_en = 1;
        cvmx_write_csr (CVMX_STXX_COM_CTL(interface), stxx_com_ctl.u64);
        cvmx_dprintf ("SPI%d: Tx is now up\n", interface);
    }

    return 0;
}

