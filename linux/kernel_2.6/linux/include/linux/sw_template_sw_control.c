#ifndef __SW_TEMPLATE_SW_CONTROL_C__
#define __SW_TEMPLATE_SW_CONTROL_C__


#include <linux/sw_template_sw_control.h>
#include <linux/sw_template_gpio.c>
#include <linux/sw_template_bcm539x_reg.h>
#include <linux/dynamic_port_num.h>

/** include file: switch RTL8306 driver **/
#include <linux/Rtl8306_AsicDrv.h>
#include <linux/Rtl8306_Driver_s.h>
#include <linux/Rtl8306_Driver_sd.h>
#include <linux/Rtl8306_types.h>

#include <linux/sw_template_bcm_acl.h>


extern rwlock_t SWRegLock;

/**
	For debug:
	Show front port 2 switch port mapping, and then you can know your port mapping is correct or not
**/
void Sw_Template_Show_Port_Map ( Sw_Template_Sw_Port_Map_t *port_map_t ) {

	int i;

	printk ( KERN_EMERG "Model(%d) LAN Port Map: ",  useSwitch);
	for ( i=0; i < CONFIG_NK_NUM_LAN; i++ )
		printk ( KERN_EMERG "%d ", port_map_t->lan2sw[i] );
	printk ( KERN_EMERG "\n" );

	printk ( KERN_EMERG "Model(%d) WAN Port Map: ",  useSwitch);
	for ( i=0; i < ( CONFIG_NK_NUM_WAN + CONFIG_NK_NUM_DMZ ); i++ )
		printk ( KERN_EMERG "%d ", port_map_t->wan2sw[i] );
	printk ( KERN_EMERG "\n" );
}

/**
	For debug:
	Show GPIO mapping, and then you can know your GPIO mapping is correct or not
**/
void Sw_Template_Show_GPIO ( Sw_Template_Sw_GPIO_t *gpio_t ) {

	uint32_t value, state;

	printk ( KERN_EMERG "Model(%d) GPIO:\n", useSwitch );

	value = Sw_Template_Get_GPIO_Value ( gpio_t->cs_lan );
	state = Sw_Template_Get_GPIO_State ( gpio_t->cs_lan );
	printk ( KERN_EMERG "cs_lan = %d, value[%s], state[%s]\n", gpio_t->cs_lan, ( value==SW_TEMPLATE_GPIO_LOW )?"LOW":"HIGH", ( state==SW_TEMPLATE_GPIO_IN)?"IN":"OUT" );
	printk ( KERN_EMERG "cs_lan_state = %d\n", gpio_t->cs_lan_state );
	value = Sw_Template_Get_GPIO_Value ( gpio_t->cs_wan );
	state = Sw_Template_Get_GPIO_State ( gpio_t->cs_wan );
	printk ( KERN_EMERG "cs_wan = %d, value[%s], state[%s]\n", gpio_t->cs_wan, ( value==SW_TEMPLATE_GPIO_LOW )?"LOW":"HIGH", ( state==SW_TEMPLATE_GPIO_IN)?"IN":"OUT" );
	printk ( KERN_EMERG "cs_wan_state = %d\n", gpio_t->cs_wan_state );
	printk ( KERN_EMERG "cs_share = %d\n", gpio_t->cs_share );
	printk ( KERN_EMERG "\n");

	value = Sw_Template_Get_GPIO_Value ( gpio_t->sw_rst );
	state = Sw_Template_Get_GPIO_State ( gpio_t->sw_rst );
	printk ( KERN_EMERG "sw_rst = %d, value[%s], state[%s]\n", gpio_t->sw_rst, ( value==SW_TEMPLATE_GPIO_LOW )?"LOW":"HIGH", ( state==SW_TEMPLATE_GPIO_IN)?"IN":"OUT" );
	value = Sw_Template_Get_GPIO_Value ( gpio_t->rst_bt );
	state = Sw_Template_Get_GPIO_State ( gpio_t->rst_bt );
	printk ( KERN_EMERG "rst_bt = %d, value[%s], state[%s]\n", gpio_t->rst_bt, ( value==SW_TEMPLATE_GPIO_LOW )?"LOW":"HIGH", ( state==SW_TEMPLATE_GPIO_IN)?"IN":"OUT" );
	printk ( KERN_EMERG "\n");

	value = Sw_Template_Get_GPIO_Value ( gpio_t->diag );
	state = Sw_Template_Get_GPIO_State ( gpio_t->diag );
	printk ( KERN_EMERG "diag = %d, value[%s], state[%s]\n", gpio_t->diag, ( value==SW_TEMPLATE_GPIO_LOW )?"LOW":"HIGH", ( state==SW_TEMPLATE_GPIO_IN)?"IN":"OUT" );
	value = Sw_Template_Get_GPIO_Value ( gpio_t->dmz );
	state = Sw_Template_Get_GPIO_State ( gpio_t->dmz );
	printk ( KERN_EMERG "dmz = %d, value[%s], state[%s]\n", gpio_t->dmz, ( value==SW_TEMPLATE_GPIO_LOW )?"LOW":"HIGH", ( state==SW_TEMPLATE_GPIO_IN)?"IN":"OUT" );
	value = Sw_Template_Get_GPIO_Value ( gpio_t->seri_clk );
	state = Sw_Template_Get_GPIO_State ( gpio_t->seri_clk );
	printk ( KERN_EMERG "seri_clk = %d, value[%s], state[%s]\n", gpio_t->seri_clk, ( value==SW_TEMPLATE_GPIO_LOW )?"LOW":"HIGH", ( state==SW_TEMPLATE_GPIO_IN)?"IN":"OUT" );
	value = Sw_Template_Get_GPIO_Value ( gpio_t->seri_dat );
	state = Sw_Template_Get_GPIO_State ( gpio_t->seri_dat );
	printk ( KERN_EMERG "seri_dat = %d, value[%s], state[%s]\n", gpio_t->seri_dat, ( value==SW_TEMPLATE_GPIO_LOW )?"LOW":"HIGH", ( state==SW_TEMPLATE_GPIO_IN)?"IN":"OUT" );
	printk ( KERN_EMERG "\n");

	value = Sw_Template_Get_GPIO_Value ( gpio_t->sw_clk );
	state = Sw_Template_Get_GPIO_State ( gpio_t->sw_clk );
	printk ( KERN_EMERG "sw_clk = %d, value[%s], state[%s]\n", gpio_t->sw_clk, ( value==SW_TEMPLATE_GPIO_LOW )?"LOW":"HIGH", ( state==SW_TEMPLATE_GPIO_IN)?"IN":"OUT" );
	value = Sw_Template_Get_GPIO_Value ( gpio_t->sw_sda );
	state = Sw_Template_Get_GPIO_State ( gpio_t->sw_sda );
	printk ( KERN_EMERG "sw_sda = %d, value[%s], state[%s]\n", gpio_t->sw_sda, ( value==SW_TEMPLATE_GPIO_LOW )?"LOW":"HIGH", ( state==SW_TEMPLATE_GPIO_IN)?"IN":"OUT" );
	value = Sw_Template_Get_GPIO_Value ( gpio_t->sw_oda );
	state = Sw_Template_Get_GPIO_State ( gpio_t->sw_oda );
	printk ( KERN_EMERG "sw_oda = %d, value[%s], state[%s]\n", gpio_t->sw_oda, ( value==SW_TEMPLATE_GPIO_LOW )?"LOW":"HIGH", ( state==SW_TEMPLATE_GPIO_IN)?"IN":"OUT" );
}


void Sw_Template_Init_Switch_Info_Port_Map ( uint32_t model, Sw_Template_Sw_Port_Map_t *port_map_t ) {

	if ( model == SWITCH005 ) {
		port_map_t->lan2sw = SW_TEMPLATE_LAN_FRONT_2_SWITCH_005;
		port_map_t->wan2sw = SW_TEMPLATE_WAN_FRONT_2_SWITCH_005;
	}
	else if ( model == SWITCH363 ) {
		port_map_t->lan2sw = SW_TEMPLATE_LAN_FRONT_2_SWITCH_363;
		port_map_t->wan2sw = SW_TEMPLATE_WAN_FRONT_2_SWITCH_363;
	}
	else if ( model == SWITCH650 || model == SWITCH1550 ) {
		port_map_t->lan2sw = SW_TEMPLATE_LAN_FRONT_2_SWITCH_650;
		port_map_t->wan2sw = SW_TEMPLATE_WAN_FRONT_2_SWITCH_650;
	}
	else if ( model == SWITCH1450 ) {
		port_map_t->lan2sw = SW_TEMPLATE_LAN_FRONT_2_SWITCH_1450;
		port_map_t->wan2sw = SW_TEMPLATE_WAN_FRONT_2_SWITCH_1450;
	}
	else if ( model == SWITCH3000 ) {
		port_map_t->lan2sw = SW_TEMPLATE_LAN_FRONT_2_SWITCH_3000;
		port_map_t->wan2sw = SW_TEMPLATE_WAN_FRONT_2_SWITCH_3000;
	}
	else {
		port_map_t->lan2sw = SW_TEMPLATE_LAN_FRONT_2_SWITCH_DEF;
		port_map_t->wan2sw = SW_TEMPLATE_WAN_FRONT_2_SWITCH_DEF;
	}
}

/**
	Init basic varible
	1. G_Sw_Port_Map: Front port to switch port mapping
	2. G_Sw_GPIO: All GPIO mapping, cs...
	3. G_Sw_Func: All Func
**/
void Sw_Template_Init_Switch_Info( uint32_t model ) {

	/* Init Sw_Template_Sw_Port_Map_t */
	G_Sw_Port_Map = kmalloc ( sizeof ( Sw_Template_Sw_Port_Map_t ), GFP_ATOMIC );
	Sw_Template_Init_Switch_Info_Port_Map ( useSwitch, G_Sw_Port_Map );

	/* Init Sw_Template_Sw_GPIO_t */
	if ( model == SWITCH005 )
		G_Sw_GPIO = &Sw_Template_Sw_GPIO_005;
	else if ( model == SWITCH363 )
		G_Sw_GPIO = &Sw_Template_Sw_GPIO_363;
	else if ( model == SWITCH650 || model == SWITCH1550 )
		G_Sw_GPIO = &Sw_Template_Sw_GPIO_650;
	else if ( model == SWITCH1450 )
		G_Sw_GPIO = &Sw_Template_Sw_GPIO_1450;
	else
		G_Sw_GPIO = &Sw_Template_Sw_GPIO_Default;

	/* Sw_Template_Sw_Func_t */
	if ( model == SWITCH005 || model == SWITCH363 )
		G_Sw_Func = &Sw_Template_Sw_Func_005;
	else if ( model == SWITCH650 || model == SWITCH1550)
		G_Sw_Func = &Sw_Template_Sw_Func_650;
	else
		G_Sw_Func = &Sw_Template_Sw_Func_Default;
}


/**
	Enable MDC/MDIO, must set SMI_EN[EN] bit to 1
 */
void Sw_Template_Set_SMI (uint32_t state)
{
	cvmx_smi_en_t config;

	config.u64 = cvmx_read_csr(CVMX_SMI_EN);

	config.s.en = state;
	cvmx_write_csr(CVMX_SMI_EN, config.u64);
}

/**
	Before control switch, must set gpio to correct state
	1. Enable MDC/MDIO
	2. CS: LAN Switch, WAN Switch chip select
	3. Reset: SW Reset, Reset Button(Input)
	4. LED: DIAG, DMZ, SERI CLK, SERI DATA
	5. SPI Switch: Switch CLK, Switch SDA, Switch ODA(Input)
**/
void Sw_Template_Init_Switch_Set_GPIO_Output ( uint32_t model, Sw_Template_Sw_GPIO_t *gpio_t ) {

	/* Enbale MDC/MDIO */
	Sw_Template_Set_SMI ( SW_TEMPLATE_ENABLE );

	/* CS */
	Sw_Template_Set_GPIO_State ( SW_TEMPLATE_GPIO_OUT, gpio_t->cs_lan );
	Sw_Template_Set_GPIO_State ( SW_TEMPLATE_GPIO_OUT, gpio_t->cs_wan );

	/* Reset */
	Sw_Template_Set_GPIO_State ( SW_TEMPLATE_GPIO_OUT, gpio_t->sw_rst );
	Sw_Template_Set_GPIO_State ( SW_TEMPLATE_GPIO_IN, gpio_t->rst_bt );

	/* LED */
	Sw_Template_Set_GPIO_State ( SW_TEMPLATE_GPIO_OUT, gpio_t->diag );
	Sw_Template_Set_GPIO_State ( SW_TEMPLATE_GPIO_OUT, gpio_t->dmz );
	Sw_Template_Set_GPIO_State ( SW_TEMPLATE_GPIO_OUT, gpio_t->seri_clk );
	Sw_Template_Set_GPIO_State ( SW_TEMPLATE_GPIO_OUT, gpio_t->seri_dat );

	/* SPI Switch */
	Sw_Template_Set_GPIO_State ( SW_TEMPLATE_GPIO_OUT, gpio_t->sw_clk );
	Sw_Template_Set_GPIO_State ( SW_TEMPLATE_GPIO_OUT, gpio_t->sw_sda );
	Sw_Template_Set_GPIO_State ( SW_TEMPLATE_GPIO_IN, gpio_t->sw_oda );
	
	Sw_Template_Set_GPIO_State ( SW_TEMPLATE_GPIO_OUT, gpio_t->usb_clk );
	Sw_Template_Set_GPIO_State ( SW_TEMPLATE_GPIO_OUT, gpio_t->usb_dat );
}

void Sw_Template_Sw_Reset ( uint32_t model, Sw_Template_Sw_GPIO_t *gpio_t ) {

	if ( useSwitch == SWITCH005 || useSwitch == SWITCH363 ) {
		Sw_Template_Set_GPIO_Value ( SW_TEMPLATE_GPIO_LOW, ( 1 << gpio_t->sw_rst ) );
		Sw_Template_Delay ( 1000 );

		Sw_Template_Set_GPIO_Value ( SW_TEMPLATE_GPIO_HIGH, ( 1 << gpio_t->sw_rst ) );
		Sw_Template_Delay ( 1000 );
	}
	else {
		Sw_Template_Set_GPIO_Value ( SW_TEMPLATE_GPIO_HIGH, ( 1 << gpio_t->sw_rst ) );
		Sw_Template_Delay ( 10 );

		Sw_Template_Set_GPIO_Value ( SW_TEMPLATE_GPIO_LOW, ( 1 << gpio_t->sw_rst ) );
		Sw_Template_Delay ( 10 );

		Sw_Template_Set_GPIO_Value ( SW_TEMPLATE_GPIO_HIGH, ( 1 << gpio_t->sw_rst ) );
		Sw_Template_Delay ( 10 );
	}
}

void Sw_Template_Force_Port_Link ( Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t sport ) {

	func_t->cs ( gpio_t, dir );
	func_t->write ( gpio_t, dir, PAGE_0, BCM539x_INTER_PHY_STATE_OVERRIDE_REGISTER_0 + sport, BCM539x_INTERNAL_PHY_STATE_1000Mbps, 0 );
}

void Sw_Template_Fix_Switch_LED ( uint32_t model, Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir ) {

	func_t->cs ( gpio_t, dir );

	func_t->write ( gpio_t, dir, PAGE_0, BCM539x_LED_ENABLE_MAP_REGISTER, 0x1F, 0 );
}

void Sw_Template_Multi_Flooding_Disable ( uint32_t model, Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir ) {

	uint64_t result;
	func_t->cs ( gpio_t, dir );
	func_t->read ( gpio_t, dir, 0x0, 0x2f, &result, 0 );
	result |= 0x1;
	func_t->write ( gpio_t, dir, 0x0, 0x2f, result, 0 );
}

void Sw_Template_MII_Control ( uint32_t model, Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t opt ) {

	uint64_t result;

	func_t->cs ( gpio_t, dir );

	func_t->read ( gpio_t, dir, 0x0, 0x0b, &result, 0 );

	if ( opt == SW_TEMPLATE_DISABLE )
		result &= ~(0x2);
	else
		result |= 0x2;

	func_t->write ( gpio_t, dir, 0x0, 0xb, result, 0 );
}

void Sw_Template_Reset_Switch_005 ( uint32_t model, Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, Sw_Template_Sw_Port_Map_t *port_map_t ) {

	int i;
	/* LAN */
	func_t->cs ( gpio_t,  SW_TEMPLATE_CS_LAN);

	/* Close LAN MII */
	if ( func_t->mii_control )
		func_t->mii_control ( model, func_t, gpio_t, SW_TEMPLATE_CS_LAN, SW_TEMPLATE_DISABLE );

	/* 005/363G has another method to cover this */
// 	rtl8306_setAsic25MClockOutput(TRUE);
	rtl8306_setAsicPhyRegBit(0, 18, 5, 0, 0);//set led blink time
	rtl8306_init();
	rtl8306_setPort5LinkStatus(TRUE);
	rtl8306_setEthernetPHY(6, FALSE, RTL8306_ETHER_AUTO_100FULL, RTL8306_ETHER_SPEED_100, TRUE);
	rtl8306_setCPUPort(6, FALSE, NULL);

	for (i = 0; i <= 5; i++) {
		rtl8306_setAsicQosPriorityEnable(i, RTL8306_PBP_PRIO, TRUE);
		rtl8306_setAsicQosPortBasedPriority(i, 0);
	}

	/* Init Switch Counter */
	for ( i=0; i <=5; i++ )
		rtl8306_setAsicMibCounterReset(i, RTL8306_MIB_RESET);

	for ( i=0; i <=5; i++ )
		rtl8306_setAsicMibCounterReset(i, RTL8306_MIB_START);

	/* WAN */
	func_t->cs ( gpio_t,  SW_TEMPLATE_CS_WAN);

	rtl8306_setAsic25MClockOutput(TRUE);
	rtl8306_setAsicPhyRegBit(0, 18, 5, 0, 0);//set led blink time
	rtl8306_init();
	rtl8306_setPort5LinkStatus(TRUE);
	rtl8306_setEthernetPHY(6, FALSE, RTL8306_ETHER_AUTO_100FULL, RTL8306_ETHER_SPEED_100, TRUE);
	rtl8306_setCPUPort(6, FALSE, NULL);

	for (i = 0; i < 5; i++)
		rtl8306_setAsicPortLearningAbility(i, FALSE);

	for (i = 0; i <= 5; i++) {
		rtl8306_setAsicQosPriorityEnable(i, RTL8306_PBP_PRIO, TRUE);
		rtl8306_setAsicQosPortBasedPriority(i, 0);
	}

	/* Init Switch Counter */
	for (i = 0; i <= 5; i++)
		rtl8306_setAsicMibCounterReset(i, RTL8306_MIB_RESET);

	for (i = 0; i <= 5; i++)
		rtl8306_setAsicMibCounterReset(i, RTL8306_MIB_START);

}

