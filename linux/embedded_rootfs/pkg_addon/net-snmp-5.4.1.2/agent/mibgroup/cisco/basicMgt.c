/* purpose   	: SNMP
 * author    	: Gavin.Lin		
 * date		: 2010-05-28
 * description  : Add '\0' on SNMP string
 *                ex : string[var_val_len] = '\0';
 */


struct variable4 basicMgt_variables[] = {
  { MGTSYSTEMRESET      , ASN_INTEGER   , RWRITE, var_basicMgt, 1, { 1 } },
  { MGTFACTORYRESET     , ASN_INTEGER   , RONLY, var_basicMgt, 1, { 2 } },
  /* purpose   	:  SNMP author :  Gavin.Lin  date :  2010-05-28                                         */	
  /* description  :  Customer requirements, set MGTADMINISTRATOR & MGTADMINUSERNAME access read only    */
  { MGTADMINISTRATOR    , ASN_OCTET_STR , RONLY, var_basicMgt, 1, { 3 } },
  { MGTADMINUSERNAME    , ASN_OCTET_STR , RONLY, var_basicMgt, 1, { 4 } },
  { MGTHOSTNAME         , ASN_OCTET_STR , RWRITE, var_basicMgt, 1, { 6 } },
  { MGTDOMAINNAME       , ASN_OCTET_STR , RWRITE, var_basicMgt, 1, { 7 } },
  { MGTNODENETADDRESS   , ASN_IPADDRESS , RWRITE, var_basicMgt, 1, { 8 } },
  { MGTNODESUBNETMASK   , ASN_IPADDRESS , RWRITE, var_basicMgt, 1, { 9 } },
  { MGTDHCPSTATUS       , ASN_INTEGER   , RWRITE, var_basicMgt, 1, { 10 } },
  { MGTDHCPSTARTNETADDR , ASN_IPADDRESS , RWRITE, var_basicMgt, 1, { 11 } },
  { MGTDHCPNUMBERUSERS  , ASN_INTEGER   , RWRITE, var_basicMgt, 1, { 12 } },
  { MGTCOMMUNITYINDEX   , ASN_INTEGER   , RONLY , var_mgtCommunityTable, 3, { 13,1,1 } },
  { MGTCOMMUNITYNAME    , ASN_OCTET_STR , RWRITE, var_mgtCommunityTable, 3, { 13,1,2 } },
  { MGTCOMMUNITYTYPE    , ASN_INTEGER   , RWRITE, var_mgtCommunityTable, 3, { 13,1,3 } },
};

oid basicMgt_variables_oid[] = { VENDOR_OID, 2, 1 };

