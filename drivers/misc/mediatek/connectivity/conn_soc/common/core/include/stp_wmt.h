



#ifndef _STP_WMT_H
#define _STP_WMT_H










typedef enum {
    BTM_RST_OP = 0,
    BTM_DMP_OP = 1,
    BTM_GET_AEE_SUPPORT_FLAG= 2,
    BTM_MAX_OP,
}MTKSTP_BTM_WMT_OP_T;

typedef enum {
    SLEEP = 0,
    HOST_AWAKE,
    WAKEUP,
    EIRQ,
    ROLL_BACK,
    STP_PSM_MAX_ACTION
}MTKSTP_PSM_ACTION_T;




extern MTK_WCN_BOOL wmt_lib_btm_cb (
    MTKSTP_BTM_WMT_OP_T op
    );

extern INT32 wmt_lib_ps_stp_cb (
    MTKSTP_PSM_ACTION_T action
    );
extern MTK_WCN_BOOL wmt_lib_is_quick_ps_support (VOID);


#endif /* _STP_WMT_H_*/



