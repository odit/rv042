#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <sys/types.h>
#include <freeswan.h>

#include "constants.h"
#include "defs.h"
#include "log.h"
#include "libaes/aes_cbc.h"
#include "alg_info.h"
#include "ike_alg.h"

#define  AES_CBC_BLOCK_SIZE	(128/BITS_PER_BYTE)
#define  AES_KEY_MIN_LEN	128
#define  AES_KEY_DEF_LEN	128
#define  AES_KEY_MAX_LEN	256

//Charles: Test for HW accel
       #include <sys/types.h>
       #include <sys/stat.h>
       #include <fcntl.h>

#include "libaes/octeon-asm.h"

#define OCT_SUCCESS          1
#define OCT_FAILURE          0
static int oct_crypto_fd = 0;
#if _MIPS_SIM == _ABIN32
typedef uint32_t cvm_ptr_long_t;
#elif _MIPS_SIM == _ABI64
typedef uint64_t cvm_ptr_long_t;
#else
#error "Unsupported ABI"
#endif


typedef struct
{
	uint64_t sizeofptr;
	uint64_t arg1;
	uint64_t arg2;
	uint64_t arg3;
	uint64_t arg4;
	uint64_t arg5;
	int64_t arg6;
	int64_t arg7;
	int64_t arg8;
	int64_t arg9;
}cvm_crypto_op_t;

/* ioctl command */
#define CVM_MOD_EXP_BASE 0xbb

/* ioctl command */
#define CRYPT_MODEXPCRT		_IOWR(CVM_MOD_EXP_BASE,0,cvm_crypto_op_t) 
#define CRYPT_MODEXP        _IOWR(CVM_MOD_EXP_BASE,1,cvm_crypto_op_t)          
#define CRYPT_ENABLE        _IOWR(CVM_MOD_EXP_BASE,2,cvm_crypto_op_t)         
#define CRYPT_DISABLE       _IOWR(CVM_MOD_EXP_BASE,3,cvm_crypto_op_t)
#define CRYPT_IPSEC         _IOWR(CVM_MOD_EXP_BASE,4,cvm_crypto_op_t)  //Charles add to support HW IPSEC HSK

static void
crypto_display_hex (void *ptr, int len, char *msg)
{
  int i;

loglog(RC_LOG_SERIOUS,"User Space Key Schedule ....");
loglog(RC_LOG_SERIOUS,"%s Len=%d", msg, len);

  for (i = 0; i < len; i=i+8) {
    loglog(RC_LOG_SERIOUS,"%02x %02x %02x %02x  %02x %02x %02x %02x", ((uint8_t *) ptr)[i],((uint8_t *) ptr)[i+1],((uint8_t *) ptr)[i+2],((uint8_t *) ptr)[i+3]
				, ((uint8_t *) ptr)[i+4],((uint8_t *) ptr)[i+5],((uint8_t *) ptr)[i+6],((uint8_t *) ptr)[i+7]);
  }
}

static int
crypto_ipsec(u_int8_t *buf, size_t buf_len, u_int8_t *key1, u_int8_t *key2, u_int8_t *key3, size_t block_size , u_int8_t *iv, bool enc, u_int8_t algo_id)
{
   cvm_crypto_op_t tokernel;
   int ret;

   tokernel.sizeofptr = sizeof(void *);
   tokernel.arg1 = (u_int64_t)(cvm_ptr_long_t)buf;
   tokernel.arg2 = (u_int64_t)(cvm_ptr_long_t)key1;
   tokernel.arg3 = (u_int64_t)(cvm_ptr_long_t)key2;
   tokernel.arg4 = (u_int64_t)(cvm_ptr_long_t)key3;
   tokernel.arg5 = (u_int64_t)(cvm_ptr_long_t)iv;
   tokernel.arg6 = (u_int64_t)buf_len;
   tokernel.arg7 = (u_int64_t)block_size;
   tokernel.arg8 = (u_int64_t)enc;
   tokernel.arg9 = (u_int64_t)algo_id;
   ret = ioctl(oct_crypto_fd, CRYPT_IPSEC, (u_int64_t)(cvm_ptr_long_t)&tokernel);
   if (ret)
      return OCT_FAILURE;
   return OCT_SUCCESS;
}

