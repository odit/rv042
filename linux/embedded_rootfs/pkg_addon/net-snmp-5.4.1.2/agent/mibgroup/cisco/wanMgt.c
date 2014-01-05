/* purpose   	: SNMP
 * author    	: Gavin.Lin		
 * date		: 2010-05-28
 * description  : Add '\0' on SNMP string
 *                ex : string[var_val_len] = '\0';
 */
	
struct variable4 wanMgt_variables[] = {
  { WANINDEX            , ASN_INTEGER   , RONLY , var_wanConnectionTable, 3, { 1,1,1 } },
  { WANIFINDEX          , ASN_INTEGER   , RONLY , var_wanConnectionTable, 3, { 1,1,2 } },
  { WANCONNECTIONTYPE   , ASN_INTEGER   , RWRITE, var_wanConnectionTable, 3, { 1,1,3 } },
  { WANNETADDRESS       , ASN_IPADDRESS , RWRITE, var_wanConnectionTable, 3, { 1,1,4 } },
  { WANPHYSICALADDR     , ASN_OCTET_STR , RWRITE, var_wanConnectionTable, 3, { 1,1,5 } },
  { WANSUBNETMASK       , ASN_IPADDRESS , RWRITE, var_wanConnectionTable, 3, { 1,1,6 } },
  { WANDEFAULTGATEWAY   , ASN_IPADDRESS , RWRITE, var_wanConnectionTable, 3, { 1,1,7 } },
  { WANDHCPSTATUS       , ASN_INTEGER   , RWRITE, var_wanConnectionTable, 3, { 1,1,8 } },
	{ WANLOGINSTATUS      , ASN_INTEGER   , RWRITE, var_wanConnectionTable, 3, { 1,1,9 } },
  { WANLOGINUSERNAME    , ASN_OCTET_STR , RWRITE, var_wanConnectionTable, 3, { 1,1,10 } },
  { WANLOGINPASSWORD    , ASN_OCTET_STR , RWRITE, var_wanConnectionTable, 3, { 1,1,11 } },
  { WANWORKINGMODE      , ASN_INTEGER   , RWRITE, var_wanConnectionTable, 3, { 1,1,13 } },
  { WANCONNECTEDSTATE   , ASN_INTEGER   , RWRITE, var_wanConnectionTable, 3, { 1,1,14 } },
  { WANCONNECTEDIDLETIME, ASN_INTEGER   , RWRITE, var_wanConnectionTable, 3, { 1,1,15 } },
  { WANCONNECTEDREDIALPERIOD, ASN_INTEGER   , RWRITE, var_wanConnectionTable, 3, { 1,1,16 } },
  { WANDNSAUTONEGOSTATUS, ASN_INTEGER   , RWRITE, var_wanConnectionTable, 3, { 1,1,17 } },
/*
  { WANDNSNETADDRESSINDEX, ASN_INTEGER   , RONLY , var_wanDnsNetAddressTable, 3, { 2,1,1 } },
  { WANDNSIFINDEX       , ASN_INTEGER   , RONLY , var_wanDnsNetAddressTable, 3, { 2,1,2 } },
  { WANDNSNETADDRESS    , ASN_IPADDRESS , RWRITE, var_wanDnsNetAddressTable, 3, { 2,1,3 } },
*/
  { WAN1DNSNETADDRESSINDEX, ASN_INTEGER   , RONLY , var_wan1DnsNetAddressTable, 4, { 2,1,1,1 } },
  { WAN1DNSIFINDEX       , ASN_INTEGER   , RONLY , var_wan1DnsNetAddressTable, 4, { 2,1,2,1 } },
  { WAN1DNSNETADDRESS    , ASN_IPADDRESS , RWRITE, var_wan1DnsNetAddressTable, 4, { 2,1,3,1 } },
  { WAN2DNSNETADDRESSINDEX, ASN_INTEGER   , RONLY , var_wan2DnsNetAddressTable, 4, { 2,1,1,2 } },
  { WAN2DNSIFINDEX       , ASN_INTEGER   , RONLY , var_wan2DnsNetAddressTable, 4, { 2,1,2,2 } },
  { WAN2DNSNETADDRESS    , ASN_IPADDRESS , RWRITE, var_wan2DnsNetAddressTable, 4, { 2,1,3,2 } },
};

oid wanMgt_variables_oid[] = { VENDOR_OID, 2, 3 };

