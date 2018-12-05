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

#ifndef _BASA_H_
#define	_BASA_H_

#include <linux/types.h>
#include <linux/pci.h>
#include <linux/kernel.h>
#include <linux/gfp.h>
#include <linux/interrupt.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/version.h>
#include <linux/completion.h>
#include <linux/hrtimer.h>
#include <linux/mutex.h>
#include <asm/uaccess.h>
#include "linux/zynq_api.h"
#include "basa_regs.h"
#include "basa_debug.h"
#include "basa_chan.h"
#include "basa_video.h"
#include "basa_cam_hci.h"
#include "basa_can.h"

#define	ZYNQ_INTR_PROC_TASKLET

#define	ZDEV_VER(zdev)			((zdev)->zdev_version & 0x0FFF0FFF)
#define	ZDEV_PL_VER(zdev)		((zdev)->zdev_version & 0xFFF)
#define	ZDEV_PS_VER(zdev)		(((zdev)->zdev_version >> 16) & 0xFFF)
#define	ZDEV_PL_DEBUG(zdev)		\
	(ZYNQ_FW_IMAGE_TYPE((zdev)->zdev_version) == 0xD)

/* FPGA capabilities */
#define	ZYNQ_HW_CAP_CAN			(1 << 0)
#define	ZYNQ_HW_CAP_GPS			(1 << 1)
#define	ZYNQ_HW_CAP_TRIGGER		(1 << 2)
#define	ZYNQ_HW_CAP_FW			(1 << 3)
#define	ZYNQ_HW_CAP_I2C			(1 << 4)
#define	ZYNQ_HW_CAP_VIDEO		(1 << 5)
#define	ZYNQ_HW_CAP_FPGA_TIME_INIT	(1 << 6)
#define	ZYNQ_HW_CAP_GPS_SMOOTH		(1 << 7)

