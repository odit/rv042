/****************************************************************
 * Copyright (c) 2003-2004, Cavium Networks. All rights reserved.
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
 * Helper functions for common, but complicated tasks.
 *
 * $Id: cvmx-helper.c 2503 2009-09-18 07:07:04Z richie $ $Name$
 */
#if defined(__KERNEL__) && defined(__linux__)
    #include <linux/kernel.h>
    #include <linux/string.h>
    #define printf printk
#else
    #include <stdio.h>
    #include <string.h>
#endif
#include "executive-config.h"
#include "cvmx-config.h"
#include "cvmx.h"
#include "cvmx-bootmem.h"
#include "cvmx-fpa.h"
#include "cvmx-pip.h"
#include "cvmx-pko.h"
#include "cvmx-ipd.h"
#include "cvmx-asx.h"
#include "cvmx-gmx.h"
#include "cvmx-spi.h"
#include "cvmx-sysinfo.h"
#include "cvmx-helper.h"

#if defined(CVMX_ENABLE_HELPER_FUNCTIONS)
/* CVMX_HELPER_FIRST_MBUFF_SKIP is the number of bytes to reserve before
    the beginning of the packet. Override in executive-config.h */
#ifndef CVMX_HELPER_FIRST_MBUFF_SKIP
#define CVMX_HELPER_FIRST_MBUFF_SKIP 184
#warning WARNING: default CVMX_HELPER_FIRST_MBUFF_SKIP used.  Defaults deprecated, please set in executive-config.h
#endif

/* CVMX_HELPER_NOT_FIRST_MBUFF_SKIP is the number of bytes to reserve in each
    chained packet element. Override in executive-config.h */
#ifndef CVMX_HELPER_NOT_FIRST_MBUFF_SKIP
#define CVMX_HELPER_NOT_FIRST_MBUFF_SKIP 0
#warning WARNING: default CVMX_HELPER_NOT_FIRST_MBUFF_SKIP used.  Defaults deprecated, please set in executive-config.h
#endif

/* CVMX_HELPER_ENABLE_BACK_PRESSURE controls whether back pressure is enabled
    for all input ports. Override in executive-config.h */
#ifndef CVMX_HELPER_ENABLE_BACK_PRESSURE
#define CVMX_HELPER_ENABLE_BACK_PRESSURE 1
#warning WARNING: default CVMX_HELPER_ENABLE_BACK_PRESSURE used.  Defaults deprecated, please set in executive-config.h
#endif

/* CVMX_HELPER_ENABLE_IPD controls if the IPD is enabled in the helper
    function. Once it is enabled the hardware starts accepting packets. You
    might want to skip the IPD enable if configuration changes are need
    from the default helper setup. Override in executive-config.h */
#ifndef CVMX_HELPER_ENABLE_IPD
#define CVMX_HELPER_ENABLE_IPD 1
#warning WARNING: default CVMX_HELPER_ENABLE_IPD used.  Defaults deprecated, please set in executive-config.h
#endif


/* Set default (defaults are deprecated) input tag type */
#ifndef  CVMX_HELPER_INPUT_TAG_TYPE
#define  CVMX_HELPER_INPUT_TAG_TYPE CVMX_POW_TAG_TYPE_ORDERED
#warning WARNING: default CVMX_HELPER_INPUT_TAG_TYPE used.  Defaults deprecated, please set in executive-config.h
#endif

#ifndef CVMX_HELPER_INPUT_PORT_SKIP_MODE
#define CVMX_HELPER_INPUT_PORT_SKIP_MODE	CVMX_PIP_PORT_CFG_MODE_SKIPL2
#warning WARNING: default CVMX_HELPER_INPUT_PORT_SKIP_MODE used.  Defaults deprecated, please set in executive-config.h
#endif

#ifndef CVMX_HELPER_INPUT_TAG_INPUT_PORT
#error VMX_HELPER_INPUT_TAG_* values for determing tag hash inputs must be defined in executive-config.h
#endif

#endif  /*  defined(CVMX_ENABLE_HELPER_FUNCTIONS) */
/**
 * Allocate memory for and initialize a single FPA pool.
 *
 * @param pool    Pool to initialize
 * @param buffer_size  Size of buffers to allocate in bytes
 * @param buffers Number of buffers to put in the pool. Zero is allowed
 * @param name    String name of the pool for debugging purposes
 * @return Zero on success, non-zero on failure
 */
static int cvmx_helper_initialize_fpa_pool(int pool, uint64_t buffer_size, uint64_t buffers, const char *name)
{
    if (buffers == 0)
        return 0;

    uint64_t current_num = cvmx_read_csr(CVMX_FPA_QUEX_AVAILABLE(pool));
    if (current_num)
    {
        cvmx_dprintf("Fpa pool %d(%s) already has %llu buffers. Skipping setup.\n", pool, name, (unsigned long long)current_num);
        return 0;
    }

    void *memory = cvmx_bootmem_alloc(buffer_size * buffers, CVMX_CACHE_LINE_SIZE);
    if (memory == NULL)
    {
        cvmx_dprintf("Out of memory initializing fpa pool %d(%s).\n", pool, name);
        return -1;
    }
    cvmx_fpa_setup_pool(pool, name, memory, buffer_size, buffers);
    return 0;
}


int cvmx_helper_initialize_fpa_internal(int pip_pool, int pip_size, int pip_buffers,
                                        int wqe_pool, int wqe_size, int wqe_entries,
                                        int pko_pool, int pko_size, int pko_buffers,
                                        int tim_pool, int tim_size, int tim_buffers,
                                        int dfa_pool, int dfa_size, int dfa_buffers)
{
    int status;

    cvmx_fpa_enable();

    if ((pip_buffers > 0) && (pip_buffers <= 64))
        cvmx_dprintf("Warning: %d packet buffers may not be enough for hardware prefetch. 65 or more is recommended.\n", pip_buffers);

    if (pip_pool >= 0)
    {
        status = cvmx_helper_initialize_fpa_pool(pip_pool, pip_size, pip_buffers, "Packet Buffers");
        if (status)
            return status;
    }

    if (wqe_pool >= 0)
    {
        status = cvmx_helper_initialize_fpa_pool(wqe_pool, wqe_size, wqe_entries, "Work Queue Entries");
        if (status)
            return status;
        }

    if (pko_pool >= 0)
    {
        status = cvmx_helper_initialize_fpa_pool(pko_pool, pko_size, pko_buffers, "PKO Command Buffers");
        if (status)
            return status;
    }

    if (tim_pool >= 0)
    {
        status = cvmx_helper_initialize_fpa_pool(tim_pool, tim_size, tim_buffers, "TIM Command Buffers");
        if (status)
            return status;
    }

    if (dfa_pool >= 0)
    {
        status = cvmx_helper_initialize_fpa_pool(dfa_pool, dfa_size, dfa_buffers, "DFA Command Buffers");
        if (status)
            return status;
    }

    return 0;
}


