/* canio.h
**
**      Copyright (c) 2001-2015 by electronic system design gmbh
**
**  This software is copyrighted by and is the sole property of
**  esd gmbh.  All rights, title, ownership, or other interests
**  in the software remain the property of esd gmbh. This
**  software may only be used in accordance with the corresponding
**  license agreement.  Any unauthorized use, duplication, transmission,
**  distribution, or disclosure of this software is expressly forbidden.
**
**  This Copyright notice may not be removed or modified without prior
**  written consent of esd gmbh.
**
**  esd gmbh, reserves the right to modify this software without notice.
**
**  electronic system design gmbh          Tel. +49-511-37298-0
**  Vahrenwalder Str 205                   Fax. +49-511-37298-68
**  30165 Hannover                         http://www.esd.eu
**  Germany                                support@esd.eu
**
**
** user-header for  io-control for esd-can-driver linux
**
*/
#ifndef __CANIO_H__
#define __CANIO_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <linux/ioctl.h>

/* Following define is used to en-/disable code on non-64-Bit platforms */
#define PLATFORM_64BIT  defined(__ppc64__) || defined(__x86_64__)

#ifdef __KERNEL__
# define CANIO_UINT8   u8
# define CANIO_INT8    s8
# define CANIO_UINT16  u16
# define CANIO_INT16   s16
# define CANIO_UINT32  u32
# define CANIO_INT32   s32
# define CANIO_UINT64  u64
# define CANIO_INT64   s64
#else
# include <stdint.h>
# define CANIO_UINT8   uint8_t
# define CANIO_INT8    int8_t
# define CANIO_UINT16  uint16_t
# define CANIO_INT16   int16_t
# define CANIO_UINT32  uint32_t
# define CANIO_INT32   int32_t
# define CANIO_UINT64  uint64_t
# define CANIO_INT64   int64_t
#endif

/* CAN-id-regions */
#define CANIO_IDS_REGION_20A 0
#define CANIO_IDS_REGION_20B 1
#define CANIO_IDS_REGION_EV  2

/* FJ: 2013-04-11
 * EtherCAN server still needs this ...
 */
typedef struct {
  int32_t  id1st;
  int32_t  idlst;
} CAN_IDSET;


#define CANIO_20B_BASE            0x20000000

/* CAN-id-values (11-Bit/20a)   */
#define CANIO_IDS                 0x800      /* range of 20a-IDs */
#define CANIO_ID_BASE             0x00000000
#define CANIO_ID_LAST             (CANIO_ID_BASE+CANIO_IDS-1)

/* CAN-id-values (29-Bit/20b)   */
#define CANIO_IDS_20B             0x20000000 /* range of 20b-IDs */
#define CANIO_ID_20B_BASE         0x20000000
#define CANIO_ID_20B_LAST         (CANIO_ID_20B_BASE+CANIO_IDS_20B-1)

#define CANIO_ID_INVALID          0xFFFFFFFF

/* CAN-event-id-values          */
#define CANIO_EVS                       0x100      /* range of events */
#define CANIO_EV_BASE                   0x40000000
#define CANIO_EV_CAN_ERROR              CANIO_EV_BASE
#define CANIO_EV_BAUD_CHANGE            (CANIO_EV_BASE + 0x01)
#define CANIO_EV_CAN_ERROR_EXT          (CANIO_EV_BASE + 0x02)
#define CANIO_EV_BUSLOAD                (CANIO_EV_BASE + 0x03)
#define CANIO_EV_CAN_DEFERRED_TX        (CANIO_EV_BASE + 0x30)
#define CANIO_EV_ECHO(NR)               (CANIO_EV_BASE + 0x40 + ((NR) & 0x1F))
#define CANIO_EV_ECHO_FIRST             (CANIO_EV_BASE + 0x40)
#define CANIO_EV_ECHO_LAST              (CANIO_EV_BASE + 0x5F)
#define CANIO_EV_DNET_EVPOLL            (CANIO_EV_BASE + 0x7C)
#define CANIO_EV_DNET_EVEXPL            (CANIO_EV_BASE + 0x7D)
#define CANIO_EV_DNET_EV                (CANIO_EV_BASE + 0x7E)
#define CANIO_EV_DNET_CALL              (CANIO_EV_BASE + 0x7F)

#define CANIO_EV_USER                   (CANIO_EV_BASE + 0x80)

#define CANIO_EV_TEMP                   (CANIO_EV_USER + 0x01) /* D31 events */
#define CANIO_EV_WATCHDOG               (CANIO_EV_USER + 0x05) /* D31 events */
#define CANIO_EV_STOP_RELAIS            (CANIO_EV_USER + 0x08)
#define CANIO_EV_TEMP_SENSOR            (CANIO_EV_USER + 0x0A) /* D31 events */

#define CANIO_EV_ARINC_TIMER            (CANIO_EV_USER + 0x20) /* Events used on CAN-xxx/400 */
#define CANIO_EV_ARINC_TIME_SET         (CANIO_EV_USER + 0x21) /* Events used on CAN-xxx/400 */
#define CANIO_EV_ARINC_INTERVAL_SET     (CANIO_EV_USER + 0x22) /* Events used on CAN-xxx/400 */
#define CANIO_EV_ARINC_INTERVAL_GET     (CANIO_EV_USER + 0x23) /* Events used on CAN-xxx/400 */
#define CANIO_EV_ARINC_START_SET        (CANIO_EV_USER + 0x24) /* Events used on CAN-xxx/400 */
#define CANIO_EV_ARINC_START_GET        (CANIO_EV_USER + 0x25) /* Events used on CAN-xxx/400 */
#define CANIO_EV_ARINC_START            (CANIO_EV_USER + 0x26) /* Events used on CAN-xxx/400 */
#define CANIO_EV_ARINC_STOP             (CANIO_EV_USER + 0x27) /* Events used on CAN-xxx/400 */

#define CANIO_EV_IRIGB_STATUS           (CANIO_EV_USER + 0x28) /* Events used on CAN-xxx/400 */
#define CANIO_EV_IRIGB_CONFIG           (CANIO_EV_USER + 0x29) /* Events used on CAN-xxx/400 */
#define CANIO_EV_IRIGB_SCRATCH          (CANIO_EV_USER + 0x2A) /* Events used on CAN-xxx/400 */
#define CANIO_EV_IRIGB_FRAME_LOW        (CANIO_EV_USER + 0x2B) /* Events used on CAN-xxx/400 */
#define CANIO_EV_IRIGB_FRAME_HIGH       (CANIO_EV_USER + 0x2C) /* Events used on CAN-xxx/400 */
#define CANIO_EV_IRIGB_FWINFO_1         (CANIO_EV_USER + 0x2D) /* Events used on CAN-xxx/400 */
#define CANIO_EV_IRIGB_FWINFO_2         (CANIO_EV_USER + 0x2E) /* Events used on CAN-xxx/400 */
#define CANIO_EV_IRIGB_FWINFO_3         (CANIO_EV_USER + 0x2F) /* Events used on CAN-xxx/400 */
#define CANIO_EV_IRIGB_INFO_1           (CANIO_EV_USER + 0x30) /* Events used on CAN-xxx/400 */
#define CANIO_EV_IRIGB_INFO_2           (CANIO_EV_USER + 0x31) /* Events used on CAN-xxx/400 */
#define CANIO_EV_IRIGB_1PPS             (CANIO_EV_USER + 0x32) /* Events used on CAN-xxx/400 */

/* #define CANIO_EV_ECHO                (CANIO_EV_USER + 0x3F)*//* only defined in <win32/canio.h> */

#define CANIO_EV_WATCHDOG_RESET         (CANIO_EV_USER + 0x45) /* D31 events */
#define CANIO_EV_TEMP_RANGE             (CANIO_EV_USER + 0x46) /* D31 events */

#define CANIO_EV_PXI_CLOCK_SOURCE       (CANIO_EV_USER + 0x50) /* Events used on CAN-xxx/400 */
#define CANIO_EV_PXI_STARTRIGGER_CONF   (CANIO_EV_USER + 0x51) /* Events used on CAN-xxx/400 */

#define CANIO_EV_LAST                   (CANIO_EV_BASE + 0xFF)

#define CANIO_DN_BASE                   0x41000000
#define CANIO_DN_LAST                   0x41FFFFFF

/*
 * Flags in CMSG's <len> field
 */
#define CANIO_RTR                       0x10     /* CAN message is RTR       */
                                                 /*  -> NTCAN_FD bit not set */
#define CANIO_ERR_PASSIVE               0x10     /* Sender is error passive  */
                                                 /*  -> NTCAN_FD bit set     */
#define CANIO_NO_DATA                   0x20     /* No updated data          */
                                                 /*  -> Object mode handle   */
#define CANIO_INTERACTION               0x20     /* Interaction data         */
                                                 /*  -> FIFO mode handle     */
#define CANIO_MARK_TX_ERROR             0x40     /* TX done with error in    */
                                                 /*        local-echo mode   */
#define CANIO_FD                        0x80     /* Frame is CAN-FD message  */

#define CANIO_AUTO_ANSWER_VALID         0x20    /* Used only in 2.x driver !!!  */

