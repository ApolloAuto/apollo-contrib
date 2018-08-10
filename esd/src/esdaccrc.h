/* -*- linux-c -*-
 * FILE NAME boardrc.h
 *           copyright 2013-2014 by esd electronic system design gmbh
 *
 * BRIEF MODULE DESCRIPTION
 *           Common "boardrc ressources" for all esdACC boards
 *
 *
 * Author:   Andreas Block
 *           andreas.block@esd-electronics.com
 *
 * history:
 *  $Log$
 *  Revision 1.16  2015/10/22 08:49:41  oliver
 *  - Definition of data structures to support a message queue based communication mechanism
 *    between USB driver callback (DPC) and the CAN driver backend which is no longer limited
 *    to a shared memory region with just 256 entries.
 *    Fixes issue #2594 (http://mantis.esd/mantis/view.php?id=2594)
 *
 *  Revision 1.15  2015/07/28 12:46:06  hauke
 *  Renamed bm busload feature to bm statistic.
 *
 *  Revision 1.14  2015/05/29 12:26:07  michael
 *  LYNXOS support
 *
 *  Revision 1.13  2015/05/08 16:39:03  stefanm
 *  Rename CAN_TS and CAN_TS_FREQ to OSIF_TS and OSIF_TS_FREQ.
 *  This should now also be used instead of CAN_TIMESTAMP / TIMESTAMP
 *  in the whole esdcan driver tree.
 *
 *  Revision 1.12  2014/12/09 11:09:42  hauke
 *  Added new Features New Prescaler and Measured Busload
 *  Added BM MSG for Measured Busload (not completed)
 *
 *  Revision 1.11  2014/06/11 14:18:25  andreas
 *  Change to compile for Solaris on Ultra601
 *
 *  Revision 1.10  2014/05/19 10:26:28  andreas
 *  Renamed register mask defines to have a more unique naming throughout esdACC sources
 *  Added CAN-FD busmaster message structure
 *  Small fix to RX/TX_DONE message
 *
 *  Revision 1.9  2014/03/24 09:26:38  stefanm
 *  Corrected comment together with Andreas.
 *
 *  Revision 1.8  2014/01/27 15:26:23  andreas
 *  Added ledXor register in overview module
 *  Added masks to decode ID and DLC in RX/TX messages
 *
 *  Revision 1.7  2013/10/24 15:47:46  andreas
 *  Added BOARD_TIMER_LOCK()/UNLOCK() macros
 *  Fixed structure for busmaster and non-busmaster compilations
 *    (struct in busmaster version had redundant members)
 *
 *  Revision 1.6  2013/09/10 17:15:50  andreas
 *  Renamed busmaster message "error" into "error warn"
 *
 *  Revision 1.5  2013/08/27 13:16:53  andreas
 *  Added msiAddrOffs register
 *
 *  Revision 1.4  2013/08/14 13:40:41  andreas
 *  Feature flag states were printed regardless of verbose level
 *
 *  Revision 1.3  2013/08/14 09:33:50  andreas
 *  Added macros for version and info register
 *  Added bitmasks for feature flag decoding
 *
 *  Revision 1.2  2013/08/01 12:33:57  andreas
 *  Added MSI data register for MSI generation in overview structure
 *
 *  Revision 1.1  2013/07/15 11:55:53  andreas
 *  Header to be shared between different esdacc busmaster implementations
 *
 *
 */
/************************************************************************
 *
 *  Copyright (c) 2013 - 2013 by electronic system design gmbh
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
 *  30165 Hannover                         http://www.esd.eu
 *  Germany                                sales@esd.eu
 *
 *************************************************************************/
/*! \file esdaccrc.h
    \brief Common esdACC board ressources

    This file contains common esdACC constants and types for board specific layer.
*/
#ifndef __ESDACCRC_H__
#define __ESDACCRC_H__

/*
 * NOTE:
 * This header needs some defines from boardrc.h, BEFORE being included!
 */

#include <osif.h>

/*
 * Default level for printing decoded strappings and feature flags on driver startup
 */
#define ESDACC_ZONE_FEATURES            OSIF_ZONE_USR_RES3

