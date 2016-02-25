#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/atomic.h>
#include <linux/xlog.h>

#include "kd_camera_hw.h"

#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_camera_feature.h"

/******************************************************************************
 * Debug configuration
******************************************************************************/
#define PFX "[kd_camera_hw]"
#define PK_DBG_NONE(fmt, arg...)    do {} while (0)
#define PK_DBG_FUNC(fmt, args...)    pr_debug(PFX  fmt, ##args)

#define DEBUG_CAMERA_HW_K
#ifdef DEBUG_CAMERA_HW_K
#define PK_DBG PK_DBG_FUNC
#define PK_ERR(fmt, arg...)   pr_err(fmt, ##arg)
#define PK_XLOG_INFO(fmt, args...) \
                do {    \
                   pr_debug(PFX  fmt, ##args); \
                } while(0)
#else
#define PK_DBG(a,...)
#define PK_ERR(a,...)
#define PK_XLOG_INFO(fmt, args...)
#endif

/*
#ifndef BOOL
typedef unsigned char BOOL;
#endif
*/

/* Mark: need to verify whether ISP_MCLK1_EN is required in here //Jessy @2014/06/04*/
extern void ISP_MCLK1_EN(BOOL En);

int cntVCAMD =0;
int cntVCAMA =0;
int cntVCAMIO =0;
int cntVCAMAF =0;
int cntVCAMD_SUB =0;
char Back_Camera[100];
static DEFINE_SPINLOCK(kdsensor_pw_cnt_lock);


bool _hwPowerOn(MT65XX_POWER powerId, int powerVolt, char *mode_name){

	if( hwPowerOn( powerId,  powerVolt, mode_name))
	{
	    spin_lock(&kdsensor_pw_cnt_lock);
		if(powerId==CAMERA_POWER_VCAM_D)
			cntVCAMD+= 1;
		else if(powerId==CAMERA_POWER_VCAM_A)
			cntVCAMA+= 1;
		else if(powerId==CAMERA_POWER_VCAM_IO)
			cntVCAMIO+= 1;
		else if(powerId==CAMERA_POWER_VCAM_AF)
			cntVCAMAF+= 1;
		else if(powerId==SUB_CAMERA_POWER_VCAM_D)
			cntVCAMD_SUB+= 1;
		spin_unlock(&kdsensor_pw_cnt_lock);
		return true;
	}
	return false;
}

bool _hwPowerDown(MT65XX_POWER powerId, char *mode_name){

	if( hwPowerDown( powerId, mode_name))
	{
	    spin_lock(&kdsensor_pw_cnt_lock);
		if(powerId==CAMERA_POWER_VCAM_D)
			cntVCAMD-= 1;
		else if(powerId==CAMERA_POWER_VCAM_A)
			cntVCAMA-= 1;
		else if(powerId==CAMERA_POWER_VCAM_IO)
			cntVCAMIO-= 1;
		else if(powerId==CAMERA_POWER_VCAM_AF)
			cntVCAMAF-= 1;
		else if(powerId==SUB_CAMERA_POWER_VCAM_D)
			cntVCAMD_SUB-= 1;
		spin_unlock(&kdsensor_pw_cnt_lock);
		return true;
	}
	return false;
}

void checkPowerBeforClose( char* mode_name)
{

	int i= 0;

	PK_DBG("[checkPowerBeforClose]cntVCAMD:%d, cntVCAMA:%d,cntVCAMIO:%d, cntVCAMAF:%d, cntVCAMD_SUB:%d,\n",
		cntVCAMD, cntVCAMA,cntVCAMIO,cntVCAMAF,cntVCAMD_SUB);


	for(i=0;i<cntVCAMD;i++)
		hwPowerDown(CAMERA_POWER_VCAM_D,mode_name);
	for(i=0;i<cntVCAMA;i++)
		hwPowerDown(CAMERA_POWER_VCAM_A,mode_name);
	for(i=0;i<cntVCAMIO;i++)
		hwPowerDown(CAMERA_POWER_VCAM_IO,mode_name);
	for(i=0;i<cntVCAMAF;i++)
		hwPowerDown(CAMERA_POWER_VCAM_AF,mode_name);
	for(i=0;i<cntVCAMD_SUB;i++)
		hwPowerDown(SUB_CAMERA_POWER_VCAM_D,mode_name);

	 cntVCAMD =0;
	 cntVCAMA =0;
	 cntVCAMIO =0;
	 cntVCAMAF =0;
	 cntVCAMD_SUB =0;

}



int kdCISModulePowerOn(CAMERA_DUAL_CAMERA_SENSOR_ENUM SensorIdx, char *currSensorName, BOOL On, char* mode_name)
{

u32 pinSetIdx = 0;//default main sensor

#define IDX_PS_CMRST 0
#define IDX_PS_CMPDN 4
#define IDX_PS_MODE 1
#define IDX_PS_ON   2
#define IDX_PS_OFF  3


u32 pinSet[3][8] = {
                        //for main sensor
                     {  CAMERA_CMRST_PIN, // The reset pin of main sensor uses GPIO10 of mt6306, please call mt6306 API to set
                        CAMERA_CMRST_PIN_M_GPIO,   /* mode */
                        GPIO_OUT_ONE,              /* ON state */
                        GPIO_OUT_ZERO,             /* OFF state */
                        CAMERA_CMPDN_PIN,
                        CAMERA_CMPDN_PIN_M_GPIO,
                        GPIO_OUT_ONE,
                        GPIO_OUT_ZERO,
                     },
                     //for sub sensor
                     {  CAMERA_CMRST1_PIN,
                        CAMERA_CMRST1_PIN_M_GPIO,
                        GPIO_OUT_ONE,
                        GPIO_OUT_ZERO,
                        CAMERA_CMPDN1_PIN,
                        CAMERA_CMPDN1_PIN_M_GPIO,
                        GPIO_OUT_ONE,
                        GPIO_OUT_ZERO,
                     },
                     //for main_2 sensor
                     {  GPIO_CAMERA_INVALID,
                        GPIO_CAMERA_INVALID,   /* mode */
                        GPIO_OUT_ONE,               /* ON state */
                        GPIO_OUT_ZERO,              /* OFF state */
                        GPIO_CAMERA_INVALID,
                        GPIO_CAMERA_INVALID,
                        GPIO_OUT_ONE,
                        GPIO_OUT_ZERO,
                     }
                   };



    if (DUAL_CAMERA_MAIN_SENSOR == SensorIdx){
        pinSetIdx = 0;
    }
    else if (DUAL_CAMERA_SUB_SENSOR == SensorIdx) {
        pinSetIdx = 1;
    }
    else if (DUAL_CAMERA_MAIN_2_SENSOR == SensorIdx) {
        pinSetIdx = 2;
    }


    //power ON
    if (On) {

            ISP_MCLK1_EN(1);

        PK_DBG("[PowerON]pinSetIdx:%d, currSensorName: %s\n", pinSetIdx, currSensorName);

        if ((currSensorName && (0 == strcmp(currSensorName,"imx135mipiraw")))||
            (currSensorName && (0 == strcmp(currSensorName,"imx220mipiraw"))))
        {
            //First Power Pin low and Reset Pin Low
            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMPDN]) {
                if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! (CMPDN)\n");}
                if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! (CMPDN)\n");}
                if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! (CMPDN)\n");}
            }


            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST]) {
                if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! (CMRST)\n");}
                if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! (CMRST)\n");}
                if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! (CMRST)\n");}
            }

            //AF_VCC
            if(TRUE != _hwPowerOn(CAMERA_POWER_VCAM_AF, VOL_2800,mode_name))
            {
                PK_DBG("[CAMERA SENSOR] Fail to enable analog power (VCAM_AF), power id = %d \n", CAMERA_POWER_VCAM_AF);
                goto _kdCISModulePowerOn_exit_;
            }

            mdelay(1);

            //VCAM_A
            if(TRUE != _hwPowerOn(CAMERA_POWER_VCAM_A, VOL_2800,mode_name))
            {
                PK_DBG("[CAMERA SENSOR] Fail to enable analog power (VCAM_A), power id = %d\n", CAMERA_POWER_VCAM_A);
                goto _kdCISModulePowerOn_exit_;
            }

            mdelay(1);

            if(TRUE != _hwPowerOn(CAMERA_POWER_VCAM_D, VOL_1000,mode_name))
            {
                 PK_DBG("[CAMERA SENSOR] Fail to enable digital power (VCAM_D), power id = %d \n", CAMERA_POWER_VCAM_D);
                 goto _kdCISModulePowerOn_exit_;
            }

            mdelay(1);

            //VCAM_IO
            if(TRUE != _hwPowerOn(CAMERA_POWER_VCAM_IO, VOL_1800, mode_name))
            {
                PK_DBG("[CAMERA SENSOR] Fail to enable digital power (VCAM_IO), power id = %d \n", CAMERA_POWER_VCAM_IO);
                goto _kdCISModulePowerOn_exit_;
            }

            mdelay(2);


            //enable active sensor
            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST]) {
                if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! (CMRST)\n");}
                if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! (CMRST)\n");}
                if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_ON])){PK_DBG("[CAMERA SENSOR] set gpio failed!! (CMRST)\n");}
            }

            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMPDN]) {
                if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_ON])){PK_DBG("[CAMERA LENS] set gpio failed!! (CMPDN)\n");}
            }

        }
        else if (currSensorName && (0 == strcmp(SENSOR_DRVNAME_OV5648_MIPI_RAW, currSensorName)))
        {
            mt_set_gpio_mode(GPIO_SPI_MOSI_PIN,GPIO_MODE_00);
            mt_set_gpio_dir(GPIO_SPI_MOSI_PIN,GPIO_DIR_OUT);
            mt_set_gpio_out(GPIO_SPI_MOSI_PIN,GPIO_OUT_ONE);
            //First Power Pin low and Reset Pin Low
            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMPDN]) {
                if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! (CMPDN)\n");}
                if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! (CMPDN)\n");}
                if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! (CMPDN)\n");}
            }

            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST]) {
                if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! (CMRST)\n");}
                if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! (CMRST)\n");}
                if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! (CMRST)\n");}
            }


            //VCAM_IO
            if(TRUE != _hwPowerOn(CAMERA_POWER_VCAM_IO, VOL_1800, mode_name))
            {
                PK_DBG("[CAMERA SENSOR] Fail to enable digital power (VCAM_IO), power id = %d \n", CAMERA_POWER_VCAM_IO);
                goto _kdCISModulePowerOn_exit_;
            }

            mdelay(1);

            //VCAM_A
            if(TRUE != _hwPowerOn(CAMERA_POWER_VCAM_A, VOL_2800,mode_name))
            {
                PK_DBG("[CAMERA SENSOR] Fail to enable analog power (VCAM_A), power id = %d\n", CAMERA_POWER_VCAM_A);
                goto _kdCISModulePowerOn_exit_;
            }

            mdelay(1);

            if(TRUE != _hwPowerOn(SUB_CAMERA_POWER_VCAM_D, VOL_1500,mode_name))
            {
                 PK_DBG("[CAMERA SENSOR] Fail to enable digital power (VCAM_D), power id = %d \n", SUB_CAMERA_POWER_VCAM_D);
                 goto _kdCISModulePowerOn_exit_;
            }

            mdelay(5);

            //AF_VCC
            if(TRUE != _hwPowerOn(CAMERA_POWER_VCAM_AF, VOL_2800,mode_name))
            {
                PK_DBG("[CAMERA SENSOR] Fail to enable analog power (VCAM_AF), power id = %d \n", CAMERA_POWER_VCAM_AF);
                goto _kdCISModulePowerOn_exit_;
            }


            mdelay(1);


            //enable active sensor
            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMPDN]) {
                if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! (CMPDN)\n");}
                if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! (CMPDN)\n");}
                if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_ON])){PK_DBG("[CAMERA LENS] set gpio failed!! (CMPDN)\n");}
            }


            mdelay(2);


            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST]) {
                if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! (CMRST)\n");}
                if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! (CMRST)\n");}
                if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_ON])){PK_DBG("[CAMERA SENSOR] set gpio failed!! (CMRST)\n");}

            }

            mdelay(20);
        }
        else  if (currSensorName && (0 == strcmp(SENSOR_DRVNAME_GC2355_MIPI_RAW, currSensorName)))
        {
            mt_set_gpio_mode(GPIO_SPI_MOSI_PIN,GPIO_MODE_00);
            mt_set_gpio_dir(GPIO_SPI_MOSI_PIN,GPIO_DIR_OUT);
            mt_set_gpio_out(GPIO_SPI_MOSI_PIN,GPIO_OUT_ONE);
            //First Power Pin low and Reset Pin Low
            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMPDN]) {
                if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! (CMPDN)\n");}
                if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! (CMPDN)\n");}
                if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! (CMPDN)\n");}
            }

            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST]) {
                if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! (CMRST)\n");}
                if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! (CMRST)\n");}
                if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! (CMRST)\n");}
            }

            mdelay(50);

            //VCAM_A
            if(TRUE != _hwPowerOn(CAMERA_POWER_VCAM_A, VOL_2800,mode_name))
            {
                PK_DBG("[CAMERA SENSOR] Fail to enable analog power (VCAM_A), power id = %d\n", CAMERA_POWER_VCAM_A);
                goto _kdCISModulePowerOn_exit_;
            }

            mdelay(10);

            //VCAM_IO
            if(TRUE != _hwPowerOn(CAMERA_POWER_VCAM_IO, VOL_1800, mode_name))
            {
                PK_DBG("[CAMERA SENSOR] Fail to enable digital power (VCAM_IO), power id = %d \n", CAMERA_POWER_VCAM_IO);
                goto _kdCISModulePowerOn_exit_;
            }

            mdelay(10);

            if(TRUE != _hwPowerOn(SUB_CAMERA_POWER_VCAM_D, VOL_1500,mode_name))
            {
                 PK_DBG("[CAMERA SENSOR] Fail to enable digital power (VCAM_D), power id = %d \n", SUB_CAMERA_POWER_VCAM_D);
                 goto _kdCISModulePowerOn_exit_;
            }

            mdelay(10);

            //AF_VCC
            if(TRUE != _hwPowerOn(CAMERA_POWER_VCAM_AF, VOL_2800,mode_name))
            {
                PK_DBG("[CAMERA SENSOR] Fail to enable analog power (VCAM_AF), power id = %d \n", CAMERA_POWER_VCAM_AF);
                goto _kdCISModulePowerOn_exit_;
            }


            mdelay(50);

            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST]) {
                if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! (CMRST)\n");}
                if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! (CMRST)\n");}
                if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! (CMRST)\n");}
                mdelay(5);
                if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_ON])){PK_DBG("[CAMERA SENSOR] set gpio failed!! (CMRST)\n");}

            }
            mdelay(5);
            //enable active sensor
            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMPDN]) {
                if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! (CMPDN)\n");}
                if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! (CMPDN)\n");}
                if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_ON])){PK_DBG("[CAMERA LENS] set gpio failed!! (CMPDN)\n");}
                mdelay(5);
                if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! (CMPDN)\n");}
            }

            mdelay(5);
        }
        else  if ((pinSetIdx == 0)&&(currSensorName && (0 == strcmp(SENSOR_DRVNAME_OV5670_MIPI_RAW, currSensorName))))
	{
		printk("TCL: aad pinSetIdx kdCISModulePower--On get in---SENSOR_DRVNAME_OV5670_MIPI_RAW \n");
		//disable inactive sensor
		if(pinSetIdx == 0 || pinSetIdx == 2) {//disable sub
			if (GPIO_CAMERA_INVALID != pinSet[1][IDX_PS_CMRST]) {
				if(mt_set_gpio_mode(pinSet[1][IDX_PS_CMRST],pinSet[1][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
				if(mt_set_gpio_mode(pinSet[1][IDX_PS_CMPDN],pinSet[1][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
				if(mt_set_gpio_dir(pinSet[1][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
				if(mt_set_gpio_dir(pinSet[1][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
				if(mt_set_gpio_out(pinSet[1][IDX_PS_CMRST],pinSet[1][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");} //low == reset sensor

				if(0 == strcmp(Back_Camera,"ov2686"))
	   		       {
	   			   printk("TCL:back_camera=%s\n",Back_Camera);
				   if(mt_set_gpio_out(pinSet[1][IDX_PS_CMPDN],pinSet[1][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");} //high == power down lens module
	   		       }
	   		       else
	   		      {
	   			   printk("TCL:back_camera is not %s\n",Back_Camera);
				  if(mt_set_gpio_out(pinSet[1][IDX_PS_CMPDN],pinSet[1][IDX_PS_CMPDN+IDX_PS_ON])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");} //high == power down lens module
	   
	   		       }
			}
		}

	        //DOVDD
	        PK_DBG("[ON_general 1.8V]sensorIdx:%d \n",SensorIdx);
	        if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_IO, VOL_1800,mode_name))
	        {
	            PK_DBG("[CAMERA SENSOR] Fail to enable digital power\n");
	            //return -EIO;
	            //goto _kdCISModulePowerOn_exit_;
	        }

		      //PDN pin
	        if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
	        if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
	        if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_ON])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}
	        mdelay(2);

	        //AVDD
	        if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_A, VOL_2800,mode_name))
	        {
	            PK_DBG("[CAMERA SENSOR] Fail to enable analog power\n");
	            //return -EIO;
	            //goto _kdCISModulePowerOn_exit_;
	        }
	        mdelay(2);

	        //DVDD
	        if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_D, VOL_1200,mode_name))
	        {
	             PK_DBG("[CAMERA SENSOR] Fail to enable digital power\n");
	             //return -EIO;
	             //goto _kdCISModulePowerOn_exit_;
	        }
	        mdelay(5);//5ms
	        

	        #if 1
	        //enable active sensor
	        if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST]) {
	           
	            mdelay(2);

		    //RST pin
	            if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
	            if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
	            if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");}
	            mdelay(10);
	            if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_ON])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");}
	            mdelay(1);
	        }
	        #endif
		}

        else  if ((pinSetIdx == 0)&&(currSensorName && (0 == strcmp(SENSOR_DRVNAME_OV5670AF_MIPI_RAW, currSensorName))))
        {
		PK_DBG("kdCISModulePower--On get in---SENSOR_DRVNAME_OV5670_MIPI_RAW \n");
		//disable inactive sensor
		if(pinSetIdx == 0 || pinSetIdx == 2) {//disable sub
			if (GPIO_CAMERA_INVALID != pinSet[1][IDX_PS_CMRST]) {
				if(mt_set_gpio_mode(pinSet[1][IDX_PS_CMRST],pinSet[1][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
				if(mt_set_gpio_mode(pinSet[1][IDX_PS_CMPDN],pinSet[1][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
				if(mt_set_gpio_dir(pinSet[1][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
				if(mt_set_gpio_dir(pinSet[1][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
				if(mt_set_gpio_out(pinSet[1][IDX_PS_CMRST],pinSet[1][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");} //low == reset sensor
			      if(0 == strcmp(Back_Camera,"ov2686"))
	   		       {
	   			   printk("TCL:back_camera=%s\n",Back_Camera);
				   if(mt_set_gpio_out(pinSet[1][IDX_PS_CMPDN],pinSet[1][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");} //high == power down lens module
	   		       }
	   		       else
	   		      {
	   			   printk("TCL:back_camera is not %s\n",Back_Camera);
				  if(mt_set_gpio_out(pinSet[1][IDX_PS_CMPDN],pinSet[1][IDX_PS_CMPDN+IDX_PS_ON])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");} //high == power down lens module
	   
	   		       }
			}
		}

	        //DOVDD
	        PK_DBG("[ON_general 1.8V]sensorIdx:%d \n",SensorIdx);
	        if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_IO, VOL_1800,mode_name))
	        {
	            PK_DBG("[CAMERA SENSOR] Fail to enable digital power\n");
	            //return -EIO;
	            //goto _kdCISModulePowerOn_exit_;
	        }
		 //PDN pin
	        if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
	        if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
	        if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_ON])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}
	        mdelay(2);

	        //AVDD
	        if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_A, VOL_2800,mode_name))
	        {
	            PK_DBG("[CAMERA SENSOR] Fail to enable analog power\n");
	            //return -EIO;
	            //goto _kdCISModulePowerOn_exit_;
	        }
	        mdelay(2);

	        //DVDD
	        if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_D, VOL_1200,mode_name))
	        {
	             PK_DBG("[CAMERA SENSOR] Fail to enable digital power\n");
	             //return -EIO;
	             //goto _kdCISModulePowerOn_exit_;
	        }
	        mdelay(5);//5ms

		       //AF_VCC
                if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_AF, VOL_2800,mode_name))
                {
               		PK_DBG("[CAMERA SENSOR] Fail to enable analog power (VCAM_AF), power id = %d \n", CAMERA_POWER_VCAM_AF);
               		//goto _kdCISModulePowerOn_exit_;
                }


                mdelay(10);
		
			/*Defect 437608 modify by yinzhanyong at 20150721 begin*/
	            //AF enable pin
	        if(mt_set_gpio_mode(GPIO_CAMERA_AF_EN_PIN,GPIO_MODE_00)){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
	        if(mt_set_gpio_dir(GPIO_CAMERA_AF_EN_PIN,GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
	        if(mt_set_gpio_out(GPIO_CAMERA_AF_EN_PIN,GPIO_OUT_ONE)){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}
	        mdelay(10);
                printk("enable AF PIN\n");
			/*Defect 437608 modify by yinzhanyong at 20150721 end*/

	        #if 1
	        //enable active sensor
	        if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST]) {

	            mdelay(2);

		    //RST pin
	            if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
	            if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
	            if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");}
	            mdelay(10);
	            if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_ON])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");}
	            mdelay(1);
	        }
	        #endif
		}


	 else  if ((pinSetIdx == 0)&&(currSensorName && (0 == strcmp(SENSOR_DRVNAME_OV8858_MIPI_RAW, currSensorName))))
	{
		PK_DBG("kdCISModulePower--On get in---SENSOR_DRVNAME_OV8858_MIPI_RAW \n");
		//disable inactive sensor
		if(pinSetIdx == 0 || pinSetIdx == 2) {//disable sub
			if (GPIO_CAMERA_INVALID != pinSet[1][IDX_PS_CMRST]) {
				if(mt_set_gpio_mode(pinSet[1][IDX_PS_CMRST],pinSet[1][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
				if(mt_set_gpio_mode(pinSet[1][IDX_PS_CMPDN],pinSet[1][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
				if(mt_set_gpio_dir(pinSet[1][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
				if(mt_set_gpio_dir(pinSet[1][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
				if(mt_set_gpio_out(pinSet[1][IDX_PS_CMRST],pinSet[1][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");} //low == reset sensor
				if(0 == strcmp(Back_Camera,"ov2686"))
	   		       {
	   			   printk("TCL:back_camera=%s\n",Back_Camera);
				   if(mt_set_gpio_out(pinSet[1][IDX_PS_CMPDN],pinSet[1][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");} //high == power down lens module
	   		       }
	   		       else
	   		      {
	   			   printk("TCL:back_camera is not %s\n",Back_Camera);
				  if(mt_set_gpio_out(pinSet[1][IDX_PS_CMPDN],pinSet[1][IDX_PS_CMPDN+IDX_PS_ON])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");} //high == power down lens module
	   
	   		       }
			}
		}

	        if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST]) {
	           
	            //PDN pin
	            if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
	            if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
	            if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}
	            mdelay(2);

		    //RST pin
	            if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
	            if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
	            if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");}
	            mdelay(10);
	   
	        }
	    
		       //AVDD
	        if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_A, VOL_2800,mode_name))
	        {
	            PK_DBG("[CAMERA SENSOR] Fail to enable analog power\n");
	            //return -EIO;
	            //goto _kdCISModulePowerOn_exit_;
	        }
	        mdelay(2);

	        //DOVDD
	        PK_DBG("[ON_general 1.8V]sensorIdx:%d \n",SensorIdx);
	        if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_IO, VOL_1800,mode_name))
	        {
	            PK_DBG("[CAMERA SENSOR] Fail to enable digital power\n");
	            //return -EIO;
	            //goto _kdCISModulePowerOn_exit_;
	        }
	        mdelay(2);

	 

	        //DVDD
	        if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_D, VOL_1200,mode_name))
	        {
	             PK_DBG("[CAMERA SENSOR] Fail to enable digital power\n");
	             //return -EIO;
	             //goto _kdCISModulePowerOn_exit_;
	        }
	        mdelay(5);//5ms
	           //AF_VCC
                if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_AF, VOL_2800,mode_name))
                {
               		PK_DBG("[CAMERA SENSOR] Fail to enable analog power (VCAM_AF), power id = %d \n", CAMERA_POWER_VCAM_AF);
               		//goto _kdCISModulePowerOn_exit_;
                }


                mdelay(10);
		
			/*Defect 437608 modify by yinzhanyong at 20150721 begin*/
	            //AF enable pin
	        if(mt_set_gpio_mode(GPIO_CAMERA_AF_EN_PIN,GPIO_MODE_00)){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
	        if(mt_set_gpio_dir(GPIO_CAMERA_AF_EN_PIN,GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
	        if(mt_set_gpio_out(GPIO_CAMERA_AF_EN_PIN,GPIO_OUT_ONE)){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}
	        mdelay(10);
                printk("enable AF PIN\n");
			/*Defect 437608 modify by yinzhanyong at 20150721 end*/

	        #if 1
	        //enable active sensor
	        if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST]) {
	           
	            //PDN pin
	            if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
	            if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
	            if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_ON])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}
	            mdelay(10);

		    //RST pin
	            if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
	            if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
	            if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");}
	            mdelay(10);
	            if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_ON])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");}
	            mdelay(1);
	        }
	        #endif
		}
	 
	 else  if ((pinSetIdx == 0)&&(currSensorName && (0 == strcmp(SENSOR_DRVNAME_S5K5E2YA_MIPI_RAW, currSensorName))))
	 {
		PK_DBG("kdCISModulePower--On get in---SENSOR_DRVNAME_S5K5E2YA_MIPI_RAW \n");
		//disable inactive sensor
		if(pinSetIdx == 0 || pinSetIdx == 2) {//disable sub
			if (GPIO_CAMERA_INVALID != pinSet[1][IDX_PS_CMRST]) {
				if(mt_set_gpio_mode(pinSet[1][IDX_PS_CMRST],pinSet[1][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
				if(mt_set_gpio_mode(pinSet[1][IDX_PS_CMPDN],pinSet[1][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
				if(mt_set_gpio_dir(pinSet[1][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
				if(mt_set_gpio_dir(pinSet[1][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
				if(mt_set_gpio_out(pinSet[1][IDX_PS_CMRST],pinSet[1][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");} //low == reset sensor
				if(0 == strcmp(Back_Camera,"ov2686"))
	   		       {
	   			   printk("TCL:back_camera=%s\n",Back_Camera);
				   if(mt_set_gpio_out(pinSet[1][IDX_PS_CMPDN],pinSet[1][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");} //high == power down lens module
	   		       }
	   		       else
	   		       {
	   			   printk("TCL:back_camera is not %s\n",Back_Camera);
				  if(mt_set_gpio_out(pinSet[1][IDX_PS_CMPDN],pinSet[1][IDX_PS_CMPDN+IDX_PS_ON])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");} //high == power down lens module
	   
	   		       }
			 }
		}



		//DVDD
		  if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_D, VOL_1200,mode_name))
		  {
			   PK_DBG("[CAMERA SENSOR] Fail to enable digital power\n");
			   //return -EIO;
			   //goto _kdCISModulePowerOn_exit_;
		  }

	       
	        mdelay(2);

	        //AVDD
	        if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_A, VOL_2800,mode_name))
	        {
	            PK_DBG("[CAMERA SENSOR] Fail to enable analog power\n");
	            //return -EIO;
	            //goto _kdCISModulePowerOn_exit_;
	        }
	        mdelay(2);
			
	//DOVDD
		   PK_DBG("[ON_general 1.8V]sensorIdx:%d \n",SensorIdx);
		   if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_IO, VOL_1800,mode_name))
		   {
			   PK_DBG("[CAMERA SENSOR] Fail to enable digital power\n");
			   //return -EIO;
			   //goto _kdCISModulePowerOn_exit_;
		   }

	  
	        mdelay(5);//5ms
		
		
		         
	        //enable active sensor
	        if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST]) {
	           
	            //PDN pin
	            if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
	            if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
	            if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_ON])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}
	            mdelay(2);

		    //RST pin
	            if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
	            if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
	            if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");}
	            mdelay(10);
	            if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_ON])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");}
	            mdelay(1);

	        }
	   
	}

	 else  if ((pinSetIdx == 0)&&(currSensorName && (0 == strcmp(SENSOR_DRVNAME_S5K5E2YAAF_MIPI_RAW, currSensorName))))	
	{
		PK_DBG("kdCISModulePower--On get in---SENSOR_DRVNAME_S5K5E2YA AF_MIPI_RAW \n");
		//disable inactive sensor
		if(pinSetIdx == 0 || pinSetIdx == 2) {//disable sub
			if (GPIO_CAMERA_INVALID != pinSet[1][IDX_PS_CMRST]) {
				if(mt_set_gpio_mode(pinSet[1][IDX_PS_CMRST],pinSet[1][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
				if(mt_set_gpio_mode(pinSet[1][IDX_PS_CMPDN],pinSet[1][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
				if(mt_set_gpio_dir(pinSet[1][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
				if(mt_set_gpio_dir(pinSet[1][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
				if(mt_set_gpio_out(pinSet[1][IDX_PS_CMRST],pinSet[1][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");} //low == reset sensor
				if(0 == strcmp(Back_Camera,"ov2686"))
	   		       {
	   			   printk("TCL:back_camera=%s\n",Back_Camera);
				   if(mt_set_gpio_out(pinSet[1][IDX_PS_CMPDN],pinSet[1][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");} //high == power down lens module
	   		       }
	   		       else
	   		      {
	   			   printk("TCL:back_camera is not %s\n",Back_Camera);
				  if(mt_set_gpio_out(pinSet[1][IDX_PS_CMPDN],pinSet[1][IDX_PS_CMPDN+IDX_PS_ON])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");} //high == power down lens module
	   
	   		      }
			}
		}



		//DVDD
		  if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_D, VOL_1200,mode_name))
		  {
			   PK_DBG("[CAMERA SENSOR] Fail to enable digital power\n");
			   //return -EIO;
			   //goto _kdCISModulePowerOn_exit_;
		  }

	       
	        mdelay(2);

	        //AVDD
	        if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_A, VOL_2800,mode_name))
	        {
	            PK_DBG("[CAMERA SENSOR] Fail to enable analog power\n");
	            //return -EIO;
	            //goto _kdCISModulePowerOn_exit_;
	        }
	        mdelay(2);
			
	//DOVDD
		   PK_DBG("[ON_general 1.8V]sensorIdx:%d \n",SensorIdx);
		   if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_IO, VOL_1800,mode_name))
		   {
			   PK_DBG("[CAMERA SENSOR] Fail to enable digital power\n");
			   //return -EIO;
			   //goto _kdCISModulePowerOn_exit_;
		   }

	  
	        mdelay(5);//5ms
		        //AF_VCC
                if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_AF, VOL_2800,mode_name))
                {
               		PK_DBG("[CAMERA SENSOR] Fail to enable analog power (VCAM_AF), power id = %d \n", CAMERA_POWER_VCAM_AF);
               		//goto _kdCISModulePowerOn_exit_;
                }


                mdelay(10);
		
			/*Defect 437608 modify by yinzhanyong at 20150721 begin*/
	            //AF enable pin
	        if(mt_set_gpio_mode(GPIO_CAMERA_AF_EN_PIN,GPIO_MODE_00)){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
	        if(mt_set_gpio_dir(GPIO_CAMERA_AF_EN_PIN,GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
	        if(mt_set_gpio_out(GPIO_CAMERA_AF_EN_PIN,GPIO_OUT_ONE)){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}
	        mdelay(10);
                printk("enable AF PIN\n");
			/*Defect 437608 modify by yinzhanyong at 20150721 end*/
		         
	        //enable active sensor
	        if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST]) {
	           
	            //PDN pin
	            if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
	            if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
	            if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_ON])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}
	            mdelay(2);

		    //RST pin
	            if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
	            if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
	            if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");}
	            mdelay(10);
	            if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_ON])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");}
	            mdelay(1);

	        }
	   
	}

	 else  if ((pinSetIdx == 0)&&(currSensorName && (0 == strcmp(SENSOR_DRVNAME_S5K4H5YC_MIPI_RAW, currSensorName))))
	{
		PK_DBG("kdCISModulePower--On get in---SENSOR_DRVNAME_S5K5E2YA_MIPI_RAW \n");
		//disable inactive sensor
		if(pinSetIdx == 0 || pinSetIdx == 2) {//disable sub
			if (GPIO_CAMERA_INVALID != pinSet[1][IDX_PS_CMRST]) {
				if(mt_set_gpio_mode(pinSet[1][IDX_PS_CMRST],pinSet[1][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
				if(mt_set_gpio_mode(pinSet[1][IDX_PS_CMPDN],pinSet[1][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
				if(mt_set_gpio_dir(pinSet[1][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
				if(mt_set_gpio_dir(pinSet[1][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
				if(mt_set_gpio_out(pinSet[1][IDX_PS_CMRST],pinSet[1][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");} //low == reset sensor
				if(0 == strcmp(Back_Camera,"ov2686"))
	   		       {
	   			   printk("TCL:back_camera=%s\n",Back_Camera);
				   if(mt_set_gpio_out(pinSet[1][IDX_PS_CMPDN],pinSet[1][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");} //high == power down lens module
	   		       }
	   		       else
	   		      {
	   			   printk("TCL:back_camera is not %s\n",Back_Camera);
				  if(mt_set_gpio_out(pinSet[1][IDX_PS_CMPDN],pinSet[1][IDX_PS_CMPDN+IDX_PS_ON])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");} //high == power down lens module
	   
	   		       }
			}
		}

		    //enable active sensor
	        if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST]) {
	           
	            //PDN pin
	            if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
	            if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
	            if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}
	            mdelay(2);

	        }



		//DVDD
		  if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_D, VOL_1200,mode_name))
		  {
			   PK_DBG("[CAMERA SENSOR] Fail to enable digital power\n");
			   //return -EIO;
			   //goto _kdCISModulePowerOn_exit_;
		  }

	       
	        mdelay(2);

	        //AVDD
	        if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_A, VOL_2800,mode_name))
	        {
	            PK_DBG("[CAMERA SENSOR] Fail to enable analog power\n");
	            //return -EIO;
	            //goto _kdCISModulePowerOn_exit_;
	        }
	        mdelay(2);
			
	//DOVDD
		   PK_DBG("[ON_general 1.8V]sensorIdx:%d \n",SensorIdx);
		   if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_IO, VOL_1800,mode_name))
		   {
			   PK_DBG("[CAMERA SENSOR] Fail to enable digital power\n");
			   //return -EIO;
			   //goto _kdCISModulePowerOn_exit_;
		   }

	  
	        mdelay(5);//5ms
		        //AF_VCC
                if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_AF, VOL_2800,mode_name))
                {
               		PK_DBG("[CAMERA SENSOR] Fail to enable analog power (VCAM_AF), power id = %d \n", CAMERA_POWER_VCAM_AF);
               		//goto _kdCISModulePowerOn_exit_;
                }


                mdelay(10);
		
			/*Defect 437608 modify by yinzhanyong at 20150721 begin*/
	            //AF enable pin
	        if(mt_set_gpio_mode(GPIO_CAMERA_AF_EN_PIN,GPIO_MODE_00)){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
	        if(mt_set_gpio_dir(GPIO_CAMERA_AF_EN_PIN,GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
	        if(mt_set_gpio_out(GPIO_CAMERA_AF_EN_PIN,GPIO_OUT_ONE)){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}
	        mdelay(10);
                printk("enable AF PIN\n");
			/*Defect 437608 modify by yinzhanyong at 20150721 end*/
		         
	        //enable active sensor
	        if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST]) {
	           
	            //PDN pin
	            if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
	            if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
	            if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_ON])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}
	            mdelay(10);

	        }
	   
	}




	
	 else  if ((pinSetIdx == 1)&&(currSensorName && (0 == strcmp(SENSOR_DRVNAME_GC0329_YUV, currSensorName))))
	{

		//disable inactive sensor

	          if (GPIO_CAMERA_INVALID != pinSet[0][IDX_PS_CMRST]) {
	             if(mt_set_gpio_mode(pinSet[0][IDX_PS_CMRST],pinSet[0][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[cam sensor]set gpio mode failed!! \n");}
	             if(mt_set_gpio_mode(pinSet[0][IDX_PS_CMPDN],pinSet[0][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[cam sensor]set gpio mode failed!! \n");}
	             if(mt_set_gpio_dir(pinSet[0][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
	             if(mt_set_gpio_dir(pinSet[0][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR]set gpio dir failed!! \n");}
	             if(mt_set_gpio_out(pinSet[0][IDX_PS_CMRST],pinSet[0][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");}		         

		    if(mt_set_gpio_out(pinSet[0][IDX_PS_CMPDN],pinSet[0][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR]set gpio failed!! \n");}
	          } 
	        if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST]) {
		     if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
	            if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
	            if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_ON])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}
	            if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
	            if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
		     if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");}    
	        }
		mdelay(2);
	   	PK_DBG("current open camera is gc0329 sensor,pinSetIdx=%d\n\r",pinSetIdx);
       		if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_IO, VOL_1800,mode_name))//IO VDD
		{
			PK_DBG("[CAMERA SENSOR] Fail to enable digital power\n");		            
			goto _kdCISModulePowerOn_exit_;
		} 
		mdelay(2);
		if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_A, VOL_2800,mode_name))//AVDD
		{
		      	PK_DBG("[CAMERA SENSOR] Fail to enable analog power\n");
		      	goto _kdCISModulePowerOn_exit_;
		}
                PK_DBG("TCL: gc0329 sensor,pinSetIdx=%d\n\r",pinSetIdx);
		mdelay(2);

	        if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST]) {
	            if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}
		    mdelay(2);
	            if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_ON])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");}
	            mdelay(2);      
	        }
	  

	 }

	else if ((pinSetIdx == 1)&&(currSensorName && (0 == strcmp(SENSOR_DRVNAME_GC2145_YUV,currSensorName)))){
		
		PK_DBG("\nTCL: Enter gc2145 sensor,pinSetIdx=%d\n\r",pinSetIdx);

				#if 1
				if(pinSetIdx == 1)
				{
				  if (GPIO_CAMERA_INVALID != pinSet[0][IDX_PS_CMRST]) {
					 if(mt_set_gpio_mode(pinSet[0][IDX_PS_CMRST],pinSet[0][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[cam sensor]set gpio mode failed!! \n");}
					 if(mt_set_gpio_mode(pinSet[0][IDX_PS_CMPDN],pinSet[0][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[cam sensor]set gpio mode failed!! \n");}
					 if(mt_set_gpio_dir(pinSet[0][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
					 if(mt_set_gpio_dir(pinSet[0][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR]set gpio dir failed!! \n");}
					 if(mt_set_gpio_out(pinSet[0][IDX_PS_CMRST],pinSet[0][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");}		         
					 if(mt_set_gpio_out(pinSet[0][IDX_PS_CMPDN],pinSet[0][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR]set gpio failed!! \n");}
				  }
				  PK_DBG("\nTCL: Disable  Main Sensor PWDN and CMREST \n\r");
				}
				#endif

		
		if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMPDN]) {
			if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
			if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
			if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_ON])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}
		}

		 if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST]) {
	        if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
	        if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
    		 if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");}    
	        }
		mdelay(2);
		
		PK_DBG("TCL: gc2145 sensor,set pwdn and reset\n\r");

		if(TRUE != hwPowerOn(MT6328_POWER_LDO_VGP1, VOL_1800,mode_name))//DVDD
//		if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_D, VOL_1500,mode_name))//DVDD
		{
			PK_DBG("[CAMERA SENSOR] Fail to enable digital power\n");		      
			goto _kdCISModulePowerOn_exit_;
		}
		mdelay(2);

		if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_IO, VOL_1800,mode_name))//IO VDD
		{
			PK_DBG("[CAMERA SENSOR] Fail to enable digital power\n");		            
			goto _kdCISModulePowerOn_exit_;
		} 
		mdelay(2);
		
		if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_A, VOL_2800,mode_name))//AVDD
		{
		      	PK_DBG("[CAMERA SENSOR] Fail to enable analog power\n");
		      	goto _kdCISModulePowerOn_exit_;
		}

		mdelay(2);
		PK_DBG("TCL: gc2145 sensor,open power \n\r");

		if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMPDN]) {	
//			if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
//			if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
			if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}
			PK_DBG("TCL: IDX_PS_CMPDN+IDX_PS_OFF \n\r");
		}
		mdelay(2);
		if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST]) {
//	      		if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
//	        	if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
    		 	if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_ON])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");}    
				PK_DBG("TCL: IDX_PS_CMRST+IDX_PS_ON \n\r");
			}
		mdelay(2);
			
		PK_DBG("TCL: gc2145 sensor,open down \n\r");

	}
	
	else if ((pinSetIdx == 1)&&(currSensorName && (0 == strcmp(SENSOR_DRVNAME_OV2686_YUV,currSensorName)))){
		
		PK_DBG("\nTCL: Enter OV2686 sensor,pinSetIdx=%d\n\r",pinSetIdx);
				#if 1
				if(pinSetIdx == 1)
				{
				  if (GPIO_CAMERA_INVALID != pinSet[0][IDX_PS_CMRST]) {
					 if(mt_set_gpio_mode(pinSet[0][IDX_PS_CMRST],pinSet[0][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[cam sensor]set gpio mode failed!! \n");}
					 if(mt_set_gpio_mode(pinSet[0][IDX_PS_CMPDN],pinSet[0][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[cam sensor]set gpio mode failed!! \n");}
					 if(mt_set_gpio_dir(pinSet[0][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
					 if(mt_set_gpio_dir(pinSet[0][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR]set gpio dir failed!! \n");}
					 if(mt_set_gpio_out(pinSet[0][IDX_PS_CMRST],pinSet[0][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");}		         
					 if(mt_set_gpio_out(pinSet[0][IDX_PS_CMPDN],pinSet[0][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR]set gpio failed!! \n");}
				  }
				  PK_DBG("\nTCL: Disable  Main Sensor PWDN and CMREST \n\r");
				}
				#endif
				
		if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMPDN]) {
			if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
			if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
			if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}
		}
		
		mdelay(2);

		if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_A, VOL_2800,mode_name))//AVDD
		{
		      	PK_DBG("[CAMERA SENSOR] Fail to enable analog power\n");
		      	goto _kdCISModulePowerOn_exit_;
		}
	       mdelay(2);
	      if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_IO, VOL_1800,mode_name))//IO VDD
		{
			PK_DBG("[CAMERA SENSOR] Fail to enable digital power\n");		            
			goto _kdCISModulePowerOn_exit_;
		} 
		mdelay(2);
		
		if(TRUE != hwPowerOn(MT6328_POWER_LDO_VGP1, VOL_1800,mode_name))//DVDD
		{
			PK_DBG("[CAMERA SENSOR] Fail to enable digital power\n");		      
			goto _kdCISModulePowerOn_exit_;
		}
		msleep(2);
		
		if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMPDN]) {	
//			if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
//			if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
			if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_ON])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}
		}
		mdelay(2);

	}
	

        else
        {
            //First Power Pin low and Reset Pin Low
            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMPDN]) {
                if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! (CMPDN)\n");}
                if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! (CMPDN)\n");}
                if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! (CMPDN)\n");}
            }

            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST]) {
                if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! (CMRST)\n");}
                if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! (CMRST)\n");}
                if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! (CMRST)\n");}
            }

            //VCAM_IO
            if(TRUE != _hwPowerOn(CAMERA_POWER_VCAM_IO, VOL_1800, mode_name))
            {
                PK_DBG("[CAMERA SENSOR] Fail to enable digital power (VCAM_IO), power id = %d \n", CAMERA_POWER_VCAM_IO);
                goto _kdCISModulePowerOn_exit_;
            }

            //VCAM_A
            if(TRUE != _hwPowerOn(CAMERA_POWER_VCAM_A, VOL_2800,mode_name))
            {
                PK_DBG("[CAMERA SENSOR] Fail to enable analog power (VCAM_A), power id = %d\n", CAMERA_POWER_VCAM_A);
                goto _kdCISModulePowerOn_exit_;
            }
            //VCAM_D
            if(currSensorName && (0 == strcmp(SENSOR_DRVNAME_S5K2P8_MIPI_RAW, currSensorName)))
            {
                if(TRUE != _hwPowerOn(CAMERA_POWER_VCAM_D, VOL_1200,mode_name))
                {
                     PK_DBG("[CAMERA SENSOR] Fail to enable digital power\n");
                     goto _kdCISModulePowerOn_exit_;
                }
            }
            else if(currSensorName && (0 == strcmp(SENSOR_DRVNAME_IMX219_MIPI_RAW, currSensorName)))
            {
                if(pinSetIdx == 0 && TRUE != _hwPowerOn(CAMERA_POWER_VCAM_D, VOL_1200,mode_name))
                {
                     PK_DBG("[CAMERA SENSOR] Fail to enable digital power\n");
                     goto _kdCISModulePowerOn_exit_;
                }
            }
            else { // Main VCAMD max 1.5V
                if(TRUE != _hwPowerOn(CAMERA_POWER_VCAM_D, VOL_1500,mode_name))
                {
                     PK_DBG("[CAMERA SENSOR] Fail to enable digital power\n");
                     goto _kdCISModulePowerOn_exit_;
                }

            }


             //AF_VCC
            if(TRUE != _hwPowerOn(CAMERA_POWER_VCAM_AF, VOL_2800,mode_name))
            {
                PK_DBG("[CAMERA SENSOR] Fail to enable analog power (VCAM_AF), power id = %d \n", CAMERA_POWER_VCAM_AF);
                goto _kdCISModulePowerOn_exit_;
            }

            mdelay(5);

            //enable active sensor
            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMPDN]) {
                if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! (CMPDN)\n");}
                if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! (CMPDN)\n");}
                if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_ON])){PK_DBG("[CAMERA LENS] set gpio failed!! (CMPDN)\n");}
            }

            mdelay(1);

            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST]) {
                if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! (CMRST)\n");}
                if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! (CMRST)\n");}
                if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_ON])){PK_DBG("[CAMERA SENSOR] set gpio failed!! (CMRST)\n");}
            }
        }
    }
    else {//power OFF

        PK_DBG("[PowerOFF]pinSetIdx:%d\n", pinSetIdx);
            ISP_MCLK1_EN(0);

        if ((currSensorName && (0 == strcmp(currSensorName,"imx135mipiraw")))||
            (currSensorName && (0 == strcmp(currSensorName,"imx220mipiraw"))))

        {
            //Set Power Pin low and Reset Pin Low
            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMPDN]) {
                if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! (CMPDN)\n");}
                if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! (CMPDN)\n");}
                if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! (CMPDN)\n");}
            }

            //Set Reset Pin Low
             if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST]) {
                if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! (CMRST)\n");}
                if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! (CMRST)\n");}
                if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! (CMRST)\n");}
            }

            //AF_VCC
            if(TRUE != _hwPowerDown(CAMERA_POWER_VCAM_AF,mode_name))
            {
                PK_DBG("[CAMERA SENSOR] Fail to OFF AF power (VCAM_AF), power id = %d \n", CAMERA_POWER_VCAM_AF);
                //return -EIO;
                goto _kdCISModulePowerOn_exit_;
            }

            //VCAM_IO
            if(TRUE != _hwPowerDown(CAMERA_POWER_VCAM_IO, mode_name)) {
                PK_DBG("[CAMERA SENSOR] Fail to OFF digital power (VCAM_IO), power id = %d \n", CAMERA_POWER_VCAM_IO);
                //return -EIO;
                goto _kdCISModulePowerOn_exit_;
            }

            if(TRUE != _hwPowerDown(CAMERA_POWER_VCAM_D,mode_name))
            {
                 PK_DBG("[CAMERA SENSOR] Fail to OFF core power (VCAM_D), power id = %d \n",CAMERA_POWER_VCAM_D);
                 goto _kdCISModulePowerOn_exit_;
            }

            //VCAM_A
            if(TRUE != _hwPowerDown(CAMERA_POWER_VCAM_A,mode_name)) {
                PK_DBG("[CAMERA SENSOR] Fail to OFF analog power (VCAM_A), power id= (%d) \n", CAMERA_POWER_VCAM_A);
                //return -EIO;
                goto _kdCISModulePowerOn_exit_;
            }

        }
        else if (currSensorName && (0 == strcmp(SENSOR_DRVNAME_OV5648_MIPI_RAW, currSensorName)))
        {
            mt_set_gpio_out(GPIO_SPI_MOSI_PIN,GPIO_OUT_ZERO);
            //Set Power Pin low and Reset Pin Low
            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMPDN]) {
                if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! (CMPDN)\n");}
                if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! (CMPDN)\n");}
                if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! (CMPDN)\n");}
            }

            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST]) {
                if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! (CMRST)\n");}
                if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! (CMRST)\n");}
                if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! (CMRST)\n");}
            }

            if(TRUE != _hwPowerDown(SUB_CAMERA_POWER_VCAM_D,mode_name))
            {
                 PK_DBG("[CAMERA SENSOR] Fail to OFF core power (VCAM_D), power id = %d \n",SUB_CAMERA_POWER_VCAM_D);
                 goto _kdCISModulePowerOn_exit_;
            }

            //VCAM_A
            if(TRUE != _hwPowerDown(CAMERA_POWER_VCAM_A,mode_name)) {
                PK_DBG("[CAMERA SENSOR] Fail to OFF analog power (VCAM_A), power id= (%d) \n", CAMERA_POWER_VCAM_A);
                //return -EIO;
                goto _kdCISModulePowerOn_exit_;
            }

            //VCAM_IO
            if(TRUE != _hwPowerDown(CAMERA_POWER_VCAM_IO, mode_name)) {
                PK_DBG("[CAMERA SENSOR] Fail to OFF digital power (VCAM_IO), power id = %d \n", CAMERA_POWER_VCAM_IO);
                //return -EIO;
                goto _kdCISModulePowerOn_exit_;
            }

            //AF_VCC
            if(TRUE != _hwPowerDown(CAMERA_POWER_VCAM_AF,mode_name))
            {
                PK_DBG("[CAMERA SENSOR] Fail to OFF AF power (VCAM_AF), power id = %d \n", CAMERA_POWER_VCAM_AF);
                //return -EIO;
                goto _kdCISModulePowerOn_exit_;
            }

        }
        else if (currSensorName && (0 == strcmp(SENSOR_DRVNAME_GC2355_MIPI_RAW, currSensorName)))
        {
            mt_set_gpio_out(GPIO_SPI_MOSI_PIN,GPIO_OUT_ZERO);
            //Set Power Pin low and Reset Pin Low
            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMPDN]) {
                if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! (CMPDN)\n");}
                if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! (CMPDN)\n");}
                if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_ON])){PK_DBG("[CAMERA LENS] set gpio failed!! (CMPDN)\n");}
            }

            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST]) {
                if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! (CMRST)\n");}
                if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! (CMRST)\n");}
                if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! (CMRST)\n");}
            }


            if(TRUE != _hwPowerDown(SUB_CAMERA_POWER_VCAM_D,mode_name))
            {
                 PK_DBG("[CAMERA SENSOR] Fail to OFF core power (VCAM_D), power id = %d \n",SUB_CAMERA_POWER_VCAM_D);
                 goto _kdCISModulePowerOn_exit_;
            }

            //VCAM_A
            if(TRUE != _hwPowerDown(CAMERA_POWER_VCAM_A,mode_name)) {
                PK_DBG("[CAMERA SENSOR] Fail to OFF analog power (VCAM_A), power id= (%d) \n", CAMERA_POWER_VCAM_A);
                //return -EIO;
                goto _kdCISModulePowerOn_exit_;
            }

            //VCAM_IO
            if(TRUE != _hwPowerDown(CAMERA_POWER_VCAM_IO, mode_name)) {
                PK_DBG("[CAMERA SENSOR] Fail to OFF digital power (VCAM_IO), power id = %d \n", CAMERA_POWER_VCAM_IO);
                //return -EIO;
                goto _kdCISModulePowerOn_exit_;
            }

            //AF_VCC
            if(TRUE != _hwPowerDown(CAMERA_POWER_VCAM_AF,mode_name))
            {
                PK_DBG("[CAMERA SENSOR] Fail to OFF AF power (VCAM_AF), power id = %d \n", CAMERA_POWER_VCAM_AF);
                //return -EIO;
                goto _kdCISModulePowerOn_exit_;
            }

        }
	  else if ((pinSetIdx == 0)&&(currSensorName && (0 == strcmp(SENSOR_DRVNAME_OV5670_MIPI_RAW, currSensorName))))
	{
		PK_DBG("kdCISModulePowerOff get in---  SENSOR_DRVNAME_OV5670_MIPI_RAW\n");
		PK_DBG("[OFF]pinSetIdx:%d \n",pinSetIdx);

	         if (GPIO_CAMERA_INVALID != pinSet[0][IDX_PS_CMRST]) {
	            if(mt_set_gpio_mode(pinSet[0][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}	          
	            if(mt_set_gpio_dir(pinSet[0][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}	         
	            if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");} //low == reset sensor
		}
		mdelay(1);

	        if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_D, mode_name)) {
		   PK_DBG("[CAMERA SENSOR] Fail to OFF digital power\n");
		   //return -EIO;
		   goto _kdCISModulePowerOn_exit_;
	        }
	    	if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_A,mode_name)) {
		   PK_DBG("[CAMERA SENSOR] Fail to OFF analog power\n");
		   //return -EIO;
		   goto _kdCISModulePowerOn_exit_;
	        }
		if(mt_set_gpio_mode(pinSet[0][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
		if(mt_set_gpio_dir(pinSet[0][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
	    	if(mt_set_gpio_out(pinSet[0][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");} //high == power down lens module
	        if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_IO,mode_name))
	        {
		   PK_DBG("[CAMERA SENSOR] Fail to enable digital power\n");
		   //return -EIO;
		   goto _kdCISModulePowerOn_exit_;
	        }

	}
    else if ((pinSetIdx == 0)&&(currSensorName && (0 == strcmp(SENSOR_DRVNAME_OV5670AF_MIPI_RAW, currSensorName))))
    {
		PK_DBG("kdCISModulePowerOff get in---  SENSOR_DRVNAME_OV5670_MIPI_RAW\n");
		PK_DBG("[OFF]pinSetIdx:%d \n",pinSetIdx);

	            if (GPIO_CAMERA_INVALID != pinSet[0][IDX_PS_CMRST]) {
	            if(mt_set_gpio_mode(pinSet[0][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}     
	            if(mt_set_gpio_dir(pinSet[0][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}     
	            if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");} //low == reset sensor
			}

		/*Defect 437608 modify by yinzhanyong at 20150721 begin*/
		  //AF disable pin
	        if(mt_set_gpio_mode(GPIO_CAMERA_AF_EN_PIN,GPIO_MODE_00)){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
	        if(mt_set_gpio_dir(GPIO_CAMERA_AF_EN_PIN,GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
	        if(mt_set_gpio_out(GPIO_CAMERA_AF_EN_PIN,GPIO_OUT_ZERO)){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}
		mdelay(10);
                printk("disable AF PIN\n");
		/*Defect 437608 modify by yinzhanyong at 20150721 end*/		
		mdelay(1);

	        if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_D, mode_name)) {
		   PK_DBG("[CAMERA SENSOR] Fail to OFF digital power\n");
		   //return -EIO;
		   goto _kdCISModulePowerOn_exit_;
	        }
	    	if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_A,mode_name)) {
		   PK_DBG("[CAMERA SENSOR] Fail to OFF analog power\n");
		   //return -EIO;
		   goto _kdCISModulePowerOn_exit_;
	        }

		if(mt_set_gpio_mode(pinSet[0][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
		if(mt_set_gpio_dir(pinSet[0][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
	    	if(mt_set_gpio_out(pinSet[0][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");} //high == power down lens module

	        if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_IO,mode_name))
	        {
		   PK_DBG("[CAMERA SENSOR] Fail to enable digital power\n");
		   //return -EIO;
		   goto _kdCISModulePowerOn_exit_;
	        }
		  //AF_VCC
           	 if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_AF,mode_name))
           	 {
               	 	PK_DBG("[CAMERA SENSOR] Fail to OFF AF power (VCAM_AF), power id = %d \n", CAMERA_POWER_VCAM_AF);
               	 	//return -EIO;
              	 	goto _kdCISModulePowerOn_exit_;
           	 }


	}
	  
      else if ((pinSetIdx == 0)&&(currSensorName && (0 == strcmp(SENSOR_DRVNAME_OV8858_MIPI_RAW, currSensorName))))
     {
		PK_DBG("kdCISModulePowerOff get in---  SENSOR_DRVNAME_OV8858_MIPI_RAW\n");
		PK_DBG("[OFF]pinSetIdx:%d \n",pinSetIdx);

	            if (GPIO_CAMERA_INVALID != pinSet[0][IDX_PS_CMRST]) {
	            if(mt_set_gpio_mode(pinSet[0][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
	            if(mt_set_gpio_mode(pinSet[0][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
	            if(mt_set_gpio_dir(pinSet[0][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
	            if(mt_set_gpio_dir(pinSet[0][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
	    	    if(mt_set_gpio_out(pinSet[0][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");} //high == power down lens module
	            if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");} //low == reset sensor
			}
			/*Defect 437608 modify by yinzhanyong at 20150721 begin*/
		       //AF enable pin
	        if(mt_set_gpio_mode(GPIO_CAMERA_AF_EN_PIN,GPIO_MODE_00)){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
	        if(mt_set_gpio_dir(GPIO_CAMERA_AF_EN_PIN,GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
	        if(mt_set_gpio_out(GPIO_CAMERA_AF_EN_PIN,GPIO_OUT_ZERO)){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}
	        mdelay(10);
                printk("disable AF PIN\n");
			/*Defect 437608 modify by yinzhanyong at 20150721 begin*/
		mdelay(1);

	        if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_D, mode_name)) {
		   PK_DBG("[CAMERA SENSOR] Fail to OFF digital power\n");
		   //return -EIO;
		   goto _kdCISModulePowerOn_exit_;
	        }
	    	if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_A,mode_name)) {
		   PK_DBG("[CAMERA SENSOR] Fail to OFF analog power\n");
		   //return -EIO;
		   goto _kdCISModulePowerOn_exit_;
	        }
	        if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_IO,mode_name))
	        {
		   PK_DBG("[CAMERA SENSOR] Fail to enable digital power\n");
		   //return -EIO;
		   goto _kdCISModulePowerOn_exit_;
	        }
		  //AF_VCC
           	 if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_AF,mode_name))
           	 {
               	 	PK_DBG("[CAMERA SENSOR] Fail to OFF AF power (VCAM_AF), power id = %d \n", CAMERA_POWER_VCAM_AF);
               	 	//return -EIO;
              	 	goto _kdCISModulePowerOn_exit_;
           	 }


	}

      else if ((pinSetIdx == 0)&&(currSensorName && (0 == strcmp(SENSOR_DRVNAME_S5K5E2YA_MIPI_RAW, currSensorName))))
     {
		PK_DBG("kdCISModulePowerOff get in---  SENSOR_DRVNAME_S5K5E2YA_MIPI_RAW\n");
		PK_DBG("[OFF]pinSetIdx:%d \n",pinSetIdx);

            

	        if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_D, mode_name)) {
		   PK_DBG("[CAMERA SENSOR] Fail to OFF digital power\n");
		   //return -EIO;
		   goto _kdCISModulePowerOn_exit_;
	        }
	    	if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_A,mode_name)) {
		   PK_DBG("[CAMERA SENSOR] Fail to OFF analog power\n");
		   //return -EIO;
		   goto _kdCISModulePowerOn_exit_;
	        }
	        if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_IO,mode_name))
	        {
		   PK_DBG("[CAMERA SENSOR] Fail to enable digital power\n");
		   //return -EIO;
		   goto _kdCISModulePowerOn_exit_;
	        }
		

		mdelay(1);



		if (GPIO_CAMERA_INVALID != pinSet[0][IDX_PS_CMRST]) {
	            if(mt_set_gpio_mode(pinSet[0][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
	            if(mt_set_gpio_mode(pinSet[0][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
	            if(mt_set_gpio_dir(pinSet[0][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
	            if(mt_set_gpio_dir(pinSet[0][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
	    	    if(mt_set_gpio_out(pinSet[0][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");} //high == power down lens module
	            if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");} //low == reset sensor
			}
			mdelay(1);
			
	}
      else if ((pinSetIdx == 0)&&(currSensorName && (0 == strcmp(SENSOR_DRVNAME_S5K5E2YAAF_MIPI_RAW, currSensorName))))
     {
		PK_DBG("kdCISModulePowerOff get in---  SENSOR_DRVNAME_S5K5E2YA AF_MIPI_RAW\n");
		PK_DBG("[OFF]pinSetIdx:%d \n",pinSetIdx);

            

	        if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_D, mode_name)) {
		   PK_DBG("[CAMERA SENSOR] Fail to OFF digital power\n");
		   //return -EIO;
		   goto _kdCISModulePowerOn_exit_;
	        }
	    	if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_A,mode_name)) {
		   PK_DBG("[CAMERA SENSOR] Fail to OFF analog power\n");
		   //return -EIO;
		   goto _kdCISModulePowerOn_exit_;
	        }
	        if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_IO,mode_name))
	        {
		   PK_DBG("[CAMERA SENSOR] Fail to enable digital power\n");
		   //return -EIO;
		   goto _kdCISModulePowerOn_exit_;
	        }
		  //AF_VCC
           	 if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_AF,mode_name))
           	 {
               	 	PK_DBG("[CAMERA SENSOR] Fail to OFF AF power (VCAM_AF), power id = %d \n", CAMERA_POWER_VCAM_AF);
               	 	//return -EIO;
              	 	goto _kdCISModulePowerOn_exit_;
           	 }
		    /*Defect 437608 modify by yinzhanyong at 20150721 begin*/
		       //AF enable pin
	        if(mt_set_gpio_mode(GPIO_CAMERA_AF_EN_PIN,GPIO_MODE_00)){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
	        if(mt_set_gpio_dir(GPIO_CAMERA_AF_EN_PIN,GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
	        if(mt_set_gpio_out(GPIO_CAMERA_AF_EN_PIN,GPIO_OUT_ZERO)){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}
	        mdelay(10);
                printk("disable AF PIN\n");
			/*Defect 437608 modify by yinzhanyong at 20150721 end*/
		mdelay(1);



		if (GPIO_CAMERA_INVALID != pinSet[0][IDX_PS_CMRST]) {
	            if(mt_set_gpio_mode(pinSet[0][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
	            if(mt_set_gpio_mode(pinSet[0][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
	            if(mt_set_gpio_dir(pinSet[0][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
	            if(mt_set_gpio_dir(pinSet[0][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
	    	    if(mt_set_gpio_out(pinSet[0][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");} //high == power down lens module
	            if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");} //low == reset sensor
			}
			mdelay(1);
			
	}

      else if ((pinSetIdx == 0)&&(currSensorName && (0 == strcmp(SENSOR_DRVNAME_S5K4H5YC_MIPI_RAW, currSensorName))))
      {
		PK_DBG("kdCISModulePowerOff get in---  SENSOR_DRVNAME_S5K4H5YC_MIPI_RAW\n");
		PK_DBG("[OFF]pinSetIdx:%d \n",pinSetIdx);
		
		mdelay(10);

		if (GPIO_CAMERA_INVALID != pinSet[0][IDX_PS_CMRST]) {
	            if(mt_set_gpio_mode(pinSet[0][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
	            if(mt_set_gpio_mode(pinSet[0][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
	            if(mt_set_gpio_dir(pinSet[0][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
	            if(mt_set_gpio_dir(pinSet[0][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
	    	    if(mt_set_gpio_out(pinSet[0][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");} //high == power down lens module
		}
		mdelay(1);


			

	        if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_D, mode_name)) {
		   PK_DBG("[CAMERA SENSOR] Fail to OFF digital power\n");
		   //return -EIO;
		   goto _kdCISModulePowerOn_exit_;
	        }
	    	if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_A,mode_name)) {
		   PK_DBG("[CAMERA SENSOR] Fail to OFF analog power\n");
		   //return -EIO;
		   goto _kdCISModulePowerOn_exit_;
	        }
	        if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_IO,mode_name))
	        {
		   PK_DBG("[CAMERA SENSOR] Fail to enable digital power\n");
		   //return -EIO;
		   goto _kdCISModulePowerOn_exit_;
	        }
		  //AF_VCC
           	 if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_AF,mode_name))
           	 {
               	 	PK_DBG("[CAMERA SENSOR] Fail to OFF AF power (VCAM_AF), power id = %d \n", CAMERA_POWER_VCAM_AF);
               	 	//return -EIO;
              	 	goto _kdCISModulePowerOn_exit_;
           	 }
		    /*Defect 437608 modify by yinzhanyong at 20150721 begin*/
		       //AF enable pin
	        if(mt_set_gpio_mode(GPIO_CAMERA_AF_EN_PIN,GPIO_MODE_00)){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
	        if(mt_set_gpio_dir(GPIO_CAMERA_AF_EN_PIN,GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
	        if(mt_set_gpio_out(GPIO_CAMERA_AF_EN_PIN,GPIO_OUT_ZERO)){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}
	        mdelay(10);
                printk("disable AF PIN\n");
			/*Defect 437608 modify by yinzhanyong at 20150721 end*/
		mdelay(1);



	
			
	}


	  


       else if ((pinSetIdx == 1)&&(currSensorName && (0 == strcmp(SENSOR_DRVNAME_GC0329_YUV, currSensorName))))
      {

	   if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
		   if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
		   if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_ON])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}


	    if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST]) {
		   if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
		   if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
		   if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");} 
	       
		
            }

	    if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_A,mode_name)) {
	           PK_DBG("[CAMERA SENSOR] Fail to OFF analog power\n");
	           goto _kdCISModulePowerOn_exit_;
	    }
	       
	    if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_IO,mode_name))
	    {
	           PK_DBG("[CAMERA SENSOR] Fail to enable analog power\n");
	           goto _kdCISModulePowerOn_exit_;
	    }
	} 

	else if ((pinSetIdx == 1)&&(currSensorName && (0 == strcmp(SENSOR_DRVNAME_GC2145_YUV,currSensorName)))){
		if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
		if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
		if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_ON])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}

		if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
		if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
		if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");}


		if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_A,mode_name)) {  //AVDD
	           PK_DBG("[CAMERA SENSOR] Fail to OFF analog power\n");
	           goto _kdCISModulePowerOn_exit_;
	    	}

		 if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_IO,mode_name))  //IOVDD
	    	{
	           PK_DBG("[CAMERA SENSOR] Fail to enable analog power\n");
	           goto _kdCISModulePowerOn_exit_;
	    	}

		 if(TRUE != hwPowerDown(MT6328_POWER_LDO_VGP1,mode_name))  //DVDD
	    	{
	           PK_DBG("[CAMERA SENSOR] Fail to enable analog power\n");
	           goto _kdCISModulePowerOn_exit_;
	   	}

	} 
	
		else if ((pinSetIdx == 1)&&(currSensorName && (0 == strcmp(SENSOR_DRVNAME_OV2686_YUV,currSensorName)))){
		if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
		if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
		if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}

		if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_A,mode_name)) {  //AVDD
	           PK_DBG("[CAMERA SENSOR] Fail to OFF analog power\n");
	           goto _kdCISModulePowerOn_exit_;
	    	}

		 if(TRUE != hwPowerDown(MT6328_POWER_LDO_VGP1,mode_name))  //DVDD
	    	{
	           PK_DBG("[CAMERA SENSOR] Fail to enable analog power\n");
	           goto _kdCISModulePowerOn_exit_;
	   	}

		  if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_IO,mode_name))  //IOVDD
	    	{
	           PK_DBG("[CAMERA SENSOR] Fail to enable analog power\n");
	           goto _kdCISModulePowerOn_exit_;
	    	}
	}

        else
        {
            //Set Power Pin low and Reset Pin Low
            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMPDN]) {
                if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! (CMPDN)\n");}
                if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! (CMPDN)\n");}
                if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! (CMPDN)\n");}
            }


            if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST]) {
                    if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! (CMRST)\n");}
                    if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! (CMRST)\n");}
                    if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! (CMRST)\n");}
            }

            if(currSensorName && (0 == strcmp(SENSOR_DRVNAME_IMX219_MIPI_RAW, currSensorName)))
	    {
		if(pinSetIdx == 0 && TRUE != _hwPowerDown(CAMERA_POWER_VCAM_D, mode_name))
		 {
		   PK_DBG("[CAMERA SENSOR] main imx220 Fail to OFF core power (VCAM_D), power id = %d \n",CAMERA_POWER_VCAM_D);
		   goto _kdCISModulePowerOn_exit_;
		 }
	    } else {

	         if(TRUE != _hwPowerDown(CAMERA_POWER_VCAM_D,mode_name))
	         {
	            PK_DBG("[CAMERA SENSOR] Fail to OFF core power (VCAM_D), power id = %d \n",CAMERA_POWER_VCAM_D);
	            goto _kdCISModulePowerOn_exit_;
	          }
	   }

            //VCAM_A
            if(TRUE != _hwPowerDown(CAMERA_POWER_VCAM_A,mode_name)) {
                PK_DBG("[CAMERA SENSOR] Fail to OFF analog power (VCAM_A), power id= (%d) \n", CAMERA_POWER_VCAM_A);
                //return -EIO;
                goto _kdCISModulePowerOn_exit_;
            }

            //VCAM_IO
            if(TRUE != _hwPowerDown(CAMERA_POWER_VCAM_IO, mode_name)) {
                PK_DBG("[CAMERA SENSOR] Fail to OFF digital power (VCAM_IO), power id = %d \n", CAMERA_POWER_VCAM_IO);
                //return -EIO;
                goto _kdCISModulePowerOn_exit_;
            }

            //AF_VCC
            if(TRUE != _hwPowerDown(CAMERA_POWER_VCAM_AF,mode_name))
            {
                PK_DBG("[CAMERA SENSOR] Fail to OFF AF power (VCAM_AF), power id = %d \n", CAMERA_POWER_VCAM_AF);
                //return -EIO;
                goto _kdCISModulePowerOn_exit_;
            }

        }

    }

    return 0;

_kdCISModulePowerOn_exit_:
    return -EIO;

}

EXPORT_SYMBOL(kdCISModulePowerOn);

//!--
//


