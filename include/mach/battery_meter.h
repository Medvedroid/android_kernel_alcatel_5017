#ifndef _BATTERY_METER_H
#define _BATTERY_METER_H

#include <mach/mt_typedefs.h>
#include "cust_battery_meter.h"
/* ============================================================ */
/* define */
/* ============================================================ */
#define FG_CURRENT_AVERAGE_SIZE 30

/* ============================================================ */
/* ENUM */
/* ============================================================ */

/* ============================================================ */
/* structure */
/* ============================================================ */
/*[470223] merge ALPS02160943(For_JHZ6735M_65C_L_ALPS.L1.MP3.V1_P77) patch by liming.zhu.hz at 20150729 bigen*/
#define FGD_NL_MSG_T_HDR_LEN 12
#define FGD_NL_MSG_MAX_LEN 9200

struct fgd_nl_msg_t{
    unsigned int fgd_cmd;
    unsigned int fgd_data_len;
    unsigned int fgd_ret_data_len;
	char fgd_data[FGD_NL_MSG_MAX_LEN];
};

enum {
	FG_MAIN = 1,
	FG_SUSPEND = 2,
	FG_RESUME = 4,
	FG_CHARGER = 8
};

enum {
	HW_FG,
	SW_FG,
	AUXADC
};

/*[470223] merge ALPS02160943(For_JHZ6735M_65C_L_ALPS.L1.MP3.V1_P77) patch by liming.zhu.hz at 20150729 end*/
/* ============================================================ */
/* typedef */
/* ============================================================ */
typedef struct {
	INT32 BatteryTemp;
	INT32 TemperatureR;
} BATT_TEMPERATURE;
/*[470223] merge ALPS02160943(For_JHZ6735M_65C_L_ALPS.L1.MP3.V1_P77) patch by liming.zhu.hz at 20150729 bigen*/
typedef enum {
	FG_DAEMON_CMD_GET_INIT_FLAG,
	FG_DAEMON_CMD_GET_SOC,
	FG_DAEMON_CMD_GET_DOD0,
	FG_DAEMON_CMD_GET_DOD1,
	FG_DAEMON_CMD_GET_HW_OCV,
	FG_DAEMON_CMD_GET_HW_FG_INIT_CURRENT,
	FG_DAEMON_CMD_GET_HW_FG_CURRENT,
	FG_DAEMON_CMD_GET_HW_FG_INIT_CURRENT_SIGN,
	FG_DAEMON_CMD_GET_HW_FG_CURRENT_SIGN,
	FG_DAEMON_CMD_GET_HW_FG_CAR_ACT,
	FG_DAEMON_CMD_GET_TEMPERTURE,
	FG_DAEMON_CMD_DUMP_REGISTER,
	FG_DAEMON_CMD_CHARGING_ENABLE,
	FG_DAEMON_CMD_GET_BATTERY_INIT_VOLTAGE,
	FG_DAEMON_CMD_GET_BATTERY_VOLTAGE,
	FG_DAEMON_CMD_FGADC_RESET,
	FG_DAEMON_CMD_GET_BATTERY_PLUG_STATUS,
	FG_DAEMON_CMD_GET_RTC_SPARE_FG_VALUE,
	FG_DAEMON_CMD_IS_CHARGER_EXIST,
	FG_DAEMON_CMD_IS_BATTERY_FULL,           //bat_is_battery_full,
	FG_DAEMON_CMD_SET_BATTERY_FULL,			 //bat_set_battery_full,
	FG_DAEMON_CMD_SET_RTC,					 //set RTC,
	FG_DAEMON_CMD_SET_POWEROFF,				 //set Poweroff,
	FG_DAEMON_CMD_IS_KPOC,           		 //is KPOC,
	FG_DAEMON_CMD_GET_BOOT_REASON,           //g_boot_reason,
	FG_DAEMON_CMD_GET_CHARGING_CURRENT,
	FG_DAEMON_CMD_GET_CHARGER_VOLTAGE,
	FG_DAEMON_CMD_GET_SHUTDOWN_COND,
	FG_DAEMON_CMD_GET_CUSTOM_SETTING,
	FG_DAEMON_CMD_GET_UI_SOC,
	FG_DAEMON_CMD_GET_CV_VALUE,
	FG_DAEMON_CMD_GET_DURATION_TIME,
	FG_DAEMON_CMD_GET_TRACKING_TIME,
	FG_DAEMON_CMD_GET_CURRENT_TH,
	FG_DAEMON_CMD_GET_CHECK_TIME,
	FG_DAEMON_CMD_GET_DIFFERENCE_VOLTAGE_UPDATE,
	FG_DAEMON_CMD_GET_AGING1_LOAD_SOC,
	FG_DAEMON_CMD_GET_AGING1_UPDATE_SOC,
	FG_DAEMON_CMD_GET_SHUTDOWN_SYSTEM_VOLTAGE,
	FG_DAEMON_CMD_GET_CHARGE_TRACKING_TIME,
	FG_DAEMON_CMD_GET_DISCHARGE_TRACKING_TIME,
	FG_DAEMON_CMD_GET_SHUTDOWN_GAUGE0,
	FG_DAEMON_CMD_GET_SHUTDOWN_GAUGE1_XMINS,
	FG_DAEMON_CMD_GET_SHUTDOWN_GAUGE1_MINS,
	FG_DAEMON_CMD_SET_SUSPEND_TIME,
	FG_DAEMON_CMD_SET_WAKEUP_SMOOTH_TIME,
	FG_DAEMON_CMD_SET_IS_CHARGING,
	FG_DAEMON_CMD_SET_RBAT,
	FG_DAEMON_CMD_SET_SWOCV,
	FG_DAEMON_CMD_SET_DOD0,
	FG_DAEMON_CMD_SET_DOD1,
	FG_DAEMON_CMD_SET_QMAX,
	FG_DAEMON_CMD_SET_SOC,
	FG_DAEMON_CMD_SET_UI_SOC,
	FG_DAEMON_CMD_SET_UI_SOC2,
	FG_DAEMON_CMD_SET_INIT_FLAG,
	FG_DAEMON_CMD_SET_DAEMON_PID,
	FG_DAEMON_CMD_NOTIFY_DAEMON,

	FG_DAEMON_CMD_FROM_USER_NUMBER
} FG_DAEMON_CTRL_CMD_FROM_USER;

