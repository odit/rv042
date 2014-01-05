#include <string.h>

extern int udhcpd(int argc, char *argv[]);
extern int udhcpc(int argc, char *argv[]);

int main(int argc, char *argv[])
{
	int ret = 0;
	char *base = strrchr(argv[0], '/');
	
	if (strstr(base ? (base + 1) : argv[0], "dhcpd"))
		ret = udhcpd(argc, argv);
	else ret = udhcpc(argc, argv);
	
	return ret;
}
