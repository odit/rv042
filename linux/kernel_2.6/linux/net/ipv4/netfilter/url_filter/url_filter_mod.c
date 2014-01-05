#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/signal.h>
#include <linux/errno.h>
#include <linux/skbuff.h>
#include <linux/mm.h>
#include <linux/poll.h>
#include <linux/random.h>
#include <linux/init.h>
#include <linux/smp_lock.h>
#include <linux/netdevice.h>
#include <linux/device.h>
#include <linux/time.h>
#include <net/pkt_sched.h>
#include <net/sock.h>
#include <net/netlink.h>
#include <asm/uaccess.h>
#include <asm/system.h>
#include <asm/io.h>
#include "url_filter_mod.h"

#include <linux/fs.h>

#include <linux/cdev.h>
#include <linux/devfs_fs_kernel.h>


#include "ip_fil.h"

extern url_filter_setting_t url_filter_setting;
extern url_filter_statics_t url_filter_statics;
extern url_filter_debug_t url_filter_debug;
url_filter_time_t url_filter_time;
extern int (*url_filter_enqueue)(char *);
unsigned int current_queue_len = 0;
unsigned int current_queue_unhandle_len = 0;
unsigned long int repoll = 0;

extern int (*restart_url_filter_mod)(void );
static int show_log_flag = 0;

static spinlock_t tm_buffer_queue = SPIN_LOCK_UNLOCKED;
static spinlock_t tm_buffer_reset = SPIN_LOCK_UNLOCKED;

#define SHOW_UNHANDLE_LOG printk

#define SHOW_LOG_HIGH_LEVEL	20
#define SHOW_LOG_LOW_LEVEL	0

#define TM_BUF_UNUSED	0
#define TM_BUF_WROTE 	1
#define TM_BUF_READ	2

typedef struct httpinfo {
    char host[MAX_URL_FILTER_HOST_LEN + 1];
    int hostlen;
    char url[MAX_URL_FILTER_URL_LEN + 1];
    int urllen;
} httpinfo_t;

typedef struct {
    url_class_info url_filter_info;
    fr_info_t *url_filter_fin;
    //for check read or write or unused
    int buf_write_read;
} url_filter_list_t;

url_filter_list_t *url_buf_queue[DEFAULT_QUEUE_LEN+1];
#define DPRINTK(args...)			\
		if (url_filter_debug.url_filter_debug_mode) {	\
			printk(KERN_EMERG args); }

#define URL_FILTER_LOG(fmt, args...) printk(KERN_ALERT fmt, ## args)

typedef struct {
	wait_queue_head_t	mesg_wq;
} url_filter_chardev_t;
static url_filter_chardev_t *dev_ptr = NULL;

unsigned int wrong_queue_cnt=0;

static void reset_url_buf_queue(void)
{
	int i;
	spin_lock_bh(&tm_buffer_queue);
	current_queue_len = (DEFAULT_QUEUE_LEN)*2;
	//DPRINTK("------ reset_url_buf_queue start ------\n");
    printk(KERN_EMERG "------ reset_url_buf_queue start ------\n");
//max sema4 start
	for(i = 0; i < DEFAULT_QUEUE_LEN; i++)
	{	if(url_buf_queue[i])
		{
            if(url_buf_queue[i]->url_filter_info.skb)
            {
                kfree_skb(url_buf_queue[i]->url_filter_info.skb);
			    url_buf_queue[i]->url_filter_info.skb = NULL;
            }
#if 0
			url_buf_queue[i]->url_filter_fin->fin_ip = NULL;
			url_buf_queue[i]->url_filter_fin->fin_dp = NULL;
			if (url_buf_queue[i])
			{
				if (url_buf_queue[i]->url_filter_fin)
					kfree(url_buf_queue[i]->url_filter_fin);
				url_buf_queue[i]->url_filter_fin = NULL;
				kfree(url_buf_queue[i]);
			}
			url_buf_queue[i] = NULL;
#else
            if (url_buf_queue[i]->url_filter_fin)
            {
                url_buf_queue[i]->url_filter_fin->fin_ip = NULL;
                url_buf_queue[i]->url_filter_fin->fin_dp = NULL;
                kfree(url_buf_queue[i]->url_filter_fin);
                url_buf_queue[i]->url_filter_fin = NULL;
            }
            kfree(url_buf_queue[i]);
            url_buf_queue[i] = NULL;
#endif
		}
	}
	//DPRINTK("------ reset_url_buf_queue finish ------\n");
    printk(KERN_EMERG "------ reset_url_buf_queue finish ------\n");

	current_queue_unhandle_len = 0;

	current_queue_len = 0;
	spin_unlock_bh(&tm_buffer_queue);

//max sema4 end
}

struct tm
{
	int tm_sec;                   /* Seconds.     [0-60] (1 leap second) */
	int tm_min;                   /* Minutes.     [0-59] */
	int tm_hour;                  /* Hours.       [0-23] */
	int tm_mday;                  /* Day.         [1-31] */
	int tm_mon;                   /* Month.       [0-11] */
	int tm_year;                  /* Year - 1900.  */
	int tm_wday;                  /* Day of week. [0-6] */
	int tm_yday;                  /* Days in year.[0-365] */
	int tm_isdst;                 /* DST.         [-1/0/1]*/

	long int tm_gmtoff;           /* we don't care, we count from GMT */
	const char *tm_zone;          /* we don't care, we count from GMT */
};

