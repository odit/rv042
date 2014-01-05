/*
 * rs5c372.c
 *
 * Device driver for Real Time Controller's rs5c372 chips
 *
 * Copyright (C) 2005 Pavel Mironchik pmironchik@optifacio.net
 *	
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This driver is modified by Robert
 */

#include <linux/config.h>
#include <linux/module.h>
#include <linux/version.h>

#include <linux/kernel.h>
#include <linux/poll.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/rtc.h>
#include <linux/string.h>
#include <linux/miscdevice.h>
#include <linux/proc_fs.h>
#include <linux/time.h>
#include <linux/bcd.h>

#define PROC_RS5C372_NAME	"driver/rs5c372"
#define RS5C372_I2C_SLAVE_ADDR  0x32
#define RS5C372_RAM_ADDR_START	0x00
#define RS5C372_RAM_ADDR_END	0x0F
#define RS5C372_RAM_SIZE 		0x10
#define I2C_DRIVERID_RS5C372    0xF6

#define RS5C372_GETDATETIME	0
#define RS5C372_SETTIME		1
#define RS5C372_SETDATETIME	2

//------------------------------------------------------------//
#define RS5C372_REG_BASE   1
#define RS5C372_REG_SECS   RS5C372_REG_BASE + 0 
#define RS5C372_REG_MINS   RS5C372_REG_BASE + 1
#define RS5C372_REG_HOURS  RS5C372_REG_BASE + 2
#define RS5C372_REG_WDAY   RS5C372_REG_BASE + 3
#define RS5C372_REG_DAY    RS5C372_REG_BASE + 4
#define RS5C372_REG_MONTH  RS5C372_REG_BASE + 5
#define RS5C372_REG_YEAR   RS5C372_REG_BASE + 6
#define RS5C372_REG_TRIM   RS5C372_REG_BASE + 7

#define RS5C372_TRIM_XSL   0x80
#define RS5C372_TRIM_MASK  0x7F
//-----------------------------------------------------------//
// ALARM A registers
#define RTC_AM		8
#define RTC_AH		9
#define RTC_AW		10

// ALARM B registers
#define RTC_BM		11
#define RTC_BH		12
#define RTC_BW		13

// Control registers
#define RTC_CR1		14
#define RTC_CR2		15

#define DRV_VERSION "0.1"  

struct i2c_driver rs5c372_driver;
struct i2c_client *rs5c372_i2c_client = NULL;
static struct i2c_client_address_data addr_data;
static spinlock_t rs5c372_rtc_lock = SPIN_LOCK_UNLOCKED;
static unsigned short slave_address = RS5C372_I2C_SLAVE_ADDR;

static unsigned short ignore[]	= 	{ I2C_CLIENT_END };
static unsigned short normal_addr[] =	{ RS5C372_I2C_SLAVE_ADDR, I2C_CLIENT_END };

static int rs5c372_rtc_ioctl(struct inode *, struct file *, unsigned int,
			      unsigned long);
static int rs5c372_rtc_open(struct inode *inode, struct file *file);
static int rs5c372_rtc_release(struct inode *inode, struct file *file);

static struct file_operations rtc_fops = {
      owner:THIS_MODULE,
      ioctl:rs5c372_rtc_ioctl,
      open:rs5c372_rtc_open,
      release:rs5c372_rtc_release,
};


static struct miscdevice rs5c372_rtc_miscdev = {
	RTC_MINOR,
	"rtc",
	&rtc_fops
};

static int rs5c372_attach(struct i2c_adapter *adapter);
static int rs5c372_detach(struct i2c_client *client);
static int rs5c372_command(struct i2c_client *client, unsigned int cmd, void *arg);
static int rs5c372_probe(struct i2c_adapter *adap, int addr);

struct i2c_driver rs5c372_driver = {
   .driver      = {
      .name   = "rtc-rs5c372",
   },
      .attach_adapter = &rs5c372_attach,
      .detach_client  = &rs5c372_detach,
};

static int rs5c372_attach(struct i2c_adapter *adapter)
{
   dev_dbg(&adapter->dev, "%s\n", __FUNCTION__);
   addr_data.probe[1] = RS5C372_I2C_SLAVE_ADDR;
   return i2c_probe(adapter, &addr_data, rs5c372_probe);
}

static int rs5c372_probe(struct i2c_adapter *adap, int addr)
{
	int ret = 0;
	struct i2c_client *c;
	struct rtc_time rtctm;
	
	if (!(c = kmalloc(sizeof(struct i2c_client), GFP_KERNEL))) {
		return -ENOMEM;
	}

	memset(c, 0, sizeof(struct i2c_client));
	strncpy(c->name, "rs5c372", I2C_NAME_SIZE);
	
	c->addr		= addr;
	c->adapter	= adap;
	c->driver	= &rs5c372_driver;
	
	ret = i2c_attach_client(c);
	rs5c372_i2c_client = c;
	rs5c372_command(rs5c372_i2c_client, RS5C372_GETDATETIME,
		(void *)&rtctm);

	return ret;
}

