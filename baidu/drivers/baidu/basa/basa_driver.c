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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/version.h>

#include "basa.h"

static const int video_port_map_mr[] = { 1, 2, 3, 4, 5 };
static unsigned int zynq_mr_cnt = 0;

zynq_dev_t *zynq_zdev_init(unsigned short zdev_did)
{
	zynq_dev_t *zdev;
	int hw_cap;
	int can_cnt, video_cnt, chan_cnt;
	unsigned long video_map, can_map;
	char code_name[ZYNQ_DEV_CODE_NAME_LEN];

	switch (zdev_did) {
	case PCI_DEVICE_ID_MOONROVER:
		hw_cap = ZYNQ_HW_CAP_CAN | ZYNQ_HW_CAP_GPS |
		    ZYNQ_HW_CAP_TRIGGER | ZYNQ_HW_CAP_FW |
		    ZYNQ_HW_CAP_I2C | ZYNQ_HW_CAP_VIDEO;
		chan_cnt = ZYNQ_DMA_CHAN_NUM_MR;
		video_map = ZYNQ_VIDEO_MAP_MR;
		can_map = ZYNQ_CAN_MAP_MR;
		sprintf(code_name, "MoonRover");
		break;
	default:
		zynq_err("%s: Unknown Device ID 0x%x\n",
		    __FUNCTION__, zdev_did);
		return NULL;
	}

	if (zynq_fwupdate_param) {
		switch (zdev_did) {
		case PCI_DEVICE_ID_MOONROVER:
			hw_cap = ZYNQ_HW_CAP_FW;
			chan_cnt = 0;
			can_map = 0;
			video_map = 0;
			break;
		default:
			break;
		}
	}

	video_cnt = hweight_long(video_map);
	can_cnt = hweight_long(can_map);
	zynq_trace(ZYNQ_TRACE_PROBE,
	    "%s: chan_cnt = %d, video_cnt = %d, can_cnt = %d\n",
	    __FUNCTION__, chan_cnt, video_cnt, can_cnt);
	ASSERT(chan_cnt >= (can_cnt + video_cnt));

	/* allocate device and channel structure */
	zdev = kzalloc(sizeof(zynq_dev_t) + chan_cnt * sizeof(zynq_chan_t) +
	    can_cnt * sizeof(zynq_can_t) + video_cnt * sizeof(zynq_video_t),
	    GFP_KERNEL);
	if (zdev == NULL) {
		return NULL;
	}

	zdev->zdev_chan_cnt = chan_cnt;
	zdev->zdev_can_cnt = can_cnt;
	zdev->zdev_video_cnt = video_cnt;
	zdev->zdev_can_map = can_map;
	zdev->zdev_video_map = video_map;
	zdev->zdev_hw_cap = hw_cap;
	memcpy(zdev->zdev_code_name, code_name, ZYNQ_DEV_CODE_NAME_LEN);

	zdev->zdev_chans = (zynq_chan_t *)((char *)zdev + sizeof(zynq_dev_t));
	zdev->zdev_cans = (zynq_can_t *)((char *)zdev->zdev_chans +
	    chan_cnt * sizeof(zynq_chan_t));
	zdev->zdev_videos = (zynq_video_t *)((char *)zdev->zdev_cans +
	    can_cnt * sizeof(zynq_can_t));

	if (zynq_fwupdate_param) {
		zdev->zcan_tx_dma = 0;
		zdev->zcan_rx_dma = 0;
	} else {
		zdev->zcan_tx_dma = 0;
		zdev->zcan_rx_dma = 1;
	}

	switch (zdev_did) {
	case PCI_DEVICE_ID_MOONROVER:
		zdev->zdev_video_port_map = video_port_map_mr;
		break;
	default:
		break;
	}

	return (zdev);
}

void zynq_check_fw_version(zynq_dev_t *zdev)
{
	zdev->zcan_rx_hw_ts = 1;
}

