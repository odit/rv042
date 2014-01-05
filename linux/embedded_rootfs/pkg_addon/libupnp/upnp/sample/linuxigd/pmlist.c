#include <stdlib.h>
#include <syslog.h>//w
#include <sys/wait.h>
#include <unistd.h>
#include <upnp/upnp.h>
#include "globals.h"
#include "config.h"
#include "pmlist.h"
#include "gatedevice.h"
#include "util.h"

#if HAVE_LIBIPTC
#include "iptc.h"
#endif

//incifer
#include "nkdef.h"
#include "nkutil.h"
#include "../../../../webconfig/webconfig_util.h"
#define INCIFER_IGD 1

//name_get_value
static char *strncpyz(char *dest, char const *src, size_t size)
{
    if (!size--)
	return dest;
    strncpy(dest, src, size);
    dest[size] = 0; /* Make sure the string is null terminated */
    return dest;
}

static int
htoi(s)
	unsigned char	*s;
{
	int	value;
	char	c;

	c = s[0];
	if (isupper(c))
		c = tolower(c);
	value = (c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10) * 16;

	c = s[1];
	if (isupper(c))
		c = tolower(c);
	value += c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10;

	return (value);
}

static void
url_unescape(str)
	unsigned char	*str;
{
	unsigned char	*dest = str;

	while (str[0])
	{
		if (str[0] == '+')
			dest[0] = ' ';
		else if (str[0] == '%' && ishex(str[1]) && ishex(str[2]))
		{
			dest[0] = (unsigned char) htoi(str + 1);
			str += 2;
		}
		else
			dest[0] = str[0];

		str++;
		dest++;
	}

	dest[0] = '\0';
}

static int find_special_word(char *offset, int len, int i)
{
f_again:
    if (((offset[i+len+1] == '\"')&&(offset[i+len+2] == '&'))||((offset[i+len+1] == '\"')&&(offset[i+len+2] == '\"')))
    {
	i++;
	for (i; (((offset[i+len+1] == '\"')&&(offset[i+len+2] == '&'))||((offset[i+len+1] == '\"')&&(offset[i+len+2] == '\"'))); i++);
	goto f_again;
    }
    return i;
}

/*purpose     : 0012882 author : michael lu date : 2010-07-21*/
/*description : support more special character           */
char* name_get_value(char *string, char *varname, char *retval, int buf_len, char *offset)
{
	int i, len=strlen(varname), break_loop=0, ValueEmpty=0;
	char searchName[257];
	char checkValueEnd[257];
	
	strcpy(searchName, varname);
	strcpy(checkValueEnd, varname);
	strcat(checkValueEnd, "=\"\"");
	strcat(searchName,"=\"");
	len+=2;
	
	if (!string)
	{
		retval[0] = '\0';
		return NULL;
	}
	offset = offset ? offset : string;
	/*purpose     : 0012882 author : Jason.Huang date : 2010-07-23*/
	/*description : support more special character              */
	
	char *p=NULL,*q=NULL;
	if(p=strstr(offset, checkValueEnd))
	{
		q=strstr( offset , searchName );
		if(p == q)
			ValueEmpty=1;
	}
	if (((offset=strstr(offset, searchName)) != 0) && (ValueEmpty==0))
	{
		for (i=0; ((offset+i+len)<(string+strlen(string)) && (break_loop==0)); i++)
		{
			if(((offset[i+len+1] == '\"')&&(offset[i+len+2] == '&'))||((offset[i+len+1] == '\"')&&(offset[i+len+2] == '\"')))
			{
				break_loop=1;
			}
		}
		
		i = find_special_word(offset, len, i);
		if (buf_len >= (i+1))
		{
			strncpyz(retval, (offset+len), i+1);
		}
	
		return (offset+len+2+(i+1));
	}
	retval[0] = '\0';
	return NULL;
}

