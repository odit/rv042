/*
 * Copyright (c) 2003-2007 Cavium Networks (support@cavium.com). All rights
 * reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 * must display the following acknowledgement:
 * This product includes software developed by Cavium Networks
 * 4. Cavium Networks' name may not be used to endorse or promote products
 * derived from this software without specific prior written permission.
 * 
 * This Software, including technical data, may be subject to U.S. export
 * control laws, including the U.S. Export Administration Act and its
 * associated regulations, and may be subject to export or import regulations
 * in other countries. You warrant that You will comply strictly in all
 * respects with all such regulations and acknowledge that you have the
 * responsibility to obtain licenses to export, re-export or import the
 * Software.
 * 
 * TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS" AND
 * WITH ALL FAULTS AND CAVIUM MAKES NO PROMISES, REPRESENTATIONS OR WARRANTIES,
 * EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO THE
 * SOFTWARE, INCLUDING ITS CONDITION, ITS CONFORMITY TO ANY REPRESENTATION OR
 * DESCRIPTION, OR THE EXISTENCE OF ANY LATENT OR PATENT DEFECTS, AND CAVIUM
 * SPECIFICALLY DISCLAIMS ALL IMPLIED (IF ANY) WARRANTIES OF TITLE,
 * MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF
 * VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. THE ENTIRE RISK ARISING OUT OF USE OR
 * PERFORMANCE OF THE SOFTWARE LIES WITH YOU.
*/
   
#if defined (CONFIG_OCTEON_NATIVE_IPSEC)

#include "octeon-asm.h"
#include <linux/slab.h>
#define ESP_HEADER_LENGTH     8
#define DES_CBC_IV_LENGTH     8
#define AES_CBC_IV_LENGTH     16
#define ESP_HMAC_LEN          12



#define ESP_HEADER_LENGTH 8
#define DES_CBC_IV_LENGTH 8  


#define CVM_LOAD_SHA_UNIT(dat, next)  { \
   if (next == 0) {                     \
      next = 1;                         \
      CVMX_MT_HSH_DAT (dat, 0);         \
   } else if (next == 1) {              \
      next = 2;                         \
      CVMX_MT_HSH_DAT (dat, 1);         \
   } else if (next == 2) {              \
      next = 3; 	                \
      CVMX_MT_HSH_DAT (dat, 2);         \
   } else if (next == 3) {              \
      next = 4;                         \
      CVMX_MT_HSH_DAT (dat, 3);         \
   } else if (next == 4) {              \
      next = 5;	                        \
      CVMX_MT_HSH_DAT (dat, 4);         \
   } else if (next == 5) {              \
      next = 6;                         \
      CVMX_MT_HSH_DAT (dat, 5);         \
   } else if (next == 6) {              \
      next = 7;                         \
      CVMX_MT_HSH_DAT (dat, 6);         \
   } else {                             \
     CVMX_MT_HSH_STARTSHA (dat);        \
     next = 0;                          \
   }                                    \
}

#define CVM_LOAD2_SHA_UNIT(dat1, dat2, next)  { \
   if (next == 0) {                      \
      CVMX_MT_HSH_DAT (dat1, 0);         \
      CVMX_MT_HSH_DAT (dat2, 1);         \
      next = 2;                          \
   } else if (next == 1) {               \
      CVMX_MT_HSH_DAT (dat1, 1);         \
      CVMX_MT_HSH_DAT (dat2, 2);         \
      next = 3;                          \
   } else if (next == 2) {               \
      CVMX_MT_HSH_DAT (dat1, 2);         \
      CVMX_MT_HSH_DAT (dat2, 3);         \
      next = 4;                          \
   } else if (next == 3) {               \
      CVMX_MT_HSH_DAT (dat1, 3);         \
      CVMX_MT_HSH_DAT (dat2, 4);         \
      next = 5;                          \
   } else if (next == 4) {               \
      CVMX_MT_HSH_DAT (dat1, 4);         \
      CVMX_MT_HSH_DAT (dat2, 5);         \
      next = 6;                          \
   } else if (next == 5) {               \
      CVMX_MT_HSH_DAT (dat1, 5);         \
      CVMX_MT_HSH_DAT (dat2, 6);         \
      next = 7;                          \
   } else if (next == 6) {               \
      CVMX_MT_HSH_DAT (dat1, 6);         \
      CVMX_MT_HSH_STARTSHA (dat2);       \
      next = 0;                          \
   } else {                              \
     CVMX_MT_HSH_STARTSHA (dat1);        \
     CVMX_MT_HSH_DAT (dat2, 0);          \
     next = 1;                           \
   }                                     \
}

#ifdef OCTEON_IPSEC_DEBUG
static void
dump_pkt(uint8_t *str, uint8_t *dat, int len)
{
	int i;
	printk("%s\n", str);

	for (i = 0; i < len; i++) {
		printk("%02x ", dat[i]);
		if (i % 8 == 7)
			printk("\n");
	}

	printk("END %s\n", str);
}
#endif

void
cav_calc_hash(__u8 auth, uint64_t *key, uint8_t *inner, uint8_t *outer)
{
   uint8_t hash_key[64];
	uint64_t *key1;
   register uint64_t xor1 = 0x3636363636363636ULL;
   register uint64_t xor2 = 0x5c5c5c5c5c5c5c5cULL;
	struct octeon_cop2_state state;
	unsigned long flags;

   memset(hash_key, 0, sizeof(hash_key));
   memcpy(hash_key, (uint8_t *)key, (auth ? 20 : 16));
   key1 = (uint64_t *)hash_key;
	flags = octeon_crypto_enable(&state);
   if (auth) {
      CVMX_MT_HSH_IV(0x67452301EFCDAB89ULL, 0);
      CVMX_MT_HSH_IV(0x98BADCFE10325476ULL, 1);
      CVMX_MT_HSH_IV(0xC3D2E1F000000000ULL, 2);
   } else {
      CVMX_MT_HSH_IV(0x0123456789ABCDEFULL, 0);
      CVMX_MT_HSH_IV(0xFEDCBA9876543210ULL, 1);
   }

   CVMX_MT_HSH_DAT((*key1 ^ xor1), 0);
   key1++;
   CVMX_MT_HSH_DAT((*key1 ^ xor1), 1);
   key1++;
   CVMX_MT_HSH_DAT((*key1 ^ xor1), 2);
   key1++;
   CVMX_MT_HSH_DAT((*key1 ^ xor1), 3);
   key1++;
   CVMX_MT_HSH_DAT((*key1 ^ xor1), 4);
   key1++;
   CVMX_MT_HSH_DAT((*key1 ^ xor1), 5);
   key1++;
   CVMX_MT_HSH_DAT((*key1 ^ xor1), 6);
   key1++;
   if (auth)
      CVMX_MT_HSH_STARTSHA((*key1 ^ xor1));
   else
      CVMX_MT_HSH_STARTMD5((*key1 ^ xor1));

   CVMX_MF_HSH_IV (((uint64_t *)inner)[0], 0);
   CVMX_MF_HSH_IV (((uint64_t *)inner)[1], 1);
   if (auth) {
      ((uint64_t *)inner)[2] = 0;
      CVMX_MF_HSH_IV (((uint64_t *)inner)[2], 2);
   }

   memset(hash_key, 0, sizeof(hash_key));
   memcpy(hash_key, (uint8_t *)key, (auth ? 20 : 16));
   key = (uint64_t *)hash_key;
   if (auth) {
      CVMX_MT_HSH_IV(0x67452301EFCDAB89ULL, 0);
      CVMX_MT_HSH_IV(0x98BADCFE10325476ULL, 1);
      CVMX_MT_HSH_IV(0xC3D2E1F000000000ULL, 2);
   } else {
      CVMX_MT_HSH_IV(0x0123456789ABCDEFULL, 0);
      CVMX_MT_HSH_IV(0xFEDCBA9876543210ULL, 1);
   }

   CVMX_MT_HSH_DAT((*key ^ xor2), 0);
   key++;
   CVMX_MT_HSH_DAT((*key ^ xor2), 1);
   key++;
   CVMX_MT_HSH_DAT((*key ^ xor2), 2);
   key++;
   CVMX_MT_HSH_DAT((*key ^ xor2), 3);
   key++;
   CVMX_MT_HSH_DAT((*key ^ xor2), 4);
   key++;
   CVMX_MT_HSH_DAT((*key ^ xor2), 5);
   key++;
   CVMX_MT_HSH_DAT((*key ^ xor2), 6);
   key++;
   if (auth) 
      CVMX_MT_HSH_STARTSHA((*key ^ xor2));
   else
      CVMX_MT_HSH_STARTMD5((*key ^ xor2));

   CVMX_MF_HSH_IV (((uint64_t *)outer)[0], 0);
   CVMX_MF_HSH_IV (((uint64_t *)outer)[1], 1);
   if (auth) {
      ((uint64_t *)outer)[2] = 0;
      CVMX_MF_HSH_IV (((uint64_t *)outer)[2], 2);
   }
	octeon_crypto_disable(&state, flags);
   return;
}

/*
 * MD5 APIs
 */

static inline uint64_t
swap64(uint64_t a)
{
	return ((a >> 56) |
		(((a >> 48) & 0xfful) << 8) |	
		(((a >> 40) & 0xfful) << 16) |	
		(((a >> 32) & 0xfful) << 24) |	
		(((a >> 24) & 0xfful) << 32) |	
		(((a >> 16) & 0xfful) << 40) |	
		(((a >> 8) & 0xfful) << 48) |	
		(((a >> 0) & 0xfful) << 56)); 
}

#define CVM_LOAD_MD5_UNIT(dat, next)  { \
   if (next == 0) {                     \
      next = 1;                         \
      CVMX_MT_HSH_DAT (dat, 0);         \
   } else if (next == 1) {              \
      next = 2;                         \
      CVMX_MT_HSH_DAT (dat, 1);         \
   } else if (next == 2) {              \
      next = 3; 	                \
      CVMX_MT_HSH_DAT (dat, 2);         \
   } else if (next == 3) {              \
      next = 4;                         \
      CVMX_MT_HSH_DAT (dat, 3);         \
   } else if (next == 4) {              \
      next = 5;	                        \
      CVMX_MT_HSH_DAT (dat, 4);         \
   } else if (next == 5) {              \
      next = 6;                         \
      CVMX_MT_HSH_DAT (dat, 5);         \
   } else if (next == 6) {              \
      next = 7;                         \
      CVMX_MT_HSH_DAT (dat, 6);         \
   } else {                             \
     CVMX_MT_HSH_STARTMD5 (dat);        \
     next = 0;                          \
   }                                    \
}


#define CVM_LOAD2_MD5_UNIT(dat1, dat2, next)  { \
   if (next == 0) {                      \
      CVMX_MT_HSH_DAT (dat1, 0);         \
      CVMX_MT_HSH_DAT (dat2, 1);         \
      next = 2;                          \
   } else if (next == 1) {               \
      CVMX_MT_HSH_DAT (dat1, 1);         \
      CVMX_MT_HSH_DAT (dat2, 2);         \
      next = 3;                          \
   } else if (next == 2) {               \
      CVMX_MT_HSH_DAT (dat1, 2);         \
      CVMX_MT_HSH_DAT (dat2, 3);         \
      next = 4;                          \
   } else if (next == 3) {               \
      CVMX_MT_HSH_DAT (dat1, 3);         \
      CVMX_MT_HSH_DAT (dat2, 4);         \
      next = 5;                          \
   } else if (next == 4) {               \
      CVMX_MT_HSH_DAT (dat1, 4);         \
      CVMX_MT_HSH_DAT (dat2, 5);         \
      next = 6;                          \
   } else if (next == 5) {               \
      CVMX_MT_HSH_DAT (dat1, 5);         \
      CVMX_MT_HSH_DAT (dat2, 6);         \
      next = 7;                          \
   } else if (next == 6) {               \
      CVMX_MT_HSH_DAT (dat1, 6);         \
      CVMX_MT_HSH_STARTMD5 (dat2);       \
      next = 0;                          \
   } else {                              \
     CVMX_MT_HSH_STARTMD5 (dat1);        \
     CVMX_MT_HSH_DAT (dat2, 0);          \
     next = 1;                           \
   }                                     \
}