int zynq_create_cdev_all(zynq_dev_t *zdev)
{
	char cdev_name[32];
	unsigned int inst;

	if (zynq_fwupdate_param && (zdev->zdev_hw_cap & ZYNQ_HW_CAP_FW)) {
		/* create /dev/zynq_fw_xx */
		switch (zdev->zdev_did) {
		case PCI_DEVICE_ID_MOONROVER:
			spin_lock(&zynq_g_lock);
			inst = zynq_mr_cnt++;
			spin_unlock(&zynq_g_lock);
			sprintf(cdev_name, ZYNQ_DEV_NAME_FW"_mr%u", inst);
			break;
		default:
			goto cdev_fail;
			break;
		}
		zdev->zdev_dev_fw = zynq_create_cdev(zdev, &zdev->zdev_cdev_fw,
		    &zynq_fw_fops, cdev_name);
		if (zdev->zdev_dev_fw == 0) {
			goto cdev_fail;
		}
	}

	/* create /dev/zynq_reg%d */
	sprintf(cdev_name, ZYNQ_DEV_NAME_REG"%d", zdev->zdev_inst);
	zdev->zdev_dev_reg = zynq_create_cdev(zdev, &zdev->zdev_cdev_reg,
	    &zynq_reg_fops, cdev_name);
	if (zdev->zdev_dev_reg == 0) {
		goto cdev_fail;
	}

	/* create /dev/zynq_i2c%d */
	if (zdev->zdev_hw_cap & ZYNQ_HW_CAP_I2C) {
		sprintf(cdev_name, ZYNQ_DEV_NAME_I2C"%d", zdev->zdev_inst);
		zdev->zdev_dev_i2c = zynq_create_cdev(zdev,
		    &zdev->zdev_cdev_i2c, &zynq_i2c_fops,
		    cdev_name);
		if (zdev->zdev_dev_i2c == 0) {
			goto cdev_fail;
		}
	}

	/* create /dev/zynq_trigger%d */
	if (zdev->zdev_hw_cap & ZYNQ_HW_CAP_TRIGGER) {
		sprintf(cdev_name, ZYNQ_DEV_NAME_TRIGGER"%d", zdev->zdev_inst);
		zdev->zdev_dev_trigger = zynq_create_cdev(zdev,
		    &zdev->zdev_cdev_trigger, &zynq_trigger_fops,
		    cdev_name);
		if (zdev->zdev_dev_trigger == 0) {
			goto cdev_fail;
		}
	}

	/* create /dev/zynq_gps%d */
	if (zdev->zdev_hw_cap & ZYNQ_HW_CAP_GPS) {
		sprintf(cdev_name, ZYNQ_DEV_NAME_GPS"%d", zdev->zdev_inst);
		zdev->zdev_dev_gps = zynq_create_cdev(zdev,
		    &zdev->zdev_cdev_gps, &zynq_gps_fops, cdev_name);
		if (zdev->zdev_dev_gps == 0) {
			goto cdev_fail;
		}
	}
	return 0;

cdev_fail:
	zynq_destroy_cdev_all(zdev);
	return -1;
}

void zynq_destroy_cdev_all(zynq_dev_t *zdev)
{
	if (zdev->zdev_dev_gps) {
		zynq_destroy_cdev(zdev->zdev_dev_gps, &zdev->zdev_cdev_gps);
	}
	if (zdev->zdev_dev_trigger) {
		zynq_destroy_cdev(zdev->zdev_dev_trigger,
		    &zdev->zdev_cdev_trigger);
	}
	if (zdev->zdev_dev_fw) {
		zynq_destroy_cdev(zdev->zdev_dev_fw, &zdev->zdev_cdev_fw);
	}
	if (zdev->zdev_dev_reg) {
		zynq_destroy_cdev(zdev->zdev_dev_reg, &zdev->zdev_cdev_reg);
	}
	if (zdev->zdev_dev_i2c) {
		zynq_destroy_cdev(zdev->zdev_dev_i2c, &zdev->zdev_cdev_i2c);
	}
}

/* Supported Sensor FPGA */
const struct pci_device_id zynq_pci_dev_tbl[] = {
	/* Vendor, Device, SubSysVendor, SubSysDev, Class, ClassMask, DrvDat */
	/* MoonRover, Sensor Box */
	{ PCI_VENDOR_ID_BAIDU, PCI_DEVICE_ID_MOONROVER,
	    PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
	{ }	/* terminate list */
};

MODULE_DEVICE_TABLE(pci, zynq_pci_dev_tbl);

/* Sensor FPGA driver module load entry point */
static int __init basa_init(void)
{
	return zynq_module_init();
}

/* Sensor FPGA driver module remove entry point */
static void __exit basa_exit(void)
{
	zynq_module_exit();
}

module_init(basa_init);
module_exit(basa_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("BAIDU USA, LLC");
MODULE_DESCRIPTION("basa: Baidu Sensor Aggregator Driver");
/*
 * 2.1.1.1: Initial release for Sensor FPGA support.
 */
MODULE_VERSION(ZYNQ_MOD_VER);