#if !defined(__sun) && !defined(__LYNXOS)
#define FLAG_PRINT_STATE(features, mask)   (((features) & (mask)) ? OSIF_DPRINT((ESDACC_ZONE_FEATURES, "+")) : OSIF_DPRINT((ESDACC_ZONE_FEATURES, "-")))
#else
#define FLAG_PRINT_STATE(features, mask)                          \
        do {                                                      \
                if ((features) & (mask)) {                        \
                        OSIF_DPRINT((ESDACC_ZONE_FEATURES, "+")); \
                } else {                                          \
                        OSIF_DPRINT((ESDACC_ZONE_FEATURES, "-")); \
                }                                                 \
        } while(0)
#endif

/*
 * Defines to be used with overview registers
 */
#define REG_OV_VERSION_MINOR_GET(reg)    (UINT8)((reg) & 0x000000FF)
#define REG_OV_VERSION_MAJOR_GET(reg)    (UINT8)(((reg) & 0x0000FF00) >> 8)
#define REG_OV_VERSION_FEATURES_GET(reg) (UINT16)(((reg) & 0xFFFF0000) >> 16)

#define FEATURE_IDX_ACCEPTANCE_FILTER   0
#define FEATURE_IDX_OVERLOAD_FRAMES     1
#define FEATURE_IDX_TRIPLE_SAMPLING     2
#define FEATURE_IDX_ALC                 3
#define FEATURE_IDX_TIMESTAMPED_TX      4
#define FEATURE_IDX_LED1_CONTROL        5
#define FEATURE_IDX_LED2_CONTROL        6
#define FEATURE_IDX_BUSMASTER           7
#define FEATURE_IDX_ERROR_INJECTION     8
#define FEATURE_IDX_BROKEN_FRAMES       9
#define FEATURE_IDX_EXTERNAL_TS         10
#define FEATURE_IDX_CAN_FD              11
#define FEATURE_IDX_NEW_PRESCALER       12
#define FEATURE_IDX_STATISTIC           13
#define FEATURE_IDX_EXTEND              15   /* for extending feature mask (look at another register if set) */

#define FEATURE_MASK_ACCEPTANCE_FILTER  (1 << FEATURE_IDX_ACCEPTANCE_FILTER)
#define FEATURE_MASK_OVERLOAD_FRAMES    (1 << FEATURE_IDX_OVERLOAD_FRAMES)
#define FEATURE_MASK_TRIPLE_SAMPLING    (1 << FEATURE_IDX_TRIPLE_SAMPLING)
#define FEATURE_MASK_ALC                (1 << FEATURE_IDX_ALC)
#define FEATURE_MASK_TIMESTAMPED_TX     (1 << FEATURE_IDX_TIMESTAMPED_TX)
#define FEATURE_MASK_LED1_CONTROL       (1 << FEATURE_IDX_LED1_CONTROL)
#define FEATURE_MASK_LED2_CONTROL       (1 << FEATURE_IDX_LED2_CONTROL)
#define FEATURE_MASK_BUSMASTER          (1 << FEATURE_IDX_BUSMASTER)
#define FEATURE_MASK_ERROR_INJECTION    (1 << FEATURE_IDX_ERROR_INJECTION)
#define FEATURE_MASK_BROKEN_FRAMES      (1 << FEATURE_IDX_BROKEN_FRAMES)
#define FEATURE_MASK_EXTERNAL_TS        (1 << FEATURE_IDX_EXTERNAL_TS)
#define FEATURE_MASK_CAN_FD             (1 << FEATURE_IDX_CAN_FD)
#define FEATURE_MASK_NEW_PRESCALER      (1 << FEATURE_IDX_NEW_PRESCALER)
#define FEATURE_MASK_STATISTIC          (1 << FEATURE_IDX_STATISTIC)
#define FEATURE_MASK_EXTEND             (1 << FEATURE_IDX_EXTEND)

#define REG_OV_INFO_NUM_CAN_GET(reg)    (UINT8)((reg) & 0x000000FF)
#define REG_OV_INFO_NUM_ACTIVE_GET(reg) (UINT8)(((reg) & 0x0000FF00) >> 8)
#define REG_OV_INFO_STRAPPINGS_GET(reg) (UINT16)(((reg) & 0xFFFF0000) >> 16)
/* Defines for strapping bits and masks are to be found in boardrc.h */