u_char *var_basicMgt(	struct variable *vp,
						oid * name,
						size_t * length,
						int exact, size_t * var_len, WriteMethod ** write_method)
{
	//static unsigned long ret;
	static char DBdata[64];
	int pos = 0;
	static long long_ret;
	static struct in_addr address;
	char *p;
	char start_ip[16], end_ip[16];	
	FILE *fp;

	if (header_generic(vp, name, length, exact, var_len, write_method) == MATCH_FAILED)
		return NULL;

	switch(vp->magic)
	{
		case MGTSYSTEMRESET:
			*write_method = write_mgtSystemReset;
			/* purpose   	:  0012890 author : Gavin.Lin   date : 2010-07-13  */
			/* description  :  Customer requirements, record startup status  */
			fp = fopen("/tmp/.startup", "r");
			if (fp == NULL)
			{
				return NULL;
			}
			else
			{
				fgets(DBdata, 64, fp);
				if(!strncmp(DBdata, "warm", 4))
				{
					long_ret = 1;
				}
				else
				{
					long_ret = 2;
				}
				fclose(fp);
			}
			return (unsigned char *) &long_ret;
		case MGTFACTORYRESET:
			//*write_method = write_mgtFactoryReset;
			long_ret = 0;
			return (unsigned char *) &long_ret;
		case MGTADMINISTRATOR:
			//*write_method = write_mgtAdministrator;
			kd_doCommand("SYSTEM PASSWD", CMD_PRINT, ASH_DO_NOTHING, DBdata);
			*var_len = strlen(DBdata);
			return (unsigned char *) DBdata;
		case MGTADMINUSERNAME:
			//*write_method = write_mgtAdminUsername;
			kd_doCommand("SYSTEM PD_USERNAME", CMD_PRINT, ASH_DO_NOTHING, DBdata);
			*var_len = strlen(DBdata);
			return (unsigned char *) DBdata;
		case MGTHOSTNAME:
			*write_method = write_mgtHostName;
			kd_doCommand("SYSTEM HOSTNAME", CMD_PRINT, ASH_DO_NOTHING, DBdata);
			*var_len = strlen(DBdata);
			return (unsigned char *) DBdata;
		case MGTDOMAINNAME:
			*write_method = write_mgtDomainName;
			kd_doCommand("SYSTEM DOMAINNAME", CMD_PRINT, ASH_DO_NOTHING, DBdata);
			*var_len = strlen(DBdata);
			return (unsigned char *) DBdata;
		case MGTNODENETADDRESS:
			*write_method = write_mgtNodeNetAddress;
			kd_doCommand("SYSTEM LAN", CMD_PRINT, ASH_DO_NOTHING, DBdata);
			if (inet_addr(DBdata)==0xFFFFFFFF)
				sprintf(DBdata, "0.0.0.0");
			address.s_addr = inet_addr(DBdata);
			*var_len = 4;
			return (unsigned char *)&address.s_addr;
		case MGTNODESUBNETMASK:
			*write_method = write_mgtNodeSubnetMask;
			kd_doCommand("SYSTEM LANMASK", CMD_PRINT, ASH_DO_NOTHING, DBdata);
			if (inet_addr(DBdata)==0xFFFFFFFF)
				sprintf(DBdata, "0.0.0.0");
			address.s_addr = inet_addr(DBdata);
			*var_len = 4;
			return (unsigned char *)&address.s_addr;
		case MGTDHCPSTATUS:
			*write_method = write_mgtDhcpStatus;
			kd_doCommand("DHCP SERVER", CMD_PRINT, ASH_DO_NOTHING, DBdata);
			if (!strcmp(DBdata, "YES"))
				long_ret = 1;
			else
				long_ret = 0;
			return (unsigned char *) &long_ret;
		case MGTDHCPSTARTNETADDR:
			*write_method = write_mgtDhcpStartNetAddr;
			kd_doCommand("DHCP_SUBNET1 RANGE", CMD_PRINT, ASH_DO_NOTHING, DBdata);
			p = strchr(DBdata, ':');
			*p = ' ';
			sscanf(DBdata, "%s %s", start_ip, end_ip);
			if (inet_addr(start_ip)==0xFFFFFFFF)
				sprintf(DBdata, "0.0.0.0");
			address.s_addr = inet_addr(start_ip);
			*var_len = 4;
			return (unsigned char *)&address.s_addr;
		case MGTDHCPNUMBERUSERS:
			*write_method = write_mgtDhcpNumberUsers;
			kd_doCommand("DHCP_SUBNET1 RANGE", CMD_PRINT, ASH_DO_NOTHING, DBdata);
			p = strchr(DBdata, ':');
			*p = ' ';
			sscanf(DBdata, "%s %s", start_ip, end_ip);
			long_ret = htonl(inet_addr(end_ip)) - htonl(inet_addr(start_ip)) + 1;
			return (unsigned char *) &long_ret;
		default:
			ERROR_MSG("");
	}

	return NULL;
}



unsigned char *var_mgtCommunityTable(struct variable *vp,
                                    oid     *name,
                                    size_t  *length,
                                    int     exact,
                                    size_t  *var_len,
                                    WriteMethod **write_method)
{
    static long long_ret;
    //static unsigned char string[SPRINT_MAX_LEN];
		static char DBdata[64];
    char    idx;

		#ifdef TABLE_SIZE
		#undef TABLE_SIZE
		#endif
    #define TABLE_SIZE 2
    if (header_simple_table(vp,name,length,exact,var_len,write_method, TABLE_SIZE)
                                                            == MATCH_FAILED )
        return NULL;


    idx = name[*length-1];
    if (idx == 1)
    {
				kd_doCommand("SNMP GCNAME", CMD_PRINT, ASH_DO_NOTHING, DBdata);
        long_ret = 1;
    }
    else if (idx == 2)
    {
        kd_doCommand("SNMP SCNAME", CMD_PRINT, ASH_DO_NOTHING, DBdata);
        long_ret = 2;
    }
    else
        return NULL;


    /* this is where we do the value assignments for the mib results. */
    switch (vp->magic)
    {
        case MGTCOMMUNITYINDEX:
            long_ret = idx;
            return (unsigned char *) &long_ret;

        case MGTCOMMUNITYNAME:
						*write_method = write_mgtCommunityName;
						*var_len = strlen(DBdata);
						return (unsigned char *) DBdata;

        case MGTCOMMUNITYTYPE:
            *write_method = write_mgtCommunityType;
            return (unsigned char *) &long_ret;

        default:
            ERROR_MSG("");
    }
    return NULL;
}