struct portMap* pmlist_NewNode(int enabled, long int duration, char *remoteHost,
			       char *externalPort, char *internalPort,
			       char *protocol, char *internalClient, char *desc)
{
	struct portMap* temp = (struct portMap*) malloc(sizeof(struct portMap));

	temp->m_PortMappingEnabled = enabled;
	
	if (remoteHost && strlen(remoteHost) < sizeof(temp->m_RemoteHost)) strcpy(temp->m_RemoteHost, remoteHost);
		else strcpy(temp->m_RemoteHost, "");
	if (strlen(externalPort) < sizeof(temp->m_ExternalPort)) strcpy(temp->m_ExternalPort, externalPort);
		else strcpy(temp->m_ExternalPort, "");
	if (strlen(internalPort) < sizeof(temp->m_InternalPort)) strcpy(temp->m_InternalPort, internalPort);
		else strcpy(temp->m_InternalPort, "");
	if (strlen(protocol) < sizeof(temp->m_PortMappingProtocol)) strcpy(temp->m_PortMappingProtocol, protocol);
		else strcpy(temp->m_PortMappingProtocol, "");
		
		
#if INCIFER_IGD
	if ( internalClient == NULL ) {
		strcpy(temp->m_InternalClient, "");
	}
	else {
		if (strlen(internalClient) < sizeof(temp->m_InternalClient)) strcpy(temp->m_InternalClient, internalClient);
			else strcpy(temp->m_InternalClient, "");
	}
#else
		
		
	if (strlen(internalClient) < sizeof(temp->m_InternalClient)) strcpy(temp->m_InternalClient, internalClient);
		else strcpy(temp->m_InternalClient, "");
		
#endif

	if (strlen(desc) < sizeof(temp->m_PortMappingDescription)) strcpy(temp->m_PortMappingDescription, desc);
		else strcpy(temp->m_PortMappingDescription, "");
	temp->m_PortMappingLeaseDuration = duration;

	temp->next = NULL;
	temp->prev = NULL;
	
	return temp;
}
	
struct portMap* pmlist_Find(char *externalPort, char *proto, char *internalClient)
{
	struct portMap* temp;
	
	temp = pmlist_Head;
	if (temp == NULL)
		return NULL;
	
	do 
	{
		if ( (strcmp(temp->m_ExternalPort, externalPort) == 0) &&
				(strcmp(temp->m_PortMappingProtocol, proto) == 0) &&
				(strcmp(temp->m_InternalClient, internalClient) == 0) )
			return temp; // We found a match, return pointer to it
		else
			temp = temp->next;
	} while (temp != NULL);
	
	// If we made it here, we didn't find it, so return NULL
	return NULL;
}

struct portMap* pmlist_FindByIndex(int index)
{
	int i=0;
	struct portMap* temp;

	temp = pmlist_Head;
	if (temp == NULL)
		return NULL;
	do
	{
		if (i == index)
			return temp;
		else
		{
			temp = temp->next;	
			i++;
		}
	} while (temp != NULL);

	return NULL;
}	

struct portMap* pmlist_FindSpecific(char *externalPort, char *protocol)
{
	struct portMap* temp;
	
	temp = pmlist_Head;
	if (temp == NULL)
		return NULL;
	
	do
	{
		if ( (strcmp(temp->m_ExternalPort, externalPort) == 0) &&
				(strcmp(temp->m_PortMappingProtocol, protocol) == 0))
			return temp;
		else
			temp = temp->next;
	} while (temp != NULL);

	return NULL;
}

int pmlist_IsEmtpy(void)
{
	if (pmlist_Head)
		return 0;
	else
		return 1;
}

int pmlist_Size(void)
{
	struct portMap* temp;
	int size = 0;
	
	temp = pmlist_Head;
	if (temp == NULL)
		return 0;
	
	while (temp->next)
	{
		size++;
		temp = temp->next;
	}
	size++;
	return size;
}	

int pmlist_FreeList(void)
{
  struct portMap *temp, *next;

  temp = pmlist_Head;
  while(temp) {
    CancelMappingExpiration(temp->expirationEventId);
    pmlist_DeletePortMapping(temp->m_PortMappingEnabled, temp->m_PortMappingProtocol, temp->m_ExternalPort,
			     temp->m_InternalClient, temp->m_InternalPort);
    next = temp->next;
    free(temp);
    temp = next;
  }
  pmlist_Head = pmlist_Tail = NULL;
  return 1;
}
		
