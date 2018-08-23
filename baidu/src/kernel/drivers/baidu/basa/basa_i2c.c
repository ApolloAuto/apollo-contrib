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

#include <linux/delay.h>

#include "basa.h"

/*
 * I2C register access
 */
#define ZYNQ_I2C_WAIT_US		300 /* 100Kbps */
#define ZYNQ_I2C_RETRY			20

static u32 i2c_control_regs[ZYNQ_I2C_BUS_NUM] = {
	ZYNQ_G_I2C_CONTROL_0,
	ZYNQ_G_I2C_CONTROL_1,
	ZYNQ_G_I2C_CONTROL_2,
	ZYNQ_G_I2C_CONTROL_3
};
static u32 i2c_config_regs[ZYNQ_I2C_BUS_NUM] = {
	ZYNQ_G_I2C_CONFIG_0,
	ZYNQ_G_I2C_CONFIG_1,
	ZYNQ_G_I2C_CONFIG_2,
	ZYNQ_G_I2C_CONFIG_3
};

static int zdev_i2c_validate(zynq_dev_t *zdev, ioc_zynq_i2c_acc_t *i2c_acc)
{
	if (!(zdev->zdev_hw_cap & ZYNQ_HW_CAP_I2C)) {
		zynq_err("%d %s: I2C is not supported on this device\n",
		    zdev->zdev_inst, __FUNCTION__);
		return -EPERM;
	}

	if (i2c_acc->i2c_bus >= ZYNQ_I2C_BUS_NUM) {
		zynq_err("%d %s invalid i2c_bus=%d",
		    zdev->zdev_inst, __FUNCTION__, i2c_acc->i2c_bus);
		return -EINVAL;
	}

	if (i2c_acc->i2c_id > ZYNQ_I2C_ID_MAX) {
		zynq_err("%d %s: invalid i2c_id=0x%x\n",
		    zdev->zdev_inst, __FUNCTION__, i2c_acc->i2c_id);
		return -EINVAL;
	}

	return 0;
}

static int zdev_i2c_check_busy(zynq_dev_t *zdev, ioc_zynq_i2c_acc_t *i2c_acc)
{
	u32 ctrl_reg;
	u32 ctrl_val;
	int i;

	ctrl_reg = i2c_control_regs[i2c_acc->i2c_bus];

	for (i = 0; i < ZYNQ_I2C_RETRY; i++) {
		ctrl_val = zynq_g_reg_read(zdev, ctrl_reg);
		if (ctrl_val & ZYNQ_I2C_CMD_ERR) {
			zynq_err("%d %s error detected, "
			    "i2c_bus=%d, i2c_ctrl=0x%x\n",
			    zdev->zdev_inst, __FUNCTION__,
			    i2c_acc->i2c_bus, ctrl_val);
			return -EFAULT;
		}
		if (!(ctrl_val & ZYNQ_I2C_CMD_BUSY)) {
			return 0;
		}
		/* wait and retry */
		udelay(ZYNQ_I2C_WAIT_US);
	}

	zynq_err("%d %s busy wait timeout, i2c_bus=%d, i2c_ctrl=0x%x\n",
	    zdev->zdev_inst, __FUNCTION__, i2c_acc->i2c_bus, ctrl_val);

	return -EAGAIN;
}

int zdev_i2c_read(zynq_dev_t *zdev, ioc_zynq_i2c_acc_t *i2c_acc)
{
	u32 ctrl_reg;
	u32 ctrl_val = 0;
	u32 cfg_reg;
	u32 cfg_val = 0;
	int err = 0;

	err = zdev_i2c_validate(zdev, i2c_acc);
	if (err) {
		return err;
	}

	spin_lock(&zdev->zdev_i2c_lock);
	err = zdev_i2c_check_busy(zdev, i2c_acc);
	if (err == -EAGAIN) {
		goto done;
	}

	ctrl_reg = i2c_control_regs[i2c_acc->i2c_bus];
	cfg_reg = i2c_config_regs[i2c_acc->i2c_bus];

	if (i2c_acc->i2c_addr_16) {
		ctrl_val = ZYNQ_I2C_ADDR_16;

		/* set the higher 8-bit I2C address */
		cfg_val = SET_BITS(ZYNQ_I2C_ADDR_HI, 0,
		    i2c_acc->i2c_addr_hi);
		zynq_g_reg_write(zdev, cfg_reg, cfg_val);
	}

	/* do I2C reading */
	ctrl_val |= ZYNQ_I2C_CMD_READ;
	ctrl_val = SET_BITS(ZYNQ_I2C_ID, ctrl_val, i2c_acc->i2c_id);
	ctrl_val = SET_BITS(ZYNQ_I2C_ADDR, ctrl_val, i2c_acc->i2c_addr);
	zynq_g_reg_write(zdev, ctrl_reg, ctrl_val);

	err = zdev_i2c_check_busy(zdev, i2c_acc);
	if (err) {
		goto done;
	}
	i2c_acc->i2c_data = (unsigned char)
	    GET_BITS(ZYNQ_I2C_DATA, zynq_g_reg_read(zdev, ctrl_reg));
done:
	spin_unlock(&zdev->zdev_i2c_lock);
	if (err) {
		zynq_err("%d %s failed: i2c_id=0x%02x, i2c_addr_hi=0x%02x, "
		    "i2c_addr=0x%02x, i2c_addr_16=%d, i2c_bus=%d\n",
		    zdev->zdev_inst, __FUNCTION__,
		    i2c_acc->i2c_id, i2c_acc->i2c_addr_hi,
		    i2c_acc->i2c_addr, i2c_acc->i2c_addr_16, i2c_acc->i2c_bus);
	} else {
		zynq_trace(ZYNQ_TRACE_REG, "%d %s OK: i2c_id=0x%02x, "
		    "i2c_addr_hi=0x%02x, i2c_addr=0x%02x, "
		    "i2c_data=0x%02x, i2c_addr_16=%d, i2c_bus=%d\n",
		    zdev->zdev_inst, __FUNCTION__, i2c_acc->i2c_id,
		    i2c_acc->i2c_addr_hi, i2c_acc->i2c_addr,
		    i2c_acc->i2c_data, i2c_acc->i2c_addr_16, i2c_acc->i2c_bus);
	}
	return err;
}

