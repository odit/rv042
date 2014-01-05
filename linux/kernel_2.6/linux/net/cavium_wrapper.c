
#if defined (CONFIG_OCTEON_NATIVE_IPSEC)

#include <linux/kernel.h>
#include <net/esp.h>
#include <net/ah.h>
#include <net/addrconf.h>
#include "cavium_ipsec.h"
#include "octeon-asm.h"

typedef int (*cavium_ipsec_fn)(uint8_t *, uint8_t *, uint32_t, uint8_t *, 
                               int, uint8_t *,
                               uint64_t *, uint64_t *);
typedef struct {
	uint8_t inner_hash[24];
	uint8_t outer_hash[24];
	cavium_ipsec_fn oct_fn;
} oct_data_t;

static cavium_ipsec_fn
get_ah_fn_name(struct xfrm_state *x)
{
	int aalg = x->props.aalgo;
	cavium_ipsec_fn ret = NULL;

	/* By default assume sha1 APIs */
	if (aalg == SADB_AALG_SHA1HMAC) {
		ret = AH_sha1;
	} else if (aalg == SADB_AALG_MD5HMAC) {
		ret = AH_md5;
	} else {
		ret = NULL;
	}
	
	return ret;
}

static cavium_ipsec_fn
get_esp_fn_name(struct xfrm_state *x)
{
	int aalg = x->props.aalgo;
	int ealg = x->props.ealgo;
	cavium_ipsec_fn ret = NULL;
	int dir;

	#if defined(CONFIG_IPV6) || defined (CONFIG_IPV6_MODULE)
	if (x->props.family == AF_INET6) {
		dir = (ipv6_chk_addr(((struct in6_addr *)(&x->id.daddr)), NULL, 0) 
		       ? 1 : 0);
	} else 
	#endif
	if (x->props.family == AF_INET) {
		dir = ((inet_addr_type(x->id.daddr.a4) == RTN_LOCAL) ? 1 : 0); 
	               /* Inbound 1, outbound 0 */
	} else {
		printk("%s: Invalid x->props.family %d \n", __FUNCTION__, 
		       x->props.family);
		return ret;
	}

	/* By default assume sha1 APIs */
	switch (ealg) {
		case SADB_EALG_DESCBC:
		case SADB_EALG_3DESCBC:
			if (dir) {
				ret = DES_CBC_sha1_decrypt;
			} else {
				ret = DES_CBC_sha1_encrypt;
			}
			break;
		case SADB_X_EALG_AESCBC:
			if (dir) {
				ret = AES_CBC_sha1_decrypt;
			} else {
				ret = AES_CBC_sha1_encrypt;
			}
			break;
		case SADB_EALG_NULL:
			if (dir) {
				ret = NULL_sha1_decrypt;
			} else {
				ret = NULL_sha1_encrypt;
			}
			break;
		default:
			return ret;
	}

	if (aalg == SADB_AALG_SHA1HMAC)
		return ret;
	
	if (aalg != SADB_AALG_MD5HMAC) {
		ret = NULL;
		return ret;
	}

	switch (ealg) {
		case SADB_EALG_DESCBC:
		case SADB_EALG_3DESCBC:
			if (dir) {
				ret = DES_CBC_md5_decrypt;
			} else {
				ret = DES_CBC_md5_encrypt;
			}
			break;
		case SADB_X_EALG_AESCBC:
			if (dir) {
				ret = AES_CBC_md5_decrypt;
			} else {
				ret = AES_CBC_md5_encrypt;
			}
			break;
		case SADB_EALG_NULL:
			if (dir) {
				ret = NULL_md5_decrypt;
			} else {
				ret = NULL_md5_encrypt;
			}
			break;
		default:
			printk("%s: Should never happen .....\n", __FUNCTION__);
			ret = NULL;
	}

	return ret;
}


void *
cavium_alloc_n_fill(struct xfrm_state *x)
{
	oct_data_t *buf;
	cavium_ipsec_fn fn = NULL;
	struct esp_data *esp = NULL;
	struct ah_data *ah = NULL;
	uint64_t *key = NULL;

	if (x->type == NULL) {
		printk("%s: xfrm->type is not filled. \n", __FUNCTION__);
		return NULL;
	}

	if (x->type->proto == IPPROTO_ESP) {
		fn = get_esp_fn_name(x);
		esp = (struct esp_data *)(x->data);
		if (x->aalg) {
			key = (uint64_t *)(esp->auth.key);
		}
	} else if (x->type->proto == IPPROTO_AH) {
		fn = get_ah_fn_name(x);
		ah = (struct ah_data *)(x->data);
		if (x->aalg) {
			key = (uint64_t *)(ah->key);
		}
	} else {
		printk("%s: Protocol type not valid. \n", __FUNCTION__);
		return NULL;
	}

	if (fn == NULL) {
		printk("%s: Cipher/Digest not supported. \n", __FUNCTION__);
		return NULL;
	} 

	buf = (oct_data_t *)kmalloc(sizeof(oct_data_t), GFP_KERNEL);
	if (buf == NULL) {
		printk("%s: Unable to allocate memory for cavium ipsec offload\n"
		       "         So, continuing software only ..\n", __FUNCTION__);
		return buf;
	}

	memset(buf, 0, sizeof(*buf));

	if (key)
		cav_calc_hash(((x->props.aalgo == SADB_AALG_SHA1HMAC) ? 1 : 0), 
		              key, buf->inner_hash, buf->outer_hash);

	buf->oct_fn = fn;
	
	return (void *)buf;
}

int
cavium_process_esp_pkt(struct esp_data *esp, struct sk_buff *skb)
{
	int ekeylen = esp->conf.key_len;
	uint8_t *ekey = esp->conf.key;
	oct_data_t *d = (oct_data_t *)(esp->oct_data);

	if (unlikely(d == NULL || d->oct_fn == NULL)) {
		return -EIO;
	}
	/* Will be used in all the encrypt functions */
	CVMX_PREFETCH0(ekey);

	return (d->oct_fn(skb->data, skb->data, skb->len, ekey, ekeylen, 
	                  (esp->conf.tfm)->crt_cipher.cit_iv,
							(uint64_t *)(d->inner_hash),
				         (uint64_t *)(d->outer_hash)));
}

int
cavium_process_ah_pkt(struct ah_data *ah, struct sk_buff *skb, uint8_t *out)
{
	oct_data_t *d = (oct_data_t *)(ah->oct_data);

	if (unlikely(d == NULL || d->oct_fn == NULL)) {
		return -EIO;
	}

	*(uint64_t *)out = 0;
	*(uint32_t *)(out + 8) = 0;
	return (d->oct_fn(skb->data, out, skb->len, NULL, 0, NULL,
							(uint64_t *)(d->inner_hash),
				         (uint64_t *)(d->outer_hash)));
}

#endif
