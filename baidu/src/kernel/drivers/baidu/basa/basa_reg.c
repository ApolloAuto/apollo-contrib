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

	case ZYNQ_IOC_STATS_GET:
	{
		ioc_zynq_stats_t *ps;
		zynq_chan_t *zchan;
		zynq_can_t *zcan;
		zynq_video_t *zvideo;
		int i, j, s;

		ps = kmalloc(sizeof(ioc_zynq_stats_t), GFP_KERNEL);
		if (!ps) {
			zynq_err("%d ZYNQ_IOC_STATS_GET: "
			    "failed to alloc buffer\n", zdev->zdev_inst);
			err = -EFAULT;
			break;
		}
		if (copy_from_user(ps, (void __user *)arg,
		    sizeof(ioc_zynq_stats_t))) {
			kfree(ps);
			zynq_err("%d ZYNQ_IOC_STATS_GET: copy_from_user "
			    "failed\n", zdev->zdev_inst);
			err = -EFAULT;
			break;
		}
		/* Initialize the data */
		for (i = 0; i <= ZYNQ_CHAN_MAX; i++) {
			ps->chs[i].type = ZYNQ_CHAN_INVAL;
			ps->chs[i].devnum = 0;
			for (j = 0; j < ZYNQ_STATS_MAX; j++) {
				ps->chs[i].stats[j] = (unsigned long)-1;
			}
		}
		/* Save the global stats at the additional channel */
		for (j = 0; j < DEV_STATS_NUM; j++) {
			ps->chs[ZYNQ_CHAN_MAX].stats[j] = zdev->stats[j].cnt;
		}
		/* Save the per-channel stats */
		ASSERT(zdev->zdev_chan_cnt <= ZYNQ_CHAN_MAX);
		zchan = zdev->zdev_chans;
		for (i = 0; i < zdev->zdev_chan_cnt; i++, zchan++) {
			switch (zchan->zchan_type) {
			case ZYNQ_CHAN_CAN:
				zcan = (zynq_can_t *)zchan->zchan_dev;
				ps->chs[i].type = zchan->zchan_type;
				ps->chs[i].devnum = zdev->zdev_can_num_start +
				    zcan->zcan_ip_num;
				for (s = 0, j = 0; (s < CHAN_STATS_NUM) &&
				    (j < ZYNQ_STATS_MAX); s++, j++) {
					ps->chs[i].stats[j] = zchan->stats[s].cnt;
				}
				for (s = 0; (s < CAN_STATS_NUM) &&
				    (j < ZYNQ_STATS_MAX); s++, j++) {
					ps->chs[i].stats[j] = zcan->stats[s].cnt;
				}
				break;
			case ZYNQ_CHAN_VIDEO:
				zvideo = (zynq_video_t *)zchan->zchan_dev;
				if (!zvideo->caps.link_up) {
					break;
				}
				ps->chs[i].type = zchan->zchan_type;
				ps->chs[i].devnum = zvideo->vdev.num;
				for (s = 0, j = 0; (s < CHAN_STATS_NUM) &&
				    (j < ZYNQ_STATS_MAX); s++, j++) {
					ps->chs[i].stats[j] = zchan->stats[s].cnt;
				}
				for (s = 0; (s < VIDEO_STATS_NUM) &&
				    (j < ZYNQ_STATS_MAX); s++, j++) {
					ps->chs[i].stats[j] = zvideo->stats[s].cnt;
				}
				break;
			default:
				break;
			}
		}
		if (copy_to_user((void __user *)arg, ps,
		    sizeof(ioc_zynq_stats_t))) {
			zynq_err("%d ZYNQ_IOC_STATS_GET: copy_to_user "
			    "failed\n", zdev->zdev_inst);
			err = -EFAULT;
		}
		kfree(ps);
		break;
	}
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
