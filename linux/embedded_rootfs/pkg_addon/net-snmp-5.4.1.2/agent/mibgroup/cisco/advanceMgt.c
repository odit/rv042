/* purpose   	: SNMP
 * author    	: Gavin.Lin		
 * date		: 2010-05-28
 * description  : Add '\0' on SNMP string
 *                ex : string[var_val_len] = '\0';
 */

struct variable4 advanceMgt_variables[] = {
  { ADVMGTSNMPSTATUS    , ASN_INTEGER   , RWRITE, var_advanceMgt, 1, { 8 } },
  { TRAPMANAGERINDEX    , ASN_INTEGER   , RONLY , var_trapManagerTable, 3, { 16,1,1 } },
  { TRAPMGRNETADDRESS   , ASN_IPADDRESS , RWRITE, var_trapManagerTable, 3, { 16,1,2 } },
  { TRAPMGRCOMMUNITYNAME, ASN_OCTET_STR , RWRITE, var_trapManagerTable, 3, { 16,1,3 } },
  { ADVMGTUPNPSTATUS    , ASN_INTEGER   , RWRITE, var_advanceMgt, 1, { 17 } },
};

oid advanceMgt_variables_oid[] = { VENDOR_OID, 2, 2 };

unsigned char *var_advanceMgt(struct variable   *vp, 
                                        oid     *name, 
                                        size_t  *length, 
                                        int     exact, 
                                        size_t  *var_len, 
                                        WriteMethod **write_method)
{
    /* variables we may use later */
    static long long_ret;
		char DBdata[64];
    
    if (header_generic(vp,name,length,exact,var_len,write_method) == MATCH_FAILED)
        return NULL;

		switch (vp->magic)
    {
        case ADVMGTSNMPSTATUS:
            *write_method = write_advMgtSNMPStatus;
						kd_doCommand("SNMP ENABLED", CMD_PRINT, ASH_DO_NOTHING, DBdata);
						if (!strcmp("YES", DBdata))
							long_ret = 1;
						else
							long_ret = 0;
            return (unsigned char *) &long_ret;
        case ADVMGTUPNPSTATUS:
            *write_method = write_advMgtUPnPStatus;
						kd_doCommand("UPNP UPNPENABLE", CMD_PRINT, ASH_DO_NOTHING, DBdata);
						if (!strcmp("YES", DBdata))
							long_ret = 1;
						else
							long_ret = 0;
            return (unsigned char *) &long_ret;

        default:
            ERROR_MSG("");
    }
    return NULL;
}


unsigned char *var_trapManagerTable(struct variable *vp,
                                            oid     *name,
                                            size_t  *length,
                                            int     exact,
                                            size_t  *var_len,
                                            WriteMethod **write_method)
{
    /* variables we may use later */
    static long long_ret;
    static unsigned char string[SPRINT_MAX_LEN];
		static char DBdata[64];
		static struct in_addr target;
    
		#ifdef TABLE_SIZE
		#undef TABLE_SIZE
		#endif
    #define TABLE_SIZE 1
    if (header_simple_table(vp,name,length,exact,var_len,write_method, TABLE_SIZE) == MATCH_FAILED)
        return NULL;

    /* this is where we do the value assignments for the mib results. */
    switch (vp->magic)
    {
        case TRAPMANAGERINDEX:
            long_ret = name[*length-1];
            return (unsigned char *) &long_ret;

        case TRAPMGRNETADDRESS:
            *write_method = write_trapMgrNetAddress;
						kd_doCommand("SNMP SSTRAP", CMD_PRINT, ASH_DO_NOTHING, DBdata);
						if (inet_addr(DBdata)==0xFFFFFFFF)
							sprintf(DBdata, "0.0.0.0");
						target.s_addr = inet_addr(DBdata);
            *var_len = 4;
            return (unsigned char *) &target.s_addr;

        case TRAPMGRCOMMUNITYNAME:
            *write_method = write_trapMgrCommunityName;
						kd_doCommand("SNMP TCNAME", CMD_PRINT, ASH_DO_NOTHING, DBdata);
            *var_len = strlen(DBdata);
            return (unsigned char *) DBdata;
            
        default:
            ERROR_MSG("");
    }
    return NULL;
}

