
/*file: stp_btif, mainly control stp & btif interaction*/


#ifdef DFT_TAG
#undef DFT_TAG
#endif
#define DFT_TAG "[STP-BTIF]"


#include "osal_typedef.h"
#include "wmt_exp.h"
#include "stp_exp.h"
#include "stp_btif.h"

#include <asm/current.h>
#define BTIF_OWNER_NAME "CONSYS_STP"

#define STP_MAX_PACKAGE_ALLOWED (2000)

#define STP_BTIF_TX_RTY_LMT (10)
#define STP_BTIF_TX_RTY_DLY (5)

SIZE_T stpBtifId = 0;
SIZE_T *pBtifRef = &stpBtifId;









INT32 mtk_wcn_consys_stp_btif_open(VOID)
{
	INT32 iRet = -1;
	
	iRet = mtk_wcn_btif_open(BTIF_OWNER_NAME,pBtifRef);
	if(iRet){
		WMT_WARN_FUNC("STP open btif fail(%d)\n",iRet);
		return -1;
	}else{
		WMT_INFO_FUNC("STP open bitf OK\n");
	}
	
	mtk_wcn_stp_register_if_tx(STP_BTIF_IF_TX,  (MTK_WCN_STP_IF_TX)mtk_wcn_consys_stp_btif_tx);
	
	return 0;
}
INT32 mtk_wcn_consys_stp_btif_close(VOID)
{
	INT32 iRet = 0;

	if(!stpBtifId){
		WMT_WARN_FUNC("NULL BTIF ID reference!\n");
		iRet = -1;
	}else{
		iRet = mtk_wcn_btif_close(stpBtifId);
		if(iRet){
			WMT_WARN_FUNC("STP close btif fail(%d)\n",iRet);
			iRet = -2;
		}else{
			stpBtifId = 0;
			WMT_INFO_FUNC("STP close btif OK\n");
		}
	}
	return iRet;
}
INT32 mtk_wcn_consys_stp_btif_rx_cb_register(MTK_WCN_BTIF_RX_CB rx_cb)
{
	INT32 iRet = 0;

	if(!stpBtifId){
		WMT_WARN_FUNC("NULL BTIF ID reference\n!");
		iRet = -1;
	}else{
		iRet = mtk_wcn_btif_rx_cb_register(stpBtifId,rx_cb);
		if(iRet){
			WMT_WARN_FUNC("STP register rxcb to btif fail(%d)\n",iRet);
			iRet = -2;
		}else{
			WMT_INFO_FUNC("STP register rxcb to  btif OK\n");
		}
	}
	return iRet;
}
INT32 mtk_wcn_consys_stp_btif_tx(const UINT8 *pBuf,const UINT32 len,UINT32 *written_len)
{
	INT32 retry_left = STP_BTIF_TX_RTY_LMT;
	INT32 wr_count = 0;
	INT32 written = 0;
	
	if(!stpBtifId){
		WMT_WARN_FUNC("NULL BTIF ID reference!\n");
		return -1;
	}

	if(len == 0)
	{
		*written_len = 0;
		WMT_INFO_FUNC("special case for STP-CORE,pbuf(%p)\n",pBuf);
		return 0;
	}

	*written_len = 0;
	
	if(len < 0 || len > STP_MAX_PACKAGE_ALLOWED){
		WMT_WARN_FUNC("abnormal pacage length,len(%d),pid[%d/%s]\n",len,current->pid,current->comm);
		return -2;
	}
	wr_count = mtk_wcn_btif_write(stpBtifId, pBuf, len);

	if(wr_count < 0)
	{
		WMT_ERR_FUNC("mtk_wcn_btif_write err(%d)\n",wr_count);
		*written_len = 0;
		return -3;
	}
	if(wr_count == len){
		/*perfect case*/
		*written_len = wr_count;
		return wr_count;
	}

	while((retry_left--)&&(wr_count < len)){
		osal_sleep_ms(STP_BTIF_TX_RTY_DLY);
		written = mtk_wcn_btif_write(stpBtifId, pBuf + wr_count, len - wr_count);
		if(written < 0)
		{
			WMT_ERR_FUNC("mtk_wcn_btif_write err(%d)when do recovered\n",written);
			break;
		}
		wr_count += written;
	}

	if(wr_count == len){
		WMT_INFO_FUNC("recovered,len(%d),retry_left(%d)\n",len,retry_left);
		/*recovered case*/
		*written_len = wr_count;
		return wr_count;
	}

	WMT_ERR_FUNC("stp btif write fail,len(%d),written(%d),retry_left(%d),pid[%d/%s]\n",
		len,wr_count,retry_left,current->pid,current->comm);
	*written_len = 0;
	return -wr_count;
}
INT32 mtk_wcn_consys_stp_btif_rx(UINT8 *pBuf,UINT32 len)
{
	return 0;
}

