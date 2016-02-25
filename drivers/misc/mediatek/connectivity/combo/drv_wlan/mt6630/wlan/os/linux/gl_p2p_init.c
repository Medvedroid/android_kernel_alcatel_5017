





#include "precomp.h"



#define P2P_MODE_INF_NAME "p2p%d";
#define AP_MODE_INF_NAME "ap%d";
/* #define MAX_INF_NAME_LEN 15 */
/* #define MIN_INF_NAME_LEN 1 */

#define RUNNING_P2P_MODE 0
#define RUNNING_AP_MODE 1




static PUCHAR ifname = P2P_MODE_INF_NAME;
static UINT_16 mode = RUNNING_P2P_MODE;





/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
VOID p2pCheckInterfaceName(VOID)
{

	if (mode) {
		mode = RUNNING_AP_MODE;
		ifname = AP_MODE_INF_NAME;
	}
#if 0
	UINT_32 ifLen = 0;

	if (ifname) {
		ifLen = strlen(ifname);

		if (ifLen > MAX_INF_NAME_LEN) {
			ifname[MAX_INF_NAME_LEN] = '\0';
		} else if (ifLen < MIN_INF_NAME_LEN) {
			ifname = P2P_MODE_INF_NAME;
		}
	} else {
		ifname = P2P_MODE_INF_NAME;
	}
#endif
}

VOID p2pSetSuspendMode(P_GLUE_INFO_T prGlueInfo, BOOLEAN fgEnable)
{
	struct net_device *prDev = NULL;

	if (!prGlueInfo) {
		return;
	}

	if (!prGlueInfo->prAdapter->fgIsP2PRegistered) {
		DBGLOG(INIT, INFO, ("%s: P2P is not enabled, SKIP!\n", __func__));
		return;
	}

	prDev = prGlueInfo->prP2PInfo->prDevHandler;
	if (!prDev) {
		DBGLOG(INIT, INFO, ("%s: P2P dev is not availiable, SKIP!\n", __func__));
		return;
	}

	kalSetNetAddressFromInterface(prGlueInfo, prDev, fgEnable);
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN p2pLaunch(P_GLUE_INFO_T prGlueInfo)
{
	printk("p2p Launch\n");

	if (prGlueInfo->prAdapter->fgIsP2PRegistered == TRUE) {
		printk("p2p already registered\n");
		return FALSE;
	} else if (glRegisterP2P(prGlueInfo, ifname, (BOOLEAN) mode)) {
		prGlueInfo->prAdapter->fgIsP2PRegistered = TRUE;
		printk("Launch success, fgIsP2PRegistered TRUE.\n");

		return TRUE;
	} else {
		printk("Launch Fail\n");
	}

	return FALSE;
}


VOID p2pSetMode(IN BOOLEAN fgIsAPMOde)
{
	if (fgIsAPMOde) {
		mode = RUNNING_AP_MODE;
		ifname = AP_MODE_INF_NAME;
	} else {
		mode = RUNNING_P2P_MODE;
		ifname = P2P_MODE_INF_NAME;
	}

	return;
}				/* p2pSetMode */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
BOOLEAN p2pRemove(P_GLUE_INFO_T prGlueInfo)
{
	printk("p2p Remove\n");

	if (prGlueInfo->prAdapter->fgIsP2PRegistered == FALSE) {
		printk("p2p is not Registered.\n");
		return FALSE;
	} else {
		prGlueInfo->prAdapter->fgIsP2PRegistered = FALSE;
		glUnregisterP2P(prGlueInfo);
		/*p2p is removed successfully */
		return TRUE;
	}

	return FALSE;
}

#if 0
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
static int initP2P(void)
{
	P_GLUE_INFO_T prGlueInfo;

	/*check interface name validation */
	p2pCheckInterfaceName();

	printk(KERN_INFO DRV_NAME "InitP2P, Ifname: %s, Mode: %s\n", ifname, mode ? "AP" : "P2P");

	/*register p2p init & exit function to wlan sub module handler */
	wlanSubModRegisterInitExit(p2pLaunch, p2pRemove, P2P_MODULE);

	/*if wlan is not start yet, do nothing
	 * p2pLaunch will be called by txthread while wlan start
	 */
	/*if wlan is not started yet, return FALSE */
	if (wlanExportGlueInfo(&prGlueInfo)) {
		wlanSubModInit(prGlueInfo);
		return prGlueInfo->prAdapter->fgIsP2PRegistered ? 0 : -EIO;
	}

	return 0;
}				/* end of initP2P() */


/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/* 1 Module Leave Point */
static VOID __exit exitP2P(void)
{
	P_GLUE_INFO_T prGlueInfo;

	printk(KERN_INFO DRV_NAME "ExitP2P\n");

	/*if wlan is not started yet, return FALSE */
	if (wlanExportGlueInfo(&prGlueInfo)) {
		wlanSubModExit(prGlueInfo);
	}
	/*UNregister p2p init & exit function to wlan sub module handler */
	wlanSubModRegisterInitExit(NULL, NULL, P2P_MODULE);
}				/* end of exitP2P() */
#endif