/**
 * Convert a CVS Name tag into a version string.
 *
 * @param cvs_name_str
 *               CVS Name tag to parse
 * @return Version string. Note this buffer is allocated statically
 *         and will be shared by all callers.
 */
const char *cvmx_helper_parse_version(const char *cvs_name_str)
{
    static char version[80];
    char *ptr;

    /* Character 7 is a space when there isn't a tag. Use this as a key to
        return the build date */
    if (1 || cvs_name_str[7] == ' ')
    {
        snprintf(version, sizeof(version), "Internal %s", __DATE__);
        version[sizeof(version)-1] = 0;
        return version;
    }
    else
    {
        /* Make a static copy of the CVS Name string so we can modify it */
        strncpy(version, cvs_name_str, sizeof(version));
        version[sizeof(version)-1] = 0;

        /* Make sure there is an ending space in case someone didn't pass us
            a CVS Name string */
        version[sizeof(version)-2] = ' ';

        /* Truncate the string at the first space after the tag */
        *strchr(version+7, ' ') = 0;

        /* Convert all underscores into spaces or dots */
        while ((ptr = strchr(version, '_')) != NULL)
        {
            if ((ptr == version) ||                     /* Assume an underscore at beginning should be a space */
                (ptr[-1] < '0') || (ptr[-1] > '9') ||   /* If the character before it isn't a digit */
                (ptr[1] < '0') || (ptr[1] > '9'))       /* If the character after it isn't a digit */
                *ptr = ' ';
            else
                *ptr = '.';
        }

        /* Skip over the dollar Name: at the front */
        return version + 7;
    }
}


/**
 * Get the version of the CVMX libraries.
 *
 * @return Version string. Note this buffer is allocated statically
 *         and will be shared by all callers.
 */
const char *cvmx_helper_get_version(void)
{
    return cvmx_helper_parse_version("$Name$");
}

#if defined(CVMX_ENABLE_HELPER_FUNCTIONS)

/**
 * This function needs to be called on all Octeon chips with
 * errata PKI-100.
 *
 * The Size field is 8 too large in WQE and next pointers
 *
 *  The Size field generated by IPD is 8 larger than it should
 *  be. The Size field is <55:40> of both:
 *      - WORD3 in the work queue entry, and
 *      - the next buffer pointer (which precedes the packet data
 *        in each buffer).
 *
 * @param work   Work queue entry to fix
 * @return Zero on success. Negative on failure
 */
int cvmx_helper_fix_ipd_packet_chain(cvmx_wqe_t *work)
{
    uint64_t number_buffers = work->word2.s.bufs;

    /* We only need to do this if the work has buffers */
    if (number_buffers)
    {
        cvmx_buf_ptr_t buffer_ptr = work->packet_ptr;
        /* Check for errata PKI-100 */
        if ( (buffer_ptr.s.pool == 0) && (((uint64_t)buffer_ptr.s.size +
                 ((uint64_t)buffer_ptr.s.back << 7) + ((uint64_t)buffer_ptr.s.addr & 0x7F))
                 != (CVMX_FPA_PACKET_POOL_SIZE+8))) {
            /* fix is not needed */
            return 0;
        }
        /* Decrement the work packet pointer */
        buffer_ptr.s.size -= 8;
        work->packet_ptr = buffer_ptr;

        /* Now loop through decrementing the size for each additional buffer */
        while (--number_buffers)
        {
            /* Chain pointers are 8 bytes before the data */
            cvmx_buf_ptr_t *ptr = (cvmx_buf_ptr_t*)cvmx_phys_to_ptr(buffer_ptr.s.addr - 8);
            buffer_ptr = *ptr;
            buffer_ptr.s.size -= 8;
            *ptr = buffer_ptr;
        }
    }
    /* Make sure that these write go out before other operations such as FPA frees */
#if defined(__linux__)
    CVMX_SYNCWS;
#else
    CVMX_SYNCWS;
#endif
    return 0;
}


/**
 * Debug routine to dump the packet structure to the console
 *
 * @param work   Work queue entry containing the packet to dump
 * @return
 */
int cvmx_helper_dump_packet(cvmx_wqe_t *work)
{
    uint64_t        count;
    uint64_t        remaining_bytes;
    cvmx_buf_ptr_t  buffer_ptr;
    uint64_t        start_of_buffer;
    uint8_t *       data_address;
    uint8_t *       end_of_data;

    printf("Packet Length:   %u\n", work->len);
    printf("    Input Port:  %u\n", work->ipprt);
    printf("    QoS:         %u\n", work->qos);
    printf("    Buffers:     %u\n", work->word2.s.bufs);

    if (work->word2.s.bufs == 0)
    {
        buffer_ptr.u64 = 0;
        buffer_ptr.s.pool = 1; /* Not used for anything */
        buffer_ptr.s.size = 128;
        buffer_ptr.s.addr = cvmx_ptr_to_phys(work->packet_data);
        if (cvmx_likely(!work->word2.s.not_IP))
        {
            if (work->word2.s.is_v6)
                buffer_ptr.s.addr += 2;
            else
                buffer_ptr.s.addr += 6;
        }
    }
    else
        buffer_ptr = work->packet_ptr;
    remaining_bytes = work->len;

    while (remaining_bytes)
    {
        start_of_buffer = ((buffer_ptr.s.addr >> 7) - buffer_ptr.s.back) << 7;
        printf("    Buffer Start:%llx\n", (unsigned long long)start_of_buffer);
        printf("    Buffer I   : %u\n", buffer_ptr.s.i);
        printf("    Buffer Back: %u\n", buffer_ptr.s.back);
        printf("    Buffer Pool: %u\n", buffer_ptr.s.pool);
        printf("    Buffer Data: %llx\n", (unsigned long long)buffer_ptr.s.addr);
        printf("    Buffer Size: %u\n", buffer_ptr.s.size);

        printf("\t\t");
        data_address = cvmx_phys_to_ptr(buffer_ptr.s.addr);
        end_of_data = data_address + buffer_ptr.s.size;
        count = 0;
        while (data_address < end_of_data)
        {
            if (remaining_bytes == 0)
                break;
            else
                remaining_bytes--;
            printf("%02x", (unsigned int)*data_address);
            data_address++;
            if (remaining_bytes && (count == 7))
            {
                printf("\n\t\t");
                count = 0;
            }
            else
                count++;
        }
        printf("\n");

        if (remaining_bytes)
            buffer_ptr = *(cvmx_buf_ptr_t*)cvmx_phys_to_ptr(buffer_ptr.s.addr - 8);
    }
    return 0;
}


