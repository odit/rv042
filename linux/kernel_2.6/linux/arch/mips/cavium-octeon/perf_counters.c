/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2004, 2005 Cavium Networks
 *
 * Simple /proc interface to the Octeon Performance Counters
 */
#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include <asm/ioctl.h>
#include <asm/mach-cavium-octeon/perf_counters.h>
#include "hal.h"

/**
 * The types of counters supported per cpu
 */
typedef enum
{
    PROC_PERF_CORE_NONE      = 0,    /**< Turn off the performance counter */
    PROC_PERF_CORE_CLK       = 1,    /**< Conditionally clocked cycles (as opposed to count/cvm_count which count even with no clocks) */
    PROC_PERF_CORE_ISSUE     = 2,    /**< Instructions issued but not retired */
    PROC_PERF_CORE_RET       = 3,    /**< Instructions retired */
    PROC_PERF_CORE_NISSUE    = 4,    /**< Cycles no issue */
    PROC_PERF_CORE_SISSUE    = 5,    /**< Cycles single issue */
    PROC_PERF_CORE_DISSUE    = 6,    /**< Cycles dual issue */
    PROC_PERF_CORE_IFI       = 7,    /**< Cycle ifetch issued (but not necessarily commit to pp_mem) */
    PROC_PERF_CORE_BR        = 8,    /**< Branches retired */
    PROC_PERF_CORE_BRMIS     = 9,    /**< Branch mispredicts */
    PROC_PERF_CORE_J         = 10,   /**< Jumps retired */
    PROC_PERF_CORE_JMIS      = 11,   /**< Jumps mispredicted */
    PROC_PERF_CORE_REPLAY    = 12,   /**< Mem Replays */
    PROC_PERF_CORE_IUNA      = 13,   /**< Cycles idle due to unaligned_replays */
    PROC_PERF_CORE_TRAP      = 14,   /**< trap_6a signal */
    PROC_PERF_CORE_UULOAD    = 16,   /**< Unexpected unaligned loads (REPUN=1) */
    PROC_PERF_CORE_UUSTORE   = 17,   /**< Unexpected unaligned store (REPUN=1) */
    PROC_PERF_CORE_ULOAD     = 18,   /**< Unaligned loads (REPUN=1 or USEUN=1) */
    PROC_PERF_CORE_USTORE    = 19,   /**< Unaligned store (REPUN=1 or USEUN=1) */
    PROC_PERF_CORE_EC        = 20,   /**< Exec clocks(must set CvmCtl[DISCE] for accurate timing) */
    PROC_PERF_CORE_MC        = 21,   /**< Mul clocks(must set CvmCtl[DISCE] for accurate timing) */
    PROC_PERF_CORE_CC        = 22,   /**< Crypto clocks(must set CvmCtl[DISCE] for accurate timing) */
    PROC_PERF_CORE_CSRC      = 23,   /**< Issue_csr clocks(must set CvmCtl[DISCE] for accurate timing) */
    PROC_PERF_CORE_CFETCH    = 24,   /**< Icache committed fetches (demand+prefetch) */
    PROC_PERF_CORE_CPREF     = 25,   /**< Icache committed prefetches */
    PROC_PERF_CORE_ICA       = 26,   /**< Icache aliases */
    PROC_PERF_CORE_II        = 27,   /**< Icache invalidates */
    PROC_PERF_CORE_IP        = 28,   /**< Icache parity error */
    PROC_PERF_CORE_CIMISS    = 29,   /**< Cycles idle due to imiss (must set CvmCtl[DISCE] for accurate timing) */
    PROC_PERF_CORE_WBUF      = 32,   /**< Number of write buffer entries created */
    PROC_PERF_CORE_WDAT      = 33,   /**< Number of write buffer data cycles used (may need to set CvmCtl[DISCE] for accurate counts) */
    PROC_PERF_CORE_WBUFLD    = 34,   /**< Number of write buffer entries forced out by loads */
    PROC_PERF_CORE_WBUFFL    = 35,   /**< Number of cycles that there was no available write buffer entry (may need to set CvmCtl[DISCE] and CvmMemCtl[MCLK] for accurate counts) */
    PROC_PERF_CORE_WBUFTR    = 36,   /**< Number of stores that found no available write buffer entries */
    PROC_PERF_CORE_BADD      = 37,   /**< Number of address bus cycles used (may need to set CvmCtl[DISCE] for accurate counts) */
    PROC_PERF_CORE_BADDL2    = 38,   /**< Number of address bus cycles not reflected (i.e. destined for L2) (may need to set CvmCtl[DISCE] for accurate counts) */
    PROC_PERF_CORE_BFILL     = 39,   /**< Number of fill bus cycles used (may need to set CvmCtl[DISCE] for accurate counts) */
    PROC_PERF_CORE_DDIDS     = 40,   /**< Number of Dstream DIDs created */
    PROC_PERF_CORE_IDIDS     = 41,   /**< Number of Istream DIDs created */
    PROC_PERF_CORE_DIDNA     = 42,   /**< Number of cycles that no DIDs were available (may need to set CvmCtl[DISCE] and CvmMemCtl[MCLK] for accurate counts) */
    PROC_PERF_CORE_LDS       = 43,   /**< Number of load issues */
    PROC_PERF_CORE_LMLDS     = 44,   /**< Number of local memory load */
    PROC_PERF_CORE_IOLDS     = 45,   /**< Number of I/O load issues */
    PROC_PERF_CORE_DMLDS     = 46,   /**< Number of loads that were not prefetches and missed in the cache */
    PROC_PERF_CORE_STS       = 48,   /**< Number of store issues */
    PROC_PERF_CORE_LMSTS     = 49,   /**< Number of local memory store issues */
    PROC_PERF_CORE_IOSTS     = 50,   /**< Number of I/O store issues */
    PROC_PERF_CORE_IOBDMA    = 51,   /**< Number of IOBDMAs */
    PROC_PERF_CORE_DTLB      = 53,   /**< Number of dstream TLB refill, invalid, or modified exceptions */
    PROC_PERF_CORE_DTLBAD    = 54,   /**< Number of dstream TLB address errors */
    PROC_PERF_CORE_ITLB      = 55,   /**< Number of istream TLB refill, invalid, or address error exceptions */
    PROC_PERF_CORE_SYNC      = 56,   /**< Number of SYNC stall cycles (may need to set CvmCtl[DISCE] for accurate counts) */
    PROC_PERF_CORE_SYNCIOB   = 57,   /**< Number of SYNCIOBDMA stall cycles (may need to set CvmCtl[DISCE] for accurate counts) */
    PROC_PERF_CORE_SYNCW     = 58,   /**< Number of SYNCWs */
    PROC_PERF_CORE_MAX
} proc_perf_core_t;

