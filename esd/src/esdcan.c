/* -*- esdcan-c -*-
 * FILE NAME esdcan.c
 *           copyright 2002-2015 by esd electronic system design gmbh
 *
 * BRIEF MODULE DESCRIPTION
 *           This file contains the Linux / RTAI-Linux entries for
 *           the esd CAN driver
 *
 *
 * Author:   Matthias Fuchs
 *           matthias.fuchs@esd.eu
 *
 * history:
 *
 *  $Log$
 *  Revision 1.166  2015/07/09 13:07:57  stefanm
 *  Added printout for OSIF_ZONE_USR_INIT to show when kernel
 *  module's initialisation starts.
 *
 *  Revision 1.165  2015/03/12 19:08:30  stefanm
 *  Mantis 2553: Adapted message that is shown if the
 *  PCI405 board doesn't boot correctly because there is
 *  an old U-Boot installed on the PCI405.
 *  Additionally needed a fix in boardrc.c.
 *
 *  Revision 1.164  2015/01/13 15:31:02  stefanm
 *  Added support for NTCAN_IOCTL_GET_INFO / IOCTL_ESDCAN_GET_INFO.
 *  Will now count active nodes in <num_nodes> of card structure.
 *  New <version_firmware2> is needed for USB400 Cypress updateable FW.
 *
 *  Revision 1.163  2014/08/21 13:17:55  manuel
 *  Added new IOCTLs: SEND_X WRITE_X TAKE_X READ_X SET_BAUD_X GET_BAUD_X
 *  Fixed vme include
 *
 *  Revision 1.162  2014/07/08 07:44:48  matthias
 *  remove printk
 *
 *  Revision 1.161  2014/07/07 14:36:20  matthias
 *  check fo CONFIG_OF_DEVICE or (!) CONFIG_OF_FLATTREE
 *
 *  Revision 1.160  2014/07/04 10:00:08  hauke
 *  Just include <nucleus.h> which has to include the removed header (in the expected sequence).
 *
 *  Revision 1.159  2014/06/23 17:02:06  stefanm
 *  Removed ESDCAN_ZONE_USR_INIT and replaced by OSIF_ZONE_USR_INIT used directly
 *  because it did not work well with the new osif_dprint() function (no output).
 *  Replaced wrong "linux-c" mode with atm. invalid "esdcan-c" mode.
 *
 *  Revision 1.158  2014/01/27 11:26:15  andreas
 *  Some cleanup, mainly tried to clear the ifdef-mess by adding some more comments...
 *
 *  Revision 1.157  2013/12/30 23:45:10  ar
 *  Better support for vmecan4.
 *
 *  Revision 1.156  2013/12/30 16:13:01  frank
 *  Added some debug output to can_attach_common()
 *
 *  Revision 1.155  2013/11/19 15:13:33  frank
 *  Added SPI support (BOARD_SPI) (see also esdcan_spi.c)
 *
 *  Revision 1.154  2013/09/04 13:00:06  andreas
 *  Startup banner cleaned up
 *
 *  Revision 1.153  2013/08/19 14:17:01  manuel
 *  Fixed procfile handling for kernel 3.8 and greater
 *
 *  Revision 1.152  2013/08/09 13:01:41  andreas
 *  Small change on startup message
 *
 *  Revision 1.151  2013/08/02 09:36:58  andreas
 *  Some whitespace deleted...
 *
 *  Revision 1.150  2013/08/01 15:03:40  andreas
 *  Changed can_board_detach_final() to macro
 *
 *  Revision 1.149  2013/08/01 12:50:52  andreas
 *  Added can_board_detach_final()
 *
 *  Revision 1.148  2013/06/27 12:48:25  andreas
 *  Removed driver parameter for TX-TS window size for boards which have NUC_TX_TS NOT defined
 *
 *  Revision 1.147  2013/01/11 18:30:20  andreas
 *  Forgot to register the new IOCTLs "old style"
 *
 *  Revision 1.146  2013/01/11 18:29:11  andreas
 *  Added driver parameter txtswin
 *  Added IOCTLs: IOCTL_ESDCAN_ID_REGION_ADD, IOCTL_ESDCAN_ID_REGION_DEL, IOCTL_ESDCAN_SET_TX_TS_WIN, IOCTL_ESDCAN_GET_TX_TS_WIN, IOCTL_ESDCAN_SET_TX_TS_TIMEOUT,
 *  IOCTL_ESDCAN_GET_TX_TS_TIMEOUT, IOCTL_ESDCAN_SET_HND_FILTER
 *
 *  Revision 1.145  2013/01/03 16:21:00  andreas
 *  Updated copyright notice
 *
 *  Revision 1.144  2012/11/08 14:14:42  andreas
 *  Fixed bug on handle close (two threads entering driver on same handle with ioctl_destroy)
 *
 *  Revision 1.143  2012/02/17 18:53:57  andreas
 *  Updated copyright
 *
 *  Revision 1.142  2011/11/01 15:31:57  andreas
 *  Waitqueue wqRx in CAN_NODE (used for select feature) got renamed to
 *    wqRxNotify, when merging with preempt_rt branch
 *
 *  Revision 1.141  2011/09/06 17:18:10  manuel
 *  Fixed rcsid (for newer gcc)
 *
 *  Revision 1.140  2011/08/31 12:50:43  manuel
 *  Added EEI stuff
 *
 *  Revision 1.139  2011/08/05 15:47:05  manuel
 *  Beautified debug output only
 *
 *  Revision 1.138  2011/07/12 17:37:42  manuel
 *  From 2.6.19 on, the irq handler has no more pt_regs.
 *  Fix for missing path_lookup in 2.6.39.
 *
 *  Revision 1.137  2011/07/05 10:34:04  michael
 *  nodes rx-queuesize can be configured. At the moment only used under nto
 *
 *  Revision 1.136  2011/06/20 17:44:00  manuel
 *  Added BOARD_SOFT support
 *
 *  Revision 1.135  2011/05/18 16:01:10  andreas
 *  Changed copyright header
 *
 *  Revision 1.134  2010/12/15 14:26:14  manuel
 *  Fixed memory leak in can_read_proc_nodeinit
 *
 *  Revision 1.133  2010/10/26 12:48:56  manuel
 *  There is no more ioctl field in struct file_operations from kernel 2.6.36 on
 *
 *  Revision 1.132  2010/10/19 12:45:19  manuel
 *  Revised proc dir/file creation; hopefully no more "badness" detections now...
 *
 *  Revision 1.131  2010/05/21 17:28:05  manuel
 *  Added linux/sched.h include. Needed for kernel 2.6.33 but shouldn't harm for older kernels.
 *
 *  Revision 1.130  2010/03/16 10:49:44  andreas
 *  Untabified
 *
 *  Revision 1.129  2010/03/16 10:33:18  andreas
 *  Added esdcan_poll to fops (select implementation)
 *
 *  Revision 1.128  2010/03/11 12:38:07  manuel
 *  Added sleep if pci405 u-boot update is running
 *
 *  Revision 1.127  2010/03/09 10:07:22  matthias
 *  Beginning with 2.6.33 linux/autoconf.h moved to generated/autoconf.h. Also there is no need to include it. The kernel's build system passes it via -include option to gcc.
 *
 *  Revision 1.126  2010/03/08 16:45:58  matthias
 *  Add support for esdcan_of_connector interface
 *
 *  Revision 1.125  2009/12/04 12:28:37  andreas
 *  Fixed 32-Bit IOCTLs for x86_64-Kernel < 2.6.10
 *
 *  Revision 1.124  2009/07/31 14:55:52  andreas
 *  Added IOCTL_ESDCAN_SER_REG_READ and IOCTL_ESDCAN_SER_REG_WRITE
 *
 *  Revision 1.123  2009/07/06 15:14:17  andreas
 *  Removed IRQF_DISABLED flag
 *  Updated copyright comment
 *
 *  Revision 1.122  2009/04/01 14:34:03  andreas
 *  Added driver command line parameter "clock". Used to override
 *  timer tick frequency on certain embedded targets.
 *
 *  Revision 1.121  2009/02/25 16:26:05  andreas
 *  Removed bus statistics proc-file
 *
 *  Revision 1.120  2008/12/02 13:18:37  andreas
 *  Removed forgotten esdcan_proc_extra_info_-calls
 *
 *  Revision 1.119  2008/12/02 11:07:39  andreas
 *  Fix/Change:
 *    - proc files were moved into /proc/bus/can/DRIVER_NAME/
 *    - Fixed problems, when loading different drivers concurrently
 *    - Fixed problems with several identical CAN boards
 *    - removed extrainfo file
 *
 *  Revision 1.118  2008/11/18 16:00:49  matthias
 *  add pcimsg module parameter
 *
 *  Revision 1.117  2008/10/29 15:29:57  andreas
 *  Fixed race condition on request_irq, which had pretty bad
 *  consequences with shared interrupts.
 *
 *  Revision 1.116  2008/09/17 16:42:07  manuel
 *  Added missing MODULE_LICENSE
 *
 *  Revision 1.115  2008/06/06 14:10:34  andreas
 *  Fixed busload IOCTLs for x86_64 kernels < 2.6.10
 *
 *  Revision 1.114  2008/03/20 10:58:36  andreas
 *  Fixed warning regarding deprecated IRQ flags
 *  Changed IRQ flags to work with kernels >= 2.6.25
 *
 *  Revision 1.113  2007/11/09 16:33:32  manuel
 *  Fix for newer kernels where ioctl32.h doesn't exist
 *
 *  Revision 1.112  2006/12/22 09:59:24  andreas
 *  Fixed for kernels 2.6.19
 *  Exchanged include order of version.h and config.h/autoconf.h
 *  Including autoconf.h instead of config.h for kernels >= 2.6.19
 *  Added generation of inodeinit-script in /proc/bus/can/
 *  Currently this feature is for internal use, only and not documented
 *
 *  Revision 1.111  2006/11/22 10:37:32  andreas
 *  Added message to syslog, after a PCI405-Firmware update
 *
 *  Revision 1.110  2006/11/13 11:35:08  matthias
 *  added newline when last net number is printed
 *
 *  Revision 1.109  2006/10/12 09:53:36  andreas
 *  Replaced some forgotten CANIO_-error codes with the OSIF_ counterparts
 *  Replaces some errnos with their OSIF_counterparts
 *  Cleaned up return of error codes to user space,
 *    please use positive error codes, only!!!
 *
 *  Revision 1.108  2006/10/11 10:12:48  andreas
 *  Fixed compilation problem with SuSE > 10.0
 *
 *  Revision 1.107  2006/08/17 13:26:59  michael
 *  Rebirth of OSIF_ errorcodes
 *
 *  Revision 1.106  2006/07/11 15:13:58  manuel
 *  Replaced nuc_timestamp with data from CAN_STAT structure in can_read_proc
 *
 *  Revision 1.105  2006/07/04 12:41:05  andreas
 *  Added missing IOCTL_ESDCAN_SET_ALT_RTR_ID
 *
 *  Revision 1.104  2006/06/29 11:53:17  andreas
 *  Added module load parameter "errorinfo" to disable extended error info
 *
 *  Revision 1.103  2006/06/27 13:12:43  andreas
 *  Exchanged OSIF_errors with CANIO_errors
 *
 *  Revision 1.102  2006/06/27 10:00:45  andreas
 *  Removed usage of nuc_baudrate_get()
 *  Moved baud_table into nucleus
 *  Added TODO about timestamps on baudrate change events
 *
 *  Revision 1.101  2006/04/28 12:30:36  andreas
 *  Added warning in syslog, if debug driver
 *  Changes to support recent kernel (>2.6.13)
 *  Added usage of compat_ioctl and unlocked_ioctl
 *
 *  Revision 1.100  2006/02/14 09:20:52  matthias
 *  Don't use macros for esdcan_un/register_ioctl32 and
 *  others when these functions are not used. Use empty
 *  functions instead. This avoid compiler warnings
 *  from gcc 4.x
 *
 *  Revision 1.99  2005/12/08 11:35:26  andreas
 *  For kernels below 2.4.0 legacy PCI support is used
 *  instead of hotplugging
 *
 *  Revision 1.98  2005/12/06 13:26:10  andreas
 *  No functional changes. Removed old "#if 0" path and
 *  small cleanup.
 *
 *  Revision 1.97  2005/11/14 13:45:30  manuel
 *  Added #if PLATFORM_64BIT around esdcan_register_ioctl32
 *  and esdcan_unregister_ioctl32 prototypes
 *
 *  Revision 1.96  2005/11/02 07:12:46  andreas
 *  Added IOCTL_ESDCAN_SET_20B_HND_FILTER (de-)registration
 *  for 64-Bit platforms
 *
 *  Revision 1.95  2005/10/06 07:39:17  matthias
 *  temporarily disabled nuc_timestamp call for proc interface
 *
 *  Revision 1.94  2005/08/29 14:33:48  andreas
 *  Fixed bug in release of memory regions
 *
 *  Revision 1.93  2005/07/29 08:19:43  andreas
 *  crd-structure stores pointer (pCardIdent) into cardFlavours
 *  structure instead of index (flavour), now
 *
 *  Revision 1.92  2005/07/28 08:07:30  andreas
 *  Separated initialization of different board types (pci, raw, usb,...).
 *  From now on this file contains function common to all (or at least several)
 *  board types. Everything specific to one board type is located in esdcan_pci.c,
 *  esdcan_raw.c, esdcan_usb.c or esdcan_pci_legacy.c.
 *  BOARD and BOARD_NAME defines have been removed. There's a ESDCAN_DRIVER_NAME defined
 *  in each boardrc.h.
 *  There's also an array cardFlavours defined in boardrc.c, which provides the
 *  "spaces" and "irq" arrays, board name and type specific IDs for all boards supported
 *  by the same driver binary. This means at least for PCI and hotplug PCI the driver
 *  can support several different devices, now.
 *  Bugfixes in ISA-initialization (behaviour, if more than one address space is defined),
 *  located in esdcan_raw.c, now.
 *
 *  Revision 1.91  2005/04/25 07:29:49  andreas
 *  Fixed bug (irrelevant, since feature isn't used until now) in ISA module paramters
 *
 *  Revision 1.90  2005/04/20 13:44:20  andreas
 *  Changed for 64-Bit Linux (ioctl registration, C-types)
 *  Cleanup
 *
 *  Revision 1.89  2005/03/04 08:41:16  andreas
 *  Added cpci200
 *
 *  Revision 1.88  2005/02/21 15:24:12  matthias
 *  -because CM-size increased we have to copy CMSG and CMSG_T struct from/to userspace
 *  -added pci405fw2
 *  -removed unused code
 *
 *  Revision 1.87  2004/11/15 13:29:52  matthias
 *  added cpci750 support
 *
 *  Revision 1.86  2004/11/10 14:33:03  michael
 *  Compilable with versioned symbols again
 *
 *  Revision 1.85  2004/09/23 14:49:32  andreas
 *  forgot to remove verbose-preset
 *
 *  Revision 1.84  2004/09/23 14:48:17  andreas
 *  Moved most of the USB code to esdcan_usb.c
 *  Changed debug output to dprint
 *
 *  11.05.04 - Added OSIF_CALLTYPE, where needed
 *             Removed warning (passing param 4 of nuc_extra_info)  ab
 *  07.05.04 - Changes for kernel 2.6:
 *               - module-parameters (and description)
 *               - module open count
 *               - check_region -> request_region
 *               - EXPORT_NO_SYMBOLS removed
 *             Some cleanup                                         ab
 *  07.04.04 - changed major of gitane to 54                        ab
 *  11.03.04 - added Gitane and GitaneLite majors
 *             added IO-space mapping for PCI-cards                 ab
 *  17.12.03 - added pci266                                         ab
 *  01.12.03 - resetting USB-modules on driver-unload
 *             some more small corrections for USB-modules          ab
 *  26.11.03 - ifdefed release_region for kernel < 2.4.0            ab
 *  13.11.03 - USB331-Support added (THIS IS STILL BETA!!!)
 *             USB-specific stuff is implemented here, to be able
 *             reimplement USB-stuff for other systems.
 *             There are NO effects for non-USB-cards. Nevertheless
 *             some cleanup is still needed.                        ab
 *  05.11.03 - Added support for four subvendor and subsystem-IDs   ab
 *  27.10.03 - Corrected usage of print-macros
 *             Added verbose module parameter                       ab
 *  29.09.03 - read/write: get node-mutex before common call
 *             (same as IRIX IOCTRL)                                sr
 *  05.09.03 - improved module parameter handling for irq and io
 *             (only for RAW_BOARDS)
 *             a) irq arg can be terminated by -1: irq=25,-1
 *             b) io arg can be temrinated by 0: io=0xf0000000,0    mf
 *  03.06.03 - undo change from 21.05.03                            mf
 *  21.05.03 - release irq when nuc_node_attach failed (0.3.3)      mf
 *  15.01.03 - common driver part extracted "../esdcan_common.c"    sr
 *  18.11.02 - added callback interface for nucleus                 mf
 *  07.11.02 - Version 0.1.1                                        mf
 *  21.05.02 - added hardware timestamping (Version 0.1.0)          mf
 *  17.05.02 - first release (Version 0.1.0)                        mf
 *  02.05.02 - first version                                        mf
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
/*! \file esdcan.c
    \brief Linux driver entries

    This file contains the Linux entries for
    the esd CAN driver.
*/
#include <linux/version.h>
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,32))
# if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,19))
#  include <linux/autoconf.h>
# else /* #if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,19)) */
#  include <linux/config.h>
# endif /* #if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,19)) */
#endif /* #if (LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,32)) */
#ifdef DISTR_SUSE
# undef CONFIG_MODVERSIONS
#endif /* #ifdef DISTR_SUSE */
#ifdef CONFIG_MODVERSIONS
# if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
#  include <config/modversions.h>
# else /* #if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0) */
#  include <linux/modversions.h>
# endif /* #if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0) */
# ifndef MODVERSIONS
#  define MODVERSIONS
# endif /* #ifndef MODVERSIONS */
#endif /* #ifdef CONFIG_MODVERSIONS */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/proc_fs.h>
#include <linux/interrupt.h>
#include <linux/poll.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
# include <linux/moduleparam.h>
# if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,21)
#  include <linux/ioctl32.h>
# endif /* #if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,21) */
#endif /* #if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0) */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,0)
# include <linux/ioport.h>
#endif /* #if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,0) */
#ifndef OSIF_OS_RTAI
# include <asm/uaccess.h>
#endif /* #ifndef OSIF_OS_RTAI */
#ifdef LTT
# warning "Tracing enabled"
# include <linux/trace.h>
#endif /* #ifdef LTT */

