/*

*/

#define OCTEON_I2C_IRQ ???   //?????? should in irq.h and check it

#define IO_ADDRESS(x) (0x8000000000000000 + x)

#define OCTEON_I2C_IOSIZE (0x20)

#define OCTEON_I2C_BASE IO_ADDRESS(0x0001180000001000)



#define OCTEON_MIO_TWS_SW_TWSI (OCTEON_I2C_BASE + 0x00)


typedef union
{
    uint64_t u64;
    struct
    {
	uint64_t v	: 1;//valid bit : set on a write operation.
	uint64_t slonly	: 1;//slave-only mode.
	uint64_t eia	: 1;//extended internal address.
	uint64_t op	: 4;//opcode field.
	uint64_t r	: 1;//read bit or result.
	uint64_t sovr	: 1;// size override. 
	uint64_t size	: 3;//size. sovr = 1.
	uint64_t scr	: 2;//scratch. Unused, but retain state.
	uint64_t a	: 10;//address fied. 
	uint64_t ia	: 5;//internal address.
	uint64_t eop_ia	: 3;// extra opcode.
	uint64_t d	: 32;//data field.

    }s;
} octeon_mio_tws_sw_twsi_t;


#define OCTEON_MIO_TWS_TWSI_SW	(OCTEON_I2C_BASE + 0x08)

typedef union{
    uint64_t u64;
    struct{
	uint64_t v	: 2;
	uint64_t rsvd	: 30;
	uint64_t d	: 32;
    }s;
} octeon_mio_tws_twsi_sw_t;


#define OCTEON_MIO_INT	(OCTEON_I2C_BASE + 0x10)

typedef union{
    uint64_t u64;
    struct{
	uint64_t rsvd1	: 55;
	uint64_t core_en : 1;
	uint64_t ts_en	: 1;
	uint64_t st_en	: 1;
	uint64_t resvd	: 1;
	uint64_t core_int : 1;
	uint64_t ts_int	:1;
	uint64_t st_int	:1;
    }s;
} octeon_mio_tws_int_t;

#define OCTEON_MIO_TWS_SW_TWSI_EXT (OCTEON_I2C_BASE + 0x18)

typedef union{
    uint64_t u64;
    struct{
	uint64_t rsvd	: 24;
    	uint64_t ia	: 8;
	uint64_t d	: 32;
		
    }s;
} octeon_mio_tws_sw_twsi_ext_t;





static inline void octeon_write_csr(uint64_t csr_addr, uint64_t val)
{
            *(volatile uint64_t *)csr_addr = val;
}

static inline uint64_t octeon_read_csr(uint64_t csr_addr)
{
            return *(volatile uint64_t *)csr_addr;
}


