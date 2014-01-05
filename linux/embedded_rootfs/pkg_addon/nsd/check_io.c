#include <time.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include "../nku/getlink.h"
#include <nkdef.h>
#include <nkutil.h>


typedef struct {
	unsigned int rx_bytes;
	unsigned int tx_bytes;
	unsigned int timeStamp;
} nsdStats;
nsdStats nsd_stats;

int is_interface_busy(int inf_num,char *NK_busy_condition,char *NK_busy_rate, unsigned int upstream_bw,unsigned int downstream_bw,char *dev_interface)
{
	char cmdBuf[200];

	unsigned int rx_bw,tx_bw;
	char buf[100];
	struct timeval	tv;

	unsigned int busy_condition = atoi(NK_busy_condition);//or rx traffic
	unsigned int busy_rate = atoi(NK_busy_rate);// default 2%


	struct interface ife;
	gettimeofday(&tv, NULL);
#if 0
	printf("gettimeofday tv.tv_sec=%u\n",tv.tv_sec);
	printf("inf_num=%u\n",inf_num);
	printf("busy_condition=%u\n",busy_condition);
	printf("busy_rate=%u\n",busy_rate);
#endif

	if (tv.tv_sec==nsd_stats.timeStamp)
	     tv.tv_sec++;

	nk_if_statistic_get_safe(dev_interface, &ife);
	rx_bw = (((ife.stats.rx_bytes - nsd_stats.rx_bytes) * 8)/(tv.tv_sec - nsd_stats.timeStamp))/1000;
	tx_bw = (((ife.stats.tx_bytes - nsd_stats.tx_bytes) * 8)/(tv.tv_sec - nsd_stats.timeStamp))/1000;
	nsd_stats.rx_bytes = ife.stats.rx_bytes;
	nsd_stats.tx_bytes = ife.stats.tx_bytes;
	nsd_stats.timeStamp = tv.tv_sec;
#if 0
	printf("ife[%u].stats.rx_bytes=%u\n",inf_num,ife.stats.rx_bytes);
	printf("ife[%u].stats.tx_bytes=%u\n",inf_num,ife.stats.tx_bytes);
	printf("rx_bw=%u,downstream_bw=%u\n",rx_bw,downstream_bw*busy_rate/100);
	printf("tx_bw=%u,upstream_bw=%u\n",tx_bw,upstream_bw*busy_rate/100);
#endif	
	

	if (busy_condition) {
		if ((rx_bw >= (downstream_bw*busy_rate/100)) && (tx_bw >= (upstream_bw*busy_rate/100))) {
			return 1;
		}
		else{
			return 0;
		}
	}
	else{
		if ((rx_bw >= (downstream_bw*busy_rate/100)) || (tx_bw >= (upstream_bw*busy_rate/100)))	{

			return 1;
		}
		else{

			return 0;
		}
	}

}
