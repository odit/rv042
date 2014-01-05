
#ifndef	_TM_CSM_LINKSYS_TMUFE_TEST
#define _TM_CSM_LINKSYS_TMUFE_TEST



#include "tmufeng.h"



typedef struct __TMUFE_interfaces {
	void *so;
	FPTR_TM_UF_initEngEx f_init;
	FPTR_TM_UF_uninitEngEx f_uninit;
	FPTR_TM_UF_setOption f_set_option;
	FPTR_TM_UF_allocEnv f_alloc_env;
	FPTR_TM_UF_freeEnv f_free_env;
	FPTR_TM_UF_rateURLEx f_rate_url;
}	TMUFE_APIs;



#endif


