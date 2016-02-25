#include <linux/kernel.h>
#include <linux/string.h>

#include <mach/mtk_rtc.h>
#include <mach/wd_api.h>

#include <mach/upmu_common.h>
extern void wdt_arch_reset(char);



void arch_reset(char mode, const char *cmd)
{
	char reboot = 0;
	int res = 0;
	struct wd_api *wd_api = NULL;
#ifdef CONFIG_FPGA_EARLY_PORTING
    return ;
#else

	res = get_wd_api(&wd_api);
	pr_notice("arch_reset: cmd = %s\n", cmd ? : "NULL");

	if (cmd && !strcmp(cmd, "charger")) {
		/* do nothing */
	} else if (cmd && !strcmp(cmd, "recovery")) {
 #ifndef CONFIG_MTK_FPGA
		rtc_mark_recovery();
        /* [defect 251249] add by bin.song.hz at 2015.5.19 for reset PMIC circuit when reboot into recovery begin */
        pmic_set_register_value(PMIC_RG_WDTRSTB_MODE, 1);
        pmic_set_register_value(PMIC_WDTRSTB_STATUS, 1);
        pmic_set_register_value(PMIC_RG_WDTRSTB_FB_EN, 1);
        /* [defect 251249] add by bin.song.hz at 2015.5.19 for reset PMIC circuit when reboot into recovery end */ 
 #endif
	} else if (cmd && !strcmp(cmd, "bootloader")) {
 #ifndef CONFIG_MTK_FPGA
		rtc_mark_fast();
        /* [defect 251249] add by bin.song.hz at 2015.5.19 for reset PMIC circuit when reboot into fastboot begin */
        pmic_set_register_value(PMIC_RG_WDTRSTB_MODE, 1);
        pmic_set_register_value(PMIC_WDTRSTB_STATUS, 1);
        pmic_set_register_value(PMIC_RG_WDTRSTB_FB_EN, 1);
        /* [defect 251249] add by bin.song.hz at 2015.5.19 for reset PMIC circuit when reboot into fastboot end */ 
 #endif
	}
#ifdef CONFIG_MTK_KERNEL_POWER_OFF_CHARGING
	else if (cmd && !strcmp(cmd, "kpoc")) {
		rtc_mark_kpoc();
	}
#endif
	else {
		reboot = 1;
	}

	if (res) {
		pr_notice("arch_reset, get wd api error %d\n", res);
	} else {
		wd_api->wd_sw_reset(reboot);
	}
 #endif
}
