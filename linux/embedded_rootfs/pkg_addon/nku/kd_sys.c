
#include <stdio.h>

/*#define WINDOWS*/
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "sysconfig.h"

#ifndef WINDOWS
 #include <strings.h>
 #include <sys/stat.h>
 #include <unistd.h>
#endif
#include <stdarg.h>
#include <time.h>

// --> Kide
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
// <--

#include "nkutil.h"
//#include "nkdef.h"

static char *Release = "RELEASE", *Version = "sysconfig 0.90 (January 25, 2004)";


static char *fconfig  = SYS_FILE; //"/tmp/sysconfig";
static char *nk_fconfig  = SYS_FILE;		// new sysconfig file
static char *nk_fconfig1  = BACKUP_SYS_FILE;	// for UI backup
static char *tmpath   = "/tmp/stemp";
#ifdef CONFIG_NK_LICENSE_COUNT_DOWN
static char *factory_tmpath="/etc/flash/stmp";
#endif

#define  GEN_ERROR          1  /*not any of the following errors*/
#define  INVALID_COMD       2  /*invalid command group; not any of -p -w -d -v -h*/
#define  INVALID_GROUP      3  /*invalid command from A to Z*/
#define  INVALID_PARAMETER  4  /*invalid parameter, wrong, or missing parameters */
#define  CAT_NOT_FOUND      5  /*fail to find the catergory field*/
#define  ITEM_NOT_FOUND     6  /*fail to find the item */
#define  FAIL_TO_SEEK       7  /*fail to move file pointer to the item position*/
#define  FAIL_TO_READ       8  /*fail to read from file*/
#define  FAIL_TO_OPEN       9  /*fail to open file*/

//#define SQUARE_BRACKET		// support [ or ] for value of item
//#define COPY_FILEOPEN		// use fopen to open file to copy, otherwise use rename() 

//void kd_Log(char *format, ...);

//TT 2005/12/20 : use static or sometimes will failed to get [PPPOEx] USERNAME
static char glvalue[ARGV_SIZE];
static char * reqBuf=(char *) NULL;

/*interface for program*/

/*interface for shell*/
static void version(void);
static void usage(void);
static int  print_setting (char type, char *param, int flag); 
static int  print_config(FILE *handle, char *category, char* item, int number);
static int  delete_setting (char type, char *param);
static int  find_category(FILE *handle, char *category);
static int  find_item(FILE *handle, char *item);
static void print_item_value(FILE *handle);
static int  insert_item(FILE *handle, char *item, int flag);
#ifdef CONFIG_NK_LICENSE_COUNT_DOWN
static int  insert_item_factory(FILE *handle, char *item, int flag);
#endif
static int  remove_item(FILE *handle);
static long find_last_item(FILE *handle, char *item);
static long find_postion(FILE *handle, char *item, int flag);
static long move_to_item(FILE *handle, char *category, char *item, int number );
static int  get_item_value(FILE *handle);
static int  create_dhcp_config(void);
static int  create_pptp_config(void);
static int  process_comd(char cmd, char type, char *paras);
static int  set_configuration (char type, char *param);
static int  printSwitchHandler(char *param, FILE *file_handle);
static int  setSwitchHandler(char *param);
static int  create_resolv(void);

//TT -->
static int  nk_print_setting (char *type, char *param, int flag, char *dbname);
static int  nk_create_db (char *type, char *param);
static int  nk_new_setting (char *type, char *param, char *dbname);
static int  nk_set_configuration (char *type, char *param, char *dbname);
#ifdef CONFIG_NK_LICENSE_COUNT_DOWN
static int  nk_set_configuration_factory (char *type, char *param, char *dbname);
#endif
static int  nk_mod_configuration (char *type, char *param, char *dbname);
static int  nk_delete_setting (char *type, char *param, char *dbname);
static int  nk_process_comd(char cmd, char *type, char *paras, char *dbname);
#ifdef CONFIG_NK_LICENSE_COUNT_DOWN
static int  nk_process_comd_factory(char cmd, char *type, char *paras, char *dbname);
#endif
static int  find_last_category_item(FILE *handle, char *item);
static long move_to_category(FILE *handle, char *category);
static int  remove_category(FILE *handle);
//<--

//--> Michael Lu add for new read function
typedef struct {
    char entry[LENGTH_READ_SIZE];
    struct db_value_get *next;
}db_value_get;
//<-- Michael Lu add for new read function
void usage(void)
{
	printf("Usage:\n  sysconfig [-p] [Type] [Parameter] [number] \n");
	printf("            [-w] [Type] [Parameter=value] \n");
	printf("            [-m] [Type] [Parameter number=value] \n");
	printf("            [-d] [Type] [Parameter] [number] \n");
	printf("            [-b] [Type] [Parameter] [number] \n");
	printf("            [-n] [Type] [Parameter=value] \n");
	printf("            [-t] [Task Name] \n");
}

void version(void)
{
   printf("%s\n%s\n", Release, Version);
}

void kdsys_printf(char *str)
{
/*
 FILE *fp;
 fp = fopen("/dev/console", "w");
 if (fp == NULL) {
   return;
 }
 fprintf(fp, "ssi.cgi: %s\r\n", str);
 fflush(fp);
 fclose(fp);*/
}
#ifdef CONFIG_NK_DYNAMIC_PORT_NUM
unsigned int Get_Num_Wan_kdsys() {
// 	char cmdBuf[128];
// 	int tmp;

// 	kd_doCommand("VERSION NUM_WAN",CMD_PRINT, ASH_DO_NOTHING, cmdBuf);
// 
// 	tmp = atoi(cmdBuf);
// 	if ( !tmp )
// 		return 8;
// 
// 	return tmp;

	FILE *fd;
	char tmpBuf[128];
	int num_wan=0;

	if ( ( fd = fopen ( "/tmp/splitDB/VERSION", "r+" ) ) == NULL ) {
		return 0;
	}

	while ( !feof(fd) ) {
		fscanf(fd, "%s\n", tmpBuf);
		//kd_Log("/tmp/splitDB/VERSION: %s", tmpBuf);
		if ( strstr ( tmpBuf, "NUM_WAN" ) ) {
			sscanf ( tmpBuf, "NUM_WAN=%d\n", &num_wan );
			break;
		}
	}

	fclose(fd);
	return num_wan;
}

unsigned int Get_Num_Dmz_kdsys() {
	FILE *fd;
	char tmpBuf[128];
	int num_dmz=0;

	if ( ( fd = fopen ( "/tmp/splitDB/VERSION", "r+" ) ) == NULL ) {
		return 0;
	}

	while ( !feof(fd) ) {
		fscanf(fd, "%s\n", tmpBuf);
		//kd_Log("/tmp/splitDB/VERSION: %s", tmpBuf);
		if ( strstr ( tmpBuf, "NUM_DMZ" ) ) {
			sscanf ( tmpBuf, "NUM_DMZ=%d\n", &num_dmz );
			break;
		}
	}

	fclose(fd);
	return num_dmz;
}

unsigned int Get_Num_USB_kdsys() {


	FILE *fd;
	char tmpBuf[128];
	int num_usb=0;

	if ( ( fd = fopen ( "/tmp/splitDB/VERSION", "r+" ) ) == NULL ) {
		return 0;
	}

	while ( !feof(fd) ) {
		fscanf(fd, "%s\n", tmpBuf);
		//kd_Log("/tmp/splitDB/VERSION: %s", tmpBuf);
		if ( strstr ( tmpBuf, "NUM_USB" ) ) {
			sscanf ( tmpBuf, "NUM_USB=%d\n", &num_usb );
			break;
		}
	}

	fclose(fd);
	return num_usb;
}
#endif

/* this is the entry point for shell program sysconfig */
int sysconfig(int argc, char argv[3][ARGV_SIZE], char *supBuf)
{
    int   idx, pidx, result;
    char  category, comd, paras[128], tmparg[ARGV_SIZE];

    /* Find any options.  argc is at least 1  the argv[0] is the program name*/
    argc--;  
    argv++;
 
    tmparg[0] = '\0';

    reqBuf = supBuf;

    result = 0;

    if ( argc > 2 )
    {
       idx = 1;
       while ( argv[idx] != NULL )
       {
          strcat(tmparg, argv[idx]);
          strcat(tmparg, " ");
          idx++;
       }
    }
    idx = strlen(tmparg);
    if (tmparg[idx-1] == ' ')
        tmparg[idx - 1] = '\0';

    else if ( NULL != argv[1] )
          strcpy ( tmparg, argv[1] );
    else 
    {
      if (argc == 0 || !strcmp(*argv, "-?") || !strcmp(*argv, "-h"))
      {
        usage();
        return 0;
      }
	
      if (!strcmp(*argv, "-v"))
      {
  	     version();
	     return 0;
      }
    } 

    if (!strcmp(*argv, "-c"))
    {
       result = create_dhcp_config();
       return (result);
    }

    if (!strcmp(*argv, "-r"))
    {
       result = create_resolv();
       return (result);
    }

    comd = argv[0][1];
    category = tmparg[1];

    if ( tmparg[2] == ' ')
      idx = 3;
    else
      idx = 2;
    //printf("tmparg1=%s",tmparg);
    while ( tmparg[idx] != '\0') 
	{
      pidx = 0;
    
	  while ( tmparg[idx] != ' ' && tmparg[idx] != '\0')
  	      paras[pidx++] = tmparg[idx++];
      paras[pidx] = '\0';
		   
      /*skip all the extra spaces */
      while ( tmparg[idx] == ' ' )
    	idx++;
             
      result = process_comd(comd, category, paras);

	  if ( (result == ITEM_NOT_FOUND ) && ((comd == 'p') || (comd == 'b')) && (category == 'F' || category == 'S' ||
	      category == 'T' || category == 'M'|| category == 'R' || category == 'Z' || category == 'C'))
	  {
         if ( reqBuf == NULL )
		     printf ( "END");
         else
           sprintf(reqBuf, "END");
	  }
	  else if ( result == INVALID_GROUP ) 
	  {
	      if ( reqBuf == NULL )
       	     printf("Unrecongnized command: %c\n", comd);		     
          else 
             sprintf(reqBuf, "Unrecongnized command: %c\n", comd);
	  }
	  else if ( result != 0 )
	  {
         if ( reqBuf == NULL )
	        printf("ERROR Code %d, Catergory %c\n", result, category);
         else
            sprintf(reqBuf, "No More %c", category);
	  }
	}
	
	if ( tmparg [2] == '\0' )
	{
		result = process_comd(comd, category, NULL);
    	if ( (result == ITEM_NOT_FOUND ) && ((comd == 'p') || (comd == 'b')) && (category == 'F' || category == 'S' ||
			 category == 'T' || category == 'M' || category == 'R' || category == 'Z'|| category == 'C'))
        {
           if ( reqBuf == NULL )
                printf ( "END");
           else
                sprintf(reqBuf, "END");
        }
        else if ( result == INVALID_GROUP )
        {
            if ( reqBuf == NULL )
                 printf("Unrecongnized command: %c\n", comd);
            else 
                 sprintf(reqBuf, "Unrecongnized command: %c\n", comd);
        }
        else if ( result != 0 )
        {
           if ( reqBuf == NULL )
                printf("ERROR Code %d, catergory %c\n", result, category);
           else
                sprintf(reqBuf, "Unrecongnized options: %c\n", category);
        }
	}
    return (result); 
}

/*idx the number of item to be skipped, starting from 1*/

static int print_config(FILE *handle, char *category, char* item, int idx )
{
	int i, result;

	if ( 0 == (result= find_category(handle, category)))
	{
		/*skip first idx items*/
		for ( i=1; i<idx; i++)
		{
		   //kd_Log("%d\n", __LINE__);
		   result = find_item(handle, item);
		   if ( 0 != result )
			   return (result);
		}
		/*get to the target item */
		if ( 0 == (result = find_item(handle, item)))
                     print_item_value(handle);
		//kd_Log("%d\n", __LINE__);
	}
	return (result);
}


int set_config(char *category, char *item, char *value, int number, char *filename)
{
	FILE *database, *ftmp;
	int result, i, count;
	char buf[100];

kdsys_printf("set_config start !!");
    if ( NULL == (database = fopen(filename, "r+")))
    {
        return  FAIL_TO_OPEN;
    }

    
	if ( 0 == (result = find_category(database, category)))
	{
		/*skip first idx items*/
		for ( i=1; i<number; i++)
		{
		   result = find_item(database, item);
		   if ( 0 != result )
		   {
			   fclose(database);
			   return (result);
		   }
		}

		if ( 0 == (result = find_item(database, item)))
		{
		   if ( (value[0] == 'y' || value [0] == 'Y') && 
			    (value[1] == 'e' || value [1] == 'E') &&
				(value[2] == 's' || value [2] == 'S') )
		   {
			   value[0] = 'Y';
			   value[1] = 'E';
			   value[2] = 'S';
		   }

		   if ( (value[0] == 'n' || value [0] == 'N') && 
			    (value[1] == 'o' || value [1] == 'O') )
		   {
			   value[0] = 'N';
			   value[1] = 'O';
		   }
		   strcat(value, "\n");

		   result = insert_item(database, value, 1);
        	}
	}
	fclose(database);
	// TT : so if category not found, still copy tmpath to fconfig ?????
	/*update the change*/
#ifdef COPY_FILEOPEN	
    if ( NULL == (database = fopen(filename, "w+")))
    { 
        return  FAIL_TO_OPEN;
    }
    
	if ( NULL == (ftmp = fopen(tmpath, "r+")))
	{
		fclose(database);
		return (FAIL_TO_OPEN);
	}

    while (!feof(ftmp))
    {
        count = fread (buf, sizeof(char), 40, ftmp);
        fwrite(buf, sizeof(char), count, database);
    }
	fclose(database);
    fclose(ftmp);
#else
	rename(tmpath, filename);
#endif
kdsys_printf("set_config end !!");
	return (result);
}

#ifdef CONFIG_NK_LICENSE_COUNT_DOWN
int set_config_factory(char *category, char *item, char *value, int number, char *filename)
{
	FILE *database, *ftmp;
	int result, i, count;
	char buf[100];

    if ( NULL == (database = fopen(filename, "r+")))
    {
        return  FAIL_TO_OPEN;
    }

    
	if ( 0 == (result = find_category(database, category)))
	{
		/*skip first idx items*/
		for ( i=1; i<number; i++)
		{
		   result = find_item(database, item);
		   if ( 0 != result )
		   {
			   fclose(database);
			   return (result);
		   }
		}

		if ( 0 == (result = find_item(database, item)))
		{
			result = insert_item_factory(database, value, 1);
        }
	}
	fclose(database);
	// TT : so if category not found, still copy factory_tmpath to fconfig ?????
	/*update the change*/
// #ifdef 0	
//     if ( NULL == (database = fopen(filename, "w+")))
//     { 
//         return  FAIL_TO_OPEN;
//     }
//     
// 	if ( NULL == (ftmp = fopen(factory_tmpath, "r+")))
// 	{
// 		fclose(database);
// 		return (FAIL_TO_OPEN);
// 	}
// 
//     while (!feof(ftmp))
//     {
//         count = fread (buf, sizeof(char), 40, ftmp);
//         fwrite(buf, sizeof(char), count, database);
//     }
// 	fclose(database);
//     fclose(ftmp);
// #else
	rename(factory_tmpath, filename);
// #endif
	return (result);
}
#endif

/* move the file pointer to the item and return -1L if failed */
static long move_to_item(FILE *handle, char *category, char *item, int number )
{
   int   idx;
   long  position = -1L;
   char  tmp[3];

   if ( 0 == find_category(handle, category))
   {
	   position = ftell(handle);
	   idx = 0;
	   do
	   {
		   position= find_postion(handle, item, number);
		   if ( number !=  0 )
		   {
	   	     do
			 {
	           if ( 0 == fread(tmp, sizeof(char), 1, handle) )
			      return (-1L);
			 }while (tmp[0] != '\n');
		     idx ++;
		   }
	   }while ( idx < number );
   }
   if ( position >= 0L )
     fseek(handle, position, SEEK_SET);
   return (position);
}

static long move_to_category(FILE *handle, char *category)
{
//   int   idx;
   long  position = -1L;
//   char  tmp[3];

   if ( 0 == find_category(handle, category))
   {
	   position = ftell(handle);
   }
//   printf ("%d %d \n",__LINE__, strlen(category));
   if ( position >= 0L )
     fseek(handle, position-strlen(category)-3, SEEK_SET); // -3 means \n [ ]
   return (position);
}

static long find_postion(FILE *handle, char *item, int flag)
{
   int   idx;
   long  position;
   char  tmp[3], temp[128], fg;
   
   if ( flag == 0 ) 
	   fg = '\n';   /*by whole item string match*/
   else
	   fg = '=';    /*by item number in the list first one is 1 */

   do 
   {
     idx = 0;
	 position = ftell(handle);
     do
	 {
	    if ( 0 != fread(tmp, sizeof(char), 1, handle) )
		{
	       temp[idx++] = tmp[0];
#ifdef SQUARE_BRACKET	
			if ( tmp[0] == '[')
			{
				if ( 0 != fread(tmp, sizeof(char), 1, handle) )
				{
					temp[idx++] = tmp[0];
					if ( tmp[0] != '[')	// TT : only one [, the beginning of [section]
						return -1L;
				}
				else
					return -1L;
			}
#else    
		   if ( tmp[0] == '[')
			   return -1L;
#endif		   
		}
	    else
		   return (-1L);
	 }while ( tmp[0] != fg );

	 if ( fg == '=') 
	    temp[idx] = '\0';
	 else
        temp[idx-1] = '\0';
#ifndef WINDOWS
	 if (!strcasecmp (item, temp))
#else
     if (!stricmp(item, temp))
#endif
	 {
	     if ( 0 == fseek(handle, position, SEEK_SET))
		   return (position);
		 else 
		   return (-1L);
	 }
	 else
	 {
	   if ( fg == '=')
	   {
		  do
		  {
		    if ( 0 == fread(tmp, sizeof(char), 1, handle))
			    return (-1L);
		  }while (tmp[0] != '\n');
	   }
	   position = ftell(handle);
	 }
#ifdef SQUARE_BRACKET	
		if (tmp[0] != '[')
			continue;
		else
		{
			if ( 0 == fread(tmp, sizeof(char), 1, handle))
				return (-1L);
			else
			{
				if (tmp[0] != '[')
					return (-1L);
				else
				{
					// TT : is it hs probelm that next is the second '[', then cannot find a pair for this ????
					fseek(handle, -1, SEEK_CUR);	//TT backward 1
				}
			}
		}
	} while (1);//(tmp[0] != '[');
#else
   } while (tmp[0] != '[');
#endif		   
   return (-1L);
}

int add_config(char *category, char *item, char *value, char *filename )
{
	FILE  *database, *ftmp; 
	int   result, idx, count;
	long  ps;
	char  tpvalue[ARGV_SIZE];
	
    if ( NULL == (database = fopen(filename, "r+")))
    {
//		kd_Log("failed to open %s for updating!\n", fconfig);
        return  FAIL_TO_OPEN;
    }
//printf ("%d \n", __LINE__);

    if ( 0 == (result = find_category(database, category)))
	{
		ps = find_last_item(database, item);

		if ( 0 != fseek(database, ps, SEEK_SET))
		{
			fclose(database);
			return(FAIL_TO_SEEK);
		}
		idx = 0;
#if 0	//TT
		while (value[idx] != '\0')
		{
			if ( value[idx] == ',' )
				value[idx] = ' ';
			idx++;
		}
#endif
		strcpy(tpvalue, item);
		strcat(tpvalue, "=");
		strcat(tpvalue, value);
		strcat(tpvalue, "\n");
		result = insert_item(database, tpvalue, 0);
	}
	fclose(database);
	/*update the change*/
	// TT : so if category not found, still copy tmpath to fconfig ?????
#ifdef COPY_FILEOPEN	
    if ( NULL == (database = fopen(filename, "w+")))
    { 
        return  FAIL_TO_OPEN;
    }
    
	if ( NULL == (ftmp = fopen(tmpath, "r+")))
	{
		fclose(database);
		return (FAIL_TO_OPEN);
	}

    while (!feof(ftmp))
    {
        count = fread (tpvalue, sizeof(char), 40, ftmp);
        fwrite(tpvalue, sizeof(char), count, database);
    }
	fclose(database);
    fclose(ftmp);
#else
	rename(tmpath, filename);
#endif
	return (result);
}