int pmlist_PushBack(struct portMap* item)
{
	int action_succeeded = 0;

	if (pmlist_Tail) // We have a list, place on the end
	{
		pmlist_Tail->next = item;
		item->prev = pmlist_Tail;
		item->next = NULL;
		pmlist_Tail = item;
		action_succeeded = 1;
	}
	else // We obviously have no list, because we have no tail :D
	{
		pmlist_Head = pmlist_Tail = pmlist_Current = item;
		item->prev = NULL;
		item->next = NULL;
 		action_succeeded = 1;
		trace(3, "appended %d %s %s %s %s %ld", item->m_PortMappingEnabled, 
				    item->m_PortMappingProtocol, item->m_ExternalPort, item->m_InternalClient, item->m_InternalPort,
				    item->m_PortMappingLeaseDuration);
	}
	if (action_succeeded == 1)
	{
		pmlist_AddPortMapping(item->m_PortMappingEnabled, item->m_PortMappingProtocol,
				      item->m_ExternalPort, item->m_InternalClient, item->m_InternalPort, item->m_PortMappingDescription);	
		return 1;
	}
	else
		return 0;
}

		
int pmlist_Delete(struct portMap* item)
{
	struct portMap *temp;
	int action_succeeded = 0;

	temp = pmlist_Find(item->m_ExternalPort, item->m_PortMappingProtocol, item->m_InternalClient);
	if (temp) // We found the item to delete
	{
	  CancelMappingExpiration(temp->expirationEventId);
		pmlist_DeletePortMapping(item->m_PortMappingEnabled, item->m_PortMappingProtocol, item->m_ExternalPort, 
				item->m_InternalClient, item->m_InternalPort);
		if (temp == pmlist_Head) // We are the head of the list
		{
			if (temp->next == NULL) // We're the only node in the list
			{
				pmlist_Head = pmlist_Tail = pmlist_Current = NULL;
				free (temp);
				action_succeeded = 1;
			}
			else // we have a next, so change head to point to it
			{
				pmlist_Head = temp->next;
				pmlist_Head->prev = NULL;
				free (temp);
				action_succeeded = 1;	
			}
		}
		else if (temp == pmlist_Tail) // We are the Tail, but not the Head so we have prev
		{
			pmlist_Tail = pmlist_Tail->prev;
			free (pmlist_Tail->next);
			pmlist_Tail->next = NULL;
			action_succeeded = 1;
		}
		else // We exist and we are between two nodes
		{
			temp->prev->next = temp->next;
			temp->next->prev = temp->prev;
			pmlist_Current = temp->next; // We put current to the right after a extraction
			free (temp);	
			action_succeeded = 1;
		}
	}
	else  // We're deleting something that's not there, so return 0
		action_succeeded = 0;

	//return action_succeeded;
	if (action_succeeded == 1)
	{
		return 1;
	}
	else 
		return 0;
}

int check_upnp_rule_limit( void ) {
	int		i;
	int		rule_limit = 100, rule_num = 0;
	int		int_num = 0;
	char	str_num[8];
	char	cmdBuf[256], ruleBuf[256];
	char	str_id[20], str_enable[20];

	sprintf ( cmdBuf, "UPNP_FORWARDING NUMBER" );
	kd_doCommand ( cmdBuf, CMD_PRINT, ASH_DO_NOTHING, str_num );
	int_num = atoi ( str_num );

	if ( int_num < 1 )
		return -1;

	if ( int_num < rule_limit )
		return 1;

	for ( i = 1; i <= int_num; i++ ) {
		sprintf ( cmdBuf, "UPNP_FORWARDING ID %d", i );
		kd_doCommand ( cmdBuf, CMD_PRINT, ASH_DO_NOTHING, ruleBuf );

		name_get_value ( ruleBuf, "ID", str_id, sizeof ( str_id ), NULL );
		name_get_value ( ruleBuf, "ENABLED", str_enable, sizeof ( str_enable ), NULL );

		if ( atoi ( str_id ) <= 20 ) {
			if ( atoi ( str_enable ) )
				rule_num ++;
		}
		else {
			rule_num ++;
		}
	}

	if ( rule_num < rule_limit )
		return 1;

	return -1;
}


