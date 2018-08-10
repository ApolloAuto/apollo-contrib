/* -*- linux-c -*-
 * FILE NAME osif_spi.h
 *
 * BRIEF MODULE DESCRIPTION
 * OSIF specials only needed with SPI CAN controllers 
 *
 * Also included in GPL'ed files
 * escan-of-spi-connector.c and esd-omap2-mcspi.c
 *
 * history:
 *   $Log$
 *
 */

/**************************************************************************************************
 *
 *  Copyright (c) 1996 - 2013 by esd electronic system design gmbh
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



#ifndef __OSIF_SPI_H__
#define __OSIF_SPI_H__

#include "sys_osiftypes.h"



#define MAX_DIRECTSPI_DEV    4
#define DIRECTSPIDEV_VERSION 1

#define MAX_SPI_CANDDEV	     16
#define SPI_CANDEV_VERSION   1


#define SPI_CAN_CTRL_TYPE_UNKOWN	0
#define SPI_CAN_CTRL_TYPE_MCP2515	1

#define SPI_MASTER_CTRL_TYPE_UNKNOWN   0
#define SPI_MASTER_CTRL_TYPE_OMAP_2_4  1


#define OSIF_SPI_BUFFER  32


typedef struct {
    uint8_t io[OSIF_SPI_BUFFER];    /* Data buffer */
    int     nBytes;                 /* # of bytes (Tx/Rx) */
} OSIF_SPIMSG;


struct spi_candev;

struct directspidev {
    u32  directspi_dev_version; 
    u32  master_id; 
    u32  master_type; /* SPI_MASTER_CTRL_TYPE_xxx */
    struct platform_device *master_dev;
    void __iomem  *master_base; /* was mcspi->base */
    unsigned long  master_phys;
    // todo FJ use a sub-structure for func ptrs!?
    int (*init)(struct spi_candev *pscd, VOID *vp);
    int (*sync_xfer)(struct spi_candev *pscd, OSIF_SPIMSG *pMsg);
    u32 reserved[32];
};

struct spi_candev {
    u32  spi_candev_version;
    u32  spi_id;
    int  spi_irq;
    int  spi_gpio_irq;
    char compatible[64];
    u32  spi_type; /* SPI_CAN_CTRL_TYPE_xxx */ 
    u32  spi_cs;   /* cs (0,1,2 ...) */
    u32  spi_clock;
    u32  spi_flags;
    u32  spi_max_frequency;
    u32  spi_mode;
    u32  spi_word_len;
    struct directspidev *pdsd;
    void __iomem  *spi_cs_base; /* was mcspi_cs->base */
    unsigned long spi_cs_phys;
	u32	spi_chconf0;
    void *driverdata; /* typically *CAN_CARD */
    // todo FJ use a sub-structure for func ptrs!?
    int (*spi_check_no_irq_pending)(struct spi_candev *pscd); /* check that there is no pending IRQ from CAN controller */
    u32  reserved[32];
};



#endif /* __OSIF_SPI_H__ */


/*---------------------------------------------------------------------------*/
/*                                 EOF                                               */
/*---------------------------------------------------------------------------*/

/* Some customisation for (X)-emacs */
/*
 * Local variables:
 * mode: c
 * indent-tabs-mode: nil
 * c-indent-level: 4
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 */