static int rs5c372_detach(struct i2c_client *client)
{
	if (client) {
		i2c_detach_client(client);
		kfree(client);
		client = rs5c372_i2c_client = NULL;
	}
	return 0;
}

static int rs5c372_get_datetime(struct i2c_client *client, struct rtc_time *tm)
{
   int iret = -1, iretry = 0;
   unsigned char buf[8] = {0};

   /* this implements the 1st reading method, according
    * to the datasheet. buf[0] is initialized with
    * address ptr and transmission format register.
    */
   struct i2c_msg msgs[] = {
      { client->addr, 0, 1, buf },
      { client->addr, I2C_M_RD, 8, buf },
   };


   while( (iret != 0) && (iretry < 3) ) 
   {
		if ((i2c_transfer(client->adapter, msgs, 2)) != 2) iret = -EIO;
		else iret = 0;
		iretry ++;
   }
   
   
   if (!iret) 
   {
		tm->tm_sec = BCD2BIN(buf[RS5C372_REG_SECS] & 0x7f);
		tm->tm_min = BCD2BIN(buf[RS5C372_REG_MINS] & 0x7f);
		tm->tm_hour = BCD2BIN(buf[RS5C372_REG_HOURS] & 0x3f);   
		tm->tm_wday = BCD2BIN(buf[RS5C372_REG_WDAY] & 0x07);
		tm->tm_mday = BCD2BIN(buf[RS5C372_REG_DAY] & 0x3f);

		/* tm->tm_mon is zero-based */
		tm->tm_mon = BCD2BIN(buf[RS5C372_REG_MONTH] & 0x1f) - 1;

		/* year is 1900 + tm->tm_year */
		tm->tm_year = BCD2BIN(buf[RS5C372_REG_YEAR]) + 100;
  
		dev_dbg(&client->dev, "%s rs5c372_get_datetime\n",__FUNCTION__);
		dev_dbg(&client->dev, "sec:%d\n",tm->tm_sec);
		dev_dbg(&client->dev, "min:%d\n",tm->tm_min);
		dev_dbg(&client->dev, "hour:%d\n",tm->tm_hour);
		dev_dbg(&client->dev, "wday:%d\n",tm->tm_wday);
		dev_dbg(&client->dev, "mday:%d\n",tm->tm_mday);
		dev_dbg(&client->dev, "mon:%d\n",tm->tm_mon);
		dev_dbg(&client->dev, "year:%d\n",tm->tm_year);
	}
	  

   if (iret != 0) dev_err(&client->dev, "%s: read error\n", __FUNCTION__);
   
   return iret;
}


static int rs5c372_set_datetime(struct i2c_client *client, struct rtc_time *tm)
{
   int iret = -1, iretry = 0;
   unsigned char buf[8] = {0};
   
   struct i2c_msg msgs[] = {
        { client->addr, 0, 1, buf },
		{rs5c372_i2c_client->addr, 0, 8, buf}
   };
	
	dev_dbg(&client->dev, "%s rs5c372_set_datetime\n",__FUNCTION__);
	dev_dbg(&client->dev, "sec:%d\n",tm->tm_sec);
	dev_dbg(&client->dev, "min:%d\n",tm->tm_min);
	dev_dbg(&client->dev, "hour:%d\n",tm->tm_hour);
	dev_dbg(&client->dev, "wday:%d\n",tm->tm_wday);
	dev_dbg(&client->dev, "mday:%d\n",tm->tm_mday);
	dev_dbg(&client->dev, "mon:%d\n",tm->tm_mon);
	dev_dbg(&client->dev, "year:%d\n",tm->tm_year);

   buf[RS5C372_REG_SECS]  = BIN2BCD(tm->tm_sec);
   buf[RS5C372_REG_MINS]  = BIN2BCD(tm->tm_min);
   buf[RS5C372_REG_HOURS] = BIN2BCD(tm->tm_hour);
   buf[RS5C372_REG_WDAY]  = BIN2BCD(tm->tm_wday);
   buf[RS5C372_REG_DAY]   = BIN2BCD(tm->tm_mday);
   buf[RS5C372_REG_MONTH] = BIN2BCD(tm->tm_mon + 1);
   buf[RS5C372_REG_YEAR]  = BIN2BCD(tm->tm_year - 100);

   while( (iret != 0) && (iretry < 3) ) 
   {
		if ((i2c_transfer(client->adapter, msgs, 2)) != 2) iret = -EIO;
		else iret = 0;
		iretry ++;
   }

   if (iret != 0) dev_err(&client->dev, "%s: write error\n", __FUNCTION__);

   return iret;
}