/**
 * The types of counters supported for L2
 */
typedef enum
{
    PROC_PERF_L2_CYCLES,                                       /* Cycles */
    PROC_PERF_L2_IMISS,                                     /* L2 Instruction Miss */
    PROC_PERF_L2_IHIT,                                      /* L2 Instruction Hit */
    PROC_PERF_L2_DMISS,                                     /* L2 Data Miss */
    PROC_PERF_L2_DHIT,                                      /* L2 Data Hit */
    PROC_PERF_L2_MISS,                                      /* L2 Miss (I/D) */
    PROC_PERF_L2_HIT,                                       /* L2 Hit (I/D) */
    PROC_PERF_L2_VICTIM_BUFFER_HIT,                         /* L2 Victim Buffer Hit (Retry Probe) */
    PROC_PERF_L2_LFB_NQ_INDEX_CONFLICT,                        /* LFB-NQ Index Conflict */
    PROC_PERF_L2_TAG_PROBE,                                 /* L2 Tag Probe (issued - could be VB-Retried) */
    PROC_PERF_L2_TAG_UPDATE,                                /* L2 Tag Update (completed). Note: Some CMD types do not update */
    PROC_PERF_L2_TAG_PROBE_COMPLETED,                       /* L2 Tag Probe Completed (beyond VB-RTY window) */
    PROC_PERF_L2_TAG_DIRTY_VICTIM,                          /* L2 Tag Dirty Victim */
    PROC_PERF_L2_DATA_STORE_NOP,                            /* L2 Data Store NOP */
    PROC_PERF_L2_DATA_STORE_READ,                           /* L2 Data Store READ */
    PROC_PERF_L2_DATA_STORE_WRITE,                          /* L2 Data Store WRITE */
    PROC_PERF_L2_MEMORY_FILL_DATA_VALID,                       /* Memory Fill Data valid */
    PROC_PERF_L2_MEMORY_WRITE_REQUEST,                         /* Memory Write Request */
    PROC_PERF_L2_MEMORY_READ_REQUEST,                          /* Memory Read Request */
    PROC_PERF_L2_MEMORY_WRITE_DATA_VALID,                      /* Memory Write Data valid */
    PROC_PERF_L2_XMC_NOP,                                      /* XMC NOP */
    PROC_PERF_L2_XMC_LDT,                                      /* XMC LDT */
    PROC_PERF_L2_XMC_LDI,                                      /* XMC LDI */
    PROC_PERF_L2_XMC_LDD,                                      /* XMC LDD */
    PROC_PERF_L2_XMC_STF,                                      /* XMC STF */
    PROC_PERF_L2_XMC_STT,                                      /* XMC STT */
    PROC_PERF_L2_XMC_STP,                                      /* XMC STP */
    PROC_PERF_L2_XMC_STC,                                      /* XMC STC */
    PROC_PERF_L2_XMC_DWB,                                      /* XMC DWB */
    PROC_PERF_L2_XMC_PL2,                                      /* XMC PL2 */
    PROC_PERF_L2_XMC_PSL1,                                     /* XMC PSL1 */
    PROC_PERF_L2_XMC_IOBLD,                                    /* XMC IOBLD */
    PROC_PERF_L2_XMC_IOBST,                                    /* XMC IOBST */
    PROC_PERF_L2_XMC_IOBDMA,                                   /* XMC IOBDMA */
    PROC_PERF_L2_XMC_IOBRSP,                                   /* XMC IOBRSP */
    PROC_PERF_L2_XMD_BUS_VALID,                                /* XMD Bus valid (all) */
    PROC_PERF_L2_XMD_BUS_VALID_DST_L2C,                        /* XMD Bus valid (DST=L2C) Memory */
    PROC_PERF_L2_XMD_BUS_VALID_DST_IOB,                        /* XMD Bus valid (DST=IOB) REFL Data */
    PROC_PERF_L2_XMD_BUS_VALID_DST_PP,                         /* XMD Bus valid (DST=PP) IOBRSP Data */
    PROC_PERF_L2_RSC_NOP,                                      /* RSC NOP */
    PROC_PERF_L2_RSC_STDN,                                     /* RSC STDN */
    PROC_PERF_L2_RSC_FILL,                                     /* RSC FILL */
    PROC_PERF_L2_RSC_REFL,                                     /* RSC REFL */
    PROC_PERF_L2_RSC_STIN,                                     /* RSC STIN */
    PROC_PERF_L2_RSC_SCIN,                                     /* RSC SCIN */
    PROC_PERF_L2_RSC_SCFL,                                     /* RSC SCFL */
    PROC_PERF_L2_RSC_SCDN,                                     /* RSC SCDN */
    PROC_PERF_L2_RSD_DATA_VALID,                               /* RSD Data Valid */
    PROC_PERF_L2_RSD_DATA_VALID_FILL,                          /* RSD Data Valid (FILL) */
    PROC_PERF_L2_RSD_DATA_VALID_STRSP,                         /* RSD Data Valid (STRSP) */
    PROC_PERF_L2_RSD_DATA_VALID_REFL,                          /* RSD Data Valid (REFL) */
    PROC_PERF_L2_LRF_REQ,                                      /* LRF-REQ (LFB-NQ) */
    PROC_PERF_L2_DT_RD_ALLOC,                                  /* DT RD-ALLOC */
    PROC_PERF_L2_DT_WR_INVA,                                   /* DT WR-INVA */
    PROC_PERF_L2_MAX
} proc_perf_l2_t;

