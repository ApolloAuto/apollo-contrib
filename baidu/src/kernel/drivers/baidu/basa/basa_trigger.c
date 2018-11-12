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

#include <linux/module.h>
#include <linux/kobject.h>

#include "basa.h"

static int zynq_trigger_enable_all(zynq_dev_t *zdev, unsigned long arg,
		int internal, int one)
{
	zynq_trigger_t trigger[1];
	zynq_video_t *zvideo;
	unsigned int val;
	int i;

	if (arg) {
		if (copy_from_user(trigger, (void __user *)arg,
		    sizeof(zynq_trigger_t))) {
			return -EFAULT;
		}
		/* overwrite the internal parameter */
		internal = trigger->internal;
	}

	spin_lock(&zdev->zdev_lock);
	/* 1. disable the global trigger */
	val = zynq_g_reg_read(zdev, ZYNQ_G_CONFIG);
	val &= ~ZYNQ_CONFIG_TRIGGER_MASK;
	zynq_g_reg_write(zdev, ZYNQ_G_CONFIG, val);

	/* 2. set FPS and enable the individual triggers */
	if (arg) {
		if ((trigger->fps == 0) || (trigger->fps > ZYNQ_FPD_FPS_MAX)) {
			trigger->fps = ZYNQ_FPD_FPS_DEFAULT;
		}
	} else {
		trigger->fps = ZYNQ_FPD_FPS_DEFAULT;
	}
	for (i = 0; i < zdev->zdev_video_cnt; i++) {
		zvideo = &zdev->zdev_videos[i];
		if (!zvideo->caps.link_up) {
			continue;
		}
		/* Set FPS */
		val = zvideo_reg_read(zvideo, ZYNQ_CAM_TRIGGER);
		val = SET_BITS(ZYNQ_CAM_TRIG_FPS, val, trigger->fps);
		zvideo_reg_write(zvideo, ZYNQ_CAM_TRIGGER, val);
		/* Enable individual trigger */
		val = zvideo_reg_read(zvideo, ZYNQ_CAM_CONFIG);
		val |= ZYNQ_CAM_EN;
		zvideo_reg_write(zvideo, ZYNQ_CAM_CONFIG, val);

		zvideo->fps = trigger->fps;
		zvideo->frame_interval = USEC_PER_SEC / trigger->fps;
		zvideo->frame_usec_max = zvideo->frame_interval * zvideo->fps;
	}

	/* 3. enable the global trigger */
	val = zynq_g_reg_read(zdev, ZYNQ_G_CONFIG);
	val &= ~ZYNQ_CONFIG_TRIGGER_MASK;
	if (one) {
		val |= ZYNQ_CONFIG_TRIGGER_ONE;
	} else {
		val |= ZYNQ_CONFIG_TRIGGER;
	}
	if (internal) {
		val |= ZYNQ_CONFIG_GPS_SW;
	}
	zynq_g_reg_write(zdev, ZYNQ_G_CONFIG, val);
	spin_unlock(&zdev->zdev_lock);

	zynq_log("%d: enable all triggers done. internal=%u\n",
	    zdev->zdev_inst, internal);

	return 0;
}

static int zynq_trigger_disable_all(zynq_dev_t *zdev)
{
	zynq_video_t *zvideo;
	unsigned int val;
	int i;

	/* 1. disable the global trigger */
	spin_lock(&zdev->zdev_lock);
	val = zynq_g_reg_read(zdev, ZYNQ_G_CONFIG);
	val &= ~ZYNQ_CONFIG_TRIGGER_MASK;
	zynq_g_reg_write(zdev, ZYNQ_G_CONFIG, val);

	/* 2. disable the individual triggers */
	for (i = 0; i < zdev->zdev_video_cnt; i++) {
		zvideo = &zdev->zdev_videos[i];
		if (!zvideo->caps.link_up) {
			continue;
		}
		val = zvideo_reg_read(zvideo, ZYNQ_CAM_CONFIG);
		val &= ~ZYNQ_CAM_EN;
		zvideo_reg_write(zvideo, ZYNQ_CAM_CONFIG, val);

		zvideo->fps = 0;
		zvideo->frame_interval = 0;
		zvideo->frame_usec_max = 0;
	}
	spin_unlock(&zdev->zdev_lock);

	return 0;
}

