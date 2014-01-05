/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2004, 2005 Cavium Networks
 */


/**
 * Octeon Simulator magic printf interface. Generates printf
 * style output on the simulator console. Format arguments
 * must all be %ll?. Nothing other than 64bit numbers can 
 * be displayed.
 * 
 * @param fmt    Format string
 */
void octeon_simprintf(const char *fmt, ...)
{
    asm volatile ("\
        sync\n\
        add $25, $0, 6\n\
        dla $15,0x8000000feffe0000\n\
        dadd $24, $31, $0\n\
        jalr $15\n\
        dadd $31, $24, $0\n\
        " : : );
}

