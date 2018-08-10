/* -*- esdcan-c -*-
 * FILE NAME osif.h
 *
 * BRIEF MODULE DESCRIPTION
 *           ...
 *
 *
 * history:
 *
 *  24.05.02 - first version                                        mf
 *  27.10.03 - New output function osif_dprint (with verbose mask)  ab
 *  13.11.03 - OSIF_BUSY_MUTEX_xxx added                            mt
 *  11.05.04 - Removed OSIF_EXTERN, added OSIF_CALLTYPE             ab
 *  23.09.04 - Corrected endianess-macros and memcmp (please define
 *             them in the osifi.h the respective system)           ab
 *  28.09.06 - Added OSIF_PRIx64, OSIF_PRId64, OSIF_PRIu64          mk
 *  05.06.07 - Added OSIF_USLEEP and C_ASSERT                       ab
 *
 */
/************************************************************************
 *
 *  Copyright (c) 1996 - 2015 by esd electronic system design gmbh
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
/*! \file osif.h
    \brief OSIF header

    This file contains the OSIF prototypes and user macros.
    Any driver should call the OSIF_<xxx> macros instead of the
    osif_<xxx> functions.
*/

#ifndef __OSIF_H__
#define __OSIF_H__

#define OSIF_KERNEL

#define OSIF_CALLTYPE
#define OSIF_INLINE   inline
#define OSIF_STATIC   static
#define OSIF_REGISTER register

#define OSIF_FUNCTION __FUNCTION__

/* Leave _before_ including osifi.h */
typedef struct _OSIF_TIMER OSIF_TIMER;

#include <osiftypes.h>
#include <osifi.h>
#include <canio.h>
#ifdef OSIF_MEMDBG
# include <memdbg.h>
#endif

/*
 * Default definition for a pattern of the type
 *
 *              do { code; }  OSIF_ONCE
 *
 * where OSIF_ONCE can be defined in osifi.h in a target specific way that the
 * compiler does not generate warnings like 'condition always constant' or
 * 'code without effect'.
 */
#ifndef OSIF_ONCE
# define OSIF_ONCE   while(0)
#endif

#define OSIF_ATTACH()                      osif_attach()
#define OSIF_DETACH()                      osif_detach()

#ifdef OSIF_MEMDBG
# define OSIF_MALLOC(s,p)                  osif_memdbg_malloc((s),(VOID**)(p), __FILE__, __LINE__)
# define OSIF_FREE(p)                      osif_memdbg_free((p))
#else
# define OSIF_MALLOC(s,p)                  osif_malloc((s),(VOID**)(p)) /*!< see osif_malloc(...) */
# define OSIF_FREE(p)                      osif_free((p))               /*!< see osif_free(...) */
#endif
#define OSIF_MEMSET(p,v,s)                 osif_memset((p),(v),(s))     /*!< see osif_memset(...) */
#define OSIF_MEMCPY(d,s,n)                 osif_memcpy((d),(s),(n))
#define OSIF_MEMCMP(d,s,n)                 osif_memcmp((d),(s),(n))
#define OSIF_MEMMEM(s,sl,p,pl)             osif_memmem((s),(sl),(p),(pl))

#define OSIF_MUTEX_CREATE(m)               osif_mutex_create(m)
#define OSIF_MUTEX_DESTROY(m)              osif_mutex_destroy(m)
#define OSIF_MUTEX_LOCK(m)                 osif_mutex_lock(m)
#define OSIF_MUTEX_UNLOCK(m)               osif_mutex_unlock(m)

#define OSIF_IRQ_MUTEX_CREATE(im)          osif_irq_mutex_create(im)
#define OSIF_IRQ_MUTEX_DESTROY(im)         osif_irq_mutex_destroy(im)
#define OSIF_IRQ_MUTEX_LOCK(im)            osif_irq_mutex_lock(im)
#define OSIF_IRQ_MUTEX_UNLOCK(im)          osif_irq_mutex_unlock(im)
#define OSIF_IN_IRQ_MUTEX_LOCK(im)         osif_in_irq_mutex_lock(im)
#define OSIF_IN_IRQ_MUTEX_UNLOCK(im)       osif_in_irq_mutex_unlock(im)

#define OSIF_STRTOUL(pc,pe,b)              osif_strtoul(pc,pe,b)

#define OSIF_PRINT(fmt)                    osif_print fmt
#define OSIF_DPRINT(fmt)                   osif_dprint fmt
#define OSIF_SNPRINTF(args)                osif_snprintf args
#define OSIF_SLEEP(ms)                     osif_sleep(ms)
#define OSIF_USLEEP(us)                    osif_usleep(us)

#define OSIF_DPC_CREATE(d,f,a)             osif_dpc_create((d),(f),(a))
#define OSIF_DPC_DESTROY(d)                osif_dpc_destroy(d)
#define OSIF_DPC_TRIGGER(d)                osif_dpc_trigger(d)

