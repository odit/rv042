

#include "nkutil.h"

#define LICENSE_ENCODE
int check_key(char* m_key,char* m_sn,char* m_mac,char* m_nowdate,struct ret_key* decodekey);
int LK_Encode_Value(long lvalue, char* m_validdate, char* EncodeKey);
int LK_Decode_Value(char* Decodekey, char* m_validdate);

#define NK_db_read_factory(a,b) kd_doCommand_factory(a, CMD_PRINT, ASH_DO_NOTHING, b)
#define LICENSE_OVALUE		"CURRENTVALUE_O"
#define LICENSE_NVALUE		"CURRENTVALUE_N"
#define LICENSE_COUNTDOWN	"COUNTDOWN"

int read_license_db(int fid,char* attribute)
{
	char Keyvalue[12]={'\0'};
	char Category_item[50]={'\0'};
	char m_nowdate[12]={'\0'};
	long Current_value=0;

	sprintf(Category_item,"FID%d M_NOWDATE",fid);
	NK_db_read_factory(Category_item,m_nowdate);
	
	sprintf(Category_item,"FID%d %s",fid,attribute);
	#ifdef LICENSE_ENCODE
	NK_db_read_factory(Category_item,Keyvalue);
	if(strlen(m_nowdate))
		Current_value = LK_Decode_Value(Keyvalue, m_nowdate);
	else
		Current_value = atoi(Keyvalue);
	#else
	NK_db_read_factory(Category_item,Keyvalue);
	Current_value=atoi(Keyvalue);
	#endif
	return Current_value;
}

int get_license_key_status(int fid)
{
	return read_license_db(fid,LICENSE_NVALUE);
}
