#ifndef __SW_TEMPLATE_SW_CONTROL_H__
#define __SW_TEMPLATE_SW_CONTROL_H__


#ifdef NK_SWITCH_DEBUG_ACL
	#define nk_switch_acl_print(arg...) printk(KERN_EMERG arg)
#else
	#define nk_switch_acl_print(arg...)
#endif

#define SW_TEMPLATE_CS_LAN		0
#define SW_TEMPLATE_CS_WAN		1

#define SW_TEMPLATE_RX			0
#define SW_TEMPLATE_TX			1

#define SW_TEMPLATE_DISABLE		0
#define SW_TEMPLATE_ENABLE		1

#define SW_TEMPLATE_PORT_0_MASK			1<<0
#define SW_TEMPLATE_PORT_1_MASK			1<<1
#define SW_TEMPLATE_PORT_2_MASK			1<<2
#define SW_TEMPLATE_PORT_3_MASK			1<<3
#define SW_TEMPLATE_PORT_4_MASK			1<<4
#define SW_TEMPLATE_PORT_5_MASK			1<<5
#define SW_TEMPLATE_PORT_6_MASK			1<<6
#define SW_TEMPLATE_PORT_7_MASK			1<<7
#define SW_TEMPLATE_PORT_IMP_MASK		1<<8
#define SW_TEMPLATE_VLAN_CMD_START		0x80
#define SW_TEMPLATE_VLAN_CMD_READ		0x1
#define SW_TEMPLATE_VLAN_CMD_WRITE		0x0


/**
 *	port status flag
 */
#define SW_TEMPLATE_STATUS_1000Mbs		0x2
#define SW_TEMPLATE_STATUS_100Mbs		0x1
#define SW_TEMPLATE_STATUS_10Mbs		0x0
#define SW_TEMPLATE_STATUS_FULL			0x1
#define SW_TEMPLATE_STATUS_HALF			0x0
#define SW_TEMPLATE_STATUS_ENABLE		0x1
#define SW_TEMPLATE_STATUS_DISABLE		0x0
#define SW_TEMPLATE_STATUS_UP			0x1
#define SW_TEMPLATE_STATUS_DOWN			0x0


#define SW_TEMPLATE_SERI_LED			0xFF
#define SW_TEMPLATE_SERI_LOW_BIT_FIRST	0x0
#define SW_TEMPLATE_SERI_HIGH_BIT_FIRST	0x1

#define SW_TEMPLATE_DIAG_LED			0x7
#define SW_TEMPLATE_DMZ_LED				0x8

#define SW_TEMPLATE_LED_ON				0x1
#define SW_TEMPLATE_LED_OFF				0x2

#define SW_TEMPLATE_LED_LOW_ACTIVE		0x0
#define SW_TEMPLATE_LED_HIGH_ACTIVE		0x1

#define SW_TEMPLATE_SW_TYPE_BCM539X		0x1
#define SW_TEMPLATE_SW_TYPE_BCM53115	0x2
#define SW_TEMPLATE_SW_TYPE_RTL8306		0x3


typedef union {
	uint64_t u64;
	struct
	{
		uint64_t unuse		:32; 
		uint64_t reserved	:10;
		uint64_t mspt_index	: 4;
		uint64_t untag_map	: 9;
		uint64_t fwd_map	: 9;
	}s;
}Sw_Template_VLAN_Table_Entry_t;


/**
	ARL Table Data Index Structure -- Incifer 2009/02
	Sync File: getlink.h, switch.h, sw_template_sw_control.h
**/
typedef struct {
	uint64_t mac_index;
	uint32_t vid_index;
	uint32_t data_value;
	int is_lan;
}Sw_Template_ARL_Data_t;