#ifndef OSIF_TIMER_SYS_PART
#error "No system specific OSIF_TIMER_SYS_PART definition!"
#endif
/* General part of timer structure, equal on all systems */
/* Note: typedef _before_ including osifi.h */
struct _OSIF_TIMER {
        VOID   *pTimerExt;    /* Used for dynamic timer extensions, be it HW timers or OS specific stuff */
        OSIF_TIMER_SYS_PART;  /* On systems with stable and well defined APIs (ALL except Linux), this can be used to statically add the system specific stuff */
};

typedef struct _OSIF_CALLBACKS OSIF_CALLBACKS;
struct _OSIF_CALLBACKS {
        INT32 (OSIF_CALLTYPE *timer_create)( OSIF_TIMER *t, VOID (OSIF_CALLTYPE *func)(VOID *), VOID *arg, VOID *pCanNode );
        INT32 (OSIF_CALLTYPE *timer_destroy)( OSIF_TIMER *t );
        INT32 (OSIF_CALLTYPE *timer_set)( OSIF_TIMER *t, UINT32 ms );
        INT32 (OSIF_CALLTYPE *timer_get)( OSIF_TIMER *t, UINT32 *ms );
};

extern OSIF_CALLBACKS  osif_callbacks;

#define OSIF_TIMER_CREATE(t,f,a,n)         osif_callbacks.timer_create((t),(f),(a),(n))
#define OSIF_TIMER_DESTROY(t)              osif_callbacks.timer_destroy(t)
#define OSIF_TIMER_SET(t,m)                osif_callbacks.timer_set((t),(m))
#define OSIF_TIMER_GET(t,m)                osif_callbacks.timer_get((t),(m))

/* ----------------------------------------------------------------
 * DMA handling abstraction for forced flush and invalidate.
 * c  = UINTPTR as context (pointer or value)
 * pc = address of UINTPTR context (pointer or value)
 * p  = physical memory address of DMA area
 * b  = virtual (CPU) base address of section to flush / invalidate
 * s  = size of section
 */

#if OSIF_CACHE_DMA_DEFAULTS
/*
 *      Provide a default implementation for the
 *      OSIF_DMA stuff if selected from osifi.h
 *      by defining OSIF_CACHE_DMA_DEFAULTS.
 */
#ifndef OSIF_CACHE_DMA_ATTACH
#       define OSIF_CACHE_DMA_ATTACH(pc,p)      (*(pc) = (UINTPTR)0)
#endif
#ifndef OSIF_CACHE_DMA_DETACH
#       define OSIF_CACHE_DMA_DETACH(pc,p)      (*(pc) = (UINTPTR)0)
#endif
#ifndef OSIF_CACHE_DMA_FLUSH
#       define OSIF_CACHE_DMA_FLUSH(c,b,s)
#endif
#ifndef OSIF_CACHE_DMA_INVAL
#       define OSIF_CACHE_DMA_INVAL(c,b,s)
#endif
#endif  /* OSIF_CACHE_DMA_DEFAULTS */
/* 
   For documentation purposes prototypes for the functions that may be
   needed to implement in osif.c are added here.

   void osif_cache_dma_attach(UINTPTR *pContext, OSIF_PCI_PADDR *pBase);
   void osif_cache_dma_detach(UINTPTR *pContext, OSIF_PCI_PADDR *pBase);

   void osif_cache_dma_flush(UINTPTR context, OSIF_PCI_VADDR *pBase, UINT32 size);
   void osif_cache_dma_inval(UINTPTR context, OSIF_PCI_VADDR *pBase, UINT32 size);
*/
/* DMA handling abstraction done */


/* OSIF_PCI_MALLOC:
 * v - virtual address, p - bus address (e.g. PCI address), m - physical address */
#define OSIF_PCI_MALLOC(d,v,p,m,s)         osif_pci_malloc((d),(v),(p),(m),(s))
#define OSIF_PCI_FREE(d,v,p,m,s)           osif_pci_free((d),(v),(p),(m),(s))
#define OSIF_PCI_GET_PHY_ADDR(p)           osif_pci_get_phy_addr(p)
#define OSIF_PCI_READ_CONFIG_BYTE(a,b,c)   osif_pci_read_config_byte((a),(b),(c))
#define OSIF_PCI_READ_CONFIG_WORD(a,b,c)   osif_pci_read_config_word((a),(b),(c))
#define OSIF_PCI_READ_CONFIG_LONG(a,b,c)   osif_pci_read_config_long((a),(b),(c))
#define OSIF_PCI_WRITE_CONFIG_BYTE(a,b,c)  osif_pci_write_config_byte((a),(b),(c))
#define OSIF_PCI_WRITE_CONFIG_WORD(a,b,c)  osif_pci_write_config_word((a),(b),(c))
#define OSIF_PCI_WRITE_CONFIG_LONG(a,b,c)  osif_pci_write_config_long((a),(b),(c))