/* Use in overview module's mode register */
/* NOTE: Not all bits need to be present in an FPGA implementation */
#define REG_OV_MODE_MASK_ENDIAN_LITTLE  0x00000001
#define REG_OV_MODE_MASK_BM_ENABLE      0x00000002
#define REG_OV_MODE_MASK_MODE_LED       0x00000004
/* #define REG_OV_MODE_MASK_RESERVED    0x00000008 */
#define REG_OV_MODE_MASK_TIMER          0x00000070
#define REG_OV_MODE_MASK_TIMER_ENABLE   0x00000010
#define REG_OV_MODE_MASK_TIMER_ONE_SHOT 0x00000020
#define REG_OV_MODE_MASK_TIMER_ABSOLUTE 0x00000040
#define REG_OV_MODE_MASK_TS_SRC         0x00000180  /* always set both bits at same time */
#define REG_OV_MODE_MASK_REARIO         0x00000200
#define REG_OV_MODE_MASK_REARIO_TEST    0x00000400
#define REG_OV_MODE_MASK_I2C_ENABLE     0x00000800
#define REG_OV_MODE_MASK_PXI_ST         0x00003000
#define REG_OV_MODE_MASK_PXI_ST_EN      0x00001000  /* Star trigger enable */
#define REG_OV_MODE_MASK_PXI_ST_INV     0x00002000  /* Star trigger invert */
#define REG_OV_MODE_MASK_MSI_ENABLE     0x00004000
#define REG_OV_MODE_MASK_NEW_PRESCALER  0x00008000
#define REG_OV_MODE_MASK_BM_STATISTIC   0x00010000
#define REG_OV_MODE_MASK_IRQ_CNT_DISABLE 0x00020000
/* #define REG_OV_MODE_MASK_RESERVED       0x7FFE0000 */
#define REG_OV_MODE_MASK_FPGA_RESET     0x80000000

#define REG_OV_MODE_TS_SRC_INTERN       0x00000000
#define REG_OV_MODE_TS_SRC_PXI          0x00000080
#define REG_OV_MODE_TS_SRC_IRIGB        0x00000100  /* equals IRIG-B enable */
#define REG_OV_MODE_TS_SRC_RESV         0x00000180

 /* Use in overview module's bmIrqMask register */
#define BM_IRQ_MASK_ALL                 0xAAAAAAAA
#define BM_IRQ_MASK                     0x00000002
#define BM_IRQ_UNMASK                   0x00000001


#define ESDACCRC_OVERVIEW_PARTITION_1                                           \
        /* System overview */                                                   \
        volatile UINT32 probe;             /* 0x0000 */                         \
        volatile UINT32 version;           /* 0x0004 */                         \
        volatile UINT32 info;              /* 0x0008 */                         \
        volatile UINT32 freqCore;          /* 0x000C */                         \
        volatile UINT32 freqTsLow;         /* 0x0010 */                         \
        volatile UINT32 freqTsHigh;        /* 0x0014 */                         \
        volatile UINT32 irqStatAll;        /* 0x0018 */                         \
        volatile UINT32 tsCurrentLow;      /* 0x001C */                         \
        volatile UINT32 tsCurrentHigh;     /* 0x0020 */                         \
        volatile UINT32 irigBScratch;      /* 0x0024 */                         \
        volatile UINT32 irqStatLocal;      /* 0x0028 */                         \
        volatile UINT32 mode;              /* 0x002C */                         \
        volatile UINT32 timerCompareLow;   /* 0x0030 */                         \
        volatile UINT32 timerCompareHigh;  /* 0x0034 */                         \
        volatile UINT32 timerCurrentLow;   /* 0x0038 */                         \
        volatile UINT32 timerCurrentHigh;  /* 0x003C */                         \
        volatile UINT32 timerCalib;        /* 0x0040 */                         \
        volatile UINT32 ledXor;            /* 0x0044 */                         \
        UINT32          p1_reserved1[10];  /* 0x0048-0x006C */                  \
        volatile UINT32 bmIrqCnt;          /* 0x0070 */                         \
        volatile UINT32 bmIrqMask;         /* 0x0074 */                         \
        UINT32          p1_reserved2[2];   /* 0x0078-0x007C */                  \
        volatile UINT32 msiData;           /* 0x0080 */                         \
        volatile UINT32 msiAddrOffs;       /* 0x0084 */                         \
        UINT32          p1_reserved3[478]  /* Pad to end of 2kB or 512 longwords per "partition"/"sub unit" */


