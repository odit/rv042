#ifndef __SW_TEMPLATE_GPIO_C__
#define __SW_TEMPLATE_GPIO_C__

#define OCTEON_MODEL OCTEON_CN31XX
#include "cvmx.h"
#include "cvmx-csr.h"

#include <linux/sw_template_gpio.h>


inline void Sw_Template_Delay ( uint64_t ms ) {

	uint64_t timeout = cvmx_get_cycle() + MS * ms;

	while ( cvmx_get_cycle () < timeout );
}

int Sw_Template_Verify_GPIO ( uint32_t gpio ) {

	if ( gpio >= SW_TEMPLATE_GPIO_NO )
		return 0;
	return 1;
}

void Sw_Template_Set_GPIO_State ( unsigned int state, unsigned int gpio ) {

	cvmx_gpio_bit_cfgx_t config;

	if ( !Sw_Template_Verify_GPIO ( gpio ) )
		return;

	if ( gpio < SW_TEMPLATE_GPIO_16 ) {
		config.u64 = cvmx_read_csr ( CVMX_GPIO_BIT_CFGX ( gpio ) );
		config.s.tx_oe = state;
		cvmx_write_csr ( CVMX_GPIO_BIT_CFGX ( gpio ), config.u64 );
	}
	else {
		config.u64 = cvmx_read_csr ( CVMX_GPIO_XBIT_CFGX ( gpio ) );
		config.s.tx_oe = state;
		cvmx_write_csr ( CVMX_GPIO_XBIT_CFGX ( gpio ), config.u64 );
	}
}

unsigned int Sw_Template_Get_GPIO_State ( unsigned int gpio ) {

	cvmx_gpio_bit_cfgx_t config;

	if ( !Sw_Template_Verify_GPIO ( gpio ) )
		return 0;

	if ( gpio < SW_TEMPLATE_GPIO_16 )
		config.u64 = cvmx_read_csr ( CVMX_GPIO_BIT_CFGX ( gpio ) );
	else
		config.u64 = cvmx_read_csr ( CVMX_GPIO_XBIT_CFGX ( gpio ) );

	return config.s.tx_oe;
}

void Sw_Template_Set_GPIO_Value ( unsigned int value, uint32_t mask ) {

	mask = mask & SW_TEMPLATE_GPIO_RANG_MASK;

	if( value == SW_TEMPLATE_GPIO_HIGH )
		cvmx_write_csr ( CVMX_GPIO_TX_SET, mask );
	else if ( value == SW_TEMPLATE_GPIO_LOW )
		cvmx_write_csr ( CVMX_GPIO_TX_CLR, mask );
}

unsigned int Sw_Template_Get_GPIO_Value ( unsigned int gpio ) {

	uint64_t value;

	if ( !Sw_Template_Verify_GPIO ( gpio ) && gpio != SW_TEMPLATE_GPIO_ALL )
		return 0;

	value = cvmx_read_csr ( CVMX_GPIO_RX_DAT );

	if ( gpio == SW_TEMPLATE_GPIO_ALL )
		return value;

	return value & ( 1 << gpio );
}

void Sw_Template_Show_GPIO_Status ( uint32_t gpio ) {

	int i;

	if ( gpio == SW_TEMPLATE_GPIO_ALL ) {
		for ( i=0; i < SW_TEMPLATE_GPIO_NO; i++ ) {
			printk ( KERN_EMERG "GPIO_%d %s %s\n",
				i,
				Sw_Template_Get_GPIO_State ( i ) == SW_TEMPLATE_GPIO_IN ? "INPUT" : "OUTPUT",
				Sw_Template_Get_GPIO_Value ( i ) == SW_TEMPLATE_GPIO_LOW ? "LOW" : "HIGH" );
		}
	}
	else
		printk ( KERN_EMERG "GPIO_%d %s %s\n",
				gpio,
				Sw_Template_Get_GPIO_State ( gpio ) == SW_TEMPLATE_GPIO_IN ? "INPUT" : "OUTPUT",
				Sw_Template_Get_GPIO_Value ( gpio ) == SW_TEMPLATE_GPIO_LOW ? "LOW" : "HIGH" );
}
#endif