/*cheked*/
static int find_category(FILE *handle, char *category)
{
	int idx, result = 0;
    char temp[ARGV_SIZE], tmp[3];

kdsys_printf("find_category start !!");
	if ( 0 != fseek(handle, 0L, SEEK_SET))
		return (FAIL_TO_SEEK);

	while ( !feof(handle) )
	{
		if ( 0 == fread(tmp, sizeof(char), 1, handle))
		{
			result = CAT_NOT_FOUND;
			break;
		}
		else if (tmp[0] == '[')
		{
		  idx = 0;
#ifdef SQUARE_BRACKET    
		    if ( 0 == fread(tmp, sizeof(char), 1, handle))
			{ 
				result = CAT_NOT_FOUND;
				break;
			}
			if (tmp[0] == '[')
				continue;
			else
		        temp[idx++] = tmp[0];
#endif		   
		  do
		  {
		    if ( 0 == fread(tmp, sizeof(char), 1, handle))
			{ 
				result = CAT_NOT_FOUND;
				break;
			}
			else
		        temp[idx++] = tmp[0];
		  }
		  while (tmp[0] != ']');

		  if ( result == CAT_NOT_FOUND )
			  break;
		  else
		  {
		     temp[idx -1 ] = '\0';
		     if (!strcmp (category, temp))
			 {
			    result = 0;
				do
				{
		          if ( 0 == fread(tmp, sizeof(char), 1, handle))
                       break;
				} while ( tmp[0] != '\n');
				break;
			 }
		  }
	   }
	}
kdsys_printf("find_category end !!");
	return (result);
}


/*
 * this function call only be called after the 
 * function find_category has been called
 */

/*cheked*/
static int find_item(FILE *handle, char *item)
{
   int loop, len, idx;
   char temp[ARGV_SIZE];

kdsys_printf("find_item start !!");
#if 0	// TT verify the movement of file pointer
char tmp[3];
long position;   
#endif
 

   loop = 1;
   while (loop)
   {
     if ( EOF == fscanf(handle, "%s", temp) )
       return ITEM_NOT_FOUND;

kdsys_printf("temp !!");
kdsys_printf(temp);
     //kd_Log("temp=%s\n",temp);
#ifdef CONFIG_NK_USE_ORIGINAL_DB
#ifdef SQUARE_BRACKET    
     if ((temp[0] == '[') && (temp[1] != '[')) 
       return ITEM_NOT_FOUND;
#else
     if (temp[0] == '[') 
       return ITEM_NOT_FOUND;
#endif		   
#endif
     len = strlen(temp);
     for (idx=0; idx<len; idx++) {
       if (temp[idx] == '=') {
	 temp[idx] = '\0';
	 break;
       }
     }

     if (idx < len) {
       if (!strcmp(item, temp)) {
         //kd_Log("temp=%s find\n",temp);
	 fseek(handle, idx-len+1, SEEK_CUR);
         //kd_Log("temp=%s find ok\n",temp);
#if 0	// TT verify the movement of file pointer
position = ftell(handle);
printf("%d, %d\n", __LINE__, position);
fread(tmp, sizeof(char), 3, handle);
position = ftell(handle);
printf("%d, %c%c%c %d\n", __LINE__, tmp[0], tmp[1], tmp[2], position);
fseek(handle, -2, SEEK_CUR);
position = ftell(handle);
printf("%d, %d\n", __LINE__, position);
fread(tmp, sizeof(char), 1, handle);
position = ftell(handle);
printf("%d, %c %d\n", __LINE__, tmp[0], position);
fseek(handle, -2, SEEK_CUR);
#endif

	 //	 fscanf(handle
	 return 0;
       }
     }
   }
kdsys_printf("end start !!");
   return ITEM_NOT_FOUND;
}

//TT -->
static int find_last_category_item(FILE *handle, char *item)
{
   int loop, len, idx;
   char temp[ARGV_SIZE];
 
   loop = 1;
   while (loop)
   {
		if ( EOF == fscanf(handle, "%s", temp) )
		{
			len = strlen(temp);
//			fseek(handle, -1, SEEK_CUR);
			return ITEM_NOT_FOUND;
     	}
		//printf("temp=%s\n",temp);
#ifdef SQUARE_BRACKET  
		if ((temp[0] == '[') && (temp[1] != '['))
#else  
		if (temp[0] == '[') 
#endif		   
		{
			len = strlen(temp);
			fseek(handle, -len-1, SEEK_CUR);
			return ITEM_NOT_FOUND;
		}
		len = strlen(temp);
		for (idx=0; idx<len; idx++) {
			if (temp[idx] == '=') {
				temp[idx] = '\0';
				break;
			}
		}

		if (idx < len) {
			if (!strcmp(item, temp)) {
				fseek(handle, idx-len+1, SEEK_CUR);
				//	 fscanf(handle
				return 0;
			}
		}
	}
	return ITEM_NOT_FOUND;
}
//<--

/*return the file pointer postion after the last item found cheked*/
static long find_last_item(FILE *handle, char *item)
{
   int idx;
   long position;
   char temp[128], tmp[3];

   position = ftell(handle);

   do 
   {
     idx = 0;
     do
	 {
	    if ( 0 != fread(tmp, sizeof(char), 1, handle) )
		{
#ifdef SQUARE_BRACKET	
			temp[idx++] = tmp[0];
			if ( tmp[0] == '[')
			{    
				if ( 0 != fread(tmp, sizeof(char), 1, handle) )
				{
					temp[idx++] = tmp[0];
					if ( tmp[0] != '[')
						return (position);
				}
				else
					return (position);
			}
#else
           if ( (tmp[0] >= 'A' && tmp[0] <= 'Z') || (tmp[0] == '_') 
	       || (tmp[0] >= '0' && tmp[0] <= '9') )
	         temp[idx++] = tmp[0];
		   else if ( tmp[0] == '[')
			   return (position);
#endif		   
		}
	    else
		   return (position);
	 }while (tmp[0] != '=');

	 temp[idx] = '\0';
	 if (!strcmp (item, temp))
	 {
	   do
	   {
          if (0 == fread(tmp, sizeof(char), 1, handle))
			  break;
	   }while ( tmp[0] != '\n');
	
	   position = ftell(handle);
	 }
	 else
	 {
	   while (tmp[0] != '\n')
	   {
		 if ( 0 == fread(tmp, sizeof(char), 1, handle))
			  return (position);
	   }
	 }
#ifdef SQUARE_BRACKET	
		if (tmp[0] != '[')
			continue;
		else
		{
			if ( 0 == fread(tmp, sizeof(char), 1, handle))
				return (position);
			else
			{
				if (tmp[0] != '[')
					return (position);
				else
					fseek(handle, -1, SEEK_CUR);	//TT backward 1
			}
	 	}
	} while (1);//(tmp[0] != '[');
#else
   } while (tmp[0] != '[');
#endif		   
   return (position);
}

/* simple print the item value to console or string buffer checked*/
static void print_item_value(FILE *handle)
{
   if ( 0 == get_item_value (handle))
   {
     if ( reqBuf == NULL )
       printf("%s\n", glvalue);
     else
       sprintf(reqBuf, glvalue);
   }
   else
   {
	   if ( reqBuf == NULL )
		   printf ("Fail to get item value\n");
	   else
		   sprintf (reqBuf, "Fail to get item value\n");
   }
}

/* return the item value string in glvalue checked*/
static int get_item_value(FILE *handle)
{
	char tmp[3];
	int  idx, result;

	idx = result = 0;
	do
	{
	    if ( 0 != fread(tmp, sizeof(char), 1, handle) )
		{
#ifdef SQUARE_BRACKET    
	        glvalue[idx++] = tmp[0];
			if ((tmp[0] == '[') || (tmp[0] == ']'))
				fread(tmp, sizeof(char), 1, handle);
#else
	        glvalue[idx++] = tmp[0];
#endif	    
		}
		else
	    {
		   result = FAIL_TO_READ;
		   break;
	    }
	 }while (tmp[0] != '\n');
	 if ( idx > 1 )
	   glvalue[idx - 1] = '\0';
	 else
	   glvalue[0] = '\0';
	 return (result);
}

/*insert item from the current postion, if flag=1 the insertion takes
  place at the beginning of the next line checked
*/
static int insert_item(FILE *handle, char *item, int flag)
{
   FILE *ftmp;
   char tmp[3], newline;
   long  postion;
int leaveFlag=0;
int needDouble=0, pre=0, behind=0;
#ifdef SQUARE_BRACKET    
   int  i, j;
   char newitem[ARGV_SIZE], tmp1[3];
#endif

kdsys_printf("  start !!");
   if ( NULL == (ftmp = fopen(tmpath, "w+")))
   {
//       kd_Log("failed to open %s", tmpath);
       return (FAIL_TO_OPEN);
   }

#ifdef SQUARE_BRACKET    
	i = j = 0;
	while ( item[i] != '\0')
	{
		if ((item[i] == '[') || (item[i] == ']'))
		{
			for (j = 0; j < i ; j++)
			{
				newitem[j] = item[j];
			}
			newitem[j++] = item[i];
			newitem[j++] = item[i];
			i++;
			//j++;
			for (; item[i] != '\0'; i++, j++)
			{
				newitem[j] = item[i];
				if ((item[i] == '[') || (item[i] == ']'))
				{
					newitem[j+1] = newitem[j];
					j++;
				}
			}
			newitem[j] = '\0';
//			printf ("%d, %s %d, %s %d\n", __LINE__, item, strlen(item), newitem, strlen(newitem));
//			printf ("%d\n", __LINE__);
			item = newitem;
			break;
		}
		i++;
	}
#endif

   postion = ftell(handle);
//printf ("%d \n", __LINE__);

   if ( 0 != fseek(handle, 0L, SEEK_SET))
   {
       fflush(ftmp);
       fclose (ftmp);
       return (FAIL_TO_SEEK);
   }

   while (postion != ftell(handle))
   {
       fread (tmp, sizeof(char), 1, handle);
       fwrite(tmp, sizeof(char), 1, ftmp);
   }
   fwrite (item, sizeof(char), strlen(item), ftmp);
   tmp[0] = '\n';

needDouble=0;
leaveFlag=0;
pre=0;
behind=0;
   /*move the the beginning of the next line*/
   if ( 1 == flag )
   {
	do
	{
		fread (tmp, sizeof(char), 1, handle);
	
		if(tmp[0] == '"')
		{
			needDouble++;
			if(pre==1)
				behind=1;
			if(behind==0)
				pre=1;
		}
		else
		{
			pre=0;
			behind=0;
		}

		if((needDouble == 0) || (needDouble == 2) || ((needDouble %2==0)&&(needDouble>2)) ||((behind==1) && (pre==1)))
		if(tmp[0] == '\n')
		{
			leaveFlag =1;
		}
	}while (leaveFlag ==0);
     //}while (tmp[0] != '\n');
   }
//printf ("%d \n", __LINE__);

	newline = tmp[0];
	while (!feof(handle))
	{
#ifdef SQUARE_BRACKET	
		fread (tmp, sizeof(char), 1, handle);
		if (tmp[0] == '[')
		{
			fread (tmp1, sizeof(char), 1, handle);
			if (tmp1[0] != '[')
			{
				fwrite("\n", sizeof(char), 1, ftmp);
				fwrite(tmp, sizeof(char), 1, ftmp);
				fseek(handle, -1, SEEK_CUR);	//TT backward 1
			}
			else	//TT two [[
			{
				fwrite(tmp, sizeof(char), 1, ftmp);
				fwrite(tmp1, sizeof(char), 1, ftmp);
				tmp[0] = tmp1[0];
			}
		}
		else if (!((newline == '\n') && (tmp[0] == '\n')))
			fwrite(tmp, sizeof(char), 1, ftmp);
		newline = tmp[0];
#else
		fread (tmp, sizeof(char), 1, handle);
#ifdef CONFIG_NK_USE_ORIGINAL_DB
			if (tmp[0] == '[')
			{
				fwrite("\n", sizeof(char), 1, ftmp);
				fwrite(tmp, sizeof(char), 1, ftmp);
			}
			else if (!((newline == '\n') && (tmp[0] == '\n')))
				fwrite(tmp, sizeof(char), 1, ftmp);
#else
			if (!((newline == '\n') && (tmp[0] == '\n')))
				fwrite(tmp, sizeof(char), 1, ftmp);
#endif
		newline = tmp[0];
#endif
	}
   fclose(ftmp);
kdsys_printf("insert_item end !!");
   return 0;
}

#ifdef CONFIG_NK_LICENSE_COUNT_DOWN
static int insert_item_factory(FILE *handle, char *item, int flag)
{
	FILE *ftmp;
	char tmp[3], newline;
	long  postion;
	int leaveFlag=0;
	int needDouble=0;
#ifdef SQUARE_BRACKET    
	int  i, j;
	char newitem[ARGV_SIZE], tmp1[3];
#endif
	//strcpy(factory_tmpath,item);
	if ( NULL == (ftmp = fopen(factory_tmpath, "w+")))
	{
	kd_Log("failed to open %s", factory_tmpath);
		return (FAIL_TO_OPEN);
	}

#ifdef SQUARE_BRACKET    
	i = j = 0;
	while ( item[i] != '\0')
	{
		if ((item[i] == '[') || (item[i] == ']'))
		{
			for (j = 0; j < i ; j++)
			{
				newitem[j] = item[j];
			}
			newitem[j++] = item[i];
			newitem[j++] = item[i];
			i++;
			//j++;
			for (; item[i] != '\0'; i++, j++)
			{
				newitem[j] = item[i];
				if ((item[i] == '[') || (item[i] == ']'))
				{
					newitem[j+1] = newitem[j];
					j++;
				}
			}
			newitem[j] = '\0';
//			printf ("%d, %s %d, %s %d\n", __LINE__, item, strlen(item), newitem, strlen(newitem));
//			printf ("%d\n", __LINE__);
			item = newitem;
			break;
		}
		i++;
	}
#endif

	postion = ftell(handle);
	//printf ("%d \n", __LINE__);

   if ( 0 != fseek(handle, 0L, SEEK_SET))
   {
       fflush(ftmp);
       fclose (ftmp);
       return (FAIL_TO_SEEK);
   }

   while (postion != ftell(handle))
   {
       fread (tmp, sizeof(char), 1, handle);
       fwrite(tmp, sizeof(char), 1, ftmp);
   }
   fwrite (item, sizeof(char), strlen(item), ftmp);
   fwrite ("\n", 1, 1, ftmp);
   tmp[0] = '\n';

	needDouble=0;
	leaveFlag=0;
   /*move the the beginning of the next line*/
	if ( 1 == flag )
	{
		do
		{
			fread (tmp, sizeof(char), 1, handle);

			if(tmp[0] == '"')
				needDouble++;

			if((needDouble == 0) || (needDouble == 2))
				if(tmp[0] == '\n')
				{
					leaveFlag =1;
				}
		}while (leaveFlag ==0);
	//}while (tmp[0] != '\n');
	}
	//printf ("%d \n", __LINE__);

	newline = tmp[0];
	while (!feof(handle))
	{
#ifdef SQUARE_BRACKET	
		fread (tmp, sizeof(char), 1, handle);
		if (tmp[0] == '[')
		{
			fread (tmp1, sizeof(char), 1, handle);
			if (tmp1[0] != '[')
			{
				fwrite("\n", sizeof(char), 1, ftmp);
				fwrite(tmp, sizeof(char), 1, ftmp);
				fseek(handle, -1, SEEK_CUR);	//TT backward 1
			}
			else	//TT two [[
			{
				fwrite(tmp, sizeof(char), 1, ftmp);
				fwrite(tmp1, sizeof(char), 1, ftmp);
				tmp[0] = tmp1[0];
			}
		}
		else if (!((newline == '\n') && (tmp[0] == '\n')))
			fwrite(tmp, sizeof(char), 1, ftmp);
		newline = tmp[0];
#else
		fread (tmp, sizeof(char), 1, handle);

		if (tmp[0] == '[')
		{
			fwrite("\n", sizeof(char), 1, ftmp);
			fwrite(tmp, sizeof(char), 1, ftmp);
		}
		else if (!((newline == '\n') && (tmp[0] == '\n')))
			fwrite(tmp, sizeof(char), 1, ftmp);
		newline = tmp[0];
#endif
	}
   fclose(ftmp);
   return 0;
}
#endif

/*checked*/
int remove_item(FILE *handle)
{
   FILE *ftmp; 
   char  tmp[3], newline;
   long  postion;
#ifdef SQUARE_BRACKET    
   char tmp1[3];
#endif

   if ( NULL == (ftmp = fopen(tmpath, "w+")))
	    return (FAIL_TO_OPEN);

   postion = ftell(handle);

   if ( 0 != fseek(handle, 0L, SEEK_SET))
   {
      fclose(ftmp);
	  return(FAIL_TO_SEEK);
   }

   while (postion != ftell(handle))
   {
	   fread (tmp, sizeof(char), 1, handle);
	   fwrite(tmp, sizeof(char), 1, ftmp);
   }
   do
   {
      fread (tmp, sizeof(char), 1, handle);
   }while (tmp[0] != '\n');

	newline = '\0';
	while (!feof(handle))
	{
#ifdef SQUARE_BRACKET	
		fread (tmp, sizeof(char), 1, handle);
		if (tmp[0] == '[')
		{
			fread (tmp1, sizeof(char), 1, handle);
			if (tmp1[0] != '[')
			{
				fwrite("\n", sizeof(char), 1, ftmp);
				fwrite(tmp, sizeof(char), 1, ftmp);
				fseek(handle, -1, SEEK_CUR);	//TT backward 1
			}
			else	//TT two [[
			{
				fwrite(tmp, sizeof(char), 1, ftmp);
				fwrite(tmp1, sizeof(char), 1, ftmp);
				tmp[0] = tmp1[0];
			}
		}
		else if (!((newline == '\n') && (tmp[0] == '\n')))
			fwrite(tmp, sizeof(char), 1, ftmp);
		newline = tmp[0];
#else
		fread (tmp, sizeof(char), 1, handle);
		if (tmp[0] == '[')
		{
			fwrite("\n", sizeof(char), 1, ftmp);
			fwrite(tmp, sizeof(char), 1, ftmp);
		}
		else if (!((newline == '\n') && (tmp[0] == '\n')))
			fwrite(tmp, sizeof(char), 1, ftmp);
		newline = tmp[0];
#endif		   
	}

#if 0
   while (!feof(handle))
   {
	   fread (tmp, sizeof(char), 1, handle);
	   fwrite(tmp, sizeof(char), 1, ftmp);
   }
#endif
  fclose(ftmp);
  return 0;
}