#include <linux/sched.h>

#include <nucleus.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0)
# if defined(BOARD_PCI)
#  define ESDCAN_PCI_HOTPLUG
#  undef BOARD_PCI /* Disable legacy PCI support */
# endif /* #if defined(BOARD_PCI) */
#endif /* #if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0) */

#if defined(BOARD_USB) || defined(ESDCAN_PCI_HOTPLUG)
# define ESDCAN_HOTPLUG_DRIVER
#endif /* #if defined(BOARD_USB) || defined(ESDCAN_PCI_HOTPLUG) */

#if defined (BOARD_USB)
# if LINUX_VERSION_CODE > KERNEL_VERSION(2,4,0)
#  include <linux/usb.h>
# endif /* #if LINUX_VERSION_CODE > KERNEL_VERSION(2,4,0) */
#endif /* #if defined (BOARD_USB) */

#if defined (BOARD_VME)
#  include <linux/vme.h>
#  define ESDCAN_HOTPLUG_DRIVER
#endif /* #if defined (BOARD_VME) */

#ifdef ESDDBG
# define CAN_DEBUG
#endif /* #ifdef ESDDBG */

#ifdef CAN_DEBUG
# define CAN_DBG(fmt)       OSIF_DPRINT(fmt)  /*!< Macro for debug prints */
# define CAN_IRQ_DBG(fmt)   OSIF_DPRINT(fmt)  /*!< Macro for debug prints on interrupt-level */
#else /* #ifdef CAN_DEBUG */
# define CAN_DBG(fmt)                         /**< Empty expression */
# define CAN_IRQ_DBG(fmt)                     /**< Empty expression */
#endif /* #ifdef CAN_DEBUG */
#define CAN_PRINT(fmt)      OSIF_PRINT(fmt)   /**< Makro for output to the general user (NOT surpressed in release version) */
#define CAN_DPRINT(fmt)     OSIF_DPRINT(fmt)  /**< Makro for output to the general user (filtered by verbose level)         */
#define CAN_IRQ_PRINT(fmt)  OSIF_DPRINT(fmt)  /**< Makro for output to the general user on interrupt level (NOT surpressed in release version) */

