/*
 * Apollo Sensor FPGA support
 *
 * Copyright (C) 2018 Baidu Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _BASA_REGS_H_
#define	_BASA_REGS_H_

/* Sensor FPGA device IDs */
#define	PCI_VENDOR_ID_BAIDU		0x1D22
#define	PCI_DEVICE_ID_MOONROVER		0x2083

#define	ZYNQ_DRV_NAME			"basa"
#define	ZYNQ_MOD_VER			"3.0.0.3"

/*
 * The system clock accuracy is within 30 ppm.
 * That is, the system time can drift up to 30 microseconds per second.
 */
#define	ZYNQ_SYS_TIME_DRIFT		30

#define	ZYNQ_GPS_SYNC_TIME_MAX	(100*24*3600)	/* 100 days */
#define	ZYNQ_GPS_SYNC_TIME_SMOOTH	1	/* 1 second */
#define	ZYNQ_GPS_SYNC_TIME_DEFAULT	10	/* 10 seconds */
#define	ZYNQ_GPS_SMOOTH_STEP_DEFAULT	1000	/* 1 ms */
#define	ZYNQ_GPS_SMOOTH_STEP_MIN	16	/* 16 us */
#define	ZYNQ_GPS_SMOOTH_STEP_MAX	500000	/* 500 ms */

/*
 * Maximum number of device nodes created.
 */
#define	ZYNQ_MINOR_COUNT		22

/* Video and CAN channel mappings */
#define	ZYNQ_DMA_CHAN_NUM_MR		16

#define	ZYNQ_VIDEO_MAP_MR		0x001F
#define	ZYNQ_CAN_MAP_MR			0x3C00

#define	ZYNQ_INT_PER_CHAN		1
#define	ZYNQ_INT_PER_CARD		16

#define	ZYNQ_I2C_BUS_NUM		4

