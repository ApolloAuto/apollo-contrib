/* -*- esdcan-c -*-
 * FILE NAME osifi.h
 *
 * BRIEF MODULE DESCRIPTION
 *           ...
 *
 * history:
 *
 *  $Log$
 *  Revision 1.88  2015/05/29 18:26:10  stefanm
 *  Need a value for OSIF_CACHE_DMA_DEFAULTS. Sigh!
 *
 *  Revision 1.87  2015/05/29 18:20:22  stefanm
 *  Enable the default (dummy) implementation for the DMA handling
 *  abstraction by defining OSIF_CACHE_DMA_DEFAULTS.
 *
 *  Revision 1.86  2014/05/20 12:27:20  andreas
 *  Whitespace change, only
 *
 *  Revision 1.85  2013/12/30 16:53:35  frank
 *  Made adaptions needed for directly accessing the SPI controller via esdcan-of-spi-connector and esd-omap2-mcspi
 *
 *  Revision 1.84  2013/11/19 15:20:01  frank
 *  Added SPI support (BOARD_SPI)
 *
 *  Revision 1.83  2013/08/08 13:09:27  matthias
 *  use software timestamps on ARM
 *
 *  Revision 1.82  2013/07/16 14:17:00  andreas
 *  Typo in comment fixed
 *
 *  Revision 1.81  2013/04/26 14:26:28  andreas
 *  White space change
 *
 *  Revision 1.80  2013/01/18 16:56:46  andreas
 *  Changed OSIF_TIMER implementation
 *  - added CAN_NODE parameter to OSIF_TIMER_CREATE
 *  - OSIF_TIMER structure is defined in osif.h and in osifi.h there's only a static extension OSIF_TIMER_SYS_PART
 *
 *  Revision 1.79  2013/01/14 12:39:16  andreas
 *  Removed rubbish in comment
 *
 *  Revision 1.78  2012/10/23 15:28:44  andreas
 *  Typo
 *
 *  Revision 1.77  2012/02/17 18:39:58  andreas
 *  Fixed OSIF_CALLTYPE position
 *  Untabbified
 *
 *  Revision 1.76  2011/11/01 15:26:55  andreas
 *  Merged with preempt_rt branch
 *  With OSIF_USE_PREEMPT_RT_IMPLEMENTATION the new implementation is used for
 *    all kernels > 2.6.20
 *  Using correct defines for IRQ handler return values as of kernels 2.6.0+
 *
 *  Revision 1.75  2011/04/05 11:31:53  andreas
 *  Changed to make use of new CHAR8 datatype
 *
 *  Revision 1.74  2010/04/16 16:58:42  andreas
 *  Fixed or at least unified OSIF_CALLTYPE declaration on dpc and timer create
 *
 *  Revision 1.73  2010/03/16 10:50:56  andreas
 *  Added include of linux/wait.h for select implementation
 *
 *  Revision 1.72  2010/03/09 10:08:05  matthias
 *  Beginning with 2.6.33 linux/autoconf.h moved to generated/autoconf.h. Also there is no need to include it. The kernel's build system passes it via -include option to gcc.
 *
 *  Revision 1.71  2009/07/03 10:42:49  tkoerper
 *  - Added #ifdefs for arm
 *
 *  Revision 1.70  2009/05/18 08:38:30  andreas
 *  Removed old osif_irq_(de)attach() stuff (already #if 0 for a long time)
 *
 *  Revision 1.69  2009/02/25 16:15:40  andreas
 *  Removed osif_sema_-calls
 *
 *  Revision 1.68  2009/01/29 10:11:05  andreas
 *  Removed osif_in_irq_lock()/-unlock() functions
 *
 *  Revision 1.67  2008/12/10 10:54:46  manuel
 *  Introduced inline functions for ISA port accesses
 *
 *  Revision 1.66  2007/06/05 09:00:06  andreas
 *  Added osif_usleep()
 *
 *  Revision 1.65  2007/05/24 12:35:53  michael
 *  BUGFIX: segfault in osif_div64_32
 *
 *  Revision 1.64  2007/05/24 10:10:28  michael
 *  osif_div64_64 removed.
 *
 *  Revision 1.63  2007/01/24 07:39:53  andreas
 *  Changed #ifdef's around osif_tick_frequency() in order to release PPC host
 *  driver for PMC331
 *
 *  Revision 1.62  2006/12/22 09:57:52  andreas
 *  Fixed for kernels 2.6.19
 *  Exchanged include order of version.h and config.h/autoconf.h
 *  Including autoconf.h instead of config.h for kernels >= 2.6.19
 *
 *  Revision 1.61  2006/10/16 07:18:38  andreas
 *  Another fix for optimized swap macros (swap16 works with gcc2 and gcc3, again)
 *
 *  Revision 1.60  2006/10/13 14:59:25  andreas
 *  Another one for the optimized swap macros.
 *  32- and 64-Bit swaps are optimized for x86_64, too
 *
 *  Revision 1.59  2006/10/13 14:08:39  andreas
 *  Optimized macro for 64-Bit wasn't working under x86_64
 *  Added another variant for x86_64
 *
 *  Revision 1.58  2006/10/13 12:02:43  andreas
 *  Deactivated optimized 32-Bit swap macro for x86_64
 *
 *  Revision 1.57  2006/10/13 10:08:17  andreas
 *  Optimized macro for 64-Bit swap deactivated for gcc 2.x
 *
 *  Revision 1.56  2006/10/12 10:35:23  manuel
 *  Introduced inline assembler macros for swaps on x86
 *
 *  Revision 1.55  2006/02/14 09:17:46  matthias
 *  -don't use OSIF_CALLTYPE with regparm(0) on PPC (not suppoted by ppc compiler)
 *
 *  Revision 1.54  2005/12/08 11:33:41  andreas
 *  Added comment for kernel 2.2.x users
 *
 *  Revision 1.53  2005/11/02 07:00:40  andreas
 *  Added declaration of osif_make_ntcan_serial()
 *
 *  Revision 1.52  2005/10/06 07:43:03  matthias
 *  added 64bit swap macros
 *
 *  Revision 1.51  2005/09/23 15:28:37  manuel
 *  Removed OSIF_PCI_OFFSET=0xC0000000 for PPC
 *
 *  Revision 1.50  2005/07/27 15:50:51  andreas
 *  Corrected OSIF_IRQ_HANDLER_CALLTYPE definition
 *
 *  Revision 1.49  2005/07/13 08:57:04  matthias
 *  added OSIF_IRQ_HANDLER_CALLTYPE
 *
 *  Revision 1.48  2005/04/20 13:00:47  andreas
 *  Added osif_ser_out() (prints debug output to serial)
 *
 *  Revision 1.47  2004/10/29 15:28:53  matthias
 *  use UINT32 instead of u32
 *
 *  Revision 1.46  2004/10/29 14:54:11  matthias
 *  added osif_div64_64
 *
 *  Revision 1.45  2004/10/11 09:11:51  andreas
 *  Changes for linux kernel 2.6
 *   (swap functions, i/o functions, etc.)
 *  Added cvs log tag
 *  Sorted history
 *
 *  13.05.04 - Added OSIF_CALLTYPE to timer functions               ab
 *  11.05.04 - Reworked interrupt handler stuff again
 *             Changed OSIF_EXTERN to extwern OSIF_CALLTYPE         ab
 *  07.05.04 - Changed definition of interrupt handler stuff to
 *               comply with kernel 2.6                             ab
 *  06.05.04 - changed include of modversions for kernel 2.6.x
 *             added asmlinkage to OSIF_EXTERN for kernel 2.6.x
 *             added OSIF_BIG_ENDIAN, etc. for kernel 2.6.x         ab
 *  01.12.03 - removed #ifdef in spinlock-structure                 ab
 *  26.11.03 - Removed OSIF_PCI_MALLOC warnings for non 405-cards   ab
 *  13.11.03 - Corrected debug-info in mutexes                      ab
 *  13.11.03 - OSIF_MUTEX is a sema now Old one is now
 *             OSIF_BUSY_MUTEX                                      mt
 *  05.11.03 - Removed IRQ-flags from mutex structure again         ab
 *  03.11.03 - added field for IRQ-flags in mutex-structure         ab
 *  27.10.03 - New output function osif_dprint (with verbose mask)  ab
 *  18.09.03 - Only one thread for all DPC's                        mt
 *  24.05.02 - first version                                        mf
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
 *  30165 Hannover                         http://www.esd.eu
 *  Germany                                support@esd.eu
 *
 *************************************************************************/