void Sw_Template_Reset_Switch_363 ( uint32_t model, Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, Sw_Template_Sw_Port_Map_t *port_map_t ) {

	int i;
	/* LAN */
	func_t->cs ( gpio_t,  SW_TEMPLATE_CS_LAN);

	/* Close LAN MII */
	if ( func_t->mii_control )
		func_t->mii_control ( model, func_t, gpio_t, SW_TEMPLATE_CS_LAN, SW_TEMPLATE_DISABLE );

	/* 005/363G has another method to cover this */
// 	rtl8306_setAsic25MClockOutput(TRUE);
	rtl8306_setAsicPhyRegBit(0, 18, 5, 0, 0);//set led blink time
	rtl8306_init();
	rtl8306_setPort5LinkStatus(TRUE);
	rtl8306_setEthernetPHY(6, FALSE, RTL8306_ETHER_AUTO_100FULL, RTL8306_ETHER_SPEED_100, TRUE);
	rtl8306_setCPUPort(6, FALSE, NULL);

	for (i = 0; i <= 5; i++) {
		rtl8306_setAsicQosPriorityEnable(i, RTL8306_PBP_PRIO, TRUE);
		rtl8306_setAsicQosPortBasedPriority(i, 0);
	}

	/* Init Switch Counter */
	for ( i=0; i <=5; i++ )
		rtl8306_setAsicMibCounterReset(i, RTL8306_MIB_RESET);

	for ( i=0; i <=5; i++ )
		rtl8306_setAsicMibCounterReset(i, RTL8306_MIB_START);

	/* WAN */
	func_t->cs ( gpio_t,  SW_TEMPLATE_CS_WAN);

	for (i = 1; i <= ( CONFIG_NK_NUM_WAN + CONFIG_NK_NUM_DMZ ); i++)
		rtl8306_setAsicPortLearningAbility( func_t->frontportmap ( port_map_t, SW_TEMPLATE_CS_WAN, i ), FALSE);
}

void Sw_Template_Reset_Switch_Default ( uint32_t model, Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, Sw_Template_Sw_Port_Map_t *port_map_t ) {

	int i;

	/* Close LAN MII */
	if ( func_t->mii_control )
		func_t->mii_control ( model, func_t, gpio_t, SW_TEMPLATE_CS_LAN, SW_TEMPLATE_DISABLE );

	/* Force Port5 link, BCM53115M only */
	if ( model == SWITCH650 || model == SWITCH1550 )
		Sw_Template_Force_Port_Link ( func_t, gpio_t, SW_TEMPLATE_CS_LAN, 5 );

	/* Force Auto-MDIX Mode(auto crossover) */
	for ( i=0; i < CONFIG_NK_NUM_LAN; i++ ) {
		func_t->cs ( gpio_t, SW_TEMPLATE_CS_LAN );
		func_t->phy_write ( gpio_t, SW_TEMPLATE_CS_LAN, i, 0x18, 0x8217, 0 );
	}
	for ( i=0; i < CONFIG_NK_NUM_WAN; i++ ) {
		func_t->cs ( gpio_t, SW_TEMPLATE_CS_WAN );
		func_t->phy_write ( gpio_t, SW_TEMPLATE_CS_WAN, i, 0x18, 0x8217, 0 );
	}

	/* Set Power/MII to A mode(Full Power Mode) */
	for ( i=0; i < CONFIG_NK_NUM_LAN; i++ ) {
		func_t->cs ( gpio_t, SW_TEMPLATE_CS_LAN );
		func_t->phy_write ( gpio_t, SW_TEMPLATE_CS_LAN, i, 0x18, 0x8282, 0 );
	}
	for ( i=0; i < CONFIG_NK_NUM_WAN; i++ ) {
		func_t->cs ( gpio_t, SW_TEMPLATE_CS_WAN );
		func_t->phy_write ( gpio_t, SW_TEMPLATE_CS_WAN, i, 0x18, 0x8282, 0 );
	}

	/* Fix Switch LED be shift issue */
	if ( useSwitch == SWITCH650 || useSwitch == SWITCH1550 ) {
	}
	else if ( useSwitch == SWITCH1450 ) {
		func_t->cs ( gpio_t, SW_TEMPLATE_CS_LAN );
		func_t->write ( gpio_t, SW_TEMPLATE_CS_LAN, 0x0, 0x16, 0x1f, 0 );
	}
	else if ( useSwitch == SWITCH3000 ) {
		/* Need not to do anything */
	}
	else {
		func_t->cs ( gpio_t, SW_TEMPLATE_CS_WAN );
		func_t->write ( gpio_t, SW_TEMPLATE_CS_WAN, 0x0, 0x16, 0x1f, 0 );
	}

	/* Mirror port ingress/egress port */
	func_t->cs ( gpio_t, SW_TEMPLATE_CS_LAN );
#ifdef BCM53115_USE_PORT5
	func_t->write ( gpio_t, SW_TEMPLATE_CS_LAN, BCM539X_MANAGEMENT_MIRRORING_REGISTER, BCM539X_INGRESS_MIRROR_CONTROL_REGISTER, 0x20, 0 );
#else
	func_t->write ( gpio_t, SW_TEMPLATE_CS_LAN, BCM539X_MANAGEMENT_MIRRORING_REGISTER, BCM539X_INGRESS_MIRROR_CONTROL_REGISTER, DEFAULT_INGRESS_VALUE, 0 );
#endif

	func_t->cs ( gpio_t, SW_TEMPLATE_CS_LAN );
#ifdef BCM53115_USE_PORT5
	func_t->write ( gpio_t, SW_TEMPLATE_CS_WAN, BCM539X_MANAGEMENT_MIRRORING_REGISTER, BCM539X_EGRESS_MIRROR_CONTROL_REGISTER, 0x20, 0 );
#else
	func_t->write ( gpio_t, SW_TEMPLATE_CS_WAN, BCM539X_MANAGEMENT_MIRRORING_REGISTER, BCM539X_EGRESS_MIRROR_CONTROL_REGISTER, DEFAULT_EGRESS_VALUE, 0 );
#endif

	/* Fix Multi Flooding */
	Sw_Template_Multi_Flooding_Disable ( model, func_t, gpio_t, SW_TEMPLATE_CS_LAN );
	Sw_Template_Multi_Flooding_Disable ( model, func_t, gpio_t, SW_TEMPLATE_CS_WAN );

	/* TC 2 COS Mapping */
	if ( useSwitch == SWITCH650 || useSwitch == SWITCH1550 ) {
		func_t->cs ( gpio_t, SW_TEMPLATE_CS_LAN );
		func_t->write ( gpio_t, SW_TEMPLATE_CS_LAN, 0x30, 0x62, 0xE4, 0 );/* 11 10 01 00 */
	}
}

void Sw_Template_Reset_Switch ( uint32_t model, Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, Sw_Template_Sw_Port_Map_t *port_map_t ) {

	printk ( KERN_EMERG "Starting Reset Switch.........\n" );

	Sw_Template_Sw_Reset ( model, gpio_t );

	if ( model == SWITCH005 ) {
		Sw_Template_Reset_Switch_005 ( model, func_t, gpio_t, port_map_t );
	}
	else if ( model == SWITCH363 ) {
		Sw_Template_Reset_Switch_363 ( model, func_t, gpio_t, port_map_t );
	}
	else {
		Sw_Template_Reset_Switch_Default ( model, func_t, gpio_t, port_map_t );
	}

	/* Init LED */
	if ( useSwitch == SWITCH1450 ) {
		/**
			Because GQF1450 has 8WANs, but it just has 5WANs LED.
			So we use Link/Speed LED to replace 5WANs LED.
			Original Link LED becomes to WAN Connect LED
		**/
		func_t->cs ( gpio_t, SW_TEMPLATE_CS_WAN );
		func_t->write ( gpio_t, SW_TEMPLATE_CS_WAN, 0x0, 0x10, 0x2814, 0 );
		func_t->write ( gpio_t, SW_TEMPLATE_CS_WAN, 0x0, 0x14, 0x0, 0 );
		Sw_Template_Set_LED ( model, func_t, gpio_t, port_map_t, SW_TEMPLATE_SERI_LED, Sw_Template_LED_Seri_Old_State & 0x0000FFFF );
	}
	else
		Sw_Template_Set_LED ( model, func_t, gpio_t, port_map_t, SW_TEMPLATE_SERI_LED, Sw_Template_LED_Seri_Old_State );
	Sw_Template_Set_LED ( model, func_t, gpio_t, port_map_t, SW_TEMPLATE_DIAG_LED, SW_TEMPLATE_LED_OFF );
	Sw_Template_Set_LED ( model, func_t, gpio_t, port_map_t, SW_TEMPLATE_DMZ_LED, SW_TEMPLATE_LED_OFF );
}


void Sw_Template_Set_Port_VLAN ( uint32_t model, Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, Sw_Template_Sw_Port_Map_t *port_map_t, uint32_t dir, uint32_t member, uint32_t fport ) {

	int i;
	uint16_t sport=0, sport_mask=0;

	for ( i=1; i <= CONFIG_NK_NUM_LAN; i++ ) {	
		if ( member & ( 1 << i ) ) {
			sport = func_t->frontportmap ( port_map_t, dir, i );
			sport_mask |= ( 1 << sport );
		}
	}

	if ( useSwitch == SWITCH650 || useSwitch == SWITCH1550 )
/* GQF650 Use PORT 5, it must sync ethernet.c, getlink.c, sw_template_sw_control.c */
#ifdef BCM53115_USE_PORT5
		sport_mask |= 0x20;//Port 5
#else
		sport_mask |= 0x100;//IMP Port
#endif
	else
		sport_mask |= 0x100;//IMP Port

	sport = func_t->frontportmap ( port_map_t, dir, fport );
	func_t->cs ( gpio_t, dir );	
	func_t->write ( gpio_t, dir, 0x31, 0x0 + ( sport * 2 ), sport_mask, 0 );
}

/**
	Set VLan Group Step:
	1. Set VLAN table entry
	2. Set VLAN table index
	3. Write command
**/
void Sw_Template_Set_VLAN_Group ( uint32_t model, struct _Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t group_index, Sw_Template_VLAN_Table_Entry_t data ) {

	uint64_t value;

	func_t->cs ( gpio_t, dir );

	/* 1. Set VLAN table entry */
	value = data.u64;
	if ( model == SWITCH650 || model == SWITCH1550 )
		func_t->write ( gpio_t, dir, BCM539x_ARL_VTBL_ACCESS_REGISTER, BCM53115M_VLAN_TABLE_ENTRY_REGISTER, value, 0 );
	else
		func_t->write ( gpio_t, dir, BCM539x_ARL_VTBL_ACCESS_REGISTER, BCM539x_VLAN_TABLE_ENTRY_REGISTER, value, 0 );

	/* 2. Set VLAN table index */
	if ( model == SWITCH650 || model == SWITCH1550 )
		func_t->write ( gpio_t, dir, BCM539x_ARL_VTBL_ACCESS_REGISTER, BCM53115M_VLAN_TABLE_ADDERSS_INDEX_REGISTER, group_index, 0 );
	else
		func_t->write ( gpio_t, dir, BCM539x_ARL_VTBL_ACCESS_REGISTER, BCM539x_VLAN_TABLE_ADDERSS_INDEX_REGISTER, group_index, 0 );

	/* 3. Write command */
	if ( model == SWITCH650 || model == SWITCH1550 )
		func_t->write ( gpio_t, dir, BCM539x_ARL_VTBL_ACCESS_REGISTER, BCM53115M_VLAN_TABLE_READ_WRITE_CONTROL_REGISTER, BCM539X_VLAN_START_WRITE, 0 );
	else
		func_t->write ( gpio_t, dir, BCM539x_ARL_VTBL_ACCESS_REGISTER, BCM539x_VLAN_TABLE_READ_WRITE_CONTROL_REGISTER, BCM539X_VLAN_START_WRITE, 0 );
}

void Sw_Template_Enable_VLAN ( uint32_t model, Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t state ) {

	func_t->cs ( gpio_t, dir );

	if ( state == SW_TEMPLATE_ENABLE )
		func_t->write ( gpio_t, dir, BCM539x_VLAN_REGISTER, BCM539x_GLOBAL_TAG_VLAN_REGISTER, BCM539x_DEFAULT_VLAN_CONFIG, 0 );
}

void Sw_Template_Init_VLAN_005 ( uint32_t model, Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, Sw_Template_Sw_Port_Map_t *port_map_t ) {

	int i;
	uint32_t vlan_group_base;
	uint32_t wan2switch_005[] = {3,2,1,0};

	/* LAN */
	func_t->cs ( gpio_t,  SW_TEMPLATE_CS_LAN);

	/*clear vlan table*/
	for(i = 0;i < 16; i++)
		rtl8306_setAsicVlan(i, 0, 0);

	/*set switch default configuration */
	rtl8306_setVlanTagAware(TRUE);/*enable tag aware*/
	rtl8306_setIngressFilter(FALSE);/*disable ingress filter*/
	rtl8306_setVlanTagOnly(FALSE);/*disable vlan tag only*/

	/*add a default vlan which contains all ports*/
	rtl8306_addVlan(1);
	for ( i=1; i <= CONFIG_NK_NUM_LAN; i++ )
		rtl8306_addVlanPortMember ( 1, port_map_t->lan2sw[i-1] );
	rtl8306_addVlanPortMember(1, 5);

	/*set all ports' vid to vlan 1*/
	for ( i=0; i < 5; i++ )
		rtl8306_setPvid ( i, 1 );
	rtl8306_setPvid(5, 1);

	for (i = 0; i < 5; i++)
		rtl8306_setAsicVlanTagInsertRemove(i, RTL8306_VLAN_RTAG);
	rtl8306_setAsicVlanTagInsertRemove(5, RTL8306_VLAN_ITAG);

	vlan_group_base = CONFIG_NK_NUM_LAN + CONFIG_NK_NUM_WAN + CONFIG_NK_NUM_DMZ;
	for ( i=0; i < 5; i++ ) {
			rtl8306_addVlan ( ( vlan_group_base + 1 ) + i);
			rtl8306_addVlanPortMember ( ( vlan_group_base + 1 ) + i, 5);
	}
	/*set vlan enabled*/
	rtl8306_setAsicVlanEnable(TRUE);


	/* WAN */
	func_t->cs ( gpio_t,  SW_TEMPLATE_CS_WAN);

	/*clear vlan table*/
	for(i = 0;i < 16; i++)
		rtl8306_setAsicVlan(i, 0, 0);

	/*set switch default configuration */
	rtl8306_setVlanTagAware(TRUE);/*enable tag aware*/
	rtl8306_setIngressFilter(FALSE);/*disable ingress filter*/
	rtl8306_setVlanTagOnly(FALSE);/*disable vlan tag only*/

	for ( i=1 ; i <= (CONFIG_NK_NUM_WAN+CONFIG_NK_NUM_DMZ); i++ ) {
		rtl8306_addVlan(i+1);
		rtl8306_addVlanPortMember(i+1, wan2switch_005[i-1]);
		rtl8306_addVlanPortMember(i+1, 5); // MII
		rtl8306_setPvid(wan2switch_005[i-1], i+1);
	}
	rtl8306_setPvid(5, 2);

	for (i = 0; i < 5; i++)
		rtl8306_setAsicVlanTagInsertRemove(i, RTL8306_VLAN_RTAG);
	rtl8306_setAsicVlanTagInsertRemove(5, RTL8306_VLAN_ITAG);

	/*set vlan enabled*/
	rtl8306_setAsicVlanEnable(TRUE);

}