int write_mgtSystemReset(int     action,
                        u_char   *var_val,
                        u_char   var_val_type,
                        size_t   var_val_len,
                        u_char   *statP,
                        oid      *name,
                        size_t   name_len)
{
	switch (action)
	{
		case RESERVE1:  /* check values for acceptability */
			if (var_val_type != ASN_INTEGER)
			{
				fprintf(stderr, "write to mgtSystemReset not ASN_INTEGER\n");
				return SNMP_ERR_WRONGTYPE;
			}
			if (var_val_len > sizeof(long))
			{
				fprintf(stderr, "write to mgtSystemReset: bad length\n");
				return SNMP_ERR_WRONGLENGTH;
			}
			if (*(long *)var_val != 1 && *(long *)var_val != 2)
			{
				fprintf(stderr, "write to mgtSystemReset: bad value\n");
				return SNMP_ERR_WRONGVALUE;
			}
			break;

		case RESERVE2:  /* allocate memory and similar resources */
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
			/* purpose   	:  0012890 author : Gavin.Lin   date : 2010-07-13  */
			/* description  :  Customer requirements, do warm/cold start     */
			if (*(long *)var_val == 1 )
			{
				kd_doCommand(NULL, (int)NULL, ASH_SYSTEM_REBOOT, NULL);
			}
			else
			{
				system("reboot");
			}
			break;
	}
	return SNMP_ERR_NOERROR;
}

#if 0
int write_mgtFactoryReset(int    action,
                        u_char   *var_val,
                        u_char   var_val_type,
                        size_t   var_val_len,
                        u_char   *statP,
                        oid      *name,
                        size_t   name_len)
{
    static long     long_rst;
    
    switch (action)
    {
        case RESERVE1:  /* check values for acceptability */
            if (var_val_type != ASN_INTEGER)
            {
                fprintf(stderr, "write to mgtFactoryReset not ASN_INTEGER\n");
                return SNMP_ERR_WRONGTYPE;
            }
            if (var_val_len > sizeof(long))
            {
                fprintf(stderr,"write to mgtFactoryReset: bad length\n");
                return SNMP_ERR_WRONGLENGTH;
            }
            if (*(long *)var_val != 0 && *(long *)var_val != 1)
            {
                fprintf(stderr, "write to mgtSystemReset: bad value\n");
                return SNMP_ERR_WRONGVALUE;
            }
            break;

        case RESERVE2:
            long_rst = *(long *)var_val;
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
            if (long_rst)
            {
              system("factoryDefault");
            }
            break;
    }
    return SNMP_ERR_NOERROR;
}

#endif