/**
 * Setup Random Early Drop on a specific input queue
 *
 * @param queue  Input queue to setup RED on (0-7)
 * @param pass_thresh
 *               Packets will begin slowly dropping when there are less than
 *               this many packet buffers free in FPA 0.
 * @param drop_thresh
 *               All incomming packets will be dropped when there are less
 *               than this many free packet buffers in FPA 0.
 * @return Zero on success. Negative on failure
 */
int cvmx_helper_setup_red_queue(int queue, int pass_thresh, int drop_thresh)
{
    /* Set RED to begin dropping packets when there are pass_thresh buffers
        left. It will linearly drop more packets until reaching drop_thresh
        buffers */
    cvmx_ipd_qos_red_marks_t red_marks;
    red_marks.u64 = 0;
    red_marks.s.drop = drop_thresh;
    red_marks.s.pass = pass_thresh;
    cvmx_write_csr(CVMX_IPD_QOSX_RED_MARKS(queue), red_marks.u64);

    /* Use the actual queue 0 counter, not the average */
    cvmx_ipd_red_quex_param_t red_param;
    red_param.u64 = 0;
    red_param.s.prb_con = (255ul<<24) / (red_marks.s.pass - red_marks.s.drop);
    red_param.s.avg_con = 1;
    red_param.s.new_con = 255;
    red_param.s.use_pcnt = 1;
    cvmx_write_csr(CVMX_IPD_RED_QUEX_PARAM(queue), red_param.u64);
    return 0;
}


/**
 * Setup Random Early Drop to automatically begin dropping packets.
 *
 * @param pass_thresh
 *               Packets will begin slowly dropping when there are less than
 *               this many packet buffers free in FPA 0.
 * @param drop_thresh
 *               All incomming packets will be dropped when there are less
 *               than this many free packet buffers in FPA 0.
 * @return Zero on success. Negative on failure
 */
int cvmx_helper_setup_red(int pass_thresh, int drop_thresh)
{
    /* Disable backpressure based on queued buffers. It needs SW support */
    cvmx_ipd_portx_bp_page_cnt_t page_cnt;
    page_cnt.u64 = 0;
    page_cnt.s.bp_enb = 0;
    page_cnt.s.page_cnt = 100;
    int port;
    if (OCTEON_IS_MODEL(OCTEON_CN30XX) || OCTEON_IS_MODEL(OCTEON_CN31XX) || OCTEON_IS_MODEL(OCTEON_CN50XX))
    {
        for (port=0; port<3; port++)
            cvmx_write_csr(CVMX_IPD_PORTX_BP_PAGE_CNT(port), page_cnt.u64);
    }
    else
    {
        for (port=0; port<36; port++)
            cvmx_write_csr(CVMX_IPD_PORTX_BP_PAGE_CNT(port), page_cnt.u64);
    }

    int queue;
    for (queue=0; queue<8; queue++)
        cvmx_helper_setup_red_queue(queue, pass_thresh, drop_thresh);

    /* Shutoff the dropping based on the per port page count. SW isn't
        decrementing it right now */
    cvmx_write_csr(CVMX_IPD_BP_PRT_RED_END, 0);

    cvmx_ipd_red_port_enable_t red_port_enable;
    red_port_enable.u64 = 0;
    red_port_enable.s.prt_enb = 0xfffffffffull;
    red_port_enable.s.avg_dly = 10000;
    red_port_enable.s.prb_dly = 10000;
    cvmx_write_csr(CVMX_IPD_RED_PORT_ENABLE, red_port_enable.u64);

    return 0;
}


void cvmx_helper_rgmii_internal_loopback(int port)
{
    int interface = (port >> 4) & 1;
    int index = port & 0xf;

    cvmx_gmxx_prtx_cfg_t gmx_cfg;
    gmx_cfg.u64 = 0;
    gmx_cfg.s.duplex = 1;
    gmx_cfg.s.slottime = 1;
    gmx_cfg.s.speed = 1;
    cvmx_write_csr(CVMX_GMXX_TXX_CLK(index, interface), 1);
    cvmx_write_csr(CVMX_GMXX_TXX_SLOT(index, interface), 0x200);
    cvmx_write_csr(CVMX_GMXX_TXX_BURST(index, interface), 0x2000);
    cvmx_write_csr(CVMX_GMXX_PRTX_CFG(index, interface), gmx_cfg.u64);
    uint64_t tmp = cvmx_read_csr(CVMX_ASXX_PRT_LOOP(interface));
    cvmx_write_csr(CVMX_ASXX_PRT_LOOP(interface), (1 << port) | tmp);
    tmp = cvmx_read_csr(CVMX_ASXX_TX_PRT_EN(interface));
    cvmx_write_csr(CVMX_ASXX_TX_PRT_EN(interface), (1 << port) | tmp);
    tmp = cvmx_read_csr(CVMX_ASXX_RX_PRT_EN(interface));
    cvmx_write_csr(CVMX_ASXX_RX_PRT_EN(interface), (1 << port) | tmp);
}

/**
 * Configure the RGMII port to match the PHY auto negotiated
 * speed and duplex
 *
 * @param port   Port to configure (0-3,16-19)
 */
