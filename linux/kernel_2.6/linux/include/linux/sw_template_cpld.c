#ifndef __SW_TEMPLATE_CPLD_C__
#define __SW_TEMPLATE_CPLD_C__

#include <linux/sw_template_cpld.h>
#include <linux/sw_template_gpio.c>
#include <linux/sw_template_util.h>
#include <linux/dynamic_port_num.h>


int Sw_Template_CPLD_Input_Data[SW_TEMPLATE_CPLD_INPUT_LEN];
Sw_Template_CPLD_t Sw_Template_CPLD_state;
Sw_Template_CPLD_GPIO_t *cpld_gpio;
int Sw_Template_CPLD_MSBq[SW_TEMPLATE_CPLD_MSBq_LEN];
int Sw_Template_CPLD_MSBp[SW_TEMPLATE_CPLD_MSBp_LEN];

void Sw_Template_Init_CPLD_GPIO ( void );
void Sw_Template_Set_CPLD_State ( uint32_t state );
void Sw_Template_Set_CPLD_CLK_Value ( uint32_t value );
void Sw_Template_Set_CPLD_SDA_Value ( uint32_t value );

void Sw_Template_CPLD_Send_0 ( void );
void Sw_Template_CPLD_Send_1 ( void );
int Sw_Template_CPLD_Get_Data ( void );

int Sw_Template_CPLD_Save_MSBq0 ( void );
int Sw_Template_CPLD_Save_MSBp0 ( void );
void Sw_Template_CPLD_Shift_MSBq ( void );
void Sw_Template_CPLD_Shift_MSBp ( void );
int Sw_Template_CPLD_XOR_MSBq ( void );
int Sw_Template_CPLD_XOR_MSBp ( void );

void Sw_Template_CPLD_Control ( unsigned char *len );
void Sw_Template_Get_CPLD_Value (void );


void Sw_Template_Init_CPLD_GPIO ( void ) {

	uint32_t model = 650;

	for ( cpld_gpio = Sw_Template_CPLD_GPIO; cpld_gpio->model != 0; cpld_gpio++ ) {
		if ( cpld_gpio->model == model )
			break;
	}

	Sw_Template_Set_GPIO_State ( SW_TEMPLATE_GPIO_OUT, cpld_gpio->cs );
	Sw_Template_Set_GPIO_State ( SW_TEMPLATE_GPIO_OUT, cpld_gpio->clk );
	Sw_Template_Set_GPIO_State ( SW_TEMPLATE_GPIO_OUT, cpld_gpio->sda );
	Sw_Template_Set_GPIO_State ( SW_TEMPLATE_GPIO_IN, cpld_gpio->oda );
}

void Sw_Template_Set_CPLD_State ( uint32_t state ) {

	if ( state == SW_TEMPLATE_CPLD_ENABLE )
		Sw_Template_Set_GPIO_Value ( cpld_gpio->cs_enable, ( 1 << cpld_gpio->cs ) );
	else
		Sw_Template_Set_GPIO_Value ( ( ( cpld_gpio->cs_enable == SW_TEMPLATE_GPIO_LOW ) ? SW_TEMPLATE_GPIO_HIGH : SW_TEMPLATE_GPIO_LOW ), ( 1 << cpld_gpio->cs ) );
}

void Sw_Template_Set_CPLD_CLK_Value ( uint32_t value ) {

	Sw_Template_Set_GPIO_Value ( value, ( 1 << cpld_gpio->clk ) );
}

void Sw_Template_Set_CPLD_SDA_Value ( uint32_t value ) {

	Sw_Template_Set_GPIO_Value ( value, ( 1 << cpld_gpio->sda ) );
}