/*! \file osifi.h
    \brief OSIF operating system wrapping header

    This file contains the OSIF prototypes and user macros.
    Any driver should call the OSIF_<xxx> macros instead of the
    osif_<xxx> functions.
*/

#ifndef __OSIFI_H__
#define __OSIFI_H__

#include <linux/version.h>
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,32))
# if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,19))
#  include <linux/autoconf.h>
# else
#  include <linux/config.h>
# endif
#endif
#include <linux/errno.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27)
# include <linux/semaphore.h>
#else
# include <asm/semaphore.h>
#endif
#include <linux/wait.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
# include <linux/irqreturn.h>
#else
# include <linux/interrupt.h>
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
# ifdef OSIF_CALLTYPE
#  undef OSIF_CALLTYPE
# endif
# if !defined(CONFIG_PPC) && !defined(CONFIG_ARM)
#  define OSIF_CALLTYPE __attribute__((regparm(0))) /* asmlinkage */
# else
#  define OSIF_CALLTYPE
# endif
# if defined(CONFIG_X86)
#  define OSIF_ARCH_X86
#  define OSIF_LITTLE_ENDIAN
# endif
# if defined(CONFIG_PPC)
#  define OSIF_ARCH_PPC
#  define OSIF_BIG_ENDIAN
# endif
# if defined(CONFIG_ARM)
#  define OSIF_ARCH_ARM
#  define OSIF_LITTLE_ENDIAN
# endif
#endif