/**
	363G has only a switch
**/
void Sw_Template_Init_VLAN_363 ( uint32_t model, Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, Sw_Template_Sw_Port_Map_t *port_map_t ) {

	int i;
	uint32_t vlan_group_base;
	uint32_t wan2switch[] = {3,2};
	uint32_t LAN_MII_PORT=5, WAN_MII_PORT=4;

	/* LAN */
	func_t->cs ( gpio_t,  SW_TEMPLATE_CS_LAN);

	/*clear vlan table*/
	for(i = 0;i < 16; i++)
		rtl8306_setAsicVlan(i, 0, 0);

	/*set switch default configuration */
	rtl8306_setVlanTagAware(TRUE);/*enable tag aware*/
	rtl8306_setIngressFilter(FALSE);/*disable ingress filter*/
	rtl8306_setVlanTagOnly(FALSE);/*disable vlan tag only*/

	/*add a default vlan which contains all ports*/
	rtl8306_addVlan(1);
	for ( i=1; i <= CONFIG_NK_NUM_LAN; i++ )
		rtl8306_addVlanPortMember ( 1, port_map_t->lan2sw[i-1] );
	rtl8306_addVlanPortMember(1, LAN_MII_PORT);

	/*set all ports' vid to vlan 1*/
	for ( i=1; i <= CONFIG_NK_NUM_LAN; i++ )
		rtl8306_setPvid ( port_map_t->lan2sw[i-1], 1 );
	rtl8306_setPvid(LAN_MII_PORT, 1);

	for ( i=1; i <= CONFIG_NK_NUM_LAN; i++ )
		rtl8306_setAsicVlanTagInsertRemove(port_map_t->lan2sw[i-1], RTL8306_VLAN_RTAG);
	rtl8306_setAsicVlanTagInsertRemove(LAN_MII_PORT, RTL8306_VLAN_ITAG);

	vlan_group_base = CONFIG_NK_NUM_LAN + CONFIG_NK_NUM_WAN + CONFIG_NK_NUM_DMZ;
	for ( i=1; i <= CONFIG_NK_NUM_LAN; i++ ) {
			rtl8306_addVlan ( vlan_group_base + i);
			rtl8306_addVlanPortMember ( vlan_group_base + i, LAN_MII_PORT);
	}

	/* WAN */
	func_t->cs ( gpio_t,  SW_TEMPLATE_CS_WAN);

	for ( i=1 ; i <= (CONFIG_NK_NUM_WAN+CONFIG_NK_NUM_DMZ); i++ ) {
		rtl8306_addVlan(i+1);
		rtl8306_addVlanPortMember(i+1, wan2switch[i-1]);
		rtl8306_addVlanPortMember(i+1, WAN_MII_PORT); // MII
		rtl8306_setPvid(wan2switch[i-1], i+1);
	}
	rtl8306_setPvid(WAN_MII_PORT, 2);

	for ( i=1 ; i <= (CONFIG_NK_NUM_WAN+CONFIG_NK_NUM_DMZ); i++ )
		rtl8306_setAsicVlanTagInsertRemove(port_map_t->wan2sw[i-1], RTL8306_VLAN_RTAG);
	rtl8306_setAsicVlanTagInsertRemove(WAN_MII_PORT, RTL8306_VLAN_ITAG);

	/*set vlan enabled*/
	rtl8306_setAsicVlanEnable(TRUE);

}

void Sw_Template_Init_VLAN_650 ( uint32_t model, Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, Sw_Template_Sw_Port_Map_t *port_map_t ) {

	int i;
	uint16_t mask, utag_mask;
	uint32_t lan_default_vid=0x1;
	Sw_Template_VLAN_Table_Entry_t value;

	/* LAN */
	func_t->cs ( gpio_t, SW_TEMPLATE_CS_LAN );

	for ( i=0; i < CONFIG_NK_NUM_LAN; i++ ) {
		func_t->write ( gpio_t, SW_TEMPLATE_CS_LAN, BCM539x_VLAN_REGISTER, BCM539x_DEFAULE_802_1Q_TAG_REGISTER_PORT_0+ (i*2), lan_default_vid, 0);
	}
/* GQF650 Use PORT 5, it must sync ethernet.c, getlink.c, sw_template_sw_control.c */
#ifdef BCM53115_USE_PORT5
	func_t->write ( gpio_t, SW_TEMPLATE_CS_LAN, BCM539x_VLAN_REGISTER, BCM539x_DEFAULE_802_1Q_TAG_REGISTER_PORT_5, lan_default_vid, 0);
#else
	func_t->write ( gpio_t, SW_TEMPLATE_CS_LAN, BCM539x_VLAN_REGISTER, BCM539x_DEFAULE_802_1Q_TAG_REGISTER_PORT_IMP, lan_default_vid, 0);
#endif

/* GQF650 Use PORT 5, it must sync ethernet.c, getlink.c, sw_template_sw_control.c */
#ifdef BCM53115_USE_PORT5
	mask = SW_TEMPLATE_PORT_0_MASK | SW_TEMPLATE_PORT_1_MASK | SW_TEMPLATE_PORT_2_MASK | SW_TEMPLATE_PORT_3_MASK | SW_TEMPLATE_PORT_4_MASK | SW_TEMPLATE_PORT_5_MASK;
#else
	mask = SW_TEMPLATE_PORT_0_MASK | SW_TEMPLATE_PORT_1_MASK | SW_TEMPLATE_PORT_2_MASK | SW_TEMPLATE_PORT_3_MASK | SW_TEMPLATE_PORT_4_MASK | SW_TEMPLATE_PORT_IMP_MASK;
#endif
	value.s.fwd_map = mask;

	utag_mask = SW_TEMPLATE_PORT_0_MASK | SW_TEMPLATE_PORT_1_MASK | SW_TEMPLATE_PORT_2_MASK | SW_TEMPLATE_PORT_3_MASK | SW_TEMPLATE_PORT_4_MASK;
	value.s.untag_map = utag_mask;

	Sw_Template_Set_VLAN_Group ( model, func_t, gpio_t, SW_TEMPLATE_CS_LAN, lan_default_vid, value );

	/* Enable VLAN */
	Sw_Template_Enable_VLAN ( model, func_t, gpio_t, SW_TEMPLATE_CS_LAN, SW_TEMPLATE_ENABLE );

	/* WAN */
	func_t->cs ( gpio_t, SW_TEMPLATE_CS_WAN );

#if 1
	/* 4 Layer PCB */
	func_t->write ( gpio_t, SW_TEMPLATE_CS_WAN, BCM539x_VLAN_REGISTER, BCM539x_DEFAULE_802_1Q_TAG_REGISTER_PORT_0, 0x6, 0 );
	func_t->write ( gpio_t, SW_TEMPLATE_CS_WAN, BCM539x_VLAN_REGISTER, BCM539x_DEFAULE_802_1Q_TAG_REGISTER_PORT_1, 0x5, 0 );
	func_t->write ( gpio_t, SW_TEMPLATE_CS_WAN, BCM539x_VLAN_REGISTER, BCM539x_DEFAULE_802_1Q_TAG_REGISTER_PORT_2, 0x4, 0 );
	func_t->write ( gpio_t, SW_TEMPLATE_CS_WAN, BCM539x_VLAN_REGISTER, BCM539x_DEFAULE_802_1Q_TAG_REGISTER_PORT_3, 0x3, 0 );
	func_t->write ( gpio_t, SW_TEMPLATE_CS_WAN, BCM539x_VLAN_REGISTER, BCM539x_DEFAULE_802_1Q_TAG_REGISTER_PORT_4, 0x2, 0 );
	func_t->write ( gpio_t, SW_TEMPLATE_CS_WAN, BCM539x_VLAN_REGISTER, BCM539x_DEFAULE_802_1Q_TAG_REGISTER_PORT_IMP, 0x2, 0 );

	utag_mask = SW_TEMPLATE_PORT_0_MASK | SW_TEMPLATE_PORT_1_MASK | SW_TEMPLATE_PORT_2_MASK | SW_TEMPLATE_PORT_3_MASK | SW_TEMPLATE_PORT_4_MASK;
	value.s.untag_map = utag_mask;

	mask = SW_TEMPLATE_PORT_0_MASK | SW_TEMPLATE_PORT_IMP_MASK;
	value.s.fwd_map = mask;
	Sw_Template_Set_VLAN_Group ( model, func_t, gpio_t, SW_TEMPLATE_CS_WAN, 6, value);

	mask = SW_TEMPLATE_PORT_1_MASK | SW_TEMPLATE_PORT_IMP_MASK;
	value.s.fwd_map = mask;
	Sw_Template_Set_VLAN_Group ( model, func_t, gpio_t, SW_TEMPLATE_CS_WAN, 5, value);

	mask = SW_TEMPLATE_PORT_2_MASK | SW_TEMPLATE_PORT_IMP_MASK;
	value.s.fwd_map = mask;
	Sw_Template_Set_VLAN_Group ( model, func_t, gpio_t, SW_TEMPLATE_CS_WAN, 4, value);

	mask = SW_TEMPLATE_PORT_3_MASK | SW_TEMPLATE_PORT_IMP_MASK;
	value.s.fwd_map = mask;
	Sw_Template_Set_VLAN_Group ( model, func_t, gpio_t, SW_TEMPLATE_CS_WAN, 3, value);

	mask = SW_TEMPLATE_PORT_4_MASK | SW_TEMPLATE_PORT_IMP_MASK;
	value.s.fwd_map = mask;
	Sw_Template_Set_VLAN_Group ( model, func_t, gpio_t, SW_TEMPLATE_CS_WAN, 2, value);

	/* enable tag vlan */
	func_t->write ( gpio_t, SW_TEMPLATE_CS_WAN, BCM539x_VLAN_REGISTER, BCM539x_GLOBAL_TAG_VLAN_REGISTER, BCM539x_DEFAULT_VLAN_CONFIG, 0 );
#else
	/* 6 Layer PCB */
	func_t->write ( gpio_t, SW_TEMPLATE_CS_WAN, BCM539x_VLAN_REGISTER, BCM539x_DEFAULE_802_1Q_TAG_REGISTER_PORT_0, 0x5, 0 );
	func_t->write ( gpio_t, SW_TEMPLATE_CS_WAN, BCM539x_VLAN_REGISTER, BCM539x_DEFAULE_802_1Q_TAG_REGISTER_PORT_1, 0x4, 0 );
	func_t->write ( gpio_t, SW_TEMPLATE_CS_WAN, BCM539x_VLAN_REGISTER, BCM539x_DEFAULE_802_1Q_TAG_REGISTER_PORT_2, 0x3, 0 );
	func_t->write ( gpio_t, SW_TEMPLATE_CS_WAN, BCM539x_VLAN_REGISTER, BCM539x_DEFAULE_802_1Q_TAG_REGISTER_PORT_3, 0x2, 0 );
	func_t->write ( gpio_t, SW_TEMPLATE_CS_WAN, BCM539x_VLAN_REGISTER, BCM539x_DEFAULE_802_1Q_TAG_REGISTER_PORT_4, 0x6, 0 );
	func_t->write ( gpio_t, SW_TEMPLATE_CS_WAN, BCM539x_VLAN_REGISTER, BCM539x_DEFAULE_802_1Q_TAG_REGISTER_PORT_IMP, 0x2, 0 );

	utag_mask = SW_TEMPLATE_PORT_0_MASK | SW_TEMPLATE_PORT_1_MASK | SW_TEMPLATE_PORT_2_MASK | SW_TEMPLATE_PORT_3_MASK | SW_TEMPLATE_PORT_4_MASK;
	value.s.untag_map = utag_mask;

	mask = SW_TEMPLATE_PORT_0_MASK | SW_TEMPLATE_PORT_IMP_MASK;
	value.s.fwd_map = mask;
	Sw_Template_Set_VLAN_Group ( model, func_t, gpio_t, SW_TEMPLATE_CS_WAN, 5, value);

	mask = SW_TEMPLATE_PORT_1_MASK | SW_TEMPLATE_PORT_IMP_MASK;
	value.s.fwd_map = mask;
	Sw_Template_Set_VLAN_Group ( model, func_t, gpio_t, SW_TEMPLATE_CS_WAN, 4, value);

	mask = SW_TEMPLATE_PORT_2_MASK | SW_TEMPLATE_PORT_IMP_MASK;
	value.s.fwd_map = mask;
	Sw_Template_Set_VLAN_Group ( model, func_t, gpio_t, SW_TEMPLATE_CS_WAN, 3, value);

	mask = SW_TEMPLATE_PORT_3_MASK | SW_TEMPLATE_PORT_IMP_MASK;
	value.s.fwd_map = mask;
	Sw_Template_Set_VLAN_Group ( model, func_t, gpio_t, SW_TEMPLATE_CS_WAN, 2, value);

	mask = SW_TEMPLATE_PORT_4_MASK | SW_TEMPLATE_PORT_IMP_MASK;
	value.s.fwd_map = mask;
	Sw_Template_Set_VLAN_Group ( model, func_t, gpio_t, SW_TEMPLATE_CS_WAN, 6, value);

	/* enable tag vlan */
	func_t->write ( gpio_t, SW_TEMPLATE_CS_WAN, BCM539x_VLAN_REGISTER, BCM539x_GLOBAL_TAG_VLAN_REGISTER, BCM539x_DEFAULT_VLAN_CONFIG, 0 );
#endif
}

void Sw_Template_Init_VLAN_1450 ( uint32_t model, Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, Sw_Template_Sw_Port_Map_t *port_map_t ) {

	int i;
	uint16_t mask, utag_mask;
	uint32_t lan_default_vid=0x1;
	Sw_Template_VLAN_Table_Entry_t value;

	/* LAN */
	func_t->cs ( gpio_t, SW_TEMPLATE_CS_LAN );

	for ( i=0; i < CONFIG_NK_NUM_LAN; i++ ) {
		func_t->write ( gpio_t, SW_TEMPLATE_CS_LAN, BCM539x_VLAN_REGISTER, BCM539x_DEFAULE_802_1Q_TAG_REGISTER_PORT_0+ (i*2), lan_default_vid, 0);
	}
	func_t->write ( gpio_t, SW_TEMPLATE_CS_LAN, BCM539x_VLAN_REGISTER, BCM539x_DEFAULE_802_1Q_TAG_REGISTER_PORT_IMP, lan_default_vid, 0);

	mask = SW_TEMPLATE_PORT_0_MASK | SW_TEMPLATE_PORT_1_MASK | SW_TEMPLATE_PORT_2_MASK | SW_TEMPLATE_PORT_3_MASK  | SW_TEMPLATE_PORT_4_MASK| SW_TEMPLATE_PORT_IMP_MASK;
	value.s.fwd_map = mask;

	utag_mask = SW_TEMPLATE_PORT_0_MASK | SW_TEMPLATE_PORT_1_MASK | SW_TEMPLATE_PORT_2_MASK | SW_TEMPLATE_PORT_3_MASK | SW_TEMPLATE_PORT_4_MASK;
	value.s.untag_map = utag_mask;

	Sw_Template_Set_VLAN_Group ( model, func_t, gpio_t, SW_TEMPLATE_CS_LAN, lan_default_vid, value );

	/* Enable VLAN */
	Sw_Template_Enable_VLAN ( model, func_t, gpio_t, SW_TEMPLATE_CS_LAN, SW_TEMPLATE_ENABLE );

	/* WAN */
	func_t->cs ( gpio_t, SW_TEMPLATE_CS_WAN );

	func_t->write ( gpio_t, SW_TEMPLATE_CS_WAN, BCM539x_VLAN_REGISTER, BCM539x_DEFAULE_802_1Q_TAG_REGISTER_PORT_0, 0x6, 0 );
	func_t->write ( gpio_t, SW_TEMPLATE_CS_WAN, BCM539x_VLAN_REGISTER, BCM539x_DEFAULE_802_1Q_TAG_REGISTER_PORT_1, 0x2, 0 );
	func_t->write ( gpio_t, SW_TEMPLATE_CS_WAN, BCM539x_VLAN_REGISTER, BCM539x_DEFAULE_802_1Q_TAG_REGISTER_PORT_2, 0x7, 0 );
	func_t->write ( gpio_t, SW_TEMPLATE_CS_WAN, BCM539x_VLAN_REGISTER, BCM539x_DEFAULE_802_1Q_TAG_REGISTER_PORT_3, 0x3, 0 );
	func_t->write ( gpio_t, SW_TEMPLATE_CS_WAN, BCM539x_VLAN_REGISTER, BCM539x_DEFAULE_802_1Q_TAG_REGISTER_PORT_4, 0x8, 0 );
	func_t->write ( gpio_t, SW_TEMPLATE_CS_WAN, BCM539x_VLAN_REGISTER, BCM539x_DEFAULE_802_1Q_TAG_REGISTER_PORT_5, 0x4, 0 );
	func_t->write ( gpio_t, SW_TEMPLATE_CS_WAN, BCM539x_VLAN_REGISTER, BCM539x_DEFAULE_802_1Q_TAG_REGISTER_PORT_6, 0x9, 0 );
	func_t->write ( gpio_t, SW_TEMPLATE_CS_WAN, BCM539x_VLAN_REGISTER, BCM539x_DEFAULE_802_1Q_TAG_REGISTER_PORT_7, 0x5, 0 );
	func_t->write ( gpio_t, SW_TEMPLATE_CS_WAN, BCM539x_VLAN_REGISTER, BCM539x_DEFAULE_802_1Q_TAG_REGISTER_PORT_IMP, 0x2, 0 );

	utag_mask = SW_TEMPLATE_PORT_0_MASK | SW_TEMPLATE_PORT_1_MASK | SW_TEMPLATE_PORT_2_MASK | SW_TEMPLATE_PORT_3_MASK | SW_TEMPLATE_PORT_4_MASK | SW_TEMPLATE_PORT_5_MASK | SW_TEMPLATE_PORT_6_MASK | SW_TEMPLATE_PORT_7_MASK;
	value.s.untag_map = utag_mask;

	mask = SW_TEMPLATE_PORT_0_MASK | SW_TEMPLATE_PORT_IMP_MASK;
	value.s.fwd_map = mask;
	Sw_Template_Set_VLAN_Group ( model, func_t, gpio_t, SW_TEMPLATE_CS_WAN, 6, value);

	mask = SW_TEMPLATE_PORT_1_MASK | SW_TEMPLATE_PORT_IMP_MASK;
	value.s.fwd_map = mask;
	Sw_Template_Set_VLAN_Group ( model, func_t, gpio_t, SW_TEMPLATE_CS_WAN, 2, value);

	mask = SW_TEMPLATE_PORT_2_MASK | SW_TEMPLATE_PORT_IMP_MASK;
	value.s.fwd_map = mask;
	Sw_Template_Set_VLAN_Group ( model, func_t, gpio_t, SW_TEMPLATE_CS_WAN, 7, value);

	mask = SW_TEMPLATE_PORT_3_MASK | SW_TEMPLATE_PORT_IMP_MASK;
	value.s.fwd_map = mask;
	Sw_Template_Set_VLAN_Group ( model, func_t, gpio_t, SW_TEMPLATE_CS_WAN, 3, value);

	mask = SW_TEMPLATE_PORT_4_MASK | SW_TEMPLATE_PORT_IMP_MASK;
	value.s.fwd_map = mask;
	Sw_Template_Set_VLAN_Group ( model, func_t, gpio_t, SW_TEMPLATE_CS_WAN, 8, value);

	mask = SW_TEMPLATE_PORT_5_MASK | SW_TEMPLATE_PORT_IMP_MASK;
	value.s.fwd_map = mask;
	Sw_Template_Set_VLAN_Group ( model, func_t, gpio_t, SW_TEMPLATE_CS_WAN, 4, value);

	mask = SW_TEMPLATE_PORT_6_MASK | SW_TEMPLATE_PORT_IMP_MASK;
	value.s.fwd_map = mask;
	Sw_Template_Set_VLAN_Group ( model, func_t, gpio_t, SW_TEMPLATE_CS_WAN, 9, value);

	mask = SW_TEMPLATE_PORT_7_MASK | SW_TEMPLATE_PORT_IMP_MASK;
	value.s.fwd_map = mask;
	Sw_Template_Set_VLAN_Group ( model, func_t, gpio_t, SW_TEMPLATE_CS_WAN, 5, value);

	/* enable tag vlan */
	func_t->write ( gpio_t, SW_TEMPLATE_CS_WAN, BCM539x_VLAN_REGISTER, BCM539x_GLOBAL_TAG_VLAN_REGISTER, BCM539x_DEFAULT_VLAN_CONFIG, 0 );
}

