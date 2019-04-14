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

#ifndef _ZYNQ_API_H_
#define	_ZYNQ_API_H_

#include "bcan_defs.h"

#define	ZYNQ_DEV_NAME_FW	"zynq_fw"
#define	ZYNQ_DEV_NAME_TRIGGER	"zynq_trigger"
#define	ZYNQ_DEV_NAME_GPS	"zynq_gps"
#define	ZYNQ_DEV_NAME_REG	"zynq_reg"
#define	ZYNQ_DEV_NAME_CAN	"zynq_can"
#define	ZYNQ_DEV_NAME_I2C	"zynq_i2c"

/*
 * ioctl argument defintion for CAN send/recv
 */
typedef struct ioc_bcan_msg {
	bcan_msg_t	*ioc_msgs;
	unsigned int	ioc_msg_num;
	unsigned int	ioc_msg_num_done;
	int		ioc_msg_err;
	int		ioc_msg_rx_clear;
} ioc_bcan_msg_t;

/*
 * CAN error and status
 */
typedef struct ioc_bcan_status_err {
	unsigned int	bcan_status;
	unsigned int	bcan_err_status;
	unsigned int	bcan_err_count;
	int		bcan_ioc_err;
} ioc_bcan_status_err_t;

/* ioctl command list */
#define	ZYNQ_IOC_MAGIC	('z' << 12 | 'y' << 8 | 'n' << 4 | 'q')
enum ZYNQ_IOC_GPS_CMD {
	IOC_GPS_GET = 1,
	IOC_GPS_GPRMC_GET,
	IOC_GPS_CMD_MAX
};

enum ZYNQ_IOC_TRIGGER_CMD {
	IOC_TRIGGER_DISABLE = IOC_GPS_CMD_MAX,
	IOC_TRIGGER_ENABLE_GPS,
	IOC_TRIGGER_ENABLE_NOGPS,
	IOC_TRIGGER_ENABLE_ONE_GPS,
	IOC_TRIGGER_ENABLE_ONE_NOGPS,
	IOC_TRIGGER_TIMESTAMP,
	IOC_TRIGGER_STATUS,
	IOC_TRIGGER_STATUS_GPS,
	IOC_TRIGGER_STATUS_PPS,
	IOC_TRIGGER_FPS_SET,
	IOC_TRIGGER_FPS_GET,
	IOC_TRIGGER_CMD_MAX
};

enum ZYNQ_IOC_FW_CMD {
	IOC_FW_IMAGE_UPLOAD_START = IOC_TRIGGER_CMD_MAX,
	IOC_FW_IMAGE_UPLOAD,
	IOC_FW_PL_UPDATE, /* PL FPGA FW image update */
	IOC_FW_PS_UPDATE, /* PS OS image update */
	IOC_FW_GET_VER, /* get the image version */
	IOC_FW_CMD_MAX
};

enum ZYNQ_IOC_CAN_CMD {
	IOC_CAN_TX_TIMEOUT_SET = IOC_FW_CMD_MAX, /* in milli-seconds */
	IOC_CAN_RX_TIMEOUT_SET,	/* in milli-seconds */
	IOC_CAN_DEV_START,
	IOC_CAN_DEV_STOP,
	IOC_CAN_DEV_RESET,
	IOC_CAN_ID_ADD,
	IOC_CAN_ID_DEL,
	IOC_CAN_BAUDRATE_SET,
	IOC_CAN_BAUDRATE_GET,
	IOC_CAN_LOOPBACK_SET,
	IOC_CAN_LOOPBACK_UNSET,
	IOC_CAN_RECV,
	IOC_CAN_SEND,
	IOC_CAN_SEND_HIPRI,
	IOC_CAN_GET_STATUS_ERR,
	IOC_CAN_CMD_MAX
};

enum ZYNQ_IOC_REG_CMD {
	IOC_REG_READ = IOC_CAN_CMD_MAX,
	IOC_REG_WRITE,
	IOC_REG_I2C_READ,
	IOC_REG_I2C_WRITE,
	IOC_REG_GPSPPS_EVENT_WAIT,
	IOC_REG_CMD_MAX
};