/* BL Note: Actually not a Linux induced version discrepancy,
            but code is only tested on versions higher 2.6.20 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,20)
# define OSIF_USE_PREEMPT_RT_IMPLEMENTATION
#endif

/*
 * Select the default (dummy) implementation for cache DMA abstraction.
 * Keep in mind that for Linux the functions always must be implemented
 * if you stop using the default (dummy) implementation.
 */
#define OSIF_CACHE_DMA_DEFAULTS 1

#include "sysdep.h"

#ifndef NULL
# define NULL ((void*)0)
#endif

#include <osif_spi.h>

#ifdef __KERNEL__

typedef struct _OSIF_SPIDEV OSIF_SPIDEV;

struct _OSIF_SPIDEV {
  // spi_funcs_t*   funcs; /* SPI master library function table */
  // struct spi_device* dev; /* SPI device handle */
  struct spi_candev *scdev;
  // int              devno; /* Device number on bus */
};

typedef struct __OSIF_MUTEX     *OSIF_MUTEX;

typedef struct __OSIF_SPINLOCK  *OSIF_BUSY_MUTEX, *OSIF_IRQ_MUTEX;

typedef struct __OSIF_DPC _OSIF_DPC, *OSIF_DPC;
struct __OSIF_DPC {
        OSIF_DPC            next;
        INT8                linked;
        INT8                running;
        INT8                destroy;
        INT8                reserved;
        VOID (OSIF_CALLTYPE *func)(VOID *);
        VOID                *arg;
};

typedef struct _OSIF_LINUX_TIMER  OSIF_LINUX_TIMER;

#define OSIF_TIMER_SYS_PART   /* no static extension of OSIF_TIMER struct */

#define OSIF_IRQ_HANDLER_CALLTYPE       OSIF_CALLTYPE
#define OSIF_IRQ_HANDLER_PARAMETER      void*
#define OSIF_IRQ_HANDLER(func,arg)      int (OSIF_IRQ_HANDLER_CALLTYPE func) ( void *arg )
#define OSIF_IRQ_HANDLER_P              int (OSIF_IRQ_HANDLER_CALLTYPE *) ( void *arg )
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
# define OSIF_IRQ_HANDLED               (IRQ_HANDLED)
# define OSIF_IRQ_HANDLED_NOT           (IRQ_NONE)
#else
# define OSIF_IRQ_HANDLED               (1)  /* Equal to Linux2.6 IRQ_HANDLED */
# define OSIF_IRQ_HANDLED_NOT           (0)
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
# define OSIF_IRQ_HANDLER_RETURN( arg ) return IRQ_RETVAL(arg)
#else
# define OSIF_IRQ_HANDLER_RETURN( arg ) return arg
#endif

