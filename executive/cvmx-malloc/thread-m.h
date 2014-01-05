/* Basic platform-independent macro definitions for mutexes and
   thread-specific data.
   Copyright (C) 1996, 1997, 1998, 2000, 2001 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Wolfram Gloger <wg@malloc.de>, 2001.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

/* $Id: thread-m.h 2 2007-04-05 08:51:12Z tt $
   One out of _LIBC, USE_PTHREADS, USE_THR or USE_SPROC should be
   defined, otherwise the token NO_THREADS and dummy implementations
   of the macros will be defined.  */

#ifndef _THREAD_M_H
#define _THREAD_M_H

#undef thread_atfork_static


#undef NO_THREADS /* No threads, provide dummy macros */

typedef int thread_id;

/* The mutex functions used to do absolutely nothing, i.e. lock,
   trylock and unlock would always just return 0.  However, even
   without any concurrently active threads, a mutex can be used
   legitimately as an `in use' flag.  To make the code that is
   protected by a mutex async-signal safe, these macros would have to
   be based on atomic test-and-set operations, for example. */
#ifdef __OCTEON__
typedef cvmx_spinlock_t mutex_t;
#define MUTEX_INITIALIZER          CMVX_SPINLOCK_UNLOCKED_VAL
#define mutex_init(m)              cvmx_spinlock_init(m)
#define mutex_lock(m)              cvmx_spinlock_lock(m)
#define mutex_trylock(m)           (cvmx_spinlock_trylock(m))
#define mutex_unlock(m)            cvmx_spinlock_unlock(m)
#else

typedef int mutex_t;

#define MUTEX_INITIALIZER          0
#define mutex_init(m)              (*(m) = 0)
#define mutex_lock(m)              ((*(m) = 1), 0)
#define mutex_trylock(m)           (*(m) ? 1 : ((*(m) = 1), 0))
#define mutex_unlock(m)            (*(m) = 0)
#endif



typedef void *tsd_key_t;
#define tsd_key_create(key, destr) do {} while(0)
#define tsd_setspecific(key, data) ((key) = (data))
#define tsd_getspecific(key, vptr) (vptr = (key))

#define thread_atfork(prepare, parent, child) do {} while(0)


#endif /* !defined(_THREAD_M_H) */
