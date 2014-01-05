
struct variable2 common_variables[] = {
	{COMMONFIRMWAREVER, ASN_OCTET_STR, RONLY, var_common, 1, {1}},
	{COMMONMODELID, ASN_OCTET_STR, RONLY, var_common, 1, {4}}
};

oid common_variables_oid[] = { VENDOR_OID, 1 };

u_char *var_common(	struct variable *vp,
						oid * name,
						size_t * length,
						int exact, size_t * var_len, WriteMethod ** write_method)
{
	static unsigned long ret;
	static char tmp[NK_SNMP_STR_LEN];
	int pos = 0;
	char DBdata[64];
			
	if (header_generic(vp, name, length, exact, var_len, write_method) == MATCH_FAILED)
		return NULL;

	switch(vp->magic)
	{
	  case COMMONFIRMWAREVER:
		{
			if(strlen(FIRMWARE_RC_VERSION))
				sprintf(tmp, "%s %s (%s %s)", FIRMWARE_VERSION, FIRMWARE_RC_VERSION, __DATE__, __TIME__);
			else
				sprintf(tmp, "%s (%s %s)", FIRMWARE_VERSION, __DATE__, __TIME__);
      *var_len = strlen(tmp);
      return (u_char *) tmp;
		}
		case COMMONMODELID:
		{
			kd_doCommand("VERSION MODEL", CMD_PRINT, ASH_DO_NOTHING, tmp);
//			sprintf(tmp, "%s", FIRMWARE_VERSION, __DATE__, __TIME__);
      *var_len = strlen(tmp);
      return (u_char *) tmp;
		}

		default:
			DEBUGMSGTL(("snmpd", "unknown sub-id %d in nk_system/var_nk_system\n", vp->magic));
	}
	sprintf(tmp, "rm -f %s", SNMP_TMP_FILE);
	system(tmp);

	return NULL;
}