void Sw_Template_CPLD_Send_0 ( void ) {

	Sw_Template_Set_GPIO_Value ( SW_TEMPLATE_GPIO_LOW, ( 1 << cpld_gpio->sda ) );
	Sw_Template_Set_GPIO_Value ( SW_TEMPLATE_GPIO_LOW, ( 1 << cpld_gpio->clk ) );
	Sw_Template_Delay ( SW_TEMPLATE_CPLD_DELAY );
	Sw_Template_Set_GPIO_Value ( SW_TEMPLATE_GPIO_HIGH, ( 1 << cpld_gpio->clk ) );
	Sw_Template_Delay ( SW_TEMPLATE_CPLD_DELAY );
	Sw_Template_Set_GPIO_Value ( SW_TEMPLATE_GPIO_LOW, ( 1 << cpld_gpio->clk ) );
}

void Sw_Template_CPLD_Send_1 ( void ){

	Sw_Template_Set_GPIO_Value ( SW_TEMPLATE_GPIO_HIGH, ( 1 << cpld_gpio->sda ) );
	Sw_Template_Set_GPIO_Value ( SW_TEMPLATE_GPIO_LOW, ( 1 << cpld_gpio->clk ) );
	Sw_Template_Delay ( SW_TEMPLATE_CPLD_DELAY );
	Sw_Template_Set_GPIO_Value ( SW_TEMPLATE_GPIO_HIGH, ( 1 << cpld_gpio->clk ) );
	Sw_Template_Delay ( SW_TEMPLATE_CPLD_DELAY );
	Sw_Template_Set_GPIO_Value ( SW_TEMPLATE_GPIO_LOW, ( 1 << cpld_gpio->clk ) );
	
}

int Sw_Template_CPLD_Get_Data ( void ){

	uint64_t result;

	result = Sw_Template_Get_GPIO_Value ( cpld_gpio->oda );
	if ( result )
		return 1;
	else
		return 0;
}

int Sw_Template_CPLD_Save_MSBq0 ( void ) {

	int MSBq;

	MSBq = Sw_Template_CPLD_MSBq[15] ^ Sw_Template_CPLD_MSBq[14] ^ Sw_Template_CPLD_MSBq[4];
	return MSBq;
}

int Sw_Template_CPLD_Save_MSBp0 ( void ) {

	int MSBp;

	MSBp = Sw_Template_CPLD_MSBp[15] ^ Sw_Template_CPLD_MSBp[7] ^ Sw_Template_CPLD_MSBp[2];
	return MSBp;
}

void Sw_Template_CPLD_Shift_MSBq ( void ) {

	int i, MSBq;

	MSBq = Sw_Template_CPLD_Save_MSBq0();
	for ( i=SW_TEMPLATE_CPLD_MSBq_LEN-1; i>=1; i-- )
		Sw_Template_CPLD_MSBq[i] = Sw_Template_CPLD_MSBq[i-1];
	Sw_Template_CPLD_MSBq[0] = MSBq;
}

void Sw_Template_CPLD_Shift_MSBp ( void ) {

	int i, MSBp;

	MSBp = Sw_Template_CPLD_Save_MSBp0();
	for ( i=SW_TEMPLATE_CPLD_MSBp_LEN-1; i>=1; i-- )
		Sw_Template_CPLD_MSBp[i] = Sw_Template_CPLD_MSBp[i-1];
	Sw_Template_CPLD_MSBp[0] = MSBp;
}

int Sw_Template_CPLD_XOR_MSBq ( void ) {

	return Sw_Template_CPLD_MSBq[15] ^ Sw_Template_CPLD_MSBq[14] ^ Sw_Template_CPLD_MSBq[10];
}

int Sw_Template_CPLD_XOR_MSBp ( void ) {

	return Sw_Template_CPLD_MSBp[10] ^ Sw_Template_CPLD_MSBp[4];
}

