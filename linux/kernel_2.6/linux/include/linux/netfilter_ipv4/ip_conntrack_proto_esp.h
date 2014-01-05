#ifndef _CONNTRACK_PROTO_ESP_H
#define _CONNTRACK_PROTO_ESP_H


/** Constant Define **/
// #define NK_ESP_DEBUG
#ifdef NK_ESP_DEBUG
    #define nk_esp_print(arg...) do { if ( nk_esp_debug ){ printk(KERN_EMERG arg); } else {} } while(0)
    #define DUMP_TUPLE_ESP(x)   do { if ( nk_esp_debug ){ printk(KERN_EMERG "ESP ALG: %s: [%u.%u.%u.%u]/[%u] -> [%u.%u.%u.%u]/[%u]\n", __func__, \
                                    NIPQUAD((x)->src.ip),(x)->src.u.esp.spi,NIPQUAD((x)->dst.ip),(x)->dst.u.esp.spi); } else {} } while(0)
#else
    #define nk_esp_print(arg...)
    #define DUMP_TUPLE_ESP(x)
#endif

#define ESP_TIMEOUT (10*HZ)
#define ESP_STREAM_TIMEOUT (60*HZ)

#define ESP_UNREPLIEDDNS_TIMEOUT (1*HZ)

#define IPSEC_FREE     0
#define IPSEC_INUSE    1
#define MAX_PORTS      256
#define TEMP_SPI_START 1500

#define ESP_MAJOR_NUM  240
#define ESP_MINOR_NUM  1

#define NK_ESP_PRINT_TABLE      _IOWR(ESP_MAJOR_NUM, 1, int)
#define NK_ESP_DEL_TABLE        _IOWR(ESP_MAJOR_NUM, 2, int)
#define NK_ESP_SET_DEBUG        _IOWR(ESP_MAJOR_NUM, 3, int)


/** Structure Declare  **/
/* this is part of ip_conntrack */
struct ip_ct_esp {
    unsigned int stream_timeout;
    unsigned int timeout;
};

enum{
    Esp_Pkt_To_Tuple_0,
    Esp_New_1,
    Esp_Packet_2,
    Esp_Packet_3
};

struct _esp_table {
        u_int32_t l_spi;
        u_int32_t r_spi;
        u_int32_t l_ip;
        u_int32_t r_ip;
        u_int32_t timeout;
        u_int16_t tspi;
        u_int32_t seq;
        int       inuse;
};

struct esphdr {
    u_int32_t spi;
    u_int32_t seq;
};

/** Function Prototype **/

#endif
