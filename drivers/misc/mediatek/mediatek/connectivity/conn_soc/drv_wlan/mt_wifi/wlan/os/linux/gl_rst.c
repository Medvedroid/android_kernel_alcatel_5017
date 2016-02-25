





#include <linux/kernel.h>
#include <linux/workqueue.h>

#include "precomp.h"
#include "gl_rst.h"

#if CFG_CHIP_RESET_SUPPORT

BOOLEAN fgIsResetting = FALSE;

static RESET_STRUCT_T wifi_rst;

static void mtk_wifi_reset(
    struct work_struct *work
    );

static void *
glResetCallback (
    ENUM_WMTDRV_TYPE_T  eSrcType,
    ENUM_WMTDRV_TYPE_T  eDstType,
    ENUM_WMTMSG_TYPE_T  eMsgType,
    void *              prMsgBody,
    unsigned int        u4MsgLength
    );

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
glResetInit(
    VOID
    )
{
#if (MTK_WCN_SINGLE_MODULE == 0)
    /* 1. Register reset callback */
    mtk_wcn_wmt_msgcb_reg(WMTDRV_TYPE_WIFI, (PF_WMT_CB)glResetCallback);
#endif /* MTK_WCN_SINGLE_MODULE */

    /* 2. Initialize reset work */
    INIT_WORK(&(wifi_rst.rst_work), mtk_wifi_reset);

    return;
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
glResetUninit(
    VOID
    )
{
#if (MTK_WCN_SINGLE_MODULE == 0)
    /* 1. Deregister reset callback */
    mtk_wcn_wmt_msgcb_unreg(WMTDRV_TYPE_WIFI);
#endif /* MTK_WCN_SINGLE_MODULE */

    return;
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static void *
glResetCallback (
    ENUM_WMTDRV_TYPE_T  eSrcType,
    ENUM_WMTDRV_TYPE_T  eDstType,
    ENUM_WMTMSG_TYPE_T  eMsgType,
    void *              prMsgBody,
    unsigned int        u4MsgLength
    )
{
    switch (eMsgType) {
    case WMTMSG_TYPE_RESET:
        if (u4MsgLength == sizeof(ENUM_WMTRSTMSG_TYPE_T)) {
            P_ENUM_WMTRSTMSG_TYPE_T prRstMsg = (P_ENUM_WMTRSTMSG_TYPE_T) prMsgBody;

            switch (*prRstMsg) {
            case WMTRSTMSG_RESET_START:
                DBGLOG(INIT, WARN, ("Whole chip reset start!\n"));
                fgIsResetting = TRUE;
                wifi_reset_start();
                break;

            case WMTRSTMSG_RESET_END:
                DBGLOG(INIT, WARN, ("Whole chip reset end!\n"));
                fgIsResetting = FALSE;
                wifi_rst.rst_data = RESET_SUCCESS;
                schedule_work(&(wifi_rst.rst_work));
                break;

            case WMTRSTMSG_RESET_END_FAIL:
                DBGLOG(INIT, WARN, ("Whole chip reset fail!\n"));
                fgIsResetting = FALSE;
                wifi_rst.rst_data = RESET_FAIL;
                schedule_work(&(wifi_rst.rst_work));
                break;

            default:
                break;
            }
        }
        break;

    default:
        break;
    }

    return NULL;
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static void mtk_wifi_reset(
    struct work_struct *work
    )
{
    RESET_STRUCT_T *rst = container_of(work, RESET_STRUCT_T, rst_work);
    wifi_reset_end(rst->rst_data);
    return;
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID
glSendResetRequest(
    VOID
    )
{
    /* WMT thread would trigger whole chip reset itself */
    return;
}


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN
kalIsResetting(
    VOID
    )
{
    return fgIsResetting;
}


#endif /* CFG_CHIP_RESET_SUPPORT */
