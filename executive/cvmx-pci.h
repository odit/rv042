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
 * PCI related structures.
 *
 * File version info: $Id: cvmx-pci.h 2 2007-04-05 08:51:12Z tt $ $Name$
 */

#ifndef __CVMX_PCI_H__
#define __CVMX_PCI_H__

#include "cvmx-wqe.h"

#ifdef	__cplusplus
extern "C" {
#endif


typedef enum { CVMX_OCT_PCI_ES_PASS = 0,
               CVMX_OCT_PCI_ES_BS64 = 1,
               CVMX_OCT_PCI_ES_BS32 = 2,
               CVMX_OCT_PCI_ES_LWS  = 3
} cvmx_pci_endian_swap_t;

typedef union {

   uint64_t u64;

   struct {
#if __BYTE_ORDER == __LITTLE_ENDIAN
      uint64_t addr              :60;
      uint64_t es_ns_ro          : 4;
#else
      uint64_t es_ns_ro          : 4;
      uint64_t addr              :60;
#endif
   } st;

   struct {
#if __BYTE_ORDER == __LITTLE_ENDIAN
      uint64_t addr              :60;
      uint64_t ro                : 1;
      uint64_t ns                : 1;
      cvmx_pci_endian_swap_t es  : 2;
#else
      cvmx_pci_endian_swap_t es  : 2;
      uint64_t ns                : 1;
      uint64_t ro                : 1;
      uint64_t addr              :60;
#endif
   } s;

} cvmx_pci_dptr_t;

typedef union {

   uint64_t u64;

   struct {
#if __BYTE_ORDER == __LITTLE_ENDIAN
      uint64_t tag               :32;
      cvmx_pow_tag_type_t tt      : 2;
      uint64_t rs                : 1;
      uint64_t grp               : 4;
      uint64_t qos               : 3;
      uint64_t fsz               : 6;
      uint64_t dlengsz           :14;
      uint64_t g                 : 1;
      uint64_t r                 : 1;
#else
      uint64_t r                 : 1;
      uint64_t g                 : 1;
      uint64_t dlengsz           :14;
      uint64_t fsz               : 6;
      uint64_t qos               : 3;
      uint64_t grp               : 4;
      uint64_t rs                : 1;
      cvmx_pow_tag_type_t tt      : 2;
      uint64_t tag               :32;
#endif
   } s;

} cvmx_pci_inst_hdr_t;

typedef union {
   uint64_t u64[8];
   struct {
      cvmx_pci_dptr_t dptr;
      cvmx_pci_inst_hdr_t inst_hdr;
      uint64_t front_data[6];
   } s;
} cvmx_pci_inst_t;

typedef union {
   uint64_t u64[5];
   struct {
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define CVMX_OCT_PCI_COMPONENT_LEN(len) (3 - (len))
#else
#define CVMX_OCT_PCI_COMPONENT_LEN(len) (len)
#endif
      uint16_t len[4];
      uint64_t ptr[4];
   } s;
} cvmx_oct_pci_dma_component_t;

typedef union {
   uint64_t u64;
   struct {
#if __BYTE_ORDER == __LITTLE_ENDIAN
      uint64_t ptr       :40;
      uint64_t nl        : 4;
      uint64_t nr        : 4;
      uint64_t fl        : 1;
      uint64_t ii        : 1;
      uint64_t fi        : 1;
      uint64_t ca        : 1;
      uint64_t c         : 1;
      uint64_t wqp       : 1;
      uint64_t dir       : 1;
      uint64_t rsvd      : 9;
#else
      uint64_t rsvd      : 9;
      uint64_t dir       : 1;
      uint64_t wqp       : 1;
      uint64_t c         : 1;
      uint64_t ca        : 1;
      uint64_t fi        : 1;
      uint64_t ii        : 1;
      uint64_t fl        : 1;
      uint64_t nr        : 4;
      uint64_t nl        : 4;
      uint64_t ptr       :40;
#endif
   } s;
} cvmx_oct_pci_dma_inst_hdr_t;

typedef union {
   uint64_t                   u64;
   void                    *ptr;
   uint64_t               *u64ptr;
   struct {
#if __BYTE_ORDER == __LITTLE_ENDIAN
      uint64_t                addr  :36;
      uint64_t            reserved  : 4;
      uint64_t                size  :13;
      uint64_t                   l  : 1;
      uint64_t                   a  : 1;
      uint64_t                   f  : 1;
      uint64_t                pool  : 3;
      uint64_t                back  : 4;
      uint64_t                   i  : 1;
#else
      uint64_t                   i  : 1;
      uint64_t                back  : 4;
      uint64_t                pool  : 3;
      uint64_t                   f  : 1;
      uint64_t                   a  : 1;
      uint64_t                   l  : 1;
      uint64_t                size  :13;
      uint64_t            reserved  : 4;
      uint64_t                addr  :36;
#endif
   } s;
} cvmx_oct_pci_dma_local_ptr_t;

typedef struct {
   cvmx_pci_dptr_t buf;
   cvmx_pci_dptr_t info;
}  cvmx_pci_bufinfo_pair_t;

typedef union {
   uint64_t u64;
   struct {
#if __BYTE_ORDER == __LITTLE_ENDIAN
      uint32_t length;
      uint32_t bufs;
#else
      uint32_t bufs;
      uint32_t length;
#endif
   } s;
} cvmx_pci_packetlength_t;



/* CSR typedefs have been moved to cvmx-csr-*.h */

#ifdef	__cplusplus
}
#endif

#endif  /* __CVMX_PCI_H__ */
