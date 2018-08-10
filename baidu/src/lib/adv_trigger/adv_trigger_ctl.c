/******************************************************************************
 * Copyright 2018 The Apollo Authors. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *****************************************************************************/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/videodev2.h>

#include "adv_trigger.h"

enum camera_type {
	CAMERA_TYPE_ALL = 0,
	CAMERA_TYPE_FPD
};

struct adv_trigger {
	const char		*video_path;
	enum camera_type	type;
	int			vnum;
	int			index;		/* index of the trigger */
	int			zdev_index;	/* index of the trigger dev */
	int			fpd_num;	/* number of fpd-link cameras */
	int			vfd;
	int			zfd[ZYNQ_TRIGGER_DEV_NUM];
};

#define	DRIVER_NAME_BASA	"basa"
#define	TRIGGER_DEV_PATH	"/dev/zynq_trigger"

#define	SYSFS_VIDEO		"video"
#define	SYSFS_VIDEO_PATH	"/sys/class/video4linux"
#define	SYSFS_VIDEO_DEV_DRV	"device/driver"

#define	PATH_LEN		128

#define	ADV_TRIGGER_CTL_VER	"3.0.0.1"

#define	adv_plat_log_fn(s...)	syslog(LOG_ERR, s)

const char *adv_trigger_version(void)
{
	return ADV_TRIGGER_CTL_VER;
}

static int adv_trigger_device_type(const char *driver)
{
	if (!strcmp(driver, DRIVER_NAME_BASA)) {
		return CAMERA_TYPE_FPD;
	} else {
		adv_plat_log_fn("%s: unknown camera driver: %s\n",
		    LIB_NAME, driver);
		return -1;
	}
}

static int adv_trigger_scan_video(struct adv_trigger *at, char *video_name)
{
	char path[PATH_LEN];
	char driver[PATH_LEN];
	char *p;
	int type;
	int ret;

	/* Get the driver and video device type */
	(void) snprintf(path, PATH_LEN, "%s/%s/%s",
	    SYSFS_VIDEO_PATH, video_name, SYSFS_VIDEO_DEV_DRV);
	memset(driver, 0, sizeof(driver));
	ret = readlink(path, driver, sizeof(driver) - 1);
	if (ret < 0) {
		adv_plat_log_fn("%s: failed to read link %s! %s\n",
		    LIB_NAME, path, strerror(errno));
		return -1;
	}

	p = strrchr(driver, '/');
	if (p) {
		p++;
	} else {
		p = driver;
	}

	type = adv_trigger_device_type(p);

	switch (type) {
	case CAMERA_TYPE_FPD:
		at->fpd_num++;
		break;
	default:
		break;
	}

	return 0;
}

/*
 * Scan the sysfs to get video device information
 */
static int adv_trigger_scan_sysfs(struct adv_trigger *at)
{
	char *cwd;
	DIR *dp;
	struct dirent *ent;
	struct stat st;
	int ret = 0;

	dp = opendir(SYSFS_VIDEO_PATH);
	if (!dp) {
		adv_plat_log_fn("%s: failed to open dir %s! %s\n",
		    LIB_NAME, SYSFS_VIDEO_PATH, strerror(errno));
		return -1;
	}

	cwd = getcwd(NULL, 0);
	if (chdir(SYSFS_VIDEO_PATH)) {
		adv_plat_log_fn("%s: failed to change dir to %s! %s\n",
		    LIB_NAME, SYSFS_VIDEO_PATH, strerror(errno));
		(void) closedir(dp);
		return -1;
	}

	while ((ent = readdir(dp))) {
		lstat(ent->d_name, &st);
		if (S_ISLNK(st.st_mode)) {
			if ((ret = adv_trigger_scan_video(at, ent->d_name))) {
				break;
			}
		}
	}

	if (cwd) {
		chdir(cwd);
		free(cwd);
	}
	(void) closedir(dp);

	return ret;
}

static void adv_trigger_fini(struct adv_trigger *at)
{
	int i;

	for (i = 0; i < ZYNQ_TRIGGER_DEV_NUM; i++) {
		if (at->zfd[i] < 0) {
			continue;
		}
		(void) close(at->zfd[i]);
	}

	if (at->vfd >= 0) {
		(void) close(at->vfd);
	}
}