typedef struct pci_dev *OSIF_PCIDEV;
#define OSIF_PCI_VADDR                  void *
#define OSIF_PCI_MADDR                  UINT32

/*
 *  Dear kernel 2.2.x user,
 *  we are not able to determine the exact version, when
 *  dma_addr_t was introduced into 2.2.x kernels.
 *  If the following typedef leads to an compiletime error,
 *  please comment the typedef.
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,0)
typedef u32 dma_addr_t;
#endif

typedef struct __OSIF_PCI_PADDR  _OSIF_PCI_PADDR, *OSIF_PCI_PADDR;

#if defined(__GNUC__) && defined(CONFIG_X86)
static inline void osif_io_out8(UINT8 *addr, UINT8 data)
{
        asm volatile("outb %0,%1" : : "a" (data), "dN" ((UINT16)(UINTPTR)addr));
}
static inline UINT8 osif_io_in8(UINT8 *addr)
{
        UINT8 data;
        asm volatile("inb %1,%0" : "=a" (data) : "dN" ((UINT16)(UINTPTR)addr));
        return data;
}
static inline void osif_io_out16(UINT16 *addr, UINT16 data)
{
        asm volatile("outw %0,%1" : : "a" (data), "dN" ((UINT16)(UINTPTR)addr));
}
static inline UINT16 osif_io_in16(UINT16 *addr)
{
        UINT16 data;
        asm volatile("inw %1,%0" : "=a" (data) : "dN" ((UINT16)(UINTPTR)addr));
        return data;
}
static inline void osif_io_out32(UINT32 *addr, UINT32 data)
{
        asm volatile("outl %0,%1" : : "a" (data), "dN" ((UINT16)(UINTPTR)addr));
}
static inline UINT32 osif_io_in32(UINT32 *addr)
{
        UINT32 data;
        asm volatile("inl %1,%0" : "=a" (data) : "dN" ((UINT16)(UINTPTR)addr));
        return data;
}
#else
extern void   OSIF_CALLTYPE osif_io_out8(UINT8 *addr, UINT8 data);
extern UINT8  OSIF_CALLTYPE osif_io_in8(UINT8 *addr);
extern void   OSIF_CALLTYPE osif_io_out16(UINT16 *addr, UINT16 data);
extern UINT16 OSIF_CALLTYPE osif_io_in16(UINT16 *addr);
extern void   OSIF_CALLTYPE osif_io_out32(UINT32 *addr, UINT32 data);
extern UINT32 OSIF_CALLTYPE osif_io_in32(UINT32 *addr);
#define OSIF_USE_IO_FUNCTIONS
#endif

#define osif_mem_out8( addr, data )  (*(volatile UINT8* )(addr)) = (data)
#define osif_mem_in8( addr )         (*(volatile UINT8* )(addr))
#define osif_mem_out16( addr, data ) (*(volatile UINT16*)(addr)) = (data)
#define osif_mem_in16( addr )        (*(volatile UINT16*)(addr))
#define osif_mem_out32( addr, data ) (*(volatile UINT32*)(addr)) = (data)
#define osif_mem_in32( addr )        (*(volatile UINT32*)(addr))

#if defined(__GNUC__) && defined(CONFIG_X86)
/* too bad we cannot use 486 assembler here, because nucleus is compiled without knowledge
   of the system it is later used on */
