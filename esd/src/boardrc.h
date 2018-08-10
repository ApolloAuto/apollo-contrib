/************************************************************************
 *
 *  Copyright (c) 1996 - 2016 by electronic system design gmbh
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
/*! \file boardrc.h
    \brief Board specific board API

    This file contains board specific constants for a particular board implementation.
    This file may be empty for some boards.
*/
#ifndef __BOARDRC_H__
#define __BOARDRC_H__

/* Check if <osif.h> is included */
#if !defined(__OSIF_H__)
#error "Header <osif.h> has to be included BEFORE <boardrc.h> !!"
#endif

#ifndef OSIF_KERNEL
#error "This file may be used in the kernel-context, only! Not for application-use!!!"
#endif

/* Valid esdACC version range */
#define ESDACC_VERSION_MIN              0x0035
#define ESDACC_VERSION_MAX              0x1000

#define ESDCAN_DRIVER_NAME              "CAN_PCIe402"

#define BOARD_PCI
#define BOARD_BUSMASTER
#define BOARD_MSI
#undef BOARD_CAN_FD

#define NUC_IRQ_NOSYNC
#define NUC_TX_TIMEOUT_IGNORE_COUNTING
#define NUC_TX_TS
#define BUS_STATISTICS

#undef ESDACC_IRIG_B
#undef ESDACC_PXI
#define ESDACC_ARINC825
#define ESDACC_HW_TIMESTAMP  /* used in windows system layer */
#undef ESDACC_USE_NO_HW_TIMER
#define ESDACC_MEASURED_AUTOBAUD
#undef ESDACC_MEASURED_BUSLOAD

#include <esdaccrc.h>  /* Has to be included BEHIND those config defines above */

/* BL TODO: remove HARDTS defines (still used in VxWorks and Win esdcan layers...) */
#define HARDTS
#define HARDTS_FREQ                     INT64_C(80000000) /* BL TODO: needed? */

/* default driver features - more feature flags will be set after hardware probing */
#define CARD_FEATURES                   (FEATURE_CAN_20B | FEATURE_LOM |         \
                                         FEATURE_TS | FEATURE_SMART_DISCONNECT | \
                                         FEATURE_DIAGNOSTIC)

#define MAX_CARDS                       8
#define NODES_PER_CARD                  4
#define EEI_MAX_UNITS                   1 /* actually zero... */

#define NUC_MAX_BRP_ESDACC              512
#define NUC_MAX_TSEG1                   15
#define NUC_MAX_TSEG2                   7
#define NUC_MAX_SJW                     3

#define NUC_MASK_BRP                    0x000001FF
#define NUC_MASK_TSEG1                  0x0000F000
#define NUC_MASK_TSEG2                  0x00700000
#define NUC_MASK_SJW                    0x06000000
#define NUC_MASK_SAM                    0x08000000

#define PCI_VENDOR_ESD                  0x12FE
#define PCI_SUB_VENDOR_ESD              0x12FE
#define PCI_DEVICE_PCIE402              0x0402
#define PCI_DEVICE_PCIE402_DEV          0x4334  /* DEVELOPMENT IDs */
#define PCI_SUB_SYSTEM_PCIE402_S        0x0401  /* with Altera CGX15 */
#define PCI_SUB_SYSTEM_PCIE402_M        0x0402  /* with Altera CGX22 */
#define PCI_SUB_SYSTEM_PCIE402_L        0x0403  /* with Altera CGX30 */
#define PCI_SUB_SYSTEM_PCIE402_DEV      0x0001  /* DEVELOPMENT IDs */

#define PCI_TYPE0_ADDRESSES             6

#define SPI_FLASH
#define SPI_FLASH_SIZE                  0x200000  /* 2MB */
#define SPI_SECTOR_SIZE                 0x10000   /* 64kB Micron M25P80/M25P16 (while Spansion has 4kB!) */
#define SPI_FLASH_PAGE_SIZE             0x100     /* 256 Byte */

/* esd products */
#define IDX_BOARD                       "C402"

#define ESDCAN_IDS_PCIE402_S {                             \
        PCI_VENDOR_ESD, PCI_DEVICE_PCIE402,                \
        PCI_SUB_VENDOR_ESD, PCI_SUB_SYSTEM_PCIE402_S,      \
        0, 0, 0 }
