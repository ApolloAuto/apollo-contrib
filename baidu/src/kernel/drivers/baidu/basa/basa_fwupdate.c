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

#include <linux/ktime.h>
#include <linux/delay.h>

#include "basa.h"

/* firmware update registers via PS channel */
#define	ZYNQ_PS_DATA_1			0x910
#define	ZYNQ_PS_DATA_2			0x914
#define	ZYNQ_PS_DATA_3			0x918
#define	ZYNQ_PS_DATA_4			0x91C
#define	ZYNQ_PS_PIO_STATUS		0x930
#define	ZYNQ_PS_PIO_TX_FULL		(1 << 2)

#define	ZYNQ_PS_PIO_TX_RETRIES		50	/* n * 200us = 10ms */
#define	ZYNQ_PS_WAIT_GRANT_RETRIES	5000	/* n * 200us = 1s */
#define	ZYNQ_PS_WAIT_FWUPDATE_RETRIES	600	/* n * 200ms = 2m */
#define	ZYNQ_FW_TIMEOUT			1000	/* 1 second: in mili-seonds */

#define	ZYNQ_FW_TYPE_MMC		1
#define	ZYNQ_FW_TYPE_QSPI		2
#define	ZYNQ_FW_TYPE_SPI		3

/*
 * In firmware updating mode, only channel 0 is enabled to perform in-field
 * FPGA	image upate by interacting with FPGA ARM OS:
 *    - S/W requests FPGA ARM OS for image data transfer by writing
 *      ZYNQ_CONFIG_FW_UPLOAD then ZYNQ_FONCIG_FW_UPLOAD_MAGIC in
 *      ZYNQ_G_CONFIG register.
 *    - S/W waits interrupt ZYNQ_PS_INTR_FWUPLOAD_GRANT to get grant from FPGA
 *      ARM OS for image data transfer.
 *    - S/W sends image data to FPGA in unit of 16 bytes: write ZYNQ_PS_DATA_*
 *      registers in sequence (needs to make sure ZYNQ_PS_PIO_TX_FULL is not
 *      set in register ZYNQ_PS_PIO_STATUS before sending next 16 bytes of data.
 *    - S/W requests FPGA to update QSPI flash/eMMC flash or SPI flash image by
 *      writing ZYNQ_G_CONFIG register with ZYNQ_CONFIG_FW_UPDATE_QSPI,
 *      ZYNQ_CONFIG_FW_UPDATE_MMC or ZYNQ_CONFIG_FW_UPDATE_SPI.
 *    - S/W waits interrupt ZYNQ_PS_INTR_FWUPDATE_DONE for FPGA ARM OS finishes
 *      image update.
 *    - Power cycle system and check FPGA image version from driver prints in
 *      dmesg.
 */

static int zynq_fw_upload_start(zynq_dev_t *zdev)
{
	u32 val32;
	int j = 0;

	val32 = zynq_g_reg_read(zdev, ZYNQ_G_CONFIG);
	if (val32 & ZYNQ_CONFIG_FW_UPLOAD) {
		zynq_err("%d %s failed: image upload pending.\n",
		    zdev->zdev_inst, __FUNCTION__);
		return -EBUSY;
	}
	/* clear the interrupt status */
	zynq_g_reg_write(zdev, ZYNQ_G_PS_INTR_STATUS,
	    ZYNQ_PS_INTR_FWUPLOAD_GRANT | ZYNQ_PS_INTR_FWUPDATE_DONE);
	/* request for uploading */
	zynq_g_reg_write(zdev, ZYNQ_G_CONFIG, ZYNQ_CONFIG_FW_UPLOAD);
	zynq_trace(ZYNQ_TRACE_FW, "%d %s: request uploading.\n",
	    zdev->zdev_inst, __FUNCTION__);
	/* wait for 10ms then write the update magic number */
	msleep(10);
	zynq_g_reg_write(zdev, ZYNQ_G_CONFIG, ZYNQ_CONFIG_FW_UPLOAD_MAGIC);
	zynq_trace(ZYNQ_TRACE_FW, "%d %s: magic for uploading.\n",
	    zdev->zdev_inst, __FUNCTION__);

	/* wait for grant response from ARM OS */
	while (!(zynq_g_reg_read(zdev, ZYNQ_G_PS_INTR_STATUS) &
	    ZYNQ_PS_INTR_FWUPLOAD_GRANT)) {
		if (j++ > ZYNQ_PS_WAIT_GRANT_RETRIES) {
			zynq_err("%d %s failed: wait for upload grant "
			    "timeout\n", zdev->zdev_inst, __FUNCTION__);
			return -EBUSY;
		}
		udelay(200);
	}
	zynq_g_reg_write(zdev, ZYNQ_G_CONFIG, 0);

	zynq_trace(ZYNQ_TRACE_FW, "%d %s: succeeded.\n", zdev->zdev_inst,
	    __FUNCTION__);
	return 0;
}