int remove_category(FILE *handle)
{
   FILE *ftmp;
   char  tmp[3];
   long  postion;

   if ( NULL == (ftmp = fopen(tmpath, "w+")))
	    return (FAIL_TO_OPEN);

   postion = ftell(handle);

   if ( 0 != fseek(handle, 0L, SEEK_SET))
   {
      fclose(ftmp);
	  return(FAIL_TO_SEEK);
   }

   while (postion != ftell(handle))
   {
	   fread (tmp, sizeof(char), 1, handle);
	   fwrite(tmp, sizeof(char), 1, ftmp);
   }
   fread (tmp, sizeof(char), 1, handle); // read [
#ifdef SQUARE_BRACKET	
	do
	{
		fread (tmp, sizeof(char), 1, handle);
		if (feof(handle))
			break;
		if (tmp[0] == '[')
		{
			fread (tmp, sizeof(char), 1, handle);
			if (feof(handle))
				break;
			if (tmp[0] != '[')
			{
				fseek(handle, -1, SEEK_CUR);	//TT backward 1
				break;
			}
		}
   }while (1); //((tmp[0] != '[') && (!feof(handle)));
#else
   do
   {
      fread (tmp, sizeof(char), 1, handle);
   }while ((tmp[0] != '[') && (!feof(handle)));
#endif		   

//	newline = '\0';
	if (!feof(handle))
		fwrite(tmp, sizeof(char), 1, ftmp);
	while (!feof(handle))
	{
		fread (tmp, sizeof(char), 1, handle);
		fwrite(tmp, sizeof(char), 1, ftmp);
	}
#if 0
   while (!feof(handle))
   {
	   fread (tmp, sizeof(char), 1, handle);
	   fwrite(tmp, sizeof(char), 1, ftmp);
   }
#endif
  fclose(ftmp);
  return 0;
}

static int create_resolv(void)
{
  FILE *wfd = NULL, *database;
  int  wanidx, result, dnsno = 0;
  char opcode[60];
#ifdef CONFIG_NK_DYNAMIC_PORT_NUM
	unsigned int DYNAMIC_NUM_WAN;
	unsigned int DYNAMIC_NUM_DMZ;
	unsigned int DYNAMIC_NUM_USB;
#endif
  
#ifdef CONFIG_NK_DYNAMIC_PORT_NUM
	DYNAMIC_NUM_WAN = Get_Num_Wan_kdsys();
	DYNAMIC_NUM_DMZ = Get_Num_Dmz_kdsys();
	DYNAMIC_NUM_USB = Get_Num_USB_kdsys();
#endif
  
	if ( NULL == (database = fopen(fconfig, "r+")))
		return  FAIL_TO_OPEN;

	if ( 0 == ( result = find_category(database, "SYSTEM")))
	{
		if ( 0 == ( result = find_item(database, "DOMAINNAME")))
		{
			result = get_item_value(database);
			if ( result == 0)
			{
				if ((wfd = fopen(RESOLV_CONF, "w")) == NULL)
				{
//					kd_Log("fail to create %s\n",dhcpconf);
					fclose(database);
					return (FAIL_TO_OPEN);
				}
				fprintf(wfd, "domain  %s\n",glvalue);
				//fprintf(wfd, "search  %s\n",glvalue);
			}
		}
	}

#ifdef CONFIG_NK_DYNAMIC_PORT_NUM
	for (wanidx = 0; wanidx < DYNAMIC_NUM_WAN+DYNAMIC_NUM_DMZ+DYNAMIC_NUM_USB; wanidx++)
	//for (wanidx = 0; wanidx < DYNAMIC_NUM_WAN; wanidx++)
#else
	for (wanidx = 0; wanidx < CONFIG_NK_NUM_WAN; wanidx++)
#endif
	{
		sprintf(opcode, "ISP%d", wanidx+1);
		if ( !find_category(database, opcode) && !find_item(database, "GATEWAY") )
		{
			get_item_value(database);
			if ( !strcmp(glvalue, "0.0.0.0") )
			{
				continue;
			}
		}
		
		sprintf(opcode, "WAN%d", wanidx+1);
		
		if ( 0 == ( result = find_category(database, opcode)))
		{
			if ( !find_item(database, "USERSPECIALDNS") &&  !get_item_value(database) )
			{
				if ( !strcmp(glvalue, "NO") )	// not USERSPECIALDNS, ISP DNS
					sprintf(opcode, "ISP%d", wanidx+1);
			}
		}
		
		if ( 0 == ( result = find_category(database, opcode)))
		{
			if ( 0 == ( result = find_item(database, "DNS1"))) 
			{
				result = get_item_value(database);
				if ( result == 0) 
				{
					if (strcmp(glvalue, "0.0.0.0")) 
					{
						fprintf(wfd, "nameserver %s\n",glvalue);
						dnsno ++;
					}
				}
			}
		}
	}
	for (wanidx = 0; wanidx < DYNAMIC_NUM_WAN+DYNAMIC_NUM_DMZ+DYNAMIC_NUM_USB; wanidx++)
	// for (wanidx = 0; wanidx < CONFIG_NK_NUM_WAN; wanidx++)
	{
		sprintf(opcode, "ISP%d", wanidx+1);
		if ( !find_category(database, opcode) && !find_item(database, "GATEWAY") )
		{
			get_item_value(database);
			if ( !strcmp(glvalue, "0.0.0.0") )
			{
				continue;
			}
		}
		
		sprintf(opcode, "WAN%d", wanidx+1);
		
		if ( 0 == ( result = find_category(database, opcode)))
		{
			if ( !find_item(database, "USERSPECIALDNS") &&  !get_item_value(database) )
			{
				if ( !strcmp(glvalue, "NO") )	// not USERSPECIALDNS, ISP DNS
					sprintf(opcode, "ISP%d", wanidx+1);
			}
		}
		
		if ( 0 == ( result = find_category(database, opcode)))
		{
			if ( 0 == ( result = find_item(database, "DNS1"))) 
			{
				result = get_item_value(database);
				if ( result == 0) 
				{
					if ( 0 == (result = find_item(database, "DNS2"))) 
					{
						result = get_item_value(database);
						if ( result == 0 ) 
						{
							if (strcmp(glvalue, "0.0.0.0"))
							{
								fprintf(wfd, "nameserver %s\n",glvalue);
								dnsno ++;
							}
						} 
					}
				}
			}
		}
	}
	if (dnsno == 0)
	{
		fprintf(wfd, "nameserver %s\n","168.95.1.1");
	}
	fclose(database);
	fclose(wfd);
	return (0);
}


/*
 * update resolv.conf if DHCP client is disabled
 */
#if 0
static int create_resolv(void)
{ 
  FILE *wfd, *database;
  int  i, result;

  if ( NULL == (database = fopen(fconfig, "r+")))
	          return  FAIL_TO_OPEN;

  if ( 0 == (result = find_category(database, "IP")))
  {
      if ( 0 == (result = find_item(database, "DNS")))
	     result = get_item_value(database);
  }
 
  if ( result != 0 )
	  return (result);

  if ((wfd = fopen(resolvconf, "w")) == NULL)
  {
      kd_Log("fail to create %s\n",dhcpconf); 
      return (FAIL_TO_OPEN);
  }

  if (strcmp(glvalue, "0.0.0.0")) 
    fprintf(wfd, "nameserver %s\n",glvalue);

  // a max of 3
  for (i=0; i<2; i++) {
    if ( 0 == (result = find_item(database, "DNS"))) {
      result = get_item_value(database);
      if ( result != 0 )
	if (strcmp(glvalue, "0.0.0.0")) 
	  fprintf(wfd, "nameserver %s\n",glvalue);
    }
  }

  fclose(wfd);
  return (0);
}
#endif


/* dhcp static leases --> Kide 2005/03/29 */
#include <netinet/in.h>
#include <arpa/inet.h>
static int nk_create_dhcp_static(void)
{
	FILE	*wfd, *database;
	long	wfd_curr_pos;
	int		i, result;
	struct {
		struct in_addr	ip;
		unsigned char	chaddr[16];
		unsigned long	expires;
		char			hostname[64];
		unsigned int	is_static;
	} lease;

	// open database and dhcp_static.leases
	if ( NULL == (database = fopen(fconfig, "r+")))
		return (FAIL_TO_OPEN);

	if ((wfd = fopen(DHCPS_STATIC_LEASE, "w")) == NULL)
	{
//		printf("fail to create %s\n", DHCPS_STATIC_LEASE);
		fclose(database);
		return (FAIL_TO_OPEN);
	}

	// search for user list
	if ( (result = find_category(database, "HOST_LIST")) )
	{
		fclose(database);
		fclose(wfd);
		return (result);
	}
	
	for (i=1; ;i++)
	{
		// not found or the end
		if ( find_item(database, "ID") || get_item_value(database) )
			break;

		// keep ID position in database
		wfd_curr_pos = ftell(database);

		// find host
		if ( (result = find_category(database, glvalue)) != 0 )
			continue;

		memset((char *)&lease, 0, sizeof(lease));
		// name
		if ( !find_item(database, "NAME") && !get_item_value(database) )
			sprintf(lease.hostname, glvalue);
		
		// ip
		if ( !find_item(database, "IP") && !get_item_value(database) )
			inet_aton(glvalue, &lease.ip);

		// mac
		if ( !find_item(database, "MAC") && !get_item_value(database) )
		{
			int k, mac_tmp[6];
			sscanf(glvalue, "%02X:%02X:%02X:%02X:%02X:%02X", &mac_tmp[0], &mac_tmp[1],
															 &mac_tmp[2], &mac_tmp[3],
															 &mac_tmp[4], &mac_tmp[5]);
			for (k=0; k < 6; k++)
				lease.chaddr[k] = (unsigned char)mac_tmp[k];
		}

		// static lease
		lease.is_static = 1;

		// lease time
		lease.expires = htonl(lease.expires);

		// write to file
		fwrite(lease.chaddr, 16, 1, wfd);
		fwrite(&(lease.ip.s_addr), 4, 1, wfd);
		fwrite(&lease.expires, 4, 1, wfd);
		fwrite(lease.hostname, 64, 1, wfd);
		fwrite(&lease.is_static, 4, 1, wfd);

		// look for next ID
		fseek(database, wfd_curr_pos, SEEK_SET);
	} // for loop

	fclose(wfd);
	fclose(database);
	return 0;
} /* nk_create_dhcp_static() */
// <-- Kide 2005/03/29


