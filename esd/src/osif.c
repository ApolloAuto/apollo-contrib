/* -*- esdcan-c -*-
 * FILE NAME osif.c
 *
 * BRIEF MODULE DESCRIPTION
 *           OSIF implementation for Linux
 *
 *
 * history:
 *
 *  $Log$
 *  Revision 1.133  2015/04/30 15:21:36  hauke
 *  Added OSIF_DIV64_SFT_FIXUP in osif_attach().
 *
 *  Revision 1.132  2015/01/09 14:36:26  stefanm
 *  Use vprintk() on kernels > 2.6.18 to avoid formatting the message
 *  myself by OSIF_VSNPRINTF().
 *  Fixed missing format string with printk().
 *
 *  Revision 1.131  2014/10/31 13:12:18  manuel
 *  Improved including of linux/sched/rt.h - needed for newer kernels, where MAX_USER_RT_PRIO isn't found in linux/sched.h any more.
 *
 *  Revision 1.130  2014/08/12 06:01:04  matthias
 *  need linux/sched/rt.h for RT_MAX_USER_PRIO (kernel 3.12+)
 *
 *  Revision 1.129  2014/07/07 14:36:21  matthias
 *  check fo CONFIG_OF_DEVICE or (!) CONFIG_OF_FLATTREE
 *
 *  Revision 1.128  2014/07/04 10:02:54  hauke
 *  Removed <canio.h> which is included in <osif.h>
 *
 *  Revision 1.127  2014/06/16 12:23:12  manuel
 *  Fixed osif_dprint
 *
 *  Revision 1.126  2014/05/20 12:40:06  andreas
 *  Fixed a bunch of violations of esd's coding style
 *
 *  Revision 1.125  2014/02/27 16:51:46  stefanm
 *  osif_pci_alloc_consisten() will now request only 32-bit DMA addresses
 *  from the kernel and check for usable addresses.
 *
 *  Revision 1.124  2014/01/16 13:40:37  frank
 *  removed "#include spi.h". No longer needed ...
 *
 *  Revision 1.123  2013/12/30 16:51:53  frank
 *  Made adaptions needed for directly accessing the SPI controller via esdcan-of-spi-connector and esd-omap2-mcspi
 *
 *  Revision 1.122  2013/12/09 17:04:52  manuel
 *  From now on all (not any) debug bits of a dprint must be set in verbose flags
 *
 *  Revision 1.121  2013/11/20 14:14:37  frank
 *  Fixed quandary with spi_sync() [...]
 *
 *  Revision 1.120  2013/11/19 15:32:35  frank
 *  Added SPI support (BOARD_SPI)
 *
 *  Revision 1.119  2013/09/16 12:56:12  matthias
 *  use <read|write><b|w|l> for ARM
 *
 *  with kernel 3.8.13 on arm IO_SPACE_LIMIT is 0. so all
 *  addresses passed to in/outb/w/l are truncated to 0.
 *
 *  Perhaps this will work on other architectures, too.
 *
 *  Revision 1.118  2013/08/19 14:20:32  manuel
 *  Added workaround to use CONFIG_DEBUG_MUTEXES
 *  Fix for kernel 3.4 and newer: do not call daemonize
 *
 *  Revision 1.117  2013/08/08 13:09:27  matthias
 *  use software timestamps on ARM
 *
 *  Revision 1.116  2013/07/16 14:16:36  andreas
 *  TODO reminder comment added
 *
 *  Revision 1.115  2013/04/26 14:27:26  andreas
 *  Added osif_div64_sft
 *
 *  Revision 1.114  2013/01/18 16:56:46  andreas
 *  Changed OSIF_TIMER implementation
 *  - added CAN_NODE parameter to OSIF_TIMER_CREATE
 *  - OSIF_TIMER structure is defined in osif.h and in osifi.h there's only a static extension OSIF_TIMER_SYS_PART
 *
 *  Revision 1.113  2012/02/17 18:40:11  andreas
 *  Fixed OSIF_CALLTYPE position
 *
 *  Revision 1.112  2011/11/04 14:44:05  andreas
 *  Small comment changes (AB -> BL)
 *
 *  Revision 1.111  2011/11/01 15:27:41  andreas
 *  Merged with preempt_rt branch
 *  With OSIF_USE_PREEMPT_RT_IMPLEMENTATION the new implementation is used for
 *    all kernels > 2.6.20
 *  Some cleanup
 *
 *  Revision 1.110  2011/09/06 17:20:36  manuel
 *  Fixed rcsid (for newer gcc)
 *
 *  Revision 1.109  2011/04/05 11:34:32  andreas
 *  Small change to use of OSIF_CALLTYPE on function pointers
 *  Changed to make use of new CHAR8 datatype
 *  Deleted loads of empty lines
 *
 *  Revision 1.108  2010/12/15 15:45:35  manuel
 *  Use spin_lock_init to initialize spinlocks (to allow lockdep checker to work)
 *  Fix SCHED_FIFO code (though it remains disabled by default)
 *
 *  Revision 1.107  2010/06/16 13:20:47  manuel
 *  Added linux/sched.h include unconditionally.
 *  Needed for kernel 2.6.33 but shouldn't harm for older kernels.
 *
 *  Revision 1.106  2010/06/01 09:56:53  tkoerper
 *  - including linux/sched.h with kernel >= 2.6.34
 *
 *  Revision 1.105  2010/04/16 17:03:58  andreas
 *  Added osif_callbacks
 *  Fixed or at least unified OSIF_CALLTYPE declarations on dpc and timer create
 *
 *  Revision 1.104  2010/03/09 13:32:23  matthias
 *  include asm/time.h when using tb_ticks_per_sec for osif_tick_frequency
 *
 *  Revision 1.103  2010/03/09 10:35:19  matthias
 *  Cleanup osif_tick_frequency for powerpc
 *  - use tb_ticks_per_sec when possible
 *
 *  Revision 1.102  2010/03/09 10:14:23  matthias
 *  Beginning with 2.6.33 linux/autoconf.h moved to generated/autoconf.h. Also there is no need to include it. The kernel's build system passes it via -include option to gcc.
 *
 *  Revision 1.101  2009/07/31 14:14:05  andreas
 *  Untabbified
 *
 *  Revision 1.100  2009/07/03 10:42:49  tkoerper
 *  - Added #ifdefs for arm
 *
 *  Revision 1.99  2009/04/01 14:33:03  andreas
 *  Quickfix for CBX-CPU5200, need to rework function for timer tick frequency
 *
 *  Revision 1.98  2009/02/25 16:16:54  andreas
 *  Removed osif_sema_-calls
 *  Removed osif_busy_mutex_-calls
 *
 *  Revision 1.97  2009/02/11 16:38:46  manuel
 *  Changed type of irqsave flags from UINT32 to UINTPTR
 *
 *  Revision 1.96  2009/01/29 10:11:05  andreas
 *  Removed osif_in_irq_lock()/-unlock() functions
 *
 *  Revision 1.95  2008/12/10 10:54:19  manuel
 *  Now osif_io_xxx are only implemented here if OSIF_USE_IO_FUNCTIONS is defined (otherwise they are in the header).
 *
 *  Revision 1.94  2008/11/18 11:38:21  matthias
 *  include linux/semaphore.h instead of asm/semaphore.h beginning with kernel 2.6.27
 *
 *  Revision 1.93  2008/08/27 14:30:34  andreas
 *  Added BOARD_pmc440
 *
 *  Revision 1.92  2008/06/03 16:32:25  manuel
 *  Added debug output to osif_pci_malloc
 *  Fixed osif_in_irq_mutex_lock and _unlock (hopefully fixing all hangings)
 *  Changed osif_dpc_thread to use down_interruptible instead of down
 *
 *  Revision 1.91  2008/03/20 11:34:32  andreas
 *  Moved rt-prio for backend into 2.4 branch.
 *
 *  Revision 1.90  2008/03/20 10:15:44  andreas
 *  Changed optimization osif_timer_set(), the old optimization
 *  didn't work for different kernel versions (2.4.19-x86 (to name one))
 *
 *  Revision 1.89  2008/02/15 16:14:21  michael
 *  OSIF dpc's now running at rt prio 98.
 *
 *  Revision 1.88  2008/02/12 12:51:55  andreas
 *  Including linux/fs.h for kernels > 2.6.22
 *
 *  Revision 1.87  2008/01/21 14:11:41  michael
 *  Osif threads now scheduled in fifo mode at prio max.
 *
 *  Revision 1.86  2007/12/11 08:34:46  andreas
 *  Removed osif_load_from_file (doesn't compile under Linux 2.6.23 anymore)
 *
 *  Revision 1.85  2007/06/05 08:57:42  andreas
 *  Add osif_usleep()
 *  Some cleanup
 *
 *  Revision 1.84  2007/05/24 12:34:54  michael
 *  osif_div64_32 renamed to _osif_div64_32_
 *
 *  Revision 1.83  2007/05/07 13:53:24  andreas
 *  Added #ifdefs to disable "config space access functions",
 *    if no PCI driver is compiled
 *
 *  Revision 1.82  2007/01/24 07:39:53  andreas
 *  Changed #ifdef's around osif_tick_frequency() in order to release PPC host
 *  driver for PMC331
 *
 *  Revision 1.81  2006/12/22 09:57:36  andreas
 *  Fixed for kernels 2.6.19
 *  Exchanged include order of version.h and config.h/autoconf.h
 *  Including autoconf.h instead of config.h for kernels >= 2.6.19
 *
 *  Revision 1.80  2006/10/12 13:31:55  manuel
 *  Fixed compiler warning
 *
 *  Revision 1.79  2006/10/11 10:12:17  andreas
 *  Fixed compilation problem with SuSE > 10.0
 *
 *  Revision 1.78  2006/08/17 13:26:59  michael
 *  Rebirth of OSIF_ errorcodes
 *
 *  Revision 1.77  2006/08/03 07:46:19  andreas
 *  Removed old debug code, which was #if 0 for quite a while, now
 *
 *  Revision 1.76  2006/06/29 15:18:40  andreas
 *  CANIO_EINTR got replaced by OSIF_OPERATION_ABORTED
 *
 *  Revision 1.75  2006/06/29 14:45:06  andreas
 *  Exchanged error codes from osif.h (OSIF_xxx) with error codes from canio.h (CANIO_xxx)
 *
 *  Revision 1.74  2006/06/27 13:13:44  andreas
 *  Exchanged OSIF_errors with CANIO_errors
 *
 *  Revision 1.73  2006/06/27 09:52:03  andreas
 *  TODO comment added (regarding timestamps on baudrate change events)
 *
 *  Revision 1.72  2006/04/28 07:54:27  andreas
 *  osif_ticks() must not be used with mecp52 (or generally mpc5200 based boards)
 *
 *  Revision 1.71  2006/02/14 09:18:37  matthias
 *  -board info structure has change to 2.6 kernels
 *  -check if _MSC_VER is defined be fore using it
 *
 *  Revision 1.70  2005/12/06 13:24:10  andreas
 *  No functional changes, only cleanup, while looking through the
 *   code in search for an error
 *
 *  Revision 1.69  2005/10/06 07:43:23  matthias
 *  added osif_make_ntcan_serial
 *
 *  Revision 1.68  2005/08/30 12:25:09  andreas
 *  Fixed osif_memcmp() !!!
 *  This one was really nice (great copy'n'paste work), comparison was done,
 *  result was piped to NIL.
 *  Instead the result was always ok :(
 *
 *  Revision 1.67  2005/07/27 15:50:14  andreas
 *  Corrected calltype of func-parameter of osif_timer_create
 *
 *  Revision 1.66  2005/04/20 13:04:16  andreas
 *  Changes for 64-Bit Linux (exchanged C-standard types with OSIF-types, use of UINTPTR)
 *  Added osif_ser_out()
 *
 *  Revision 1.65  2004/11/15 17:35:54  matthias
 *  fixed frequency of PPC750 performance counter frequency
 *
 *  Revision 1.64  2004/11/15 17:25:14  matthias
 *  added temporary osif_tick_frequency() for cpci750
 *
 *  Revision 1.63  2004/11/08 11:13:46  michael
 *  Missing OSIF_CALLTYPE added
 *
 *  Revision 1.62  2004/10/29 14:56:05  matthias
 *  -added osif_div64_64
 *  -fixed osif_timer_set rounding
 *
 *  Revision 1.61  2004/10/11 09:23:44  andreas
 *  Added io-functions (for kernel 2.6)
 *  Added some debug zones
 *  Sorted history
 *
 *  13.05.04 - corrected SMP-bug in OSIF_SLEEP for kernel < 2.6
 *             new OSIF_SLEEP branch for kernel > 2.6.5
 *             timer functions needed OSIF_CALLTYPE, too
 *             changed dpc_thread kernel2.6-style (daemonize()...)  ab
 *  11.05.04 - Changed OSIF_EXTERN into OSIF_CALLTYPE
 *             Added rcsid                                          ab
 *  07.05.04 - Removed gcc3-warning (LVALUE-casts)
 *             Including linux/version.h
 *             Changed prototype of osif_irq_attach a bit to cope
 *               with the needs of kernel 2.6                       ab
 *  06.05.04 - Include of modversions changed for kernel 2.6.x
 *             Added OSIF_EXTERN to nearly all functions (problem
 *             with release-archive for kernel 2.6.x)               ab
 *  01.12.03 - OSIF doesn't use OSIF-macros for mutex/sema anymore  mt
 *  26.11.03 - Corrected OSIF_VSNPRINTF for kernel < 2.4.0
 *             Removed osif_pci_malloc-warning for non-pci405-cards
 *             Exchanged init_MUTEX with sema_init()                ab
 *  13.11.03 - Corrected debug-info in mutexes                      ab
 *  13.11.03 - OSIF_MUTEX do not busy wait, Old OSIF_MUTEX now
 *             OSIF_BUSY_MUTEX                                      mt
 *  12.11.03 - OSIF_TIMER as DPC                                    mt
 *  05.11.03 - Saving of IRQ-flags was wrong! Removed again!        ab
 *  29.10.03 - changed IRQ-spinlocks (IRQ-flags get saved, now)     ab
 *  27.10.03 - New output function osif_dprint (with verbose mask)  ab
 *  18.09.03 - Only one thread for all DPC's                        mt
 *  02.01.03 - added SA_INTERRUPT flag for atomic interrupt handler mf
 *  24.05.02 - first version                                        mf
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
 *  Germany                                support@esd.eu
 *
 *************************************************************************/
