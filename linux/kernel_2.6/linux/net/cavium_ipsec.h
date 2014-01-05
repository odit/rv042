
#ifndef CONFIG_OCTEON_NATIVE_IPSEC
#error "cavium_ipsec.h included without OCTEON_NATIVE_IPSEC defined"
#endif

#ifndef _CAVIUM_IPSEC_H_
#define _CAVIUM_IPSEC_H_

#include <net/xfrm.h>
#include <net/esp.h>
#include <net/ah.h>

/* Wrapper APIs */
void * cavium_alloc_n_fill(struct xfrm_state *x);
int cavium_process_esp_pkt(struct esp_data *esp, struct sk_buff *skb);
int cavium_process_ah_pkt(struct ah_data *ah, struct sk_buff *skb, uint8_t *out);

/* IPSec APIs */
void cav_calc_hash(int sha1, uint64_t *key, uint8_t *inner, uint8_t *outer);

int AES_CBC_sha1_encrypt(uint8_t *input, uint8_t *output, uint32_t pktlen, 
                         uint8_t *aes_key, int aes_key_len, uint8_t *aes_iv,
                         uint64_t *start_inner_sha, uint64_t *start_outer_sha);
int AES_CBC_sha1_decrypt(uint8_t *input, uint8_t *output, uint32_t pktlen, 
                         uint8_t *aes_key, int aes_key_len, uint8_t *aes_iv,
                         uint64_t *start_inner_sha, uint64_t *start_outer_sha);
int DES_CBC_sha1_encrypt (uint8_t *input, uint8_t *output, uint32_t pktlen, 
                          uint8_t *des_key, int des_key_len, uint8_t *des_iv, 
								  uint64_t *start_inner_sha, uint64_t *start_outer_sha);
int DES_CBC_sha1_decrypt (uint8_t *input, uint8_t *output, uint32_t pktlen, 
                          uint8_t *des_key, int des_key_len, uint8_t *des_iv, 
								  uint64_t *start_inner_sha, uint64_t *start_outer_sha);
int AES_CBC_md5_encrypt(uint8_t *input, uint8_t *output, uint32_t pktlen, 
                        uint8_t *aes_key, int aes_key_len, uint8_t *aes_iv,
                        uint64_t *start_inner_md5, uint64_t *start_outer_md5);
int AES_CBC_md5_decrypt(uint8_t *input, uint8_t *output, uint32_t pktlen, 
                        uint8_t *aes_key, int aes_key_len, uint8_t *aes_iv,
                        uint64_t *start_inner_md5, uint64_t *start_outer_md5);
int DES_CBC_md5_encrypt(uint8_t *input, uint8_t *output, uint32_t pktlen, 
                        uint8_t *des_key, int des_key_len, uint8_t *des_iv, 
								uint64_t *start_inner_md5, uint64_t *start_outer_md5);
int DES_CBC_md5_decrypt (uint8_t *input, uint8_t *output, uint32_t pktlen, 
                         uint8_t *des_key, int des_key_len, uint8_t *des_iv, 
								 uint64_t *start_inner_md5, uint64_t *start_outer_md5);
int AH_sha1(uint8_t *input, uint8_t *output, uint32_t pktlen, 
            uint8_t *unused1, int unused2, uint8_t *unused3, 
            uint64_t *start_inner_sha, uint64_t *start_outer_sha);
int AH_md5(uint8_t *input, uint8_t *output, uint32_t pktlen, 
           uint8_t *unused1, int unused2, uint8_t *unused3, 
           uint64_t *start_inner_md5, uint64_t *start_outer_md5);
int NULL_sha1_encrypt(uint8_t *input, uint8_t *output, uint32_t pktlen, 
           			    uint8_t *unused1, int unused2, uint8_t *unused3, 
                      uint64_t *start_inner_sha, uint64_t *start_outer_sha);
int NULL_sha1_decrypt(uint8_t *input, uint8_t *output, uint32_t pktlen, 
           			    uint8_t *unused1, int unused2, uint8_t *unused3, 
                      uint64_t *start_inner_sha, uint64_t *start_outer_sha);
int NULL_md5_encrypt(uint8_t *input, uint8_t *output, uint32_t pktlen, 
           			   uint8_t *unused1, int unused2, uint8_t *unused3, 
                     uint64_t *start_inner_md5, uint64_t *start_outer_md5);
int NULL_md5_decrypt(uint8_t *input, uint8_t *output, uint32_t pktlen, 
           			   uint8_t *unused1, int unused2, uint8_t *unused3, 
                     uint64_t *start_inner_md5, uint64_t *start_outer_md5);
int AES_CBC_encrypt(uint8_t *input, uint8_t *output, uint32_t pktlen, 
                    uint8_t *aes_key, int aes_key_len, uint8_t *aes_iv,
                    uint64_t *unused1, uint64_t *unused2);
int AES_CBC_decrypt(uint8_t *input, uint8_t *output, uint32_t pktlen, 
                    uint8_t *aes_key, int aes_key_len, uint8_t *aes_iv,
                    uint64_t *unused1, uint64_t *unused2);
int DES_CBC_encrypt(uint8_t *input, uint8_t *output, uint32_t pktlen, 
                    uint8_t *des_key, int des_key_len, uint8_t *des_iv, 
                    uint64_t *unused1, uint64_t *unused2);
int DES_CBC_decrypt (uint8_t *input, uint8_t *output, uint32_t pktlen, 
                     uint8_t *des_key, int des_key_len, uint8_t *des_iv, 
                     uint64_t *unused1, uint64_t *unused2);

#endif /* _CAVIUM_IPSEC_H_ */