/**
 * IO addresses for L2 registers
 */
#define  OCTEON_L2C_PFCTL   0x8001180080000090ull
#define  OCTEON_L2C_PFC0    0x8001180080000098ull
#define  OCTEON_L2C_PFC1    0x80011800800000A0ull
#define  OCTEON_L2C_PFC2    0x80011800800000A8ull
#define  OCTEON_L2C_PFC3    0x80011800800000B0ull
#define  OCTEON_LMC_DCLK_CNT_HI 0x8001180088000070ull
#define  OCTEON_LMC_DCLK_CNT_LO 0x8001180088000068ull
#define  OCTEON_LMC_OPS_CNT_HI  0x8001180088000060ull
#define  OCTEON_LMC_OPS_CNT_LO  0x8001180088000058ull

/**
 * Bit description of the core counters control register
 */
typedef union
{
    uint32_t u32;
    struct
    {
        uint32_t            M       : 1;
        uint32_t            W       : 1;
        uint32_t            reserved: 19;
        proc_perf_core_t    event   : 6;
        uint32_t            IE      : 1;
        uint32_t            U       : 1;
        uint32_t            S       : 1;
        uint32_t            K       : 1;
        uint32_t            EX      : 1;
    } s;
} proc_perf_core_control_t;

/**
 * Bit description of the L2 counters control register
 */
typedef union
{
    uint64_t u64;
    struct
    {
        uint64_t        reserved:32;
        uint64_t        cnt3ena : 1;
        uint64_t        cnt3clr : 1;
        proc_perf_l2_t  cnt3sel : 6;
        uint64_t        cnt2ena : 1;
        uint64_t        cnt2clr : 1;
        proc_perf_l2_t  cnt2sel : 6;
        uint64_t        cnt1ena : 1;
        uint64_t        cnt1clr : 1;
        proc_perf_l2_t  cnt1sel : 6;
        uint64_t        cnt0ena : 1;
        uint64_t        cnt0clr : 1;
        proc_perf_l2_t  cnt0sel : 6;
    } s;
} proc_perf_l2_control_t;

/**
 * Module parameters used to control the counters. Can be
 * changed on the fly through sysfs or ioctls.
 */
static char counter0[32] = "sissue";
static char counter1[32] = "dissue";
module_param_string(counter0, counter0, sizeof(counter0), 0666);
module_param_string(counter1, counter1, sizeof(counter1), 0666);