#define OSIF_SPI_XFER(a,b)                 osif_spi_xfer((a), (b))

#define OSIF_IRQ_ATTACH(i,h,n,a)           osif_irq_attach((i),(h),(n),(a))
#define OSIF_IRQ_DETACH(i,a)               osif_irq_detach((i),(a))

#define OSIF_TICKS                         osif_ticks()

#ifndef OSIF_TICK_FREQ
# define OSIF_TICK_FREQ                    osif_tick_frequency()
#endif

#define OSIF_IO_OUT8( addr, data )         osif_io_out8( (addr), (data) )
#define OSIF_IO_IN8( addr )                osif_io_in8( (addr) )
#define OSIF_IO_OUT16( addr, data )        osif_io_out16( (addr), (data) )
#define OSIF_IO_IN16( addr )               osif_io_in16( (addr) )
#define OSIF_IO_OUT32( addr, data )        osif_io_out32( (addr), (data) )
#define OSIF_IO_IN32( addr )               osif_io_in32( (addr) )

#define OSIF_MEM_OUT8( addr, data )        osif_mem_out8( (addr), (data) )
#define OSIF_MEM_IN8( addr )               osif_mem_in8( (addr) )
#define OSIF_MEM_OUT16( addr, data )       osif_mem_out16( (addr), (data) )
#define OSIF_MEM_IN16( addr )              osif_mem_in16( (addr) )
#define OSIF_MEM_OUT32( addr, data )       osif_mem_out32( (addr), (data) )
#define OSIF_MEM_IN32( addr )              osif_mem_in32( (addr) )

#define OSIF_READW(base, reg)              (*(volatile UINT16*)((base)+(reg)))
#define OSIF_WRITEW(base, reg, data)       (*(volatile UINT16*)((base)+(reg))) = (data)
#define OSIF_READLB(base, reg)             (*(volatile UINT32*)((base)+(reg)))
#define OSIF_WRITEL(base, reg, data)       (*(volatile UINT32*)((base)+(reg))) = (data)

#define OSIF_CPU2BE16(x)                   osif_cpu2be16(x)
#define OSIF_CPU2BE32(x)                   osif_cpu2be32(x)
#define OSIF_CPU2BE64(x)                   osif_cpu2be64(x)
#define OSIF_BE162CPU(x)                   osif_be162cpu(x)
#define OSIF_BE322CPU(x)                   osif_be322cpu(x)
#define OSIF_BE642CPU(x)                   osif_be642cpu(x)
#define OSIF_CPU2LE16(x)                   osif_cpu2le16(x)
#define OSIF_CPU2LE32(x)                   osif_cpu2le32(x)
#define OSIF_CPU2LE64(x)                   osif_cpu2le64(x)
#define OSIF_LE162CPU(x)                   osif_le162cpu(x)
#define OSIF_LE322CPU(x)                   osif_le322cpu(x)
#define OSIF_LE642CPU(x)                   osif_le642cpu(x)
#define OSIF_SWAP16(x)                     osif_swap16(x)
#define OSIF_SWAP32(x)                     osif_swap32(x)
#define OSIF_SWAP64(x)                     osif_swap64(x)

#define OSIF_LOAD_FROM_FILE( d, f )        osif_load_from_file( d, f )

#define OSIF_DIV64_32(n, div)              osif_div64_32(n, div)
extern INT32  osif_div64_sft;  /* if your board needs to divide by values larger than 0xFFFFFFFF, set osif_div64_sft respectively */
#define OSIF_DIV64_64(n, div)                                                   \
        do {                                                                    \
                osif_div64_32((n), (UINT32)((div) >> osif_div64_sft));          \
                (n) >>= osif_div64_sft;                                         \
        } OSIF_ONCE

#ifndef OSIF_DIV64_SFT_FIXUP
#define OSIF_DIV64_SFT_FIXUP()                          \
        do {                                            \
                OSIF_TS_FREQ f;                         \
                INT32 min_sft=0;                        \
                f.tick=OSIF_TICK_FREQ;                  \
                while(f.h.HighPart != 0) {              \
                        min_sft++;                      \
                        f.tick >>= 1;                   \
                }                                       \
                if (osif_div64_sft < min_sft) {         \
                        osif_div64_sft = min_sft;       \
                }                                       \
        } OSIF_ONCE
#endif

#define OSIF_SER_OUT(str)                  osif_ser_out((str))

#define OSIF_MAKE_NTCAN_SERIAL(pszSerial, pulSerial) \
        osif_make_ntcan_serial(pszSerial, pulSerial)

/*
 * Macros to round up/down a value to a given multiple which has to be a power of 2.
 */
#define OSIF_ALIGN_UP(x, align)    (((x) + (align - 1)) & ~((unsigned)(align - 1)))
#define OSIF_ALIGN_DOWN(x, align)   ((x) & ~((unsigned)(align - 1)))