typedef struct {
	uint32_t cs_lan;/* Switch chip select GPIO pin */
	uint32_t cs_lan_state;/* What is the state when cs to Lan */
	uint32_t cs_wan;
	uint32_t cs_wan_state;
	uint32_t cs_share;/* Is cs_lan pin shares with cs_wan */
	uint32_t sw_rst;/* Software Reset */
	uint32_t rst_bt;/* Reset Button */
	uint32_t diag;/* Diag LED */
	uint32_t diag_active;
	uint32_t dmz;/* DMZ LED */
	uint32_t dmz_active;
	uint32_t dmz_seri;
	uint32_t seri_clk;/* Serial Clock, it maybe control the WAN Connect LED */
	uint32_t seri_dat;/* Serial Data, it maybe control the WAN Connect LED */
	uint32_t seri_no;/* Serial No. */
	uint32_t seri_bit_pri;/* Bit priority */
	uint32_t sw_clk;/* Switch Clock */
	uint32_t sw_sda;/* Switch Data IN, CPU(Output) */
	uint32_t sw_oda;/* Switch Data Out, CPU(Input) */
	uint32_t usb_clk;/* USB LED Clock */	
	uint32_t usb_dat;/* USB LED Data */	
}Sw_Template_Sw_GPIO_t;

typedef struct {
	uint32_t *lan2sw;
	uint32_t *wan2sw;
}Sw_Template_Sw_Port_Map_t;

typedef struct _Sw_Template_Sw_Func_t {
	void		( *cs )					( Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir );
	uint32_t	( *frontportmap )		( Sw_Template_Sw_Port_Map_t *port_map_t, uint32_t dir, uint32_t fport );

	int			( *read )				( Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t page, uint32_t addr, uint64_t *value, uint32_t reserve );
	int			( *write )				( Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t page, uint32_t addr, uint64_t value, uint32_t reserve );
	int			( *phy_read )			( Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t page, uint32_t addr, uint64_t *value, uint32_t reserve );
	int			( *phy_write )			( Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t page, uint32_t addr, uint64_t value, uint32_t reserve );

	void		( *get_link_status )	( struct _Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t *status );
	void		( *get_speed_status )	( struct _Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t *status );
	void		( *get_duplex_status )	( struct _Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t *status );

	uint64_t	( *get_mib_byte_cnt )		( struct _Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t type, uint32_t sport );
	uint64_t	( *get_mib_broadcast_pkts )	( struct _Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t type, uint32_t sport );
	uint64_t	( *get_mib_multicast_pkts )	( struct _Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t type, uint32_t sport );
	uint64_t	( *get_mib_unicast_pkts )	( struct _Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t type, uint32_t sport );
	uint64_t	( *get_mib_collision_pkts )	( struct _Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t type, uint32_t sport );
	uint64_t	( *get_mib_error_pkts )		( struct _Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t type, uint32_t sport );

	void		( *set_port_status )	( struct _Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, Sw_Template_Sw_Port_Map_t *port_map_t, uint32_t dir, struct PortStatusNStatistics status );
	void    	( *get_port_status )	( struct _Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, Sw_Template_Sw_Port_Map_t *port_map_t, uint32_t dir, struct PortStatusNStatistics *status );

	void		( *set_port_vlan )		( uint32_t model, struct _Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, Sw_Template_Sw_Port_Map_t *port_map_t, uint32_t dir, uint32_t member, uint32_t fport );
	void		( *set_vlan_group )		( uint32_t model, struct _Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t ggroup_index, Sw_Template_VLAN_Table_Entry_t data );

	void		( *set_led )			( uint32_t model, struct _Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, Sw_Template_Sw_Port_Map_t *port_map_t, uint32_t type, uint32_t state );

	void		( *mii_control )		( uint32_t model, struct _Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t opt );

	void		( *set_mirror )			( uint32_t model, struct _Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, Sw_Template_Sw_Port_Map_t *port_map_t, uint32_t fport );

	void		( *set_static_mac )		( uint32_t model, struct _Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, Sw_Template_ARL_Data_t arl_data );

	int			switch_type;
}Sw_Template_Sw_Func_t;


/* Sw_Template_Sw_Func for BCM5397/BCM5398 Series */
#define SW_TEMPLATE_BCM_PHYAD			0x1E

typedef union
{
    uint16_t u16;
    struct
    {
        uint16_t page_num              : 8;
        uint16_t reserved              : 7;
        uint16_t A                     : 1;
    } s;
} Sw_Template_phyreg16_t;

typedef union
{
    uint16_t u16;
    struct
    {
        uint16_t reg_addr              : 8;
        uint16_t reserved              : 6;
        uint16_t op                    : 2;
    } s;
} Sw_Template_phyreg17_t;

