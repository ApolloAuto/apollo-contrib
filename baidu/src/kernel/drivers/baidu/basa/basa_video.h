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

#ifndef _BASA_VIDEO_H_
#define	_BASA_VIDEO_H_

#include <linux/videodev2.h>
#include <media/v4l2-device.h>
#include <media/v4l2-dev.h>
#include <media/v4l2-ctrls.h>
#include <media/videobuf2-core.h>
#include <media/videobuf2-v4l2.h>

#define	ZVIDEO_VBUF_MIN_NUM		4

/* Video frame default parameters */
#define	ZVIDEO_IMAGE_WIDTH		1920
#define	ZVIDEO_IMAGE_HEIGHT		1080
#define	ZVIDEO_EM_DATA_LINES		2
#define	ZVIDEO_EM_STATS_LINES		2
#define	ZVIDEO_WIDTH			ZVIDEO_IMAGE_WIDTH
#define	ZVIDEO_HEIGHT			(ZVIDEO_IMAGE_HEIGHT + \
		ZVIDEO_EM_DATA_LINES + ZVIDEO_EM_STATS_LINES)
#define	ZVIDEO_EXT_META_DATA_BYTES	32
#define	ZVIDEO_BYTES_PER_PIXEL_YUV	2
#define	ZVIDEO_BYTES_PER_PIXEL_RGB	3
#define	ZVIDEO_BYTES_PER_LINE(f)	\
		(ZVIDEO_BYTES_PER_PIXEL_ ## f * ZVIDEO_IMAGE_WIDTH)
#define	ZVIDEO_IMAGE_BYTES(f)		\
		(ZVIDEO_BYTES_PER_LINE(f) * ZVIDEO_IMAGE_HEIGHT)
#define	ZVIDEO_BYTES(f)			\
		(ZVIDEO_BYTES_PER_LINE(f) * ZVIDEO_HEIGHT + \
		 ZVIDEO_EXT_META_DATA_BYTES)

#define	T_ROW(llp, pck)			\
	div_u64((u64)(llp) * USEC_PER_SEC, (pck))
#define	T_FRAME(fll, llp, pck)	\
	mul_u64_u32_div((u64)(llp) * USEC_PER_SEC, (fll), (pck))

#define	ZVIDEO_STATE_STREAMING		0x01
#define	ZVIDEO_STATE_LINK_CHANGE	0x10
#define	ZVIDEO_STATE_CAM_FAULT		0x40
#define	ZVIDEO_STATE_CHAN_FAULT		0x80

#define	ZVIDEO_LINK_CHANGE_DELAY	1000	/* msec */

struct zynq_dev;

typedef struct zynq_video_buffer {
	struct vb2_v4l2_buffer	vbuf;
	struct list_head	list;
	u32			offset;
	unsigned char		*addr;
	unsigned long		size;
} zynq_video_buffer_t;

typedef struct zynq_video {
	int			index;
	zynq_cam_caps_t		caps;
	u8 __iomem		*reg_base;
	struct zynq_dev		*zdev;
	struct zynq_chan	*zchan;
	struct v4l2_device	v4l2_dev;
	struct video_device	vdev;
	struct v4l2_ctrl_handler ctrl_handler;
	struct v4l2_pix_format	format;
	struct vb2_queue	queue;
	struct list_head	buf_list;
	struct list_head	pending_list;
	struct mutex		mlock;	/* Lock for main serialization */
	struct mutex		slock;	/* Lock for vb queue streaming */
	spinlock_t		qlock;	/* Lock for driver owned queues */
	spinlock_t		rlock;	/* lock for rx proc and reset */
	unsigned char		*dev_cfg;
	unsigned int		meta_header_lines;
	unsigned int		meta_footer_lines;

	unsigned int		state;
	unsigned int		input;
	unsigned int		sequence;
	unsigned int		buf_total; /* Total buf num in buf_list */
	unsigned int		buf_avail; /* Available buf num in buf_list */

	unsigned int		fps;
	unsigned int		t_row;
	unsigned int		t_first;
	unsigned int		t_middle;
	unsigned int		t_last;
	unsigned int		frame_interval;
	unsigned int		frame_usec_max;
	unsigned int		frame_err_cnt;
	unsigned int		last_exp_time;
	unsigned int		last_trig_delay;
	unsigned int		last_meta_cnt;
	unsigned int		last_trig_cnt;
	struct timeval		last_tv_intr;
	struct timeval		tv_intr;
	unsigned long		ts_link_change;

	zynq_stats_t		stats[VIDEO_STATS_NUM];
	char			prefix[ZYNQ_LOG_PREFIX_LEN];

	void			(*read_em_data)(struct zynq_video *, void *);
} zynq_video_t;

extern void zvideo_rx_proc(zynq_video_t *zvideo);
extern void zvideo_err_proc(zynq_video_t *zvideo, int ch_err_status);
extern int zvideo_init(zynq_video_t *zvideo);
extern void zvideo_fini(zynq_video_t *zvideo);
extern int zvideo_register_vdev(zynq_video_t *zvideo);
extern void zvideo_unregister_vdev(zynq_video_t *zvideo);
extern void zvideo_link_change(zynq_video_t *zvideo);
extern void zvideo_get_timestamp(struct timeval *tv);
extern void zvideo_watchdog(zynq_video_t *zvideo);

extern struct v4l2_pix_format zvideo_formats[];

extern unsigned int zynq_video_buf_num;
extern unsigned int zynq_video_zero_copy;
extern unsigned int zynq_video_ts_type;
extern int zynq_video_flash_16;
extern int zynq_video_pin_swap;

#endif	/* _BASA_VIDEO_H_ */
