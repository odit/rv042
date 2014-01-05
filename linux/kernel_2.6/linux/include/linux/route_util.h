#define MAJOR_NUM		237
#define MINOR_NUM		1
struct SessionDstHead{
	int TableSize;
	struct SessionDstInfo *DstTablePtr;
};

struct SessionLockHead{
	int TableSize;
	struct SessionLockInfo *LockTablePtr;
};

struct SessionSrcHead{
	struct SessionSrcInfo *next;
	struct SessionSrcInfo **end;
};

struct SessionDstInfo{
	u32 dst_sip;
	u32 dst_eip;
	u32 dst_sport;
	u32 dst_eport;
	u32 protocol;
};

struct SessionLockInfo{
	u_int32_t protocol;
	u_int32_t dst_sport;
	u_int32_t dst_eport;
};

struct SessionSrcInfo{
	u32 src_ip;
	u32 dst_index;
	u32 dst_ip;
        u32 dst_port;
	u32 protocol;
	struct net_device *inf;
	struct SessionSrcInfo *next;
	unsigned char table;
	unsigned long lastuse;
};

struct SessionSrcFlowInfo{
	u32 src_ip;
	unsigned long lastuse;
};

struct SessiondstFlowInfo{
        int used;//if this value = 1, it means "used", else value means "unused"
	u32 dst_ip;
	struct net_device *inf;
	unsigned char table;
	unsigned long lastuse;
};

#define ROUTE_TRANS_DST_TABLE		_IOWR(MAJOR_NUM, 0x35, struct SessionDstHead)
#define ROUTE_TRANS_LOCK_TABLE		_IOWR(MAJOR_NUM, 0x41, struct SessionLockHead)
