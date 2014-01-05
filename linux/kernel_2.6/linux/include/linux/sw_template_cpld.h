#ifndef __SW_TEMPLATE_CPLD_H__
#define __SW_TEMPLATE_CPLD_H__


#define SW_TEMPLATE_CPLD_INPUT_LEN		40

#define SW_TEMPLATE_CPLD_MSBq_LEN		16
#define SW_TEMPLATE_CPLD_MSBp_LEN		16

#define SW_TEMPLATE_CPLD_DISABLE		0
#define SW_TEMPLATE_CPLD_ENABLE			1

#define SW_TEMPLATE_CPLD_DELAY			10

#define SW_TEMPLATE_CPLD_GPIO_NO		3


typedef struct {
	unsigned char len[200];
	unsigned int return_number[200];
	int length;
}Sw_Template_CPLD_t;

typedef struct {
	uint32_t model;
	uint32_t cs;
	uint32_t cs_enable;
	uint32_t clk;
	uint32_t sda;
	uint32_t oda;
}Sw_Template_CPLD_GPIO_t;


typedef struct {
	void ( *get_CPLD_value )	( void );
}Sw_Template_CPLD_Func_t;


Sw_Template_CPLD_GPIO_t Sw_Template_CPLD_GPIO[] = {
//	Model, CS(enable pin, CPU output), CS_Enable(cs enable value), CLK(clock pin, CPU output), SDA(CPLD data input, CPU output), ODA(CPLD data output, CPU input)
	{  650,  5, 0,  2,  3,  4 },/* 6 Layer PCB */
	{ 1100,  7, 0,  9, 10, 11 },
	{    0,  0, 0,  0,  0,  0 }
};


typedef struct {
	uint32_t model;
	uint32_t num_lan;
	uint32_t num_wan;
	uint32_t num_dmz;
	uint32_t num_max_session;
	uint32_t num_hash;
	int MSBq[SW_TEMPLATE_CPLD_MSBq_LEN];
	int MSBp[SW_TEMPLATE_CPLD_MSBp_LEN];
}Sw_Template_CPLD_Model_Info_t;


Sw_Template_CPLD_Model_Info_t Sw_Template_CPLD_Model_Info[] = {
	/* GQF650 */
	{
		650, 4, 5, 0,
		{	0,1,0,0,
			0,1,0,1,
			1,0,0,0,
			0,0,0,0
		},
		{
			0,0,1,0,
			1,0,0,1,
			1,1,0,0,
			0,0,0,0
		}
	},

	/* GQF1100 */
	{
		1100, 8, 5, 0,
		{
			1,0,1,1,
			1,0,0,0,
			1,0,1,0,
			1,0,0,1
		},
		{
			0,1,1,0,
			0,0,1,1,
			1,0,1,0,
			1,1,0,0
		}
	},

	/* GQF1150 */
	{
		1150, 8, 5, 0,
		{
			1,0,1,0,
			0,1,1,0,
			0,1,0,0,
			1,0,0,1
		},
		{
			0,0,0,0,
			1,0,1,0,
			1,0,0,0,
			1,0,0,0
		}
	},
	/* GQF1450 */
	{
		1450, 5, 8, 0,
		{
			0,0,1,0,
			1,0,0,1,
			1,1,0,0,
			0,0,0,0
		},
		{
			0,1,0,0,
			0,1,0,1,
			1,0,0,0,
			0,0,0,0
		}
	},

	/* GQF2000 */
	{
		2000, 8, 5, 0,
		{
			1,1,1,0,
			0,0,1,1,
			0,1,1,0,
			0,1,0,0
		},
		{
			0,0,1,1,
			1,0,1,0,
			0,1,1,0,
			1,0,0,1
		}
	},

	/* GQF2050 */
	{
		2050, 8, 5, 0,
		{
			0,1,1,0,
			0,1,1,1,
			0,1,0,0,
			0,1,0,0
		},
		{
			0,0,0,1,
			1,0,1,1,
			0,1,1,0,
			0,0,1,1
		}
	},

	/* GQF2100 */
	{
		2100, 8, 5, 0,
		{
			1,0,0,0,
			1,0,0,1,
			0,0,0,1,
			0,1,0,0
		},
		{
			0,0,0,0,
			0,0,0,1,
			1,0,0,0,
			1,1,0,0
		}
	},

	/* Null */
	{
		0, 0, 0, 0,
		{
			0,0,0,0,
			0,0,0,0,
			0,0,0,0,
			0,0,0,0
		},
		{
			0,0,0,0,
			0,0,0,0,
			0,0,0,0,
			0,0,0,0
		}
	}
};

Sw_Template_CPLD_Model_Info_t Sw_Template_CPLD_Model_Info_650[] = {
	/* GQF650 6 Layer PCB */
	{
		650, 4, 5, 0,
		{	0,1,0,0,
			0,1,0,1,
			1,0,0,0,
			0,0,0,0
		},
		{
			0,0,1,0,
			1,0,0,1,
			1,1,0,0,
			0,0,0,0
		}
	},

	/* Null */
	{
		0, 0, 0, 0,
		{
			0,0,0,0,
			0,0,0,0,
			0,0,0,0,
			0,0,0,0
		},
		{
			0,0,0,0,
			0,0,0,0,
			0,0,0,0,
			0,0,0,0
		}
	}
};

