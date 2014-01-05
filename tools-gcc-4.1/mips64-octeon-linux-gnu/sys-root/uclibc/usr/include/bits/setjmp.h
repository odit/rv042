/* Define the machine-dependent type `jmp_buf'.  MIPS version.
   Copyright (C) 1992,93,95,97,2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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

#ifndef _SETJMP_H
# error "Never include <bits/setjmp.h> directly; use <setjmp.h> instead."
#endif

#ifdef __GNUC__
/* Ensure that these are word-sized regardless of ABI.  */
typedef int __attribute__ ((mode (word))) __WORDint;
typedef void * __attribute__ ((mode (word))) __WORDptr;
#else
typedef int __WORDint;
typedef void * __WORDptr;
#endif

typedef struct
  {
    /* Program counter.  */
    __WORDptr __pc;

    /* Stack pointer.  */
    __WORDptr __sp;

    /* Callee-saved registers s0 through s7.  */
    __WORDint __regs[8];

    /* The frame pointer.  */
    __WORDptr __fp;

    /* The global pointer.  */
    __WORDptr __gp;

    /* Floating point status register.  */
    int __fpc_csr;

    /* Callee-saved floating point registers.  */
    double __fpregs[6];
  } __jmp_buf[1];

#ifdef __USE_MISC
/* Offset to the program counter in `jmp_buf'.  */
# define JB_PC	0
#endif


/* Test if longjmp to JMPBUF would unwind the frame
   containing a local variable at ADDRESS.  */
#define _JMPBUF_UNWINDS(jmpbuf, address) \
  ((void *) (address) < (jmpbuf)[0].__sp)