#define ESDCAN_ZONE_INIFU    OSIF_ZONE_ESDCAN|OSIF_ZONE_INIT|OSIF_ZONE_FUNC
#define ESDCAN_ZONE_INI      OSIF_ZONE_ESDCAN|OSIF_ZONE_INIT
#define ESDCAN_ZONE_FU       OSIF_ZONE_ESDCAN|OSIF_ZONE_FUNC
#define ESDCAN_ZONE          OSIF_ZONE_ESDCAN
#define ESDCAN_ZONE_IRQ      OSIF_ZONE_IRQ    /* DO NOT USE ANY MORE BITS (nto will thank you:) */
#define ESDCAN_ZONE_ALWAYS   0xFFFFFFFF
#define ESDCAN_ZONE_WARN     OSIF_ZONE_ESDCAN|OSIF_ZONE_WARN

#ifdef LTT
INT32 can_ltt0;
UINT8 can_ltt0_byte;
#endif /* #ifdef LTT */

/* module parameter verbose: default changes regarding to debug flag */
#ifndef CAN_DEBUG
/*
 *  The released driver writes load- and unload-messages to syslog,
 *  if the user hasn't specified a verbose level.
 */
uint verbose = 0x00000001;
#else /* #ifndef CAN_DEBUG */
/*
 *  The debug-version of the driver writes all "release-version"-output
 *  and warning- and error-messages, if the developer doesn't specify
 *  otherwise.
 */
/* uint verbose = 0xF8F00001; */ /* AB setting */
/* uint verbose = 0xF7FFFFFF; */ /* AB all but IRQ  */
/* uint verbose = (0xF7FFFFFF & (~OSIF_ZONE_FILTER)); */ /* AB all but IRQ and NUC (filter) */
/* uint verbose= (OSIF_ZONE_CTRL|OSIF_ZONE_BACKEND); */
uint verbose = 0xC00000FF;
#endif /* #ifndef CAN_DEBUG */

unsigned int major = MAJOR_LINUX; /* MAJOR_LINUX is set in .cfg files */

VOID esdcan_show_card_info( CAN_CARD *crd );
VOID esdcan_show_card_info_all( VOID );
VOID esdcan_show_driver_info( VOID );
INT32 can_attach_common(CAN_CARD *crd);

CAN_NODE *nodes[MAX_CARDS * NODES_PER_CARD];  /* index: minor */

int mode = 0;

int errorinfo = 1;
int pcimsg = 1;
int clock = 0;
int txtswin = TX_TS_WIN_DEF;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
/* common module parameter */
module_param(major, uint, 0);
MODULE_PARM_DESC(major, "major number to be used for the CAN card");
module_param(mode, int, 0);      /* e.g. LOM */
MODULE_PARM_DESC(mode, "activate certain driver modes, e.g. LOM");
module_param(verbose, uint, 0);  /* mask for output selection */
MODULE_PARM_DESC(verbose, "change verbose level of driver");
module_param(errorinfo, int, 0); /* flag for extended error info */
MODULE_PARM_DESC(errorinfo, "enable/disable extended error info (default: on)");
unsigned int  compat32 = 1;
module_param(compat32, int, 0);  /* setting this to zero disables 32-bit compatibility on 64-bit systems */
MODULE_PARM_DESC(compat32, "disable 32-Bit compatibility on 64-Bit systems");
#if defined(BOARD_PCI) || defined(ESDCAN_PCI_HOTPLUG)
module_param(pcimsg, int, 0);    /* enable/disable pcimsg interface on firmware drivers */
MODULE_PARM_DESC(pcimsg, "enable/disable pcimsg interface on firmware drivers (default: on)");
#endif /* defined(BOARD_PCI) || defined(ESDCAN_PCI_HOTPLUG) */
module_param(clock, int, 0);     /* override TS tick frequency */
MODULE_PARM_DESC(clock, "LEAVE THIS ONE ALONE, works with special hardware, only");
#ifdef NUC_TX_TS
module_param(txtswin, int, 0);   /* override TX TS window size (ms) */
MODULE_PARM_DESC(txtswin, "override default TX-TS-window size (in ms)");
#endif /* #ifdef NUC_TX_TS */
#else /* #if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0) */
/* common module parameter */
MODULE_PARM(major, "1i");
MODULE_PARM(mode, "1i");      /* e.g. LOM */
MODULE_PARM(verbose, "1i");   /* mask for output selection */
MODULE_PARM(errorinfo, "1i"); /* enable/disable extended error info (default: on) */
#if defined(BOARD_PCI) || defined(ESDCAN_PCI_HOTPLUG)
MODULE_PARM(pcimsg, "1i");    /* enable/disable pcimsg interface on firmware drivers */
#endif /* defined(BOARD_PCI) || defined(ESDCAN_PCI_HOTPLUG) */
MODULE_PARM(clock, "1i");     /* override TS tick frequency */
#ifdef NUC_TX_TS
MODULE_PARM(txtswin, "1i");   /* override TX TS window size (ms) */
#endif /* #ifdef NUC_TX_TS */
#endif /* #if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0) */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
# define INODE2CANNODE(inode) (iminor(inode))
#else /* #if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0) */
# define INODE2CANNODE(inode) (MINOR(inode->i_rdev) & 0xff)
#endif /* #if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0) */

/* BL: Actually came into kernel.org with 2.6.18,
 *     from 2.6.25 it is mandatory (old defines removed)
 *     In 2.6.22 "deprecated warnings" came in */
# if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,22))
#  if defined(ESDCAN_SPECIAL_IRQ_FLAGS) /* see e.g. mcp2515/cpuca8/boardrc.h */
#    define ESDCAN_IRQ_FLAGS ESDCAN_SPECIAL_IRQ_FLAGS
#  else /* #if defined(ESDCAN_SPECIAL_IRQ_FLAGS) */
#    define ESDCAN_IRQ_FLAGS IRQF_SHARED
#  endif /* #if defined(ESDCAN_SPECIAL_IRQ_FLAGS) */
# else /* #if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,22)) */
#  define ESDCAN_IRQ_FLAGS   SA_SHIRQ | SA_INTERRUPT
# endif /* #if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,22)) */

#ifdef OSIF_OS_LINUX
#define ESDCAN_IOCTL_PROTO \
        static int esdcan_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
# if (PLATFORM_64BIT) && (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,10))
static long esdcan_compat_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
# endif /* #if (PLATFORM_64BIT) && (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,10)) */
# if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,10)
static long esdcan_unlocked_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
# endif /* #if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,10) */
#endif /* OSIF_OS_LINUX */

#ifdef OSIF_OS_RTAI
#define ESDCAN_IOCTL_PROTO \
        INT32 esdcan_ctrl(INT32 hnd, UINT32 cmd, UINT32 arg)
#endif /* OSIF_OS_RTAI */

static unsigned int esdcan_poll(struct file *pFile, struct poll_table_struct *pPollTable);
static int esdcan_open(struct inode *inode, struct file *file);
static int esdcan_release(struct inode *inode, struct file *file);
ESDCAN_IOCTL_PROTO;

void esdcan_unregister_ioctl32(void);
int32_t esdcan_register_ioctl32(void);

#define RETURN_TO_USER(ret) return(-ret)

#ifndef OSIF_OS_RTAI
struct file_operations esdcan_fops =
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,0)
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,  /* no support of poll/select planned for 2.2.x */
        esdcan_ioctl,
        NULL,
        esdcan_open,
        NULL,
        esdcan_release,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
#elif LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
        owner:   THIS_MODULE,
/*      poll:    esdcan_poll, */ /* no support for poll/select on 2.4.x, yet */
        ioctl:   esdcan_ioctl,
        open:    esdcan_open,
        release: esdcan_release,
#else /* LINUX_VERSION_CODE < KERNEL_VERSION(2,4,0) */
        .owner =   THIS_MODULE,
        .poll =    esdcan_poll,
# if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36)
        .ioctl =   esdcan_ioctl,
# endif /* #if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36) */
# if (PLATFORM_64BIT) && (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,10))
        .compat_ioctl = esdcan_compat_ioctl,
# endif /* #if (PLATFORM_64BIT) && (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,10)) */
# if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,10)
        .unlocked_ioctl = esdcan_unlocked_ioctl,
# endif /* #if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,10) */
        .open =    esdcan_open,
        .release = esdcan_release,
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(2,4,0) */
};
#endif /* OSIF_OS_RTAI */

#define PROCFN_LEN 30
#define PROC_NODEINIT
#if (defined PROC_NODEINIT) && ( LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0) ) && CONFIG_PROC_FS
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
#include <linux/namei.h>
#else /* #if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0) */
#include <linux/fs.h>
#endif /* #if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0) */