static int create_dhcp_config(void)
{
	FILE			*wfd, *database;
	struct in_addr	gw, mask, brcast;

	char lanIp[40];
	int  wanidx, index, result, dnsOpt, isp_gwno;
	FILE *rfd;
	char opcode[60];
	int dnsidx, dnsNum;
	char dnslist[4][20];
#ifdef CONFIG_NK_DYNAMIC_PORT_NUM
	unsigned int DYNAMIC_NUM_WAN;
#endif

#ifdef CONFIG_NK_DYNAMIC_PORT_NUM
	DYNAMIC_NUM_WAN = Get_Num_Wan_kdsys();
#endif

	// open sysconfig file
	if ( NULL == (database = fopen(fconfig, "r+")))
		return  FAIL_TO_OPEN;

	// open udhcpd.conf file
	if ((wfd = fopen(DHCPD_CONF, "w")) == NULL)
	{
//		printf("fail to create %s\n", DHCPD_CONF);
		fclose(database);
		return (FAIL_TO_OPEN);
	}

#if 0	// TT	
	// not found DHCP category, return directly
	if ( find_category(database, "DHCP") || find_item(database, "SERVER") )
	{
		fclose(wfd);
		fclose(database);
		return (FAIL_TO_OPEN);
	}
#endif

	isp_gwno = 0;
#ifdef CONFIG_NK_DYNAMIC_PORT_NUM
	for (wanidx = 0; wanidx < DYNAMIC_NUM_WAN; wanidx++)
#else
	for (wanidx = 0; wanidx < CONFIG_NK_NUM_WAN; wanidx++)
#endif
	{	
		sprintf(opcode, "ISP%d", wanidx+1);
		if ( !find_category(database, opcode) && !find_item(database, "GATEWAY") )
		{
			get_item_value(database);
			if ( strcmp(glvalue, "0.0.0.0") )
			{
				isp_gwno ++;
			}
		}
	}

	// gateway
	if ( !find_category(database, "SYSTEM") && !find_item(database, "LAN") && !get_item_value(database) )
	{
		strcpy(lanIp, glvalue);
		inet_aton(glvalue, &gw);
	}

	// mask
	if ( !find_item(database, "LANMASK") && !get_item_value(database) )
		inet_aton(glvalue, &mask);

	brcast.s_addr = (gw.s_addr & mask.s_addr) | htonl(255);
	
	// start to build udhcpd.conf
	fprintf(wfd,"# %s\n", DHCPD_CONF);
	fprintf(wfd,"#\n");
	if ( !find_item(database, "LANINTERFACE") && !get_item_value(database) )
		fprintf(wfd, "interface         %s\n", glvalue);

	// ip range
	if ( !find_category(database, "DHCP") && !find_item(database, "RANGE") )
	{
	     result = get_item_value(database);
		 if ( result == 0 )
		 {
		   if (strcmp(glvalue, "0.0.0.0"))
		   {
		     index = 0;
		     while ( glvalue[index] != ':' && index < 30 ) 	       
		       index ++;
		     glvalue[index] = '\0';
		     fprintf(wfd, "start             %s\n", glvalue);
		     fprintf(wfd, "end               %s\n", &(glvalue[index+1]));
		   } 
		 }
#if 0	//TT 	
		char	buf[32];
		struct in_addr	start, end;

		// save back to sysconfig
		start.s_addr = (gw.s_addr & mask.s_addr) | htonl(100);
		end.s_addr =	(gw.s_addr & mask.s_addr) | htonl(109);
		brcast.s_addr = (gw.s_addr & mask.s_addr) | htonl(255);
		sprintf(buf, "RANGE=%s:%s\n", inet_ntoa(start), inet_ntoa(end));
		insert_item(database, buf, 0);
		// store to udhcpd.conf
		fprintf(wfd, "start             %s\n", inet_ntoa(start));
		fprintf(wfd, "end               %s\n", inet_ntoa(end));
#endif	
	}

	// timeout
	if ( 0 == (result = find_category(database, "DHCP")))
	{
		if ( 0 == ( result = find_item(database, "TIMEOUT")))
			result = get_item_value(database);
	}

	if ( result != 0 )
	{
		fclose(wfd);
		fclose(database);
		return (result);
	}

	if (strcmp(glvalue, "0"))
	{
		if (isp_gwno == 0)
			fprintf(wfd, "opt     lease     300\n");
		else
			fprintf(wfd, "opt     lease     %s\n", glvalue);
	}
	else
		fprintf(wfd, "opt     lease     604800\n");

	// mtu
	if ( 0 == (result = find_category(database, "WAN1")))
	{
		if ( 0 == ( result = find_item(database, "MTU")))
			result = get_item_value(database);
	}

	if ( result != 0 )
	{
		fclose(wfd);
		fclose(database);
		return (result);
	}
	fprintf(wfd, "opt     mtu     %s\n", glvalue);
	
	// subnet
	fprintf(wfd, "option subnet     %s\n", inet_ntoa(mask));
	// broadcast
	fprintf(wfd, "option  broadcast %s\n", inet_ntoa(brcast));
	// router
	fprintf(wfd, "option  router    %s\n", inet_ntoa(gw));

	//WINS
	if ( 0 == (result = find_category(database, "DHCP")))
	{
		if ( 0 == ( result = find_item(database, "WINSERVER")))
			result = get_item_value(database);
	}

	if ( result == 0 )
	{
		if (strcmp(glvalue, "0.0.0.0"))
			fprintf(wfd, "opt     wins     %s\n", glvalue);
	}
	
	// DNS from WEB page has higher priority
	// if nothing from web entries, use resolv.conf
	dnsOpt = 0;
//	if ( 0 == ( result = find_category(database, "DHCP")))
	{
		if ( !find_item(database, "DNS1") )
		{
			result = get_item_value(database);
			if ( result == 0)
			{
				if (strcmp(glvalue, "0.0.0.0"))
				{
					if (!dnsOpt)
					{
						fprintf(wfd, "option  dns       %s ",glvalue);
						dnsOpt = 1;
					}
					else
						fprintf(wfd, "%s ",glvalue);

					if ( 0 == (result = find_item(database, "DNS2")))
						result = get_item_value(database);
					if ( result == 0 )
					{
						if (strcmp(glvalue, "0.0.0.0"))
							fprintf(wfd,"%s ",glvalue);
					}
				}
			}
		}
	}

	if (dnsOpt != 0)
		goto dnsok;
#if 0	
// do by resolv.conf, fix : WAN1 static IP & WAN2 DHCP, then only WAN1 DNS pass to DHCP client
	for (wanidx = 0; wanidx < CONFIG_NK_NUM_WAN; wanidx++)
	{	
	sprintf(opcode, "WAN%d", wanidx+1);
	if ( 0 == ( result = find_category(database, opcode)))
	{
		if ( !find_item(database, "USERSPECIALDNS") &&  !get_item_value(database) )
		{
			if ( !strcmp(glvalue, "NO") )
//				goto RESOLV;
				continue;
		}

		if ( !find_item(database, "DNS1") )
		{
			result = get_item_value(database);
			if ( result == 0)
			{
				if (strcmp(glvalue, "0.0.0.0"))
				{
					if (!dnsOpt)
					{
						fprintf(wfd, "option  dns       %s ",glvalue);
						dnsOpt = 1;
					}
					else
						fprintf(wfd, "%s ",glvalue);

					if ( 0 == (result = find_item(database, "DNS2")))
						result = get_item_value(database);
					if ( result == 0 )
					{
						if (strcmp(glvalue, "0.0.0.0"))
							fprintf(wfd,"%s ",glvalue);
					}
				}
			}
		}
	}
	}
RESOLV:
#endif
	
	/* Kide 2005/04/19: set dns to our lan ip, when wan is down */
	if (dnsOpt == 0)
	{
//		if ( !find_category(database, "ISP1") && !find_item(database, "WAN") )
//		{
//			get_item_value(database);
			// wan is down
//			if ( !strcmp(glvalue, "0.0.0.0") )
			if (isp_gwno == 0)
			{
				if ( !find_category(database, "SYSTEM") && !find_item(database, "LAN") )
				{
					get_item_value(database);
					fprintf(wfd, "option  dns       %s ",glvalue);
					dnsOpt = 1;
				}
			}
//		}
	}
	// <--
	if (dnsOpt == 0)
	{
		if ((rfd = fopen(RESOLV_CONF, "r")) != NULL)
		{
// 2007/06/25 jane: 4 nameservers at most and do not take the same one
#if 0
			while (fscanf(rfd, "%s", lanIp) != EOF)
			{
				if (!strcmp(lanIp, "nameserver"))
				{
					if (!dnsOpt)
					{
						fprintf(wfd, "option  dns       ");
						dnsOpt = 1;
					}
					fscanf(rfd, "%s", lanIp);
					fprintf(wfd, "%s ",lanIp);
				}
 			}
#else
		dnsNum = 0;
		memset(dnslist, 0, sizeof(dnslist));
		for (dnsidx = 0; dnsidx < 3; dnsidx++) {
#ifdef CONFIG_NK_DYNAMIC_PORT_NUM
		for (wanidx = 0; wanidx < DYNAMIC_NUM_WAN && dnsNum < 4; wanidx++) {
#else
		for (wanidx = 0; wanidx < CONFIG_NK_NUM_WAN && dnsNum < 4; wanidx++) {
#endif
			sprintf(opcode, "ISP%d", wanidx+1);			
			if (!find_category(database, opcode) && !find_item(database, "GATEWAY")) {
				result = get_item_value(database);
				if (result !=0 || !strcmp(glvalue, "0.0.0.0"))
					continue;
			}

			sprintf(opcode, "WAN%d", wanidx+1);
			if (0 == (result = find_category(database, opcode))) {
				if (!find_item(database, "USERSPECIALDNS") &&  !get_item_value(database)) {
					if (!strcmp(glvalue, "NO"))	// not USERSPECIALDNS, ISP DNS
						sprintf(opcode, "ISP%d", wanidx+1);
				}
			}

			if (0 == (result = find_category(database, opcode))) {
				sprintf(opcode, "DNS%d", dnsidx+1);
				if ( !find_item(database, opcode) && !get_item_value(database)) {
					//result = get_item_value(database);
					if ( /*result == 0 &&*/ strcmp(glvalue, "0.0.0.0") ) {
						int i;
						for (i=0; i<4; i++)
							if (dnslist[i][0]!=NULL && !strcmp(dnslist[i], glvalue)) break; 
						if (i!=4) continue;
						for (i=0; i<4; i++)
							if (dnslist[i][0] == 0) break;
						if (dnsNum == 0) fprintf(wfd, "option  dns       ");
						sprintf(dnslist[i], glvalue);
						fprintf(wfd, "%s ", glvalue);
						dnsNum++;
					}
				}
			}
		}
		}
		fprintf(wfd,"\n");
#endif
			fclose(rfd);
		}
	}

dnsok:
	if (dnsOpt)
		fprintf(wfd,"\n");
	dnsOpt = 0;

	// should get from HOST
	if ( 0 == ( result = find_category(database, "SYSTEM")))
	{
		if ( 0 == ( result = find_item(database, "DOMAINNAME")))
		{
			result = get_item_value(database);
			if ( result == 0 )
			{
				if (strlen(glvalue))
					fprintf(wfd, "option  domain    %s\n", glvalue);
			}
		}
	}

	// update udhcpd.leases in seconds
	fprintf(wfd, "auto_time         900\n");
	fclose(wfd);
	fclose(database);

	// dhcp static leases --> Kide 2005/03/29
//	nk_create_dhcp_static();
	// <--
	return (0);
}
#define PPTPD_CHAP			"/etc/ppp/chap-secrets"
#define PPTPD_PAP			"/etc/ppp/pap-secrets"
#define PPTPD_CONF			"/etc/pptpd.conf"
#define PPTPD_OPTION			"/etc/ppp/options.pptpd"
static int create_pptp_config(void)
{
	FILE			*wfd, *database;
	long	wfd_curr_pos;
	int		i, result;
	int		ip[4], sip[4];
	/*purpose     : 0013068    author : Jason.Huang date : 2010-07-29*/
	/*description : Fix PPTP login fail issue	                 */
	char username[64],passwd[64],*p,*pp;
	char start_ip_str[16];
	
	// open sysconfig file
	if ( NULL == (database = fopen(fconfig, "r+")))
		return  FAIL_TO_OPEN;
	// open udhcpd.conf file
	if ((wfd = fopen(PPTPD_CHAP, "w")) == NULL)
	{
		printf("fail to create %s\n", DHCPD_CONF);
		fclose(database);
		return (FAIL_TO_OPEN);
	}

	/*
	search for user list
	if ( (result = find_category(database, "PPTP_SERVER")) )
	{
		fclose(database);
		fclose(wfd);
		return (result);
	}
	*/

	/*purpose     : 0012882 author : Jason.Huang date : 2010-07-23*/
	/*description : support more special character              */

	FILE *getstatus;
	int idx;
	char spword[128],buf[128];
	getstatus = fopen("/tmp/splitDB/PPTP_SERVER", "r");
	if(getstatus == NULL)
		return NULL;
	for(idx=0;fgets(spword,128,getstatus)!=NULL;idx++)
	{
		if(strncmp(spword,"ID=",3))
		{
			continue;
		}
		sprintf(buf,spword+3);
		
		/* User Name */
		pp = buf;
        p =  strchr(pp, '=');
        *p = '\0';
        pp = p + 2;
        p =  strstr(pp, "\"&");
        *p = '\0';
        strcpy(username, pp);
		
			/* Password */
        pp = p + 2;
        p =  strchr(pp, '=');
        *p = '\0';
        pp = p + 2;
        p =  strstr(pp, "\"\"");
        *p = '\0';
        strcpy(passwd, pp);
		
		if(strlen(username))
		{
			fprintf(wfd, "\"%s\" * \"%s\" *\n", username,passwd);
		}
	}
	fclose(wfd);
	fclose(getstatus);
	
#if 0
	for (i=1; ; i++)
	{
		// not found or the end
		if ( find_item(database, "ID")  )
			break;
		if (get_item_value(database) )
			break;

		// keep ID position in database
		wfd_curr_pos = ftell(database);

		// find host
//		if ( (result = find_category(database, glvalue)) != 0 )
//			continue;
        /* User Name */
        pp = glvalue;
        p =  strchr(pp, '=');
        *p = '\0';
        pp = p + 2;
        p =  strstr(pp, "\"&");
        *p = '\0';
        strcpy(username, pp);

	/* Password */
        pp = p + 2;
        p =  strchr(pp, '=');
        *p = '\0';
        pp = p + 2;
        p =  strstr(pp, "\"\"");
        *p = '\0';
        strcpy(passwd, pp);

		if(strlen(username))
			fprintf(wfd, "\"%s\" * \"%s\" *\n", username,passwd);
		// look for next ID
		fseek(database, wfd_curr_pos, SEEK_SET);
	} // for loop
	fclose(wfd);
	fclose(database);
#endif
	
	
	// open sysconfig file
	if ( NULL == (database = fopen(fconfig, "r+")))
		return  FAIL_TO_OPEN;
	// open udhcpd.conf file
	if ((wfd = fopen(PPTPD_CONF, "w")) == NULL)
	{
//		printf("fail to create %s\n", DHCPD_CONF);
		fclose(database);
		return (FAIL_TO_OPEN);
	}

	if(1)
	{
		fprintf(wfd, "ppp /sbin/pppd\n");
		fprintf(wfd, "option /etc/ppp/options.pptpd\n");
//		fprintf(wfd, "logwtmp\n");
	}
	if ( (result = find_category(database, "SYSTEM")) )
	{
		fclose(database);
		fclose(wfd);
		return (result);
	}
	if ( 0 == ( result = find_item(database, "LAN")))
	{
		result = get_item_value(database);
		if ( result == 0 )
		{
			kk_printf("[%s]\n", glvalue);
			if (strlen(glvalue))
				fprintf(wfd, "localip %s\n", glvalue);
		}
	}
		// search for user list
	if ( (result = find_category(database, "PPTP_SERVER")) )
	{
		fclose(database);
		fclose(wfd);
		return (result);
	}
		if ( 0 == ( result = find_item(database, "START_IP")))
	{
		result = get_item_value(database);
		if ( result == 0 )
		{
			kk_printf("glvalue\n");
			if (strlen(glvalue))
				fprintf(wfd, "remoteip %s", glvalue);
		}
	}
	strcpy(start_ip_str, glvalue);
	sscanf(glvalue,"%d.%d.%d.%d",&sip[0],&sip[1],&sip[2],&sip[3]);
	if ( 0 == ( result = find_item(database, "END_IP")))
	{
		result = get_item_value(database);
		if ( result == 0 )
		{
			sscanf(glvalue,"%d.%d.%d.%d",&ip[0],&ip[1],&ip[2],&ip[3]);
		}
	}
	if(sip[2] == ip[2])
		fprintf(wfd, "-%d\n", ip[3]);
	else
	{
		fprintf(wfd, "-254,");
		for( sip[2]++; sip[2] < ip[2]; sip[2]++)
		{
			fprintf(wfd, "%d.%d.%d.1-254,", sip[0], sip[1], sip[2]);
		}
		fprintf(wfd, "%d.%d.%d.1-%d\n", ip[0], ip[1], ip[2], ip[3]);
	}

	fclose(wfd);
	fclose(database);

#if 0
		// open sysconfig file
	if ( NULL == (database = fopen(fconfig, "r+")))
		return  FAIL_TO_OPEN;
	// open udhcpd.conf file
	if ((wfd = fopen(PPTPD_OPTION, "w")) == NULL)
	{
//		printf("fail to create %s\n", DHCPD_CONF);
		fclose(database);
		return (FAIL_TO_OPEN);
	}

	if(1)
	{
		fprintf(wfd, "name pptpd\n");
		fprintf(wfd, "+pap\n");
		fprintf(wfd, "+chap\n");
		fprintf(wfd, "+mschap\n");
		fprintf(wfd, "+mschap-v2\n");
		fprintf(wfd, "mppe required,stateless\n");
//		fprintf(wfd, "logwtmp\n");
	}
	if ( 0 == ( result = find_item(database, "LAN")))
	{
		result = get_item_value(database);
		if ( result == 0 )
		{
			if (strlen(glvalue))
				fprintf(wfd, "ms-dns %s\n", glvalue);
		}
	}
	if(1)
	{
		fprintf(wfd, "proxyarp\n");
#if 1
		fprintf(wfd, "debug\n");
		fprintf(wfd, "dump\n");
#endif
		fprintf(wfd, "lock\n");
		fprintf(wfd, "nobsdcomp\n");
		fprintf(wfd, "nologfd\n");
	}
	fclose(wfd);
	fclose(database);
#endif
	
	if(1)
	{
		char cmdBuf[200];
		sprintf(cmdBuf,"cp -f %s %s",PPTPD_CHAP,PPTPD_PAP);
		system(cmdBuf);
	}
	return (0);
}
static int process_comd(char cmd, char type, char *paras)
{
    int result;

	switch ( cmd )
	{
	   case 'b': // for sysconfig1 (UI backup) file
            result = print_setting(type, paras, 1);
		    break;
	   case 'p':
            result = print_setting(type, paras, 0);
		    break;
       case 'w':
            result = set_configuration(type, paras);
			break;
       case 'd':
		    result = delete_setting(type, paras);
		    break;
	   default :
		    result = INVALID_GROUP;
	}
	return (result);
}

static int print_setting (char type, char *param, int flag)
{
    FILE *database;

    char opchar;
	int  idx, number, result;
	char * cptr;


	if (flag == 1)
	{
		if ( NULL == (database = fopen(nk_fconfig1, "r+")))
			return  FAIL_TO_OPEN;
	}
	else
	{
		if ( NULL == (database = fopen(nk_fconfig, "r+")))
			return  FAIL_TO_OPEN;
	}
    
	result = 0;

	switch ( type )
	{
       case 'A':
			   result = print_config( database, "REMOTE_ADMIN", "STATUS", 1);
			   break;
       case 'B':
			   result = print_config(database, "BLOCK", "PING_FROM_OUTSIDE", 1);
			   break;
	   case 'C':
		       if ( param == NULL )
			   {
				  print_config(database, "DHCP", "CLIENT", 1 );
			      print_config(database, "DHCP", "SERVER", 1 );
				  print_config(database, "DHCP", "MASK", 1 );
                  print_config(database, "DHCP", "RANGE", 1 );
//                  print_config(database, "DHCP", "RANGE", 2 );
                  print_config(database, "DHCP", "DNS", 1 );
                  print_config(database, "DHCP", "DNS", 2 );
			      print_config(database, "DHCP", "GATEWAY", 1 );
			      print_config(database, "DHCP", "TIMEOUT", 1 );
				  print_config(database, "DHCP", "WINSERVER", 1 );
				  print_config(database, "DHCP", "DOMAINNAME", 1 );
				  break;
			   }

			   /*be careful to use this and a and d option can not mixed
			     with other options you can only do a
				 -p -C a=n1 or -p -C d=n2 or -p -C scmgtwn
			   */
			   if ( strlen(param) >= 2 )
			   {
			     if ( param[2] == 'N' || param[2] == 'n' )
				 {
			       opchar = param[0];
		           idx = 3;
		           while ( param[idx] != '\0')
				   {
			         param[idx - 3] = param[idx];
			         idx++;
				   }
		           param[idx - 3] = '\0';
		           number = atoi(param);
			       if ( opchar == 'A' || opchar == 'a') 
				   {
				     result = print_config(database, "DHCP", "RANGE", number);
				   }    
				   else if ( opchar == 'D' || opchar == 'd') 
				   {
				     result = print_config(database, "DHCP", "DNS", number);
				   }
				   else 
			         result = INVALID_COMD;
				   break;
				 }
			   }

			   idx = 0;
			   while ((opchar = param[idx++]) != '\0' )
			   {
					if ( opchar == 'C' || opchar == 'c')
					  result = print_config(database, "DHCP", "CLIENT", 1 );
					else if ( opchar == 'A' || opchar == 'a')
					{
					  result = print_config(database, "DHCP", "RANGE", 1 );
//                      result = print_config(database, "DHCP", "RANGE", 2 );
					}
					else if ( opchar == 'S' || opchar == 's')
					  result = print_config(database, "DHCP", "SERVER", 1 );
					else if ( opchar == 'D' || opchar == 'd')
					{
					   result = print_config(database, "DHCP", "DNS", 1 );
                       result = print_config(database, "DHCP", "DNS", 2 );
					}
					else if ( opchar == 'G' || opchar == 'g')
					   result = print_config(database, "DHCP", "GATEWAY", 1 );
					else if ( opchar == 'T' || opchar == 't')
					   result = print_config(database, "DHCP", "TIMEOUT", 1 );
					else if ( opchar == 'W' || opchar == 'w')
					   result = print_config(database, "DHCP", "WINSERVER", 1 );
					else if ( opchar == 'N' || opchar == 'n')
					   result = print_config(database, "DHCP", "DOMAINNAME", 1 );
					else if ( opchar == 'M' || opchar == 'm')
					   result = print_config(database, "DHCP", "MASK", 1 );
					else
			   		   result = INVALID_COMD;
			   }
			   break;
       case 'D':
			   result =print_config(database, "HOST", "DOMAINNAME", 1);
			   break;
	   case 'E':
		       if ( param == NULL )
			   {
					result = print_config( database, "PPPOE", "STATUS", 1 );
					print_config( database, "PPPOE", "USERNAME", 1 );
					print_config( database, "PPPOE", "PASSWORD", 1 );
					print_config( database, "PPPOE", "CONNECTONDEM", 1);
					print_config( database, "PPPOE", "KEEPALIVE", 1 );
					print_config( database, "PPPOE", "MAXIDLE", 1 );
					break;
			   }
			   idx = 0;
			   while ((opchar = param[idx++]) != '\0')
			   {
				 if ( opchar == 's' || opchar == 'S')
					result = print_config( database, "PPPOE", "STATUS", 1 );
				 else if ( opchar == 'u' || opchar == 'U')
                    result = print_config( database, "PPPOE", "USERNAME", 1 );
				 else if ( opchar == 'd' || opchar == 'D')
					result = print_config( database, "PPPOE", "CONNECTONDEM", 1);
				 else  if ( opchar == 'p' || opchar == 'P' )
					result = print_config( database, "PPPOE", "PASSWORD", 1 );
				 else if ( opchar == 'A' || opchar == 'a' )
                    result = print_config( database, "PPPOE", "KEEPALIVE", 1 );
				 else if ( opchar == 'I' || opchar == 'i' )
                    result = print_config( database, "PPPOE", "MAXIDLE", 1 );
				 else
			   	    result = INVALID_COMD;
			   }
		       break;
        case 'F':
		       if ( param == NULL )
			   {
                  idx = 1;
				  while ( 0 == ( result = print_config(database, "IP_FILTER", "SOURCE", idx++)));
				  idx = 1;
				  while ( 0 == ( result = print_config(database, "IP_FILTER", "DESTINATION", idx++)));
				  break;
			   }
		       idx = 2;
			   opchar = param[0];

			   if ( strlen (param) >= 2 )
			   {
			     if ( param[2] == 'N' || param[2] == 'n' )
				 {
		            idx = 3;
		            while ( param[idx] != '\0')
					{
			           param[idx - 3] = param[idx];
			           idx++;
					}
		             param[idx - 3] = '\0';
		             number = atoi(param);
				 }
			     else
			      number = 0;
			   }
			   else
				  number = 0;

			   if ( opchar == 'S' || opchar == 's')
				  result = print_config(database, "IP_FILTER", "SOURCE", number);
			   else if ( opchar == 'd' || opchar == 'D')
				  result = print_config(database, "IP_FILTER", "DESTINATION", number);
			   else 
			      result = INVALID_PARAMETER;
			   break;
	    case 'G':
		       result = print_config(database, "IP", "GATEWAY", 1);
			   break;
        case 'H':
		       result = print_config(database, "HOST", "HOSTNAME", 1);
			   break;
 	    case 'I':
		       result = print_config(database, "PPPOE", "USERNAME", 1 );
			   break;
        case 'J':
		       result = print_config(database, "IP", "MTU", 1 );
			   break;
        case 'K':
		       result = print_config(database, "PPTP", "STATUS", 1 );
			   break;
	    case 'L':
			   result = print_config(database, "IP", "LAN", 1 );
			   break;
        case 'M':
		       if ( param == NULL )
		       {
                           idx = 1;
			     while ( 0 == ( result = print_config(database, "MAC_FILTER", "SOURCE", idx++)));
			     break;
			   }
			   idx = 2;
               opchar = param[0];

			   if ( strlen(param) >= 2 )
			   {
                 if ( param[2] == 'N' || param[2] == 'n' )
                 {
                   idx = 3;
                   while ( param[idx] != '\0')
                   {
                      param[idx - 3] = param[idx];
                      idx++;
                   }
                   param[idx - 3] = '\0';
                   number = atoi(param);
                  }
                  else
                   number = 0;
			   }
			   else
				   number = 0;

			 result = print_config(database, "MAC_FILTER", "SOURCE", number);
			 break;
       case 'N':
			   result = print_config(database, "REMOTE_UPGRADE", "STATUS", 1);
			   break;
       case 'O':
               result = print_config(database, "IP", "LANMASK", 1);
               break;

       case 'P':
		       print_config(database, "PPPOE", "PASSWORD", 1);
			   break;
       case 'Q':
               result = print_config(database, "IP", "WANMASK", 1);
               break;

       case 'R':
		       if ( param == NULL )
			   {
                  idx = 1;
				  while ( 0 == ( result = print_config(database, "REDIRECT", "RD", idx++)));
				  break;
			   }
		       idx = 1;
			   while ( param[idx ] != '\0')
			   {
				   param[idx - 1] = param[idx];
			       idx++;
			   }
			   param[idx - 1] = '\0';
  			   idx = atoi(param);
			   if ( idx < 1)
				   idx = 1;
	           result = print_config(database, "REDIRECT", "RD", idx );
			   break;
	    case 'S':
	          // this is a special case with -S n=1
	          if ( param == NULL ) 
			  {
                 for ( idx = 1; idx <= 2; idx++)
		           result = print_config(database, "IP", "DNS", idx);
		         break;
			  }
	          cptr = param;
	          while ((*cptr != '=') && (*cptr != '\0'))
		         cptr++;
	          if (*cptr == '=')
		         cptr++;
              idx = atoi(cptr);
	          if ( idx < 1 )
		         idx = 1;
	          result = print_config(database, "IP", "DNS", idx );
	          break;
        case 'T':
		       if ( param == NULL )
			   {
                  idx = 1;
				  while ( 0 == ( result = print_config(database, "PORT_FILTER", "SOURCE", idx++)));
				  idx = 1;
				  while ( 0 == ( result = print_config(database, "PORT_FILTER", "DESTINATION", idx++)));
				  break;
			   }
		       idx = 3;
			   opchar = param[0];

			   if ( strlen (param) < 3 )
			   {
				   fclose(database);
				   return (INVALID_PARAMETER);
			   }

			   while ( param[idx ] != '\0')
			   {
				   param[idx - 3] = param[idx];
			       idx++;
			   }
			   param[idx - 3] = '\0';
  			   idx = atoi(param);

			   if ( opchar == 'S' || opchar == 's')
				  result = print_config(database, "PORT_FILTER", "SOURCE", idx);
			   else if ( opchar == 'd' || opchar == 'D')
				  result = print_config(database, "PORT_FILTER", "DESTINATION", idx);
			   else 
			      result = INVALID_PARAMETER;
			   break;
	  case 'U':
		       result = print_config(database, "MULTICASTING", "STATUS", 1 );
			   break;
	  case 'V':
	           result = printSwitchHandler(param, database);
	           break;
      case 'W':
		       result = print_config(database, "IP", "WAN", 1);
			   break;
	  case 'X':
		       result = print_config(database, "BLOCK", "MALFORVED_PACKET", 1);
               result = print_config(database, "BLOCK", "SPOODED_PACKET", 1);
			   break;
      case 'Y':
		       result = print_config(database, "HCLONE", "MAC", 1 );
			   break;
      case 'Z':
		       if ( param == NULL )
			   {
	              idx = 1;
				  while ( 0 == ( result = print_config(database, "DMZ", "ZONE", idx++)));
				  break;
			   }
		       idx = 1;
			   opchar = param[0];
			   while ( param[idx ] != '\0')
			   {
				   param[idx - 1] = param[idx];
			       idx++;
			   }
			   param[idx - 1] = '\0';
  			   idx = atoi(param);
			   if ( idx < 1 )
				   idx = 1;
		       result = print_config(database, "DMZ", "ZONE", idx );
			   break;
	  default: 
		      result = INVALID_COMD;
	}
	fclose(database);
	return (result);
}

static int set_configuration (char type, char *param) 
{
	int result, idx, id;
	char opchar, temp[128], tmp[128];

	result = 0;

	if ( param == NULL || strlen(param) < 2)
	     return(INVALID_COMD);

	if ( param[1] == '=' )
	{
	   idx = 2;
	   opchar = toupper(param[0]);
	   while ( param[idx] != '\0')
	   {
		   tmp[idx - 2] = param[idx];
		   idx++;
	   }
	   tmp[idx - 2] = '\0';
	}
	else
    {
	   strcpy (tmp, param);
           opchar = '\0';
    }


    if ( tmp == NULL )
		return (INVALID_COMD);

	switch ( type )
	{
	   case 'A':
             result = set_config("REMOTE_ADMIN", "STATUS", tmp, 1, fconfig);
			 break;	   
	   case 'B':
			 result = set_config("BLOCK", "PING_FROM_OUTSIDE", tmp, 1, fconfig);
			 break;
	   case 'C':
			 if ( opchar == 'S')
                result = set_config("DHCP", "SERVER", tmp, 1, fconfig);
			 else if ( opchar == 'C' )
                result = set_config("DHCP", "CLIENT", tmp, 1, fconfig);
			 else if ( opchar == 'A' )
                result = add_config("DHCP", "RANGE", tmp, fconfig);
			 else if ( opchar == 'D')
                result = add_config("DHCP", "DNS", tmp, fconfig);
			 else if ( opchar == 'G' )
				result = set_config("DHCP", "GATEWAY", tmp, 1, fconfig);
			 else if ( opchar == 'T' )
                result = set_config("DHCP", "TIMEOUT", tmp, 1, fconfig);
			 else if ( opchar == 'W' )
                result = set_config("DHCP", "WINSERVER", tmp, 1, fconfig);
			 else if ( opchar == 'N' )
                result = set_config("DHCP", "DOMAINNAME", tmp, 1, fconfig);
			 else if ( opchar == 'M' )
                result = set_config("DHCP", "MASK", tmp, 1, fconfig);
			 else 
				result = INVALID_PARAMETER;
			 break;
       case 'D':
	         result = set_config("HOST", "DOMAINNAME", tmp, 1, fconfig);
		     break;
	   case 'E':
			 if (opchar == 'S')
                  result = set_config("PPPOE", "STATUS", tmp, 1, fconfig);
			 else if ( opchar == 'U' )
                  result = set_config("PPPOE", "USERNAME", tmp, 1, fconfig);
			 else if ( opchar == 'D' )
                  result = set_config("PPPOE", "CONNECTONDEM", tmp, 1, fconfig);
			 else if ( opchar == 'P' )
                  result = set_config("PPPOE", "PASSWORD", tmp, 1, fconfig);
			 else if ( opchar == 'A' )
				  result = set_config("PPPOE", "KEEPALIVE", tmp, 1, fconfig);
			 else if ( opchar == 'I' )
                  result = set_config("PPPOE", "MAXIDLE", tmp, 1, fconfig);
			 else
				  result = INVALID_PARAMETER;
			 break;
       case 'F':
			 if ( opchar == 'S' )
                  result = add_config("IP_FILTER", "SOURCE", tmp, fconfig);
			 else if ( opchar == 'D' )
                  result = add_config("IP_FILTER", "DESTINATION", tmp, fconfig);
	         break;
	   case 'G':
	         result = set_config("IP", "GATEWAY", tmp, 1, fconfig);
		     break;
       case 'H':
	         result = set_config("HOST", "HOSTNAME", tmp, 1, fconfig);
			 break;
	   case 'I':
	         result = set_config("PPPOE", "USERNAME", tmp, 1, fconfig);
		     break;
       case 'J':
	         result = set_config("IP", "MTU", tmp, 1, fconfig);
			 break;
       case 'K':
	         result = set_config("PPTP", "STATUS", tmp, 1, fconfig);
		     break;
 	   case 'L':
	         strcpy(temp, tmp);
		     idx = 0;
			 while (temp[idx] != ':' && temp[idx] != '/' && temp[idx] != '\0')
				   idx++;
             temp[idx] = '\0'; 
			 id = idx + 1;
			 if ( tmp[idx] == ':' || tmp [idx] == '/')
			 {
				idx++;
				while ( tmp[idx] != '\0')
				{
					tmp[idx - id ] = tmp[idx];
					idx++;
				}
				tmp[idx-id] = '\0';
				result = set_config("IP", "LANMASK", tmp, 1 , fconfig);
			 }
			 result = set_config("IP", "LAN", temp, 1, fconfig);   
		     break;
       case 'M':
		     idx = 0;
			 while ( tmp[idx] != '\0')
			 { 
			    if ( tmp[idx] == ':' )
					 tmp[idx] = ' ';
				else if ( tmp[idx] == '.' )
					 tmp[idx] = ':';
				idx++;
			 }

		     if ( opchar == 'S')
                 result = add_config("MAC_FILTER", "SOURCE", tmp, fconfig);
		     else if ( opchar == 'D' )
                 result = add_config("MAC_FILTER", "DESTINATION", tmp, fconfig);
	         break;
       case 'N':
	         result = set_config("REMOTE_UPGRADE", "STATUS", tmp, 1, fconfig);
		     break;
	   case 'P':
	  	     result = set_config("PPPOE", "PASSWORD", tmp, 1, fconfig);
			 break;
       case 'R':
             result = add_config("REDIRECT", "RD", tmp, fconfig);
			 break;
	   case 'S':
	         result = add_config("IP", "DNS", tmp, fconfig);
		     break;
       case 'T':
	 	     if ( opchar == 'S' )
                  result = add_config("PORT_FILTER", "SOURCE", tmp, fconfig);
			 else if ( opchar == 'D' )
                  result = add_config("PORT_FILTER", "DESTINATION", tmp, fconfig);
			 break;
      case 'U':
			 result = set_config("MULTICASTING", "STATUS", tmp, 1, fconfig);
			 break;

	  case 'V':
	         return setSwitchHandler(param);

      case 'W':
	    strcpy(temp, tmp);
	    idx = 0;
	    while (temp[idx] != ':' && temp[idx] != '/' && temp[idx] != '\0')
                idx++;
            temp[idx] = '\0';
            result = set_config("IP", "WAN", temp, 1, nk_fconfig);
            id = idx + 1;
            if ( tmp[idx] == ':' || tmp [idx] == '/')
            {
               idx++;
               while ( tmp[idx] != '\0')
               {
                 tmp[idx - id ] = tmp[idx];
                 idx++;
               }
	       tmp[idx-id] = '\0';
	       result = set_config("IP", "WANMASK", tmp, 1, fconfig);
	    }
	    break;
       case 'Q':
		     result = set_config("IP", "WANMASK", tmp, 1, fconfig);
		     break;
	   case 'X':
   	         result = set_config("BLOCK", "MALFORMED_PACKET", tmp, 1, fconfig);
             result = set_config("BLOCK", "SPOODED_PACKET", tmp, 1, fconfig);
			 break;
       case 'Y':
	         result = set_config("HCLONE", "MAC", tmp, 1, fconfig);
	  	     break;
       case 'Z':
			 result = add_config("DMZ", "ZONE", param, fconfig);
			 break;
	   default: 
			 result = INVALID_COMD;
	}
	return (result);
}

/*remove one item at per function call, otherwise, the result will be wrong*/
static int delete_setting (char type, char *param)
{
	FILE *database, *ftmp;

	int result, idx, number, count;
        char opchar, options[128];

	result = 0;


	if ( param == NULL || strlen(param) < 2)
		 return (INVALID_COMD);

//	printf ("LINE : %d, param %s\n", __LINE__, param);
	if ( param[1] == '=' )
	{
	   idx = 2;
	   opchar = toupper(param[0]);
//	printf ("LINE : %d, opcode %c", __LINE__, opchar);

	   if ( strlen(param) < 3 )
             return (INVALID_COMD);
 
	   if ( param[2] == 'N' || param[2] == 'n' )
	   {
		 idx = 3;
		 while ( param[idx] != '\0')
		 {
			options[idx - 3] = param[idx];
			idx++;
		 }
		 options[idx - 3] = '\0';
		 number = atoi(options);
		 if ( number < 1 )
			 number = 1;
//	printf ("LINE : %d, option %s number %d\n", __LINE__, options, number);
	   }
	   else
	   {
		  while ( param[idx] != '\0')
		  {
		    options[idx - 2] = param[idx];
		    idx++;
		  }
		  options[idx - 2] = '\0';
		  number = 0;
//	printf ("LINE : %d, option %s number %d\n", __LINE__, options, number);
	   }  
	}
    else
	{
       if ( param[0] == 'N' || param[0] == 'n' )
	   {
		 idx = 1;
		 while ( param[idx] != '\0')
		 {
			options[idx - 1] = param[idx];
			idx++;
		 }
		 options[idx - 1] = '\0';
		 number = atoi(options);
		 if ( number < 1 )
			 number = 1;
//	printf ("LINE : %d, option %s number %d\n", __LINE__, options, number);
	   }
	   else
	   {
		  strcpy(options, param);
		  number = 0;
//	printf ("LINE : %d, option %s number %d\n", __LINE__, options, number);
	   }
       opchar = '\0';
    }

	
    if ( NULL == (database = fopen(fconfig, "r+")))
    {
//        kd_Log("failed to open %s for updating!\n", fconfig);
        return  FAIL_TO_OPEN;
    }

    switch ( type )
	{
       case 'C':
		         if ( opchar == 'A')
				 strcpy ( param, "RANGE=" );
			 else if ( opchar == 'D' )
			     strcpy ( param, "DNS=");

	   		 if ( number == 0 )
	  		      strcat(param, options);

			 if ( -1L != move_to_item(database, "DHCP", param, number))
	  		      result = remove_item(database);
			 else
				  result = ITEM_NOT_FOUND;
			 break;
	   case 'F':
			 if ( opchar == 'S' )
				  strcpy(param, "SOURCE=");
			 else
				  strcpy(param, "DESTINATION=");

			 idx = 0;
			 while ( options[idx] != '\n')
			 {
			     if ( options[idx] == ',' )
			          options[idx] = ' ';
			     idx++;
			 }

	   		 if ( number == 0 )
	  		      strcat(param, options);

			 if ( -1L != move_to_item(database, "IP_FILTER", param, number))
	  		        result = remove_item(database);
			 else
				result = ITEM_NOT_FOUND;
			 break;
       case 'M':
	 	     if ( opchar == 'S' )
				  strcpy( param, "SOURCE=");
			 else
				  strcpy( param, "DESTINATION=");

             idx = 0;
             while ( options[idx] != '\n')
             {
                 if ( options[idx] == '.' )
                      options[idx] = ':';
                 else if ( options[idx] ==':' )
                      options[idx] = ' ';
                 idx++;
             }

             if ( number == 0 )
                  strcat( param, options );

			 if ( -1L != move_to_item(database, "MAC_FILTER", param, number))
	  		        result = remove_item(database);
			 else
				 result = ITEM_NOT_FOUND;
			 break;
       case 'R':
	  		 strcpy(param, "RD=");
             idx = 0;
             while ( options[idx] != '\n')
             {
                if ( options[idx] == ',' )
                     options[idx] = ' ';
                     idx++;
             }

             if ( number == 0 )
                  strcat(param, options);

			 if ( -1L != move_to_item(database, "REDIRECT", param, number))
	  		     result = remove_item(database);
			 else
				 result = ITEM_NOT_FOUND;
			 break;
	   case 'S':
		     strcpy(param, "DNS=");
       		 if ( number == 0 )
				strcat(param, options);
			 if ( -1L != move_to_item(database, "IP", param, number))
	  		     result = remove_item(database);
			 else
				 result = ITEM_NOT_FOUND;
			 break;
       case 'T':
			 if ( opchar == 'S' )
				  strcpy( param, "SOURCE=");
			 else
				  strcpy( param, "DESTINATION=");

			 idx = 0;
			 while ( options[idx] != '\n')
			 {
				 if ( options[idx] == ',' )
					  options[idx] = ' ';
				 idx++;
			 }

	   		 if ( number == 0 )
	  		      strcat(param, options);

			 if ( -1L != move_to_item(database, "PORT_FILTER", param, number))
                    result = remove_item(database);
			 else
				 result = ITEM_NOT_FOUND;
			 break;
	   case 'Z':
	  		 strcpy( param, "ZONE=" );
       		 if ( number == 0 )
				strcat( param, options );
			 if ( -1L != move_to_item(database, "DMZ", param, number))
	  		       result =  remove_item(database);
			 else
				 result = ITEM_NOT_FOUND;
			 break;
	   default: 
		     result = INVALID_COMD;
	}
	fclose(database);
	/*update the change*/
#ifdef COPY_FILEOPEN	
    if ( NULL == (database = fopen(fconfig, "w+")))
    { 
        return  FAIL_TO_OPEN;
//		kd_Log("failed to open %s, for update!\n", fconfig);
    }
    
	if ( NULL == (ftmp = fopen(tmpath, "r+")))
	{
//         kd_Log("failed to open %s, for update!\n", tmpath);
		 fclose(database);
         return (FAIL_TO_OPEN);
	}

    while (!feof(ftmp))
    {
        count = fread (options, sizeof(char), 40, ftmp);
        fwrite(options, sizeof(char), count, database);
    }
	fclose(database);
    fclose(ftmp);
#else
	rename(tmpath, fconfig);
#endif
	return (result);
}

static int printSwitchHandler(char *param, FILE *file_handle)
{
  if ((strlen(param) == 1) && (*param == 's'))
    return print_config(file_handle, "SWITCH", "STATUS", 1 );
  return print_config(file_handle, "SWITCH", param, 1 );
}

/*
 * for switch, param is xxx=value
 * where xxx is the item
 */
static int setSwitchHandler(char *param)
{
  char * bptr;

  bptr = strrchr(param, '=');
  if (bptr == NULL)
    return INVALID_COMD;
  *bptr = '\0';
  if ((strlen(param) == 1) && (*param == 's'))
    return set_config("SWITCH", "STATUS", bptr+1, 1, fconfig);
  //kd_Log("parm %d:%s",strlen(param),param);
  return set_config("SWITCH", param, bptr+1, 1, fconfig);
}

int  remove_setting(char *category, char *item, int number)
{
   FILE *database, *ftmp;
   int  result, count;
   char buf[120];

   if ( NULL == (database = fopen(fconfig, "r+")))
   {
//        kd_Log("failed to open %s for updating!\n", fconfig);
        return  FAIL_TO_OPEN;
   }

   if ( -1L != move_to_item(database, category, item, number))
      result = remove_item(database);
   else
   {
      fclose(database);
	  result = ITEM_NOT_FOUND;
	  return (result);
   }
   fclose(database);
   
   /*update the change*/
#ifdef COPY_FILEOPEN	
   if ( NULL == (database = fopen(fconfig, "w+")))
   { 
        return  FAIL_TO_OPEN;
//		kd_Log("failed to open %s, for update!\n", fconfig);
   }
    
   if ( NULL == (ftmp = fopen(tmpath, "r+")))
   {
//         kd_Log("failed to open %s, for update!\n", tmpath);
		 fclose(database);
         return (FAIL_TO_OPEN);
   }

   while (!feof(ftmp))
   {
        count = fread (buf, sizeof(char), 40, ftmp);
        fwrite(buf, sizeof(char), count, database);
   }
   fclose(database);
   fclose(ftmp);
#else
	rename(tmpath, fconfig);
#endif
   return (result);
}

char *get_setting(char *category, char *item, int number)
{
   FILE *database;
   int i, result;

   if ( NULL == (database = fopen(fconfig, "r+")))
   {
//        kd_Log("failed to open %s for updating!\n", fconfig);
        return  NULL;
   }

   if ( 0 == (result= find_category(database, category)))
   {
		/*skip first idx items*/
		for ( i=1; i<number; i++)
		{
		   result = find_item(database, item);
		   if ( 0 != result )
		   {
			   fclose(database);
			   return (NULL);
		   }
		}
		/*get to the target item */
		if ( 0 == (result = find_item(database, item)))
		{
             if ( 0 != get_item_value (database))
			 {
			   fclose(database);
			   return (NULL);
			 }
		}
		else
		{
			fclose(database);
			return (NULL);
		}
		fclose(database);
		return (glvalue);
   }
   fclose(database);
   return (NULL);
}


/* Kide 2005/03/14 */
static int nk_create_ddns_config(int wanidx)
{
	FILE	*wfd = NULL, *database = NULL;
	char	*p_usrname = NULL, *p_passwd = NULL;
	int		result = 0;
//	int		wan_proto = 0;
	char	opcode[60];

	if ( (database = fopen(nk_fconfig, "r+")) == NULL )
		return  FAIL_TO_OPEN;

	// don't have ddns related params in database
	sprintf(opcode, "DDNS%d", wanidx);
	if ( (result = find_category(database, opcode)) != 0 )
		goto Exit;

	// if ddns is disabled, return directly
	if ( (result = find_item(database, "STATUS")) == 0 )
	{
		result = get_item_value(database);
		if (result != 0 || !strcmp(glvalue, "NO") )
			goto Exit;
	}
	else
		goto Exit;

	// open and build ez-ipupdate.conf
	sprintf(opcode, "%s-%d", DDNS_CONF, wanidx);
	if ( (wfd = fopen(opcode, "w")) == NULL )
	{
//		kd_Log("fail to create %s\n",DDNS_CONF); 
//		printf("fail to create /etc/ez-ipupdate.conf\n");
		goto Exit;
	}

	// service
	if ( (result = find_item(database, "SERVICE")) == 0 )
	{
		result = get_item_value(database);
		if (result == 0)
			fprintf(wfd, "service-type=%s\n", glvalue);
		else
			fprintf(wfd, "service-type=dyndns\n");
	}
	else
		goto Exit;

	// username
	if ( (result = find_item(database, "USERNAME")) == 0 )
	{
		result = get_item_value(database);
		p_usrname = strdup(glvalue);
	}
	else
		goto Exit;

	// password
	if ( (result = find_item(database, "PASSWORD")) == 0 )
	{
		result = get_item_value(database);
		p_passwd = strdup(glvalue);
	}
	else
		goto Exit;

	fprintf(wfd, "user=%s:%s\n", p_usrname ? p_usrname : "", p_passwd ? p_passwd : "");

	// hostname
	if ( (result = find_item(database, "HOSTNAME")) == 0 )
	{
		result = get_item_value(database);
		fprintf(wfd, "host=%s\n", glvalue);
	}
	else
		goto Exit;

	sprintf(opcode, "ISP%d", wanidx);		
	if ( !find_category(database, opcode) && !find_item(database, "INTERFACE") && !get_item_value(database) )
		fprintf(wfd, "interface=%s\n", glvalue);

	// interface
//	if (wan_proto == NK_WAN_CONN_PPPOE)
//		fprintf(wfd, "interface=ppp0\n");	
//	else
//		fprintf(wfd, "interface=eth0\n");

	// time interval
	fprintf(wfd, "max-interval=2073600\n");

	// cache file
//	fprintf(wfd, "cache-file=/tmp/ez-ipupdate.cache\n");

	// daemon
	fprintf(wfd, "daemon\n");

Exit:
	// free resource if needed
	if (p_usrname)		free(p_usrname);
	if (p_passwd)		free(p_passwd);
	if (wfd)			fclose(wfd);
	if (database)		fclose(database);
	return (result);
} /* nk_create_ddns_config() */
// <-- Kide

//TT --->
/* this is the entry point for shell program sysconfig */ 
   // static int staticii=0;
int nk_sysconfig(int argc, char argv[3][ARGV_SIZE], char *supBuf)
{
    int   idx, pidx, result, i, j, fd;
    char  category[ARGV_SIZE], comd, paras[ARGV_SIZE], tmparg[ARGV_SIZE],dbname[512],FlagFile[512];
	static struct sigaction oldact[19];
  //  static int staticnn=0;
	struct stat buf;

	// argv[0] is sysconfig or sysconfig1
//	printf ("---- %s, %d", *argv, backup); 
	if (argv[2][0] == '-')
	{
    	return (sysconfig(argc, argv, supBuf)); //--> Kendin old structure
	}
	
	/* Find any options.  argc is at least 1  the argv[0] is the program name*/
    argc--;  
    argv++;
 
	if (argc == 0 || !strcmp(*argv, "-?") || !strcmp(*argv, "-h"))
	{
		usage();
		return 0;
	}
	if (!strcmp(*argv, "-v"))
	{
		version();
		return 0;
	}
		
	for (i = 1; i < 19; i++)
	{	
		sigaction(i, NULL, &oldact[i]);
		signal(i, SIG_IGN);
	}
#if 0	
	//-->
	stat("/tmp/nk_sysconfig", &buf);
	if (buf.st_size == 0)
		kd_Log("**** LINE %d buf.st_size == 0", __LINE__);
	//<--
#endif
	tmparg[0] = '\0';

    reqBuf = supBuf;

    result = 0;

    if ( argc > 2 )
    {
       idx = 1;
       while ( argv[idx] != NULL )
       {
          strcat(tmparg, argv[idx]);
          strcat(tmparg, " ");
          idx++;
       }
    }

    idx = strlen(tmparg);
    //kd_Log("tmparg=[%s] [%d]",tmparg, idx);
    //2007/4/17, david, modify to fix CGI crash issue.
    //if (tmparg[idx-1] == ' ')
    if (idx && tmparg[idx-1] == ' ')
        tmparg[idx - 1] = '\0';
    else if ( NULL != argv[1] )
          strcpy ( tmparg, argv[1] );
    
	comd = argv[0][1];
	
	pidx=0; idx=0;
	while ( tmparg[idx] != ' ' && tmparg[idx] != '\0')
		category[pidx++] = tmparg[idx++];
	category[pidx] = '\0';
    
	//kd_Log ("%d category[%s] %d, %d\n", __LINE__, category, pidx, idx);

//    if ( tmparg[2] == ' ')
//      idx = 3;
//    else
//     idx = 2;

        //when Merge to Flash, stop access all file to wait
#ifdef USE_SPLIT_DB_FLAG
	if(argv[0][1]!='a')
	{
		umask(0);
		fd = open("/tmp/test.txt", O_RDONLY | O_CREAT | O_EXCL, 0666);
		j = 0;
		while ( fd < 0)
		{
				srand(time(0));
				j = 700 + (int) (900.0 * rand()/(RAND_MAX + 1.0));
				for (i = 0; i < j; i ++);

				fd = open (FlagFile, O_RDONLY | O_CREAT | O_EXCL, 0666);
		}            
		i = close(fd);
		i = unlink("/tmp/test.txt");
		
	}
	sprintf(FlagFile,"/tmp/%s.txt",category);
#else
	strcpy(FlagFile,"/tmp/test.txt");
#endif
	if(argv[0][1]!='a')
	{
			umask(0);
			fd = open(FlagFile, O_RDONLY | O_CREAT | O_EXCL, 0666);
		//	printf("%d 1ERROR %d %s %s\n", __LINE__, fd, argv[1], argv[2]);
			j = 0;
			//kd_Log("%d 0ERROR %d %s %s\n", __LINE__, fd, argv[1], argv[2]);
		while ( fd < 0)
		{
				srand(time(0));
				j = 700 + (int) (900.0 * rand()/(RAND_MAX + 1.0));
				for (i = 0; i < j; i ++);
		//		if (j == 34)
		//		kd_Log("**** %d, database conflicted %c %s %s\n", __LINE__, argv[0][1], argv[1], argv[2]);
				
				fd = open (FlagFile, O_RDONLY | O_CREAT | O_EXCL, 0666);
		}            
			//kd_Log("%d 1ERROR %d argc[%d] [%s] [%s]\n", __LINE__, fd, argc, argv[1], argv[2]);
		
	}

	sprintf(dbname,"/tmp/splitDB/%s",category);

	while ( tmparg[idx] != '\0') 
	{
		while ( tmparg[idx] == ' ' )
		{
			idx++;
		}
		pidx = 0;
    
		while (tmparg[idx] != '\0')
			paras[pidx++] = tmparg[idx++];
		paras[pidx] = '\0';
		//kd_Log ("category [%s], paras [%s]\n", category, paras);
		   
		while ( tmparg[idx] == ' ' )
		idx++;
             
		result = nk_process_comd(comd, category, paras, dbname);

		//if (result != 0)
			//kd_Log("%d nk_process_comd %c %s %s %d\n", __LINE__, comd, category, paras, result);
		
		if ( (result == ITEM_NOT_FOUND ) && ((comd == 'p') || (comd == 'b')) )
		{
			if ( reqBuf == NULL )
				printf ( "END");
			else
				sprintf(reqBuf, "END");
		}
		else if ( result == INVALID_GROUP ) 
		{
			if ( reqBuf == NULL )
				printf("Unrecongnized command: %c\n", comd);		     
			else 
				sprintf(reqBuf, "Unrecongnized command: %c\n", comd);
		}
		else if ( result != 0 )
		{
			if ( reqBuf == NULL )
				printf("ERROR Code %d, Catergory %s\n", result, category);
			else
				sprintf(reqBuf, "No More %s", category);
		}
		if(argv[0][1]!='a')
		{
			i = close(fd);
			i = unlink(FlagFile);
		}

		for (i = 1; i < 19; i++)
		{	
			sigaction(i, &oldact[i], NULL);
		}
	//kd_Log("%d 2ERROR %d %s %s\n", __LINE__, fd, argv[1], argv[2]);
#if 0		
	//-->
	stat("/tmp/nk_sysconfig", &buf);
	if (buf.st_size == 0)
	{
		kd_Log("**** LINE %d buf.st_size == 0 %s %s %s", __LINE__, argv[0], argv[1], argv[2]);
	}
	//<--
#endif
		return (result); 
	}
	
	if ( tmparg [idx] == '\0' )
	{
//		printf ("%d category1 %s, paras %s\n", __LINE__, category1, paras);
		result = nk_process_comd(comd, category, NULL, dbname);
		if (result != 0)
			//kd_Log("%d nk_process_comd %c %s %d\n", __LINE__, comd, category, result);
		if ( (result == ITEM_NOT_FOUND ) && ((comd == 'p') || (comd == 'b')) )
		{
			if ( reqBuf == NULL )
				printf ( "END");
			else
				sprintf(reqBuf, "END");
		}
		else if ( result == INVALID_GROUP )
		{
			if ( reqBuf == NULL )
				printf("Unrecongnized command: %c\n", comd);
			else 
				sprintf(reqBuf, "Unrecongnized command: %c\n", comd);
		}
		else if ( result != 0 )
		{
			if ( reqBuf == NULL )
				printf("ERROR Code %d, catergory %s\n", result, category);
			else
				sprintf(reqBuf, "Unrecongnized options: %s\n", category);
		}
	}
	if(argv[0][1]!='a')
	{
		i = close(fd);
		i = unlink(FlagFile);
	}
	for (i = 1; i < 19; i++)
	{	
		sigaction(i, &oldact[i], NULL);
	}
	//kd_Log("%d 3ERROR %d %s %s\n", __LINE__, fd, argv[1], argv[2]);
#if 0	
	//-->
	stat("/tmp/nk_sysconfig", &buf);
	if (buf.st_size == 0)
		kd_Log("**** LINE %d buf.st_size == 0", __LINE__);
	//<--
#endif
	return (result); 
}

#ifdef CONFIG_NK_LICENSE_COUNT_DOWN
int nk_sysconfig_factory(int argc, char argv[3][ARGV_SIZE], char *supBuf)
{
    int   idx, pidx, result, i, j, fd;
    char  category[ARGV_SIZE], comd, paras[ARGV_SIZE], tmparg[ARGV_SIZE],dbname[512],FlagFile[512];
	static struct sigaction oldact[19];
	struct stat buf;
	
	/* Find any options.  argc is at least 1  the argv[0] is the program name*/
    argc--;  
    argv++;
 
	if (argc == 0 || !strcmp(*argv, "-?") || !strcmp(*argv, "-h"))
	{
		usage();
		return 0;
	}
	if (!strcmp(*argv, "-v"))
	{
		version();
		return 0;
	}
		
	for (i = 1; i < 19; i++)
	{	
		sigaction(i, NULL, &oldact[i]);
		signal(i, SIG_IGN);
	}
#if 0	
	stat("/etc/flash/FID.txt", &buf);
	if (buf.st_size == 0)
		kd_Log("**** LINE %d buf.st_size == 0", __LINE__);
#endif
	tmparg[0] = '\0';
    reqBuf = supBuf;
    result = 0;

    if ( argc > 2 )
    {
       idx = 1;
       while ( argv[idx] != NULL )
       {
          strcat(tmparg, argv[idx]);
          strcat(tmparg, " ");
          idx++;
       }
    }

    idx = strlen(tmparg);
    //kd_Log("tmparg=[%s] [%d]",tmparg, idx);
    if (idx && tmparg[idx-1] == ' ')
        tmparg[idx - 1] = '\0';
    else if ( NULL != argv[1] )
          strcpy ( tmparg, argv[1] );
    
	comd = argv[0][1];
	
	pidx=0; idx=0;
	while ( tmparg[idx] != ' ' && tmparg[idx] != '\0')
		category[pidx++] = tmparg[idx++];
	category[pidx] = '\0';
    
	//kd_Log ("%d category[%s] %d, %d\n", __LINE__, category, pidx, idx);

	strcpy(FlagFile,"/etc/flash/test.txt");
	if(argv[0][1]!='a')
	{
		umask(0);
		fd = open(FlagFile, O_RDONLY | O_CREAT | O_EXCL, 0666);
		//printf("%d 1ERROR %d %s %s\n", __LINE__, fd, argv[1], argv[2]);
		j = 0;
		//kd_Log("%d 0ERROR %d %s %s\n", __LINE__, fd, argv[1], argv[2]);
		while ( fd < 0)
		{
			srand(time(0));
			j = 700 + (int) (900.0 * rand()/(RAND_MAX + 1.0));
			for (i = 0; i < j; i ++);
			//if (j == 34)
			//kd_Log("**** %d, database conflicted %c %s %s\n", __LINE__, argv[0][1], argv[1], argv[2]);

			fd = open (FlagFile, O_RDONLY | O_CREAT | O_EXCL, 0666);
		}            
		//kd_Log("%d 1ERROR %d argc[%d] [%s] [%s]\n", __LINE__, fd, argc, argv[1], argv[2]);
	}

	strcpy(dbname,"/etc/flash/FID.txt");

	while ( tmparg[idx] != '\0') 
	{
		while ( tmparg[idx] == ' ' )
		{
			idx++;
		}
		pidx = 0;
    
		while (tmparg[idx] != '\0')
			paras[pidx++] = tmparg[idx++];
		paras[pidx] = '\0';
		//kd_Log ("category [%s], paras [%s]\n", category, paras);
		   
		while ( tmparg[idx] == ' ' )
		idx++;
             
		result = nk_process_comd_factory(comd, category, paras, dbname);

		//if (result != 0)
		//kd_Log("%d nk_process_comd_factory %c %s %s %d\n", __LINE__, comd, category, paras, result);
		
		if ( (result == ITEM_NOT_FOUND ) && ((comd == 'p') || (comd == 'b')) )
		{
			if ( reqBuf == NULL )
				printf ( "END");
			else
				sprintf(reqBuf, "END");
		}
		else if ( result == INVALID_GROUP ) 
		{
			if ( reqBuf == NULL )
				printf("Unrecongnized command: %c\n", comd);		     
			else 
				sprintf(reqBuf, "Unrecongnized command: %c\n", comd);
		}
		else if ( result != 0 )
		{
			if ( reqBuf == NULL )
				printf("ERROR Code %d, Catergory %s\n", result, category);
			else
				sprintf(reqBuf, "No More %s", category);
		}
		if(argv[0][1]!='a')
		{
			i = close(fd);
			i = unlink(FlagFile);
		}

		for (i = 1; i < 19; i++)
		{	
			sigaction(i, &oldact[i], NULL);
		}
	//kd_Log("%d 2ERROR %d %s %s\n", __LINE__, fd, argv[1], argv[2]);
#if 0		
	stat("/etc/flash/FID.txt", &buf);
	if (buf.st_size == 0)
	{
		kd_Log("**** LINE %d buf.st_size == 0 %s %s %s", __LINE__, argv[0], argv[1], argv[2]);
	}
#endif
		return (result); 
	}
	
	if ( tmparg [idx] == '\0' )
	{
		//printf ("%d category1 %s, paras %s\n", __LINE__, category1, paras);
		result = nk_process_comd_factory(comd, category, NULL, dbname);
		if (result != 0)
			//kd_Log("%d nk_process_comd %c %s %d\n", __LINE__, comd, category, result);
		if ( (result == ITEM_NOT_FOUND ) && ((comd == 'p') || (comd == 'b')) )
		{
			if ( reqBuf == NULL )
				printf ( "END");
			else
				sprintf(reqBuf, "END");
		}
		else if ( result == INVALID_GROUP )
		{
			if ( reqBuf == NULL )
				printf("Unrecongnized command: %c\n", comd);
			else 
				sprintf(reqBuf, "Unrecongnized command: %c\n", comd);
		}
		else if ( result != 0 )
		{
			if ( reqBuf == NULL )
				printf("ERROR Code %d, catergory %s\n", result, category);
			else
				sprintf(reqBuf, "Unrecongnized options: %s\n", category);
		}
	}
	if(argv[0][1]!='a')
	{
		i = close(fd);
		i = unlink(FlagFile);
	}
	for (i = 1; i < 19; i++)
	{	
		sigaction(i, &oldact[i], NULL);
	}
	//kd_Log("%d 3ERROR %d %s %s\n", __LINE__, fd, argv[1], argv[2]);
#if 0	
	stat("/etc/flash/FID.txt", &buf);
	if (buf.st_size == 0)
		kd_Log("**** LINE %d buf.st_size == 0", __LINE__);
#endif
	return (result); 
}
#endif

static int nk_process_comd(char cmd, char *type, char *paras, char *dbname)
{
    int result;
kdsys_printf("nk_process_comd start !!");
	//kd_Log ("%d %c\n", __LINE__, cmd);
	switch ( cmd )
	{
	   case 't': // create database for each task 
            result = nk_create_db(type, paras);
		    break;
	   case 'n': // for new a parameter in sysconfig
            result = nk_new_setting(type, paras,dbname);
		    break;
	   case 'b': // for sysconfig1 (UI backup) file
            result = nk_print_setting(type, paras, 1, dbname);
		    break;
	   case 'p':
            result = nk_print_setting(type, paras, 0, dbname);
		    break;
       case 'w':
            result = nk_set_configuration(type, paras, dbname);
			break;
       case 'm': // modify value for specific entry of multi-list like entry of port forwarding
            result = nk_mod_configuration(type, paras, dbname);
			break;
       case 'd':
		    result = nk_delete_setting(type, paras, dbname);
		    break;
	   case 'a':
            result = nk_print_setting(type, paras, 0, dbname);
		    break;
	   default :
		    result = INVALID_GROUP;
	}
kdsys_printf("nk_process_comd end !!");
	return (result);
}

#ifdef CONFIG_NK_LICENSE_COUNT_DOWN
static int nk_process_comd_factory(char cmd, char *type, char *paras, char *dbname)
{
    int result;

	//kd_Log ("%d %c\n", __LINE__, cmd);
	switch ( cmd )
	{
	   // case 't': // create database for each task 
            // result = nk_create_db(type, paras);
		    // break;
	   // case 'n': // for new a parameter in sysconfig
            // result = nk_new_setting(type, paras,dbname);
		    // break;
	   // case 'b': // for sysconfig1 (UI backup) file
            // result = nk_print_setting(type, paras, 1, dbname);
		    // break;
	   case 'p':
            result = nk_print_setting(type, paras, 0, dbname);
		    break;
       case 'w':
            result = nk_set_configuration_factory(type, paras, dbname);
			break;
       // case 'm': // modify value for specific entry of multi-list like entry of port forwarding
            // result = nk_mod_configuration(type, paras, dbname);
			// break;
       // case 'd':
		    // result = nk_delete_setting(type, paras, dbname);
		    // break;
	   // case 'a':
            // result = nk_print_setting(type, paras, 0, dbname);
		    // break;
	   default :
		    result = INVALID_GROUP;
	}
	return (result);
}
#endif

static int nk_print_setting (char *type, char *param, int flag, char *dbname) 
{
    FILE *database;

    char opcode[128], num_str[10];
	int  idx, i, number, result;
//	char * cptr;

	//printf("%d\n", __LINE__);
	//kd_Log ("%d nk_print_setting type[%s] param[%s]\n", __LINE__, type, param );

	if (flag == 1)
	{
		if ( NULL == (database = fopen(nk_fconfig1, "r+")))
			return  FAIL_TO_OPEN;
	}
	else
	{
		if ( NULL == (database = fopen(dbname, "r+")))
			return ITEM_NOT_FOUND;
		//	return  FAIL_TO_OPEN;
	}
	//kd_Log ("%d nk_print_setting fopen OK\n", __LINE__ );

	result = 0;

	{
		number = 1;
		idx = i = 0;
		while ( param[idx] != ' ' && param[idx] != '\0')
			opcode[i++] = param[idx++];
		opcode[i] = '\0';
		while ( param[idx] == ' ' )
			idx++;
		//printf("%d - %s\n", __LINE__, opcode);

		if (param[idx] != '\0') // opcode number
		{
			i = 0;
			while (param[idx] != '\0')
				num_str[i++] = param[idx++];
			num_str[i] = '\0';
		    number = atoi(num_str);
			//printf("%d - %s %d\n", __LINE__, opcode, number);
			//kd_Log("%d - %s %d\n", __LINE__, opcode, number);
			result = print_config( database, type, opcode, number);
		}
		else //opcode, no number
		{
			//kd_Log("%d - %s no number\n", __LINE__, opcode);
			result = print_config( database, type, opcode, 1);
		}
	}
	fclose(database);
	return (result);
}

static int nk_create_db (char *type, char *param)
{
	int  result, i;
	char *opcode;

	if (!strcmp (type, "DHCP"))
	{
		result = create_dhcp_config();	// still from sysconfig not nk_sysconfig, need modify later
		return (result);
	}
	else if (!strcmp (type, "PPTP"))	// still from sysconfig not nk_sysconfig, need modify later
	{
		result = create_pptp_config();
		return (result);
	}
	else if (!strcmp (type, "RESOLV"))	// still from sysconfig not nk_sysconfig, need modify later
	{
		result = create_resolv();
		return (result);
	}
	else if (strstr (type, "DDNS"))
	{
		// Kide 2005/03/14
//		strcpy(opcode, type[i]);
		i = atoi ((char *)type+4);
		result = nk_create_ddns_config(i);
		return (result);
	}
	else
		return ITEM_NOT_FOUND;
}

static int nk_new_setting (char *type, char *param, char *dbname)
{
	FILE  *database, *ftmp; 
	int   result, idx, i, count,k;
	char  tpvalue[ARGV_SIZE], opcode[128];
	//char dbname[512];
#ifdef SQUARE_BRACKET    
//   int  i, j;
   int  j;
   char newitem[ARGV_SIZE];
#endif
	
	result = 0;

	if ( NULL == (database = fopen(dbname, "r+")))
	{
		if ( NULL == (database = fopen(dbname, "w+")))
		{
//			kd_Log("failed to open %s", tmpath);
			return (FAIL_TO_OPEN);
		}
	}

	if ( 0 == (result = find_category(database, type)))
	{
		idx = i = 0;
		while ( param[idx] != '=')
			opcode[i++] = param[idx++];
		opcode[i] = '\0';
//		printf ("%d, opcode %s\n", __LINE__, opcode);
		if (ITEM_NOT_FOUND == find_last_category_item(database, opcode))
		{
//		if ( 0 != fseek(database, ps, SEEK_SET))
//			return(FAIL_TO_SEEK);
//		idx = 0;
//		while (value[idx] != '\0')
//		{
//			if ( value[idx] == ',' )
//				value[idx] = ' ';
//			idx++;
//		}
			strcpy(tpvalue, param);
			strcat(tpvalue, "\n");
			result = insert_item(database, tpvalue, 0);
			fclose(database);

			/*update the change*/
#ifdef COPY_FILEOPEN	
			if ( NULL == (database = fopen(dbname, "w+")))
			{ 
				return  FAIL_TO_OPEN;
			}
    
			if ( NULL == (ftmp = fopen(tmpath, "r+")))
			{
				fclose(database);
				return (FAIL_TO_OPEN);
			}

			while (!feof(ftmp))
			{
				count = fread (tpvalue, sizeof(char), 40, ftmp);
				fwrite(tpvalue, sizeof(char), count, database);
    		}
			fclose(database);
    		fclose(ftmp);
#else
	rename(tmpath, dbname);
#endif
		}
	}
	else
	{
		fclose(database);
		strcpy(tpvalue, "\n[");
		strcat(tpvalue, type);
		strcat(tpvalue, "]");
		strcat(tpvalue, "\n");
		
#ifdef SQUARE_BRACKET    
	i = j = 0;
	while ( param[i] != '\0')
	{
		if ((param[i] == '[') || (param[i] == ']'))
		{
			for (j = 0; j < i ; j++)
			{
				newitem[j] = param[j];
			}
			newitem[j++] = param[i];
			newitem[j++] = param[i];
			i++;
			//j++;
			for (; param[i] != '\0'; i++, j++)
			{
				newitem[j] = param[i];
				if ((param[i] == '[') || (param[i] == ']'))
				{
					newitem[j+1] = newitem[j];
					j++;
				}
			}
			newitem[j] = '\0';
//			printf ("%d, %s %d, %s %d\n", __LINE__, item, strlen(item), newitem, strlen(newitem));
//			printf ("%d\n", __LINE__);
			param = newitem;
			break;
		}
		i++;
	}
#endif

		
		strcat(tpvalue, param);
		strcat(tpvalue, "\n");
	
//	printf ("%d, %s, %s, %s\n", __LINE__, type, param, tpvalue);
		if ( NULL == (database = fopen(dbname, "a+")))
		{
//			kd_Log("failed to open %s", tmpath);
			return (FAIL_TO_OPEN);
		}
		fwrite (tpvalue, sizeof(char), strlen(tpvalue), database);

		fclose(database);
		result = 0;
		if ( NULL == (database = fopen("/tmp/category_list", "a+")))
		    return (FAIL_TO_OPEN);

		strcpy(tpvalue,type);
		strcat(tpvalue,"\n");

		fwrite (tpvalue, sizeof(char), strlen(tpvalue), database);
		fclose(database);
	}
	return (result);

}

static int nk_set_configuration (char *type, char *param, char *dbname)
{
	int result, idx, i;
	char opcode[128], tmp[ARGV_SIZE];

	result = 0;

	if ( param == NULL || strlen(param) < 2)
	     return(INVALID_COMD);

	idx = i = 0;
	while ( param[idx] != '=')
		opcode[i++] = param[idx++];
	opcode[i] = '\0';

	i = 0;
	idx ++; //skip '='
	while ( param[idx] != '\0')
	{
		tmp[i ++] = param[idx ++];
	}
	tmp[i] = '\0';

    if ( tmp == NULL )
		return (INVALID_COMD);

//printf ("%d \n", __LINE__);
//	printf ("%d, %s, %s, %s\n", __LINE__, type, opcode, tmp);

	if (!(strcmp (opcode, "DNS")))
	{
		result = add_config(type, opcode, tmp, dbname);
	}
	else if (!(strcmp (type, "DHCP")) && !(strcmp (opcode, "RANGE")))
	{
		result = add_config("DHCP", "RANGE", tmp, dbname);
	}
	else if ((!(strcmp (type, "IP_FILTER"))) || (!(strcmp (type, "MAC_FILTER"))) ||
		(!(strcmp (type, "REDIRECT"))) || (!(strcmp (type, "PORT_FILTER"))) ||
		(!(strcmp (type, "DMZ"))) || 
		(!(strcmp (type, "HOST_LIST")))
		)
	{
		result = add_config(type, "ID", tmp, dbname);
//printf ("%d result %d\n", __LINE__, result);
	}
	else if (((!(strcmp (type, "SERVICE_PORT")))||(!(strcmp (type, "SINGLE_SERVICE_PORT"))) ||(!(strcmp (type, "GROUP_SERVICE_PORT")))|| (!(strcmp (type, "APPLICATION_LIST"))) ||(!(strcmp (type, "RC_LIST")))||(!(strcmp (type, "RC_DISPLAY_LIST"))) ||
		(!(strcmp (type, "PORT_FORWARDING"))) || (!(strcmp (type, "UPNP_FORWARDING"))) || (!(strcmp (type, "PR_LIST"))) || (!(strcmp (type, "PR_DISPLAY_LIST"))) ||
		(!(strcmp (type, "AUTOBALANCE"))) ||(!(strcmp (type, "ACCESS_RULE"))) ||(!(strcmp (type, "ACCESS_RULE_V6"))) || (!(strcmp (type, "STATIC_ROUTE"))) || (!(strcmp (type, "STATIC_ROUTE_V6"))) || (!(strcmp (type, "PENALTY_IP"))) ||
		(!(strcmp (type, "PORT_TRIGGERING"))) || (!(strcmp (type, "TRUST_DOMAIN"))) || (!(strcmp (type, "8021Q"))) || (!(strcmp (type, "MULTI_WAN"))) ||
		(!(strcmp (type, "STATIC_IP"))) ||(!(strcmp (type, "121NAT"))) ||(!(strcmp (type, "MULTI21NAT"))) ||(!(strcmp (type, "MULTIPLE_SUBNET"))) ||(!(strcmp (type, "SESSION_ENHANCE"))) ||(!(strcmp (type, "ALLOW_DOMAIN_EXP_IP"))) ||(!(strcmp (type, "ALLOW_DOMAIN_EXP_GROUP"))) ||
		(!(strcmp (type, "ALLOW_DOMAIN"))) || (!(strcmp (type, "FORBIDDEN_DOMAIN"))) || (!(strcmp (type, "KEYWORD_BLOCK"))) || (!(strcmp (type, "IPGROUP_LIST"))) || (!(strcmp (type, "QOS_DISPLAY_EXCEPT"))) || (!(strcmp (type, "QOS_EXCEPT"))) ||
		(!(strcmp (type, "UPNP_SERVICE_PORT"))) || (!(strcmp (type, "QOS_SERVICE_PORT"))) ||
		(!(strcmp (type, "IPGROUP"))) || (!(strcmp (type, "SESSION_LIST"))) ||  (!(strcmp (type, "PPTP_SERVER"))) ||
		(!(strcmp (type, "STROUTE"))) || (!(strcmp (type, "ACL_RULE")))|| (!(strcmp (type, "ACL_HIDDEN_RULE")))|| (!(strcmp (type, "ACL_DISPLAY_RULE")))||
		(!(strcmp (type, "AUTOBALANCE_RULE"))) ||(!(strcmp (type, "AGING_TIME"))) ||
		(!(strcmp (type, "IPGROUP_RULE"))) ||
		(!(strcmp (type, "STROUTE_RULE"))) ||
		(!(strcmp (type, "ACCESS_LIST"))) || (!(strcmp (type, "ACCESS_LIST_V6"))) ||
		(!(strncmp (type, "USB_KEY_USER_LIST",17))) ||//match USB_KEY_USER_LIST0001&USB_KEY_USER_LIST0002
		(!(strcmp (type, "USB_GROUP"))) ||
		(!(strcmp (type, "SROUTE_SDS0"))) ||
		(!(strcmp (type, "SROUTE_SDS1"))) ||
		(!(strcmp (type, "SROUTE_SDS2"))) ||
		(!(strcmp (type, "MULTI21NAT")))  ||
		(!(strcmp (type, "AV_EXCLUSION_LIST"))) ||
		(!(strcmp (type, "AV_EXCLUSION_DISPLAY_LIST"))) ||
		(!(strcmp (type, "AI_EXCLUSION_LIST"))) ||
		(!(strcmp (type, "AI_EXCLUSION_DISPLAY_LIST"))) ||
		(!(strcmp (type, "BLACKLIST"))) ||
		(!(strcmp (type, "WHITELIST"))) ||
		(!(strcmp (type, "DNS_LOCAL_DATABASE"))) ||
		(!(strcmp (type, "DNS_LOCAL_DATABASE_V6"))) ||
		(!(strcmp (type, "DDNS5"))) ||
		(!(strcmp (type, "QUICK_VPN_LIST"))) ||
		(!(strcmp (type, "TM_APPR_URL"))) ||
		(!(strcmp (type, "TM_APPR_IP"))) ||
		/*For Group management*/
		(!(strcmp (type, "SINGLE_SERVICE_PORT"))) ||(!(strcmp (type, "GROUP_SERVICE_PORT")))|| 
		(!(strcmp (type, "EXCEPTION_QQ_NUMBER"))) ||
		(!(strcmp (type, "REMOTEIPGROUP_LIST"))) ||
		(!(strcmp (type, "DEST_IPGROUP")))||
		(!(strcmp (type, "IPGROUP_BEF_LIST"))) ||
		(!(strcmp (type, "IP_LIST")))||(!(strcmp (type, "REMOTEIP_LIST"))) || (!(strcmp (type, "SERVICE_LIST"))) || (!(strcmp (type, "PORT_FWD_LIST"))) ||
		(!(strcmp (type, "SESSION_BASE_LIST"))) || (!(strcmp (type, "SESSION_BASE_LOCK_LIST"))) || (!(strcmp (type, "SESSION_BASE_DISPLAY_LIST"))) ||
		(!(strcmp (type, "ACCESS_WAN"))) || (!(strcmp (type, "ACCESS_WAN_V6"))) || (!(strcmp (type, "ACCESS_DMZ"))) || (!(strcmp (type, "ACCESS_DMZ_V6"))) ) && (!(strcmp (opcode, "ID"))) 
		)
	{
		result = add_config(type, opcode, tmp, dbname);
//printf ("%d result %d\n", __LINE__, result);
	}
	else if ((
		(!(strcmp (type, "IPSEC_G2G")))
		||(!(strcmp (type, "QVM_SERVER")))
		||(!(strcmp (type, "IPSEC_GRP")))
		)&& (!(strcmp (opcode, "IPS"))))
	{
		result = add_config(type, opcode, tmp, dbname);
//printf ("%d result %d\n", __LINE__, result);
	}
	else if ((!(strcmp (type, "SSL_GROUP_RESOURCE"))) && (!(strcmp (opcode, "RS"))))
	{
		result = add_config(type, opcode, tmp, dbname);
//printf ("%d result %d\n", __LINE__, result);
	}
	else if ((!(strcmp (type, "SSL_TSC"))) && (!(strcmp (opcode, "RS"))))
	{
		result = add_config(type, opcode, tmp, dbname);
//printf ("%d result %d\n", __LINE__, result);
	}
	else if (((!(strcmp (type, "DHCP_SUBNET1"))) || (!(strcmp (type, "DHCP_SUBNET2"))) || (!(strcmp (type, "DHCP_SUBNET3"))) || (!(strcmp (type, "DHCP_SUBNET4")))) && (!(strcmp (opcode, "MAC"))))
	{
		result = add_config(type, opcode, tmp, dbname);
	}
	else if ((!strncasecmp (type, "USER", 4)) &&
	((!(strcmp (opcode, "SCHEDULER"))) || (!(strcmp (opcode, "URL_BLOCK"))) ||
	(!(strcmp (opcode, "APPLICATION_BLOCK"))) || (!(strcmp (opcode, "APPLICATION_GUARANTEE"))))
	)
	{
		result = add_config(type, opcode, tmp, dbname);
	}
	else if (((!(strcmp (type, "L7_EXCEPTION"))) ||(!(strcmp (type, "L7_FILE_EXCEPTION")))) && (!(strcmp (opcode, "ID"))))
	{
		result = add_config(type, opcode, tmp, dbname);
//printf ("%d result %d\n", __LINE__, result);
	}
	else if ((!(strcmp (type, "SKYPE_DST_EXP"))) && (!(strcmp (opcode, "ID"))))
	{
		result = add_config(type, opcode, tmp, dbname);
//printf ("%d result %d\n", __LINE__, result);
	}
	else
	{
		result = set_config(type, opcode, tmp, 1, dbname);
//printf ("%d result %d\n", __LINE__, result);
	}
//printf ("%d result %d\n", __LINE__, result);

	return (result);
}

#ifdef CONFIG_NK_LICENSE_COUNT_DOWN
static int nk_set_configuration_factory (char *type, char *param, char *dbname)
{
	int result, idx, i;
	char opcode[128], tmp[ARGV_SIZE];

	result = 0;

	if ( param == NULL || strlen(param) < 2)
		return(INVALID_COMD);

	idx = i = 0;
	while ( param[idx] != '=')
		opcode[i++] = param[idx++];
	opcode[i] = '\0';

	i = 0;
	idx ++; //skip '='
	while ( param[idx] != '\0')
	{
		tmp[i ++] = param[idx ++];
	}
	tmp[i] = '\0';

	if ( tmp == NULL )
		return (INVALID_COMD);

	//printf ("%d \n", __LINE__);
	//printf ("%d, %s, %s, %s\n", __LINE__, type, opcode, tmp);
	result = set_config_factory(type, opcode, tmp, 1, dbname);
	//printf ("%d result %d\n", __LINE__, result);
	return (result);
}
#endif

static int nk_mod_configuration (char *type, char *param, char *dbname)
{
	int result, idx, i, number;
	char opcode[128], tmp[ARGV_SIZE], num_str[10];

kdsys_printf("nk_mod_configuration start !!");
	result = 0;

	if ( param == NULL || strlen(param) < 2)
	     return(INVALID_COMD);

	idx = i = 0;
	while ( param[idx] != ' ')
		opcode[i++] = param[idx++];
	opcode[i] = '\0';
	
	i = 0;
	idx ++; //skip space
	while ( param[idx] != '=')
		num_str[i++] = param[idx++];
	num_str[i] = '\0';
	number = atoi(num_str);

	i = 0;
	idx ++; //skip '='
	while ( param[idx] != '\0')
	{
		tmp[i ++] = param[idx ++];
	}
	tmp[i] = '\0';

    if ( tmp == NULL )
		return (INVALID_COMD);

//	printf ("%d, %s, %s, %d, %s\n", __LINE__, type, opcode, number, tmp);

	result = set_config(type, opcode, tmp, number, dbname);
//printf ("%d result %d\n", __LINE__, result);
//printf ("%d result %d\n", __LINE__, result);

kdsys_printf("nk_mod_configuration end !!");
	return (result);
}

/*remove one item at per function call, otherwise, the result will be wrong*/
static int nk_delete_setting (char *type, char *param, char *dbname)
{
	FILE *database, *ftmp;

	int result, idx, i, number, count;
	char opcode[128], num_str[10], options[128];

	result = 0;

//	printf("%d - %s, %s\n", __LINE__, type, param);

//	if ( param == NULL || strlen(param) < 2)
//		 return (INVALID_COMD);

	number = 1;
	if (param != NULL)
	{
	idx = i = 0;
	opcode[i] = '\0';
	while ( param[idx] != ' ' && param[idx] != '\0')
		opcode[i++] = param[idx++];
	opcode[i] = '\0';
	while ( param[idx] == ' ' )
		idx++;
//		printf("%d - %s", __LINE__, opcode);

	if (param[idx] != '\0') // opcode number
	{
		i = 0;
		while (param[idx] != '\0')
			num_str[i++] = param[idx++];
		num_str[i] = '\0';
	    number = atoi(num_str);
//		printf("%d - %s %s, %d\n", __LINE__, type, opcode, number);
	}
	else //opcode, no number
	{
//		printf("%d - %s %s, %d\n", __LINE__, type, opcode, number);
	}
	}
    if ( NULL == (database = fopen(dbname, "r+")))
    {
//        kd_Log("failed to open %s for updating!\n", fconfig);
        return  FAIL_TO_OPEN;
    }

	if ( param == NULL )	//TT : delete all
	{
//		if (!strncasecmp (type, "USER", 4))
		{
			if ( -1L != move_to_category(database, type))
				result = remove_category(database);
			else
				result = CAT_NOT_FOUND;
		}
	}
	else if (!(strcmp (opcode, "DNS")))
	{
		strcpy ( opcode, "DNS=");
		if ( -1L != move_to_item(database, type, opcode, number))
			result = remove_item(database);
		else
			result = ITEM_NOT_FOUND;
	}
	else if (!(strcmp (type, "DHCP")) && !(strcmp (opcode, "RANGE")))
	{
		strcpy ( opcode, "RANGE=");
		if ( -1L != move_to_item(database, type, opcode, number))
			result = remove_item(database);
		else
			result = ITEM_NOT_FOUND;
	}
	else if ((!(strcmp (type, "IP_FILTER"))) || (!(strcmp (type, "MAC_FILTER"))) ||(!(strcmp (type, "RC_LIST"))) ||(!(strcmp (type, "RC_DISPLAY_LIST")))||
		(!(strcmp (type, "REDIRECT"))) || (!(strcmp (type, "PORT_FILTER"))) ||(!(strcmp (type, "PR_LIST"))) || (!(strcmp (type, "PR_DISPLAY_LIST"))) ||
		(!(strcmp (type, "DMZ"))) || (!(strcmp (type, "PORT_FORWARDING"))) || (!(strcmp (type, "121NAT"))) ||(!(strcmp (type, "MULTI21NAT"))) || (!(strcmp (type, "QOS_SERVICE_PORT"))) ||
		(!(strcmp (type, "SERVICE_PORT"))) || (!(strcmp (type, "SINGLE_SERVICE_PORT")))||(!(strcmp (type, "GROUP_SERVICE_PORT")))|| (!(strcmp (type, "PORT_TRIGGERING"))) || (!(strcmp (type, "MULTIPLE_SUBNET"))) ||
		(!(strcmp (type, "UPNP_FORWARDING"))) || (!(strcmp (type, "APPLICATION_LIST"))) || (!(strcmp (type, "SESSION_ENHANCE"))) ||(!(strcmp (type, "SESSION_LIST"))) ||
		(!(strcmp (type, "AUTOBALANCE"))) || (!(strcmp (type, "ACCESS_RULE"))) || (!(strcmp (type, "ACCESS_RULE_V6"))) || (!(strcmp (type, "8021Q"))) || (!(strcmp (type, "MULTI_WAN"))) ||
		(!(strcmp (type, "ALLOW_DOMAIN"))) || (!(strcmp (type, "FORBIDDEN_DOMAIN"))) || (!(strcmp (type, "KEYWORD_BLOCK"))) || (!(strcmp (type, "PENALTY_IP"))) ||(!(strcmp (type, "ALLOW_DOMAIN_EXP_IP"))) || (!(strcmp (type, "ALLOW_DOMAIN_EXP_GROUP"))) ||
		(!(strcmp (type, "STATIC_ROUTE"))) || (!(strcmp (type, "STATIC_ROUTE_V6"))) || (!(strcmp (type, "TRUST_DOMAIN")))||(!(strcmp (type, "STATIC_IP")))|| (!(strcmp (type, "QOS_DISPLAY_EXCEPT")))||(!(strcmp (type, "QOS_EXCEPT"))) ||
		(!(strcmp (type, "DNS_LOCAL_DATABASE"))) ||
		(!(strcmp (type, "DNS_LOCAL_DATABASE_V6"))) ||
		(!(strcmp (type, "TM_APPR_URL"))) ||
		(!(strcmp (type, "TM_APPR_IP"))) ||
		(!(strncmp (type, "USB_KEY_USER_LIST",17))) ||//match USB_KEY_USER_LIST0001&USB_KEY_USER_LIST0002
		(!(strcmp (type, "USB_GROUP"))) || (!(strcmp (type, "SESSION_BASE_LIST"))) || (!(strcmp (type, "SESSION_BASE_LOCK_LIST"))) || (!(strcmp (type, "SESSION_BASE_DISPLAY_LIST"))) || 
		(!(strcmp (type, "AV_EXCLUSION_LIST"))) || (!(strcmp (type, "AV_EXCLUSION_DISPLAY_LIST"))) ||
		(!(strcmp (type, "AI_EXCLUSION_LIST"))) || (!(strcmp (type, "AI_EXCLUSION_DISPLAY_LIST"))) ||
		(!(strcmp (type, "BLACKLIST"))) || (!(strcmp (type, "WHITELIST"))) ||(!(strcmp (type, "AGING_TIME"))) ||
		(!(strcmp (type, "HOST_LIST"))) || (!(strcmp (type, "UPNP_SERVICE_PORT"))) || (!(strcmp (type, "IPGROUP_LIST"))) || (!(strcmp (type, "IPGROUP"))) ||
		(!(strcmp (type, "STROUTE"))) || (!(strcmp (type, "AUTOBALANCE_RULE"))) || (!(strcmp (type, "IPGROUP_RULE"))) || (!(strcmp (type, "STROUTE_RULE"))) || (!(strcmp (type, "ACCESS_LIST"))) || (!(strcmp (type, "ACCESS_LIST_V6"))) || 
		(!(strcmp (type, "SROUTE_SDS0"))) || (!(strcmp (type, "SROUTE_SDS1"))) || (!(strcmp (type, "SROUTE_SDS2"))) || (!(strcmp (type, "ACCESS_WAN"))) || (!(strcmp (type, "ACCESS_WAN_V6"))) || (!(strcmp (type, "ACCESS_DMZ"))) || (!(strcmp (type, "ACCESS_DMZ_V6"))) || (!(strcmp (type, "EXCEPTION_QQ_NUMBER"))) ||
		(!(strcmp (type, "IP_LIST")))||(!(strcmp (type, "REMOTEIP_LIST")))||(!(strcmp (type, "SERVICE_LIST")))|| (!(strcmp (type, "PPTP_SERVER"))) ||(!(strcmp (type, "ACL_RULE")))||(!(strcmp (type, "ACL_HIDDEN_RULE")))||(!(strcmp (type, "ACL_DISPLAY_RULE")))||
		(!(strcmp (type, "SINGLE_SERVICE_PORT")))||(!(strcmp (type, "GROUP_SERVICE_PORT")))||
		(!(strcmp (type, "REMOTEIPGROUP_LIST"))) || (!(strcmp (type, "DEST_IPGROUP")))|| (!(strcmp (type, "IPGROUP_BEF_LIST"))) || (!(strcmp (type, "PORT_FWD_LIST"))) || (!(strcmp (type, "QUICK_VPN_LIST")))
 )
	{
		strcpy(opcode, "ID=");

		if ( -1L != move_to_item(database, type, opcode, number))
			result = remove_item(database);
		else
			result = ITEM_NOT_FOUND;
	}
	else if ((
		(!(strcmp(type,"IPSEC_G2G"))) 
		|| (!(strcmp (type, "QVM_SERVER")))
		|| (!(strcmp (type, "IPSEC_GRP")))
		)&&!(strcmp (opcode, "IPS")))
	{
		strcat(opcode, "=");

		if ( -1L != move_to_item(database, type, opcode, number))
			result = remove_item(database);
		else
			result = ITEM_NOT_FOUND;
	}
	else if (!(strcmp(type,"SSL_GROUP_RESOURCE"))&&!(strcmp (opcode, "RS")))
	{
		strcat(opcode, "=");

		if ( -1L != move_to_item(database, type, opcode, number))
			result = remove_item(database);
		else
			result = ITEM_NOT_FOUND;
	}
	else if (!(strcmp(type,"SSL_TSC"))&&!(strcmp (opcode, "RS")))
	{
		strcat(opcode, "=");

		if ( -1L != move_to_item(database, type, opcode, number))
			result = remove_item(database);
		else
			result = ITEM_NOT_FOUND;
	}
	else if (((!(strcmp (type, "DHCP_SUBNET1"))) || (!(strcmp (type, "DHCP_SUBNET2"))) || (!(strcmp (type, "DHCP_SUBNET3"))) || (!(strcmp (type, "DHCP_SUBNET4")))) && (!(strcmp (opcode, "MAC"))))
	{
		strcat(opcode, "=");

		if ( -1L != move_to_item(database, type, opcode, number))
			result = remove_item(database);
		else
			result = ITEM_NOT_FOUND;
	}
	else if (((!(strcmp (type, "L7_EXCEPTION"))) ||(!(strcmp (type, "L7_FILE_EXCEPTION")))) && (!(strcmp (opcode, "ID"))))
	{
		strcat(opcode, "=");

		if ( -1L != move_to_item(database, type, opcode, number))
			result = remove_item(database);
		else
			result = ITEM_NOT_FOUND;
	}
	else if (!(strcmp(type,"SKYPE_DST_EXP"))&&!(strcmp (opcode, "ID")))
	{
		strcat(opcode, "=");

		if ( -1L != move_to_item(database, type, opcode, number))
			result = remove_item(database);
		else
			result = ITEM_NOT_FOUND;
	}
	else if ((!strncasecmp (type, "USER", 4)) &&
	((!(strcmp (opcode, "SCHEDULER"))) || (!(strcmp (opcode, "URL_BLOCK"))) ||
	(!(strcmp (opcode, "APPLICATION_BLOCK"))) || (!(strcmp (opcode, "APPLICATION_GUARANTEE"))))
	)
	{
		strcat(opcode, "=");
		if ( -1L != move_to_item(database, type, opcode, number))
			result = remove_item(database);
		else
			result = ITEM_NOT_FOUND;
	}
	else
	{
		strcat(opcode, "=");
		if ( -1L != move_to_item(database, type, opcode, number))
			result = remove_item(database);
		else
			result = ITEM_NOT_FOUND;
	}


	fclose(database);
	/*update the change*/
#ifdef COPY_FILEOPEN	
    if ( NULL == (database = fopen(dbname, "w+")))
    {
        return  FAIL_TO_OPEN;
    }

	if ( NULL == (ftmp = fopen(tmpath, "r+")))
	{
		 fclose(database);
         return (FAIL_TO_OPEN);
	}

    while (!feof(ftmp))
    {
        count = fread (options, sizeof(char), 40, ftmp);
        fwrite(options, sizeof(char), count, database);
    }
	fclose(database);
    fclose(ftmp);
#else
	rename(tmpath, dbname);
#endif
	return (result);
}


//#define SYS_FILE		"/tmp/nk_sysconfig"
#define BATCH_SYS_FILE		"/tmp/kd_batch"
#define BATCH_BACKUP_FILE	"/tmp/kd_batch_backup"


void kk_printf(char *str)
{
 FILE *fp;
 fp = fopen("/dev/console", "w");
 if (fp == NULL) {
   return;
 }
 fprintf(fp, "kk_pintf: %s\r\n", str);
 fflush(fp);
 fclose(fp);
}

void NK_db_write_new(char *entry, char *value)
{
    kd_batch(entry, 1, value);
    return;
}

int kd_batch(char * parm2, int cmdType, char * printBuf)
{
	FILE *bfp;
	int   idx, pidx, result, i, j, fd;
	char DBfield[ARGV_SIZE], DBSubfield[ARGV_SIZE], DBentry[ARGV_SIZE], DBsave[ARGV_SIZE];

	//get field name
	idx = pidx = 0;
	while ( parm2[idx] != ' ')
		DBfield[pidx++] = parm2[idx++];
	DBfield[pidx] = '\0';

	//get sub field name
	pidx = 0;
	idx++;
	while ( parm2[idx] != '=')
	{
		DBSubfield[pidx++] = parm2[idx++];
	}
	DBSubfield[pidx] = '\0';

	sprintf(DBsave,"%s=%s\n",DBSubfield,printBuf);

	result=strlen(DBsave);
	DBsave[result]='\0';

	if((bfp = fopen(BATCH_SYS_FILE,"a"))==NULL)
		kk_printf("open batch sys file error !!! \n");

	fd=fwrite(DBsave, sizeof(char),strlen(DBsave),bfp );
	fclose(bfp);
	return 0;
}

int kdBatchUpdateDB(char *field,char *subfeild)
{
	int   idx, pidx, result, i, j, fd;
    char dbname[520],FlagFile[520];

#ifdef USE_SPLIT_DB_FLAG
	sprintf(FlagFile,"/tmp/%s.txt",field);
#else
	strcpy(FlagFile,"/tmp/test.txt");
#endif
	umask(0);
	fd = open(FlagFile, O_RDONLY | O_CREAT | O_EXCL, 0666);
	j = 0;
	while ( fd < 0)
	{
		srand(time(0));
		j = 700 + (int) (900.0 * rand()/(RAND_MAX + 1.0));
		for (i = 0; i < j; i ++);
		fd = open (FlagFile, O_RDONLY | O_CREAT | O_EXCL, 0666);
	}

	sprintf(dbname,"/tmp/splitDB/%s",field);


	result=Batch_config(field,subfeild);
		i = close(fd);
		i = unlink(FlagFile);
	if(result)
	{
		rename(BATCH_BACKUP_FILE, dbname);
		system("rm -f /tmp/kd_batch");
	}
	return result;
}
/*
#define SYS_FILE		"/tmp/nk_sysconfig"
#define BATCH_SYS_FILE		"/tmp/kd_batch"
#define BATCH_BACKUP_FILE		"/tmp/kd_batch_backup"*/

int Batch_config(char *field, char *subfeild)
{
    FILE  *srcFile, *destFile, *batchFile;
    int i=0,idx,entryLen=0,flag=0,result=1;
    char fileLine[LENGTH_READ_BUF],subLine[LENGTH_READ_BUF],tmp[LENGTH_READ_BUF],dbname[520];

    fileLine[0]='\0';
    subLine[0]='\0';
    tmp[0]='\0';

    sprintf(dbname,"/tmp/splitDB/%s",field);


    if ( NULL == (srcFile = fopen(dbname, "r")))
	result= 0;
    else
    {
	if ( NULL == (destFile = fopen(BATCH_BACKUP_FILE, "w+")))
	    result= 0;
	while(fgets(fileLine,LENGTH_READ_SIZE,srcFile) != NULL)
	{
	    fputs(fileLine,destFile);
	    if(fileLine[0]=='[' && flag==0)
	    {
		strcpy(tmp,"");
		entryLen=strlen(fileLine);
		idx=0;

		do
		{
		    tmp[idx]=fileLine[idx+1];
		    idx++;
		}while( ((fileLine[idx+1]!='=') && (fileLine[idx+1]!=']') && (idx<entryLen)));

		tmp[idx]='\0';
	
		if(!strcmp(tmp,field))
		{
		    flag=1;
		    if ( NULL == (batchFile = fopen(BATCH_SYS_FILE, "r")))
			goto nextdo;

		    while(fgets(subLine,LENGTH_READ_SIZE,batchFile) != NULL)
		    {
			fputs(subLine,destFile);
		    }
		    if(feof(batchFile))
			fclose(batchFile);
		    
nextdo:
		    while(fgets(fileLine,LENGTH_READ_SIZE,srcFile) != NULL)
		    {
			strcpy(tmp,"");
			entryLen=strlen(fileLine);
			idx=0;

			if(fileLine[0]=='[')
			{
			    fputs(fileLine,destFile);
			    break;
			}

			do
			{
			    tmp[idx]=fileLine[idx];
			    idx++;
			}while( ((fileLine[idx]!='=') && (fileLine[idx]!=']') && (idx<entryLen)));

			tmp[idx]='\0';

			if(strcmp(tmp,subfeild))
			    fputs(fileLine,destFile);
		    }
		}
	    }//if(fileLine[0]=='[')
	}//while(fgets(fileLine,LENGTH_READ_SIZE,srcFile) != NULL)
	fclose(destFile);
    }//if( NULL == (srcFile = fopen(SYS_FILE, "r")))
	

    fclose(srcFile);
    return 1;
}

db_value_get *BatchGet_config(char *field, char *subfield)
{
    FILE  *srcFile;
    int i=0,idx,sidx,entryLen=0,subentryLen=0,flag=0,result=1;
    char fileLine[LENGTH_READ_BUF],subLine[LENGTH_READ_BUF],tmp[LENGTH_READ_BUF],entry[LENGTH_READ_BUF],dbname[520],FlagFile[520];
    db_value_get *ip_mac_list,*nk_ip_mac_p=NULL, *tmp1,original_list;

	int   l,j, fd;
#ifdef USE_SPLIT_DB_FLAG
	sprintf(FlagFile,"/tmp/%s.txt",field);
#else
	strcpy(FlagFile,"/tmp/test.txt");
#endif
	umask(0);
	fd = open(FlagFile, O_RDONLY | O_CREAT | O_EXCL, 0666);
	j = 0;
	while ( fd < 0)
	{
		srand(time(0));
		j = 700 + (int) (900.0 * rand()/(RAND_MAX + 1.0));
		for (l = 0; l < j; l ++);

		fd = open (FlagFile, O_RDONLY | O_CREAT | O_EXCL, 0666);
	}

    fileLine[0]='\0';
    subLine[0]='\0';
    tmp[0]='\0';

    sprintf(dbname,"/tmp/splitDB/%s",field);

    if ( NULL == (srcFile = fopen(dbname, "r")))
	original_list.next= NULL;
    else
    {
	while(fgets(fileLine,LENGTH_READ_SIZE,srcFile) != NULL)
	{
	    if(fileLine[0]=='[' && flag==0)
	    {
		strcpy(tmp,"");
		entryLen=strlen(fileLine);
		idx=0;

		do
		{
		    tmp[idx]=fileLine[idx+1];
		    idx++;
		}while( ((fileLine[idx+1]!='=') && (fileLine[idx+1]!=']') && (idx<entryLen)));

		tmp[idx]='\0';
	
		if(!strcmp(tmp,field))
		{
		    flag=1;
		    while(fgets(fileLine,LENGTH_READ_SIZE,srcFile) != NULL)
		    {
			if(fileLine[0]=='[')
			{
			    break;
			}
			strcpy(tmp,"");
			entryLen=strlen(fileLine);
			idx=0;
			sidx=0;
			do
			{
			    tmp[idx]=fileLine[idx];
			    idx++;
			}while( ((fileLine[idx]!='=') && (fileLine[idx]!=']') && (idx<entryLen)));
			tmp[idx]='\0';

			if(!strcmp(tmp,subfield))
			{
			    idx++;
			    subentryLen=entryLen-idx;
			    do
			    {
				entry[sidx]=fileLine[idx];
				sidx++;
				idx++;
			    }while(((fileLine[idx]!='\0') && (fileLine[idx]!='\n')) && (sidx<subentryLen));
			    entry[sidx]='\0';

			    ip_mac_list = (db_value_get *)malloc(sizeof(db_value_get));
			    bzero((char *)ip_mac_list, sizeof(db_value_get));
			    ip_mac_list->next = nk_ip_mac_p;
			    strcpy(ip_mac_list->entry,entry);
			    nk_ip_mac_p = ip_mac_list;
			}
		    }
		}
		if(flag==1)
		{
			original_list.next= nk_ip_mac_p;
			break;
		}
	    }//if(fileLine[0]=='[')
	}//while(fgets(fileLine,LENGTH_READ_SIZE,srcFile) != NULL)
    }//if( NULL == (srcFile = fopen(SYS_FILE, "r")))
    i = close(fd);
    i = unlink(FlagFile);

    fclose(srcFile);

    return &original_list;
}


void splitDB()
{
	FILE  *srcFile, *destFile, *listFile;
	int i=0,idx,entryLen=0;
	char fileLine[LENGTH_READ_BUF],tmp[LENGTH_READ_BUF],mulu[LENGTH_READ_PATH];
	
    int l,j, fd;
//protect split process
    umask(0);
    fd = open("/tmp/test.txt", O_RDONLY | O_CREAT | O_EXCL, 0666);
    j = 0;
    while ( fd < 0)
    {
	srand(time(0));
	j = 700 + (int) (900.0 * rand()/(RAND_MAX + 1.0));
	for (i = 0; i < j; i ++);
	fd = open ("/tmp/test.txt", O_RDONLY | O_CREAT | O_EXCL, 0666);
    }
	
	fileLine[0]='\0';
	tmp[0]='\0';
	
	if ( NULL == (srcFile = fopen(SYS_FILE, "r")))
		return ;
	else
	{
	    if ( NULL == (listFile = fopen("/tmp/category_list", "w+")))
	    {
		fclose(srcFile);
		return ;
	    }

	    while(fgets(fileLine,LENGTH_READ_SIZE,srcFile) != NULL)
	    {
		if(fileLine[0]=='[')
		{
nextsplit:
		    strcpy(tmp,"");
		    entryLen=strlen(fileLine);
		    idx=0;
	
		    do
		    {
			tmp[idx]=fileLine[idx+1];
			idx++;
		    }while( ((fileLine[idx+1]!='=') && (fileLine[idx+1]!=']') && (idx<entryLen)));
	
		    tmp[idx]='\0';
		    sprintf(mulu,"%s",tmp);
		    strcat(mulu, "\n");
		    fputs(mulu,listFile);//write file name to /tmp/category_list

		    sprintf(mulu,"/tmp/splitDB/%s",tmp);
		    if ( NULL == (destFile = fopen(mulu, "w+")))
			break;
		    else
		    {
			fputs(fileLine,destFile);
			while(fgets(fileLine,LENGTH_READ_SIZE,srcFile) != NULL)
			{
			    strcpy(tmp,"");
			    entryLen=strlen(fileLine);
			    idx=0;
	
			    if(fileLine[0]=='[')
			    {
				fclose(destFile);
				goto nextsplit;
			    }
			    if(fileLine[0]!='\n')
			    fputs(fileLine,destFile);
			}
		    }
		    fclose(destFile);
		}//if(fileLine[0]=='[')
	    }//while(fgets(fileLine,LENGTH_READ_SIZE,srcFile) != NULL)
	
	}//if( NULL == (srcFile = fopen(SYS_FILE, "r")))
	fclose(listFile);
	fclose(srcFile);
    i = close(fd);
    i = unlink("/tmp/test.txt");
}

#include "snortrules.c"

