#ifndef sysconfig_h
#define sysconfig_h

#define ARGV_SIZE   3000

int  nk_sysconfig(int argc, char argv[3][ARGV_SIZE], char *supBuf);
int  sysconfig(int argc, char argv[3][ARGV_SIZE], char *supBuf);
int  remove_setting(char *category, char *item, int number);
int  add_config(char *category, char *item, char *value, char *filename );	//TT
int  set_config(char *category, char *item, char *value, int number, char *filename );	//TT
char *get_setting(char *category, char *item, int number);

int nk_snortconfig(int argc, char argv[4][ARGV_SIZE], char *retBuf, char *actionBuf, \
                   char *classBuf, char *sidBuf);
int NK_IDSRules_Read(char* parm2, int idx, char *printBuf, char *actionBuf, \
                     char *classBuf, char *sidBuf);
int NK_IDSRules_Write(char* parm2, int idx, char *actionBuf);

#endif
