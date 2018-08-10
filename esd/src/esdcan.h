/* -*- esdcan-c -*-
 * FILE NAME esdcan.h
 *           copyright 2002-2015 by esd electronic system design gmbh
 *
 * BRIEF MODULE DESCRIPTION
 *
 *
 *
 * history:
 *
 *  $Log$
 *  Revision 1.64  2015/05/29 18:14:23  stefanm
 *  Added <doNotTouch> structure member for power management (other OSes).
 *
 *  Revision 1.63  2015/05/29 17:02:49  stefanm
 *  Adaption to VERSION96 type.
 *
 *  Revision 1.62  2015/05/29 14:32:10  stefanm
 *  Now the <crd> pointer in the CAN_NODE structure has its real CAN_CARD type.
 *
 *  Revision 1.61  2015/01/13 15:31:03  stefanm
 *  Added support for NTCAN_IOCTL_GET_INFO / IOCTL_ESDCAN_GET_INFO.
 *  Will now count active nodes in <num_nodes> of card structure.
 *  New <version_firmware2> is needed for USB400 Cypress updateable FW.
 *
 *  Revision 1.60  2015/01/09 15:18:51  stefanm
 *  Provide a real board status for the canStatus() call which is kept
 *  now in the card structure.
 *
 *  Revision 1.59  2014/11/03 12:44:54  manuel
 *  Completely removed PENDING_TXOBJ states
 *  (TXOBJ does not use tx_done so esdcan layer does not need to remember it).
 *
 *  Revision 1.58  2014/10/27 07:01:34  oliver
 *  Added trx_type to _CAN_NODE structure.
 *
 *  Revision 1.57  2014/08/21 13:16:32  manuel
 *  cmbuf and cmbuf in in CAN_OCB are now CAN_MSG_X if BOARD_CAN_FD is defined
 *
 *  Revision 1.56  2014/07/04 10:01:56  hauke
 *  Removed <canio.h> which is included in osif.h and include <boardrc.h> befor <cm.h>.
 *
 *  Revision 1.55  2013/06/27 12:48:40  andreas
 *  Comment removed
 *
 *  Revision 1.54  2013/04/26 14:34:18  andreas
 *  Removed unsused cmd member from CAN_OCB structure
 *
 *  Revision 1.53  2013/01/11 18:27:24  andreas
 *  Added define TX_TS_WIN_DEF
 *  Added tx_ts_win to CAN_NODE
 *  Reworked CAN_OCB structure with rx and tx substructures (like in other esdcan layers)
 *
 *  Revision 1.52  2012/11/08 14:14:42  andreas
 *  Fixed bug on handle close (two threads entering driver on same handle with ioctl_destroy)
 *
 *  Revision 1.51  2011/11/01 15:30:02  andreas
 *  Merged with preempt_rt branch
 *  With OSIF_USE_PREEMPT_RT_IMPLEMENTATION the new implementation is used for
 *    all kernels > 2.6.20
 *  Some cleanup
 *  Updated copyright notice
 *
 *  Revision 1.50  2011/05/18 16:00:21  andreas
 *  Removed sema_rx_abort from OCB structure
 *
 *  Revision 1.49  2011/02/17 16:28:19  andreas
 *  Added another RX state flag: RX_STATE_CLOSING
 *
 *  Revision 1.48  2010/06/16 15:00:50  michael
 *  Smart id filter nucleus support.
 *  New filter not yet connected to user api.
 *  3rd trial.
 *
 *  Revision 1.47  2010/04/16 16:26:11  andreas
 *  Moved LNK forward declaration into cm.h
 *
 *  Revision 1.46  2010/03/16 10:32:12  andreas
 *  Added wqRx waitqueue to CAN_NODE structure (for select implementation)
 *
 *  Revision 1.45  2009/07/31 14:59:36  andreas
 *  Untabbified
 *  Removed some old, redundant forgotten pci405fw code
 *
 *  Revision 1.44  2009/03/02 17:50:04  andreas
 *  Fixed Linux dependency (caused by replacement of OSIF_SEMA with
 *    struct semaphore)
 *
 *  Revision 1.43  2009/02/25 16:23:18  andreas
 *  Replaced usage of OSIF_SEMA by direct use of struct semaphore
 *  Added two semaphores for RX/TX jobs which are interrupted by signal
 *
 *  Revision 1.42  2008/12/02 11:13:08  andreas
 *  Removed redundant br_info from CAN_NODE struct
 *
 *  Revision 1.41  2008/11/18 11:41:31  matthias
 *  pmc440fw also requires host handle
 *
 *  Revision 1.40  2007/11/05 14:58:54  andreas
 *  Added can_stat to CAN_NODE structure
 *
 *  Revision 1.39  2006/06/27 09:57:08  andreas
 *  Added ctrl_type, ctrl_clock and br_info to CAN_NODE
 *
 *  Revision 1.38  2005/09/14 15:58:53  michael
 *  filter 20b added
 *
 *  Revision 1.37  2005/08/22 16:26:58  andreas
 *  Added filter_cmd to ocb-structure
 *
 *  Revision 1.36  2005/08/03 11:28:08  andreas
 *  Added serial to crd structure (fur use with ioctl get serial)
 *
 *  Revision 1.35  2005/07/29 08:20:00  andreas
 *  crd-structure stores pointer (pCardIdent) into cardFlavours structure instead of index (flavour), now
 *
 *  Revision 1.34  2005/07/28 07:34:24  andreas
 *  version_firmware and version_hardware got removed from node-structure.
 *  flavour was added to crd-structure.
 *
 *  11.05.04 - removed esdcan_err_put                               ab
 *  13.02.03 - added file-pointer to OCB-structure
 *           - changed node->lock_irq into an OSIF_IRQ_MUTEX        ab
 *  27.05.02 - first version                                        mf
 *
 */
