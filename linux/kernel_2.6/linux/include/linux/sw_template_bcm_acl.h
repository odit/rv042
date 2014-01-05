#ifndef __SW_TEMPLATE_BCM_ACL_H__
#define __SW_TEMPLATE_BCM_ACL_H__


/**
	sync: getlink.h, sw_template_bcm_acl.h switch.h
**/
typedef struct {
	uint32_t dir;/* LAN or WAN */
	int type;/* IPv4 or NonIP */
	/* TCAM Conf IPv4 */
	uint32_t ip_proto;
	/* UDF */
	int mac_type;/* src_mac[0]/dst_mac[1]/none[2] */
	uint64_t src_mac;
	uint64_t src_mac_mask;
	uint64_t dst_mac;
	uint64_t dst_mac_mask;

	int ip_type;
	uint64_t src_ip;
	uint64_t src_ip_mask;
	uint64_t dst_ip;
	uint64_t dst_ip_mask;

	int srcport_type, dstport_type;
	uint64_t src_port;/* ip protocol must be TCP or UDP */
	uint64_t dst_port;/* ip protocol must be TCP or UDP */

	uint32_t eth_type;

	int act_type;
	uint32_t rate;
	uint32_t prio;
}BCM_ACL_Control_t;


/** CFP Access Reg: 0xA0 0x00 **/
#define OFFSET_RD_STATUS				28
#define OFFSET_SEARCH_STATUS			27
#define OFFSET_ACCESS_ADDR				16
#define OFFSET_TCAM_RESET				15
	#define TCAM_RESET					( 1 << OFFSET_TCAM_RESET )
#define OFFSET_RAM_SEL					10
	#define OB_RAM						( 0x10 << OFFSET_RAM_SEL )
	#define IB_RAM						( 0x8 << OFFSET_RAM_SEL )
	#define Meter_RAM					( 0x4 << OFFSET_RAM_SEL )
	#define ACT_RAM						( 0x2 << OFFSET_RAM_SEL )
	#define TCAM						( 0x1 << OFFSET_RAM_SEL )
#define OFFSET_CFP_RAM_CLEAR			4
	#define CFP_RAM_CLEAR				( 0x1 << OFFSET_CFP_RAM_CLEAR )
#define OFFSET_OP_SEL					1
	#define READ_OP						( 0x1 << OFFSET_OP_SEL )
	#define WRITE_OP					( 0x2 << OFFSET_OP_SEL )
	#define SEARCH_OP					( 0x4 << OFFSET_OP_SEL )
#define OFFSET_OP_START					0
	#define OP_START					( 1 << OFFSET_OP_START )

/** CFP TCAM DATA 0xA0 0x10-0x2C **/
#define OFFSET_UDF_VALID				24
#define OFFSET_SRC_PORT_MAP				0
	#define SPM_P0						( 0x1 << OFFSET_SRC_PORT_MAP )
	#define SPM_P1						( 0x2 << OFFSET_SRC_PORT_MAP )
	#define SPM_P2						( 0x4 << OFFSET_SRC_PORT_MAP )
	#define SPM_P3						( 0x8 << OFFSET_SRC_PORT_MAP )
	#define SPM_P4						( 0x10 << OFFSET_SRC_PORT_MAP )
	#define SPM_P5						( 0x20 << OFFSET_SRC_PORT_MAP )
#define OFFSET_L3_FRAMING				24
	#define L3_FRAMING_IPV4				( 0x0 << OFFSET_L3_FRAMING )
	#define L3_FRAMING_IPV6				( 0x1 << OFFSET_L3_FRAMING )
	#define L3_FRAMING_NONIP			( 0x2 << OFFSET_L3_FRAMING )
	#define L3_IPV4						0x0
	#define L3_IPV6						0x1
	#define L3_NONIP					0x2
	#define L3_NULL						0xA