#define CANIO_DLC(len)                  ((len) & 0x0F)
#define CANIO_DLC_AND_TYPE(len)         ((len) & (0x0F | CANIO_RTR))
#define CANIO_IS_RTR(len)               (((len) & (CANIO_FD | CANIO_RTR)) == CANIO_RTR)
#define CANIO_IS_INTERACTION(len)       ((len) & CANIO_INTERACTION)
#define CANIO_IS_FD(len)                ((len) & CANIO_FD)
#define CANIO_IS_FD_ERR_PASSIVE(len)    (((len) & (CANIO_FD | CANIO_ERR_PASSIVE)) == \
                                                  (CANIO_FD | CANIO_ERR_PASSIVE))

#define CANIO_LEN_TO_DATASIZE(len)           __canLenToDataSize((len))

static __inline__ uint8_t __rotl8(uint8_t value, uint8_t shift)
{
    return (value << shift) | (value >> (8 - shift));
}

static __inline__ uint8_t __canLenToDataSize(uint8_t len) {
    static const uint8_t ucLenCan[] = {
        0, 0, 1, 1,  2,  2, 3,  3, 4,  4, 5,  5, 6,  6, 7,  7,
        8, 8, 8, 12, 8, 16, 8, 20, 8, 24, 8, 32, 8, 48, 8, 64,
        0, 0, 0, 1,  0,  2, 0,  3, 0,  4, 0,  5, 0,  6, 0,  7,
        0, 8, 0, 12, 0, 16, 0, 20, 0, 24, 0, 32, 0, 48, 0, 64
    };

    return(ucLenCan[__rotl8(len, 1) & 0x3F]);
}

#define CANIO_MAX_TX_QUEUESIZE          0x4000
#define CANIO_MAX_RX_QUEUESIZE          0x4000

#define CANIO_MAX_TX_TIMEOUT            0xFFFF
#define CANIO_MAX_RX_TIMEOUT            0xFFFF

/*
 * Bits for baudrate configuration (must be in sync with ntcan.h)
 *
 * The MSB of the baudrate constant is reserved to indicate the
 * meaning of the lower 24 bits. The two macros CANIO_BAUD_MODE()
 * and CANIO_BAUD_ARG() can be used to mask either the mode bits
 * or the argument.
 *
 * In addition there is a special constant to indicate an unconfigured
 * CAN controller.
 */
#define CANIO_BAUD_NOT_SET        0x7FFFFFFF
#define CANIO_USER_BAUD           0x80000000
#define CANIO_LISTEN_ONLY_MODE    0x40000000
#define CANIO_USER_BAUD_NUM       0x20000000
#define CANIO_AUTOBAUD            0x00FFFFFE
#define CANIO_AUTOBAUD_OLD        0x7FFFFFFE  /* Old autobaud constant used */
                                              /* in drivers prior V3.8.3    */
#define CANIO_NO_IMPLICIT_CLK_DIV 0x00000800

#define CANIO_BAUD_ARG(baud)      ((baud) & 0x00FFFFFF)
#define CANIO_BAUD_MODE(baud)     ((baud) & 0xFF000000)

/* Defines to decode CAN controller type, as delivered by canStatus(), etc. */
#define CANIO_CANCTL_SJA1000      0x00
#define CANIO_CANCTL_I82527       0x01
#define CANIO_CANCTL_FUJI         0x02
#define CANIO_CANCTL_LPC2292      0x03
#define CANIO_CANCTL_LPC17XX      CANIO_CANCTL_LPC2292
#define CANIO_CANCTL_MSCAN        0x04
#define CANIO_CANCTL_ATSAM        0x05
#define CANIO_CANCTL_ESDACC       0x06
#define CANIO_CANCTL_STM32        0x07
#define CANIO_CANCTL_CC770        0x08
#define CANIO_CANCTL_SPEAR        0x09
#define CANIO_CANCTL_FLEXCAN      0x0A
#define CANIO_CANCTL_SITARA       0x0B
#define CANIO_CANCTL_MCP2515      0x0C
#define CANIO_CANCTL_MCAN         0x0D

/* Defines for CAN transceiver types */
#define CANIO_TRX_PCA82C251      0x00  /* NXP PCA82C251                      */
#define CANIO_TRX_SN65HVD251     0x01  /* TI SN65HVD251                      */
#define CANIO_TRX_SN65HVD265     0x02  /* TI SN65HVD265                      */

/*********************************************************************
 * IO-controls                                                       *
 *********************************************************************/
/*
 * Define own private IOCTL for "candev"-driver
 */
#define IOCTL_CAN_SET_ID          1
#define IOCTL_CAN_SET_BAUD        2
#define IOCTL_CAN_SET_MODE        3
#define IOCTL_CAN_SET_TIMEOUT     4
#define IOCTL_CAN_SEND            5
#define IOCTL_CAN_TAKE            7
#define IOCTL_CAN_UPDATE          8
#define IOCTL_CAN_SET_QUEUESIZE   9  /* Also used to distinguish old and new driver */
#define IOCTL_CAN_TX_BUFFER       10
#define IOCTL_CAN_RX_BUFFER       11
#define IOCTL_CAN_DEBUG           12
#define IOCTL_CAN_GET_BAUD        17
#define IOCTL_CAN_SEND_DEVNET     18
#define IOCTL_CAN_FLUSH_RX_FIFO   19
#define IOCTL_CAN_GET_RX_MESSAGES 20
#define IOCTL_CAN_GET_RX_TIMEOUT  21
#define IOCTL_CAN_GET_TX_TIMEOUT  22
#define IOCTL_CAN_ABORT           23
#define IOCTL_CAN_GET_SERIAL      24
#define IOCTL_CAN_RAW             250


/*
 * Structures used as args for "candev"-ioctls
 */
/*
 * IOCTL_CAN_SET_BAUD:
 *  arg = CAN_BAUDSET*
 *  Values for baud-argument:
 *  baud = int 0   = 1000.0 kBit/s
 *             1   =  666.6 kBit/s
 *             2   =  500.0 kBit/s
 *             3   =  333.3 kBit/s
 *             4   =  250.0 kBit/s
 *             5   =  166.0 kBit/s
 *             6   =  125.0 kBit/s
 *             7   =  100.0 kBit/s
 *             8   =   66.6 kBit/s
 *             9   =   50.0 kBit/s
 *             10  =   33.3 kBit/s
 *             11  =   20.0 kBit/s
 *             12  =   12.5 kBit/s
 *             13  =   10.0 kBit/s
 *             14  =  800.0 kBit/s
 *             15  = 1600.0 kBit/s
 *             16  =   83.3 kBit/s
 */
typedef struct          /* structure for baud setting */
{
     int32_t net;           /* net number                 */
     int32_t baud;          /* baudrate                   */
} CAN_BAUDSET, *PCAN_BAUDSET;


typedef union {
        uint64_t          tick;
#if ( defined(__BYTE_ORDER) && (__BYTE_ORDER == __BIG_ENDIAN) ) || ( !defined(__BYTE_ORDER) && defined(__BIG_ENDIAN))
        struct {
                uint32_t  HighPart;
                uint32_t  LowPart;
        } h;
#endif /* __BIG_ENDIAN */
#if ( defined(__BYTE_ORDER) && (__BYTE_ORDER == __LITTLE_ENDIAN) ) || ( !defined(__BYTE_ORDER) && defined(__LITTLE_ENDIAN))
        struct {
                uint32_t  LowPart;
                uint32_t  HighPart;
        } h;
#endif /* __LITTLE_ENDIAN */
} CAN_TIMESTAMP, CAN_TIMESTAMP_FREQ;


/*
 * arg = CAN_MSG*
 */
typedef struct {              /* structure for action-routines */
        int32_t  id;          /* id of can-frame or event-id   */
        uint8_t  len;         /* count of data-bytes (0...8)   */
        uint8_t  msg_lost;    /* message-lost-counter          */
        uint8_t  reserved[2]; /* reserved for future           */
        union {               /* data-buffer                   */
                int8_t   c[8];
                int16_t  s[4];
                int32_t  l[2];
                int64_t  ll[1];
        } buf;
} CAN_MSG, *PCAN_MSG;

typedef struct {
        int32_t       id;             /* can-id                                   */
        uint8_t       len;            /* length of message: 0-8                   */
        uint8_t       msg_lost;       /* count of lost rx-messages                */
        uint8_t       reserved[2];    /* reserved                                 */
        union {               /* data-buffer                   */
                int8_t   c[8];
                int16_t  s[4];
                int32_t  l[2];
                int64_t  ll[1];
        } buf;
        CAN_TIMESTAMP timestamp;      /* time stamp of this message               */
} CAN_MSG_T, *PCAN_MSG_T;

typedef struct {
        int32_t       id;             /* can-id                                   */
        uint8_t       len;            /* length of message: 0-8                   */
        uint8_t       msg_lost;       /* count of lost rx-messages                */
        uint8_t       reserved[2];    /* reserved                                 */
        union {               /* data-buffer                   */
                int8_t   c[64];
                int16_t  s[32];
                int32_t  l[16];
                int64_t  ll[8];
        } buf;
        CAN_TIMESTAMP timestamp;      /* time stamp of this message               */
} CAN_MSG_X, *PCAN_MSG_X;

typedef struct _CAN_SCHED
{
        int32_t  id;
        int32_t  flags;
        uint64_t time_start;
        uint64_t time_interval;
        uint32_t countStart; /* Start value for counting */
        uint32_t countStop;  /* Stop value for counting. After reaching this
                                value, the counter is loaded with the countStart value. */
} CAN_SCHED;


