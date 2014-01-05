#include "MIB.h"

#include "../../../../tool/nkuserlandconf.h"


#ifdef CONFIG_MODEL_RV0XX
#include "common.c"
#include "basicMgt.c"
#include "advanceMgt.c"
#include "wanMgt.c"
#include "entity.c"
#endif

/*
static int nk_awk(char *src, int pos, char *dst)
{
	char *ptmp = src;
	int j;

	while((*ptmp != '\0') && ((*ptmp == ' ') || (*ptmp == '\t')))	ptmp++;
	for(j = 0; j < pos; j++)
	{
		sscanf(ptmp, "%s", dst);
		ptmp += strlen(dst);
		while((*ptmp != '\0') && ((*ptmp == ' ') || (*ptmp == '\t')))	ptmp++;
		if(*ptmp == '\0')
		{
			*dst = '\0';
			return -1;
		}
	}
	return 0;
}

int nk_parse_tmp_file(char *filename, int pos, char *dst)
{
	char tmp[256];
	int ret = 0;

	FILE *fp = fopen(filename, "r");
	if(fp)
	{
		memset(tmp, '\0', sizeof(tmp));
		fgets(tmp, sizeof(tmp), fp);
		if(nk_awk(tmp, pos, dst) < 0)
		{
			ret = -1;
		}
		fclose(fp);
	}
	else
	{
		ret = -2;
	}
	return ret;
}
*/
void init_cisco_MIB(void)
{
#ifdef CONFIG_MODEL_RV0XX
    REGISTER_MIB("common", common_variables, variable2, common_variables_oid);
		REGISTER_MIB("basicMgt", basicMgt_variables, variable4, basicMgt_variables_oid);
		REGISTER_MIB("advanceMgt", advanceMgt_variables, variable4, advanceMgt_variables_oid);
		REGISTER_MIB("wanMgt", wanMgt_variables, variable4, wanMgt_variables_oid);
		REGISTER_MIB("entity", entity_variables, variable2, entity_variables_oid);
#endif
}


