/* -*- esdcan-c -*-
 * FILE NAME cm.h
 *           copyright 2002-2015 by esd electronic system design gmbh
 *
 * BRIEF MODULE DESCRIPTION
 *
 *
 *
 * history:
 *
 *  $Log$
 *  Revision 1.63  2015/05/27 13:32:43  stefanm
 *  Avoid name clash under LynxOS with 96-bit version structure now named VERSION96.
 *
 *  Revision 1.62  2015/05/08 16:39:08  stefanm
 *  Rename CAN_TS and CAN_TS_FREQ to OSIF_TS and OSIF_TS_FREQ.
 *  This should now also be used instead of CAN_TIMESTAMP / TIMESTAMP
 *  in the whole esdcan driver tree.
 *
 *  Revision 1.61  2014/09/26 12:06:22  michael
 *  Bugfix: FD Drivers have compile errors
 *
 *  Revision 1.60  2014/09/26 11:00:54  michael
 *  OBJ-MODE for RX rewritten. 11 Bit CAN-ID limit removed.
 *
 *  Revision 1.59  2014/09/15 08:38:09  oliver
 *  - Added ESDCAN_CTL_CAN_INFO
 *  - Extended CARD_IDENT structure for USB devices with endpoint description.
 *
 *  Revision 1.58  2014/07/24 15:24:13  hauke
 *  Changed from BAUDRATE_FD to BAUDRATE_X
 *
 *  Revision 1.57  2014/07/04 10:12:23  oliver
 *  - Removed include of <osif.h>.
 *  - Added sanity check that <boardrc.h> is already included.
 *  - Moved C_ASSERT macro into osif.h.
 *
 *  Revision 1.56  2014/07/04 09:37:17  hauke
 *  Moved CAN_TS and CAN_TS_FREQ from cm.h to osif.h
 *  Moved LNK definitions and macros from cm.h to osif.h
 *
 *  Revision 1.55  2014/06/06 13:45:58  michael
 *  CAN FD Support: NULL-Device, Driver Kernel and QNX Layer. Untested and not ready.
 *
 *  Revision 1.54  2013/08/27 13:38:43  andreas
 *  Added defines for driver parameters
 *
 *  Revision 1.53  2013/08/16 12:31:19  andreas
 *  Fixed online comments (// does not compile well under VxWorks)
 *
 *  Revision 1.52  2013/05/22 13:52:15  oliver
 *  Fixed MSC compiler warning if /Wp4 is enabled.
 *
 *  Revision 1.51  2013/05/16 13:50:37  frank
 *  Adaptions for CAN-USB/2 with OnTime RTOS-32
 *
 *  Revision 1.50  2013/01/07 15:34:24  andreas
 *  Added ESDCAN_CTL_TX_TS_WIN_SET, ESDCAN_CTL_TX_TS_WIN_GET, ESDCAN_CTL_TX_TS_TIMEOUT_SET, ESDCAN_CTL_TX_TS_TIMEOUT_GET
 *
 *  Revision 1.49  2013/01/03 16:17:27  andreas
 *  Updated copyright notice
 *
 *  Revision 1.48  2012/11/22 12:38:00  andreas
 *  Improved C_ASSERT macro
 *
 *  Revision 1.47  2011/10/24 14:34:43  andreas
 *  changed copyright notice
 *
 *  Revision 1.46  2011/08/22 13:43:26  hauke
 *  added errorInjection ioctls
 *
 *  Revision 1.45  2011/06/20 18:28:21  manuel
 *  Added CARD_IDENT_TERMINATE, removed CARD_IDENT_xxx_TERMINATE
 *
 *  Revision 1.44  2011/04/05 10:29:36  andreas
 *  Changed to make use of new CHAR8 data type
 *
 *  Revision 1.43  2010/12/10 16:36:48  andreas
 *  Added ESDCAN_CTL_RESET_CAN_ERROR_CNT
 *
 *  Revision 1.42  2010/08/30 11:59:56  andreas
 *  Added C_ASSERT macro to do compile time size checking of data types
 *
 *  Revision 1.41  2010/04/20 12:47:04  andreas
 *  Removed Win32 compile time warning
 *
 *  Revision 1.40  2010/04/16 16:20:45  andreas
 *  Moved most of linked list stuff from nucleus into this file
 *
 *  Revision 1.39  2009/07/31 14:14:55  andreas
 *  Add ESDCAN_CTL_SER_REG_READ and ESDCAN_CTL_SER_REG_WRITE
 *  Untabbified
 *
 *  Revision 1.38  2009/02/25 15:45:52  andreas
 *  Removed CAN_STAT structure (now in canio.h)
 *  Added ESDCAN_CTL_BUS_STATISTIC_GET, ESDCAN_CTL_BUS_STATISTIC_RESET,
 *    ESDCAN_CTL_ERROR_COUNTER_GET, ESDCAN_CTL_BITRATE_DETAILS_GET
 *
 *  Revision 1.37  2008/02/28 15:44:25  michael
 *  ESDCAN_CTL_DEBUG added.
 *
 *  Revision 1.36  2007/12/13 14:02:24  michael
 *  Member countStart/countStop added to CMSCHED
 *
 *  Revision 1.35  2007/11/05 14:44:38  andreas
 *  Added ESDCAN_CTL_BUSLOAD_INTERVAL_GET and ESDCAN_CTL_BUSLOAD_INTERVAL_SET
 *
 *  Revision 1.34  2006/11/22 10:29:45  andreas
 *  Fixed (already deprecated) FILTER_OBJ_RTR_ENABLE define
 *
 *  Revision 1.33  2006/07/11 15:12:41  manuel
 *  Added ts and ts_freq to CAN_STAT
 *
 *  Revision 1.32  2006/06/27 09:53:56  andreas
 *  Added  ESDCAN_CTL_BAUDRATE_AUTO and ESDCAN_CTL_BAUDRATE_BTR
 *  Added baud to CAN_STAT (quick fix in order to remove nuc_baudrate_get())
 *
 *  Revision 1.31  2005/09/29 07:22:47  michael
 *  internal ioctl-codes added
 *
 *  Revision 1.30  2005/09/14 13:25:36  manuel
 *  Added CM_LEN2DATALEN macro
 *
 *  Revision 1.29  2005/07/27 15:53:01  andreas
 *  Added CARD_IDENT structure (see some boardrc.c/h for usage info)
 *
 *  27.05.02 - first version                         mf
 *
 */