#if (__GNUC__ > 2)
#define osif_swap16(x) \
({ \
        register UINT16 __x; \
        asm ("xchg %b1,%h1" : "=Q" (__x) : "0" ((UINT16)x)); \
        __x; \
})
#else
#define osif_swap16(x) \
({ \
        register UINT16 __x; \
        asm ("xchg %b1,%h1" : "=q" (__x) : "0" ((UINT16)x)); \
        __x; \
})
#endif
#else    /* sorry, no inline assembler macros for you */
#define osif_swap16(x) \
({ \
        UINT16 __x = (x); \
        ((UINT16)( \
                (((UINT16)(__x) & (UINT16)0x00ffU) << 8) | \
                (((UINT16)(__x) & (UINT16)0xff00U) >> 8) )); \
})
#endif

#if defined(__GNUC__) && defined(CONFIG_X86)
#if !defined(CONFIG_X86_64)
#define osif_swap32(x) \
({ \
        register UINT32 __x; \
        asm ("xchg %b1,%h1; roll $16,%1; xchg %b1,%h1" : "=q" (__x) : "0" ((UINT32)x)); \
        __x; \
})
#else
#define osif_swap32(x) \
({ \
        register UINT32 __x; \
        asm ("bswap %1" : "=r" (__x) : "0" ((UINT32)x)); \
        __x; \
})
#endif
#else    /* sorry, no inline assembler macros for you */
#define osif_swap32(x) \
({ \
        UINT32 __x = (x); \
        ((UINT32)( \
                (((UINT32)(__x) & (UINT32)0x000000ffUL) << 24) | \
                (((UINT32)(__x) & (UINT32)0x0000ff00UL) <<  8) | \
                (((UINT32)(__x) & (UINT32)0x00ff0000UL) >>  8) | \
                (((UINT32)(__x) & (UINT32)0xff000000UL) >> 24) )); \
})
#endif

#if (__GNUC__ > 2) && defined(CONFIG_X86)
#if !defined(CONFIG_X86_64)
#define osif_swap64(x) \
({ \
        register UINT64 __x; \
        asm ("xchg %%ah,%%al; roll $16,%%eax; xchg %%ah,%%al; " \
             "xchg %%eax,%%edx; " \
             "xchg %%ah,%%al; roll $16,%%eax; xchg %%ah,%%al" : "=A" (__x) : "0" ((UINT64)x)); \
        __x; \
})
#else
#define osif_swap64(x) \
({ \
        register UINT64 __x; \
        asm ("bswap %1" : "=r" (__x) : "0" ((UINT64)x)); \
        __x; \
})
#endif
#else    /* sorry, no inline assembler macros for you */
#define osif_swap64(x) \
({ \
        UINT64 __x = (x); \
        ((UINT64)( \
                (((UINT64)(__x) & (UINT64)0x00000000000000ffULL) << (8*7)) | \
                (((UINT64)(__x) & (UINT64)0x000000000000ff00ULL) << (8*5)) | \
                (((UINT64)(__x) & (UINT64)0x0000000000ff0000ULL) << (8*3)) | \
                (((UINT64)(__x) & (UINT64)0x00000000ff000000ULL) << (8*1)) | \
                (((UINT64)(__x) & (UINT64)0x000000ff00000000ULL) >> (8*1)) | \
                (((UINT64)(__x) & (UINT64)0x0000ff0000000000ULL) >> (8*3)) | \
                (((UINT64)(__x) & (UINT64)0x00ff000000000000ULL) >> (8*5)) | \
                (((UINT64)(__x) & (UINT64)0xff00000000000000ULL) >> (8*7)) )); \
})
#endif

#ifdef OSIF_LITTLE_ENDIAN
#define osif_cpu2be16(x) osif_swap16(x)
#define osif_cpu2be32(x) osif_swap32(x)
#define osif_cpu2be64(x) osif_swap64(x)
#define osif_be162cpu(x) osif_swap16(x)
#define osif_be322cpu(x) osif_swap32(x)
#define osif_be642cpu(x) osif_swap64(x)
#define osif_cpu2le16(x) (x)
#define osif_cpu2le32(x) (x)
#define osif_cpu2le64(x) (x)
#define osif_le162cpu(x) (x)
#define osif_le322cpu(x) (x)
#define osif_le642cpu(x) (x)
#else
#define osif_cpu2be16(x) (x)
#define osif_cpu2be32(x) (x)
#define osif_cpu2be64(x) (x)
#define osif_be162cpu(x) (x)
#define osif_be322cpu(x) (x)
#define osif_be642cpu(x) (x)
#define osif_cpu2le16(x) osif_swap16(x)
#define osif_cpu2le32(x) osif_swap32(x)
#define osif_cpu2le64(x) osif_swap64(x)
#define osif_le162cpu(x) osif_swap16(x)
#define osif_le322cpu(x) osif_swap32(x)
#define osif_le642cpu(x) osif_swap64(x)
#endif