typedef union
{
    uint64_t u64;
    struct
    {
        uint64_t reg27              : 16;
        uint64_t reg26              : 16;
        uint64_t reg25              : 16;
	uint64_t reg24              : 16;
    } s;
} Sw_Template_result_reg_t;


Sw_Template_Sw_Func_t *G_Sw_Func;
Sw_Template_Sw_GPIO_t *G_Sw_GPIO;
Sw_Template_Sw_Port_Map_t *G_Sw_Port_Map;

uint32_t Sw_Template_LED_Seri_Old_State=0xFFFFFFFF;

/**
	LAN FRONT 2 SWITCH: LANX map to Switch Port
	WAN FRONT 2 SWITCH: Left->Right, Up->Down
**/
uint32_t SW_TEMPLATE_LAN_FRONT_2_SWITCH_005[4] = {0,1,2,3};
uint32_t SW_TEMPLATE_WAN_FRONT_2_SWITCH_005[4] = {0,1,2,3};
uint32_t SW_TEMPLATE_LAN_FRONT_2_SWITCH_363[2] = {0,1};
uint32_t SW_TEMPLATE_WAN_FRONT_2_SWITCH_363[2] = {2,3};
uint32_t SW_TEMPLATE_LAN_FRONT_2_SWITCH_650[5] = {0,1,2,3,4};
uint32_t SW_TEMPLATE_WAN_FRONT_2_SWITCH_650[5] = {0,1,2,3,4};
uint32_t SW_TEMPLATE_LAN_FRONT_2_SWITCH_1450[5]={4,3,2,1,0};
uint32_t SW_TEMPLATE_WAN_FRONT_2_SWITCH_1450[8]={1,3,5,7,0,2,4,6};
uint32_t SW_TEMPLATE_LAN_FRONT_2_SWITCH_3000[8]={1,3,5,7,0,2,4,6};
uint32_t SW_TEMPLATE_WAN_FRONT_2_SWITCH_3000[8]={1,3,5,7,0,2,4,6};
uint32_t SW_TEMPLATE_LAN_FRONT_2_SWITCH_DEF[8]={1,3,5,7,0,2,4,6};
uint32_t SW_TEMPLATE_WAN_FRONT_2_SWITCH_DEF[4+1]={0,1,2,3,4};


/* Template, for reference */
#if 0
Sw_Template_Sw_GPIO_t Sw_Template_Sw_GPIO = {
	.cs_lan = ,
	.cs_lan_state = ,
	.cs_wan = ,
	.cs_wan_state = ,
	.cs_share = ,
	.sw_rst = ,
	.rst_bt = ,
	.diag = ,
	.dmz = ,
	.seri_clk = ,
	.seri_dat = ,
	.sw_clk = ,
	.sw_sda = ,
	.sw_oda = ,
};
#endif

Sw_Template_Sw_GPIO_t Sw_Template_Sw_GPIO_005 = {
	.cs_lan = 2,
	.cs_lan_state = 0,
	.cs_wan = 2,
	.cs_wan_state = 1,
	.cs_share = 1,

	.sw_rst = 1,
	.rst_bt = 16,

	.diag = 12,
	.diag_active = SW_TEMPLATE_LED_HIGH_ACTIVE,
	.dmz = 13,/* 30 means no this pin */
	.dmz_active = SW_TEMPLATE_LED_HIGH_ACTIVE,
	.dmz_seri = 0,
	.seri_clk = 14,
	.seri_dat = 15,
	.seri_no = 8,
	.seri_bit_pri = SW_TEMPLATE_SERI_LOW_BIT_FIRST,

	.sw_clk = 5,
	.sw_sda = 6,
	.sw_oda = 6,
	
	.usb_clk = 0,
	.usb_dat = 3,	
};