#define OFFSET_IP_PROTOCOL				8
	#define IP_PROTOCOL_ICMP			0x01
	#define IP_PROTOCOL_TCP				0x06
	#define IP_PROTOCOL_UDP				0x11
	#define IP_PROTOCOL_ESP				0x32
	#define IP_PROTOCOL_GRE				0x47
#define OFFSET_SLICE_ID					2
	#define SLICE_ID_0					( 0x0 << OFFSET_SLICE_ID )
	#define SLICE_ID_1					( 0x1 << OFFSET_SLICE_ID )
	#define SLICE_ID_2					( 0x2 << OFFSET_SLICE_ID )
	#define SLICE_ID_3					( 0x3 << OFFSET_SLICE_ID )
	#define SLICE0						0x0
	#define SLICE1						0x1
	#define SLICE2						0x2
	#define SLICE3						0x3
	#define SLICE_NULL					0xA
#define OFFSET_ETHER_TYPE				8
	#define ETHER_TYPE_ARP				0x0806
	#define ETHER_TYPE_MAC				0x8808

/** CFP Action Data 0 Reg: 0xA0 0x50 **/
#define OFFSET_CHANGE_FWD_OB			23
	#define CHANGE_FWD_OB_RDIR			( 0x2 << OFFSET_CHANGE_FWD_OB )
	#define CHANGE_FWD_OB_COPY			( 0x3 << OFFSET_CHANGE_FWD_OB )
#define OFFSET_DST_MAP_OB				16
	#define NULL_OB						0x0
#define OFFSET_CHANGE_FWD_IB			7
	#define CHANGE_FWD_IB_RDIR			( 0x2 << OFFSET_CHANGE_FWD_IB )
	#define CHANGE_FWD_IB_COPY			( 0x3 << OFFSET_CHANGE_FWD_IB )
#define OFFSET_DST_MAP_IB				0
	#define NULL_IB						0x0


/** CFP Action Data 1 Reg: 0xA0 0x54 **/
#define OFFSET_CHANGE_TC				13
#define OFFSET_NEW_TC					10


/** UDF Conf **/
#define OFFSET_BASE						5
	#define START_OF_PACKET				( 0x0 << OFFSET_BASE )
	#define END_OF_L2					( 0x2 << OFFSET_BASE )
	#define END_OF_L3					( 0x3 << OFFSET_BASE )


/** Template **/
#define T_DSTMAC						0x0
#define T_SRCMAC						0x1
#define T_DSTIP							0x2
#define T_SRCIP							0x4
#define T_DSTPORT						0x8
#define T_SRCPORT						0x10
#define T_ETHTYPE						0x20
#define T_IPPROTO						0x40

/** Action Type **/
#define RPDROP							0x0
#define DROP_OB							0x1
#define CHANGE_TC						0x2
#define OB_COPY							0x3


/** Port Map **/
#define PORT0_MAP					0x1
#define PORT1_MAP					0x2
#define PORT2_MAP					0x4
#define PORT3_MAP					0x8
#define PORT4_MAP					0x10
#define PORT5_MAP					0x20
#define ALL_PORT_MAP				0x3F


int BCM_ACL_Is_Init_CFP=0;
int BCM_ACL_Entry=0;

/** CFP UDF **/
unsigned int BCM_ACL_CFP_n_X[9];//[]
unsigned int BCM_ACL_CFP_n_X_Mask[9];//[]
unsigned int BCM_ACL_CFP_n_X_Conf[3][3][9];//[Slice][L3_type][]

/** TCAM DATA/MASK **/
unsigned int BCM_ACL_CFP_TCAM_DATA[8];
unsigned int BCM_ACL_CFP_TCAM_MASK[8];

/** CFP_n_A[] & CFP_TCAM_DATA MAP **/
unsigned int BCM_ACL_CFP_n_X_Map[9][2] = {
//	{Start Reg, End Reg}
	{0, 0},//CFP_n_A0
	{0, 1},//CFP_n_A1
	{1, 1},//CFP_n_A2
	{1, 2},//CFP_n_A3
	{2, 2},//CFP_n_A4
	{2, 3},//CFP_n_A5
	{3, 3},//CFP_n_A6
	{3, 4},//CFP_n_A7
	{4, 4} //CFP_n_A8
};

