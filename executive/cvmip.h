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
 * Cavium Networks Internet Protocol (IP)
 *
 * Definitions for the Internet Protocol (IP) support.
 *
 * $Id: cvmip.h 2 2007-04-05 08:51:12Z tt $ $Name$
 *
 */

#ifndef __CVMIP_H__
#define __CVMIP_H__


/*
 * IP protocol values (1 byte)
 *
 */
#define  CVMIP_PROTO_ICMP  1    /* Internet Control Message Protocol */
#define  CVMIP_PROTO_TCP   6    /* Transmission Control Protocol */
#define  CVMIP_PROTO_UDP  17    /* User Datagram Protocol */
#define  CVMIP_PROTO_ESP  50    /* Encapsulated Security Payload */
#define  CVMIP_PROTO_AH   51    /* Authentication Header */


/**
 * network packet header definitions
 * (originally from octane_hw.h)
 *
 */

/**
 * UDP Packet header
 */
typedef struct {
   union {
      int32_t           s32     ;
      uint32_t          u32     ;
      struct {
         uint16_t        src_prt ;
         uint16_t        dst_prt ;
      } s;
   } prts;
   uint16_t            len     ;
   uint16_t            chksum  ;
} cvmip_udp_hdr_t;

/**
 * TCP Packet header
 */
typedef struct {
   uint16_t            src_prt ;
   uint16_t            dst_prt ;
   uint32_t            seq     ;
   uint32_t            ack_seq ;
   uint32_t            hlen    :4;
   uint32_t            rsvd    :6;
   uint32_t            urg     :1;
   uint32_t            ack     :1;
   uint32_t            psh     :1;
   uint32_t            rst     :1;
   uint32_t            syn     :1;
   uint32_t            fin     :1;
   uint16_t            win_sz  ;
   uint16_t            chksum  ;
   uint16_t            urg_ptr ;
   uint32_t            junk    ;
} cvmip_tcp_hdr_t;

/**
 * L4 Packet header
 */
typedef union {
   cvmip_udp_hdr_t udphdr;
   cvmip_tcp_hdr_t tcphdr;
   struct {
      union {
         int32_t           s32    ;
         uint32_t          u32    ;
         struct {
            uint16_t        src_prt;
            uint16_t        dst_prt;
         } s;
      } prts;
      uint16_t            len     ;
      uint16_t            chksum  ;
      char              dat[48] ; // 48 for IPv6 with no extension hdrs, 64 for IPv4 without options
   } udp;
   struct {
      uint16_t            src_prt ;
      uint16_t            dst_prt ;
      uint32_t            seq     ;
      uint32_t            ack_seq ;
      uint32_t            hlen    :4;
      uint32_t            rsvd    :6;
      uint32_t            urg     :1;
      uint32_t            ack     :1;
      uint32_t            psh     :1;
      uint32_t            rst     :1;
      uint32_t            syn     :1;
      uint32_t            fin     :1;
      uint16_t            win_sz  ;
      uint16_t            chksum  ;
      uint16_t            urg_ptr ;
      char              dat[36] ; // 36 for IPv6 with no extension hdrs, 52 for IPv6 without options
   } tcp;
} cvmip_l4_info_t;

/**
 * Special struct to add a pad to IPv4 header
 */
typedef struct {
   uint32_t            pad;

   uint32_t            version : 4;
   uint32_t            hl      : 4;
   uint8_t             tos     ;
   uint16_t            len     ;

   uint16_t            id      ;
   uint32_t            mbz     : 1;
   uint32_t            df      : 1;
   uint32_t            mf      : 1;
   uint32_t            off     :13;

   uint8_t             ttl     ;
   uint8_t             protocol;
   uint16_t            chksum  ;

   union {
      uint64_t          u64;
      struct {
         uint32_t        src;
         uint32_t        dst;
      } s;
   } src_dst;
} cvmip_ipv4_hdr_t;

/**
 * IPv6 Packet header
 */
typedef struct {

   uint32_t            version : 4;
   uint32_t            v6class : 8;
   uint32_t            flow    :20;

   uint16_t            len     ;    // includes extension headers plus payload (add 40 to be equiv to v4 len field)
   uint8_t             next_hdr;    // equivalent to the v4 protocol field
   uint8_t             hop_lim ;    // equivalent to the v4 TTL field

   union {
      uint64_t          u64[4];
      struct {
         uint64_t        src[2];
         uint64_t        dst[2];
      } s;
   } src_dst;

} cvmip_ipv6_hdr_t;


#endif /* __CVMIP_H__ */