int AES_CBC_md5_encrypt(uint8_t *input, uint8_t *output, uint32_t pktlen, 
                        uint8_t *aes_key, int aes_key_len, uint8_t *aes_iv,
                        uint64_t *start_inner_md5, uint64_t *start_outer_md5)
{
   
   uint64_t *res = (uint64_t *)output, *data = (uint64_t *)input;
   register uint64_t in1, in2, out1, out2;
   register int next = 0;
   int inplen;
	struct octeon_cop2_state state;
	unsigned long flags;
   
   if (unlikely((input == NULL || output == NULL || pktlen == 0 || 
                 aes_key == NULL || aes_iv == NULL || aes_key_len == 0 || 
                 start_inner_md5 == NULL || 
                 start_outer_md5 == NULL))) {
      printk("%s Wrong parameters \n", __FUNCTION__);   
      return -1;
   }

   CVMX_PREFETCH0(aes_iv);
   CVMX_PREFETCH0(start_inner_md5);
   CVMX_PREFETCH0(data);

	flags = octeon_crypto_enable(&state);

   CVMX_MT_AES_KEY(((uint64_t *)aes_key)[0], 0);
   CVMX_MT_AES_KEY(((uint64_t *)aes_key)[1], 1);

   if (aes_key_len == 16) {
      CVMX_MT_AES_KEY(0x0, 2);
      CVMX_MT_AES_KEY(0x0, 3);
   } else if (aes_key_len == 24) {
      CVMX_MT_AES_KEY (((uint64_t *)aes_key)[2], 2);
      CVMX_MT_AES_KEY (0x0, 3);
   } else if (aes_key_len == 32) {
      CVMX_MT_AES_KEY (((uint64_t *)aes_key)[2], 2);
      CVMX_MT_AES_KEY (((uint64_t *)aes_key)[3], 3);
   } else {
      printk("%s: Wrong Key length \n", __FUNCTION__);
      return -1;
   }
   CVMX_MT_AES_KEYLENGTH (aes_key_len / 8 - 1);

   CVMX_MT_AES_IV (((uint64_t *)aes_iv)[0], 0);
   CVMX_MT_AES_IV (((uint64_t *)aes_iv)[1], 1);

   /* Load MD5 IV */
   CVMX_MT_HSH_IV (start_inner_md5[0], 0);
   CVMX_MT_HSH_IV (start_inner_md5[1], 1);

   /* Inplace */
   res = (uint64_t *)(input + ESP_HEADER_LENGTH + AES_CBC_IV_LENGTH);
   data = (uint64_t *)(input + ESP_HEADER_LENGTH + AES_CBC_IV_LENGTH);
   inplen = pktlen - ESP_HEADER_LENGTH - AES_CBC_IV_LENGTH;

   in1 = *data++;
   in2 = *data++;
   inplen -= 16;

   CVMX_MT_AES_ENC_CBC0 (in1);
   CVMX_MT_AES_ENC_CBC1 (in2);
   
   CVM_LOAD_MD5_UNIT(*(uint64_t *)input, next);
   CVM_LOAD_MD5_UNIT(*(uint64_t *)(input + sizeof(uint64_t)), next);
   CVM_LOAD_MD5_UNIT(*(uint64_t *)(input + 2*sizeof(uint64_t)), next);

   if (unlikely(inplen < 16)) 
      goto res_done;
   
   in1 = *data++;
   in2 = *data++;

   /* Loop through input */
   /* Assumed that data is 16 byte aligned */
   if (likely(inplen >= 32)) {
      while (1) {
         CVMX_MF_AES_RESULT (out1, 0);
         CVMX_MF_AES_RESULT (out2, 1);
         CVMX_MT_AES_ENC_CBC0 (in1);
         CVMX_MT_AES_ENC_CBC1 (in2);
         CVM_LOAD2_MD5_UNIT (out1, out2, next);
         res[0] = out1;
         res[1] = out2;
         in1 = data[0];
         in2 = data[1];
         res += 2;
         data += 2;
         CVMX_MF_AES_RESULT (out1, 0);
         CVMX_MF_AES_RESULT (out2, 1);
         CVMX_MT_AES_ENC_CBC0 (in1);
         CVMX_MT_AES_ENC_CBC1 (in2);
         CVM_LOAD2_MD5_UNIT (out1, out2, next);
         res[0] = out1;
         res[1] = out2;
         in1 = data[0];
         in2 = data[1];
         res += 2;
         data += 2;
         inplen -= 32;
         if (inplen < 32)
             break;
      }
   }
   /* inplen < 32  ==> inplen = 16 or inplen = 0 
      (Assuming 16 byte aligned data only) */
   if (inplen) {
       CVMX_MF_AES_RESULT (out1, 0);
       CVMX_MF_AES_RESULT (out2, 1);
       CVMX_MT_AES_ENC_CBC0 (in1);
       CVMX_MT_AES_ENC_CBC1 (in2);
       CVM_LOAD2_MD5_UNIT (out1, out2, next);
       res[0] = out1;
       res[1] = out2;
       res += 2;
   }

res_done:    
    CVMX_MF_AES_RESULT (out1, 0);
    CVMX_MF_AES_RESULT (out2, 1);
    CVM_LOAD2_MD5_UNIT (out1, out2, next);
    res[0] = out1;
    res[1] = out2;


   CVMX_PREFETCH0(start_outer_md5);
   /* Finish Inner hash */
   {
      CVM_LOAD_MD5_UNIT(0x8000000000000000ULL, next);
      while (next != 7) {
         CVM_LOAD_MD5_UNIT(((uint64_t)0x0ULL), next);
      } 
      CVM_LOAD_MD5_UNIT(swap64((uint64_t)((pktlen + 64) << 3)), next);
   } 

   /* Get the inner hash of HMAC */
   CVMX_MF_HSH_IV (in1, 0);
   CVMX_MF_HSH_IV (in2, 1);

   /* Initialize hash unit */
   CVMX_MT_HSH_IV (start_outer_md5[0], 0);
   CVMX_MT_HSH_IV (start_outer_md5[1], 1);

   CVMX_MT_HSH_DAT (in1, 0);
   CVMX_MT_HSH_DAT (in2, 1);
   out1 = 0x8000000000000000ULL;
   CVMX_MT_HSH_DAT (out1, 2);
   CVMX_MT_HSH_DATZ(3);
   CVMX_MT_HSH_DATZ(4);
   CVMX_MT_HSH_DATZ(5);
   CVMX_MT_HSH_DATZ(6);
   CVMX_MT_HSH_STARTMD5(swap64((uint64_t)((64 + 16) << 3)));

   /* Get the HMAC */
   res = (uint64_t *)(output+pktlen);
   CVMX_MF_HSH_IV (*res++, 0);
   CVMX_MF_HSH_IV (out1, 1);
   *((uint32_t *)res) = (uint32_t)(out1 >> 32);
   
	octeon_crypto_disable(&state, flags);
   return 0;
}

int AES_CBC_md5_decrypt(uint8_t *input, uint8_t *output, uint32_t pktlen, 
                        uint8_t *aes_key, int aes_key_len, uint8_t *aes_iv,
                        uint64_t *start_inner_md5, uint64_t *start_outer_md5)
{
   
   uint64_t *res = (uint64_t *)output, *data = (uint64_t *)input;
   register uint64_t in1, in2, out1, out2;
   register int next = 0;
   int inplen;
	struct octeon_cop2_state state;
	unsigned long flags;
   
   if (unlikely((input == NULL || output == NULL || pktlen == 0 || 
                 aes_key == NULL || aes_iv == NULL || aes_key_len == 0 || 
                 start_inner_md5 == NULL || 
                 start_outer_md5 == NULL))) {
      printk("%s Wrong parameters \n", __FUNCTION__);   
      return -1;
   }


   CVMX_PREFETCH0(aes_iv);
   CVMX_PREFETCH0(start_inner_md5);
   CVMX_PREFETCH0(data);

	flags = octeon_crypto_enable(&state);

   CVMX_MT_AES_KEY(((uint64_t *)aes_key)[0], 0);
   CVMX_MT_AES_KEY(((uint64_t *)aes_key)[1], 1);

   if (aes_key_len == 16) {
      CVMX_MT_AES_KEY(0x0, 2);
      CVMX_MT_AES_KEY(0x0, 3);
   } else if (aes_key_len == 24) {
      CVMX_MT_AES_KEY (((uint64_t *)aes_key)[2], 2);
      CVMX_MT_AES_KEY (0x0, 3);
   } else if (aes_key_len == 32) {
      CVMX_MT_AES_KEY (((uint64_t *)aes_key)[2], 2);
      CVMX_MT_AES_KEY (((uint64_t *)aes_key)[3], 3);
   } else {
      printk("%s: Wrong Key length \n", __FUNCTION__);
      return -1;
   }
   CVMX_MT_AES_KEYLENGTH (aes_key_len / 8 - 1);

   CVMX_MT_AES_IV (((uint64_t *)aes_iv)[0], 0);
   CVMX_MT_AES_IV (((uint64_t *)aes_iv)[1], 1);

   /* Load MD5 IV */
   CVMX_MT_HSH_IV (start_inner_md5[0], 0);
   CVMX_MT_HSH_IV (start_inner_md5[1], 1);

   data = (uint64_t *)(input + ESP_HEADER_LENGTH + AES_CBC_IV_LENGTH);
   res = (uint64_t *)(output + ESP_HEADER_LENGTH + AES_CBC_IV_LENGTH);
   inplen = pktlen - ESP_HEADER_LENGTH - AES_CBC_IV_LENGTH - 12;

   in1 = *data++;
   in2 = *data++;
   inplen -= 16;

   CVMX_MT_AES_DEC_CBC0 (in1);
   CVMX_MT_AES_DEC_CBC1 (in2);
   
   CVM_LOAD_MD5_UNIT(*(uint64_t *)input, next);
   CVM_LOAD_MD5_UNIT(*(uint64_t *)(input + sizeof(uint64_t)), next);
   CVM_LOAD_MD5_UNIT(*(uint64_t *)(input + 2*sizeof(uint64_t)), next);

   CVM_LOAD2_MD5_UNIT(in1, in2, next);

   if (unlikely(inplen < 16)) 
      goto res_done;
   
   in1 = *data++;
   in2 = *data++;

   /* Loop through input */
   /* Assumed that data is 16 byte aligned */
   if (likely(inplen >= 32)) {
      while (1) {
         CVMX_MF_AES_RESULT (out1, 0);
         CVMX_MF_AES_RESULT (out2, 1);
         CVMX_MT_AES_DEC_CBC0 (in1);
         CVMX_MT_AES_DEC_CBC1 (in2);
         CVM_LOAD2_MD5_UNIT (in1, in2, next);
         res[0] = out1;
         res[1] = out2;
         in1 = data[0];
         in2 = data[1];
         res += 2;
         data += 2;
         CVMX_MF_AES_RESULT (out1, 0);
         CVMX_MF_AES_RESULT (out2, 1);
         CVMX_MT_AES_DEC_CBC0 (in1);
         CVMX_MT_AES_DEC_CBC1 (in2);
         CVM_LOAD2_MD5_UNIT (in1, in2, next);
         res[0] = out1;
         res[1] = out2;
         in1 = data[0];
         in2 = data[1];
         res += 2;
         data += 2;
         inplen -= 32;
         if (inplen < 32)
             break;
      }
   }
   /* inplen < 32  ==> inplen = 16 or inplen = 0 
      (Assuming 16 byte aligned data only) */
   if (inplen) {
       CVMX_MF_AES_RESULT (out1, 0);
       CVMX_MF_AES_RESULT (out2, 1);
       CVMX_MT_AES_DEC_CBC0 (in1);
       CVMX_MT_AES_DEC_CBC1 (in2);
       CVM_LOAD2_MD5_UNIT (in1, in2, next);
       res[0] = out1;
       res[1] = out2;
       res += 2;
   }

res_done:    
    CVMX_MF_AES_RESULT (out1, 0);
    CVMX_MF_AES_RESULT (out2, 1);
    res[0] = out1;
    res[1] = out2;


   pktlen -= 12;
   CVMX_PREFETCH0(start_outer_md5);
   /* Finish Inner hash */
   {
      CVM_LOAD_MD5_UNIT(0x8000000000000000ULL, next);
      while (next != 7) {
         CVM_LOAD_MD5_UNIT(((uint64_t)0x0ULL), next);
      }
      CVM_LOAD_MD5_UNIT(swap64((uint64_t)((pktlen + 64) << 3)), next);
   } 


   /* Get the inner hash of HMAC */
   CVMX_MF_HSH_IV (in1, 0);
   CVMX_MF_HSH_IV (in2, 1);

   CVMX_PREFETCH0(input + pktlen);
   /* Initialize hash unit */
   CVMX_MT_HSH_IV (start_outer_md5[0], 0);
   CVMX_MT_HSH_IV (start_outer_md5[1], 1);

   CVMX_MT_HSH_DAT (in1, 0);
   CVMX_MT_HSH_DAT (in2, 1);
   out1 = 0x8000000000000000ULL;
   CVMX_MT_HSH_DAT (out1, 2);
   CVMX_MT_HSH_DATZ(3);
   CVMX_MT_HSH_DATZ(4);
   CVMX_MT_HSH_DATZ(5);
   CVMX_MT_HSH_DATZ(6);
   CVMX_MT_HSH_STARTMD5(swap64((uint64_t)((64 + 16) << 3)));

   /* Get and compare the HMAC */
   {
      res = (uint64_t *)(input+pktlen);
      CVMX_MF_HSH_IV (out1, 0);
      if (unlikely(out1 != *res)) {
			#ifdef OCTEON_IPSEC_DEBUG
         printk("%s: Decryption/Mac Mismatch \n", __FUNCTION__);
			#endif
	      octeon_crypto_disable(&state, flags);
         return -1;
      }
      CVMX_MF_HSH_IV (out2, 1);
      if (unlikely(*(uint32_t *)(res + 1) != (uint32_t)(out2 >> 32))) {
			#ifdef OCTEON_IPSEC_DEBUG
         printk("%s: Decryption/Mac Mismatch \n", __FUNCTION__);
			#endif
	      octeon_crypto_disable(&state, flags);
         return -1;
      }
   }
   
	octeon_crypto_disable(&state, flags);
   return 0;

}