#define	ZYNQ_CAP(zdev, cap)		\
	((zdev)->zdev_hw_cap & ZYNQ_HW_CAP_ ## cap)

/* Zynq device structure */
typedef struct zynq_dev {
	struct pci_dev		*zdev_pdev;	/* pci access data structure */
	char			zdev_name[ZYNQ_DEV_NAME_LEN];
	char			zdev_code_name[ZYNQ_DEV_CODE_NAME_LEN];
	int			zdev_inst;	/* instance number */
	unsigned int		zdev_version;	/* version number */
	unsigned int		zdev_hw_cap;
	struct cdev		zdev_cdev_trigger; /* cdev for sensor trigger */
	dev_t			zdev_dev_trigger;
	struct cdev		zdev_cdev_fw; /* cdev for zynq firmware update */
	dev_t			zdev_dev_fw;
	unsigned int		zdev_fw_tx_cnt;
	atomic_t		zdev_fw_opened;
	struct cdev		zdev_cdev_gps; /* cdev for GPS time */
	dev_t			zdev_dev_gps;
	struct cdev		zdev_cdev_reg; /* cdev for register access */
	dev_t			zdev_dev_reg;
	struct cdev		zdev_cdev_i2c; /* cdev for i2c access */
	dev_t			zdev_dev_i2c;
	struct kobject		zdev_kobj;
	spinlock_t		zdev_lock;	/* device-wide lock */
	/* use a separated I2C access lock as the operation is very slow */
	spinlock_t		zdev_i2c_lock;
	u16			zdev_vid;
	u16			zdev_did;
	u8 __iomem		*zdev_bar0;	/* mapped kernel VA for BAR0 */
	u8 __iomem		*zdev_bar2;	/* mapped kernel VA for BAR2 */
	unsigned int		zdev_bar0_len;
	unsigned int		zdev_bar2_len;
	unsigned int		zdev_chan_cnt;	/* total # of channels */
	unsigned int		zdev_can_cnt;	/* total # of CAN channels */
	unsigned int		zdev_video_cnt;	/* total # of Video channels */
	unsigned long		zdev_can_map;	/* mapping of CAN/DMA ch */
	unsigned long		zdev_video_map;	/* mapping of Video/DMA ch */
	const int		*zdev_video_port_map; /* map of video ch/port */
	unsigned int		zdev_can_num_start; /* start global CAN IP # */

	int			zcan_tx_dma;
	int			zcan_rx_dma;
	int			zcan_rx_hw_ts;

	/* interrupt related */
	int			zdev_msi_num;
	int			zdev_msi_vec;
	int			zdev_msix_num;
	struct msix_entry	*zdev_msixp;
	struct tasklet_struct	zdev_ta[ZYNQ_INT_PER_CARD];

	struct completion	zdev_gpspps_event_comp;
	struct timespec		zdev_gps_ts_first;
	struct timespec		zdev_gps_ts;	/* last valid GPS timestamp */
	unsigned int		zdev_gps_smoothing;
	unsigned int		zdev_gps_cnt;
	long			zdev_sys_drift;

	/* channels */
	zynq_chan_t		*zdev_chans;
	zynq_video_t		*zdev_videos;
	zynq_can_t		*zdev_cans;

	zynq_stats_t		stats[DEV_STATS_NUM];
	char			prefix[ZYNQ_LOG_PREFIX_LEN];

	struct zynq_dev		*zdev_next;
} zynq_dev_t;

#define	_GLOBAL_REGS_ADDR(zdev, off)	((zdev)->zdev_bar0 + (off))
#define	_CHAN_REGS_ADDR(zchan, off)	((zchan)->zchan_reg + (off))
#define	_CAN_IP_REGS_ADDR(zcan, off)	((zcan)->zcan_ip_reg + (off))
#define	_VIDEO_REGS_ADDR(zvideo, off)	((zvideo)->reg_base + (off))

/* Zynq global register access */
#define	ZDEV_G_REG32(zdev, off)		\
		(*((volatile u32 *)_GLOBAL_REGS_ADDR(zdev, off)))
#define	ZDEV_G_REG32_RD(zdev, off)	ZDEV_G_REG32(zdev, off)

/* BAR2 register access */
#define	_BAR2_REGS_ADDR(zdev, off)	((zdev)->zdev_bar2 + (off))
#define	ZDEV_BAR2_REG32(zdev, off)	\
		(*((volatile u32 *)_BAR2_REGS_ADDR(zdev, off)))
#define	ZDEV_BAR2_REG32_RD(zdev, off)	 ZDEV_BAR2_REG32(zdev, off)

/* DMA channel register access */
#define	ZCHAN_REG32(zchan, off)		\
		(*((volatile u32 *)_CHAN_REGS_ADDR(zchan, off)))
#define	ZCHAN_REG32_RD(zchan, off)	ZCHAN_REG32(zchan, off)

/* CAN channel (CAN IP) register access */
#define	ZCAN_REG32(zcan, off)		\
	(*((volatile u32 *)_CAN_IP_REGS_ADDR(zcan, off)))
#define	ZCAN_REG32_RD(zcan, off)	ZCAN_REG32(zcan, off)

/* Video channel register access */
#define	ZVIDEO_REG32(zvideo, off)	\
	(*((volatile u32 *)_VIDEO_REGS_ADDR(zvideo, off)))

#define	HI32(x)		((u32)((u64)(x) >> 32))
#define	LO32(x)		((u32)(x))

/* Get the round-up integer result of x/y */
#define	CEILING(x, y)	(((x) + (y) - 1) / (y))
#define	MIN(x, y)	(((x) < (y)) ? (x) : (y))

#define	ZDEV_IS_ERR(zdev)	(ZDEV_G_REG32_RD((zdev), ZYNQ_G_VERSION) == -1)

/*
 * Register read/write functions
 */
static inline u32 zynq_g_reg_read(zynq_dev_t *zdev, u32 reg)
{
	u32 val = 0;

	if (!zynq_bringup_param) {
		val = ZDEV_G_REG32_RD(zdev, reg);
	}
	zynq_trace(ZYNQ_TRACE_REG, "%d %s: g_reg 0x%x = 0x%x\n",
	    zdev->zdev_inst, __FUNCTION__, reg, val);
	return (val);
}

static inline void zynq_g_reg_write(zynq_dev_t *zdev, u32 reg, u32 val)
{
	if (!zynq_bringup_param) {
		ZDEV_G_REG32(zdev, reg) = val;
	}
	zynq_trace(ZYNQ_TRACE_REG, "%d %s: write 0x%x to g_reg 0x%x\n",
	    zdev->zdev_inst, __FUNCTION__, val, reg);
}

static inline u32 zynq_bar2_reg_read(zynq_dev_t *zdev, u32 reg)
{
	u32 val = 0;

	if (!zynq_bringup_param) {
		val = ZDEV_BAR2_REG32_RD(zdev, reg);
	}
	zynq_trace(ZYNQ_TRACE_REG, "%d %s: bar2_reg 0x%x = 0x%x\n",
	    zdev->zdev_inst, __FUNCTION__, reg, val);
	return (val);
}

static inline void zynq_bar2_reg_write(zynq_dev_t *zdev, u32 reg, u32 val)
{
	if (!zynq_bringup_param) {
		ZDEV_BAR2_REG32(zdev, reg) = val;
	}
	zynq_trace(ZYNQ_TRACE_REG, "%d %s: write 0x%x to bar2_reg 0x%x\n",
	    zdev->zdev_inst, __FUNCTION__, val, reg);
}

static inline u32 zchan_reg_read(zynq_chan_t *zchan, u32 reg)
{
	u32 val = 0;

	if (!zynq_bringup_param) {
		val = ZCHAN_REG32_RD(zchan, reg);
	}
	zynq_trace(ZYNQ_TRACE_REG, "%d ch %d %s: ch_reg 0x%x = 0x%x\n",
	    ZYNQ_INST(zchan), zchan->zchan_num, __FUNCTION__, reg, val);
	return (val);
}

static inline void zchan_reg_write(zynq_chan_t *zchan, u32 reg, u32 val)
{
	if (!zynq_bringup_param) {
		ZCHAN_REG32(zchan, reg) = val;
	}
	zynq_trace(ZYNQ_TRACE_REG, "%d ch %d %s: write 0x%x to ch_reg 0x%x\n",
	    ZYNQ_INST(zchan), zchan->zchan_num, __FUNCTION__, val, reg);
}

static inline u32 zcan_reg_read(zynq_can_t *zcan, u32 reg)
{
	u32 val = 0;

	if (!zynq_bringup_param) {
		val = ZCAN_REG32_RD(zcan, reg);
	}
	zynq_trace(ZYNQ_TRACE_REG, "%d CAN IP %d %s: can_ip_reg 0x%x = 0x%x\n",
	    ZYNQ_INST(zcan->zchan), zcan->zcan_ip_num, __FUNCTION__, reg, val);
	return (val);
}

static inline void zcan_reg_write(zynq_can_t *zcan, u32 reg, u32 val)
{
	if (!zynq_bringup_param) {
		ZCAN_REG32(zcan, reg) = val;
	}
	zynq_trace(ZYNQ_TRACE_REG,
	    "%d CAN IP %d %s: write 0x%x to can_ip_reg 0x%x\n",
	    ZYNQ_INST(zcan->zchan), zcan->zcan_ip_num, __FUNCTION__, val, reg);
}

static inline u32 zvideo_reg_read(zynq_video_t *zvideo, u32 reg)
{
	u32 val = 0;

	if (!zynq_bringup_param) {
		val = ZVIDEO_REG32(zvideo, reg);
	}
	zynq_trace(ZYNQ_TRACE_REG, "%d video %d %s: reg 0x%x = 0x%x\n",
	    ZYNQ_INST(zvideo->zchan), zvideo->index, __FUNCTION__, reg, val);

	return (val);
}

static inline void zvideo_reg_write(zynq_video_t *zvideo, u32 reg, u32 val)
{
	if (!zynq_bringup_param) {
		ZVIDEO_REG32(zvideo, reg) = val;
	}

	zynq_trace(ZYNQ_TRACE_REG, "%d video %d %s: reg 0x%x = 0x%x\n",
	    ZYNQ_INST(zvideo->zchan), zvideo->index, __FUNCTION__, reg, val);
}

/* function declareation */
extern zynq_dev_t *zynq_zdev_init(unsigned short zdev_did);
extern int zynq_alloc_irq(zynq_dev_t *zdev);
extern void zynq_free_irq(zynq_dev_t *zdev);
extern void zynq_check_hw_caps(zynq_dev_t *zdev);
extern int zynq_create_cdev_all(zynq_dev_t *zdev);
extern void zynq_destroy_cdev_all(zynq_dev_t *zdev);

extern void zdev_clear_intr_ch(zynq_dev_t *zdev, int ch);
extern void zynq_gps_pps_changed(zynq_dev_t *zdev);
extern void zynq_chan_err_proc(zynq_chan_t *zchan);
extern void zynq_chan_rx_proc(zynq_chan_t *zchan);
extern void zynq_chan_tx_proc(zynq_chan_t *zchan);
extern void zynq_chan_proc(zynq_chan_t *zchan, int status);
extern void zynq_chan_tasklet(unsigned long arg);
extern void zynq_tasklet(unsigned long arg);
extern dev_t zynq_create_cdev(void *drvdata, struct cdev *cdev,
    struct file_operations *fops, char *devname);
extern void zynq_destroy_cdev(dev_t dev, struct cdev *cdev);
extern int zynq_module_init(void);
extern void zynq_module_exit(void);

extern int zdev_i2c_read(zynq_dev_t *zdev, ioc_zynq_i2c_acc_t *i2c_acc);
extern int zdev_i2c_write(zynq_dev_t *zdev, ioc_zynq_i2c_acc_t *i2c_acc);
extern int zynq_sysfs_init(zynq_dev_t *zdev);
extern void zynq_sysfs_fini(zynq_dev_t *zdev);
extern void zynq_gps_init(zynq_dev_t *zdev);
extern void zynq_gps_fini(zynq_dev_t *zdev);

extern const struct pci_device_id zynq_pci_dev_tbl[];
extern struct file_operations zynq_fw_fops;
extern struct file_operations zynq_gps_fops;
extern struct file_operations zynq_reg_fops;
extern struct file_operations zynq_i2c_fops;
extern struct file_operations zynq_trigger_fops;

extern unsigned int zynq_dbg_reg_dump_param;
extern unsigned int zynq_fwupdate_param;
extern unsigned int zynq_enable_can_rx_dma;
extern unsigned int zynq_enable_can_tx_dma;
extern spinlock_t zynq_g_lock;
extern spinlock_t zynq_gps_lock;

#endif	/* _BASA_H_ */