Sw_Template_Sw_GPIO_t Sw_Template_Sw_GPIO_363 = {
	.cs_lan = 30,
	.cs_lan_state = 0,
	.cs_wan = 30,
	.cs_wan_state = 1,
	.cs_share = 1,

	.sw_rst = 1,
	.rst_bt = 16,

	.diag = 12,
	.diag_active = SW_TEMPLATE_LED_HIGH_ACTIVE,
	.dmz = 13,/* 30 means no this pin */
	.dmz_active = SW_TEMPLATE_LED_HIGH_ACTIVE,
	.dmz_seri = 1,/* it has no DMZ LED */
	.seri_clk = 14,
	.seri_dat = 15,
	.seri_no = 8,
	.seri_bit_pri = SW_TEMPLATE_SERI_LOW_BIT_FIRST,

	.sw_clk = 5,
	.sw_sda = 6,
	.sw_oda = 6,
	
	.usb_clk = 0,
	.usb_dat = 3,		
};

Sw_Template_Sw_GPIO_t Sw_Template_Sw_GPIO_650 = {
#if 1
	/* 4 Layer PCB */
	.cs_lan = 2,
	.cs_lan_state = 0,
	.cs_wan = 2,
	.cs_wan_state = 1,
	.cs_share = 1,

	.sw_rst = 1,
	.rst_bt = 16,

	.diag = 12,
	.diag_active = SW_TEMPLATE_LED_LOW_ACTIVE,
	.dmz = 13,/* 30 means no this pin */
	.dmz_active = SW_TEMPLATE_LED_LOW_ACTIVE,
	.dmz_seri = 0,
	.seri_clk = 14,
	.seri_dat = 15,
	.seri_no = 8,
	.seri_bit_pri = SW_TEMPLATE_SERI_LOW_BIT_FIRST,

	.sw_clk = 5,
	.sw_sda = 6,
	.sw_oda = 6,
	
	/* not define */
	.usb_clk = 30,  /*SW_TEMPLATE_GPIO_NO_THIS_PIN*/
	.usb_dat = 30,  /*SW_TEMPLATE_GPIO_NO_THIS_PIN*/
#else
	/* 6 Layer PCB */
	.cs_lan = 6,
	.cs_lan_state = 0,
	.cs_wan = 22,
	.cs_wan_state = 0,
	.cs_share = 0,

	.sw_rst = 1,
	.rst_bt = 0,

	.diag = 9,
	.diag_active = SW_TEMPLATE_LED_LOW_ACTIVE,
	.dmz = 30,/* 30 means no this pin */
	.dmz_active = SW_TEMPLATE_LED_LOW_ACTIVE,
	.dmz_seri = 1,
	.seri_clk = 8,
	.seri_dat = 7,
	.seri_no = 5,
	.seri_bit_pri = SW_TEMPLATE_SERI_HIGH_BIT_FIRST,

	.sw_clk = 23,
	.sw_sda = 21,
	.sw_oda = 20,
	
	/* not define */
	.usb_clk = 30,  /*SW_TEMPLATE_GPIO_NO_THIS_PIN*/
	.usb_dat = 30,  /*SW_TEMPLATE_GPIO_NO_THIS_PIN*/
#endif
};

Sw_Template_Sw_GPIO_t Sw_Template_Sw_GPIO_1450 = {
	.cs_lan = 2,
	.cs_lan_state = 0,
	.cs_wan = 2,
	.cs_wan_state = 1,
	.cs_share = 1,

	.sw_rst = 1,
	.rst_bt = 16,

	.diag = 12,
	.diag_active = SW_TEMPLATE_LED_HIGH_ACTIVE,
	.dmz = 13,
	.dmz_active = SW_TEMPLATE_LED_HIGH_ACTIVE,
	.dmz_seri = 0,
	.seri_clk = 14,
	.seri_dat = 15,
	.seri_no = 8,
	.seri_bit_pri = SW_TEMPLATE_SERI_LOW_BIT_FIRST,

	.sw_clk = 5,
	.sw_sda = 4,
	.sw_oda = 6,
	
	/* not define */
	.usb_clk = 30,  /*SW_TEMPLATE_GPIO_NO_THIS_PIN*/
	.usb_dat = 30,  /*SW_TEMPLATE_GPIO_NO_THIS_PIN*/
};

