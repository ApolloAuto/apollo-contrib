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
#include <linux/delay.h>
#include <linux/time.h>
#include <linux/hrtimer.h>
#include <linux/kthread.h>
#include <linux/sched.h>

#include "basa.h"

#define	ZYNQ_GPS_SYNC_TIME_MAX	(100*24*3600)	/* 100 days */
#define	ZYNQ_GPS_SYNC_TIME_DEFAULT	60	/* 1 minute */

static zynq_dev_t *zynq_gps_master_dev = NULL;
static struct task_struct *zynq_gps_taskp = NULL;
static struct completion zynq_gps_wait;

/*
 * GPS sync time interval in seconds: 0 to disable; default 60.
 */
static unsigned int zynq_gps_sync_time = ZYNQ_GPS_SYNC_TIME_DEFAULT;
/*
 * We define two water marks for GPS time check. The water marks are calculated
 * by multiplying the system time drift value (in unit of microsecond) with
 * specific factors.
 */
static unsigned int zynq_gps_high_factor = 100;
static unsigned int zynq_gps_low_factor = 10;

static int zynq_get_gps_time(zynq_dev_t *zdev, u32 *gps_val)
{
	spin_lock(&zdev->zdev_lock);
	gps_val[0] = zynq_g_reg_read(zdev, ZYNQ_G_NTP_LO);
	if (gps_val[0] == -1) {
		spin_unlock(&zdev->zdev_lock);
		zynq_err("%d %s: Failed to read GPS time\n",
		    zdev->zdev_inst, __FUNCTION__);
		return -1;
	}

	if (!(gps_val[0] & ZYNQ_NTP_LO_VALID)) {
		spin_unlock(&zdev->zdev_lock);
		zynq_err("%d %s: GPS time is not valid\n",
		    zdev->zdev_inst, __FUNCTION__);
		return -1;
	}

	gps_val[1] = zynq_g_reg_read(zdev, ZYNQ_G_NTP_HI);
	gps_val[2] = zynq_g_reg_read(zdev, ZYNQ_G_NTP_DATE);
	spin_unlock(&zdev->zdev_lock);

	zynq_trace(ZYNQ_TRACE_GPS,
	    "%d %s: gps_ddmmyy=0x%x, gps_hhmmss=0x%x, gps_msusns_valid=0x%x\n",
	    zdev->zdev_inst, __FUNCTION__, gps_val[2], gps_val[1], gps_val[0]);

	return 0;
}

static int zynq_get_gprmc(zynq_dev_t *zdev, u32 *gprmc_val)
{
	/* 'CAFE': GPS is connected but not locked */
	u32 inval_gprmc = 0x45464143;
	/* 'DEAD': GPS is not connected */
	u32 dead_gprmc = 0x44414544;
	int i;

	spin_lock(&zdev->zdev_lock);
	gprmc_val[0] = ntohl(zynq_g_reg_read(zdev, ZYNQ_G_GPRMC));
	if (gprmc_val[0] == -1 || gprmc_val[0] == dead_gprmc) {
		spin_unlock(&zdev->zdev_lock);
		zynq_err("%d %s: GPS is not connected\n",
		    zdev->zdev_inst, __FUNCTION__);
		return -1;
	}

	if (gprmc_val[0] == inval_gprmc) {
		spin_unlock(&zdev->zdev_lock);
		zynq_err("%d %s: GPS is not locked\n",
		    zdev->zdev_inst, __FUNCTION__);
		return -1;
	}

	for (i = 1; i < ZYNQ_GPS_GPRMC_VAL_SZ/4; i++) {
		gprmc_val[i] = ntohl(zynq_g_reg_read(zdev, ZYNQ_G_GPRMC));
	}
	spin_unlock(&zdev->zdev_lock);

	return 0;
}

/*
 * convert the GPS time from FPGA registers to "struct timespec"
 */
static int zynq_gps_time_ts(zynq_dev_t *zdev, struct timespec *ts)
{
	u32 gps_val[3];

	if (zynq_get_gps_time(zdev, gps_val)) {
		return -1;
	}

	ts->tv_sec = mktime(2000 + (gps_val[2] & 0xff),
	    (gps_val[2] & 0xff00) >> 8, (gps_val[2] & 0xff0000) >> 16,
	    (gps_val[1] & 0xff0000) >> 16, (gps_val[1] & 0xff00) >> 8,
	    gps_val[1] & 0xff);
	ts->tv_nsec = ((gps_val[0] & 0xfe) << 2) +
	    ((gps_val[0] >> 8) & 0xfff) * NSEC_PER_USEC +
	    ((gps_val[0] >> 20) & 0xfff) * NSEC_PER_MSEC;

	return 0;
}