enum ZYNQ_IOC_TRIGGER_EXT_CMD {
	IOC_TRIGGER_INIT_USB = IOC_REG_CMD_MAX,
	IOC_TRIGGER_ENABLE_ONE,
	IOC_TRIGGER_DISABLE_ONE,
	IOC_TRIGGER_ENABLE,
	IOC_TRIGGER_DELAY_SET,
	IOC_TRIGGER_DELAY_GET,
	IOC_TRIGGER_DEV_NAME,
	IOC_TRIGGER_EXT_MAX
};

enum ZYNQ_IOC_CAM_CMD {
	IOC_CAM_REG_READ = 100,
	IOC_CAM_REG_WRITE,
	IOC_CAM_FLASH_INIT,
	IOC_CAM_FLASH_FINI,
	IOC_CAM_FLASH_READ,
	IOC_CAM_FLASH_WRITE,
	IOC_CAM_CAPS,
	IOC_CAM_RESET
};

enum ZYNQ_IOC_GPS_EXT_CMD {
	IOC_GPS_FPGA_INIT = 110,
	IOC_GPS_SYNC
};

enum ZYNQ_IOC_FW_EXT_CMD {
	IOC_FW_QSPI_UPDATE = 120, /* QSPI flash image update */
	IOC_FW_MMC_UPDATE,	/* eMMC image update */
	IOC_FW_SPI_UPDATE	/* SPI flash image update */
};

enum ZYNQ_IOC_MISC_CMD {
	IOC_STATS_GET = 200
};

enum zynq_baudrate_val {
	ZYNQ_BAUDRATE_1M,
	ZYNQ_BAUDRATE_500K,
	ZYNQ_BAUDRATE_250K,
	ZYNQ_BAUDRATE_150K,
	ZYNQ_BAUDRATE_NUM
};

/* Misc ioctl cmds */
#define	ZYNQ_IOC_STATS_GET \
		_IOWR(ZYNQ_IOC_MAGIC, IOC_STATS_GET, unsigned long)

/* GPS update ioctl cmds */
#define	ZYNQ_GPS_VAL_SZ			12
#define	ZYNQ_IOC_GPS_GET		\
		_IOR(ZYNQ_IOC_MAGIC, IOC_GPS_GET, unsigned char *)
#define	ZYNQ_GPS_GPRMC_VAL_SZ		68
#define	ZYNQ_IOC_GPS_GPRMC_GET		\
		_IOR(ZYNQ_IOC_MAGIC, IOC_GPS_GPRMC_GET, unsigned char *)
#define	ZYNQ_IOC_GPS_FPGA_INIT		\
		_IO(ZYNQ_IOC_MAGIC, IOC_GPS_FPGA_INIT)
#define	ZYNQ_IOC_GPS_SYNC		\
		_IO(ZYNQ_IOC_MAGIC, IOC_GPS_SYNC)

/* Trigger ioctl cmds */
#define	ZYNQ_IOC_TRIGGER_DISABLE	\
		_IOW(ZYNQ_IOC_MAGIC, IOC_TRIGGER_DISABLE, unsigned long)
#define	ZYNQ_IOC_TRIGGER_ENABLE_GPS	\
		_IOW(ZYNQ_IOC_MAGIC, IOC_TRIGGER_ENABLE_GPS, unsigned long)
#define	ZYNQ_IOC_TRIGGER_ENABLE_NOGPS	\
		_IOW(ZYNQ_IOC_MAGIC, IOC_TRIGGER_ENABLE_NOGPS, unsigned long)
#define	ZYNQ_IOC_TRIGGER_ENABLE_ONE_GPS	\
		_IOW(ZYNQ_IOC_MAGIC, IOC_TRIGGER_ENABLE_ONE_GPS, unsigned long)
#define	ZYNQ_IOC_TRIGGER_ENABLE_ONE_NOGPS	\
		_IOW(ZYNQ_IOC_MAGIC, IOC_TRIGGER_ENABLE_ONE_NOGPS, unsigned long)
#define	ZYNQ_IOC_TRIGGER_TIMESTAMP	\
		_IOW(ZYNQ_IOC_MAGIC, IOC_TRIGGER_TIMESTAMP, unsigned long)