/*
 * IOCTL_CAN_SET_QUEUESIZE:
 * arg = CAN_QUEUE*
 */
typedef struct {
        int32_t  tx_size; /* queuesize for tx-direction */
        int32_t  rx_size; /* queuesize for rx-direction */
} CAN_QUEUE, *PCAN_QUEUE;

#pragma pack(1)
typedef struct {        /* structure for action-routines   */
        uint32_t  id;   /* 0x40/0x01/macid/len of dnet req */
        uint16_t  x1;   /* para2                           */
        uint16_t  x2;   /* para2                           */
        union {         /* data-buffer                     */
                int8_t   c[8];
                int16_t  s[4];
                int32_t  l[2];
                int64_t  ll[1];
        } buf;
} DNET_MSG, *PDNET_MSG;
#pragma pack()

/*
 * Define own private IOCTL for "esdcan"-driver
 * AB: To esd developers:
 *     Don't forget to add the "_32-defines" (for 64-platforms)
 *     and additions to esdcan_unregister_ioctl32() and
 *     esdcan_register_ioctl32() in esdcan.c, when adding new IOCTLs.
 */
#define IOCTL_BASE  'C'

#define IOCTL_ESDCAN_CREATE                    _IOW  (IOCTL_BASE,  0, IOCTL_ARG*)
#define IOCTL_ESDCAN_ID_RANGE_ADD              _IOW  (IOCTL_BASE,  1, IOCTL_ARG*)
#define IOCTL_ESDCAN_ID_RANGE_DEL              _IOW  (IOCTL_BASE,  2, IOCTL_ARG*)
#define IOCTL_ESDCAN_SET_BAUD                  _IOW  (IOCTL_BASE,  3, IOCTL_ARG*)
#define IOCTL_ESDCAN_SET_MODE                  _IOW  (IOCTL_BASE,  4, IOCTL_ARG*) /* AB: DEPRECATED/REMOVED, not needed in esdcan (candev legacy)*/
#define IOCTL_ESDCAN_SET_TIMEOUT               _IOW  (IOCTL_BASE,  5, IOCTL_ARG*)
#define IOCTL_ESDCAN_SEND                      _IOW  (IOCTL_BASE,  6, IOCTL_ARG*)
#define IOCTL_ESDCAN_TAKE                      _IOR  (IOCTL_BASE,  7, IOCTL_ARG*)
#define IOCTL_ESDCAN_DEBUG                     _IOWR (IOCTL_BASE,  9, IOCTL_ARG*)
#define IOCTL_ESDCAN_GET_BAUD                  _IOR  (IOCTL_BASE, 10, IOCTL_ARG*)
#define IOCTL_ESDCAN_DESTROY                   _IO   (IOCTL_BASE, 11)
#define IOCTL_ESDCAN_DESTROY_DEPRECATED        _IOW  (IOCTL_BASE, 11, unsigned long) /* Compatibility to libs <= 3.0.7 */
#define IOCTL_ESDCAN_TX_ABORT                  _IOW  (IOCTL_BASE, 12, IOCTL_ARG*)
#define IOCTL_ESDCAN_RX_ABORT                  _IOW  (IOCTL_BASE, 13, IOCTL_ARG*)
#define IOCTL_ESDCAN_TX_OBJ_CREATE             _IOW  (IOCTL_BASE, 16, IOCTL_ARG*)
#define IOCTL_ESDCAN_TX_OBJ_UPDATE             _IOW  (IOCTL_BASE, 17, IOCTL_ARG*)
#define IOCTL_ESDCAN_TX_OBJ_AUTOANSWER_OFF     _IOW  (IOCTL_BASE, 18, IOCTL_ARG*)
#define IOCTL_ESDCAN_TX_OBJ_DESTROY            _IOW  (IOCTL_BASE, 19, IOCTL_ARG*)
#define IOCTL_ESDCAN_RX_OBJ                    _IOW  (IOCTL_BASE, 20, IOCTL_ARG*)
#define IOCTL_ESDCAN_READ                      _IOWR (IOCTL_BASE, 21, IOCTL_ARG*)
#define IOCTL_ESDCAN_WRITE                     _IOWR (IOCTL_BASE, 22, IOCTL_ARG*)
#define IOCTL_ESDCAN_UPDATE                    _IOW  (IOCTL_BASE, 23, IOCTL_ARG*)
#define IOCTL_ESDCAN_FLUSH_RX_FIFO             _IO   (IOCTL_BASE, 24)
#define IOCTL_ESDCAN_FLUSH_RX_FIFO_DEPRECATED  _IOW  (IOCTL_BASE, 24, unsigned long) /* Compatibility to libs <= 3.0.7 */
#define IOCTL_ESDCAN_GET_RX_MESSAGES           _IOR  (IOCTL_BASE, 25, IOCTL_ARG*)
#define IOCTL_ESDCAN_GET_RX_TIMEOUT            _IOR  (IOCTL_BASE, 26, IOCTL_ARG*)
#define IOCTL_ESDCAN_GET_TX_TIMEOUT            _IOR  (IOCTL_BASE, 27, IOCTL_ARG*)
#define IOCTL_ESDCAN_GET_TIMESTAMP_FREQ        _IOR  (IOCTL_BASE, 28, IOCTL_ARG*)
#define IOCTL_ESDCAN_GET_TIMESTAMP             _IOR  (IOCTL_BASE, 29, IOCTL_ARG*)
#define IOCTL_ESDCAN_GET_TICKS_FREQ            _IOR  (IOCTL_BASE, 30, IOCTL_ARG*) /* BL: DEPRECATED/REMOVED, use IOCTL_ESDCAN_GET_TIMESTAMP_FREQ */
#define IOCTL_ESDCAN_GET_TICKS                 _IOR  (IOCTL_BASE, 31, IOCTL_ARG*) /* BL: DEPRECATED/REMOVED, use IOCTL_ESDCAN_GET_TIMESTAMP */
#define IOCTL_ESDCAN_SEND_T                    _IOW  (IOCTL_BASE, 32, IOCTL_ARG*)
#define IOCTL_ESDCAN_WRITE_T                   _IOW  (IOCTL_BASE, 33, IOCTL_ARG*)
#define IOCTL_ESDCAN_TAKE_T                    _IOWR (IOCTL_BASE, 34, IOCTL_ARG*)
#define IOCTL_ESDCAN_READ_T                    _IOWR (IOCTL_BASE, 35, IOCTL_ARG*)
#define IOCTL_ESDCAN_PURGE_TX_FIFO             _IO   (IOCTL_BASE, 36)
#define IOCTL_ESDCAN_PURGE_TX_FIFO_DEPRECATED  _IOW  (IOCTL_BASE, 36, unsigned long) /* Compatibility to libs <= 3.0.7 */
#define IOCTL_ESDCAN_TX_OBJ_AUTOANSWER_ON      _IOW  (IOCTL_BASE, 37, IOCTL_ARG*)
#define IOCTL_ESDCAN_SET_RX_TIMEOUT            _IOW  (IOCTL_BASE, 38, IOCTL_ARG*)
#define IOCTL_ESDCAN_SET_TX_TIMEOUT            _IOW  (IOCTL_BASE, 39, IOCTL_ARG*)
#define IOCTL_ESDCAN_GET_FEATURES              _IOR  (IOCTL_BASE, 40, IOCTL_ARG*)
#define IOCTL_ESDCAN_TX_OBJ_SCHEDULE           _IOW  (IOCTL_BASE, 41, IOCTL_ARG*)
#define IOCTL_ESDCAN_TX_OBJ_SCHEDULE_START     _IO   (IOCTL_BASE, 42)
#define IOCTL_ESDCAN_TX_OBJ_SCHEDULE_STOP      _IO   (IOCTL_BASE, 43)
#define IOCTL_ESDCAN_SET_20B_HND_FILTER        _IOW  (IOCTL_BASE, 44, IOCTL_ARG*)
#define IOCTL_ESDCAN_GET_SERIAL                _IOR  (IOCTL_BASE, 45, IOCTL_ARG*)
#define IOCTL_ESDCAN_SET_ALT_RTR_ID            _IO   (IOCTL_BASE, 46)
#define IOCTL_ESDCAN_SET_BUSLOAD_INTERVAL      _IOW  (IOCTL_BASE, 47, IOCTL_ARG*)
#define IOCTL_ESDCAN_GET_BUSLOAD_INTERVAL      _IOR  (IOCTL_BASE, 48, IOCTL_ARG*)
#define IOCTL_ESDCAN_GET_BITRATE_DETAILS       _IOR  (IOCTL_BASE, 49, IOCTL_ARG*)
#define IOCTL_ESDCAN_GET_BUS_STATISTIC         _IOR  (IOCTL_BASE, 50, IOCTL_ARG*)
#define IOCTL_ESDCAN_RESET_BUS_STATISTIC       _IO   (IOCTL_BASE, 51)
#define IOCTL_ESDCAN_GET_ERROR_COUNTER         _IOR  (IOCTL_BASE, 52, IOCTL_ARG*)
#define IOCTL_ESDCAN_SER_REG_READ              _IOWR (IOCTL_BASE, 53, IOCTL_ARG*) /* Special IOCTL, works only on specific hardware, NOT accessible via NTCAN */
#define IOCTL_ESDCAN_SER_REG_WRITE             _IOWR (IOCTL_BASE, 54, IOCTL_ARG*) /* Special IOCTL, works only on specific hardware, NOT accessible via NTCAN */
#define IOCTL_ESDCAN_RESET_CAN_ERROR_CNT       _IO   (IOCTL_BASE, 55)
#define IOCTL_ESDCAN_ID_REGION_ADD             _IOWR (IOCTL_BASE, 56, IOCTL_ARG*)
#define IOCTL_ESDCAN_ID_REGION_DEL             _IOWR (IOCTL_BASE, 57, IOCTL_ARG*)
#define IOCTL_ESDCAN_SET_TX_TS_WIN             _IOW  (IOCTL_BASE, 58, IOCTL_ARG*)
#define IOCTL_ESDCAN_GET_TX_TS_WIN             _IOR  (IOCTL_BASE, 59, IOCTL_ARG*)
#define IOCTL_ESDCAN_SET_TX_TS_TIMEOUT         _IOW  (IOCTL_BASE, 60, IOCTL_ARG*)
#define IOCTL_ESDCAN_GET_TX_TS_TIMEOUT         _IOR  (IOCTL_BASE, 61, IOCTL_ARG*)
#define IOCTL_ESDCAN_SET_HND_FILTER            _IOW  (IOCTL_BASE, 62, IOCTL_ARG*)