#define ESDCAN_IDS_PCIE402_M {                             \
        PCI_VENDOR_ESD, PCI_DEVICE_PCIE402,                \
        PCI_SUB_VENDOR_ESD, PCI_SUB_SYSTEM_PCIE402_M,      \
        0, 0, 0 }
#define ESDCAN_IDS_PCIE402_L {                             \
        PCI_VENDOR_ESD, PCI_DEVICE_PCIE402,                \
        PCI_SUB_VENDOR_ESD, PCI_SUB_SYSTEM_PCIE402_L,      \
        0, 0, 0 }

#define ESDCAN_IDS_PCIE402_DEV {                           \
        PCI_VENDOR_ESD, PCI_DEVICE_PCIE402_DEV,            \
        PCI_SUB_VENDOR_ESD, PCI_SUB_SYSTEM_PCIE402_DEV,    \
        0, 0, 0 }

#define ESDCAN_IDS_PCI                  ESDCAN_IDS_PCIE402_S, ESDCAN_IDS_PCIE402_M, ESDCAN_IDS_PCIE402_L, ESDCAN_IDS_PCIE402_DEV

#define C402_BUS_TYPE_PCIE              0x00        /* PCIe version       */
#define C402_BUS_TYPE_PCI               0x01        /* PCI  version       */
#define C402_BUS_TYPE_CPCIS             0x02        /* CPCIserial version */
#define C402_BUS_TYPE_CPCI              0x03        /* CPCI version       */

typedef struct _PCIE_EP                 PCIE_EP;
typedef struct _SPI_CTRL                SPI_CTRL;
typedef struct _SYSTEM_ID               SYSTEM_ID;
typedef struct _FLASH_TAIL              FLASH_TAIL;
typedef struct _FPGA_SYSTEM             FPGA_SYSTEM;


struct _PCIE_EP {
        UINT32          reserved0[16];         /* 0x0000-0x003C */
        volatile UINT32 intStat;               /* 0x0040 */
        UINT32          reserved1[3];          /* 0x0044-0x004C */
        volatile UINT32 intEnable;             /* 0x0050 */
        UINT32          reserved2[3];          /* 0x0054-0x005C */
        UINT32          reserved3[488];        /* 0x0060-0x07FC */
        volatile UINT32 pcie2AvalonMb[8];      /* 0x0800-0x081C */
        UINT32          reserved4[56];         /* 0x0820-0x08FC */
        volatile UINT32 avalon2PcieMb[8];      /* 0x0900-0x091C */
        UINT32          reserved5[56];         /* 0x0920-0x09FC */
        UINT32          reserved6[384];        /* 0x0A00-0x0FFC */
        volatile UINT32 avalon2PcieAT[1024];   /* 0x1000-0x1FFC */
        UINT32          reserved7[1024];       /* 0x2000-0x2FFC, reserved by Altera */
        UINT32          reserved8[1024];       /* 0x3000-0x3FFC, reserved for Avalon access */
};

struct _SPI_CTRL {
        volatile UINT32 rx;                /* 0x0000 */
        volatile UINT32 tx;                /* 0x0004 */
        volatile UINT32 status;            /* 0x0008 */
        volatile UINT32 control;           /* 0x000C */
        volatile UINT32 reserved;          /* 0x0010 */
        volatile UINT32 slave_select;      /* 0x0014 */
};

struct _SYSTEM_ID {
        UINT32          uiId;
        UINT32          uiTime;
};

#include "flashtail.h"

