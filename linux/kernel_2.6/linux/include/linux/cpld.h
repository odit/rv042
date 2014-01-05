#ifndef __CPLD_H__
#define __CPLD_H__

#define OCTEON_MODEL OCTEON_CN31XX
#include "../../../../../executive/cvmx.h"
#include "../../../../../executive/cvmx-csr.h"

/** Constant Define **/
#define nk_CPLD_input_len			40
#define nk_MSBq_len					16
#define nk_MSBp_len					16

#define CN_GPIO_0		0
#define CN_GPIO_1		1
#define CN_GPIO_2		2
#define CN_GPIO_3		3
#define CN_GPIO_4		4
#define CN_GPIO_5		5
#define CN_GPIO_6		6
#define CN_GPIO_7		7
#define CN_GPIO_8		8
#define CN_GPIO_9		9
#define CN_GPIO_10		10
#define CN_GPIO_11		11
#define CN_GPIO_12		12
#define CN_GPIO_13		13
#define CN_GPIO_14		14
#define CN_GPIO_15		15
#define CN_GPIO_16		16
#define CN_GPIO_17		17
#define CN_GPIO_18		18
#define CN_GPIO_19		19
#define CN_GPIO_20		20
#define CN_GPIO_21		21
#define CN_GPIO_22		22
#define CN_GPIO_23		23
#define CN_GPIO_NO		24
#define CN_GPIO_OUT		1
#define CN_GPIO_IN		0
#define CN_GPIO_HIGH	1
#define CN_GPIO_LOW		0

#define CPLD_CS 1<<CN_GPIO_7
#define CPLD_SDA 1<<CN_GPIO_10
#define CPLD_ODA 1<<CN_GPIO_11
#define CPLD_SCLK 1<<CN_GPIO_9 
#define CPLD_CS_STATE CN_GPIO_7
#define CPLD_SDA_STATE CN_GPIO_10
#define CPLD_ODA_STATE CN_GPIO_11
#define CPLD_SCLK_STATE CN_GPIO_9

#define DISABLE_BOOTBUS_MASK	0xFFFFFFFFFFFFF0FF

struct NK_CPLD{
	unsigned char len[200];
	unsigned int return_number[200];
	int length;
} ;

/** Global Variable **/
int nk_input_data[nk_CPLD_input_len];
int nk_MSBq[nk_MSBq_len];
int nk_MSBp[nk_MSBp_len];
struct NK_CPLD nk_CPLD_state;

int nk_MSBq_1100[nk_MSBq_len]={//Low -> High
			1,0,1,1,
			1,0,0,0,
			1,0,1,0,
			1,0,0,1};
int nk_MSBp_1100[nk_MSBp_len]={//Low -> High
			0,1,1,0,
			0,0,1,1,
			1,0,1,0,
			1,1,0,0};

int nk_MSBq_1150[nk_MSBq_len]={//Low -> High
			1,0,1,0,
			0,1,1,0,
			0,1,0,0,
			1,0,0,1};
int nk_MSBp_1150[nk_MSBp_len]={//Low -> High
			0,0,0,0,
			1,0,1,0,
			1,0,0,0,
			1,0,0,0};

int nk_MSBq_1450[nk_MSBq_len]={//Low -> High
			0,0,1,0,
			1,0,0,1,
			1,1,0,0,
			0,0,0,0};

int nk_MSBp_1450[nk_MSBp_len]={//Low -> High
			0,1,0,0,
			0,1,0,1,
			1,0,0,0,
			0,0,0,0};

int nk_MSBq_650[nk_MSBq_len]={//Low -> High
			0,1,0,0,
			0,1,0,1,
			1,0,0,0,
			0,0,0,0};

int nk_MSBp_650[nk_MSBp_len]={//Low -> High
			0,0,1,0,
			1,0,0,1,
			1,1,0,0,
			0,0,0,0};


/** Function Implement **/
int save_nk_MSBq0(void) {//save Q0
	int nk_MSBq_0;

	nk_MSBq_0 = nk_MSBq[15] ^ nk_MSBq[14] ^ nk_MSBq[4];
	return nk_MSBq_0;
}

void shift_nk_MSBq(void) {
	int i, nk_MSBq_0;

	nk_MSBq_0 = save_nk_MSBq0();//save Q0
	for ( i = nk_MSBq_len - 1 ;i >= 1; i-- )
		nk_MSBq[i] = nk_MSBq[i-1];
	nk_MSBq[0] = nk_MSBq_0;
}

int save_nk_MSBp0(void) {//save Q0
	int nk_MSBp_0;

	nk_MSBp_0 = nk_MSBp[15] ^ nk_MSBp[7] ^ nk_MSBp[2];
	return nk_MSBp_0;
}

void shift_nk_MSBp(void) {
	int i,nk_MSBp_0;

	nk_MSBp_0 = save_nk_MSBp0();//save P0
	for( i = nk_MSBp_len - 1 ; i >= 1 ; i--)
		nk_MSBp[i] = nk_MSBp[i-1];
	nk_MSBp[0] = nk_MSBp_0;
}

int XOR_nk_MSBq(void) {
	return nk_MSBq[15] ^ nk_MSBq[14] ^ nk_MSBq[10];
}