#if INCIFER_IGD
int pmlist_AddPortMapping (int enable, char *protocol, char *externalPort, char *internalClient, char *internalPort, char *PortMappingDescription)
{
    static char cmdBuf[100], cmdBuf2[100], cmdBuf3[100], counter[4], number[4], proto[4], u_number[4];
    int ucounter, unumber, unumber2;
    int i,j, match=0, match2=0;
    static char forwarding[255], eport[20], iport[20], prot[20], prot1[20], name[50], enabled[2], wanifc[10];
    static char id[5], ip[20], id2[5];
    static char command[500];
    static char ipt_prot[20], ipt_eport[20], ipt_iport[20], ipt_iClient[20];
    int status;
    int int_id2;
	char *p;

    //if(g_debug) syslog(LOG_DEBUG, "[yami]ActionName = pmlist_AddPortMapping");
    //if(g_debug) syslog(LOG_DEBUG, "enable=%d, protocol=%s, eport=%s, internalClient=%s, iport=%s, des:%s", enable, protocol, externalPort, internalClient, internalPort, PortMappingDescription);
    if(!strcmp(protocol, "TCP")) strcpy(protocol, "6");
    if(!strcmp(protocol, "UDP")) strcpy(protocol, "17");

/*Yami : Add to DB , 2006/04/17*/

	//* Get upnp_service_port counter and Add new one
	sprintf(cmdBuf, "UPNP_SERVICE_PORT COUNTER");
	kd_doCommand(cmdBuf, CMD_PRINT, ASH_DO_NOTHING, counter);
	sprintf(cmdBuf2, "UPNP_SERVICE_PORT NUMBER");
	kd_doCommand(cmdBuf2, CMD_PRINT, ASH_DO_NOTHING, number);

	sprintf(cmdBuf3, "UPNP_FORWARDING NUMBER");
	kd_doCommand(cmdBuf3, CMD_PRINT, ASH_DO_NOTHING, u_number);

	/* check upnp forwarding rule limit */
	if ( check_upnp_rule_limit() == -1 ) {
		syslog ( LOG_DEBUG, "UPNP Rule is FULL!!!\n" );
		return 1;
	}
	syslog ( LOG_DEBUG, "UPNP Rule is not FULL!!!\n" );

	//if(g_debug) syslog(LOG_DEBUG, "UPNP_SERVICE_PORT COUNTER[%s]/NUMBER[%s], UPNP_FORWARDING NUMBER[%s]\n", counter, number, u_number);

	// " " ->  "_" , prevent DB error
	//if(g_debug ) syslog ( LOG_DEBUG, "PortMappingDescription=%s\n", PortMappingDescription );
	p = PortMappingDescription;
	while ( p ) {
		p = strchr ( p, ' ' );
		if ( p )
			*p = '_';
	}
	// "[" ->  "(" , prevent DB error
	p = PortMappingDescription;
	while ( p ) {
		p = strpbrk ( p, "{[" );
		if ( p )
			*p = '(';
	}
	// " " ->  "_" , prevent DB error
	p = PortMappingDescription;
	while ( p ) {
		p = strpbrk ( p, "}]" );
		if ( p )
			*p = ')';
	}

	/*Add upn_service_port entry*/
	//check if upnp_service exist
	for(j=0; j<atoi(number); j++)
	{
		sprintf(cmdBuf, "UPNP_SERVICE_PORT ID %d", j+1);
		kd_doCommand(cmdBuf, CMD_PRINT, ASH_DO_NOTHING, forwarding);
		//syslog(LOG_DEBUG, "%s=%s", cmdBuf, forwarding);
		name_get_value(forwarding, "START", eport, sizeof(eport), NULL);
		name_get_value(forwarding, "PROTOCOL", prot, sizeof(prot), NULL);
		name_get_value(forwarding, "ID", id2, sizeof(id2), NULL);

		//if(g_debug) syslog(LOG_DEBUG, "eport:%s, externalPort:%s ; prot:%s protocol:%s", eport, externalPort, prot, protocol);
		if((!strcmp(eport, externalPort))&&(!strcmp(prot,protocol)))
		{
			//if(g_debug) syslog(LOG_DEBUG, "match ... %s port %s, id=%s", prot, eport, id2);
			match=999;
			break;
		}
	}

	//Add
	//if !exist, New Service
	//if(g_debug) syslog(LOG_DEBUG, "match=%d", match);
	if(match!=999)
	{
		ucounter = atoi(counter)+1;
		unumber = atoi(number)+1;
		sprintf(cmdBuf, "UPNP_SERVICE_PORT ID=\"ID=\"%d\"&PROTOCOL=\"%s\"&START=\"%s\"&END=\"%s\"&NAME=\"%s\"&\"",
		ucounter, protocol, externalPort, internalPort, PortMappingDescription);
		KdDoCmdWrite(cmdBuf,NULL);
		//if(g_debug) syslog(LOG_DEBUG, "[yami]Add %s", cmdBuf);
		sprintf(cmdBuf, "UPNP_SERVICE_PORT COUNTER=%d", ucounter);
		KdDoCmdWrite(cmdBuf,NULL);
		//if(g_debug) syslog(LOG_DEBUG, "[yami]%s", cmdBuf);
		sprintf(cmdBuf2, "UPNP_SERVICE_PORT NUMBER=%d", unumber);
		KdDoCmdWrite(cmdBuf2,NULL);
		//if(g_debug) syslog(LOG_DEBUG, "[yami]%s", cmdBuf2);
	}

	//check if upnp_forwarding exist
	unumber2=atoi(u_number);
	for(i=0; i<atoi(u_number); i++)
	{
		sprintf(cmdBuf, "UPNP_FORWARDING ID %d", i+1);
		kd_doCommand(cmdBuf, CMD_PRINT, ASH_DO_NOTHING, forwarding);

		name_get_value(forwarding, "ID", id, sizeof(id), NULL);
		name_get_value(forwarding, "IP", ip, sizeof(ip), NULL);
		name_get_value(forwarding, "ENABLED", enabled, sizeof(enabled), NULL);
		name_get_value(forwarding, "PROTOCOL", prot, sizeof(prot), NULL);
		name_get_value(forwarding, "START", eport, sizeof(eport), NULL);
		name_get_value(forwarding, "END", iport, sizeof(iport), NULL);
		name_get_value(forwarding, "NAME", name, sizeof(name), NULL);

		//if(g_debug) syslog(LOG_DEBUG, "[upnp_f]eport:%s, externalPort:%s, prot:%s, protocol:%s", eport, externalPort, prot, protocol);
		if((!strcmp(eport, externalPort))&&(!strcmp(prot,protocol))) //check eport & protocol
		{
			sprintf(cmdBuf, "UPNP_FORWARDING ID %d", i+1);
			KdDoCmdDelete(cmdBuf);
			match2=999;
			//Del ipt rule
			if(strlen(ip))
			{
				sprintf(command, "iptables -t nat -D upnp -p %s --dport %s -j DNAT --to %s:%s",
				protocol, externalPort, ip, iport);
			}
			else
			{
				sprintf(command, "iptables -t nat -D upnp -p %s --dport %s -j DNAT --to %s:%s",
				protocol, externalPort, internalClient, iport);
			}
			//if(g_debug) syslog(LOG_DEBUG, "[yami]%s", command);

			if (!fork()) {
			exit(system(command));
			} else {
				wait(&status);
			}
			break;
		}
	}

	//if(g_debug) syslog(LOG_DEBUG, "match=%d", match);
	//if(g_debug) syslog(LOG_DEBUG, "match2=%d", match2);
	if(match2==999) //del first, then add
	{
		if(!strcmp(prot, "6")) strcpy(prot1, "TCP");
		else if(!strcmp(prot, "17")) strcpy(prot1, "UDP");
		if(strlen(ip))
		{
			sprintf(cmdBuf3, "UPNP_FORWARDING ID=\"ID=\"%s\"&IP=\"%s\"&ENABLED=\"%d\"&%s=\"%s:%s\"&PROTOCOL=\"%s\"&START=\"%s\"&END=\"%s\"&NAME=\"%s\"&\"",
			id2, ip, enable, prot1, externalPort, iport, prot, externalPort, iport, name);

			int_id2 = atoi(id2);
			int_id2++;
			sprintf(id2, "%d", int_id2);
		}
		else
		{
			sprintf(cmdBuf3, "UPNP_FORWARDING ID=\"ID=\"%s\"&IP=\"%s\"&ENABLED=\"%d\"&%s=\"%s:%s\"&PROTOCOL=\"%s\"&START=\"%s\"&END=\"%s\"&NAME=\"%s\"&\"",
			id2, internalClient, enable, prot1, externalPort, iport, prot, externalPort, iport, name);

			int_id2 = atoi(id2);
			int_id2++;
			sprintf(id2, "%d", int_id2);
		}
		unumber2-=1;
		strcpy(ipt_prot, prot1);
		strcpy(ipt_eport, iport);
		if(strlen(ip))
			strcpy(ipt_iClient, ip);
		else	strcpy(ipt_iClient, internalClient);
		strcpy(ipt_iport, iport);
	}
	else
	{
		if(!strcmp(protocol, "6")) strcpy(prot1, "TCP");
		else if(!strcmp(protocol, "17")) strcpy(prot1, "UDP");
		//if(g_debug) syslog(LOG_DEBUG, "match=%d", match);
		if(match==999) //Old Service
		{
			//syslog(LOG_DEBUG, "id2");
			sprintf(cmdBuf3, "UPNP_FORWARDING ID=\"ID=\"%s\"&IP=\"%s\"&ENABLED=\"%d\"&%s=\"%s:%s\"&PROTOCOL=\"%s\"&START=\"%s\"&END=\"%s\"&NAME=\"%s\"&\"",
			id2, internalClient, enable, prot1, externalPort, internalPort, protocol, externalPort, internalPort, PortMappingDescription);
			strcpy(ipt_prot, prot1);
			strcpy(ipt_eport, externalPort);
			strcpy(ipt_iClient, internalClient);
			strcpy(ipt_iport, internalPort);

			int_id2 = atoi(id2);
			int_id2++;
			sprintf(id2, "%d", int_id2);
		}
		else
		{
			//syslog(LOG_DEBUG, "(atoi(counter)+1)");
			sprintf(cmdBuf3, "UPNP_FORWARDING ID=\"ID=\"%d\"&IP=\"%s\"&ENABLED=\"%d\"&%s=\"%s:%s\"&PROTOCOL=\"%s\"&START=\"%s\"&END=\"%s\"&NAME=\"%s\"&\"",
			(atoi(counter)+1), internalClient, enable, prot1, externalPort, internalPort, protocol, externalPort, internalPort, PortMappingDescription);
			strcpy(ipt_prot, prot1);
			strcpy(ipt_eport, externalPort);
			strcpy(ipt_iClient, internalClient);
			strcpy(ipt_iport, internalPort);
		}
	}

	KdDoCmdWrite(cmdBuf3,NULL);
	//if(g_debug) syslog(LOG_DEBUG, "[yami]Add %s", cmdBuf3);
	unumber2+=1;
	sprintf(cmdBuf, "UPNP_FORWARDING NUMBER=%d", unumber2);
	KdDoCmdWrite(cmdBuf,NULL);
	//if(g_debug) syslog(LOG_DEBUG, "[yami]%s", cmdBuf);

	if (enable)
	{
		sprintf(command, "iptables -t nat -A upnp -p %s --dport %s -j DNAT --to %s:%s",
		ipt_prot, ipt_eport, ipt_iClient, ipt_iport);
		//if(g_debug) syslog(LOG_DEBUG, "[yami]%s", command);

		if (!fork()) {
		exit(system(command));
		} else {
			wait(&status);
		}
	}

	match=match2=0;
	//End Add to DB

    return 1;
}
#else

