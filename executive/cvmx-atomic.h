/****************************************************************
 * Copyright (c) 2005, Cavium Networks. All rights reserved.
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
 * This file provides atomic operations
 *
 * $Id: cvmx-atomic.h 2 2007-04-05 08:51:12Z tt $ $Name$
 *
 *
 */


#ifndef __CVMX_ATOMIC_H__
#define __CVMX_ATOMIC_H__

#ifdef	__cplusplus
extern "C" {
#endif


/**
 * Atomically adds a signed value to a 32 bit (aligned) memory location,
 * returns new value.
 *
 * This version does not perform 'sync' operations to enforce memory
 * operations.  This should only be used when there are no memory operation
 * ordering constraints.  (This should NOT be used for reference counting -
 * use the standard version instead.)
 *
 * @param ptr    address in memory to add incr to
 * @param incr   amount to increment memory location by (signed)
 *
 * @return Value of memory location after increment
 */
static inline int32_t cvmx_atomic_add32_nosync(int32_t *ptr, int32_t incr)
{
    uint32_t tmp, ret;

    __asm__ __volatile__(
    ".set noreorder         \n"
    "1: ll   %[tmp], %[val] \n"
    "   addu %[tmp], %[inc] \n"
    "   move %[ret], %[tmp] \n"
    "   sc   %[tmp], %[val] \n"
    "   beqz %[tmp], 1b     \n"
    "   nop                 \n"
    ".set reorder           \n"
    : [val] "+m" (*ptr), [tmp] "=&r" (tmp), [ret] "=&r" (ret)
    : [inc] "r" (incr)
    : "memory");

    return(ret);
}

/**
 * Atomically adds a signed value to a 32 bit (aligned) memory location,
 * returns new value.
 *
 * Memory access ordering is enforced before/after the atomic operation,
 * so no additional 'sync' instructions are required.
 *
 *
 * @param ptr    address in memory to add incr to
 * @param incr   amount to increment memory location by (signed)
 *
 * @return Value of memory location after increment
 */
static inline int32_t cvmx_atomic_add32(int32_t *ptr, int32_t incr)
{
    uint32_t ret;

    CVMX_SYNCWS;
    ret = cvmx_atomic_add32_nosync(ptr, incr);
    CVMX_SYNCWS;

    return ret;
}

/**
 * Atomically sets a 32 bit (aligned) memory location to a value
 *
 * @param ptr    address of memory to set
 * @param value  value to set memory location to.
 */
static inline void cvmx_atomic_set32(int32_t *ptr, int32_t value)
{
    uint32_t tmp;

    __asm__ __volatile__(
    ".set noreorder         \n"
    "1: ll   %[tmp], %[val] \n"
    "   move %[tmp], %[new_val] \n"
    "   sc   %[tmp], %[val] \n"
    "   beqz %[tmp], 1b     \n"
    "   nop                 \n"
    "   syncw               \n"
    ".set reorder           \n"
    : [val] "+m" (*ptr), [tmp] "=&r" (tmp)
    : [new_val] "r" (value)
    : "memory");
}

/**
 * Returns the current value of a 32 bit (aligned) memory
 * location.
 *
 * @param ptr    Address of memory to get
 * @return Value of the memory
 */
static inline int32_t cvmx_atomic_get32(int32_t *ptr)
{
    return *(volatile int32_t *)ptr;
}

/**
 * Atomically adds a signed value to a 64 bit (aligned) memory location,
 * returns new value.
 *
 * This version does not perform 'sync' operations to enforce memory
 * operations.  This should only be used when there are no memory operation
 * ordering constraints.  (This should NOT be used for reference counting -
 * use the standard version instead.)
 *
 * @param ptr    address in memory to add incr to
 * @param incr   amount to increment memory location by (signed)
 *
 * @return Value of memory location after increment
 */
static inline int64_t cvmx_atomic_add64_nosync(int64_t *ptr, int64_t incr)
{
    uint64_t tmp, ret;

    __asm__ __volatile__(
    ".set noreorder         \n"
    "1: lld  %[tmp], %[val] \n"
    "   daddu %[tmp], %[inc] \n"
    "   move %[ret], %[tmp] \n"
    "   scd  %[tmp], %[val] \n"
    "   beqz %[tmp], 1b     \n"
    "   nop                 \n"
    ".set reorder           \n"
    : [val] "+m" (*ptr), [tmp] "=&r" (tmp), [ret] "=&r" (ret)
    : [inc] "r" (incr)
    : "memory");

    return(ret);
}

/**
 * Atomically adds a signed value to a 64 bit (aligned) memory location,
 * returns new value.
 *
 * Memory access ordering is enforced before/after the atomic operation,
 * so no additional 'sync' instructions are required.
 *
 *
 * @param ptr    address in memory to add incr to
 * @param incr   amount to increment memory location by (signed)
 *
 * @return Value of memory location after increment
 */
static inline int64_t cvmx_atomic_add64(int64_t *ptr, int64_t incr)
{
    uint64_t ret;

    CVMX_SYNCWS;
    ret = cvmx_atomic_add64_nosync(ptr, incr);
    CVMX_SYNCWS;

    return ret;
}

/**
 * Atomically sets a 64 bit (aligned) memory location to a value
 *
 * @param ptr    address of memory to set
 * @param value  value to set memory location to.
 */
static inline void cvmx_atomic_set64(int64_t *ptr, int64_t value)
{
    uint64_t tmp;

    __asm__ __volatile__(
    ".set noreorder         \n"
    "1: lld  %[tmp], %[val] \n"
    "   move %[tmp], %[new_val] \n"
    "   scd  %[tmp], %[val] \n"
    "   beqz %[tmp], 1b     \n"
    "   nop                 \n"
    "   syncw               \n"
    ".set reorder           \n"
    : [val] "+m" (*ptr), [tmp] "=&r" (tmp)
    : [new_val] "r" (value)
    : "memory");
}

/**
 * Returns the current value of a 64 bit (aligned) memory
 * location.
 *
 * @param ptr    Address of memory to get
 * @return Value of the memory
 */
static inline int64_t cvmx_atomic_get64(int64_t *ptr)
{
    return *(volatile int64_t *)ptr;
}

/**
 * Atomically compares the old value with the value at ptr, and if they match,
 * stores new_val to ptr.
 * If *ptr and old don't match, function returns failure immediately.
 * If *ptr and old match, function spins until *ptr updated to new atomically, or
 *  until *ptr and old no longer match
 *
 * Does no memory synchronization.
 *
 * @return 1 on success (match and store)
 *         0 on no match
 */
static inline uint32_t cvmx_atomic_compare_and_store32_nosync(uint32_t *ptr, uint32_t old_val, uint32_t new_val)
{
    uint32_t tmp, ret;

    __asm__ __volatile__(
    ".set noreorder         \n"
    "1: ll   %[tmp], %[val] \n"
    "   li   %[ret], 0     \n"
    "   bne  %[tmp], %[old], 2f \n"
    "   move %[tmp], %[new_val] \n"
    "   sc   %[tmp], %[val] \n"
    "   beqz %[tmp], 1b     \n"
    "   li   %[ret], 1      \n"
    "2: nop               \n"
    ".set reorder           \n"
    : [val] "+m" (*ptr), [tmp] "=&r" (tmp), [ret] "=&r" (ret)
    : [old] "r" (old_val), [new_val] "r" (new_val)
    : "memory");

    return(ret);

}

/**
 * Atomically compares the old value with the value at ptr, and if they match,
 * stores new_val to ptr.
 * If *ptr and old don't match, function returns failure immediately.
 * If *ptr and old match, function spins until *ptr updated to new atomically, or
 *  until *ptr and old no longer match
 *
 * Does memory synchronization that is required to use this as a locking primitive.
 *
 * @return 1 on success (match and store)
 *         0 on no match
 */
static inline uint32_t cvmx_atomic_compare_and_store32(uint32_t *ptr, uint32_t old_val, uint32_t new_val)
{
    uint32_t ret;
    CVMX_SYNCWS;
    ret = cvmx_atomic_compare_and_store32_nosync(ptr, old_val, new_val);
    CVMX_SYNCWS;
    return ret;


}

/**
 * Atomically compares the old value with the value at ptr, and if they match,
 * stores new_val to ptr.
 * If *ptr and old don't match, function returns failure immediately.
 * If *ptr and old match, function spins until *ptr updated to new atomically, or
 *  until *ptr and old no longer match
 *
 * Does no memory synchronization.
 *
 * @return 1 on success (match and store)
 *         0 on no match
 */
static inline uint64_t cvmx_atomic_compare_and_store64_nosync(uint64_t *ptr, uint64_t old_val, uint64_t new_val)
{
    uint64_t tmp, ret;

    __asm__ __volatile__(
    ".set noreorder         \n"
    "1: lld  %[tmp], %[val] \n"
    "   li   %[ret], 0     \n"
    "   bne  %[tmp], %[old], 2f \n"
    "   move %[tmp], %[new_val] \n"
    "   scd  %[tmp], %[val] \n"
    "   beqz %[tmp], 1b     \n"
    "   li   %[ret], 1      \n"
    "2: nop               \n"
    ".set reorder           \n"
    : [val] "+m" (*ptr), [tmp] "=&r" (tmp), [ret] "=&r" (ret)
    : [old] "r" (old_val), [new_val] "r" (new_val)
    : "memory");

    return(ret);

}

/**
 * Atomically compares the old value with the value at ptr, and if they match,
 * stores new_val to ptr.
 * If *ptr and old don't match, function returns failure immediately.
 * If *ptr and old match, function spins until *ptr updated to new atomically, or
 *  until *ptr and old no longer match
 *
 * Does memory synchronization that is required to use this as a locking primitive.
 *
 * @return 1 on success (match and store)
 *         0 on no match
 */
static inline uint64_t cvmx_atomic_compare_and_store64(uint64_t *ptr, uint64_t old_val, uint64_t new_val)
{
    uint64_t ret;
    CVMX_SYNCWS;
    ret = cvmx_atomic_compare_and_store64_nosync(ptr, old_val, new_val);
    CVMX_SYNCWS;
    return ret;


}

/**
 * Atomically adds a signed value to a 64 bit (aligned) memory location,
 * and returns previous value.
 *
 * This version does not perform 'sync' operations to enforce memory
 * operations.  This should only be used when there are no memory operation
 * ordering constraints.  (This should NOT be used for reference counting -
 * use the standard version instead.)
 *
 * @param ptr    address in memory to add incr to
 * @param incr   amount to increment memory location by (signed)
 *
 * @return Value of memory location before increment
 */
static inline int64_t cvmx_atomic_fetch_and_add64_nosync(int64_t *ptr, int64_t incr)
{
    uint64_t tmp, ret;

    __asm__ __volatile__(
    ".set noreorder          \n"
    "1: lld   %[tmp], %[val] \n"
    "   move  %[ret], %[tmp] \n"
    "   daddu %[tmp], %[inc] \n"
    "   scd   %[tmp], %[val] \n"
    "   beqz  %[tmp], 1b     \n"
    "   nop                  \n"
    ".set reorder            \n"
    : [val] "+m" (*ptr), [tmp] "=&r" (tmp), [ret] "=&r" (ret)
    : [inc] "r" (incr)
    : "memory");

    return (ret);
}

/**
 * Atomically adds a signed value to a 32 bit (aligned) memory location,
 * and returns previous value.
 *
 * This version does not perform 'sync' operations to enforce memory
 * operations.  This should only be used when there are no memory operation
 * ordering constraints.  (This should NOT be used for reference counting -
 * use the standard version instead.)
 *
 * @param ptr    address in memory to add incr to
 * @param incr   amount to increment memory location by (signed)
 *
 * @return Value of memory location before increment
 */
static inline int32_t cvmx_atomic_fetch_and_add32_nosync(int32_t *ptr, int32_t incr)
{
    uint32_t tmp, ret;

    __asm__ __volatile__(
    ".set noreorder         \n"
    "1: ll   %[tmp], %[val] \n"
    "   move %[ret], %[tmp] \n"
    "   addu %[tmp], %[inc] \n"
    "   sc   %[tmp], %[val] \n"
    "   beqz %[tmp], 1b     \n"
    "   nop                 \n"
    ".set reorder           \n"
    : [val] "+m" (*ptr), [tmp] "=&r" (tmp), [ret] "=&r" (ret)
    : [inc] "r" (incr)
    : "memory");

    return (ret);
}

/**
 * Atomically set bits in a 64 bit (aligned) memory location,
 * and returns previous value.
 *
 * This version does not perform 'sync' operations to enforce memory
 * operations.  This should only be used when there are no memory operation
 * ordering constraints.
 *
 * @param ptr    address in memory
 * @param mask   mask of bits to set
 *
 * @return Value of memory location before setting bits
 */
static inline uint64_t cvmx_atomic_fetch_and_bset64_nosync(uint64_t *ptr, uint64_t mask)
{
    uint64_t tmp, ret;

    __asm__ __volatile__(
    ".set noreorder         \n"
    "1: lld  %[tmp], %[val] \n"
    "   move %[ret], %[tmp] \n"
    "   or   %[tmp], %[msk] \n"
    "   scd  %[tmp], %[val] \n"
    "   beqz %[tmp], 1b     \n"
    "   nop                 \n"
    ".set reorder           \n"
    : [val] "+m" (*ptr), [tmp] "=&r" (tmp), [ret] "=&r" (ret)
    : [msk] "r" (mask)
    : "memory");

    return (ret);
}

/**
 * Atomically set bits in a 32 bit (aligned) memory location,
 * and returns previous value.
 *
 * This version does not perform 'sync' operations to enforce memory
 * operations.  This should only be used when there are no memory operation
 * ordering constraints.
 *
 * @param ptr    address in memory
 * @param mask   mask of bits to set
 *
 * @return Value of memory location before setting bits
 */
static inline uint32_t cvmx_atomic_fetch_and_bset32_nosync(uint32_t *ptr, uint32_t mask)
{
    uint32_t tmp, ret;

    __asm__ __volatile__(
    ".set noreorder         \n"
    "1: ll   %[tmp], %[val] \n"
    "   move %[ret], %[tmp] \n"
    "   or   %[tmp], %[msk] \n"
    "   sc   %[tmp], %[val] \n"
    "   beqz %[tmp], 1b     \n"
    "   nop                 \n"
    ".set reorder           \n"
    : [val] "+m" (*ptr), [tmp] "=&r" (tmp), [ret] "=&r" (ret)
    : [msk] "r" (mask)
    : "memory");

    return (ret);
}

/**
 * Atomically clear bits in a 64 bit (aligned) memory location,
 * and returns previous value.
 *
 * This version does not perform 'sync' operations to enforce memory
 * operations.  This should only be used when there are no memory operation
 * ordering constraints.  
 *
 * @param ptr    address in memory
 * @param mask   mask of bits to clear
 *
 * @return Value of memory location before clearing bits
 */
static inline uint64_t cvmx_atomic_fetch_and_bclr64_nosync(uint64_t *ptr, uint64_t mask)
{
    uint64_t tmp, ret;

    __asm__ __volatile__(
    ".set noreorder         \n"
    "   nor  %[msk], 0      \n"
    "1: lld  %[tmp], %[val] \n"
    "   move %[ret], %[tmp] \n"
    "   and  %[tmp], %[msk] \n"
    "   scd  %[tmp], %[val] \n"
    "   beqz %[tmp], 1b     \n"
    "   nop                 \n"
    ".set reorder           \n"
    : [val] "+m" (*ptr), [tmp] "=&r" (tmp), [ret] "=&r" (ret)
    : [msk] "r" (mask)
    : "memory");

    return (ret);
}

/**
 * Atomically clear bits in a 32 bit (aligned) memory location,
 * and returns previous value.
 *
 * This version does not perform 'sync' operations to enforce memory
 * operations.  This should only be used when there are no memory operation
 * ordering constraints.  
 *
 * @param ptr    address in memory
 * @param mask   mask of bits to clear
 *
 * @return Value of memory location before clearing bits
 */
static inline uint32_t cvmx_atomic_fetch_and_bclr32_nosync(uint32_t *ptr, uint32_t mask)
{
    uint32_t tmp, ret;

    __asm__ __volatile__(
    ".set noreorder         \n"
    "   nor  %[msk], 0      \n"
    "1: ll   %[tmp], %[val] \n"
    "   move %[ret], %[tmp] \n"
    "   and  %[tmp], %[msk] \n"
    "   sc   %[tmp], %[val] \n"
    "   beqz %[tmp], 1b     \n"
    "   nop                 \n"
    ".set reorder           \n"
    : [val] "+m" (*ptr), [tmp] "=&r" (tmp), [ret] "=&r" (ret)
    : [msk] "r" (mask)
    : "memory");

    return (ret);
}

/**
 * Atomically swaps value in 64 bit (aligned) memory location, 
 * and returns previous value.
 *
 * This version does not perform 'sync' operations to enforce memory
 * operations.  This should only be used when there are no memory operation
 * ordering constraints.
 *
 * @param ptr       address in memory
 * @param new_val   new value to write
 *
 * @return Value of memory location before swap operation
 */
static inline uint64_t cvmx_atomic_swap64_nosync(uint64_t *ptr, uint64_t new_val)
{
    uint64_t tmp, ret;

    __asm__ __volatile__(
    ".set noreorder         \n"
    "1: lld  %[ret], %[val] \n"
    "   move %[tmp], %[new_val] \n"
    "   scd  %[tmp], %[val] \n"
    "   beqz %[tmp],  1b    \n"
    "   nop                 \n"
    ".set reorder           \n"
    : [val] "+m" (*ptr), [tmp] "=&r" (tmp), [ret] "=&r" (ret)
    : [new_val] "r"  (new_val)
    : "memory"); 

    return (ret);
}

/**
 * Atomically swaps value in 32 bit (aligned) memory location, 
 * and returns previous value.
 *
 * This version does not perform 'sync' operations to enforce memory
 * operations.  This should only be used when there are no memory operation
 * ordering constraints.
 *
 * @param ptr       address in memory
 * @param new_val   new value to write
 *
 * @return Value of memory location before swap operation
 */
static inline uint32_t cvmx_atomic_swap32_nosync(uint32_t *ptr, uint32_t new_val)
{
    uint32_t tmp, ret;

    __asm__ __volatile__(
    ".set noreorder         \n"
    "1: ll   %[ret], %[val] \n"
    "   move %[tmp], %[new_val] \n"
    "   sc   %[tmp], %[val] \n"
    "   beqz %[tmp],  1b    \n"
    "   nop                 \n"
    ".set reorder           \n"
    : [val] "+m" (*ptr), [tmp] "=&r" (tmp), [ret] "=&r" (ret)
    : [new_val] "r"  (new_val)
    : "memory"); 

    return (ret);
}

/** 
 * This atomic operation is now named cvmx_atomic_compare_and_store32_nosync
 * and the (deprecated) macro is provided for backward compatibility.  
 * @deprecated 
 */
#define cvmx_atomic_compare_and_store_nosync32 cvmx_atomic_compare_and_store32_nosync

/** 
 * This atomic operation is now named cvmx_atomic_compare_and_store64_nosync
 * and the (deprecated) macro is provided for backward compatibility.  
 * @deprecated 
 */
#define cvmx_atomic_compare_and_store_nosync64 cvmx_atomic_compare_and_store64_nosync



#ifdef	__cplusplus
}
#endif

#endif /* __CVMX_ATOMIC_H__ */
