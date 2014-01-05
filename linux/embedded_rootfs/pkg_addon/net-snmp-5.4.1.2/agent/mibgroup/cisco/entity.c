
FindVarMethod var_entity;

#define	entPhysicalDescr						2
#define entPhysicalName		  				7
#define entPhysicalHardwareRev		  8
#define entPhysicalFirmwareRev			9
#define entPhysicalSoftwareRev		  10
#define entPhysicalSerialNum 				11
#define entPhysicalMfgName 					12
#define entPhysicalModelName 				13



struct variable2 entity_variables[] = {
	{entPhysicalDescr, ASN_OCTET_STR, RONLY, var_entity, 1, {2}},
	{entPhysicalName, ASN_OCTET_STR, RONLY, var_entity, 1, {7}},
	{entPhysicalHardwareRev, ASN_OCTET_STR, RONLY, var_entity, 1, {8}},
	{entPhysicalFirmwareRev, ASN_OCTET_STR, RONLY, var_entity, 1, {9}},
	{entPhysicalSoftwareRev, ASN_OCTET_STR, RONLY, var_entity, 1, {10}},
	{entPhysicalSerialNum, ASN_OCTET_STR, RONLY, var_entity, 1, {11}},
	{entPhysicalMfgName, ASN_OCTET_STR, RONLY, var_entity, 1, {12}},
	{entPhysicalModelName, ASN_OCTET_STR, RONLY, var_entity, 1, {13}}
};

oid entity_variables_oid[] = { 	1, 3, 6, 1, 2, 1, 47, 1, 1, 1, 1 };

u_char *var_entity(	struct variable *vp,
						oid * name,
						size_t * length,
						int exact, size_t * var_len, WriteMethod ** write_method)
{
	static unsigned long ret;
	static char tmp[NK_SNMP_STR_LEN];
	int pos = 0;
	char DBdata[64];

	#ifdef TABLE_SIZE
	#undef TABLE_SIZE
	#endif
	#define TABLE_SIZE 1
	if (header_simple_table(vp,name,length,exact,var_len,write_method, TABLE_SIZE) == MATCH_FAILED)
			return NULL;

	memset(tmp, 0, sizeof(tmp));
	switch(vp->magic)
	{
	  case entPhysicalDescr:
		{
			/* purpose      :  0012731 author :  Gavin.Lin  date :  2010-06-23         */
			/* description  :  Set entPhysicalDescr by Model                           */
			kd_doCommand("VERSION MODEL", CMD_PRINT, ASH_DO_NOTHING, DBdata);
			if (!strcmp("RV042", DBdata))
			{
				sprintf(tmp, "10/100 4-Port VPN Router");
			}
			if (!strcmp("RV082", DBdata))
			{
				sprintf(tmp, "10/100 8-Port VPN Router");
			}
			if (!strcmp("RV016", DBdata))
			{
				sprintf(tmp, "10/100 16-Port VPN Router");
			}
      *var_len = strlen(tmp);
      return (u_char *) tmp;
		}
		case entPhysicalName:
		{
			kd_doCommand("VERSION MODEL", CMD_PRINT, ASH_DO_NOTHING, tmp);
      *var_len = strlen(tmp);
      return (u_char *) tmp;
		}
		case entPhysicalHardwareRev:
		{
			strcpy(tmp, "V03");
      *var_len = strlen(tmp);
      return (u_char *) tmp;
		}
		case entPhysicalFirmwareRev:
		{
			if(strlen(FIRMWARE_RC_VERSION))
				sprintf(tmp, "%s %s (%s %s)", FIRMWARE_VERSION, FIRMWARE_RC_VERSION, __DATE__, __TIME__);
			else
				sprintf(tmp, "%s (%s %s)", FIRMWARE_VERSION, __DATE__, __TIME__);
      *var_len = strlen(tmp);
      return (u_char *) tmp;		
		}
		case entPhysicalSoftwareRev:
		{
			if(strlen(FIRMWARE_RC_VERSION))
				sprintf(tmp, "%s %s (%s %s)", FIRMWARE_VERSION, FIRMWARE_RC_VERSION, __DATE__, __TIME__);
			else
				sprintf(tmp, "%s (%s %s)", FIRMWARE_VERSION, __DATE__, __TIME__);
      *var_len = strlen(tmp);
      return (u_char *) tmp;		
		}
		case entPhysicalSerialNum:
		{
			kd_doCommand("VERSION SERIALNO", CMD_PRINT, ASH_DO_NOTHING, tmp);
      *var_len = strlen(tmp);
      return (u_char *) tmp;		
		}
		case entPhysicalMfgName:
		{
			strcpy(tmp, "Cisco Small Business");
      *var_len = strlen(tmp);
      return (u_char *) tmp;		
		}
		case entPhysicalModelName:
		{
			kd_doCommand("VERSION MODEL", CMD_PRINT, ASH_DO_NOTHING, tmp);
      *var_len = strlen(tmp);
      return (u_char *) tmp;		
		}	
		default:
			DEBUGMSGTL(("snmpd", "unknown sub-id %d \n", vp->magic));
	}
	sprintf(tmp, "rm -f %s", SNMP_TMP_FILE);
	system(tmp);

	return NULL;
}