/*
 *  Output-zones, use these defines together with OSIF_DPRINT to restrict output
 *  of messages to certain zones. Only zones marked with the lowest byte can produce
 *  output to the user in release-versions. All other versions are debug-only.
 */
#define OSIF_ZONE_NONE     0x00000000   /* no output                                    */
#define OSIF_ZONE_USR_INIT 0x00000001   /* driver load/unload  messages                 */
#define OSIF_ZONE_USR_INFO 0x00000002   /* detailed driver information and runtime msgs */
#define OSIF_ZONE_USR_RES3 0x00000004   /* reserved for future use                      */
#define OSIF_ZONE_USR_STAT 0x00000008   /* driver statistics on unload                  */
#define OSIF_ZONE_USR_RES5 0x00000010   /* reserved for future use                      */
#define OSIF_ZONE_USR_DBG  0x00000020   /* driver troubleshooting information           */
#define OSIF_ZONE_USR_RES7 0x00000040   /* reserved for future use                      */
#define OSIF_ZONE_USR_RES8 0x00000080   /* reserved for future use                      */


#define OSIF_ZONE_BAUD     0x00020000   /* messages from bitrate calculation            */
#define OSIF_ZONE_BM_MSI   0x00040000   /* messages from busmaster or MSI stuff         */
#define OSIF_ZONE_FILTER   0x00080000   /* messages from id filter stuff                */
#define OSIF_ZONE_ESDCAN   0x00100000   /* messages from esdcan-layer                   */
#define OSIF_ZONE_NUC      0x00200000   /* messages from nucleus                        */
#define OSIF_ZONE_BOARD    0x00400000   /* messages from board-layer                    */
#define OSIF_ZONE_CTRL     0x00800000   /* messages from controller-layer               */
#define OSIF_ZONE_OSIF     0x01000000   /* messages from osif-layer                     */

#define OSIF_ZONE_FUNC     0x02000000   /* function entry/exit-messages                 */
#define OSIF_ZONE_PARAM    0x04000000   /* parameter output                             */
#define OSIF_ZONE_IRQ      0x08000000   /* messages from ISR (USE THIS ZONE ALONE!!!)   */
#define OSIF_ZONE_BACKEND  0x10000000   /* output from backend run level                */
#define OSIF_ZONE_INIT     0x20000000   /* output from drivers initialization           */
#define OSIF_ZONE_WARN     0x40000000   /* warnings                                     */
#define OSIF_ZONE_ERROR    0x80000000   /* error-messages                               */

/*
 * Under PCI, each device has 256 bytes of configuration address space,
 * of which the first 64 bytes are standardized as follows:
 */
#define OSIF_PCI_VENDOR_ID                    0x00  /* 16 bits */
#define OSIF_PCI_DEVICE_ID                    0x02  /* 16 bits */
#define OSIF_PCI_COMMAND                      0x04  /* 16 bits */
#define OSIF_PCI_COMMAND_IO                   0x01  /* Enable response in I/O space       */
#define OSIF_PCI_COMMAND_MEMORY               0x02  /* Enable response in Memory space    */
#define OSIF_PCI_COMMAND_MASTER               0x04  /* Enable bus mastering               */
#define OSIF_PCI_COMMAND_SPECIAL              0x08  /* Enable response to special cycles  */
#define OSIF_PCI_COMMAND_INVALIDATE           0x10  /* Use memory write and invalidate    */
#define OSIF_PCI_COMMAND_VGA_PALETTE          0x20  /* Enable palette snooping            */
#define OSIF_PCI_COMMAND_PARITY               0x40  /* Enable parity checking             */
#define OSIF_PCI_COMMAND_WAIT                 0x80  /* Enable address/data stepping       */
#define OSIF_PCI_COMMAND_SERR                0x100  /* Enable SERR                        */
#define OSIF_PCI_COMMAND_FAST_BACK           0x200  /* Enable back-to-back writes         */

#define OSIF_PCI_STATUS                       0x06  /* 16 bits */
#define OSIF_PCI_STATUS_66MHZ                 0x20  /* Support 66 Mhz PCI 2.1 bus         */
#define OSIF_PCI_STATUS_UDF                   0x40  /* Support User Definable Features    */
#define OSIF_PCI_STATUS_FAST_BACK             0x80  /* Accept fast-back to back           */
#define OSIF_PCI_STATUS_PARITY               0x100  /* Detected parity error              */
#define OSIF_PCI_STATUS_DEVSEL_MASK          0x600  /* DEVSEL timing                      */
#define OSIF_PCI_STATUS_DEVSEL_FAST          0x000
#define OSIF_PCI_STATUS_DEVSEL_MEDIUM        0x200
#define OSIF_PCI_STATUS_DEVSEL_SLOW          0x400
#define OSIF_PCI_STATUS_SIG_TARGET_ABORT     0x800  /* Set on target abort                */
#define OSIF_PCI_STATUS_REC_TARGET_ABORT    0x1000  /* Master ack of "                    */
#define OSIF_PCI_STATUS_REC_MASTER_ABORT    0x2000  /* Set on master abort                */
#define OSIF_PCI_STATUS_SIG_SYSTEM_ERROR    0x4000  /* Set when we drive SERR             */
#define OSIF_PCI_STATUS_DETECTED_PARITY     0x8000  /* Set on parity error                */