#define	ZYNQ_IOC_TRIGGER_STATUS		\
		_IOR(ZYNQ_IOC_MAGIC, IOC_TRIGGER_STATUS, int *)
#define	ZYNQ_IOC_TRIGGER_STATUS_GPS	\
		_IOR(ZYNQ_IOC_MAGIC, IOC_TRIGGER_STATUS_GPS, int *)
#define	ZYNQ_IOC_TRIGGER_STATUS_PPS	\
		_IOR(ZYNQ_IOC_MAGIC, IOC_TRIGGER_STATUS_PPS, int *)
#define	ZYNQ_IOC_TRIGGER_FPS_SET	\
		_IOW(ZYNQ_IOC_MAGIC, IOC_TRIGGER_FPS_SET, int *)
#define	ZYNQ_IOC_TRIGGER_FPS_GET	\
		_IOW(ZYNQ_IOC_MAGIC, IOC_TRIGGER_FPS_GET, int *)
#define	ZYNQ_IOC_TRIGGER_INIT_USB	\
		_IOW(ZYNQ_IOC_MAGIC, IOC_TRIGGER_INIT_USB, unsigned long)
#define	ZYNQ_IOC_TRIGGER_ENABLE_ONE	\
		_IOW(ZYNQ_IOC_MAGIC, IOC_TRIGGER_ENABLE_ONE, unsigned long)
#define	ZYNQ_IOC_TRIGGER_DISABLE_ONE	\
		_IOW(ZYNQ_IOC_MAGIC, IOC_TRIGGER_DISABLE_ONE, unsigned long)
#define	ZYNQ_IOC_TRIGGER_ENABLE		\
		_IOW(ZYNQ_IOC_MAGIC, IOC_TRIGGER_ENABLE, unsigned long)
#define	ZYNQ_IOC_TRIGGER_DELAY_SET      \
		_IOW(ZYNQ_IOC_MAGIC, IOC_TRIGGER_DELAY_SET, unsigned long)
#define	ZYNQ_IOC_TRIGGER_DELAY_GET      \
		_IOWR(ZYNQ_IOC_MAGIC, IOC_TRIGGER_DELAY_GET, unsigned long)
#define	ZYNQ_IOC_TRIGGER_DEV_NAME	\
		_IOR(ZYNQ_IOC_MAGIC, IOC_TRIGGER_DEV_NAME, char *)

/* Camera register I2C cmds */
#define	ZYNQ_IOC_CAM_REG_READ	\
		_IOWR(ZYNQ_IOC_MAGIC, IOC_CAM_REG_READ, unsigned long)
#define	ZYNQ_IOC_CAM_REG_WRITE	\
		_IOW(ZYNQ_IOC_MAGIC, IOC_CAM_REG_WRITE, unsigned long)
#define	ZYNQ_IOC_CAM_FLASH_INIT \
		_IOW(ZYNQ_IOC_MAGIC, IOC_CAM_FLASH_INIT, unsigned long)
#define	ZYNQ_IOC_CAM_FLASH_FINI \
		_IO(ZYNQ_IOC_MAGIC, IOC_CAM_FLASH_FINI)
#define	ZYNQ_IOC_CAM_FLASH_READ \
		_IOW(ZYNQ_IOC_MAGIC, IOC_CAM_FLASH_READ, unsigned long)
#define	ZYNQ_IOC_CAM_FLASH_WRITE \
		_IOW(ZYNQ_IOC_MAGIC, IOC_CAM_FLASH_WRITE, unsigned long)
#define	ZYNQ_IOC_CAM_CAPS \
		_IOWR(ZYNQ_IOC_MAGIC, IOC_CAM_CAPS, unsigned long)
#define	ZYNQ_IOC_CAM_RESET \
		_IO(ZYNQ_IOC_MAGIC, IOC_CAM_RESET)

#define	ZYNQ_CAM_FW_BLOCK_SIZE		8
#define	ZYNQ_CAM_FW_BLOCK_SIZE_MAX	250

