#ifndef _URL_FILTER_DEF_H_
#define _URL_FILTER_DEF_H_

#include "nk_def.h"
#define URLFILTER_CHRDEV_MAJOR 	245
#define URLFILTER_CHRDEV_NAME 	"tmufd"

#define	URL_QUERY_RESPONSE	1
#define	MAX_URL_FILTER_CATE_NUMBER	4

typedef struct webfil_data_t {
    unsigned int checked;
} webfil_data_t;

typedef struct {
    unsigned char url[MAX_URL_FILTER_URL_LEN + 1];	// packet url from NAT/Firewall
    unsigned char host[MAX_URL_FILTER_HOST_LEN + 1];	// packet host from NAT/Firewall
    unsigned int cate_number;
    unsigned int class_id[MAX_URL_FILTER_CATE_NUMBER];// url belong to which class id
    unsigned int block_id;
    int status;				// this class id is block or pass or timeout
    int wrs_score;
    struct sk_buff *skb;
    unsigned int queue_idx;
    int is_last;
} url_class_info;

typedef struct {
    unsigned char category[MAX_CATEGORY_LEN];
    unsigned int bz_hour;	//0:disable, 1:enable
    unsigned int leisure_hour;	//0:disable, 1:enable
} class_id_setting_t;

typedef struct {
    unsigned char bz_day[URL_FILTER_WEEK];	//[0]~[6]:Monday~Sunday, [0]=1(bz);[0]=0(nonbz)
    unsigned int enable_all_day;		//0:disable, 1:enable
    unsigned int enable_morn;			//0:disable, 1:enable
    unsigned int bz_morn_from;
    unsigned int bz_morn_to;
    unsigned int enable_aft;			//0:disable, 1:enable
    unsigned int bz_aft_from;
    unsigned int bz_aft_to;
    int gmt_offset;
    /*purpose     : 0013213 author : Ben date : 2010-08-19*/
    /*description : add data for day_life_saving*/
    int enable_daylight_saving;			//0:disable, 1:enable
    int daylight_saving_smonth;
    int daylight_saving_sday;
    int daylight_saving_emonth;
    int daylight_saving_eday;
} class_bz_time_t;

typedef struct {
    unsigned int day;
    unsigned int hour;
} url_filter_time_t;

typedef struct {
    unsigned int time_out_sec;
    unsigned int queue_len;
    unsigned int cache_size;
} url_setting_t;

typedef struct {
    unsigned char 	appr_url[MAX_APPR_URL_LIST][MAX_APPR_URL_LEN];
    uint32_t 		from_ip[MAX_APPR_IP_LIST];
    uint32_t 		to_ip[MAX_APPR_IP_LIST];
} url_filter_appr_list_t;

typedef struct {
    class_id_setting_t 		class_id_setting[MAX_CLASS_NUMBER];
    url_setting_t 		url_setting;
    unsigned int		filter_enable;
    unsigned int 		wrs_enable;
    unsigned int 		wrs_level;
    unsigned int 		appr_url_enable;
    unsigned int 		appr_ip_enable;
    unsigned int 		output_block;
    unsigned int		overflow_control;
    class_bz_time_t     	class_bz_time;
    url_filter_appr_list_t	appr_list;
    unsigned char model[20];
} url_filter_setting_t;

typedef struct {
    unsigned int 		lic_valid;
} url_filter_lic_t;

typedef struct {
    /*purpose     : 0013117 author : Ben date : 2010-08-06*/
    /*description : copy_from_user or copy_to_user will transfer long to int, so variable should set as int*/
    unsigned int pkt_drop;
} class_id_pkt_cnt_t;

typedef struct {
    unsigned int 		url_filter_debug_mode;
} url_filter_debug_t;

typedef struct {
    class_id_pkt_cnt_t url_class_id_pkt_cnt[MAX_CLASS_NUMBER];
} url_filter_statics_t;

#endif