#define OSIF_PCI_CLASS_REVISION               0x08  /* High 24 bits are class, low 8 revision */
#define OSIF_PCI_REVISION_ID                  0x08  /* Revision ID */
#define OSIF_PCI_CLASS_PROG                   0x09  /* Reg. Level Programming Interface */
#define OSIF_PCI_CLASS_DEVICE                 0x0A  /* Device class */
#define OSIF_PCI_CACHE_LINE_SIZE              0x0C  /* 8 bits */
#define OSIF_PCI_LATENCY_TIMER                0x0D  /* 8 bits */
#define OSIF_PCI_HEADER_TYPE                  0x0E  /* 8 bits */
#define OSIF_PCI_HEADER_TYPE_NORMAL              0
#define OSIF_PCI_HEADER_TYPE_BRIDGE              1
#define OSIF_PCI_HEADER_TYPE_CARDBUS             2

#define OSIF_PCI_BIST                         0x0F  /* 8 bits */
#define OSIF_PCI_BIST_CODE_MASK               0x0F  /* Return result                      */
#define OSIF_PCI_BIST_START                   0x40  /* 1 to start BIST, 2 secs or less    */
#define OSIF_PCI_BIST_CAPABLE                 0x80  /* 1 if BIST capable                  */

/*
 * Base addresses specify locations in memory or I/O space.
 * Decoded size can be determined by writing a value of
 * 0xffffffff to the register, and reading it back.  Only
 * 1 bits are decoded.
 */
#define OSIF_PCI_BASE_ADDRESS_0               0x10  /* 32 bits */
#define OSIF_PCI_BASE_ADDRESS_1               0x14  /* 32 bits [htype 0,1 only] */
#define OSIF_PCI_BASE_ADDRESS_2               0x18  /* 32 bits [htype 0 only] */
#define OSIF_PCI_BASE_ADDRESS_3               0x1C  /* 32 bits */
#define OSIF_PCI_BASE_ADDRESS_4               0x20  /* 32 bits */
#define OSIF_PCI_BASE_ADDRESS_5               0x24  /* 32 bits */
#define OSIF_PCI_BASE_ADDR_SPACE              0x01  /* 0 = memory, 1 = I/O                */
#define OSIF_PCI_BASE_ADDR_SPACE_IO           0x01
#define OSIF_PCI_BASE_ADDR_SPACE_MEMORY       0x00
#define OSIF_PCI_BASE_ADDR_MEM_TYPE_MASK      0x06
#define OSIF_PCI_BASE_ADDR_MEM_TYPE_32        0x00  /* 32 bit address                     */
#define OSIF_PCI_BASE_ADDR_MEM_TYPE_1M        0x02  /* Below 1M                           */
#define OSIF_PCI_BASE_ADDR_MEM_TYPE_64        0x04  /* 64 bit address                     */
#define OSIF_PCI_BASE_ADDR_MEM_PREFETCH       0x08  /* prefetchable?                      */
#define OSIF_PCI_BASE_ADDR_MEM_MASK      (~0x0FUL)
#define OSIF_PCI_BASE_ADDR_IO_MASK       (~0x03UL)
/* bit 1 is reserved if address_space = 1 */

/* Header type 0 (normal devices) */
#define OSIF_PCI_CARDBUS_CIS                  0x28
#define OSIF_PCI_SUBSYSTEM_VENDOR_ID          0x2C
#define OSIF_PCI_SUBSYSTEM_ID                 0x2E
#define OSIF_PCI_ROM_ADDRESS                  0x30  /* Bits 31..11 are address, 10..1 reserved */
#define OSIF_PCI_ROM_ADDRESS_ENABLE           0x01
#define OSIF_PCI_ROM_ADDRESS_MASK       (~0x7FFUL)

/* 0x34-0x3b are reserved */
#define OSIF_PCI_INTERRUPT_LINE               0x3C  /* 8 bits */
#define OSIF_PCI_INTERRUPT_PIN                0x3D  /* 8 bits */
#define OSIF_PCI_MIN_GNT                      0x3E  /* 8 bits */
#define OSIF_PCI_MAX_LAT                      0x3F  /* 8 bits */

/* Device power states */
typedef UINT32  OSIF_POWER_STATE;

#define POWER_STATE_UNSPECIFIED     0U      /* Power state undefined                          */
#define POWER_STATE_D0              1U      /* Fully On is the operating state                */
#define POWER_STATE_D1              2U      /* Intermediate power-state                       */
#define POWER_STATE_D2              3U      /* Intermediate power-state                       */
#define POWER_STATE_D3              4U      /* Device powered off and unresponsive to its bus */