/*! \file osif.c
    \brief OSIF implementation

    This file contains the implementation of all osif functions.
*/

/*---------------------------------------------------------------------------*/
/*                               INCLUDES                                    */
/*---------------------------------------------------------------------------*/
#include <linux/version.h>
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,32))
# if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,19))
#  include <linux/autoconf.h>
# else
#  include <linux/config.h>
# endif
#endif
#ifdef DISTR_SUSE
# undef CONFIG_MODVERSIONS
#endif
#ifdef CONFIG_MODVERSIONS
# if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
#  include <config/modversions.h>
# else
#  include <linux/modversions.h>
# endif
# ifndef MODVERSIONS
#  define MODVERSIONS
# endif
#endif
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/init.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/interrupt.h>
#include <linux/time.h>
#include <linux/pci.h>
#include <linux/types.h>
#include <linux/timer.h>
#include <linux/delay.h>
#ifdef LTT
# include <linux/trace.h>
#endif
#include <linux/errno.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
# include <linux/interrupt.h>
#endif

#if ( (LINUX_VERSION_CODE < KERNEL_VERSION(2,4,8)) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0)) )
# if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,22)
#  include <linux/fs.h>
# endif
  /* needed for vsnprintf */
# include <asm/div64.h>
# include <linux/ctype.h>
#endif

#ifdef BOARD_mcp2515
/* Unfortunately we don't have defined BOARD_SPI in osif.c,      */
/* so we temporarily introduce an OSIF_BOARD_SPI define here ... */
# define OSIF_BOARD_SPI 1
#endif

#include <linux/sched.h>
#ifndef MAX_USER_RT_PRIO
#include <linux/sched/rt.h> /* maybe it's hidden here now (newer kernels...) */
#endif

#include <osif.h>

#ifdef OSIF_USE_PREEMPT_RT_IMPLEMENTATION
# include <linux/kthread.h>
#endif

/*---------------------------------------------------------------------------*/
/*                                DEFINES                                    */
/*---------------------------------------------------------------------------*/

#ifdef ESDDBG
# define OSIF_DEBUG                       /*!< enables debug output for osif functions */
#endif

#ifdef OSIF_DEBUG
# define OSIF_DBG(fmt)          OSIF_DPRINT(fmt) /*!< Macro for debug prints */
#else
# define OSIF_DBG(fmt)
#endif

#ifdef KBUILD_MODNAME
#       define  K_DPC_NAME      "k" KBUILD_MODNAME
#else
#       define  K_DPC_NAME      "dpc thread"
#endif

/*---------------------------------------------------------------------------*/
/*                                TYPEDEFS                                   */
/*---------------------------------------------------------------------------*/

#ifdef OSIF_USE_PREEMPT_RT_IMPLEMENTATION
struct __OSIF_MUTEX {
        struct mutex lock;
};

#ifdef CONFIG_DEBUG_MUTEXES
#include <linux/atomic.h>
#define OSIF_MAX_DEBUG_MUTEXES 128
static atomic_t osif_debug_mutexes_cnt;
static struct lock_class_key osif_debug_mutexes_keys[OSIF_MAX_DEBUG_MUTEXES];
#endif

#else
struct __OSIF_MUTEX {
        struct semaphore sema;
};
#endif  /* #ifdef OSIF_USE_PREEMPT_RT_IMPLEMENTATION */

struct __OSIF_SPINLOCK {
        spinlock_t spinlock;
        UINTPTR    flags;
        UINT32     magic;
};