/************************************************************************
 *
 *  Copyright (c) 1996 - 2013 by electronic system design gmbh
 *
 *  This software is copyrighted by and is the sole property of
 *  esd gmbh.  All rights, title, ownership, or other interests
 *  in the software remain the property of esd gmbh. This
 *  software may only be used in accordance with the corresponding
 *  license agreement.  Any unauthorized use, duplication, transmission,
 *  distribution, or disclosure of this software is expressly forbidden.
 *
 *  This Copyright notice may not be removed or modified without prior
 *  written consent of esd gmbh.
 *
 *  esd gmbh, reserves the right to modify this software without notice.
 *
 *  electronic system design gmbh          Tel. +49-511-37298-0
 *  Vahrenwalder Str 207                   Fax. +49-511-37298-68
 *  30165 Hannover                         http://www.esd-electronics.com
 *  Germany                                sales@esd-electronics.com
 *
 *************************************************************************/
/*! \file cm.h
 *  \brief Contains common CAN-message-structure (CM) and defines such as
 *  feature-flags, mode-flags. These defines are visible to the user,
 *  nevertheless: Dear user, please, use the defines in ntcan.h!
 *
 */

#ifndef __CM_H__
#define __CM_H__

#ifndef OSIF_KERNEL
#error "This file may be used in the kernel-context, only! Not for application-use!!!"
#endif

    /* Check if <boardrc.h> is included with the presence of the CARD_FEATURES macro */
#if !defined(CARD_FEATURES)
# error "Header <boardrc.h> has to be included BEFORE <cm.h> !!"
#endif