typedef struct zynq_cam_fw {
	unsigned char	*data;
	unsigned int	size;
	unsigned int	address;
} zynq_cam_fw_t;

#define	ZYNQ_TRIGGER_DEV_NUM	4

#define	ZYNQ_USB_FPS_MAX	30
#define	ZYNQ_USB_FPS_DEFAULT	30
#define	ZYNQ_USB_TRIG_NUM	5
#define	ZYNQ_USB_TRIG_TOTAL	ZYNQ_USB_TRIG_NUM * ZYNQ_TRIGGER_DEV_NUM

#define	ZYNQ_FPD_FPS_MAX	20
#define	ZYNQ_FPD_FPS_DEFAULT	10	/* 10Hz default */
#define	ZYNQ_FPD_TRIG_NUM	10
#define	ZYNQ_FPD_TRIG_TOTAL	ZYNQ_FPD_TRIG_NUM * ZYNQ_TRIGGER_DEV_NUM
#define	ZYNQ_FPD_CAM_NAME	"FPD"

#define	ZYNQ_DEV_NAME_LEN	64
#define	ZYNQ_DEV_CODE_NAME_LEN	32
#define	ZYNQ_VDEV_NAME_LEN	56

/*
 * For USB cameras, we assign trigger IDs to designated trigger lines.
 * The user applications and kernel driver should always use the same
 * mappings:
 *  0: LI trigger
 *  1: GH trigger
 *  2: BB trigger
 *  3: LB trigger
 *  4: Test trigger
 * For FPD-link cameras, the trigger id mapping is always fixed:
 *  0: 1st FPD-link
 *  1: 2nd FPD-link
 *  ...
 */
typedef struct zynq_trigger {
	unsigned char	id;		/* Trigger id */
	unsigned char	fps;		/* Frame-Per-Second */
	unsigned char	internal;	/* 1: Internal PPS; 0: GPS PPS */
	unsigned char	enabled;	/* 1: Enabled; 0: Disabled */
	unsigned int	trigger_delay;
	unsigned int	exposure_time;
	/* Video number, e.g. 0 for /dev/video0 */
	int		vnum;
	/* Camera name, e.g. AR023ZWDR(Rev663) */
	char		name[ZYNQ_VDEV_NAME_LEN];
} zynq_trigger_t;

/*
 * Camera trigger delay, currently used by sensor_sync
 */
typedef struct zynq_trigger_delay {
	int vnum;
	unsigned int trigger_delay;
	unsigned int exposure_time;
} zynq_trigger_delay_t;

/* FW update ioctl cmds */
#define	ZYNQ_FW_PADDING		0x00000000
#define	ZYNQ_FW_MSG_SZ		16
typedef struct ioc_zynq_fw_upload {
	/*
	 * image data size must be multiple of 4 as each polling transfer is in
	 * 16-byte, so padding the data if needed.
	 */
	unsigned int	*ioc_zynq_fw_data;
	unsigned int	ioc_zynq_fw_num;
	unsigned int	ioc_zynq_fw_done;
	int		ioc_zynq_fw_err;
} ioc_zynq_fw_upload_t;

#define	ZYNQ_IOC_FW_IMAGE_UPLOAD_START	\
		_IOW(ZYNQ_IOC_MAGIC, IOC_FW_IMAGE_UPLOAD_START, unsigned long)
#define	ZYNQ_IOC_FW_IMAGE_UPLOAD	\
		_IOW(ZYNQ_IOC_MAGIC, IOC_FW_IMAGE_UPLOAD, ioc_zynq_fw_upload_t *)
#define	ZYNQ_IOC_FW_PL_UPDATE		\
		_IOW(ZYNQ_IOC_MAGIC, IOC_FW_PL_UPDATE, unsigned long)
#define	ZYNQ_IOC_FW_PS_UPDATE		\
		_IOW(ZYNQ_IOC_MAGIC, IOC_FW_PS_UPDATE, unsigned long)
#define	ZYNQ_IOC_FW_GET_VER		\
		_IOW(ZYNQ_IOC_MAGIC, IOC_FW_GET_VER, unsigned int *)