/*
 * Messages used in busmaster FIFOs.
 * Data is already swapped correctly by FPGA to host endianess
 */
#define BM_MSG_FIFO_NUM_MSGS            256  /* Size in messages */

#define BM_MSG_ID_RXTX_DONE             0x01
#define BM_MSG_ID_TX_ABORT              0x02
#define BM_MSG_ID_OVERRUN               0x03
#define BM_MSG_ID_BUS_ERROR             0x04
#define BM_MSG_ID_ERROR_PASSIVE         0x05
#define BM_MSG_ID_ERROR_WARN            0x06
#define BM_MSG_ID_TIMESLICE             0x07
#define BM_MSG_ID_HW_TIMER              0x08
#define BM_MSG_ID_HOTPLUG               0x09
#define BM_MSG_ID_CAN_FD_DATA_0         0x0A
#define BM_MSG_ID_CAN_FD_DATA_1         0x0B
#define BM_MSG_ID_STATISTIC             0x0C

typedef struct _BM_MSG_CAN_ANY             BM_MSG_CAN_ANY;
typedef struct _BM_MSG_CAN_RXTX_DONE       BM_MSG_CAN_RXTX_DONE;
typedef struct _BM_MSG_CAN_FD_DATA         BM_MSG_CAN_FD_DATA;
typedef struct _BM_MSG_CAN_TX_ABORT        BM_MSG_CAN_TX_ABORT;
typedef struct _BM_MSG_CAN_OVERRUN         BM_MSG_CAN_OVERRUN;
typedef struct _BM_MSG_CAN_BUS_ERROR       BM_MSG_CAN_BUS_ERROR;
typedef struct _BM_MSG_CAN_ERROR_PASSIVE   BM_MSG_CAN_ERROR_PASSIVE;
typedef struct _BM_MSG_CAN_ERROR_WARN      BM_MSG_CAN_ERROR_WARN;
typedef struct _BM_MSG_CAN_TIMESLICE       BM_MSG_CAN_TIMESLICE;
typedef struct _BM_MSG_CAN_STATISTIC       BM_MSG_CAN_STATISTIC;
typedef struct _BM_MSG_OV_HW_TIMER         BM_MSG_OV_HW_TIMER;
typedef struct _BM_MSG_OV_HOTPLUG          BM_MSG_OV_HOTPLUG;
typedef struct _BM_MSG_STATISTIC           BM_MSG_STATISTIC;
typedef union  _BM_MSG                     BM_MSG;
typedef UINT32  BM_MSG_PLAIN[8];

typedef struct _BM_STUFF                   BM_STUFF;
typedef struct _BM_USB_STUFF               BM_USB_STUFF;

struct _BM_MSG_CAN_RXTX_DONE {
        UINT8   msgId;
        UINT8   txFifoLevel;
        UINT8   reserved1[2];
        UINT8   txTsFifoLevel;
        UINT8   reserved2[3];
        UINT32  id;
        union {
                UINT32  ui; /* Don't use longword access, as dlc is NOT swapped to host endianess in FPGA */
                struct {
                        UINT8   len;
                        UINT8   reserved0;
                        UINT8   bits;
                        UINT8   state;
                } rxtx;
                struct {
                        UINT8   len;
                        UINT8   msg_lost;       /* TODO                                  */
                        UINT8   bits;
                        UINT8   state;
                } rx;
                struct {
                        UINT8   len;
                        UINT8   txFifoIdx;
                        UINT8   bits;
                        UINT8   state;
                } tx;
                /* BL TODO: since byte four is used for state bits, we are missing room for an ECC byte */
        } dlc;
        UINT8   data[8];
        OSIF_TS timestamp;
};

