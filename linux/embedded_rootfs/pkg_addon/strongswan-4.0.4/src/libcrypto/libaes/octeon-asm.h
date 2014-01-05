/*************************************************************************
Copyright (c) 2003-2004, Cavium Networks. All rights reserved.

This Software is the property of Cavium Networks.  The Software and all
accompanying documentation are copyrighted.  The Software made available
here constitutes the proprietary information of Cavium Networks.  You
agree to take reasonable steps to prevent the disclosure, unauthorized use
or unauthorized distribution of the Software.  You shall use this Software
solely with Cavium hardware.

Except as expressly permitted in a separate Software License Agreement
between You and Cavium Networks, you shall not modify, decompile,
disassemble, extract, or otherwise reverse engineer this Software.  You
shall not make any copy of the Software or its accompanying documentation,
except for copying incident to the ordinary and intended use of the
Software and the Underlying Program and except for the making of a single
archival copy.

This Software, including technical data, may be subject to U.S.  export
control laws, including the U.S.  Export Administration Act and its
associated regulations, and may be subject to export or import regulations
in other countries.  You warrant that You will comply strictly in all
respects with all such regulations and acknowledge that you have the
responsibility to obtain licenses to export, re-export or import the
Software.

TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
AND WITH ALL FAULTS AND CAVIUM MAKES NO PROMISES, REPRESENTATIONS OR
WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
TO THE SOFTWARE, INCLUDING ITS CONDITION, ITS CONFORMITY TO ANY
REPRESENTATION OR DESCRIPTION, OR THE EXISTENCE OF ANY LATENT OR PATENT
DEFECTS, AND CAVIUM SPECIFICALLY DISCLAIMS ALL IMPLIED (IF ANY) WARRANTIES
OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR
PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET
POSSESSION OR CORRESPONDENCE TO DESCRIPTION.  THE ENTIRE RISK ARISING OUT
OF USE OR PERFORMANCE OF THE SOFTWARE LIES WITH YOU.

*************************************************************************/

/*
 * Derived from cvmx-asm.h
 */

#ifndef __OCTEON_ASM_H__
#define __OCTEON_ASM_H__