#define	ZYNQ_IOC_FW_QSPI_UPDATE		\
		_IOW(ZYNQ_IOC_MAGIC, IOC_FW_QSPI_UPDATE, unsigned long)
#define	ZYNQ_IOC_FW_MMC_UPDATE		\
		_IOW(ZYNQ_IOC_MAGIC, IOC_FW_MMC_UPDATE, unsigned long)
#define	ZYNQ_IOC_FW_SPI_UPDATE		\
		_IOW(ZYNQ_IOC_MAGIC, IOC_FW_SPI_UPDATE, unsigned long)

/* CAN channel ioctl cmds */
#define	ZYNQ_IOC_CAN_TX_TIMEOUT_SET	\
		_IOW(ZYNQ_IOC_MAGIC, IOC_CAN_TX_TIMEOUT_SET, unsigned long)

#define	ZYNQ_IOC_CAN_RX_TIMEOUT_SET	\
		_IOW(ZYNQ_IOC_MAGIC, IOC_CAN_RX_TIMEOUT_SET, unsigned long)

#define	ZYNQ_IOC_CAN_DEV_START	\
		_IOW(ZYNQ_IOC_MAGIC, IOC_CAN_DEV_START, unsigned long)

#define	ZYNQ_IOC_CAN_DEV_STOP	\
		_IOW(ZYNQ_IOC_MAGIC, IOC_CAN_DEV_STOP, unsigned long)

#define	ZYNQ_IOC_CAN_DEV_RESET	\
		_IOW(ZYNQ_IOC_MAGIC, IOC_CAN_DEV_RESET, unsigned long)

#define	ZYNQ_IOC_CAN_ID_ADD	\
		_IOW(ZYNQ_IOC_MAGIC, IOC_CAN_ID_ADD, unsigned long)

#define	ZYNQ_IOC_CAN_ID_DEL	\
		_IOW(ZYNQ_IOC_MAGIC, IOC_CAN_ID_DEL, unsigned long)

#define	ZYNQ_IOC_CAN_BAUDRATE_SET	\
		_IOW(ZYNQ_IOC_MAGIC, IOC_CAN_BAUDRATE_SET, unsigned long)

#define	ZYNQ_IOC_CAN_BAUDRATE_GET	\
		_IOW(ZYNQ_IOC_MAGIC, IOC_CAN_BAUDRATE_GET, unsigned long)

#define	ZYNQ_IOC_CAN_LOOPBACK_SET	\
		_IOW(ZYNQ_IOC_MAGIC, IOC_CAN_LOOPBACK_SET, unsigned long)

#define	ZYNQ_IOC_CAN_LOOPBACK_UNSET	\
		_IOW(ZYNQ_IOC_MAGIC, IOC_CAN_LOOPBACK_UNSET, unsigned long)

#define	ZYNQ_IOC_CAN_RECV		\
		_IOWR(ZYNQ_IOC_MAGIC, IOC_CAN_RECV, ioc_bcan_msg_t *)

#define	ZYNQ_IOC_CAN_SEND		\
		_IOWR(ZYNQ_IOC_MAGIC, IOC_CAN_SEND, ioc_bcan_msg_t *)

#define	ZYNQ_IOC_CAN_SEND_HIPRI		\
		_IOWR(ZYNQ_IOC_MAGIC, IOC_CAN_SEND_HIPRI, ioc_bcan_msg_t *)

#define	ZYNQ_IOC_CAN_GET_STATUS_ERR	\
		_IOR(ZYNQ_IOC_MAGIC, IOC_CAN_GET_STATUS_ERR, \
		ioc_bcan_status_err_t *)

/* register read/write ioctl cmds */
typedef struct ioc_zynq_reg_acc {
	unsigned int	reg_bar;
	unsigned int	reg_offset;
	unsigned int	reg_data;
} ioc_zynq_reg_acc_t;

/* I2C ID definitions */
#define	ZYNQ_I2C_ID_JANUS	0x5c
#define	ZYNQ_I2C_ID_MAX		0x7f /* 7-bit */

