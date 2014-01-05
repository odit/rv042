#ifndef __SW_TEMPLATE_GPIO_H__
#define __SW_TEMPLATE_GPIO_H__

#define SW_TEMPLATE_GPIO_0		00
#define SW_TEMPLATE_GPIO_1		01
#define SW_TEMPLATE_GPIO_2		02
#define SW_TEMPLATE_GPIO_3		03
#define SW_TEMPLATE_GPIO_4		04
#define SW_TEMPLATE_GPIO_5		05
#define SW_TEMPLATE_GPIO_6		06
#define SW_TEMPLATE_GPIO_7		07
#define SW_TEMPLATE_GPIO_8		08
#define SW_TEMPLATE_GPIO_9		09
#define SW_TEMPLATE_GPIO_10		10
#define SW_TEMPLATE_GPIO_11		11
#define SW_TEMPLATE_GPIO_12		12
#define SW_TEMPLATE_GPIO_13		13
#define SW_TEMPLATE_GPIO_14		14
#define SW_TEMPLATE_GPIO_15		15
#define SW_TEMPLATE_GPIO_16		16
#define SW_TEMPLATE_GPIO_17		17
#define SW_TEMPLATE_GPIO_18		18
#define SW_TEMPLATE_GPIO_19		19
#define SW_TEMPLATE_GPIO_20		20
#define SW_TEMPLATE_GPIO_21		21
#define SW_TEMPLATE_GPIO_22		22
#define SW_TEMPLATE_GPIO_23		23
#define SW_TEMPLATE_GPIO_ALL	99
#define SW_TEMPLATE_GPIO_NO		24
#define SW_TEMPLATE_GPIO_NO_THIS_PIN	30
#define SW_TEMPLATE_GPIO_RANG_MASK		0xFFFFFF

#define SW_TEMPLATE_GPIO_IN		0x00
#define SW_TEMPLATE_GPIO_OUT	0x01

#define SW_TEMPLATE_GPIO_LOW	0x00
#define SW_TEMPLATE_GPIO_HIGH	0x01


#define MS (500000ull) /* Not exactly a millsecond, but close enough */

#define SW_TEMPLATE_GET_MASK(x)	( 1 << x )


int			Sw_Template_Verify_GPIO		( uint32_t gpio );
void		Sw_Template_Set_GPIO_State	( uint32_t state, uint32_t gpio );
uint32_t	Sw_Template_Get_GPIO_State	( uint32_t gpio );
void		Sw_Template_Set_GPIO_Value	( uint32_t value, uint32_t mask );
uint32_t	Sw_Template_Get_GPIO_Value	( uint32_t gpio );
void		Sw_Template_Show_GPIO_Status ( uint32_t gpio );

#endif