void cvmx_helper_rgmii_speed(int port)
{
    int interface = port >> 4;
    int index = port & 0xf;
    const char *link_duplex;
    const char *link_speed;
    const char *link_status;

    cvmx_gmxx_rxx_rx_inbnd_t link;
    if (cvmx_sysinfo_get()->board_type == CVMX_BOARD_TYPE_TRANTOR)
    {
        /* Inband status does not seem to work */
        cvmx_dprintf("Forcing link speed to 1000Mbps for Trantor board.\n");
        link.s.speed = 2;
        link.s.duplex = 1;
        link.s.status = 1;
    }
    else if (cvmx_sysinfo_get()->board_type == CVMX_BOARD_TYPE_SIM)
    {
       /* simulates 1 Gbps full-duplex interfaces */
       link.u64 = 0;
       link.s.speed = 2;
       link.s.status = 1;
       link.s.duplex = 1;
    }
    else if (OCTEON_IS_MODEL(OCTEON_CN31XX) || OCTEON_IS_MODEL(OCTEON_CN3010) || OCTEON_IS_MODEL(OCTEON_CN3005) || OCTEON_IS_MODEL(OCTEON_CN50XX))
    {
        /* Bootloader must set up GMXX_INF_MODE appropriately */
        cvmx_gmxx_inf_mode_t mode;
        mode.u64 = cvmx_read_csr(CVMX_GMXX_INF_MODE(interface));
        if (port == 1 && mode.s.type)
        {
            /* Ports 1/2 are combined into a single GMII interface
            ** GMII only supports Gigabit full duplex */
            link.u64 = 0;
            link.s.speed = 2;
            link.s.status = 1;
            link.s.duplex = 1;
        }
        else if (cvmx_sysinfo_get()->board_type == CVMX_BOARD_TYPE_CN3005_EVB_HS5 && port == 1)
        {
            /* Port 1 is RGMII link to switch on this board, so force speed */
            link.u64 = 0;
            link.s.speed = 2;
            link.s.status = 1;
            link.s.duplex = 1;
        }
        else

            link.u64 = cvmx_read_csr(CVMX_GMXX_RXX_RX_INBND(index, interface));
    }
    else
      link.u64 = cvmx_read_csr(CVMX_GMXX_RXX_RX_INBND(index, interface));

    cvmx_gmxx_prtx_cfg_t gmx_cfg;
    gmx_cfg.u64 = cvmx_read_csr(CVMX_GMXX_PRTX_CFG(index, interface));

    if (cvmx_octeon_is_pass1())
        gmx_cfg.s.duplex = 1;   /* Half duplex is broken for Pass 1 */
    else
        gmx_cfg.s.duplex = link.s.duplex;

    if (link.s.duplex)
        link_duplex = "Full";
    else
        link_duplex = "Half";

    if (link.s.speed == 0)
    {
        link_speed = " 10Mbs";
        gmx_cfg.s.slottime = 0;
        gmx_cfg.s.speed = 0;
        cvmx_write_csr(CVMX_GMXX_TXX_CLK(index, interface), 50);
        cvmx_write_csr(CVMX_GMXX_TXX_SLOT(index, interface), 0x40);
        cvmx_write_csr(CVMX_GMXX_TXX_BURST(index, interface), 0);
    }
    else if (link.s.speed == 1)
    {
        link_speed = "100Mbs";
        gmx_cfg.s.slottime = 0;
        gmx_cfg.s.speed = 0;
        cvmx_write_csr(CVMX_GMXX_TXX_CLK(index, interface), 5);
        cvmx_write_csr(CVMX_GMXX_TXX_SLOT(index, interface), 0x40);
        cvmx_write_csr(CVMX_GMXX_TXX_BURST(index, interface), 0);
    }
    else if (link.s.speed == 2)
    {
        link_speed = "  1Gbs";
        gmx_cfg.s.slottime = 1;
        gmx_cfg.s.speed = 1;
        cvmx_write_csr(CVMX_GMXX_TXX_CLK(index, interface), 1);
        cvmx_write_csr(CVMX_GMXX_TXX_SLOT(index, interface), 0x200);
        cvmx_write_csr(CVMX_GMXX_TXX_BURST(index, interface), 0x2000);
    }
    else
    {
        link_speed = " Rsrvd";
        gmx_cfg.s.slottime = 1;
        gmx_cfg.s.speed = 1;
        cvmx_write_csr(CVMX_GMXX_TXX_CLK(index, interface), 1);
        cvmx_write_csr(CVMX_GMXX_TXX_SLOT(index, interface), 0x200);
        cvmx_write_csr(CVMX_GMXX_TXX_BURST(index, interface), 0x2000);
    }

    if (link.s.status)
        link_status = "Up  ";
    else
        link_status = "Down";

    cvmx_write_csr(CVMX_GMXX_PRTX_CFG(index, interface), gmx_cfg.u64);
    cvmx_dprintf("Port %d: %s %s %s duplex\n", port, link_status, link_speed, link_duplex);
}


/**
 * Configure all of the ASX, GMX, and PKO regsiters required
 * to get RGMII to function on the supplied interface.
 *
 * @param interface PKO Interface to configure (0 or 1)
 * @param num_ports Number of RGMII ethernet ports to configure (1 - 4)
 * @return Zero on success
 */