int AES_CBC_sha1_encrypt(uint8_t *input, uint8_t *output, uint32_t pktlen, 
                         uint8_t *aes_key, int aes_key_len, uint8_t *aes_iv,
                         uint64_t *start_inner_sha, uint64_t *start_outer_sha)
{
   
   uint64_t *res = (uint64_t *)output, *data = (uint64_t *)input;
   register uint64_t in1, in2, out1, out2;
   register int next = 0;
   int inplen;
	struct octeon_cop2_state state;
	unsigned long flags;
   
   if (unlikely((input == NULL || output == NULL || pktlen == 0 ||
                 aes_key == NULL || aes_iv == NULL || aes_key_len == 0 ||
                 start_inner_sha == NULL || 
                 start_outer_sha == NULL))) {
      printk("%s Wrong parameters \n", __FUNCTION__);   
      return -1;
   }

   CVMX_PREFETCH0(aes_iv);
   /* PERF: start_inner_sha and start_outer_sha (if placed together) and 
      in a single cache line */
   CVMX_PREFETCH0(start_inner_sha);
   CVMX_PREFETCH0(data);

	flags = octeon_crypto_enable(&state);

   CVMX_MT_AES_KEY(((uint64_t *)aes_key)[0], 0);
   CVMX_MT_AES_KEY(((uint64_t *)aes_key)[1], 1);

   if (aes_key_len == 16) {
      CVMX_MT_AES_KEY(0x0, 2);
      CVMX_MT_AES_KEY(0x0, 3);
   } else if (aes_key_len == 24) {
      CVMX_MT_AES_KEY (((uint64_t *)aes_key)[2], 2);
      CVMX_MT_AES_KEY (0x0, 3);
   } else if (aes_key_len == 32) {
      CVMX_MT_AES_KEY (((uint64_t *)aes_key)[2], 2);
      CVMX_MT_AES_KEY (((uint64_t *)aes_key)[3], 3);
   } else {
      printk("%s: Wrong Key length \n", __FUNCTION__);
	   octeon_crypto_disable(&state, flags);
      return -1;
   }
   CVMX_MT_AES_KEYLENGTH (aes_key_len /8 - 1);

   CVMX_MT_AES_IV (((uint64_t *)aes_iv)[0], 0);
   CVMX_MT_AES_IV (((uint64_t *)aes_iv)[1], 1);

   /* Load SHA1 IV */
   CVMX_MT_HSH_IV (start_inner_sha[0], 0);
   CVMX_MT_HSH_IV (start_inner_sha[1], 1);
   CVMX_MT_HSH_IV (start_inner_sha[2], 2);

   /* Inplace */
   res = (uint64_t *)(input + ESP_HEADER_LENGTH + AES_CBC_IV_LENGTH);
   data = (uint64_t *)(input + ESP_HEADER_LENGTH + AES_CBC_IV_LENGTH);
   inplen = pktlen - ESP_HEADER_LENGTH - AES_CBC_IV_LENGTH;

   in1 = *data++;
   in2 = *data++;
   inplen -= 16;

   CVMX_MT_AES_ENC_CBC0 (in1);
   CVMX_MT_AES_ENC_CBC1 (in2);
   
   CVM_LOAD_SHA_UNIT(*(uint64_t *)input, next);
   CVM_LOAD_SHA_UNIT(*(uint64_t *)(input + sizeof(uint64_t)), next);
   CVM_LOAD_SHA_UNIT(*(uint64_t *)(input + 2*sizeof(uint64_t)), next);

   if (unlikely(inplen < 16)) 
      goto res_done;
   
   in1 = *data++;
   in2 = *data++;

   /* Loop through input */
   /* Assumed that data is 16 byte aligned */
   if (likely(inplen >= 32)) {
      while (1) {
         CVMX_MF_AES_RESULT (out1, 0);
         CVMX_MF_AES_RESULT (out2, 1);
         CVMX_MT_AES_ENC_CBC0 (in1);
         CVMX_MT_AES_ENC_CBC1 (in2);
         CVM_LOAD2_SHA_UNIT (out1, out2, next);
         res[0] = out1;
         res[1] = out2;
         in1 = data[0];
         in2 = data[1];
         res += 2;
         data += 2;
         CVMX_MF_AES_RESULT (out1, 0);
         CVMX_MF_AES_RESULT (out2, 1);
         CVMX_MT_AES_ENC_CBC0 (in1);
         CVMX_MT_AES_ENC_CBC1 (in2);
         CVM_LOAD2_SHA_UNIT (out1, out2, next);
         res[0] = out1;
         res[1] = out2;
         in1 = data[0];
         in2 = data[1];
         res += 2;
         data += 2;
         inplen -= 32;
         if (inplen < 32)
             break;
      }
   }
   /* inplen < 32  ==> inplen = 16 or inplen = 0 
      (Assuming 16 byte aligned data only) */
   if (inplen) {
       CVMX_MF_AES_RESULT (out1, 0);
       CVMX_MF_AES_RESULT (out2, 1);
       CVMX_MT_AES_ENC_CBC0 (in1);
       CVMX_MT_AES_ENC_CBC1 (in2);
       CVM_LOAD2_SHA_UNIT (out1, out2, next);
       res[0] = out1;
       res[1] = out2;
       res += 2;
   }

res_done:    
    CVMX_MF_AES_RESULT (out1, 0);
    CVMX_MF_AES_RESULT (out2, 1);
    CVM_LOAD2_SHA_UNIT (out1, out2, next);
    res[0] = out1;
    res[1] = out2;


   CVMX_PREFETCH0(start_outer_sha);
   /* Finish Inner hash */
   {
      CVM_LOAD_SHA_UNIT(0x8000000000000000ULL, next);
      while (next != 7) {
         CVM_LOAD_SHA_UNIT(((uint64_t)0x0ULL), next);
      }
      CVM_LOAD_SHA_UNIT((uint64_t)((pktlen + 64) << 3), next);
   } 


   /* Get the inner hash of HMAC */
   CVMX_MF_HSH_IV (in1, 0);
   CVMX_MF_HSH_IV (in2, 1);
   out1 = 0;
   CVMX_MF_HSH_IV (out1, 2);

   /* Initialize hash unit */
   CVMX_MT_HSH_IV (start_outer_sha[0], 0);
   CVMX_MT_HSH_IV (start_outer_sha[1], 1);
   CVMX_MT_HSH_IV (start_outer_sha[2], 2);

   CVMX_MT_HSH_DAT (in1, 0);
   CVMX_MT_HSH_DAT (in2, 1);
   out1 |= 0x0000000080000000;
   CVMX_MT_HSH_DAT (out1, 2);
   CVMX_MT_HSH_DATZ(3);
   CVMX_MT_HSH_DATZ(4);
   CVMX_MT_HSH_DATZ(5);
   CVMX_MT_HSH_DATZ(6);
   CVMX_MT_HSH_STARTSHA((uint64_t)((64 + 20) << 3));

   /* Write the HMAC */
   res = (uint64_t *)(output+pktlen);
   CVMX_MF_HSH_IV (*res++, 0);
   CVMX_MF_HSH_IV (out1, 1);
   *((uint32_t *)res) = (uint32_t)(out1 >> 32);
   CVMX_MF_HSH_IV (out1, 2);
   
	octeon_crypto_disable(&state, flags);
   return 0;

}

int AES_CBC_sha1_decrypt(uint8_t *input, uint8_t *output, uint32_t pktlen, 
                         uint8_t *aes_key, int aes_key_len, uint8_t *aes_iv,
                         uint64_t *start_inner_sha, uint64_t *start_outer_sha)
{
   
   uint64_t *res = (uint64_t *)output, *data = (uint64_t *)input;
   register uint64_t in1, in2, out1, out2;
   register int next = 0;
   int inplen;
	struct octeon_cop2_state state;
	unsigned long flags;
   
   if (unlikely((input == NULL || output == NULL || pktlen == 0 || 
                 aes_key == NULL || aes_iv == NULL || aes_key_len == 0 || 
                 start_inner_sha == NULL || 
                 start_outer_sha == NULL))) {
      printk("%s Wrong parameters \n", __FUNCTION__);   
      return -1;
   }

   CVMX_PREFETCH0(aes_iv);
   CVMX_PREFETCH0(start_inner_sha);
   CVMX_PREFETCH0(data);

	flags = octeon_crypto_enable(&state);

   CVMX_MT_AES_KEY(((uint64_t *)aes_key)[0], 0);
   CVMX_MT_AES_KEY(((uint64_t *)aes_key)[1], 1);

   if (aes_key_len == 16) {
      CVMX_MT_AES_KEY(0x0, 2);
      CVMX_MT_AES_KEY(0x0, 3);
   } else if (aes_key_len == 24) {
      CVMX_MT_AES_KEY (((uint64_t *)aes_key)[2], 2);
      CVMX_MT_AES_KEY (0x0, 3);
   } else if (aes_key_len == 32) {
      CVMX_MT_AES_KEY (((uint64_t *)aes_key)[2], 2);
      CVMX_MT_AES_KEY (((uint64_t *)aes_key)[3], 3);
   } else {
	   octeon_crypto_disable(&state, flags);
      printk("%s: Wrong Key length \n", __FUNCTION__);
      return -1;
   }
   CVMX_MT_AES_KEYLENGTH (aes_key_len / 8  - 1);

   CVMX_MT_AES_IV (((uint64_t *)aes_iv)[0], 0);
   CVMX_MT_AES_IV (((uint64_t *)aes_iv)[1], 1);

   /* Load SHA1 IV */
   CVMX_MT_HSH_IV (start_inner_sha[0], 0);
   CVMX_MT_HSH_IV (start_inner_sha[1], 1);
   CVMX_MT_HSH_IV (start_inner_sha[2], 2);

   data = (uint64_t *)(input + ESP_HEADER_LENGTH + AES_CBC_IV_LENGTH);
   res = (uint64_t *)(output + ESP_HEADER_LENGTH + AES_CBC_IV_LENGTH);
   inplen = pktlen - ESP_HEADER_LENGTH - AES_CBC_IV_LENGTH - 12;

   in1 = *data++;
   in2 = *data++;
   inplen -= 16;

   CVMX_MT_AES_DEC_CBC0 (in1);
   CVMX_MT_AES_DEC_CBC1 (in2);
   
   CVM_LOAD_SHA_UNIT(*(uint64_t *)input, next);
   CVM_LOAD_SHA_UNIT(*(uint64_t *)(input + sizeof(uint64_t)), next);
   CVM_LOAD_SHA_UNIT(*(uint64_t *)(input + 2*sizeof(uint64_t)), next);

   CVM_LOAD2_SHA_UNIT(in1, in2, next);

   if (unlikely(inplen < 16)) 
      goto res_done;
   
   in1 = *data++;
   in2 = *data++;

   /* Loop through input */
   /* Assumed that data is 16 byte aligned */
   if (likely(inplen >= 32)) {
      while (1) {
         CVMX_MF_AES_RESULT (out1, 0);
         CVMX_MF_AES_RESULT (out2, 1);
         CVMX_MT_AES_DEC_CBC0 (in1);
         CVMX_MT_AES_DEC_CBC1 (in2);
         CVM_LOAD2_SHA_UNIT (in1, in2, next);
         res[0] = out1;
         res[1] = out2;
         in1 = data[0];
         in2 = data[1];
         res += 2;
         data += 2;
         CVMX_MF_AES_RESULT (out1, 0);
         CVMX_MF_AES_RESULT (out2, 1);
         CVMX_MT_AES_DEC_CBC0 (in1);
         CVMX_MT_AES_DEC_CBC1 (in2);
         CVM_LOAD2_SHA_UNIT (in1, in2, next);
         res[0] = out1;
         res[1] = out2;
         in1 = data[0];
         in2 = data[1];
         res += 2;
         data += 2;
         inplen -= 32;
         if (inplen < 32)
             break;
      }
   }
   /* inplen < 32  ==> inplen = 16 or inplen = 0 
      (Assuming 16 byte aligned data only) */
   if (inplen) {
       CVMX_MF_AES_RESULT (out1, 0);
       CVMX_MF_AES_RESULT (out2, 1);
       CVMX_MT_AES_DEC_CBC0 (in1);
       CVMX_MT_AES_DEC_CBC1 (in2);
       CVM_LOAD2_SHA_UNIT (in1, in2, next);
       res[0] = out1;
       res[1] = out2;
       res += 2;
   }

res_done:    
    CVMX_MF_AES_RESULT (out1, 0);
    CVMX_MF_AES_RESULT (out2, 1);
    res[0] = out1;
    res[1] = out2;


   pktlen -= 12;
   CVMX_PREFETCH0(start_outer_sha);
   /* Finish Inner hash */
   {
      CVM_LOAD_SHA_UNIT(0x8000000000000000ULL, next);
      while (next != 7) {
         CVM_LOAD_SHA_UNIT(((uint64_t)0x0ULL), next);
      }
      CVM_LOAD_SHA_UNIT((uint64_t)((pktlen + 64) << 3), next);
   } 


   /* Get the inner hash of HMAC */
   CVMX_MF_HSH_IV (in1, 0);
   CVMX_MF_HSH_IV (in2, 1);
   out1 = 0;
   CVMX_MF_HSH_IV (out1, 2);

   CVMX_PREFETCH0(input + pktlen);
   /* Initialize hash unit */
   CVMX_MT_HSH_IV (start_outer_sha[0], 0);
   CVMX_MT_HSH_IV (start_outer_sha[1], 1);
   CVMX_MT_HSH_IV (start_outer_sha[2], 2);

   CVMX_MT_HSH_DAT (in1, 0);
   CVMX_MT_HSH_DAT (in2, 1);
   out1 |= 0x0000000080000000;
   CVMX_MT_HSH_DAT (out1, 2);
   CVMX_MT_HSH_DATZ(3);
   CVMX_MT_HSH_DATZ(4);
   CVMX_MT_HSH_DATZ(5);
   CVMX_MT_HSH_DATZ(6);
   CVMX_MT_HSH_STARTSHA((uint64_t)((64 + 20) << 3));

   /* Get and compare the HMAC */
   {
      res = (uint64_t *)(input+pktlen);
      CVMX_MF_HSH_IV (out1, 0);
      if (unlikely(out1 != *res)) {
			#ifdef OCTEON_IPSEC_DEBUG
         printk("%s: MAC Mismatch/Decrypt Failed\n", __FUNCTION__);
			#endif
	      octeon_crypto_disable(&state, flags);
         return -1;
      }
      CVMX_MF_HSH_IV (out2, 1);
      if (*(uint32_t *)(res + 1) != (uint32_t)(out2 >> 32)) {
			#ifdef OCTEON_IPSEC_DEBUG
         printk("%s: MAC Mismatch/Decrypt Failed\n", __FUNCTION__);
			#endif
	      octeon_crypto_disable(&state, flags);
         return -1;
      }
      CVMX_MF_HSH_IV (out2, 2);
   }
   
	octeon_crypto_disable(&state, flags);
   return 0;

}