#define RXTX_LEN_MASK_LENGTH    0x0F
#define RXTX_LEN_BIT_RTR        0x10
#define RXTX_LEN_BIT_TX         0x20    /* TX Done */
#define RXTX_LEN_BIT_SR         0x40    /* Self reception */
#define RXTX_LEN_BIT_TXFIFO     0x80    /* Frame was transmitted through TX FIFO (0) or TX TS FIFO (1) */

#define RXTX_STATE_RESERVED     0x3F
#define RXTX_STATE_CAN_FD       0x40
#define RXTX_STATE_OVERRUN      0x80

#define RXTX_ID_MASK_ID         0x2FFFFFFF  /* Mask for part of ID passed through to NTCAN */
#define RXTX_ID_BIT_20B         0x20000000

struct _BM_MSG_CAN_FD_DATA {
        UINT8   msgId;
        UINT8   reserved1[3];
        union {
                UINT8   ui8[28];
                UINT32  ui32[7];
        } d;
};

struct _BM_MSG_CAN_TX_ABORT {
        UINT8   msgId;
        UINT8   txFifoLevel;
        UINT16  abortMask;
        UINT8   txTsFifoLevel;
        UINT8   reserved2[1];
        UINT16  abortMaskTxTs;
        OSIF_TS ts;
        UINT32  reserved3[4];
};

struct _BM_MSG_CAN_OVERRUN {
        UINT8   msgId;
        UINT8   txFifoLevel;
        UINT8   lostCnt;  /* TODO */
        UINT8   reserved1;
        UINT8   txTsFifoLevel;
        UINT8   reserved2[3];
        OSIF_TS ts;
        UINT32  reserved3[4];
};

struct _BM_MSG_CAN_BUS_ERROR {
        UINT8   msgId;
        UINT8   txFifoLevel;
        UINT8   ecc;
        UINT8   reserved1;
        UINT8   txTsFifoLevel;
        UINT8   reserved2[3];
        OSIF_TS ts;
        UINT32  regStatus;
        UINT32  regBtr;
        UINT32  reserved3[2];
};

struct _BM_MSG_CAN_ERROR_PASSIVE {
        UINT8   msgId;
        UINT8   txFifoLevel;
        UINT8   reserved1[2];
        UINT8   txTsFifoLevel;
        UINT8   reserved2[3];
        OSIF_TS ts;
        UINT32  regStatus;
        UINT32  reserved3[3];
};

struct _BM_MSG_CAN_ERROR_WARN {
        UINT8   msgId;
        UINT8   txFifoLevel;
        UINT8   reserved1[2];
        UINT8   txTsFifoLevel;
        UINT8   reserved2[3];
        OSIF_TS ts;
        UINT32  regStatus;
        UINT32  reserved3[3];
};

struct _BM_MSG_CAN_TIMESLICE {
        UINT8   msgId;
        UINT8   txFifoLevel;
        UINT8   reserved1[2];
        UINT8   txTsFifoLevel;
        UINT8   reserved2[3];
        OSIF_TS ts;
        UINT32  reserved3[4];
};

struct _BM_MSG_CAN_STATISTIC {
        UINT8   msgId;
        UINT8   txFifoLevel;
        UINT8   reserved1[2];
        UINT8   txTsFifoLevel;
        UINT8   reserved2[3];
        OSIF_TS ts;
        UINT32  regStatus;
        UINT32  idleCnt;
        UINT32  reserved3[2];
};

struct _BM_MSG_CAN_ANY {
        UINT8   msgId;
        UINT8   txFifoLevel;
        UINT8   reserved1[2];
        UINT8   txTsFifoLevel;
        UINT8   reserved2[3];
        OSIF_TS ts;     /* any except RX/TX done */
        UINT32  reserved3[4];
};

struct _BM_MSG_OV_HW_TIMER {
        UINT8   msgId;
        UINT8   reserved1[3];
        UINT32  reserved2[1];
        OSIF_TS timer;
        UINT32  reserved3[4];
};

struct _BM_MSG_OV_HOTPLUG {
        UINT8   msgId;
        UINT8   reserved1[3];
        UINT32  reserved2[7];
};

