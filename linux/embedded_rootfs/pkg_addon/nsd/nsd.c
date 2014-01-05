
#include <stdio.h>
#include <netdb.h>
#include <nkutil.h>

//#define nsd_debug
#ifdef nsd_debug
  #define trenchen_printf(x...) printf(x)
#else
  #define trenchen_printf(x...)
#endif

#define NSD_SUCCESS 0
#define NSD_FAIL -1

/*purpose     : 0013315 author : Ben date : 2010-10-12*/
/*description : reduse sleep time on retry to 1 on RV0XX*/
#ifdef CONFIG_MODEL_RV0XX
#define RETRYCHIP 1
#else
#define RETRYCHIP 5
#endif

enum pingstate {
	CHECKIO,	//2007/5/24 trenchen : copy from SR1
	PINGDEFAULT,
	PINGISPHOST,
	PINGREMOTEHOST,
	DNSRESOLVING  //2006/1/18 trenchen : support DNS resolving
};

#define PINGSTART CHECKIO
//#define PINGSTART PINGDEFAULT
#define PINGLAST DNSRESOLVING+1

extern int ping(const char *host,const char *SourceIp);

//max add for vpn backup
//ipsec_doi.c ,nsd.c and nk_ipsec.h set NKIPSECD_IPC
#define NKIPSECD_IPC "/var/nk_ipsecd.fifo" 
//both ipsec_doi.h and nsd.c set IPSEC_IPC_NSD_LINKDOWN
#define IPSEC_IPC_NSD_LINKDOWN 16
//both ipsec_doi.h and nsd.c.c set IPSEC_IPC_NSD_LINKUP
#define IPSEC_IPC_NSD_LINKUP	17
#define max_printf(x...)   kd_Log(x)
//<<
/*purpose     : 0013074 author : trenchen date : 2010-07-29 */
/*description : get value formant changed, fllow new rule   */
#define NK_IPSEC_FILEN 120