unsigned char *var_wanConnectionTable(struct variable *vp,
                                            oid     *name,
                                            size_t  *length,
                                            int     exact,
                                            size_t  *var_len,
                                            WriteMethod **write_method)
{
    static long long_ret;
    static unsigned char string[SPRINT_MAX_LEN];
    static struct in_addr address;
		char DBtmp[64];
		char DBdata[64];
		int wan_number;
		int mac1, mac2, mac3, mac4, mac5, mac6;

Repeat:
		kd_doCommand("VERSION NUM_WAN", CMD_PRINT, ASH_DO_NOTHING, DBdata);
		wan_number = atoi(DBdata);

		#ifdef TABLE_SIZE
		#undef TABLE_SIZE
		#endif
		#define TABLE_SIZE wan_number
    if (header_simple_table(vp,name,length,exact,var_len,write_method, TABLE_SIZE) == MATCH_FAILED)
        return NULL;


    switch (vp->magic)
    {
        case WANINDEX:
            long_ret = name[*length-1];
            return (unsigned char *) &long_ret;

        case WANIFINDEX:
            long_ret = name[*length-1];
            return (unsigned char *) &long_ret;
        case WANCONNECTIONTYPE:
            *write_method = write_wanConnectionType;
						sprintf(DBtmp, "WAN%d WANCONNECTION", name[*length-1]);
						kd_doCommand(DBtmp, CMD_PRINT, ASH_DO_NOTHING, DBdata);
						switch(atoi(DBdata))
						{
							case 0: // dynamic 
								long_ret = 1;
								break;
							case 1: // static
								long_ret = 2;
								break;
							case 2: // pppoe
								long_ret = 3;
								break;
							case 3: // pptp 
								long_ret = 5;
								break;
							default:
								return NULL;
						
						}
            return (unsigned char *) &long_ret;

        case WANNETADDRESS:
            *write_method = write_wanNetAddress;
						sprintf(DBtmp, "ISP%d WAN", name[*length-1]);
						kd_doCommand(DBtmp, CMD_PRINT, ASH_DO_NOTHING, DBdata);
						if (inet_addr(DBdata)==0xFFFFFFFF)
							sprintf(DBdata, "0.0.0.0");
						address.s_addr = inet_addr(DBdata);
            *var_len = 4;
            return (unsigned char *)&address.s_addr;

        case WANPHYSICALADDR:
            *write_method = write_wanPhysicalAddr;
						sprintf(DBtmp, "WAN%d MACCLONE", name[*length-1]);
						kd_doCommand(DBtmp, CMD_PRINT, ASH_DO_NOTHING, DBdata);
						sscanf(DBdata, "%02X%02X%02X%02X%02X%02X", &mac1, &mac2, &mac3, &mac4, &mac5, &mac6);
						sprintf(DBdata, "%02X:%02X:%02X:%02X:%02X:%02X", mac1, mac2, mac3, mac4, mac5, mac6);
            *var_len = strlen(DBdata);
            return (unsigned char *) DBdata;

        case WANSUBNETMASK:
            *write_method = write_wanSubnetMask;
						sprintf(DBtmp, "ISP%d WANMASK", name[*length-1]);
						kd_doCommand(DBtmp, CMD_PRINT, ASH_DO_NOTHING, DBdata);
						if (strcmp("255.255.255.255", DBdata) && inet_addr(DBdata)==0xFFFFFFFF)
							sprintf(DBdata, "0.0.0.0");
						address.s_addr = inet_addr(DBdata);
            *var_len = 4;
            return (unsigned char *)&address.s_addr;

        case WANDEFAULTGATEWAY:
            *write_method = write_wanDefaultGateway;
						sprintf(DBtmp, "ISP%d GATEWAY", name[*length-1]);
						kd_doCommand(DBtmp, CMD_PRINT, ASH_DO_NOTHING, DBdata);
						if (inet_addr(DBdata)==0xFFFFFFFF)
							sprintf(DBdata, "0.0.0.0");
						address.s_addr = inet_addr(DBdata);
            *var_len = 4;
            return (unsigned char *)&address.s_addr;
        case WANDHCPSTATUS:
						sprintf(DBtmp, "WAN%d WANCONNECTION", name[*length-1]);
						kd_doCommand(DBtmp, CMD_PRINT, ASH_DO_NOTHING, DBdata);
						if (atoi(DBdata) != 0)
						{
                if (!exact)
                    goto Repeat;
								return NULL;
						}
            *write_method = write_wanDHCPStatus;
            long_ret = 0;
            return (unsigned char *) &long_ret;
        case WANLOGINSTATUS:
						sprintf(DBtmp, "WAN%d WANCONNECTION", name[*length-1]);
						kd_doCommand(DBtmp, CMD_PRINT, ASH_DO_NOTHING, DBdata);
						if (atoi(DBdata) != 2)
						{
                if (!exact)
                    goto Repeat;
								return NULL;
						}
            *write_method = write_wanLoginStatus;
            long_ret = 0;
            return (unsigned char *) &long_ret;            
        case WANLOGINUSERNAME:
						sprintf(DBtmp, "WAN%d WANCONNECTION", name[*length-1]);
						kd_doCommand(DBtmp, CMD_PRINT, ASH_DO_NOTHING, DBdata);
						if (atoi(DBdata) != 2)
						{
                if (!exact)
                    goto Repeat;
								return NULL;
						}             
            *write_method = write_wanLoginUserName;
						sprintf(DBtmp, "PPPOE%d USERNAME", name[*length-1]);
						kd_doCommand(DBtmp, CMD_PRINT, ASH_DO_NOTHING, DBdata);
            *var_len = strlen(DBdata);
            return (unsigned char *) DBdata;

        case WANLOGINPASSWORD:
						sprintf(DBtmp, "WAN%d WANCONNECTION", name[*length-1]);
						kd_doCommand(DBtmp, CMD_PRINT, ASH_DO_NOTHING, DBdata);
						if (atoi(DBdata) != 2)
						{
                if (!exact)
                    goto Repeat;
								return NULL;
						}  
            *write_method = write_wanLoginPassword;
						//sprintf(DBtmp, "PPPOE%d PASSWORD", name[*length-1]);
						//kd_doCommand(DBtmp, CMD_PRINT, ASH_DO_NOTHING, DBdata);
						sprintf(DBdata, "******");
            *var_len = 6;
            return (unsigned char *) DBdata;

        case WANWORKINGMODE:
            *write_method = write_wanWorkingMode;
						kd_doCommand("DYNAMIC_ROUTE WMODE", CMD_PRINT, ASH_DO_NOTHING, DBdata);
            if (atoi(DBdata) == 1)
                long_ret = 2;
            else
                long_ret = 1;
            return (unsigned char *) &long_ret;

        case WANCONNECTEDSTATE:
						sprintf(DBtmp, "WAN%d WANCONNECTION", name[*length-1]);
						kd_doCommand(DBtmp, CMD_PRINT, ASH_DO_NOTHING, DBdata);
						if (atoi(DBdata) != 2)
						{
                if (!exact)
                    goto Repeat;
								return NULL;
						}

            *write_method = write_wanConnectedState;
						sprintf(DBtmp, "PPPOE%d KEEPALIVE", name[*length-1]);
						kd_doCommand(DBtmp, CMD_PRINT, ASH_DO_NOTHING, DBdata);
            if (strcmp("YES", DBdata))
                long_ret = 1;
            else
                long_ret = 0;
            return (unsigned char *) &long_ret;

        case WANCONNECTEDIDLETIME:
						sprintf(DBtmp, "WAN%d WANCONNECTION", name[*length-1]);
						kd_doCommand(DBtmp, CMD_PRINT, ASH_DO_NOTHING, DBdata);
						if (atoi(DBdata) != 2)
						{
                if (!exact)
                    goto Repeat;
								return NULL;
						}
                
            *write_method = write_wanConnectedIdleTime;
						sprintf(DBtmp, "PPPOE%d MAXIDLE", name[*length-1]);
						kd_doCommand(DBtmp, CMD_PRINT, ASH_DO_NOTHING, DBdata);
            long_ret = atoi(DBdata);
            return (unsigned char *) &long_ret;

        case WANCONNECTEDREDIALPERIOD:
						sprintf(DBtmp, "WAN%d WANCONNECTION", name[*length-1]);
						kd_doCommand(DBtmp, CMD_PRINT, ASH_DO_NOTHING, DBdata);
						if (atoi(DBdata) != 2)
						{
                if (!exact)
                    goto Repeat;
								return NULL;
						}
                
            *write_method = write_wanConnectedRedialPeriod;
						sprintf(DBtmp, "PPPOE%d REDIALPERIOD", name[*length-1]);
						kd_doCommand(DBtmp, CMD_PRINT, ASH_DO_NOTHING, DBdata);
            long_ret = atoi(DBdata);
            return (unsigned char *) &long_ret;

        case WANDNSAUTONEGOSTATUS:
						sprintf(DBtmp, "WAN%d WANCONNECTION", name[*length-1]);
						kd_doCommand(DBtmp, CMD_PRINT, ASH_DO_NOTHING, DBdata);
						if (atoi(DBdata) != 0)
						{
                if (!exact)
                    goto Repeat;
								return NULL;
						}
            *write_method = write_wanDnsAutoNegoStatus;
						sprintf(DBtmp, "WAN%d USERSPECIALDNS", name[*length-1]);
						kd_doCommand(DBtmp, CMD_PRINT, ASH_DO_NOTHING, DBdata);
            if (!strcmp("NO", DBdata))
                long_ret = 0;
            else
                long_ret = 1;
            return (unsigned char *) &long_ret;

        default:
            ERROR_MSG("");
    }
    return NULL;
}