int DES_CBC_sha1_encrypt (uint8_t *input, uint8_t *output, uint32_t pktlen, 
                          uint8_t *des_key, int des_key_len, uint8_t *des_iv, 
                          uint64_t *start_inner_sha, uint64_t *start_outer_sha)
{
   uint64_t *res = (uint64_t *)output, *data = (uint64_t *)input;
   register uint64_t in, out, tmp;
   register int next = 0;
   int inplen;
	struct octeon_cop2_state state;
	unsigned long flags;

   if (unlikely(input == NULL || output == NULL || pktlen == 0  || 
                des_key == NULL || des_iv == NULL || 
                start_inner_sha == NULL || start_outer_sha == NULL)) { 
      printk("\n %s: Wrong parameters \n", __FUNCTION__);   
      return -1;
   }

	#ifdef OCTEON_IPSEC_DEBUG
	dump_pkt("First pkt to Enc", input, pktlen);
	dump_pkt("des_key", des_key, des_key_len);
	dump_pkt("des_iv", des_iv, des_key_len);
	dump_pkt("start_inner_sha", (uint8_t *)start_inner_sha, 20);
	dump_pkt("start_outer_sha", (uint8_t *)start_outer_sha, 20);
	#endif
  
   CVMX_PREFETCH0(des_iv);
   CVMX_PREFETCH0(start_inner_sha);
   CVMX_PREFETCH0(data);

	flags = octeon_crypto_enable(&state);

   /* load 3DES Key */
   CVMX_MT_3DES_KEY (((uint64_t *)des_key)[0], 0);
   if (des_key_len == 24) {
      CVMX_MT_3DES_KEY (((uint64_t *)des_key)[1], 1);
      CVMX_MT_3DES_KEY (((uint64_t *)des_key)[2], 2);
   } else if (des_key_len == 8) {
      CVMX_MT_3DES_KEY (((uint64_t *)des_key)[0], 1);
      CVMX_MT_3DES_KEY (((uint64_t *)des_key)[0], 2);
   } else {
      printk("%s: Wrong Key Length %d \n", __FUNCTION__, des_key_len); 
      return -1;
   }

	out = *(uint64_t *)des_iv;
   CVMX_MT_3DES_IV (out);

   /* Load SHA1 IV */
   CVMX_MT_HSH_IV (start_inner_sha[0], 0);
   CVMX_MT_HSH_IV (start_inner_sha[1], 1);
   CVMX_MT_HSH_IV (start_inner_sha[2], 2);

   if (input == output) {
      /* Inplace */
      res = (uint64_t *)(input + ESP_HEADER_LENGTH + DES_CBC_IV_LENGTH);
   } else {
      /* Not Inplace, so copy esp header to output */
      memcpy((uint8_t *)res, input, (ESP_HEADER_LENGTH + DES_CBC_IV_LENGTH));
      res = (uint64_t *)((uint8_t *)res + ESP_HEADER_LENGTH + DES_CBC_IV_LENGTH);
   }
   data = (uint64_t *)(input + ESP_HEADER_LENGTH + DES_CBC_IV_LENGTH);
   inplen = pktlen - ESP_HEADER_LENGTH - DES_CBC_IV_LENGTH;

   in = *data++;
   inplen -= 8;

   CVMX_MT_3DES_ENC_CBC(in);
   
   CVM_LOAD_SHA_UNIT(*(uint64_t *)input, next);
   CVM_LOAD_SHA_UNIT(out, next);


   if (unlikely(inplen < 8))
      goto res_done;
   
   in = *data++;

   /* Loop through input */
   /* Assumed that data is 8 byte aligned */
   if (likely(inplen >= 16)) {
      while (1) {
         CVMX_MF_3DES_RESULT(out);
         CVMX_MT_3DES_ENC_CBC(in);
         CVM_LOAD_SHA_UNIT (out, next);
         res[0] = out;
         in = data[0];
         res++;
         data++;
         CVMX_MF_3DES_RESULT(out);
         CVMX_MT_3DES_ENC_CBC(in);
         CVM_LOAD_SHA_UNIT(out, next);
         res[0] = out;
         in = data[0];
         res++;
         data++;
         inplen -= 16;
         if (inplen < 16)
             break;
      }
   }
   /* inplen < 16 ==> inplen = 8 or inplen = 0 
      (Assuming 8 byte aligned data only) */
   if (inplen) {
       CVMX_MF_3DES_RESULT(out);
       CVMX_MT_3DES_ENC_CBC(in);
       CVM_LOAD_SHA_UNIT (out, next);
       res[0] = out;
       res++;
   }

res_done:    
    CVMX_MF_3DES_RESULT(out);
    CVM_LOAD_SHA_UNIT (out, next);
    res[0] = out;


   CVMX_PREFETCH0(start_outer_sha);
   /* Finish Inner hash */
   {
      CVM_LOAD_SHA_UNIT(0x8000000000000000ULL, next);
      while (next != 7) {
         CVM_LOAD_SHA_UNIT(((uint64_t)0x0ULL), next);
      }
      CVM_LOAD_SHA_UNIT((uint64_t)((pktlen + 64) << 3), next);
   } 


   /* Get the inner hash of HMAC */
   CVMX_MF_HSH_IV (in, 0);
   CVMX_MF_HSH_IV (tmp, 1);
   out = 0;
   CVMX_MF_HSH_IV (out, 2);

   /* Initialize hash unit */
   CVMX_MT_HSH_IV (start_outer_sha[0], 0);
   CVMX_MT_HSH_IV (start_outer_sha[1], 1);
   CVMX_MT_HSH_IV (start_outer_sha[2], 2);

   CVMX_MT_HSH_DAT (in, 0);
   CVMX_MT_HSH_DAT (tmp, 1);
   out |= 0x0000000080000000;
   CVMX_MT_HSH_DAT (out, 2);
   CVMX_MT_HSH_DATZ(3);
   CVMX_MT_HSH_DATZ(4);
   CVMX_MT_HSH_DATZ(5);
   CVMX_MT_HSH_DATZ(6);
   CVMX_MT_HSH_STARTSHA((uint64_t)((64 + 20) << 3));

   /* Get the HMAC */
   res = (uint64_t *)(output+pktlen);
   CVMX_MF_HSH_IV (*res++, 0);
   CVMX_MF_HSH_IV (out, 1);
   *((uint32_t *)res) = (uint32_t)(out >> 32);
   CVMX_MF_HSH_IV (out, 2);
   
	#ifdef OCTEON_IPSEC_DEBUG
	dump_pkt("Encrypted packet", output, pktlen+12);
	#endif
	octeon_crypto_disable(&state, flags);
   return 0;
}