struct _OSIF_LINUX_TIMER {
        struct timer_list   timer;
        OSIF_DPC            dpc;
};

struct __OSIF_PCI_PADDR {
        UINT32              addr32;
        dma_addr_t          sys_dma_addr;
};

static struct {
        OSIF_DPC                 first;
        spinlock_t               spinlock;
        INT32                    thread_running;
        INT32                    destroy;
#ifdef OSIF_USE_PREEMPT_RT_IMPLEMENTATION
        wait_queue_head_t        wqDpcNotify;
        atomic_t                 trig;
        struct task_struct      *tsk;
#else
        struct semaphore         sema_notify;
        struct semaphore         sema_trigger;
#endif
} dpc_root;

OSIF_CALLBACKS  osif_callbacks = {
        osif_timer_create,
        osif_timer_destroy,
        osif_timer_set,
        osif_timer_get,
};

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0)
DECLARE_COMPLETION(dpcThreadCompletion);
#endif

/*---------------------------------------------------------------------------*/
/*                 DEFINITION OF LOCAL DATA                                  */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/*                 DECLARATION OF LOCAL FUNCTIONS                            */
/*---------------------------------------------------------------------------*/
static INT32 osif_dpc_root_create(VOID);
static INT32 osif_dpc_root_destroy(VOID);


/*---------------------------------------------------------------------------*/
/*                 DEFINITION OF GLOBAL DATA                                 */
/*---------------------------------------------------------------------------*/

extern UINT32 verbose;  /* has to be declared in OS-level-sources (esdcan.c), should be set by module-parameter */

INT32  osif_div64_sft = 0; /* see extern declaration osif.h */


/*---------------------------------------------------------------------------*/
/*                 DEFINITION OF EXPORTED FUNCTIONS                          */
/*---------------------------------------------------------------------------*/

#if defined(OSIF_BOARD_SPI)
int osif_spi_xfer(OSIF_SPIDEV *pDev, OSIF_SPIMSG *pMsg)
{
  int ret;

#if 0
        {
                int i;

                printk("%s:l=%d  in=", __FUNCTION__, pMsg->nBytes);
                for(i =0; i < pMsg->nBytes; i++) {
                        printk("%02x ", pMsg->io[i]);
                }
                printk("\n");
        }
#endif
        ret = pDev->scdev->pdsd->sync_xfer(pDev->scdev, pMsg);
        if (ret) {
                osif_print("%s: spi transfer failed: ret=%d\n", __FUNCTION__, ret);
                return ret;
        }
#if 0
        {
                int i;

                printk("%s:l=%d out=", __FUNCTION__, pMsg->nBytes);
                for(i =0; i < pMsg->nBytes; i++) {
                        printk("%02x ", pMsg->io[i]);
                }
                printk("\n");
        }
#endif
        return(OSIF_SUCCESS);
}
#endif /* OSIF_BOARD_SPI */

INT32 OSIF_CALLTYPE osif_attach()
{
        INT32 result;

        OSIF_DBG((OSIF_ZONE_OSIF, "%s: enter\n", OSIF_FUNCTION));
        result = osif_dpc_root_create();
        OSIF_DIV64_SFT_FIXUP();
        OSIF_DBG((OSIF_ZONE_OSIF, "%s: leave\n", OSIF_FUNCTION));
        return result;
}

INT32 OSIF_CALLTYPE osif_detach()
{
        INT32 result;

        OSIF_DBG((OSIF_ZONE_OSIF, "%s: enter\n", OSIF_FUNCTION));
        result = osif_dpc_root_destroy();
        OSIF_DBG((OSIF_ZONE_OSIF, "%s: leave\n", OSIF_FUNCTION));
        return result;
}

/*! Allocate \a size bytes of memory. A pointer to the new allocated memory is
  returned through \a **p.
*/
#define OSIF_MEM_IS_NOTUSED 0x00000000
#define OSIF_MEM_IS_KMEM    0x11111111
#define OSIF_MEM_IS_VMEM    0x22222222
INT32 OSIF_CALLTYPE osif_malloc(UINT32 size, VOID **p)
{
        VOID    *ptr;

        OSIF_DBG((OSIF_ZONE_OSIF, "%s: enter, size=%08x\n", OSIF_FUNCTION,size));
        ptr = kmalloc(size + sizeof(UINT32), GFP_KERNEL);
        if (!ptr) {
                OSIF_DBG((OSIF_ZONE_OSIF, "%s: kmalloc for %d bytes failed, using vmalloc\n", OSIF_FUNCTION, size));
                ptr = vmalloc(size + sizeof(UINT32));
                if (!ptr) {
                        *p = NULL;
                        OSIF_DBG((OSIF_ZONE_OSIF, "%s: OSIF_MALLOC failed !!!!\n", OSIF_FUNCTION));
                        return OSIF_INSUFFICIENT_RESOURCES;
                } else {
                        *(UINT32*)ptr = OSIF_MEM_IS_VMEM;
                }
        } else {
                *(UINT32*)ptr = OSIF_MEM_IS_KMEM;
        }
        *p = (VOID*)((UINT8*)ptr + sizeof(UINT32));
        OSIF_DBG((OSIF_ZONE_OSIF, "%s: leave\n", OSIF_FUNCTION));
        return OSIF_SUCCESS;
}

/*! Free memory that has been allocated by osif_malloc. \a ptr points to the
  pointer returned by osif_malloc().
*/
INT32 OSIF_CALLTYPE osif_free(VOID *ptr)
{
        UINT32 *pFree = (UINT32*)((UINT8*)ptr - sizeof(UINT32));

        if (ptr) {
                if (*pFree == OSIF_MEM_IS_KMEM) {
                        OSIF_DBG((OSIF_ZONE_OSIF, "%s: using kfree\n", OSIF_FUNCTION));
                        *pFree = OSIF_MEM_IS_NOTUSED;
                        kfree(pFree);
                } else if (*pFree == OSIF_MEM_IS_VMEM) {
                        OSIF_DBG((OSIF_ZONE_OSIF, "%s: using vfree\n", OSIF_FUNCTION));
                        *pFree = OSIF_MEM_IS_NOTUSED;
                        vfree(pFree);
                } else {
                        OSIF_DBG((OSIF_ZONE_OSIF, "%s: Either unknown mem type or forbidden free!!!\n", OSIF_FUNCTION));
                }
        }
        return OSIF_SUCCESS;
}

/*! Initialize a memory region pointed by \a ptr to a constant value \a val.
  \a size is the size of the region.
*/
INT32 OSIF_CALLTYPE osif_memset(VOID *ptr, UINT32 val, UINT32 size)
{
        if (ptr) {
                memset(ptr, (char)val, size);
        }
        return OSIF_SUCCESS;
}

INT32 OSIF_CALLTYPE osif_memcpy(VOID *dst, VOID *src, UINT32 size)
{
        memcpy(dst, src, size);
        return OSIF_SUCCESS;
}

INT32 OSIF_CALLTYPE osif_memcmp(VOID *dst, VOID *src, UINT32 size)
{
        return memcmp(dst, src, size);
}

INT32 OSIF_CALLTYPE osif_pci_read_config_byte(OSIF_PCIDEV pcidev, INT32 offs, UINT8 *ptr)
{
#ifdef CONFIG_PCI
        pci_read_config_byte(pcidev, offs, ptr);
#endif
        return OSIF_SUCCESS;
}

INT32 OSIF_CALLTYPE osif_pci_read_config_word(OSIF_PCIDEV pcidev, INT32 offs, UINT16 *ptr)
{
#ifdef CONFIG_PCI
        pci_read_config_word(pcidev, offs, ptr);
#endif
        return OSIF_SUCCESS;
}

INT32 OSIF_CALLTYPE osif_pci_read_config_long(OSIF_PCIDEV pcidev, INT32 offs, UINT32 *ptr)
{
#ifdef CONFIG_PCI
        pci_read_config_dword(pcidev, offs, (u32 *)ptr);
#endif
        return OSIF_SUCCESS;
}

INT32 OSIF_CALLTYPE osif_pci_write_config_byte(OSIF_PCIDEV pcidev, INT32 offs, UINT8 val)
{
#ifdef CONFIG_PCI
        pci_write_config_byte(pcidev, offs, val);
#endif
        return OSIF_SUCCESS;
}

INT32 OSIF_CALLTYPE osif_pci_write_config_word(OSIF_PCIDEV pcidev, INT32 offs, UINT16 val)
{
#ifdef CONFIG_PCI
        pci_write_config_word(pcidev, offs, val);
#endif
        return OSIF_SUCCESS;
}

INT32 OSIF_CALLTYPE osif_pci_write_config_long(OSIF_PCIDEV pcidev, INT32 offs, UINT32 val)
{
#ifdef CONFIG_PCI
        pci_write_config_dword(pcidev, offs, val);
#endif
        return OSIF_SUCCESS;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,0)