struct _FPGA_SYSTEM {
        ESDACCRC_OVERVIEW_PARTITION_1;
        /* UP TO HERE IDENTICAL FOR ALL ESDACC BOARDS */
        UINT32          reserved3[512];    /* 2kB or 512 longwords per "partition"/"sub unit" */
        UINT32          reserved4[512];    /* 2kB or 512 longwords per "partition"/"sub unit" */
        UINT32          reserved7[512];    /* 2kB or 512 longwords per "partition"/"sub unit" */
        UINT32   reserved9[0x4C00];
        /* IRIG-B module */
        volatile UINT32   irigbImpID;
        volatile UINT32   irigbImpRev;
        volatile UINT32   irigbImpCfg;
        volatile UINT32   irigbImpReserved;
        volatile UINT32   irigbImpTimestampHigh;
        volatile UINT32   irigbImpTimestampLow;
        volatile UINT32   irigbImpFrequency;
        UINT32   reserved10[0x1F9];
        volatile UINT32 irigBStatus;       /* 0x15800: one status byte, 0x15801: one config byte, 0x15802: 16 bit IRIG info value */
        volatile UINT32 irigBInfo1;        /* 0x15804: 2x 16 bit IRIG Info values */
        volatile UINT32 irigBInfo2;        /* 0x15808: 2x 16 bit IRIG Info values */
        volatile UINT32 irigBInfo3;        /* 0x1580C: 2x 16 bit IRIG Info values */
        volatile UINT16 irigBAdc[8];       /* 0x15810: ADC1, 0x15812: ADC2, ... ADC5, 0x1581E: ADC counter */
        UINT32          reserved5[36];
        volatile INT8   irigBFrame[16];    /* 0x158B0 */
        volatile INT8   irigBInfo[64];     /* 0x158C0 */
        volatile INT8   irigBYearMode;     /* 0x16800 */
        volatile INT8   irigBYear;         /* 0x16801 */
        INT8            reserved6[2];      /* 0x16802-0x16803 */
        volatile UINT32 irigBTimeStart;    /* 0x16804 */
        volatile UINT32 irigBSyncMode;     /* 0x16808 */ /* Write 0x0001 to increase tolerance */
};

#define CIF_CARD                                   \
        FPGA_SYSTEM    *pBaseOverview;             \
        SPI_CTRL       *pBaseSpi;                  \
        SYSTEM_ID      *pBaseSystemId;             \
        PCIE_EP        *pBasePcieEP;               \
        INT32           numCores;                  \
        INT32           numCoresInFpga;            \
        UINT8           fpgaType;                  \
        UINT8           busType;                   \
        UINT32          irigBFrame[4];             \
        CHAR8           irigBStrModule[16];        \
        CHAR8           irigBStrFirmware[24];      \
        CHAR8           irigBStrDate[8];           \
        CHAR8           irigBStrTime[8];           \
        VOID           *us_pcimsgram;              \
        UINT32          us_pcimsgram_phy_addr;     \
        OSIF_PCIDEV     pciDev;                    \
        OSIF_PCI_VADDR  pcibuf_virt_addr;          \
        OSIF_PCI_VADDR  pcibuf_virt_addr_alloc;    \
        UINT32          pcibuf_phy_addr;           \
        OSIF_PCI_PADDR  pcibuf_phy_addr_alloc;     \
        OSIF_PCI_MADDR  buffer_map;                \
        UINTPTR         cacheDmaCtxt;              \
        UINT32          flagIrqInit;               \
        BM_STUFF        bm[NODES_PER_CARD+1]; /* +1 for overview module */ \
        OSIF_IRQ_MUTEX  lockIrqUnMask;             \
        INT32           idxCoreIrq;                \
        VOID           *eei_base;                  \
        OSIF_MUTEX      eei_lock;                  \
        UINT32          eei_units_count;           \
        CAN_OCB        *eei_units[EEI_MAX_UNITS];  \
        OSIF_TS_FREQ    timestamp_freq;            \
        CIF_TIMER_BASE  hwTimer;                   \
        UINT32          flagMsi; /* needs BOARD_MSI define */ \
        UINT32          flagFlashSectorNotErased;  \
        UINT32          flagPageAllFF;             \
        UINT32          flashEraseSector;          \
        INT32           iCntPassWdRetry;           \
        CARD_IDENT      ident;

#define STRAPPING_BIT_2ND_CAN           0
#define STRAPPING_BIT_B4                1
#define STRAPPING_BIT_STRAP_0           2
#define STRAPPING_BIT_STRAP_1           3
#define STRAPPING_BIT_STRAP_2           4
#define STRAPPING_BIT_STRAP_3           5
#define STRAPPING_BIT_JP_0              6
#define STRAPPING_BIT_JP_1              7
#define STRAPPING_BIT_JP_2              8
#define STRAPPING_BIT_JP_3              9
#define STRAPPING_BIT_ADDON             10
#define STRAPPING_BIT_ADDON_STRAP_0     11
#define STRAPPING_BIT_ADDON_STRAP_1     12
#define STRAPPING_BIT_ADDON_STRAP_2     13
#define STRAPPING_BIT_ADDON_STRAP_3     14

