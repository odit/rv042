/** Global Variable **/
#ifdef CONFIG_NK_DYNAMIC_PORT_NUM
	/* Switch Type */
	extern int useSwitch;

	extern int DYNAMIC_NUM_LAN;
	extern int DYNAMIC_NUM_WAN;
	extern int DYNAMIC_NUM_DMZ;
	extern int DYNAMIC_MAX_SESSION;
	extern int DYNAMIC_HASH_SIZE;
	#define SWITCH005		5
	#define SWITCH363		363
	#define SWITCH650		650
	#define SWITCH1100		1100
	#define SWITCH1150		1150
	#define SWITCH1450		1450
	#define SWITCH1550		1550
	#define SWITCH2000		2000
	#define SWITCH2050		2050
	#define SWITCH2100		2100
	#define SWITCH3000		3000
	#define SWITCH016		16
	#define SWITCH042		42
	#define SWITCH082		82
	#define SWITCHUNKNOW	9999

	#undef CONFIG_NK_NUM_LAN
	#define CONFIG_NK_NUM_LAN	DYNAMIC_NUM_LAN
	#undef CONFIG_NK_NUM_WAN
	#define CONFIG_NK_NUM_WAN	DYNAMIC_NUM_WAN
	#undef CONFIG_NK_NUM_DMZ
	#define CONFIG_NK_NUM_DMZ	DYNAMIC_NUM_DMZ
#endif
