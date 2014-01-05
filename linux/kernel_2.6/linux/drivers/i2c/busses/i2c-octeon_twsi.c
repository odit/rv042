/*
*  use the poll mode not the interrupt mode.
*  modified from the i2c-omap.c
*/
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/ioport.h>
#include <linux/version.h>
#include <linux/i2c.h>
#include <asm/irq.h>
#include <asm/io.h>
//#include <asm/hardware/clock.h>

#include <asm/uaccess.h>
#include <linux/errno.h>
#include <linux/sched.h>
//#include <asm/arch/hardware.h>
#include <linux/interrupt.h>
#include <linux/moduleparam.h>
//#include <asm/arch/mux.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include "i2c-octeon_twsi.h"

#define 	I2C_MAX_TIMEOUT 	10000	// 10ms

//#define	I2C_OCTEON_DEBUG

#ifdef I2C_OCTEON_DEBUG
static int i2c_debug = 0;
#define DEB0(format, arg...)	printk(KERN_DEBUG MODULE_NAME " DEBUG: " format "\n",  ## arg )
#define DEB1(format, arg...)	\
	if (i2c_debug>=1) {	\
  		printk(KERN_DEBUG MODULE_NAME " DEBUG: " format "\n",  ## arg ); \
    	}
#define DEB2(format, arg...)	\
	if (i2c_debug>=2) {	\
  		printk(KERN_DEBUG MODULE_NAME " DEBUG: " format "\n",  ## arg ); \
    	}
#define DEB3(format, arg...)	\
	if (i2c_debug>=3) {	\
    		printk(KERN_DEBUG MODULE_NAME " DEBUG: " format "\n",  ## arg ); \
    	}
#define DEB9(format, arg...)	\
/* debug the protocol by showing transferred bits */	\
	if (i2c_debug>=9) {	\
    		printk(KERN_DEBUG MODULE_NAME " DEBUG: " format "\n",  ## arg ); \
   	}
#else
#define DEB0(fmt, args...)
#define DEB1(fmt, args...)
#define DEB2(fmt, args...)
#define DEB3(fmt, args...)
#define DEB9(fmt, args...)
#endif


#define MODULE_NAME "OCTEON I2C"

static int i2c_scan = 0;

//invalid address will return 1; TEN bit addr is valid
static inline int i2c_invalid_address(const struct i2c_msg* p)
{
    return (p->addr > 0x3ff) || (!(p->flags & I2C_M_TEN) && (p->addr > 0x7f));
}

static void octeon_i2c_reset(void)
{
    /* Nothing */
}

static int octeon_i2c_xfer_msg(struct i2c_adapter *adap, struct i2c_msg *msg, int combined)
{
    uint64_t data = 0;
    int i;
    int timeout = 0;
    octeon_mio_tws_sw_twsi_t temp, mio_tws_sw_twsi;
    octeon_mio_tws_sw_twsi_ext_t mio_tws_sw_twsi_ext;

    DEB2("addr: 0x%04x, len: %d, flags: 0x%x, buf[0] = %x\n", msg->addr, msg->len, msg->flags, msg->buf[0]);


    mio_tws_sw_twsi.u64 = 0x0;

    mio_tws_sw_twsi.s.v = 1;

    //ten bit address op<1> = 1
    if( msg->flags & I2C_M_TEN) mio_tws_sw_twsi.s.op |= 0x2;
    mio_tws_sw_twsi.s.a = msg->addr & 0x3ff;

    // check the msg->len  0<=len <8
    if( msg->len > 8 ){
	printk("%s %d Error len msg->len %d\n", __FILE__, __LINE__, msg->len);
	return (-1);
    }
    mio_tws_sw_twsi.s.sovr = 1;			// size override.
    if ( msg->len == 0 )	
       mio_tws_sw_twsi.s.size = 0;
    else
       mio_tws_sw_twsi.s.size = msg->len-1;	// Size: 0 = 1 byte, 1 = 2 bytes, ..., 7 = 8 bytes

    if( msg->flags & I2C_M_RD ){
	mio_tws_sw_twsi.s.r = 1;		// Enable Read bit 
    }else{	
	for(i =0; i <= mio_tws_sw_twsi.s.size; i++){
	    data = data << 8;	
	    data |= msg->buf[i];
	}

	mio_tws_sw_twsi.s.d = data;
	mio_tws_sw_twsi_ext.s.d = data >> 32;	
    }
	
#ifdef I2C_OCTEON_DEBUG
    if ( mio_tws_sw_twsi.s.r == 1 )
	printk("twsi-read  op: data=%llx %llx len=%d\n", mio_tws_sw_twsi.u64, mio_tws_sw_twsi_ext.u64, msg->len);
    else
        printk("twsi-write op: data=%llx %llx len=%d\n", mio_tws_sw_twsi.u64, mio_tws_sw_twsi_ext.u64, msg->len);
#endif

    octeon_write_csr(OCTEON_MIO_TWS_SW_TWSI_EXT, mio_tws_sw_twsi_ext.u64);
    octeon_write_csr(OCTEON_MIO_TWS_SW_TWSI, mio_tws_sw_twsi.u64);


    //Poll! wait the transfer complete and timeout (10ms).
    do{
	temp.u64 = octeon_read_csr(OCTEON_MIO_TWS_SW_TWSI);	
	udelay(1);
    }while (temp.s.v && (timeout++ < I2C_MAX_TIMEOUT));

    mio_tws_sw_twsi.u64 = octeon_read_csr(OCTEON_MIO_TWS_SW_TWSI);

    if (timeout >= I2C_MAX_TIMEOUT) {
	printk("Octeon twsi I2C Timeout!\n");
	octeon_i2c_reset();
	return -EIO;
    }

    //transfer ERROR
    if (!mio_tws_sw_twsi.s.r){
	octeon_i2c_reset();
	return -EIO;
    }

    if (msg->flags & I2C_M_RD){

	mio_tws_sw_twsi_ext.u64 = octeon_read_csr(OCTEON_MIO_TWS_SW_TWSI_EXT);
	data = ((uint64_t) mio_tws_sw_twsi_ext.s.d << 32) | mio_tws_sw_twsi.s.d;
	
#ifdef I2C_OCTEON_DEBUG
	printk("twsi-read result: data=%llx %llx len=%d\n", mio_tws_sw_twsi.u64, mio_tws_sw_twsi_ext.u64, msg->len);
#endif

	for(i = mio_tws_sw_twsi.s.size; i >= 0; i--){
		msg->buf[i] = data;
		data = data >> 8; 
	}	
    }

    return msg->len;
}

static int octeon_i2c_xfer(struct i2c_adapter *adap, struct i2c_msg *msgs, int num)
{
    int i;
    int r = 0;

    DEB1("msgs: %d\n", num);

    if(unlikely(!num)){
	printk("num is zero!!\n");
	return 0;
    }

    if (unlikely(i2c_invalid_address(&msgs[0]))){
	printk("invalid address 0x%03x (%d-bit)\n", msgs[0].addr, msgs[0].flags & I2C_M_TEN ? 10 : 7);
	return -EINVAL;
    }

    for (i = 0; i < num; ++i){
	DEB2("msg: %d, addr: 0x%04x, len: %d, flags: 0x%x\n", i, msgs[i].addr, msgs[i].len, msgs[i].flags);
	
	r = octeon_i2c_xfer_msg(adap, &msgs[i], 0);

	DEB2("r: %d\n", r);

	if(r != msgs[i].len)
	    break;
	
    }

    DEB1("r: %d msgs: %d", r, num);

    return (r < 0) ? r : num;
}



static int octeon_i2c_remove(struct device *dev)
{
    /* Nothing */
    return 0;
}

static void octeon_i2c_device_release(struct device *dev)
{
    /* Nothing */
}



//should contain I2C_FUNC_SMBUS_BYTE_DATA
static u32 octeon_i2c_func(struct i2c_adapter * adap)
{
    //should be check
    return I2C_FUNC_I2C | I2C_FUNC_SMBUS_EMUL;
}

static struct i2c_algorithm octeon_i2c_algo = {
    .master_xfer = octeon_i2c_xfer,
    .smbus_xfer	= NULL,//emulate by master_xfer
    .slave_send = NULL,
    .slave_recv = NULL, //
    .algo_control =NULL,
    .functionality = octeon_i2c_func,
};



static struct i2c_adapter octeon_i2c_adap = {
    .owner	= THIS_MODULE,
    .class	= I2C_CLASS_HWMON,//I2C_CLASS_ALL
    .name	= "octeon twsi",
    .algo	= &octeon_i2c_algo,
    .algo_data	=NULL,
    .client_register = NULL,
    .client_unregister	= NULL,
};

static struct device_driver octeon_i2c_driver = {
    .name	= "octeon_i2c",
    .bus	= &platform_bus_type,
    .remove	= octeon_i2c_remove,
};

static struct platform_device octeon_i2c_device = {
    .name	= "i2c",
    .id		= -1,
    .dev	={
	.driver	= &octeon_i2c_driver,
	.release = octeon_i2c_device_release,
    },
};

static void __init octeon_i2c_chip_init(void)
{
    /* Nothing */
}


static int
octeon_i2c_scan_bus(struct i2c_adapter *adap)
{
    int found = 0;
    int i;
    struct i2c_msg msg;
    char data[1];

    printk("scanning for active I2C devices on the bus...");

    for (i = 1; i < 0x7f; i++) {
	
	msg.addr = i;
	msg.buf = data;
	msg.len = 0;
	msg.flags = I2C_M_RD;
	
	if (octeon_i2c_xfer(adap, &msg, 1) == 0) {
	    printk("I2C device 0x%02x found", i);
	    found++;
	}
    }

    if (!found)
	printk("found nothing");

    return found;
}

static int __init octeon_i2c_init(void)
{
    int r;

   r = i2c_add_adapter(&octeon_i2c_adap);

   if(r){
	printk(KERN_ERR "failed to add adapter");
	goto do_release_region; //do_free_irq;
	return r;
    }


    octeon_i2c_chip_init();

    if(i2c_scan)
	octeon_i2c_scan_bus(&octeon_i2c_adap);

    if(driver_register(&octeon_i2c_driver)!=0)
	printk(KERN_ERR "Driver register failed for octeon_i2c\n");

    if(platform_device_register(&octeon_i2c_device)!=0){
	printk(KERN_ERR "Device register failed for i2c\n");
	driver_unregister(&octeon_i2c_driver);
    }



    return 0;

do_release_region:
//    release_region(OCTEON_I2C_BASE, OCTEON_I2C_IOSIZE);
    return 0;

}


static void __exit octeon_i2c_exit(void)
{
    i2c_del_adapter(&octeon_i2c_adap);
//    free_irq(OCTEON_I2C_IRQ, &octeon_i2c_dev);
//    release_region(OCTEON_I2C_BASE, OCTEON_I2C_IOSIZE);
    driver_unregister(&octeon_i2c_driver);
    platform_device_unregister(&octeon_i2c_device);
}


MODULE_AUTHOR("RichardXY_huang@asus.com.cn");
MODULE_DESCRIPTION("Cavium Octeon I2C bus adapter");
MODULE_DESCRIPTION("Octeon i2c driver");
MODULE_LICENSE("GPL");

//subsys_initcall(octeon_iic_init);
module_init(octeon_i2c_init);
module_exit(octeon_i2c_exit);