void get_url_filter_time(void)
{
    struct tm currenttime;

    struct timeval kerneltimeval;
    do_gettimeofday(&kerneltimeval);

    localtime(kerneltimeval.tv_sec, &currenttime);

    /*purpose     : 0013213 author : Ben date : 2010-08-19*/
    /*description : check if in day_life_saving days, add 1 hour*/
    if (url_filter_setting.class_bz_time.enable_daylight_saving == 1)
    {
        int	smonth,sday,emonth,eday;
        int	sDateValue=0,eDateValue=0,nowDateValue=0;
        smonth = url_filter_setting.class_bz_time.daylight_saving_smonth;
        sday =   url_filter_setting.class_bz_time.daylight_saving_sday;
        emonth = url_filter_setting.class_bz_time.daylight_saving_emonth;
        eday =   url_filter_setting.class_bz_time.daylight_saving_eday;
        sDateValue=smonth*100+sday;
        eDateValue=emonth*100+eday;
        nowDateValue=(currenttime.tm_mon+1)*100+currenttime.tm_mday;

        if( ((sDateValue <= eDateValue) && ((sDateValue<=nowDateValue)&&(nowDateValue<=eDateValue))) ||
        ( (sDateValue > eDateValue) && ((nowDateValue > sDateValue) ||(nowDateValue<eDateValue))) )
        {
            kerneltimeval.tv_sec = kerneltimeval.tv_sec + 3600;
            localtime(kerneltimeval.tv_sec, &currenttime);
        }
    }

    url_filter_time.day  = currenttime.tm_wday;//0 as sunday, 0~6
    if ((int)currenttime.tm_hour+url_filter_setting.class_bz_time.gmt_offset<0)
    {
        url_filter_time.hour = (unsigned int)((int)currenttime.tm_hour+url_filter_setting.class_bz_time.gmt_offset+24);
        if (url_filter_time.day>0)
            url_filter_time.day--;
        else
            url_filter_time.day=6;
    }
    else if ((int)currenttime.tm_hour+url_filter_setting.class_bz_time.gmt_offset>23)
    {
        url_filter_time.hour = (unsigned int)((int)currenttime.tm_hour+url_filter_setting.class_bz_time.gmt_offset-24);
        if (url_filter_time.day<6)
            url_filter_time.day++;
        else
            url_filter_time.day=0;
    }
    else
        url_filter_time.hour = (unsigned int)((int)currenttime.tm_hour+url_filter_setting.class_bz_time.gmt_offset);
}

/* Return 1 for match, 0 for accept, -1 for partial. */
static int find_pattern2(const char *data, size_t dlen,
       const char *pattern, size_t plen,
       char term,
       unsigned int *numoff,
       unsigned int *numlen)
{
    size_t i, j, k;
    int state = 0;
    *numoff = *numlen = 0;

    if (dlen == 0)
        return 0;

    if (dlen <= plen) { /* Short packet: try for partial? */
        if (strnicmp(data, pattern, dlen) == 0)
            return -1;
        else
            return 0;
    }
    for (i = 0; i <= (dlen - plen); i++) {
    /* DFA : \r\n\r\n :: 1234 */
    if (*(data + i) == '\r') {
        if (!(state % 2)) state++;  /* forwarding move */
        else state = 0;             /* reset */
    }
    else if (*(data + i) == '\n') {
        if (state % 2) state++;
        else state = 0;
    }
    else state = 0;

    if (state >= 4)
        break;

    /* pattern compare */
    if (memcmp(data + i, pattern, plen ) != 0)
        continue;

    /* Here, it means patten match!! */
    *numoff=i + plen;
    for (j = *numoff, k = 0; data[j] != term; j++, k++)
        if (j > dlen) return -1 ;   /* no terminal char */

    *numlen = k;
    return 1;
    }
    return 0;
}

int get_http_info(const struct sk_buff **pskb, httpinfo_t *info)
{
    struct iphdr *iph = (*pskb)->nh.iph;
    struct tcphdr *tcph = (void *)iph + iph->ihl*4;
    unsigned char *data = (void *)tcph + tcph->doff*4;
    unsigned int datalen = (*pskb)->len - (iph->ihl*4) - (tcph->doff*4);

    int found, offset;
    int hostlen, pathlen;
    int ret = 0;

    /* Basic checking, is it HTTP packet? */
    if (datalen < 10)
        return ret;     /* Not enough length, ignore it */

    /* find the 'Host: ' value */
    found = find_pattern2(data, datalen, "Host: ", sizeof("Host: ") - 1, '\r', &offset, &hostlen);

    if (!found || !hostlen)
        return ret;

    ret++;      /* Host found, increase the return value */
    memset(info->host, 0, sizeof(info->host));
    memset(info->url, 0, sizeof(info->url));
    hostlen = (hostlen < MAX_URL_FILTER_HOST_LEN) ? hostlen : MAX_URL_FILTER_HOST_LEN;
    strncpy(info->host, data + offset, hostlen);
    *(info->host + hostlen) = 0;                /* null-terminated */
    info->hostlen = hostlen;

    /* find the 'GET ' or 'POST ' value */
    found = find_pattern2(data, datalen, "GET ", sizeof("GET ") - 1, '\r', &offset, &pathlen);
    if (!found)
        found = find_pattern2(data, datalen, "POST ", sizeof("POST ") - 1, '\r', &offset, &pathlen);

    if (!found || (pathlen -= (sizeof(" HTTP/x.x") - 1)) <= 0)/* ignor this field */
        return ret;

    ret++;      /* GET/POST found, increase the return value */
    pathlen = (pathlen < MAX_URL_FILTER_URL_LEN) ? pathlen : MAX_URL_FILTER_URL_LEN;
    strncpy(info->url, data + offset, pathlen);
    *(info->url + pathlen) = 0;                /* null-terminated */
    info->urllen = pathlen;

    return ret;
}