void
AES_cbc_hwencrypt (const unsigned char *in, unsigned char *out,
  const unsigned long length, uint64_t * key,const int aes_Nkey,
  unsigned char *ivec, const int enc)
{
  uint64_t *iv;
  uint64_t *inp, *outp;
  uint64_t i0, i1, r0, r1;
  unsigned long len = length;

  inp = (uint64_t *) in;
  outp = (uint64_t *) out;
  iv = (uint64_t *) ivec;
  CVMX_MT_AES_IV (iv[0], 0);
  CVMX_MT_AES_IV (iv[1], 1);

  /* Initialise the keys */
  CVMX_MT_AES_KEY (*((uint64_t *)key + 0) , 0);
  CVMX_MT_AES_KEY (*((uint64_t *)key + 1) , 1);
  CVMX_MT_AES_KEY (*((uint64_t *)key + 2) , 2);
  CVMX_MT_AES_KEY (*((uint64_t *)key + 3) , 3);
  CVMX_MT_AES_KEYLENGTH (aes_Nkey/2 - 1);  //The aes_Nkey is 4 for AES 128 ; 6 for AES 192; 8 for AES 256
  i0 = inp[0];
  i1 = inp[1];

  if (enc) {
    if (len >= 32) {

      CVMX_MT_AES_ENC_CBC0 (i0);
      CVMX_MT_AES_ENC_CBC1 (i1);
      //  unrolled the loop. The first iteration doesn't store data
      /* The crypto takes 24 cycles so do some stuffs in the gap */
      len -= 16;
      inp += 2;
      outp += 2;
      if (len >= 16) {
        i0 = inp[0];
        i1 = inp[1];
        CVMX_MF_AES_RESULT (r0, 0);
        CVMX_MF_AES_RESULT (r1, 1);
        CVMX_MT_AES_ENC_CBC0 (i0);
        CVMX_MT_AES_ENC_CBC1 (i1);

        while (1) {
          outp[-2] = r0;
          outp[-1] = r1;
          len -= 16;
          inp += 2;
          outp += 2;
          if (len < 16)
            break;
          i0 = inp[0];
          i1 = inp[1];
          CVMX_PREFETCH (inp, 64);

          CVMX_MF_AES_RESULT (r0, 0);
          CVMX_MF_AES_RESULT (r1, 1);
          CVMX_MT_AES_ENC_CBC0 (i0);
          CVMX_MT_AES_ENC_CBC1 (i1);
        }
      }
      CVMX_MF_AES_RESULT (r0, 0);
      CVMX_MF_AES_RESULT (r1, 1);
      outp[-2] = r0;
      outp[-1] = r1;
    }

    if (len) {
      if (len <= 16) {
        uint64_t in64[2] = { 0, 0 };
        memcpy (&(in64[0]), inp, len);
        CVMX_MT_AES_ENC_CBC0 (in64[0]);
        CVMX_MT_AES_ENC_CBC1 (in64[1]);
        CVMX_MF_AES_RESULT (*(outp), 0);
        CVMX_MF_AES_RESULT (*(outp + 1), 1);
        memcpy (iv, outp, 16);
      }
    } else {
      memcpy (iv, (outp - 2), 16);
    }
  } else {
    /* Decrypt */
    if (len >= 32) {
      CVMX_MT_AES_DEC_CBC0 (i0);
      CVMX_MT_AES_DEC_CBC1 (i1);

      len -= 16;
      outp += 2;
      inp += 2;

      if (len >= 16) {
        /* Load ahead */
        i0 = inp[0];
        i1 = inp[1];
        CVMX_MF_AES_RESULT (r0, 0);
        CVMX_MF_AES_RESULT (r1, 1);

        CVMX_MT_AES_DEC_CBC0 (i0);
        CVMX_MT_AES_DEC_CBC1 (i1);
        while (1) {
          outp[-2] = r0;
          outp[-1] = r1;
          len -= 16;
          outp += 2;
          inp += 2;
          if (len < 16)
            break;
          i0 = inp[0];
          i1 = inp[1];
          CVMX_PREFETCH (inp, 64);
          CVMX_MF_AES_RESULT (r0, 0);
          CVMX_MF_AES_RESULT (r1, 1);
          CVMX_MT_AES_DEC_CBC0 (i0);
          CVMX_MT_AES_DEC_CBC1 (i1);
        }
      }
      /* Fetch the result of the last 16B in the 16B stuff */
      CVMX_MF_AES_RESULT (r0, 0);
      CVMX_MF_AES_RESULT (r1, 1);
      outp[-2] = r0;
      outp[-1] = r1;
    }
    if (len) {
      if (len <= 16) {
        /* To avoid len>16  tat is when len=17-31 
         * to enter into this loop */
        uint64_t in64[2] = { 0, 0 };
        memcpy (&(in64[0]), inp, len);
        CVMX_MT_AES_DEC_CBC0 (in64[0]);
        CVMX_MT_AES_DEC_CBC1 (in64[1]);
        CVMX_MF_AES_RESULT (*outp, 0);
        CVMX_MF_AES_RESULT (*(outp + 1), 1);
        memcpy (iv, inp, 16);
      }
    } else {
      memcpy (iv, (inp - 2), 16);
    }
  }

}