static inline void *pci_alloc_consistent(struct pci_dev *hwdev, size_t size,
                                         dma_addr_t *dma_handle)
{
        void *virt_ptr;

        virt_ptr = kmalloc(size, GFP_KERNEL);
        *dma_handle = virt_to_bus(virt_ptr);
        return virt_ptr;
}
#define pci_free_consistent(cookie, size, ptr, dma_ptr) kfree(ptr)
#endif

INT32 OSIF_CALLTYPE osif_pci_malloc(OSIF_PCIDEV pcidev, OSIF_PCI_VADDR *vaddr,
                                    OSIF_PCI_PADDR *paddr, OSIF_PCI_MADDR *maddr, UINT32 size)
{
        OSIF_DBG((OSIF_ZONE_OSIF, "%s: enter\n", OSIF_FUNCTION));
        *paddr = vmalloc( sizeof( **paddr ) );
        if ( NULL == *paddr) {
                return OSIF_INSUFFICIENT_RESOURCES;
        }
#if  (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,33))   /* LXR is currently out of order, in 2.6.33 this function was available for sure, probably way earlier */
       if (pci_set_consistent_dma_mask(pcidev, DMA_BIT_MASK(32))) {
               OSIF_PRINT(("esd CAN driver: No 32-Bit bus master buffer available.\n"));
               return OSIF_INSUFFICIENT_RESOURCES;
        }
#endif
        *vaddr = pci_alloc_consistent( pcidev, size, &((*paddr)->sys_dma_addr) );
        if ( NULL == *vaddr ) {
                vfree( *paddr );
                return OSIF_INSUFFICIENT_RESOURCES;
        }
        (*paddr)->addr32 = (UINT32)((*paddr)->sys_dma_addr);
        if ((dma_addr_t)((*paddr)->addr32) != (*paddr)->sys_dma_addr) {
                OSIF_PRINT(("esd CAN driver: Bus master addr conflict: dma 0x%llx(%u), addr32 0x%lx(%u)\n",
                            (unsigned long long)((*paddr)->sys_dma_addr), sizeof (*paddr)->sys_dma_addr,
                            (*paddr)->addr32, sizeof (*paddr)->addr32));
        }
        OSIF_DBG((OSIF_ZONE_OSIF, "%s: leave\n", OSIF_FUNCTION));
        return OSIF_SUCCESS;
}

INT32 OSIF_CALLTYPE osif_pci_free(OSIF_PCIDEV pcidev, OSIF_PCI_VADDR vaddr,
                                  OSIF_PCI_PADDR *paddr, OSIF_PCI_MADDR maddr, UINT32 size)
{
        OSIF_DBG((OSIF_ZONE_OSIF, "%s: enter\n", OSIF_FUNCTION));
        pci_free_consistent(pcidev, size, vaddr, (*paddr)->sys_dma_addr );
        vfree( *paddr );
        OSIF_DBG((OSIF_ZONE_OSIF, "%s: leave\n", OSIF_FUNCTION));
        return OSIF_SUCCESS;
}

UINT32 OSIF_CALLTYPE osif_pci_get_phy_addr(OSIF_PCI_PADDR paddr)
{
        return paddr->addr32;
}

#ifdef OSIF_PROVIDES_VSNPRINTF
#warning "vsnprintf not implemented in kernels pre 2.4.8"
#define ZEROPAD 1               /* pad with zero */
#define SIGN    2               /* unsigned/signed long */
#define PLUS    4               /* show plus */
#define SPACE   8               /* space if plus */
#define LEFT    16              /* left justified */
#define SPECIAL 32              /* 0x */
#define LARGE   64              /* use 'ABCDEF' instead of 'abcdef' */
/*!
 *  \brief Needed for vsnprintf()
 */
static INT32 skip_atoi(const CHAR8 **s)
{
        INT32 i = 0;

        while (isdigit(**s))
                i = i*10 + *((*s)++) - '0';
        return i;
}

/*!
 *  \brief Needed for vsnprintf()
 */
static CHAR8* number(CHAR8* buf, CHAR8* end, INT64 num, INT32 base, INT32 size, INT32 precision, INT32 type)
{
        CHAR8        c, sign, tmp[66];
        const CHAR8 *digits;
        const CHAR8  small_digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";
        const CHAR8  large_digits[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        INT32        i;

        digits = (type & LARGE) ? large_digits : small_digits;
        if (type & LEFT) {
                type &= ~ZEROPAD;
        }
        if (base < 2 || base > 36) {
                return 0;
        }
        c = (type & ZEROPAD) ? '0' : ' ';
        sign = 0;
        if (type & SIGN) {
                if (num < 0) {
                        sign = '-';
                        num = -num;
                        size--;
                } else if (type & PLUS) {
                        sign = '+';
                        size--;
                } else if (type & SPACE) {
                        sign = ' ';
                        size--;
                }
        }
        if (type & SPECIAL) {
                if (base == 16)
                        size -= 2;
                else if (base == 8)
                        size--;
        }
        i = 0;
        if (num == 0) {
                tmp[i++]='0';
        } else {
                while (num != 0) {
                        tmp[i++] = digits[do_div(num,base)];
                }
        }
        if (i > precision) {
                precision = i;
        }
        size -= precision;
        if (!(type&(ZEROPAD+LEFT))) {
                while(size-->0) {
                        if (buf <= end) {
                                *buf = ' ';
                        }
                        ++buf;
                }
        }
        if (sign) {
                if (buf <= end) {
                        *buf = sign;
                }
                ++buf;
        }
        if (type & SPECIAL) {
                if (base==8) {
                        if (buf <= end) {
                                *buf = '0';
                        }
                        ++buf;
                } else if (base==16) {
                        if (buf <= end) {
                                *buf = '0';
                        }
                        ++buf;
                        if (buf <= end) {
                                *buf = digits[33];
                        }
                        ++buf;
                }
        }
        if (!(type & LEFT)) {
                while (size-- > 0) {
                        if (buf <= end) {
                                *buf = c;
                        }
                        ++buf;
                }
        }
        while (i < precision--) {
                if (buf <= end) {
                        *buf = '0';
                }
                ++buf;
        }
        while (i-- > 0) {
                if (buf <= end) {
                        *buf = tmp[i];
                }
                ++buf;
        }
        while (size-- > 0) {
                if (buf <= end) {
                        *buf = ' ';
                }
                ++buf;
        }
        return buf;
}

/*!
 * \brief Format a string and place it in a buffer
 *
 * \param buf The buffer to place the result into
 * \param size The size of the buffer, including the trailing null space
 * \param fmt The format string to use
 * \param args Arguments for the format string
 *
 *  This function was copied directly out of the 2.4.18.SuSE-kernel.
 *  Since kernels pre 2.4.6 don't support this function, we need to
 *  provide it on our own.
 */
INT32 OSIF_CALLTYPE osif_vsnprintf(CHAR8 *buf, size_t size, const CHAR8 *fmt, va_list args)
{
        INT32        len;
        UINT64       num;
        INT32        i, base;
        CHAR8       *str, *end, c;
        const CHAR8 *s;
        INT32 flags;            /* flags to number() */
        INT32 field_width;      /* width of output field */
        INT32 precision;        /* min. # of digits for integers; max
                                   number of chars for from string */
        INT32 qualifier;        /* 'h', 'l', or 'L' for integer fields */
                                /* 'z' support added 23/7/1999 S.H.    */
                                /* 'z' changed to 'Z' --davidm 1/25/99 */

        str = buf;
        end = buf + size - 1;
        if (end < buf - 1) {
                end = ((void *) (-1));
                size = end - buf + 1;
        }
        for (; *fmt ; ++fmt) {
                if (*fmt != '%') {
                        if (str <= end) {
                                *str = *fmt;
                        }
                        ++str;
                        continue;
                }
                /* process flags */
                flags = 0;
                repeat:
                        ++fmt;          /* this also skips first '%' */
                        switch (*fmt) {
                        case '-': flags |= LEFT; goto repeat;
                        case '+': flags |= PLUS; goto repeat;
                        case ' ': flags |= SPACE; goto repeat;
                        case '#': flags |= SPECIAL; goto repeat;
                        case '0': flags |= ZEROPAD; goto repeat;
                        }
                /* get field width */
                field_width = -1;
                if (isdigit(*fmt)) {
                        field_width = skip_atoi(&fmt);
                } else if (*fmt == '*') {
                        ++fmt;
                        /* it's the next argument */
                        field_width = va_arg(args, INT32);
                        if (field_width < 0) {
                                field_width = -field_width;
                                flags |= LEFT;
                        }
                }
                /* get the precision */
                precision = -1;
                if (*fmt == '.') {
                        ++fmt;
                        if (isdigit(*fmt)) {
                                precision = skip_atoi(&fmt);
                        } else if (*fmt == '*') {
                                ++fmt;
                                /* it's the next argument */
                                precision = va_arg(args, INT32);
                        }
                        if (precision < 0) {
                                precision = 0;
                        }
                }
                /* get the conversion qualifier */
                qualifier = -1;
                if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L' || *fmt =='Z') {
                        qualifier = *fmt;
                        ++fmt;
                        if (qualifier == 'l' && *fmt == 'l') {
                                qualifier = 'L';
                                ++fmt;
                        }
                }
                /* default base */
                base = 10;
                switch (*fmt) {
                        case 'c':
                                if (!(flags & LEFT)) {
                                        while (--field_width > 0) {
                                                if (str <= end) {
                                                        *str = ' ';
                                                }
                                                ++str;
                                        }
                                }
                                c = (CHAR8)va_arg(args, INT32);
                                if (str <= end) {
                                        *str = c;
                                }
                                ++str;
                                while (--field_width > 0) {
                                        if (str <= end) {
                                                *str = ' ';
                                        }
                                        ++str;
                                }
                                continue;

                        case 's':
                                s = va_arg(args, CHAR*);
                                if (!s)
                                        s = "<NULL>";
                                len = strnlen(s, precision);
                                if (!(flags & LEFT)) {
                                        while (len < field_width--) {
                                                if (str <= end) {
                                                        *str = ' ';
                                                }
                                                ++str;
                                        }
                                }
                                for (i = 0; i < len; ++i) {
                                        if (str <= end) {
                                                *str = *s;
                                        }
                                        ++str; ++s;
                                }
                                while (len < field_width--) {
                                        if (str <= end) {
                                                *str = ' ';
                                        }
                                        ++str;
                                }
                                continue;

                        case 'p':
                                if (field_width == -1) {
                                        field_width = 2*sizeof(void*);
                                        flags |= ZEROPAD;
                                }
                                str = number(str, end, (INT32)va_arg(args, void*),
                                             16, field_width, precision, flags);
                                continue;

                        case 'n':
                                /* FIXME:
                                * What does C99 say about the overflow case here? */
                                if (qualifier == 'l') {
                                        INT32 *ip = va_arg(args, INT32*);
                                        *ip = (str - buf);
                                } else if (qualifier == 'Z') {
                                        size_t *ip = va_arg(args, size_t*);
                                        *ip = (str - buf);
                                } else {
                                        INT32 *ip = va_arg(args, INT32*);
                                        *ip = (str - buf);
                                }
                                continue;

                        case '%':
                                if (str <= end) {
                                        *str = '%';
                                }
                                ++str;
                                continue;

                                /* integer number formats - set up the flags and "break" */
                        case 'o':
                                base = 8;
                                break;

                        case 'X':
                                flags |= LARGE;
                        case 'x':
                                base = 16;
                                break;

                        case 'd':
                        case 'i':
                                flags |= SIGN;
                        case 'u':
                                break;

                        default:
                                if (str <= end) {
                                        *str = '%';
                                }
                                ++str;
                                if (*fmt) {
                                        if (str <= end) {
                                                *str = *fmt;
                                        }
                                        ++str;
                                } else {
                                        --fmt;
                                }
                                continue;
                }
                if (qualifier == 'L')
                        num = va_arg(args, INT64);
                else if (qualifier == 'l') {
                        num = va_arg(args, UINT32);
                        if (flags & SIGN) {
                                num = (INT32) num;
                        }
                } else if (qualifier == 'Z') {
                        num = va_arg(args, size_t);
                } else if (qualifier == 'h') {
                        num = (UINT16)va_arg(args, INT32);
                        if (flags & SIGN) {
                                num = (INT16) num;
                        }
                } else {
                        num = va_arg(args, UINT32);
                        if (flags & SIGN) {
                                num = (INT32) num;
                        }
                }
                str = number(str, end, num, base, field_width, precision, flags);
        }
        if (str <= end) {
                *str = '\0';
        } else if (size > 0) {
                /* don't write out a null byte if the buf size is zero */
                *end = '\0';
        }
        /* the trailing null byte doesn't count towards the total
         * ++str;
         */
        return str-buf;
}
#endif  /* ifdef OSIF_PROVIDES_VSNPRINTF */

