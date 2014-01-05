/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2005 Cavium Networks, Inc.
 */
#include <linux/config.h>
#include <linux/module.h>


void clear_page(void *page)
{
	void *end = page + PAGE_SIZE;

    asm volatile (
        "   .set	push		\n"
        "   .set	mips64		\n"
        "   .set	noreorder	\n"
        "1: pref    30, 0(%0)   \n" /* Prepare cache line for store */
        "   sd      $0, 0(%0)   \n" /* Write zeros to the cache line */
        "   sd      $0, 8(%0)   \n"
        "   sd      $0, 16(%0)  \n"
        "   sd      $0, 24(%0)  \n"
        "   sd      $0, 32(%0)  \n"
        "   sd      $0, 40(%0)  \n"
        "   sd      $0, 48(%0)  \n"
        "   sd      $0, 56(%0)  \n"
        "   sd      $0, 64(%0)  \n"
        "   sd      $0, 72(%0)  \n"
        "   sd      $0, 80(%0)  \n"
        "   sd      $0, 88(%0)  \n"
        "   sd      $0, 96(%0)  \n"
        "   sd      $0, 104(%0) \n"
        "   daddu   %0, 128     \n" /* Increment to the next address. Will be dual issued */
        "   sd      $0, -16(%0) \n"
        "   blt     %0, %1, 1b  \n" /* Loop until we've completed the page */
        "    sd     $0, -8(%0)  \n"
        "   .set	pop		\n"
        : "+r" (page)
        : "r" (end)
        : "memory"
    );
}


void copy_page(void *to, void *from)
{
	void *end = to + PAGE_SIZE;

    asm volatile (
        "   .set	push		\n"
        "   .set	mips64		\n"
        "   .set	noreorder	\n"
        "   pref    0,  0(%1)   \n" /* Prefetch the first cache line of "from" */
        "   pref    0,  128(%1) \n" /* Prefetch the next "from" cache line for the next iteration */
        "1: pref    0,  256(%1) \n" /* Prefetch for two loops ahead */
        "   pref    30, 0(%0)   \n" /* Prepare the "to" cache line for store */
        "   ld      $12, 0(%1)  \n" /* Copy 32 bytes at a time */
        "   ld      $13, 8(%1)  \n"
        "   ld      $14, 16(%1) \n"
        "   ld      $15, 24(%1) \n"
        "   daddu   %1, 32      \n" /* Dual issued */
        "   sd      $12, 0(%0)  \n"
        "   sd      $13, 8(%0)  \n"
        "   sd      $14, 16(%0) \n"
        "   sd      $15, 24(%0) \n"
        "   daddu   %0, 32      \n" /* Dual issued */
        "   ld      $12, 0(%1)  \n" /* Copy 32 bytes at a time */
        "   ld      $13, 8(%1)  \n"
        "   ld      $14, 16(%1) \n"
        "   ld      $15, 24(%1) \n"
        "   daddu   %1, 32      \n" /* Dual issued */
        "   sd      $12, 0(%0)  \n"
        "   sd      $13, 8(%0)  \n"
        "   sd      $14, 16(%0) \n"
        "   sd      $15, 24(%0) \n"
        "   daddu   %0, 32      \n" /* Dual issued */
        "   ld      $12, 0(%1)  \n" /* Copy 32 bytes at a time */
        "   ld      $13, 8(%1)  \n"
        "   ld      $14, 16(%1) \n"
        "   ld      $15, 24(%1) \n"
        "   daddu   %1, 32      \n" /* Dual issued */
        "   sd      $12, 0(%0)  \n"
        "   sd      $13, 8(%0)  \n"
        "   sd      $14, 16(%0) \n"
        "   sd      $15, 24(%0) \n"
        "   daddu   %0, 32      \n" /* Dual issued */
        "   ld      $12, 0(%1)  \n" /* Copy 32 bytes at a time */
        "   ld      $13, 8(%1)  \n"
        "   ld      $14, 16(%1) \n"
        "   ld      $15, 24(%1) \n"
        "   daddu   %1, 32      \n" /* Dual issued */
        "   sd      $12, 0(%0)  \n"
        "   sd      $13, 8(%0)  \n"
        "   daddu   %0, 32      \n" /* Dual issued */
        "   sd      $14, -16(%0)\n"
        "   blt     %0, %2, 1b  \n" /* Loop until we've completed the page */
        "    sd     $15, -8(%0) \n"
        "   .set	pop		\n"
        : "+r" (to), "+r" (from)
        : "r" (end)
        : "$12", "$13", "$14", "$15", "memory"
    );
}

EXPORT_SYMBOL(clear_page);
EXPORT_SYMBOL(copy_page);
