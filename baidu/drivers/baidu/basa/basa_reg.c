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

#include "basa.h"

/*
 * Register access support ioctls
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35)
static int zynq_reg_ioctl(struct inode *inode, struct file *filp,
		unsigned int cmd, unsigned long arg)
#else
static long zynq_reg_ioctl(struct file *filp, unsigned int cmd,
		unsigned long arg)
#endif
{
	zynq_dev_t *zdev = filp->private_data;
	ioc_zynq_reg_acc_t reg_acc;
	ioc_zynq_i2c_acc_t i2c_acc;
	int err = 0;

	switch (cmd) {
	case ZYNQ_IOC_REG_READ:
		if (copy_from_user(&reg_acc, (void __user *)arg,
		    sizeof(ioc_zynq_reg_acc_t))) {
			zynq_err("%d ZYNQ_IOC_REG_READ: copy_from_user "
			    "failed\n", zdev->zdev_inst);
			err = -EFAULT;
			break;
		}
		if (reg_acc.reg_bar == 0) {
			reg_acc.reg_data = zynq_g_reg_read(zdev,
			    reg_acc.reg_offset);
		} else if (reg_acc.reg_bar == 2) {
			reg_acc.reg_data = zynq_bar2_reg_read(zdev,
			    reg_acc.reg_offset);
		} else {
			zynq_err("%d ZYNQ_IOC_REG_READ: wrong BAR number "
			    "%d\n", zdev->zdev_inst, reg_acc.reg_bar);
			err = -EINVAL;
			break;
		}
		zynq_trace(ZYNQ_TRACE_REG, "zynq %d ZYNQ_IOC_REG_READ: "
		    "reg_bar=%d, reg_offset=0x%x, reg_data=0x%x\n",
		    zdev->zdev_inst, reg_acc.reg_bar, reg_acc.reg_offset,
		    reg_acc.reg_data);

		if (copy_to_user((void __user *)arg, &reg_acc,
		    sizeof(ioc_zynq_reg_acc_t))) {
			zynq_err("%d ZYNQ_IOC_REG_READ: copy_to_user "
			    "failed\n", zdev->zdev_inst);
			err = -EFAULT;
		}
		break;

	case ZYNQ_IOC_REG_WRITE:
		if (copy_from_user(&reg_acc, (void __user *)arg,
		    sizeof(ioc_zynq_reg_acc_t))) {
			zynq_err("%d ZYNQ_IOC_REG_WRITE: copy_from_user "
			    "failed\n", zdev->zdev_inst);
			err = -EFAULT;
			break;
		}
		if (reg_acc.reg_bar == 0) {
			zynq_g_reg_write(zdev, reg_acc.reg_offset,
			    reg_acc.reg_data);
		} else if (reg_acc.reg_bar == 2) {
			zynq_bar2_reg_write(zdev, reg_acc.reg_offset,
			    reg_acc.reg_data);
		} else {
			zynq_err("%d ZYNQ_IOC_REG_WRITE: wrong BAR number "
			    "%d\n", zdev->zdev_inst, reg_acc.reg_bar);
			err = -EINVAL;
			break;
		}
		zynq_trace(ZYNQ_TRACE_REG, "zynq %d ZYNQ_IOC_REG_WRITE: "
		    "reg_bar=%d, reg_offset=0x%x, reg_data=0x%x\n",
		    zdev->zdev_inst, reg_acc.reg_bar, reg_acc.reg_offset,
		    reg_acc.reg_data);
		break;

	case ZYNQ_IOC_REG_I2C_READ:
		if (copy_from_user(&i2c_acc, (void __user *)arg,
		    sizeof(ioc_zynq_i2c_acc_t))) {
			zynq_err("%d ZYNQ_IOC_REG_I2C_READ: copy_from_user "
			    "failed\n", zdev->zdev_inst);
			err = -EFAULT;
			break;
		}

		err = zdev_i2c_read(zdev, &i2c_acc);
		if (err) {
			break;
		}

		if (copy_to_user((void __user *)arg, &i2c_acc,
		    sizeof(ioc_zynq_i2c_acc_t))) {
			zynq_err("%d ZYNQ_IOC_REG_I2C_READ: copy_to_user "
			    "failed\n", zdev->zdev_inst);
			err = -EFAULT;
		}
		break;

	case ZYNQ_IOC_REG_I2C_WRITE:
		if (copy_from_user(&i2c_acc, (void __user *)arg,
		    sizeof(ioc_zynq_i2c_acc_t))) {
			zynq_err("%d ZYNQ_IOC_REG_I2C_WRITE: copy_from_user "
			    "failed\n", zdev->zdev_inst);
			err = -EFAULT;
			break;
		}

		err = zdev_i2c_write(zdev, &i2c_acc);
		break;

	case ZYNQ_IOC_REG_GPSPPS_EVENT_WAIT:
		/* wait for the event notification */
		err = wait_for_completion_interruptible(
		    &zdev->zdev_gpspps_event_comp);
		reinit_completion(&zdev->zdev_gpspps_event_comp);
		break;

	default:
		err = -EINVAL;
		break;
	}

	zynq_trace(ZYNQ_TRACE_PROBE, "%d %s done: cmd=0x%x, error=%d\n",
	    zdev->zdev_inst, __FUNCTION__, cmd, err);
	return err;
}

static int zynq_reg_open(struct inode *inode, struct file *filp)
{
	zynq_dev_t *zdev;

	zdev = container_of(inode->i_cdev, zynq_dev_t, zdev_cdev_reg);
	filp->private_data = zdev;

	zynq_trace(ZYNQ_TRACE_PROBE, "%d %s done\n",
	    zdev->zdev_inst, __FUNCTION__);
	return 0;
}

static int zynq_reg_release(struct inode *inode, struct file *filp)
{
	return 0;
}

struct file_operations zynq_reg_fops = {
	.owner		= THIS_MODULE,
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35)
	.ioctl		= zynq_reg_ioctl,
#else
	.unlocked_ioctl = zynq_reg_ioctl,
#endif
	.open		= zynq_reg_open,
	.release	= zynq_reg_release
};