int DES_CBC_sha1_decrypt (uint8_t *input, uint8_t *output, uint32_t pktlen, 
                          uint8_t *des_key, int des_key_len, uint8_t *des_iv, 
                          uint64_t *start_inner_sha, uint64_t *start_outer_sha)
{
   uint64_t *res = (uint64_t *)output, *data = (uint64_t *)input;
   register uint64_t in, out, tmp;
   register int next = 0;
   int inplen;
	struct octeon_cop2_state state;
	unsigned long flags;
   

   if (unlikely(input == NULL || output == NULL || pktlen == 0  || 
                des_key == NULL || des_iv == NULL || 
                start_inner_sha == NULL || start_outer_sha == NULL)) { 
      printk("\n %s: Wrong parameters %d \n", __FUNCTION__, des_key_len);   
      return -1;
   }
  
	#ifdef OCTEON_IPSEC_DEBUG
	dump_pkt("First pkt to Dec", input, pktlen);
	#endif

   CVMX_PREFETCH0(des_iv);
   CVMX_PREFETCH0(start_inner_sha);
   CVMX_PREFETCH0(data);

	flags = octeon_crypto_enable(&state);

   /* load 3DES Key */
   CVMX_MT_3DES_KEY (((uint64_t *)des_key)[0], 0);
   if (des_key_len == 24) {
      CVMX_MT_3DES_KEY (((uint64_t *)des_key)[1], 1);
      CVMX_MT_3DES_KEY (((uint64_t *)des_key)[2], 2);
   } else if (des_key_len == 8) {
      CVMX_MT_3DES_KEY (((uint64_t *)des_key)[0], 1);
      CVMX_MT_3DES_KEY (((uint64_t *)des_key)[0], 2);
   } else {
      printk("%s: Wrong Key Length %d \n", __FUNCTION__, des_key_len); 
	   octeon_crypto_disable(&state, flags);
      return -1;
   }

   CVMX_MT_3DES_IV (*((uint64_t *)des_iv));

   /* Load SHA1 IV */
   CVMX_MT_HSH_IV (start_inner_sha[0], 0);
   CVMX_MT_HSH_IV (start_inner_sha[1], 1);
   CVMX_MT_HSH_IV (start_inner_sha[2], 2);

   data = (uint64_t *)(input + ESP_HEADER_LENGTH + DES_CBC_IV_LENGTH);
   res = (uint64_t *)(output + ESP_HEADER_LENGTH + DES_CBC_IV_LENGTH);
   inplen = pktlen - ESP_HEADER_LENGTH - DES_CBC_IV_LENGTH - 12;

   in = *data++;
   inplen -= 8;

   CVMX_MT_3DES_DEC_CBC(in);
   
   CVM_LOAD_SHA_UNIT(*(uint64_t *)input, next);
   CVM_LOAD_SHA_UNIT(*(uint64_t *)(input + sizeof(uint64_t)), next);

   CVM_LOAD_SHA_UNIT(in, next);

   if (unlikely(inplen < 8)) 
      goto res_done;
   
   in = *data++;

   /* Loop through input */
   /* Assumed that data is 8 byte aligned */
   if (likely(inplen >= 16)) {
      while (1) {
         CVMX_MF_3DES_RESULT(out);
         CVMX_MT_3DES_DEC_CBC(in);
         CVM_LOAD_SHA_UNIT(in, next);
         res[0] = out;
         in = data[0];
         res++;
         data++;
         CVMX_MF_3DES_RESULT(out);
         CVMX_MT_3DES_DEC_CBC(in);
         CVM_LOAD_SHA_UNIT(in, next);
         res[0] = out;
         in = data[0];
         res++;
         data++;
         inplen -= 16;
         if (inplen < 16)
             break;
      }
   }
	#ifdef OCTEON_IPSEC_DEBUG
	dump_pkt("First pkt after Dec", input, pktlen);
	#endif
   /* inplen < 16 ==> inplen = 8 or inplen = 0 
      (Assuming 8 byte aligned data only) */
   if (inplen) {
       CVMX_MF_3DES_RESULT(out);
       CVMX_MT_3DES_DEC_CBC(in);
       CVM_LOAD_SHA_UNIT (in, next);
       res[0] = out;
       res++;
   }

res_done:    
    CVMX_MF_3DES_RESULT(out);
    res[0] = out;


   pktlen -= 12;
   CVMX_PREFETCH0(start_outer_sha);
   /* Finish Inner hash */
   {
       CVM_LOAD_SHA_UNIT(0x8000000000000000ULL, next);
       while (next != 7) {
         CVM_LOAD_SHA_UNIT(((uint64_t)0x0ULL), next);
       }
       CVM_LOAD_SHA_UNIT((uint64_t)((pktlen + 64) << 3), next);
   } 


   /* Get the inner hash of HMAC */
   CVMX_MF_HSH_IV (in, 0);
   CVMX_MF_HSH_IV (tmp, 1);
   out = 0;
   CVMX_MF_HSH_IV (out, 2);

   CVMX_PREFETCH0(input + pktlen);
   /* Initialize hash unit */
   CVMX_MT_HSH_IV (start_outer_sha[0], 0);
   CVMX_MT_HSH_IV (start_outer_sha[1], 1);
   CVMX_MT_HSH_IV (start_outer_sha[2], 2);

   CVMX_MT_HSH_DAT (in, 0);
   CVMX_MT_HSH_DAT (tmp, 1);
   out |= 0x0000000080000000;
   CVMX_MT_HSH_DAT (out, 2);
   CVMX_MT_HSH_DATZ(3);
   CVMX_MT_HSH_DATZ(4);
   CVMX_MT_HSH_DATZ(5);
   CVMX_MT_HSH_DATZ(6);
   CVMX_MT_HSH_STARTSHA((uint64_t)((64 + 20) << 3));

   /* Get and compare the HMAC */
   {
      res = (uint64_t *)(input+pktlen);
      CVMX_MF_HSH_IV (out, 0);
      if (unlikely(out != *res)) {
			#ifdef OCTEON_IPSEC_DEBUG
			printk("%s: MAC Mismatch failed  %lx \n", __FUNCTION__, res);
			#endif
	      octeon_crypto_disable(&state, flags);
         return -1;
      }
      CVMX_MF_HSH_IV (out, 1);
      if (*(uint32_t *)(res + 1) != (uint32_t)(out >> 32)) {
		   #ifdef OCTEON_IPSEC_DEBUG
			printk("%s: MAC Mismatch 1 failed  %lx \n", __FUNCTION__, res);
			#endif
	      octeon_crypto_disable(&state, flags);
         return -1;
      }
   }
   
	octeon_crypto_disable(&state, flags);
   return 0;
}

int DES_CBC_md5_encrypt(uint8_t *input, uint8_t *output, uint32_t pktlen, 
                        uint8_t *des_key, int des_key_len, uint8_t *des_iv, 
                        uint64_t *start_inner_md5, uint64_t *start_outer_md5)
{
   uint64_t *res = (uint64_t *)output, *data = (uint64_t *)input;
   register uint64_t in, out;
   register int next = 0;
   int inplen;
	struct octeon_cop2_state state;
	unsigned long flags;
   

   if (unlikely(input == NULL || output == NULL || pktlen == 0  || 
                des_key == NULL || des_iv == NULL || 
                start_inner_md5 == NULL || start_outer_md5 == NULL)) { 
      printk("\n %s: Wrong parameters \n", __FUNCTION__);   
      return -1;
   }
  
   CVMX_PREFETCH0(des_iv);
   CVMX_PREFETCH0(start_inner_md5);
   CVMX_PREFETCH0(data);

	flags = octeon_crypto_enable(&state);

   /* load 3DES Key */
   CVMX_MT_3DES_KEY (((uint64_t *)des_key)[0], 0);
   if (des_key_len == 24) {
      CVMX_MT_3DES_KEY (((uint64_t *)des_key)[1], 1);
      CVMX_MT_3DES_KEY (((uint64_t *)des_key)[2], 2);
   } else if (des_key_len == 8) {
      CVMX_MT_3DES_KEY (((uint64_t *)des_key)[0], 1);
      CVMX_MT_3DES_KEY (((uint64_t *)des_key)[0], 2);
   } else {
	   octeon_crypto_disable(&state, flags);
      printk("%s: Wrong Key Length \n", __FUNCTION__); 
      return -1;
   }

	out = *(uint64_t *)des_iv;
   CVMX_MT_3DES_IV (out);

   /* Load MD5 IV */
   CVMX_MT_HSH_IV (start_inner_md5[0], 0);
   CVMX_MT_HSH_IV (start_inner_md5[1], 1);

   /* Inplace */
   res = (uint64_t *)(input + ESP_HEADER_LENGTH + DES_CBC_IV_LENGTH);
   data = (uint64_t *)(input + ESP_HEADER_LENGTH + DES_CBC_IV_LENGTH);
   inplen = pktlen - ESP_HEADER_LENGTH - DES_CBC_IV_LENGTH;

   in = *data++;
   inplen -= 8;

   CVMX_MT_3DES_ENC_CBC(in);
   
   CVM_LOAD_MD5_UNIT(*(uint64_t *)input, next);
   CVM_LOAD_MD5_UNIT(out, next);

   if (unlikely(inplen < 8)) 
      goto res_done;
   
   in = *data++;

   /* Loop through input */
   /* Assumed that data is 8 byte aligned */
   if (likely(inplen >= 16)) {
      while (1) {
         CVMX_MF_3DES_RESULT(out);
         CVMX_MT_3DES_ENC_CBC(in);
         CVM_LOAD_MD5_UNIT (out, next);
         res[0] = out;
         in = data[0];
         res++;
         data++;
         CVMX_MF_3DES_RESULT(out);
         CVMX_MT_3DES_ENC_CBC(in);
         CVM_LOAD_MD5_UNIT(out, next);
         res[0] = out;
         in = data[0];
         res++;
         data++;
         inplen -= 16;
         if (inplen < 16)
             break;
      }
   }
   /* inplen < 16 ==> inplen = 8 or inplen = 0 
      (Assuming 8 byte aligned data only) */
   if (inplen) {
       CVMX_MF_3DES_RESULT(out);
       CVMX_MT_3DES_ENC_CBC(in);
       CVM_LOAD_MD5_UNIT (out, next);
       res[0] = out;
       res++;
   }

res_done:    
    CVMX_MF_3DES_RESULT(out);
    CVM_LOAD_MD5_UNIT (out, next);
    res[0] = out;


   CVMX_PREFETCH0(start_outer_md5);
   /* Finish Inner hash */
   {
      CVM_LOAD_MD5_UNIT(0x8000000000000000ULL, next);
      while (next != 7) {
         CVM_LOAD_MD5_UNIT(((uint64_t)0x0ULL), next);
      }
      CVM_LOAD_MD5_UNIT(swap64((uint64_t)((pktlen + 64) << 3)), next);
   } 


   /* Get the inner hash of HMAC */
   CVMX_MF_HSH_IV (in, 0);
   CVMX_MF_HSH_IV (out, 1);

   /* Initialize hash unit */
   CVMX_MT_HSH_IV (start_outer_md5[0], 0);
   CVMX_MT_HSH_IV (start_outer_md5[1], 1);

   CVMX_MT_HSH_DAT (in, 0);
   CVMX_MT_HSH_DAT (out, 1);
   CVMX_MT_HSH_DAT (0x8000000000000000ULL, 2);
   CVMX_MT_HSH_DATZ(3);
   CVMX_MT_HSH_DATZ(4);
   CVMX_MT_HSH_DATZ(5);
   CVMX_MT_HSH_DATZ(6);
   CVMX_MT_HSH_STARTMD5(swap64((uint64_t)((64 + 16) << 3)));

   /* Get the HMAC */
   res = (uint64_t *)(output+pktlen);
   CVMX_MF_HSH_IV (*res++, 0);
   CVMX_MF_HSH_IV (out, 1);
   *((uint32_t *)res) = (uint32_t)(out >> 32);
   
	octeon_crypto_disable(&state, flags);
   return 0;
}


int DES_CBC_md5_decrypt (uint8_t *input, uint8_t *output, uint32_t pktlen, 
                         uint8_t *des_key, int des_key_len, uint8_t *des_iv, 
                         uint64_t *start_inner_md5, uint64_t *start_outer_md5)
{
   uint64_t *res = (uint64_t *)output, *data = (uint64_t *)input;
   register uint64_t in, out;
   register int next = 0;
   int inplen;
	struct octeon_cop2_state state;
	unsigned long flags;
   

   if (unlikely(input == NULL || output == NULL || pktlen == 0  || 
                des_key == NULL || des_iv == NULL || 
                start_inner_md5 == NULL || start_outer_md5 == NULL)) { 
      printk("\n %s: Wrong parameters \n", __FUNCTION__);   
      return -1;
   }
  
   CVMX_PREFETCH0(des_iv);
   CVMX_PREFETCH0(start_inner_md5);
   CVMX_PREFETCH0(data);

	flags = octeon_crypto_enable(&state);

   /* load 3DES Key */
   CVMX_MT_3DES_KEY (((uint64_t *)des_key)[0], 0);
   if (des_key_len == 24) {
      CVMX_MT_3DES_KEY (((uint64_t *)des_key)[1], 1);
      CVMX_MT_3DES_KEY (((uint64_t *)des_key)[2], 2);
   } else if (des_key_len == 8) {
      CVMX_MT_3DES_KEY (((uint64_t *)des_key)[0], 1);
      CVMX_MT_3DES_KEY (((uint64_t *)des_key)[0], 2);
   } else {
      printk("%s: Wrong Key Length \n", __FUNCTION__); 
      return -1;
   }

   CVMX_MT_3DES_IV (*((uint64_t *)des_iv));

   /* Load MD5 IV */
   CVMX_MT_HSH_IV (start_inner_md5[0], 0);
   CVMX_MT_HSH_IV (start_inner_md5[1], 1);

   data = (uint64_t *)(input + ESP_HEADER_LENGTH + DES_CBC_IV_LENGTH);
   res = (uint64_t *)(output + ESP_HEADER_LENGTH + DES_CBC_IV_LENGTH);
   inplen = pktlen - ESP_HEADER_LENGTH - DES_CBC_IV_LENGTH - 12;

   in = *data++;
   inplen -= 8;

   CVMX_MT_3DES_DEC_CBC(in);
   
   CVM_LOAD_MD5_UNIT(*(uint64_t *)input, next);
   CVM_LOAD_MD5_UNIT(*(uint64_t *)(input + sizeof(uint64_t)), next);

   CVM_LOAD_MD5_UNIT(in, next);

   if (unlikely(inplen < 8)) 
      goto res_done;
   
   in = *data++;

   /* Loop through input */
   /* Assumed that data is 8 byte aligned */
   if (likely(inplen >= 16)) {
      while (1) {
         CVMX_MF_3DES_RESULT(out);
         CVMX_MT_3DES_DEC_CBC(in);
         CVM_LOAD_MD5_UNIT(in, next);
         res[0] = out;
         in = data[0];
         res++;
         data++;
         CVMX_MF_3DES_RESULT(out);
         CVMX_MT_3DES_DEC_CBC(in);
         CVM_LOAD_MD5_UNIT(in, next);
         res[0] = out;
         in = data[0];
         res++;
         data++;
         inplen -= 16;
         if (inplen < 16)
             break;
      }
   }
   /* inplen < 16 ==> inplen = 8 or inplen = 0 
      (Assuming 8 byte aligned data only) */
   if (inplen) {
       CVMX_MF_3DES_RESULT(out);
       CVMX_MT_3DES_DEC_CBC(in);
       CVM_LOAD_MD5_UNIT (in, next);
       res[0] = out;
       res++;
   }

res_done:    
    CVMX_MF_3DES_RESULT(out);
    res[0] = out;


   pktlen -= 12;
   CVMX_PREFETCH0(start_outer_md5);
   /* Finish Inner hash */
   {
      CVM_LOAD_MD5_UNIT(0x8000000000000000ULL, next);
      while (next != 7) {
         CVM_LOAD_MD5_UNIT(((uint64_t)0x0ULL), next);
      }
      CVM_LOAD_MD5_UNIT(swap64((uint64_t)((pktlen + 64) << 3)), next);
   } 


   /* Get the inner hash of HMAC */
   CVMX_MF_HSH_IV (in, 0);
   CVMX_MF_HSH_IV (out, 1);

   CVMX_PREFETCH0(input + pktlen);
   /* Initialize hash unit */
   CVMX_MT_HSH_IV (start_outer_md5[0], 0);
   CVMX_MT_HSH_IV (start_outer_md5[1], 1);

   CVMX_MT_HSH_DAT (in, 0);
   CVMX_MT_HSH_DAT (out, 1);
   CVMX_MT_HSH_DAT (0x8000000000000000ULL, 2);
   CVMX_MT_HSH_DATZ(3);
   CVMX_MT_HSH_DATZ(4);
   CVMX_MT_HSH_DATZ(5);
   CVMX_MT_HSH_DATZ(6);
   CVMX_MT_HSH_STARTMD5(swap64((uint64_t)((64 + 16) << 3)));

   /* Get and compare the HMAC */
   {
      res = (uint64_t *)(input+pktlen);
      CVMX_MF_HSH_IV (out, 0);
      if (unlikely(out != *res)) {
			#ifdef OCTEON_IPSEC_DEBUG
         printk("%s: MAC Mismatch/Decrypt Failed \n", __FUNCTION__);
			#endif
	      octeon_crypto_disable(&state, flags);
         return -1;
      }
      CVMX_MF_HSH_IV (out, 1);
      if (*(uint32_t *)(res + 1) != (uint32_t)(out >> 32)) {
			#ifdef OCTEON_IPSEC_DEBUG
         printk("%s: MAC Mismatch/Decrypt Failed \n", __FUNCTION__);
			#endif
	      octeon_crypto_disable(&state, flags);
         return -1;
      }
   }
   
	octeon_crypto_disable(&state, flags);
   return 0;
}