static void adv_trigger_params_init(struct adv_trigger *at)
{
	int i;

	memset(at, 0, sizeof(struct adv_trigger));

	for (i = 0; i < ZYNQ_TRIGGER_DEV_NUM; i++) {
		at->zfd[i] = -1;
	}
}

static int adv_trigger_vdev_init(struct adv_trigger *at,
		const char *video_path)
{
	struct v4l2_capability caps;
	struct stat st;
	char path[PATH_LEN] = { 0 };
	const char *video_name;
	int fd;

	at->video_path = video_path;
	at->type = CAMERA_TYPE_ALL;
	at->vnum = -1;
	at->vfd = -1;

	if (video_path == NULL) {
		return 0;
	}

	fd = open(video_path, O_RDWR);
	if (fd < 0) {
		adv_plat_log_fn("%s: failed to open device %s! %s\n",
		    LIB_NAME, video_path, strerror(errno));
		return -1;
	}

	if (ioctl(fd, VIDIOC_QUERYCAP, &caps)) {
		adv_plat_log_fn("%s: failed to query capability of %s! "
		    "%s\n", LIB_NAME, video_path, strerror(errno));
		close(fd);
		return -1;
	}

	at->type = adv_trigger_device_type((const char *)caps.driver);
	if (at->type < 0) {
		adv_plat_log_fn(
		    "%s: unsupported device %s %s driver %s\n",
		    LIB_NAME, video_path,
		    (const char *)caps.card,
		    (const char *)caps.driver);
		close(fd);
		return -1;
	}

	at->vfd = fd;

	lstat(video_path, &st);
	if (S_ISLNK(st.st_mode)) {
		if (readlink(video_path, path, sizeof(path) - 1) < 0) {
			adv_plat_log_fn("%s: failed to read link %s! %s\n",
			    LIB_NAME, video_path, strerror(errno));
			return -1;
		}
		video_path = path;
	}

	video_name = strrchr(video_path, '/');
	if (video_name) {
		video_name++;
	} else {
		video_name = video_path;
	}
	if (sscanf(video_name, SYSFS_VIDEO "%d", &at->vnum) != 1) {
		adv_plat_log_fn("%s: failed to read video number %s! %s\n",
		    LIB_NAME, video_name, strerror(errno));
		return -1;
	}

	return 0;
}

static int adv_trigger_zdev_init(struct adv_trigger *at)
{
	char zdev_path[32];
	int fd;
	int zfd;
	int i;

	zfd = -1;
	for (i = ZYNQ_TRIGGER_DEV_NUM - 1; i >= 0; i--) {
		(void) snprintf(zdev_path, sizeof(zdev_path),
		    "%s%d", TRIGGER_DEV_PATH, i);
		fd = open(zdev_path, O_RDWR);
		if (fd >= 0) {
			at->zfd[i] = fd;
			zfd = fd;
		}
	}

	return zfd;
}

static int adv_trigger_init(struct adv_trigger *at, const char *video_path)
{
	adv_trigger_params_init(at);

	/*
	 * Read the video device information
	 */
	if (adv_trigger_vdev_init(at, video_path)) {
		return -1;
	}

	if (at->type == CAMERA_TYPE_FPD) {
		/* Nothing to do for FPD-link camera */
		return 0;
	}

	/*
	 * Open all trigger devices and save the file descriptors
	 */
	if (adv_trigger_zdev_init(at) < 0) {
		goto init_fail;
	}

	if (adv_trigger_scan_sysfs(at)) {
		goto init_fail;
	}

	return 0;

init_fail:
	adv_trigger_fini(at);
	return -1;
}

static int adv_trigger_enable_fpd(struct adv_trigger *at,
		unsigned char fps, unsigned char internal)
{
	zynq_trigger_t trigger[1] = {{ 0 }};

	trigger->fps = fps;
	trigger->internal = internal;

	if (ioctl(at->vfd, ZYNQ_IOC_TRIGGER_ENABLE_ONE, trigger)) {
		adv_plat_log_fn("%s: failed to enable trigger for %s! %s\n",
		    LIB_NAME, at->video_path, strerror(errno));
		return -1;
	}

	adv_plat_log_fn("%s: trigger enabled for %s, fps = %d%s\n",
	    LIB_NAME, at->video_path, trigger->fps,
	    (trigger->internal) ? ", internal PPS" : "");

	return 0;
}