static int zynq_gps_ts_check(zynq_dev_t *zdev, struct timespec *ts)
{
	struct timespec ts_now;
	unsigned int delay;
	int thresh;
	int diff;

	ktime_get_real_ts(&ts_now);

	if (zynq_gps_time_ts(zdev, ts)) {
		return -1;
	}

	/* check the gap between system time and gps time in usec */
	diff = (ts->tv_sec - ts_now.tv_sec) * USEC_PER_SEC +
	    (ts->tv_nsec - ts_now.tv_nsec) / NSEC_PER_USEC;

	zynq_log("%d SYS time: %ld.%09ld, GPS time: %ld.%09ld, %dus\n",
	    zdev->zdev_inst, ts_now.tv_sec, ts_now.tv_nsec,
	    ts->tv_sec, ts->tv_nsec, diff);

	/*
	 * Check the validity of the GPS time.
	 *
	 * When the gap between the system time and gps time is more than
	 * the low water mark, the system time sync is still performed
	 * while a warning is issued.
	 *
	 * When the gap is more than the high water mark, we'll fail the
	 * system time sync.
	 */

	/* If it is the first sync, bypass the time check */
	if (!zdev->zdev_gps_ts.tv_sec && !zdev->zdev_gps_ts.tv_nsec) {
		return 0;
	}

	/* Firstly find the delay since the last successful sync in seconds */
	delay = ts_now.tv_sec - zdev->zdev_gps_ts.tv_sec + 1;

	/* Check the high water mark in usec */
	thresh = zynq_gps_high_factor * delay * ZYNQ_SYS_TIME_DRIFT;
	if ((diff > thresh) || (diff < -thresh)) {
		zynq_err("%d %s: GPS/SYS gap %dus exceeds high threshold %dus "
		    "SYS/GPS time sync is aborted.\n",
		    zdev->zdev_inst, __FUNCTION__, diff, thresh);
		return -1;
	}

	/* Check the low water mark in usec */
	thresh = zynq_gps_low_factor * delay * ZYNQ_SYS_TIME_DRIFT;
	if ((diff > thresh) || (diff < -thresh)) {
		zynq_err("%d %s: GPS/SYS gap %dus exceeds low threshold %dus\n",
		    zdev->zdev_inst, __FUNCTION__, diff, thresh);
	}

	return 0;
}

static void zynq_do_gps_sync(zynq_dev_t *zdev)
{
	struct timespec ts;

	/* get current GPS time */
	if (zynq_gps_ts_check(zdev, &ts)) {
		return;
	}

	/* update system time */
	if (do_settimeofday(&ts)) {
		zynq_err("%d %s: failed to set system time. GPS sync aborted!",
		    zdev->zdev_inst, __FUNCTION__);
		return;
	}

	zdev->zdev_gps_ts = ts;
}

/*
 * GPS sync thread: sync system time to GPS time periodically per
 * zynq_gps_sync_time seconds. (Note, we can't use a hrtimer to
 * do this as there are side effects to do do_settimeofday() via
 * a hrtimer per my testing.)
 */
static int zynq_gps_sync_thread(void *arg)
{
	zynq_dev_t *zdev;
	unsigned int delay_time;
	long result;

	while (!kthread_should_stop()) {
		zdev = zynq_gps_master_dev;

		/* sync system time to GPS time */
		zynq_do_gps_sync(zdev);

		/*
		 * Wait for specified zynq_gps_sync_time seconds.
		 */
		delay_time = zynq_gps_sync_time;
		if (delay_time == 0) {
			delay_time = ZYNQ_GPS_SYNC_TIME_MAX;
		}
		result = wait_for_completion_interruptible_timeout(
		    &zynq_gps_wait, msecs_to_jiffies(delay_time * 1000));

		if (result > 0) {
			reinit_completion(&zynq_gps_wait);
		} else if (result != 0) {
			/* Interrupted or module is being unloaded */
			break;
		}
	}

	return 0;
}

/*
 * create the GPS sync thread and schedule it with the second highest priority
 * that is lower than the watchdog (99) and higher than the interrupts (50).
 */
static void zynq_gps_thread_init(zynq_dev_t *zdev)
{
	struct sched_param param = { .sched_priority = 60 };

	init_completion(&zynq_gps_wait);
	zynq_gps_taskp = kthread_run(zynq_gps_sync_thread, NULL,
	    "zynq_gps_sync_thread");
	/* set the thread scheduling class as SCHED_FIFO  */
	sched_setscheduler(zynq_gps_taskp, SCHED_FIFO, &param);
}

/*
 * destroy the GPS sync thread
 */
static void zynq_gps_thread_fini(zynq_dev_t *zdev)
{
	complete(&zynq_gps_wait);
	if (zynq_gps_taskp) {
		kthread_stop(zynq_gps_taskp);
	}
}