static char l2counter0[32] = "imiss";
static char l2counter1[32] = "ihit";
static char l2counter2[32] = "dmiss";
static char l2counter3[32] = "dhit";
module_param_string(l2counter0, l2counter0, sizeof(l2counter0), 0666);
module_param_string(l2counter1, l2counter1, sizeof(l2counter1), 0666);
module_param_string(l2counter2, l2counter2, sizeof(l2counter2), 0666);
module_param_string(l2counter3, l2counter3, sizeof(l2counter3), 0666);

static struct       proc_dir_entry *proc_perf_entry;
static uint64_t     proc_perf_counter_control[2];
static uint64_t     proc_perf_counter_data[NR_CPUS][2];
static uint64_t     proc_perf_l2counter_control[4];
static uint64_t     proc_perf_l2counter_data[4];
static const char * proc_perf_label[PROC_PERF_CORE_MAX];
static const char * proc_perf_l2label[PROC_PERF_L2_MAX];
static uint64_t     proc_perf_dram_clocks;
static uint64_t     proc_perf_dram_operations;


/**
 * Setup the core counters. Called on each core
 *
 * @param arg
 */
static void proc_perf_setup_counters(void *arg)
{
    proc_perf_core_control_t control;
    control.u32 = 0;
    control.s.event = proc_perf_counter_control[0];
    control.s.U = 1;
    control.s.S = 1;
    control.s.K = 1;
    control.s.EX = 1;
    __write_64bit_c0_register($25, 0, control.u32);

    control.s.event = proc_perf_counter_control[1];
    __write_64bit_c0_register($25, 2, control.u32);

    __write_64bit_c0_register($25, 1, 0);
    __write_64bit_c0_register($25, 3, 0);
}


/**
 * Update the counters for each core.
 *
 * @param arg
 */
static void proc_perf_update_counters(void *arg)
{
    int cpu = smp_processor_id();

    proc_perf_counter_data[cpu][0] = __read_64bit_c0_register($25, 1);
    proc_perf_counter_data[cpu][1] = __read_64bit_c0_register($25, 3);
    mb();
}


/**
 * Cleanup the input of sysfs
 *
 * @param str
 * @param len
 */
static inline void clean_string(char *str, int len)
{
    int i;
    for (i=0; i<len; i++)
        if (str[i] <= 32)
            str[i] = 0;
}


/**
 * Setup the counters using the current config
 */
static void proc_perf_setup(void)
{
    int i;
    proc_perf_l2_control_t l2control;

    proc_perf_counter_control[0] = 0;
    proc_perf_counter_control[1] = 0;
    proc_perf_l2counter_control[0] = 0;
    proc_perf_l2counter_control[1] = 0;
    proc_perf_l2counter_control[2] = 0;
    proc_perf_l2counter_control[3] = 0;

    /* Cleanup junk on end of param strings */
    clean_string(counter0, sizeof(counter0));
    clean_string(counter1, sizeof(counter1));
    clean_string(l2counter0, sizeof(l2counter0));
    clean_string(l2counter1, sizeof(l2counter1));
    clean_string(l2counter2, sizeof(l2counter2));
    clean_string(l2counter3, sizeof(l2counter3));

    /* Set the core counters to match the string parameters */
    for (i=0; i<PROC_PERF_CORE_MAX; i++)
    {
        if (proc_perf_label[i])
        {
            if (strcmp(proc_perf_label[i], counter0) == 0)
                proc_perf_counter_control[0] = i;
            if (strcmp(proc_perf_label[i], counter1) == 0)
                proc_perf_counter_control[1] = i;
        }
    }

    /* Set the L2 counters to match the string parameters */
    for (i=0; i<PROC_PERF_L2_MAX; i++)
    {
        if (proc_perf_l2label[i])
        {
            if (strcmp(proc_perf_l2label[i], l2counter0) == 0)
                proc_perf_l2counter_control[0] = i;
            if (strcmp(proc_perf_l2label[i], l2counter1) == 0)
                proc_perf_l2counter_control[1] = i;
            if (strcmp(proc_perf_l2label[i], l2counter2) == 0)
                proc_perf_l2counter_control[2] = i;
            if (strcmp(proc_perf_l2label[i], l2counter3) == 0)
                proc_perf_l2counter_control[3] = i;
        }
    }

    /* Update strings to match final config */
    strcpy(counter0, proc_perf_label[proc_perf_counter_control[0]]);
    strcpy(counter1, proc_perf_label[proc_perf_counter_control[1]]);
    strcpy(l2counter0, proc_perf_l2label[proc_perf_l2counter_control[0]]);
    strcpy(l2counter1, proc_perf_l2label[proc_perf_l2counter_control[1]]);
    strcpy(l2counter2, proc_perf_l2label[proc_perf_l2counter_control[2]]);
    strcpy(l2counter3, proc_perf_l2label[proc_perf_l2counter_control[3]]);

    on_each_cpu(proc_perf_setup_counters, NULL, 1, 1);

    l2control.u64 = 0;
    l2control.s.cnt3ena = 1;
    l2control.s.cnt3clr = 1;
    l2control.s.cnt3sel = proc_perf_l2counter_control[3];
    l2control.s.cnt2ena = 1;
    l2control.s.cnt2clr = 1;
    l2control.s.cnt2sel = proc_perf_l2counter_control[2];
    l2control.s.cnt1ena = 1;
    l2control.s.cnt1clr = 1;
    l2control.s.cnt1sel = proc_perf_l2counter_control[1];
    l2control.s.cnt0ena = 1;
    l2control.s.cnt0clr = 1;
    l2control.s.cnt0sel = proc_perf_l2counter_control[0];

    octeon_write_csr(OCTEON_L2C_PFCTL, l2control.u64);
}