typedef struct ioc_zynq_i2c_acc {
	unsigned char	i2c_id; /* 7-bit */
	unsigned char	i2c_addr_hi;
	unsigned char	i2c_addr;
	unsigned char	i2c_data;
	unsigned char	i2c_addr_16;
	unsigned char	i2c_bus;
} ioc_zynq_i2c_acc_t;

typedef struct zynq_cam_acc {
	unsigned short	addr;
	unsigned short	data_sz;
	unsigned int	data;
} zynq_cam_acc_t;

#define	ZYNQ_IOC_REG_READ		\
		_IOR(ZYNQ_IOC_MAGIC, IOC_REG_READ, ioc_zynq_reg_acc_t *)
#define	ZYNQ_IOC_REG_WRITE		\
		_IOW(ZYNQ_IOC_MAGIC, IOC_REG_WRITE, ioc_zynq_reg_acc_t *)
#define	ZYNQ_IOC_REG_I2C_READ		\
		_IOR(ZYNQ_IOC_MAGIC, IOC_REG_I2C_READ, ioc_zynq_i2c_acc_t *)
#define	ZYNQ_IOC_REG_I2C_WRITE		\
		_IOW(ZYNQ_IOC_MAGIC, IOC_REG_I2C_WRITE, ioc_zynq_i2c_acc_t *)
/* wait for GPS/PPS status change event notification */
#define	ZYNQ_IOC_REG_GPSPPS_EVENT_WAIT	\
		_IOW(ZYNQ_IOC_MAGIC, IOC_REG_GPSPPS_EVENT_WAIT, unsigned long)

#define	ZVIDEO_EXT_MARKER		0xFE12
#define	ZVIDEO_EXT_ERR_FRAME_FORMAT	0x01
#define	ZVIDEO_EXT_ERR_SHORT_FRAME	0x02
#define	ZVIDEO_EXT_ERR_LONG_FRAME	0x04
#define	ZVIDEO_EXT_ERR_ALL		0x07
#define	ZVIDEO_EXT_ERR_INVALID		0xFF
#define	ZVIDEO_FRAME_CORRUPTED(ext)	\
	((ext->marker[0] != ZVIDEO_EXT_MARKER) || \
	(ext->marker[1] != ZVIDEO_EXT_MARKER) || \
	(ext->marker[2] != ZVIDEO_EXT_MARKER) || \
	(*(unsigned int *)ext->rsv1) || \
	(*(unsigned int *)&ext->rsv2[0]) || \
	(*(unsigned int *)&ext->rsv2[3]) || \
	(ext->error & ~(unsigned char)ZVIDEO_EXT_ERR_ALL))

typedef struct zynq_video_ext_meta_data {
	unsigned int	trigger_cnt;
	struct {
		unsigned int	usec:20;
		unsigned int	:12;
		unsigned int	sec;
	} time_stamp;
	unsigned char	rsv1[4];
	struct {
		unsigned short	us_cnt:12;
		unsigned short	sec:4;
	} debug_ts;
	unsigned short	marker[3];
	unsigned char	rsv2[7];
	unsigned char	error;
} zynq_video_ext_meta_data_t;

/* Code name */
#define	CAM_CAP_CODENAME_ARGUS		0
#define	CAM_CAP_CODENAME_SHARPVISION	1

/* Trigger mode */
#define	CAM_CAP_TRIGGER_STANDARD	0
#define	CAM_CAP_TRIGGER_DETERMINISTIC	1
#define	CAM_CAP_TRIGGER_SLAVE_STANDARD	2
#define	CAM_CAP_TRIGGER_SLAVE_SHUTTER_SYNC 3

/* Timestamp type */
#define	CAM_CAP_TIMESTAMP_FPGA		0
#define	CAM_CAP_TIMESTAMP_TRIGGER	1
#define	CAM_CAP_TIMESTAMP_FORMATION	2
#define	CAM_CAP_TIMESTAMP_HOST		3

/* Camera feature */
#define	CAM_CAP_FEATURE_FLASH_ADDR_16	0x1	/* 0 for 24b */

/* Interface type */
#define	CAM_CAP_INTERFACE_PARALLEL	0
#define	CAM_CAP_INTERFACE_MIPI		1