void Sw_Template_CPLD_Control ( unsigned char *len ) {

	int i;
	long int number[200];
	unsigned char temp[200];

	for ( i=0; i < strlen(len); i++) {
		temp[0] = *(len+i);
		temp[1] = '\0';

		number[i] = Sw_Template_Trans_Str_2_Hex(*(len+i));
	}

	/* Set CPLD GPIO pins to correct state, output or input */
	Sw_Template_Init_CPLD_GPIO ();
	Sw_Template_Delay ( SW_TEMPLATE_CPLD_DELAY );
	/* Set CPLD CS to enable value */
	Sw_Template_Set_CPLD_State ( SW_TEMPLATE_CPLD_ENABLE );
	Sw_Template_Delay ( SW_TEMPLATE_CPLD_DELAY );

	Sw_Template_Set_CPLD_CLK_Value ( SW_TEMPLATE_GPIO_LOW );
	Sw_Template_Delay ( SW_TEMPLATE_CPLD_DELAY );
	Sw_Template_Set_CPLD_SDA_Value ( SW_TEMPLATE_GPIO_LOW );
	Sw_Template_Delay ( SW_TEMPLATE_CPLD_DELAY );
	Sw_Template_Set_CPLD_State ( SW_TEMPLATE_CPLD_DISABLE );
	Sw_Template_Delay ( SW_TEMPLATE_CPLD_DELAY );

	for ( i=0; i < strlen ( len ); i++ ) {
		if ( number[i] & 0x1 )
			Sw_Template_CPLD_Send_1();
		else
			Sw_Template_CPLD_Send_0();

		Sw_Template_CPLD_state.return_number[i] = Sw_Template_CPLD_Get_Data();
	}

	Sw_Template_Delay ( SW_TEMPLATE_CPLD_DELAY );
}

void Sw_Template_Get_CPLD_Value ( void ) {

	int i, number, flag;
	Sw_Template_CPLD_Model_Info_t *model_info;

	/* Init CPLD Struct */
	for ( i=0; i < SW_TEMPLATE_CPLD_INPUT_LEN; i++ ) {
		number = 0;
		Sw_Template_CPLD_Input_Data[i] = number;
		Sw_Template_CPLD_state.len[i] = number + '0';
		Sw_Template_CPLD_state.return_number[i] = 1;
	}
	Sw_Template_CPLD_state.len[SW_TEMPLATE_CPLD_INPUT_LEN] = '\0';
	Sw_Template_CPLD_state.length = SW_TEMPLATE_CPLD_INPUT_LEN;

	Sw_Template_CPLD_Control ( Sw_Template_CPLD_state.len );

	/* For debug CPLD value */
	for ( i=0; i < SW_TEMPLATE_CPLD_INPUT_LEN; i++ )
	printk ( KERN_EMERG "Kernel CPLD[%d]=%d\n", i, Sw_Template_CPLD_state.return_number[i] );


	/* Check Model */
	for ( model_info = Sw_Template_CPLD_Model_Info; model_info->model != 0; model_info++ ) {
		flag = 0;
		memcpy ( Sw_Template_CPLD_MSBq, model_info->MSBq, sizeof ( int ) * SW_TEMPLATE_CPLD_MSBq_LEN );
		memcpy ( Sw_Template_CPLD_MSBp, model_info->MSBp, sizeof ( int ) * SW_TEMPLATE_CPLD_MSBp_LEN );

		for ( i=0; i < SW_TEMPLATE_CPLD_INPUT_LEN; i++ ) {
			Sw_Template_CPLD_Shift_MSBq();
			Sw_Template_CPLD_Shift_MSBp();
			if ( ( Sw_Template_CPLD_XOR_MSBq() ^ Sw_Template_CPLD_XOR_MSBp() ^ Sw_Template_CPLD_Input_Data[i] ) != Sw_Template_CPLD_state.return_number[i] ) {
				/* FAIL */
				flag = 1;
				break;;
			}
		}

		if ( !flag ) {
			/* SUCCESS */
			useSwitch = model_info->model;
			DYNAMIC_NUM_LAN = model_info->num_lan;
			DYNAMIC_NUM_WAN = model_info->num_wan;
			DYNAMIC_NUM_DMZ = model_info->num_dmz;
			DYNAMIC_MAX_SESSION = model_info->num_max_session;
			DYNAMIC_HASH_SIZE = model_info->num_hash;
			printk ( KERN_EMERG "Kernel dectect model is %d\n", useSwitch );
			return;
		}
	}

	/* If goto here, it means that CPLD Value is not in Sw_Template_CPLD_Model_Info List, so we use default setting */
	useSwitch = SWITCHUNKNOW;
	DYNAMIC_NUM_LAN = 8;
	DYNAMIC_NUM_WAN = 5;
	DYNAMIC_NUM_DMZ = 0;
	DYNAMIC_MAX_SESSION = 200000;
	DYNAMIC_HASH_SIZE = 262140;
	printk ( KERN_EMERG "Kernel dectect unkown model, assume model is GQF1100\n" );

}