Sw_Template_Sw_GPIO_t Sw_Template_Sw_GPIO_Default = {
	.cs_lan = 2,
	.cs_lan_state = 1,
	.cs_wan = 2,
	.cs_wan_state = 0,
	.cs_share = 1,

	.sw_rst = 1,
	.rst_bt = 16,

	.diag = 12,
	.diag_active = SW_TEMPLATE_LED_HIGH_ACTIVE,
	.dmz = 13,
	.dmz_active = SW_TEMPLATE_LED_HIGH_ACTIVE,
	.dmz_seri = 0,
	.seri_clk = 14,
	.seri_dat = 15,
	.seri_no = 8,
	.seri_bit_pri = SW_TEMPLATE_SERI_LOW_BIT_FIRST,

	.sw_clk = 5,
	.sw_sda = 4,
	.sw_oda = 6,
	
	/* not define */
	.usb_clk = 30,  /*SW_TEMPLATE_GPIO_NO_THIS_PIN*/
	.usb_dat = 30,  /*SW_TEMPLATE_GPIO_NO_THIS_PIN*/
};


void		Sw_Template_Init_Switch_Info		( uint32_t model );
void		Sw_Template_Set_GPIO_Output			( Sw_Template_Sw_GPIO_t *gpio_t );
void		Sw_Template_Sw_Reset				( uint32_t model, Sw_Template_Sw_GPIO_t *gpio_t );
void		Sw_Template_Reset_Switch			( uint32_t model, Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, Sw_Template_Sw_Port_Map_t *port_map_t );

void		Sw_Template_Sw_CS					( Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir );
uint32_t 	Sw_Template_Front_Port_Map 			( Sw_Template_Sw_Port_Map_t *port_map_t, uint32_t dir, uint32_t fport );

/** BCM539X Driver **/
int			Sw_Template_Sw_Read_BCMX			( Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t page, uint32_t addr, uint64_t *value, uint32_t reserve );
int			Sw_Template_Sw_Write_BCMX			( Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t page, uint32_t addr, uint64_t value, uint32_t reserve );
int			Sw_Template_Sw_Phy_Read_BCMX 		( Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t page, uint32_t addr, uint64_t *value, uint32_t reserve );
int			Sw_Template_Sw_Phy_Write_BCMX		( Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t page, uint32_t addr, uint64_t value, uint32_t reserve );

void		Sw_Template_Get_Link_Status_BCMX	( Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t *status );
void		Sw_Template_Get_Speed_Status_BCMX	( Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t *status );
void		Sw_Template_Get_Duplex_Status_BCMX	( Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t *status );

uint64_t	Sw_Template_Get_MIB_Byte_Cnt		( Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t type, uint32_t sport );
uint64_t	Sw_Template_Get_MIB_Broadcast_Pkts	( Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t type, uint32_t sport );
uint64_t	Sw_Template_Get_MIB_Multicast_Pkts	( Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t type, uint32_t sport );
uint64_t	Sw_Template_Get_MIB_Unicast_Pkts	( Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t type, uint32_t sport );
uint64_t	Sw_Template_Get_MIB_Collision_Pkts	( Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t type, uint32_t sport );
uint64_t	Sw_Template_Get_MIB_Error_Pkts		( Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t type, uint32_t sport );

void 		Sw_Template_Set_Port_Status			( Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, Sw_Template_Sw_Port_Map_t *port_map_t, uint32_t dir, struct PortStatusNStatistics status );
void 		Sw_Template_Get_Port_Status			( Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, Sw_Template_Sw_Port_Map_t *port_map_t, uint32_t dir, struct PortStatusNStatistics *status);

void		Sw_Template_Set_Port_VLAN			( uint32_t model, Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, Sw_Template_Sw_Port_Map_t *port_map_t, uint32_t dir, uint32_t member, uint32_t fport );
void		Sw_Template_Set_VLAN_Group			( uint32_t model, struct _Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t ggroup_index, Sw_Template_VLAN_Table_Entry_t data );

void		Sw_Template_Set_LED					( uint32_t model, Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, Sw_Template_Sw_Port_Map_t *port_map_t, uint32_t type, uint32_t state );

void		Sw_Template_MII_Control				( uint32_t model, Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t opt );

void		Sw_Template_Set_Mirror_Port			( uint32_t model, Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, Sw_Template_Sw_Port_Map_t *port_map_t, uint32_t fport );

