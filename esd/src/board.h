/* -*- linux-c -*-
 * FILE NAME board.h
 *           copyright 2002 - 2014 by esd electronic system design gmbh
 *
 * BRIEF MODULE DESCRIPTION
 *           board layer API
 *
 *
 * Author:   Matthias Fuchs
 *           matthias.fuchs@esd-electronics.com
 *
 * history:
 *
 *  27.05.02 - first version                                       mf
 *  11.05.04 - changed OSIF_EXTERN into extern and OSIF_CALLTYPE   ab
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
/*! \file board.h
    \brief Card interface API

    This file contains the API for accessing the CAN driver's card layer.
    The functions are called by the nucleus modules.
*/

#ifndef __BOARD_H__
#define __BOARD_H__

#include <esdcan.h>

#ifdef ESDDBG   /*!< Macro for debug prints, same in ALL board layer files... */
# define BOARD_DBG(fmt) OSIF_DPRINT(fmt)
#else
# define BOARD_DBG(fmt)
#endif

/* Zones to use with debug prints in boardrc.c */
#define RC_ZONE_FU          OSIF_ZONE_BOARD | OSIF_ZONE_FUNC
#define RC_ZONE_INIFU       OSIF_ZONE_BOARD | OSIF_ZONE_INIT|OSIF_ZONE_FUNC
#define RC_ZONE_INI         OSIF_ZONE_BOARD | OSIF_ZONE_INIT
#define RC_ZONE_IRQ         OSIF_ZONE_IRQ    /* DO NOT USE ANY MORE BITS (nto will thank you:) */
#define RC_ZONE_ALWAYS      0xFFFFFFFF
#define RC_ZONE_WARN        OSIF_ZONE_BOARD | OSIF_ZONE_WARN
#define RC_ZONE_USR_INIT    OSIF_ZONE_BOARD | OSIF_ZONE_USR_INIT
#define RC_ZONE             OSIF_ZONE_BOARD

/* Zones to use with debug prints in board.c */
#define BOARD_ZONE_INI      OSIF_ZONE_BOARD | OSIF_ZONE_INIT                    /* Driver init/deinit */
#define BOARD_ZONE_FU       OSIF_ZONE_BOARD | OSIF_ZONE_FUNC                    /* Function entry/exit */
#define BOARD_ZONE_INIFU    OSIF_ZONE_BOARD | OSIF_ZONE_INIT | OSIF_ZONE_FUNC
#define BOARD_ZONE_BAUD     OSIF_ZONE_BOARD | OSIF_ZONE_FUNC | OSIF_ZONE_BAUD   /* Function entry/exit in bitrate setting baud calculation... */
#define BOARD_ZONE_IRQ      OSIF_ZONE_IRQ                                       /* DO NOT USE ANY MORE BITS (nto will thank you:) */
#define BOARD_ZONE          OSIF_ZONE_BOARD

#ifndef CIF_TS2MS
#define CIF_TS2MS(ts, tsf)  ts *= 1000; OSIF_DIV64_64(ts, tsf)
#endif

extern CARD_IDENT cardFlavours[];

extern INT32 cif_open( CAN_OCB *ocb, CAN_NODE *node, INT32 flags );
extern INT32 cif_close( CAN_OCB *ocb );
extern INT32 cif_node_attach( CAN_NODE *node, UINT32 queue_size_rx);
extern INT32 cif_node_detach( CAN_NODE *node );
extern INT32 cif_id_filter( CAN_NODE *node, UINT32 cmd, UINT32 id, UINT32 *count );
extern INT32 OSIF_CALLTYPE cif_tx( CAN_NODE *node );
extern INT32 cif_tx_obj( CAN_NODE *node, UINT32 cmd, VOID *arg );
extern INT32 cif_tx_abort( CAN_NODE *node, CM *cm, INT32 status );
extern INT32 cif_baudrate_set( CAN_NODE *node, UINT32 baud );

/* can_node MUST be provided, ocb and waitContext might be NULL (should fail if ocb or waitContext needed then) */
extern INT32 cif_ioctl( CAN_NODE *can_node, CAN_OCB *ocb, VOID *waitContext, UINT32 cmd,
                        VOID *buf_in, UINT32 len_in, VOID *buf_out, UINT32 *len_out );

extern INT32 cif_timestamp( CAN_NODE *node, OSIF_TS *timestamp );

/* 1) implementation for soft timestamps is in board/<board>/board.c
   2) implementation for hard timestamps is in board/<board>/boardrc.c
      (this is very hardware specific !)
*/
extern INT32 cif_softts_get( VOID *dummy, OSIF_TS *ts );
extern INT32 cif_softts_freq_get( VOID *dummy, OSIF_TS_FREQ *ts_freq );

/* Enable/disable the interrupt (also implicitely called during attach/detach) */
extern INT32 OSIF_CALLTYPE can_board_enable_interrupt(CAN_CARD *crd);
extern INT32 OSIF_CALLTYPE can_board_disable_interrupt(CAN_CARD *crd);

INT32 OSIF_CALLTYPE can_board_attach_pre(CAN_CARD *crd);

#if defined(OSIF_PNP_OS)
INT32 OSIF_CALLTYPE can_board_attach( CAN_CARD *crd, OSIF_POWER_STATE targetState );
INT32 OSIF_CALLTYPE can_board_attach_final( CAN_CARD *crd, OSIF_POWER_STATE targetState );
INT32 OSIF_CALLTYPE can_board_detach( CAN_CARD *crd, OSIF_POWER_STATE targetState );
    /* CAN_BOARD_DETACH_FINAL may be defined in boardrc.h */
# ifndef CAN_BOARD_DETACH_FINAL
#  define CAN_BOARD_DETACH_FINAL(pCrd, targetState)
#  else
INT32 OSIF_CALLTYPE can_board_detach_final( CAN_CARD *crd, OSIF_POWER_STATE targetState );
# endif
#else
INT32 OSIF_CALLTYPE can_board_attach( CAN_CARD *crd );
INT32 OSIF_CALLTYPE can_board_attach_final( CAN_CARD *crd );
INT32 OSIF_CALLTYPE can_board_detach( CAN_CARD *crd );
    /* CAN_BOARD_DETACH_FINAL may be defined in boardrc.h */
#ifndef CAN_BOARD_DETACH_FINAL
# define CAN_BOARD_DETACH_FINAL(pCrd)
# else
INT32 OSIF_CALLTYPE can_board_detach_final( CAN_CARD *crd );
#endif
#endif /* of OSIF_PNP_OS */

#if defined (BOARD_USB)
VOID OSIF_CALLTYPE usb_write_tx_cm(const volatile UINT32 *pAddr, CM *pCm);
VOID OSIF_CALLTYPE usb_write_tx_ts_cm(const volatile UINT32 *pAddr, CM *pCm);
#endif

#endif /* __BOARD_H__ */