static void proc_perf_update(void)
{
    on_each_cpu(proc_perf_update_counters, NULL, 1, 1);
    mb();
    proc_perf_l2counter_data[0] = octeon_read_csr(OCTEON_L2C_PFC0);
    proc_perf_l2counter_data[1] = octeon_read_csr(OCTEON_L2C_PFC1);
    proc_perf_l2counter_data[2] = octeon_read_csr(OCTEON_L2C_PFC2);
    proc_perf_l2counter_data[3] = octeon_read_csr(OCTEON_L2C_PFC3);
}


/**
 * Show the counters to the user
 *
 * @param m
 * @param v
 * @return
 */
static int proc_perf_show(struct seq_file *m, void *v)
{
    int cpu;
    int i;
    uint64_t dram_clocks;
    uint64_t dram_operations;

    proc_perf_update();

    seq_printf(m, "       %16s %16s\n",
               proc_perf_label[proc_perf_counter_control[0]],
               proc_perf_label[proc_perf_counter_control[1]]);
    for (cpu=0; cpu<NR_CPUS; cpu++)
    {
        if (cpu_online(cpu))
		    seq_printf(m, "CPU%2d: %16lu %16lu\n", cpu, proc_perf_counter_data[cpu][0], proc_perf_counter_data[cpu][1]);
    }

    seq_printf(m, "\n");
    for (i=0; i<4; i++)
        seq_printf(m, "%s: %lu\n", proc_perf_l2label[proc_perf_l2counter_control[i]], proc_perf_l2counter_data[i]);

    /* Compute DRAM utilization */
    dram_operations = (octeon_read_csr(OCTEON_LMC_OPS_CNT_HI) << 32) | octeon_read_csr(OCTEON_LMC_OPS_CNT_LO);
    dram_clocks = (octeon_read_csr(OCTEON_LMC_DCLK_CNT_HI) << 32) | octeon_read_csr(OCTEON_LMC_DCLK_CNT_LO);
    if (dram_clocks > proc_perf_dram_clocks)
    {
        uint64_t delta_clocks = dram_clocks - proc_perf_dram_clocks;
        uint64_t delta_operations = dram_operations - proc_perf_dram_operations;
        uint64_t percent_x100 = 10000 * delta_operations / delta_clocks;
        seq_printf(m, "\nDRAM ops count: %lu, dclk count: %lu, utilization: %lu.%02lu%%\n",
                   delta_operations, delta_clocks, percent_x100 / 100, percent_x100 % 100);
    }
    proc_perf_dram_operations = dram_operations;
    proc_perf_dram_clocks = dram_clocks;

    seq_printf(m,
               "\n"
               "Configuration of the performance counters is controller by writing\n"
               "one of the following values to:\n"
               "    /sys/module/perf_counters/parameters/counter{0,1}\n"
               "    /sys/module/perf_counters/parameters/l2counter{0-3}\n"
               "\n"
               "Possible CPU counters:");
    for (i=0; i<PROC_PERF_CORE_MAX; i++)
    {
        if ((i & 7) == 0)
            seq_printf(m, "\n    ");
        if (proc_perf_label[i])
            seq_printf(m, "%s ", proc_perf_label[i]);
    }

    seq_printf(m, "\n\nPossible L2 counters:");
    for (i=0; i<PROC_PERF_L2_MAX; i++)
    {
        if ((i & 3) == 0)
            seq_printf(m, "\n    ");
        if (proc_perf_l2label[i])
            seq_printf(m, "%s ", proc_perf_l2label[i]);
    }
    seq_printf(m, "\nWarning: Counter configuration doesn't update till you access /proc/octeon_perf.\n");

    proc_perf_setup();
    return 0;
}


/**
 * /proc/octeon_perf was openned. Use the single_open iterator
 *
 * @param inode
 * @param file
 * @return
 */
static int proc_perf_open(struct inode *inode, struct file *file)
{
	return single_open(file, proc_perf_show, NULL);
}


/**
 * IOCTL on /proc/octeon_perf
 *
 * @param inode
 * @param file
 * @param cmd
 * @param arg
 * @return
 */