int DES_CBC_encrypt(uint8_t *input, uint8_t *output, uint32_t pktlen, 
                    uint8_t *des_key, int des_key_len, uint8_t *des_iv, 
                    uint64_t *unused1, uint64_t *unused2)
{
   uint64_t *res = (uint64_t *)output, *data = (uint64_t *)input;
   register uint64_t in, out;
   int inplen;
	struct octeon_cop2_state state;
	unsigned long flags;
   

   if (unlikely(input == NULL || output == NULL || pktlen == 0  || 
                des_key == NULL || des_iv == NULL)) { 
      printk("\n %s: Wrong parameters \n", __FUNCTION__);   
      return -1;
   }
  
   CVMX_PREFETCH0(des_iv);
   CVMX_PREFETCH0(data);

	flags = octeon_crypto_enable(&state);

   /* load 3DES Key */
   CVMX_MT_3DES_KEY (((uint64_t *)des_key)[0], 0);
   if (des_key_len == 24) {
      CVMX_MT_3DES_KEY (((uint64_t *)des_key)[1], 1);
      CVMX_MT_3DES_KEY (((uint64_t *)des_key)[2], 2);
   } else if (des_key_len == 8) {
      CVMX_MT_3DES_KEY (((uint64_t *)des_key)[0], 1);
      CVMX_MT_3DES_KEY (((uint64_t *)des_key)[0], 2);
   } else {
      printk("%s: Wrong Key Length \n", __FUNCTION__); 
      return -1;
   }

	out = *(uint64_t *)des_iv;
   CVMX_MT_3DES_IV (out);

   /* Inplace */
   res = (uint64_t *)(input + ESP_HEADER_LENGTH + DES_CBC_IV_LENGTH);
   data = (uint64_t *)(input + ESP_HEADER_LENGTH + DES_CBC_IV_LENGTH);
   inplen = pktlen - ESP_HEADER_LENGTH - DES_CBC_IV_LENGTH;

   in = *data++;
   inplen -= 8;

   CVMX_MT_3DES_ENC_CBC(in);
   
   if (unlikely(inplen < 8)) 
      goto res_done;
   
   in = *data++;

   /* Loop through input */
   /* Assumed that data is 8 byte aligned */
   if (likely(inplen >= 16)) {
      while (1) {
         CVMX_MF_3DES_RESULT(out);
         CVMX_MT_3DES_ENC_CBC(in);
         res[0] = out;
         in = data[0];
         res++;
         data++;
         CVMX_MF_3DES_RESULT(out);
         CVMX_MT_3DES_ENC_CBC(in);
         res[0] = out;
         in = data[0];
         res++;
         data++;
         inplen -= 16;
         if (inplen < 16)
             break;
      }
   }
   /* inplen < 16 ==> inplen = 8 or inplen = 0 
      (Assuming 8 byte aligned data only) */
   if (inplen) {
       CVMX_MF_3DES_RESULT(out);
       CVMX_MT_3DES_ENC_CBC(in);
       res[0] = out;
       res++;
   }

res_done:    
    CVMX_MF_3DES_RESULT(out);
    res[0] = out;

	octeon_crypto_disable(&state, flags);
   return 0;
}


int DES_CBC_decrypt (uint8_t *input, uint8_t *output, uint32_t pktlen, 
                     uint8_t *des_key, int des_key_len, uint8_t *des_iv, 
                     uint64_t *unused1, uint64_t *unused2)
{
   uint64_t *res = (uint64_t *)output, *data = (uint64_t *)input;
   register uint64_t in, out;
   int inplen;
	struct octeon_cop2_state state;
	unsigned long flags;
   

   if (unlikely(input == NULL || output == NULL || pktlen == 0  || 
                des_key == NULL || des_iv == NULL)) {  
      printk("\n %s: Wrong parameters \n", __FUNCTION__);   
      return -1;
   }
  
   CVMX_PREFETCH0(des_iv);
   CVMX_PREFETCH0(data);

	flags = octeon_crypto_enable(&state);

   /* load 3DES Key */
   CVMX_MT_3DES_KEY (((uint64_t *)des_key)[0], 0);
   if (des_key_len == 24) {
      CVMX_MT_3DES_KEY (((uint64_t *)des_key)[1], 1);
      CVMX_MT_3DES_KEY (((uint64_t *)des_key)[2], 2);
   } else if (des_key_len == 8) {
      CVMX_MT_3DES_KEY (((uint64_t *)des_key)[0], 1);
      CVMX_MT_3DES_KEY (((uint64_t *)des_key)[0], 2);
   } else {
	   octeon_crypto_disable(&state, flags);
      printk("%s: Wrong Key Length \n", __FUNCTION__); 
      return -1;
   }

   CVMX_MT_3DES_IV (*((uint64_t *)des_iv));

   data = (uint64_t *)(input + ESP_HEADER_LENGTH + DES_CBC_IV_LENGTH);
   res = (uint64_t *)(output + ESP_HEADER_LENGTH + DES_CBC_IV_LENGTH);
   inplen = pktlen - ESP_HEADER_LENGTH - DES_CBC_IV_LENGTH;

   in = *data++;
   inplen -= 8;

   CVMX_MT_3DES_DEC_CBC(in);
   
   if (unlikely(inplen < 8)) 
      goto res_done;
   
   in = *data++;

   /* Loop through input */
   /* Assumed that data is 8 byte aligned */
   if (likely(inplen >= 16)) {
      while (1) {
         CVMX_MF_3DES_RESULT(out);
         CVMX_MT_3DES_DEC_CBC(in);
         res[0] = out;
         in = data[0];
         res++;
         data++;
         CVMX_MF_3DES_RESULT(out);
         CVMX_MT_3DES_DEC_CBC(in);
         res[0] = out;
         in = data[0];
         res++;
         data++;
         inplen -= 16;
         if (inplen < 16)
             break;
      }
   }
   /* inplen < 16 ==> inplen = 8 or inplen = 0 
      (Assuming 8 byte aligned data only) */
   if (inplen) {
       CVMX_MF_3DES_RESULT(out);
       CVMX_MT_3DES_DEC_CBC(in);
       res[0] = out;
       res++;
   }

res_done:    
    CVMX_MF_3DES_RESULT(out);
    res[0] = out;

	octeon_crypto_disable(&state, flags);
   return 0;
}

int AES_CBC_encrypt(uint8_t *input, uint8_t *output, uint32_t pktlen, 
                    uint8_t *aes_key, int aes_key_len, uint8_t *aes_iv,
                    uint64_t *unused1, uint64_t *unused2)
{
   
   uint64_t *res = (uint64_t *)output, *data = (uint64_t *)input;
   register uint64_t in1, in2, out1, out2;
   int inplen;
	struct octeon_cop2_state state;
	unsigned long flags;
   
   if (unlikely((input == NULL || output == NULL || pktlen == 0 || 
                 aes_key == NULL || aes_iv == NULL || aes_key_len == 0))) {  
      printk("%s Wrong parameters \n", __FUNCTION__);   
      return -1;
   }

   CVMX_PREFETCH0(aes_iv);
   CVMX_PREFETCH0(data);

	flags = octeon_crypto_enable(&state);

   CVMX_MT_AES_KEY(((uint64_t *)aes_key)[0], 0);
   CVMX_MT_AES_KEY(((uint64_t *)aes_key)[1], 1);

   if (aes_key_len == 16) {
      CVMX_MT_AES_KEY(0x0, 2);
      CVMX_MT_AES_KEY(0x0, 3);
   } else if (aes_key_len == 24) {
      CVMX_MT_AES_KEY (((uint64_t *)aes_key)[2], 2);
      CVMX_MT_AES_KEY (0x0, 3);
   } else if (aes_key_len == 32) {
      CVMX_MT_AES_KEY (((uint64_t *)aes_key)[2], 2);
      CVMX_MT_AES_KEY (((uint64_t *)aes_key)[3], 3);
   } else {
	   octeon_crypto_disable(&state, flags);
      printk("%s: Wrong Key length \n", __FUNCTION__);
      return -1;
   }
   CVMX_MT_AES_KEYLENGTH (aes_key_len / 8 - 1);

   CVMX_MT_AES_IV (((uint64_t *)aes_iv)[0], 0);
   CVMX_MT_AES_IV (((uint64_t *)aes_iv)[1], 1);

   /* Inplace */
   res = (uint64_t *)(input + ESP_HEADER_LENGTH + AES_CBC_IV_LENGTH);
   data = (uint64_t *)(input + ESP_HEADER_LENGTH + AES_CBC_IV_LENGTH);
   inplen = pktlen - ESP_HEADER_LENGTH - AES_CBC_IV_LENGTH;

   in1 = *data++;
   in2 = *data++;
   inplen -= 16;

   CVMX_MT_AES_ENC_CBC0 (in1);
   CVMX_MT_AES_ENC_CBC1 (in2);
   
   if (unlikely(inplen < 16)) 
      goto res_done;
   
   in1 = *data++;
   in2 = *data++;

   /* Loop through input */
   /* Assumed that data is 16 byte aligned */
   if (likely(inplen >= 32)) {
      while (1) {
         CVMX_MF_AES_RESULT (out1, 0);
         CVMX_MF_AES_RESULT (out2, 1);
         CVMX_MT_AES_ENC_CBC0 (in1);
         CVMX_MT_AES_ENC_CBC1 (in2);
         res[0] = out1;
         res[1] = out2;
         in1 = data[0];
         in2 = data[1];
         res += 2;
         data += 2;
         CVMX_MF_AES_RESULT (out1, 0);
         CVMX_MF_AES_RESULT (out2, 1);
         CVMX_MT_AES_ENC_CBC0 (in1);
         CVMX_MT_AES_ENC_CBC1 (in2);
         res[0] = out1;
         res[1] = out2;
         in1 = data[0];
         in2 = data[1];
         res += 2;
         data += 2;
         inplen -= 32;
         if (inplen < 32)
             break;
      }
   }
   /* inplen < 32  ==> inplen = 16 or inplen = 0 
      (Assuming 16 byte aligned data only) */
   if (inplen) {
       CVMX_MF_AES_RESULT (out1, 0);
       CVMX_MF_AES_RESULT (out2, 1);
       CVMX_MT_AES_ENC_CBC0 (in1);
       CVMX_MT_AES_ENC_CBC1 (in2);
       res[0] = out1;
       res[1] = out2;
       res += 2;
   }

res_done:    
    CVMX_MF_AES_RESULT (out1, 0);
    CVMX_MF_AES_RESULT (out2, 1);
    res[0] = out1;
    res[1] = out2;

	octeon_crypto_disable(&state, flags);
   return 0;
}