/* Camera capabilities */
typedef struct zynq_camera_capabilities {
	char		name[ZYNQ_VDEV_NAME_LEN];
	unsigned short	unique_id;
	union {
		struct {
			unsigned char	position;
			unsigned char	project;
		};
		unsigned short	major_version;
	};
	union {
		struct {
			unsigned char	version:4;
			unsigned char	:2;
			unsigned char	feature:2;
			unsigned char	vendor:5;
			unsigned char	code_name:3;
		};
		unsigned short	minor_version;
	};
	unsigned char	timestamp_type:2;
	unsigned char	trigger_mode:2;
	unsigned char	embedded_data:1;
	unsigned char	interface_type:1;
	unsigned char	pin_swap:1;
	unsigned char	:1;
	unsigned char	link_up;
	unsigned short	frame_len_lines;
	unsigned short	line_len_pck;
	unsigned int	pixel_clock;
} zynq_cam_caps_t;

enum zynq_dev_stats {
	DEV_STATS_INTR = 0,
	DEV_STATS_INTR_INVALID,
	DEV_STATS_GPS_UNLOCK,
	DEV_STATS_NUM
};

enum zynq_chan_stats {
	CHAN_STATS_RX_INTR = 0,
	CHAN_STATS_TX_INTR,
	CHAN_STATS_RX_ERR_INTR,
	CHAN_STATS_TX_ERR_INTR,
	CHAN_STATS_RX_DROP,
	CHAN_STATS_RX,
	CHAN_STATS_TX,
	CHAN_STATS_NUM
};

enum zynq_video_stats {
	VIDEO_STATS_FRAME = 0,
	VIDEO_STATS_RESET,
	VIDEO_STATS_TRIG_PULSE_ERR,
	VIDEO_STATS_LINK_CHANGE,
	VIDEO_STATS_DMA_RX_BUF_FULL,
	VIDEO_STATS_DMA_RX_FIFO_FULL,
	VIDEO_STATS_TRIG_COUNT_MISMATCH,
	VIDEO_STATS_META_COUNT_MISMATCH,
	VIDEO_STATS_FRAME_GAP_ERR,
	VIDEO_STATS_FRAME_FORMAT_ERR,
	VIDEO_STATS_FRAME_SHORT,
	VIDEO_STATS_FRAME_LONG,
	VIDEO_STATS_FRAME_CORRUPT,
	VIDEO_STATS_FRAME_DROP,
	VIDEO_STATS_FRAME_DROP_1,
	VIDEO_STATS_FRAME_DROP_2,
	VIDEO_STATS_FRAME_DROP_M,
	VIDEO_STATS_NO_VIDEO_BUF,
	VIDEO_STATS_NUM
};

enum zynq_can_stats {
	CAN_STATS_PIO_RX = 0,
	CAN_STATS_PIO_TX,
	CAN_STATS_PIO_TX_HI,
	CAN_STATS_USR_RX_WAIT,
	CAN_STATS_USR_RX_WAIT_INT,
	CAN_STATS_USR_RX_TIMEOUT,
	CAN_STATS_USR_RX_PARTIAL,
	CAN_STATS_BUS_OFF,
	CAN_STATS_STATUS_ERR,
	CAN_STATS_RX_IP_FIFO_OVF,
	CAN_STATS_RX_USR_FIFO_OVF,
	CAN_STATS_TX_TIMEOUT,
	CAN_STATS_TX_LP_FIFO_FULL,
	CAN_STATS_RX_USR_FIFO_FULL,
	CAN_STATS_TX_HP_FIFO_FULL,
	CAN_STATS_CRC_ERR,
	CAN_STATS_FRAME_ERR,
	CAN_STATS_STUFF_ERR,
	CAN_STATS_BIT_ERR,
	CAN_STATS_ACK_ERR,
	CAN_STATS_NUM
};

/* channel type definition */
enum zynq_chan_type {
	ZYNQ_CHAN_INVAL,
	ZYNQ_CHAN_CAN,		/* CAN channel */
	ZYNQ_CHAN_VIDEO,	/* Video channel */
	ZYNQ_CHAN_TYPE_NUM
};