static void update_statisc(unsigned int class_id, int status)
{
    switch (status)
    {
	case URL_FILTER_BLOCK:
	    if (class_id >= 0 && class_id <= MAX_CLASS_ID)
	    	url_filter_statics.url_class_id_pkt_cnt[class_id].pkt_drop++;
		if (url_filter_statics.url_class_id_pkt_cnt[class_id].pkt_drop > MAX_DROP_PKT_COUNT)
			url_filter_statics.url_class_id_pkt_cnt[class_id].pkt_drop = 0;
	    break;
	default:
	    //DPRINTK("update_statisc : class_id[%d] and unknown status[%d]\n", class_id, status);
	    break;
    }
    //DPRINTK("update_statisc : class_id[%d] pkt_drop[%d]\n", lass_id, url_filter_statics.url_class_id_pkt_cnt[class_id].pkt_drop);
}

void uf_log(struct in_addr src_ip, unsigned int class_id, char *phost, char *pUrl, int log_status)
{
    if (url_filter_setting.output_block==1)
    {
        DPRINTK("uf_log : class_id[%d] src_ip[%u.%u.%u.%u] log_status[%d] phost[%s] pUrl[%s]\n",
            class_id, NIPQUAD(src_ip), log_status, phost, pUrl);

        if (log_status == URL_BLOCK)
        {
            /*purpose     : 12638 author : Ben date : 2010-07-07*/
            /*description : Show log message*/
            printk(KERN_ALERT  "The URL(%s) you(%u.%u.%u.%u) are attempting to access has been identified as %s site and has been blocked.\n",
            phost, NIPQUAD(src_ip), url_filter_setting.class_id_setting[class_id].category);
        }
        else
        {
            /*purpose     : 12638 author : Ben date : 2010-07-07*/
            /*description : Set Access rule of transparent bridge after set access rules default*/
            printk(KERN_ALERT "The URL(%s) you(%u.%u.%u.%u) are attempting to access has been identified as low reputation and has been blocked.\n",
            phost, NIPQUAD(src_ip));
        }
    }
}

void make_decision(url_filter_list_t *buf, int status, int blk_st)
{
    //DPRINTK("make_decision : buf[%x] buf->url_filter_fin[%x] buf->url_filter_fin->apps_data[%x] status[%d] block_status[%d]\n",
    //	buf, buf->url_filter_fin, buf->url_filter_fin->apps_data, status, blk_st);

    if (buf == NULL || buf->url_filter_fin == NULL || buf->url_filter_info.skb == NULL)
    {
        DPRINTK("\nError! make_decision == NULL\n");
        return;
    }
    if (status == URL_FILTER_BLOCK)
    {
        //DPRINTK("url_filter_mod block_status[%d]\n", blk_st);
//	buf->url_filter_fin->fin_ip->saddr = buf->url_filter_fin->url_filter_src.s_addr;
//--> White modified 2008-03-02. Attempt to solve crash problem (Jerry committed to CVS)
	if (buf->url_filter_fin->apps_data)
		((webfil_data_t *)buf->url_filter_fin->apps_data)->checked = 1;
//<--
	url_tmufe_filter_block(buf->url_filter_info.skb, blk_st, url_filter_debug.url_filter_debug_mode);
	if (buf->url_filter_info.skb)
	    kfree_skb(buf->url_filter_info.skb);
    }
    else
    {
        ip_rcv_finish(buf->url_filter_info.skb);
    }

    buf->url_filter_info.skb = NULL;
    buf->url_filter_fin->fin_ip = NULL;
    buf->url_filter_fin->fin_dp = NULL;
}