#define osif_strtoul(pc,pe,b)   simple_strtoul(pc,pe,b)

extern INT32 OSIF_CALLTYPE osif_attach( VOID );
extern INT32 OSIF_CALLTYPE osif_detach( VOID );


extern INT32 osif_spi_attach(OSIF_SPIDEV *pDev, uint32_t spiClk);
extern INT32 osif_spi_detach(OSIF_SPIDEV *pDev);
extern INT32 osif_spi_xfer(OSIF_SPIDEV *pDev, OSIF_SPIMSG *pMsg);


extern INT32 OSIF_CALLTYPE osif_malloc(UINT32 size, VOID **p);
extern INT32 OSIF_CALLTYPE osif_free(VOID *ptr);
extern INT32 OSIF_CALLTYPE osif_memset(VOID *ptr, UINT32 val, UINT32 size);
extern INT32 OSIF_CALLTYPE osif_memcpy(VOID *dst, VOID *src, UINT32 size);
extern INT32 OSIF_CALLTYPE osif_memcmp(VOID *dst, VOID *src, UINT32 size);

extern VOID OSIF_CALLTYPE set_output_mask(UINT32 mask);

#ifdef OSIF_OS_RTAI
# ifdef BOARD_pci405fw
extern INT32 OSIF_CALLTYPE pci405_printk(const char *fmt, ...);
extern INT32 OSIF_CALLTYPE pci405_printk_init( VOID );
extern INT32 OSIF_CALLTYPE pci405_printk_cleanup( VOID );
#  define osif_print(fmt... ) pci405_printk( ## fmt )
# else
#  define osif_print(fmt... ) rt_printk( ## fmt )
# endif
#else
extern INT32 OSIF_CALLTYPE osif_print(const CHAR8 *fmt, ...);
#endif
extern INT32 OSIF_CALLTYPE osif_dprint(UINT32 output_mask, const CHAR8 *fmt, ...);

extern INT32 OSIF_CALLTYPE osif_snprintf(CHAR8 *str, INT32 size, const CHAR8 *format, ...);

extern INT32 OSIF_CALLTYPE osif_mutex_create(OSIF_MUTEX *m);
extern INT32 OSIF_CALLTYPE osif_mutex_destroy(OSIF_MUTEX *m);
extern INT32 OSIF_CALLTYPE osif_mutex_lock(OSIF_MUTEX *m);
extern INT32 OSIF_CALLTYPE osif_mutex_unlock(OSIF_MUTEX *m);

extern INT32 OSIF_CALLTYPE osif_irq_mutex_create(OSIF_IRQ_MUTEX *im);
extern INT32 OSIF_CALLTYPE osif_irq_mutex_destroy(OSIF_IRQ_MUTEX *im);
extern INT32 OSIF_CALLTYPE osif_irq_mutex_lock(OSIF_IRQ_MUTEX *im);
extern INT32 OSIF_CALLTYPE osif_irq_mutex_unlock(OSIF_IRQ_MUTEX *im);

extern INT32 OSIF_CALLTYPE osif_sleep(INT32 ms);
extern INT32 OSIF_CALLTYPE osif_usleep(INT32 us);

extern INT32 OSIF_CALLTYPE osif_dpc_create( OSIF_DPC  *dpc, VOID(OSIF_CALLTYPE *func)(VOID *), VOID *arg );
extern INT32 OSIF_CALLTYPE osif_dpc_destroy( OSIF_DPC *dpc );
extern INT32 OSIF_CALLTYPE osif_dpc_trigger( OSIF_DPC *dpc );