/*[470223] merge ALPS02160943(For_JHZ6735M_65C_L_ALPS.L1.MP3.V1_P77) patch by liming.zhu.hz at 20150729 end*/
/* ============================================================ */
/* External Variables */
/* ============================================================ */

/* ============================================================ */
/* External function */
/* ============================================================ */
extern kal_int32 battery_meter_get_battery_voltage(kal_bool update);
extern kal_int32 battery_meter_get_charging_current_imm(void);
extern kal_int32 battery_meter_get_charging_current(void);
extern kal_int32 battery_meter_get_battery_current(void);
extern kal_bool battery_meter_get_battery_current_sign(void);
extern kal_int32 battery_meter_get_car(void);
extern kal_int32 battery_meter_get_battery_temperature(void);
extern kal_int32 battery_meter_get_charger_voltage(void);
extern kal_int32 battery_meter_get_battery_percentage(void);
extern kal_int32 battery_meter_initial(void);
extern kal_int32 battery_meter_reset(void);
/*[326405] battery dicharged too fast by liming.zhu.hz at 20150614 bigen*/
#ifdef CONFIG_CAR_TUNE_VALUE_Check
extern kal_bool battery_car_tune_low_check_in_1percentage(void);
extern kal_bool battery_car_tune_high_check_in_low_percentage(void) ;
extern kal_bool battery_car_tune_high_check_in_chargefull(void); 
extern kal_bool battery_car_tune_low_check_in_chargefull(void);
extern kal_bool battery_write_tune_cfg(int val);
extern kal_bool battery_read_tune_cfg(int * val);
extern int battery_read_file(const char* path, void* buf, size_t size);
extern int battery_write_file(const char* path, const void* buf, size_t size) ;
extern int battery_is_file_ready(const char* path) ;
#endif
/*[326405] battery dicharged too fast by liming.zhu.hz at 20150614 end*/
extern kal_int32 battery_meter_sync(kal_int32 bat_i_sense_offset);

extern kal_int32 battery_meter_get_battery_zcv(void);
extern kal_int32 battery_meter_get_battery_nPercent_zcv(void);	/* 15% zcv,  15% can be customized */
extern kal_int32 battery_meter_get_battery_nPercent_UI_SOC(void);	/* tracking point */

extern kal_int32 battery_meter_get_tempR(kal_int32 dwVolt);
extern kal_int32 battery_meter_get_tempV(void);
extern kal_int32 battery_meter_get_VSense(void);	/* isense voltage */
/*[470223] merge ALPS02160943(For_JHZ6735M_65C_L_ALPS.L1.MP3.V1_P77) patch by liming.zhu.hz at 20150729 bigen*/
extern int wakeup_fg_algo(int flow_state);
/*[470223] merge ALPS02160943(For_JHZ6735M_65C_L_ALPS.L1.MP3.V1_P77) patch by liming.zhu.hz at 20150729 end*/
#endif				/* #ifndef _BATTERY_METER_H */
