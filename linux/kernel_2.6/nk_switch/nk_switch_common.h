#ifndef __NK_SWITCH_COMMON_H__
#define __NK_SWITCH_COMMON_H__


#if defined(__KERNEL__)
#include <linux/kernel.h>
#include <linux/module.h>
#include <asm-mips/mipsregs.h>  /*  For OCTEON_IS_MODEL */
#include "cvmx.h"
#include "cvmx-csr.h"
#ifdef CONFIG_NK_DYNAMIC_PORT_NUM
#include <linux/dynamic_port_num.h>
#endif
#endif



#define NK_SWITCH_LAN   0
#define NK_SWITCH_WAN   1
#define NK_SWITCH_LAN2  2
#define NK_SWITCH_WAN2  3


/* For Set Port Status */
#define NK_SWITCH_PORT_DISABLE      0
#define NK_SWITCH_PORT_ENABLE       1
#define NK_SWITCH_PORT_1000M        0x2
#define NK_SWITCH_PORT_100M         0x1
#define NK_SWITCH_PORT_10M          0x0
#define NK_SWITCH_PORT_FULL         0x1
#define NK_SWITCH_PORT_HALF         0x0

#define NK_SWITCH_SERI_LOW_BIT_FIRST  0x0
#define NK_SWITCH_SERI_HIGH_BIT_FIRST 0x1

typedef struct {
    uint32_t sw_reset;
    uint32_t sw_reset_act;
    uint32_t reset_bt;
    uint32_t reset_bt_act;

    uint32_t diag;
    uint32_t diag_act;
    uint32_t dmz;
    uint32_t dmz_act;
    uint32_t dmz_seri;      /* Is DMZ Serial, at Some Model DMZ is not a GPIO Pin */

    uint32_t cs_lan;
    uint32_t cs_lan_st;
    uint32_t cs_wan;
    uint32_t cs_wan_st;

    uint32_t clk;
    uint32_t sda;
    uint32_t oda;

    uint32_t    seri_clk;      /* Serial Clock, it maybe control the WAN Connect LED */
    uint32_t    seri_dat;      /* Serial Data, it maybe control the WAN Connect LED */
    uint32_t    seri_no;       /* Serial Number */
    uint32_t    seri_bit_pri;  /* Bit priority, High Bit First or Low Bit First */

    uint32_t    usb_clk;       /* USB LED Clock */
    uint32_t    usb_dat;       /* USB LED Data */
} nk_gpio_t;

typedef struct {
    uint32_t *lan2sw;/* LAN Front Port to Switch Port */
    uint32_t *wan2sw;/* WAN Front Port to Switch Port */
} nk_portmap_t;

typedef struct {
    uint32_t port;
    uint32_t index;
    uint32_t vid;
    uint32_t member;
} nk_switch_vlan_table_t;

typedef struct {
    uint32_t dir;
    uint32_t fport;
    uint32_t sport;
    uint32_t enable;
    uint32_t an;
    uint32_t speed;
    uint32_t duplex;
    uint32_t link;
    uint32_t priority;
    uint32_t inited;

    uint32_t recv_packet_cnt;
    uint32_t recv_byte_cnt;
    uint32_t tran_packet_cnt;
    uint32_t tran_byte_cnt;
    uint32_t collision_cnt;
    uint32_t error_cnt;

    nk_switch_vlan_table_t table;
} nk_switch_port_status_t;

/* purpose : 0013292 author : incifer date : 2010-10-06 */
/* description : hw mac clone                                        */
/*               mode : 0 : disable                                  */
/*                      x : wanx use hw macclone                     */
/*               mac : macclone mac                                  */
typedef struct
{
    uint8_t  mode;
    uint64_t mac;
    uint32_t sport;
} nk_switch_hw_macclone_t;

typedef struct
{
/* purpose : 0013292 author : incifer date : 2010-10-06 */
/* description : hw macclone                            */
/*               0 : not support                        */
/*               > 0 : hw macclone rule numbers         */
    uint8_t hw_macclone;
} nk_switch_opt_t;

typedef struct {
    uint32_t type;
    uint32_t switchnum;
    uint32_t lcpu_port;
    uint32_t wcpu_port;
    uint32_t mii_speed;
    uint32_t mii_clk;
    uint32_t usb_type;/* USB LED Control: 1: SERI MIX USB, share Clock */
    uint32_t wanled_type;/* WAN LED Control: 1: WAN <-> LAN LED Control, and No Connect LED, ex RV016 */
    nk_switch_opt_t opt;

    nk_gpio_t *gpio;
    nk_portmap_t *portmap;
    nk_switch_vlan_table_t *vtable;
    nk_switch_port_status_t *port_status[4];

    void ( *read ) ( uint32_t paddr, uint32_t raddr, uint64_t *rdata, uint32_t reserve );
    void ( *write ) ( uint32_t paddr, uint32_t raddr, uint64_t rdata, uint32_t reserve );
    void ( *pread ) ( uint32_t paddr, uint32_t raddr, uint64_t *rdata, uint32_t reserve );
    void ( *pwrite ) ( uint32_t paddr, uint32_t raddr, uint64_t rdata, uint32_t reserve );

    void ( *init_switch ) ( void );
    void ( *init_vlan ) ( void );

    void ( *set_vlan ) ( nk_switch_vlan_table_t *vtable );
    void ( *print_vlan ) ( void );

    void ( *print_pqos ) ( void );
    void ( *get_link_status ) ( uint32_t fport, uint32_t *status );
    void ( *set_port_disable ) ( uint32_t sport, uint32_t status );
    void ( *get_port_status ) ( nk_switch_port_status_t *status );
    void ( *set_port_status ) ( nk_switch_port_status_t status );

    /* The Second Switch */
    nk_switch_vlan_table_t *vtable2;

    void ( *read2 ) ( uint32_t paddr, uint32_t raddr, uint64_t *rdata, uint32_t reserve );
    void ( *write2 ) ( uint32_t paddr, uint32_t raddr, uint64_t rdata, uint32_t reserve );
    void ( *pread2 ) ( uint32_t paddr, uint32_t raddr, uint64_t *rdata, uint32_t reserve );
    void ( *pwrite2 ) ( uint32_t paddr, uint32_t raddr, uint64_t rdata, uint32_t reserve );

    void ( *set_vlan2 ) ( nk_switch_vlan_table_t *vtable );
    void ( *print_vlan2 ) ( void );

    void ( *print_pqos2 ) ( void );
    void ( *get_link_status2 ) ( uint32_t fport, uint32_t *status );
    void ( *set_port_disable2 ) ( uint32_t sport, uint32_t status );
    void ( *get_port_status2 ) ( nk_switch_port_status_t *status );
    void ( *set_port_status2 ) ( nk_switch_port_status_t status );

    void ( *hw_macclone ) ( const nk_switch_hw_macclone_t *clone );
    void ( *print_hw_macclone ) ( void );

} nk_switch_t;


#endif