union _BM_MSG {
        BM_MSG_PLAIN                plain;
        BM_MSG_CAN_ANY              canAny;
        BM_MSG_CAN_RXTX_DONE        rxtxDone;
        BM_MSG_CAN_FD_DATA          canFdData;
        BM_MSG_CAN_TX_ABORT         txAbort;
        BM_MSG_CAN_OVERRUN          overrun;
        BM_MSG_CAN_BUS_ERROR        busError;
        BM_MSG_CAN_ERROR_PASSIVE    errorPassive;
        BM_MSG_CAN_ERROR_WARN       errorWarn;
        BM_MSG_CAN_TIMESLICE        timeslice;
        BM_MSG_CAN_STATISTIC        statistic;
        BM_MSG_OV_HW_TIMER          hwTimer;
        BM_MSG_OV_HOTPLUG           hotplug;
};

typedef BM_MSG  BM_IRQ_MSG_FIFO[BM_MSG_FIFO_NUM_MSGS];

#if defined(BOARD_USB)
struct _BM_USB_STUFF {
    /* USB related stuff */
    LNK         lnk;            /* Linked list of pending BM messages */
    OSIF_DPC    dpc;            /* DPC of backends                    */
    UINT32      msg_lost;       /* Message lost counter               */
};

typedef struct _BM_USB_MSG {
    LNK             lnk;
    BM_MSG          bm_msg;
} BM_USB_MSG;

#else
struct _BM_STUFF {
        /* busmaster stuff */
        volatile UINT32 *pBmIrqCntNew;    /* IRQ counter delivered by FPGA (above busmaster FIFOs) */
        UINT32           bmIrqCntLocal;   /* IRQ counter as far as dpc already processed the bm messages */
        UINT32           bmFifoTail;      /* FIFO tail pointer to next message, which needs to be processed */
        BM_IRQ_MSG_FIFO *pBmFifo;
        OSIF_DPC         dpc;             /* only used for CAN cores */
};
#endif

#ifndef ESDACC_USE_NO_HW_TIMER
/*
 *  HW Timer stuff
 */
#ifdef BOARD_BUSMASTER
#define BOARD_TIMER_LOCK(pHwTimer)         OSIF_MUTEX_LOCK(&(pHwTimer)->lockTimer)
#define BOARD_TIMER_UNLOCK(pHwTimer)       OSIF_MUTEX_UNLOCK(&(pHwTimer)->lockTimer)
#else
#define BOARD_TIMER_LOCK(pHwTimer)         OSIF_IRQ_MUTEX_LOCK(&(pHwTimer)->lockTimerIrq)
#define BOARD_TIMER_UNLOCK(pHwTimer)       OSIF_IRQ_MUTEX_UNLOCK(&(pHwTimer)->lockTimerIrq)
#endif /* ifdef BOARD_BUSMASTER */

typedef struct _CIF_TIMER_BASE  CIF_TIMER_BASE;
typedef struct _CIF_TIMER       CIF_TIMER;

struct _CIF_TIMER_BASE {
        LNK             lnkTimer;
        LNK             lnkTimerActive;
#ifdef BOARD_BUSMASTER
        OSIF_MUTEX      lockTimer;
#else
        OSIF_IRQ_MUTEX  lockTimerIrq;
#endif /* ifdef BOARD_BUSMASTER */
        OSIF_IRQ_MUTEX  lockTimerAccess;
        UINT32          idxCalib;
        UINT32          cntCalib; /* To be subtracted from lower 32-Bits of every timer's duration (system latency) */
        OSIF_TIMER      timerCalib;
};

struct _CIF_TIMER {
        LNK             lnkTimer;
        LNK             lnkTimerActive;
        OSIF_TS         expires;
        VOID           *pCrd;
#ifndef BOARD_BUSMASTER
        OSIF_DPC        dpc;
#endif /* ifndef BOARD_BUSMASTER */
        VOID (OSIF_CALLTYPE *func)(VOID *);
        VOID           *arg;
        volatile UINT32 flagRunning;
};
#else
typedef VOID *CIF_TIMER_BASE;
#endif /* ifndef ESDACC_USE_NO_HW_TIMER */

#endif /* #ifndef __ESDACCRC_H__ */

