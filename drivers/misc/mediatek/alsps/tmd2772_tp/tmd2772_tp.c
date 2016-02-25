

#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/irq.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/kobject.h>
#include <linux/earlysuspend.h>
#include <linux/platform_device.h>
#include <asm/atomic.h>

#include <mach/mt_typedefs.h>
#include <mach/mt_gpio.h>
#include <mach/mt_pm_ldo.h>

#define POWER_NONE_MACRO MT65XX_POWER_NONE

#include <linux/hwmsensor.h>
#include <linux/hwmsen_dev.h>
#include <linux/sensors_io.h>
#include <asm/io.h>
#include <cust_eint.h>
#include <cust_alsps.h>
#include "cust_gpio_usage.h"
#include <alsps.h>
static struct tmd2771_priv *tmd2771_obj = NULL;
static struct platform_driver tmd2771_alsps_driver;
/*----------------------------------------------------------------------------*/

#define TMD2771_DEV_NAME     "TMD2771"
/*----------------------------------------------------------------------------*/
#define APS_TAG                  "[ALS/PS] "
#define APS_FUN(f)               printk(KERN_INFO APS_TAG"%s\n", __FUNCTION__)
/********************************************/
 
extern   int FT3407_pls_enable(void);
 
extern  int FT3407_pls_disable(void);

extern  int  get_FT3407_data(void );

extern   int msg22xx_pls_enable(int);

extern  int  get_msg22xx_data(void );

extern int ft6336_pls_enable(void);
 
extern int ft6336_pls_disable(void);

extern int  get_ft6336_data(void );

int tp_vendor_id_ps;

#if defined(MTK_AUTO_DETECT_ALSPS)

extern int hwmsen_alsps_add(struct sensor_init_info* obj);

#endif//#if defined(MTK_AUTO_DETECT_ALSPS)
static int pls_enable(void)
{
	printk("%s\n", __func__);
	if(tp_vendor_id_ps==0) //tp is FT3407;
		return FT3407_pls_enable();
	else if(tp_vendor_id_ps == 1)
		return msg22xx_pls_enable(1);
	else
		return -1;
}

static int pls_disable(void)
{
	printk("%s\n", __func__);
	if(tp_vendor_id_ps==0) //tp is FT3407;
		return FT3407_pls_disable();
	else if(tp_vendor_id_ps == 1)
		return msg22xx_pls_enable(0);
	else
		return -1;
}

/*----------------------------------------------------------------------------*/
struct tmd2771_priv {
    struct alsps_hw  *hw;
    struct i2c_client *client;
    struct work_struct  eint_work;
    struct workqueue_struct * eint_workqueue;

    /*i2c address group*/
    //struct tmd2772_i2c_addr  addr;

    /*misc*/
    u16             als_modulus;
    atomic_t    i2c_retry;
    atomic_t    als_suspend;
    atomic_t    als_debounce;   /*debounce time after enabling als*/
    atomic_t    als_deb_on;     /*indicates if the debounce is on*/
    atomic_t    als_deb_end;    /*the jiffies representing the end of debounce*/
    atomic_t    ps_mask;        /*mask ps: always return far away*/
    atomic_t    ps_debounce;    /*debounce time after enabling ps*/
    atomic_t    ps_deb_on;      /*indicates if the debounce is on*/
    atomic_t    ps_deb_end;     /*the jiffies representing the end of debounce*/
    atomic_t    ps_suspend;


    /*data*/
    u16         als;
    u16          ps;
    u8          _align;
    u16         als_level_num;
    u16         als_value_num;
    u32         als_level[C_CUST_ALS_LEVEL-1];
    u32         als_value[C_CUST_ALS_LEVEL];

    atomic_t    als_cmd_val;    /*the cmd value can't be read, stored in ram*/
    atomic_t    ps_cmd_val;     /*the cmd value can't be read, stored in ram*/
    atomic_t    ps_thd_val_high;     /*the cmd value can't be read, stored in ram*/
    atomic_t    ps_thd_val_low;     /*the cmd value can't be read, stored in ram*/
    ulong       enable;         /*enable mask*/
    ulong       pending_intr;   /*pending interrupt*/

    /*early suspend*/
#if defined(CONFIG_HAS_EARLYSUSPEND)
    struct early_suspend    early_drv;
#endif
};
/*----------------------------------------------------------------------------*/

typedef enum {
    CMC_BIT_ALS    = 1,
    CMC_BIT_PS     = 2,
} CMC_BIT;

static long tmd2772_unlocked_ioctl(struct file *file, unsigned int cmd,
       unsigned long arg)
{
	return 0;
}

static int tmd2772_release(struct inode *inode, struct file *file)
{
        file->private_data = NULL;
        return 0;
}