int is_bz_time(unsigned int class_id)
{
    int ret = 0;

    get_url_filter_time();
    if ((url_filter_time.day >= 0 && url_filter_time.day <= 6) && (url_filter_setting.class_bz_time.bz_day[url_filter_time.day] == '1'))
    {DPRINTK("is_bz_time : class_id[%d] day[%d] hour[%d], is bz day\n", class_id, url_filter_time.day, url_filter_time.hour);}
    else
    {DPRINTK("is_bz_time : class_id[%d] day[%d] hour[%d], not bz day\n", class_id, url_filter_time.day, url_filter_time.hour);}
    if ((url_filter_time.day >= 0 && url_filter_time.day <= 6) && (url_filter_setting.class_bz_time.bz_day[url_filter_time.day] == '1'))
    {
	DPRINTK("is_bz_time : enable_all_day=[%d]\n", url_filter_setting.class_bz_time.enable_all_day);
	if (!url_filter_setting.class_bz_time.enable_all_day==1)
	{
	    DPRINTK("is_bz_time : enable_morn=[%d] from[%d]to[%d], enable_aft=[%d] from[%d]to[%d]\n", url_filter_setting.class_bz_time.enable_morn, url_filter_setting.class_bz_time.bz_morn_from, url_filter_setting.class_bz_time.bz_morn_to, url_filter_setting.class_bz_time.enable_aft, url_filter_setting.class_bz_time.bz_aft_from, url_filter_setting.class_bz_time.bz_aft_to);
	    if (url_filter_time.hour < 12 && url_filter_setting.class_bz_time.enable_morn==1)
	    {
		if (url_filter_time.hour >= url_filter_setting.class_bz_time.bz_morn_from &&
		    url_filter_time.hour < url_filter_setting.class_bz_time.bz_morn_to)
		{
		    DPRINTK("is_bz_time : disabled by morning\n");
		    ret = 1;
		}
	    }
	    else if (url_filter_time.hour >= 12 && url_filter_setting.class_bz_time.enable_aft==1)
	    {
		if (url_filter_time.hour >= url_filter_setting.class_bz_time.bz_aft_from &&
		    url_filter_time.hour < url_filter_setting.class_bz_time.bz_aft_to)
		{
		    DPRINTK("is_bz_time : disabled by afternoon\n");
		    ret = 1;
		}
	    }
	    else
	    {
		DPRINTK("is_bz_time : not in disabled time\n");
		/*purpose     : 0012707 author : Ben date : 2010-06-22*/
		/*description : if not businss day, set as leisure time*/
		ret = 0;
	    }
	}
	else
	{
	    DPRINTK("is_bz_time : disabled all day\n");
	    /*purpose     : 0012707 author : Ben date : 2010-06-22*/
	    /*description : if not businss day, set as leisure time*/
	    ret = 2;
	}
    }

    DPRINTK("is_bz_time : class_id[%d] ret[%d]\n", class_id, ret);
    return ret;
}

static int lookup_class_setting(url_class_info *url_info)
{
   int i, classId = 0, ret = URL_FILTER_PASS;
   int in_bz_time;

   if (url_info == NULL)
   	return ret;

   for (i = 0; i < url_info->cate_number; i++)
   {
   	classId = url_info->class_id[i];
   	in_bz_time = is_bz_time(classId);//0:in leisure , 1:in bz , 2:bz all day
   	if (classId >= 0 && classId <= MAX_CLASS_ID)
   	{
		DPRINTK("lookup_class_setting : index[%d] classId[%d] bz_hour[%d] leisure_hour[%d]\n",
			i, classId, url_filter_setting.class_id_setting[classId].bz_hour, url_filter_setting.class_id_setting[classId].leisure_hour);
		/*purpose     : 0012707 author : Ben date : 2010-06-22*/
		/*description : if not businss day, set as leisure time*/
		if (url_filter_setting.class_id_setting[classId].bz_hour==1 && (in_bz_time==1 || in_bz_time==2))
		{
			ret = URL_FILTER_BLOCK;
			url_info->block_id = classId;
			break;
		}
		else if (url_filter_setting.class_id_setting[classId].leisure_hour==1 && (in_bz_time==0 || in_bz_time==2))
		{
			ret = URL_FILTER_BLOCK;
			url_info->block_id = classId;
			break;
		}
   	}
   }
   if (ret == URL_FILTER_PASS)
   {DPRINTK("lookup_class_setting : url_info->cate_number[%d] classId[%d] URL_FILTER_PASS\n", url_info->cate_number, classId);}
   else
   {DPRINTK("lookup_class_setting : url_info->cate_number[%d] classId[%d] URL_FILTER_BLOCK\n", url_info->cate_number, classId);}
   return ret;
}

int check_uf_status(url_class_info *url_info)
{
    int ret = URL_FILTER_PASS;

    if (url_filter_setting.filter_enable==1)
    {
	ret = lookup_class_setting(url_info);
    }

    DPRINTK("check_uf_status : url_info[%x] ret[%d]\n", url_info, ret);
    return ret;
}

int check_wrs_status(int score)
{
    int ret = URL_FILTER_PASS;

    if (url_filter_setting.wrs_enable==1)
    {
        if (score <= url_filter_setting.wrs_level)
	    ret = URL_FILTER_BLOCK;
    }

    DPRINTK("check_wrs_status : score[%d] wrs_level[%d] ret[%d]\n", score, url_filter_setting.wrs_level, ret);
    return ret;
}