/*
 *  System independent definition of USB relevant stuff, used in board layers
 */
typedef struct __OSIF_USB_EP  OSIF_USB_EP;
struct __OSIF_USB_EP {
        INT32  iDir;
        INT32  iType;
        INT32  iUse;
};

#ifndef OSIF_USB_EP_DIR_DUMMY
# define OSIF_USB_EP_DIR_DUMMY            0x00  /* Use to ignore entry                    */
#endif
#ifndef OSIF_USB_EP_DIR_IN
# define OSIF_USB_EP_DIR_IN               0x01  /* Endpoint direction IN (device->host)   */
#endif
#ifndef OSIF_USB_EP_DIR_OUT
# define OSIF_USB_EP_DIR_OUT              0x02  /* Endpoint direction OUT (host->device)  */
#endif
#ifndef OSIF_USB_EP_TYPE_DUMMY
# define OSIF_USB_EP_TYPE_DUMMY           0x00  /* Use to ignore entry                    */
#endif
#ifndef OSIF_USB_EP_TYPE_CTRL
# define OSIF_USB_EP_TYPE_CTRL            0x01  /* Endpoint type: Control                 */
#endif
#ifndef OSIF_USB_EP_TYPE_ISOC
# define OSIF_USB_EP_TYPE_ISOC            0x02  /* Endpoint type: Isochronous             */
#endif
#ifndef OSIF_USB_EP_TYPE_BULK
# define OSIF_USB_EP_TYPE_BULK            0x03  /* Endpoint type: Bulk                    */
#endif
#ifndef OSIF_USB_EP_TYPE_INT
# define OSIF_USB_EP_TYPE_INT             0x04  /* Endpoint type: Interrupt               */
#endif
#ifndef OSIF_USB_EP_USE_NONE
# define OSIF_USB_EP_USE_NONE               -1  /* Endpoint usage: Don't use              */
#endif
#ifndef OSIF_USB_EP_USE_RX
# define OSIF_USB_EP_USE_RX               0x00  /* Endpoint usage: RX channel             */
#endif
#ifndef OSIF_USB_EP_USE_TX
# define OSIF_USB_EP_USE_TX               0x01  /* Endpoint usage: TX channel             */
#endif
#ifndef OSIF_USB_EP_USE_CMD
# define OSIF_USB_EP_USE_CMD              0x02  /* Endpoint usage: Command channel        */
#endif

#define OSIF_USB_TYPE_CONTROL           0x00
#define OSIF_USB_TYPE_ISOCHRONOUS       0x01
#define OSIF_USB_TYPE_BULK              0x02
#define OSIF_USB_TYPE_INTERRUPT         0x03

#define OSIF_USB_DIR_OUT                0x00
#define OSIF_USB_DIR_IN                 0x01

#define OSIF_USB_EP_DESC(no, dir, type)                                 \
        (((no) & 0x0F) | (((dir) & 0x1) << 7) | (((type) & 0x3) << 8))

#define OSIF_USB_REQ_HOST_TO_DEVICE     0x8000
#define OSIF_USB_REQ_DEVICE_TO_HOST     0x0000

/*
 * Macro to do a compile time size check
 */
#ifndef C_ASSERT
# if defined(_MSC_VER)
#  define C_ASSERT(e) typedef char __C_ASSERT__[(e)?1:-1]
# elif defined(__GNUC__)
#  define C_ASSERT(e) extern char __C_ASSERT__[(e)?1:-1] __attribute__((unused))
# else
#  define C_ASSERT(e)
# endif
#endif

/*
 * Define the highest address which can be stored in a pointer
 */
#define OSIF_PTR_MAX        (~(UINTPTR)0x0)

/*
 * Macro with default definition for a pattern to mark a parameter as unused
 * in a function.
 */
#ifndef OSIF_UNUSED
# define OSIF_UNUSED(arg)   ((void)(arg))
#endif

/*
 *  Error codes (if not defined system specific)
 */