int pmlist_AddPortMapping (int enabled, char *protocol, char *externalPort, char *internalClient, char *internalPort)
{
    if (enabled)
    {
#if HAVE_LIBIPTC
	char *buffer = malloc(strlen(internalClient) + strlen(internalPort) + 2);
	if (buffer == NULL) {
		fprintf(stderr, "failed to malloc memory\n");
		return 0;
	}

	strcpy(buffer, internalClient);
	strcat(buffer, ":");
	strcat(buffer, internalPort);

	if (g_vars.forwardRules)
		iptc_add_rule("filter", g_vars.forwardChainName, protocol, NULL, NULL, NULL, internalClient, NULL, internalPort, "ACCEPT", NULL, FALSE);

	iptc_add_rule("nat", g_vars.preroutingChainName, protocol, g_vars.extInterfaceName, NULL, NULL, NULL, NULL, externalPort, "DNAT", buffer, TRUE);
	free(buffer);
#else
	char command[COMMAND_LEN];
	int status;
	
	{
	  char dest[DEST_LEN];
	  char *args[] = {"iptables", "-t", "nat", "-I", g_vars.preroutingChainName, "-i", g_vars.extInterfaceName, "-p", protocol, "--dport", externalPort, "-j", "DNAT", "--to", dest, NULL};

	  snprintf(dest, DEST_LEN, "%s:%s", internalClient, internalPort);
	  snprintf(command, COMMAND_LEN, "%s -t nat -I %s -i %s -p %s --dport %s -j DNAT --to %s:%s", g_vars.iptables, g_vars.preroutingChainName, g_vars.extInterfaceName, protocol, externalPort, internalClient, internalPort);
	  trace(3, "%s", command);
	  if (!fork()) {
	    int rc = execv(g_vars.iptables, args);
	    exit(rc);
	  } else {
	    wait(&status);		
	  }
	}

	if (g_vars.forwardRules)
	{
	  char *args[] = {"iptables", "-A", g_vars.forwardChainName, "-p", protocol, "-d", internalClient, "--dport", internalPort, "-j", "ACCEPT", NULL};
	  
	  snprintf(command, COMMAND_LEN, "%s -A %s -p %s -d %s --dport %s -j ACCEPT", g_vars.iptables,g_vars.forwardChainName, protocol, internalClient, internalPort);
	  trace(3, "%s", command);
	  if (!fork()) {
	    int rc = execv(g_vars.iptables, args);
	    exit(rc);
	  } else {
	    wait(&status);		
	  }
	}
#endif
    }
    return 1;
}