void Sw_Template_Init_CPLD_GPIO_Multi_Model (  Sw_Template_CPLD_GPIO_Model *tmp_cpld_gpio_model ) {

	cpld_gpio = tmp_cpld_gpio_model->gpio;

	Sw_Template_Set_GPIO_State ( SW_TEMPLATE_GPIO_OUT, cpld_gpio->cs );
	Sw_Template_Set_GPIO_State ( SW_TEMPLATE_GPIO_OUT, cpld_gpio->clk );
	Sw_Template_Set_GPIO_State ( SW_TEMPLATE_GPIO_OUT, cpld_gpio->sda );
	Sw_Template_Set_GPIO_State ( SW_TEMPLATE_GPIO_IN, cpld_gpio->oda );
}

void Sw_Template_CPLD_Control_Multi_Model ( Sw_Template_CPLD_GPIO_Model *tmp_cpld_gpio_model, unsigned char *len ) {

	int i;
	long int number[200];
	unsigned char temp[200];

	for ( i=0; i < strlen(len); i++) {
		temp[0] = *(len+i);
		temp[1] = '\0';

		number[i] = Sw_Template_Trans_Str_2_Hex(*(len+i));
	}

	/* Set CPLD GPIO pins to correct state, output or input */
	Sw_Template_Init_CPLD_GPIO_Multi_Model ( tmp_cpld_gpio_model );
	Sw_Template_Delay ( SW_TEMPLATE_CPLD_DELAY );
	/* Set CPLD CS to enable value */
	Sw_Template_Set_CPLD_State ( SW_TEMPLATE_CPLD_ENABLE );
	Sw_Template_Delay ( SW_TEMPLATE_CPLD_DELAY );

	Sw_Template_Set_CPLD_CLK_Value ( SW_TEMPLATE_GPIO_LOW );
	Sw_Template_Delay ( SW_TEMPLATE_CPLD_DELAY );
	Sw_Template_Set_CPLD_SDA_Value ( SW_TEMPLATE_GPIO_LOW );
	Sw_Template_Delay ( SW_TEMPLATE_CPLD_DELAY );
	Sw_Template_Set_CPLD_State ( SW_TEMPLATE_CPLD_DISABLE );
	Sw_Template_Delay ( SW_TEMPLATE_CPLD_DELAY );

	for ( i=0; i < strlen ( len ); i++ ) {
		if ( number[i] & 0x1 )
			Sw_Template_CPLD_Send_1();
		else
			Sw_Template_CPLD_Send_0();

		Sw_Template_CPLD_state.return_number[i] = Sw_Template_CPLD_Get_Data();
	}

	Sw_Template_Delay ( SW_TEMPLATE_CPLD_DELAY );
}