int AES_CBC_decrypt(uint8_t *input, uint8_t *output, uint32_t pktlen, 
                    uint8_t *aes_key, int aes_key_len, uint8_t *aes_iv,
                    uint64_t *unused1, uint64_t *unused2)
{
   
   uint64_t *res = (uint64_t *)output, *data = (uint64_t *)input;
   register uint64_t in1, in2, out1, out2;
   int inplen;
	struct octeon_cop2_state state;
	unsigned long flags;
   
   if (unlikely((input == NULL || output == NULL || pktlen == 0 || 
                 aes_key == NULL || aes_iv == NULL || aes_key_len == 0))) { 
      printk("%s Wrong parameters \n", __FUNCTION__);   
      return -1;
   }


   CVMX_PREFETCH0(aes_iv);
   CVMX_PREFETCH0(data);

	flags = octeon_crypto_enable(&state);

   CVMX_MT_AES_KEY(((uint64_t *)aes_key)[0], 0);
   CVMX_MT_AES_KEY(((uint64_t *)aes_key)[1], 1);

   if (aes_key_len == 16) {
      CVMX_MT_AES_KEY(0x0, 2);
      CVMX_MT_AES_KEY(0x0, 3);
   } else if (aes_key_len == 24) {
      CVMX_MT_AES_KEY (((uint64_t *)aes_key)[2], 2);
      CVMX_MT_AES_KEY (0x0, 3);
   } else if (aes_key_len == 32) {
      CVMX_MT_AES_KEY (((uint64_t *)aes_key)[2], 2);
      CVMX_MT_AES_KEY (((uint64_t *)aes_key)[3], 3);
   } else {
	   octeon_crypto_disable(&state, flags);
      printk("%s: Wrong Key length \n", __FUNCTION__);
      return -1;
   }
   CVMX_MT_AES_KEYLENGTH (aes_key_len / 8 - 1);

   CVMX_MT_AES_IV (((uint64_t *)aes_iv)[0], 0);
   CVMX_MT_AES_IV (((uint64_t *)aes_iv)[1], 1);

   data = (uint64_t *)(input + ESP_HEADER_LENGTH + AES_CBC_IV_LENGTH);
   res = (uint64_t *)(output + ESP_HEADER_LENGTH + AES_CBC_IV_LENGTH);
   inplen = pktlen - ESP_HEADER_LENGTH - AES_CBC_IV_LENGTH;

   in1 = *data++;
   in2 = *data++;
   inplen -= 16;

   CVMX_MT_AES_DEC_CBC0 (in1);
   CVMX_MT_AES_DEC_CBC1 (in2);
   
   if (unlikely(inplen < 16)) 
      goto res_done;
   
   in1 = *data++;
   in2 = *data++;

   /* Loop through input */
   /* Assumed that data is 16 byte aligned */
   if (likely(inplen >= 32)) {
      while (1) {
         CVMX_MF_AES_RESULT (out1, 0);
         CVMX_MF_AES_RESULT (out2, 1);
         CVMX_MT_AES_DEC_CBC0 (in1);
         CVMX_MT_AES_DEC_CBC1 (in2);
         res[0] = out1;
         res[1] = out2;
         in1 = data[0];
         in2 = data[1];
         res += 2;
         data += 2;
         CVMX_MF_AES_RESULT (out1, 0);
         CVMX_MF_AES_RESULT (out2, 1);
         CVMX_MT_AES_DEC_CBC0 (in1);
         CVMX_MT_AES_DEC_CBC1 (in2);
         res[0] = out1;
         res[1] = out2;
         in1 = data[0];
         in2 = data[1];
         res += 2;
         data += 2;
         inplen -= 32;
         if (inplen < 32)
             break;
      }
   }
   /* inplen < 32  ==> inplen = 16 or inplen = 0 
      (Assuming 16 byte aligned data only) */
   if (inplen) {
       CVMX_MF_AES_RESULT (out1, 0);
       CVMX_MF_AES_RESULT (out2, 1);
       CVMX_MT_AES_DEC_CBC0 (in1);
       CVMX_MT_AES_DEC_CBC1 (in2);
       res[0] = out1;
       res[1] = out2;
       res += 2;
   }

res_done:    
    CVMX_MF_AES_RESULT (out1, 0);
    CVMX_MF_AES_RESULT (out2, 1);
    res[0] = out1;
    res[1] = out2;

	octeon_crypto_disable(&state, flags);
   return 0;
}

int NULL_sha1_encrypt (uint8_t *input, uint8_t *output, uint32_t pktlen, 
           			     uint8_t *unused1, int unused2, uint8_t *unused3, 
                       uint64_t *start_inner_sha, uint64_t *start_outer_sha)
{
   uint64_t *res = (uint64_t *)output, *data = (uint64_t *)input;
   register uint64_t in, out, tmp;
   register int next = 0;
   int inplen = pktlen;
	struct octeon_cop2_state state;
	unsigned long flags;
   

   if (unlikely(input == NULL || output == NULL || pktlen == 0  || 
                start_inner_sha == NULL || start_outer_sha == NULL)) { 
      printk("\n %s: Wrong parameters \n", __FUNCTION__);   
      return -1;
   }
  
   CVMX_PREFETCH0(start_inner_sha);
   CVMX_PREFETCH0(data);

	flags = octeon_crypto_enable(&state);

   /* Load SHA1 IV */
   CVMX_MT_HSH_IV (start_inner_sha[0], 0);
   CVMX_MT_HSH_IV (start_inner_sha[1], 1);
   CVMX_MT_HSH_IV (start_inner_sha[2], 2);

   while (inplen >= 8) {
		in = *data++;
		CVM_LOAD_SHA_UNIT(in, next);
		inplen -= 8;
	}

   CVMX_PREFETCH0(start_outer_sha);
   if (unlikely(inplen)) {
		uint64_t tmp = 0;
		uint8_t *p = (uint8_t *)&tmp;
		p[inplen] = 0x80;
		do {
			inplen--;
			p[inplen] = ((uint8_t *)data)[inplen];
		} while (inplen);
      CVM_LOAD_SHA_UNIT (tmp, next);
   } else {
      CVM_LOAD_SHA_UNIT(0x8000000000000000ULL, next);
	}

   /* Finish Inner hash */
   while (next != 7) {
   	CVM_LOAD_SHA_UNIT(((uint64_t)0x0ULL), next);
   }
   CVM_LOAD_SHA_UNIT((uint64_t)((pktlen + 64) << 3), next);

   /* Get the inner hash of HMAC */
   CVMX_MF_HSH_IV (in, 0);
   CVMX_MF_HSH_IV (tmp, 1);
   out = 0;
   CVMX_MF_HSH_IV (out, 2);

   /* Initialize hash unit */
   CVMX_MT_HSH_IV (start_outer_sha[0], 0);
   CVMX_MT_HSH_IV (start_outer_sha[1], 1);
   CVMX_MT_HSH_IV (start_outer_sha[2], 2);

   CVMX_MT_HSH_DAT (in, 0);
   CVMX_MT_HSH_DAT (tmp, 1);
   out |= 0x0000000080000000;
   CVMX_MT_HSH_DAT (out, 2);
   CVMX_MT_HSH_DATZ(3);
   CVMX_MT_HSH_DATZ(4);
   CVMX_MT_HSH_DATZ(5);
   CVMX_MT_HSH_DATZ(6);
   CVMX_MT_HSH_STARTSHA((uint64_t)((64 + 20) << 3));

   /* Get the HMAC */
   res = (uint64_t *)(output+pktlen);
   CVMX_MF_HSH_IV (*res++, 0);
   CVMX_MF_HSH_IV (out, 1);
   *((uint32_t *)res) = (uint32_t)(out >> 32);
   CVMX_MF_HSH_IV (out, 2);
   
	octeon_crypto_disable(&state, flags);
   return 0;
}


int NULL_sha1_decrypt (uint8_t *input, uint8_t *output, uint32_t pktlen, 
           			   uint8_t *unused1, int unused2, uint8_t *unused3, 
                       uint64_t *start_inner_sha, uint64_t *start_outer_sha)
{
   uint64_t *res = (uint64_t *)output, *data = (uint64_t *)input;
   register uint64_t in, out, tmp;
   register int next = 0;
   int inplen = pktlen - 12;
	struct octeon_cop2_state state;
	unsigned long flags;
   

   if (unlikely(input == NULL || output == NULL || pktlen == 0  || 
                start_inner_sha == NULL || start_outer_sha == NULL)) { 
      printk("\n %s: Wrong parameters \n", __FUNCTION__);   
      return -1;
   }
  
   CVMX_PREFETCH0(start_inner_sha);
   CVMX_PREFETCH0(data);

	flags = octeon_crypto_enable(&state);

   /* Load SHA1 IV */
   CVMX_MT_HSH_IV (start_inner_sha[0], 0);
   CVMX_MT_HSH_IV (start_inner_sha[1], 1);
   CVMX_MT_HSH_IV (start_inner_sha[2], 2);

   while (inplen >= 8) {
		in = *data++;
		CVM_LOAD_SHA_UNIT(in, next);
		inplen -= 8;
	}

   CVMX_PREFETCH0(start_outer_sha);
   if (unlikely(inplen)) {
		uint64_t tmp = 0;
		uint8_t *p = (uint8_t *)&tmp;
		p[inplen] = 0x80;
		do {
			inplen--;
			p[inplen] = ((uint8_t *)data)[inplen];
		} while (inplen);
      CVM_LOAD_SHA_UNIT (tmp, next);
   } else {
      CVM_LOAD_SHA_UNIT(0x8000000000000000ULL, next);
	}

	pktlen -= 12;
   /* Finish Inner hash */
   while (next != 7) {
   	CVM_LOAD_SHA_UNIT(((uint64_t)0x0ULL), next);
   }
   CVM_LOAD_SHA_UNIT((uint64_t)((pktlen + 64) << 3), next);

   /* Get the inner hash of HMAC */
   CVMX_MF_HSH_IV (in, 0);
   CVMX_MF_HSH_IV (tmp, 1);
   out = 0;
   CVMX_MF_HSH_IV (out, 2);

   /* Initialize hash unit */
   CVMX_MT_HSH_IV (start_outer_sha[0], 0);
   CVMX_MT_HSH_IV (start_outer_sha[1], 1);
   CVMX_MT_HSH_IV (start_outer_sha[2], 2);

   CVMX_MT_HSH_DAT (in, 0);
   CVMX_MT_HSH_DAT (tmp, 1);
   out |= 0x0000000080000000;
   CVMX_MT_HSH_DAT (out, 2);
   CVMX_MT_HSH_DATZ(3);
   CVMX_MT_HSH_DATZ(4);
   CVMX_MT_HSH_DATZ(5);
   CVMX_MT_HSH_DATZ(6);
   CVMX_MT_HSH_STARTSHA((uint64_t)((64 + 20) << 3));

   /* Get and compare the HMAC */
   {
      res = (uint64_t *)(input+pktlen);
      CVMX_MF_HSH_IV (out, 0);
      if (unlikely(out != *res)) {
	      octeon_crypto_disable(&state, flags);
         return -1;
      }
      CVMX_MF_HSH_IV (out, 1);
      if (*(uint32_t *)(res + 1) != (uint32_t)(out >> 32)) {
	      octeon_crypto_disable(&state, flags);
         return -1;
      }
   }
   
	octeon_crypto_disable(&state, flags);
   return 0;
}