int check_appr_list(char *pHost, char *pUrl)
{
    int i, ret = URL_FILTER_BLOCK, appr_url_len = 0, host_len = 0, url_len = 0;

    if (!pHost || !pUrl)
        return ret;

    host_len = strlen(pHost);
    url_len = strlen(pUrl);
    //DPRINTK("check_appr_list : pHost[%s] host_len[%d] pUrl[%s] url_len[%d]\n", pHost, host_len, pUrl, url_len);
    if (url_filter_setting.appr_url_enable==1)
    {
    	for (i = 0; i < MAX_APPR_URL_LIST; i++)
    	{
	    appr_url_len = strlen(url_filter_setting.appr_list.appr_url[i]);
	    //DPRINTK("check_appr_list : index[%d] appr_url[%s] appr_url_len[%d]\n", i, url_filter_setting.appr_list.appr_url[i], appr_url_len);
	    if (appr_url_len <= 0)
	        break;
	    else
	    {
	        if (url_filter_setting.appr_list.appr_url[i][appr_url_len - 1] == '/')
		   appr_url_len--;
		//DPRINTK("check_appr_list : index[%d] appr_url_len[%d]\n", i, appr_url_len);
	    }
            if (appr_url_len <= host_len)
	    {
	    	if (!memcmp(url_filter_setting.appr_list.appr_url[i], pHost, appr_url_len))
		{
		    //DPRINTK("check_appr_list : cmp host name, appr_url_len[%d] host_len[%d] appr_url[%d][%s]\n", appr_url_len, host_len, i, url_filter_setting.appr_list.appr_url[i]);
	    	    ret = URL_FILTER_PASS;
		    break;
		}
    	    }
	    else if (appr_url_len > host_len && (appr_url_len <= (host_len + url_len)))
	    {
	    	if (!memcmp(url_filter_setting.appr_list.appr_url[i], pHost, host_len) &&
		    !memcmp(url_filter_setting.appr_list.appr_url[i], pUrl, appr_url_len - host_len))
		{
		    //DPRINTK("check_appr_list : cmp host and url, appr_url_len[%d] host_len[%d] url_len[%d] appr_url[%d][%s]\n", appr_url_len, host_len, url_len, i, url_filter_setting.appr_list.appr_url[i]);
	    	    ret = URL_FILTER_PASS;
		    break;
	  	}
	    }
    	}
    }
    if (ret == URL_FILTER_BLOCK)
        {DPRINTK("check_appr_list : ret = URL_FILTER_BLOCK\n");}
    else
        {DPRINTK("check_appr_list : ret = URL_FILTER_PASS\n");}
    return ret;
}


void queue_mgt_dequeue(url_class_info *url_info)
{
    int status = URL_FILTER_PASS, rf = URL_FILTER_PASS, rw = URL_FILTER_PASS, blk_st = URL_WRS_BLOCK;

    if (!url_info)
    {
        DPRINTK("Error! url_info = NULL\n");
        return;
    }

	spin_lock_bh(&tm_buffer_queue);
    if(!url_buf_queue[url_info->queue_idx])
    {
        DPRINTK("Error! queue_mgt_dequeue = NULL\n");
        spin_unlock_bh(&tm_buffer_queue);
        return;
    }
    if(url_buf_queue[url_info->queue_idx]->buf_write_read != TM_BUF_READ)
	{DPRINTK("Error! queue_mgt_dequeue!= TM_BUF_READ\n");}

    if(!url_buf_queue[url_info->queue_idx]->url_filter_fin)
    {
        DPRINTK("Error! url_buf_queue[url_info->queue_idx]->url_filter_fin = NULL\n");
        spin_unlock_bh(&tm_buffer_queue);
        return;
    }

    if ((url_buf_queue[url_info->queue_idx]->url_filter_fin->apps_data) &&
	(((webfil_data_t *)url_buf_queue[url_info->queue_idx]->url_filter_fin->apps_data)->checked == 1))
    {
        if (url_buf_queue[url_info->queue_idx]->url_filter_info.skb)
	    kfree_skb(url_buf_queue[url_info->queue_idx]->url_filter_info.skb);

    	url_buf_queue[url_info->queue_idx]->url_filter_info.skb = NULL;
    	url_buf_queue[url_info->queue_idx]->url_filter_fin->fin_ip = NULL;
    	url_buf_queue[url_info->queue_idx]->url_filter_fin->fin_dp = NULL;
    	DPRINTK("queue_mgt_dequeue : apps_data->checked = 1, drop pkt!\n");
    }
    else
    {
    	if (url_info->status == 1)
    	{
		rf = check_uf_status(url_info);
		if (rf == URL_FILTER_PASS)
	    		rw = check_wrs_status(url_info->wrs_score);
		else
	    		blk_st = URL_BLOCK;

		if (rf == URL_FILTER_BLOCK || rw == URL_FILTER_BLOCK)
		{
	    		status = check_appr_list(url_buf_queue[url_info->queue_idx]->url_filter_info.host, url_buf_queue[url_info->queue_idx]->url_filter_info.url);
	    		if (rf == URL_FILTER_BLOCK && status == URL_FILTER_BLOCK)
	        		update_statisc(url_info->block_id, URL_FILTER_BLOCK);

	    		/*purpose     : 12638 author : Ben date : 2010-07-07*/
	    		/*description : Show log message*/
	    		uf_log(url_buf_queue[url_info->queue_idx]->url_filter_fin->url_filter_src,
	    			url_info->block_id, url_buf_queue[url_info->queue_idx]->url_filter_info.host, url_buf_queue[url_info->queue_idx]->url_filter_info.url, blk_st);
        	}
     	}

     	make_decision(url_buf_queue[url_info->queue_idx], status, blk_st);
     }

     if (url_buf_queue[url_info->queue_idx])
     {
     	if (url_buf_queue[url_info->queue_idx]->url_filter_fin)
     	{
		kfree(url_buf_queue[url_info->queue_idx]->url_filter_fin);
     	}
     	url_buf_queue[url_info->queue_idx]->url_filter_fin = NULL;

	kfree(url_buf_queue[url_info->queue_idx]);
     }

	 /* add by chihmou, show log if queue length down to safe level */
	 if((show_log_flag) && (current_queue_len == SHOW_LOG_LOW_LEVEL))
	{
		printk(KERN_ALERT "ProtectLink URL queue is empty now.\n");
		show_log_flag = 0;
	}

     DPRINTK("queue_mgt_dequeue : url_info->class_id[%d %d %d %d] url_info->status[%d] queue_idx[%d] current_queue_len[%d]\n",
     	url_info->class_id[0], url_info->class_id[1], url_info->class_id[2], url_info->class_id[3], url_info->status, url_info->queue_idx, current_queue_len);
    url_buf_queue[url_info->queue_idx] = NULL;
//sema4     
    if(current_queue_len >0)
    current_queue_len--;
	spin_unlock_bh(&tm_buffer_queue);
//<<
}