extern INT32 OSIF_CALLTYPE osif_timer_create( OSIF_TIMER *timer, VOID(OSIF_CALLTYPE *func)(VOID *), VOID *arg, VOID *pCanNode );
extern INT32 OSIF_CALLTYPE osif_timer_destroy( OSIF_TIMER *timer );
extern INT32 OSIF_CALLTYPE osif_timer_set( OSIF_TIMER *timer, UINT32 ms );
extern INT32 OSIF_CALLTYPE osif_timer_get( OSIF_TIMER *t, UINT32 *ms );

extern UINT64 OSIF_CALLTYPE osif_ticks( VOID );
extern UINT64 OSIF_CALLTYPE osif_tick_frequency( VOID );

extern INT32 OSIF_CALLTYPE osif_pci_malloc(OSIF_PCIDEV pcidev, OSIF_PCI_VADDR *vaddr,
                                           OSIF_PCI_PADDR *paddr, OSIF_PCI_MADDR *maddr, UINT32 size);
extern INT32 OSIF_CALLTYPE osif_pci_free(OSIF_PCIDEV pcidev, OSIF_PCI_VADDR vaddr,
                                         OSIF_PCI_PADDR *paddr, OSIF_PCI_MADDR maddr, UINT32 size);
extern UINT32 OSIF_CALLTYPE osif_pci_get_phy_addr(OSIF_PCI_PADDR paddr);

extern INT32 OSIF_CALLTYPE osif_pci_read_config_byte(OSIF_PCIDEV pcidev, INT32 offs, UINT8 *ptr);
extern INT32 OSIF_CALLTYPE osif_pci_read_config_word(OSIF_PCIDEV pcidev, INT32 offs, UINT16 *ptr);
extern INT32 OSIF_CALLTYPE osif_pci_read_config_long(OSIF_PCIDEV pcidev, INT32 offs, UINT32 *ptr);
extern INT32 OSIF_CALLTYPE osif_pci_write_config_byte(OSIF_PCIDEV pcidev, INT32 offs, UINT8 val);
extern INT32 OSIF_CALLTYPE osif_pci_write_config_word(OSIF_PCIDEV pcidev, INT32 offs, UINT16 val);
extern INT32 OSIF_CALLTYPE osif_pci_write_config_long(OSIF_PCIDEV pcidev, INT32 offs, UINT32 val);

extern INT32 OSIF_CALLTYPE osif_load_from_file( UINT8 *destaddr, UINT8 *filename );

extern VOID OSIF_CALLTYPE osif_ser_out(CHAR8 *str);

#define osif_div64_32(n, base) _osif_div64_32_(&(n), (base))
extern UINT32 OSIF_CALLTYPE _osif_div64_32_(UINT64 *n, UINT32 base);

/*
 * If we get a serial it is assumed that it has the format CCNNNNNN where
 * C is a character from 'A'to 'P' and N is a number from '0' to '9'
 * This is converted into an 32-bit integer with bit 31-28 as alphabetical
 * offset to 'A' of the first char in serial, bit 27-24 as alphabetical
 * offset to 'A' of the second char in serial and the remaining 24 bits
 * as serial. So we have an effective range from AA000000 to PPFFFFFF.
 */
extern INT32 OSIF_CALLTYPE osif_make_ntcan_serial(const CHAR8 *pszSerial, UINT32 *pulSerial);

#ifndef OSIF_PCI_OFFSET
# define OSIF_PCI_OFFSET 0
#endif

/*
 * If OSIF_TICK_FREQ is not defined here, UINT64 osif_tick_frequency()
 * must be implemented in osif.c
 * Here we define the frequency to 1MHz in order to work with
 * gettimeofday()
 */
#if defined(HOST_DRIVER) || defined(OSIF_ARCH_ARM)
# define OSIF_TICK_FREQ             1000000LL
#endif

#define OSIF_READB(base, reg)             (*(volatile UINT8* )((base)+(reg)))
#define OSIF_WRITEB(base, reg, data)      (*(volatile UINT8* )((base)+(reg))) = (data)

#define OSIF_KERNEL
#endif /* __KERNEL__ */

#define OSIF_ERRNO_BASE             0x00000100  /* base for esd-codes */

#endif