static int nk_header_wanDnsNetAddressTable(struct variable *vp,
                                                        oid *name,
                                                        size_t *length,
                                                        int exact,
                                                        size_t *var_len,
                                                        WriteMethod **write_method)
{
    //dev_if_t    *dev = NULL;
    oid         newname[MAX_OID_LEN];
    char        dev_name[10];
    int         i, iface, rst;

		char DBtmp[64];
		char DBdata[64];
		int wan_number;

		kd_doCommand("VERSION NUM_WAN", CMD_PRINT, ASH_DO_NOTHING, DBdata);
		wan_number = atoi(DBdata);

    memset(newname, 0, sizeof(newname));
    memcpy((char *)newname, (char *)vp->name, (int)vp->namelen*sizeof(oid));

    /* find "next" wan interface */
    for (iface=1; 1; iface++)
    {
system("echo 444 >> 1111");
				if (iface > wan_number)
					return -1;

				sprintf(DBtmp, "WAN%d WANCONNECTION", iface);
				kd_doCommand(DBtmp, CMD_PRINT, ASH_DO_NOTHING, DBdata);

				if (atoi(DBdata) != 0)
					continue;

        // wan index
        newname[vp->namelen] = (oid)iface;
        // entry index
				#ifdef TABLE_SIZE
				#undef TABLE_SIZE
				#endif
        #define TABLE_SIZE 2
        for (i=1; i <= TABLE_SIZE; i++)
        {
            newname[vp->namelen+1] = (oid)i;
            rst = snmp_oid_compare(name, *length, newname, (int)vp->namelen + 2);
            if ((exact && (rst == 0)) || (!exact && (rst < 0)))
                goto Stop;
        }
    } /* for loop */

Stop:
    memcpy((char *)name, (char *)newname, ((int)vp->namelen + 2)*sizeof(oid));
    *length = vp->namelen + 2;
    *write_method = 0;
    *var_len = sizeof(long);        /* default to 'long' results */
system("echo 333 >> 1111");
    return 0;
} /* nk_header_wanDnsNetAddressTable() */


unsigned char *var_wanDnsNetAddressTable(struct variable *vp,
                                                oid     *name,
                                                size_t  *length,
                                                int     exact,
                                                size_t  *var_len,
                                                WriteMethod **write_method)
{
    static long long_ret;
    static struct in_addr dns_ip;

		char DBtmp[64];
		char DBdata[64];


system("echo 1111 >> 1111");
    if (nk_header_wanDnsNetAddressTable(vp, name, length, exact, var_len, write_method) != 0);
			return NULL;
system("echo 2222 >> 1111");


    switch (vp->magic)
    {
        case WANDNSNETADDRESSINDEX:
            long_ret = name[*length-1];
            return (unsigned char *) &long_ret;
        case WANDNSIFINDEX:
            long_ret = name[*length-1];
            return (unsigned char *) &long_ret;
        case WANDNSNETADDRESS:
            //*write_method = write_wan1DnsNetAddress;
						sprintf(DBtmp, "ISP%d DNS%d", name[*length-2], name[*length-1]);
						kd_doCommand(DBtmp, CMD_PRINT, ASH_DO_NOTHING, DBdata);
						if (inet_addr(DBdata)==0xFFFFFFFF)
							sprintf(DBdata, "0.0.0.0");
						dns_ip.s_addr = inet_addr(DBdata);
            *var_len = 4;
            return (unsigned char *) &dns_ip.s_addr;
        default:
            ERROR_MSG("");
    }
    return NULL;
}

unsigned char *var_wan1DnsNetAddressTable(struct variable *vp,
                                                oid     *name,
                                                size_t  *length,
                                                int     exact,
                                                size_t  *var_len,
                                                WriteMethod **write_method)
{
    static long long_ret;
    static struct in_addr dns_ip;

		char DBtmp[64];
		char DBdata[64];

		#ifdef TABLE_SIZE
		#undef TABLE_SIZE
		#endif
		#define TABLE_SIZE 2
    if (header_simple_table(vp,name,length,exact,var_len,write_method, TABLE_SIZE) == MATCH_FAILED)
        return NULL;

    switch (vp->magic)
    {
        case WAN1DNSNETADDRESSINDEX:
            long_ret = name[*length-1];
            return (unsigned char *) &long_ret;

        case WAN1DNSIFINDEX:
            long_ret = name[*length-1];
            return (unsigned char *) &long_ret;
        case WAN1DNSNETADDRESS:
            *write_method = write_wan1DnsNetAddress;
						
						kd_doCommand("WAN1 USERSPECIALDNS", CMD_PRINT, ASH_DO_NOTHING, DBdata);
            if (!strcmp("NO", DBdata))
                sprintf(DBtmp, "ISP1 DNS%d", name[*length-1]);
            else
                sprintf(DBtmp, "WAN1 DNS%d", name[*length-1]);						
						kd_doCommand(DBtmp, CMD_PRINT, ASH_DO_NOTHING, DBdata);
						if (inet_addr(DBdata)==0xFFFFFFFF)
							sprintf(DBdata, "0.0.0.0");
						dns_ip.s_addr = inet_addr(DBdata);
            *var_len = 4;
            return (unsigned char *) &dns_ip.s_addr;
        default:
            ERROR_MSG("");
    }
    return NULL;
}