static int put_to_queue(struct sk_buff **pskb)
{
    fr_info_t 			*url_fin = NULL;
    url_filter_list_t 	*url_info = NULL;
    static int 			url_filter_queue_index = -1;
    int					tmp_ii=0;
    httpinfo_t htinfo;
    char *httpdata;	
    int payloadln = 0, httpdataln = 0;

#if 0
    for(tmp_ii=0; tmp_ii<DEFAULT_QUEUE_LEN; tmp_ii++)
    {
    	url_filter_queue_index = (url_filter_queue_index + 1) % DEFAULT_QUEUE_LEN;
    	if(url_buf_queue[url_filter_queue_index] == NULL)
    		break;
    }
#else
    for(url_filter_queue_index=0; url_filter_queue_index<DEFAULT_QUEUE_LEN; url_filter_queue_index++)
    {
    	if(url_buf_queue[url_filter_queue_index] == NULL)
    		break;
    }
#endif

#if 0
    if(DEFAULT_QUEUE_LEN <= tmp_ii)
#else
    if(DEFAULT_QUEUE_LEN <= url_filter_queue_index)
#endif
    {
	printk(KERN_EMERG "Error! put_to_queue DEFAULT_QUEUE_LEN should not equal to tmp_ii\n");
	return -1;	
    }

    url_fin = kmalloc(sizeof(fr_info_t), GFP_ATOMIC);
    if (!url_fin)
    {
    	DPRINTK("put_to_queue : Error : kos_alloc fail!\n");
	return -1;
    }

    url_info = kmalloc(sizeof(url_filter_list_t), GFP_ATOMIC);
    if (!url_info)
    {
        kfree(url_fin);
    	DPRINTK("put_to_queue : Error : kos_alloc fail!\n");
	return -1;
    }

    memset(url_fin, 0, sizeof(fr_info_t));
    memset(url_info, 0, sizeof(url_filter_list_t));
    if (get_http_info(pskb, &htinfo) < 1)
    {
        kfree(url_fin);
        kfree(url_info);
    	DPRINTK("put_to_queue : Error : get_http_info fail!\n");
        return -1;
    }

    if (htinfo.hostlen < MAX_URL_FILTER_HOST_LEN)
        memcpy(url_info->url_filter_info.host, htinfo.host, htinfo.hostlen);
    else
    {
        kfree(url_fin);
    	kfree(url_info);
    	DPRINTK("put_to_queue : host Len[%d] > MAX_URL_FILTER_HOST_LEN[%d]\n", strlen(htinfo.host), MAX_URL_FILTER_HOST_LEN);
	return -1;
    }

    if (htinfo.urllen < MAX_URL_FILTER_URL_LEN)
        memcpy(url_info->url_filter_info.url , htinfo.url, htinfo.urllen);
    else
    {
        kfree(url_fin);
    	kfree(url_info);
    	DPRINTK("put_to_queue : url Len[%d] > MAX_URL_FILTER_URL_LEN[%d]\n", strlen(htinfo.url), MAX_URL_FILTER_URL_LEN);
	return -1;
    }

    url_info->url_filter_fin = (void *)url_fin;
    url_info->url_filter_fin->fin_ip = (*pskb)->nh.iph;/**< Pointer to the IP header */
    struct tcphdr *tcph = (void *)((*pskb)->nh.iph) + (*pskb)->nh.iph->ihl * 4;
    httpdata = strstr((char *)tcph + tcph->doff*4, "\r\n\r\n");
    url_info->url_filter_fin->fin_dp = httpdata;/**< start of data past IP header */
    payloadln = (*pskb)->len - (*pskb)->nh.iph->ihl*4 - tcph->doff*4;
    httpdataln = payloadln - (httpdata - ((char *)tcph + tcph->doff*4)) - 4;
    url_info->url_filter_fin->fin_dlen = httpdataln;/**< length of data portion of packet */
    url_info->url_filter_fin->url_filter_src.s_addr = (*pskb)->orig_src_ip;/**< in_addr */
    url_info->url_filter_info.skb = (*pskb);
    url_info->url_filter_info.queue_idx = url_filter_queue_index;
    url_buf_queue[url_filter_queue_index] = url_info;
    url_buf_queue[url_filter_queue_index]->buf_write_read = TM_BUF_WROTE;
    
//sema4     
    current_queue_len++;
//<<    

//max 0713
//sema4     
    current_queue_unhandle_len++;
//<<

    if (current_queue_len == 1)
    {
	wake_up_interruptible(&dev_ptr->mesg_wq);
    }

    return 0;
}

