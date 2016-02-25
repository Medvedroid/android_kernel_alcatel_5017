#ifndef _CUST_BAT_H_
#define _CUST_BAT_H_
/* High battery support */
/*[101253] [pixi3-4.5 4g] turn on high battery voltage suport by [liming.zhu.hz] at [2015.0121] bigen*/
#define HIGH_BATTERY_VOLTAGE_SUPPORT
/*[101253] [pixi3-4.5 4g] turn on high battery voltage suport by [liming.zhu.hz] at [2015.0121] end*/

/* stop charging while in talking mode */
#define STOP_CHARGING_IN_TAKLING
#define TALKING_RECHARGE_VOLTAGE 3800
#define TALKING_SYNC_TIME		   60
/*[964522] change battery charge behave accord with Temperature Behavior Definition_V1 0_20140715.docx by liming.zhu at 201500404 bigen*/
/* Battery Temperature Protection */
#define MTK_TEMPERATURE_RECHARGE_SUPPORT
#define MAX_CHARGE_TEMPERATURE  55
#define MAX_CHARGE_TEMPERATURE_MINUS_X_DEGREE	50
#define MIN_CHARGE_TEMPERATURE  0
#define MIN_CHARGE_TEMPERATURE_PLUS_X_DEGREE	1
#define ERR_CHARGE_TEMPERATURE  0xFF
/*[964522] change battery charge behave accord with Temperature Behavior Definition_V1 0_20140715.docx by liming.zhu at 201500404 end*/

/* Linear Charging Threshold */
#define V_PRE2CC_THRES	 		3400	//mV
#define V_CC2TOPOFF_THRES		4050
/*[212006] The capacity diaplay abnormally by liming.zhu at 20150506 bigen*/
#ifdef HIGH_BATTERY_VOLTAGE_SUPPORT
#define RECHARGING_VOLTAGE      4270
#else
#define RECHARGING_VOLTAGE      4110
#endif
/*[212006] The capacity diaplay abnormally by liming.zhu at 20150506 end*/
/*[101253] change the charge full current to cut down the charge time by liming.zhu at 20150413 bigen*/
#define CHARGING_FULL_CURRENT    150	//mA
/*[101253] change the charge full current to cut down the charge time by liming.zhu at 20150413 end*/

/* Charging Current Setting */
//#define CONFIG_USB_IF 						   
#define USB_CHARGER_CURRENT_SUSPEND			0		// def CONFIG_USB_IF
#define USB_CHARGER_CURRENT_UNCONFIGURED	CHARGE_CURRENT_70_00_MA	// 70mA
#define USB_CHARGER_CURRENT_CONFIGURED		CHARGE_CURRENT_500_00_MA	// 500mA

#define USB_CHARGER_CURRENT					CHARGE_CURRENT_500_00_MA	//500mA
/*[138932] [pixi3-4.5 4G] [mt6735m] change AC charge curren from 800ma to 650ma [liming.zhu]at [20150318] bigen*/
#define AC_CHARGER_CURRENT					CHARGE_CURRENT_650_00_MA
/*[138932] [pixi3-4.5 4G] [mt6735m] change AC charge curren from 800ma to 650ma [liming.zhu]at [20150318] end*/
//#define AC_CHARGER_CURRENT					CHARGE_CURRENT_1000_00_MA
#define NON_STD_AC_CHARGER_CURRENT			CHARGE_CURRENT_500_00_MA
#define CHARGING_HOST_CHARGER_CURRENT       CHARGE_CURRENT_650_00_MA
#define APPLE_0_5A_CHARGER_CURRENT          CHARGE_CURRENT_500_00_MA
#define APPLE_1_0A_CHARGER_CURRENT          CHARGE_CURRENT_650_00_MA
#define APPLE_2_1A_CHARGER_CURRENT          CHARGE_CURRENT_800_00_MA
/*[388754]select charging current min of normal charging current and limit charging current by liming.zhu at 20150625 bigen*/
#define CHARGE_CURRENT_LIMIT_MAX                          CHARGE_CURRENT_650_00_MA
/*[388754]select charging current min of normal charging current and limit charging current by liming.zhu at 20150625 end*/

/* Precise Tunning */
#define BATTERY_AVERAGE_DATA_NUMBER	3	
#define BATTERY_AVERAGE_SIZE 	30

/* charger error check */
/*[927242] [mt6735m] [pixi345] enable low temprature charge proctect by [liming.zhu] at [201535] bigen*/ 
#define BAT_LOW_TEMP_PROTECT_ENABLE         // stop charging if temp < MIN_CHARGE_TEMPERATURE
/*[927242] [mt6735m] [pixi345] enable low temprature charge proctect by [liming.zhu] at [201535] end*/
#define V_CHARGER_ENABLE 0				// 1:ON , 0:OFF	
#define V_CHARGER_MAX 6500				// 6.5 V
#define V_CHARGER_MIN 4400				// 4.4 V

/* Tracking TIME */
#define ONEHUNDRED_PERCENT_TRACKING_TIME	10	// 10 second
#define NPERCENT_TRACKING_TIME	   			20	// 20 second
#define SYNC_TO_REAL_TRACKING_TIME  		60	// 60 second
#define V_0PERCENT_TRACKING							3450 //3450mV

/* Battery Notify */
#define BATTERY_NOTIFY_CASE_0001_VCHARGER
#define BATTERY_NOTIFY_CASE_0002_VBATTEMP
//#define BATTERY_NOTIFY_CASE_0003_ICHARGING
//#define BATTERY_NOTIFY_CASE_0004_VBAT
//#define BATTERY_NOTIFY_CASE_0005_TOTAL_CHARGINGTIME