int write_trapMgrNetAddress(int  action,
                        u_char   *var_val,
                        u_char   var_val_type,
                        size_t   var_val_len,
                        u_char   *statP,
                        oid      *name,
                        size_t   name_len)
{
    const int IP_LENGTH = 4;
    static struct in_addr target;
		static unsigned char string[64];

    switch (action)
    {
        case RESERVE1:
            if (var_val_type != ASN_IPADDRESS)
            {
                fprintf(stderr, "write to trapMgrNetAddress not ASN_IPADDRESS\n");
                return SNMP_ERR_WRONGTYPE;
            }
            if (var_val_len != IP_LENGTH)
            {
                fprintf(stderr,"write to trapMgrNetAddress: bad length\n");
                return SNMP_ERR_WRONGLENGTH;
            }
            /* check ip value */
            if (var_val[0] > 255 || var_val[1] > 255 || var_val[2] > 255 ||
                var_val[3] > 254)
            {
                fprintf(stderr,"write to trapMgrNetAddress: bad value\n");
                return SNMP_ERR_WRONGVALUE;
            }
            break;
        case RESERVE2:
            break;

        case FREE:
            /* Release any resources that have been allocated */
            break;

        case ACTION:
            /* The variable has been stored in string for
                you to use, and you have just been asked to do something with
                it. Note that anything done here must be reversable in the UNDO case */
            break;

        case UNDO:
            /* Back out any changes made in the ACTION case */
            break;
            
        case COMMIT:
            /* Things are working well, so it's now safe to make the change
                permanently.  Make sure that anything done here can't fail! */
						memcpy((char *)&target.s_addr, (char *)var_val, IP_LENGTH);
						memset(string, 0, sizeof(string));
            sprintf(string, "SNMP SSTRAP=%s", inet_ntoa(target));
						kd_doCommand(string, CMD_WRITE, ASH_DO_NOTHING, NULL);
						kd_updateFlash(USER_CHANGE_DB);
            break;
    } /* end of switch */
    return SNMP_ERR_NOERROR;
}


int write_trapMgrCommunityName(int   action,
                            u_char   *var_val,
                            u_char   var_val_type,
                            size_t   var_val_len,
                            u_char   *statP,
                            oid      *name,
                            size_t   name_len)
{
    #define TRAP_COMMUNITY_SIZE 33
    static unsigned char string[64+TRAP_COMMUNITY_SIZE];
    u_char         *cp;
		int count = 0;

    switch (action)
    {
        case RESERVE1:
            if (var_val_type != ASN_OCTET_STR)
            {
                fprintf(stderr, "write to trapMgrCommunityName not ASN_OCTET_STR\n");
                return SNMP_ERR_WRONGTYPE;
            }
            if (var_val_len >= TRAP_COMMUNITY_SIZE)
            {
                fprintf(stderr,"write to trapMgrCommunityName: bad length\n");
                return SNMP_ERR_WRONGLENGTH;
            }
						/* purpose   	:  0012885 author : Gavin.Lin   date : 2010-07-12          */
						/* description  :  accept alphanumeric characters for SNMP community names */
						for (cp = var_val, count = 0; count < (int) var_val_len; count++, cp++)
						{
							if (!isalnum(*cp))
							{
								snmp_log(LOG_ERR, "not print %x\n", *cp);
								return SNMP_ERR_WRONGVALUE;
							}
						}
            break;

        case RESERVE2:
            break;
            
        case FREE:
            /* Release any resources that have been allocated */
            break;
            
        case ACTION:
            /* The variable has been stored in string for
                you to use, and you have just been asked to do something with
                it. Note that anything done here must be reversable in the UNDO case */
            break;

        case UNDO:
            /* Back out any changes made in the ACTION case */
            break;
            
        case COMMIT:
            /* Things are working well, so it's now safe to make the change
                permanently.  Make sure that anything done here can't fail! */
						memset(string, 0, sizeof(string));
            sprintf(string, "SNMP TCNAME=%s", (char *)var_val);
            string[var_val_len+12] = '\0';
						kd_doCommand(string, CMD_WRITE, ASH_DO_NOTHING, NULL);
						kd_updateFlash(USER_CHANGE_DB);
            break;
    } /* end of switch */
    return SNMP_ERR_NOERROR;
}