/*! This function prints a formated text string.
*/
INT32 OSIF_CALLTYPE osif_print(const CHAR8 *fmt, ...)
{
        va_list arglist;
        INT32   len;

#ifdef OSIF_USE_VPRINTK
        va_start(arglist, fmt);
        len = vprintk(fmt, arglist);
        va_end(arglist);
#else
        CHAR8   output[400];

        va_start(arglist, fmt);
        len = OSIF_VSNPRINTF(output, 400, fmt, arglist);
        va_end(arglist);
        printk("%s", output);
#endif
        return len;
}

/*! This function prints a formated text string, only, if one of the bits
 *  specified in output_mask was set at module-load-time (global variable verbose).
 */
INT32 OSIF_CALLTYPE osif_dprint(UINT32 output_mask, const CHAR8 *fmt, ...)
{
        va_list arglist;
        INT32   len = 0;
        UINT32  verbose_flags = verbose;

#ifndef ESDDBG
        verbose_flags &= 0x000000FF;
#endif
        if ( (verbose_flags & output_mask) == output_mask ) {
#ifdef OSIF_USE_VPRINTK
                va_start(arglist, fmt);
                len = vprintk(fmt, arglist);
                va_end(arglist);
#else
                CHAR8   output[400];

                va_start(arglist, fmt);
                len = OSIF_VSNPRINTF(output, 400, fmt, arglist);
                va_end(arglist);
                printk("%s", output);
#endif
        }
        return len;
}

INT32 OSIF_CALLTYPE osif_snprintf(CHAR8 *str, INT32 size, const CHAR8 *format, ...)
{
        va_list arglist;
        INT32   len;

        va_start(arglist, format);
        len = OSIF_VSNPRINTF(str, size, format, arglist);
        va_end(arglist);
        return len;
}

/*! Sleep for a given number (\a ms) of milli seconds.
*/
INT32 OSIF_CALLTYPE osif_sleep( INT32 ms )
{
# if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
        wait_queue_head_t wq;
        INT32  result = OSIF_SUCCESS;
        UINT32 timeout = ((ms*HZ)+999)/1000;

        init_waitqueue_head(&wq);
        result = wait_event_interruptible_timeout(wq, (timeout == 0), timeout);
        if (result == -ERESTARTSYS) {
                OSIF_DBG((OSIF_ZONE_OSIF, "%s: Sleep cancelled by SIGTERM!", OSIF_FUNCTION));
        }
#else
        OSIF_DECL_WAIT_QUEUE(p);
        interruptible_sleep_on_timeout( &p, ((ms*HZ)+999)/1000);
#endif
        return OSIF_SUCCESS;
}

/*! (Possibly busy-) sleep for a given number (\a us) of microseconds.
*/
INT32 OSIF_CALLTYPE osif_usleep( INT32 us )
{
        udelay(us);
        return OSIF_SUCCESS;
}


/*
** mutex entries
*/
INT32 OSIF_CALLTYPE osif_mutex_create(OSIF_MUTEX *m)
{
        *m = vmalloc(sizeof(**m));
        if(NULL == *m) {
                return OSIF_INSUFFICIENT_RESOURCES;
        }
        memset(*m, 0x00, sizeof(**m));
#ifdef OSIF_USE_PREEMPT_RT_IMPLEMENTATION
#ifdef CONFIG_DEBUG_MUTEXES
        {
                int nr = atomic_inc_return(&osif_debug_mutexes_cnt)-1;
                if (nr >= OSIF_MAX_DEBUG_MUTEXES) {
                        vfree(*m);
                        *m = NULL;
                        return OSIF_INSUFFICIENT_RESOURCES;
                }
                __mutex_init(&((*m)->lock), "osif_mutex", &osif_debug_mutexes_keys[nr]);
        }
#else
        mutex_init(&((*m)->lock));
#endif
#else
        sema_init(&((*m)->sema), 1);
#endif
        return OSIF_SUCCESS;
}

INT32 OSIF_CALLTYPE osif_mutex_destroy(OSIF_MUTEX *m)
{
        vfree(*m);
        *m = NULL;
        return OSIF_SUCCESS;
}

INT32 OSIF_CALLTYPE osif_mutex_lock(OSIF_MUTEX *m)
{
#ifdef OSIF_USE_PREEMPT_RT_IMPLEMENTATION
        mutex_lock(&((*m)->lock));
#else
        down(&((*m)->sema));
#endif
        return OSIF_SUCCESS;
}

INT32 OSIF_CALLTYPE osif_mutex_unlock(OSIF_MUTEX *m)
{
#ifdef OSIF_USE_PREEMPT_RT_IMPLEMENTATION
        mutex_unlock(&((*m)->lock));
#else
        up(&((*m)->sema));
#endif
        return OSIF_SUCCESS;
}

/*
 * mutexes for synchronisation against the irq-level
 */
INT32 OSIF_CALLTYPE osif_irq_mutex_create(OSIF_IRQ_MUTEX *im)
{
        *im = vmalloc(sizeof(**im));
        if(NULL == *im) {
                return OSIF_INSUFFICIENT_RESOURCES;
        }
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
        spin_lock_init(&(((*im)->spinlock)));
#else
        (*im)->spinlock = SPIN_LOCK_UNLOCKED;
#endif
#ifdef OSIF_DEBUG
        (*im)->magic = 0x0;
#endif
        return OSIF_SUCCESS;
}