Sw_Template_CPLD_Model_Info_t Sw_Template_CPLD_Model_Info_1100[] = {
	/* SSL005 */
	{
		5, 4, 4, 0, 200000, 262140,
		{
			1,0,1,0,
			1,0,0,0,
			0,1,1,0,
			0,0,0,0
		},
		{
			1,0,0,1,
			1,0,0,0,
			1,1,1,0,
			0,0,0,0
		}
	},
	/* 363G */
	{
		363, 2, 2, 0, 200000, 262140,
		{
			1,0,0,0,
			0,1,0,0,
			1,0,1,0,
			0,0,0,0
		},
		{
			1,1,0,0,
			1,1,0,0,
			0,1,1,0,
			0,0,0,0
		}
	},
	/* GQF650 4 Layer PCB */
	{
		650, 5, 5, 0, 200000, 262140,
		{	0,1,0,0,
			0,1,0,1,
			1,0,0,0,
			0,0,0,0
		},
		{
			0,0,1,0,
			1,0,0,1,
			1,1,0,0,
			0,0,0,0
		}
	},
	/* GQF1100 */
	{
		1100, 8, 4, 1, 200000, 262140,
		{
			1,0,1,1,
			1,0,0,0,
			1,0,1,0,
			1,0,0,1
		},
		{
			0,1,1,0,
			0,0,1,1,
			1,0,1,0,
			1,1,0,0
		}
	},

	/* GQF1150 */
	{
		1150, 8, 5, 0, 200000, 262140,
		{
			1,0,1,0,
			0,1,1,0,
			0,1,0,0,
			1,0,0,1
		},
		{
			0,0,0,0,
			1,0,1,0,
			1,0,0,0,
			1,0,0,0
		}
	},
	/* GQF1450 */
	{
		1450, 5, 8, 0, 200000, 262140,
		{
			0,0,1,0,
			1,0,0,1,
			1,1,0,0,
			0,0,0,0
		},
		{
			0,1,0,0,
			0,1,0,1,
			1,0,0,0,
			0,0,0,0
		}
	},

	/* QVM1550 */
	{
		1550, 5, 5, 0, 200000, 262140,
		{
			1,0,1,0,
			1,0,0,0,
			0,1,0,0,
			0,0,0,0
		},
		{
			0,0,0,0,
			0,1,0,0,
			0,1,1,0,
			0,0,0,0
		}
	},

	/* GQF2000 */
	{
		2000, 8, 4, 1, 200000, 262140,
		{
			1,1,1,0,
			0,0,1,1,
			0,1,1,0,
			0,1,0,0
		},
		{
			0,0,1,1,
			1,0,1,0,
			0,1,1,0,
			1,0,0,1
		}
	},

	/* GQF2050 */
	{
		2050, 8, 4, 1, 200000, 262140,
		{
			0,1,1,0,
			0,1,1,1,
			0,1,0,0,
			0,1,0,0
		},
		{
			0,0,0,1,
			1,0,1,1,
			0,1,1,0,
			0,0,1,1
		}
	},

	/* GQF2100 */
	{
		2100, 8, 4, 1, 200000, 262140,
		{
			1,0,0,0,
			1,0,0,1,
			0,0,0,1,
			0,1,0,0
		},
		{
			0,0,0,0,
			0,0,0,1,
			1,0,0,0,
			1,1,0,0
		}
	},

	/* QTM3000 */
	{
		3000, 8, 8, 0, 200000, 262140,
		{
			0,1,0,0,
			1,0,0,0,
			0,0,1,0,
			0,1,1,0
		},
		{
			1,0,1,0,
			0,0,0,0,
			1,0,0,1,
			1,0,0,0
		}
	},

	/* RV016 */
	{
		16, 8, 7, 1, 40000, 32768,
		{
			1,0,1,0,
			1,0,0,0,
			0,1,0,0,
			0,0,0,0
		},
		{
			0,1,1,0,
			0,0,0,1,
			0,1,1,0,
			0,0,0,0
		}
	},

	/* RV042 */
	{
		42, 4, 2, 0, 12000, 8192,
		{
			0,0,0,0,
			1,0,0,0,
			0,1,0,0,
			1,1,0,0
		},
		{
			0,1,0,0,
			0,1,1,0,
			0,1,1,0,
			0,0,0,0
		}
	},

	/* RV082 */
	{
		82, 8, 2, 0, 20000, 16384,
		{
			0,0,0,1,
			1,0,0,0,
			1,0,0,0,
			0,0,0,0
		},
		{
			0,1,0,0,
			1,1,1,0,
			0,1,1,0,
			0,0,0,0
		}
	},

	/* Null */
	{
		0, 0, 0, 0,
		{
			0,0,0,0,
			0,0,0,0,
			0,0,0,0,
			0,0,0,0
		},
		{
			0,0,0,0,
			0,0,0,0,
			0,0,0,0,
			0,0,0,0
		}
	}
};


typedef struct {
	Sw_Template_CPLD_GPIO_t *gpio;
	Sw_Template_CPLD_Model_Info_t *model_info;
}Sw_Template_CPLD_GPIO_Model;


Sw_Template_CPLD_GPIO_Model cpld_gpio_model[] = {
#if 0
	/* 6 Layer PCB */
	{ &( Sw_Template_CPLD_GPIO[0] ), Sw_Template_CPLD_Model_Info_650 },
#endif
	{ &( Sw_Template_CPLD_GPIO[1] ), Sw_Template_CPLD_Model_Info_1100 },
	{ NULL, NULL }
};

#endif