#define STRAPPING_MASK_2ND_CAN          (1 << STRAPPING_BIT_2ND_CAN)
#define STRAPPING_MASK_B4               (1 << STRAPPING_BIT_B4)
#define STRAPPING_MASK_STRAP_0          (1 << STRAPPING_BIT_STRAP_0)
#define STRAPPING_MASK_STRAP_1          (1 << STRAPPING_BIT_STRAP_1)
#define STRAPPING_MASK_STRAP_2          (1 << STRAPPING_BIT_STRAP_2)
#define STRAPPING_MASK_STRAP_3          (1 << STRAPPING_BIT_STRAP_3)
#define STRAPPING_MASK_STRAPS_0_3       (STRAPPING_BIT_STRAP_0 | STRAPPING_BIT_STRAP_1 | STRAPPING_BIT_STRAP_2 | STRAPPING_BIT_STRAP_3)
#define STRAPPING_MASK_JP_0             (1 << STRAPPING_BIT_JP_0)
#define STRAPPING_MASK_JP_1             (1 << STRAPPING_BIT_JP_1)
#define STRAPPING_MASK_JP_2             (1 << STRAPPING_BIT_JP_2)
#define STRAPPING_MASK_JP_3             (1 << STRAPPING_BIT_JP_3)
#define STRAPPING_MASK_JPS_0_3          (STRAPPING_BIT_JP_0 | STRAPPING_BIT_JP_1 | STRAPPING_BIT_JP_2 | STRAPPING_BIT_JP_3)
#define STRAPPING_MASK_ADDON            (1 << STRAPPING_BIT_ADDON)
#define STRAPPING_MASK_ADDON_STRAP_0    (1 << STRAPPING_BIT_ADDON_STRAP_0)
#define STRAPPING_MASK_ADDON_STRAP_1    (1 << STRAPPING_BIT_ADDON_STRAP_1)
#define STRAPPING_MASK_ADDON_STRAP_2    (1 << STRAPPING_BIT_ADDON_STRAP_2)
#define STRAPPING_MASK_ADDON_STRAP_3    (1 << STRAPPING_BIT_ADDON_STRAP_3)
#define STRAPPING_MASK_ADDON_STRAPS_0_3 (STRAPPING_MASK_ADDON_STRAP_0 | STRAPPING_MASK_ADDON_STRAP_1 | STRAPPING_MASK_ADDON_STRAP_2 | STRAPPING_MASK_ADDON_STRAP_3)

INT32 boardrc_ts_set(VOID *dummy, OSIF_TS *pTs);
INT32 boardrc_tsfreq_get(VOID *dummy, OSIF_TS_FREQ *pFreq);
INT32 boardrc_corefreq_get(VOID *dummy, UINT32 *pFreq);
UINT32 boardrc_fpga_type_get(VOID *pCrd);
INT32 boardrc_timer_set(VOID *dummy, OSIF_TS *pTs);
INT32 boardrc_timer_get(VOID *dummy, OSIF_TS *pTs);

VOID boardrc_spi_sector_erase(SPI_CTRL *pSpiCtrl, UINT32 uiAddr);
VOID boardrc_spi_bulk_erase(SPI_CTRL *pSpiCtrl);
VOID boardrc_spi_page_program(SPI_CTRL *pSpiCtrl, const UINT8 *pArr, INT32 iLen, UINT32 uiAddr);
VOID boardrc_spi_read_data(SPI_CTRL *pSpiCtrl, UINT8 *pArr, INT32 iLen, UINT32 uiAddr);

VOID boardrc_strappings_print(UINT16 uiStrappings);
#define BOARDRC_STRAPPINGS_PRINT(strap) boardrc_strappings_print(strap)

#if defined(OSIF_PNP_OS)
# define CAN_BOARD_DETACH_FINAL(pCrd, targetState) can_board_detach_final(pCrd, targetState)
#else
# define CAN_BOARD_DETACH_FINAL(pCrd)              can_board_detach_final(pCrd)
#endif

#endif
