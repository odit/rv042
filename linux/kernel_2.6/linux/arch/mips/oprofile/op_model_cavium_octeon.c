/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2004 by Ralf Baechle
 */
#include <linux/oprofile.h>
#include <linux/interrupt.h>
#include <linux/smp.h>

#include "op_impl.h"

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
        uint32_t            event   : 6;
        uint32_t            IE      : 1;
        uint32_t            U       : 1;
        uint32_t            S       : 1;
        uint32_t            K       : 1;
        uint32_t            EX      : 1;
    } s;
} core_control_t;

static struct
{
    core_control_t  control[2];
    uint64_t        reset_value[2];
} octeon_config;

/* Compute all of the registers in preparation for enabling profiling.  */

static void octeon_reg_setup(struct op_counter_config *ctr)
{
    int i;
    for (i=0; i<2; i++)
    {
        octeon_config.control[i].u32 = 0;
        if (ctr[i].enabled)
        {
            octeon_config.control[i].s.event = ctr[i].event;
            octeon_config.control[i].s.IE = 1;
            octeon_config.control[i].s.U = ctr[i].user;
            octeon_config.control[i].s.S = 1;
            octeon_config.control[i].s.K = ctr[i].kernel;
            octeon_config.control[i].s.EX = ctr[i].exl;
            octeon_config.reset_value[i] = (1ull<<63) - ctr[i].count;
        }
    }
}

/* Program all of the registers in preparation for enabling profiling.  */

static void octeon_cpu_setup (void *args)
{
    __write_64bit_c0_register($25, 1, octeon_config.reset_value[0]);
    __write_64bit_c0_register($25, 3, octeon_config.reset_value[1]);
}

static void octeon_cpu_start(void *args)
{
    /* Start all counters on current CPU */
    __write_64bit_c0_register($25, 0, octeon_config.control[0].u32);
    __write_64bit_c0_register($25, 2, octeon_config.control[1].u32);
}

static void octeon_cpu_stop(void *args)
{
    /* Stop all counters on current CPU */
    __write_64bit_c0_register($25, 0, 0);
    __write_64bit_c0_register($25, 2, 0);
}

static irqreturn_t octeon_perfcount_handler(int irq, void * dev_id,
                                            struct pt_regs *regs)
{
    uint64_t counter;

    counter = __read_64bit_c0_register($25, 1);
    if (counter & (1ull<<63))
    {
        oprofile_add_sample(regs, 0);
        __write_64bit_c0_register($25, 1, octeon_config.reset_value[0]);
    }

    counter = __read_64bit_c0_register($25, 3);
    if (counter & (1ull<<63))
    {
        oprofile_add_sample(regs, 1);
        __write_64bit_c0_register($25, 3, octeon_config.reset_value[1]);
    }

    return IRQ_HANDLED;
}

static int octeon_init(void)
{
    return request_irq(7, octeon_perfcount_handler, SA_SHIRQ, "Perfcounter", octeon_perfcount_handler);
}

static void octeon_exit(void)
{
    free_irq(7, octeon_perfcount_handler);
}

struct op_mips_model op_model_octeon = {
    .reg_setup	= octeon_reg_setup,
    .cpu_setup	= octeon_cpu_setup,
    .init		= octeon_init,
    .exit		= octeon_exit,
    .cpu_start	= octeon_cpu_start,
    .cpu_stop	= octeon_cpu_stop,
    .cpu_type	= "mips/octeon",
    .num_counters	= 2
};