static int proc_perf_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
    //printk("proc_perf_ioctl(cmd=0x%x(%u), arg=0x%lx)\n", cmd, cmd, arg);
    switch (cmd)
    {
        case PROC_PERF_IOCTL_SETUP_COUNTER0:
            if ((arg<=PROC_PERF_CORE_MAX) && proc_perf_label[arg])
            {
                strcpy(counter0, proc_perf_label[arg]);
                proc_perf_setup();
                return 0;
            }
            return -EINVAL;
        case PROC_PERF_IOCTL_SETUP_COUNTER1:
            if ((arg<=PROC_PERF_CORE_MAX) && proc_perf_label[arg])
            {
                strcpy(counter1, proc_perf_label[arg]);
                proc_perf_setup();
                return 0;
            }
            return -EINVAL;
        case PROC_PERF_IOCTL_SETUP_L2COUNTER0:
            if ((arg<=PROC_PERF_L2_MAX) && proc_perf_l2label[arg])
            {
                strcpy(l2counter0, proc_perf_l2label[arg]);
                proc_perf_setup();
                return 0;
            }
            return -EINVAL;
        case PROC_PERF_IOCTL_SETUP_L2COUNTER1:
            if ((arg<=PROC_PERF_L2_MAX) && proc_perf_l2label[arg])
            {
                strcpy(l2counter1, proc_perf_l2label[arg]);
                proc_perf_setup();
                return 0;
            }
            return -EINVAL;
        case PROC_PERF_IOCTL_SETUP_L2COUNTER2:
            if ((arg<=PROC_PERF_L2_MAX) && proc_perf_l2label[arg])
            {
                strcpy(l2counter2, proc_perf_l2label[arg]);
                proc_perf_setup();
                return 0;
            }
            return -EINVAL;
        case PROC_PERF_IOCTL_SETUP_L2COUNTER3:
            if ((arg<=PROC_PERF_L2_MAX) && proc_perf_l2label[arg])
            {
                strcpy(l2counter3, proc_perf_l2label[arg]);
                proc_perf_setup();
                return 0;
            }
            return -EINVAL;
        case PROC_PERF_IOCTL_READ_COUNTER0:
            proc_perf_update();
            copy_to_user((void*)arg, proc_perf_counter_data[smp_processor_id()] + 0, sizeof(long long));
            return 0;
        case PROC_PERF_IOCTL_READ_COUNTER1:
            proc_perf_update();
            copy_to_user((void*)arg, proc_perf_counter_data[smp_processor_id()] + 1, sizeof(long long));
            return 0;
        case PROC_PERF_IOCTL_READ_L2COUNTER0:
            proc_perf_update();
            copy_to_user((void*)arg, proc_perf_l2counter_data + 0, sizeof(long long));
            return 0;
        case PROC_PERF_IOCTL_READ_L2COUNTER1:
            proc_perf_update();
            copy_to_user((void*)arg, proc_perf_l2counter_data + 1, sizeof(long long));
            return 0;
        case PROC_PERF_IOCTL_READ_L2COUNTER2:
            proc_perf_update();
            copy_to_user((void*)arg, proc_perf_l2counter_data + 2, sizeof(long long));
            return 0;
        case PROC_PERF_IOCTL_READ_L2COUNTER3:
            proc_perf_update();
            copy_to_user((void*)arg, proc_perf_l2counter_data + 3, sizeof(long long));
            return 0;
        default:
            return -EINVAL;
    }
}


static struct file_operations proc_perf_operations = {
	.open		= proc_perf_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
    .ioctl      = proc_perf_ioctl,
};


/**
 * Module initialization
 *
 * @return
 */