void		Sw_Template_Set_Static_MAC			( uint32_t model, Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, Sw_Template_ARL_Data_t arl_data );


/** RTL8306 Driver **/
int smiRead(unsigned int phyad, unsigned int regad, unsigned int * data);
int smiWrite(unsigned int phyad, unsigned int regad, unsigned int data);
int Sw_Template_Sw_Read_RTL8306 ( Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t phyad, uint32_t regad, uint64_t *value, uint32_t npage );
int Sw_Template_Sw_Write_RTL8306 ( Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t phyad, uint32_t regad, uint64_t value, uint32_t npage );

void Sw_Template_Get_Link_Status_RTL8306 ( Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t *status );
void Sw_Template_Get_Speed_Status_RTL8306 ( Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t *status );
void Sw_Template_Get_Duplex_Status_RTL8306 ( Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t *status );

uint64_t Sw_Template_Get_MIB_Byte_Cnt_RTL8306 ( Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t type, uint32_t sport );
uint64_t Sw_Template_Get_MIB_Broadcast_Pkts_RTL8306 ( Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t type, uint32_t sport );
uint64_t Sw_Template_Get_MIB_Multicast_Pkts_RTL8306 ( Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t type, uint32_t sport );
uint64_t Sw_Template_Get_MIB_Unicast_Pkts_RTL8306 ( Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t type, uint32_t sport );
uint64_t Sw_Template_Get_MIB_Collision_Pkts_RTL8306 ( Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t type, uint32_t sport );
uint64_t Sw_Template_Get_MIB_Error_Pkts_RTL8306 ( Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t type, uint32_t sport );

void Sw_Template_Set_Port_Status_RTL8306 ( Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, Sw_Template_Sw_Port_Map_t *port_map_t, uint32_t dir, struct PortStatusNStatistics status );
void Sw_Template_Get_Port_Status_RTL8306 ( Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, Sw_Template_Sw_Port_Map_t *port_map_t, uint32_t dir, struct PortStatusNStatistics *status);


void Sw_Template_Set_Port_VLAN_RTL8306 ( uint32_t model, Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, Sw_Template_Sw_Port_Map_t *port_map_t, uint32_t dir, uint32_t member, uint32_t fport );
void Sw_Template_Set_VLAN_Group_RTL8306 ( uint32_t model, struct _Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t ggroup_index, Sw_Template_VLAN_Table_Entry_t data );

void Sw_Template_Set_Mirror_Port_RTL8306 ( uint32_t model, Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, Sw_Template_Sw_Port_Map_t *port_map_t, uint32_t fport );


Sw_Template_Sw_Func_t Sw_Template_Sw_Func_005 = {
	.cs = Sw_Template_Sw_CS,
	.frontportmap = Sw_Template_Front_Port_Map,

	.read = Sw_Template_Sw_Read_RTL8306,
	.write = Sw_Template_Sw_Write_RTL8306,
	.phy_read = Sw_Template_Sw_Read_RTL8306,
	.phy_write = Sw_Template_Sw_Write_RTL8306,

	.get_link_status = Sw_Template_Get_Link_Status_RTL8306,
	.get_speed_status = Sw_Template_Get_Speed_Status_RTL8306,
	.get_duplex_status = Sw_Template_Get_Duplex_Status_RTL8306,

	.get_mib_byte_cnt = Sw_Template_Get_MIB_Byte_Cnt_RTL8306,
	.get_mib_broadcast_pkts = Sw_Template_Get_MIB_Broadcast_Pkts_RTL8306,
	.get_mib_multicast_pkts = Sw_Template_Get_MIB_Multicast_Pkts_RTL8306,
	.get_mib_unicast_pkts = Sw_Template_Get_MIB_Unicast_Pkts_RTL8306,
	.get_mib_collision_pkts = Sw_Template_Get_MIB_Collision_Pkts_RTL8306,
	.get_mib_error_pkts = Sw_Template_Get_MIB_Error_Pkts_RTL8306,

	.set_port_status = Sw_Template_Set_Port_Status_RTL8306,
	.get_port_status = Sw_Template_Get_Port_Status_RTL8306,

	.set_port_vlan = Sw_Template_Set_Port_VLAN_RTL8306,
	.set_vlan_group = Sw_Template_Set_VLAN_Group_RTL8306,

	.set_led = Sw_Template_Set_LED,

// 	.mii_control = Sw_Template_MII_Control,

	.set_mirror = Sw_Template_Set_Mirror_Port_RTL8306,

// 	.set_static_mac = Sw_Template_Set_Static_MAC,

	.switch_type = SW_TEMPLATE_SW_TYPE_RTL8306,
};