void zynq_gps_init(zynq_dev_t *zdev)
{
	if (zynq_fwupdate_param) {
		return;
	}

	spin_lock(&zynq_g_lock);
	if (zynq_gps_master_dev != NULL) {
		spin_unlock(&zynq_g_lock);
		return;
	}

	zynq_trace(ZYNQ_TRACE_PROBE, "%d %s: GPS master device set\n",
	    zdev->zdev_inst, __FUNCTION__);
	zynq_gps_master_dev = zdev;
	spin_unlock(&zynq_g_lock);

	zynq_gps_thread_init(zdev);
}

void zynq_gps_fini(zynq_dev_t *zdev)
{
	spin_lock(&zynq_g_lock);
	if (zdev != zynq_gps_master_dev) {
		spin_unlock(&zynq_g_lock);
		return;
	}
	zynq_gps_master_dev = NULL;
	spin_unlock(&zynq_g_lock);

	zynq_gps_thread_fini(zdev);
}

void zynq_gps_pps_changed(zynq_dev_t *zdev)
{
	u32 status;

	/* notify the waiting thread */
	complete(&zdev->zdev_gpspps_event_comp);

	status = zynq_g_reg_read(zdev, ZYNQ_G_STATUS);
	zynq_log("%d %s: PPS is %s, GPS %s locked\n",
	    zdev->zdev_inst, __FUNCTION__,
	    (status & ZYNQ_STATUS_PPS_LOCKED) ? "ON" : "OFF",
	    (status & ZYNQ_STATUS_GPS_LOCKED) ? "is" : "is NOT");

	if (!(status & ZYNQ_STATUS_GPS_LOCKED)) {
		ZYNQ_STATS_LOGX(zdev, DEV_STATS_GPS_UNLOCK, 1, 0);
	}
}

/*
 * GPS support ioctls
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35)
static int zynq_gps_ioctl(struct inode *inode, struct file *filp,
		unsigned int cmd, unsigned long arg)
#else
static long zynq_gps_ioctl(struct file *filp, unsigned int cmd,
		unsigned long arg)
#endif
{
	zynq_dev_t *zdev = filp->private_data;
	int err;

	switch (cmd) {
	case ZYNQ_IOC_GPS_GET:
	{
		u32 gps_val[3];

		if (zynq_get_gps_time(zdev, gps_val)) {
			err = -EAGAIN;
			break;
		}

		if (copy_to_user((void __user *)arg, gps_val,
		    ZYNQ_GPS_VAL_SZ)) {
			err = -EFAULT;
			break;
		}

		return 0;
	}

	case ZYNQ_IOC_GPS_GPRMC_GET:
	{
		u8 gprmc_buf[ZYNQ_GPS_GPRMC_VAL_SZ + 1];
		u32 *gprmc_val = (u32 *)gprmc_buf;

		if (zynq_get_gprmc(zdev, gprmc_val)) {
			err = -EAGAIN;
			break;
		}

		gprmc_buf[ZYNQ_GPS_GPRMC_VAL_SZ] = '\0';

		if (copy_to_user((void __user *)arg, gprmc_val,
		    ZYNQ_GPS_GPRMC_VAL_SZ)) {
			err = -EFAULT;
			break;
		}
		zynq_trace(ZYNQ_TRACE_GPS, "%s GPRMC time: %s\n", __FUNCTION__,
		    (char *)gprmc_val);
		return 0;
	}

	default:
		err = -EINVAL;
		break;
	}

	zynq_trace(ZYNQ_TRACE_PROBE, "%s: error = %d\n", __FUNCTION__, err);
	return err;
}

static int zynq_gps_open(struct inode *inode, struct file *filp)
{
	zynq_dev_t *zdev;

	zdev = container_of(inode->i_cdev, zynq_dev_t, zdev_cdev_gps);
	filp->private_data = zdev;

	zynq_trace(ZYNQ_TRACE_PROBE, "%s done.", __FUNCTION__);
	return 0;
}

static int zynq_gps_release(struct inode *inode, struct file *filp)
{
	return 0;
}

struct file_operations zynq_gps_fops = {
	.owner		= THIS_MODULE,
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35)
	.ioctl		= zynq_gps_ioctl,
#else
	.unlocked_ioctl = zynq_gps_ioctl,
#endif
	.open		= zynq_gps_open,
	.release	= zynq_gps_release
};

module_param_named(gpssynctime, zynq_gps_sync_time, uint, S_IRUGO|S_IWUSR);
MODULE_PARM_DESC(gpssynctime, "GPS sync interval in seconds");
module_param_named(gpshifactor, zynq_gps_high_factor, uint, S_IRUGO|S_IWUSR);
MODULE_PARM_DESC(gpshifactor, "high water mark factor for GPS time check");
module_param_named(gpslofactor, zynq_gps_low_factor, uint, S_IRUGO|S_IWUSR);
MODULE_PARM_DESC(gpslofactor, "low water mark factor for GPS time check");