static UINT8 procfn_nodeinit[PROCFN_LEN];
static UINT8 procfn_cardpath[PROCFN_LEN];

static int esdcan_path_check(const char *name) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,39)
        struct path p;
        return kern_path(name, 0, &p);
#else /* #if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,39) */
        struct nameidata nd;
        return path_lookup(name, 0, &nd);
#endif /* #if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,39) */
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,8,0)
/* the new way: provide file_ops table to proc_create() */

#include <linux/seq_file.h>
static int esdcan_proc_show(struct seq_file *m, void *v)
{
        int n, num = 0;

        seq_printf(m, "#!/bin/sh\n");
        for (n = 0; n < MAX_CARDS * NODES_PER_CARD; n++) {
                CAN_NODE *node = nodes[n];
                if ( node ) {
                        num++;
                        seq_printf(m,"net_num=$((${1}+%d))\n", node->net_no);
                        seq_printf(m,"test -e /dev/can${net_num} || mknod -m 666 /dev/can${net_num} c %d %d\n",
                                        major, node->net_no);
                }
        }
        if ( !num ) {
                seq_printf(m,"echo \"No CAN card found!\"\n");
        }
        return 0;
}

static int esdcan_proc_open(struct inode *inode, struct file *file)
{
        return single_open(file, esdcan_proc_show, NULL);
}

static const struct file_operations esdcan_proc_file_ops = {
        .owner          = THIS_MODULE,
        .open           = esdcan_proc_open,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
 };
#define ESDCAN_USE_PROC_CREATE

#else /* #if LINUX_VERSION_CODE >= KERNEL_VERSION(3,8,0) */

/* the old way: provide the following function to create_proc_read_entry() */
int can_read_proc_nodeinit(char *buf, char **start, off_t offset, int count, int *eof, void *data)
{
        CAN_NODE **nodes = (CAN_NODE**)data;
        INT32      len = 0, num = 0, result;
        int        n;
        char      *tmp_buf;
        const int  BUFSIZE = 4096;

        CAN_DBG((ESDCAN_ZONE_INI, "%s: buf: 0x%08X, start: 0x%08X, offs: %d, count: %d, eof: %d\n",
                 OSIF_FUNCTION, buf, start, offset, count, *eof));
        result = OSIF_MALLOC(BUFSIZE, &tmp_buf);
        if(OSIF_SUCCESS != result) {
                *eof = 1;
                return 0;
        }
        len += snprintf(tmp_buf+len, BUFSIZE-len, "#!/bin/sh\n");
        for (n = 0; n < MAX_CARDS * NODES_PER_CARD; n++) {
                CAN_NODE *node = nodes[n];
                if ( node ) {
                        if ( 80 >= (BUFSIZE-len) ) {
                                /* shouldn't happen */
                                CAN_DBG((ESDCAN_ZONE_INI, "%s: Not enough memory for inode script!\n",
                                         OSIF_FUNCTION));
                                break;
                        }
                        num++;
                        len += snprintf(tmp_buf+len, BUFSIZE-len,
                                        "net_num=$((${1}+%d))\n", node->net_no);
                        len += snprintf(tmp_buf+len, BUFSIZE-len,
                                        "test -e /dev/can${net_num} || mknod -m 666 /dev/can${net_num} c %d %d\n",
                                        major, node->net_no);
                }
        }
        if ( !num ) {
                len += snprintf(tmp_buf+len, BUFSIZE-len,
                                "echo \"No CAN card found!\"\n");
        }
        CAN_DBG((ESDCAN_ZONE_INI, "%s: complete script len: %d\n",
                 OSIF_FUNCTION, len));
        if ((offset+count) >= len) {
                *eof = 1;
                len -= offset;
        } else {
                len = count;
        }
        OSIF_MEMCPY(buf+offset, tmp_buf+offset, len);
        OSIF_FREE(tmp_buf);
        CAN_DBG((ESDCAN_ZONE_INI, "%s: len: %d, eof: %d\n",
                 OSIF_FUNCTION, len, *eof));
        return offset+len;
}
#endif /* #if LINUX_VERSION_CODE >= KERNEL_VERSION(3,8,0) */