static int cvmx_helper_initialize_rgmii_interface(int interface, int num_ports)
{
    int port;
    cvmx_sysinfo_t *sys_info_ptr = cvmx_sysinfo_get();

    cvmx_gmxx_inf_mode_t mode;
    mode.u64 = cvmx_read_csr(CVMX_GMXX_INF_MODE(interface));

    if (mode.s.en == 0)
        return -1;
    if ((OCTEON_IS_MODEL(OCTEON_CN38XX) || OCTEON_IS_MODEL(OCTEON_CN58XX)) && mode.s.type == 1)   /* Ignore SPI interfaces */
        return -1;

    /* Configure the ASX registers needed to use the RGMII ports */
    cvmx_asxx_tx_prt_en_t asx_tx;
    asx_tx.u64 = 0;
    asx_tx.s.prt_en = cvmx_build_mask(num_ports);
    cvmx_write_csr(CVMX_ASXX_TX_PRT_EN(interface), asx_tx.u64);

    cvmx_asxx_rx_prt_en_t asx_rx;
    asx_rx.u64 = 0;
    asx_rx.s.prt_en = cvmx_build_mask(num_ports);
    cvmx_write_csr(CVMX_ASXX_RX_PRT_EN(interface), asx_rx.u64);

    /* Configure the GMX registers needed to use the RGMII ports */
    for (port=0; port<num_ports; port++)
    {
        cvmx_write_csr(CVMX_GMXX_TXX_THRESH(port, interface), 32);

        cvmx_helper_rgmii_speed(port + interface * 16);

        if (cvmx_octeon_is_pass1())
        {
            /* Set hi water mark as per errata GMX-4 */
            if (sys_info_ptr->cpu_clock_hz >= 325000000 && sys_info_ptr->cpu_clock_hz < 375000000)
                cvmx_write_csr(CVMX_ASXX_TX_HI_WATERX(port, interface), 12);
            else if (sys_info_ptr->cpu_clock_hz >= 375000000 && sys_info_ptr->cpu_clock_hz < 437000000)
                cvmx_write_csr(CVMX_ASXX_TX_HI_WATERX(port, interface), 11);
            else if (sys_info_ptr->cpu_clock_hz >= 437000000 && sys_info_ptr->cpu_clock_hz < 550000000)
                cvmx_write_csr(CVMX_ASXX_TX_HI_WATERX(port, interface), 10);
            else if (sys_info_ptr->cpu_clock_hz >= 550000000 && sys_info_ptr->cpu_clock_hz < 687000000)
                cvmx_write_csr(CVMX_ASXX_TX_HI_WATERX(port, interface), 9);
            else
                cvmx_dprintf("Illegal clock frequency (%d). CVMX_ASXX_TX_HI_WATERX not set\n", (unsigned int)sys_info_ptr->cpu_clock_hz);
        }
        /* Configure more flexible RGMII preamble checking. Pass 1 doesn't
        ** support this feature. */
        if (!cvmx_octeon_is_pass1())
        {
            cvmx_gmxx_rxx_frm_ctl_t frm_ctl;
            frm_ctl.u64 = cvmx_read_csr(CVMX_GMXX_RXX_FRM_CTL(port, interface));
            frm_ctl.s.pre_free = 1;  /* New field, so must be compile time */
            cvmx_write_csr(CVMX_GMXX_RXX_FRM_CTL(port, interface), frm_ctl.u64);
        }

        /*
         * Each pause frame transmitted will ask for about 10M bit times before resume.  If buffer space comes
         * available before that time has expired, an XON pause frame (0 time) will be transmitted to restart
         * the flow.
         */
        cvmx_write_csr(CVMX_GMXX_TXX_PAUSE_PKT_TIME(port, interface), 20000);
        cvmx_write_csr(CVMX_GMXX_TXX_PAUSE_PKT_INTERVAL(port, interface), 19000);

        if (sys_info_ptr->board_type == CVMX_BOARD_TYPE_TRANTOR)
        {
            cvmx_write_csr(CVMX_ASXX_TX_CLK_SETX(port, interface), 0);
            cvmx_write_csr(CVMX_ASXX_RX_CLK_SETX(port, interface), 16);
        }
        else if (sys_info_ptr->board_type == CVMX_BOARD_TYPE_CN3005_EVB_HS5 && port == 1)
        {
            /* Different config for switch port */
            cvmx_write_csr(CVMX_ASXX_TX_CLK_SETX(port, interface), 0);
            cvmx_write_csr(CVMX_ASXX_RX_CLK_SETX(port, interface), 0);
        }
        else
        {
            cvmx_write_csr(CVMX_ASXX_TX_CLK_SETX(port, interface), 24);
            cvmx_write_csr(CVMX_ASXX_RX_CLK_SETX(port, interface), 24);
        }
    }

    cvmx_gmxx_tx_prts_t gmx_tx_prts;
    gmx_tx_prts.u64 = 0;
    gmx_tx_prts.s.prts = num_ports;
    cvmx_write_csr(CVMX_GMXX_TX_PRTS(interface), gmx_tx_prts.u64);

    cvmx_gmxx_rx_prts_t gmx_rx_prts;
    gmx_rx_prts.u64 = 0;
    gmx_rx_prts.s.prts = num_ports;
    cvmx_write_csr(CVMX_GMXX_RX_PRTS(interface), gmx_rx_prts.u64);

    if (OCTEON_IS_MODEL(OCTEON_CN38XX) 
	|| OCTEON_IS_MODEL(OCTEON_CN58XX) 
	|| OCTEON_IS_MODEL(OCTEON_CN56XX))
    {
        /* PKO registers */
        cvmx_pko_reg_gmx_port_mode_t pko_mode;
        pko_mode.u64 = cvmx_read_csr(CVMX_PKO_REG_GMX_PORT_MODE);
        if (interface == 0)
        {
            if (num_ports == 1)
                pko_mode.s.mode0 = 4;
            else if (num_ports == 2)
                pko_mode.s.mode0 = 3;
            else
                pko_mode.s.mode0 = 2;
        }
        else
        {
            if (num_ports == 1)
                pko_mode.s.mode1 = 4;
            else if (num_ports == 2)
                pko_mode.s.mode1 = 3;
            else
                pko_mode.s.mode1 = 2;
        }
        cvmx_write_csr(CVMX_PKO_REG_GMX_PORT_MODE, pko_mode.u64);
    }

#if CVMX_HELPER_DISABLE_RGMII_BACKPRESSURE
    /* Disable backpressure if configured to do so */
    /* Disable backpressure (pause frame) generation */
    cvmx_gmx_set_backpressure_override(interface, 0xf);
    cvmx_dprintf("Disabling backpressure\n");
#endif

    /* enable the ports now */
    for (port=0; port<num_ports; port++)
    {
        cvmx_gmxx_prtx_cfg_t gmx_cfg;
        gmx_cfg.u64 = cvmx_read_csr(CVMX_GMXX_PRTX_CFG(port, interface));
        gmx_cfg.s.en = (port<num_ports) ? 1 : 0;
        cvmx_write_csr(CVMX_GMXX_PRTX_CFG(port, interface), gmx_cfg.u64);
    }

    return 0;
}


static CVMX_SHARED int interface_port_count[2] = {-1. -1};  /* Port count per interface */

/**
 * Enables the packet interfaces (RGMII and SPI).  This must be
 * done after the internal blocks such as the FPA and IPD are enabled.
 * 
 * @return 0 on success,
 *         -1 on failure
 */