#define	GET_BITS(bits, reg_val)	\
	(((reg_val) >> bits ## _OFFSET) & bits ## _MASK)
#define	SET_BITS(bits, reg_val, data)	\
	(((reg_val) & ~(bits ## _MASK << bits ## _OFFSET)) | \
	(((data) & bits ## _MASK) << bits ## _OFFSET))

/*
 * Global registers: BAR0 0x0000 - 0x01FF
 */
#define	ZYNQ_G_VERSION			0x000
#define	ZYNQ_VER_ARM_MAJOR(x)		(((x) >> 24) & 0xF)
#define	ZYNQ_VER_ARM_MINOR(x)		(((x) >> 16) & 0xF)
#define	ZYNQ_VER_PL_MAJOR(x)		(((x) >> 8) & 0xF)
#define	ZYNQ_VER_PL_MINOR(x)		((x) & 0xF)
#define	ZYNQ_FW_IMAGE_TYPE(x)		(((x) >> 12) & 0xF)

#define	ZYNQ_G_SCRATCH			0x004

#define	ZYNQ_G_STATUS			0x008
#define	ZYNQ_STATUS_BUS_MASTER_DISABLE	(1 << 0)
#define	ZYNQ_STATUS_TRIGGER_ALARM	(1 << 16)
#define	ZYNQ_STATUS_PPS_ALARM		(1 << 17)
#define	ZYNQ_STATUS_GPS_LOCKED		(1 << 18)
#define	ZYNQ_STATUS_PPS_LOCKED		(1 << 19)

#define	ZYNQ_G_CONFIG			0x020
#define	ZYNQ_CONFIG_FW_UPLOAD		(1 << 0)
#define	ZYNQ_CONFIG_FW_UPLOAD_MAGIC	0x00000BCD
/* FLASH update: bit2:1, 00 and 11 are invalid values */
#define	ZYNQ_CONFIG_FW_UPDATE_QSPI	(1 << 1)
#define	ZYNQ_CONFIG_FW_UPDATE_MMC	(1 << 2)
#define	ZYNQ_CONFIG_FW_UPDATE_SPI	(1 << 3)
#define	ZYNQ_CONFIG_GPS_ERR_EN		(1 << 12)
#define	ZYNQ_CONFIG_TRIGGER_ONE		(1 << 16)
#define	ZYNQ_CONFIG_GPS_SW		(1 << 17)
#define	ZYNQ_CONFIG_TRIGGER		(1 << 18)
#define	ZYNQ_CONFIG_TRIGGER_MASK	0x00070000
#define	ZYNQ_CONFIG_PS_TXHP_EN		(1 << 24)
#define	ZYNQ_CONFIG_PS_TXLP_EN		(1 << 25)

#define	ZYNQ_G_RESET			0x024
#define	ZYNQ_GLOBAL_RESET		(1 << 0)
#define	ZYNQ_I2C_RESET			(1 << 4)

#define	ZYNQ_G_INTR_STATUS_TX		0x100
#define	ZYNQ_G_INTR_MASK_TX		0x104
#define	ZYNQ_G_INTR_UNMASK_TX		0x108
#define	ZYNQ_INTR_CH_TX(ch)		(1 << (ch))
#define	ZYNQ_INTR_CH_TX_ERR(ch)		(1 << ((ch) + 16))
#define	ZYNQ_INTR_CH_TX_ALL(ch)		\
		(ZYNQ_INTR_CH_TX(ch) | ZYNQ_INTR_CH_TX_ERR(ch))
#define	ZYNQ_INTR_TX_ALL		0xFFFFFFFF

#define	ZYNQ_G_INTR_STATUS_RX		0x110
#define	ZYNQ_G_INTR_MASK_RX		0x114
#define	ZYNQ_G_INTR_UNMASK_RX		0x118
#define	ZYNQ_INTR_CH_RX(ch)		(1 << (ch))
#define	ZYNQ_INTR_CH_RX_ERR(ch)		(1 << ((ch) + 16))
#define	ZYNQ_INTR_CH_RX_ALL(ch)		\
		(ZYNQ_INTR_CH_RX(ch) | ZYNQ_INTR_CH_RX_ERR(ch))
#define	ZYNQ_INTR_RX_ALL		0xFFFFFFFF

#define	ZYNQ_G_PS_INTR_STATUS		0x120
#define	ZYNQ_G_PS_INTR_MASK		0x124
#define	ZYNQ_PS_INTR_FWUPLOAD_GRANT	(1 << 0)
#define	ZYNQ_PS_INTR_FWUPDATE_DONE	(1 << 1)
#define	ZYNQ_PS_INTR_GPS_PPS_CHG	(1 << 4)
#define	ZYNQ_PS_INTR_GPS_PPS		0x10
#define	ZYNQ_PS_INTR_FW			0x3
#define	ZYNQ_PS_INTR_ALL		0x13

#define	ZYNQ_G_DEBUG_CFG0		0x1D0
#define	ZYNQ_G_DEBUG_CFG1		0x1D4
#define	ZYNQ_G_DEBUG_CFG2		0x1D8
#define	ZYNQ_G_DEBUG_CFG3		0x1DC
#define	ZYNQ_G_DEBUG_STA0		0x1E0
#define	ZYNQ_G_DEBUG_STA1		0x1E4
#define	ZYNQ_G_DEBUG_STA2		0x1E8
#define	ZYNQ_G_DEBUG_STA3		0x1EC
#define	ZYNQ_G_DEBUG_CNT_CFG		0x1F0
#define	ZYNQ_G_DEBUG_CNT0		0x1F4
#define	ZYNQ_G_DEBUG_CNT1		0x1F8
#define	ZYNQ_G_DEBUG_CNT2		0x1FC

/*
 * Per DMA Channel Control and Status Regsters:
 *	BAR0 0x1000 - 0x1FFFF
 *	Channel Offset: 0x100 * N
 */
#define	ZYNQ_CHAN_REG_MIN		0x1000
#define	ZYNQ_CHAN_REG_MAX		0x1FFF
#define	ZYNQ_CHAN_REG_OFFSET		0x100

#define	ZYNQ_CH_TX_HEAD_HI		0x1000
#define	ZYNQ_CH_TX_HEAD_LO		0x1004
#define	ZYNQ_CH_TX_TAIL_HI		0x1008
#define	ZYNQ_CH_TX_TAIL_LO		0x100C
#define	ZYNQ_CH_TX_RING_SZ		0x1010

#define	ZYNQ_CH_DMA_CONTROL		0x1020
#define	ZYNQ_CH_DMA_RD_START		(1 << 0)
#define	ZYNQ_CH_DMA_RD_STOP		(1 << 1)
#define	ZYNQ_CH_DMA_WR_START		(1 << 4)
#define	ZYNQ_CH_DMA_WR_STOP		(1 << 5)

#define	ZYNQ_CH_DMA_STATUS		0x1024
#define	ZYNQ_CH_DMA_RD_BUSY		(1 << 0)
#define	ZYNQ_CH_DMA_WR_BUSY		(1 << 4)
#define	ZYNQ_CH_DMA_RD_BUF_FULL		(1 << 8)
#define	ZYNQ_CH_DMA_WR_STATE_OFFSET	4
#define	ZYNQ_CH_DMA_WR_STATE_MASK	0xF
#define	ZYNQ_CH_DMA_READY		0x5

#define	ZYNQ_CH_RX_PDT_HI		0x1040
#define	ZYNQ_CH_RX_PDT_LO		0x1044
#define	ZYNQ_CH_RX_PDT_SZ		0x1048
#define	ZYNQ_CH_RX_TAIL			0x1050
#define	ZYNQ_CH_RX_HEAD			0x1054
#define	ZYNQ_CH_RX_BUF_FULL		(1 << 0)

#define	ZYNQ_CH_WR_TABLE_CONFIG		0x104C
#define	ZYNQ_CH_WR_LAST_PT_SZ_MASK	0xFFF

#define	ZYNQ_CH_DMA_CONFIG		0x1058
#define	ZYNQ_CH_DMA_RX_EN		(1 << 0)
#define	ZYNQ_CH_DMA_TX_EN		(1 << 2)
#define	ZYNQ_CH_DMA_EN			(ZYNQ_CH_DMA_RX_EN | ZYNQ_CH_DMA_TX_EN)
#define	ZYNQ_CH_DMA_FRAME_BUF_ALIGN	(1 << 4)
#define	ZYNQ_CH_DMA_CAN_HWTS		(1 << 5)
#define	ZYNQ_CH_DMA_FRAME_SZ_OFFSET	8
#define	ZYNQ_CH_DMA_FRAME_SZ_MASK	0xFFFFFF00

#define	ZYNQ_CH_RESET			0x105C
#define	ZYNQ_CH_RESET_EN		(1 << 0)
#define	ZYNQ_CH_DMA_RESET_EN		(1 << 1)

#define	ZYNQ_CH_ERR_STATUS		0x1080
#define	ZYNQ_CH_ERR_CAN_BUSOFF		(1 << 0)
#define	ZYNQ_CH_ERR_CAN_STATERR		(1 << 1)
#define	ZYNQ_CH_ERR_CAN_RX_IPOVERFLOW	(1 << 2)
#define	ZYNQ_CH_ERR_CAN_RX_USROVERFLOW	(1 << 3)
#define	ZYNQ_CH_ERR_CAN_TX_TIMEOUT	(1 << 4)
#define	ZYNQ_CH_ERR_CAN_TX_LPFIFOFULL	(1 << 5)
#define	ZYNQ_CH_ERR_CAN_RX_FIFOFULL	(1 << 6)
#define	ZYNQ_CH_ERR_CAN_TX_HPFIFOFULL	(1 << 7)
#define	ZYNQ_CH_ERR_CAN_CRC_ERR		(1 << 8)
#define	ZYNQ_CH_ERR_CAN_FRAME_ERR	(1 << 9)
#define	ZYNQ_CH_ERR_CAN_STUFF_ERR	(1 << 10)
#define	ZYNQ_CH_ERR_CAN_BIT_ERR		(1 << 11)
#define	ZYNQ_CH_ERR_CAN_ACK_ERR		(1 << 12)
#define	ZYNQ_CH_ERR_CAM_TRIGGER		(1 << 13)
#define	ZYNQ_CH_ERR_CAM_LINK_CHANGE	(1 << 14)
#define	ZYNQ_CH_ERR_DMA_RX_BUF_FULL	(1 << 24)
#define	ZYNQ_CH_ERR_DMA_RX_FIFO_FULL	(1 << 25)

#define	ZYNQ_CH_ERR_MASK_TX		0x1088
#define	ZYNQ_CH_ERR_MASK_TX_DEFAULT	0xFF0001EC

#define	ZYNQ_CH_ERR_MASK_RX		0x108C
#define	ZYNQ_CH_ERR_MASK_RX_DEFAULT	0xFE0010B0

/*
 * Device global config and status regsters:
 *	BAR0 0x2000 - 0x2FFF
 */
#define	ZYNQ_G_CAM_TRIG_CFG		0x2000
#define	ZYNQ_G_CAM_FPS_DRIFT_OFFSET	4
#define	ZYNQ_G_CAM_FPS_DRIFT_MASK	0xF
#define	ZYNQ_G_CAM_DELAY_UPDATE_OFFSET	0
#define	ZYNQ_G_CAM_DELAY_UPDATE_MASK	0xF

/* Needs to read LO first, then HI and Day */
#define	ZYNQ_G_NTP_LO			0x2040
#define	ZYNQ_G_NTP_HI			0x2044
#define	ZYNQ_G_NTP_DATE			0x2048

/* 64 bytes: 16 4-byte read */
#define	ZYNQ_G_GPRMC			0x204C

#define	ZYNQ_G_I2C_CONTROL_0		0x2080
#define	ZYNQ_I2C_ADDR_OFFSET		0
#define	ZYNQ_I2C_ADDR_MASK		0xFF
#define	ZYNQ_I2C_ID_OFFSET		8
#define	ZYNQ_I2C_ID_MASK		0x7F
#define	ZYNQ_I2C_DATA_OFFSET		16
#define ZYNQ_I2C_DATA_MASK		0xFF
#define	ZYNQ_I2C_CMD_READ		(1 << 24)
#define	ZYNQ_I2C_ADDR_16		(1 << 25)
#define	ZYNQ_I2C_CMD_ERR		(1 << 30) /* read only bit */
#define	ZYNQ_I2C_CMD_BUSY		(1 << 31) /* read only bit */

#define	ZYNQ_G_I2C_CONFIG_0		0x2084
#define	ZYNQ_I2C_ADDR_HI_OFFSET		0
#define	ZYNQ_I2C_ADDR_HI_MASK		0xFF

#define	ZYNQ_G_I2C_CONTROL_1		0x2088
#define	ZYNQ_G_I2C_CONFIG_1		0x208C
#define	ZYNQ_G_I2C_CONTROL_2		0x2090
#define	ZYNQ_G_I2C_CONFIG_2		0x2094
#define	ZYNQ_G_I2C_CONTROL_3		0x2098
#define	ZYNQ_G_I2C_CONFIG_3		0x209C

#define	ZYNQ_G_GPS_CONFIG		0x2100
#define	ZYNQ_GPS_ADJ_STEP_OFFSET	0
#define	ZYNQ_GPS_ADJ_STEP_MASK		0xFFFF
#define	ZYNQ_GPS_MAX_TOLERANCE_OFFSET	16
#define	ZYNQ_GPS_MAX_TOLERANCE_MASK	0xFFF
#define	ZYNQ_GPS_LOOPBACK_EN		(1 << 28)
#define	ZYNQ_GPS_DISABLE_SMOOTH		(1 << 30)
#define	ZYNQ_GPS_USE_LOCAL_ONLY		(1 << 31)

#define	ZYNQ_G_GPS_TIME_LO_INIT		0x2104
#define	ZYNQ_GPS_TIME_VALID		(1 << 0)
#define	ZYNQ_GPS_TIME_NSEC_OFFSET	1
#define	ZYNQ_GPS_TIME_NSEC_MASK		0x7F
#define	ZYNQ_GPS_TIME_USEC_OFFSET	8
#define	ZYNQ_GPS_TIME_USEC_MASK		0xFFF
#define	ZYNQ_GPS_TIME_MSEC_OFFSET	20
#define	ZYNQ_GPS_TIME_MSEC_MASK		0xFFF

#define	ZYNQ_G_GPS_TIME_HI_INIT		0x2108
#define	ZYNQ_GPS_TIME_SEC_OFFSET	0
#define	ZYNQ_GPS_TIME_SEC_MASK		0xFF
#define	ZYNQ_GPS_TIME_MIN_OFFSET	8
#define	ZYNQ_GPS_TIME_MIN_MASK		0xFF
#define	ZYNQ_GPS_TIME_HOUR_OFFSET	16
#define	ZYNQ_GPS_TIME_HOUR_MASK		0xFF

#define	ZYNQ_G_GPS_CONFIG_2		0x210C
#define	ZYNQ_GPS_TIME_YEAR_OFFSET	0
#define	ZYNQ_GPS_TIME_YEAR_MASK		0xFF
#define	ZYNQ_GPS_TIME_MON_OFFSET	8
#define	ZYNQ_GPS_TIME_MON_MASK		0xFF
#define	ZYNQ_GPS_TIME_DAY_OFFSET	16
#define	ZYNQ_GPS_TIME_DAY_MASK		0xFF
#define	ZYNQ_GPS_CHECKSUM_CHECK		(1 << 31)

#define	ZYNQ_G_GPS_STATUS		0x2110
#define	ZYNQ_GPS_INIT_SET		(1 << 29)
#define	ZYNQ_GPS_SMOOTH_IN_PROGRESS	(1 << 30)
#define	ZYNQ_GPS_LOCAL_SYNC_DONE	(1 << 31)

#define	ZYNQ_G_GPS_LAST_TIME_LO		0x2118
#define	ZYNQ_G_GPS_LAST_TIME_HI		0x211C
#define	ZYNQ_G_GPS_LAST_TIME_DATE	0x2120

/*
 * Camera Per Channel Registers:
 *	BAR0 0x3000 - 0x3FFF
 *	Channel Offset: 0x100 * N
 */
#define	ZYNQ_CAM_REG_MIN		0x3000
#define	ZYNQ_CAM_REG_MAX		0x3FFF
#define	ZYNQ_CAM_REG_OFFSET		0x100

#define	ZYNQ_CAM_CONFIG			0x3000
#define	ZYNQ_CAM_EN			(1 << 0)
#define	ZYNQ_CAM_TEST_PATTERN_OFFSET	1
#define	ZYNQ_CAM_TEST_PATTERN_MASK	0x3
#define	ZYNQ_CAM_RGB_YUV_SEL		(1 << 3)
#define	ZYNQ_CAM_COLOR_SWAP		(1 << 4)
#define	ZYNQ_CAM_TIMESTAMP_RCV		(1 << 5)
#define	ZYNQ_CAM_SENSOR_RESET		(1 << 6)
#define	ZYNQ_CAM_POWER_DOWN		(1 << 7)
#define	ZYNQ_CAM_FOOTER_OFFSET		8
#define	ZYNQ_CAM_FOOTER_MASK		0xF
#define	ZYNQ_CAM_HEADER_OFFSET		12
#define	ZYNQ_CAM_HEADER_MASK		0xF
#define	ZYNQ_CAM_PIN_SWAP		(1 << 20)

#define	ZYNQ_CAM_TRIGGER		0x3004
#define	ZYNQ_CAM_TRIG_DELAY_OFFSET	0
#define	ZYNQ_CAM_TRIG_DELAY_MASK	0x1FFFF
#define	ZYNQ_CAM_TRIG_POLARITY_LOW	(1 << 20)
#define	ZYNQ_CAM_TRIG_FPS_OFFSET	24
#define	ZYNQ_CAM_TRIG_FPS_MASK		0x1F

#define	ZYNQ_CAM_STATUS			0x3020
#define	ZYNQ_CAM_FPD_UNLOCK		(1 << 0)
#define	ZYNQ_CAM_FPD_BIST_FAIL		(1 << 2)
#define	ZYNQ_CAM_FPD_RESET		(1 << 3)
#define	ZYNQ_CAM_FPD_PWDN		(1 << 4)
#define	ZYNQ_CAM_RGB_TPG_ERR		(1 << 5)
#define	ZYNQ_CAM_YUV_TPG_ERR		(1 << 6)
#define	ZYNQ_CAM_FIFO_FULL		(1 << 7)

/*
 * Xilinx CAN IP Per Channel Registers:
 *	BAR2 0x0000 - 0x0FFF
 *	Channel Offset: 0x200 * N
 */

#define	ZCAN_IP_OFFSET		0x200

/* Software Reset Register */
#define	ZCAN_IP_SRR		0x000
#define	ZCAN_IP_SRR_SRST	(1 << 0) /* Softweare reset bit */
#define	ZCAN_IP_SRR_CEN		(1 << 1) /* CAN enable bit */

/* Mode Select Register */
#define	ZCAN_IP_MSR		0x004
#define	ZCAN_IP_MSR_NORMAL	0x0
#define	ZCAN_IP_MSR_SLEEP	0x1
#define	ZCAN_IP_MSR_LOOPBACK	0x2

/* Baud Rate Prescaler Register */
#define	ZCAN_IP_BRPR		0x008
#define	ZCAN_IP_BRPR_1M		0x000
#define	ZCAN_IP_BRPR_500K	0x001
#define	ZCAN_IP_BRPR_250K	0x003
#define	ZCAN_IP_BRPR_125K	0x007

/* Bit Timing Register */
#define	ZCAN_IP_BTR		0x00C
#define	ZCAN_IP_BTR_DEFAULT	0xA3

/* Error Counter Register */
#define	ZCAN_IP_ECR		0x010

/* Error Status Register */
#define	ZCAN_IP_ESR		0x014

/* Status Register */
#define	ZCAN_IP_SR		0x018

/* Interrupt Status Register */
#define	ZCAN_IP_ISR		0x01C
/* Interrupt Enable Register */
#define	ZCAN_IP_IER		0x020
/* Interrupt Clear Register */
#define	ZCAN_IP_ICR		0x024
#define	ZCAN_IP_INTR_ARBLST	(1 << 0) /* Arbitration Lost */
#define	ZCAN_IP_INTR_TXOK	(1 << 1) /* Tx OK */
#define	ZCAN_IP_INTR_TXFULL	(1 << 2) /* Tx FIFO full */
#define	ZCAN_IP_INTR_TXBFULL	(1 << 3) /* High priority Tx FIFO full */
#define	ZCAN_IP_INTR_RXOK	(1 << 4) /* Rx OK */
#define	ZCAN_IP_INTR_RXUFLW	(1 << 5) /* Rx FIFO underflow */
#define	ZCAN_IP_INTR_RXOFLW	(1 << 6) /* Rx FIFO overflow */
#define	ZCAN_IP_INTR_RXNEMP	(1 << 7) /* Rx FIFO not empty */
#define	ZCAN_IP_INTR_ERROR	(1 << 8) /* error interrupt */
#define	ZCAN_IP_INTR_BUSOFF	(1 << 9) /* Bus Off interrupt */
#define	ZCAN_IP_INTR_SLEEP	(1 << 10) /* sleep interrupt */
#define	ZCAN_IP_INTR_WAKEUP	(1 << 11) /* wakeup interrupt */
#define	ZCAN_IP_INTR_ALL	0x352

/* Acceptance Filtering registers: Filter Mask/Filter ID */
#define	ZCAN_IP_AFR		0x060
#define	ZCAN_IP_AFR_UAF1	(1 << 0) /* use acceptance filter number 1 */
#define	ZCAN_IP_AFR_UAF2	(1 << 1) /* use acceptance filter number 2 */
#define	ZCAN_IP_AFR_UAF3	(1 << 2) /* use acceptance filter number 3 */
#define	ZCAN_IP_AFR_UAF4	(1 << 3) /* use acceptance filter number 4 */
#define	ZCAN_IP_AFR_NONE	0

#define	ZCAN_IP_AFMR_1		0x064
#define	ZCAN_IP_AFIR_1		0x068
#define	ZCAN_IP_AFMR_2		0x06C
#define	ZCAN_IP_AFIR_2		0x070
#define	ZCAN_IP_AFMR_3		0x074
#define	ZCAN_IP_AFIR_3		0x078
#define	ZCAN_IP_AFMR_4		0x07C
#define	ZCAN_IP_AFIR_4		0x080

#define	ZCAN_IP_AF_ERTR		(1 < 0) /* extended msg: Remote Tx Request */
#define ZCAN_IP_AF_EID		0x7FFFE /* extended message ID[17:0] */
#define	ZCAN_IP_AF_IDE		(1 << 19) /* indentifier extension bit */
#define	ZCAN_IP_AF_SRTR		(1 << 20) /* standard msg: Remote Tx Request */
#define	ZCAN_IP_AF_SID		0xFFE00000 /* standard message ID[28:18] */

/* TXFIFO/TXHPB/RXFIFO message storage register */
#define	ZCAN_IP_TXFIFO_ID	0x100
#define	ZCAN_IP_TXFIFO_DLC	0x104
#define	ZCAN_IP_TXFIFO_DW1	0x108
#define	ZCAN_IP_TXFIFO_DW2	0x10C
#define	ZCAN_IP_TXHPB_ID	0x110
#define	ZCAN_IP_TXHPB_DLC	0x114
#define	ZCAN_IP_TXHPB_DW1	0x118
#define	ZCAN_IP_TXHPB_DW2	0x11C
#define	ZCAN_IP_RXFIFO_ID	0x120
#define	ZCAN_IP_RXFIFO_DLC	0x124
#define	ZCAN_IP_RXFIFO_DW1	0x128
#define	ZCAN_IP_RXFIFO_DW2	0x12C

#define	ZCAN_IP_PIO_STATUS	0x130
#define	ZCAN_IP_PIO_RX_READY	(1 << 0)
#define	ZCAN_IP_PIO_TX_FULL	(1 << 1)
#define	ZCAN_IP_PIO_TX_HI_FULL	(1 << 2)

/* Debug registers */
#define	ZCAN_IP_CFG0		0x134
#define	ZCAN_IP_DBG_CNT0	0x180
#define	ZCAN_IP_DBG_CNT1	0x184

#endif /* _BASA_REGS_H_ */