void Sw_Template_Init_VLAN_3000 ( uint32_t model, Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, Sw_Template_Sw_Port_Map_t *port_map_t ) {


	int i;
	uint16_t mask, utag_mask;
	uint32_t lan_default_vid=0x1;
	Sw_Template_VLAN_Table_Entry_t value;

	/* LAN */
	func_t->cs ( gpio_t, SW_TEMPLATE_CS_LAN );

	for ( i=0; i < CONFIG_NK_NUM_LAN; i++ ) {
		func_t->write ( gpio_t, SW_TEMPLATE_CS_LAN, BCM539x_VLAN_REGISTER, BCM539x_DEFAULE_802_1Q_TAG_REGISTER_PORT_0+ (i*2), lan_default_vid, 0);
	}
	func_t->write ( gpio_t, SW_TEMPLATE_CS_LAN, BCM539x_VLAN_REGISTER, BCM539x_DEFAULE_802_1Q_TAG_REGISTER_PORT_IMP, lan_default_vid, 0);

	mask = SW_TEMPLATE_PORT_0_MASK | SW_TEMPLATE_PORT_1_MASK | SW_TEMPLATE_PORT_2_MASK | SW_TEMPLATE_PORT_3_MASK  | SW_TEMPLATE_PORT_4_MASK  | SW_TEMPLATE_PORT_5_MASK  | SW_TEMPLATE_PORT_6_MASK  | SW_TEMPLATE_PORT_7_MASK | SW_TEMPLATE_PORT_IMP_MASK;
	value.s.fwd_map = mask;

	utag_mask = SW_TEMPLATE_PORT_0_MASK | SW_TEMPLATE_PORT_1_MASK | SW_TEMPLATE_PORT_2_MASK | SW_TEMPLATE_PORT_3_MASK | SW_TEMPLATE_PORT_4_MASK  | SW_TEMPLATE_PORT_5_MASK  | SW_TEMPLATE_PORT_6_MASK  | SW_TEMPLATE_PORT_7_MASK;
	value.s.untag_map = utag_mask;

	Sw_Template_Set_VLAN_Group ( model, func_t, gpio_t, SW_TEMPLATE_CS_LAN, lan_default_vid, value );

	/* Enable VLAN */
	Sw_Template_Enable_VLAN ( model, func_t, gpio_t, SW_TEMPLATE_CS_LAN, SW_TEMPLATE_ENABLE );

	/* WAN */
	func_t->cs ( gpio_t, SW_TEMPLATE_CS_WAN );

	func_t->write ( gpio_t, SW_TEMPLATE_CS_WAN, BCM539x_VLAN_REGISTER, BCM539x_DEFAULE_802_1Q_TAG_REGISTER_PORT_0, 0x9, 0 );
	func_t->write ( gpio_t, SW_TEMPLATE_CS_WAN, BCM539x_VLAN_REGISTER, BCM539x_DEFAULE_802_1Q_TAG_REGISTER_PORT_1, 0x5, 0 );
	func_t->write ( gpio_t, SW_TEMPLATE_CS_WAN, BCM539x_VLAN_REGISTER, BCM539x_DEFAULE_802_1Q_TAG_REGISTER_PORT_2, 0x8, 0 );
	func_t->write ( gpio_t, SW_TEMPLATE_CS_WAN, BCM539x_VLAN_REGISTER, BCM539x_DEFAULE_802_1Q_TAG_REGISTER_PORT_3, 0x4, 0 );
	func_t->write ( gpio_t, SW_TEMPLATE_CS_WAN, BCM539x_VLAN_REGISTER, BCM539x_DEFAULE_802_1Q_TAG_REGISTER_PORT_4, 0x7, 0 );
	func_t->write ( gpio_t, SW_TEMPLATE_CS_WAN, BCM539x_VLAN_REGISTER, BCM539x_DEFAULE_802_1Q_TAG_REGISTER_PORT_5, 0x3, 0 );
	func_t->write ( gpio_t, SW_TEMPLATE_CS_WAN, BCM539x_VLAN_REGISTER, BCM539x_DEFAULE_802_1Q_TAG_REGISTER_PORT_6, 0x6, 0 );
	func_t->write ( gpio_t, SW_TEMPLATE_CS_WAN, BCM539x_VLAN_REGISTER, BCM539x_DEFAULE_802_1Q_TAG_REGISTER_PORT_7, 0x2, 0 );
	func_t->write ( gpio_t, SW_TEMPLATE_CS_WAN, BCM539x_VLAN_REGISTER, BCM539x_DEFAULE_802_1Q_TAG_REGISTER_PORT_IMP, 0x2, 0 );

	utag_mask = SW_TEMPLATE_PORT_0_MASK | SW_TEMPLATE_PORT_1_MASK | SW_TEMPLATE_PORT_2_MASK | SW_TEMPLATE_PORT_3_MASK | SW_TEMPLATE_PORT_4_MASK | SW_TEMPLATE_PORT_5_MASK | SW_TEMPLATE_PORT_6_MASK | SW_TEMPLATE_PORT_7_MASK;
	value.s.untag_map = utag_mask;

	mask = SW_TEMPLATE_PORT_0_MASK | SW_TEMPLATE_PORT_IMP_MASK;
	value.s.fwd_map = mask;
	Sw_Template_Set_VLAN_Group ( model, func_t, gpio_t, SW_TEMPLATE_CS_WAN, 9, value);

	mask = SW_TEMPLATE_PORT_1_MASK | SW_TEMPLATE_PORT_IMP_MASK;
	value.s.fwd_map = mask;
	Sw_Template_Set_VLAN_Group ( model, func_t, gpio_t, SW_TEMPLATE_CS_WAN, 5, value);

	mask = SW_TEMPLATE_PORT_2_MASK | SW_TEMPLATE_PORT_IMP_MASK;
	value.s.fwd_map = mask;
	Sw_Template_Set_VLAN_Group ( model, func_t, gpio_t, SW_TEMPLATE_CS_WAN, 8, value);

	mask = SW_TEMPLATE_PORT_3_MASK | SW_TEMPLATE_PORT_IMP_MASK;
	value.s.fwd_map = mask;
	Sw_Template_Set_VLAN_Group ( model, func_t, gpio_t, SW_TEMPLATE_CS_WAN, 4, value);

	mask = SW_TEMPLATE_PORT_4_MASK | SW_TEMPLATE_PORT_IMP_MASK;
	value.s.fwd_map = mask;
	Sw_Template_Set_VLAN_Group ( model, func_t, gpio_t, SW_TEMPLATE_CS_WAN, 7, value);

	mask = SW_TEMPLATE_PORT_5_MASK | SW_TEMPLATE_PORT_IMP_MASK;
	value.s.fwd_map = mask;
	Sw_Template_Set_VLAN_Group ( model, func_t, gpio_t, SW_TEMPLATE_CS_WAN, 3, value);

	mask = SW_TEMPLATE_PORT_6_MASK | SW_TEMPLATE_PORT_IMP_MASK;
	value.s.fwd_map = mask;
	Sw_Template_Set_VLAN_Group ( model, func_t, gpio_t, SW_TEMPLATE_CS_WAN, 6, value);

	mask = SW_TEMPLATE_PORT_7_MASK | SW_TEMPLATE_PORT_IMP_MASK;
	value.s.fwd_map = mask;
	Sw_Template_Set_VLAN_Group ( model, func_t, gpio_t, SW_TEMPLATE_CS_WAN, 2, value);

	/* enable tag vlan */
	func_t->write ( gpio_t, SW_TEMPLATE_CS_WAN, BCM539x_VLAN_REGISTER, BCM539x_GLOBAL_TAG_VLAN_REGISTER, BCM539x_DEFAULT_VLAN_CONFIG, 0 );
}

/**
	1. Init Default VLAN ID
	2. Config Forward port and Untag port
	3. Enable VLAN
**/
void Sw_Template_Init_VLAN_Default ( uint32_t model, Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, Sw_Template_Sw_Port_Map_t *port_map_t ) {


	int i;
	uint16_t mask, utag_mask;
	uint32_t lan_default_vid=0x1;
	Sw_Template_VLAN_Table_Entry_t value;

	/* LAN */
	func_t->cs ( gpio_t, SW_TEMPLATE_CS_LAN );

	for ( i=0; i < CONFIG_NK_NUM_LAN; i++ ) {
		func_t->write ( gpio_t, SW_TEMPLATE_CS_LAN, BCM539x_VLAN_REGISTER, BCM539x_DEFAULE_802_1Q_TAG_REGISTER_PORT_0+ (i*2), lan_default_vid, 0);
	}
	func_t->write ( gpio_t, SW_TEMPLATE_CS_LAN, BCM539x_VLAN_REGISTER, BCM539x_DEFAULE_802_1Q_TAG_REGISTER_PORT_IMP, lan_default_vid, 0);

	mask = SW_TEMPLATE_PORT_0_MASK | SW_TEMPLATE_PORT_1_MASK | SW_TEMPLATE_PORT_2_MASK | SW_TEMPLATE_PORT_3_MASK  | SW_TEMPLATE_PORT_4_MASK  | SW_TEMPLATE_PORT_5_MASK  | SW_TEMPLATE_PORT_6_MASK  | SW_TEMPLATE_PORT_7_MASK | SW_TEMPLATE_PORT_IMP_MASK;
	value.s.fwd_map = mask;

	utag_mask = SW_TEMPLATE_PORT_0_MASK | SW_TEMPLATE_PORT_1_MASK | SW_TEMPLATE_PORT_2_MASK | SW_TEMPLATE_PORT_3_MASK | SW_TEMPLATE_PORT_4_MASK  | SW_TEMPLATE_PORT_5_MASK  | SW_TEMPLATE_PORT_6_MASK  | SW_TEMPLATE_PORT_7_MASK;
	value.s.untag_map = utag_mask;

	Sw_Template_Set_VLAN_Group ( model, func_t, gpio_t, SW_TEMPLATE_CS_LAN, lan_default_vid, value );

	/* Enable VLAN */
	Sw_Template_Enable_VLAN ( model, func_t, gpio_t, SW_TEMPLATE_CS_LAN, SW_TEMPLATE_ENABLE );

	/* WAN */
	func_t->cs ( gpio_t, SW_TEMPLATE_CS_WAN );

	func_t->write ( gpio_t, SW_TEMPLATE_CS_WAN, BCM539x_VLAN_REGISTER, BCM539x_DEFAULE_802_1Q_TAG_REGISTER_PORT_0, 0x5, 0 );
	func_t->write ( gpio_t, SW_TEMPLATE_CS_WAN, BCM539x_VLAN_REGISTER, BCM539x_DEFAULE_802_1Q_TAG_REGISTER_PORT_1, 0x4, 0 );
	func_t->write ( gpio_t, SW_TEMPLATE_CS_WAN, BCM539x_VLAN_REGISTER, BCM539x_DEFAULE_802_1Q_TAG_REGISTER_PORT_2, 0x3, 0 );
	func_t->write ( gpio_t, SW_TEMPLATE_CS_WAN, BCM539x_VLAN_REGISTER, BCM539x_DEFAULE_802_1Q_TAG_REGISTER_PORT_3, 0x2, 0 );
	func_t->write ( gpio_t, SW_TEMPLATE_CS_WAN, BCM539x_VLAN_REGISTER, BCM539x_DEFAULE_802_1Q_TAG_REGISTER_PORT_4, 0x6, 0 );
	func_t->write ( gpio_t, SW_TEMPLATE_CS_WAN, BCM539x_VLAN_REGISTER, BCM539x_DEFAULE_802_1Q_TAG_REGISTER_PORT_IMP, 0x2, 0 );

	utag_mask = SW_TEMPLATE_PORT_0_MASK | SW_TEMPLATE_PORT_1_MASK | SW_TEMPLATE_PORT_2_MASK | SW_TEMPLATE_PORT_3_MASK | SW_TEMPLATE_PORT_4_MASK;
	value.s.untag_map = utag_mask;

	mask = SW_TEMPLATE_PORT_0_MASK | SW_TEMPLATE_PORT_IMP_MASK;
	value.s.fwd_map = mask;
	Sw_Template_Set_VLAN_Group ( model, func_t, gpio_t, SW_TEMPLATE_CS_WAN, 5, value);

	mask = SW_TEMPLATE_PORT_1_MASK | SW_TEMPLATE_PORT_IMP_MASK;
	value.s.fwd_map = mask;
	Sw_Template_Set_VLAN_Group ( model, func_t, gpio_t, SW_TEMPLATE_CS_WAN, 4, value);

	mask = SW_TEMPLATE_PORT_2_MASK | SW_TEMPLATE_PORT_IMP_MASK;
	value.s.fwd_map = mask;
	Sw_Template_Set_VLAN_Group ( model, func_t, gpio_t, SW_TEMPLATE_CS_WAN, 3, value);

	mask = SW_TEMPLATE_PORT_3_MASK | SW_TEMPLATE_PORT_IMP_MASK;
	value.s.fwd_map = mask;
	Sw_Template_Set_VLAN_Group ( model, func_t, gpio_t, SW_TEMPLATE_CS_WAN, 2, value);

	mask = SW_TEMPLATE_PORT_4_MASK | SW_TEMPLATE_PORT_IMP_MASK;
	value.s.fwd_map = mask;
	Sw_Template_Set_VLAN_Group ( model, func_t, gpio_t, SW_TEMPLATE_CS_WAN, 6, value);

	/* enable tag vlan */
	func_t->write ( gpio_t, SW_TEMPLATE_CS_WAN, BCM539x_VLAN_REGISTER, BCM539x_GLOBAL_TAG_VLAN_REGISTER, BCM539x_DEFAULT_VLAN_CONFIG, 0 );
}

void Sw_Template_Init_VLAN ( uint32_t model, Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, Sw_Template_Sw_Port_Map_t *port_map_t ) {

	if ( model == SWITCH005 ) {
		Sw_Template_Init_VLAN_005 ( model, func_t, gpio_t, port_map_t );
	}
	else if ( model == SWITCH363 ) {
		Sw_Template_Init_VLAN_363 ( model, func_t, gpio_t, port_map_t );
	}
	else if ( model == SWITCH650 || model == SWITCH1550 ) {
		Sw_Template_Init_VLAN_650 ( model, func_t, gpio_t, port_map_t );
	}
	else if ( model == SWITCH1450 ) {
		Sw_Template_Init_VLAN_1450 ( model, func_t, gpio_t, port_map_t );
	}
	else if ( model == SWITCH3000 ) {
		Sw_Template_Init_VLAN_3000 ( model, func_t, gpio_t, port_map_t );
	}
	else {
		Sw_Template_Init_VLAN_Default ( model, func_t, gpio_t, port_map_t );
	}
}


void Sw_Template_Sw_CS ( Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir ) {

	/* If cs_lan pin dont share with cs_wan, it means when switch lan enable then switch wan must be disable at the same time */
	if ( gpio_t->cs_share ) {
		if ( dir == SW_TEMPLATE_CS_LAN ) {
			Sw_Template_Set_GPIO_Value ( gpio_t->cs_lan_state, ( 1 << gpio_t->cs_lan ) );
		}
		else {
			Sw_Template_Set_GPIO_Value ( gpio_t->cs_wan_state, ( 1 << gpio_t->cs_wan ) );
		}
	}
	else {
		if ( dir == SW_TEMPLATE_CS_LAN ) {
			Sw_Template_Set_GPIO_Value ( gpio_t->cs_lan_state, ( 1 << gpio_t->cs_lan ) );
			Sw_Template_Set_GPIO_Value ( ( ( gpio_t->cs_wan_state == SW_TEMPLATE_GPIO_LOW ) ? SW_TEMPLATE_GPIO_HIGH : SW_TEMPLATE_GPIO_LOW ), ( 1 << gpio_t->cs_wan ) );
		}
		else {
			Sw_Template_Set_GPIO_Value ( gpio_t->cs_wan_state, ( 1 << gpio_t->cs_wan ) );
			Sw_Template_Set_GPIO_Value ( ( ( gpio_t->cs_lan_state == SW_TEMPLATE_GPIO_LOW ) ? SW_TEMPLATE_GPIO_HIGH : SW_TEMPLATE_GPIO_LOW ), ( 1 << gpio_t->cs_lan ) );
		}
	}
}

