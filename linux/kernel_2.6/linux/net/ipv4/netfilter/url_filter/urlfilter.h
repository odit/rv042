#include "tmufeng.h"

//#include <include/nk_def.h>

#define LICENSE_ID  	NK_MODEL_NAME
#define VENDER_ID   	"LNKS"
#define RS_HOST     	"linksys10.url.trendmicro.com"
#define BAKUP_RS_HOST   "1.2.3.4"
#define URL_FILTER_FILE_NAME   "/dev/url_filter"

typedef struct __TMUFE_interfaces {
	void *so;
	FPTR_TM_UF_initEngEx f_init;
	FPTR_TM_UF_uninitEngEx f_uninit;
	FPTR_TM_UF_setOption f_set_option;
	FPTR_TM_UF_allocEnv f_alloc_env;
	FPTR_TM_UF_freeEnv f_free_env;
	FPTR_TM_UF_rateURLEx f_rate_url;
	FPTR_TM_UF_initLog f_init_log;
}	TMUFE_APIs;
