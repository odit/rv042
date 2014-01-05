/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2006 Cavium Networks
 */
#ifndef __CAVIUM_PCI_BAR_SETUP_H
#define __CAVIUM_PCI_BAR_SETUP_H

#define OCTEON_PCI_BAR1_HOLE_BITS 5
#define OCTEON_PCI_BAR1_HOLE_SIZE (1ul<<(OCTEON_PCI_BAR1_HOLE_BITS+3))

/**
 * This is a flag to tell the DMA mapping system in dma-octeon.c
 * to use the big bar options for BAR0 and BAR1. This optimizes
 * memory references by not using the scarse BAR1 movable entries.
 */
extern int octeon_dma_use_big_bar;

#endif