uint32_t Sw_Template_Front_Port_Map ( Sw_Template_Sw_Port_Map_t *port_map_t, uint32_t dir, uint32_t fport ) {

	if ( dir == SW_TEMPLATE_CS_LAN ) {
		if ( fport > CONFIG_NK_NUM_LAN || fport < 1 ) {
			printk ( KERN_EMERG "%s: LAN fport[%d] out of range[1~%d]!!!\n", __func__, fport, CONFIG_NK_NUM_LAN );
			return port_map_t->lan2sw[0];
		}
		return port_map_t->lan2sw[fport - 1];
	}
	else {
		if ( fport > ( CONFIG_NK_NUM_WAN + CONFIG_NK_NUM_DMZ ) || fport < 1 ) {
			printk ( KERN_EMERG "%s: WAN fport[%d] out of range[1~%d]!!!\n", __func__, fport, ( CONFIG_NK_NUM_WAN + CONFIG_NK_NUM_DMZ ) );
			return port_map_t->wan2sw[0];
		}
		return port_map_t->wan2sw[fport - 1];
	}
}


/* Sw_Template_Sw_Func for BCM5397/BCM5398 Series */
int Sw_Template_Sw_Read_BCMX ( Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t page, uint32_t addr, uint64_t *value, uint32_t reserve ) {

	int err, timeout;
	Sw_Template_phyreg16_t reg16;
	Sw_Template_phyreg17_t reg17;
	Sw_Template_result_reg_t mdio_read_value;
	u16 result;

	mdio_read_value.u64 = 0;

	spin_lock_bh(&SWRegLock);

	/* 1.write pseudo-phy reg16 */
	reg16.u16 = 0;
	reg16.s.page_num = page;
	reg16.s.A = 1;
	//if((err=mdio_write(SW_TEMPLATE_BCM_PHYAD, 16, reg16.u16))!=0)
	if ( ( err = Sw_Template_Sw_Phy_Write_BCMX ( gpio_t, dir, SW_TEMPLATE_BCM_PHYAD, 16, reg16.u16, 0 ) ) != 0 )
		goto ERROR;

	/* 2.write pseudo-phy reg17 */
	reg17.u16 = 0;
	reg17.s.reg_addr = addr;
	reg17.s.op = 2;
	if((err=mdio_write(SW_TEMPLATE_BCM_PHYAD, 17, reg17.u16))!=0)
		goto ERROR;

	/* read pseudo-phy reg17 */
	reg17.u16 = 0;
	timeout = 10;
	do
	{
		delay(delay_t);
		if((err=mdio_read(SW_TEMPLATE_BCM_PHYAD, 17, &reg17.u16))!=0)
			goto ERROR;
		
	}while(reg17.s.op !=0 && (--timeout>0));
	delay(delay_t);
	/* 4.read pseudo-phy reg24 */
	result = 0;
	if((err=mdio_read(SW_TEMPLATE_BCM_PHYAD, 24, &result))!=0)
		goto ERROR;
	mdio_read_value.s.reg24 = result;

	/* 5.read pseudo-phy reg25 */
	result = 0;
	if((err=mdio_read(SW_TEMPLATE_BCM_PHYAD, 25, &result))!=0)
		goto ERROR;
	mdio_read_value.s.reg25 = result;

	/* 6.read pseudo-phy reg26 */
	result = 0;
	if((err=mdio_read(SW_TEMPLATE_BCM_PHYAD, 26, &result))!=0)
		goto ERROR;
	mdio_read_value.s.reg26 = result;

	/* 7.read pseudo-phy reg27 */
	result = 0;
	if((err=mdio_read(SW_TEMPLATE_BCM_PHYAD, 27, &result))!=0)
		goto ERROR;
	mdio_read_value.s.reg27 = result;

	*value = mdio_read_value.u64;

	spin_unlock_bh(&SWRegLock);

	return 0;
ERROR:
	printk(KERN_EMERG "%s: (page:%x, addr:%x) fail\n", __func__, page, addr);

	spin_unlock_bh(&SWRegLock);

	return -1;
}

int Sw_Template_Sw_Write_BCMX ( Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t page, uint32_t addr, uint64_t value, uint32_t reserve ) {

	int err, timeout;
	Sw_Template_phyreg16_t reg16;
	Sw_Template_phyreg17_t reg17;
	Sw_Template_result_reg_t mdio_write_value;

	spin_lock_bh(&SWRegLock);

	/* 1.write pseudo-phy reg16 */
	reg16.u16 = 0;
	reg16.s.page_num = page;
	reg16.s.A = 1;

	if((err=mdio_write(SW_TEMPLATE_BCM_PHYAD, 16, reg16.u16))!=0)
		goto ERROR;

	/* 2.write pseudo-phy reg24 */
	mdio_write_value.u64 = value;
	if((err=mdio_write(SW_TEMPLATE_BCM_PHYAD, 24, mdio_write_value.s.reg24))!=0)
		goto ERROR;

	/* 3.write pseudo-phy reg25 */
	if((err=mdio_write(SW_TEMPLATE_BCM_PHYAD, 25, mdio_write_value.s.reg25))!=0)
		goto ERROR;

	/* 4.write pseudo-phy reg26 */
	if((err=mdio_write(SW_TEMPLATE_BCM_PHYAD, 26, mdio_write_value.s.reg26))!=0)
		goto ERROR;

	/* 5.write pseudo-phy reg27 */
	if((err=mdio_write(SW_TEMPLATE_BCM_PHYAD, 27, mdio_write_value.s.reg27))!=0)
		goto ERROR;

	/* 6.write pseudo-phy reg17 */
	reg17.u16 = 0;
	reg17.s.reg_addr = addr;
	reg17.s.op = 1;
	if((err=mdio_write(SW_TEMPLATE_BCM_PHYAD, 17, reg17.u16))!=0)
		goto ERROR;

	/* 7.read pseudo-phy reg17 */
	reg17.u16 = 0;
	timeout = 10;
	do
	{
		delay(delay_t);
		if((err=mdio_read(SW_TEMPLATE_BCM_PHYAD, 17, &reg17.u16))!=0)
			goto ERROR;
		
	}while(reg17.s.op !=0 && (--timeout>0));
	delay(delay_t);

	spin_unlock_bh(&SWRegLock);

	return 0;
ERROR:
	printk(KERN_EMERG "%s: (%x, %x) fail\n", __func__, page, addr);

	spin_unlock_bh(&SWRegLock);

	return -1;
}

int Sw_Template_Sw_Phy_Read_BCMX ( Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t page, uint32_t addr, uint64_t *value, uint32_t reserve ) {

	cvmx_smi_cmd_t smi_cmd;
	cvmx_smi_rd_dat_t smi_rd;
	uint32_t            timeout;

	smi_cmd.u64 = 0;
	smi_cmd.s.phy_op = 1;
	smi_cmd.s.phy_adr = page;
	smi_cmd.s.reg_adr = addr;

	cvmx_write_csr (CVMX_SMI_CMD, smi_cmd.u64);

	timeout = 300000000;
	/* ~1 sec at 400 MHz--ought to be enough at any speed */

	do {
		smi_rd.u64 = cvmx_read_csr (CVMX_SMI_RD_DAT);
	} while (smi_rd.s.pending && (--timeout > 0));

	if (!timeout) {
		printk ( KERN_EMERG "%s fail!!!\n", __func__ );
		return -1;
	}

	*value = smi_rd.s.dat;

	return 0;
}

int Sw_Template_Sw_Phy_Write_BCMX ( Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t page, uint32_t addr, uint64_t value, uint32_t reserve ) {

	cvmx_smi_cmd_t smi_cmd;
	cvmx_smi_wr_dat_t smi_wr;
	uint32_t            timeout;

	smi_wr.u64 = 0;
	smi_wr.s.dat = value;

	cvmx_write_csr (CVMX_SMI_WR_DAT, smi_wr.u64);

	smi_cmd.u64 = 0;
	smi_cmd.s.phy_op = 0;
	smi_cmd.s.phy_adr = page;
	smi_cmd.s.reg_adr = addr;

	cvmx_write_csr (CVMX_SMI_CMD, smi_cmd.u64);

	timeout = 3000000;
	/* ~1 sec at 400 MHz -- ought to be enough at any speed */

	do {
		smi_wr.u64 = cvmx_read_csr (CVMX_SMI_WR_DAT);
	} while (smi_wr.s.pending && (--timeout > 0));

	if (!timeout) {
		printk ( KERN_EMERG "%s fail!!!\n", __func__ );
		return -1;
	}

	return 0;
}


void Sw_Template_Get_Link_Status_BCMX ( Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t *status ) {

	uint64_t result=0;

	func_t->cs ( gpio_t, dir );
	func_t->read ( gpio_t, dir, PAGE_1, BCM539x_LINK_STATUS_REGISTER, &result, 0 );

	*status = result;
}

void Sw_Template_Get_Speed_Status_BCMX ( Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t *status ) {
	uint64_t result=0;

	func_t->cs ( gpio_t, dir );
	func_t->read ( gpio_t, dir, PAGE_1, BCM539x_SPEED_STATUS_REGISTER, &result, 0 );

	*status = result;
}

void Sw_Template_Get_Duplex_Status_BCMX ( Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t *status ) {
	uint64_t result=0;

	func_t->cs ( gpio_t, dir );
	func_t->read ( gpio_t, dir, PAGE_1, BCM539x_DUPLEX_STATUS_REGISTER, &result, 0 );

	*status = result;
}

uint64_t Sw_Template_Get_MIB_Byte_Cnt ( Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t type, uint32_t sport ) {

	uint64_t result=0;

	func_t->cs ( gpio_t, dir );
	if ( type == SW_TEMPLATE_RX ) {
		if ( useSwitch == SWITCH650 || useSwitch == SWITCH1550 )
			func_t->read ( gpio_t, dir, BCM539x_PORT0_MIB_REGISTER + sport, BCM53115M_MIB_RX_OCTETS_REGISTER, &result, 0 );
		else
			func_t->read ( gpio_t, dir, BCM539x_PORT0_MIB_REGISTER + sport, BCM539x_MIB_RX_OCTETS_REGISTER, &result, 0 );
	}
	else
		func_t->read ( gpio_t, dir, BCM539x_PORT0_MIB_REGISTER + sport, BCM539x_MIB_TX_OCTETS_REGISTER, &result, 0 );

	return result;
}

uint64_t Sw_Template_Get_MIB_Broadcast_Pkts	( Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t type, uint32_t sport ) {

	uint64_t result=0;

	func_t->cs ( gpio_t, dir );
	if ( type == SW_TEMPLATE_RX ) {
		if ( useSwitch == SWITCH650 || useSwitch == SWITCH1550 )
			func_t->read ( gpio_t, dir, BCM539x_PORT0_MIB_REGISTER + sport, BCM53115M_MIB_RX_BORADCASTPKTS_REGISTER, &result, 0 );
		else
			func_t->read ( gpio_t, dir, BCM539x_PORT0_MIB_REGISTER + sport, BCM539x_MIB_RX_BORADCASTPKTS_REGISTER, &result, 0 );
	}
	else
		func_t->read ( gpio_t, dir, BCM539x_PORT0_MIB_REGISTER + sport, BCM539x_MIB_TX_BORADCASTPKTS_REGISTER, &result, 0 );

	return result;
}

uint64_t Sw_Template_Get_MIB_Multicast_Pkts	( Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t type, uint32_t sport ) {

	uint64_t result=0;

	func_t->cs ( gpio_t, dir );
	if ( type == SW_TEMPLATE_RX ) {
		if ( useSwitch == SWITCH650 || useSwitch == SWITCH1550 )
			func_t->read ( gpio_t, dir, BCM539x_PORT0_MIB_REGISTER + sport, BCM53115M_MIB_RX_MULTICASTPKTS_REGISTER, &result, 0 );
		else
			func_t->read ( gpio_t, dir, BCM539x_PORT0_MIB_REGISTER + sport, BCM539x_MIB_RX_MULTICASTPKTS_REGISTER, &result, 0 );
	}
	else
		func_t->read ( gpio_t, dir, BCM539x_PORT0_MIB_REGISTER + sport, BCM539x_MIB_TX_MULTICASTPKTS_REGISTER, &result, 0 );

	return result;
}

uint64_t Sw_Template_Get_MIB_Unicast_Pkts	( Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t type, uint32_t sport ) {

	uint64_t result=0;

	func_t->cs ( gpio_t, dir );
	if ( type == SW_TEMPLATE_RX ) {
		if ( useSwitch == SWITCH650 || useSwitch == SWITCH1550 )
			func_t->read ( gpio_t, dir, BCM539x_PORT0_MIB_REGISTER + sport, BCM53115M_MIB_RX_UNICASTPKTS_REGISTER, &result, 0 );
		else
			func_t->read ( gpio_t, dir, BCM539x_PORT0_MIB_REGISTER + sport, BCM539x_MIB_RX_UNICASTPKTS_REGISTER, &result, 0 );
	}
	else
		func_t->read ( gpio_t, dir, BCM539x_PORT0_MIB_REGISTER + sport, BCM539x_MIB_TX_UNICASTPKTS_REGISTER, &result, 0 );

	return result;
}

uint64_t Sw_Template_Get_MIB_Collision_Pkts	( Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t type, uint32_t sport ) {

	uint64_t result=0;

	func_t->cs ( gpio_t, dir );
	if ( type == SW_TEMPLATE_TX )
		func_t->read ( gpio_t, dir, BCM539x_PORT0_MIB_REGISTER + sport, BCM539x_MIB_TX_COLLISIONS_REGISTER, &result, 0 );
	else
		result = 0;

	return result;
}

uint64_t Sw_Template_Get_MIB_Error_Pkts		( Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t type, uint32_t sport ) {

	uint64_t result=0;

	func_t->cs ( gpio_t, dir );
	if ( type == SW_TEMPLATE_RX ) {
		if ( useSwitch == SWITCH650 || useSwitch == SWITCH1550 )
			func_t->read ( gpio_t, dir, BCM539x_PORT0_MIB_REGISTER + sport, BCM53115M_MIB_RX_FCSERRORS_REGISTER, &result, 0 );
		else
			func_t->read ( gpio_t, dir, BCM539x_PORT0_MIB_REGISTER + sport, BCM539x_MIB_RX_FCSERRORS_REGISTER, &result, 0 );
	}
	else
		result = 0x0;

	return result;
}

void Sw_Template_Set_Port_Status ( Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, Sw_Template_Sw_Port_Map_t *port_map_t, uint32_t dir, struct PortStatusNStatistics status ) {

	uint32_t sport;
	uint64_t value=0;

	func_t->cs ( gpio_t, dir );
	sport = func_t->frontportmap ( port_map_t, dir, status.port );

	if(status.enabled == _DISABLE)
		value = BCM539x_MII_PORT_Disable;
	else if(status.enabled == _ENABLE && status.auto_negotiation == _ENABLE)
		value = BCM539x_MII_AUTO_NEGOTIATION;
	else if(status.enabled == _ENABLE && status.auto_negotiation == _DISABLE && status.speed == _1000Mbs)
		value = BCM539x_MII_SPEED_1000Mbps;
	else if(status.enabled == _ENABLE && status.auto_negotiation == _DISABLE && status.speed == _100Mbs && status.duplex == _FULL)
		value = BCM539x_MII_SPEEED_100Mbps_Full;
	else if(status.enabled == _ENABLE && status.auto_negotiation == _DISABLE && status.speed == _100Mbs && status.duplex == _HALF)
		value = BCM539x_MII_SPEEED_100Mbps_Half;
	else if(status.enabled == _ENABLE && status.auto_negotiation == _DISABLE && status.speed == _10Mbs && status.duplex == _FULL)
		value = BCM539x_MII_SPEEED_10Mbps_Full;
	else if(status.enabled == _ENABLE && status.auto_negotiation == _DISABLE && status.speed == _100Mbs && status.duplex == _HALF)
		value = BCM539x_MII_SPEEED_10Mbps_Half;

	func_t->phy_write ( gpio_t, dir, sport, BCM539x_MII_CONTROL_REGISTER_0, value, 0 );
}