static int zynq_fw_update(zynq_dev_t *zdev, int fw_type, int time_to_wait)
{
	u32 val32, fw_enable;
	int j = 0;

	/* add some delay here to make sure the uploading is fully done */
	msleep(1);

	if (fw_type == ZYNQ_FW_TYPE_MMC) {
		fw_enable = ZYNQ_CONFIG_FW_UPDATE_MMC;
	} else if (fw_type == ZYNQ_FW_TYPE_QSPI) {
		fw_enable = ZYNQ_CONFIG_FW_UPDATE_QSPI;
	} else if (fw_type == ZYNQ_FW_TYPE_SPI) {
		fw_enable = ZYNQ_CONFIG_FW_UPDATE_SPI;
	} else {
		return -EINVAL;
	}

	val32 = zynq_g_reg_read(zdev, ZYNQ_G_CONFIG);
	if (val32 & fw_enable) {
		zynq_err("%d %s failed: image update pending.\n",
		    zdev->zdev_inst, __FUNCTION__);
		return -EBUSY;
	}
	/* clear the interrupt status */
	zynq_g_reg_write(zdev, ZYNQ_G_PS_INTR_STATUS,
	    ZYNQ_PS_INTR_FWUPLOAD_GRANT | ZYNQ_PS_INTR_FWUPDATE_DONE);
	/* request for updating */
	zynq_g_reg_write(zdev, ZYNQ_G_CONFIG, fw_enable);
	zynq_trace(ZYNQ_TRACE_FW, "%d %s: request updating.\n",
	    zdev->zdev_inst, __FUNCTION__);

	/* wait for updating response from ARM OS */
	while (!(zynq_g_reg_read(zdev, ZYNQ_G_PS_INTR_STATUS) &
	    ZYNQ_PS_INTR_FWUPDATE_DONE)) {
		if (j++ > time_to_wait * 5 * 2) {
			zynq_err("%d %s failed: wait for image updating "
			    "timeout\n", zdev->zdev_inst, __FUNCTION__);
			return -EBUSY;
		}
		msleep(200);
	}
	zynq_g_reg_write(zdev, ZYNQ_G_CONFIG, 0);

	zynq_trace(ZYNQ_TRACE_FW, "%d %s: succeeded.\n", zdev->zdev_inst,
	    __FUNCTION__);
	return 0;
}

/*
 * 16-byte data transfer from host to FPGA ARM OS.
 */