/** Action Reg **/
unsigned int BCM_ACL_Act_Reg[2];

/** Rate Meter Reg **/
unsigned int BCM_ACL_Rate_Reg[2];

/** CFP Rate Meter Data 0 Reg: 0xA0 0x60 **/
#define CURR_QUOTA						0
	#define QUOTA_16KB					0x003FFF
	#define QUOTA_32KB					0x007FFF
	#define QUOTA_64KB					0x00FFFF
	#define QUOTA_128KB					0x01FFFF
	#define QUOTA_256KB					0x03FFFF
	#define QUOTA_512KB					0x07FFFF
	#define QUOTA_1MB					0x0FFFFF
	#define QUOTA_2MB					0x1FFFFF
	#define QUOTA_4MB					0x3FFFFF
	#define QUOTA_8MB					0x7FFFFF
	#define QUOTA_16MB					0xFFFFFF
/** CFP Rate Meter Data 1 Reg: 0xA0 0x64 **/
#define OFFSET_RATE_REFRESH_EN			31
	#define RATE_EN						( 0x1 << OFFSET_RATE_REFRESH_EN )
#define OFFSET_REF_CAP					8
	#define BURST_16KB					0xA
	#define BURST_32KB					0x9
	#define BURST_64KB					0x8
	#define BURST_128KB					0x7
	#define BURST_256KB					0x6
	#define BURST_512KB					0x5
	#define BURST_1MB					0x4
	#define BURST_2MB					0x3
	#define BURST_4MB					0x2
	#define BURST_8MB					0x1
	#define BURST_16MB					0x0
#define OFFSET_TOKEN_NUM				0
/** Token Num 1->28 **/
	#define RATE_64Kb					0x01
	#define RATE_128Kb					0x02
	#define RATE_192Kb					0x03
	#define RATE_256Kb					0x04
	#define RATE_320Kb					0x05
	#define RATE_384Kb					0x06
	#define RATE_448Kb					0x07
	#define RATE_512Kb					0x08
	#define RATE_576Kb					0x09
	#define RATE_640Kb					0x0A
	#define RATE_704Kb					0x0B
	#define RATE_768Kb					0x0C
	#define RATE_832Kb					0x0D
	#define RATE_896Kb					0x0E
	#define RATE_960Kb					0x0F
	#define RATE_1024Kb					0x10
	#define RATE_1088Kb					0x11
	#define RATE_1152Kb					0x12
	#define RATE_1216Kb					0x13
	#define RATE_1280Kb					0x14
	#define RATE_1344Kb					0x15
	#define RATE_1408Kb					0x16
	#define RATE_1472Kb					0x17
	#define RATE_1536Kb					0x18
	#define RATE_1600Kb					0x19
	#define RATE_1664Kb					0x1A
	#define RATE_1728Kb					0x1B
	#define RATE_1792Kb					0x1C