INT32 mtk_wcn_consys_stp_btif_wakeup(VOID)
{
	INT32 iRet = 0;
	
	if(!stpBtifId){
		WMT_WARN_FUNC("NULL BTIF ID reference!\n");
		iRet = -1;
	}else{
		iRet = mtk_wcn_btif_wakeup_consys(stpBtifId);
		if(iRet){
			WMT_WARN_FUNC("STP btif wakeup consys fail(%d)\n",iRet);
			iRet = -2;
		}
		else{
			WMT_DBG_FUNC("STP btif wakeup consys ok\n");
		}
	}

	return iRet;
}

INT32 mtk_wcn_consys_stp_btif_dpidle_ctrl(ENUM_BTIF_DPIDLE_CTRL en_flag)
{
	INT32 iRet = 0;

	if(!stpBtifId){
		WMT_WARN_FUNC("NULL BTIF ID reference!\n");
		iRet = -1;
	}else{
		mtk_wcn_btif_dpidle_ctrl(stpBtifId,en_flag);
		WMT_DBG_FUNC("stp btif dpidle ctrl done,en_flag(%d)\n",en_flag);
	}

	return iRet;
}

INT32 mtk_wcn_consys_stp_btif_lpbk_ctrl(ENUM_BTIF_LPBK_MODE mode)
{
	INT32 iRet = 0;
	
	if(!stpBtifId){
		WMT_WARN_FUNC("NULL BTIF ID reference!\n");
		iRet = -1;
	}else{
		iRet = mtk_wcn_btif_loopback_ctrl(stpBtifId,mode);
		if(iRet){
			WMT_WARN_FUNC("STP btif lpbk ctrl fail(%d)\n",iRet);
			iRet = -2;
		}else{
			WMT_INFO_FUNC("stp btif lpbk ctrl ok,mode(%d)\n",mode);
		}
	}

	return iRet;
}

INT32 mtk_wcn_consys_stp_btif_logger_ctrl(ENUM_BTIF_DBG_ID flag)
{
	INT32 iRet = 0;
	
	if(!stpBtifId){
		WMT_WARN_FUNC("NULL BTIF ID reference!\n");
		iRet = -1;
	}else{
		iRet = mtk_wcn_btif_dbg_ctrl(stpBtifId,flag);
		if(iRet){
			WMT_WARN_FUNC("STP btif log dbg ctrl fail(%d)\n",iRet);
			iRet = -2;
		}else{
			WMT_INFO_FUNC("stp btif log dbg ctrl ok,flag(%d)\n",flag);
		}
	}

	return iRet;
}

INT32 mtk_wcn_consys_stp_btif_parser_wmt_evt(const UINT8 *str,UINT32 len)
{
	if(!stpBtifId){
		WMT_WARN_FUNC("NULL BTIF ID reference!\n");
		return -1;
	}else{
		return (INT32)mtk_wcn_btif_parser_wmt_evt(stpBtifId, str, len);
	}
}