unsigned char *var_wan2DnsNetAddressTable(struct variable *vp,
                                                oid     *name,
                                                size_t  *length,
                                                int     exact,
                                                size_t  *var_len,
                                                WriteMethod **write_method)
{
    static long long_ret;
    static struct in_addr dns_ip;

		char DBtmp[64];
		char DBdata[64];
		
		#ifdef TABLE_SIZE
		#undef TABLE_SIZE
		#endif
		#define TABLE_SIZE 2
    if (header_simple_table(vp,name,length,exact,var_len,write_method, TABLE_SIZE) == MATCH_FAILED)
        return NULL;

    switch (vp->magic)
    {
        case WAN2DNSNETADDRESSINDEX:
            long_ret = name[*length-1];
            return (unsigned char *) &long_ret;

        case WAN2DNSIFINDEX:
            long_ret = name[*length-1];
            return (unsigned char *) &long_ret;
        case WAN2DNSNETADDRESS:
            *write_method = write_wan2DnsNetAddress;
						kd_doCommand("WAN2 USERSPECIALDNS", CMD_PRINT, ASH_DO_NOTHING, DBdata);
            if (!strcmp("NO", DBdata))
                sprintf(DBtmp, "ISP2 DNS%d", name[*length-1]);
            else
                sprintf(DBtmp, "WAN2 DNS%d", name[*length-1]);	
						kd_doCommand(DBtmp, CMD_PRINT, ASH_DO_NOTHING, DBdata);
						if (inet_addr(DBdata)==0xFFFFFFFF)
							sprintf(DBdata, "0.0.0.0");
						dns_ip.s_addr = inet_addr(DBdata);
            *var_len = 4;
            return (unsigned char *) &dns_ip.s_addr;
        default:
            ERROR_MSG("");
    }
    return NULL;
}