#define IOCTL_ESDCAN_SEND_X                    _IOWR (IOCTL_BASE, 72, IOCTL_ARG*)
#define IOCTL_ESDCAN_WRITE_X                   _IOWR (IOCTL_BASE, 73, IOCTL_ARG*)
#define IOCTL_ESDCAN_TAKE_X                    _IOWR (IOCTL_BASE, 74, IOCTL_ARG*)
#define IOCTL_ESDCAN_READ_X                    _IOWR (IOCTL_BASE, 75, IOCTL_ARG*)

#define IOCTL_ESDCAN_SET_BAUD_X                _IOW  (IOCTL_BASE, 76, IOCTL_ARG*)
#define IOCTL_ESDCAN_GET_BAUD_X                _IOWR (IOCTL_BASE, 77, IOCTL_ARG*)

#define IOCTL_ESDCAN_EEI_CREATE                _IOR  (IOCTL_BASE, 81, IOCTL_ARG*)
#define IOCTL_ESDCAN_EEI_DESTROY               _IOW  (IOCTL_BASE, 82, IOCTL_ARG*)
#define IOCTL_ESDCAN_EEI_STATUS                _IOWR (IOCTL_BASE, 83, IOCTL_ARG*)
#define IOCTL_ESDCAN_EEI_CONFIGURE             _IOW  (IOCTL_BASE, 84, IOCTL_ARG*)
#define IOCTL_ESDCAN_EEI_START                 _IOW  (IOCTL_BASE, 85, IOCTL_ARG*)
#define IOCTL_ESDCAN_EEI_STOP                  _IOW  (IOCTL_BASE, 86, IOCTL_ARG*)
#define IOCTL_ESDCAN_EEI_TRIGGER_NOW           _IOW  (IOCTL_BASE, 87, IOCTL_ARG*)

#define IOCTL_ESDCAN_TX_OBJ_UPDATE_T           _IOW  (IOCTL_BASE,101, IOCTL_ARG*)
#define IOCTL_ESDCAN_TX_OBJ_AUTOANSWER_OFF_T   _IOW  (IOCTL_BASE,102, IOCTL_ARG*)
#define IOCTL_ESDCAN_TX_OBJ_AUTOANSWER_ON_T    _IOW  (IOCTL_BASE,103, IOCTL_ARG*)
#define IOCTL_ESDCAN_TX_OBJ_DESTROY_T          _IOW  (IOCTL_BASE,104, IOCTL_ARG*)
#define IOCTL_ESDCAN_TX_OBJ_CREATE_X           _IOW  (IOCTL_BASE,105, IOCTL_ARG*)
#define IOCTL_ESDCAN_TX_OBJ_UPDATE_X           _IOW  (IOCTL_BASE,106, IOCTL_ARG*)
#define IOCTL_ESDCAN_TX_OBJ_AUTOANSWER_OFF_X   _IOW  (IOCTL_BASE,107, IOCTL_ARG*)
#define IOCTL_ESDCAN_TX_OBJ_AUTOANSWER_ON_X    _IOW  (IOCTL_BASE,108, IOCTL_ARG*)
#define IOCTL_ESDCAN_TX_OBJ_DESTROY_X          _IOW  (IOCTL_BASE,109, IOCTL_ARG*)

#define IOCTL_ESDCAN_GET_INFO                  _IOR  (IOCTL_BASE,110, IOCTL_ARG*)


#if PLATFORM_64BIT
#define GEN_IOCTL32(arg) (((arg) & (~IOCSIZE_MASK)) | (0x4 << IOCSIZE_SHIFT)) /* patch pointer size difference in ioctl commands */

