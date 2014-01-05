/****************************************************************
 * Copyright (c) 2003-2005, Cavium Networks. All rights reserved.
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
 * Implementation of spinlocks.
 *
 * File version info: $Id: cvmx-spinlock.h 2 2007-04-05 08:51:12Z tt $ $Name$
 */


#ifndef __CVMX_SPINLOCK_H__
#define __CVMX_SPINLOCK_H__

#include "cvmx-asm.h"

#ifdef	__cplusplus
extern "C" {
#endif

/* Spinlocks for Octeon */


// define these to enable spinlock debugging/profiling
//#define CVMX_SPINLOCK_DEBUG
//#define CVMX_SPINLOCK_PROFILE


/**
 * Spinlocks for Octeon
 */
typedef struct {
    volatile uint32_t value;
#ifdef CVMX_SPINLOCK_PROFILE
    volatile uint64_t wait_cnt;
#endif
} cvmx_spinlock_t;

// note - macros not expanded in inline ASM, so values hardcoded
#define  CVMX_SPINLOCK_UNLOCKED_VAL  0
#define  CVMX_SPINLOCK_LOCKED_VAL    1


#ifdef CVMX_SPINLOCK_PROFILE
#define CVMX_SPINLOCK_UNLOCKED_INITIALIZER  {CVMX_SPINLOCK_UNLOCKED_VAL, 0}
#else
#define CVMX_SPINLOCK_UNLOCKED_INITIALIZER  {CVMX_SPINLOCK_UNLOCKED_VAL}
#endif


/**
 * Initialize a spinlock
 *
 * @param lock   Lock to initialize
 */
static inline void cvmx_spinlock_init(cvmx_spinlock_t *lock)
{
    lock->value = CVMX_SPINLOCK_UNLOCKED_VAL;
#ifdef CVMX_SPINLOCK_PROFILE
    lock->wait_cnt = 0;
#endif
}


/**
 * Return non-zero if the spinlock is currently locked
 *
 * @param lock   Lock to check
 * @return Non-zero if locked
 */
static inline int cvmx_spinlock_locked(cvmx_spinlock_t *lock)
{
    return (lock->value != CVMX_SPINLOCK_UNLOCKED_VAL);
}


/**
 * Releases lock
 *
 * @param lock   pointer to lock structure
 */
static inline void cvmx_spinlock_unlock(cvmx_spinlock_t *lock)
{
#if defined(__linux__)
    CVMX_SYNCWS;
#else
    CVMX_SYNCWS;
#endif
    lock->value = 0;
#if defined(__linux__)
    CVMX_SYNCWS;
#else
    CVMX_SYNCWS;
#endif
}


/**
 * Attempts to take the lock, but does not spin if lock is not available.
 * May take some time to acquire the lock even if it is available
 * due to the ll/sc not succeeding.
 *
 * @param lock   pointer to lock structure
 *
 * @return 0: lock successfully taken
 *         1: lock not taken, held by someone else
 * These return values match the Linux semantics.
 */

static inline unsigned int cvmx_spinlock_trylock(cvmx_spinlock_t *lock)
{
    unsigned int tmp;

    __asm__ __volatile__(
    ".set noreorder         \n"
    "1: ll   %[tmp], %[val] \n"
    "   bnez %[tmp], 2f     \n"  // if lock held, fail immediately
    "   li   %[tmp], 1      \n"
    "   sc   %[tmp], %[val] \n"
    "   beqz %[tmp], 1b     \n"
    "   li   %[tmp], 0      \n"
    "2:                     \n"
    ".set reorder           \n"
    : [val] "+m" (lock->value), [tmp] "=&r" (tmp)
    :
    : "memory");

    return (!!tmp);  /* normalize to 0 or 1 */
}

/**
 * Gets lock, spins until lock is taken
 *
 * @param lock   pointer to lock structure
 */
static inline void cvmx_spinlock_lock(cvmx_spinlock_t *lock)
{
    unsigned int tmp;
#ifdef CVMX_SPINLOCK_PROFILE
    volatile uint64_t count = 0;
#endif

    __asm__ __volatile__(
    ".set noreorder         \n"
    "1: ll   %[tmp], %[val]  \n"
    "   bnez %[tmp], 1b     \n"
#ifdef CVMX_SPINLOCK_PROFILE
    "   daddiu %[cnt], %[cnt], 1 \n"  // must be in delay slot, part of loop
#endif
    "   li   %[tmp], 1      \n"
    "   sc   %[tmp], %[val] \n"
    "   beqz %[tmp], 1b     \n"
    "   nop                \n"
    ".set reorder           \n"
    : [val] "+m" (lock->value), [tmp] "=&r" (tmp)
#ifdef CVMX_SPINLOCK_PROFILE
      , [cnt] "+&r" (count)
#endif
    :
    : "memory");
#ifdef CVMX_SPINLOCK_PROFILE
    lock->wait_cnt += count;
#endif

}



/** ********************************************************************
 * Bit spinlocks
 * These spinlocks use a single bit (bit 31) of a 32 bit word for locking.
 * The rest of the bits in the word are left undisturbed.  This enables more
 * compact data structures as only 1 bit is consumed for the lock.
 *
 */

/**
 * Gets lock, spins until lock is taken
 * Preserves the low 31 bits of the 32 bit
 * word used for the lock.
 *
 *
 * @param word  word to lock bit 31 of
 */
static inline void cvmx_spinlock_bit_lock(uint32_t *word)
{
    unsigned int tmp;
    unsigned int sav;

    __asm__ __volatile__(
    ".set noreorder         \n"
    ".set noat              \n"
    "1: ll    %[tmp], %[val]  \n"
    "   bbit1 %[tmp], 31, 1b    \n"
    "   li    $at, 1      \n"
    "   ins   %[tmp], $at, 31, 1  \n"
    "   sc    %[tmp], %[val] \n"
    "   beqz  %[tmp], 1b     \n"
    "   nop                \n"
    ".set at              \n"
    ".set reorder           \n"
    : [val] "+m" (*word), [tmp] "=&r" (tmp), [sav] "=&r" (sav)
    :
    : "memory");

}

/**
 * Attempts to get lock, returns immediately with success/failure
 * Preserves the low 31 bits of the 32 bit
 * word used for the lock.
 *
 *
 * @param word  word to lock bit 31 of
 * @return 0: lock successfully taken
 *         1: lock not taken, held by someone else
 * These return values match the Linux semantics.
 */
static inline unsigned int cvmx_spinlock_bit_trylock(uint32_t *word)
{
    unsigned int tmp;

    __asm__ __volatile__(
    ".set noreorder         \n"
    ".set noat              \n"
    "1: ll    %[tmp], %[val] \n"
    "   bbit1 %[tmp], 31, 2f     \n"  // if lock held, fail immediately
    "   li    $at, 1      \n"
    "   ins   %[tmp], $at, 31, 1  \n"
    "   sc    %[tmp], %[val] \n"
    "   beqz  %[tmp], 1b     \n"
    "   li    %[tmp], 0      \n"
    "2:                     \n"
    ".set at              \n"
    ".set reorder           \n"
    : [val] "+m" (*word), [tmp] "=&r" (tmp)
    :
    : "memory");

    return (!!tmp);  /* normalize to 0 or 1 */
}
/**
 * Releases bit lock
 *
 * Unconditionally clears bit 31 of the lock word.  Note that this is
 * done non-atomically, as this implementation assumes that the rest
 * of the bits in the word are protected by the lock.
 *
 * @param word  word to unlock bit 31 in
 */
static inline void cvmx_spinlock_bit_unlock(uint32_t *word)
{
#if defined(__linux__)
    CVMX_SYNCWS;
#else
    CVMX_SYNCWS;
#endif
    *word &= ~(1UL << 31) ;
#if defined(__linux__)
    CVMX_SYNCWS;
#else
    CVMX_SYNCWS;
#endif
}



/** ********************************************************************
 * Recursive spinlocks
 */
typedef struct {
	volatile unsigned int value;
	volatile unsigned int core_num;
#ifdef CVMX_SPINLOCK_PROFILE
    volatile uint64_t wait_cnt;
#endif
} cvmx_spinlock_rec_t;


/**
 * Initialize a recursive spinlock
 *
 * @param lock   Lock to initialize
 */
static inline void cvmx_spinlock_rec_init(cvmx_spinlock_rec_t *lock)
{
    lock->value = CVMX_SPINLOCK_UNLOCKED_VAL;
#ifdef CVMX_SPINLOCK_PROFILE
    lock->wait_cnt = 0;
#endif
}


/**
 * Return non-zero if the recursive spinlock is currently locked
 *
 * @param lock   Lock to check
 * @return Non-zero if locked
 */
static inline int cvmx_spinlock_rec_locked(cvmx_spinlock_rec_t *lock)
{
    return (lock->value != CVMX_SPINLOCK_UNLOCKED_VAL);
}


/**
* Unlocks one level of recursive spinlock.  Lock is not unlocked
* unless this is the final unlock call for that spinlock
*
* @param lock   ptr to recursive spinlock structure
*/
static inline void cvmx_spinlock_rec_unlock(cvmx_spinlock_rec_t *lock);

#ifdef CVMX_SPINLOCK_DEBUG
#define cvmx_spinlock_rec_unlock(x)  _int_cvmx_spinlock_rec_unlock((x), __FILE__, __LINE__)
static inline void _int_cvmx_spinlock_rec_unlock(cvmx_spinlock_rec_t *lock, char *filename, int linenum)
#else
static inline void cvmx_spinlock_rec_unlock(cvmx_spinlock_rec_t *lock)
#endif
{

	unsigned int temp, result;
    int core_num;
    core_num = cvmx_get_core_num();

#ifdef CVMX_SPINLOCK_DEBUG
    {
        if (lock->core_num != core_num)
        {
            cvmx_dprintf("ERROR: Recursive spinlock release attemped by non-owner! file: %s, line: %d\n", filename, linenum);
            return;
        }
    }
#endif

	__asm__ __volatile__(
		".set  noreorder                 \n"
		"     addi  %[tmp], %[pid], 0x80 \n"
		"     sw    %[tmp], %[lid]       # set lid to invalid value\n"
		"     sync                       \n"
		"1:   ll    %[tmp], %[val]       \n"
		"     addu  %[res], %[tmp], -1   # decrement lock count\n"
		"     sc    %[res], %[val]       \n"
		"     beqz  %[res], 1b           \n"
		"     nop                        \n"
		"     beq   %[tmp], %[res], 2f   # res is 1 on successful sc       \n"
		"     nop                        \n"
		"     sw   %[pid], %[lid]        # set lid to pid, only if lock still held\n"
		"2:   nop                        \n"
		"     syncw                       \n"
		".set  reorder                   \n"
		: [res] "=&r" (result), [tmp] "=&r" (temp), [val] "+m" (lock->value), [lid] "+m" (lock->core_num)
		: [pid] "r" (core_num)
		: "memory");


#ifdef CVMX_SPINLOCK_DEBUG
    {
        if (lock->value == ~0UL)
        {
            cvmx_dprintf("ERROR: Recursive spinlock released too many times! file: %s, line: %d\n", filename, linenum);
        }
    }
#endif


}

/**
 * Takes recursive spinlock for a given core.  A core can take the lock multiple
 * times, and the lock is released only when the corresponding number of
 * unlocks have taken place.
 *
 * NOTE: This assumes only one thread per core, and that the core ID is used as
 * the lock 'key'.  (This implementation cannot be generalized to allow
 * multiple threads to use the same key (core id) .)
 *
 * @param lock   address of recursive spinlock structure.  Note that this is
 *               distinct from the standard spinlock
 */
static inline void cvmx_spinlock_rec_lock(cvmx_spinlock_rec_t *lock);

#ifdef CVMX_SPINLOCK_DEBUG
#define cvmx_spinlock_rec_lock(x)  _int_cvmx_spinlock_rec_lock((x), __FILE__, __LINE__)
static inline void _int_cvmx_spinlock_rec_lock(cvmx_spinlock_rec_t *lock, char *filename, int linenum)
#else
static inline void cvmx_spinlock_rec_lock(cvmx_spinlock_rec_t *lock)
#endif
{


	volatile unsigned int tmp;
	volatile int core_num;
#ifdef CVMX_SPINLOCK_PROFILE
    volatile uint64_t count = 0;
#endif

	core_num = cvmx_get_core_num();


	__asm__ __volatile__(
		".set  noreorder              \n"
		"1: ll   %[tmp], %[val]       # load the count\n"
		"   bnez %[tmp], 2f           # if count!=zero branch to 2\n"
#ifdef CVMX_SPINLOCK_PROFILE
      "   daddiu %[cnt], %[cnt], 1 \n"  // must be in delay slot, part of loop
#endif
		"   addu %[tmp], %[tmp], 1    \n"
		"   sc   %[tmp], %[val]       \n"
		"   beqz %[tmp], 1b           # go back if not success\n"
		"   nop                       \n"
		"   j    3f                   # go to write core_num \n"
		"2: lw   %[tmp], %[lid]       # load the core_num \n"
		"   bne  %[tmp], %[pid], 1b   # core_num no match, restart\n"
		"   nop                       \n"
		"   lw   %[tmp], %[val]       \n"
		"   addu %[tmp], %[tmp], 1    \n"
		"   sw   %[tmp], %[val]       # update the count\n"
		"3: sw   %[pid], %[lid]       # store the core_num\n"
		"   sync                      \n"
		".set  reorder                \n"
		: [tmp] "=&r" (tmp), [val] "+m" (lock->value), [lid] "+m" (lock->core_num)
#ifdef CVMX_SPINLOCK_PROFILE
      , [cnt] "+&r" (count)
#endif
		: [pid] "r" (core_num)
		: "memory");

#ifdef CVMX_SPINLOCK_DEBUG
    if (lock->core_num != core_num)
    {
        cvmx_dprintf("cvmx_spinlock_rec_lock: lock taken, but core_num is incorrect. file: %s, line: %d\n", filename, linenum);
    }
#endif

#ifdef CVMX_SPINLOCK_PROFILE
    lock->wait_cnt += count;
#endif

}

#ifdef	__cplusplus
}
#endif

#endif /* __CVMX_SPINLOCK_H__ */