int write_mgtAdministrator(int   action,
                        u_char   *var_val,
                        u_char   var_val_type,
                        size_t   var_val_len,
                        u_char   *statP,
                        oid      *name,
                        size_t   name_len)
{
    #define PASSWD_MAX_LEN 65
    static unsigned char string[16+PASSWD_MAX_LEN];
		static unsigned char passwd[PASSWD_MAX_LEN];
		char DBdata[64];

    switch ( action )
    {
        case RESERVE1:
            if (var_val_type != ASN_OCTET_STR)
            {
                fprintf(stderr, "write to mgtAdministrator not ASN_OCTET_STR\n");
                return SNMP_ERR_WRONGTYPE;
            }
            if (var_val_len >= PASSWD_MAX_LEN)
            {
                fprintf(stderr,"write to mgtAdministrator: bad length\n");
                return SNMP_ERR_WRONGLENGTH;
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
                it.  Note that anything done here must be reversable in the UNDO case */
            break;
            
        case UNDO:
            /* Back out any changes made in the ACTION case */
            break;
            
        case COMMIT:
            /* Things are working well, so it's now safe to make the change
                permanently.  Make sure that anything done here can't fail! */
						sprintf(passwd, "%s", (char *)var_val);
						passwd[var_val_len] = '\0';
						kd_doCommand("SYSTEM PASSWD", CMD_PRINT, ASH_DO_NOTHING, DBdata);
						if (strcmp(passwd, DBdata))
						{
							sprintf(string, "SYSTEM PASSWD=%s", passwd);
							kd_doCommand(string, CMD_WRITE, ASH_DO_NOTHING, NULL);
							kd_updateFlash(USER_CHANGE_DB);
						}

            break;
    }
    return SNMP_ERR_NOERROR;
}


int write_mgtAdminUsername(int   action,
                        u_char   *var_val,
                        u_char   var_val_type,
                        size_t   var_val_len,
                        u_char   *statP,
                        oid      *name,
                        size_t   name_len)
{
    #define USERNAME_MAX_LEN 65
    static unsigned char string[16+USERNAME_MAX_LEN];
		static unsigned char uname[USERNAME_MAX_LEN];
    switch (action)
    {
        case RESERVE1:
            if (var_val_type != ASN_OCTET_STR)
            {
                fprintf(stderr, "write to mgtAdminUsername not ASN_OCTET_STR\n");
                return SNMP_ERR_WRONGTYPE;
            }
            if (var_val_len >= USERNAME_MAX_LEN)
            {
                fprintf(stderr,"write to mgtAdminUsername: bad length\n");
                return SNMP_ERR_WRONGLENGTH;
            }
            break;

        case RESERVE2:
            sprintf(uname, "%s", (char *)var_val);
            uname[var_val_len] = '\0';
            break;

        case FREE:
            /* Release any resources that have been allocated */
            break;

        case ACTION:
            /* The variable has been stored in string for
                you to use, and you have just been asked to do something with
                it.  Note that anything done here must be reversable in the UNDO case */
            break;

        case UNDO:
            /* Back out any changes made in the ACTION case */
            break;

        case COMMIT:
            /* Things are working well, so it's now safe to make the change
                permanently.  Make sure that anything done here can't fail! */
						sprintf(string, "SYSTEM PD_USERNAME=%s", uname);
						kd_doCommand(string, CMD_WRITE, ASH_DO_NOTHING, NULL);
						kd_updateFlash(USER_CHANGE_DB);
            break;
    }
    return SNMP_ERR_NOERROR;
}

int write_mgtHostName(int    action,
                    u_char   *var_val,
                    u_char   var_val_type,
                    size_t   var_val_len,
                    u_char   *statP,
                    oid      *name,
                    size_t   name_len)
{
    #define HOSTNAME_MAX_LEN 33
		static unsigned char hostname[HOSTNAME_MAX_LEN];
    static unsigned char string[20+HOSTNAME_MAX_LEN];

    switch( action )
    {
        case RESERVE1:
            if (var_val_type != ASN_OCTET_STR)
            {
                fprintf(stderr, "write to mgtHostName not ASN_OCTET_STR\n");
                return SNMP_ERR_WRONGTYPE;
            }
            if (var_val_len >= HOSTNAME_MAX_LEN)
            {
                fprintf(stderr,"write to mgtHostName: bad length\n");
                return SNMP_ERR_WRONGLENGTH;
            }
            break;

        case RESERVE2:
            sprintf(hostname, "%s", (char *)var_val);
            hostname[var_val_len] = '\0';
            break;

        case FREE:
            /* Release any resources that have been allocated */
            break;

        case ACTION:
            /* The variable has been stored in string for
                you to use, and you have just been asked to do something with
                it.  Note that anything done here must be reversable in the UNDO case */
            break;

        case UNDO:
            /* Back out any changes made in the ACTION case */
            break;

        case COMMIT:
            /* Things are working well, so it's now safe to make the change
                permanently.  Make sure that anything done here can't fail! */
						sprintf(string, "SYSTEM HOSTNAME=%s", hostname);
						kd_doCommand(string, CMD_WRITE, ASH_DO_NOTHING, NULL);
						kd_updateFlash(USER_CHANGE_DB);
						sprintf(string, "hostname %s", hostname);
						system(string);
            break;
    }
    return SNMP_ERR_NOERROR;
}

int write_mgtDomainName(int  action,
                    u_char   *var_val,
                    u_char   var_val_type,
                    size_t   var_val_len,
                    u_char   *statP,
                    oid      *name,
                    size_t   name_len)
{
    #define DOMAINNAME_MAX_LEN 65
    static unsigned char string[20+DOMAINNAME_MAX_LEN];
		static unsigned char domainname[DOMAINNAME_MAX_LEN];
 
    switch (action)
    {
        case RESERVE1:
            if (var_val_type != ASN_OCTET_STR)
            {
                fprintf(stderr, "write to mgtDomainName not ASN_OCTET_STR\n");
                return SNMP_ERR_WRONGTYPE;
            }
            if (var_val_len >= DOMAINNAME_MAX_LEN)
            {
                fprintf(stderr,"write to mgtDomainName: bad length\n");
                return SNMP_ERR_WRONGLENGTH;
            }
            break;

        case RESERVE2:
            sprintf(domainname, "%s", (char *)var_val);
            domainname[var_val_len] = '\0';
            break;

        case FREE:
            /* Release any resources that have been allocated */
            break;

        case ACTION:
            /* The variable has been stored in string for
                you to use, and you have just been asked to do something with
                it.  Note that anything done here must be reversable in the UNDO case */
            break;

        case UNDO:
            /* Back out any changes made in the ACTION case */
            break;

        case COMMIT:
            /* Things are working well, so it's now safe to make the change
                permanently. Make sure that anything done here can't fail! */
						sprintf(string, "SYSTEM DOMAINNAME=%s", domainname);
						kd_doCommand(string, CMD_WRITE, ASH_DO_NOTHING, NULL);
						kd_updateFlash(USER_CHANGE_DB);
						/* purpose   	:  0013212 author : Gavin.Lin   date : 2010-08-19  */
						/* description  :  when domain name be update, restart udhcpd    */
						kd_doCommand(NULL, CMD_WRITE, ASH_PAGE_DHCP, NULL);
            break;
    }
    return SNMP_ERR_NOERROR;
}

int write_mgtNodeNetAddress(int      action,
                            u_char   *var_val,
                            u_char   var_val_type,
                            size_t   var_val_len,
                            u_char   *statP,
                            oid      *name,
                            size_t   name_len)
{
    static struct in_addr ip_addr;
    const int IP_LENGTH = 4;
		static unsigned char string[64];
		
    switch (action)
    {
        case RESERVE1:
            if (var_val_type != ASN_IPADDRESS)
            {
                fprintf(stderr, "write to mgtNodeNetAddress not ASN_IPADDRESS\n");
                return SNMP_ERR_WRONGTYPE;
            }
            if (var_val_len != IP_LENGTH)
            {
                fprintf(stderr,"write to mgtNodeNetAddress: bad length\n");
                return SNMP_ERR_WRONGLENGTH;
            }
            /* check ip value */
            if (var_val[0] > 255 || var_val[1] > 255 || var_val[2] > 255 ||
                var_val[3] > 254)
            {
                fprintf(stderr,"write to mgtNodeNetAddress: bad value\n");
                return SNMP_ERR_WRONGVALUE;
            }
            break;

        case RESERVE2:
            memcpy((char *)&ip_addr.s_addr, (char *)var_val, IP_LENGTH);
            break;

        case FREE:
            /* Release any resources that have been allocated */
            break;

        case ACTION:
            /* The variable has been stored in string for
                you to use, and you have just been asked to do something with
                it.  Note that anything done here must be reversable in the UNDO case */
            break;

        case UNDO:
            /* Back out any changes made in the ACTION case */
            break;

        case COMMIT:
            /* Things are working well, so it's now safe to make the change
                permanently.  Make sure that anything done here can't fail! */
						memset(string, 0, sizeof(string));
            sprintf(string, "SYSTEM LAN=%s", inet_ntoa(ip_addr));
						kd_doCommand(string, CMD_WRITE, ASH_DO_NOTHING, NULL);
						kd_updateFlash(USER_CHANGE_DB);
						kd_doCommand(NULL, (int)NULL, ASH_PAGE_SETUP, NULL);
						//kd_doCommand(NULL, CMD_WRITE, ASH_PAGE_DHCP, NULL);
            break;
    }
    return SNMP_ERR_NOERROR;
}


int write_mgtNodeSubnetMask(int      action,
                            u_char   *var_val,
                            u_char   var_val_type,
                            size_t   var_val_len,
                            u_char   *statP,
                            oid      *name,
                            size_t   name_len)
{
    static struct in_addr netmask;
    const int IP_LENGTH = 4;
		static unsigned char string[64];

    switch (action)
    {
        case RESERVE1:
            if (var_val_type != ASN_IPADDRESS)
            {
                fprintf(stderr, "write to mgtNodeSubnetMask not ASN_IPADDRESS\n");
                return SNMP_ERR_WRONGTYPE;
            }
            if (var_val_len != IP_LENGTH)
            {
                fprintf(stderr,"write to mgtNodeSubnetMask: bad length\n");
                return SNMP_ERR_WRONGLENGTH;
            }
            /* check subnet mask value */
            if (var_val[0] > 255 || var_val[1] > 255 || var_val[2] > 255 ||
                var_val[3] > 254)
            {
                fprintf(stderr,"write to mgtNodeSubnetMask: bad value\n");
                return SNMP_ERR_WRONGVALUE;
            }
						if (var_val[0] != 255 && var_val[1] != 255 && var_val[2] != 255)
						{
                fprintf(stderr,"write to mgtNodeSubnetMask: bad value\n");
                return SNMP_ERR_WRONGVALUE;
						}
						if (!(var_val[3] == 0 || var_val[3] ==  128 || var_val[3] == 192 || var_val[3] == 224 ||
						    var_val[3] == 240 || var_val[3] == 284 || var_val[3] == 252))
						{
                fprintf(stderr,"write to mgtNodeSubnetMask: bad value\n");
                return SNMP_ERR_WRONGVALUE;
						}
            break;
            
        case RESERVE2:
            memcpy((char *)&netmask.s_addr, (char *)var_val, IP_LENGTH);
            break;

        case FREE:
            /* Release any resources that have been allocated */
            break;

        case ACTION:
            /* The variable has been stored in string for
                you to use, and you have just been asked to do something with
                it.  Note that anything done here must be reversable in the UNDO case */
            break;

        case UNDO:
            /* Back out any changes made in the ACTION case */
            break;

        case COMMIT:
            /* Things are working well, so it's now safe to make the change
                permanently.  Make sure that anything done here can't fail! */
						memset(string, 0, sizeof(string));
            sprintf(string, "SYSTEM LANMASK=%s", inet_ntoa(netmask));
						kd_doCommand(string, CMD_WRITE, ASH_DO_NOTHING, NULL);
						kd_updateFlash(USER_CHANGE_DB);
						kd_doCommand(NULL, (int)NULL, ASH_PAGE_SETUP, NULL);
						//kd_doCommand(NULL, CMD_WRITE, ASH_PAGE_DHCP, NULL);
						break;
    }
    return SNMP_ERR_NOERROR;
}

write_mgtDhcpStatus(int      action,
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
                fprintf(stderr, "write to mgtDhcpStatus not ASN_INTEGER\n");
                return SNMP_ERR_WRONGTYPE;
            }
            if (var_val_len > sizeof(long))
            {
                fprintf(stderr,"write to mgtDhcpStatus: bad length\n");
                return SNMP_ERR_WRONGLENGTH;
            }
            if (*(long *)var_val != 0 && *(long *)var_val != 1)
            {
                fprintf(stderr, "write to mgtDhcpStatus: bad value\n");
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
							kd_doCommand("DHCP SERVER=YES", CMD_WRITE, ASH_DO_NOTHING, NULL);
						else
							kd_doCommand("DHCP SERVER=NO", CMD_WRITE, ASH_DO_NOTHING, NULL);
						kd_updateFlash(USER_CHANGE_DB);
						kd_doCommand(NULL, CMD_WRITE, ASH_PAGE_DHCP, NULL);
            break;
    }
    return SNMP_ERR_NOERROR;
}