int write_wanConnectionType(int      action,
                            u_char   *var_val,
                            u_char   var_val_type,
                            size_t   var_val_len,
                            u_char   *statP,
                            oid      *name,
                            size_t   name_len)
{
    static long      long_ret;
		char idx = name[name_len-1];
		char DBdata[64];
		int wanconnectiontype;
    switch (action)
    {
        case RESERVE1:
            if (var_val_type != ASN_INTEGER)
            {
                fprintf(stderr, "write to wanConnectionType not ASN_INTEGER\n");
                return SNMP_ERR_WRONGTYPE;
            }
            if (var_val_len > sizeof(long))
            {
                fprintf(stderr,"write to wanConnectionType: bad length\n");
                return SNMP_ERR_WRONGLENGTH;
            }
            if (*(long *)var_val != 1 && *(long *)var_val != 2 &&
                *(long *)var_val != 3 && *(long *)var_val != 5)
            {
                fprintf(stderr, "write to wanConnectionType: bad value\n");
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
            /* keep this connection type */
            long_ret = *(long *)var_val;
						switch(long_ret)
						{
							case 1:
								wanconnectiontype = 0;
								break;
							case 2:
								wanconnectiontype = 1;
								break;
							case 3:
								wanconnectiontype = 2;
								break;
							case 5:
								wanconnectiontype = 3;
								break;
							default:
								return NULL;
						}
						sprintf(DBdata, "WAN%d WANCONNECTION=%d", idx, wanconnectiontype);
						kd_doCommand(DBdata, CMD_WRITE, ASH_DO_NOTHING, NULL);
						kd_updateFlash(USER_CHANGE_DB);
						kd_doCommand(NULL, (int)NULL, ASH_PAGE_SETUP, NULL);
            break;
    } /* end of switch */
    return SNMP_ERR_NOERROR;
}

int write_wanNetAddress(int      action,
                        u_char   *var_val,
                        u_char   var_val_type,
                        size_t   var_val_len,
                        u_char   *statP,
                        oid      *name,
                        size_t   name_len)
{
    const int IP_LENGTH = 4;
    static struct in_addr ip_addr;
		int wanconnectiontype;
		char DBdata[64];
		char DBtmp[64];
				
    switch (action)
    {
        case RESERVE1:
            if (var_val_type != ASN_IPADDRESS)
            {
                fprintf(stderr, "write to wanNetAddress not ASN_IPADDRESS\n");
                return SNMP_ERR_WRONGTYPE;
            }
            if (var_val_len != IP_LENGTH)
            {
                fprintf(stderr,"write to wanNetAddress: bad length\n");
                return SNMP_ERR_WRONGLENGTH;
            }
            /* check ip value */
            if (var_val[0] > 255 || var_val[1] > 255 || var_val[2] > 255 ||
                var_val[3] > 254)
            {
                fprintf(stderr,"write to wanNetAddress: bad value\n");
                return SNMP_ERR_WRONGVALUE;
            }
            break;

        case RESERVE2:
						sprintf(DBtmp, "WAN%d WANCONNECTION", name[name_len-1]);
						kd_doCommand(DBtmp, CMD_PRINT, ASH_DO_NOTHING, DBdata);
						wanconnectiontype = atoi(DBdata);
						/* if device is pppoe or dynamic, ip cannot be set */
						if (wanconnectiontype == 0 || wanconnectiontype == 2)
						{
                fprintf(stderr,"write to wanNetAddress: bad value\n");
								return SNMP_ERR_GENERR;
						}
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
						memcpy((char *)&ip_addr.s_addr, (char *)var_val, IP_LENGTH);
						sprintf(DBdata, "WAN%d WAN=%s", name[name_len-1], inet_ntoa(ip_addr));
						kd_doCommand(DBdata, CMD_WRITE, ASH_DO_NOTHING, NULL);
						sprintf(DBdata, "PPTP%d LOCALIP=%s", name[name_len-1], inet_ntoa(ip_addr));
						kd_doCommand(DBdata, CMD_WRITE, ASH_DO_NOTHING, NULL);
						kd_updateFlash(USER_CHANGE_DB);						
						kd_doCommand(NULL, (int)NULL, ASH_PAGE_SETUP, NULL);
            break;
    } /* end of switch */
    return SNMP_ERR_NOERROR;
}

int write_wanPhysicalAddr(int    action,
                        u_char   *var_val,
                        u_char   var_val_type,
                        size_t   var_val_len,
                        u_char   *statP,
                        oid      *name,
                        size_t   name_len)
{
		char DBdata[64];
		int mac1, mac2, mac3, mac4, mac5, mac6;
		char mac[20];
		
    switch (action)
    {
        case RESERVE1:
            if (var_val_type != ASN_OCTET_STR)
            {
                fprintf(stderr, "write to wanPhysicalAddr not ASN_OCTET_STR\n");
                return SNMP_ERR_WRONGTYPE;
            }
            if (var_val_len != 17)
            {
                fprintf(stderr,"write to wanPhysicalAddr: bad length\n");
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
						sscanf((char *)var_val, "%02X:%02X:%02X:%02X:%02X:%02X", &mac1, &mac2, &mac3, &mac4, &mac5, &mac6);
						sprintf(mac, "%02X%02X%02X%02X%02X%02X", mac1, mac2, mac3, mac4, mac5, mac6);						
						sprintf(DBdata, "WAN%d MACCLONE=%s", name[name_len-1], mac);
						kd_doCommand(DBdata, CMD_WRITE, ASH_DO_NOTHING, NULL);
						kd_updateFlash(USER_CHANGE_DB);						
						kd_doCommand(NULL, (int)NULL, ASH_PAGE_MACCLONE, NULL);
            break;
    } /* end of switch */
    return SNMP_ERR_NOERROR;
}

int write_wanSubnetMask(int      action,
                        u_char   *var_val,
                        u_char   var_val_type,
                        size_t   var_val_len,
                        u_char   *statP,
                        oid      *name,
                        size_t   name_len)
{
		const int IP_LENGTH = 4;
    static struct in_addr ip_addr;
		int wanconnectiontype;
		char DBdata[64];
		char DBtmp[64];

    switch (action)
    {
        case RESERVE1:
            if (var_val_type != ASN_IPADDRESS)
            {
                fprintf(stderr, "write to wanSubnetMask not ASN_IPADDRESS\n");
                return SNMP_ERR_WRONGTYPE;
            }
            if (var_val_len != IP_LENGTH)
            {
                fprintf(stderr,"write to wanSubnetMask: bad length\n");
                return SNMP_ERR_WRONGLENGTH;
            }
            /* check subnet mask value */
            if (var_val[0] > 255 || var_val[1] > 255 || var_val[2] > 255 ||
                var_val[3] > 254)
            {
                fprintf(stderr,"write to wanSubnetMask: bad value\n");
                return SNMP_ERR_WRONGVALUE;
            }
            break;

        case RESERVE2:
						sprintf(DBtmp, "WAN%d WANCONNECTION", name[name_len-1]);
						kd_doCommand(DBtmp, CMD_PRINT, ASH_DO_NOTHING, DBdata);
						wanconnectiontype = atoi(DBdata);
						/* if device is pppoe or dynamic, mask cannot be set */
						if (wanconnectiontype == 0 || wanconnectiontype == 2)
						{
                fprintf(stderr,"write to wanNetAddress: bad value\n");
								return SNMP_ERR_GENERR;
						}
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
						memcpy((char *)&ip_addr.s_addr, (char *)var_val, IP_LENGTH);
						sprintf(DBdata, "WAN%d WANMASK=%s", name[name_len-1], inet_ntoa(ip_addr));
						kd_doCommand(DBdata, CMD_WRITE, ASH_DO_NOTHING, NULL);
						sprintf(DBdata, "PPTP%d SUBNETMASK=%s", name[name_len-1], inet_ntoa(ip_addr));
						kd_doCommand(DBdata, CMD_WRITE, ASH_DO_NOTHING, NULL);
						kd_updateFlash(USER_CHANGE_DB);						
						kd_doCommand(NULL, (int)NULL, ASH_PAGE_SETUP, NULL);
            break;
    } /* end of switch */
    return SNMP_ERR_NOERROR;
}


int write_wanDefaultGateway(int      action,
                            u_char   *var_val,
                            u_char   var_val_type,
                            size_t   var_val_len,
                            u_char   *statP,
                            oid      *name,
                            size_t   name_len)
{
		const int IP_LENGTH = 4;
    static struct in_addr ip_addr;
		int wanconnectiontype;
		char DBdata[64];
		char DBtmp[64];

    switch (action)
    {
        case RESERVE1:
            if (var_val_type != ASN_IPADDRESS)
            {
                fprintf(stderr, "write to wanDefaultGateway not ASN_IPADDRESS\n");
                return SNMP_ERR_WRONGTYPE;
            }
            if (var_val_len != IP_LENGTH)
            {
                fprintf(stderr,"write to wanDefaultGateway: bad length\n");
                return SNMP_ERR_WRONGLENGTH;
            }
            /* check ip value */
            if (var_val[0] > 255 || var_val[1] > 255 || var_val[2] > 255 ||
                var_val[3] > 254)
            {
                fprintf(stderr,"write to wanDefaultGateway: bad value\n");
                return SNMP_ERR_WRONGVALUE;
            }
            break;
            
        case RESERVE2:
						sprintf(DBtmp, "WAN%d WANCONNECTION", name[name_len-1]);
						kd_doCommand(DBtmp, CMD_PRINT, ASH_DO_NOTHING, DBdata);
						wanconnectiontype = atoi(DBdata);
						/* if device is pppoe or dynamic, gateway cannot be set */
						if (wanconnectiontype == 0 || wanconnectiontype == 2)
						{
                fprintf(stderr,"write to wanNetAddress: bad value\n");
								return SNMP_ERR_GENERR;
						}
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
						memcpy((char *)&ip_addr.s_addr, (char *)var_val, IP_LENGTH);
						sprintf(DBdata, "WAN%d GATEWAY=%s", name[name_len-1], inet_ntoa(ip_addr));
						kd_doCommand(DBdata, CMD_WRITE, ASH_DO_NOTHING, NULL);
						sprintf(DBdata, "PPTP%d REMOTEIP=%s", name[name_len-1], inet_ntoa(ip_addr));
						kd_doCommand(DBdata, CMD_WRITE, ASH_DO_NOTHING, NULL);
						kd_updateFlash(USER_CHANGE_DB);						
						kd_doCommand(NULL, (int)NULL, ASH_PAGE_SETUP, NULL);
            break;
    } /* end of switch */
    return SNMP_ERR_NOERROR;
}

int write_wanDHCPStatus(int      action,
                        u_char   *var_val,
                        u_char   var_val_type,
                        size_t   var_val_len,
                        u_char   *statP,
                        oid      *name,
                        size_t   name_len)
{
		int wanconnectiontype;
		char DBdata[64];
		char DBtmp[64];
		
    switch (action)
    {
        case RESERVE1:
            if (var_val_type != ASN_INTEGER)
            {
                fprintf(stderr, "write to wanDHCPStatus not ASN_INTEGER\n");
                return SNMP_ERR_WRONGTYPE;
            }
            if (var_val_len > sizeof(long))
            {
                fprintf(stderr,"write to wanDHCPStatus: bad length\n");
                return SNMP_ERR_WRONGLENGTH;
            }
            if (*(long *)var_val != 0 && *(long *)var_val != 1)
            {
                fprintf(stderr, "write to wanDHCPStatus: bad value\n");
                return SNMP_ERR_WRONGVALUE;
            }
            break;

        case RESERVE2:
						sprintf(DBtmp, "WAN%d WANCONNECTION", name[name_len-1]);
						kd_doCommand(DBtmp, CMD_PRINT, ASH_DO_NOTHING, DBdata);
						wanconnectiontype = atoi(DBdata);
            if (wanconnectiontype!=0)
                return SNMP_ERR_GENERR;
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
						sprintf(DBtmp,"WAN%d",name[name_len-1]);
						if (*(long *)var_val == 0)
								kd_doCommand(DBtmp, (int)NULL, ASH_BUTTON_DHCP_RELEASE, NULL);
            else if (*(long *)var_val == 1)
                kd_doCommand(DBtmp, (int)NULL, ASH_BUTTON_DHCP_RENEW, NULL);
            break;
    } /* end of switch */
    return SNMP_ERR_NOERROR;
}

int write_wanLoginStatus(int      action,
                        u_char   *var_val,
                        u_char   var_val_type,
                        size_t   var_val_len,
                        u_char   *statP,
                        oid      *name,
                        size_t   name_len)
{
		int wanconnectiontype;
		char DBdata[64];
		char DBtmp[64];
		
    switch (action)
    {
        case RESERVE1:
            if (var_val_type != ASN_INTEGER)
            {
                fprintf(stderr, "write to wanDHCPStatus not ASN_INTEGER\n");
                return SNMP_ERR_WRONGTYPE;
            }
            if (var_val_len > sizeof(long))
            {
                fprintf(stderr,"write to wanDHCPStatus: bad length\n");
                return SNMP_ERR_WRONGLENGTH;
            }
            if (*(long *)var_val != 0 && *(long *)var_val != 1)
            {
                fprintf(stderr, "write to wanDHCPStatus: bad value\n");
                return SNMP_ERR_WRONGVALUE;
            }
            break;

        case RESERVE2:
						sprintf(DBtmp, "WAN%d WANCONNECTION", name[name_len-1]);
						kd_doCommand(DBtmp, CMD_PRINT, ASH_DO_NOTHING, DBdata);
						wanconnectiontype = atoi(DBdata);
            if (wanconnectiontype!=2)
                return SNMP_ERR_GENERR;
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
						sprintf(DBtmp,"WAN%d",name[name_len-1]);
						if (*(long *)var_val == 0)
								kd_doCommand(DBtmp, (int)NULL, ASH_BUTTON_PPPOE_DISCONNECT, NULL);
            else if (*(long *)var_val == 1)
                kd_doCommand(DBtmp, (int)NULL, ASH_BUTTON_PPPOE_CONNECT, NULL);
            break;
    } /* end of switch */
    return SNMP_ERR_NOERROR;
}

int write_wanLoginUserName(int   action,
                        u_char   *var_val,
                        u_char   var_val_type,
                        size_t   var_val_len,
                        u_char   *statP,
                        oid      *name,
                        size_t   name_len)
{
    #define PPP_USERNAME_MAX_LEN 65
    unsigned char string[PPP_USERNAME_MAX_LEN];
		int wanconnectiontype;
		char DBdata[90];
		char DBtmp[90];

    switch (action)
    {
        case RESERVE1:
            if (var_val_type != ASN_OCTET_STR)
            {
                fprintf(stderr, "write to wanLoginUserName not ASN_OCTET_STR\n");
                return SNMP_ERR_WRONGTYPE;
            }
            if (var_val_len >= PPP_USERNAME_MAX_LEN)
            {
                fprintf(stderr,"write to wanLoginUserName: bad length\n");
                return SNMP_ERR_WRONGLENGTH;
            }
            break;

        case RESERVE2:
						sprintf(DBtmp, "WAN%d WANCONNECTION", name[name_len-1]);
						kd_doCommand(DBtmp, CMD_PRINT, ASH_DO_NOTHING, DBdata);
						wanconnectiontype = atoi(DBdata);
            if (wanconnectiontype!=2)
                return SNMP_ERR_GENERR;
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
						sprintf(string, "%s",  (char *)var_val);
						string[var_val_len] = '\0';
						sprintf(DBdata, "PPPOE%d USERNAME=%s", name[name_len-1],  string);
						kd_doCommand(DBdata, CMD_WRITE, ASH_DO_NOTHING, NULL);
						kd_updateFlash(USER_CHANGE_DB);						
						kd_doCommand(NULL, (int)NULL, ASH_PAGE_SETUP, NULL);								
            break;
    } /* end of switch */
    return SNMP_ERR_NOERROR;
}

int write_wanLoginPassword(int   action,
                        u_char   *var_val,
                        u_char   var_val_type,
                        size_t   var_val_len,
                        u_char   *statP,
                        oid      *name,
                        size_t   name_len)
{
    #define PPP_PASSWD_MAX_LEN 65
    unsigned char string[PPP_PASSWD_MAX_LEN];
		int wanconnectiontype;
		char DBdata[90];
		char DBtmp[90];

    switch (action)
    {
        case RESERVE1:
            if (var_val_type != ASN_OCTET_STR)
            {
                fprintf(stderr, "write to wanLoginPassword not ASN_OCTET_STR\n");
                return SNMP_ERR_WRONGTYPE;
            }
            if (var_val_len >= PPP_PASSWD_MAX_LEN)
            {
                fprintf(stderr,"write to wanLoginPassword: bad length\n");
                return SNMP_ERR_WRONGLENGTH;
            }
            break;

        case RESERVE2:
						sprintf(DBtmp, "WAN%d WANCONNECTION", name[name_len-1]);
						kd_doCommand(DBtmp, CMD_PRINT, ASH_DO_NOTHING, DBdata);
						wanconnectiontype = atoi(DBdata);
            if (wanconnectiontype!=2)
                return SNMP_ERR_GENERR;
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
						sprintf(string, "%s",  (char *)var_val);
						string[var_val_len] = '\0';
						sprintf(DBdata, "PPPOE%d PASSWORD=%s", name[name_len-1], string);
						kd_doCommand(DBdata, CMD_WRITE, ASH_DO_NOTHING, NULL);
						kd_updateFlash(USER_CHANGE_DB);						
						kd_doCommand(NULL, (int)NULL, ASH_PAGE_SETUP, NULL);	
            break;
    } /* end of switch */
    return SNMP_ERR_NOERROR;
}

int
write_wanWorkingMode(int      action,
            u_char   *var_val,
            u_char   var_val_type,
            size_t   var_val_len,
            u_char   *statP,
            oid      *name,
            size_t   name_len)
{
#if 0
  static long *long_ret;
  int size;


  switch ( action ) {
        case RESERVE1:
          if (var_val_type != ASN_INTEGER){
              fprintf(stderr, "write to wanWorkingMode not ASN_INTEGER\n");
              return SNMP_ERR_WRONGTYPE;
          }
          if (var_val_len > sizeof(long_ret)){
              fprintf(stderr,"write to wanWorkingMode: bad length\n");
              return SNMP_ERR_WRONGLENGTH;
          }
          break;


        case RESERVE2:
          size = var_val_len;
          long_ret = (long *) var_val;


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
#endif
  return SNMP_ERR_NOERROR;
}

int write_wanConnectedState(int  action,
                    u_char   *var_val,
                    u_char   var_val_type,
                    size_t   var_val_len,
                    u_char   *statP,
                    oid      *name,
                    size_t   name_len)
{
		int wanconnectiontype;
		char DBdata[90];
		char DBtmp[90];

    switch (action)
    {
        case RESERVE1:
            if (var_val_type != ASN_INTEGER)
            {
                fprintf(stderr, "write to wanConnectedState not ASN_INTEGER\n");
                return SNMP_ERR_WRONGTYPE;
            }
            if (var_val_len > sizeof(long))
            {
                fprintf(stderr,"write to wanConnectedState: bad length\n");
                return SNMP_ERR_WRONGLENGTH;
            }
            if (*(long *)var_val != 0 && *(long *)var_val != 1)
            {
                fprintf(stderr, "write to wanConnectedState: bad value\n");
                return SNMP_ERR_WRONGVALUE;
            }
            break;

        case RESERVE2:
						sprintf(DBtmp, "WAN%d WANCONNECTION", name[name_len-1]);
						kd_doCommand(DBtmp, CMD_PRINT, ASH_DO_NOTHING, DBdata);
						wanconnectiontype = atoi(DBdata);
            if (wanconnectiontype!=2)
                return SNMP_ERR_GENERR;
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
						if(*(long *)var_val==0)
							sprintf(DBdata, "PPPOE%d KEEPALIVE=YES", name[name_len-1]);
						else
							sprintf(DBdata, "PPPOE%d KEEPALIVE=NO", name[name_len-1]);
						kd_doCommand(DBdata, CMD_WRITE, ASH_DO_NOTHING, NULL);
						kd_updateFlash(USER_CHANGE_DB);						
						kd_doCommand(NULL, (int)NULL, ASH_PAGE_SETUP, NULL);	
            break;
    } /* end of switch */
    return SNMP_ERR_NOERROR;
}

int write_wanConnectedIdleTime(int   action,
                            u_char   *var_val,
                            u_char   var_val_type,
                            size_t   var_val_len,
                            u_char   *statP,
                            oid      *name,
                            size_t   name_len)
{
		int wanconnectiontype;
		char DBdata[90];
		char DBtmp[90];

    switch (action)
    {
        case RESERVE1:
            if (var_val_type != ASN_INTEGER)
            {
                fprintf(stderr, "write to wanConnectedIdleTime not ASN_INTEGER\n");
                return SNMP_ERR_WRONGTYPE;
            }
            if (var_val_len > sizeof(long))
            {
                fprintf(stderr,"write to wanConnectedIdleTime: bad length\n");
                return SNMP_ERR_WRONGLENGTH;
            }
            break;
            
        case RESERVE2:
						sprintf(DBtmp, "WAN%d WANCONNECTION", name[name_len-1]);
						kd_doCommand(DBtmp, CMD_PRINT, ASH_DO_NOTHING, DBdata);
						wanconnectiontype = atoi(DBdata);
            if (wanconnectiontype!=2)
                return SNMP_ERR_GENERR;
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
						sprintf(DBdata, "PPPOE%d MAXIDLE=%d", name[name_len-1], *(long *)var_val);
						kd_doCommand(DBdata, CMD_WRITE, ASH_DO_NOTHING, NULL);
						kd_updateFlash(USER_CHANGE_DB);						
						kd_doCommand(NULL, (int)NULL, ASH_PAGE_SETUP, NULL);	
            break;
    } /* end of switch */
    return SNMP_ERR_NOERROR;
}


int write_wanConnectedRedialPeriod(int   action,
                                u_char   *var_val,
                                u_char   var_val_type,
                                size_t   var_val_len,
                                u_char   *statP,
                                oid      *name,
                                size_t   name_len)
{
		int wanconnectiontype;
		char DBdata[90];
		char DBtmp[90];

    switch (action)
    {
        case RESERVE1:
            if (var_val_type != ASN_INTEGER)
            {
                fprintf(stderr, "write to wanConnectedRedialPeriod not ASN_INTEGER\n");
                return SNMP_ERR_WRONGTYPE;
            }
            if (var_val_len > sizeof(long))
            {
                fprintf(stderr,"write to wanConnectedRedialPeriod: bad length\n");
                return SNMP_ERR_WRONGLENGTH;
            }
            break;

        case RESERVE2:
						sprintf(DBtmp, "WAN%d WANCONNECTION", name[name_len-1]);
						kd_doCommand(DBtmp, CMD_PRINT, ASH_DO_NOTHING, DBdata);
						wanconnectiontype = atoi(DBdata);
            if (wanconnectiontype!=2)
                return SNMP_ERR_GENERR;
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
						sprintf(DBdata, "PPPOE%d REDIALPERIOD=%d", name[name_len-1], *(long *)var_val);
						kd_doCommand(DBdata, CMD_WRITE, ASH_DO_NOTHING, NULL);
						kd_updateFlash(USER_CHANGE_DB);						
						kd_doCommand(NULL, (int)NULL, ASH_PAGE_SETUP, NULL);
            break;
    } /* end of switch */
    return SNMP_ERR_NOERROR;
}

int write_wanDnsAutoNegoStatus(int   action,
                            u_char   *var_val,
                            u_char   var_val_type,
                            size_t   var_val_len,
                            u_char   *statP,
                            oid      *name,
                            size_t   name_len)
{
    long        long_ret;
		int wanconnectiontype;
		char DBdata[64];
		char DBtmp[64];
		
    switch (action)
    {
        case RESERVE1:
            if (var_val_type != ASN_INTEGER)
            {
                fprintf(stderr, "write to wanDnsAutoNegoStatus not ASN_INTEGER\n");
                return SNMP_ERR_WRONGTYPE;
            }
            if (var_val_len > sizeof(long))
            {
                fprintf(stderr,"write to wanDnsAutoNegoStatus: bad length\n");
                return SNMP_ERR_WRONGLENGTH;
            }
            if (*(long *)var_val != 0 && *(long *)var_val != 1)
            {
                fprintf(stderr, "write to wanDnsAutoNegoStatus: bad value\n");
                return SNMP_ERR_WRONGVALUE;
            }
            break;
            
        case RESERVE2:
						sprintf(DBtmp, "WAN%d WANCONNECTION", name[name_len-1]);
						kd_doCommand(DBtmp, CMD_PRINT, ASH_DO_NOTHING, DBdata);
						wanconnectiontype = atoi(DBdata);
            if (wanconnectiontype!=0)
                return SNMP_ERR_GENERR;
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
						long_ret =	*(long *)var_val;
						if(long_ret==0)
							sprintf(DBdata, "WAN%d USERSPECIALDNS=NO", name[name_len-1]);
						else
							sprintf(DBdata, "WAN%d USERSPECIALDNS=YES", name[name_len-1]);
						kd_doCommand(DBdata, CMD_WRITE, ASH_DO_NOTHING, NULL);
						kd_updateFlash(USER_CHANGE_DB);						
						kd_doCommand(NULL, (int)NULL, ASH_PAGE_SETUP, NULL);	
            break;
    } /* end of switch */
    return SNMP_ERR_NOERROR;
}


int write_wan1DnsNetAddress(int   action,
                        u_char   *var_val,
                        u_char   var_val_type,
                        size_t   var_val_len,
                        u_char   *statP,
                        oid      *name,
                        size_t   name_len)
{
    const int IP_LENGTH = 4;
    struct in_addr dns_ip;
		int wanconnectiontype;
		char DBdata[64];
		char DBtmp[64];
		    
    switch (action)
    {
        case RESERVE1:
            if (var_val_type != ASN_IPADDRESS)
            {
                fprintf(stderr, "write to wanDnsNetAddress not ASN_IPADDRESS\n");
                return SNMP_ERR_WRONGTYPE;
            }
            if (var_val_len != IP_LENGTH)
            {
                fprintf(stderr,"write to wanDnsNetAddress: bad length\n");
                return SNMP_ERR_WRONGLENGTH;
            }
            /* check ip value */
            if (var_val[0] > 255 || var_val[1] > 255 || var_val[2] > 255 ||
                var_val[3] > 254)
            {
                fprintf(stderr,"write to wanDnsNetAddress: bad value\n");
                return SNMP_ERR_WRONGVALUE;
            }
            break;

        case RESERVE2:
						kd_doCommand("WAN1 WANCONNECTION", CMD_PRINT, ASH_DO_NOTHING, DBdata);
						wanconnectiontype = atoi(DBdata);
            if (wanconnectiontype!=0)
                return SNMP_ERR_GENERR;
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

						memcpy((char *)&dns_ip.s_addr, (char *)var_val, IP_LENGTH);
						sprintf(DBdata, "WAN1 DNS%d=%s", name[name_len-1], inet_ntoa(dns_ip));
						kd_doCommand(DBdata, CMD_WRITE, ASH_DO_NOTHING, NULL);
						kd_updateFlash(USER_CHANGE_DB);						
						kd_doCommand(NULL, (int)NULL, ASH_PAGE_SETUP, NULL);
            break;
    } /* end of switch */
    return SNMP_ERR_NOERROR;
}

int write_wan2DnsNetAddress(int   action,
                        u_char   *var_val,
                        u_char   var_val_type,
                        size_t   var_val_len,
                        u_char   *statP,
                        oid      *name,
                        size_t   name_len)
{
    const int IP_LENGTH = 4;
    struct in_addr dns_ip;
		int wanconnectiontype;
		char DBdata[64];
		char DBtmp[64];
		    
    switch (action)
    {
        case RESERVE1:
            if (var_val_type != ASN_IPADDRESS)
            {
                fprintf(stderr, "write to wanDnsNetAddress not ASN_IPADDRESS\n");
                return SNMP_ERR_WRONGTYPE;
            }
            if (var_val_len != IP_LENGTH)
            {
                fprintf(stderr,"write to wanDnsNetAddress: bad length\n");
                return SNMP_ERR_WRONGLENGTH;
            }
            /* check ip value */
            if (var_val[0] > 255 || var_val[1] > 255 || var_val[2] > 255 ||
                var_val[3] > 254)
            {
                fprintf(stderr,"write to wanDnsNetAddress: bad value\n");
                return SNMP_ERR_WRONGVALUE;
            }
            break;

        case RESERVE2:
						kd_doCommand("WAN2 WANCONNECTION", CMD_PRINT, ASH_DO_NOTHING, DBdata);
						wanconnectiontype = atoi(DBdata);
            if (wanconnectiontype!=0)
                return SNMP_ERR_GENERR;
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

						memcpy((char *)&dns_ip.s_addr, (char *)var_val, IP_LENGTH);
						sprintf(DBdata, "WAN2 DNS%d=%s", name[name_len-1], inet_ntoa(dns_ip));
						kd_doCommand(DBdata, CMD_WRITE, ASH_DO_NOTHING, NULL);
						kd_updateFlash(USER_CHANGE_DB);						
						kd_doCommand(NULL, (int)NULL, ASH_PAGE_SETUP, NULL);
            break;
    } /* end of switch */
    return SNMP_ERR_NOERROR;
}