INT32 OSIF_CALLTYPE osif_irq_mutex_destroy(OSIF_IRQ_MUTEX *im)
{
        vfree(*im);
        *im = NULL;
        return OSIF_SUCCESS;
}

/*
 *  Call this entry, if you are not inside in an irq handler
 */
INT32 OSIF_CALLTYPE osif_irq_mutex_lock(OSIF_IRQ_MUTEX *im)
{
#ifdef OSIF_DEBUG
        struct task_struct *tsk = current;
#endif
        UINTPTR flags;
        spin_lock_irqsave(&((*im)->spinlock), flags);
        (*im)->flags = flags;
#ifdef OSIF_DEBUG
        (*im)->magic = tsk->pid | 0x80000000; /* MSB=1 marks entry */
#endif
        return OSIF_SUCCESS;
}

/*
 *  Call this entry, if you are not inside an irq handler
 */
INT32 OSIF_CALLTYPE osif_irq_mutex_unlock(OSIF_IRQ_MUTEX *im)
{
        UINTPTR flags = (*im)->flags;
#ifdef OSIF_DEBUG
        struct task_struct *tsk = current;
        (*im)->magic = tsk->pid; /* MSB=0 marks exit (assuming pid's smaller than 0x7FFFFFFF) */
#endif
        spin_unlock_irqrestore(&((*im)->spinlock), flags);
        return OSIF_SUCCESS;
}


/* --------------------------------------------------------------------------------
 *      kernel thread that handles all our OSIF_DPC jobs
 */
static int osif_dpc_thread( void *context )
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
        struct task_struct *tsk = current;
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
        /* Code for Linux version range: 2.4.0 <= LXV < 2.6.0 */
        OSIF_DBG((OSIF_ZONE_OSIF, "%s: enter\n", OSIF_FUNCTION));

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,4,0)
        daemonize(K_DPC_NAME);  /* Call wasn't really needed but didn't harm before 3.4. */
#endif
#ifdef OSIF_BOARD_SPI
        {
                extern int esdcan_spi_connector_set_cur_tsk_sched(int policy, const unsigned int prio);
                esdcan_spi_connector_set_cur_tsk_sched(SCHED_FIFO, MAX_USER_RT_PRIO-1);
        }
#endif /* ifdef OSIF_BOARD_SPI */
# if 0

/* BL TODO: set affinity to CPU, which executes interrupts (to get accurate timestamps...)
long sched_setaffinity(pid_t pid, cpumask_t new_mask)
long sched_getaffinity(pid_t pid, cpumask_t *mask)
 */
        /* Rescheduling of this kernel thread could be done with the following call
         * if we were allowed to use that GPL only function. Therefore the user has
         * to use 'chrt' or 'renice' to set a scheduling policy and priority suitable
         * for his setup.
         */
        {
                struct sched_param param = { .sched_priority = MAX_USER_RT_PRIO/2, };
                sched_setscheduler(current, SCHED_FIFO, &param);
        }
# endif

#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0)
        /* Code for Linux version range: 2.4.0 <= LXV < 2.6.0 */
        OSIF_DBG((OSIF_ZONE_OSIF, "%s: enter tsk=%p\n", OSIF_FUNCTION, tsk));
        daemonize();
        /* todo: do we need this ? */ /*  tsk->pgrp = 1; */
        sigfillset(&tsk->blocked); /* block all signals */
        strcpy(tsk->comm, K_DPC_NAME);
        /* mf test-only */
        tsk->policy = 1; /* SCHED_FIFO */
        tsk->rt_priority = 98; /* max */
        tsk->need_resched = 1;

#else
        /* Code for Linux version range: LXV < 2.4.0 */
        OSIF_DBG((OSIF_ZONE_OSIF, "%s: enter tsk=%p\n", OSIF_FUNCTION, tsk));
        tsk->session = 1;
        tsk->pgrp = 1;
        strcpy(tsk->comm, K_DPC_NAME);
        spin_lock_irq(&tsk->sigmask_lock);
        sigfillset(&tsk->blocked);             /* block all signals */
        recalc_sigpending(tsk);
        spin_unlock_irq(&tsk->sigmask_lock);
        /* BL TODO: check if realtime priorities can be realized
                    with kernels below 2.4.0 */
#endif

        OSIF_DBG((OSIF_ZONE_OSIF, "%s: can dpc running\n", OSIF_FUNCTION));
        /* notify that backend thread is running */
        dpc_root.thread_running = 1;
#ifdef OSIF_USE_PREEMPT_RT_IMPLEMENTATION
        wake_up(&dpc_root.wqDpcNotify);
        __set_current_state(TASK_RUNNING);
#else
        up(&dpc_root.sema_notify);
#endif
        while (!dpc_root.destroy) {
#ifdef OSIF_USE_PREEMPT_RT_IMPLEMENTATION
                atomic_add_unless(&dpc_root.trig, -1, 0);  /* BL: In with 2.6.15 */
                smp_mb();
#else
                if (down_interruptible(&dpc_root.sema_trigger)<0) {
                        continue;
                }
                if (dpc_root.destroy) {
                        break;
                }
#endif
                /* call dpc function */
                OSIF_DBG((OSIF_ZONE_OSIF, "%s: dpc triggered\n", OSIF_FUNCTION));
                spin_lock_irq( &dpc_root.spinlock );
                while( NULL != dpc_root.first ) {
                        OSIF_DPC dpc;
                        dpc = dpc_root.first;
                        dpc_root.first = dpc->next;    /* link out          */
                        dpc->linked = 0;               /* mark: linked out  */
                        dpc->running = 1;              /* mark: running     */
                        spin_unlock_irq( &dpc_root.spinlock );
                        dpc->func(dpc->arg);
                        spin_lock_irq( &dpc_root.spinlock );
                        dpc->running = 0;              /* mark: not running */
                }
#ifdef OSIF_USE_PREEMPT_RT_IMPLEMENTATION
                set_current_state(TASK_INTERRUPTIBLE);
                if (!atomic_read(&dpc_root.trig)) {
                        spin_unlock_irq( &dpc_root.spinlock );
                        schedule();
                } else {
                        spin_unlock_irq( &dpc_root.spinlock );
                }
                __set_current_state(TASK_RUNNING);
#else
                spin_unlock_irq( &dpc_root.spinlock );
#endif
        }
        /* notify that backend thread is exiting */
        dpc_root.thread_running = 0;
#ifdef OSIF_USE_PREEMPT_RT_IMPLEMENTATION
        wake_up(&dpc_root.wqDpcNotify);
#else
        up(&dpc_root.sema_notify);
#endif
        OSIF_DBG((OSIF_ZONE_OSIF, "%s: leave\n", OSIF_FUNCTION));
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0)
        complete_and_exit(&dpcThreadCompletion, 0);
#endif
        return 0;  /* This will never reached, if kernel >= 2.4.0 */
}

static INT32 osif_dpc_root_create( VOID )
{
#ifndef OSIF_USE_PREEMPT_RT_IMPLEMENTATION
        INT32 threadId;
#endif

        OSIF_DBG((OSIF_ZONE_OSIF, "%s: enter\n", OSIF_FUNCTION));
        dpc_root.thread_running = 0;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
        spin_lock_init(&dpc_root.spinlock);
#else
        dpc_root.spinlock = SPIN_LOCK_UNLOCKED;
#endif
#ifdef OSIF_USE_PREEMPT_RT_IMPLEMENTATION
        init_waitqueue_head(&dpc_root.wqDpcNotify);
        dpc_root.tsk = kthread_run(osif_dpc_thread, NULL, K_DPC_NAME);
        if (IS_ERR(dpc_root.tsk)) {
                return OSIF_EIO;
        }
        if (!wait_event_timeout(dpc_root.wqDpcNotify, (1 == dpc_root.thread_running), 100)) { /* wait for DPC thread to start */
                return OSIF_INSUFFICIENT_RESOURCES;
        }
#else
        sema_init( &dpc_root.sema_trigger, 0 );
        sema_init( &dpc_root.sema_notify, 0 );
        threadId = kernel_thread(osif_dpc_thread, 0, 0);
        if ( 0 == threadId ) {
                return CANIO_EIO;
        }
        down( &dpc_root.sema_notify );
        if ( 0 == dpc_root.thread_running ) {
                return OSIF_INSUFFICIENT_RESOURCES;
        }
#endif
        OSIF_DBG((OSIF_ZONE_OSIF, "%s: leave\n", OSIF_FUNCTION));
        return OSIF_SUCCESS;
}

static INT32 osif_dpc_root_destroy( VOID )
{
        OSIF_DBG((OSIF_ZONE_OSIF, "%s: enter\n", OSIF_FUNCTION));
        dpc_root.destroy = 1;
#ifdef OSIF_USE_PREEMPT_RT_IMPLEMENTATION
        wake_up_process(dpc_root.tsk);
        wait_event_timeout(dpc_root.wqDpcNotify, (0 == dpc_root.thread_running), 100);
#else
        up(&dpc_root.sema_trigger);
        down(&dpc_root.sema_notify);
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0)
        wait_for_completion(&dpcThreadCompletion);