Sw_Template_Sw_Func_t Sw_Template_Sw_Func_650 = {
	.cs = Sw_Template_Sw_CS,
	.frontportmap = Sw_Template_Front_Port_Map,

	.read = Sw_Template_Sw_Read_BCMX,
	.write = Sw_Template_Sw_Write_BCMX,
	.phy_read = Sw_Template_Sw_Phy_Read_BCMX,
	.phy_write = Sw_Template_Sw_Phy_Write_BCMX,

	.get_link_status = Sw_Template_Get_Link_Status_BCMX,
	.get_speed_status = Sw_Template_Get_Speed_Status_BCMX,
	.get_duplex_status = Sw_Template_Get_Duplex_Status_BCMX,

	.get_mib_byte_cnt = Sw_Template_Get_MIB_Byte_Cnt,
	.get_mib_broadcast_pkts = Sw_Template_Get_MIB_Broadcast_Pkts,
	.get_mib_multicast_pkts = Sw_Template_Get_MIB_Multicast_Pkts,
	.get_mib_unicast_pkts = Sw_Template_Get_MIB_Unicast_Pkts,
	.get_mib_collision_pkts = Sw_Template_Get_MIB_Collision_Pkts,
	.get_mib_error_pkts = Sw_Template_Get_MIB_Error_Pkts,

	.set_port_status = Sw_Template_Set_Port_Status,
	.get_port_status = Sw_Template_Get_Port_Status,

	.set_port_vlan = Sw_Template_Set_Port_VLAN,
	.set_vlan_group = Sw_Template_Set_VLAN_Group,

	.set_led = Sw_Template_Set_LED,

	.mii_control = Sw_Template_MII_Control,

	.set_mirror = Sw_Template_Set_Mirror_Port,

	.set_static_mac = Sw_Template_Set_Static_MAC,

	.switch_type = SW_TEMPLATE_SW_TYPE_BCM53115,
};

Sw_Template_Sw_Func_t Sw_Template_Sw_Func_Default = {
	.cs = Sw_Template_Sw_CS,
	.frontportmap = Sw_Template_Front_Port_Map,

	.read = Sw_Template_Sw_Read_BCMX,
	.write = Sw_Template_Sw_Write_BCMX,
	.phy_read = Sw_Template_Sw_Phy_Read_BCMX,
	.phy_write = Sw_Template_Sw_Phy_Write_BCMX,

	.get_link_status = Sw_Template_Get_Link_Status_BCMX,
	.get_speed_status = Sw_Template_Get_Speed_Status_BCMX,
	.get_duplex_status = Sw_Template_Get_Duplex_Status_BCMX,

	.get_mib_byte_cnt = Sw_Template_Get_MIB_Byte_Cnt,
	.get_mib_broadcast_pkts = Sw_Template_Get_MIB_Broadcast_Pkts,
	.get_mib_multicast_pkts = Sw_Template_Get_MIB_Multicast_Pkts,
	.get_mib_unicast_pkts = Sw_Template_Get_MIB_Unicast_Pkts,
	.get_mib_collision_pkts = Sw_Template_Get_MIB_Collision_Pkts,
	.get_mib_error_pkts = Sw_Template_Get_MIB_Error_Pkts,

	.set_port_status = Sw_Template_Set_Port_Status,
	.get_port_status = Sw_Template_Get_Port_Status,

	.set_port_vlan = Sw_Template_Set_Port_VLAN,
	.set_vlan_group = Sw_Template_Set_VLAN_Group,

	.set_led = Sw_Template_Set_LED,

	.mii_control = Sw_Template_MII_Control,

	.set_mirror = Sw_Template_Set_Mirror_Port,

	.set_static_mac = Sw_Template_Set_Static_MAC,

	.switch_type = SW_TEMPLATE_SW_TYPE_BCM539X,
};

#endif