/************************************************************************
 *
 *  Copyright (c) 1996 - 2014 by electronic system design gmbh
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
/*! \file esdcan.h
 *  \brief Contains CAN_OCB, CAN_NODE-defines and related stuff.
 *
 *  \par General rules:
 *
 */

#ifndef __ESDCAN_H__
#define __ESDCAN_H__

#include <osif.h>
#include <boardrc.h>
#include <cm.h>

#ifndef OSIF_KERNEL
#error "This file may be used in the kernel-context, only! Not for application-use!!!"
#endif

#ifdef NUC_TX_TS
# define TX_TS_WIN_DEF 10
#else
# define TX_TS_WIN_DEF 0
#endif

#ifdef OSIF_USE_PREEMPT_RT_IMPLEMENTATION
 #define TX_STATE_PENDING_WRITE      0x01
 #define TX_STATE_PENDING_SEND       0x02
 #define TX_STATE_ABORT              0x10
 #define TX_STATE_SIGNAL_INTERRUPT   0x20
 #define TX_STATE_CLOSING            0x40

 #define RX_STATE_PENDING_READ       0x01
 #define RX_STATE_PENDING_TAKE       0x02
 #define RX_STATE_ABORT              0x10
 #define RX_STATE_SIGNAL_INTERRUPT   0x20
 #define RX_STATE_CLOSING            0x40
#else
 #define TX_STATE_PENDING_WRITE      0x01
 #define TX_STATE_PENDING_SEND       0x02
 #define TX_STATE_ABORTING           0x80

 #define RX_STATE_PENDING_READ       0x01
 #define RX_STATE_PENDING_TAKE       0x02
 #define RX_STATE_CLOSING            0x40
 #define RX_STATE_ABORTING           0x80
#endif

#define CLOSE_STATE_CLOSE_DONE       0x00000001
#define CLOSE_STATE_HANDLE_CLOSED    0x80000000

typedef struct _CAN_OCB   CAN_OCB;
typedef struct _OSIF_CARD OSIF_CARD;
typedef struct _CAN_NODE  CAN_NODE;
typedef struct _CAN_CARD  CAN_CARD;