#endif
        dpc_root.destroy = 0;
        return OSIF_SUCCESS;
}

INT32 OSIF_CALLTYPE osif_dpc_create( OSIF_DPC *dpc, VOID (OSIF_CALLTYPE *func)(VOID *), VOID *arg )
{
        INT32 result;

        OSIF_DBG((OSIF_ZONE_OSIF, "%s: enter\n", OSIF_FUNCTION));
        result = OSIF_MALLOC( sizeof(**dpc), (VOID*)dpc );
        if ( OSIF_SUCCESS != result ) {
                return result;
        }
        OSIF_MEMSET( *dpc, 0, sizeof(**dpc) );
        (*dpc)->func = func;
        (*dpc)->arg  = arg;
        OSIF_DBG((OSIF_ZONE_OSIF, "%s: leave\n", OSIF_FUNCTION));
        return OSIF_SUCCESS;
}

INT32 OSIF_CALLTYPE osif_dpc_destroy( OSIF_DPC *dpc )
{
        OSIF_DPC  dpc2;
        UINTPTR   flags;

        OSIF_DBG((OSIF_ZONE_OSIF, "%s: enter\n", OSIF_FUNCTION));
        if (NULL == *dpc) {
                return CANIO_EFAULT;
        }
        spin_lock_irqsave( &dpc_root.spinlock, flags );
        (*dpc)->destroy = 1;                       /* now triggers will ret ENOENT */
        if(*dpc == dpc_root.first) {
                dpc_root.first = (*dpc)->next;     /* link out         */
                (*dpc)->next = NULL;               /* mark: linked out */
        } else {
                dpc2 = dpc_root.first;
                while( NULL != dpc2 ) {
                        if(*dpc == dpc2->next) {
                                dpc2->next = (*dpc)->next;         /* link out */
                                (*dpc)->next = NULL;               /* mark: linked out   */
                                break;
                        }
                        dpc2 = dpc2->next;                      /* move up */
                }
        }
        /* if it was linked, it is now unlinked and ready for removal */
        /* if it was unlinked, it could just run... */
        while( 0 != (*dpc)->running ) {
                spin_unlock_irqrestore( &dpc_root.spinlock, flags );
                OSIF_SLEEP(300);
                spin_lock_irqsave( &dpc_root.spinlock, flags );
        }
        spin_unlock_irqrestore( &dpc_root.spinlock, flags );
        OSIF_FREE(*dpc);
        *dpc = NULL;
        OSIF_DBG((OSIF_ZONE_OSIF, "%s: leave\n", OSIF_FUNCTION));
        return OSIF_SUCCESS;
}

INT32 OSIF_CALLTYPE osif_dpc_trigger( OSIF_DPC *dpc )
{
        UINTPTR flags;

        OSIF_DBG((OSIF_ZONE_OSIF, "%s: enter, dpc=%p\n", OSIF_FUNCTION,(UINTPTR)dpc ));
        if (NULL == *dpc) {
                return CANIO_EFAULT;
        }
        spin_lock_irqsave( &dpc_root.spinlock, flags );
        if( 0 != (*dpc)->destroy ) {
                spin_unlock_irqrestore( &dpc_root.spinlock, flags );
                return CANIO_ENOENT;
        }
        if( 0 == (*dpc)->linked ) {
                if( NULL == dpc_root.first) {
                        dpc_root.first = *dpc;
                } else {
                        OSIF_DPC dpc2;

                        dpc2 = dpc_root.first;
                        while( NULL != dpc2->next ) {
                                dpc2 = dpc2->next;
                        }
                        dpc2->next = *dpc;
                }
                (*dpc)->next = NULL;     /* mark end of chain */
                (*dpc)->linked = 1;      /* mark linked */
                spin_unlock_irqrestore( &dpc_root.spinlock, flags );
#ifdef OSIF_USE_PREEMPT_RT_IMPLEMENTATION
                atomic_inc(&dpc_root.trig); /* BL TODO: check, if usable with all kernel.org kernels...!!! */
                smp_mb();
                wake_up_process(dpc_root.tsk);
#else
                up(&dpc_root.sema_trigger);
#endif
        } else {
                spin_unlock_irqrestore( &dpc_root.spinlock, flags );
        }
        OSIF_DBG((OSIF_ZONE_OSIF, "%s: leave\n", OSIF_FUNCTION));
        return OSIF_SUCCESS;
}

/*
** this timer stub runs on soft-irq-level and triggers a dpc for the user timer handler
*/
static void timer_stub( OSIF_LINUX_TIMER *pLxTimer)
{
        OSIF_DPC_TRIGGER( &pLxTimer->dpc );
}

/*! \fn osif_timer_create( OSIF_TIMER **timer, VOID(OSIF_CALLTYPE *func)(VOID *), VOID *arg, VOID *pCanNode );
 *  \brief Create a timer object.
 *  \param timer a pointer to the created timer structure is returned through this parameter.
 *  \param func func(arg) will be called when timer expires.
 *  \param arg is passed as argument to the timer handler \a func.
 *  The created timer is initially disabled after creation.
 */
INT32 OSIF_CALLTYPE osif_timer_create( OSIF_TIMER *t, VOID (OSIF_CALLTYPE *func)(VOID *), VOID *arg, VOID *pCanNode )
{
        OSIF_LINUX_TIMER *pLxTimer;
        INT32             result;

        OSIF_DBG((OSIF_ZONE_OSIF, "%s: enter\n", OSIF_FUNCTION));
        result = OSIF_MALLOC(sizeof(*pLxTimer), &pLxTimer);
        if (result != OSIF_SUCCESS) {
                return result;
        }
        t->pTimerExt = (VOID*)pLxTimer;
        init_timer(&(pLxTimer->timer));
        pLxTimer->timer.function = (VOID*)timer_stub;
        pLxTimer->timer.data     = (UINTPTR)pLxTimer;
        pLxTimer->timer.expires  = 0;
        OSIF_DPC_CREATE(&pLxTimer->dpc, func, arg);
        OSIF_DBG((OSIF_ZONE_OSIF, "%s: leave\n", OSIF_FUNCTION));
        return OSIF_SUCCESS;
}

/*! \fn osif_timer_destroy( OSIF_TIMER *timer );
 *  \brief Destroy a timer object.
 *  \param timer a pointer to the timer structure.
 */
INT32 OSIF_CALLTYPE osif_timer_destroy( OSIF_TIMER *t )
{
        OSIF_LINUX_TIMER *pLxTimer;

        OSIF_DBG((OSIF_ZONE_OSIF, "%s: enter\n", OSIF_FUNCTION));
        if (NULL == t->pTimerExt) {
                return CANIO_ERROR;
        }
        pLxTimer = (OSIF_LINUX_TIMER*)t->pTimerExt;
        t->pTimerExt = NULL;
        del_timer(&pLxTimer->timer);
        OSIF_DPC_DESTROY(&pLxTimer->dpc);
        OSIF_FREE(pLxTimer);
        OSIF_DBG((OSIF_ZONE_OSIF, "%s: leave\n", OSIF_FUNCTION));
        return OSIF_SUCCESS;
}

/*! \fn osif_timer_set( OSIF_TIMER *timer, UINT32 ms );
 *  \brief Set the expiration time for a timer.
 *  \param timer is a pointer to the timer that should be modified
 *  \param ms new expiration time for this timer in ms. A value of 0
 *  disables the timer.
 */
INT32 OSIF_CALLTYPE osif_timer_set( OSIF_TIMER *t, UINT32 ms )
{
        OSIF_LINUX_TIMER *pLxTimer = (OSIF_LINUX_TIMER*)t->pTimerExt;

        OSIF_DBG((OSIF_ZONE_OSIF, "%s: enter (ms = %d)\n", OSIF_FUNCTION, ms));
        if (ms) {
                if (HZ == 1000) {
                        mod_timer(&(pLxTimer->timer), jiffies + ms);
                } else {
                        mod_timer(&(pLxTimer->timer), jiffies + (((ms * HZ) + 500) / 1000)); /* round correctly */
                }
        } else {
                del_timer(&(pLxTimer->timer));
        }
        OSIF_DBG((OSIF_ZONE_OSIF, "%s: leave\n", OSIF_FUNCTION));
        return OSIF_SUCCESS;
}

INT32 OSIF_CALLTYPE osif_timer_get( OSIF_TIMER *t, UINT32 *ms )
{
        OSIF_DBG((OSIF_ZONE_OSIF, "%s: enter\n", OSIF_FUNCTION));
        if (ms) {
                *ms = jiffies * 1000 / HZ;
        }
        OSIF_DBG((OSIF_ZONE_OSIF, "%s: leave (ms=%d)\n", OSIF_FUNCTION, *ms));
        return OSIF_SUCCESS;
}

/* Defined in "board".cfg,
 *   if the driver is running in a Linux host system
 *   (is not an embedded driver) */
