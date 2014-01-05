/******************************************************************************
*
* FILE: emprimitive.h
*
*------------------------------------------------------------------------------
*
*  Copyright 2004, 2005 Trend Micro, Inc.  All rights reserved
*
******************************************************************************/

#ifndef _EMPRIMITIVE_H_
#define _EMPRIMITIVE_H_

#ifndef TRUE
#	define TRUE       (1)
#endif

#ifndef FALSE
#	define FALSE      (0)
#endif

#if defined (WIN32) /* for Win32 platform */

#	if defined _MSC_VER && _MSC_VER >= 1400                 /* 1400 == VC++ 8.0 */
#		if !defined(_CRT_SECURE_NO_WARNINGS)
#			define _CRT_SECURE_NO_WARNINGS
#		endif
#	endif

#	include <BaseTsd.h>

	typedef __int64             em_int64;
	typedef long                em_int32;
	typedef short int           em_int16;
	typedef char                em_int8;
	typedef em_int8             em_bool;
	typedef float               em_float32;
	typedef double              em_float64;

	typedef unsigned __int64    em_uint64;
	typedef unsigned long       em_uint32;
	typedef unsigned short int  em_uint16;
	typedef unsigned char       em_uint8;

/* for X86 32bit Linux platform */
#elif defined(LINUX) && defined (I386)

#	include <sys/types.h>
#	include <stdint.h>
#	include <inttypes.h>

	typedef int64_t             em_int64;
	typedef int32_t             em_int32;
	typedef int16_t             em_int16;
	typedef int8_t              em_int8;
	typedef em_int8             em_bool;
	typedef float               em_float32;
	typedef double              em_float64;

	typedef uint64_t            em_uint64;
	typedef uint32_t            em_uint32;
	typedef uint16_t            em_uint16;
	typedef uint8_t             em_uint8;

/* for X86 32bit FreeBSD platform */
#elif defined(FREEBSD) && defined (I386)

	typedef long long           em_int64;
	typedef int                 em_int32;
	typedef short int           em_int16;
	typedef char                em_int8;
	typedef em_int8             em_bool;
	typedef float               em_float32;
	typedef double              em_float64;

	typedef unsigned int        em_uint32;
	typedef unsigned short int  em_uint16;
	typedef unsigned char       em_uint8;

#elif defined (__SPARC) /* for SPARC 32bit platform */

	typedef long long           em_int64;
	typedef long                em_int32;
	typedef short int           em_int16;
	typedef char                em_int8;
	typedef em_int8             em_bool;
	typedef float               em_float32;
	typedef double              em_float64;

	typedef unsigned long       em_uint32;
	typedef unsigned short int  em_uint16;
	typedef unsigned char       em_uint8;
#else
//#elif defined (INTEL_IPX) || defined (LINKSYS_SERCOMM) || (defined(LINUX) && defined(MIPS)) || defined (LINKSYS_CAVIUM)
#	include <sys/types.h>
#	include <stdint.h>
#	include <inttypes.h>

	typedef int64_t             em_int64;
	typedef int32_t             em_int32;
	typedef int16_t             em_int16;
	typedef int8_t              em_int8;
	typedef em_int8             em_bool;
	typedef float               em_float32;
	typedef double              em_float64;

	typedef uint64_t            em_uint64;
	typedef uint32_t            em_uint32;
	typedef uint16_t            em_uint16;
	typedef uint8_t             em_uint8;

#endif

#endif /* _EMPRIMITIVE_H_ */