static void
do_aes(u_int8_t *buf, size_t buf_len, u_int8_t *key, size_t key_size, u_int8_t *iv, bool enc)
{
    aes_context aes_ctx;
    char iv_bak[AES_CBC_BLOCK_SIZE];
    char *new_iv = NULL;	/* logic will avoid copy to NULL */

    aes_set_key(&aes_ctx, key, key_size, 0);

    /*	
     *	my AES cbc does not touch passed IV (optimization for
     *	ESP handling), so I must "emulate" des-like IV
     *	crunching
     */
    if (!enc)
	memcpy(new_iv=iv_bak, (char*) buf + buf_len - AES_CBC_BLOCK_SIZE
		, AES_CBC_BLOCK_SIZE);
#if 0
memcpy(iv_tmp,iv,AES_CBC_BLOCK_SIZE);
crypto_display_hex (key , key_size,  "Usr Key:");
crypto_display_hex (iv , key_size,  "User IV:");

crypto_ipsec( buf, buf_len , key , key , key , AES_CBC_BLOCK_SIZE ,iv , enc , OAKLEY_AES_CBC);
#endif

    //AES_cbc_encrypt(&aes_ctx, buf, buf, buf_len, iv, enc);
#if 0
crypto_display_hex (iv , key_size,  "SW IV:");
crypto_display_hex (buf2 , buf_len,  "SW OUTPUT:");
#endif
    AES_cbc_hwencrypt (buf, buf, buf_len, key, aes_ctx.aes_Nkey, iv , enc);
#if 0
crypto_display_hex (iv_tmp , key_size,  "HW IV:");
crypto_display_hex (buf , buf_len,  "HW OUTPUT:");
#endif

    if (enc)
	new_iv = (char*) buf + buf_len-AES_CBC_BLOCK_SIZE;

    memcpy(iv, new_iv, AES_CBC_BLOCK_SIZE);
}

struct encrypt_desc algo_aes =
{
	algo_type: 	IKE_ALG_ENCRYPT,
	algo_id:   	OAKLEY_AES_CBC,
	algo_next: 	NULL, 
	enc_ctxsize: 	sizeof(aes_context),
	enc_blocksize: 	AES_CBC_BLOCK_SIZE,
	keyminlen: 	AES_KEY_MIN_LEN,
	keydeflen: 	AES_KEY_DEF_LEN,
	keymaxlen: 	AES_KEY_MAX_LEN,
	do_crypt: 	do_aes,
};

int ike_alg_aes_init(void);

int
ike_alg_aes_init(void)
{
	int ret = ike_alg_register_enc(&algo_aes);
	return ret;
}
/*
IKE_ALG_INIT_NAME: ike_alg_aes_init
*/