#endif


int pmlist_DeletePortMapping(int enable, char *protocol, char *externalPort, char *internalClient, char *internalPort)
{
#if INCIFER_IGD
    char cmdBuf[100], cmdBuf2[100], cmdBuf3[100], counter[4], number[4], u_number[4], name[50];
    int i, ucounter, unumber, unumber2;
    char ip[255];
    char forwarding[255], eport[20], iport[20], prot[20], proto[20], id[5], enabled[2], wanifc[10];
    char ids[5];

     //if(g_debug) syslog(LOG_DEBUG, "ActionName = pmlist_DeletePortMapping");
     //if(g_debug) syslog(LOG_DEBUG, "enable=%d, proto=%s, eport=%s, internalClient=%s, iport=%s", enable, protocol, externalPort, internalClient, internalPort);

	 
	 
    //if (enabled)
    //{
#if HAVE_LIBIPTC
	char *buffer = malloc(strlen(internalClient) + strlen(internalPort) + 2);
	strcpy(buffer, internalClient);
	strcat(buffer, ":");
	strcat(buffer, internalPort);

	if (g_vars.forwardRules)
	    iptc_delete_rule("filter", g_vars.forwardChainName, protocol, NULL, NULL, NULL, internalClient, NULL, internalPort, "ACCEPT", NULL);

	iptc_delete_rule("nat", g_vars.preroutingChainName, protocol, g_vars.extInterfaceName, NULL, NULL, NULL, NULL, externalPort, "DNAT", buffer);
	free(buffer);
#else
	char command[COMMAND_LEN];
	int status;
	
	/*{
	  char dest[DEST_LEN];
	  char *args[] = {"iptables", "-t", "nat", "-D", g_vars.preroutingChainName, "-i", g_vars.extInterfaceName, "-p", protocol, "--dport", externalPort, "-j", "DNAT", "--to", dest, NULL};

	  snprintf(dest, DEST_LEN, "%s:%s", internalClient, internalPort);
	  snprintf(command, COMMAND_LEN, "%s -t nat -D %s -i %s -p %s --dport %s -j DNAT --to %s:%s",
		  g_vars.iptables, g_vars.preroutingChainName, g_vars.extInterfaceName, protocol, externalPort, internalClient, internalPort);
	  trace(3, "%s", command);
	  
	  if (!fork()) {
	    int rc = execv(g_vars.iptables, args);
	    exit(rc);
	  } else {
	    wait(&status);		
	  }
	}

	if (g_vars.forwardRules)
	{
	  char *args[] = {"iptables", "-D", g_vars.forwardChainName, "-p", protocol, "-d", internalClient, "--dport", internalPort, "-j", "ACCEPT", NULL};
	  
	  snprintf(command, COMMAND_LEN, "%s -D %s -p %s -d %s --dport %s -j ACCEPT", g_vars.iptables, g_vars.forwardChainName, protocol, internalClient, internalPort);
	  trace(3, "%s", command);
	  if (!fork()) {
	    int rc = execv(g_vars.iptables, args);
	    exit(rc);
	  } else {
	    wait(&status);		
	  }
	}
#endif
    }
    return 1;*/
	#endif
  //  }
//  if(strlen(internalClient))
	/*Yami: Delete from DB, 2006/04/24*/
	/*Yami: Delete UPNP_FORWARDING */
	sprintf(cmdBuf, "UPNP_FORWARDING NUMBER");
	kd_doCommand(cmdBuf, CMD_PRINT, ASH_DO_NOTHING, number);

	for (i=0; i<atoi(number); i++)
	{
		sprintf(cmdBuf, "UPNP_FORWARDING ID %d", i+1);
		kd_doCommand(cmdBuf, CMD_PRINT, ASH_DO_NOTHING, forwarding);

		name_get_value(forwarding, "START", eport, sizeof(eport), NULL);
		name_get_value(forwarding, "END", iport, sizeof(iport), NULL);
		name_get_value(forwarding, "IP", ip, sizeof(ip), NULL);
		name_get_value(forwarding, "ENABLED", enabled, sizeof(enabled), NULL);
		name_get_value(forwarding, "ID", id, sizeof(id), NULL);
		name_get_value(forwarding, "PROTOCOL", prot, sizeof(prot), NULL);
		name_get_value(forwarding, "NAME", name, sizeof(name), NULL);
		//if(g_debug) syslog(LOG_DEBUG, "%s", forwarding);


		if(!strcmp(prot, "6")) strcpy(proto, "TCP");
		else if(!strcmp(prot, "17")) strcpy(proto, "UDP");

		//if(g_debug) syslog(LOG_DEBUG, "[yami]eport=%s, externalPort=%s, protocol=%s proto=%s", eport, externalPort, protocol, proto);
		if( ((!strcmp(proto, protocol)) && (!strcmp(eport, externalPort))) )
		{
			sprintf(command, "iptables -t nat -D upnp -p %s --dport %s -j DNAT --to %s:%s",
				proto, eport, ip, iport);
			//if(g_debug) syslog(LOG_DEBUG, "[yami]%s", command);

			if (!fork()) {
				exit(system(command));
			} else {
				wait(&status);
			}
			
			//if(g_debug) syslog(LOG_DEBUG, "[yami]Delete %s from UPNP_FORWARDING ", name);
			sprintf(cmdBuf, "UPNP_FORWARDING ID %d", i+1);
			KdDoCmdDelete(cmdBuf);
			//if(g_debug) syslog(LOG_DEBUG, "[yami]Del %s", cmdBuf);
			unumber = atoi(number) - 1;
			if(atoi(id)<21)
			{
				sprintf(cmdBuf3, "UPNP_FORWARDING ID=\"ID=\"%s\"&IP=\"\"&ENABLED=\"0\"&%s=\"%s:%s\"&PROTOCOL=\"%s\"&START=\"%s\"&END=\"%s\"&NAME=\"%s\"&\"",
				id, proto, eport, iport, prot, eport, iport, name);
				KdDoCmdWrite(cmdBuf3,NULL);
				unumber++;
			}
			sprintf(cmdBuf, "UPNP_FORWARDING NUMBER=%d", unumber);
			KdDoCmdWrite(cmdBuf,NULL);
			//if(g_debug) syslog(LOG_DEBUG, "[yami]%s", cmdBuf);
		}
	}

	/*Yami: Delete UPNP_SERVICE_PORT*/
	sprintf(cmdBuf, "UPNP_SERVICE_PORT NUMBER");
	kd_doCommand(cmdBuf, CMD_PRINT, ASH_DO_NOTHING, number);
	sprintf(cmdBuf, "UPNP_SERVICE_PORT COUNTER");
	kd_doCommand(cmdBuf, CMD_PRINT, ASH_DO_NOTHING, counter);
	for (i=0; i<atoi(number); i++)
	{
		sprintf(cmdBuf, "UPNP_SERVICE_PORT ID %d", i+1);
		kd_doCommand(cmdBuf, CMD_PRINT, ASH_DO_NOTHING, forwarding);

		name_get_value(forwarding, "START", eport, sizeof(eport), NULL);
		name_get_value(forwarding, "END", iport, sizeof(iport), NULL);
		name_get_value(forwarding, "ID", ids, sizeof(ids), NULL);
		name_get_value(forwarding, "PROTOCOL", prot, sizeof(prot), NULL);
		name_get_value(forwarding, "NAME", name, sizeof(name), NULL);
		//h=gethostbyname(ip);

		if(!strcmp(prot, "6")) strcpy(proto, "TCP");
		else if(!strcmp(prot, "17")) strcpy(proto, "UDP");

		////if(g_debug) syslog(LOG_DEBUG, "[yami]eport=%s, iport=%s,ip=%s, protocol=%s name=%s", eport, iport, ip, prot, name);
		////if(g_debug) syslog(LOG_DEBUG, "[yami]externalPort=%s, internalPort=%s", externalPort, internalPort);
		if((!strcmp(externalPort, eport)) && (!strcmp(proto, protocol)) && (atoi(ids)>20))
		{
			//if(g_debug) syslog(LOG_DEBUG, "[yami]Delete %s from UPNP_SERVICE_PORT ", name);
			sprintf(cmdBuf, "UPNP_SERVICE_PORT ID %d", i+1);
			KdDoCmdDelete(cmdBuf);
			//if(g_debug) syslog(LOG_DEBUG, "[yami]Del %s", cmdBuf);
			unumber = atoi(number) - 1;
			ucounter = atoi(counter) - 1;
			sprintf(cmdBuf, "UPNP_SERVICE_PORT NUMBER=%d", unumber);
			KdDoCmdWrite(cmdBuf,NULL);
			//if(g_debug) syslog(LOG_DEBUG, "[yami]%s", cmdBuf);
			sprintf(cmdBuf, "UPNP_SERVICE_PORT COUNTER=%d", ucounter);
			KdDoCmdWrite(cmdBuf,NULL);
			//if(g_debug) syslog(LOG_DEBUG, "[yami]%s", cmdBuf);
		}
	}
	/*==================================================*/
    	return 1;
#else
    if (enabled)
    {
#if HAVE_LIBIPTC
	char *buffer = malloc(strlen(internalClient) + strlen(internalPort) + 2);
	strcpy(buffer, internalClient);
	strcat(buffer, ":");
	strcat(buffer, internalPort);

	if (g_forwardRules)
	    iptc_delete_rule("filter", g_forwardChainName, protocol, NULL, NULL, NULL, internalClient, NULL, internalPort, "ACCEPT", NULL);

	iptc_delete_rule("nat", g_preroutingChainName, protocol, g_extInterfaceName, NULL, NULL, NULL, NULL, externalPort, "DNAT", buffer);
	free(buffer);
#else
	char command[500];
	int status;
	
	//sprintf(command, "%s -t nat -D %s -i %s -p %s --dport %s -j DNAT --to %s:%s",
	//		g_iptables, g_preroutingChainName, g_extInterfaceName, protocol, externalPort, internalClient, internalPort);
	//sprintf(command, "%s -t nat -D upnp -i %s -p %s --dport %s -j DNAT --to %s:%s",
	//		g_iptables, g_extInterfaceName, protocol, externalPort, internalClient, internalPort);
	sprintf(command, "%s -t nat -D upnp -p %s --dport %s -j DNAT --to %s:%s",
			g_iptables, protocol, externalPort, internalClient, internalPort);
	//if(g_debug) syslog(LOG_DEBUG, command);

/*	if (!fork()) {
		system (command);
	} else {
		wait(&status);
	}
*/
	system(command);

	if (g_forwardRules)
	{
	    sprintf(command,"%s -D %s -p %s -d %s --dport %s -j ACCEPT", g_iptables, g_forwardChainName, protocol, internalClient, internalPort);
	    sprintf(command,"%s -D upnp -p %s -d %s --dport %s -j ACCEPT", g_iptables, protocol, internalClient, internalPort);
	    //if(g_debug) syslog(LOG_DEBUG, command);
			/*if (!fork()) {
				system (command);
			} else {
				wait(&status);
			}*/
		system(command);
	}
#endif
    }
    return 1;
#endif

	
	
	
	
	
	
	
	
}