static int __init proc_perf_init(void)
{
    printk("/proc/octeon_perf: Octeon performace counter interface loaded\n");

    memset(proc_perf_label, 0, sizeof(proc_perf_label));
    memset(proc_perf_l2label, 0, sizeof(proc_perf_l2label));

    proc_perf_label[PROC_PERF_CORE_NONE] = "none";
    proc_perf_label[PROC_PERF_CORE_CLK] = "clk";
    proc_perf_label[PROC_PERF_CORE_ISSUE] = "issue";
    proc_perf_label[PROC_PERF_CORE_RET] = "ret";
    proc_perf_label[PROC_PERF_CORE_NISSUE] = "nissue";
    proc_perf_label[PROC_PERF_CORE_SISSUE] = "sissue";
    proc_perf_label[PROC_PERF_CORE_DISSUE] = "dissue";
    proc_perf_label[PROC_PERF_CORE_IFI] = "ifi";
    proc_perf_label[PROC_PERF_CORE_BR] = "br";
    proc_perf_label[PROC_PERF_CORE_BRMIS] = "brmis";
    proc_perf_label[PROC_PERF_CORE_J] = "j";
    proc_perf_label[PROC_PERF_CORE_JMIS] = "jmis";
    proc_perf_label[PROC_PERF_CORE_REPLAY] = "replay";
    proc_perf_label[PROC_PERF_CORE_IUNA] = "iuna";
    proc_perf_label[PROC_PERF_CORE_TRAP] = "trap";
    proc_perf_label[PROC_PERF_CORE_UULOAD] = "uuload";
    proc_perf_label[PROC_PERF_CORE_UUSTORE] = "uustore";
    proc_perf_label[PROC_PERF_CORE_ULOAD] = "uload";
    proc_perf_label[PROC_PERF_CORE_USTORE] = "ustore";
    proc_perf_label[PROC_PERF_CORE_EC] = "ec";
    proc_perf_label[PROC_PERF_CORE_MC] = "mc";
    proc_perf_label[PROC_PERF_CORE_CC] = "cc";
    proc_perf_label[PROC_PERF_CORE_CSRC] = "csrc";
    proc_perf_label[PROC_PERF_CORE_CFETCH] = "cfetch";
    proc_perf_label[PROC_PERF_CORE_CPREF] = "cpref";
    proc_perf_label[PROC_PERF_CORE_ICA] = "ica";
    proc_perf_label[PROC_PERF_CORE_II] = "ii";
    proc_perf_label[PROC_PERF_CORE_IP] = "ip";
    proc_perf_label[PROC_PERF_CORE_CIMISS] = "cimiss";
    proc_perf_label[PROC_PERF_CORE_WBUF] = "wbuf";
    proc_perf_label[PROC_PERF_CORE_WDAT] = "wdat";
    proc_perf_label[PROC_PERF_CORE_WBUFLD] = "wbufld";
    proc_perf_label[PROC_PERF_CORE_WBUFFL] = "wbuffl";
    proc_perf_label[PROC_PERF_CORE_WBUFTR] = "wbuftr";
    proc_perf_label[PROC_PERF_CORE_BADD] = "badd";
    proc_perf_label[PROC_PERF_CORE_BADDL2] = "baddl2";
    proc_perf_label[PROC_PERF_CORE_BFILL] = "bfill";
    proc_perf_label[PROC_PERF_CORE_DDIDS] = "ddids";
    proc_perf_label[PROC_PERF_CORE_IDIDS] = "idids";
    proc_perf_label[PROC_PERF_CORE_DIDNA] = "didna";
    proc_perf_label[PROC_PERF_CORE_LDS] = "lds";
    proc_perf_label[PROC_PERF_CORE_LMLDS] = "lmlds";
    proc_perf_label[PROC_PERF_CORE_IOLDS] = "iolds";
    proc_perf_label[PROC_PERF_CORE_DMLDS] = "dmlds";
    proc_perf_label[PROC_PERF_CORE_STS] = "sts";
    proc_perf_label[PROC_PERF_CORE_LMSTS] = "lmsts";
    proc_perf_label[PROC_PERF_CORE_IOSTS] = "iosts";
    proc_perf_label[PROC_PERF_CORE_IOBDMA] = "iobdma";
    proc_perf_label[PROC_PERF_CORE_DTLB] = "dtlb";
    proc_perf_label[PROC_PERF_CORE_DTLBAD] = "dtlbad";
    proc_perf_label[PROC_PERF_CORE_ITLB] = "itlb";
    proc_perf_label[PROC_PERF_CORE_SYNC] = "sync";
    proc_perf_label[PROC_PERF_CORE_SYNCIOB] = "synciob";
    proc_perf_label[PROC_PERF_CORE_SYNCW] = "syncw";

    proc_perf_l2label[PROC_PERF_L2_CYCLES] = "cycles";
    proc_perf_l2label[PROC_PERF_L2_IMISS] = "imiss";
    proc_perf_l2label[PROC_PERF_L2_IHIT] = "ihit";
    proc_perf_l2label[PROC_PERF_L2_DMISS] = "dmiss";
    proc_perf_l2label[PROC_PERF_L2_DHIT] = "dhit";
    proc_perf_l2label[PROC_PERF_L2_MISS] = "miss";
    proc_perf_l2label[PROC_PERF_L2_HIT] = "hit";
    proc_perf_l2label[PROC_PERF_L2_VICTIM_BUFFER_HIT] = "victim-buffer-hit";
    proc_perf_l2label[PROC_PERF_L2_LFB_NQ_INDEX_CONFLICT] = "lfb-nq-index-conflict";
    proc_perf_l2label[PROC_PERF_L2_TAG_PROBE] = "tag-probe";
    proc_perf_l2label[PROC_PERF_L2_TAG_UPDATE] = "tag-update";
    proc_perf_l2label[PROC_PERF_L2_TAG_PROBE_COMPLETED] = "tag-probe-completed";
    proc_perf_l2label[PROC_PERF_L2_TAG_DIRTY_VICTIM] = "tag-dirty-victim";
    proc_perf_l2label[PROC_PERF_L2_DATA_STORE_NOP] = "data-store-nop";
    proc_perf_l2label[PROC_PERF_L2_DATA_STORE_READ] = "data-store-read";
    proc_perf_l2label[PROC_PERF_L2_DATA_STORE_WRITE] = "data-store-write";
    proc_perf_l2label[PROC_PERF_L2_MEMORY_FILL_DATA_VALID] = "memory-fill-data-valid";
    proc_perf_l2label[PROC_PERF_L2_MEMORY_WRITE_REQUEST] = "memory-write-request";
    proc_perf_l2label[PROC_PERF_L2_MEMORY_READ_REQUEST] = "memory-read-request";
    proc_perf_l2label[PROC_PERF_L2_MEMORY_WRITE_DATA_VALID] = "memory-write-data-valid";
    proc_perf_l2label[PROC_PERF_L2_XMC_NOP] = "xmc-nop";
    proc_perf_l2label[PROC_PERF_L2_XMC_LDT] = "xmc-ldt";
    proc_perf_l2label[PROC_PERF_L2_XMC_LDI] = "xmc-ldi";
    proc_perf_l2label[PROC_PERF_L2_XMC_LDD] = "xmc-ldd";
    proc_perf_l2label[PROC_PERF_L2_XMC_STF] = "xmc-stf";
    proc_perf_l2label[PROC_PERF_L2_XMC_STT] = "xmc-stt";
    proc_perf_l2label[PROC_PERF_L2_XMC_STP] = "xmc-stp";
    proc_perf_l2label[PROC_PERF_L2_XMC_STC] = "xmc-stc";
    proc_perf_l2label[PROC_PERF_L2_XMC_DWB] = "xmc-dwb";
    proc_perf_l2label[PROC_PERF_L2_XMC_PL2] = "xmc-pl2";
    proc_perf_l2label[PROC_PERF_L2_XMC_PSL1] = "xmc-psl1";
    proc_perf_l2label[PROC_PERF_L2_XMC_IOBLD] = "xmc-iobld";
    proc_perf_l2label[PROC_PERF_L2_XMC_IOBST] = "xmc-iobst";
    proc_perf_l2label[PROC_PERF_L2_XMC_IOBDMA] = "xmc-iobdma";
    proc_perf_l2label[PROC_PERF_L2_XMC_IOBRSP] = "xmc-iobrsp";
    proc_perf_l2label[PROC_PERF_L2_XMD_BUS_VALID] = "xmd-bus-valid";
    proc_perf_l2label[PROC_PERF_L2_XMD_BUS_VALID_DST_L2C] = "xmd-bus-valid-dst-l2c";
    proc_perf_l2label[PROC_PERF_L2_XMD_BUS_VALID_DST_IOB] = "xmd-bus-valid-dst-iob";
    proc_perf_l2label[PROC_PERF_L2_XMD_BUS_VALID_DST_PP] = "xmd-bus-valid-dst-pp";
    proc_perf_l2label[PROC_PERF_L2_RSC_NOP] = "rsc-nop";
    proc_perf_l2label[PROC_PERF_L2_RSC_STDN] = "rsc-stdn";
    proc_perf_l2label[PROC_PERF_L2_RSC_FILL] = "rsc-fill";
    proc_perf_l2label[PROC_PERF_L2_RSC_REFL] = "rsc-refl";
    proc_perf_l2label[PROC_PERF_L2_RSC_STIN] = "rsc-stin";
    proc_perf_l2label[PROC_PERF_L2_RSC_SCIN] = "rsc-scin";
    proc_perf_l2label[PROC_PERF_L2_RSC_SCFL] = "rsc-scfl";
    proc_perf_l2label[PROC_PERF_L2_RSC_SCDN] = "rsc-scdn";
    proc_perf_l2label[PROC_PERF_L2_RSD_DATA_VALID] = "rsd-data-valid";
    proc_perf_l2label[PROC_PERF_L2_RSD_DATA_VALID_FILL] = "rsd-data-valid-fill";
    proc_perf_l2label[PROC_PERF_L2_RSD_DATA_VALID_STRSP] = "rsd-data-valid-strsp";
    proc_perf_l2label[PROC_PERF_L2_RSD_DATA_VALID_REFL] = "rsd-data-valid-refl";
    proc_perf_l2label[PROC_PERF_L2_LRF_REQ] = "lrf-req";
    proc_perf_l2label[PROC_PERF_L2_DT_RD_ALLOC] = "dt-rd-alloc";
    proc_perf_l2label[PROC_PERF_L2_DT_WR_INVA] = "dt-wr-inva";

	proc_perf_entry = create_proc_entry("octeon_perf", 0, NULL);
	if (proc_perf_entry)
		proc_perf_entry->proc_fops = &proc_perf_operations;

    proc_perf_setup();
	return 0;
}


/**
 * Module cleanup
 *
 * @return
 */
static void __exit proc_perf_cleanup(void)
{
	if (proc_perf_entry)
        remove_proc_entry("octeon_perf", NULL);
}


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Cavium Networks <support@caviumnetworks.com>");
MODULE_DESCRIPTION("Cavium Networks Octeon performance counter interface.");
module_init(proc_perf_init);
module_exit(proc_perf_cleanup);