/* [834193] [MT6581][Pixi3-4NA]Support new temperature behaviour definition by [bin.song.hz] at [2014.11.7] begin */
#define REDUCE_CURRENT_TEMPERATURE_P1 10
#define REDUCE_CURRENT_TEMPERATURE_P2 38
#define REDUCE_CURRENT_TEMPERATURE_P3 45

#define MAX_CHARGE_CURRENT_NORMAL_TEMPERATURE  AC_CHARGER_CURRENT
#define MAX_CHARGE_CURRENT_OVER_TEMPERATURE_P1 AC_CHARGER_CURRENT
#define MAX_CHARGE_CURRENT_OVER_TEMPERATURE_P2 AC_CHARGER_CURRENT
#define MAX_CHARGE_CURRENT_OVER_TEMPERATURE_P3 AC_CHARGER_CURRENT
#ifdef HIGH_BATTERY_VOLTAGE_SUPPORT
#define CV_VOLTAGE_NORMAL_TEMPERATURE  BATTERY_VOLT_04_350000_V
#else
#define CV_VOLTAGE_NORMAL_TEMPERATURE  BATTERY_VOLT_04_200000_V
#endif

#ifdef HIGH_BATTERY_VOLTAGE_SUPPORT
#define CV_VOLTAGE_OVER_TEMPERATURE_P1 BATTERY_VOLT_04_350000_V
#else
#define CV_VOLTAGE_OVER_TEMPERATURE_P1 BATTERY_VOLT_04_200000_V
#endif

#ifdef HIGH_BATTERY_VOLTAGE_SUPPORT
#define CV_VOLTAGE_OVER_TEMPERATURE_P2 BATTERY_VOLT_04_350000_V
#else
#define CV_VOLTAGE_OVER_TEMPERATURE_P2 BATTERY_VOLT_04_200000_V
#endif
#define CV_VOLTAGE_OVER_TEMPERATURE_P3 BATTERY_VOLT_04_100000_V
/* [834193] [MT6581][Pixi3-4NA]Support new temperature behaviour definition by [bin.song.hz] at [2014.11.7] end */

/* JEITA parameter */
//#define MTK_JEITA_STANDARD_SUPPORT
#define CUST_SOC_JEITA_SYNC_TIME 30
#define JEITA_RECHARGE_VOLTAGE  4110	// for linear charging
#ifdef HIGH_BATTERY_VOLTAGE_SUPPORT
#define JEITA_TEMP_ABOVE_POS_60_CV_VOLTAGE		BATTERY_VOLT_04_240000_V
#define JEITA_TEMP_POS_45_TO_POS_60_CV_VOLTAGE		BATTERY_VOLT_04_240000_V
#define JEITA_TEMP_POS_10_TO_POS_45_CV_VOLTAGE		BATTERY_VOLT_04_340000_V
#define JEITA_TEMP_POS_0_TO_POS_10_CV_VOLTAGE		BATTERY_VOLT_04_240000_V
#define JEITA_TEMP_NEG_10_TO_POS_0_CV_VOLTAGE		BATTERY_VOLT_04_040000_V
#define JEITA_TEMP_BELOW_NEG_10_CV_VOLTAGE		BATTERY_VOLT_04_040000_V
#else
#define JEITA_TEMP_ABOVE_POS_60_CV_VOLTAGE		BATTERY_VOLT_04_100000_V
#define JEITA_TEMP_POS_45_TO_POS_60_CV_VOLTAGE	BATTERY_VOLT_04_100000_V
#define JEITA_TEMP_POS_10_TO_POS_45_CV_VOLTAGE	BATTERY_VOLT_04_200000_V
#define JEITA_TEMP_POS_0_TO_POS_10_CV_VOLTAGE	BATTERY_VOLT_04_100000_V
#define JEITA_TEMP_NEG_10_TO_POS_0_CV_VOLTAGE	BATTERY_VOLT_03_900000_V
#define JEITA_TEMP_BELOW_NEG_10_CV_VOLTAGE		BATTERY_VOLT_03_900000_V
#endif
/* For JEITA Linear Charging only */
#define JEITA_NEG_10_TO_POS_0_FULL_CURRENT  120	//mA 
#define JEITA_TEMP_POS_45_TO_POS_60_RECHARGE_VOLTAGE  4000
#define JEITA_TEMP_POS_10_TO_POS_45_RECHARGE_VOLTAGE  4100
#define JEITA_TEMP_POS_0_TO_POS_10_RECHARGE_VOLTAGE   4000
#define JEITA_TEMP_NEG_10_TO_POS_0_RECHARGE_VOLTAGE   3800
#define JEITA_TEMP_POS_45_TO_POS_60_CC2TOPOFF_THRESHOLD	4050
#define JEITA_TEMP_POS_10_TO_POS_45_CC2TOPOFF_THRESHOLD	4050
#define JEITA_TEMP_POS_0_TO_POS_10_CC2TOPOFF_THRESHOLD	4050
#define JEITA_TEMP_NEG_10_TO_POS_0_CC2TOPOFF_THRESHOLD	3850


/* For CV_E1_INTERNAL */
#define CV_E1_INTERNAL

/* Disable Battery check for HQA */
#ifdef CONFIG_MTK_DISABLE_POWER_ON_OFF_VOLTAGE_LIMITATION
#define CONFIG_DIS_CHECK_BATTERY
#endif

#ifdef CONFIG_MTK_FAN5405_SUPPORT
#define FAN5405_BUSNUM 1
#endif

#define MTK_PLUG_OUT_DETECTION
#define CHARGING_HOST_SUPPORT

#endif /* _CUST_BAT_H_ */ 