static int adv_trigger_enable_all(struct adv_trigger *at,
		unsigned char fps, unsigned char internal)
{
	zynq_trigger_t trigger[1] = {{ 0 }};
	int i;
	int ret = -1;

	trigger->fps = fps;
	trigger->internal = internal;

	for (i = 0; i < ZYNQ_TRIGGER_DEV_NUM; i++) {
		if (at->zfd[i] < 0) {
			continue;
		}
		if ((ret = ioctl(at->zfd[i],
		    ZYNQ_IOC_TRIGGER_ENABLE, trigger))) {
			break;
		}
	}
	if (ret) {
		adv_plat_log_fn("%s: failed to enable all triggers! %s\n",
		    LIB_NAME, strerror(errno));
		return -1;
	}

	adv_plat_log_fn("%s: all trigger enabled, fps = %d%s\n",
	    LIB_NAME, trigger->fps,
	    (trigger->internal) ? ", internal PPS" : "");

	return 0;
}

static int adv_trigger_disable_fpd(struct adv_trigger *at)
{
	if (ioctl(at->vfd, ZYNQ_IOC_TRIGGER_DISABLE_ONE)) {
		adv_plat_log_fn("%s: failed to disable trigger for %s! %s\n",
		    LIB_NAME, at->video_path, strerror(errno));
		return -1;
	}

	adv_plat_log_fn("%s: trigger disabled for %s\n",
	    LIB_NAME, at->video_path);

	return 0;
}

static int adv_trigger_disable_all(struct adv_trigger *at)
{
	int i;
	int ret = -1;

	/*
	 * Disable all triggers
	 */
	for (i = 0; i < ZYNQ_TRIGGER_DEV_NUM; i++) {
		if (at->zfd[i] < 0) {
			continue;
		}
		if ((ret = ioctl(at->zfd[i],
		    ZYNQ_IOC_TRIGGER_DISABLE, NULL))) {
			break;
		}
	}
	if (ret) {
		adv_plat_log_fn("%s: failed to disable all triggers! %s\n",
		    LIB_NAME, strerror(errno));
		return -1;
	}

	adv_plat_log_fn("%s: all trigger disabled\n", LIB_NAME);

	return 0;
}

int adv_trigger_enable(const char *video_path,
		unsigned char fps, unsigned char internal)
{
	struct adv_trigger at;
	int ret;

	ret = adv_trigger_init(&at, video_path);
	if (ret) {
		return ret;
	}

	switch (at.type) {
	case CAMERA_TYPE_FPD:
		ret = adv_trigger_enable_fpd(&at, fps, internal);
		break;
	default:
		ret = adv_trigger_enable_all(&at, fps, internal);
		break;
	}

	adv_trigger_fini(&at);

	return ret;
}

int adv_trigger_disable(const char *video_path)
{
	struct adv_trigger at;
	int ret;

	ret = adv_trigger_init(&at, video_path);
	if (ret) {
		return ret;
	}

	switch (at.type) {
	case CAMERA_TYPE_FPD:
		ret = adv_trigger_disable_fpd(&at);
		break;
	default:
		ret = adv_trigger_disable_all(&at);
		break;
	}

	adv_trigger_fini(&at);

	return ret;
}