int main(int argc, char *argv[])
{
//max add for vpn backup
    /*purpose     : 0013074 author : trenchen date : 2010-07-29 */
    /*description : get value formant changed, fllow new rule   */
    /*char cmdBuf[30];*/
    char cmdBuf[NK_IPSEC_FILEN];
//<<
	int inf = atoi(argv[1]);
	char PathName[20];
	char SourceIp[20];
	char getcount[5];
	char gettimeout[5];
	char getdefaultip[20];
	char getisphostip[20];
	char getremotehostip[20];
	char getdnshost[60];//2006/1/18 trenchen : support DNS resolving
	char getwantype[10];
	char getaction[5];
	char defaultenabled[5];
	char ispenabled[5];
	char remoteenabled[5];
	char dnsenabled[5];//2006/1/18 trenchen : support DNS resolving
	char nsdstate[5];
	int retrycount=5;
	int retrytimeout=30;
	int nsdok=1;
	int count;
	int action;
	int res=NSD_FAIL;
#ifdef CONFIG_NK_SUPPORT_USB_3G	
	char INTERFACE[10];
	char iUSBPort[4];
	int wan_num = 2;
	char tmp[140];
#endif
	//2007/5/24 trenchen : add check io, copy from SR1
	char checkioenabled[5];
	char busy_condition[5];
	char busy_rate[5];
	char downstream[15];
	char upstream[15];
	char dev_interface[50];
	unsigned int upstream_bw , downstream_bw;

	struct hostent *dnsres;
	enum pingstate state = PINGSTART;

#ifdef CONFIG_NK_LINE_DROP
	char LineDropped_Status[8];

	sprintf(PathName,"LINE_DROPPED STATUS");
	kd_doCommand(PathName, CMD_PRINT, ASH_DO_NOTHING, LineDropped_Status);
	if (LineDropped_Status[inf-1]=='1')
	{
		trenchen_printf(LERR,"Line_dropped Schedule is running on [%s],so cant start nsd on [%s]\n",ifc,ifc);
		return;
	}
#endif

	trenchen_printf("start nsd inf[%d]\n",inf);

#ifdef CONFIG_NK_SUPPORT_USB_3G	
	kd_doCommand( "VERSION NUM_WAN", CMD_PRINT, ASH_DO_NOTHING, tmp);
	wan_num = atoi(tmp);
#endif

	sprintf(PathName,"ISP%d WAN", inf);
	kd_doCommand( PathName, CMD_PRINT, ASH_DO_NOTHING, SourceIp);
	trenchen_printf("nsd source[%s]\n",SourceIp);
	//if source ip ==0 return;
	
	sprintf(PathName,"NSD%d RETRYCOUNT",inf);
	kd_doCommand( PathName, CMD_PRINT, ASH_DO_NOTHING, getcount);
	retrycount = atoi(getcount);
	trenchen_printf("nsd count[%d]\n", retrycount);

	sprintf(PathName,"NSD%d RETRYPERIOD",inf);
	kd_doCommand( PathName, CMD_PRINT, ASH_DO_NOTHING, gettimeout);
	retrytimeout = atoi(gettimeout);
	trenchen_printf("nsd period[%d]\n", retrytimeout);

	sprintf(PathName,"NSD%d NSDOK",inf);
	kd_doCommand( PathName, CMD_PRINT, ASH_DO_NOTHING, nsdstate);
	nsdok = atoi(nsdstate);
	trenchen_printf("nsd state[%d]\n", nsdok);
	
	sprintf(PathName,"NSD%d EN_DEFAULT",inf);
	kd_doCommand( PathName, CMD_PRINT, ASH_DO_NOTHING, defaultenabled);
#ifdef CONFIG_NK_SUPPORT_USB_3G
	if (inf > wan_num)
	{
		memset(defaultenabled, 0, sizeof(defaultenabled));
		strcpy(defaultenabled, "NO");
	}
#endif
	sprintf(PathName,"ISP%d INTERFACE",inf);
	kd_doCommand( PathName, CMD_PRINT, ASH_DO_NOTHING, getwantype);
	if( !strncmp(getwantype, "ppp", 3) ) {
		sprintf(PathName,"ISP%d COD_GATEWAY",inf);
	} else {
		sprintf(PathName,"ISP%d GATEWAY",inf);
	}
	kd_doCommand( PathName, CMD_PRINT, ASH_DO_NOTHING, getdefaultip);
	trenchen_printf("ping default ip[%s]\n",getdefaultip);

	sprintf(PathName,"NSD%d EN_ISPHOST",inf);
	kd_doCommand( PathName, CMD_PRINT, ASH_DO_NOTHING, ispenabled);
	sprintf(PathName,"NSD%d ISPHOST",inf);
	kd_doCommand( PathName, CMD_PRINT, ASH_DO_NOTHING, getisphostip);
	trenchen_printf("ping ISP HOST ip[%s]\n",getisphostip);
	
	sprintf(PathName,"NSD%d EN_REMOTEHOST",inf);
	kd_doCommand( PathName, CMD_PRINT, ASH_DO_NOTHING, remoteenabled);
	sprintf(PathName,"NSD%d REMOTEHOST",inf);
	kd_doCommand( PathName, CMD_PRINT, ASH_DO_NOTHING, getremotehostip);
	trenchen_printf("ping REMOTE HOST ip[%s]\n",getremotehostip);

	//2006/1/18 trenchen : support DNS resolving
	sprintf(PathName,"NSD%d EN_DNSRESOLV",inf);
	kd_doCommand( PathName, CMD_PRINT, ASH_DO_NOTHING, dnsenabled);
	sprintf(PathName,"NSD%d DNSHOST",inf);
	kd_doCommand( PathName, CMD_PRINT, ASH_DO_NOTHING, getdnshost);
	trenchen_printf("dns resolving HOST name[%s]\n",getdnshost);


	sprintf(PathName,"NSD%d ACTION",inf);
	kd_doCommand( PathName, CMD_PRINT, ASH_DO_NOTHING, getaction);
	action = atoi(getaction);
	trenchen_printf("NSD ACTION[%d]\n",action);

	//2007/5/24 trenchen : add check io
	sprintf(PathName,"NSD%d EN_HEAVYLOAD",inf);
	kd_doCommand( PathName, CMD_PRINT, ASH_DO_NOTHING, checkioenabled);
#ifdef CONFIG_NK_SUPPORT_USB_3G
	if (inf > wan_num)
	{
		memset(checkioenabled, 0, sizeof(checkioenabled));
		strcpy(checkioenabled, "NO");
	}
#endif
	sprintf(PathName,"NSD%d CONDITION",inf);
	kd_doCommand( PathName, CMD_PRINT, ASH_DO_NOTHING, busy_condition);
	sprintf(PathName,"NSD%d HEAVY_LOAD",inf);
	kd_doCommand( PathName, CMD_PRINT, ASH_DO_NOTHING, busy_rate);

	//2007/06/04 trenchen : move codes from check_io.c to reduce the problem process can't be del
	sprintf(PathName,"WAN%d DOWNSTREAMBW",inf);
	kd_doCommand(PathName, CMD_PRINT, ASH_DO_NOTHING, downstream);
	downstream_bw = atoi(downstream);
	if(!downstream_bw)
		downstream_bw = 1;
	
	sprintf(PathName,"WAN%d UPSTREAMBW",inf);
	kd_doCommand(PathName, CMD_PRINT, ASH_DO_NOTHING, upstream);
	upstream_bw = atoi(upstream);
	if(!upstream_bw)
		upstream_bw = 1;

	sprintf(PathName,"WAN%d WANINTERFACE",inf);
	kd_doCommand(PathName, CMD_PRINT, ASH_DO_NOTHING, dev_interface);

#ifdef CONFIG_NK_SUPPORT_USB_3G
	if (inf > wan_num)
	{
		if (!strcmp(ispenabled, "NO") && !strcmp(remoteenabled, "NO") && !strcmp(dnsenabled, "NO"))
		{
			sprintf(PathName,"NSD%d NSDOK=0",inf);
			kd_doCommand( PathName, CMD_WRITE, ASH_DO_NOTHING, (char *)NULL);
			return 0;
		}
	}
#endif

	while(1) {
		switch(state) {

			//2007/5/24 trenchen : add check io copy from SR1
			case CHECKIO:
					trenchen_printf("nsd CHECK IO start\n");
				if( strncmp(checkioenabled, "YES", 3)  ) {
					state++;
					break;
				}
				if(is_interface_busy(inf,busy_condition,busy_rate,upstream_bw,downstream_bw,dev_interface))
				{
					trenchen_printf("nsd CHECK IO success\n");
					res = NSD_SUCCESS;
				}
				else
				{
					trenchen_printf("nsd CHECK IO fail\n");
					state++;
				}
				break;

			case PINGDEFAULT:
				/*ping default gateway*/
				if( strncmp(defaultenabled, "YES", 3) || !(*getdefaultip) ) {
					state++;
					break;
				}

				for(count=0; count < retrycount; count++) {
					if( (res=ping(getdefaultip,SourceIp)) == NSD_SUCCESS ) {
						trenchen_printf("nsd default gateway success\n");
						break;
					} else //2005/12/22 trenchen : delay the retry ping 
						sleep(RETRYCHIP);
				}
				if( count == retrycount ){
					state++;
					trenchen_printf("nsd default gateway fail\n");
				}

				break;

			case PINGISPHOST:
				/*ping ISP HOST*/

				if( strncmp(ispenabled, "YES", 3) || !(*getisphostip) ) {
					state++;
					break;
				}

				for(count=0; count < retrycount; count++) {
					if( (res=ping(getisphostip,SourceIp)) == NSD_SUCCESS ) {
						trenchen_printf("nsd ISP HOST success\n");
						break;
					} else
						sleep(RETRYCHIP);
				}
				if( count == retrycount ){
					state++;
					trenchen_printf("nsd ISP HOST fail\n");
				}

				break;
				
			case PINGREMOTEHOST:
				/*ping ISP HOST*/

				if( strncmp(remoteenabled, "YES", 3) || !(*getremotehostip) ) {
					state++;
					break;
				}

				
				for(count=0; count < retrycount; count++) {
					if( (res=ping(getremotehostip,SourceIp)) == NSD_SUCCESS ) {
						trenchen_printf("nsd REMOTE HOST success\n");
						break;
					} else
						sleep(RETRYCHIP);
				}
				if( count == retrycount ){
					state++;
					trenchen_printf("nsd REMOTE HOST fail\n");
				}

				break;

			//2006/1/18 trenchen : support DNS resolving
			case DNSRESOLVING:
				if( strncmp(dnsenabled, "YES", 3) || !(*getdnshost) ) {
					state++;
					break;
				}
				
				for(count=0; count < retrycount; count++) {
					if( dnsres=gethostbyname_nsd(getdnshost,SourceIp) ) {
						res=NSD_SUCCESS;
						trenchen_printf("nsd DNS RESOLVING success\n");
						break;
					} else
						sleep(RETRYCHIP);
				}
				if( count == retrycount ){
					state++;
					trenchen_printf("nsd DNS RESOLVING fail\n");
				}
				
				break;
		}
		
		/*deal the detection result*/
		if( res == NSD_SUCCESS ) {
			/*set ISP NSD=OK AND something else*/
			if( nsdok == 0 ){
				nsdok=1;
				//weboot.c have done
				//sprintf(PathName,"ISP%d NSDOK=1",inf);
				//kd_doCommand( PathName, CMD_WRITE, ASH_DO_NOTHING, (char *)NULL);
				sprintf(PathName,"NSD%d NSDOK=1",inf);
				kd_doCommand( PathName, CMD_WRITE, ASH_DO_NOTHING, (char *)NULL);

				NK_LOG_SYS(LOG_WARNING, " NSD SUCCESS WAN[%d]", inf);
#ifdef CONFIG_NK_SUPPORT_USB_3G
			if (inf > wan_num)
			{
				NK_LOG_SYS(LOG_WARNING, "USB%d NSD SUCCESS", inf-wan_num);
			}
			else
			{
#endif
				if( action ) {
					sprintf(PathName,"WAN%d",inf);
					kd_doCommand( PathName, NULL, ASH_NSD_OK, (char *)NULL);
//max add for vpn backup
				/*purpose     : 0013074 author : trenchen date : 2010-07-29 */
				/*description : get value formant changed, fllow new rule   */
 				/*sprintf(cmdBuf,"echo \"CMD=%d&WANNUM=%d\" > %s",IPSEC_IPC_NSD_LINKUP,inf,NKIPSECD_IPC);*/
				sprintf(cmdBuf,"echo \"CMD=\\\"%d\\\"&WANNUM=\\\"%d\\\"&\" > %s",IPSEC_IPC_NSD_LINKUP,inf,NKIPSECD_IPC);
				system(cmdBuf);
//<				
				} else {
					//NK_LOG_SYS(LOG_WARNING, " NSD SUCCESS WAN[%d]", inf);
					trenchen_printf("nsd ok [%d]\n",inf);
				}
#ifdef CONFIG_NK_SUPPORT_USB_3G
				// stop 3G Backup
				kd_doCommand("0", NULL, ASK_CHECK_3G_BACKUP, (char *)NULL);
			}
#endif
			}
			state = PINGSTART;
			res = NSD_FAIL;
			sleep(retrytimeout);
		} else if (state == PINGLAST) {
			/*set ISP NSD=FAIL and something else*/
			if( nsdok == 1)
			{
#ifdef NK_CONFIG_IPV6
				char cmdBuf[128],tunnel_interface[10];
#endif
				
				nsdok=0;
				//weboot.c have done
				//sprintf(PathName,"ISP%d NSDOK=0",inf);
				//kd_doCommand( PathName, CMD_WRITE, ASH_DO_NOTHING, (char *)NULL);
				sprintf(PathName,"NSD%d NSDOK=0",inf);
				kd_doCommand( PathName, CMD_WRITE, ASH_DO_NOTHING, (char *)NULL);
				
#ifdef NK_CONFIG_IPV6
				sprintf(cmdBuf,"TUNNEL_6TO4 INTERFACE");
				kd_doCommand(cmdBuf, CMD_PRINT, ASH_DO_NOTHING, tunnel_interface);
				if(atoi(tunnel_interface) == inf )
				{
					kd_doCommand(NULL, NULL, ASH_PAGE_6TO4, (char *)NULL);
					kd_doCommand(NULL, NULL, ASH_PAGE_RADVD, (char *)NULL);
				}
#endif

				NK_LOG_SYS(LOG_WARNING, " NSD FAIL WAN[%d]", inf);
#ifdef CONFIG_NK_SUPPORT_USB_3G
			if (inf > wan_num)
			{
				if( action ) {
					sprintf(PathName, "USB%d INTERFACE", inf-wan_num);			
					kd_doCommand(PathName, CMD_PRINT, ASH_DO_NOTHING, INTERFACE);
					sprintf(tmp, "ps  | grep \"unit %s\" > /tmp/usbppp%d.tmp3", INTERFACE+3, inf-wan_num);
					system(tmp);
					sprintf(tmp, "tmp=`cat /tmp/usbppp%d.tmp3 | grep pppd | awk '{print $1}'` ; kill $tmp ; rm /tmp/usbppp%d.tmp3", inf-wan_num, inf-wan_num);
					system(tmp);
					sprintf(iUSBPort, "%d", inf-wan_num);
					kd_doCommand(iUSBPort, NULL, ASH_3G_PPP_ON, (char *)NULL);
				}
				NK_LOG_SYS(LOG_WARNING, "USB%d NSD FAIL", inf-wan_num);
			}
			else
			{
#endif
				if( action ) {
					sprintf(PathName,"WAN%d",inf);
					kd_doCommand( PathName, NULL, ASH_NSD_FAIL, (char *)NULL);
//max add for vpn backup
				/*purpose     : 0013074 author : trenchen date : 2010-07-29 */
				/*description : get value formant changed, fllow new rule   */
				/*sprintf(cmdBuf,"echo \"CMD=%d&WANNUM=%d\" > %s",IPSEC_IPC_NSD_LINKDOWN,inf,NKIPSECD_IPC);*/
				sprintf(cmdBuf,"echo \"CMD=\\\"%d\\\"&WANNUM=\\\"%d\\\"&\" > %s",IPSEC_IPC_NSD_LINKDOWN,inf,NKIPSECD_IPC);
				system(cmdBuf);
//<										
				} else {
					//NK_LOG_SYS(LOG_WARNING, " NSD FAIL WAN[%d]", inf);
					trenchen_printf("nsd fail [%d]\n",inf);
				}
#ifdef CONFIG_NK_SUPPORT_USB_3G
				// start 3G Backup
				kd_doCommand("1", NULL, ASK_CHECK_3G_BACKUP, (char *)NULL);
			}
#endif
			}
			state = PINGSTART;
			sleep(retrytimeout);
		}

	}
	return 0;
}