/** Token Num 29->127 **/
	#define RATE_2Mb					0x1D
	#define RATE_3Mb					0x1E
	#define RATE_4Mb					0x1F
	#define RATE_5Mb					0x20
	#define RATE_6Mb					0x21
	#define RATE_7Mb					0x22
	#define RATE_8Mb					0x23
	#define RATE_9Mb					0x24
	#define RATE_10Mb					0x25
	#define RATE_11Mb					0x26
	#define RATE_12Mb					0x27
	#define RATE_13Mb					0x28
	#define RATE_14Mb					0x29
	#define RATE_15Mb					0x2A
	#define RATE_16Mb					0x2B
	#define RATE_17Mb					0x2C
	#define RATE_18Mb					0x2D
	#define RATE_19Mb					0x2E
	#define RATE_20Mb					0x2F
	#define RATE_21Mb					0x30
	#define RATE_22Mb					0x31
	#define RATE_23Mb					0x32
	#define RATE_24Mb					0x33
	#define RATE_25Mb					0x34
	#define RATE_26Mb					0x35
	#define RATE_27Mb					0x36
	#define RATE_28Mb					0x37
	#define RATE_29Mb					0x38
	#define RATE_30Mb					0x39
	#define RATE_31Mb					0x3A
	#define RATE_32Mb					0x3B
	#define RATE_33Mb					0x3C
	#define RATE_34Mb					0x3D
	#define RATE_35Mb					0x3E
	#define RATE_36Mb					0x3F
	#define RATE_37Mb					0x40
	#define RATE_38Mb					0x41
	#define RATE_39Mb					0x42
	#define RATE_40Mb					0x43
	#define RATE_41Mb					0x44
	#define RATE_42Mb					0x45
	#define RATE_43Mb					0x46
	#define RATE_44Mb					0x47
	#define RATE_45Mb					0x48
	#define RATE_46Mb					0x49
	#define RATE_47Mb					0x4A
	#define RATE_48Mb					0x4B
	#define RATE_49Mb					0x4C
	#define RATE_50Mb					0x4D
	#define RATE_51Mb					0x4E
	#define RATE_52Mb					0x4F
	#define RATE_53Mb					0x50
	#define RATE_54Mb					0x51
	#define RATE_55Mb					0x52
	#define RATE_56Mb					0x53
	#define RATE_57Mb					0x54
	#define RATE_58Mb					0x55
	#define RATE_59Mb					0x56
	#define RATE_60Mb					0x57
	#define RATE_61Mb					0x58
	#define RATE_62Mb					0x59
	#define RATE_63Mb					0x5A
	#define RATE_64Mb					0x5B
	#define RATE_65Mb					0x5C
	#define RATE_66Mb					0x5D
	#define RATE_67Mb					0x5E
	#define RATE_68Mb					0x5F
	#define RATE_69Mb					0x60
	#define RATE_70Mb					0x61
	#define RATE_71Mb					0x62
	#define RATE_72Mb					0x63
	#define RATE_73Mb					0x64
	#define RATE_74Mb					0x65
	#define RATE_75Mb					0x66
	#define RATE_76Mb					0x67
	#define RATE_77Mb					0x68
	#define RATE_78Mb					0x69
	#define RATE_79Mb					0x6A
	#define RATE_80Mb					0x6B
	#define RATE_81Mb					0x6C
	#define RATE_82Mb					0x6D
	#define RATE_83Mb					0x6E
	#define RATE_84Mb					0x6F
	#define RATE_85Mb					0x70
	#define RATE_86Mb					0x71
	#define RATE_87Mb					0x72
	#define RATE_88Mb					0x73
	#define RATE_89Mb					0x74
	#define RATE_90Mb					0x75
	#define RATE_91Mb					0x76
	#define RATE_92Mb					0x77
	#define RATE_93Mb					0x78
	#define RATE_94Mb					0x79
	#define RATE_95Mb					0x7A
	#define RATE_96Mb					0x7B
	#define RATE_97Mb					0x7C
	#define RATE_98Mb					0x7D
	#define RATE_99Mb					0x7E
	#define RATE_100Mb					0x7F