int write_advMgtSNMPStatus(int   action,
                        u_char   *var_val,
                        u_char   var_val_type,
                        size_t   var_val_len,
                        u_char   *statP,
                        oid      *name,
                        size_t   name_len)
{
    static long long_ret;

    switch (action)
    {
        case RESERVE1:
            if (var_val_type != ASN_INTEGER)
            {
                fprintf(stderr, "write to advMgtSNMPStatus not ASN_INTEGER\n");
                return SNMP_ERR_WRONGTYPE;
            }
            if (var_val_len > sizeof(long))
            {
                fprintf(stderr,"write to advMgtSNMPStatus: bad length\n");
                return SNMP_ERR_WRONGLENGTH;
            }
            if (*(long *)var_val != 0 && *(long *)var_val != 1)
            {
                fprintf(stderr, "write to advMgtSNMPStatus: bad value\n");
                return SNMP_ERR_WRONGVALUE;
            }
            break;
        case RESERVE2:
            long_ret = *(long *)var_val;
            break;
        case FREE:
            /* Release any resources that have been allocated */
            break;
        case ACTION:
            /* The variable has been stored in long_ret for
                you to use, and you have just been asked to do something with
                it.  Note that anything done here must be reversable in the UNDO case */
            break;
        case UNDO:
            /* Back out any changes made in the ACTION case */
            break;
        case COMMIT:
            /* Things are working well, so it's now safe to make the change
                permanently.  Make sure that anything done here can't fail! */
						if (long_ret == 1)
							kd_doCommand("SNMP ENABLED=YES", CMD_WRITE, ASH_DO_NOTHING, NULL);
						else
							kd_doCommand("SNMP ENABLED=NO", CMD_WRITE, ASH_DO_NOTHING, NULL);
						kd_updateFlash(USER_CHANGE_DB);
						kd_doCommand(NULL, CMD_WRITE, ASH_PAGE_SNMP, NULL);
            break;
    }
    return SNMP_ERR_NOERROR;
}


int write_advMgtUPnPStatus(int   action,
                        u_char   *var_val,
                        u_char   var_val_type,
                        size_t   var_val_len,
                        u_char   *statP,
                        oid      *name,
                        size_t   name_len)
{
    static long long_ret;

    switch (action)
    {
        case RESERVE1:
            if (var_val_type != ASN_INTEGER)
            {
                fprintf(stderr, "write to advMgtUPnPStatus not ASN_INTEGER\n");
                return SNMP_ERR_WRONGTYPE;
            }
            if (var_val_len > sizeof(long))
            {
                fprintf(stderr,"write to advMgtUPnPStatus: bad length\n");
                return SNMP_ERR_WRONGLENGTH;
            }
            if (*(long *)var_val != 0 && *(long *)var_val != 1)
            {
                fprintf(stderr, "write to advMgtUPnPStatus: bad value\n");
                return SNMP_ERR_WRONGVALUE;
            }
            break;

        case RESERVE2:
            long_ret = *(long *)var_val;
            break;

        case FREE:
            /* Release any resources that have been allocated */
            break;
            
        case ACTION:
            /* The variable has been stored in long_ret for
                you to use, and you have just been asked to do something with
                it. Note that anything done here must be reversable in the UNDO case */
            break;

        case UNDO:
            /* Back out any changes made in the ACTION case */
            break;
            
        case COMMIT:
            /* Things are working well, so it's now safe to make the change
                permanently.  Make sure that anything done here can't fail! */
						if (long_ret == 1)
							kd_doCommand("UPNP UPNPENABLE=YES", CMD_WRITE, ASH_DO_NOTHING, NULL);
						else
							kd_doCommand("UPNP UPNPENABLE=NO", CMD_WRITE, ASH_DO_NOTHING, NULL);
						kd_updateFlash(USER_CHANGE_DB);
						kd_doCommand(NULL, CMD_WRITE, ASH_PAGE_UPNP, NULL);
            break;
    } /* end of switch */
    return SNMP_ERR_NOERROR;
}