/* calculate the data length from a given frame length value */
#ifdef BOARD_CAN_FD
# define CM_LEN_MASK 0x9f
#else
# define CM_LEN_MASK 0x1f
#endif

/* command-flags for nuc_id_filter                                           */
#define FILTER_ON         0x00000000      /* enable flag                     */
#define FILTER_OFF        0x80000000      /* disable flag                    */

#define FILTER_DATA       0x00000001      /* enabled for queued data input   */
#define FILTER_RTR        0x00000002      /* enabled for queued rtr input    */
#define FILTER_OBJ        0x00000004      /* enabled for object data input   */
#define FILTER_OBJ_RTR    0x00000008      /* enabled for object rtr input    */
#define FILTER_DATA_FD    0x00000010      /* enabled for queued fd data input*/

#ifdef BOARD_CAN_FD
#define FILTER_ALL (FILTER_DATA | FILTER_RTR | FILTER_OBJ | FILTER_OBJ_RTR | FILTER_DATA_FD)
#else
#define FILTER_ALL (FILTER_DATA | FILTER_RTR | FILTER_OBJ | FILTER_OBJ_RTR)
#endif

/* Following is defined for the board files but DEPRECATED */
/* DEPRECATED BEGIN */
#define FILTER_DATA_ENABLE    FILTER_DATA
#define FILTER_RTR_ENABLE     FILTER_RTR
#define FILTER_OBJ_ENABLE     FILTER_OBJ
#define FILTER_OBJ_RTR_ENABLE FILTER_OBJ_RTR
#define FILTER_DISABLE        0x00000000
/* DEPRECATED END */

/* Driver flags (aka driver start parameter) used in node->mode */
#define DRIVER_PARAM_LOM                FEATURE_LOM
#define DRIVER_PARAM_SMART_DISCONNECT   FEATURE_SMART_DISCONNECT
#define DRIVER_PARAM_SMART_SUSPEND      0x01000000
#define DRIVER_PARAM_ESDACC_AUTOBAUD    0x02000000
#define DRIVER_PARAM_PCIE402_FORCE      0x04000000
#define DRIVER_PARAM_PXI_TRIG_INVERT    0x08000000
#define DRIVER_PARAM_ESDACC_TS_SOURCE   0x10000000
#define DRIVER_PARAM_I20_NO_FAST_MODE   0x20000000
#define DRIVER_PARAM_ESDACC_TS_MODE     0x20000000
#define DRIVER_PARAM_PLX_FIFO_MODE      0x40000000

/* Driver internal IOCTL codes */
#define ESDCAN_CTL_TIMESTAMP_GET         1
#define ESDCAN_CTL_TIMEST_FREQ_GET       2
#define ESDCAN_CTL_BAUDRATE_GET          3
#define ESDCAN_CTL_BAUDRATE_SET          4
#define ESDCAN_CTL_ID_FILTER             5
#define ESDCAN_CTL_BAUDRATE_AUTO         6
#define ESDCAN_CTL_BAUDRATE_BTR          7
#define ESDCAN_CTL_BUSLOAD_INTERVAL_GET  8
#define ESDCAN_CTL_BUSLOAD_INTERVAL_SET  9
#define ESDCAN_CTL_DEBUG                10
#define ESDCAN_CTL_BUS_STATISTIC_GET    11
#define ESDCAN_CTL_BUS_STATISTIC_RESET  12
#define ESDCAN_CTL_ERROR_COUNTER_GET    13
#define ESDCAN_CTL_BITRATE_DETAILS_GET  14
#define ESDCAN_CTL_SER_REG_READ         15
#define ESDCAN_CTL_SER_REG_WRITE        16
#define ESDCAN_CTL_RESET_CAN_ERROR_CNT  17
#define ESDCAN_CTL_EEI_CREATE           18
#define ESDCAN_CTL_EEI_DESTROY          19
#define ESDCAN_CTL_EEI_STATUS           20
#define ESDCAN_CTL_EEI_CONFIGURE        21
#define ESDCAN_CTL_EEI_START            22
#define ESDCAN_CTL_EEI_STOP             23
#define ESDCAN_CTL_EEI_TRIGGER_NOW      24
#define ESDCAN_CTL_TX_TS_WIN_SET        25
#define ESDCAN_CTL_TX_TS_WIN_GET        26
#define ESDCAN_CTL_TX_TS_TIMEOUT_SET    27
#define ESDCAN_CTL_TX_TS_TIMEOUT_GET    28
#define ESDCAN_CTL_BAUDRATE_X_GET       29
#define ESDCAN_CTL_BAUDRATE_X_SET       30
#define ESDCAN_CTL_CAN_INFO             31