static int tmd2772_open(struct inode *inode, struct file *file)
{
	return 0;
}
static struct file_operations tmd2772_fops = {
        .owner = THIS_MODULE,
        .open = tmd2772_open,
        .release = tmd2772_release,
        .unlocked_ioctl = tmd2772_unlocked_ioctl,
};

static struct miscdevice tmd2772_device = {
        .minor = MISC_DYNAMIC_MINOR,
        .name = "als_ps_tp",
        .fops = &tmd2772_fops,
};
static int ps_open_report_data(int open)
{
        //should queuq work to report event if  is_report_input_direct=true
        return 0;
}

static int ps_enable_nodata(int en)
{
	int err;
	printk("%s\n", __func__);
	if(en)
	{
		if(err != pls_enable())
		{
			printk("enable ps fail: %d\n", err);
			return -1;
		}
	}
	else
	{
		if(err != pls_disable())
		{
			printk("disable ps fail: %d\n", err);
			return -1;
		}
	}

	return 0;
}
static int ps_set_delay(u64 ns)
{
        return 0;
}
static int ps_get_data(int* value, int* status)
{
	int alsps_value=-1;
	printk("%s\n", __func__);
	if(tp_vendor_id_ps==0) //tp is FT3407;
		alsps_value= get_FT3407_data();
	else if(tp_vendor_id_ps==1)
		alsps_value= get_msg22xx_data();
	if(alsps_value<0)
		return 1 ;//1 is far;
	else
		return alsps_value;
}


/*----------------------------------------------------------------------------*/
static int tmd2772_for_auto_local_init(void)
{	
	struct tmd2771_priv *obj;
	struct hwmsen_object obj_ps, obj_als;
	int err = 0;
	struct ps_control_path ps_ctl={0};
	struct ps_data_path ps_data={0};

	printk("hugo %s\n", __func__);
	if(!(obj = kzalloc(sizeof(*obj), GFP_KERNEL)))
	{
		err = -ENOMEM;
		goto exit;
	}
	memset(obj, 0, sizeof(*obj));
	tmd2771_obj = obj;

	//obj->hw = get_cust_alsps_hw();
	//tmd2772_get_addr(obj->hw, &obj->addr);

         if(err = misc_register(&tmd2772_device))
         {
                 printk("tmd2772_device register failed\n");
                 goto exit_misc_device_register_failed;
         }

	obj_ps.self = tmd2771_obj;
	/*for interrup work mode support -- by liaoxl.lenovo 12.08.2011*/
        ps_ctl.open_report_data= ps_open_report_data;
        ps_ctl.enable_nodata = ps_enable_nodata;
        ps_ctl.set_delay  = ps_set_delay;
        ps_ctl.is_report_input_direct = true;
#ifdef CUSTOM_KERNEL_SENSORHUB
        ps_ctl.is_support_batch = obj->hw->is_batch_supported_ps;
#else
	ps_ctl.is_support_batch = false;
#endif
        err = ps_register_control_path(&ps_ctl);
        if(err)
        {
                printk("register fail = %d\n", err);
                goto exit_sensor_obj_attach_fail;
        }
	
         ps_data.get_data = ps_get_data;
         ps_data.vender_div = 100;
         err = ps_register_data_path(&ps_data);
         if(err)
         {
                 printk("tregister fail = %d\n", err);
                 goto exit_sensor_obj_attach_fail;
         }

         err = batch_register_support_info(ID_PROXIMITY,ps_ctl.is_support_batch, 100, 0);
         if(err)
         {
                 printk("register proximity batch support err = %d\n", err);
         }

        printk("%s: OK\n", __func__);
	obj_ps.polling = 0; //llf change to interruption mode;

	return 0;
        exit_sensor_obj_attach_fail:
	exit_misc_device_register_failed:
		misc_deregister(&tmd2772_device);
		kfree(obj);
exit :
	return -1;
}
/*----------------------------------------------------------------------------*/

static int tmd2771_remove(struct platform_device *pdev)
{
	APS_FUN();    
	return 0;
}

static struct sensor_init_info tmd2772_init_info = {
		.name = "tmd2772_tp",
		.init = tmd2772_for_auto_local_init,
		.uninit = tmd2771_remove,
	
};
/*----------------------------------------------------------------------------*/
static int __init tmd2771_init(void)
{
	APS_FUN();
	alsps_driver_add(&tmd2772_init_info);
	return 0;
}
/*----------------------------------------------------------------------------*/

static void __exit tmd2771_exit(void)

{
	APS_FUN();
#if defined(MTK_AUTO_DETECT_ALSPS)
#else
	platform_driver_unregister(&tmd2771_alsps_driver);
#endif
}
/*----------------------------------------------------------------------------*/
module_init(tmd2771_init);
module_exit(tmd2771_exit);
/*----------------------------------------------------------------------------*/
MODULE_AUTHOR("wenli.wang");
MODULE_DESCRIPTION("tmd2772_tp driver");
MODULE_LICENSE("GPL");