#if defined(HOST_DRIVER) || defined(OSIF_ARCH_ARM)
UINT64 OSIF_CALLTYPE osif_ticks( VOID )
{
        struct timeval tv;
        do_gettimeofday(&tv);
        return ((UINT64)tv.tv_sec * 1000000LL) + (UINT64)tv.tv_usec;
}
#else
# ifdef OSIF_ARCH_PPC
UINT64 __attribute__((__unused__)) osif_ticks( VOID )
{
        unsigned __lo;
        unsigned __hi;
        unsigned __tmp;

        __asm__ __volatile__(
                "1:;"
                "  mftbu        %0;"
                "  mftb         %1;"
                "  mftbu        %2;"
                "  cmplw        %0,%2;"
                "bne- 1b;"
                : "=r" (__hi), "=r" (__lo), "=r" (__tmp)
        );
        return (UINT64) __hi << 32 | (UINT64) __lo;
}

# if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
#   if (defined(BOARD_mecp52))
#    include <asm-ppc/ppcboot.h>  /* AB: needed for mpc5200 ports? */
#   endif
/*extern bd_t __res;*/
# endif  /* LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0) */

#if defined(CONFIG_OF_DEVICE) || defined(CONFIG_OF_FLATTREE)
# include <asm/time.h>
#endif
UINT64 OSIF_CALLTYPE osif_tick_frequency()
{
#if defined(CONFIG_OF_DEVICE) || defined(CONFIG_OF_FLATTREE)
        return tb_ticks_per_sec;
#elif defined (BOARD_pmc440) || defined (BOARD_pmc440fw)
        return (UINT64)(533334400); /* TODO: get CPU frequency from somewhere in kernel */

#elif defined (BOARD_mecp52) || defined (BOARD_mecp512x) /* BL TODO: dirty!!! fix it fix it fix it!!! */
        return (UINT64)(33000000); /* TODO: get CPU frequency from somewhere in kernel */

#elif defined(BOARD_cpci750)
        return (UINT64)(33333000); /* TODO: get CPU frequency from somewhere in kernel */

#else
#   if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
        bd_t *bd = (bd_t *)&__res;
#   else
        bd_t *bd = (bd_t *)__res; /* we find 'unsigned char __res[]' in old kernels */
#   endif  /* LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0) */
        OSIF_DBG((OSIF_ZONE_OSIF, "%s: cpu freq = %d\n", OSIF_FUNCTION, bd->bi_procfreq));
        return (UINT64)(bd->bi_procfreq);
#endif
}
# endif  /* #ifdef OSIF_ARCH_PPC */
#if 0
# ifdef OSIF_ARCH_ARM
UINT64 OSIF_CALLTYPE osif_ticks( VOID )
{
        return 1;
}
UINT64 OSIF_CALLTYPE osif_tick_frequency( VOID )
{
        return 1;
}
# endif
#endif
#endif  /* #ifdef HOST_DRIVER */

UINT32 OSIF_CALLTYPE _osif_div64_32_(UINT64 *n, UINT32 base)
{
        UINT64 rem = *n;
        UINT64 b = base;
        UINT64 res, d = 1;
        UINT32 high = rem >> 32;

        /* Reduce the thing a bit first */
        res = 0;
        if (high >= base) {
                high /= base;
                res = (UINT64) high << 32;
                rem -= (UINT64) (high*base) << 32;
        }
        while ((INT64)b > 0 && b < rem) {
                b <<= 1;
                d <<= 1;
        }
        do {
                if (rem >= b) {
                        rem -= b;
                        res += d;
                }
                b >>= 1;
                d >>= 1;
        } while (d);
        *n = res;
        return rem;
}

VOID OSIF_CALLTYPE osif_ser_out( CHAR8 *str )
{
        UINT32 i;

        /* disable serial interrupt */
        outb( (unsigned char)0, 0x3F9 );
        /* output of string */
        for (i = 0; i < strlen(str); i++) {
                while ( !(inb(0x3F8+5) & 0x20) );
                outb( str[i], 0x3F8 );
        }
        while ( !(inb(0x3F8+5) & 0x20) );
        /* reenable serial interrupt */
/*        outb(0x3F9, 0x??); */ /* AB TODO */
        return;
}

#ifdef OSIF_USE_IO_FUNCTIONS
/*! \fn osif_io_out8(void *addr, UINT8 data);
 *  \brief Write access to I/O-space (8 bit wide)
 *  \param addr target address
 *  \param data this data is written to target address
 */
void OSIF_CALLTYPE osif_io_out8(UINT8 *addr, UINT8 data)
{
#if defined(OSIF_ARCH_ARM)
        writeb((UINT8)data, (void *)(addr));
#else
        outb((UINT8)data, (UINTPTR)(addr));
#endif
}

/*! \fn osif_io_in8(void *addr);
 *  \brief Read access to I/O-space (8 bit wide)
 *  \param addr target address
 *  \return data from target address
 */
UINT8 OSIF_CALLTYPE osif_io_in8(UINT8 *addr)
{
#if defined(OSIF_ARCH_ARM)
        return readb((void *)addr);
#else
        return inb((UINTPTR)addr);
#endif
}

/*! \fn osif_io_out16(void *addr, UINT16 data);
 *  \brief Write access to I/O-space (16 bit wide)
 *  \param addr target address
 *  \param data this data is written to target address
 */
void OSIF_CALLTYPE osif_io_out16(UINT16 *addr, UINT16 data)
{
#if defined(OSIF_ARCH_ARM)
        writew((UINT16)data, (void *)(addr));
#else
        outw((UINT16)data, (UINTPTR)(addr));
#endif
}

/*! \fn osif_io_in16(void *addr);
 *  \brief Read access to I/O-space (16 bit wide)
 *  \param addr target address
 *  \return data from target address
 */
UINT16 OSIF_CALLTYPE osif_io_in16(UINT16 *addr)
{
#if defined(OSIF_ARCH_ARM)
        return readw((void *)addr);
#else
        return inw((UINTPTR)addr);
#endif
}

/*! \fn osif_io_out32(void *addr, UINT32 data);
 *  \brief Write access to I/O-space (32 bit wide)
 *  \param addr target address
 *  \param data this data is written to target address
 */
void OSIF_CALLTYPE osif_io_out32(UINT32 *addr, UINT32 data)
{
#if defined(OSIF_ARCH_ARM)
        writel((UINT32)data, (void *)(addr));
#else
        outl((UINT32)data, (UINTPTR)(addr));
#endif
}

/*! \fn osif_io_in32(void *addr);
 *  \brief Read access to I/O-space (32 bit wide)
 *  \param addr target address
 *  \return data from target address
 */
UINT32 OSIF_CALLTYPE osif_io_in32(UINT32 *addr)
{
#if defined(OSIF_ARCH_ARM)
        return readl((void *)addr);
#else
        return inl((UINTPTR)addr);
#endif
}
#endif

/************************************************************************/
/*                     UTILITY CODE                                     */
/************************************************************************/
INT32 OSIF_CALLTYPE osif_make_ntcan_serial(const CHAR8* pszSerial,
                                           UINT32 *pulSerial)
{
        INT32 lTemp;

        *pulSerial = 0;
        if(pszSerial[0] >= 'A' && pszSerial[0] <= 'P' &&
           pszSerial[1] >= 'A' && pszSerial[1] <= 'P')  {
                lTemp = ((pszSerial[0] - 'A') << 28) | ((pszSerial[1] - 'A') << 24);
                *pulSerial = simple_strtoul(&pszSerial[2], NULL, 10);
                *pulSerial |= (UINT32)lTemp;
        }
        return OSIF_SUCCESS;
}

/*
 * RCS/CVS id and build tag with support to prevent compiler warnings about
 * unused vars for different compiler
 */
#if defined(_MSC_VER) && (_MSC_VER >= 800)                           /* VC++ 32 Bit compiler */
 static char* rcsid = "$Id: osif.c 14792 2015-07-13 19:01:09Z stefanm $";
#elif defined (__ghs__)                         /* Green Hills compiler */
# pragma ident "$Id: osif.c 14792 2015-07-13 19:01:09Z stefanm $"
#elif ((__GNUC__ > 3) || (__GNUC__ == 3 && __GNUC_MINOR__ >=1)) /* GCC >= 3.1 */
 static char* rcsid __attribute__((unused,used)) = "$Id: osif.c 14792 2015-07-13 19:01:09Z stefanm $";
#elif ((__GNUC__ > 2) || (__GNUC__ == 2 && __GNUC_MINOR__ >=7)) /* GCC >= 2.7 */
 static char* rcsid __attribute__((unused)) = "$Id: osif.c 14792 2015-07-13 19:01:09Z stefanm $";
#else                                           /* No or old GNU compiler */
# define USE(var) static void use_##var(void *x) {if(x) use_##var((void *)var);}
 static char* rcsid = "$Id: osif.c 14792 2015-07-13 19:01:09Z stefanm $";
 USE(rcsid);
#endif /* of _MSC_VER >= 800 */