int NULL_md5_encrypt (uint8_t *input, uint8_t *output, uint32_t pktlen, 
           			  uint8_t *unused1, int unused2, uint8_t *unused3, 
                      uint64_t *start_inner_md5, uint64_t *start_outer_md5)
{
   uint64_t *res = (uint64_t *)output, *data = (uint64_t *)input;
   register uint64_t in, out, tmp;
   register int next = 0;
   int inplen = pktlen;
	struct octeon_cop2_state state;
	unsigned long flags;
   

   if (unlikely(input == NULL || output == NULL || pktlen == 0  || 
                start_inner_md5 == NULL || start_outer_md5 == NULL)) { 
      printk("\n %s: Wrong parameters \n", __FUNCTION__);   
      return -1;
   }
  
   CVMX_PREFETCH0(start_inner_md5);
   CVMX_PREFETCH0(data);

	flags = octeon_crypto_enable(&state);

   /* Load MD5 IV */
   CVMX_MT_HSH_IV (start_inner_md5[0], 0);
   CVMX_MT_HSH_IV (start_inner_md5[1], 1);

   while (inplen >= 8) {
		in = *data++;
		CVM_LOAD_MD5_UNIT(in, next);
		inplen -= 8;
	}

   CVMX_PREFETCH0(start_outer_md5);
   if (unlikely(inplen)) {
		uint64_t tmp = 0;
		uint8_t *p = (uint8_t *)&tmp;
		p[inplen] = 0x80;
		do {
			inplen--;
			p[inplen] = ((uint8_t *)data)[inplen];
		} while (inplen);
      CVM_LOAD_MD5_UNIT (tmp, next);
   } else {
      CVM_LOAD_MD5_UNIT(0x8000000000000000ULL, next);
	}

   /* Finish Inner hash */
   while (next != 7) {
   	CVM_LOAD_MD5_UNIT(((uint64_t)0x0ULL), next);
   }
	CVMX_ES64(in, ((pktlen + 64) << 3));
   CVM_LOAD_MD5_UNIT(in, next);

   /* Get the inner hash of HMAC */
   CVMX_MF_HSH_IV (in, 0);
   CVMX_MF_HSH_IV (tmp, 1);

   /* Initialize hash unit */
   CVMX_MT_HSH_IV (start_outer_md5[0], 0);
   CVMX_MT_HSH_IV (start_outer_md5[1], 1);

   CVMX_MT_HSH_DAT (in, 0);
   CVMX_MT_HSH_DAT (tmp, 1);
   CVMX_MT_HSH_DAT (0x8000000000000000ULL, 2);
   CVMX_MT_HSH_DATZ(3);
   CVMX_MT_HSH_DATZ(4);
   CVMX_MT_HSH_DATZ(5);
   CVMX_MT_HSH_DATZ(6);
	CVMX_ES64(in, ((64 + 16) << 3));
   CVMX_MT_HSH_STARTMD5(in);

   /* Get the HMAC */
   res = (uint64_t *)(output+pktlen);
   CVMX_MF_HSH_IV (*res++, 0);
   CVMX_MF_HSH_IV (out, 1);
   *((uint32_t *)res) = (uint32_t)(out >> 32);
   
	octeon_crypto_disable(&state, flags);
   return 0;
}


int NULL_md5_decrypt (uint8_t *input, uint8_t *output, uint32_t pktlen, 
           			    uint8_t *unused1, int unused2, uint8_t *unused3, 
                      uint64_t *start_inner_md5, uint64_t *start_outer_md5)
{
   uint64_t *res = (uint64_t *)output, *data = (uint64_t *)input;
   register uint64_t in, out, tmp;
   register int next = 0;
   int inplen = pktlen - 12;
	struct octeon_cop2_state state;
	unsigned long flags;
   

   if (unlikely(input == NULL || output == NULL || pktlen == 0  || 
                start_inner_md5 == NULL || start_outer_md5 == NULL)) { 
      printk("\n %s: Wrong parameters \n", __FUNCTION__);   
      return -1;
   }
  
   CVMX_PREFETCH0(start_inner_md5);
   CVMX_PREFETCH0(data);

	flags = octeon_crypto_enable(&state);

   /* Load MD5 IV */
   CVMX_MT_HSH_IV (start_inner_md5[0], 0);
   CVMX_MT_HSH_IV (start_inner_md5[1], 1);

   while (inplen >= 8) {
		in = *data++;
		CVM_LOAD_MD5_UNIT(in, next);
		inplen -= 8;
	}

   CVMX_PREFETCH0(start_outer_md5);
   if (unlikely(inplen)) {
		uint64_t tmp = 0;
		uint8_t *p = (uint8_t *)&tmp;
		p[inplen] = 0x80;
		do {
			inplen--;
			p[inplen] = ((uint8_t *)data)[inplen];
		} while (inplen);
      CVM_LOAD_MD5_UNIT (tmp, next);
   } else {
      CVM_LOAD_MD5_UNIT(0x8000000000000000ULL, next);
	}

	pktlen -= 12;
   /* Finish Inner hash */
   while (next != 7) {
   	CVM_LOAD_MD5_UNIT(((uint64_t)0x0ULL), next);
   }
	CVMX_ES64(in, ((pktlen + 64) << 3));
   CVM_LOAD_MD5_UNIT(in, next);

   /* Get the inner hash of HMAC */
   CVMX_MF_HSH_IV (in, 0);
   CVMX_MF_HSH_IV (tmp, 1);

   /* Initialize hash unit */
   CVMX_MT_HSH_IV (start_outer_md5[0], 0);
   CVMX_MT_HSH_IV (start_outer_md5[1], 1);

   CVMX_MT_HSH_DAT (in, 0);
   CVMX_MT_HSH_DAT (tmp, 1);
   CVMX_MT_HSH_DAT (0x8000000000000000ULL, 2);
   CVMX_MT_HSH_DATZ(3);
   CVMX_MT_HSH_DATZ(4);
   CVMX_MT_HSH_DATZ(5);
   CVMX_MT_HSH_DATZ(6);
	CVMX_ES64(in, ((64 + 16) << 3));
   CVMX_MT_HSH_STARTMD5(in);

   /* Get and compare the HMAC */
   {
      res = (uint64_t *)(input+pktlen);
      CVMX_MF_HSH_IV (out, 0);
      if (unlikely(out != *res)) {
	      octeon_crypto_disable(&state, flags);
         return -1;
      }
      CVMX_MF_HSH_IV (out, 1);
      if (*(uint32_t *)(res + 1) != (uint32_t)(out >> 32)) {
	      octeon_crypto_disable(&state, flags);
         return -1;
      }
   }
   
	octeon_crypto_disable(&state, flags);
   return 0;
}

int AH_sha1(uint8_t *input, uint8_t *output, uint32_t pktlen, 
            uint8_t *unused1, int unused2, uint8_t *unused3, 
            uint64_t *start_inner_sha, uint64_t *start_outer_sha)
{
   uint64_t *res = (uint64_t *)output, *data = (uint64_t *)input;
   register uint64_t in, out, tmp;
   register int next = 0;
   int inplen = pktlen;
	struct octeon_cop2_state state;
	unsigned long flags;
   

   if (unlikely(input == NULL || output == NULL || pktlen == 0  || 
                start_inner_sha == NULL || start_outer_sha == NULL)) { 
      printk("\n %s: Wrong parameters \n", __FUNCTION__);   
      return -1;
   }
  
   CVMX_PREFETCH0(start_inner_sha);
   CVMX_PREFETCH0(data);

	flags = octeon_crypto_enable(&state);

   /* Load SHA1 IV */
   CVMX_MT_HSH_IV (start_inner_sha[0], 0);
   CVMX_MT_HSH_IV (start_inner_sha[1], 1);
   CVMX_MT_HSH_IV (start_inner_sha[2], 2);

   while (inplen >= 8) {
		in = *data++;
		CVM_LOAD_SHA_UNIT(in, next);
		inplen -= 8;
	}

   CVMX_PREFETCH0(start_outer_sha);
   if (unlikely(inplen)) {
		uint64_t tmp = 0;
		uint8_t *p = (uint8_t *)&tmp;
		p[inplen] = 0x80;
		do {
			inplen--;
			p[inplen] = ((uint8_t *)data)[inplen];
		} while (inplen);
      CVM_LOAD_SHA_UNIT (tmp, next);
   } else {
      CVM_LOAD_SHA_UNIT(0x8000000000000000ULL, next);
	}

   /* Finish Inner hash */
   while (next != 7) {
   	CVM_LOAD_SHA_UNIT(((uint64_t)0x0ULL), next);
   }
   CVM_LOAD_SHA_UNIT((uint64_t)((pktlen + 64) << 3), next);

   /* Get the inner hash of HMAC */
   CVMX_MF_HSH_IV (in, 0);
   CVMX_MF_HSH_IV (tmp, 1);
   out = 0;
   CVMX_MF_HSH_IV (out, 2);

   /* Initialize hash unit */
   CVMX_MT_HSH_IV (start_outer_sha[0], 0);
   CVMX_MT_HSH_IV (start_outer_sha[1], 1);
   CVMX_MT_HSH_IV (start_outer_sha[2], 2);

   CVMX_MT_HSH_DAT (in, 0);
   CVMX_MT_HSH_DAT (tmp, 1);
   out |= 0x0000000080000000;
   CVMX_MT_HSH_DAT (out, 2);
   CVMX_MT_HSH_DATZ(3);
   CVMX_MT_HSH_DATZ(4);
   CVMX_MT_HSH_DATZ(5);
   CVMX_MT_HSH_DATZ(6);
   CVMX_MT_HSH_STARTSHA((uint64_t)((64 + 20) << 3));

   /* Get the HMAC */
   res = (uint64_t *)output;
   CVMX_MF_HSH_IV (*res++, 0);
   CVMX_MF_HSH_IV (out, 1);
   *((uint32_t *)res) = (uint32_t)(out >> 32);
   CVMX_MF_HSH_IV (out, 2);
   
	octeon_crypto_disable(&state, flags);
   return 0;
}

int AH_md5(uint8_t *input, uint8_t *output, uint32_t pktlen, 
           uint8_t *unused1, int unused2, uint8_t *unused3, 
           uint64_t *start_inner_md5, uint64_t *start_outer_md5)
{
   uint64_t *res = (uint64_t *)output, *data = (uint64_t *)input;
   register uint64_t in, out, tmp;
   register int next = 0;
   int inplen = pktlen;
	struct octeon_cop2_state state;
	unsigned long flags;
   

   if (unlikely(input == NULL || output == NULL || pktlen == 0  || 
                start_inner_md5 == NULL || start_outer_md5 == NULL)) { 
      printk("\n %s: Wrong parameters \n", __FUNCTION__);   
      return -1;
   }
  
   CVMX_PREFETCH0(start_inner_md5);
   CVMX_PREFETCH0(data);

	flags = octeon_crypto_enable(&state);

   /* Load MD5 IV */
   CVMX_MT_HSH_IV (start_inner_md5[0], 0);
   CVMX_MT_HSH_IV (start_inner_md5[1], 1);

   while (inplen >= 8) {
		in = *data++;
		CVM_LOAD_MD5_UNIT(in, next);
		inplen -= 8;
	}

   CVMX_PREFETCH0(start_outer_md5);
   if (unlikely(inplen)) {
		uint64_t tmp = 0;
		uint8_t *p = (uint8_t *)&tmp;
		p[inplen] = 0x80;
		do {
			inplen--;
			p[inplen] = ((uint8_t *)data)[inplen];
		} while (inplen);
      CVM_LOAD_MD5_UNIT (tmp, next);
   } else {
      CVM_LOAD_MD5_UNIT(0x8000000000000000ULL, next);
	}

   /* Finish Inner hash */
   while (next != 7) {
   	CVM_LOAD_MD5_UNIT(((uint64_t)0x0ULL), next);
   }
	CVMX_ES64(in, ((pktlen + 64) << 3));
   CVM_LOAD_MD5_UNIT(in, next);

   /* Get the inner hash of HMAC */
   CVMX_MF_HSH_IV (in, 0);
   CVMX_MF_HSH_IV (tmp, 1);

   /* Initialize hash unit */
   CVMX_MT_HSH_IV (start_outer_md5[0], 0);
   CVMX_MT_HSH_IV (start_outer_md5[1], 1);

   CVMX_MT_HSH_DAT (in, 0);
   CVMX_MT_HSH_DAT (tmp, 1);
   CVMX_MT_HSH_DAT (0x8000000000000000ULL, 2);
   CVMX_MT_HSH_DATZ(3);
   CVMX_MT_HSH_DATZ(4);
   CVMX_MT_HSH_DATZ(5);
   CVMX_MT_HSH_DATZ(6);
	CVMX_ES64(in, ((64 + 16) << 3));
   CVMX_MT_HSH_STARTMD5(in);

   /* Get the HMAC */
   res = (uint64_t *)(output);
   CVMX_MF_HSH_IV (*res++, 0);
   CVMX_MF_HSH_IV (out, 1);
   *((uint32_t *)res) = (uint32_t)(out >> 32);
   
	octeon_crypto_disable(&state, flags);
   return 0;
}

#endif /* CONFIG_OCTEON_NATIVE_IPSEC */