void Sw_Template_Get_Port_Status ( Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, Sw_Template_Sw_Port_Map_t *port_map_t, uint32_t dir, struct PortStatusNStatistics *status) {

	uint32_t sport;
	uint32_t port_status;

	sport = func_t->frontportmap ( port_map_t, dir, status->port );

	func_t->get_link_status ( func_t, gpio_t, dir, &port_status );
	status->link = ( port_status >> sport ) & 0x1;

	func_t->get_speed_status ( func_t, gpio_t, dir, &port_status );
	status->speed = ( port_status >> sport*2 ) & 0x3;

	func_t->get_duplex_status ( func_t, gpio_t, dir, &port_status );
	status->duplex = ( port_status >> sport) & 0x1;

	status->recv_packet_cnt = 
		func_t->get_mib_broadcast_pkts ( func_t, gpio_t, dir, SW_TEMPLATE_RX, sport ) + 
		func_t->get_mib_multicast_pkts ( func_t, gpio_t, dir, SW_TEMPLATE_RX, sport ) + 
		func_t->get_mib_unicast_pkts (func_t, gpio_t, dir, SW_TEMPLATE_RX, sport );

	status->tran_packet_cnt = 
		func_t->get_mib_broadcast_pkts ( func_t, gpio_t, dir, SW_TEMPLATE_TX, sport ) + 
		func_t->get_mib_multicast_pkts ( func_t, gpio_t, dir, SW_TEMPLATE_TX, sport ) + 
		func_t->get_mib_unicast_pkts ( func_t, gpio_t, dir, SW_TEMPLATE_TX, sport );

	status->recv_byte_cnt = func_t->get_mib_byte_cnt ( func_t, gpio_t, dir, SW_TEMPLATE_RX, sport );

	status->tran_byte_cnt = func_t->get_mib_byte_cnt ( func_t, gpio_t, dir, SW_TEMPLATE_TX, sport );

	status->collision_cnt = func_t->get_mib_collision_pkts ( func_t, gpio_t, dir, SW_TEMPLATE_TX, sport );
	
	status->error_cnt = func_t->get_mib_error_pkts ( func_t, gpio_t, dir, SW_TEMPLATE_RX, sport );
}


void Sw_Template_Set_LED_DIAG ( uint32_t model, Sw_Template_Sw_GPIO_t *gpio_t, uint32_t state ) {

	if ( state == SW_TEMPLATE_LED_ON ) {
		Sw_Template_Set_GPIO_Value ( gpio_t->diag_active, SW_TEMPLATE_GET_MASK ( gpio_t->diag ) );
	}
	else {
		Sw_Template_Set_GPIO_Value ( ( gpio_t->diag_active == SW_TEMPLATE_GPIO_LOW )?SW_TEMPLATE_GPIO_HIGH:SW_TEMPLATE_GPIO_LOW, SW_TEMPLATE_GET_MASK ( gpio_t->diag ) );
	}
}

void Sw_Template_Set_LED_DMZ ( uint32_t model, Sw_Template_Sw_GPIO_t *gpio_t, uint32_t state ) {

	if ( state == SW_TEMPLATE_LED_ON ) {
		Sw_Template_Set_GPIO_Value ( gpio_t->dmz_active, SW_TEMPLATE_GET_MASK ( gpio_t->dmz ) );
	}
	else {
		Sw_Template_Set_GPIO_Value ( ( gpio_t->dmz_active == SW_TEMPLATE_GPIO_LOW )?SW_TEMPLATE_GPIO_HIGH:SW_TEMPLATE_GPIO_LOW, SW_TEMPLATE_GET_MASK ( gpio_t->dmz ) );
	}
}

void Sw_Template_Seri_Send_0 ( uint32_t model, Sw_Template_Sw_GPIO_t *gpio_t ) {

	Sw_Template_Set_GPIO_Value ( SW_TEMPLATE_GPIO_LOW, SW_TEMPLATE_GET_MASK ( gpio_t->seri_clk ) | SW_TEMPLATE_GET_MASK ( gpio_t->seri_dat ) );
// 	Sw_Template_Delay ( 10 );

	Sw_Template_Set_GPIO_Value ( SW_TEMPLATE_GPIO_HIGH, SW_TEMPLATE_GET_MASK ( gpio_t->seri_clk ) );
// 	Sw_Template_Delay ( 10 );

	Sw_Template_Set_GPIO_Value ( SW_TEMPLATE_GPIO_LOW, SW_TEMPLATE_GET_MASK ( gpio_t->seri_clk ) );
// 	Sw_Template_Delay ( 10 );
}

void Sw_Template_Seri_Send_1 ( uint32_t model, Sw_Template_Sw_GPIO_t *gpio_t ) {

	Sw_Template_Set_GPIO_Value ( SW_TEMPLATE_GPIO_HIGH, SW_TEMPLATE_GET_MASK ( gpio_t->seri_dat ) );
	Sw_Template_Set_GPIO_Value ( SW_TEMPLATE_GPIO_LOW, SW_TEMPLATE_GET_MASK ( gpio_t->seri_clk ) );
// 	Sw_Template_Delay ( 10 );

	Sw_Template_Set_GPIO_Value ( SW_TEMPLATE_GPIO_HIGH, SW_TEMPLATE_GET_MASK ( gpio_t->seri_clk ) );
// 	Sw_Template_Delay ( 10 );

	Sw_Template_Set_GPIO_Value ( SW_TEMPLATE_GPIO_LOW, SW_TEMPLATE_GET_MASK ( gpio_t->seri_clk ) );
// 	Sw_Template_Delay ( 10 );
}

void Sw_Template_Chagne_LED_Func ( uint32_t model, Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t sport, uint32_t state ) {

	uint64_t result;

	func_t->cs ( gpio_t, dir );
	func_t->read ( gpio_t, dir, 0x0, 0x14, &result, 0 );

	if ( state == 0x0 ) {/* ON */
		func_t->cs ( gpio_t, dir );
		result |= ( 1 << sport );
		func_t->write ( gpio_t, dir, 0x0, 0x14, result, 0 );/* Select LED function 1, it means LED ON mode */
	}
	else {/* OFF */
		func_t->cs ( gpio_t, dir );
		result &= ~( 1 << sport );
		func_t->write ( gpio_t, dir, 0x0, 0x14, result, 0 );/* Select LED function 0, it means LED ON mode */
	}
}

void Sw_Template_LED_Seri ( uint32_t model, Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, Sw_Template_Sw_Port_Map_t *port_map_t, uint32_t state ) {

	int i;
	uint32_t sport;

	if ( useSwitch == SWITCH1450 ) {
		if ( !( state & 0xFFFF0000 ) )/* It means turn off Seri LED, 1450 has Seri LED but does not use it, so must turn off it */
			goto NORMAL_SERI;
		if ( Sw_Template_LED_Seri_Old_State ^ state ) {/* Use XOR to fine changed WAN */
			for ( i=0; i < CONFIG_NK_NUM_WAN; i++ ) {
				if ( ( Sw_Template_LED_Seri_Old_State ^ state ) & ( 1 << i ) ) {
					sport = func_t->frontportmap ( port_map_t, SW_TEMPLATE_CS_WAN, ( i + 1 ) );
					Sw_Template_Chagne_LED_Func ( model, func_t, gpio_t, SW_TEMPLATE_CS_WAN, sport, ( state & ( 1 << i ) ) );
				}
			}
		}
		else {
			/* Not changed, do nothing */
		}
		Sw_Template_LED_Seri_Old_State = state;
	}
	else {
NORMAL_SERI:
		/* Use Serial Control WAN Connect LED */
		Sw_Template_LED_Seri_Old_State = state;
	
		if ( gpio_t->seri_bit_pri == SW_TEMPLATE_SERI_LOW_BIT_FIRST ) {
			for ( i=0; i < gpio_t->seri_no; i++ ) {
				if ( ( state >> i ) & 0x1 )
					Sw_Template_Seri_Send_1 ( model, gpio_t );
				else
					Sw_Template_Seri_Send_0 ( model, gpio_t );
			}
		}
		else {
			for ( i=(gpio_t->seri_no-1); i >= 0; i-- ) {
				if ( ( state >> i ) & 0x1 )
					Sw_Template_Seri_Send_1 ( model, gpio_t );
				else
					Sw_Template_Seri_Send_0 ( model, gpio_t );
			}
		}
	}
}

void Sw_Template_Set_LED ( uint32_t model, Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, Sw_Template_Sw_Port_Map_t *port_map_t, uint32_t type, uint32_t state ) {

	if ( type == SW_TEMPLATE_DIAG_LED ) {
		Sw_Template_Set_LED_DIAG ( model, gpio_t, state );
	}
	else if ( type == SW_TEMPLATE_DMZ_LED ) {
		if ( gpio_t->dmz_seri ) {
			uint32_t tmp_state, dmz_mask;

			dmz_mask = ( 1 << ( CONFIG_NK_NUM_WAN + CONFIG_NK_NUM_DMZ -1 ) );

			if ( state == SW_TEMPLATE_LED_ON )
				tmp_state = Sw_Template_LED_Seri_Old_State & (~dmz_mask);
			else
				tmp_state = Sw_Template_LED_Seri_Old_State | dmz_mask;

			Sw_Template_LED_Seri ( model, func_t, gpio_t, port_map_t, tmp_state );
		}
		else
			Sw_Template_Set_LED_DMZ ( model, gpio_t, state );
	}
	else if ( type == SW_TEMPLATE_SERI_LED ) {
		Sw_Template_LED_Seri ( model, func_t, gpio_t, port_map_t, state );
	}
}


void Sw_Template_Set_Mirror_Port ( uint32_t model, Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, Sw_Template_Sw_Port_Map_t *port_map_t, uint32_t fport ) {

	uint64_t sport;

	if ( fport > 0 ) {/* Enable Mirror Port */
		sport = func_t->frontportmap ( port_map_t, SW_TEMPLATE_CS_LAN, fport );
		func_t->cs ( gpio_t, SW_TEMPLATE_CS_LAN );
		func_t->write ( gpio_t, SW_TEMPLATE_CS_LAN, BCM539X_MANAGEMENT_MIRRORING_REGISTER, BMC539X_MIRROR_CAPTURE_CONTROL_REGISTER, ( sport | 0x8000 ), 0 );
	}
	else {/* Disable Mirror Port */
		func_t->cs ( gpio_t, SW_TEMPLATE_CS_LAN );
		func_t->write ( gpio_t, SW_TEMPLATE_CS_LAN, BCM539X_MANAGEMENT_MIRRORING_REGISTER, BMC539X_MIRROR_CAPTURE_CONTROL_REGISTER, 0x0, 0 );
	}
}

void Sw_Template_Set_Static_MAC ( uint32_t model, Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, Sw_Template_ARL_Data_t arl_data ) {

	uint64_t tmp64=0;
	uint32_t dir;

	dir = ( ( arl_data.is_lan ) ? SW_TEMPLATE_CS_LAN : SW_TEMPLATE_CS_WAN );
	func_t->cs ( gpio_t, dir );
	/* 1. Set (05h,02h) MAC Address Index */
	func_t->write ( gpio_t, dir, 0x05, 0x02, arl_data.mac_index, 0 );

	/* 2. Set (05h,08h) VID Index */
	func_t->write ( gpio_t, dir, 0x05, 0x08, arl_data.vid_index, 0);

	/* 3. Set (05h,10h) ARL Table MAC/VID Entry */
	tmp64 = arl_data.vid_index;
	tmp64 = (tmp64<<48)+arl_data.mac_index;
	func_t->write ( gpio_t, dir, 0x05, 0x10, tmp64, 0);

	/* 4. Set (05h,18h) ARL Table Data Entry */
	func_t->write ( gpio_t, dir, 0x05, 0x18, arl_data.data_value, 0);

	/* 5. Set (05h,00h) ARL_R/W bit 0(Write Cmd) and START?DONE bit 1(START) */
	func_t->write ( gpio_t, dir, 0x05, 0x00, 0x80, 0);
}

/** RTL8306 Drvier **/
int smiRead(unsigned int phyad, unsigned int regad, unsigned int * data) {

	cvmx_smi_cmd_t      smi_cmd;
	cvmx_smi_rd_dat_t   smi_rd;
	uint32_t            timeout;

	smi_cmd.u64 = 0;
	smi_cmd.s.phy_op = 1;
	smi_cmd.s.phy_adr = phyad;
	smi_cmd.s.reg_adr = regad;

	cvmx_write_csr (CVMX_SMI_CMD, smi_cmd.u64);

	timeout = 300000000;
	/* ~1 sec at 400 MHz--ought to be enough at any speed */

	do {
		smi_rd.u64 = cvmx_read_csr (CVMX_SMI_RD_DAT);
	} while (smi_rd.s.pending && (--timeout > 0));

	if (!timeout) {
		printk (KERN_EMERG "%s: timeout\n", __func__ );
		return (-1);
	}

	*data = smi_rd.s.dat;

	return 0;
}

int smiWrite(unsigned int phyad, unsigned int regad, unsigned int data) {

	cvmx_smi_cmd_t      smi_cmd;
	cvmx_smi_wr_dat_t   smi_wr;
	uint32_t            timeout;

	smi_wr.u64 = 0;
	smi_wr.s.dat = data;

	cvmx_write_csr (CVMX_SMI_WR_DAT, smi_wr.u64);

	smi_cmd.u64 = 0;
	smi_cmd.s.phy_op = 0;
	smi_cmd.s.phy_adr = phyad;
	smi_cmd.s.reg_adr = regad;

	cvmx_write_csr (CVMX_SMI_CMD, smi_cmd.u64);

	timeout = 3000000;
	/* ~1 sec at 400 MHz -- ought to be enough at any speed */

	do {
		smi_wr.u64 = cvmx_read_csr (CVMX_SMI_WR_DAT);
	} while (smi_wr.s.pending && (--timeout > 0));

	if (!timeout) {
		printk (KERN_EMERG "%s: timeout\n", __func__ );
		return (-1);
	}

	return 0;
}

int Sw_Template_Sw_Read_RTL8306 ( Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t phyad, uint32_t regad, uint64_t *value, uint32_t npage ) {

	unsigned int result=0;

	rtl8306_getAsicPhyReg ( phyad, regad, npage, &result );
	*value = result;

	return 0;
}

int Sw_Template_Sw_Write_RTL8306 ( Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t phyad, uint32_t regad, uint64_t value, uint32_t npage ) {

	unsigned int result=0;

	result = value;
	rtl8306_setAsicPhyReg ( phyad, regad, npage, result );

	return 0;
}


void Sw_Template_Get_Link_Status_RTL8306 ( Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t *status ) {

	int i, loop;
	uint32_t link_status=0;

	loop = 5;/* RT8306 has 5 ports */

	func_t->cs ( gpio_t, dir );

	for ( i=0; i < loop; i++ ) {
		rtl8306_getAsicPHYLinkStatus ( i, &link_status );
		*status |= link_status << i;
	}
}

void Sw_Template_Get_Speed_Status_RTL8306 ( Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t *status ) {


}

void Sw_Template_Get_Duplex_Status_RTL8306 ( Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t *status ) {


}


uint64_t Sw_Template_Get_MIB_Byte_Cnt_RTL8306 ( Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t type, uint32_t sport ) {

	return 0;
}

uint64_t Sw_Template_Get_MIB_Broadcast_Pkts_RTL8306 ( Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t type, uint32_t sport ) {

	return 0;
}

uint64_t Sw_Template_Get_MIB_Multicast_Pkts_RTL8306 ( Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t type, uint32_t sport ) {

	return 0;
}

uint64_t Sw_Template_Get_MIB_Unicast_Pkts_RTL8306 ( Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t type, uint32_t sport ) {

	return 0;
}

uint64_t Sw_Template_Get_MIB_Collision_Pkts_RTL8306 ( Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t type, uint32_t sport ) {

	return 0;
}

uint64_t Sw_Template_Get_MIB_Error_Pkts_RTL8306 ( Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t type, uint32_t sport ) {

	return 0;
}


void Sw_Template_Set_Port_Status_RTL8306 ( Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, Sw_Template_Sw_Port_Map_t *port_map_t, uint32_t dir, struct PortStatusNStatistics status ) {

	uint32_t sport;

	func_t->cs ( gpio_t, dir );
	sport = func_t->frontportmap ( port_map_t, dir, status.port );

	/* For Debug */
// 	printk ( KERN_EMERG "%s: FPort[%d], SPort[%d], %s, %s, %s, %s, %s\n",
// 		__func__,
// 		status.port, sport,
// 		dir==SW_TEMPLATE_CS_LAN?"LAN":"WAN",
// 		status.enabled==_DISABLE?"DISABLE":"ENABLE",
// 		status.auto_negotiation==_ENABLE?"ENABLE":"DISABLE",
// 		status.speed==_1000Mbs?"1000Mbs":"100/10Mbs",
// 		status.duplex==_FULL?"FULL":"HALF" );

	if(status.enabled == _DISABLE)
		rtl8306_setAsicPortPowerDown(sport, 1);
	else if(status.enabled == _ENABLE && status.auto_negotiation == _ENABLE) {
		rtl8306_setAsicPortPowerDown(sport, 0);
		rtl8306_setEthernetPHY(sport, TRUE, RTL8306_ETHER_AUTO_100FULL, RTL8306_ETHER_SPEED_100, TRUE);
	}
	else if(status.enabled == _ENABLE && status.auto_negotiation == _DISABLE && status.speed == _1000Mbs) {
	}
	else if(status.enabled == _ENABLE && status.auto_negotiation == _DISABLE && status.speed == _100Mbs && status.duplex == _FULL) {
		rtl8306_setAsicPortPowerDown(sport, 0);
		rtl8306_setEthernetPHY(sport, FALSE, RTL8306_ETHER_AUTO_100FULL, RTL8306_ETHER_SPEED_100, FALSE);
	}
	else if(status.enabled == _ENABLE && status.auto_negotiation == _DISABLE && status.speed == _100Mbs && status.duplex == _HALF) {
		rtl8306_setAsicPortPowerDown(sport, 0);
		rtl8306_setEthernetPHY(sport, FALSE, RTL8306_ETHER_AUTO_100HALF, RTL8306_ETHER_SPEED_100, FALSE);
	}
	else if(status.enabled == _ENABLE && status.auto_negotiation == _DISABLE && status.speed == _10Mbs && status.duplex == _FULL) {
		rtl8306_setAsicPortPowerDown(sport, 0);
		rtl8306_setEthernetPHY(sport, FALSE, RTL8306_ETHER_AUTO_10FULL, RTL8306_ETHER_SPEED_10, FALSE);
	}
	else if(status.enabled == _ENABLE && status.auto_negotiation == _DISABLE && status.speed == _100Mbs && status.duplex == _HALF) {
		rtl8306_setAsicPortPowerDown(sport, 0);
		rtl8306_setEthernetPHY(sport, FALSE, RTL8306_ETHER_AUTO_10HALF, RTL8306_ETHER_SPEED_10, FALSE);
	}

	rtl8306_setAsicQosPortBasedPriority ( sport, status.priority);
}