#ifndef OSIF_SUCCESS
#  define OSIF_SUCCESS                CANIO_SUCCESS
#endif
#ifndef OSIF_ERROR
#  define OSIF_ERROR                  CANIO_ERROR
#endif
#ifndef OSIF_INVALID_PARAMETER
#  define OSIF_INVALID_PARAMETER      CANIO_INVALID_PARAMETER
#endif
#ifndef OSIF_INVALID_HANDLE
#  define OSIF_INVALID_HANDLE         CANIO_INVALID_HANDLE
#endif
#ifndef OSIF_NET_NOT_FOUND
#  define OSIF_NET_NOT_FOUND          CANIO_NET_NOT_FOUND
#endif
#ifndef OSIF_INSUFFICIENT_RESOURCES
#  define OSIF_INSUFFICIENT_RESOURCES CANIO_INSUFFICIENT_RESOURCES
#endif
#ifndef OSIF_EAGAIN
#  define OSIF_EAGAIN                 CANIO_EAGAIN
#endif
#ifndef OSIF_EBUSY
#  define OSIF_EBUSY                  CANIO_EBUSY
#endif
#ifndef OSIF_RX_TIMEOUT
#  define OSIF_RX_TIMEOUT             CANIO_RX_TIMEOUT
#endif
#ifndef OSIF_TX_TIMEOUT
#  define OSIF_TX_TIMEOUT             CANIO_TX_TIMEOUT
#endif
#ifndef OSIF_TX_ERROR
#  define OSIF_TX_ERROR               CANIO_TX_ERROR
#endif
#ifndef OSIF_CONTR_OFF_BUS
#  define OSIF_CONTR_OFF_BUS          CANIO_CONTR_OFF_BUS
#endif
#ifndef OSIF_CONTR_BUSY
#  define OSIF_CONTR_BUSY             CANIO_CONTR_BUSY
#endif
#ifndef OSIF_CONTR_WARN
#  define OSIF_CONTR_WARN             CANIO_CONTR_WARN
#endif
#ifndef OSIF_OLDDATA
#  define OSIF_OLDDATA                CANIO_OLDDATA
#endif
#ifndef OSIF_NO_ID_ENABLED
#  define OSIF_NO_ID_ENABLED          CANIO_NO_ID_ENABLED
#endif
#ifndef OSIF_ID_ALREADY_ENABLED
#  define OSIF_ID_ALREADY_ENABLED     CANIO_ID_ALREADY_ENABLED
#endif
#ifndef OSIF_ID_NOT_ENABLED
#  define OSIF_ID_NOT_ENABLED         CANIO_ID_NOT_ENABLED
#endif
#ifndef OSIF_INVALID_FIRMWARE
#  define OSIF_INVALID_FIRMWARE       CANIO_INVALID_FIRMWARE
#endif
#ifndef OSIF_MESSAGE_LOST
#  define OSIF_MESSAGE_LOST           CANIO_MESSAGE_LOST
#endif
#ifndef OSIF_INVALID_HARDWARE
#  define OSIF_INVALID_HARDWARE       CANIO_INVALID_HARDWARE
#endif
#ifndef OSIF_PENDING_WRITE
#  define OSIF_PENDING_WRITE          CANIO_PENDING_WRITE
#endif
#ifndef OSIF_PENDING_READ
#  define OSIF_PENDING_READ           CANIO_PENDING_READ
#endif
#ifndef OSIF_INVALID_DRIVER
#  define OSIF_INVALID_DRIVER         CANIO_INVALID_DRIVER
#endif
#ifndef OSIF_WRONG_DEVICE_STATE
#  define OSIF_WRONG_DEVICE_STATE     CANIO_WRONG_DEVICE_STATE
#endif
#ifndef OSIF_OPERATION_ABORTED
#  define OSIF_OPERATION_ABORTED      CANIO_OPERATION_ABORTED
#endif
#ifndef OSIF_WRONG_DEVICE_STATE
#  define OSIF_WRONG_DEVICE_STATE     CANIO_WRONG_DEVICE_STATE
#endif
#ifndef OSIF_HANDLE_FORCED_CLOSE
#  define OSIF_HANDLE_FORCED_CLOSE    CANIO_HANDLE_FORCED_CLOSE
#endif
#ifndef OSIF_NOT_IMPLEMENTED
#  define OSIF_NOT_IMPLEMENTED        CANIO_NOT_IMPLEMENTED
#endif
#ifndef OSIF_NOT_SUPPORTED
#  define OSIF_NOT_SUPPORTED          CANIO_NOT_SUPPORTED
#endif
#ifndef OSIF_CONTR_ERR_PASSIVE
#  define OSIF_CONTR_ERR_PASSIVE      CANIO_CONTR_ERR_PASSIVE
#endif
#ifndef OSIF_EFAULT
#  define OSIF_EFAULT                 CANIO_EFAULT
#endif
#ifndef OSIF_EIO
#  define OSIF_EIO                    CANIO_EIO
#endif
#ifndef OSIF_ERROR_NO_BAUDRATE
#  define OSIF_ERROR_NO_BAUDRATE      CANIO_ERROR_NO_BAUDRATE
#endif
#ifndef OSIF_ERROR_LOM
#  define OSIF_ERROR_LOM              CANIO_ERROR_LOM
#endif
#ifndef OSIF_EIO
#  define OSIF_EIO                    CANIO_EIO
#endif
#ifndef OSIF_ERESTART
#  define OSIF_ERESTART               CANIO_ERESTART
#endif