static int zynq_trigger_status_fpd(zynq_dev_t *zdev, unsigned long arg)
{
	zynq_trigger_t triggers[ZYNQ_FPD_TRIG_NUM] = {{ 0 }};
	zynq_trigger_t *t;
	zynq_video_t *zvideo;
	unsigned int val;
	unsigned char enabled;
	unsigned char internal;
	int i;

	spin_lock(&zdev->zdev_lock);
	/* 1. check the global trigger status */
	val = zynq_g_reg_read(zdev, ZYNQ_G_CONFIG);
	enabled = (val & ZYNQ_CONFIG_TRIGGER) ? 1 : 0;
	internal = (val & ZYNQ_CONFIG_GPS_SW) ? 1 : 0;

	/* 2. check individual triggers status */
	for (i = 0; i < zdev->zdev_video_cnt; i++) {
		zvideo = &zdev->zdev_videos[i];
		t = &triggers[i];
		if (!zvideo->caps.link_up) {
			t->vnum = -1;
			continue;
		}

		if (zdev->zdev_video_port_map) {
			/* Use the video port number as the id */
			t->id = zdev->zdev_video_port_map[i];
		}
		val = zvideo_reg_read(zvideo, ZYNQ_CAM_CONFIG);
		t->enabled = (enabled && (val & ZYNQ_CAM_EN)) ?
		    1 : 0;
		t->internal = (internal && t->enabled) ? 1 : 0;
		if (t->enabled) {
			val = zvideo_reg_read(zvideo, ZYNQ_CAM_TRIGGER);
			t->fps = GET_BITS(ZYNQ_CAM_TRIG_FPS, val);
		}
		strncpy(t->name, zvideo->caps.name, ZYNQ_VDEV_NAME_LEN);
		t->name[ZYNQ_VDEV_NAME_LEN - 1] = '\0';
		t->vnum = zvideo->vdev.num;
	}
	spin_unlock(&zdev->zdev_lock);

	if (copy_to_user((void __user *)arg, triggers,
	    sizeof(zynq_trigger_t) * zdev->zdev_video_cnt)) {
		return -EFAULT;
	}

	if (zynq_trace_param & ZYNQ_TRACE_PROBE) {
		for (i = 0; i < zdev->zdev_video_cnt; i++) {
			t = &triggers[i];
			zynq_log("[id=%u, fps=%u, internal=%u, "
			    "enabled=%u, vnum=%d, name=%s]\n",
			    t->id, t->fps, t->internal,
			    t->enabled, t->vnum, t->name);
		}
	}

	return 0;
}

/*
 * Trigger support ioctls
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35)
static int zynq_trigger_ioctl(struct inode *inode, struct file *filp,
    unsigned int cmd, unsigned long arg)
#else
static long zynq_trigger_ioctl(struct file *filp, unsigned int cmd,
    unsigned long arg)
#endif
{
	zynq_dev_t *zdev = filp->private_data;
	int err = 0, status;

	switch (cmd) {
	case ZYNQ_IOC_TRIGGER_DEV_NAME:
		zynq_trace(ZYNQ_TRACE_PROBE,
		    "%d: ZYNQ_IOC_TRIGGER_DEV_NAME\n", zdev->zdev_inst);
		if (copy_to_user((void __user *)arg,
		    zdev->zdev_name, ZYNQ_DEV_NAME_LEN)) {
			err = -EFAULT;
			break;
		}
		break;

	case ZYNQ_IOC_TRIGGER_DISABLE:
		zynq_trace(ZYNQ_TRACE_PROBE,
		    "%d: ZYNQ_IOC_TRIGGER_DISABLE\n", zdev->zdev_inst);
		err = zynq_trigger_disable_all(zdev);
		break;

	case ZYNQ_IOC_TRIGGER_ENABLE:
		zynq_trace(ZYNQ_TRACE_PROBE,
		    "%d: ZYNQ_IOC_TRIGGER_ENABLE\n", zdev->zdev_inst);
		err = zynq_trigger_enable_all(zdev, arg, 0, 0);
		break;

	case ZYNQ_IOC_TRIGGER_STATUS:
		zynq_trace(ZYNQ_TRACE_PROBE,
		    "%d: ZYNQ_IOC_TRIGGER_STATUS\n", zdev->zdev_inst);
		err = zynq_trigger_status_fpd(zdev, arg);
		break;

	case ZYNQ_IOC_TRIGGER_STATUS_GPS:
		zynq_trace(ZYNQ_TRACE_PROBE,
		    "%d: ZYNQ_IOC_TRIGGER_STATUS_GPS\n", zdev->zdev_inst);
		spin_lock(&zdev->zdev_lock);
		status = zynq_g_reg_read(zdev, ZYNQ_G_STATUS);
		status = (status & ZYNQ_STATUS_GPS_LOCKED) ? 1 : 0;
		spin_unlock(&zdev->zdev_lock);
		if (copy_to_user((void __user *)arg, &status, sizeof(status))) {
			err = -EFAULT;
			break;
		}
		break;

	case ZYNQ_IOC_TRIGGER_STATUS_PPS:
		zynq_trace(ZYNQ_TRACE_PROBE,
		    "%d: ZYNQ_IOC_TRIGGER_STATUS_PPS\n", zdev->zdev_inst);
		spin_lock(&zdev->zdev_lock);
		status = zynq_g_reg_read(zdev, ZYNQ_G_STATUS);
		status = (status & ZYNQ_STATUS_PPS_LOCKED) ? 1 : 0;
		spin_unlock(&zdev->zdev_lock);
		if (copy_to_user((void __user *)arg, &status, sizeof(status))) {
			err = -EFAULT;
			break;
		}
		break;

	default:
		zynq_err("%d %s: unknown ioctl command\n",
		    zdev->zdev_inst, __FUNCTION__);
		err = -EINVAL;
		break;
	}

	return err;
}

static int zynq_trigger_open(struct inode *inode, struct file *filp)
{
	zynq_dev_t *zdev;

	zdev = container_of(inode->i_cdev, zynq_dev_t, zdev_cdev_trigger);
	filp->private_data = zdev;
	zdev->zdev_version = zynq_g_reg_read(zdev, ZYNQ_G_VERSION);

	zynq_trace(ZYNQ_TRACE_PROBE, "%d %s done.\n",
	    zdev->zdev_inst, __FUNCTION__);
	return 0;
}

static int zynq_trigger_release(struct inode *inode, struct file *filp)
{
	return 0;
}

struct file_operations zynq_trigger_fops = {
	.owner		= THIS_MODULE,
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35)
	.ioctl		= zynq_trigger_ioctl,
#else
	.unlocked_ioctl = zynq_trigger_ioctl,
#endif
	.open		= zynq_trigger_open,
	.release	= zynq_trigger_release
};