static int rs5c372_command(struct i2c_client *client, unsigned int cmd, void *arg)
{

	switch (cmd) {
	case RS5C372_GETDATETIME:
		return rs5c372_get_datetime(client, arg);
	case RS5C372_SETTIME:
		return rs5c372_set_datetime(client, arg);
	case RS5C372_SETDATETIME:
		return rs5c372_set_datetime(client, arg);
	default:
		return -EINVAL;
	}
	return 0;
}

static int rs5c372_rtc_open(struct inode *inode, struct file *file)
{
	return 0;
}


static int rs5c372_rtc_release(struct inode *inode, struct file *file)
{
	return 0;
}

static int rs5c372_rtc_ioctl(struct inode *inode, struct file *file,
		   unsigned int cmd, unsigned long arg)
{
	unsigned long flags;
	struct rtc_time wtime;
	int status = 0;

	switch (cmd) {
	default:
	case RTC_UIE_ON:
	case RTC_UIE_OFF:
	case RTC_PIE_ON:
	case RTC_PIE_OFF:
	case RTC_AIE_ON:
	case RTC_AIE_OFF:
	case RTC_ALM_SET:
	case RTC_ALM_READ:
	case RTC_IRQP_READ:
	case RTC_IRQP_SET:
	case RTC_EPOCH_SET:
	case RTC_WKALM_SET:
	case RTC_WKALM_RD:
		status = -EINVAL;
		break;
	case RTC_EPOCH_READ:
		return put_user(1970, (unsigned long *)arg);
	case RTC_RD_TIME:
		spin_lock_irqsave(&rs5c372_rtc_lock, flags);
		rs5c372_command(rs5c372_i2c_client, RS5C372_GETDATETIME,
				 &wtime);
		spin_unlock_irqrestore(&rs5c372_rtc_lock, flags);
		if (copy_to_user((void *)arg, &wtime, sizeof(struct rtc_time)))
			status = -EFAULT;
		break;

	case RTC_SET_TIME:
		if (!capable(CAP_SYS_TIME)) {
			status = -EACCES;
			break;
	}

		if (copy_from_user
		    (&wtime, (struct rtc_time *)arg, sizeof(struct rtc_time))) {
			status = -EFAULT;
			break;
		}

		spin_lock_irqsave(&rs5c372_rtc_lock, flags);
		rs5c372_command(rs5c372_i2c_client, RS5C372_SETDATETIME,
				 &wtime);
		spin_unlock_irqrestore(&rs5c372_rtc_lock, flags);
		break;
	}

	return status;
}

static int rs5c372_rtc_write_proc(char *page, char **start, off_t off,
				  int count, int *eof, void *data)
{
	struct rtc_time wtime;
	unsigned long flags;	
	
	wtime.tm_sec  = 1;
	wtime.tm_min  = 1;
	wtime.tm_hour = 1;
	wtime.tm_wday = 5;
	wtime.tm_mday = 1;
	wtime.tm_mon  = 0;
	wtime.tm_year = 110;
	
	spin_lock_irqsave(&rs5c372_rtc_lock, flags);
	rs5c372_command(rs5c372_i2c_client, RS5C372_SETDATETIME,
			 &wtime);
	spin_unlock_irqrestore(&rs5c372_rtc_lock, flags);
	
	return 8;
}


static __init int rs5c372_init(void)
{
	int retval = 0;
	int i;
	struct proc_dir_entry *res=NULL;
	

	normal_addr[0] = slave_address;
	retval = i2c_add_driver(&rs5c372_driver);
	if (retval == 0) {
		res=create_proc_entry(PROC_RS5C372_NAME, 0, 0);
		if (res) {
			res->write_proc=rs5c372_rtc_write_proc;
			res->data=NULL;
		}
		misc_register(&rs5c372_rtc_miscdev);
		printk("I2C: rs5c372 RTC driver successfully loaded\n");
	}

return retval;
}

static __exit void rs5c372_exit(void)
{
	remove_proc_entry(PROC_RS5C372_NAME, NULL);
	misc_deregister(&rs5c372_rtc_miscdev);
	i2c_del_driver(&rs5c372_driver);

}

module_init(rs5c372_init);
module_exit(rs5c372_exit);


MODULE_PARM(slave_address, "i");
MODULE_PARM_DESC(slave_address, "I2C slave address for RS5C372 RTC.");

MODULE_AUTHOR("Pavel Mironchik pmironchik@optifacio.net");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRV_VERSION);