/* macros used for printing 64 bit numbers */
#ifndef OSIF_PRIx64
#  define OSIF_PRIx64 "llx"
#endif
#ifndef OSIF_PRId64
#  define OSIF_PRId64 "lld"
#endif
#ifndef OSIF_PRIu64
#  define OSIF_PRIu64 "llu"
#endif

    /* Double stringification to convert value into "value" */
#define OSIF_STRINGIZE(str)      #str
#define OSIF_INT2STRING(val)     OSIF_STRINGIZE(val)

    /* Build info string for several compilers/platforms */
#if defined(_MSC_VER)   /* Microsoft C/C++ Compiler */
# ifdef _M_X64
#  define OSIF_BUILD_INFO __DATE__ " @ " __TIME__ \
          " with MSC V" OSIF_INT2STRING(_MSC_VER) " (64-Bit)"
# else
#  define OSIF_BUILD_INFO __DATE__ " @ " __TIME__ \
          " with MSC V" OSIF_INT2STRING(_MSC_VER) " (32-Bit)"
# endif /* of _M_X64 */
#elif defined(__GNUC__) /* GNU Compiler Collection */
# if ((__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 9)) /* GCC >= 4.9 */
#  pragma GCC diagnostic ignored "-Wdate-time"
# endif
# define OSIF_BUILD_INFO __DATE__ " @ " __TIME__ \
           " with GCC V" OSIF_INT2STRING(__GNUC__) "." OSIF_INT2STRING(__GNUC_MINOR__)
#elif defined(_UCC)     /* OS/9 Ultra C */
#define  OSIF_BUILD_INFO __DATE__ " @ " __TIME__ \
           " with UCC V" OSIF_INT2STRING(_MAJOR_REV) "." OSIF_INT2STRING(_MINOR_REV)
#else
# define OSIF_BUILD_INFO __DATE__ " @ " __TIME__
#endif /* of _MSC_VER */

    /*
     * Macros to suppress MSC warnings. The MSC specific __pragma complements
     * the #pragma preprocessor directive but allows a usage in macros. See
     * http://msdn.microsoft.com/en-us/library/d9x1s805.aspx for details.
     */
#if defined(_MSC_VER)
    /*lint -emacro(19, MSC_DISABLE_*, MSC_ENABLE_*) */ /* Useless Declaration */

    /*
     * MSC_SUPPRESS_WARNING stores the current state for all warnings, disables
     * the specified warning w for the next line, and then restores the warning
     * stack.
     */
# define MSC_DISABLE_WARNING_ONCE(w)    __pragma(warning(suppress:w))

    /*
    * MSC_DISABLE_WARNING stores the current state for all warnings and disables
    * the specified warning(s) |w|. The warning(s) remain disabled until the
    * warning stack is restored by MSC_ENABLE_WARNING. Several warning numbers
    * have to be separated by blanks.
    */
# define MSC_DISABLE_WARNING(w)         __pragma(warning(push)) \
                                        __pragma(warning(disable:w))

    /*
     * MSC_DISABLE_WARNING_LEVEL stores the current state for all warnings and
     * sets the global warning level to l (1..4). The level remains in effect
     * until restored by MSC_ENABLE_WARNING. Use 0 to disable all warnings.
     */
# define MSC_DISABLE_WARNING_LEVEL(l)   __pragma(warning(push, l))

    /* Reverses effects of innermost MSC_DISABLE_* macro. */
# define MSC_ENABLE_WARNING             __pragma(warning(pop))

    /* Control the MSC optimizer */
# define MSC_DISABLE_OPTIMIZER          __pragma(optimize("", off))
# define MSC_ENABLE_OPTIMIZER           __pragma(optimize("", on))

#else
    /* Define this for !MSC compilers to use it without compiler check */
# define MSC_VERSION_AT_LEAST(version)  0

    /* Empty defines for !MSC compilers to suppress warnings */
# define MSC_DISABLE_WARNING_ONCE(w)
# define MSC_DISABLE_WARNING(w)
# define MSC_DISABLE_WARNING_LEVEL(l)
# define MSC_ENABLE_WARNING

    /* Empty defines for !MSC compilers to control the optimizer */
# define MSC_DISABLE_OPTIMIZER
# define MSC_ENABLE_OPTIMIZER
#endif /* of _MSC_VER */

/*
 * Used for internal split handling of 64-bit timestamps
 */
typedef union
{
        UINT64 tick;
        struct {
#ifdef OSIF_LITTLE_ENDIAN
                UINT32 LowPart;
                UINT32 HighPart;
#endif
#ifdef OSIF_BIG_ENDIAN
                UINT32 HighPart;
                UINT32 LowPart;
#endif
        } h;
} OSIF_TS, OSIF_TS_FREQ;


/*
 * Used for double linked list with reference to base object.
 *
 * Helper macros and code are part of the nucleus as OSIF has no common code
 */
typedef struct _LNK  LNK;
struct _LNK {
        LNK         *next;
        LNK         *prev;
        VOID        *base;
};

#endif