int write_mgtCommunityName(int   action,
                        u_char   *var_val,
                        u_char   var_val_type,
                        size_t   var_val_len,
                        u_char   *statP,
                        oid      *name,
                        size_t   name_len)
{
    #define COMM_MAX_LEN 33
    static unsigned char string[24+COMM_MAX_LEN];
    char idx = name[name_len-1];
    u_char         *cp;
		int count = 0;
 
    switch (action)
    {
        case RESERVE1:
            if (var_val_type != ASN_OCTET_STR)
            {
                fprintf(stderr, "write to mgtCommunityName not ASN_OCTET_STR\n");
                return SNMP_ERR_WRONGTYPE;
            }
            if (var_val_len >= COMM_MAX_LEN)
            {
                fprintf(stderr,"write to mgtCommunityName: bad length\n");
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
                it.  Note that anything done here must be reversable in the UNDO case */
            break;

        case UNDO:
            /* Back out any changes made in the ACTION case */
            break;

        case COMMIT:
            /* Things are working well, so it's now safe to make the change
                permanently.  Make sure that anything done here can't fail! */
            
            
            if (idx == 1)
								sprintf(string, "SNMP GCNAME=%s", (char *)var_val);
            else if (idx == 2)
								sprintf(string, "SNMP SCNAME=%s", (char *)var_val);
						string[var_val_len+12] = '\0';
						kd_doCommand(string, CMD_WRITE, ASH_DO_NOTHING, NULL);
						kd_updateFlash(USER_CHANGE_DB);
						kd_doCommand(NULL, CMD_WRITE, ASH_PAGE_SNMP, NULL);
            break;
    }
    return SNMP_ERR_NOERROR;
}


int write_mgtCommunityType(int   action,
                        u_char   *var_val,
                        u_char   var_val_type,
                        size_t   var_val_len,
                        u_char   *statP,
                        oid      *name,
                        size_t   name_len)
{
    switch (action)
    {
        case RESERVE1:
            if (var_val_type != ASN_INTEGER)
            {
                fprintf(stderr, "write to mgtCommunityType not ASN_INTEGER\n");
                return SNMP_ERR_WRONGTYPE;
            }
            if (var_val_len > sizeof(long))
            {
                fprintf(stderr,"write to mgtCommunityType: bad length\n");
                return SNMP_ERR_WRONGLENGTH;
            }
            if (*(long *)var_val != 1 && *(long *)var_val != 2)
            {
                fprintf(stderr, "write to mgtCommunityType: bad value\n");
                return SNMP_ERR_WRONGVALUE;
            }
            break;

        case RESERVE2:
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
            break;
    }
    return SNMP_ERR_NOERROR;
}

int write_mgtDhcpStartNetAddr(int    action,
                            u_char   *var_val,
                            u_char   var_val_type,
                            size_t   var_val_len,
                            u_char   *statP,
                            oid      *name,
                            size_t   name_len)
{
//    dhcps_info_t dhcps_info;
//    static struct in_addr lan_ip, lan_netmask, start_ip;
    const int IP_LENGTH = 4;
//    int             users_cnt;
//    request_t       req;
		static struct in_addr ip_addr;
		static char lan_ip[16];
		static char start_ip[16], end_ip[16];
		static char *p;
		static int ip1, ip2, ip3, ip4;
		static char DBdata[64];
		static int cnt;
		
    switch (action)
    {
        case RESERVE1:
            if (var_val_type != ASN_IPADDRESS)
            {
                fprintf(stderr, "write to mgtDhcpStartNetAddr not ASN_IPADDRESS\n");
                return SNMP_ERR_WRONGTYPE;
            }
            if (var_val_len != IP_LENGTH)
            {
                fprintf(stderr,"write to mgtDhcpStartNetAddr: bad length\n");
                return SNMP_ERR_WRONGLENGTH;
            }
            /* check ip value */
            kd_doCommand("SYSTEM LAN", CMD_PRINT, ASH_DO_NOTHING, lan_ip);
						memcpy((char *)&ip_addr.s_addr, (char *)var_val, IP_LENGTH);
            if ( !strcmp(lan_ip, inet_ntoa(ip_addr)) ||
                (var_val[3] > 254)  || (var_val[3] == 0))
            {
                fprintf(stderr, "write to mgtDhcpStartNetAddr: bad value\n");
                return SNMP_ERR_WRONGVALUE;
            }
						kd_doCommand("DHCP_SUBNET1 RANGE", CMD_PRINT, ASH_DO_NOTHING, DBdata);
						p = strchr(DBdata, ':');
						*p = ' ';
						sscanf(DBdata, "%s %s", start_ip, end_ip);
						cnt = htonl(inet_addr(end_ip)) - htonl(inet_addr(start_ip)) + 1;
						sscanf(inet_ntoa(ip_addr), "%d.%d.%d.%d", &ip1, &ip2, &ip3, &ip4);
						if ((ip4 + cnt - 1)> 254)
						{
                fprintf(stderr, "write to mgtDhcpStartNetAddr: bad value\n");
                return SNMP_ERR_WRONGVALUE;						
						}
            break;

        case RESERVE2:
            //sprintf(start_ip, "%s", inet_ntoa(ip_addr));
            break;

        case FREE:
            /* Release any resources that have been allocated */
            break;

        case ACTION:
            /* The variable has been stored in string for
                you to use, and you have just been asked to do something with
                it.  Note that anything done here must be reversable in the UNDO case */
            break;

        case UNDO:
            /* Back out any changes made in the ACTION case */
            break;

        case COMMIT:
            /* Things are working well, so it's now safe to make the change
                permanently.  Make sure that anything done here can't fail! */
						sprintf(DBdata, "DHCP_SUBNET1 RANGE=%s:%d.%d.%d.%d", inet_ntoa(ip_addr), ip1, ip2, ip3, (ip4 + cnt - 1 ));
						kd_doCommand(DBdata, CMD_WRITE, ASH_DO_NOTHING, NULL);
						kd_updateFlash(USER_CHANGE_DB);
						kd_doCommand(NULL, CMD_WRITE, ASH_PAGE_DHCP, NULL);
            break;
    }
    return SNMP_ERR_NOERROR;
}

int write_mgtDhcpNumberUsers(int      action,
                            u_char   *var_val,
                            u_char   var_val_type,
                            size_t   var_val_len,
                            u_char   *statP,
                            oid      *name,
                            size_t   name_len)
{
		static char start_ip[16], end_ip[16];
		static char *p;
		static int ip1, ip2, ip3, ip4;
		static char DBdata[64];
		kd_doCommand("DHCP_SUBNET1 RANGE", CMD_PRINT, ASH_DO_NOTHING, DBdata);
		p = strchr(DBdata, ':');
		*p = ' ';
		sscanf(DBdata, "%s %s", start_ip, end_ip);
    switch (action)
    {
        case RESERVE1:
            if (var_val_type != ASN_INTEGER)
            {
                fprintf(stderr, "write to mgtDhcpNumberUsers not ASN_INTEGER\n");
                return SNMP_ERR_WRONGTYPE;
            }
            if (var_val_len > sizeof(long))
            {
                fprintf(stderr,"write to mgtDhcpNumberUsers: bad length\n");
                return SNMP_ERR_WRONGLENGTH;
            }
						sscanf(start_ip, "%d.%d.%d.%d", &ip1, &ip2, &ip3, &ip4);
            if (*(long *)var_val < 0 ||
                (ip4 + *(long *)var_val - 1 ) > 254)
            {
                fprintf(stderr, "write to mgtDhcpNumberUsers: bad value\n");
                return SNMP_ERR_WRONGVALUE;
            }
            break;

        case RESERVE2:
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
						sprintf(DBdata, "DHCP_SUBNET1 RANGE=%s:%d.%d.%d.%d", start_ip, ip1, ip2, ip3, (ip4 + *(long *)var_val - 1 ));
						kd_doCommand(DBdata, CMD_WRITE, ASH_DO_NOTHING, NULL);	
						kd_updateFlash(USER_CHANGE_DB);
						kd_doCommand(NULL, CMD_WRITE, ASH_PAGE_DHCP, NULL);
            break;
    }
    return SNMP_ERR_NOERROR;
}

