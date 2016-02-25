


#ifndef _OSAL_TYPEDEF_H_
#define _OSAL_TYPEDEF_H_

#include <linux/version.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/sched.h>
#include <linux/poll.h>
#include <asm/current.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/proc_fs.h>
#include <linux/workqueue.h>
#include <linux/wait.h>
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/vmalloc.h>
#include <linux/firmware.h>
#include <linux/kthread.h>
#include <linux/jiffies.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/earlysuspend.h>
#include <linux/suspend.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#ifdef WMT_PLAT_ALPS
#include <linux/aee.h>
#endif
#include <linux/kfifo.h>
#include <linux/wakelock.h>
#include <linux/log2.h>


#ifndef _TYPEDEFS_H     /*fix redifine*/
typedef char		INT8;
#endif



typedef void VOID, *PVOID, **PPVOID;
typedef char		*PINT8, **PPINT8;
typedef short		INT16, *PINT16, **PPINT16;
typedef int		INT32, *PINT32, **PPINT32;
typedef long long	INT64, *PINT64, **PPINT64;

typedef unsigned char		UINT8, *PUINT8, **PPUINT8;
typedef unsigned short		UINT16, *PUINT16, **PPUINT16;
typedef unsigned int		UINT32, *PUINT32, **PPUINT32;
typedef unsigned long long	UINT64, *PUINT64, **PPUINT64;

typedef size_t SIZE_T;

typedef int MTK_WCN_BOOL;
#ifndef MTK_WCN_BOOL_TRUE
#define MTK_WCN_BOOL_FALSE               ((MTK_WCN_BOOL) 0)
#define MTK_WCN_BOOL_TRUE                ((MTK_WCN_BOOL) 1)
#endif

#endif /*_OSAL_TYPEDEF_H_*/

