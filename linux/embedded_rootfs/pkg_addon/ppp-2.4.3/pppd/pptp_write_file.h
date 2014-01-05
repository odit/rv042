#include <nkutil.h>
#define nk_PPTP_write_path "/tmp/.pptp.status"
#define nk_PPTP_FILE_STATUS "/tmp/.pptp.file.status"
struct nk_PPTP_write
{
	char username[128];
#ifdef CONFIG_SUPPORT_PPTPD_READ_CONfIGFILE_BY_SIGNAL
	char password[128];
#endif
	char ipparam_local_ip[32];
	char remote_ip[32];
	unsigned int Pid;
	int connect;
	char PPTP_name[32];
};