#define	ZYNQ_CHAN_MAX			16
#define	ZYNQ_STATS_MAX			32
typedef struct ioc_zynq_stats {
	struct {
		enum zynq_chan_type	type;
		unsigned int		devnum;
		unsigned long		stats[ZYNQ_STATS_MAX];
	} chs[ZYNQ_CHAN_MAX + 1];
} ioc_zynq_stats_t;

/* Fixed embedded data header: 00 0A 00 AA 00 30 00 A5 00 00 00 5A */
#define	EM_HDR_LEN			12
#define	EM_HDR_DWORD0			0xAA000A00
#define	EM_HDR_DWORD1			0xA5003000
#define	EM_HDR_DWORD2			0x5A000000
#define	EM_HDR_MASK			0xFFFF00FF

#define	EM_REG_CHIP_VER			0x3000	/* Common */
#define	EM_REG_FRAME_CNT_HI		0x2000	/* AR0231 only */
#define	EM_REG_EXP_T1_ROW		0x2020	/* AR0231 only */

/* AR0230, single register group from 0x3000 */
#define	EM_REG_OFFSET_BASE_230		0x0004
#define	EM_REG_OFFSET_FRAME_LNS_230	0x0034	/* Register 0x300A */
#define	EM_REG_OFFSET_LINE_PCK_230	0x003C	/* Register 0x300C */
#define	EM_REG_OFFSET_COARSE_INT_230	0x0054	/* Register 0x3012 */
#define	EM_REG_OFFSET_FRAME_CNT_230	0x00F4	/* Register 0x303A */

/* AR0231, register group 0x2000 - 0x2018 */
#define	EM_REG_OFFSET_BASE1_231		0x0004
#define	EM_REG_OFFSET_FRAME_CNT_HI_231	0x000C	/* Register 0x2000 */
#define	EM_REG_OFFSET_FRAME_CNT_LO_231	0x0014	/* Register 0x2002 */
/* AR0231, register group 0x2020 - 0x2050 */
#define	EM_REG_OFFSET_BASE2_231		0x0074
#define	EM_REG_OFFSET_EXP_T1_ROW_231	0x007C	/* Register 0x2020 */
#define EM_REG_OFFSET_EXP_T2_ROW_231    0x0084  /* Register 0x2028 */
#define EM_REG_OFFSET_EXP_T3_ROW_231    0x008C  /* Register 0x2030 */
#define	EM_REG_OFFSET_EXP_T1_CLK_HI_231	0x009C	/* Register 0x2028 */
#define	EM_REG_OFFSET_EXP_T1_CLK_LO_231	0x00A4	/* Register 0x202A */

#define	EM_REG_VAL_MSB(buf, x)		\
	(*((unsigned char *)(buf) + (x) + 1))
#define	EM_REG_VAL_LSB(buf, x)		\
	(*((unsigned char *)(buf) + (x) + 5))
#define	EM_REG_VAL(buf, x)	\
	((EM_REG_VAL_MSB(buf, x) << 8) + EM_REG_VAL_LSB(buf, x))
#define	EM_REG_VAL_230(buf, x)		\
	EM_REG_VAL(buf, EM_REG_OFFSET_ ## x ## _230)
#define	EM_REG_VAL_231(buf, x)		\
	EM_REG_VAL(buf, EM_REG_OFFSET_ ## x ## _231)

#define	EM_DATA_VALID(buf)	\
	((((unsigned int *)(buf))[0] == EM_HDR_DWORD0) && \
	((((unsigned int *)(buf))[1] & EM_HDR_MASK) == \
	(EM_HDR_DWORD1 & EM_HDR_MASK)) && \
	((((unsigned int *)(buf))[2] & EM_HDR_MASK) == \
	(EM_HDR_DWORD2 & EM_HDR_MASK)))

#define	DEFAULT_FRAME_LINES_230		1112
#define	DEFAULT_LINE_PCK_230		2876
#define	DEFAULT_PIXCLK_230		64000000

#define	DEFAULT_FRAME_LINES_231		1238
#define	DEFAULT_LINE_PCK_231		2580
#define	DEFAULT_PIXCLK_231		64000000

#endif	/* _ZYNQ_API_H_ */
