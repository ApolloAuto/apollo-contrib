/* -*- linux-c -*-
 * FILE NAME flashtail.h
 *           copyright 2014 by esd electronic system design gmbh
 *
 * BRIEF MODULE DESCRIPTION
 *           system info page resources for CAN-CPIe/402 boards
 *
 *
 * Author:   Michael Schmidt
 *           michael.schmidt@esd.eu
 *
 * history:
 *  $Log$
 *  Revision 1.2  2015/05/11 17:29:16  manuel
 *  Removed unused stuff causing trouble for win32
 *
 *  Revision 1.1  2014/10/21 17:56:13  mschmidt
 *  Extracted struct _FLASH_TAIL from boardrc.h into flashtail.h.
 *
 *
 *
 */
/************************************************************************
 *
 *  Copyright (c) 1996 - 2015 by electronic system design gmbh
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
/*!
 * \file flashtail.h
 * \brief Structure of the system info page
 */

#ifndef BOARD_ESDACC_PCIE402_FLASHTAIL_H_
#define BOARD_ESDACC_PCIE402_FLASHTAIL_H_

#define FPGA_TYPE_ALTERA_GX15   15u
#define FPGA_TYPE_ALTERA_GX22   22u
#define FPGA_TYPE_ALTERA_GX30   30u

#if !defined OSIF_KERNEL
#  include <stdint.h>

typedef   int8_t        INT8;
typedef   uint8_t       UINT8;
typedef   int16_t       INT16;
typedef   uint16_t      UINT16;
typedef   int32_t       INT32;
typedef   uint32_t      UINT32;
typedef   int64_t       INT64;
typedef   uint64_t      UINT64;
typedef   char          CHAR8;

#endif

struct _FLASH_TAIL {
        CHAR8           strBoard[16];
        CHAR8           strOrderNumber[16];
        CHAR8           strSerialNumber[16];
        INT8            ucNumberOfNets;
        INT8            ucBaseNet;
        INT8            ucFeatures[2];
        UINT32          uiFeaturesExt;
        CHAR8           strBoardDetails[16];
        UINT8           ucTrxId;
        UINT8           ucFpgaTypeId;
        UINT8           ucReserved1[2];
        UINT32          uiReserved1[41];
        INT8            ucMagic[8];
        UINT8           ucVersionResv[2];  /* Don't use! Needed for updpcie402, only... */
        INT8            ucReserved0[2];
        UINT32          uiCrc32;
};

static const size_t lengthCrcRegion    = 0xFC;

#endif /* BOARD_ESDACC_PCIE402_FLASHTAIL_H_ */