int XOR_nk_MSBp(void) {
	return nk_MSBp[10] ^ nk_MSBp[4];
}

void CPLDdelay_270000(void)
{
	unsigned int i;

	for (i=0 ; i<270000 ; i++);
}

void CPLDdelay_160000(void)
{
	unsigned int i;

	for (i=0 ; i<200000 ; i++);
}

int nk_trans_str_to_hex(char str)
{
	if(str<='9'&&str>='0')
		return (str - '0');
	if(str<='f'&&str>='a')
		return (str - 'a')+10;
	if(str<='F'&&str>='A')
		return (str - 'A')+10;
		return 0;
}

void set_gpio_value(int value, u64 mask)
{
	if(value == CN_GPIO_HIGH)
		cvmx_write_csr(CVMX_GPIO_TX_SET, mask);
	else if(value == CN_GPIO_LOW)
		cvmx_write_csr(CVMX_GPIO_TX_CLR, mask);
}

void set_gpio_state(int state, int gpio)
{
	cvmx_gpio_bit_cfgx_t config;

	if(gpio < CN_GPIO_16)
	{
		config.u64 = cvmx_read_csr (CVMX_GPIO_BIT_CFGX(gpio));
		config.s.tx_oe = state;
		cvmx_write_csr (CVMX_GPIO_BIT_CFGX(gpio), config.u64);
	}
	else
	{
		config.u64 = cvmx_read_csr (CVMX_GPIO_XBIT_CFGX(gpio));
		config.s.tx_oe = state;
		cvmx_write_csr (CVMX_GPIO_XBIT_CFGX(gpio), config.u64);
	}
}

void Set_CPLDToState(void)
{
	set_gpio_value(CN_GPIO_OUT, CPLD_ODA);
	set_gpio_state(CN_GPIO_IN, CPLD_ODA_STATE);
	set_gpio_state(CN_GPIO_OUT, CPLD_SDA_STATE);
	set_gpio_state(CN_GPIO_OUT, CPLD_SCLK_STATE);
#ifdef CONFIG_NK_DYNAMIC_PORT_NUM
	set_gpio_state(CN_GPIO_OUT, CPLD_CS_STATE); // uncomment -- incifer 2009/04
#endif
}

void Set_0ToCPLDCS(void)
{
	set_gpio_value(CN_GPIO_LOW, CPLD_CS);
}

void Set_1ToCPLDStateOff(void)
{
	set_gpio_value(CN_GPIO_LOW, CPLD_SCLK);
}

void Set_1ToCPLDDataOff(void)
{
	set_gpio_value(CN_GPIO_LOW, CPLD_SDA);
}

void Set_1ToCPLDCS(void)
{
	set_gpio_value(CN_GPIO_HIGH, CPLD_CS);
}

int DetCPLDDataPin(void)
{
	u64 result;

	result = cvmx_read_csr(CVMX_GPIO_RX_DAT);
	result = result & CPLD_ODA;
	if(result)
		return 1;
	else
		return 0;
}

void Set_1ToCPLDData(void)
{
	set_gpio_value(CN_GPIO_HIGH, CPLD_SDA);
	set_gpio_value(CN_GPIO_LOW, CPLD_SCLK);
	CPLDdelay_160000();
	set_gpio_value(CN_GPIO_HIGH, CPLD_SCLK);
	CPLDdelay_270000();
	set_gpio_value(CN_GPIO_HIGH, CPLD_SCLK);
	CPLDdelay_270000();
	set_gpio_value(CN_GPIO_LOW, CPLD_SCLK);
	CPLDdelay_160000();
}

void Set_0ToCPLDData(void)
{
	set_gpio_value(CN_GPIO_LOW, CPLD_SDA);
	set_gpio_value(CN_GPIO_LOW, CPLD_SCLK);
	CPLDdelay_160000();
	set_gpio_value(CN_GPIO_HIGH, CPLD_SCLK);
	CPLDdelay_270000();
	set_gpio_value(CN_GPIO_HIGH, CPLD_SCLK);
	CPLDdelay_270000();
	set_gpio_value(CN_GPIO_LOW, CPLD_SCLK);
	CPLDdelay_160000();
}

void CPLD_control(unsigned char* len)
{
	int i;
	long int number[200];
	unsigned char temp[200];

	for(i=0;i<strlen(len);i++)
	{
		temp[0]=*(len+i);
		temp[1]='\0';

		number[i]=nk_trans_str_to_hex(*(len+i));

	}
	Set_CPLDToState();
	CPLDdelay_270000();
	Set_0ToCPLDCS();
	CPLDdelay_270000();
	Set_1ToCPLDStateOff();
	CPLDdelay_270000();
	Set_1ToCPLDDataOff();
	CPLDdelay_270000();
	Set_1ToCPLDCS();
	CPLDdelay_270000();

	for(i=0;i<strlen(len);i++)
	{
		if(number[i]&0X1)
			Set_1ToCPLDData();
		else
			Set_0ToCPLDData();

		nk_CPLD_state.return_number[i]=DetCPLDDataPin();
	}

	CPLDdelay_270000();
	CPLDdelay_270000();
}
#endif