static int cvmx_helper_packet_input_enable_internal(void)
{
    int interface;
    cvmx_gmxx_inf_mode_t mode;
    int max_interface = 1;
    if (OCTEON_IS_MODEL(OCTEON_CN38XX))
        max_interface = 2;


    for (interface=0; interface<max_interface; interface++)
    {
        mode.u64 = cvmx_read_csr(CVMX_GMXX_INF_MODE(interface));
        if (mode.s.en)
        {
            if ((OCTEON_IS_MODEL(OCTEON_CN38XX) || OCTEON_IS_MODEL(OCTEON_CN58XX)) && mode.s.type)
            {
                if (interface_port_count[interface] > 0)
                {

                    if (cvmx_spi4000_initialize(interface) == 0)
                    {
                        cvmx_dprintf("SPI4000 card detected on interface %d.\n", interface);
                    }
                    else
                    {
                        cvmx_pko_reg_crc_enable_t enable;
                        enable.u64 = cvmx_read_csr(CVMX_PKO_REG_CRC_ENABLE);
                        enable.s.enable |= 0xffff << (interface*16);
                        cvmx_write_csr(CVMX_PKO_REG_CRC_ENABLE, enable.u64);
                    }
                }
            }
            else
            {
                cvmx_helper_initialize_rgmii_interface(interface, interface_port_count[interface]);
            }
        }
    }

    return 0;
}


/**
 * Function to adjust internal IPD pointer alignments
 * 
 * @return 0 on success
 *         !0 on failure
 */
static int cvmx_helper_fix_ipd_ptr_alignment(void)
{
#define FIX_IPD_FIRST_BUFF_PAYLOAD_BYTES     (CVMX_FPA_PACKET_POOL_SIZE-8-CVMX_HELPER_FIRST_MBUFF_SKIP)
#define FIX_IPD_NON_FIRST_BUFF_PAYLOAD_BYTES (CVMX_FPA_PACKET_POOL_SIZE-8-CVMX_HELPER_NOT_FIRST_MBUFF_SKIP)
#define FIX_IPD_OUTPORT 0
#define INTERFACE(port) (port >> 4) /* Ports 0-15 are interface 0, 16-31 are interface 1 */
#define INDEX(port) (port & 0xf)
    uint64_t tx_command_state;
    uint64_t *tx_command_ptr, *p64;
    uint64_t tx_command_index;
    cvmx_pko_command_word0_t    pko_command;
    cvmx_buf_ptr_t              g_buffer, pkt_buffer;
    cvmx_wqe_t *work;
    int size, num_segs = 0, wqe_pcnt, pkt_pcnt;
    cvmx_gmxx_prtx_cfg_t gmx_cfg;
    int retry_cnt;
    int retry_loop_cnt;
    int mtu;
    int i;

    /* Save values for restore at end */
    uint64_t prtx_cfg = cvmx_read_csr(CVMX_GMXX_PRTX_CFG(INDEX(FIX_IPD_OUTPORT), INTERFACE(FIX_IPD_OUTPORT)));
    uint64_t tx_ptr_en = cvmx_read_csr(CVMX_ASXX_TX_PRT_EN(INTERFACE(FIX_IPD_OUTPORT)));
    uint64_t rx_ptr_en = cvmx_read_csr(CVMX_ASXX_RX_PRT_EN(INTERFACE(FIX_IPD_OUTPORT)));
    uint64_t rxx_jabber = cvmx_read_csr(CVMX_GMXX_RXX_JABBER(INDEX(FIX_IPD_OUTPORT), INTERFACE(FIX_IPD_OUTPORT)));

    /* Configure port to gig FDX as required for loopback mode */
    cvmx_helper_rgmii_internal_loopback(FIX_IPD_OUTPORT);

    /* Disable reception on all ports so if traffic is present it will not interfere. */
    cvmx_write_csr(CVMX_ASXX_RX_PRT_EN(INTERFACE(FIX_IPD_OUTPORT)), 0);

    cvmx_wait(100000000ull);

    for (retry_loop_cnt = 0;retry_loop_cnt < 10;retry_loop_cnt++)
    {
        retry_cnt = 100000;
        wqe_pcnt = cvmx_read_csr(CVMX_IPD_PTR_COUNT);
        pkt_pcnt = (wqe_pcnt >> 7) & 0x7f;
        wqe_pcnt &= 0x7f;

        num_segs = (2 + pkt_pcnt - wqe_pcnt) & 3;

        if (num_segs == 0)
            goto fix_ipd_exit;

        num_segs += 1;

        size = FIX_IPD_FIRST_BUFF_PAYLOAD_BYTES + ((num_segs-1)*FIX_IPD_NON_FIRST_BUFF_PAYLOAD_BYTES) -
            (FIX_IPD_NON_FIRST_BUFF_PAYLOAD_BYTES / 2);

        cvmx_write_csr(CVMX_ASXX_PRT_LOOP(INTERFACE(FIX_IPD_OUTPORT)), 1 << INDEX(FIX_IPD_OUTPORT));
        CVMX_SYNC;

        g_buffer.u64 = 0;
        g_buffer.s.addr = cvmx_ptr_to_phys(cvmx_fpa_alloc(CVMX_FPA_WQE_POOL));
        if (g_buffer.s.addr == 0) {
            cvmx_dprintf("WARNING: FIX_IPD_PTR_ALIGNMENT buffer allocation failure.\n");
            goto fix_ipd_exit;
        }

        g_buffer.s.pool = CVMX_FPA_WQE_POOL;
        g_buffer.s.size = num_segs;

        pkt_buffer.u64 = 0;
        pkt_buffer.s.addr = cvmx_ptr_to_phys(cvmx_fpa_alloc(CVMX_FPA_PACKET_POOL));
        if (pkt_buffer.s.addr == 0) {
            cvmx_dprintf("WARNING: FIX_IPD_PTR_ALIGNMENT buffer allocation failure.\n");
            goto fix_ipd_exit;
        }
        pkt_buffer.s.i = 1;
        pkt_buffer.s.pool = CVMX_FPA_PACKET_POOL;
        pkt_buffer.s.size = FIX_IPD_FIRST_BUFF_PAYLOAD_BYTES;

        p64 = (uint64_t*) cvmx_phys_to_ptr(pkt_buffer.s.addr);
        p64[0] = 0xffffffffffff0000ull;
        p64[1] = 0x08004510ull;
        p64[2] = ((uint64_t)(size-14) << 48) | 0x5ae740004000ull;
        p64[3] = 0x3a5fc0a81073c0a8ull;

        for (i=0;i<num_segs;i++)
        {
            if (i>0)
                pkt_buffer.s.size = FIX_IPD_NON_FIRST_BUFF_PAYLOAD_BYTES;

            if (i==(num_segs-1))
                pkt_buffer.s.i = 0;

            *(uint64_t*)cvmx_phys_to_ptr(g_buffer.s.addr + 8*i) = pkt_buffer.u64;
        }

        /* Build the PKO command */
        pko_command.u64 = 0;
        pko_command.s.segs = num_segs;
        pko_command.s.total_bytes = size;
        pko_command.s.dontfree = 0;
        pko_command.s.gather = 1;

        /* Get the queue command ptr location from the FAU */
        tx_command_state = cvmx_fau_fetch_and_add64(CVMX_FAU_REG_OQ_ADDR_INDEX + (8*cvmx_pko_get_base_queue(FIX_IPD_OUTPORT)), 2);
        tx_command_ptr = cvmx_phys_to_ptr(tx_command_state>>CVMX_PKO_INDEX_BITS);
        tx_command_index = tx_command_state & CVMX_PKO_INDEX_MASK;
        tx_command_ptr += tx_command_index;

        /* No buffer needed. Output the command and go */
        tx_command_ptr[0] = pko_command.u64;
        tx_command_ptr[1] = g_buffer.u64;

        gmx_cfg.u64 = cvmx_read_csr(CVMX_GMXX_PRTX_CFG(INDEX(FIX_IPD_OUTPORT), INTERFACE(FIX_IPD_OUTPORT)));
        gmx_cfg.s.en = 1; 
        cvmx_write_csr(CVMX_GMXX_PRTX_CFG(INDEX(FIX_IPD_OUTPORT), INTERFACE(FIX_IPD_OUTPORT)), gmx_cfg.u64);
        cvmx_write_csr(CVMX_ASXX_TX_PRT_EN(INTERFACE(FIX_IPD_OUTPORT)), 1 << INDEX(FIX_IPD_OUTPORT));
        cvmx_write_csr(CVMX_ASXX_RX_PRT_EN(INTERFACE(FIX_IPD_OUTPORT)), 1 << INDEX(FIX_IPD_OUTPORT));

        mtu = cvmx_read_csr(CVMX_GMXX_RXX_JABBER(INDEX(FIX_IPD_OUTPORT), INTERFACE(FIX_IPD_OUTPORT)));
        cvmx_write_csr(CVMX_GMXX_RXX_JABBER(INDEX(FIX_IPD_OUTPORT), INTERFACE(FIX_IPD_OUTPORT)), 65392-14-4);
        CVMX_SYNCWS;

        cvmx_pko_doorbell(FIX_IPD_OUTPORT, cvmx_pko_get_base_queue(FIX_IPD_OUTPORT), 2);

        CVMX_SYNC;

        do {
            work = cvmx_pow_work_request_sync(CVMX_POW_WAIT);
            retry_cnt--;
        } while ((work == NULL) && (retry_cnt > 0));

        if (!retry_cnt)
            cvmx_dprintf("WARNING: FIX_IPD_PTR_ALIGNMENT get_work() timeout occured.\n");


        /* Free packet */
        if (work)
            cvmx_helper_free_packet_data(work);
    }

fix_ipd_exit:

    /* Return CSR configs to saved values */
    cvmx_write_csr(CVMX_GMXX_PRTX_CFG(INDEX(FIX_IPD_OUTPORT), INTERFACE(FIX_IPD_OUTPORT)), prtx_cfg);
    cvmx_write_csr(CVMX_ASXX_TX_PRT_EN(INTERFACE(FIX_IPD_OUTPORT)), tx_ptr_en);
    cvmx_write_csr(CVMX_ASXX_RX_PRT_EN(INTERFACE(FIX_IPD_OUTPORT)), rx_ptr_en);
    cvmx_write_csr(CVMX_GMXX_RXX_JABBER(INDEX(FIX_IPD_OUTPORT), INTERFACE(FIX_IPD_OUTPORT)), rxx_jabber);
    cvmx_write_csr(CVMX_ASXX_PRT_LOOP(INTERFACE(FIX_IPD_OUTPORT)), 0);
    CVMX_SYNC;
    if (num_segs)
        cvmx_dprintf("WARNING: FIX_IPD_PTR_ALIGNMENT failed.\n");

    return(!!num_segs);

}