struct _CAN_OCB {
        VOID             *nuc_ocb;
        VOID             *cif_ocb;
        CAN_NODE         *node;
        CM               *rx_cm;
        struct _rx {
                UINT32            tout;
                INT32             result;
                volatile INT32    state;
                INT32             cm_count;
                INT32             cm_size;
#ifdef BOARD_CAN_FD
                CAN_MSG_X        *cmbuf;
                CAN_MSG_X        *cmbuf_in;
#else
                CAN_MSG_T        *cmbuf;
                CAN_MSG_T        *cmbuf_in;
#endif
                UINT8            *user_buf;
        } rx;
        struct _tx {
                CM               *cm;
                UINT32            tout;
                INT32             result;
                volatile INT32    state;
                INT32             cm_count;
                INT32             cm_size;
                UINT8            *user_buf;
                CM               *cm_buf;
        } tx;
        INT32             minor;
        UINT32            mode;
        UINT32            filter_cmd;
        struct _filter20b {
                UINT32 acr;
                UINT32 amr;
        } filter20b;
        volatile UINT32   close_state;
        struct file      *file;
        /* !!!Leave at end, Linux specific!!! */
#ifdef OSIF_USE_PREEMPT_RT_IMPLEMENTATION
        wait_queue_head_t wqRx;
        wait_queue_head_t wqTx;
        wait_queue_head_t wqCloseDelay;
#else
        struct semaphore  sema_rx;
        struct semaphore  sema_tx;
        struct semaphore  sema_tx_abort; /* uninterruptible */
        struct semaphore  sema_close;
#endif
};


#ifdef BOARD_CAN_FD
#define MAX_NUM_DLC     16      /* number of different length DLC codes */
#else
#define MAX_NUM_DLC     (8+1)   /* number of different length DLC codes */
#endif

typedef struct _CAN_NODE_STAT {
        CAN_BUS_STAT    cbs;            /* struct to interface to NTCAN */
        /* driver internal part */
        uint32_t        msg_count_std[MAX_NUM_DLC];     /*# of 2.0A msgs*/
        uint32_t        msg_count_ext[MAX_NUM_DLC];     /*# of 2.0B msgs*/
} CAN_NODE_STAT;


struct _CAN_NODE {
        VOID            *nuc_node;
        VOID            *cif_node;
        UINT32          features;    /* flags are ored in layer's attach function */
        UINT32          mode;        /* enabled features       */
        UINT32          status;      /* node status flags (e.g. CANIO_STATUS_WRONG_* + CANIO_STATUS_MASK_BUSSTATE) */
        VOID            *base[4];    /* base[0] = sja address  */
        CAN_CARD        *crd;
        INT32           net_no;      /* counts nets over all cards, globally */
        INT32           node_no;     /* counts nets _per_ card, locally */
        OSIF_MUTEX      lock;
        OSIF_IRQ_MUTEX  lock_irq;
        volatile INT32  irq_flag;
        UINT32          ctrl_clock;  /* frequency of CAN controller */
        UINT8           ctrl_type;   /* type of CAN controller (use CANIO_CANCTL_xxx defines) */
        UINT8           trx_type;    /* Type of CAN transceiver (use CANIO_TRX_XXX defines) */
        UINT32          tx_ts_win;
        CAN_NODE_STAT   can_stat;       /* driver internal node statistic */
        /* !!!Leave at end, Linux specific!!! */
        wait_queue_head_t  wqRxNotify;
};

struct _CAN_CARD {
        VOID            *base[8];
        UINT32          range[8];
        UINT32          irq[8];
        CAN_NODE        *node[8];
        INT32           num_nodes;              /* # of active nodes on this card for NTCAN_INFO */
        INT32           card_no;
        UINT32          features;
        CARD_IRQ        irqs[8];
        VERSION96       version_firmware;
        VERSION96       version_firmware2;      /* for updateable second firmware e.g. Cypress Bootloader USB400   */
                                                /* This is NOT the IRIG FW version which is read via the IRIG lib. */
        VERSION96       version_hardware;
        CARD_IDENT      *pCardIdent;
        UINT32          serial;
        UINT16          board_status;           /* board status flags (wrong firmware/hardware) */
        UINT8           doNotTouch;             /* TRUE if HW must not be touched */
        UINT8           reserved;
#if defined (CIF_CARD)
        /* board specific part (see boardrc.h)... */
        CIF_CARD
#endif
};

#endif
