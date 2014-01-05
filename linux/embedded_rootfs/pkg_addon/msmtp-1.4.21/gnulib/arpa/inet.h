/* DO NOT EDIT! GENERATED AUTOMATICALLY! */
/* A GNU-like <arpa/inet.h>.

   Copyright (C) 2005-2006, 2008-2010 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.  */

#ifndef _GL_ARPA_INET_H

#if __GNUC__ >= 3
#pragma GCC system_header
#endif

/* Gnulib's sys/socket.h is responsible for pulling in winsock2.h etc
   under MinGW.
   But avoid namespace pollution on glibc systems.  */
#ifndef __GLIBC__
# include <sys/socket.h>
#endif

#if 1

/* The include_next requires a split double-inclusion guard.  */
# include_next <arpa/inet.h>

#endif

#ifndef _GL_ARPA_INET_H
#define _GL_ARPA_INET_H

/* The definition of _GL_ARG_NONNULL is copied here.  */
/* _GL_ARG_NONNULL((n,...,m)) tells the compiler and static analyzer tools
   that the values passed as arguments n, ..., m must be non-NULL pointers.
   n = 1 stands for the first argument, n = 2 for the second argument etc.  */
#ifndef _GL_ARG_NONNULL
# if (__GNUC__ == 3 && __GNUC_MINOR__ >= 3) || __GNUC__ > 3
#  define _GL_ARG_NONNULL(params) __attribute__ ((__nonnull__ params))
# else
#  define _GL_ARG_NONNULL(params)
# endif
#endif

/* The definition of _GL_WARN_ON_USE is copied here.  */
#ifndef _GL_WARN_ON_USE

# if 4 < __GNUC__ || (__GNUC__ == 4 && 3 <= __GNUC_MINOR__)
/* A compiler attribute is available in gcc versions 4.3.0 and later.  */
#  define _GL_WARN_ON_USE(function, message) \
extern __typeof__ (function) function __attribute__ ((__warning__ (message)))
# elif __GNUC__ >= 3 && GNULIB_STRICT_CHECKING
/* Verify the existence of the function.  */
#  define _GL_WARN_ON_USE(function, message) \
extern __typeof__ (function) function
# else /* Unsupported.  */
#  define _GL_WARN_ON_USE(function, message) \
_GL_WARN_EXTERN_C int _gl_warn_on_use
# endif
#endif

/* _GL_WARN_ON_USE_CXX (function, rettype, parameters_and_attributes, "string")
   is like _GL_WARN_ON_USE (function, "string"), except that the function is
   declared with the given prototype, consisting of return type, parameters,
   and attributes.
   This variant is useful for overloaded functions in C++. _GL_WARN_ON_USE does
   not work in this case.  */
#ifndef _GL_WARN_ON_USE_CXX
# if 4 < __GNUC__ || (__GNUC__ == 4 && 3 <= __GNUC_MINOR__)
#  define _GL_WARN_ON_USE_CXX(function,rettype,parameters_and_attributes,msg) \
extern rettype function parameters_and_attributes \
     __attribute__ ((__warning__ (msg)))
# elif __GNUC__ >= 3 && GNULIB_STRICT_CHECKING
/* Verify the existence of the function.  */
#  define _GL_WARN_ON_USE_CXX(function,rettype,parameters_and_attributes,msg) \
extern rettype function parameters_and_attributes
# else /* Unsupported.  */
#  define _GL_WARN_ON_USE_CXX(function,rettype,parameters_and_attributes,msg) \
_GL_WARN_EXTERN_C int _gl_warn_on_use
# endif
#endif

/* _GL_WARN_EXTERN_C declaration;
   performs the declaration with C linkage.  */
#ifndef _GL_WARN_EXTERN_C
# if defined __cplusplus
#  define _GL_WARN_EXTERN_C extern "C"
# else
#  define _GL_WARN_EXTERN_C extern
# endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if 1
# if !1
/* Converts an internet address from internal format to a printable,
   presentable format.
   AF is an internet address family, such as AF_INET or AF_INET6.
   SRC points to a 'struct in_addr' (for AF_INET) or 'struct in6_addr'
   (for AF_INET6).
   DST points to a buffer having room for CNT bytes.
   The printable representation of the address (in numeric form, not
   surrounded by [...], no reverse DNS is done) is placed in DST, and
   DST is returned.  If an error occurs, the return value is NULL and
   errno is set.  If CNT bytes are not sufficient to hold the result,
   the return value is NULL and errno is set to ENOSPC.  A good value
   for CNT is 46.

   For more details, see the POSIX:2001 specification
   <http://www.opengroup.org/susv3xsh/inet_ntop.html>.  */
extern const char *inet_ntop (int af, const void *restrict src,
                              char *restrict dst, socklen_t cnt)
     _GL_ARG_NONNULL ((2, 3));
# endif
#elif defined GNULIB_POSIXCHECK
# undef inet_ntop
# if HAVE_RAW_DECL_INET_NTOP
_GL_WARN_ON_USE (inet_ntop, "inet_ntop is unportable - "
                 "use gnulib module inet_ntop for portability");
# endif
#endif

#if 0
# if !1
extern int inet_pton (int af, const char *restrict src, void *restrict dst)
     _GL_ARG_NONNULL ((2, 3));
# endif
#elif defined GNULIB_POSIXCHECK
# undef inet_pton
# if HAVE_RAW_DECL_INET_PTON
_GL_WARN_ON_USE (inet_pton, "inet_pton is unportable - "
                 "use gnulib module inet_pton for portability");
# endif
#endif

#ifdef __cplusplus
}
#endif

#endif /* _GL_ARPA_INET_H */
#endif /* _GL_ARPA_INET_H */
