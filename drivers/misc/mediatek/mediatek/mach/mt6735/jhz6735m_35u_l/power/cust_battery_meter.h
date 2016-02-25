#ifndef _CUST_BATTERY_METER_H
#define _CUST_BATTERY_METER_H

#include <mach/mt_typedefs.h>

// ============================================================
// define
// ============================================================
//#define SOC_BY_AUXADC
#define SOC_BY_HW_FG
//#define HW_FG_FORCE_USE_SW_OCV
//#define SOC_BY_SW_FG

//#define CONFIG_DIS_CHECK_BATTERY
//#define FIXED_TBAT_25

/* ADC Channel Number */
#if 0
#define CUST_TABT_NUMBER 17
#define VBAT_CHANNEL_NUMBER      7
#define ISENSE_CHANNEL_NUMBER	 6
#define VCHARGER_CHANNEL_NUMBER  4
#define VBATTEMP_CHANNEL_NUMBER  5
#endif
/* ADC resistor  */
#define R_BAT_SENSE 4					
#define R_I_SENSE 4						
#define R_CHARGER_1 330
#define R_CHARGER_2 39

#define TEMPERATURE_T0             110
#define TEMPERATURE_T1             0
#define TEMPERATURE_T2             25
#define TEMPERATURE_T3             50
#define TEMPERATURE_T              255 // This should be fixed, never change the value

#define FG_METER_RESISTANCE 	0

/*[212006] The capacity diaplay abnormally by liming.zhu at 20150506 bigen*/
/* Qmax for battery  */
#define Q_MAX_POS_SCUD_50	1769
#define Q_MAX_POS_SCUD_25	1764
#define Q_MAX_POS_SCUD_0	1788
#define Q_MAX_NEG_SCUD_10	1825

#define Q_MAX_POS_50_H_SCUD_CURRENT	1752
#define Q_MAX_POS_25_H_SCUD_CURRENT	1718
#define Q_MAX_POS_0_H_SCUD_CURRENT	1203
#define Q_MAX_NEG_10_H_SCUD_CURRENT	573

#define Q_MAX_POS_BYD_50	1799
#define Q_MAX_POS_BYD_25	1775
#define Q_MAX_POS_BYD_0		1374
#define Q_MAX_NEG_BYD_10	924

#define Q_MAX_POS_50_H_BYD_CURRENT	1205
#define Q_MAX_POS_25_H_BYD_CURRENT	1775
#define Q_MAX_POS_0_H_BYD_CURRENT	916
#define Q_MAX_NEG_10_H_BYD_CURRENT	916
/*[212006] The capacity diaplay abnormally by liming.zhu at 20150506 end*/

/* Discharge Percentage */
#define OAM_D5		 1		//  1 : D5,   0: D2


/* battery meter parameter */
#define CHANGE_TRACKING_POINT
/*[212006] The capacity diaplay abnormally by liming.zhu at 20150506 bigen*/
#ifdef CONFIG_CAR_TUNE_VALUE_Check
#define CUST_TRACKING_POINT  0
#else
#define CUST_TRACKING_POINT  15
#endif
/*[212006] The capacity diaplay abnormally by liming.zhu at 20150506 end*/
/*[350634] chage charge Isense by liming.zhu at 20150617 begin*/
#define CUST_R_SENSE         56
/*[350634] chage charge Isense by liming.zhu at 20150617 end*/
#define CUST_HW_CC 		    0
#define AGING_TUNING_VALUE   103
#define CUST_R_FG_OFFSET    0

#define OCV_BOARD_COMPESATE	0 //mV 
#define R_FG_BOARD_BASE		1000
#define R_FG_BOARD_SLOPE	1000 //slope
/*[212006] The capacity diaplay abnormally by liming.zhu at 20150506 bigen*/
/*[326405] battery dicharged too fast by liming.zhu.hz at 20150614 bigen*/
#ifdef CONFIG_CAR_TUNE_VALUE_Check
extern int CAR_TUNE_VALUE;//1.00
//#define CAR_TUNE_VALUE_Debug 
#define MAX_CAR_TUNE_VALUE 95
#define MIN_CAR_TUNE_VALUE  70
#define BAT_CFG_PATH "/data/misc/battery/TCL_battery.cfg"
#else
#define CAR_TUNE_VALUE 80
#endif
/*[326405] battery dicharged too fast by liming.zhu.hz at 20150614 end*/
/*[212006] The capacity diaplay abnormally by liming.zhu at 20150506 end*/

/* HW Fuel gague  */
#define CURRENT_DETECT_R_FG	10  //1mA
#define MinErrorOffset       1000
#define FG_VBAT_AVERAGE_SIZE 18
#define R_FG_VALUE 			10 // mOhm, base is 20

#define CUST_POWERON_DELTA_CAPACITY_TOLRANCE	30
#define CUST_POWERON_LOW_CAPACITY_TOLRANCE		5
#define CUST_POWERON_MAX_VBAT_TOLRANCE			90
#define CUST_POWERON_DELTA_VBAT_TOLRANCE		30
#define CUST_POWERON_DELTA_HW_SW_OCV_CAPACITY_TOLRANCE	10


/* Disable Battery check for HQA */
#ifdef MTK_DISABLE_POWER_ON_OFF_VOLTAGE_LIMITATION
#define FIXED_TBAT_25
#endif

/* Dynamic change wake up period of battery thread when suspend*/
#define VBAT_NORMAL_WAKEUP		3600		//3.6V
#define VBAT_LOW_POWER_WAKEUP		3500		//3.5v
#define NORMAL_WAKEUP_PERIOD		5400 		//90 * 60 = 90 min
#define LOW_POWER_WAKEUP_PERIOD		300		//5 * 60 = 5 min
#define CLOSE_POWEROFF_WAKEUP_PERIOD	30	//30 s

#define FG_BAT_INT
#define IS_BATTERY_REMOVE_BY_PMIC


#endif	//#ifndef _CUST_BATTERY_METER_H