static int zynq_fw_tx_one(zynq_dev_t *zdev, u32 *fw_data, int wait)
{
	int j = 0;

	/* poll status regsiter to see if the Tx FIFO is FULL */
	if (wait) {
		while (zynq_bar2_reg_read(zdev, ZYNQ_PS_PIO_STATUS) &
		    ZYNQ_PS_PIO_TX_FULL) {
			if (j++ > ZYNQ_PS_PIO_TX_RETRIES) {
				zynq_err("%d %s failed: Tx FIFO is FULL\n",
				    zdev->zdev_inst, __FUNCTION__);
				return -ENOSPC;
			}
			udelay(200);
		}
		zynq_trace(ZYNQ_TRACE_FW, "%d %s: wait_time = %dus\n",
		    zdev->zdev_inst, __FUNCTION__, j * 200);
	} else {
		if (zynq_bar2_reg_read(zdev, ZYNQ_PS_PIO_STATUS) &
		    ZYNQ_PS_PIO_TX_FULL) {
			return -ENOSPC;
		}
	}

	zynq_bar2_reg_write(zdev, ZYNQ_PS_DATA_1, fw_data[0]);
	zynq_bar2_reg_write(zdev, ZYNQ_PS_DATA_2, fw_data[1]);
	zynq_bar2_reg_write(zdev, ZYNQ_PS_DATA_3, fw_data[2]);
	zynq_bar2_reg_write(zdev, ZYNQ_PS_DATA_4, fw_data[3]);
	zdev->zdev_fw_tx_cnt++;
	udelay(100);

	zynq_trace(ZYNQ_TRACE_FW, "%d %s: msg %u: fw_data[0]=0x%x, "
	    "fw_data[1]=0x%x, fw_data[2]=0x%x, fw_data[3]=0x%x\n",
	    zdev->zdev_inst, __FUNCTION__, zdev->zdev_fw_tx_cnt,
	    fw_data[0], fw_data[1], fw_data[2], fw_data[3]);

	return 0;
}

static int zynq_fw_tx_user(zynq_dev_t *zdev, ioc_zynq_fw_upload_t *ioc_arg)
{
	int tx_num = ioc_arg->ioc_zynq_fw_num;
	int tx_num_done = 0;
	u32 *fw_data, *fw_data_alloc = NULL;
	ktime_t ktime_end, ktime_now;
	int ret = 0;

	/* alloc memory and copy-in all the sending data */
	fw_data_alloc = kmalloc(tx_num * ZYNQ_FW_MSG_SZ, GFP_KERNEL);
	if (fw_data_alloc == NULL) {
		return -ENOMEM;
	}
	fw_data = fw_data_alloc;
	if (copy_from_user(fw_data, (void __user *)ioc_arg->ioc_zynq_fw_data,
	    tx_num * ZYNQ_FW_MSG_SZ)) {
		kfree(fw_data_alloc);
		return -EFAULT;
	}
	ktime_end = ktime_add_ns(ktime_get(), (u64)1000000 * ZYNQ_FW_TIMEOUT);

	/* send the msg one by one till finished or timeout */
	while (tx_num) {
		ret = zynq_fw_tx_one(zdev, fw_data, 0);
		if (ret) {
			/* checking timeout */
			ktime_now = ktime_get();
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 10, 0)
			if (ktime_now.tv64 < ktime_end.tv64) {
#else
			if (ktime_now < ktime_end) {
#endif
				msleep(1);
				continue;
			}
			break;
		}
		tx_num_done++;
		fw_data += 4; /* 4 * sizeof(int) = 16 bytes */
		tx_num--;
	}

	ioc_arg->ioc_zynq_fw_done = tx_num_done;

	kfree(fw_data_alloc);

	zynq_trace(ZYNQ_TRACE_FW, "%d %s done: tx_num=%d, tx_num_done=%d\n",
	    zdev->zdev_inst, __FUNCTION__,
	    ioc_arg->ioc_zynq_fw_num, tx_num_done);

	return ret;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35)
static int zynq_fw_ioctl(struct inode *inode, struct file *filp,
		unsigned int cmd, unsigned long arg)
#else
static long zynq_fw_ioctl(struct file *filp,
		unsigned int cmd, unsigned long arg)
