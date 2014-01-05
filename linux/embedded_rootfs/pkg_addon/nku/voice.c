#include <stdlib.h>
#include <stdio.h>

#include <features.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>

#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "nkdef.h"
#include "nkutil.h"
#include "cpld.h"
int Voice_play(char *messages,char *type)
{
	voice_message_t vm;
	int fd;
	int voicecheck ;
	int voicetime ;
	int voiceenable ;
	int voiceperiod ;
	char sBuf[100];
	if(!strcmp(WANCHANGEIP,type))
	{
    		kd_doCommand("VOICE WAN_CHANGE_IP_CHECK", CMD_PRINT, ASH_DO_NOTHING, sBuf);
//		kd_Log("VOICE LOGIN_CHECK=%s",sBuf);	
		voicecheck = atoi(sBuf);	
    		kd_doCommand("VOICE WAN_CHANGE_IP_TIME", CMD_PRINT, ASH_DO_NOTHING, sBuf);		
//		kd_Log("VOICE LOGIN_TIME=%s",sBuf);	
		voicetime = atoi(sBuf);	
    		kd_doCommand("VOICE ENABLED", CMD_PRINT, ASH_DO_NOTHING, sBuf);		
//		kd_Log("VOICE ENABLED=%s",sBuf);	
		voiceenable = atoi(sBuf);	
    		kd_doCommand("VOICE WAN_CHANGE_IP_PERIOD", CMD_PRINT, ASH_DO_NOTHING, sBuf);		
//		kd_Log("VOICE LOGIN_PERIOD=%s",sBuf);	
		voiceperiod = atoi(sBuf);	
		sprintf(vm.message, "%s0%d", messages, voicetime);
		vm.pri=HIGH_PRI;
		vm.interval=voiceperiod;
	}
	else if(!strcmp(WANDOWN,type))
	{
//		kd_Log("WANDOWN start");
    		kd_doCommand("VOICE WAN_DOWN_CHECK", CMD_PRINT, ASH_DO_NOTHING, sBuf);
//		kd_Log("VOICE LOGIN_CHECK=%s",sBuf);	
		voicecheck = atoi(sBuf);	
    		kd_doCommand("VOICE WAN_DOWN_TIME", CMD_PRINT, ASH_DO_NOTHING, sBuf);		
//		kd_Log("VOICE LOGIN_TIME=%s",sBuf);	
		voicetime = atoi(sBuf);	
    		kd_doCommand("VOICE ENABLED", CMD_PRINT, ASH_DO_NOTHING, sBuf);		
//		kd_Log("VOICE ENABLED=%s",sBuf);	
		voiceenable = atoi(sBuf);	
  //  		kd_doCommand("VOICE WAN_DOWN_IP_PERIOD", CMD_PRINT, ASH_DO_NOTHING, sBuf);		
//		kd_Log("VOICE LOGIN_PERIOD=%s",sBuf);	
//		voiceperiod = atoi(sBuf);	
		sprintf(vm.message, "%s0%d", messages, voicetime);
//		kd_Log("vm.message=%s",vm.message);
		vm.pri=HIGH_PRI;
		vm.interval=5;
	}
	else if(!strcmp(WANUP,type))
	{
//		kd_Log("WANDOWN start");
    		kd_doCommand("VOICE WAN_UP_CHECK", CMD_PRINT, ASH_DO_NOTHING, sBuf);
//		kd_Log("VOICE LOGIN_CHECK=%s",sBuf);	
		voicecheck = atoi(sBuf);	
    		kd_doCommand("VOICE WAN_UP_TIME", CMD_PRINT, ASH_DO_NOTHING, sBuf);		
//		kd_Log("VOICE LOGIN_TIME=%s",sBuf);	
		voicetime = atoi(sBuf);	
    		kd_doCommand("VOICE ENABLED", CMD_PRINT, ASH_DO_NOTHING, sBuf);		
//		kd_Log("VOICE ENABLED=%s",sBuf);	
		voiceenable = atoi(sBuf);	
  //  		kd_doCommand("VOICE WAN_DOWN_IP_PERIOD", CMD_PRINT, ASH_DO_NOTHING, sBuf);		
//		kd_Log("VOICE LOGIN_PERIOD=%s",sBuf);	
//		voiceperiod = atoi(sBuf);	
		sprintf(vm.message, "%s0%d", messages, voicetime);
//		kd_Log("vm.message=%s",vm.message);
		vm.pri=HIGH_PRI;
		vm.interval=5;
	}
	else if(!strcmp(WANDOWNJAM,type))
	{
    		kd_doCommand("VOICE DOWNSRTEAM_JAM_CHECK", CMD_PRINT, ASH_DO_NOTHING, sBuf);
//		kd_Log("VOICE LOGIN_CHECK=%s",sBuf);	
		voicecheck = atoi(sBuf);	
    		kd_doCommand("VOICE DOWNSRTEAM_JAM_TIME", CMD_PRINT, ASH_DO_NOTHING, sBuf);		
//		kd_Log("VOICE LOGIN_TIME=%s",sBuf);	
		voicetime = atoi(sBuf);	
    		kd_doCommand("VOICE ENABLED", CMD_PRINT, ASH_DO_NOTHING, sBuf);		
//		kd_Log("VOICE ENABLED=%s",sBuf);	
		voiceenable = atoi(sBuf);	
    		kd_doCommand("VOICE DOWNSRTEAM_JAM_PERIOD", CMD_PRINT, ASH_DO_NOTHING, sBuf);		
//		kd_Log("VOICE LOGIN_PERIOD=%s",sBuf);	
		voiceperiod = atoi(sBuf);	
		sprintf(vm.message, "%s0%d", messages, voicetime);
		vm.pri=LOW_PRI;
		vm.interval=voiceperiod;
	}
	else if(!strcmp(WANUPJAM,type))
	{
    		kd_doCommand("VOICE UPSRTEAM_JAM_CHECK", CMD_PRINT, ASH_DO_NOTHING, sBuf);
//		kd_Log("VOICE LOGIN_CHECK=%s",sBuf);	
		voicecheck = atoi(sBuf);	
    		kd_doCommand("VOICE UPSRTEAM_JAM_TIME", CMD_PRINT, ASH_DO_NOTHING, sBuf);		
//		kd_Log("VOICE LOGIN_TIME=%s",sBuf);	
		voicetime = atoi(sBuf);	
    		kd_doCommand("VOICE ENABLED", CMD_PRINT, ASH_DO_NOTHING, sBuf);		
//		kd_Log("VOICE ENABLED=%s",sBuf);	
		voiceenable = atoi(sBuf);	
    		kd_doCommand("VOICE UPSRTEAM_JAM_PERIOD", CMD_PRINT, ASH_DO_NOTHING, sBuf);		
//		kd_Log("VOICE LOGIN_PERIOD=%s",sBuf);	
		voiceperiod = atoi(sBuf);	
		sprintf(vm.message, "%s0%d", messages, voicetime);
		vm.pri=LOW_PRI;
		vm.interval=voiceperiod;
	}
	if( (voicecheck==1) && (voicetime>0) && (voiceenable==1) )
	{
		if ((fd = open("/dev/nk_switch", O_RDONLY)) < 0)
		{
			printf("failed open /dev/nk_switch");
			close(fd);
			return -1;
		}
		if (ioctl(fd, VOICE_IC_CONTROL1, &vm) != 0) 
			printf("ioctl error\n");	
		close(fd);
	}
	return 0;
}