int zdev_i2c_write(zynq_dev_t *zdev, ioc_zynq_i2c_acc_t *i2c_acc)
{
	u32 ctrl_reg;
	u32 ctrl_val = 0;
	u32 cfg_reg;
	u32 cfg_val = 0;
	int err = 0;

	err = zdev_i2c_validate(zdev, i2c_acc);
	if (err) {
		return err;
	}

	spin_lock(&zdev->zdev_i2c_lock);
	err = zdev_i2c_check_busy(zdev, i2c_acc);
	if (err == -EAGAIN) {
		goto done;
	}

	ctrl_reg = i2c_control_regs[i2c_acc->i2c_bus];
	cfg_reg = i2c_config_regs[i2c_acc->i2c_bus];

	if (i2c_acc->i2c_addr_16) {
		ctrl_val = ZYNQ_I2C_ADDR_16;

		/* set the higher 8-bit I2C address */
		cfg_val = SET_BITS(ZYNQ_I2C_ADDR_HI, 0,
		    i2c_acc->i2c_addr_hi);
		zynq_g_reg_write(zdev, cfg_reg, cfg_val);
	}

	/* do I2C writing */
	ctrl_val = SET_BITS(ZYNQ_I2C_DATA, ctrl_val, i2c_acc->i2c_data);
	ctrl_val = SET_BITS(ZYNQ_I2C_ID, ctrl_val, i2c_acc->i2c_id);
	ctrl_val = SET_BITS(ZYNQ_I2C_ADDR, ctrl_val, i2c_acc->i2c_addr);
	ctrl_val &= ~ZYNQ_I2C_CMD_READ;
	zynq_g_reg_write(zdev, ctrl_reg, ctrl_val);

	err = zdev_i2c_check_busy(zdev, i2c_acc);
done:
	spin_unlock(&zdev->zdev_i2c_lock);
	if (err) {
		zynq_err("%d %s failed: i2c_id=0x%02x, "
		    "i2c_addr_hi=0x%02x, i2c_addr=0x%02x, "
		    "i2c_data=0x%02x, i2c_addr_16=%d, i2c_bus=%d\n",
		    zdev->zdev_inst, __FUNCTION__, i2c_acc->i2c_id,
		    i2c_acc->i2c_addr_hi, i2c_acc->i2c_addr,
		    i2c_acc->i2c_data, i2c_acc->i2c_addr_16, i2c_acc->i2c_bus);
	} else {
		zynq_trace(ZYNQ_TRACE_REG, "%d %s OK: i2c_id=0x%02x, "
		    "i2c_addr_hi=0x%02x, i2c_addr=0x%02x, "
		    "i2c_data=0x%02x, i2c_addr_16=%d, i2c_bus=%d\n",
		    zdev->zdev_inst, __FUNCTION__, i2c_acc->i2c_id,
		    i2c_acc->i2c_addr_hi, i2c_acc->i2c_addr,
		    i2c_acc->i2c_data, i2c_acc->i2c_addr_16, i2c_acc->i2c_bus);
	}
	return err;
}

static long zynq_i2c_ioctl(struct file *filp, unsigned int cmd,
		unsigned long arg)
{
	zynq_dev_t *zdev = filp->private_data;
	ioc_zynq_i2c_acc_t i2c_acc;
	int err = 0;

	switch (cmd) {
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

	default:
		err = -EINVAL;
		break;
	}

	zynq_trace(ZYNQ_TRACE_PROBE, "%d %s done: cmd=0x%x, error=%d\n",
	    zdev->zdev_inst, __FUNCTION__, cmd, err);
	return err;
}

static int zynq_i2c_open(struct inode *inode, struct file *filp)
{
	zynq_dev_t *zdev;

	zdev = container_of(inode->i_cdev, zynq_dev_t, zdev_cdev_i2c);
	filp->private_data = zdev;

	zynq_trace(ZYNQ_TRACE_PROBE, "%d %s done\n",
	    zdev->zdev_inst, __FUNCTION__);
	return 0;
}

static int zynq_i2c_release(struct inode *inode, struct file *filp)
{
	return 0;
}

struct file_operations zynq_i2c_fops = {
	.owner		= THIS_MODULE,
	.unlocked_ioctl = zynq_i2c_ioctl,
	.open		= zynq_i2c_open,
	.release	= zynq_i2c_release
};