/** Token Num 128->240 **/
	#define RATE_104Mb					0x80
	#define RATE_112Mb					0x81
	#define RATE_120Mb					0x82
	#define RATE_128Mb					0x83
	#define RATE_136Mb					0x84
	#define RATE_144Mb					0x85
	#define RATE_152Mb					0x86
	#define RATE_160Mb					0x87
	#define RATE_168Mb					0x88
	#define RATE_176Mb					0x89
	#define RATE_184Mb					0x8A
	#define RATE_192Mb					0x8B
	#define RATE_200Mb					0x8C
	#define RATE_208Mb					0x8D
	#define RATE_216Mb					0x8E
	#define RATE_224Mb					0x8F
	#define RATE_232Mb					0x90
	#define RATE_240Mb					0x91
	#define RATE_248Mb					0x92
	#define RATE_256Mb					0x93
	#define RATE_264Mb					0x94
	#define RATE_272Mb					0x95
	#define RATE_280Mb					0x96
	#define RATE_288Mb					0x97
	#define RATE_296Mb					0x98
	#define RATE_304Mb					0x99
	#define RATE_312Mb					0x9A
	#define RATE_320Mb					0x9B
	#define RATE_328Mb					0x9C
	#define RATE_336Mb					0x9D
	#define RATE_344Mb					0x9E
	#define RATE_352Mb					0x9F
	#define RATE_360Mb					0xA0
	#define RATE_368Mb					0xA1
	#define RATE_376Mb					0xA2
	#define RATE_384Mb					0xA3
	#define RATE_392Mb					0xA4
	#define RATE_400Mb					0xA5
	#define RATE_408Mb					0xA6
	#define RATE_416Mb					0xA7
	#define RATE_424Mb					0xA8
	#define RATE_432Mb					0xA9
	#define RATE_440Mb					0xAA
	#define RATE_448Mb					0xAB
	#define RATE_456Mb					0xAC
	#define RATE_464Mb					0xAD
	#define RATE_472Mb					0xAE
	#define RATE_480Mb					0xAF
	#define RATE_488Mb					0xB0
	#define RATE_496Mb					0xB1
	#define RATE_504Mb					0xB2
	#define RATE_512Mb					0xB3
	#define RATE_520Mb					0xB4
	#define RATE_528Mb					0xB5
	#define RATE_536Mb					0xB6
	#define RATE_544Mb					0xB7
	#define RATE_552Mb					0xB8
	#define RATE_560Mb					0xB9
	#define RATE_568Mb					0xBA
	#define RATE_576Mb					0xBB
	#define RATE_584Mb					0xBC
	#define RATE_592Mb					0xBD
	#define RATE_600Mb					0xBE
	#define RATE_608Mb					0xBF
	#define RATE_616Mb					0xC0
	#define RATE_624Mb					0xC1
	#define RATE_632Mb					0xC2
	#define RATE_640Mb					0xC3
	#define RATE_648Mb					0xC4
	#define RATE_656Mb					0xC5
	#define RATE_664Mb					0xC6
	#define RATE_672Mb					0xC7
	#define RATE_680Mb					0xC8
	#define RATE_688Mb					0xC9
	#define RATE_696Mb					0xCA
	#define RATE_704Mb					0xCB
	#define RATE_712Mb					0xCC
	#define RATE_720Mb					0xCD
	#define RATE_728Mb					0xCE
	#define RATE_736Mb					0xCF
	#define RATE_744Mb					0xD0
	#define RATE_752Mb					0xD1
	#define RATE_760Mb					0xD2
	#define RATE_768Mb					0xD3
	#define RATE_776Mb					0xD4
	#define RATE_784Mb					0xD5
	#define RATE_792Mb					0xD6
	#define RATE_800Mb					0xD7
	#define RATE_808Mb					0xD8
	#define RATE_816Mb					0xD9
	#define RATE_824Mb					0xDA
	#define RATE_832Mb					0xDB
	#define RATE_840Mb					0xDC
	#define RATE_848Mb					0xDD
	#define RATE_856Mb					0xDE
	#define RATE_864Mb					0xDF
	#define RATE_872Mb					0xE0
	#define RATE_880Mb					0xE1
	#define RATE_888Mb					0xE2
	#define RATE_896Mb					0xE3
	#define RATE_904Mb					0xE4
	#define RATE_912Mb					0xE5
	#define RATE_920Mb					0xE6
	#define RATE_928Mb					0xE7
	#define RATE_936Mb					0xE8
	#define RATE_944Mb					0xE9
	#define RATE_952Mb					0xEA
	#define RATE_960Mb					0xEB
	#define RATE_968Mb					0xEC
	#define RATE_976Mb					0xED
	#define RATE_984Mb					0xEE
	#define RATE_992Mb					0xEF
	#define RATE_1000Mb					0xF0

#endif
