/* -*- esdcan-c -*-
 * FILE NAME sysdep.h
 *           copyright 2002-2015 by esd electronic system design gmbh
 *
 * BRIEF MODULE DESCRIPTION
 *           Linux kernel version dependency adaption
 *
 *
 *
 * history:
 *
 *  09.01.15 - Use vprintk() for kernels > 2.6.18              stm
 *  19.08.13 - fix for kernel greater 3.7                      mk
 *  23.09.04 - removed include of version.h                    ab
 *  26.11.03 - corrected OSIF_VSNPRINTF for kernel < 2.4.0     ab
 *  13.11.03 - added urb-typedef for some linux-versions       ab
 *  02.09.02 - first version                                   mf
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
 *  30165 Hannover                         http://www.esd-electronics.com
 *  Germany                                sales@esd-electronics.com
 *
 *************************************************************************/
/*! \file sysdep.h
    \brief Adaption of Linux kernel version changes

    This file fixes changes in the Linux kernel version.
*/

#ifndef __SYSDEP_H__
#define __SYSDEP_H__

# if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,0)
#  define OSIF_VSNPRINTF(buf, cnt, fmt, arg) vsprintf(buf, fmt, arg)
# elif LINUX_VERSION_CODE < KERNEL_VERSION(2,4,8)
/*
 *  AB: I've tried to find out, when vsnprintf came into the kernel.
 *      At least for Alan Cox kernels it seems to be 2.4.6.
 *      Increased to 2.4.8, since further science suggested, that
 *      integration happened on 2001-07-13 and changelogs on kernel.org
 *      suggest, that it might have happened with 2.4.7 (2001-07-20).
 *      To be on the save side, I chose 2.4.8 (2001-08-10).
 */
#  define OSIF_VSNPRINTF(buf, cnt, fmt, arg) osif_vsnprintf(buf, cnt, fmt, arg)
/* Use implementation of osif_vsnprintf() in osif.c */
#  define OSIF_PROVIDES_VSNPRINTF
# else
#  define OSIF_VSNPRINTF(buf, cnt, fmt, arg) vsnprintf(buf, cnt, fmt, arg)
# endif
/*
 * STM: Kernels since 2.6.0 should have vprintk(). Checked for sure with
 *      kernel 2.6.18 and later.
 */
# if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
#  define OSIF_USE_VPRINTK      1
# endif


# if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,0)
#  define PCI_BASE_ADDR_ACC(dev, n) (dev->base_address[n] & (~0x0000000F))
#  define OSIF_PCI_GET_FLAGS(dev, n) (dev->base_address[n] & 0x0000000F)
#  define OSIF__INIT
#  define OSIF__EXIT
# elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
#  define PCI_BASE_ADDR_ACC(dev, n) (pci_resource_start(dev, n))
#  define OSIF_PCI_GET_FLAGS(dev, n) (pci_resource_flags(dev, n))
#  if LINUX_VERSION_CODE <= KERNEL_VERSION(3,7,0)
#   define OSIF__INIT __devinit
#   define OSIF__EXIT __devexit
#  else
#   define OSIF__INIT
#   define OSIF__EXIT
#  endif
# else
#  define PCI_BASE_ADDR_ACC(dev, n) (dev->resource[n].start)
#  define OSIF_PCI_GET_FLAGS(dev, n) (dev->resource[n].flags)
#  define OSIF__INIT __init
#  define OSIF__EXIT __exit
# endif


# if LINUX_VERSION_CODE < KERNEL_VERSION(2,2,18) /* AB TODO: check minor version */
#  define OSIF_MODULE_INIT(fct) INT32 init_module(VOID) { return fct(); }
#  define OSIF_MODULE_EXIT(fct) VOID cleanup_module(VOID) { fct(); }
#  define OSIF_DECL_WAIT_QUEUE(p) struct wait_queue *p = NULL
#  define OSIF_PCI_GET_RANGE(dev, n) \
        (dev->PCI_BASE_ADDR_ACC(n) & PCI_BASE_ADDRESS_SPACE ? \
                /* I/O-space */ \
                (~(dev->PCI_BASE_ADDR_ACC(n) & PCI_BASE_ADDRESS_IO_MASK) + 1) : \
                /* Memory-space */ \
                (~(dev->PCI_BASE_ADDR_ACC(n) & PCI_BASE_ADDRESS_MEM_MASK) + 1) \
        )
# else
#  define OSIF_MODULE_INIT(fct)  module_init(fct)
#  define OSIF_MODULE_EXIT(fct)  module_exit(fct)
#  define OSIF_DECL_WAIT_QUEUE(p) DECLARE_WAIT_QUEUE_HEAD(p)
#  define OSIF_PCI_GET_RANGE(dev, n) pci_resource_len(dev, n)
# endif

#endif
