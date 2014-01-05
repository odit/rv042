/****************************************************************
 * Copyright (c) 2003-2004, Cavium Networks. All rights reserved.
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
 * Header file for the zip (deflate) block
 *
 * $Id: cvmx-zip.h 2 2007-04-05 08:51:12Z tt $ $Name$
 */

#ifndef __CVMX_ZIP_H__
#define __CVMX_ZIP_H__

#ifdef	__cplusplus
extern "C" {
#endif

typedef union {
   uint64_t u64;
   struct {
#if __BYTE_ORDER == __LITTLE_ENDIAN
      uint64_t ptr                 : 40;
      uint64_t length              : 16;
      uint64_t little_endian       :  1;
      uint64_t no_l2_alloc         :  1;
      uint64_t full_block_write    :  1;
      uint64_t unused              :  5;
#else
      uint64_t unused              :  5;
      uint64_t full_block_write    :  1;
      uint64_t no_l2_alloc         :  1;
      uint64_t little_endian       :  1;
      uint64_t length              : 16;
      uint64_t ptr                 : 40;
#endif
   } s;
} cvmx_zip_ptr_t;
#define CVMX_ZIP_PTR_MAX_LEN    ((1 << 16) - 1)


typedef enum {
   CVMX_ZIP_COMPLETION_NOTDONE = 0,
   CVMX_ZIP_COMPLETION_SUCCESS = 1,
   CVMX_ZIP_COMPLETION_OTRUNC  = 2,
   CVMX_ZIP_COMPLETION_STOP    = 3,
   CVMX_ZIP_COMPLETION_ITRUNC  = 4,
   CVMX_ZIP_COMPLETION_RBLOCK  = 5,
   CVMX_ZIP_COMPLETION_NLEN    = 6,
   CVMX_ZIP_COMPLETION_BADCODE = 7
} cvmx_zip_completion_code_t;

typedef union {
   uint64_t u64[3];
   struct {

      // WORD 0
#if __BYTE_ORDER == __LITTLE_ENDIAN
      uint64_t adler               : 32;
      uint64_t crc32               : 32;
#else
      uint64_t crc32               : 32;
      uint64_t adler               : 32;
#endif

      // WORD 1
#if __BYTE_ORDER == __LITTLE_ENDIAN
      uint64_t totalbytesread      : 32;
      uint64_t totalbyteswritten   : 32;
#else
      uint64_t totalbyteswritten   : 32;
      uint64_t totalbytesread      : 32;
#endif

      // WORD 2
#if __BYTE_ORDER == __LITTLE_ENDIAN
      cvmx_zip_completion_code_t    completioncode      :  8; // If polling, SW should set this to zero and wait for non-zero
      uint64_t                      eof                 :  1; // decompression only
      uint64_t                      unused22            :  7;
      uint64_t                      exbits              :  7; // compression only
      uint64_t                      unused21            :  1;
      uint64_t                      exnum               :  3; // compression only
      uint64_t                      unused20            :  5;
      uint64_t                      totalbitsprocessed  : 32; // decompression only
#else
      uint64_t                      totalbitsprocessed  : 32; // decompression only
      uint64_t                      unused20            :  5;
      uint64_t                      exnum               :  3; // compression only
      uint64_t                      unused21            :  1;
      uint64_t                      exbits              :  7; // compression only
      uint64_t                      unused22            :  7;
      uint64_t                      eof                 :  1; // decompression only
      cvmx_zip_completion_code_t    completioncode      :  8; // If polling, SW should set this to zero and wait for non-zero
#endif
   } s;
} cvmx_zip_result_t;

typedef union {
   uint64_t u64[8];
   struct {

      // WORD 0
#if __BYTE_ORDER == __LITTLE_ENDIAN
      uint64_t hgather             :  1;
      uint64_t dgather             :  1;
      uint64_t dscatter            :  1;
      uint64_t unused04            :  1;
      uint64_t compress            :  1;
      uint64_t bof                 :  1;
      uint64_t eof                 :  1;
      uint64_t forcedynamic        :  1;
      uint64_t forcefixed          :  1;
      uint64_t speed               :  1;
      uint64_t unused03            :  6;
      uint64_t exbits              :  7;
      uint64_t unused02            :  1;
      uint64_t exnum               :  3;
      uint64_t unused01            :  5;
      uint64_t totaloutputlength   : 24;
      uint64_t unused00            :  8;
#else
      uint64_t unused00            :  8;
      uint64_t totaloutputlength   : 24;
      uint64_t unused01            :  5;
      uint64_t exnum               :  3;
      uint64_t unused02            :  1;
      uint64_t exbits              :  7;
      uint64_t unused03            :  6;
      uint64_t speed               :  1;
      uint64_t forcefixed          :  1;
      uint64_t forcedynamic        :  1;
      uint64_t eof                 :  1;
      uint64_t bof                 :  1;
      uint64_t compress            :  1;
      uint64_t unused04            :  1;
      uint64_t dscatter            :  1;
      uint64_t dgather             :  1;
      uint64_t hgather             :  1;
#endif

      // WORD 1
#if __BYTE_ORDER == __LITTLE_ENDIAN
      uint64_t adler32             : 32;
      uint64_t unused10            : 16;
      uint64_t historylength       : 16;
#else
      uint64_t historylength       : 16;
      uint64_t unused10            : 16;
      uint64_t adler32             : 32;
#endif

      // WORD 2
      cvmx_zip_ptr_t ctx_ptr;

      // WORD 3
      cvmx_zip_ptr_t hist_ptr;

      // WORD 4
      cvmx_zip_ptr_t in_ptr;

      // WORD 5
      cvmx_zip_ptr_t out_ptr;

      // WORD 6
      cvmx_zip_ptr_t result_ptr;

      // WORD 7
      cvmx_zip_ptr_t wq_ptr;

   } s;
} cvmx_zip_command_t;


/* CSR typedefs have been moved to cvmx-csr-*.h */

#ifdef	__cplusplus
}
#endif

#endif  /* __CVMX_ZIP_H__ */
