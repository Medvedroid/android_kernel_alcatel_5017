
#ifdef DFT_TAG
#undef DFT_TAG
#endif
#define DFT_TAG         "[ANT-MOD-INIT]"

#include "wmt_detect.h"
#include "ant_drv_init.h"


int do_ant_drv_init(int chip_id)
{
	int i_ret = -1;
	WMT_DETECT_INFO_FUNC("start to do ANT driver init \n");
	switch (chip_id)
	{
		case 0x6630:
			i_ret = mtk_wcn_stpant_drv_init();
			WMT_DETECT_INFO_FUNC("finish ANT driver init, i_ret:%d\n", i_ret);
			break;
		default:
			WMT_DETECT_ERR_FUNC("chipid is not 6630,ANT is not supported!\n");
	}
	return i_ret;
}