int cvmx_helper_ipd_and_packet_input_enable(void)
{
    cvmx_ipd_enable();
    if ((OCTEON_IS_MODEL(OCTEON_CN30XX_PASS1) || OCTEON_IS_MODEL(OCTEON_CN31XX_PASS1)) &&
        (cvmx_sysinfo_get()->board_type != CVMX_BOARD_TYPE_SIM))
        cvmx_helper_fix_ipd_ptr_alignment();
    return(cvmx_helper_packet_input_enable_internal());
}

int cvmx_helper_ports_on_interface(int interface)
{
    return(interface_port_count[interface]);
}
/**
 * Initialize the PIP, IPD, and PKO hardware to support
 * simple priority based queues for the ethernet ports. Each
 * port is configured with a number of priority queues based
 * on CVMX_PKO_QUEUES_PER_PORT_* where each queue is lower
 * priority than the previous. Use cvmx_helper_initialize_packet_io_global
 * instead of this one directly.
 *
 * @return Zero on success, non-zero on failure
 */
int cvmx_helper_initialize_packet_io_internal_global(void)
{
    /* Each packet output queue has an associated priority. The higher the
        priority, the more often it can send a packet. A priority of 8 means
        it can send in all 8 rounds of contention. We're going to make each
        queue one less that the last */
    const uint64_t priorities[8] = {8,7,6,5,4,3,2,1};

    cvmx_gmxx_inf_mode_t mode;
    int max_interface = 1;
    if (OCTEON_IS_MODEL(OCTEON_CN38XX))
        max_interface = 2;

    int interface;
    for (interface=0; interface<max_interface; interface++)
    {
        mode.u64 = cvmx_read_csr(CVMX_GMXX_INF_MODE(interface));
        if (mode.s.en)
        {
            int num_ports = 0;
            if ((OCTEON_IS_MODEL(OCTEON_CN38XX) || OCTEON_IS_MODEL(OCTEON_CN58XX)) && mode.s.type)
            {

                if (cvmx_spi_start_interface(interface, CVMX_SPI_MODE_DUPLEX, 10) == 0)
                    num_ports = 10;
                else
                    num_ports = 0;
            }
            else
            {
                /* Handle RGMII/GMII/MII interfaces here.  The .type bit has different
                ** meanings on the various Octeon models. */
                mode.u64 = cvmx_read_csr(CVMX_GMXX_INF_MODE(interface));
                if (OCTEON_IS_MODEL(OCTEON_CN31XX))
                {
                    if (mode.s.type)
                        num_ports = 2;
                    else
                        num_ports = 3;
                }
                else if (OCTEON_IS_MODEL(OCTEON_CN3010) || OCTEON_IS_MODEL(OCTEON_CN3005) || OCTEON_IS_MODEL(OCTEON_CN50XX))
                {
                    if (mode.s.type)
                        num_ports = 2;
                    else
                        num_ports = 3;

                    if (OCTEON_IS_MODEL(OCTEON_CN3005))
                        num_ports--;
                }
                else if (OCTEON_IS_MODEL(OCTEON_CN38XX) 
			 || OCTEON_IS_MODEL(OCTEON_CN58XX) 
			 || OCTEON_IS_MODEL(OCTEON_CN56XX))
                    num_ports = 4;
            }

            /* Save number of ports on each interface for later use when enabling them */
            interface_port_count[interface] = num_ports;

            /* We're going to configure every input and output port identically. Each
                port is configured to processes level 2 and 3 headers and maintain
                packet order. The user can change this later if needed. */
            int port = interface * 16;
            while (num_ports--)
            {
                cvmx_pip_port_cfg_t port_config;
                port_config.u64 = 0;
                port_config.s.qos = port & 0x7; /* Have each port go to a different POW queue */
                if ((OCTEON_IS_MODEL(OCTEON_CN38XX) || OCTEON_IS_MODEL(OCTEON_CN58XX)))
                    port_config.s.crc_en = mode.s.type; /* Enable CRC32 checking, but only for SPI */
                port_config.s.mode = CVMX_HELPER_INPUT_PORT_SKIP_MODE; /* Process the headers and place the IP header in the work queue */

                cvmx_pip_port_tag_cfg_t tag_config;
                tag_config.u64 = 0;
                tag_config.s.ip6_src_flag  = CVMX_HELPER_INPUT_TAG_IPV6_SRC_IP;
                tag_config.s.ip6_dst_flag  = CVMX_HELPER_INPUT_TAG_IPV6_DST_IP;
                tag_config.s.ip6_sprt_flag = CVMX_HELPER_INPUT_TAG_IPV6_SRC_PORT;
                tag_config.s.ip6_dprt_flag = CVMX_HELPER_INPUT_TAG_IPV6_DST_PORT;
                tag_config.s.ip6_nxth_flag = CVMX_HELPER_INPUT_TAG_IPV6_NEXT_HEADER;
                tag_config.s.ip4_src_flag  = CVMX_HELPER_INPUT_TAG_IPV4_SRC_IP;
                tag_config.s.ip4_dst_flag  = CVMX_HELPER_INPUT_TAG_IPV4_DST_IP;
                tag_config.s.ip4_sprt_flag = CVMX_HELPER_INPUT_TAG_IPV4_SRC_PORT;
                tag_config.s.ip4_dprt_flag = CVMX_HELPER_INPUT_TAG_IPV4_DST_PORT;
                tag_config.s.ip4_pctl_flag = CVMX_HELPER_INPUT_TAG_IPV4_PROTOCOL;
                tag_config.s.inc_prt_flag  = CVMX_HELPER_INPUT_TAG_INPUT_PORT;  /* Include the hardware port in the tag */
                tag_config.s.tcp6_tag_type = CVMX_HELPER_INPUT_TAG_TYPE; /* Keep the order of each port */
                tag_config.s.tcp4_tag_type = CVMX_HELPER_INPUT_TAG_TYPE;
                tag_config.s.ip6_tag_type = CVMX_HELPER_INPUT_TAG_TYPE;
                tag_config.s.ip4_tag_type = CVMX_HELPER_INPUT_TAG_TYPE;
                tag_config.s.non_tag_type = CVMX_HELPER_INPUT_TAG_TYPE;
                tag_config.s.grp = 0;           /* Put all packets in group 0. Other groups can be used by the app */

                /* Finally do the actual setup */
                cvmx_pip_config_port(port, port_config, tag_config);
                /* Packet output configures Queue and Ports */
                cvmx_pko_config_port(port, cvmx_pko_get_base_queue(port), cvmx_pko_get_num_queues(port), priorities);
                port++;
            }
        }
        else
            interface_port_count[interface] = 0;
    }

    /* Setup the global packet input options */
    cvmx_ipd_config(CVMX_FPA_PACKET_POOL_SIZE/8,
                    CVMX_HELPER_FIRST_MBUFF_SKIP/8,
                    CVMX_HELPER_NOT_FIRST_MBUFF_SKIP/8,
                    CVMX_HELPER_FIRST_MBUFF_SKIP / 128,
                    CVMX_HELPER_NOT_FIRST_MBUFF_SKIP / 128,
                    CVMX_FPA_WQE_POOL,
                    CVMX_IPD_OPC_MODE_STT,
                    CVMX_HELPER_ENABLE_BACK_PRESSURE);

    /* Pass 1 PKI-12 Errata: Ignore ipv4 header checksum violations */
    if (cvmx_octeon_is_pass1())
    {
        cvmx_pip_gbl_ctl_t global_control;
        global_control.u64 = cvmx_read_csr(CVMX_PIP_GBL_CTL);
        global_control.s.ip_chk = 0;
        cvmx_write_csr(CVMX_PIP_GBL_CTL, global_control.u64);
    }

    cvmx_pko_enable();

    /* Disable tagwait FAU timeout */
    cvmx_iob_fau_timeout_t fau_to;
    fau_to.u64 = 0;
    fau_to.s.tout_val = 0xfff;
    fau_to.s.tout_enb = 0;
    cvmx_write_csr(CVMX_IOB_FAU_TIMEOUT, fau_to.u64);

    /* Make sure to push out interface_port_count  */
    CVMX_SYNCWS;

    #if CVMX_HELPER_ENABLE_IPD
    cvmx_helper_ipd_and_packet_input_enable();
    #endif

    return 0;
}

#endif