void esdcan_proc_nodeinit_create(void)
{
        int   err;
        char  sPath[PROCFN_LEN+6];

        OSIF_SNPRINTF((procfn_nodeinit, PROCFN_LEN, "bus/can/%s/inodes", ESDCAN_DRIVER_NAME));
        OSIF_SNPRINTF((procfn_cardpath, PROCFN_LEN, "bus/can/%s"       , ESDCAN_DRIVER_NAME));
        CAN_DBG((ESDCAN_ZONE_INI, "%s: creating proc entry (%s)\n",
                 OSIF_FUNCTION, procfn_nodeinit));
        /* create /proc/bus/can */
        err = esdcan_path_check("/proc/bus/can");
        if (err) {
                struct proc_dir_entry *ent;
                ent=proc_mkdir("bus/can", NULL);
                if (ent==NULL) {
                    CAN_PRINT(("%s: error creating proc entry (%s)\n", OSIF_FUNCTION, "bus/can"));
                    procfn_nodeinit[0]=0;
                    procfn_cardpath[0]=0;
                    return;
                }
        }
        /* create /proc/bus/can/<DRIVER_NAME> */
        OSIF_SNPRINTF((sPath, PROCFN_LEN+6,"/proc/%s", procfn_cardpath));
        err = esdcan_path_check(sPath);
        if (err) {
                struct proc_dir_entry *ent;
                ent=proc_mkdir(procfn_cardpath, NULL);
                if (ent==NULL) {
                        CAN_PRINT(("%s: error creating proc entry (%s)\n", OSIF_FUNCTION, procfn_cardpath));
                        procfn_nodeinit[0]=0;
                        procfn_cardpath[0]=0;
                        return;
                }
        }
        /* create /proc/bus/can/<DRIVER_NAME>/inodes */
#ifdef ESDCAN_USE_PROC_CREATE
        if (!proc_create( procfn_nodeinit,
                                     S_IFREG | S_IRUGO | S_IXUSR, /* read all, execute user */
                                     NULL /* parent dir */,
                                     &esdcan_proc_file_ops)) {
#else /* #ifdef ESDCAN_USE_PROC_CREATE */
        if (!create_proc_read_entry( procfn_nodeinit,
                                     0x16D /* default mode */,
                                     NULL /* parent dir */,
                                     can_read_proc_nodeinit,
                                     (VOID*)nodes /* arg */)) {
#endif /* #ifdef ESDCAN_USE_PROC_CREATE */
                CAN_PRINT(("%s: error creating proc entry (%s)\n", OSIF_FUNCTION, procfn_nodeinit));
                procfn_nodeinit[0]=0;
        }
}

void esdcan_proc_nodeinit_remove(void)
{
        if (procfn_nodeinit[0]) {
                CAN_DBG((ESDCAN_ZONE_INI, "%s: removing proc entry %s\n",
                         OSIF_FUNCTION, procfn_nodeinit));
                remove_proc_entry(procfn_nodeinit, NULL);
                procfn_nodeinit[0]=0;
        }
        if (procfn_cardpath[0]) {
                CAN_DBG((ESDCAN_ZONE_INI, "%s: removing proc entry %s\n",
                         OSIF_FUNCTION, procfn_cardpath));
                remove_proc_entry(procfn_cardpath, NULL);
                procfn_cardpath[0]=0;
        }
}
#else /* #if (defined PROC_NODEINIT) && ( LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0) ) && CONFIG_PROC_FS */
void esdcan_proc_nodeinit_create(void) {}
void esdcan_proc_nodeinit_remove(void) {}
#endif /* #if (defined PROC_NODEINIT) && ( LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0) ) && CONFIG_PROC_FS */

#if (PLATFORM_64BIT) && (LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,10))
static int esdcan_ioctl_32(unsigned int fd, unsigned int cmd, unsigned long arg, struct file *file);

void esdcan_unregister_ioctl32(void)
{
        if (compat32) {
                UNREGISTER_IOCTL_32(IOCTL_ESDCAN_CREATE);
                UNREGISTER_IOCTL_32(IOCTL_ESDCAN_ID_RANGE_ADD);
                UNREGISTER_IOCTL_32(IOCTL_ESDCAN_ID_RANGE_DEL);
                UNREGISTER_IOCTL_32(IOCTL_ESDCAN_SET_BAUD);
                UNREGISTER_IOCTL_32(IOCTL_ESDCAN_SET_TIMEOUT);
                UNREGISTER_IOCTL_32(IOCTL_ESDCAN_SEND);
                UNREGISTER_IOCTL_32(IOCTL_ESDCAN_TAKE);
                UNREGISTER_IOCTL_32(IOCTL_ESDCAN_DEBUG);
                UNREGISTER_IOCTL_32(IOCTL_ESDCAN_GET_BAUD);
                UNREGISTER_IOCTL_COMPAT(IOCTL_ESDCAN_DESTROY);
                UNREGISTER_IOCTL_32(IOCTL_ESDCAN_DESTROY_DEPRECATED);
                UNREGISTER_IOCTL_32(IOCTL_ESDCAN_TX_ABORT);
                UNREGISTER_IOCTL_32(IOCTL_ESDCAN_RX_ABORT);
                UNREGISTER_IOCTL_32(IOCTL_ESDCAN_TX_OBJ_CREATE);
                UNREGISTER_IOCTL_32(IOCTL_ESDCAN_TX_OBJ_UPDATE);
                UNREGISTER_IOCTL_32(IOCTL_ESDCAN_TX_OBJ_AUTOANSWER_OFF);
                UNREGISTER_IOCTL_32(IOCTL_ESDCAN_TX_OBJ_DESTROY);
                UNREGISTER_IOCTL_32(IOCTL_ESDCAN_RX_OBJ);
                UNREGISTER_IOCTL_32(IOCTL_ESDCAN_READ);
                UNREGISTER_IOCTL_32(IOCTL_ESDCAN_WRITE);
                UNREGISTER_IOCTL_32(IOCTL_ESDCAN_UPDATE);
                UNREGISTER_IOCTL_COMPAT(IOCTL_ESDCAN_FLUSH_RX_FIFO);
                UNREGISTER_IOCTL_32(IOCTL_ESDCAN_FLUSH_RX_FIFO_DEPRECATED);
                UNREGISTER_IOCTL_32(IOCTL_ESDCAN_GET_RX_MESSAGES);
                UNREGISTER_IOCTL_32(IOCTL_ESDCAN_GET_RX_TIMEOUT);
                UNREGISTER_IOCTL_32(IOCTL_ESDCAN_GET_TX_TIMEOUT);
                UNREGISTER_IOCTL_32(IOCTL_ESDCAN_GET_TIMESTAMP_FREQ);
                UNREGISTER_IOCTL_32(IOCTL_ESDCAN_GET_TIMESTAMP);
                UNREGISTER_IOCTL_32(IOCTL_ESDCAN_GET_TICKS_FREQ);
                UNREGISTER_IOCTL_32(IOCTL_ESDCAN_GET_TICKS);
                UNREGISTER_IOCTL_32(IOCTL_ESDCAN_SEND_T);
                UNREGISTER_IOCTL_32(IOCTL_ESDCAN_WRITE_T);
                UNREGISTER_IOCTL_32(IOCTL_ESDCAN_TAKE_T);
                UNREGISTER_IOCTL_32(IOCTL_ESDCAN_READ_T);
                UNREGISTER_IOCTL_COMPAT(IOCTL_ESDCAN_PURGE_TX_FIFO);
                UNREGISTER_IOCTL_32(IOCTL_ESDCAN_PURGE_TX_FIFO_DEPRECATED);
                UNREGISTER_IOCTL_32(IOCTL_ESDCAN_TX_OBJ_AUTOANSWER_ON);
                UNREGISTER_IOCTL_32(IOCTL_ESDCAN_SET_RX_TIMEOUT);
                UNREGISTER_IOCTL_32(IOCTL_ESDCAN_SET_TX_TIMEOUT);
                UNREGISTER_IOCTL_32(IOCTL_ESDCAN_GET_FEATURES);
                UNREGISTER_IOCTL_32(IOCTL_ESDCAN_TX_OBJ_SCHEDULE);
                UNREGISTER_IOCTL_COMPAT(IOCTL_ESDCAN_TX_OBJ_SCHEDULE_START);
                UNREGISTER_IOCTL_COMPAT(IOCTL_ESDCAN_TX_OBJ_SCHEDULE_STOP);
                UNREGISTER_IOCTL_32(IOCTL_ESDCAN_SET_20B_HND_FILTER);
                UNREGISTER_IOCTL_32(IOCTL_ESDCAN_GET_SERIAL);
                UNREGISTER_IOCTL_COMPAT(IOCTL_ESDCAN_SET_ALT_RTR_ID);
                UNREGISTER_IOCTL_32(IOCTL_ESDCAN_SET_BUSLOAD_INTERVAL);
                UNREGISTER_IOCTL_32(IOCTL_ESDCAN_GET_BUSLOAD_INTERVAL);
                UNREGISTER_IOCTL_32(IOCTL_ESDCAN_GET_BITRATE_DETAILS);
                UNREGISTER_IOCTL_32(IOCTL_ESDCAN_GET_BUS_STATISTIC);
                UNREGISTER_IOCTL_COMPAT(IOCTL_ESDCAN_RESET_BUS_STATISTIC);
                UNREGISTER_IOCTL_32(IOCTL_ESDCAN_GET_ERROR_COUNTER);
                UNREGISTER_IOCTL_32(IOCTL_ESDCAN_SER_REG_READ);
                UNREGISTER_IOCTL_32(IOCTL_ESDCAN_SER_REG_WRITE);
                UNREGISTER_IOCTL_COMPAT(IOCTL_ESDCAN_RESET_CAN_ERROR_CNT);
                UNREGISTER_IOCTL_32(IOCTL_ESDCAN_ID_REGION_ADD);
                UNREGISTER_IOCTL_32(IOCTL_ESDCAN_ID_REGION_DEL);
                UNREGISTER_IOCTL_32(IOCTL_ESDCAN_SET_TX_TS_WIN);
                UNREGISTER_IOCTL_32(IOCTL_ESDCAN_GET_TX_TS_WIN);
                UNREGISTER_IOCTL_32(IOCTL_ESDCAN_SET_TX_TS_TIMEOUT);
                UNREGISTER_IOCTL_32(IOCTL_ESDCAN_GET_TX_TS_TIMEOUT);
                UNREGISTER_IOCTL_32(IOCTL_ESDCAN_SET_HND_FILTER);
                UNREGISTER_IOCTL_32(IOCTL_ESDCAN_EEI_CREATE);
                UNREGISTER_IOCTL_32(IOCTL_ESDCAN_EEI_DESTROY);
                UNREGISTER_IOCTL_32(IOCTL_ESDCAN_EEI_STATUS);
                UNREGISTER_IOCTL_32(IOCTL_ESDCAN_EEI_CONFIGURE);
                UNREGISTER_IOCTL_32(IOCTL_ESDCAN_EEI_START);
                UNREGISTER_IOCTL_32(IOCTL_ESDCAN_EEI_STOP);
                UNREGISTER_IOCTL_32(IOCTL_ESDCAN_EEI_TRIGGER_NOW);
                UNREGISTER_IOCTL_COMPAT(IOCTL_CAN_SET_QUEUESIZE); /* special ioctl (from candev driver) for driver version detection */
                UNREGISTER_IOCTL_32(IOCTL_ESDCAN_SEND_X);
                UNREGISTER_IOCTL_32(IOCTL_ESDCAN_WRITE_X);
                UNREGISTER_IOCTL_32(IOCTL_ESDCAN_TAKE_X);
                UNREGISTER_IOCTL_32(IOCTL_ESDCAN_READ_X);
                UNREGISTER_IOCTL_32(IOCTL_ESDCAN_SET_BAUD_X);
                UNREGISTER_IOCTL_32(IOCTL_ESDCAN_GET_BAUD_X);
        }
}

int32_t esdcan_register_ioctl32(void)
{
        int32_t result = 0;

        if (compat32) {
                /* register 32-bit ioctls */
                REGISTER_IOCTL_32(IOCTL_ESDCAN_CREATE, result);
                REGISTER_IOCTL_32(IOCTL_ESDCAN_ID_RANGE_ADD, result);
                REGISTER_IOCTL_32(IOCTL_ESDCAN_ID_RANGE_DEL, result);
                REGISTER_IOCTL_32(IOCTL_ESDCAN_SET_BAUD, result);
                REGISTER_IOCTL_32(IOCTL_ESDCAN_SET_TIMEOUT, result);
                REGISTER_IOCTL_32(IOCTL_ESDCAN_SEND, result);
                REGISTER_IOCTL_32(IOCTL_ESDCAN_TAKE, result);
                REGISTER_IOCTL_32(IOCTL_ESDCAN_DEBUG, result);
                REGISTER_IOCTL_32(IOCTL_ESDCAN_GET_BAUD, result);
                REGISTER_IOCTL_COMPAT(IOCTL_ESDCAN_DESTROY, result);
                REGISTER_IOCTL_32(IOCTL_ESDCAN_DESTROY_DEPRECATED, result);
                REGISTER_IOCTL_32(IOCTL_ESDCAN_TX_ABORT, result);
                REGISTER_IOCTL_32(IOCTL_ESDCAN_RX_ABORT, result);
                REGISTER_IOCTL_32(IOCTL_ESDCAN_TX_OBJ_CREATE, result);
                REGISTER_IOCTL_32(IOCTL_ESDCAN_TX_OBJ_UPDATE, result);
                REGISTER_IOCTL_32(IOCTL_ESDCAN_TX_OBJ_AUTOANSWER_OFF, result);
                REGISTER_IOCTL_32(IOCTL_ESDCAN_TX_OBJ_DESTROY, result);
                REGISTER_IOCTL_32(IOCTL_ESDCAN_RX_OBJ, result);
                REGISTER_IOCTL_32(IOCTL_ESDCAN_READ, result);
                REGISTER_IOCTL_32(IOCTL_ESDCAN_WRITE, result);
                REGISTER_IOCTL_32(IOCTL_ESDCAN_UPDATE, result);
                REGISTER_IOCTL_COMPAT(IOCTL_ESDCAN_FLUSH_RX_FIFO, result);
                REGISTER_IOCTL_32(IOCTL_ESDCAN_FLUSH_RX_FIFO_DEPRECATED, result);
                REGISTER_IOCTL_32(IOCTL_ESDCAN_GET_RX_MESSAGES, result);
                REGISTER_IOCTL_32(IOCTL_ESDCAN_GET_RX_TIMEOUT, result);
                REGISTER_IOCTL_32(IOCTL_ESDCAN_GET_TX_TIMEOUT, result);
                REGISTER_IOCTL_32(IOCTL_ESDCAN_GET_TIMESTAMP_FREQ, result);
                REGISTER_IOCTL_32(IOCTL_ESDCAN_GET_TIMESTAMP, result);
                REGISTER_IOCTL_32(IOCTL_ESDCAN_GET_TICKS_FREQ, result);
                REGISTER_IOCTL_32(IOCTL_ESDCAN_GET_TICKS, result);
                REGISTER_IOCTL_32(IOCTL_ESDCAN_SEND_T, result);
                REGISTER_IOCTL_32(IOCTL_ESDCAN_WRITE_T, result);
                REGISTER_IOCTL_32(IOCTL_ESDCAN_TAKE_T, result);
                REGISTER_IOCTL_32(IOCTL_ESDCAN_READ_T, result);
                REGISTER_IOCTL_COMPAT(IOCTL_ESDCAN_PURGE_TX_FIFO, result);
                REGISTER_IOCTL_32(IOCTL_ESDCAN_PURGE_TX_FIFO_DEPRECATED, result);
                REGISTER_IOCTL_32(IOCTL_ESDCAN_TX_OBJ_AUTOANSWER_ON, result);
                REGISTER_IOCTL_32(IOCTL_ESDCAN_SET_RX_TIMEOUT, result);
                REGISTER_IOCTL_32(IOCTL_ESDCAN_SET_TX_TIMEOUT, result);
                REGISTER_IOCTL_32(IOCTL_ESDCAN_GET_FEATURES, result);
                REGISTER_IOCTL_32(IOCTL_ESDCAN_TX_OBJ_SCHEDULE, result);
                REGISTER_IOCTL_COMPAT(IOCTL_ESDCAN_TX_OBJ_SCHEDULE_START, result);
                REGISTER_IOCTL_COMPAT(IOCTL_ESDCAN_TX_OBJ_SCHEDULE_STOP, result);
                REGISTER_IOCTL_32(IOCTL_ESDCAN_SET_20B_HND_FILTER, result);
                REGISTER_IOCTL_32(IOCTL_ESDCAN_GET_SERIAL, result);
                REGISTER_IOCTL_COMPAT(IOCTL_ESDCAN_SET_ALT_RTR_ID, result);
                REGISTER_IOCTL_32(IOCTL_ESDCAN_SET_BUSLOAD_INTERVAL, result);
                REGISTER_IOCTL_32(IOCTL_ESDCAN_GET_BUSLOAD_INTERVAL, result);
                REGISTER_IOCTL_32(IOCTL_ESDCAN_GET_BITRATE_DETAILS, result);
                REGISTER_IOCTL_32(IOCTL_ESDCAN_GET_BUS_STATISTIC, result);
                REGISTER_IOCTL_COMPAT(IOCTL_ESDCAN_RESET_BUS_STATISTIC, result);
                REGISTER_IOCTL_32(IOCTL_ESDCAN_GET_ERROR_COUNTER, result);
                REGISTER_IOCTL_32(IOCTL_ESDCAN_SER_REG_READ, result);
                REGISTER_IOCTL_32(IOCTL_ESDCAN_SER_REG_WRITE, result);
                REGISTER_IOCTL_COMPAT(IOCTL_ESDCAN_RESET_CAN_ERROR_CNT, result);
                REGISTER_IOCTL_32(IOCTL_ESDCAN_ID_REGION_ADD, result);
                REGISTER_IOCTL_32(IOCTL_ESDCAN_ID_REGION_DEL, result);
                REGISTER_IOCTL_32(IOCTL_ESDCAN_SET_TX_TS_WIN, result);
                REGISTER_IOCTL_32(IOCTL_ESDCAN_GET_TX_TS_WIN, result);
                REGISTER_IOCTL_32(IOCTL_ESDCAN_SET_TX_TS_TIMEOUT, result);
                REGISTER_IOCTL_32(IOCTL_ESDCAN_GET_TX_TS_TIMEOUT, result);
                REGISTER_IOCTL_32(IOCTL_ESDCAN_EEI_CREATE, result);
                REGISTER_IOCTL_32(IOCTL_ESDCAN_EEI_DESTROY, result);
                REGISTER_IOCTL_32(IOCTL_ESDCAN_EEI_STATUS, result);
                REGISTER_IOCTL_32(IOCTL_ESDCAN_EEI_CONFIGURE, result);
                REGISTER_IOCTL_32(IOCTL_ESDCAN_EEI_START, result);
                REGISTER_IOCTL_32(IOCTL_ESDCAN_EEI_STOP, result);
                REGISTER_IOCTL_32(IOCTL_ESDCAN_EEI_TRIGGER_NOW, result);
                REGISTER_IOCTL_COMPAT(IOCTL_CAN_SET_QUEUESIZE, result); /* special ioctl (from candev driver) for driver version detection */
                REGISTER_IOCTL_32(IOCTL_ESDCAN_SEND_X, result);
                REGISTER_IOCTL_32(IOCTL_ESDCAN_WRITE_X, result);
                REGISTER_IOCTL_32(IOCTL_ESDCAN_TAKE_X, result);
                REGISTER_IOCTL_32(IOCTL_ESDCAN_READ_X, result);
                REGISTER_IOCTL_32(IOCTL_ESDCAN_SET_BAUD_X, result);
                REGISTER_IOCTL_32(IOCTL_ESDCAN_GET_BAUD_X, result);
                if ( 0 != result ) {
                        CAN_PRINT(("esd CAN driver: An ioctl collides with an ioctl of another!\n"));
                        CAN_PRINT(("esd CAN driver: driver on your system!\n"));
                        CAN_PRINT(("esd CAN driver: Since this is a problem of the 32-bit compatibility\n"));
                        CAN_PRINT(("esd CAN driver: layer of linux, you can still load the driver with\n"));
                        CAN_PRINT(("esd CAN driver: module parameter \"compat32=0\", but you won't be\n"));
                        CAN_PRINT(("esd CAN driver: able to use 32-bit applications.\n"));
                        esdcan_unregister_ioctl32();
                }
        }
        return result;
}
#else /* #if (PLATFORM_64BIT) && (LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,10)) */
int32_t esdcan_register_ioctl32(void) {return 0;}
void esdcan_unregister_ioctl32(void) {}
#endif /* #if (PLATFORM_64BIT) && (LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,10)) */

/*
 *  The following includes contain system-specific code, which helps to reduce code
 *  duplication and ifdefs for different hardware.
 */
#if defined (BOARD_USB)
# if defined(OSIF_OS_LINUX)
#  include "esdcan_usb.c"
# else /* #if defined(OSIF_OS_LINUX) */
#  error "Currently USB is implemented for Linux, only!"
# endif /* #if defined(OSIF_OS_LINUX) */
#elif defined(ESDCAN_PCI_HOTPLUG)
# include "esdcan_pci.c"
#elif defined(BOARD_SPI)
# include "esdcan_spi.c"
#elif defined(BOARD_RAW)
# include "esdcan_raw.c"
#elif defined(BOARD_PCI)
# include "esdcan_pci_legacy.c"
#elif defined(BOARD_VME)
# include "esdcan_vme.c"
#elif defined(BOARD_SOFT)
# include "esdcan_soft.c"
#else /* #if defined (BOARD_USB) */
# error "Board type not properly defined!!!"
#endif /* #if defined (BOARD_USB) */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,19)
static irqreturn_t irq_stub( int irq, void *arg )
{
        return ((CARD_IRQ*)arg)->handler(((CARD_IRQ*)arg)->context);
}
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
static irqreturn_t irq_stub( int irq, void *arg, struct pt_regs* regs )
{
        return ((CARD_IRQ*)arg)->handler(((CARD_IRQ*)arg)->context);
}
#else
static void irq_stub( int irq, void *arg, struct pt_regs* regs )
{
        ((CARD_IRQ*)arg)->handler(((CARD_IRQ*)arg)->context);
        return;
}
#endif

#include "esdcan_common.c" /* include common part of unix system layer */

#ifdef OSIF_OS_LINUX
static int esdcan_open(struct inode *inode, struct file *file)
{
        CAN_DBG((ESDCAN_ZONE_FU, "%s: pre LOCK\n", OSIF_FUNCTION));
        HOTPLUG_GLOBAL_LOCK;
# if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
        MOD_INC_USE_COUNT;
# endif /* # if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0) */
        CAN_DBG((ESDCAN_ZONE_FU, "%s: enter\n", OSIF_FUNCTION));
        file->private_data = 0;
        HOTPLUG_GLOBAL_UNLOCK;
        CAN_DBG((ESDCAN_ZONE_FU, "%s: leave\n", OSIF_FUNCTION));
        RETURN_TO_USER(OSIF_SUCCESS);
}
#endif /* #ifdef OSIF_OS_LINUX */

#ifdef OSIF_OS_LINUX
static int esdcan_release(struct inode *inode, struct file *file)
{
        INT32 result = OSIF_SUCCESS;
        CAN_OCB *ocb = (CAN_OCB*)file->private_data;

        CAN_DBG((ESDCAN_ZONE_FU, "%s: enter\n", OSIF_FUNCTION));
        if (ocb) {
                CAN_NODE *node = ocb->node;

                if ( 0 != HOTPLUG_BOLT_USER_ENTRY( ((CAN_CARD*)node->crd)->p_mod ) ) { /* BL: seems no good, needs rework */
                        CAN_DBG((ESDCAN_ZONE_FU, "%s: release needs abort!!!\n", OSIF_FUNCTION));
                        RETURN_TO_USER(result);
                }
                OSIF_MUTEX_LOCK(&node->lock);
                file->private_data = 0;
                if (!(ocb->close_state & CLOSE_STATE_HANDLE_CLOSED)) {
                        result = ocb_destroy(ocb);
                }
                OSIF_FREE(ocb->rx.cmbuf);
                OSIF_FREE(ocb);
                OSIF_MUTEX_UNLOCK(&node->lock);
                HOTPLUG_BOLT_USER_EXIT( ((CAN_CARD*)node->crd)->p_mod );
        }
# if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
        MOD_DEC_USE_COUNT;
# endif /* # if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0) */
        CAN_DBG((ESDCAN_ZONE_FU, "%s: leave\n", OSIF_FUNCTION));
        RETURN_TO_USER(result);
}
#endif /* OSIF_OS_LINUX */

INT32 can_attach_common(CAN_CARD *crd)
{
        VOID       *vptr;
        INT32       idx_node, idx_irq;
        INT32       result = OSIF_SUCCESS, lastError = OSIF_SUCCESS;
        CARD_IDENT *pCardIdent;

        CAN_DBG((ESDCAN_ZONE_INI, "%s: enter (crd=%p)\n", OSIF_FUNCTION, crd));
        for(idx_node = 0; idx_node < NODES_PER_CARD; idx_node++) {
                CAN_NODE *node;

                result = OSIF_MALLOC(sizeof(*node), &vptr);
                if(OSIF_SUCCESS != result) {
                        break;
                }
                node = (CAN_NODE*)vptr;
                OSIF_MEMSET(node, 0, sizeof(*node));
                node->crd     = crd;
                node->mode    = mode;  /* insmod parameter */
                node->net_no  = crd->card_no*NODES_PER_CARD + idx_node;
                node->node_no = idx_node;
                init_waitqueue_head(&node->wqRxNotify);
                if (clock != 0) {
                        node->ctrl_clock = clock;
                }
                node->tx_ts_win = txtswin;
                OSIF_MUTEX_CREATE(&node->lock);
                OSIF_IRQ_MUTEX_CREATE(&node->lock_irq);
                crd->node[idx_node] = node;
        }
        if( OSIF_SUCCESS != result ) {
                lastError = result;
                goto common_attach_1;
        }
        result = can_board_attach(crd);
        if( OSIF_SUCCESS != result ) {
                lastError = result;
                if( OSIF_ERESTART == result ) {
                        CAN_PRINT(("esd CAN driver: !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"));
                        CAN_PRINT(("esd CAN driver: !!!!!      Bootloader update necessary.          !!!!!\n"));
                        CAN_PRINT(("esd CAN driver: !!!!!    Load a driver with version < 3.9.7      !!!!!\n"));
                        CAN_PRINT(("esd CAN driver: !!!!!   to automatically update the bootloader.  !!!!!\n"));
                        CAN_PRINT(("esd CAN driver: !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"));
                }
                goto common_attach_1;   /* AB TODO: this leads to an undestroyed crd->lock_irq !!! */
        }
        pCardIdent = crd->pCardIdent;
        for(idx_irq = 0;
            (idx_irq < 8) &&
            (pCardIdent->all.irqs[idx_irq].handler != (VOID*)(~0x0));
            idx_irq++) {
                if (NULL == pCardIdent->all.irqs[idx_irq].handler) {
                        continue;
                }
/* BL TODO: timestamps on baudrate change events
Idea: force affinity to the same CPU as DPC thread
set_ioapic_affinity
irq_desc[irq].handler->set_affinity(irq, new_value)
UINT32 cpus = num_online_cpus();
cpumask_t mask;
cpus_clear(mask);
cpus -= 1;
cpu_set(cpus, mask);
long sched_setaffinity(pid_t pid, cpumask_t new_mask)
 */
                crd->irqs[idx_irq] = pCardIdent->all.irqs[idx_irq];
                if (0 != request_irq(crd->irq[idx_irq],
                                     (void*)irq_stub,
                                     ESDCAN_IRQ_FLAGS,
                                     pCardIdent->all.name,
                                     &crd->irqs[idx_irq])) {
                        crd->irqs[idx_irq].handler = NULL;
                        crd->irqs[idx_irq].context = NULL;
                        result = CANIO_EBUSY;
                        CAN_DBG((ESDCAN_ZONE_INI,
                                 "%s: IRQ request FAILURE: IRQ: %d, Stub: %p, Param: %p\n",
                                 OSIF_FUNCTION,
                                 crd->irq[idx_irq],
                                 (void*)irq_stub,
                                 &crd->irqs[idx_irq]));
                        break;
                }
                CAN_DBG((ESDCAN_ZONE_INI, "%s: irq requested: IRQ: %d, Stub: %p, Param: %p\n",
                         OSIF_FUNCTION,
                         crd->irq[idx_irq],
                         (void*)irq_stub,
                         &crd->irqs[idx_irq]));
        }
        if( OSIF_SUCCESS != result ) {
                lastError = result;
                goto common_attach_2;
        }
        crd->num_nodes = 0;
        for( idx_node = 0; idx_node < NODES_PER_CARD; idx_node++ ) {
                CAN_NODE *node = crd->node[idx_node];
                if(NULL == node) {
                        continue;
                }
                node->features |= crd->features;
                CAN_DBG((ESDCAN_ZONE_INI, "%s: calling nuc_node_attach\n", OSIF_FUNCTION));
                result = nuc_node_attach( node, 0 );
                if (OSIF_SUCCESS != result) {
                        CAN_DBG((ESDCAN_ZONE_INI, "%s: nuc_node_attach failed (%d)\n", OSIF_FUNCTION, result));
                        OSIF_IRQ_MUTEX_DESTROY( &node->lock_irq );
                        OSIF_MUTEX_DESTROY( &node->lock );
                        OSIF_FREE( node );

                        crd->node[idx_node] = NULL;
                }
                nodes[crd->card_no*NODES_PER_CARD + idx_node] = crd->node[idx_node];
                if ( NULL == crd->node[idx_node]) {
                        continue;
                }
                CAN_DBG((ESDCAN_ZONE_INI, "%s: Node attached (features=%08x)\n", OSIF_FUNCTION, node->features));
                ++crd->num_nodes;
        }
        if( 0 == crd->num_nodes ) {
                CAN_DBG((ESDCAN_ZONE_INI, "%s: no node working\n", OSIF_FUNCTION));
                lastError = result;
                goto common_attach_3;
        }
        result = can_board_attach_final(crd);
        if( OSIF_SUCCESS != result ) {
                lastError = result;
                goto common_attach_3;
        }
        CAN_DBG((ESDCAN_ZONE_INI, "%s: leave (result=%d)\n", OSIF_FUNCTION, result));
        return result;

 common_attach_3:
        CAN_DBG((ESDCAN_ZONE_INI,
                 "%s: common_attach_3: can_board_attach_final or nuc_node_attach failed\n",
                 OSIF_FUNCTION));
        for( idx_node = 0; idx_node < NODES_PER_CARD; idx_node++ ) {
                CAN_NODE *node = crd->node[idx_node];
                if ( NULL == node ) {
                        continue;
                }
                nodes[crd->card_no * NODES_PER_CARD + idx_node] = NULL;
                nuc_node_detach( node );
        }

 common_attach_2:
        CAN_DBG((ESDCAN_ZONE_INI,
                 "%s: common_attach_2: Failed to request IRQ\n",
                 OSIF_FUNCTION));
        for( idx_irq = 0;
             (idx_irq < 8) &&
             (pCardIdent->all.irqs[idx_irq].handler != (VOID*)(-1));
             idx_irq++ ) {
                if( NULL != crd->irqs[idx_irq].context ) {
                        free_irq(crd->irq[idx_irq], &crd->irqs[idx_irq]);
                }
        }
        can_board_detach(crd);

 common_attach_1:
        CAN_DBG((ESDCAN_ZONE_INI,
                 "%s: common_attach_1: Allocation of node-struct or can_board_attach failed\n",
                 OSIF_FUNCTION));
        for( idx_node-- /* current idx did't get the mem */;
             idx_node >= 0;
             idx_node--) {
                CAN_NODE *node = crd->node[idx_node];
                if ( NULL == node ) {
                        continue;
                }
                OSIF_IRQ_MUTEX_DESTROY(&node->lock_irq);
                OSIF_MUTEX_DESTROY(&node->lock);
                OSIF_FREE(node);
        }
        CAN_DBG((ESDCAN_ZONE_INI, "%s: leave (error=%d)\n", OSIF_FUNCTION, lastError));
        return lastError;
}

VOID can_detach_common( CAN_CARD *crd, INT32 crd_no )
{
        UINT32      j;
        CARD_IDENT *pCardIdent = crd->pCardIdent;

        CAN_DBG((ESDCAN_ZONE_INI, "%s: enter\n", OSIF_FUNCTION));
        can_board_detach(crd);
        for ( j = 0; (j < 8) && ((VOID *)(~0x0) != pCardIdent->all.irqs[j].handler); j++ ) {
                if ( NULL != crd->irqs[j].context ) {
                        free_irq(crd->irq[j], &crd->irqs[j]);
                        CAN_DBG((ESDCAN_ZONE_INI, "%s:%d: free irq IRQ: %d\n",
                                 OSIF_FUNCTION, j, crd->irq[j]));
                }
        }
        for ( j = 0; j < NODES_PER_CARD; j++ ) {
                CAN_NODE *node = crd->node[j];
                if ( NULL == node ) {
                        continue;
                }
                nodes[crd_no * NODES_PER_CARD + j] = NULL;
                nuc_node_detach( node );
                OSIF_IRQ_MUTEX_DESTROY( &node->lock_irq);
                OSIF_MUTEX_DESTROY( &node->lock);
                OSIF_FREE(node);
        }
        CAN_BOARD_DETACH_FINAL(crd);
#ifdef ESDCAN_PCI_HOTPLUG
        for ( j = 0; j < 8; j++ ) {
                if ( 0 != crd->range[j] ) {
                        if ( pci_resource_flags(crd->pciDev, j) & IORESOURCE_MEM ) {
                                /* Memory space */
                                iounmap(crd->base[j]);
                        } else if ( pci_resource_flags(crd->pciDev, j) & IORESOURCE_IO ) {
                                /* IO-space */
                                release_region((UINTPTR)crd->base[j], crd->range[j]);
                        }
                }
        }
#else /* !ESDCAN_PCI_HOTPLUG */
# ifndef BOARD_SPI
        for ( j = 0; (j < 8) && (0xFFFFFFFF != pCardIdent->all.spaces[j]); j++ ) {
            if( 0 != (pCardIdent->all.spaces[j] & 0x00000001) ) { /* if io-space */
#  if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0)
                release_region((UINTPTR)crd->base[j], crd->range[j]);
#  else /* #if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0) */
#   warning "AB TODO: Still need mechanism to release mem-regions in 2.2.x kernels!!!"
#  endif /* #if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0) */
            } else {
#  if !(defined(CONFIG_OF_DEVICE) || defined(ONFIG_OF_FLATTREE))
                iounmap(crd->base[j]);
#  endif /* !(CONFIG_OF_DEVICE || COFIG_OF_FLATTREE)*/
            }
        }
# endif /* !BOARD_SPI */
#endif /* ESDCAN_PCI_HOTPLUG */
        CAN_DBG((ESDCAN_ZONE_INI, "%s: leave\n", OSIF_FUNCTION));
        return;
}

VOID esdcan_show_card_info( CAN_CARD *crd )
{
        INT32 i, n;

        if ( (crd->version_firmware.level != 0) ||
             (crd->version_firmware.revision != 0) ||
             (crd->version_firmware.build != 0) ) {
                CAN_DPRINT((OSIF_ZONE_USR_INIT,
                            "esd CAN driver: Firmware-version = %1X.%1X.%02X (hex)\n",
                            crd->version_firmware.level,
                            crd->version_firmware.revision,
                            crd->version_firmware.build));
        }
        if ( (crd->version_hardware.level != 0) ||
             (crd->version_hardware.revision != 0) ||
             (crd->version_hardware.build != 0) ) {
                        CAN_DPRINT((OSIF_ZONE_USR_INIT,
                                    "esd CAN driver: Hardware-version = %1X.%1X.%02X (hex)\n",
                                    crd->version_hardware.level,
                                    crd->version_hardware.revision,
                                    crd->version_hardware.build));
        } else {
                CAN_DPRINT((OSIF_ZONE_USR_INIT, "esd CAN driver: Hardware-version = 1.0.00\n"));
        }
        CAN_DPRINT((OSIF_ZONE_USR_INIT, "esd CAN driver: Card = %d Minor(s) =", crd->card_no));
        for ( i = 0, n = 0; NODES_PER_CARD > i; i++ ) {
                if ( NULL == crd->node[i] ) {
                        continue;
                }
                CAN_DPRINT((OSIF_ZONE_USR_INIT, "%s%d", n++?", ":" ", crd->card_no * NODES_PER_CARD + i));
        }
        CAN_DPRINT((OSIF_ZONE_USR_INIT, "\n"));
        return;
}

/*!
 *  \brief This function prints all information, that might be useful to the user
 *  at the time the driver is loaded. Use this function in init-functions (such as
 *  esdcan_init())
 */
VOID esdcan_show_driver_info( VOID )
{
        CAN_DPRINT((OSIF_ZONE_USR_INIT,
                    "esd CAN driver: %s\n", ESDCAN_DRIVER_NAME));
        CAN_DPRINT((OSIF_ZONE_USR_INIT,
                    "esd CAN driver: mode=0x%08x, major=%d, verbose=0x%08x\n",
                    mode, major, verbose));
        CAN_DPRINT((OSIF_ZONE_USR_INIT,
                    "esd CAN driver: Version %d.%d.%d ("__TIME__", "__DATE__")\n",
                    LEVEL, REVI, BUILD));
        CAN_DPRINT((OSIF_ZONE_USR_INIT, "esd CAN driver: successfully loaded\n"));
#if defined(CAN_DEBUG)
        CAN_PRINT(("esd CAN driver: !!! DEBUG VERSION !!!\n"));
        CAN_PRINT(("Please note:\n"));
        CAN_PRINT(("You're using a debug version of the esd CAN driver.\n"));
        CAN_PRINT(("This version is NOT intended for productive use.\n"));
        CAN_PRINT(("The activated debug code might have bad influence\n"));
        CAN_PRINT(("on performance and system load.\n"));
        CAN_PRINT(("If you received this driver from somebody else than\n"));
        CAN_PRINT(("esd support, please report to \"support@esd.eu\".\n"));
        CAN_PRINT(("Thank you!\n"));
#endif /* #if defined(CAN_DEBUG) */
        return;
}

#if !defined(ESDCAN_HOTPLUG_DRIVER) && !defined(BOARD_SPI)
VOID esdcan_show_card_info_all( VOID )
{
        UINT32 i;

        for ( i = 0; i < MAX_CARDS; i++ ) {
                CAN_CARD *crd;

                HOTPLUG_BOLT_SYSTEM_ENTRY(modules[i]);
                crd = GET_CARD_POINTER(i);
                if ( NULL == crd ) {
                        HOTPLUG_BOLT_SYSTEM_EXIT(modules[i]);
                        continue;
                }
                esdcan_show_card_info(crd);
                HOTPLUG_BOLT_SYSTEM_EXIT(modules[i]);
                CAN_DPRINT((OSIF_ZONE_USR_INIT, "\n"));
        }
        return;
}

INT32 can_detach_legacy( VOID )
{
        INT32 result = OSIF_SUCCESS;
        UINT32 i;

        CAN_DBG((ESDCAN_ZONE_INIFU, "%s: enter\n", OSIF_FUNCTION));
        for( i = 0; i < MAX_CARDS; i ++) {
                CAN_CARD *crd = GET_CARD_POINTER(i);
                if ( NULL == crd ) {
                        continue;
                }
                GET_CARD_POINTER(i) = NULL;
                can_detach_common(crd, i);
                OSIF_FREE( crd );
        }
        CAN_DBG((ESDCAN_ZONE_INIFU, "%s: leave\n", OSIF_FUNCTION));
        return result;
}

/*! Executed ONCE when driver module is loaded */
int OSIF__INIT esdcan_init_legacy(void)
{
        INT32 result = 0;

        CAN_DBG((ESDCAN_ZONE_INIFU, "%s: enter\n", OSIF_FUNCTION));        
        CAN_DPRINT((OSIF_ZONE_USR_INIT, "esd CAN driver: init start\n"));
# ifdef LTT
        can_ltt0 = trace_create_event("CAN_LTT_0",
                                      NULL,
                                      CUSTOM_EVENT_FORMAT_TYPE_HEX,
                                      NULL);
# endif /* LTT */
        if (esdcan_register_ioctl32()) {
                RETURN_TO_USER(OSIF_INVALID_PARAMETER);
        }
        if ( OSIF_ATTACH() ) {
                CAN_DBG((ESDCAN_ZONE_INI, "%s: osif_attach failed!!!\n", OSIF_FUNCTION));
                esdcan_unregister_ioctl32();
                RETURN_TO_USER(OSIF_INSUFFICIENT_RESOURCES);
        }
        result = can_attach();
        if ( result != OSIF_SUCCESS ) {
                OSIF_DETACH();
                esdcan_unregister_ioctl32();
                RETURN_TO_USER(result);
        }
        esdcan_show_driver_info();
        esdcan_show_card_info_all();
# ifndef OSIF_OS_RTAI
        /* register device */
        if ( 0 != register_chrdev( major, ESDCAN_DRIVER_NAME, &esdcan_fops ) ) {
                CAN_PRINT(("esd CAN driver: cannot register character device for major=%d\n", major));
                CAN_DETACH;
                OSIF_DETACH();
                esdcan_unregister_ioctl32();
                RETURN_TO_USER(EBUSY);
        }
# endif /* OSIF_OS_RTAI */
        esdcan_proc_nodeinit_create();
        CAN_DBG((ESDCAN_ZONE_INIFU, "%s: leave\n", OSIF_FUNCTION));
        RETURN_TO_USER(result);
}

/*! Executed when driver module is unloaded */
void OSIF__EXIT esdcan_exit_legacy(void)
{
        CAN_DBG((ESDCAN_ZONE_INIFU, "%s: enter, unloading esd CAN driver\n", OSIF_FUNCTION));
        unregister_chrdev(major, ESDCAN_DRIVER_NAME);
        esdcan_unregister_ioctl32();
        CAN_DETACH;
# ifdef LTT
        trace_destroy_event(can_ltt0);
# endif /* #ifdef LTT */
        esdcan_proc_nodeinit_remove();
        OSIF_DETACH();
        CAN_DPRINT((OSIF_ZONE_USR_INIT, "esd CAN driver: unloaded\n"));
        CAN_DBG((ESDCAN_ZONE_INIFU, "%s: leave\n", OSIF_FUNCTION));
}

OSIF_MODULE_INIT(esdcan_init_legacy);
OSIF_MODULE_EXIT(esdcan_exit_legacy);
#endif /* !defined(ESDCAN_HOTPLUG_DRIVER) && !defined(BOARD_SPI) */

MODULE_AUTHOR("esd gmbh, support@esd.eu");
MODULE_DESCRIPTION("esd CAN driver");
MODULE_LICENSE("Proprietary");

#ifdef OSIF_OS_RTAI
EXPORT_SYMBOL(esdcan_read);
EXPORT_SYMBOL(esdcan_write);
EXPORT_SYMBOL(esdcan_ctrl);
#else /* #ifdef OSIF_OS_RTAI */
# if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
EXPORT_NO_SYMBOLS;
# endif /* #if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0) */
#endif /* #ifdef OSIF_OS_RTAI */


/* Id for RCS version system */
#if ((__GNUC__ > 3) || (__GNUC__ == 3 && __GNUC_MINOR__ >=1)) /* GCC >= 3.1 */
static char* rcsid __attribute__((unused,used)) = "$Id: esdcan.c 14790 2015-07-13 18:56:47Z stefanm $";
#elif ((__GNUC__ > 2) || (__GNUC__ == 2 && __GNUC_MINOR__ >=7))
static char* rcsid __attribute__((unused)) = "$Id: esdcan.c 14790 2015-07-13 18:56:47Z stefanm $";
#else  /* No or old GNU compiler */
# define USE(var) static void use_##var(void *x) {if(x) use_##var((void *)var);}
static char* rcsid = "$Id: esdcan.c 14790 2015-07-13 18:56:47Z stefanm $";
USE(rcsid);
#endif /* of GNU compiler */

/*---------------------------------------------------------------------------*/
/*                                 EOF                                       */
/*---------------------------------------------------------------------------*/