#endif
{
	zynq_dev_t *zdev = filp->private_data;
	int err = 0;
	char ioc_name[32];

	zynq_trace(ZYNQ_TRACE_CAN_PIO, "%d %s: cmd=0x%x, arg=0x%lx, "
	    "zdev=0x%p\n", zdev->zdev_inst, __FUNCTION__, cmd, arg, zdev);

	switch (cmd) {
	case ZYNQ_IOC_FW_IMAGE_UPLOAD_START:
		sprintf(ioc_name, "ZYNQ_IOC_FW_IMAGE_UPLOAD_START");
		err = zynq_fw_upload_start(zdev);
		break;

	case ZYNQ_IOC_FW_IMAGE_UPLOAD:
	{
		ioc_zynq_fw_upload_t ioc_zynq_fw_arg;

		sprintf(ioc_name, "ZYNQ_IOC_FW_IMAGE_UPLOAD");
		if (copy_from_user(&ioc_zynq_fw_arg, (void __user *)arg,
		    sizeof(ioc_zynq_fw_upload_t))) {
			err = -EFAULT;
			break;
		}

		if (ZDEV_IS_ERR(zdev)) {
			err = -EIO;
			break;
		}

		/* call the tx routine to upload the data */
		if ((err = zynq_fw_tx_user(zdev, &ioc_zynq_fw_arg))) {
			break;
		}

		if (copy_to_user((void __user *)
		    (&((ioc_zynq_fw_upload_t *)arg)->ioc_zynq_fw_done),
		    &ioc_zynq_fw_arg.ioc_zynq_fw_done, 2 * sizeof(int))) {
			err = -EFAULT;
		}

		break;
	}
	case ZYNQ_IOC_FW_QSPI_UPDATE:
		sprintf(ioc_name, "ZYNQ_IOC_FW_QSPI_UPDATE");
		err = zynq_fw_update(zdev, ZYNQ_FW_TYPE_QSPI, arg);
		break;

	case ZYNQ_IOC_FW_MMC_UPDATE:
		sprintf(ioc_name, "ZYNQ_IOC_FW_MMC_UPDATE");
		err = zynq_fw_update(zdev, ZYNQ_FW_TYPE_MMC, arg);
		break;

	case ZYNQ_IOC_FW_SPI_UPDATE:
		sprintf(ioc_name, "ZYNQ_IOC_FW_SPI_UPDATE");
		err = zynq_fw_update(zdev, ZYNQ_FW_TYPE_SPI, arg);
		break;

	case ZYNQ_IOC_FW_GET_VER:
		sprintf(ioc_name, "ZYNQ_IOC_FW_GET_VER");
		if (copy_to_user((void __user *)(int *)arg,
		    &zdev->zdev_version, sizeof(int))) {
			err = -EFAULT;
		}
		break;
	default:
		sprintf(ioc_name, "ZYNQ_IOC_FW_UNKNOWN");
		err = -EINVAL;
		break;
	}

	zynq_trace(ZYNQ_TRACE_CAN_PIO, "zynq %d %s done: cmd=0x%x(%s), "
	    "error=%d\n", zdev->zdev_inst, __FUNCTION__, cmd, ioc_name, err);
	return (err);
}

static int zynq_fw_open(struct inode *inode, struct file *filp)
{
	zynq_dev_t *zdev;

	zdev = container_of(inode->i_cdev, zynq_dev_t, zdev_cdev_fw);
	if (atomic_read(&zdev->zdev_fw_opened)) {
		zynq_err("%s failed: device is opened already\n", __FUNCTION__);
		return (-EBUSY);
	}
	atomic_set(&zdev->zdev_fw_opened, 1);
	filp->private_data = zdev;

	zynq_trace(ZYNQ_TRACE_FW, "%s done.", __FUNCTION__);
	return (0);
}

static int zynq_fw_release(struct inode *inode, struct file *filp)
{
	zynq_dev_t *zdev = filp->private_data;

	atomic_set(&zdev->zdev_fw_opened, 0);

	return (0);
}

struct file_operations zynq_fw_fops = {
	.owner		= THIS_MODULE,
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35)
	.ioctl		= zynq_fw_ioctl,
#else
	.unlocked_ioctl = zynq_fw_ioctl,
#endif
	.open		= zynq_fw_open,
	.release	= zynq_fw_release
};