void Sw_Template_Get_Port_Status_RTL8306 ( Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, Sw_Template_Sw_Port_Map_t *port_map_t, uint32_t dir, struct PortStatusNStatistics *status) {

	uint32_t sport;
	uint32_t port_status;
	uint32_t autoNegotiation, flowCtl, speed, fullDuplex, unit;

	sport = func_t->frontportmap ( port_map_t, dir, status->port );

	func_t->cs ( gpio_t, dir );

	rtl8306_getAsicPHYLinkStatus ( sport, &port_status );
	status->link = port_status;

	rtl8306_getEthernetStatus ( sport, &autoNegotiation, &flowCtl, &speed, &fullDuplex );

	status->speed = speed;
	status->duplex = fullDuplex;

	rtl8306_setAsicPhyRegBit ( sport, 17, 4, 2, 1 );//set packet count
	rtl8306_getAsicMibCounter ( sport, RTL8306_MIB_CNT2, &( status->recv_packet_cnt ) );
	rtl8306_getAsicMibCounterUnit ( sport, RTL8306_MIB_CNT2, &unit );

	rtl8306_setAsicPhyRegBit ( sport, 17, 3, 2, 1 );
	rtl8306_getAsicMibCounter ( sport, RTL8306_MIB_CNT1, & ( status->tran_packet_cnt ) );
	rtl8306_getAsicMibCounterUnit ( sport, RTL8306_MIB_CNT1, &unit );

	rtl8306_getAsicMibCounter ( sport, RTL8306_MIB_CNT3, & ( status->collision_cnt ) );

	rtl8306_getAsicMibCounter ( sport, RTL8306_MIB_CNT5, & ( status->error_cnt ) );

	rtl8306_getAsicQosPortBasedPriority ( sport, & (status->priority ) );
}


void Sw_Template_Set_Port_VLAN_RTL8306 ( uint32_t model, Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, Sw_Template_Sw_Port_Map_t *port_map_t, uint32_t dir, uint32_t member, uint32_t fport ) {

}

void Sw_Template_Set_VLAN_Tag_RTL8306 ( uint32_t model, struct _Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, Sw_Template_Sw_Port_Map_t *port_map_t, uint32_t dir, struct PortStatusNStatistics status ) {

	int i, j;
	uint32_t sport, vlan_group_base;

	/*	VLAN ID from ( CONFIG_NK_NUM_LAN + CONFIG_NK_NUM_WAN + CONFIG_NK_NUM_DMZ + 1 ) ~ ( CONFIG_NK_NUM_LAN*2 + CONFIG_NK_NUM_WAN + CONFIG_NK_NUM_DMZ ) 
		Ex: SSL005
	*/

	vlan_group_base = CONFIG_NK_NUM_LAN + CONFIG_NK_NUM_WAN + CONFIG_NK_NUM_DMZ;

	sport = func_t->frontportmap ( port_map_t, dir, status.port );

	if ( status.vlangroup.vlanid != 77 ) {

		for ( i=1; i <= CONFIG_NK_NUM_LAN; i++ ) {

			if ( ( status.vlangroup.member_port ) & ( 1 << i ) ) {

				func_t->cs ( gpio_t, SW_TEMPLATE_CS_LAN );
				for ( j=1; j <= CONFIG_NK_NUM_LAN; j++ ) {

					rtl8306_delVlanPortMember ( vlan_group_base + j, func_t->frontportmap ( port_map_t, dir, i ) );
				}
				func_t->cs ( gpio_t, SW_TEMPLATE_CS_LAN );

				rtl8306_addVlanPortMember ( status.vlangroup.vlanid, func_t->frontportmap ( port_map_t, dir, i ) );
			}
		}
		/* For Debug */
// 		printk ( KERN_EMERG "%s: Set LAN_%d, Sport_%d, VID[%d], memeber[0x%x]\n", __func__, status.port, sport, status.vlangroup.vlanid, status.vlangroup.member_port );
		rtl8306_setPvid ( sport, status.vlangroup.vlanid );
	}
	else {/* VLAN ALL */

		for ( i=1; i <= CONFIG_NK_NUM_LAN; i++ ) {

			func_t->cs ( gpio_t, SW_TEMPLATE_CS_LAN );
			rtl8306_addVlanPortMember ( vlan_group_base + i, sport );
		}
		func_t->cs ( gpio_t, SW_TEMPLATE_CS_LAN );
		rtl8306_setPvid(sport, 1);
	}

}


void Sw_Template_Set_VLAN_Group_RTL8306 ( uint32_t model, struct _Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, uint32_t dir, uint32_t ggroup_index, Sw_Template_VLAN_Table_Entry_t data ) {

}


void Sw_Template_Set_Mirror_Port_RTL8306 ( uint32_t model, Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, Sw_Template_Sw_Port_Map_t *port_map_t, uint32_t fport ) {

	uint32_t sport;
	rtl8306_mirrorPara_t mir;
	memset ( &mir, 0, sizeof ( rtl8306_mirrorPara_t ) );

	if ( fport > 0 ) {/* Enable Mirror Port */
		sport = func_t->frontportmap ( port_map_t, SW_TEMPLATE_CS_LAN, fport );

		mir.mirport = sport;
		mir.rxport = 0x20;
		mir.txport = 0x20;
		mir.enMirMac = FALSE;
		func_t->cs ( gpio_t, SW_TEMPLATE_CS_LAN );
		rtl8306_setMirror(mir);
	}
	else {/* Disable Mirror Port */
		mir.mirport = 7;
		mir.enMirMac = FALSE;
		func_t->cs ( gpio_t, SW_TEMPLATE_CS_LAN );
		rtl8306_setMirror(mir);
	}
}


/**
	BCM53115M ACL Rule Control
**/

void Show_ACL_Conf ( BCM_ACL_Control_t *cmd ) {

	nk_switch_acl_print ( "------------Switch Driver BCM ACL Conf(%d)\n", BCM_ACL_Entry );
	nk_switch_acl_print ( "L3 Type: %d\n", cmd->type );
	nk_switch_acl_print ( "MAC Type: %d\n", cmd->mac_type );
	nk_switch_acl_print ( "SRC MAC: 0x%012lx\n", cmd->src_mac );
	nk_switch_acl_print ( "DST MAC: 0x%012lx\n", cmd->dst_mac );
	nk_switch_acl_print ( "IP Type: %d\n", cmd->ip_type );
	nk_switch_acl_print ( "SRC IP: %lx\n", cmd->src_ip );
	nk_switch_acl_print ( "DST IP: %lx\n", cmd->dst_ip );
	nk_switch_acl_print ( "IP PROTOCOL: %d\n", cmd->ip_proto );
	nk_switch_acl_print ( "SRC Port Type: %d\n", cmd->srcport_type );
	nk_switch_acl_print ( "SRC Port:%ld\n", cmd->src_port );
	nk_switch_acl_print ( "DST Port Type: %d\n", cmd->dstport_type );
	nk_switch_acl_print ( "DST Port: %ld\n", cmd->dst_port );
	nk_switch_acl_print ( "Ether Type: 0x%04x\n", cmd->eth_type );
	nk_switch_acl_print ( "Action Type: %d\n", cmd->act_type );
	nk_switch_acl_print ( "Rate: 0x%x\n", cmd->rate );
	nk_switch_acl_print ( "Priority: %d\n", cmd->prio );
	nk_switch_acl_print ( "----------------------------------\n" );

}

/**
	Reset CFP(Clear CFP RAM and TCAM Reset)
**/
void BCM_ACL_Reset_CFP ( uint32_t model, Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, Sw_Template_Sw_Port_Map_t *port_map_t, uint32_t dir ) {

	nk_switch_acl_print ( "## Reset CFP(Clear CFP RAM and TCAM Reset)\n" );
	nk_switch_acl_print ( "s 0xA0 0x00 0x%08X\n", CFP_RAM_CLEAR | TCAM_RESET );

	func_t->cs ( gpio_t, dir );
	func_t->write ( gpio_t, dir, 0xA0, 0x00, CFP_RAM_CLEAR | TCAM_RESET, 0 );
}

void BCM_ACL_Enable_Port ( uint32_t model, Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, Sw_Template_Sw_Port_Map_t *port_map_t, uint32_t dir, uint32_t pbm ) {

	nk_switch_acl_print ( "## Enable Per Port\n" );
	nk_switch_acl_print ( "s 0xA1 0x00 0x%08X\n", pbm );

	func_t->cs ( gpio_t, dir );
	func_t->write ( gpio_t, dir, 0xA1, 0x00, pbm, 0 );
}

void BCM_ACL_Print_UDF_Conf ( uint32_t model, Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, Sw_Template_Sw_Port_Map_t *port_map_t, uint32_t dir, int slice, int l3_type ) {
	int i, t1, t2;

	nk_switch_acl_print ( "\n## UDF Configuration\n");
	if ( slice == SLICE_NULL ) {
		for ( t1=0; t1<3; t1++ ) {
			for ( t2=0; t2<3; t2++ ) {
				for ( i=0; i <= 8; i++ ) {
					nk_switch_acl_print ( "s 0xA1 0x%x 0x%02x\n", ( 0x10 + ( t1 * 0x10 ) + ( t2 * 0x30 ) + i), 0x0 );
					func_t->cs ( gpio_t, dir );
					func_t->write ( gpio_t, dir, 0xA1, ( 0x10 + ( t1 * 0x10 ) + ( t2 * 0x30 ) + i), 0x0, 0 );
				}
			}
		}
	}
	else {
		for ( i=0; i <= 8; i++ ) {
			nk_switch_acl_print ( "s 0xA1 0x%x 0x%02x\n", ( 0x10 + ( slice * 0x10 ) + ( l3_type * 0x30 ) + i), BCM_ACL_CFP_n_X_Conf[slice][l3_type][i] );
			func_t->cs ( gpio_t, dir );
			func_t->write ( gpio_t, dir, 0xA1, ( 0x10 + ( slice * 0x10 ) + ( l3_type * 0x30 ) + i ), BCM_ACL_CFP_n_X_Conf[slice][l3_type][i], 0 );
		}
	}

}

void BCM_ACL_Init_UDF_Conf ( uint32_t model, Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, Sw_Template_Sw_Port_Map_t *port_map_t, uint32_t dir ) {

	int i, j;

	for ( i=0; i < 3; i++ ) {
		for ( j=0; j < 3; j++ ) {
			memset ( BCM_ACL_CFP_n_X_Conf[i][j], 0, sizeof ( unsigned int ) * 9 );
		}
	}

/* IPV4 */
	/* SRC MAC */
	BCM_ACL_CFP_n_X_Conf[SLICE0][L3_IPV4][0] = START_OF_PACKET | 5;
	BCM_ACL_CFP_n_X_Conf[SLICE0][L3_IPV4][1] = START_OF_PACKET | 4;
	BCM_ACL_CFP_n_X_Conf[SLICE0][L3_IPV4][2] = START_OF_PACKET | 3;
	/* SRC IP */
	BCM_ACL_CFP_n_X_Conf[SLICE0][L3_IPV4][3] = END_OF_L2 | 7;
	BCM_ACL_CFP_n_X_Conf[SLICE0][L3_IPV4][4] = END_OF_L2 | 6;
	/* SRC Port */
	BCM_ACL_CFP_n_X_Conf[SLICE0][L3_IPV4][5] = END_OF_L3 | 0;
	/* DST Port */
	BCM_ACL_CFP_n_X_Conf[SLICE0][L3_IPV4][6] = END_OF_L3 | 1;

	/* DST MAC */
	BCM_ACL_CFP_n_X_Conf[SLICE1][L3_IPV4][0] = START_OF_PACKET | 2;
	BCM_ACL_CFP_n_X_Conf[SLICE1][L3_IPV4][1] = START_OF_PACKET | 1;
	BCM_ACL_CFP_n_X_Conf[SLICE1][L3_IPV4][2] = START_OF_PACKET | 0;
	/* DST IP */
	BCM_ACL_CFP_n_X_Conf[SLICE1][L3_IPV4][3] = END_OF_L2 | 9;
	BCM_ACL_CFP_n_X_Conf[SLICE1][L3_IPV4][4] = END_OF_L2 | 8;
	/* SRC Port */
	BCM_ACL_CFP_n_X_Conf[SLICE1][L3_IPV4][5] = END_OF_L3 | 0;
	/* DST Port */
	BCM_ACL_CFP_n_X_Conf[SLICE1][L3_IPV4][6] = END_OF_L3 | 1;

	BCM_ACL_Print_UDF_Conf ( model, func_t, gpio_t, port_map_t, dir, SLICE0, L3_IPV4 );
	BCM_ACL_Print_UDF_Conf ( model, func_t, gpio_t, port_map_t, dir, SLICE1, L3_IPV4 );

/* NonIP */
	/* SRC MAC */
	BCM_ACL_CFP_n_X_Conf[SLICE0][L3_NONIP][0] = START_OF_PACKET | 5;
	BCM_ACL_CFP_n_X_Conf[SLICE0][L3_NONIP][1] = START_OF_PACKET | 4;
	BCM_ACL_CFP_n_X_Conf[SLICE0][L3_NONIP][2] = START_OF_PACKET | 3;

	/* DST MAC */
	BCM_ACL_CFP_n_X_Conf[SLICE1][L3_NONIP][0] = START_OF_PACKET | 2;
	BCM_ACL_CFP_n_X_Conf[SLICE1][L3_NONIP][1] = START_OF_PACKET | 1;
	BCM_ACL_CFP_n_X_Conf[SLICE1][L3_NONIP][2] = START_OF_PACKET | 0;

	BCM_ACL_Print_UDF_Conf ( model, func_t, gpio_t, port_map_t, dir, SLICE0, L3_NONIP );
	BCM_ACL_Print_UDF_Conf ( model, func_t, gpio_t, port_map_t, dir, SLICE1, L3_NONIP );
}

void BCM_ACL_CFP_COPY_UDF ( unsigned int tmp, unsigned int *loc, int idx, unsigned int tmp_mask ) {

	//Set DATA/MASK
	if ( loc[0] == loc[1] ) {
		BCM_ACL_CFP_TCAM_DATA[ loc[0] ] |= tmp << 8;
		BCM_ACL_CFP_TCAM_MASK[ loc[0] ] |= tmp_mask << 8;
	}
	else
	{
		BCM_ACL_CFP_TCAM_DATA[ loc[0] ] |= ( tmp & 0x00FF ) << 24;
		BCM_ACL_CFP_TCAM_MASK[ loc[0] ] |= ( tmp_mask & 0x00FF ) << 24;

		BCM_ACL_CFP_TCAM_DATA[ loc[1] ] |= ( tmp & 0xFF00 ) >> 8;
		BCM_ACL_CFP_TCAM_MASK[ loc[1] ] |= ( tmp_mask & 0xFF00 ) >> 8;
	}

	//Set Valid
	if ( idx == 8 ) {
		BCM_ACL_CFP_TCAM_DATA[6] |= 0x1;
		BCM_ACL_CFP_TCAM_MASK[6] |= 0x1;
	}
	else {
		BCM_ACL_CFP_TCAM_DATA[5] |= ( 0x1 << idx ) << OFFSET_UDF_VALID;
		BCM_ACL_CFP_TCAM_MASK[5] |= ( 0x1 << idx ) << OFFSET_UDF_VALID;
	}
}

void BCM_ACL_CFP_Set_UDF ( uint64_t Arg, uint64_t Arg_Mask, int start, int bytes ) {

	int i, max_idx=( bytes/2 );

	for ( i=0; i < max_idx; i++ ) {
		BCM_ACL_CFP_n_X[i+start] = ( Arg >> i*16 ) & 0xFFFF;
		BCM_ACL_CFP_n_X_Mask[i+start] = ( Arg_Mask >> i*16 ) & 0xFFFF;
		BCM_ACL_CFP_COPY_UDF ( BCM_ACL_CFP_n_X[i+start], BCM_ACL_CFP_n_X_Map[i+start], i+start,  BCM_ACL_CFP_n_X_Mask[i+start] );
	}
}


void BCM_ACL_Set_Src_Port_Map ( unsigned int pbm ) {

	if ( pbm == ALL_PORT_MAP ) {
		//All port means that we do not care ports
	}
	else {
		BCM_ACL_CFP_TCAM_DATA[7] |= ( pbm << OFFSET_SRC_PORT_MAP );
		BCM_ACL_CFP_TCAM_MASK[7] |= ( pbm << OFFSET_SRC_PORT_MAP );
	}
}

void BCM_ACL_Set_L3_Framing ( unsigned int l3_type ) {
	unsigned int l3;

	if ( l3_type == L3_NONIP )
		l3 = 0x03;
	else
		l3 = l3_type;

	if ( l3_type == L3_NULL ) {
		//Do nothing
	}
	else {
		BCM_ACL_CFP_TCAM_DATA[6] |= ( l3 << OFFSET_L3_FRAMING );
		BCM_ACL_CFP_TCAM_MASK[6] |= ( 0x3 << OFFSET_L3_FRAMING );
	}
}

void Set_IP_Protocol ( unsigned int proto ) {

	if ( proto == 0 )
		return;

	BCM_ACL_CFP_TCAM_DATA[6] |= ( proto << OFFSET_IP_PROTOCOL );
	BCM_ACL_CFP_TCAM_MASK[6] |= ( 0xFF << OFFSET_IP_PROTOCOL );
}

