/*
     2006-12 Encounter: separate from sqrtrem.c
*/
#include "gmp.h"
#include "gmp-impl.h"
#include "longlong.h"

void
mpn_sqr_diagonal (mp_ptr rp, mp_srcptr up, mp_size_t n)
{
    mp_size_t _i;
    for (_i = 0; _i < (n); _i++)
      {
	mp_limb_t ul, lpl;
	ul = (up)[_i];
	umul_ppmm ((rp)[2 * _i + 1], lpl, ul, ul << GMP_NAIL_BITS);
	(rp)[2 * _i] = lpl >> GMP_NAIL_BITS;
      }

}
		