int isAppr_ip(uint32_t src_ip)
{
    int i;

    if (url_filter_setting.appr_ip_enable==1)
    {
        for (i = 0; i < MAX_APPR_IP_LIST; i++)
        {
	    if (url_filter_setting.appr_list.from_ip[i] != 0)
	    {
    	    	if (src_ip >= url_filter_setting.appr_list.from_ip[i] &&
	     	    src_ip <= url_filter_setting.appr_list.to_ip[i])
    	    	{
		    DPRINTK("isAppr_ip : appr_list[%d] src_ip[%x], appr_list from_ip[%x] -  to_ip[%x]\n",
		    	i, src_ip, url_filter_setting.appr_list.from_ip[i], url_filter_setting.appr_list.to_ip[i]);
		    return 1;
    	    	}
	    }
	    else
	        break;
    	}
    }

    return 0;
}

int queue_mgt_enqueue(struct sk_buff **pskb)
{
    int ret = 0;
    unsigned int current_queue_len_tmp = 0;

    if (isAppr_ip((*pskb)->orig_src_ip))
    {
	    //current_queue_len--;
	    return URL_FILTER_UNHANDLE;//pass
    }

	spin_lock_bh(&tm_buffer_queue);
    current_queue_len_tmp = current_queue_len + 1;
    if (current_queue_len_tmp >= DEFAULT_QUEUE_LEN)
    {
        DPRINTK("queue_mgt_enqueue : current_queue_len_tmp[%d] >= DEFAULT_QUEUE_LEN[%d]\n", current_queue_len_tmp, DEFAULT_QUEUE_LEN);
        if (url_filter_setting.overflow_control == 0)//block
	{
//	    (*pskb)->nh.iph->saddr = (*pskb)->orig_src_ip;
	    url_tmufe_filter_block(*pskb, URL_OVERFLOW_BLOCK, url_filter_debug.url_filter_debug_mode);
	spin_unlock_bh(&tm_buffer_queue);
	    return URL_FILTER_DROP;
	}
	else
	{
	spin_unlock_bh(&tm_buffer_queue);
	    return URL_FILTER_UNHANDLE;//pass
	}
    }
    else
    {
	ret = put_to_queue(pskb);

	/* add by chihmou, show log if queue length up to warning level */
	if((!show_log_flag) && (current_queue_len == SHOW_LOG_HIGH_LEVEL))
	{
		printk(KERN_ALERT "Warning! ProtectLink URL queue is almost full (80 percent) now.  URL blocking might occur.\n");
		show_log_flag = 1;
	}

	if(ret < 0)
	{
	    DPRINTK("queue_mgt_enqueue : put_to_queue return ret[%d], this URL filter don't handle \n", ret);
	spin_unlock_bh(&tm_buffer_queue);
	    return URL_FILTER_UNHANDLE;
	}
   }
	spin_unlock_bh(&tm_buffer_queue);

    return URL_FILTER_HANDLE;
}

static int url_filter_chardev_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
	url_class_info *user_url_query_info = (url_class_info *)arg;
	url_class_info kernel_url_query_info[1];

	memset(&kernel_url_query_info[0], 0, sizeof(url_class_info));
	if (copy_from_user(&kernel_url_query_info[0], user_url_query_info, sizeof(url_class_info)))
	{
		printk(KERN_EMERG "Error! copy_from_user fail\n");
		return -1;
	}
	queue_mgt_dequeue(&kernel_url_query_info[0]);
	if (current_queue_unhandle_len > 0 )
	{
	    wake_up_interruptible(&dev_ptr->mesg_wq);
	}

	switch(cmd){
        case URL_QUERY_RESPONSE:
	    break;
	default:
	    //DPRINTK("url_filter_chardev_ioctl invalid para[%d]!!\n", cmd);
	    return cmd;
	}
	return 0;
}
static int url_filter_chardev_read(struct file *file, char *buf, size_t count, loff_t *f_pos)
{
    int ret = 0;
    url_filter_chardev_t *dev = (url_filter_chardev_t *)file->private_data;
    static int 		url_filter_read_index = -1;
    int			tmp_ii=0;

#if 0
    for(tmp_ii=0; tmp_ii<DEFAULT_QUEUE_LEN; tmp_ii++)
    {
    	url_filter_read_index = (url_filter_read_index + 1) % DEFAULT_QUEUE_LEN;
    	if(url_buf_queue[url_filter_read_index] != NULL && 
    	   url_buf_queue[url_filter_read_index]->buf_write_read == TM_BUF_WROTE)
    	   {
//max 0713
//sema4     
		spin_lock_bh(&tm_buffer_unhandle_counter);     
		if (current_queue_unhandle_len>0)
		current_queue_unhandle_len--;
		spin_unlock_bh(&tm_buffer_unhandle_counter);     
//<<
		url_buf_queue[url_filter_read_index]->buf_write_read = TM_BUF_READ;
		break;
    	   }
    }
#else
	spin_lock_bh(&tm_buffer_queue);
    for(url_filter_read_index=0; url_filter_read_index<DEFAULT_QUEUE_LEN; url_filter_read_index++)
    {
    	if(url_buf_queue[url_filter_read_index] != NULL && 
    	   url_buf_queue[url_filter_read_index]->buf_write_read == TM_BUF_WROTE)
    	   {
		if (current_queue_unhandle_len>0)
		current_queue_unhandle_len--;
		url_buf_queue[url_filter_read_index]->buf_write_read = TM_BUF_READ;
		break;
    	   }
    }
#endif
#if 0
    if(DEFAULT_QUEUE_LEN <= tmp_ii)
#else
    if(DEFAULT_QUEUE_LEN <= url_filter_read_index)
#endif
    {	
	spin_unlock_bh(&tm_buffer_queue);
    printk(KERN_EMERG "url_filter_chardev_read => reset_url_buf_queue\n");
	reset_url_buf_queue();
	return -1;
    }
//--> chihmou modified 2008-03-05. Attempt to solve crash problem
#if 0
    if(!url_buf_queue[url_filter_read_index])
    {
	printk("Null pointer found. index[%d]/[%d] => reset queue\n", url_filter_read_index, current_queue_len);
	reset_url_buf_queue();
	printk("reset queue finish. index[%d]/[%d]\n", url_filter_read_index, current_queue_len);
	//spin_unlock_bh(&tm_buffer_queue);
	return -1;
  }
#endif
//<--
    if (current_queue_unhandle_len >= 0 && url_buf_queue[url_filter_read_index] && url_buf_queue[url_filter_read_index]->url_filter_fin)
    {
	ret = copy_to_user((char *)buf, (char *)&url_buf_queue[url_filter_read_index]->url_filter_info, sizeof(url_class_info));
    }
//--> Jerry modified 2008-03-02. Moniter the crash problem
    else
    {
	/* print first 100 faults */
	if(wrong_queue_cnt <100)
	{
	    printk(KERN_EMERG "Error! wrong_queue_cnt[%d] current_queue_len[%d] current_queue_unhandle_len[%d]\n",wrong_queue_cnt,current_queue_len,current_queue_unhandle_len);
	    wrong_queue_cnt++;
	}
    }
//<--
	spin_unlock_bh(&tm_buffer_queue);
    return ret;
}