void Set_Ether_Type ( unsigned int ether ) {

	if ( ether == 0 )
		return;

	BCM_ACL_CFP_TCAM_DATA[6] |= ( ether << OFFSET_ETHER_TYPE );
	BCM_ACL_CFP_TCAM_MASK[6] |= ( 0xFFFF << OFFSET_ETHER_TYPE );
}

void BCM_ACL_Set_Slice ( int slice ) {

	if ( slice == SLICE_NULL ) {
		BCM_ACL_CFP_TCAM_DATA[0] |= 0x3;
		BCM_ACL_CFP_TCAM_MASK[0] |= 0x3;
	}
	else {
		BCM_ACL_CFP_TCAM_DATA[0] |= ( slice << OFFSET_SLICE_ID ) | 0x3;
		BCM_ACL_CFP_TCAM_MASK[0] |= ( 0x3 << OFFSET_SLICE_ID ) | 0x3;
	}
}

void BCM_ACL_Set_TCAM_Conf ( unsigned int pbm, unsigned int l3_type, int slice ) {

	BCM_ACL_Set_Src_Port_Map ( pbm );
	BCM_ACL_Set_L3_Framing ( l3_type );
	BCM_ACL_Set_Slice ( slice );

// 	BCM_ACL_CFP_TCAM_DATA[6] |= 0x3 << 28;
// 	BCM_ACL_CFP_TCAM_MASK[6] |= 0x3 << 28;
// 
// 	BCM_ACL_CFP_TCAM_DATA[5] |= 0x1;
// 	BCM_ACL_CFP_TCAM_MASK[5] |= 0xFF;
}

void BCM_ACL_Set_Action ( uint32_t model, Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, Sw_Template_Sw_Port_Map_t *port_map_t, uint32_t dir, int type, unsigned int arg1 ) {

	memset ( BCM_ACL_Act_Reg, 0, sizeof ( unsigned int ) * 2 );
	switch ( type ) {
		case RPDROP:
			BCM_ACL_Act_Reg[0] = ( CHANGE_FWD_OB_RDIR | NULL_OB ) | ( CHANGE_FWD_IB_RDIR | NULL_IB );
			break;
		case DROP_OB:
			BCM_ACL_Act_Reg[0] = ( CHANGE_FWD_OB_RDIR | NULL_OB );
			break;
		case CHANGE_TC:
			BCM_ACL_Act_Reg[1] = ( 1 << OFFSET_CHANGE_TC ) | ( arg1 << OFFSET_NEW_TC );
			break;
		case OB_COPY:
			BCM_ACL_Act_Reg[0] = ( CHANGE_FWD_OB_COPY ) | ( arg1 << OFFSET_DST_MAP_OB );
			break;
	}

	nk_switch_acl_print ( "## Action\n" );
	nk_switch_acl_print ( "s 0xA0 0x50 0x%08x\n", BCM_ACL_Act_Reg[0]);
	nk_switch_acl_print ( "s 0xA0 0x54 0x%08x\n", BCM_ACL_Act_Reg[1]);

	func_t->cs ( gpio_t, dir );
	func_t->write ( gpio_t, dir, 0xA0, 0x50, BCM_ACL_Act_Reg[0], 0 );
	func_t->write ( gpio_t, dir, 0xA0, 0x54, BCM_ACL_Act_Reg[1], 0 );
}

void BCM_ACL_Start_ACT ( uint32_t model, Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, Sw_Template_Sw_Port_Map_t *port_map_t, uint32_t dir, unsigned int entry ) {

	unsigned int value;

	value = ( ( entry & 0xFF ) << OFFSET_ACCESS_ADDR ) | ACT_RAM | WRITE_OP | OP_START;

	nk_switch_acl_print ( "## Start Action and Set Entry(%d)\n", entry);
	nk_switch_acl_print ( "s 0xA0 0x00 0x%08x\n", value);

	func_t->cs ( gpio_t, dir );
	func_t->write ( gpio_t, dir, 0xA0, 0x00, value, 0 );
}

void BCM_ACL_Print_TCAM ( uint32_t model, Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, Sw_Template_Sw_Port_Map_t *port_map_t, uint32_t dir ) {
	int i;

	nk_switch_acl_print ( "## TCAM DATA\n");
	for ( i=0; i<8; i++ ) {
		nk_switch_acl_print ( "s 0xA0 0x%x 0x%08x\n", ( 0x10 + i*4 ), BCM_ACL_CFP_TCAM_DATA[i] );
		func_t->cs ( gpio_t, dir );
		func_t->write ( gpio_t, dir, 0xA0, ( 0x10 + i*4 ), BCM_ACL_CFP_TCAM_DATA[i], 0 );
	}

	nk_switch_acl_print ( "## TCAM MASK\n");
	for ( i=0; i < 8; i++ ) {
		nk_switch_acl_print ( "s 0xA0 0x%x 0x%08x\n", ( 0x30 + i*4 ), BCM_ACL_CFP_TCAM_MASK[i] );
		func_t->cs ( gpio_t, dir );
		func_t->write ( gpio_t, dir, 0xA0, ( 0x30 + i*4 ), BCM_ACL_CFP_TCAM_MASK[i], 0 );
	}

}

void BCM_ACL_Start_TCAM ( uint32_t model, Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, Sw_Template_Sw_Port_Map_t *port_map_t, uint32_t dir, unsigned int entry ) {

	unsigned int value;

	value = ( ( entry & 0xFF ) << OFFSET_ACCESS_ADDR ) | TCAM | WRITE_OP | OP_START;

	nk_switch_acl_print ( "## Start TCAM and Set Entry(%d)\n", entry);
	nk_switch_acl_print ( "s 0xA0 0x00 0x%08x\n", value);

	func_t->cs ( gpio_t, dir );
	func_t->write ( gpio_t, dir, 0xA0, 0x00, value, 0 );
}

void BCM_ACL_Set_Meter ( uint32_t model, Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, Sw_Template_Sw_Port_Map_t *port_map_t, uint32_t dir, unsigned int rate, unsigned int burst ) {

	BCM_ACL_Rate_Reg[1] = RATE_EN | rate | ( burst << OFFSET_REF_CAP );
}


void BCM_ACL_Start_Meter ( uint32_t model, Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, Sw_Template_Sw_Port_Map_t *port_map_t, uint32_t dir, unsigned int entry ) {
	unsigned int value;

	value = ( ( entry & 0xFF ) << OFFSET_ACCESS_ADDR ) | Meter_RAM | WRITE_OP | OP_START;

	nk_switch_acl_print ( "## Start Rate Meter and Set Entry(%d)\n", entry);
	nk_switch_acl_print ( "s 0xA0 0x00 0x%08x\n", value);

	func_t->cs ( gpio_t, dir );
	func_t->write ( gpio_t, dir, 0xA0, 0x00, value, 0 );
}

void BCM_ACL_Print_Meter ( uint32_t model, Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, Sw_Template_Sw_Port_Map_t *port_map_t, uint32_t dir ) {

	nk_switch_acl_print ( "##Rate Meter Data\n" );
	nk_switch_acl_print ( "s 0xA0 0x60 0x%08x\n", BCM_ACL_Rate_Reg[0] );
	nk_switch_acl_print ( "s 0xA0 0x64 0x%08x\n", BCM_ACL_Rate_Reg[1] );

	func_t->cs ( gpio_t, dir );
	func_t->write ( gpio_t, dir, 0xA0, 0x60, BCM_ACL_Rate_Reg[0], 0 );
	func_t->write ( gpio_t, dir, 0xA0, 0x64, BCM_ACL_Rate_Reg[1], 0 );
}

void BCM_ACL_Set_IB_Counter ( uint32_t model, Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, Sw_Template_Sw_Port_Map_t *port_map_t, uint32_t dir, unsigned int val ) {

	nk_switch_acl_print ( "##IB Counter\n" );
	nk_switch_acl_print ( "s 0xA0 0x70 0x%08x\n", val );

	func_t->cs ( gpio_t, dir );
	func_t->write ( gpio_t, dir, 0xA0, 0x70, val, 0 );
}

void BCM_ACL_Set_OB_Counter ( uint32_t model, Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, Sw_Template_Sw_Port_Map_t *port_map_t, uint32_t dir, unsigned int val ) {

	nk_switch_acl_print ( "##OB Counter\n" );
	nk_switch_acl_print ( "s 0xA0 0x74 0x%08x\n", val );

	func_t->cs ( gpio_t, dir );
	func_t->write ( gpio_t, dir, 0xA0, 0x74, val, 0 );
}

void BCM_ACL_Start_IB_Counter ( uint32_t model, Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, Sw_Template_Sw_Port_Map_t *port_map_t, uint32_t dir, unsigned int entry ) {
	unsigned int value;

	value = ( ( entry & 0xFF ) << OFFSET_ACCESS_ADDR ) | IB_RAM | WRITE_OP | OP_START;

	nk_switch_acl_print ( "##Start IB Counter\n" );
	nk_switch_acl_print ( "s 0xA0 0x00 0x%08x\n", value );

	func_t->cs ( gpio_t, dir );
	func_t->write ( gpio_t, dir, 0xA0, 0x00, value, 0 );
}

void BCM_ACL_Start_OB_Counter ( uint32_t model, Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, Sw_Template_Sw_Port_Map_t *port_map_t, uint32_t dir, unsigned int entry ) {
	unsigned int value;

	value = ( ( entry & 0xFF ) << OFFSET_ACCESS_ADDR ) | OB_RAM | WRITE_OP | OP_START;

	nk_switch_acl_print ( "##Start OB Counter\n" );
	nk_switch_acl_print ( "s 0xA0 0x00 0x%08x\n", value );

	func_t->cs ( gpio_t, dir );
	func_t->write ( gpio_t, dir, 0xA0, 0x00, value, 0 );
}

int BCM_ACL_Conf ( uint32_t model, Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, Sw_Template_Sw_Port_Map_t *port_map_t, uint32_t dir, BCM_ACL_Control_t cmd ) {

	uint32_t l3_type=L3_NULL, slice=SLICE_NULL;

	memset ( BCM_ACL_CFP_n_X, 0, sizeof ( unsigned int )*9 );
	memset ( BCM_ACL_CFP_n_X_Mask, 0, sizeof ( unsigned int )*9 );
	memset ( BCM_ACL_CFP_TCAM_DATA, 0, sizeof ( unsigned int )*8 );
	memset ( BCM_ACL_CFP_TCAM_MASK, 0, sizeof ( unsigned int )*8 );

	/* MAC */
	if ( cmd.mac_type == 1 ) {/*SRC MAC*/
		slice = SLICE0;
		BCM_ACL_CFP_Set_UDF ( cmd.src_mac, cmd.src_mac_mask, 0, 6 );
	}
	else if ( cmd.mac_type == 2 ) {/*DST MAC*/
		slice = SLICE1;
		BCM_ACL_CFP_Set_UDF ( cmd.dst_mac, cmd.dst_mac_mask, 0, 6 );
	}
	else {
		/* Do Nothing */
	}

	/* IPv4 */
	if ( cmd.type == L3_IPV4 ) {
		l3_type = L3_IPV4;

		/* IP */
		if ( cmd.ip_type == 1 ) {/*SRC IP*/
			if ( cmd.mac_type == 2 )
				goto INVAILD_TYPE;

			if ( slice == SLICE_NULL )
				slice = SLICE0;
			BCM_ACL_CFP_Set_UDF ( cmd.src_ip, cmd.src_ip_mask, 3, 4 );
		}
		else if ( cmd.ip_type == 2 ) {/*DST IP*/
			if ( cmd.mac_type == 1 )
				goto INVAILD_TYPE;

			if ( slice == SLICE_NULL )
				slice = SLICE1;
			BCM_ACL_CFP_Set_UDF ( cmd.dst_ip, cmd.dst_ip_mask, 3, 4 );
		}
		else {
			/* Do Nothing */
		}

		/* PROTOCOL */
		Set_IP_Protocol ( cmd.ip_proto );

		/* PORT */
		if ( cmd.ip_proto == IP_PROTOCOL_TCP || cmd.ip_proto == IP_PROTOCOL_UDP ) {
			if ( cmd.srcport_type == 1 ) {/*SRC PORT*/
				if ( slice == SLICE_NULL )
					slice = SLICE0;
				BCM_ACL_CFP_Set_UDF ( cmd.src_port, 0xFFFF, 5, 2 );
			}
			if ( cmd.dstport_type == 1 ) {/*DST PORT*/
				if ( slice == SLICE_NULL )
					slice = SLICE0;
				BCM_ACL_CFP_Set_UDF ( cmd.dst_port, 0xFFFF, 6, 2 );
			}
		}
		else {
			/* Do Nothing */
		}
	}
	/* None IP */
	else if ( cmd.type == L3_NONIP ) {
		l3_type = L3_NONIP;

		Set_Ether_Type ( cmd.eth_type );
	}
	else
		goto INVAILD_TYPE;

	BCM_ACL_Set_TCAM_Conf ( ALL_PORT_MAP, l3_type, slice );

	BCM_ACL_Print_TCAM ( model, func_t, gpio_t, port_map_t, dir );
	BCM_ACL_Start_TCAM ( model, func_t, gpio_t, port_map_t, dir, BCM_ACL_Entry );

	/* Action */
	if ( cmd.act_type == 1 ) {/* Rate Control */
		BCM_ACL_Set_Meter ( model, func_t, gpio_t, port_map_t, dir, cmd.rate, BURST_32KB );
		BCM_ACL_Print_Meter ( model, func_t, gpio_t, port_map_t, dir );
		BCM_ACL_Start_Meter ( model, func_t, gpio_t, port_map_t, dir, BCM_ACL_Entry );

		BCM_ACL_Set_Action ( model, func_t, gpio_t, port_map_t, dir, DROP_OB, 0x0 );
		BCM_ACL_Start_ACT ( model, func_t, gpio_t, port_map_t, dir, BCM_ACL_Entry );
	}
	else if ( cmd.act_type == 2 ) {/* Block */
		BCM_ACL_Set_Action ( model, func_t, gpio_t, port_map_t, dir, RPDROP, 0x0 );
		BCM_ACL_Start_ACT ( model, func_t, gpio_t, port_map_t, dir, BCM_ACL_Entry );
	}
	else if ( cmd.act_type == 3 ) {/* Change Priority */
		BCM_ACL_Set_Meter ( model, func_t, gpio_t, port_map_t, dir, RATE_1000Mb, BURST_32KB );
		BCM_ACL_Print_Meter ( model, func_t, gpio_t, port_map_t, dir );
		BCM_ACL_Start_Meter ( model, func_t, gpio_t, port_map_t, dir, BCM_ACL_Entry );

		BCM_ACL_Set_IB_Counter ( model, func_t, gpio_t, port_map_t, dir, 0x0 );
		BCM_ACL_Set_OB_Counter ( model, func_t, gpio_t, port_map_t, dir, 0x0 );
	
		BCM_ACL_Start_IB_Counter ( model, func_t, gpio_t, port_map_t, dir, BCM_ACL_Entry );
		BCM_ACL_Start_OB_Counter ( model, func_t, gpio_t, port_map_t, dir, BCM_ACL_Entry );

		BCM_ACL_Set_Action ( model, func_t, gpio_t, port_map_t, dir, CHANGE_TC, cmd.prio );
		BCM_ACL_Start_ACT ( model, func_t, gpio_t, port_map_t, dir, BCM_ACL_Entry );
	}
	else if ( cmd.act_type == 4 ) {/* Statistic */
		BCM_ACL_Set_Meter ( model, func_t, gpio_t, port_map_t, dir, RATE_1000Mb, BURST_32KB );
		BCM_ACL_Print_Meter ( model, func_t, gpio_t, port_map_t, dir );
		BCM_ACL_Start_Meter ( model, func_t, gpio_t, port_map_t, dir, BCM_ACL_Entry );

		BCM_ACL_Set_IB_Counter ( model, func_t, gpio_t, port_map_t, dir, 0x0 );
		BCM_ACL_Set_OB_Counter ( model, func_t, gpio_t, port_map_t, dir, 0x0 );
	
		BCM_ACL_Start_IB_Counter ( model, func_t, gpio_t, port_map_t, dir, BCM_ACL_Entry );
		BCM_ACL_Start_OB_Counter ( model, func_t, gpio_t, port_map_t, dir, BCM_ACL_Entry );
	}


	return 0;

INVAILD_TYPE:
	printk ( KERN_EMERG "%s: Recieve invaild type\n", __func__ );
	return -1;
}


/**
	Init CFP TCAM
	1. Reset CFP
	2. Enable CFP Per Port
	3. Init UDF Configruation
**/
void BCM_ACL_Init ( uint32_t model, Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, Sw_Template_Sw_Port_Map_t *port_map_t, uint32_t dir, uint32_t pbm ) {

	if ( BCM_ACL_Is_Init_CFP ) {
		BCM_ACL_Entry++;
		return;
	}
	else {
		BCM_ACL_Is_Init_CFP = 1;
		BCM_ACL_Entry = 1;
	}

	BCM_ACL_Enable_Port ( model, func_t, gpio_t, port_map_t, dir, pbm );

	BCM_ACL_Init_UDF_Conf ( model, func_t, gpio_t, port_map_t, dir );
}

void BCM_ACL_Reset ( uint32_t model, Sw_Template_Sw_Func_t *func_t, Sw_Template_Sw_GPIO_t *gpio_t, Sw_Template_Sw_Port_Map_t *port_map_t, uint32_t dir ) {

	BCM_ACL_Is_Init_CFP = 0;
	BCM_ACL_Entry = 0;

	BCM_ACL_Reset_CFP ( model, func_t, gpio_t, port_map_t, dir );
}
#endif