typedef	union {
        UINT64 ul64[1];
        UINT32 ul32[2];
        VOID*  ptr;
} HOST_HND;


typedef struct _CMSCHED CMSCHED;
struct _CMSCHED
{
        UINT32  id;
        INT32   flags;
        OSIF_TS timeStart;
        OSIF_TS timeInterval;
        UINT32  countStart; /* Start value for counting*/
        UINT32  countStop;  /* Stop value for counting. After reaching this
                               value, the counter is loaded with the countStart value. */
};

typedef struct _CM CM;
struct _CM {
        UINT32        id;             /* can-id                                   */
        UINT8         len;            /* length of message: 0-8                   */
        UINT8         msg_lost;       /* count of lost rx-messages                */
        UINT8         reserved[1];    /* reserved                                 */
        UINT8         ecc;            /* ECC (marks "broken" frames)              */
#ifdef BOARD_CAN_FD
        UINT8         data[64];       /* 64 data-bytes                            */
#else
        UINT8         data[8];        /* 8 data-bytes                             */
#endif
        OSIF_TS       timestamp;      /* 64 bit timestamp                         */
        HOST_HND      host_hnd;
};

typedef struct _VERSION96 {
        UINT32        level;
        UINT32        revision;
        UINT32        build;
} VERSION96;

typedef struct _CARD_IRQ {
        OSIF_IRQ_HANDLER( *handler, context);
        VOID *context;
} CARD_IRQ;

/* Use this to terminate cardFlavours arrays */
#define CARD_IDENT_TERMINATE {{ NULL, NULL, NULL, {0, 0, 0, 0, 0, 0, 0} }}
/* Do not use these any more, just use the one above */
/* #define CARD_IDENT_PCI_TERMINATE {{ NULL, NULL, NULL, {0, 0, 0, 0, 0, 0, 0} }}       */
/* #define CARD_IDENT_RAW_TERMINATE {{ NULL, NULL, NULL }}                              */
/* fj todo remove usage of CARD_IDENT_USB_TERMINATE */
#define CARD_IDENT_USB_TERMINATE {{ NULL, NULL, NULL, {0, 0} }}

#define CARD_MAX_USB_ENDPOINTS      4       /* Devices may have up to 4 endpoints */

typedef union _CARD_IDENT {
        struct {
                CARD_IRQ    *irqs;
                UINT32      *spaces;
                const CHAR8 *name;
                struct {
                        UINT32   vendor;
                        UINT32   device;
                        UINT32   subVendor;
                        UINT32   subDevice;
                        UINT32   class;     /* For future use, init with 0    */
                        UINT32   classMask; /* For future use, init with 0    */
                        UINTPTR  misc;      /* For future use, init with NULL */
                } ids;
        } pci;
        struct {
                CARD_IRQ    *irqs;
                UINT32      *spaces;
                const CHAR8 *name;
        } raw;
        struct {
                CARD_IRQ    *irqs;
                UINT32      *spaces;
                const CHAR8 *name;
                struct {
                        UINT32   vendor;
                        UINT32   device;
                        UINT32   numEndpoints;
                        UINT32   endpoint[CARD_MAX_USB_ENDPOINTS];
                } desc;
        } usb;
        struct { /* This part needs to be identical for all kinds of boards */
                CARD_IRQ    *irqs;
                UINT32      *spaces;
                const CHAR8 *name;
        } all;
} CARD_IDENT;

#define MAKE_VERSION(l,r,b) (UINT16)(((l) << 12) | ((r) << 8) | ((b) << 0))

#endif  /* #ifndef __CM_H__ */