#ifdef	__cplusplus
extern "C" {
#endif

/* turn the variable name into a string */
#define CVMX_TMP_STR(x) CVMX_TMP_STR2(x)
#define CVMX_TMP_STR2(x) #x

#define CVMX_PREFETCH0(address) CVMX_PREFETCH(address, 0)
// a normal prefetch
#define CVMX_PREFETCH(address, offset) CVMX_PREFETCH_PREF0(address, offset)
// normal prefetches that use the pref instruction
#define CVMX_PREFETCH_PREF0(address, offset) asm volatile ("pref 0, " CVMX_TMP_STR(offset) "(%[rbase])" : : [rbase] "d" (address) )

/* mul stuff */
#define CVMX_MTM0(m) asm volatile ("mtm0 %[rs]" : : [rs] "d" (m))
#define CVMX_MTM1(m) asm volatile ("mtm1 %[rs]" : : [rs] "d" (m))
#define CVMX_MTM2(m) asm volatile ("mtm2 %[rs]" : : [rs] "d" (m))
#define CVMX_MTP0(p) asm volatile ("mtp0 %[rs]" : : [rs] "d" (p))
#define CVMX_MTP1(p) asm volatile ("mtp1 %[rs]" : : [rs] "d" (p))
#define CVMX_MTP2(p) asm volatile ("mtp2 %[rs]" : : [rs] "d" (p))
// VMUL: {p3,p2,p1,p0,rd} = mpcand[63:0]*Mplier[255:0]+accum[63:0]
#define CVMX_VMUL(dest,mpcand,accum) asm volatile ("vmul %[rd],%[rs],%[rt]" : [rd] "=d" (dest) : [rs] "d" (mpcand), [rt] "d" (accum))
#define CVMX_VMULU(dest,mpcand,accum) asm volatile ("vmulu %[rd],%[rs],%[rt]" : [rd] "=d" (dest) : [rs] "d" (mpcand), [rt] "d" (accum))
// VSMUL:      {p1,p0,rd} = mpcand[63:0]*Mplier[127:0]+accum[63:0]
#define CVMX_V3MUL(dest,mpcand,accum) asm volatile ("v3mul %[rd],%[rs],%[rt]" : [rd] "=d" (dest) : [rs] "d" (mpcand), [rt] "d" (accum))
#define CVMX_V3MULU(dest,mpcand,accum) asm volatile ("v3mulu %[rd],%[rs],%[rt]" : [rd] "=d" (dest) : [rs] "d" (mpcand), [rt] "d" (accum))


// MD5 and SHA-1

// pos can be 0-6
#define CVMX_MT_HSH_DAT(val,pos)    asm volatile ("dmtc2 %[rt],0x0040+" CVMX_TMP_STR(pos) :                 : [rt] "d" (val))
#define CVMX_MT_HSH_DATZ(pos)       asm volatile ("dmtc2    $0,0x0040+" CVMX_TMP_STR(pos) :                 :               )
// pos can be 0-14
#define CVMX_MT_HSH_DATW(val,pos)   asm volatile ("dmtc2 %[rt],0x0240+" CVMX_TMP_STR(pos) :                 : [rt] "d" (val))
#define CVMX_MT_HSH_DATWZ(pos)      asm volatile ("dmtc2    $0,0x0240+" CVMX_TMP_STR(pos) :                 :               )
#define CVMX_MT_HSH_STARTMD5(val)   asm volatile ("dmtc2 %[rt],0x4047"                   :                 : [rt] "d" (val))
#define CVMX_MT_HSH_STARTSHA(val)   asm volatile ("dmtc2 %[rt],0x4057"                   :                 : [rt] "d" (val))
#define CVMX_MT_HSH_STARTSHA256(val)   asm volatile ("dmtc2 %[rt],0x404f"                   :                 : [rt] "d" (val))
#define CVMX_MT_HSH_STARTSHA512(val)   asm volatile ("dmtc2 %[rt],0x424f"                   :                 : [rt] "d" (val))
// pos can be 0-3
#define CVMX_MT_HSH_IV(val,pos)     asm volatile ("dmtc2 %[rt],0x0048+" CVMX_TMP_STR(pos) :                 : [rt] "d" (val))
// pos can be 0-7
#define CVMX_MT_HSH_IVW(val,pos)     asm volatile ("dmtc2 %[rt],0x0250+" CVMX_TMP_STR(pos) :                 : [rt] "d" (val))

// pos can be 0-6
#define CVMX_MF_HSH_DAT(val,pos)    asm volatile ("dmfc2 %[rt],0x0040+" CVMX_TMP_STR(pos) : [rt] "=d" (val) :               )
// pos can be 0-14
#define CVMX_MF_HSH_DATW(val,pos)   asm volatile ("dmfc2 %[rt],0x0240+" CVMX_TMP_STR(pos) : [rt] "=d" (val) :               )
// pos can be 0-3
#define CVMX_MF_HSH_IV(val,pos)     asm volatile ("dmfc2 %[rt],0x0048+" CVMX_TMP_STR(pos) : [rt] "=d" (val) :               )
// pos can be 0-7
#define CVMX_MF_HSH_IVW(val,pos)     asm volatile ("dmfc2 %[rt],0x0250+" CVMX_TMP_STR(pos) : [rt] "=d" (val) :               )

// 3DES

// pos can be 0-2
#define CVMX_MT_3DES_KEY(val,pos)   asm volatile ("dmtc2 %[rt],0x0080+" CVMX_TMP_STR(pos) :                 : [rt] "d" (val))
#define CVMX_MT_3DES_IV(val)        asm volatile ("dmtc2 %[rt],0x0084"                   :                 : [rt] "d" (val))
#define CVMX_MT_3DES_ENC_CBC(val)   asm volatile ("dmtc2 %[rt],0x4088"                   :                 : [rt] "d" (val))
#define CVMX_MT_3DES_ENC(val)       asm volatile ("dmtc2 %[rt],0x408a"                   :                 : [rt] "d" (val))
#define CVMX_MT_3DES_DEC_CBC(val)   asm volatile ("dmtc2 %[rt],0x408c"                   :                 : [rt] "d" (val))
#define CVMX_MT_3DES_DEC(val)       asm volatile ("dmtc2 %[rt],0x408e"                   :                 : [rt] "d" (val))
#define CVMX_MT_3DES_RESULT(val)    asm volatile ("dmtc2 %[rt],0x0098"                   :                 : [rt] "d" (val))

// pos can be 0-2
#define CVMX_MF_3DES_KEY(val,pos)   asm volatile ("dmfc2 %[rt],0x0080+" CVMX_TMP_STR(pos) : [rt] "=d" (val) :               )
#define CVMX_MF_3DES_IV(val)        asm volatile ("dmfc2 %[rt],0x0084"                   : [rt] "=d" (val) :               )
#define CVMX_MF_3DES_RESULT(val)    asm volatile ("dmfc2 %[rt],0x0088"                   : [rt] "=d" (val) :               )

// AES

#define CVMX_MT_AES_ENC_CBC0(val)   asm volatile ("dmtc2 %[rt],0x0108"                   :                 : [rt] "d" (val))
#define CVMX_MT_AES_ENC_CBC1(val)   asm volatile ("dmtc2 %[rt],0x3109"                   :                 : [rt] "d" (val))
#define CVMX_MT_AES_ENC0(val)       asm volatile ("dmtc2 %[rt],0x010a"                   :                 : [rt] "d" (val))
#define CVMX_MT_AES_ENC1(val)       asm volatile ("dmtc2 %[rt],0x310b"                   :                 : [rt] "d" (val))
#define CVMX_MT_AES_DEC_CBC0(val)   asm volatile ("dmtc2 %[rt],0x010c"                   :                 : [rt] "d" (val))
#define CVMX_MT_AES_DEC_CBC1(val)   asm volatile ("dmtc2 %[rt],0x310d"                   :                 : [rt] "d" (val))
#define CVMX_MT_AES_DEC0(val)       asm volatile ("dmtc2 %[rt],0x010e"                   :                 : [rt] "d" (val))
#define CVMX_MT_AES_DEC1(val)       asm volatile ("dmtc2 %[rt],0x310f"                   :                 : [rt] "d" (val))
// pos can be 0-3
#define CVMX_MT_AES_KEY(val,pos)    asm volatile ("dmtc2 %[rt],0x0104+" CVMX_TMP_STR(pos) :                 : [rt] "d" (val))
// pos can be 0-1
#define CVMX_MT_AES_IV(val,pos)     asm volatile ("dmtc2 %[rt],0x0102+" CVMX_TMP_STR(pos) :                 : [rt] "d" (val))
#define CVMX_MT_AES_KEYLENGTH(val)  asm volatile ("dmtc2 %[rt],0x0110"                   :                 : [rt] "d" (val)) // write the keylen
// pos can be 0-1
#define CVMX_MT_AES_RESULT(val,pos) asm volatile ("dmtc2 %[rt],0x0100+" CVMX_TMP_STR(pos) :                 : [rt] "d" (val))

// pos can be 0-1
#define CVMX_MF_AES_RESULT(val,pos) asm volatile ("dmfc2 %[rt],0x0100+" CVMX_TMP_STR(pos) : [rt] "=d" (val) :               )
// pos can be 0-1
#define CVMX_MF_AES_IV(val,pos)     asm volatile ("dmfc2 %[rt],0x0102+" CVMX_TMP_STR(pos) : [rt] "=d" (val) :               )
// pos can be 0-3
#define CVMX_MF_AES_KEY(val,pos)    asm volatile ("dmfc2 %[rt],0x0104+" CVMX_TMP_STR(pos) : [rt] "=d" (val) :               )
#define CVMX_MF_AES_KEYLENGTH(val)  asm volatile ("dmfc2 %[rt],0x0110"                   : [rt] "=d" (val) :               ) // read the keylen
#define CVMX_MF_AES_DAT0(val)       asm volatile ("dmfc2 %[rt],0x0111"                   : [rt] "=d" (val) :               ) // first piece of input data
/* GFM COP2 macros */
/* index can be 0 or 1 */
#define CVMX_MF_GFM_MUL(val, index)     asm volatile ("dmfc2 %[rt],0x0258+" CVMX_TMP_STR(index) : [rt] "=d" (val) :               )
#define CVMX_MF_GFM_POLY(val)           asm volatile ("dmfc2 %[rt],0x025e"                      : [rt] "=d" (val) :               )
#define CVMX_MF_GFM_RESINP(val, index)  asm volatile ("dmfc2 %[rt],0x025a+" CVMX_TMP_STR(index) : [rt] "=d" (val) :               )

#define CVMX_MT_GFM_MUL(val, index)     asm volatile ("dmtc2 %[rt],0x0258+" CVMX_TMP_STR(index) :                 : [rt] "d" (val))
#define CVMX_MT_GFM_POLY(val)           asm volatile ("dmtc2 %[rt],0x025e"                      :                 : [rt] "d" (val))
#define CVMX_MT_GFM_RESINP(val, index)  asm volatile ("dmtc2 %[rt],0x025a+" CVMX_TMP_STR(index) :                 : [rt] "d" (val))
#define CVMX_MT_GFM_XOR0(val)           asm volatile ("dmtc2 %[rt],0x025c"                      :                 : [rt] "d" (val))
#define CVMX_MT_GFM_XORMUL1(val)        asm volatile ("dmtc2 %[rt],0x425d"                      :                 : [rt] "d" (val))


#if 0
#define CVMX_MF_CYCLE(dest)         asm volatile ("dmfc0 %[rt],$9,6" : [rt] "=d" (dest) : ) // Use (64-bit) CvmCount register rather than Count
#else
#define CVMX_MF_CYCLE(dest)         CVMX_RDHWR(dest, 31) /* reads the current (64-bit) CvmCount value */
#endif

#define CVMX_MT_CYCLE(src)         asm volatile ("dmtc0 %[rt],$9,6" : [rt] "d" (src) : )

#define CVMX_DSBH(result, input1) asm ("dsbh %[rd],%[rt]" : [rd] "=d" (result) : [rt] "d" (input1))
#define CVMX_DSHD(result, input1) asm ("dshd %[rd],%[rt]" : [rd] "=d" (result) : [rt] "d" (input1))

// Endian swap
#define CVMX_ES64(result, input) CVMX_DSBH(result, input); \
                                 CVMX_DSHD(result, result)
#ifdef	__cplusplus
}
#endif

#endif /* __CVMX_ASM_H__ */