int adv_trigger_get_status(struct adv_trigger_status *s)
{
	struct adv_trigger at;
	zynq_trigger_t *t;
	int result;
	int i, j;
	int ret;

	memset(s, 0, sizeof(struct adv_trigger_status));

	for (i = 0; i < ZYNQ_TRIGGER_DEV_NUM; i++) {
		for (j = 0; j < ZYNQ_FPD_TRIG_NUM; j++) {
			t = &s->status[i].fpd_triggers[j];
			t->vnum = -1;
		}
	}

	ret = adv_trigger_init(&at, NULL);
	if (ret) {
		return ret;
	}

	for (i = 0; i < ZYNQ_TRIGGER_DEV_NUM; i++) {
		if (at.zfd[i] < 0) {
			s->status[i].zdev_name[0] = '\0';
			continue;
		}

		ret = ioctl(at.zfd[i],
		    ZYNQ_IOC_TRIGGER_DEV_NAME, s->status[i].zdev_name);
		if (ret) {
			adv_plat_log_fn(LIB_NAME
			    ": failed to get trigger device name on %s%d! %s\n",
			    TRIGGER_DEV_PATH, i, strerror(errno));
			break;
		}

		ret = ioctl(at.zfd[i], ZYNQ_IOC_TRIGGER_STATUS_GPS, &result);
		if (ret) {
			adv_plat_log_fn(LIB_NAME
			    ": failed to get GPS status on %s%d! %s\n",
			    TRIGGER_DEV_PATH, i, strerror(errno));
			break;
		}
		if (result) {
			s->status[i].flags |= FLAG_GPS_VALID;
		}

		ret = ioctl(at.zfd[i], ZYNQ_IOC_TRIGGER_STATUS_PPS, &result);
		if (ret) {
			adv_plat_log_fn(LIB_NAME
			    ": failed to get PPS status on %s%d! %s\n",
			    TRIGGER_DEV_PATH, i, strerror(errno));
			break;
		}
		if (result) {
			s->status[i].flags |= FLAG_PPS_VALID;
		}

		if (at.fpd_num > 0) {
			ret = ioctl(at.zfd[i], ZYNQ_IOC_TRIGGER_STATUS,
			    s->status[i].fpd_triggers);
			if (ret) {
				adv_plat_log_fn(LIB_NAME ": failed to get "
				    "FPD-link trigger status on %s%d! %s\n",
				    TRIGGER_DEV_PATH, i, strerror(errno));
				break;
			}
		}
	}

	adv_trigger_fini(&at);

	return ret;
}

static int adv_trigger_delay_fpd(struct adv_trigger *at, int action,
		unsigned int *trigger_delay, unsigned int *exp_time)
{
	zynq_trigger_delay_t delay_arg;
	int status;
	int ret = 0;

	delay_arg.vnum = at->vnum;
	/* Set delay */
	if (action == 1) {
		delay_arg.trigger_delay = *trigger_delay;

		status = ioctl(at->vfd,
		    ZYNQ_IOC_TRIGGER_DELAY_SET, &delay_arg);
		if (status == 0) {
			adv_plat_log_fn("%s: video%d SET trigger_delay=%uus\n",
			    LIB_NAME, delay_arg.vnum, delay_arg.trigger_delay);
		} else {
			adv_plat_log_fn("%s: video%d "
			    "SET trigger delay failed! %s\n",
			    LIB_NAME, delay_arg.vnum, strerror(errno));
			ret = -1;
		}
	}

	/* Get delay and exposure time */
	status = ioctl(at->vfd,
	    ZYNQ_IOC_TRIGGER_DELAY_GET, &delay_arg);
	if (status == 0) {
		adv_plat_log_fn("%s: video%d GET trigger_delay=%uus, "
		    "exp_time=%uus\n", LIB_NAME, delay_arg.vnum,
		    delay_arg.trigger_delay, delay_arg.exposure_time);

		*trigger_delay = delay_arg.trigger_delay;
		*exp_time = delay_arg.exposure_time;
	} else {
		adv_plat_log_fn("%s: video%d GET trigger delay failed! %s\n",
		    LIB_NAME, delay_arg.vnum, strerror(errno));
		ret = -1;
	}

	return ret;
}

int adv_trigger_delay_ctl(const char *video_path, int action,
		unsigned int *trigger_delay, unsigned int *exp_time)
{
	struct adv_trigger at;
	int ret = 0;

	adv_trigger_params_init(&at);

	if (adv_trigger_vdev_init(&at, video_path)) {
		return -1;
	}

	switch (at.type) {
	case CAMERA_TYPE_FPD:
		ret = adv_trigger_delay_fpd(&at, action,
		    trigger_delay, exp_time);
		break;
	default:
		adv_plat_log_fn("%s: invalid video path\n", LIB_NAME);
		ret = -1;
		break;
	}

	adv_trigger_fini(&at);

	return ret;
}