/**
	New Vision Get CPLD Value
	1. Select CPLD pins Group
	2. Init and Get CPLD value
	3. Check Model Accroding witch CPLD pins Group
**/
void Sw_Template_Get_CPLD_Value_Multi_Model ( void ) {

	int i, number, flag;
	Sw_Template_CPLD_Model_Info_t *model_info;
	Sw_Template_CPLD_GPIO_Model *tmp_cpld_gpio_model;

	for ( tmp_cpld_gpio_model=cpld_gpio_model; tmp_cpld_gpio_model->gpio; tmp_cpld_gpio_model++ ) {

		/* Init CPLD Struct, After it then you get CPLD value */
		for ( i=0; i < SW_TEMPLATE_CPLD_INPUT_LEN; i++ ) {
			number = 0;
			Sw_Template_CPLD_Input_Data[i] = number;
			Sw_Template_CPLD_state.len[i] = number + '0';
			Sw_Template_CPLD_state.return_number[i] = 1;
		}
		Sw_Template_CPLD_state.len[SW_TEMPLATE_CPLD_INPUT_LEN] = '\0';
		Sw_Template_CPLD_state.length = SW_TEMPLATE_CPLD_INPUT_LEN;

		Sw_Template_CPLD_Control_Multi_Model ( tmp_cpld_gpio_model, Sw_Template_CPLD_state.len );

		printk ( KERN_EMERG "Kernel CPLD Pings Group Type[%d]\n", tmp_cpld_gpio_model->gpio->model );
		/* For debug CPLD value */
// 		for ( i=0; i < SW_TEMPLATE_CPLD_INPUT_LEN; i++ )
// 			printk ( KERN_EMERG "Kernel CPLD Pins Group Type[%d], CPLD[%d]=%d\n", tmp_cpld_gpio_model->gpio->model, i, Sw_Template_CPLD_state.return_number[i] );

		/* Check Model */
		for ( model_info = tmp_cpld_gpio_model->model_info; model_info->model != 0; model_info++ ) {
			flag = 0;
			memcpy ( Sw_Template_CPLD_MSBq, model_info->MSBq, sizeof ( int ) * SW_TEMPLATE_CPLD_MSBq_LEN );
			memcpy ( Sw_Template_CPLD_MSBp, model_info->MSBp, sizeof ( int ) * SW_TEMPLATE_CPLD_MSBp_LEN );
	
			for ( i=0; i < SW_TEMPLATE_CPLD_INPUT_LEN; i++ ) {
				Sw_Template_CPLD_Shift_MSBq();
				Sw_Template_CPLD_Shift_MSBp();
				if ( ( Sw_Template_CPLD_XOR_MSBq() ^ Sw_Template_CPLD_XOR_MSBp() ^ Sw_Template_CPLD_Input_Data[i] ) != Sw_Template_CPLD_state.return_number[i] ) {
					/* FAIL */
					flag = 1;
					break;;
				}
			}
	
			if ( !flag ) {
				/* SUCCESS */
				useSwitch = model_info->model;
				DYNAMIC_NUM_LAN = model_info->num_lan;
				DYNAMIC_NUM_WAN = model_info->num_wan;
				DYNAMIC_NUM_DMZ = model_info->num_dmz;
				DYNAMIC_MAX_SESSION = model_info->num_max_session;
				DYNAMIC_HASH_SIZE = model_info->num_hash;
				printk ( KERN_EMERG "Kernel dectect model is %d\n", useSwitch );
				return;
			}
		}
		printk ( KERN_EMERG "Kernel CPLD Not Found Match Model\n" );
	}

	/* If goto here, it means that CPLD Value is not in Sw_Template_CPLD_Model_Info List, so we use default setting */
	useSwitch = SWITCHUNKNOW;
	DYNAMIC_NUM_LAN = 8;
	DYNAMIC_NUM_WAN = 5;
	DYNAMIC_NUM_DMZ = 0;
	DYNAMIC_MAX_SESSION = 200000;
	DYNAMIC_HASH_SIZE = 262140;
	printk ( KERN_EMERG "Kernel dectect unkown model, assume model is GQF1100\n" );
}

#endif