#define REGISTER_IOCTL_32(arg, ret) \
        if ( ret |= register_ioctl32_conversion(arg##_32, esdcan_ioctl_32) ) { \
                CAN_PRINT(("esd CAN driver: Problem with registering IOCTL32: 0x%x!\n", arg##_32)); \
        }
#define REGISTER_IOCTL_COMPAT(arg, ret) \
        if ( ret |= register_ioctl32_conversion(arg, NULL) ) { \
                CAN_PRINT(("esd CAN driver: Problem with registering IOCTL32: 0x%x!\n", arg)); \
        }
#define UNREGISTER_IOCTL_32(arg) \
        if ( unregister_ioctl32_conversion(arg##_32) ) { \
                CAN_PRINT(("esd CAN driver: Problem with unregistering IOCTL32: 0x%x!\n", arg##_32)); \
        }
#define UNREGISTER_IOCTL_COMPAT(arg) \
        if ( unregister_ioctl32_conversion(arg) ) { \
                CAN_PRINT(("esd CAN driver: Problem with unregistering IOCTL32: 0x%x!\n", arg)); \
        }

#define IOCTL_ESDCAN_CREATE_32                     GEN_IOCTL32(IOCTL_ESDCAN_CREATE)
#define IOCTL_ESDCAN_ID_RANGE_ADD_32               GEN_IOCTL32(IOCTL_ESDCAN_ID_RANGE_ADD)
#define IOCTL_ESDCAN_ID_RANGE_DEL_32               GEN_IOCTL32(IOCTL_ESDCAN_ID_RANGE_DEL)
#define IOCTL_ESDCAN_SET_BAUD_32                   GEN_IOCTL32(IOCTL_ESDCAN_SET_BAUD)
#define IOCTL_ESDCAN_SET_MODE_32                   GEN_IOCTL32(IOCTL_ESDCAN_SET_MODE)
#define IOCTL_ESDCAN_SET_TIMEOUT_32                GEN_IOCTL32(IOCTL_ESDCAN_SET_TIMEOUT)
#define IOCTL_ESDCAN_SEND_32                       GEN_IOCTL32(IOCTL_ESDCAN_SEND)
#define IOCTL_ESDCAN_TAKE_32                       GEN_IOCTL32(IOCTL_ESDCAN_TAKE)
#define IOCTL_ESDCAN_DEBUG_32                      GEN_IOCTL32(IOCTL_ESDCAN_DEBUG)
#define IOCTL_ESDCAN_GET_BAUD_32                   GEN_IOCTL32(IOCTL_ESDCAN_GET_BAUD)
#define IOCTL_ESDCAN_DESTROY_32                    /* same as IOCTL_ESDCAN_DESTROY */
#define IOCTL_ESDCAN_DESTROY_DEPRECATED_32         GEN_IOCTL32(IOCTL_ESDCAN_DESTROY_DEPRECATED)
#define IOCTL_ESDCAN_TX_ABORT_32                   GEN_IOCTL32(IOCTL_ESDCAN_TX_ABORT)
#define IOCTL_ESDCAN_RX_ABORT_32                   GEN_IOCTL32(IOCTL_ESDCAN_RX_ABORT)
#define IOCTL_ESDCAN_TX_OBJ_CREATE_32              GEN_IOCTL32(IOCTL_ESDCAN_TX_OBJ_CREATE)
#define IOCTL_ESDCAN_TX_OBJ_UPDATE_32              GEN_IOCTL32(IOCTL_ESDCAN_TX_OBJ_UPDATE)
#define IOCTL_ESDCAN_TX_OBJ_AUTOANSWER_OFF_32      GEN_IOCTL32(IOCTL_ESDCAN_TX_OBJ_AUTOANSWER_OFF)
#define IOCTL_ESDCAN_TX_OBJ_DESTROY_32             GEN_IOCTL32(IOCTL_ESDCAN_TX_OBJ_DESTROY)
#define IOCTL_ESDCAN_RX_OBJ_32                     GEN_IOCTL32(IOCTL_ESDCAN_RX_OBJ)
#define IOCTL_ESDCAN_READ_32                       GEN_IOCTL32(IOCTL_ESDCAN_READ)
#define IOCTL_ESDCAN_WRITE_32                      GEN_IOCTL32(IOCTL_ESDCAN_WRITE)
#define IOCTL_ESDCAN_UPDATE_32                     GEN_IOCTL32(IOCTL_ESDCAN_UPDATE)
#define IOCTL_ESDCAN_FLUSH_RX_FIFO_32              /* same as IOCTL_ESDCAN_PURGE_TX_FIFO */
#define IOCTL_ESDCAN_FLUSH_RX_FIFO_DEPRECATED_32   GEN_IOCTL32(IOCTL_ESDCAN_FLUSH_RX_FIFO_DEPRECATED)
#define IOCTL_ESDCAN_GET_RX_MESSAGES_32            GEN_IOCTL32(IOCTL_ESDCAN_GET_RX_MESSAGES)
#define IOCTL_ESDCAN_GET_RX_TIMEOUT_32             GEN_IOCTL32(IOCTL_ESDCAN_GET_RX_TIMEOUT)
#define IOCTL_ESDCAN_GET_TX_TIMEOUT_32             GEN_IOCTL32(IOCTL_ESDCAN_GET_TX_TIMEOUT)
#define IOCTL_ESDCAN_GET_TIMESTAMP_FREQ_32         GEN_IOCTL32(IOCTL_ESDCAN_GET_TIMESTAMP_FREQ)
#define IOCTL_ESDCAN_GET_TIMESTAMP_32              GEN_IOCTL32(IOCTL_ESDCAN_GET_TIMESTAMP)
#define IOCTL_ESDCAN_GET_TICKS_FREQ_32             GEN_IOCTL32(IOCTL_ESDCAN_GET_TICKS_FREQ)
#define IOCTL_ESDCAN_GET_TICKS_32                  GEN_IOCTL32(IOCTL_ESDCAN_GET_TICKS)
#define IOCTL_ESDCAN_SEND_T_32                     GEN_IOCTL32(IOCTL_ESDCAN_SEND_T)
#define IOCTL_ESDCAN_WRITE_T_32                    GEN_IOCTL32(IOCTL_ESDCAN_WRITE_T)
#define IOCTL_ESDCAN_TAKE_T_32                     GEN_IOCTL32(IOCTL_ESDCAN_TAKE_T)
#define IOCTL_ESDCAN_READ_T_32                     GEN_IOCTL32(IOCTL_ESDCAN_READ_T)
#define IOCTL_ESDCAN_PURGE_TX_FIFO_32              /* same as IOCTL_ESDCAN_PURGE_TX_FIFO */
#define IOCTL_ESDCAN_PURGE_TX_FIFO_DEPRECATED_32   GEN_IOCTL32(IOCTL_ESDCAN_PURGE_TX_FIFO_DEPRECATED)
#define IOCTL_ESDCAN_TX_OBJ_AUTOANSWER_ON_32       GEN_IOCTL32(IOCTL_ESDCAN_TX_OBJ_AUTOANSWER_ON)
#define IOCTL_ESDCAN_SET_RX_TIMEOUT_32             GEN_IOCTL32(IOCTL_ESDCAN_SET_RX_TIMEOUT)
#define IOCTL_ESDCAN_SET_TX_TIMEOUT_32             GEN_IOCTL32(IOCTL_ESDCAN_SET_TX_TIMEOUT)
#define IOCTL_ESDCAN_GET_FEATURES_32               GEN_IOCTL32(IOCTL_ESDCAN_GET_FEATURES)
#define IOCTL_ESDCAN_TX_OBJ_SCHEDULE_32            GEN_IOCTL32(IOCTL_ESDCAN_TX_OBJ_SCHEDULE)
#define IOCTL_ESDCAN_TX_OBJ_SCHEDULE_START_32      /* same as IOCTL_ESDCAN_TX_OBJ_SCHEDULE_START */
#define IOCTL_ESDCAN_TX_OBJ_SCHEDULE_STOP_32       /* same as IOCTL_ESDCAN_TX_OBJ_SCHEDULE_STOP */
#define IOCTL_ESDCAN_SET_20B_HND_FILTER_32         GEN_IOCTL32(IOCTL_ESDCAN_SET_20B_HND_FILTER)
#define IOCTL_ESDCAN_GET_SERIAL_32                 GEN_IOCTL32(IOCTL_ESDCAN_GET_SERIAL)
#define IOCTL_ESDCAN_SET_ALT_RTR_ID_32             /* same as IOCTL_ESDCAN_SET_ALT_RTR_ID */
#define IOCTL_ESDCAN_SET_BUSLOAD_INTERVAL_32       GEN_IOCTL32(IOCTL_ESDCAN_SET_BUSLOAD_INTERVAL)
#define IOCTL_ESDCAN_GET_BUSLOAD_INTERVAL_32       GEN_IOCTL32(IOCTL_ESDCAN_GET_BUSLOAD_INTERVAL)
#define IOCTL_ESDCAN_GET_BITRATE_DETAILS_32        GEN_IOCTL32(IOCTL_ESDCAN_GET_BITRATE_DETAILS)
#define IOCTL_ESDCAN_GET_BUS_STATISTIC_32          GEN_IOCTL32(IOCTL_ESDCAN_GET_BUS_STATISTIC)
#define IOCTL_ESDCAN_RESET_BUS_STATISTIC_32        /* same as IOCTL_ESDCAN_RESET_BUS_STATISTIC */
#define IOCTL_ESDCAN_GET_ERROR_COUNTER_32          GEN_IOCTL32(IOCTL_ESDCAN_GET_ERROR_COUNTER)
#define IOCTL_ESDCAN_SER_REG_READ_32               GEN_IOCTL32(IOCTL_ESDCAN_SER_REG_READ)
#define IOCTL_ESDCAN_SER_REG_WRITE_32              GEN_IOCTL32(IOCTL_ESDCAN_SER_REG_WRITE)
#define IOCTL_ESDCAN_RESET_CAN_ERROR_CNT_32        /* same as IOCTL_ESDCAN_RESET_CAN_ERROR_CNT */
#define IOCTL_ESDCAN_ID_REGION_ADD_32              GEN_IOCTL32(IOCTL_ESDCAN_ID_REGION_ADD)
#define IOCTL_ESDCAN_ID_REGION_DEL_32              GEN_IOCTL32(IOCTL_ESDCAN_ID_REGION_DEL)
#define IOCTL_ESDCAN_SET_TX_TS_WIN_32              GEN_IOCTL32(IOCTL_ESDCAN_SET_TX_TS_WIN)
#define IOCTL_ESDCAN_GET_TX_TS_WIN_32              GEN_IOCTL32(IOCTL_ESDCAN_GET_TX_TS_WIN)
#define IOCTL_ESDCAN_SET_TX_TS_TIMEOUT_32          GEN_IOCTL32(IOCTL_ESDCAN_SET_TX_TS_TIMEOUT)
#define IOCTL_ESDCAN_GET_TX_TS_TIMEOUT_32          GEN_IOCTL32(IOCTL_ESDCAN_GET_TX_TS_TIMEOUT)
#define IOCTL_ESDCAN_SET_HND_FILTER_32             GEN_IOCTL32(IOCTL_ESDCAN_SET_HND_FILTER)

#define IOCTL_ESDCAN_SEND_X_32                     GEN_IOCTL32(IOCTL_ESDCAN_SEND_X)
#define IOCTL_ESDCAN_WRITE_X_32                    GEN_IOCTL32(IOCTL_ESDCAN_WRITE_X)
#define IOCTL_ESDCAN_TAKE_X_32                     GEN_IOCTL32(IOCTL_ESDCAN_TAKE_X)
#define IOCTL_ESDCAN_READ_X_32                     GEN_IOCTL32(IOCTL_ESDCAN_READ_X)
#define IOCTL_ESDCAN_SET_BAUD_X_32                 GEN_IOCTL32(IOCTL_ESDCAN_SET_BAUD_X)
#define IOCTL_ESDCAN_GET_BAUD_X_32                 GEN_IOCTL32(IOCTL_ESDCAN_GET_BAUD_X)

#define IOCTL_ESDCAN_EEI_CREATE_32                 GEN_IOCTL32(IOCTL_ESDCAN_EEI_CREATE)
#define IOCTL_ESDCAN_EEI_DESTROY_32                GEN_IOCTL32(IOCTL_ESDCAN_EEI_DESTROY)
#define IOCTL_ESDCAN_EEI_STATUS_32                 GEN_IOCTL32(IOCTL_ESDCAN_EEI_STATUS)
#define IOCTL_ESDCAN_EEI_CONFIGURE_32              GEN_IOCTL32(IOCTL_ESDCAN_EEI_CONFIGURE)
#define IOCTL_ESDCAN_EEI_START_32                  GEN_IOCTL32(IOCTL_ESDCAN_EEI_START)
#define IOCTL_ESDCAN_EEI_STOP_32                   GEN_IOCTL32(IOCTL_ESDCAN_EEI_STOP)
#define IOCTL_ESDCAN_EEI_TRIGGER_NOW_32            GEN_IOCTL32(IOCTL_ESDCAN_EEI_TRIGGER_NOW)

#define IOCTL_ESDCAN_TX_OBJ_UPDATE_T_32            GEN_IOCTL32(IOCTL_ESDCAN_TX_OBJ_UPDATE_T)
#define IOCTL_ESDCAN_TX_OBJ_AUTOANSWER_OFF_T_32    GEN_IOCTL32(IOCTL_ESDCAN_TX_OBJ_AUTOANSWER_OFF_T)
#define IOCTL_ESDCAN_TX_OBJ_AUTOANSWER_ON_T_32     GEN_IOCTL32(IOCTL_ESDCAN_TX_OBJ_AUTOANSWER_ON_T)
#define IOCTL_ESDCAN_TX_OBJ_DESTROY_T_32           GEN_IOCTL32(IOCTL_ESDCAN_TX_OBJ_DESTROY_T)
#define IOCTL_ESDCAN_TX_OBJ_CREATE_X_32            GEN_IOCTL32(IOCTL_ESDCAN_TX_OBJ_CREATE_X)
#define IOCTL_ESDCAN_TX_OBJ_UPDATE_X_32            GEN_IOCTL32(IOCTL_ESDCAN_TX_OBJ_UPDATE_X)
#define IOCTL_ESDCAN_TX_OBJ_AUTOANSWER_OFF_X_32    GEN_IOCTL32(IOCTL_ESDCAN_TX_OBJ_AUTOANSWER_OFF_X)
#define IOCTL_ESDCAN_TX_OBJ_AUTOANSWER_ON_X_32     GEN_IOCTL32(IOCTL_ESDCAN_TX_OBJ_AUTOANSWER_ON_X)
#define IOCTL_ESDCAN_TX_OBJ_DESTROY_X_32           GEN_IOCTL32(IOCTL_ESDCAN_TX_OBJ_DESTROY_X)

#define IOCTL_ESDCAN_GET_INFO_32                   GEN_IOCTL32(IOCTL_ESDCAN_GET_INFO)

#endif

/*
 * Structures used as args for "esdcan"-ioctls
 */
typedef struct {
        void     *buffer;
        uint32_t  size;
} IOCTL_ARG;

#if PLATFORM_64BIT
typedef struct {
        uint32_t  buffer;
        uint32_t  size;
} IOCTL_ARG_32;
#endif

/* argument for CAN_INIT ioctl */
typedef struct {
        uint32_t  mode;
        uint32_t  queue_size_tx;
        uint32_t  queue_size_rx;
} CAN_INIT_ARG;

/* argument for CAN_ID_ADD/DEL ioctl */
typedef struct {
        int32_t  id_start;
        int32_t  id_stop;
} CAN_ID_RANGE_ARG;

typedef struct {
        int32_t  id1st;
        int32_t  cnt;
} CAN_ID_REGION_ARG;

/* argument for IOCTL_ESDCAN_SER_REG_READ/IOCTL_ESDCAN_SER_REG_WRITE ioctl */
typedef struct {
        uint32_t  addr;
        uint32_t  value;
} SERIAL_ARG;


/*
 * Structures used as args for "esdcan"- and "candev"-ioctls
 */
/*
 * IOCTL_CAN_SET_TIMEOUT, IOCTL_ESDCAN_SET_TIMEOUT:
 * arg = CAN_TIMEOUT*
 */
typedef struct {         /* structure for timeout-setting */
        uint32_t txtout; /* tx-timeout in msec            */
        uint32_t rxtout; /* rx-timeout in msec            */
} CAN_TIMEOUT, *PCAN_TIMEOUT, CAN_TIMEOUT_ARG;


/*
 * Flag for usSize member of flash_ex structure to indicate wheter
 * flash data is provided or the provided data is to be programmed.
 */
#define FLASH_DATA_PROVIDE 0x8000

/*
 * IOCTL_CAN_DEBUG, IOCTL_ESDCAN_DEBUG:
 * arg = CAN_DEBUG*
 */
typedef struct {                               /* structure for action-routines */
        uint32_t  password;                    /* password                      */
        uint32_t  cmd;                         /* command to execute            */
        union {
                struct {
                        uint8_t   access;      /* byte/word/long access */
                        uint8_t   bus;         /* bus-nr                */
                        uint8_t   reserved[2];
                        uint32_t  address;
                        union {
                                uint8_t   c;
                                uint16_t  s;
                                uint32_t  l;
                        } data;
                } dm_sm;
                struct {
                        uint16_t  drv_ver;
                        uint16_t  firm_ver;
                        uint16_t  hard_ver;
                        uint16_t  board_stat;
                        int8_t    firm_info[14];
                        uint16_t  features;
                        int8_t    ctrl_type;
                        int8_t    reserved[5];
                } ver;
                struct {
                        uint32_t  sector;
                        uint16_t  offset;
                        uint16_t  len;
                        uint8_t   data[14];
                } flash;
                struct {
                        uint16_t  usSize;
                        uint32_t  ulOffset;
                        uint8_t   data[16];
                } flash_ex;
                struct {
                        int8_t mode;
                } trans_mode;
                struct {
                        uint32_t  uiVerbose;
                } regs;
                struct {
                        uint32_t  uiXor;  /* to be used with esdACC boards */
                } leds;
                struct {
                        uint32_t  uiVerbose;
                } bm_info;
                struct {
                        uint32_t  count;
                        uint32_t  delay;
                } usb_test;
                struct {
                        uint32_t  config;
                        uint32_t  output;
                        uint32_t  input;
                } usb_io_test;
        } p;
} CAN_DEBUG, *PCAN_DEBUG, CAN_DEBUG_ARG;

typedef struct {          /* handle filter mask */
        uint32_t acr;
        uint32_t amr;
        uint32_t idArea;
} CAN_FILTER_MASK, *PCAN_FILTER_MASK;

#define CAN_BITRATE_FLAG_SAM  0x00000001

typedef struct
{
        uint32_t  baud;         /* value configured by user via canSetBaudrate()      */
        uint32_t  valid;        /* validity of all _following_ infos (-1 = invalid, CANIO_SUCCESS, CANIO_NOT_IMPLEMENTED) */
        uint32_t  rate;         /* CAN bitrate in Bit/s                               */
        uint32_t  clock;        /* frequency of CAN controller                        */
        uint8_t   ctrl_type;    /* CANIO_CANCTL_XXX defines                           */
        uint8_t   tq_pre_sp;    /* number of time quantas before samplepoint (TSEG1)  */
        uint8_t   tq_post_sp;   /* number of time quantas past samplepoint (TSEG2)    */
        uint8_t   sjw;          /* syncronization jump width in time quantas (SJW)    */
        uint32_t  error;        /* actual deviation of configured baudrate in %*100   */
        uint32_t  flags;        /* baudrate flags (possibly ctrl. specific, e.g. SAM) */
        uint32_t  reserved1;    /* for future use                                     */
        uint32_t  reserved2;    /* for future use                                     */
        uint32_t  baud_req;     /* requested bitrate, while request pending, else -1 (RESERVED IN NTCAN) */
} CAN_BITRATE;


#define CAN_BAUDRATE_MODE_OFF     0
#define CAN_BAUDRATE_MODE_INDEX   1
#define CAN_BAUDRATE_MODE_BTR     2
#define CAN_BAUDRATE_MODE_NUM     3
#define CAN_BAUDRATE_MODE_OLDBTR  4

typedef struct {
        uint32_t mode;                         /* One of CAN_BAUDRATE_MODE_XXX constants   */
        union {
                uint32_t idx;                  /* esd bitrate table index                  */
                struct {
                        uint32_t brp;          /* Bitrate pre-scaler                       */
                        uint32_t tseg1;        /* TSEG1 register                           */
                        uint32_t tseg2;        /* TSEG2 register                           */
                        uint32_t sjw;          /* SJW register                             */
                } btr;
                uint32_t rate;                 /* numerical bitrate                        */
                uint32_t oldbtr;               /* old bitrate timing register (btr) format */
        } m;
} CAN_BAUDRATE_CFG;

#define CAN_BAUDRATE_CONTROL_STM    0x10000000 /* Self test mode               */
#define CAN_BAUDRATE_CONTROL_LOM    0x40000000 /* Listen only mode             */

typedef struct {
        uint32_t           control;            /* Combination of CAN_BAUDRATE_CONTROL_XXX constants */
        CAN_BAUDRATE_CFG   arbitration;        /* Bitrate in arbitration phase */
        CAN_BAUDRATE_CFG   data;               /* Bitrate in data phase        */
        uint32_t           reserved;
} CAN_BAUDRATE_X;


typedef struct _CAN_FRAME_COUNT {
        uint32_t  std_data;     /* # of std CAN messages    */
        uint32_t  std_rtr;      /* # of std RTR requests    */
        uint32_t  ext_data;     /* # of ext CAN messages    */
        uint32_t  ext_rtr;      /* # of ext RTR requests    */
} CAN_FRAME_COUNT;

typedef struct _CAN_BUS_STAT {
        /* keep this struct in sync with NTCAN_BUS_STATISTIC */
        uint64_t        timestamp;          /* Timestamp                     */
        CAN_FRAME_COUNT rcv_count;          /* # of received frames          */
        CAN_FRAME_COUNT xmit_count;         /* # of transmitted frames       */
        uint32_t        ctrl_ovr_count;     /* # of controller overruns      */
        uint32_t        fifo_ovr_count;     /* # of FIFO overruns            */
        uint32_t        err_frames_count;   /* # of error frames             */
        uint32_t        rcv_byte_count;     /* # of received bytes           */
        uint32_t        xmit_byte_count;    /* # of transmitted bytes        */
        uint32_t        abort_frames_count; /* # of aborted frames           */
        uint32_t        rcv_count_fd;       /* # of received FD frames       */
        uint32_t        xmit_count_fd;      /* # of transmitted FD frames    */
        uint64_t        bit_count;          /* # of received bits            */
} CAN_BUS_STAT;

typedef struct _CAN_CTRL_STATE {
        uint8_t         rcv_err_counter;    /* Receive error counter         */
        uint8_t         xmit_err_counter;   /* Transmit error counter        */
        uint8_t         status;             /* CAN controller status         */
        uint8_t         type;               /* CAN controller type           */
} CAN_CTRL_STATE;

typedef struct _EEI_UNIT {
        uint32_t  handle;                     /* Handle for ErrorInjection Unit       */

        uint8_t   mode_trigger;               /* Trigger mode                         */
        uint8_t   mode_trigger_option;        /* Options to trigger                   */
        uint8_t   mode_triggerarm_delay;      /* Enable delayed arming of trigger unit*/
        uint8_t   mode_triggeraction_delay;   /* Enable delayed TX out                */
        uint8_t   mode_repeat;                /* Enable repeat                        */
        uint8_t   mode_trigger_now;           /* Trigger with next TX point           */
        uint8_t   mode_ext_trigger_option;    /* Switch between trigger and sending   */
        uint8_t   mode_send_async;            /* Send without timing synchronization  */
        uint8_t   reserved1[4];

        uint64_t  timestamp_send;             /* Timestamp for Trigger Timestamp      */
        uint32_t  trigger_pattern[5];         /* Trigger for mode Pattern Match       */
        uint32_t  trigger_mask[5];            /* Mask to trigger Pattern              */
        uint8_t   trigger_ecc;                /* ECC for Trigger Field Position       */
        uint8_t   reserved2[3];
        uint32_t  external_trigger_mask;      /* Enable Mask for external Trigger     */
        uint32_t  reserved3[16];

        uint32_t  tx_pattern[5];              /* TX pattern                           */
        uint32_t  tx_pattern_len;             /* Length of TX pattern                 */
        uint32_t  triggerarm_delay;           /* Delay for mode triggerarm delay      */
        uint32_t  triggeraction_delay;        /* Delay for mode trigger delay         */
        uint32_t  number_of_repeat;           /* Number of repeats in mode repeat     */
        uint32_t  reserved4;
        uint32_t  tx_pattern_recessive[5];    /* Recessive TX pattern (USB400 Addon)  */
        uint32_t  reserved5[9];
} EEI_UNIT;

typedef struct _EEI_STATUS {
        uint32_t      handle;                 /* Handle for ErrorInjection Unit       */
        uint8_t       status;                 /* Status form Unit                     */
        uint8_t       unit_index;             /* Error Injection Unit ID              */
        uint8_t       units_total;            /* Max Error Units in esdacc core       */
        uint8_t       units_free;             /* Free Error Units in esdacc core      */
        CAN_TIMESTAMP trigger_timestamp;      /* Timestamp of trigger time            */
        uint16_t      trigger_cnt;            /* Count of trigger in Repeat mode      */
        uint16_t      reserved0;
        uint32_t      reserved1[27];
} EEI_STATUS;

typedef struct _CAN_INFO {
        uint16_t hardware;                    /* Hardware version                     */
        uint16_t firmware;                    /* Firmware / FPGA version (0 = N/A)    */
        uint16_t driver;                      /* Driver version                       */
        uint16_t dll;                         /* NTCAN library version                */
        uint32_t features;                    /* Device/driver capability flags       */
        uint32_t serial;                      /* Serial # (0 = N/A)                   */
        uint64_t timestamp_freq;              /* Timestamp frequency (in Hz, 1 = N/A) */
        uint32_t ctrl_clock;                  /* Frequency of CAN controller (in Hz)  */
        uint8_t  ctrl_type;                   /* Controller type (CANIO_CANCTL_XXX)   */
        uint8_t  base_net;                    /* Base net number                      */ 
        uint8_t  ports;                       /* Number of physical ports             */
        uint8_t  trx_type;                    /* Transceiver type (CANIO_TRX_XXX)     */
        uint16_t boardstatus;                 /* Hardware status                      */
        uint16_t firmware2;                   /* Second firmware version (0 = N/A)    */
        char     boardid[32];                 /* Board ID string                      */
        char     serial_string[16];           /* Serial # as string                   */
        char     drv_build_info[64];          /* Build info of driver                 */
        char     lib_build_info[64];          /* Build info of library                */
        uint8_t  reserved2[44];               /* Reserved for future use              */
} CAN_INFO;


/*
 *  Mode-flags (prefix CANIO_MODE_) are used by user/application
 *  with IOCTL_ESDCAN_SET_MODE to set the driver into a certain mode.
 *  !!! They have to be defined according to ntcan.h !!!
 */
#define CANIO_MODE_AUTO_ANSWER       0x00000002 /* mode-mask for auto-answer         */
#define CANIO_MODE_NO_RTR            0x00000010 /* mode-mask for ignoring rtr        */
#define CANIO_MODE_NO_DATA           0x00000020 /* mode-mask for ignoring data       */
#define CANIO_MODE_SET_RTR_RCV       0x00000040 /* used to mark active RTR-reception (TODO: ???) */
#define CANIO_MODE_RTR_RCV           0x00000060 /* mode-mask for object mode         (TODO: ???) */
#define CANIO_MODE_NO_INTERACTION    0x00000100 /* Ignore locally send interaction messages */
#define CANIO_MODE_MARK_INTERACTION  0x00000200 /* Mark interaction messages in len field */
#define CANIO_MODE_LOCAL_ECHO        0x00000400 /* Echo sent frames to same handle   */
/* reserved NTCAN_MODE_SPS_CNTRL     0x00008000    Enable firmware SPS             */
#define CANIO_MODE_LOM               0x00010000 /* mode-mask for listening only mode */
#define CANIO_MODE_DEFERRED_TX       0x00020000 /* mode-mask for "scheduled tx"      */
#define CANIO_MODE_FD                0x00040000 /* Enable CAN-FD support             */
#define CANIO_MODE_OBJECT            0x10000000 /* mode-mask for object mode         */
#define CANIO_MODE_OVERLAPPED        0x20000000 /* reserved for windows, only        */
#define CANIO_MODE_EVENTS            0x40000000 /* mode-mask for enabled events (TODO: ???) */

/*
 *  Feature flags
 */
#define FEATURE_FULL_CAN         (1<<0)  /*! Marks Full-CAN-controller, used by nucleus for flat-mode,... */
#define FEATURE_CAN_20B          (1<<1)  /*! CAN 2.OB support                                             */
#define FEATURE_DEVICE_NET       (1<<2)
#define FEATURE_CYCLIC_TX        (1<<3)  /*! On certain boards with special customer firmware only        */
#define FEATURE_TIMESTAMPED_TX   (1<<3)  /*! SAME AS CYCLIC_TX, Timestamped TX support                    */
#define FEATURE_RX_OBJECT_MODE   (1<<4)  /*! DIFFERS FROM MODE FLAG: flat (receive object) mode           */
/* AB: the following feature-flags aren't implemented in candev-driver */
#define FEATURE_TS               (1<<5)  /*! feature flag for hardware timestamping                       */
#define FEATURE_LOM              (1<<6)  /*! feature/mode flag for listening-only-mode                    */
#define FEATURE_SMART_DISCONNECT (1<<7)  /*! feature/mode flag for stay-on-bus-after-last-close           */
#define FEATURE_LOCAL_ECHO       (1<<8)  /*! feature/mode interaction with local echo                     */
#define FEATURE_SMART_ID_FILTER  (1<<9)  /*! Enabling/disabling ID ranges                                 */
#define FEATURE_SCHEDULING       (1<<10) /*! Scheduling feature                                           */
#define FEATURE_DIAGNOSTIC       (1<<11) /*! Bus diagnostic support                                       */
#define FEATURE_ERROR_INJECTION  (1<<12) /*! esdACC error injection support                               */
#define FEATURE_IRIGB            (1<<13) /*! IRIG-B support                                               */
#define FEATURE_PXI              (1<<14) /*! Board supports backplane clock and startrigger for timestamp */
#define FEATURE_CAN_FD           (1<<15) /*! CAN-FD support                                               */ 
#define FEATURE_SELF_TEST        (1<<16) /*! Self-test support                                            */

#define FEATURE_MASK_DOCUMENTED  (FEATURE_FULL_CAN |         \
                                  FEATURE_CAN_20B |          \
                                  FEATURE_DEVICE_NET |       \
                                  FEATURE_TIMESTAMPED_TX |   \
                                  FEATURE_RX_OBJECT_MODE |   \
                                  FEATURE_TS |               \
                                  FEATURE_SMART_DISCONNECT | \
                                  FEATURE_LOM |              \
                                  FEATURE_LOCAL_ECHO |       \
                                  FEATURE_SMART_ID_FILTER |  \
                                  FEATURE_SCHEDULING |       \
                                  FEATURE_DIAGNOSTIC |       \
                                  FEATURE_ERROR_INJECTION |  \
                                  FEATURE_IRIGB |            \
                                  FEATURE_PXI)

/*
 *  Following defines are used in a nodes status cell for
 *    - i20: wrong firmware / hardware
 *    - bus status (ok, warn, err. passive, bus off)
 */
#define CANIO_STATUS_WRONG_FIRMWARE  0x00000001  /*! Wrong firmware version (disables CAN-functionality) */
#define CANIO_STATUS_WRONG_HARDWARE  0x00000002  /*! Wrong hardware version (disables CAN-functionality) */
#define CANIO_STATUS_OK              0x00000000  /*! can Status = OK                                     */
#define CANIO_STATUS_WARN            0x40000000  /*! can status = controller warn                        */
#define CANIO_STATUS_ERRPASSIVE      0x80000000  /*! can status = error passive                          */
#define CANIO_STATUS_BUSOFF          0xC0000000  /*! can status = controller off bus                     */

#define CANIO_STATUS_MASK_BUSSTATE   0xC0000000  /*! use to retrieve bus states                          */

/*
 * Flags for scheduling mode
 */
#define CANIO_SCHED_FLAG_EN          0x00000000  /* ID is enabled          */
#define CANIO_SCHED_FLAG_DIS         0x00000002  /* ID is disabled         */
#define CANIO_SCHED_FLAG_REL         0x00000000  /* Start time is relative */
#define CANIO_SCHED_FLAG_ABS         0x00000001  /* Start time is absolute */
#define CANIO_SCHED_FLAG_INC8        0x00000100  /*  8 Bit incrementer     */
#define CANIO_SCHED_FLAG_INC16       0x00000200  /* 16 Bit incrementer     */
#define CANIO_SCHED_FLAG_INC32       0x00000300  /* 32 Bit incrementer     */
#define CANIO_SCHED_FLAG_DEC8        0x00000400  /*  8 Bit decrementer     */
#define CANIO_SCHED_FLAG_DEC16       0x00000500  /* 16 Bit decrementer     */
#define CANIO_SCHED_FLAG_DEC32       0x00000600  /* 32 Bit decrementer     */
#define CANIO_SCHED_FLAG_OFS0        0x00000000  /* Counter at offset 0    */
#define CANIO_SCHED_FLAG_OFS1        0x00001000  /* Counter at offset 1    */
#define CANIO_SCHED_FLAG_OFS2        0x00002000  /* Counter at offset 2    */
#define CANIO_SCHED_FLAG_OFS3        0x00003000  /* Counter at offset 3    */
#define CANIO_SCHED_FLAG_OFS4        0x00004000  /* Counter at offset 4    */
#define CANIO_SCHED_FLAG_OFS5        0x00005000  /* Counter at offset 5    */
#define CANIO_SCHED_FLAG_OFS6        0x00006000  /* Counter at offset 6    */
#define CANIO_SCHED_FLAG_OFS7        0x00007000  /* Counter at offset 7    */

/* flags for scheduling mode */
#define CANIO_SCHED_FLAG_EN          0x00000000  /* ID is enabled          */
#define CANIO_SCHED_FLAG_DIS         0x00000002  /* ID is disabled         */
#define CANIO_SCHED_FLAG_REL         0x00000000  /* Start time is relative */
#define CANIO_SCHED_FLAG_ABS         0x00000001  /* Start time is absolute */
#define CANIO_SCHED_FLAG_INC8        0x00000100  /*  8 Bit incrementer     */
#define CANIO_SCHED_FLAG_INC16       0x00000200  /* 16 Bit incrementer     */
#define CANIO_SCHED_FLAG_INC32       0x00000300  /* 32 Bit incrementer     */
#define CANIO_SCHED_FLAG_DEC8        0x00000400  /*  8 Bit decrementer     */
#define CANIO_SCHED_FLAG_DEC16       0x00000500  /* 16 Bit decrementer     */
#define CANIO_SCHED_FLAG_DEC32       0x00000600  /* 32 Bit decrementer     */
#define CANIO_SCHED_FLAG_OFS0        0x00000000  /* Counter at offset 0    */
#define CANIO_SCHED_FLAG_OFS1        0x00001000  /* Counter at offset 1    */
#define CANIO_SCHED_FLAG_OFS2        0x00002000  /* Counter at offset 2    */
#define CANIO_SCHED_FLAG_OFS3        0x00003000  /* Counter at offset 3    */
#define CANIO_SCHED_FLAG_OFS4        0x00004000  /* Counter at offset 4    */
#define CANIO_SCHED_FLAG_OFS5        0x00005000  /* Counter at offset 5    */
#define CANIO_SCHED_FLAG_OFS6        0x00006000  /* Counter at offset 6    */
#define CANIO_SCHED_FLAG_OFS7        0x00007000  /* Counter at offset 7    */

/*
 * Debug-command-values
 */
#define CAN_DEBUG_DM                   0   /* dump memory */
#define CAN_DEBUG_SM                   1   /* set memory  */
#define CAN_DEBUG_IP                   2   /* input port  */
#define CAN_DEBUG_OP                   3   /* output port */
#define CAN_DEBUG_VER                  4   /* get version-information */
/*#define CAN_DEBUG_MAP                5   */    /* map memory-space to user-space */
#define CAN_DEBUG_REGISTER            10   /* display register */
/*#define CAN_DEBUG_RXFIFO            11   */    /* display interface rx fifo */

/* #define CAN_DEBUG_CONFIG            20  ?*//* Set/Get/Save config data */

#define CAN_DEBUG_FLASH_ENTER        128   /* enter flash-mode */
#define CAN_DEBUG_FLASH_ERASE        129   /* erase sector */
#define CAN_DEBUG_FLASH_PROG         130   /* flash some bytes */
#define CAN_DEBUG_FLASH_EXIT         131   /* exit flash-mode  */
#define CAN_DEBUG_FLASH_PROG_EX      132   /* flash some bytes */

#define CAN_DEBUG_SET_TRANS          140   /* Set firmware mode to 2.0A or 2.0B */

#define CAN_DEBUG_LEDS               150   /* Toggle LEDs, etc. */
#define CAN_DEBUG_BUSMASTER_INFO     151   /* Print information regarding busmastering */
#define CAN_DEBUG_USB_TEST           152   /* Send generated testframes */
#define CAN_DEBUG_USB_IO_TEST        153   /* IO Config for IRIG Trigger Lines */

/* debug-access-values */
#define CAN_ACCESS_BYTE 0
#define CAN_ACCESS_WORD 1
#define CAN_ACCESS_LONG 2

/*
 * can-error-codes
 */
#define CANIO_ERRNO_BASE             0x00000100  /* base for esd-codes */

#define CANIO_SUCCESS                0

#define CANIO_RX_TIMEOUT             (CANIO_ERRNO_BASE+1)
#define CANIO_TX_TIMEOUT             (CANIO_ERRNO_BASE+2)
/* #define CANIO_UNUSED              (CANIO_ERRNO_BASE+3) */
#define CANIO_TX_ERROR               (CANIO_ERRNO_BASE+4)
#define CANIO_CONTR_OFF_BUS          (CANIO_ERRNO_BASE+5)
#define CANIO_CONTR_BUSY             (CANIO_ERRNO_BASE+6)
#define CANIO_CONTR_WARN             (CANIO_ERRNO_BASE+7)
#define CANIO_OLDDATA                (CANIO_ERRNO_BASE+8)
#define CANIO_NO_ID_ENABLED          (CANIO_ERRNO_BASE+9)
#define CANIO_ID_ALREADY_ENABLED     (CANIO_ERRNO_BASE+10)
#define CANIO_ID_NOT_ENABLED         (CANIO_ERRNO_BASE+11)
/* #define CANIO_UNUSED              (CANIO_ERRNO_BASE+12) */
#define CANIO_INVALID_FIRMWARE       (CANIO_ERRNO_BASE+13)
#define CANIO_MESSAGE_LOST           (CANIO_ERRNO_BASE+14)
#define CANIO_INVALID_HARDWARE       (CANIO_ERRNO_BASE+15)
#define CANIO_PENDING_WRITE          (CANIO_ERRNO_BASE+16)
#define CANIO_PENDING_READ           (CANIO_ERRNO_BASE+17)
#define CANIO_INVALID_DRIVER         (CANIO_ERRNO_BASE+18)

#define CANIO_INVALID_PARAMETER      EINVAL
#define CANIO_INVALID_HANDLE         EBADFD
/* #define CANIO_IO_INCOMPLETE       not reasonable under Linux */
/* #define CANIO_IO_PENDING          not reasonable under Linux */
#define CANIO_NET_NOT_FOUND          ENODEV
#define CANIO_INSUFFICIENT_RESOURCES ENOMEM

#define CANIO_OPERATION_ABORTED      EINTR
#define CANIO_WRONG_DEVICE_STATE     (CANIO_ERRNO_BASE+19)
#define CANIO_HANDLE_FORCED_CLOSE    (CANIO_ERRNO_BASE+20)
#define CANIO_NOT_IMPLEMENTED        ENOSYS
#define CANIO_NOT_SUPPORTED          (CANIO_ERRNO_BASE+21)
#define CANIO_CONTR_ERR_PASSIVE      (CANIO_ERRNO_BASE+22)
#define CANIO_ERROR_NO_BAUDRATE      (CANIO_ERRNO_BASE+23)
#define CANIO_ERROR_LOM              (CANIO_ERRNO_BASE+24) /* Der MT Gedenk Fehlercode... */

/* Errors used in esdcan, but not formerly present in canio.h
 * Used in situations, where errno-systems return the respecting errno */
#define CANIO_EIO                    EIO
#define CANIO_EFAULT                 EFAULT
#define CANIO_EBUSY                  EBUSY
#define CANIO_EAGAIN                 EAGAIN
#define CANIO_ENOENT                 ENOENT
#define CANIO_ERESTART               ERESTART

/* This error should never ever reach user level!!! */
#define CANIO_ERROR                  1  /* todo: Generic Error...not EPERM */

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __CANIO_H__ */