static unsigned int url_filter_chardev_select(struct file *file,struct poll_table_struct *wait)
{
    url_filter_chardev_t *dev = (url_filter_chardev_t *)file->private_data;

    poll_wait(file, &dev->mesg_wq, wait);

    if(current_queue_unhandle_len)
    {
	//in heavy URL loading test, if current_queue_unhandle_len minus fail, 
	//it will return (POLLIN | POLLRDNORM) forever, and let urlfilterd into non-ending cycle and use all CPU
	//(even all function are still ok)
	spin_lock_bh(&tm_buffer_reset);
	repoll++;
	spin_unlock_bh(&tm_buffer_reset);
	if (repoll>100000)
	{
		printk(KERN_EMERG "url_filter_chardev_select : repoll > 100000\n");
	spin_lock_bh(&tm_buffer_reset);
		repoll=0;
	spin_unlock_bh(&tm_buffer_reset);
		reset_url_buf_queue();
	}
	return (POLLIN | POLLRDNORM);
    }
	spin_lock_bh(&tm_buffer_reset);
	repoll=0;
	spin_unlock_bh(&tm_buffer_reset);
    return 0;
}

static unsigned int url_filter_chardev_open(struct inode *inode, struct file *file)
{
    url_filter_chardev_t *dev;

    if (!(dev = kmalloc(sizeof(url_filter_chardev_t),GFP_KERNEL)))
    {
	DPRINTK("url filter memory allocation failed\n");
	return -1;
    }

    memset(dev, 0, sizeof(url_filter_chardev_t));
    init_waitqueue_head(&dev->mesg_wq);
    file->private_data = dev;
    dev_ptr = dev;
    return 0;
}


int url_filter_chardev_init(void);
int url_filter_chardev_uninit(void);

static void restart_url_filter_module(void)
{
	url_filter_chardev_uninit();
	url_filter_chardev_init();
}

static int url_filter_chardev_close(struct inode *inode, struct file *file)
{
    if (file->private_data)
    	kfree(file->private_data);
    return 0;
}

static struct file_operations urlfilter_chardev_fops = {
	.owner   = THIS_MODULE,
	.read    = url_filter_chardev_read,
	.poll    = url_filter_chardev_select,
	.ioctl   = url_filter_chardev_ioctl,
	.open    = url_filter_chardev_open,
	.release = url_filter_chardev_close
};

static struct class *urlfilter_class;

int url_filter_chardev_init(void)
{
    int i;

    /* Attempt to register the URL control device */
    if ((register_chrdev(URLFILTER_CHRDEV_MAJOR, URLFILTER_CHRDEV_NAME, &urlfilter_chardev_fops)) < 0) 
    {
	printk(KERN_EMERG "register_urlfilter_chrdev failed\n");
//	return -EIO;
    }

    /* create dev under /dev for send/recv data between kernel and urlfilter daemon */
//    if (mknod("/dev/tmufd", S_IFCHR|660, makedev(245,0))==-1) {
//	printk(KERN_EMERG "mknod tmufd return fail");
//    }
    /* <= move to pkg_makefiles */

    url_filter_enqueue = queue_mgt_enqueue;
    restart_url_filter_mod = restart_url_filter_module;

    for (i = 0; i < MAX_CLASS_NUMBER; i++)
        url_filter_statics.url_class_id_pkt_cnt[i].pkt_drop = 0;
    url_filter_debug.url_filter_debug_mode = 0;

    return 1;
}

int url_filter_chardev_uninit(void)
{
    url_filter_enqueue = NULL;
    restart_url_filter_mod = NULL;
    
    reset_url_buf_queue();

    unregister_chrdev(URLFILTER_CHRDEV_MAJOR, URLFILTER_CHRDEV_NAME);

    return 0;
}
#ifdef MODULE
    module_init(url_filter_chardev_init);
    module_exit(url_filter_chardev_uninit);
#endif
