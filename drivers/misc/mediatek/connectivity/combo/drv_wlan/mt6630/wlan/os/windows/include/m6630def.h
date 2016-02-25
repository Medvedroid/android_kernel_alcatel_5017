




#ifndef _M6630DEF_H
#define _M6630DEF_H



/* NIC driver information */
#define NIC_VENDOR                      "MediaTek Inc."
#define NIC_VENDOR_OUI                  {0x00, 0x0C, 0xE7}

/* #define NIC_DEVICE_ID                   "MT6630" */
#define NIC_PRODUCT_NAME                "MediaTek Inc. MT6630 Wireless LAN Adapter"
#define NIC_DRIVER_NAME                 "MediaTek Inc. MT6630 Wireless LAN Adapter Driver"

#ifdef WINDOWS_CE
#ifdef _HIF_SDIO
#define FILE_NAME                   "MT6630SD.DLL"
#endif
#ifdef _HIF_EHPI
#define FILE_NAME                   "MT6630EH.DLL"
#endif
#ifdef _HIF_SPI
#define FILE_NAME                   "MT6630SP.DLL"
#endif
#else
#ifdef _HIF_SDIO
#define FILE_NAME                   "MT6630SDx.DLL"
#endif
#ifdef _HIF_EHPI
#define FILE_NAME                   "MT6630EHx.DLL"
#endif
#endif

#define NIC_DRIVER_INTERNAL_NAME        FILE_NAME
#define NIC_DRIVER_ORIGINAL_FILE_NAME   FILE_NAME


#ifdef WINDOWS_CE
#define NIC_DRIVER_FILE_DESCRIPTION "NDIS 5.1/5.0 WINCE Driver"
#else
#ifdef NDIS51_MINIPORT
#define NIC_DRIVER_FILE_DESCRIPTION "NT 5 (NDIS 5.1/5.0) x86 Driver"
#endif

#ifdef NDIS50_MINIPORT
#error "No support for NDIS 5.0"
#endif
#endif






#endif				/* _M6630DEF_H */
