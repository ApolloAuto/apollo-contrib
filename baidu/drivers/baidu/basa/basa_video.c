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
#include <linux/delay.h>
#include <linux/kthread.h>
#include <media/v4l2-ioctl.h>
#include <media/v4l2-event.h>
#include <media/videobuf2-vmalloc.h>
#include "basa.h"

/* Video configurations */
static unsigned int zynq_video_err_tolerant = 0;
static unsigned int zynq_video_err_thresh = 4;
static unsigned int zynq_video_format = 0;

int zynq_video_pin_swap = -1;
int zynq_video_flash_16 = -1;
unsigned int zynq_video_ts_type = CAM_CAP_TIMESTAMP_FPGA;
unsigned int zynq_video_buf_num = ZVIDEO_VBUF_MIN_NUM;
unsigned int zynq_video_zero_copy = 1;

struct v4l2_pix_format zvideo_formats[2] = {
	{
		.width		= ZVIDEO_WIDTH,
		.height		= ZVIDEO_HEIGHT,
		.pixelformat	= V4L2_PIX_FMT_YUYV,
		.field		= V4L2_FIELD_NONE,
		.bytesperline	= ZVIDEO_BYTES_PER_LINE(YUV),
		.sizeimage	= ZVIDEO_BYTES(YUV),
		.colorspace	= V4L2_COLORSPACE_JPEG,
		.priv		= 0
	},
	{
		.width		= ZVIDEO_WIDTH,
		.height		= ZVIDEO_HEIGHT,
		.pixelformat	= V4L2_PIX_FMT_RGB24,
		.field		= V4L2_FIELD_NONE,
		.bytesperline	= ZVIDEO_BYTES_PER_LINE(RGB),
		.sizeimage	= ZVIDEO_BYTES(RGB),
		.colorspace	= V4L2_COLORSPACE_SRGB,
		.priv		= 0
	}
};

static const char zvideo_stats_label[VIDEO_STATS_NUM][ZYNQ_STATS_LABEL_LEN] = {
	"Frames received",
	"Reset count",
	"Trigger pulse error",
	"Link change",
	"DMA Rx buffer full",
	"FPD link unlock",
	"FIFO full",
	"Trigger count mismatch",
	"Metadata count mismatch",
	"Frame gap error",
	"Frame format error",
	"Frame too short",
	"Frame too long",
	"Frame corrupted",
	"Frame drop",
	"No video buffer"
};

static const char ts_label_host[] = "HOST";
static const char ts_label_formation[] = "EXP";
static const char ts_label_trigger[] = "TRIG";
static const char ts_label_fpga[] = "FPGA";

/*
 * Set the data ranges that will be used for copying the image data
 * with all meta data excluded.
 */
static void zvideo_set_data_range(zynq_video_t *zvideo)
{
	unsigned long offset;

	offset = zvideo->format.bytesperline * zvideo->meta_header_lines;
	zvideo->data_range[0] = offset;
	offset += zvideo->format.bytesperline * ZVIDEO_IMAGE_HEIGHT;
	zvideo->data_range[1] = offset;
	offset += zvideo->format.bytesperline * zvideo->meta_footer_lines;
	zvideo->data_range[2] = offset;
	offset += ZVIDEO_EXT_META_DATA_BYTES;
	zvideo->data_range[3] = offset;
}

static void zvideo_set_format(zynq_video_t *zvideo)
{
	zvideo->format = zvideo_formats[zynq_video_format];
	zvideo->meta_header_lines = ZVIDEO_EM_DATA_LINES;
	zvideo->meta_footer_lines = ZVIDEO_EM_STATS_LINES;

	if (zvideo->caps.embedded_data != 1) {
		zvideo->meta_header_lines = 0;
		zvideo->meta_footer_lines = 0;
		zvideo->format.width = ZVIDEO_IMAGE_WIDTH;
		zvideo->format.height = ZVIDEO_IMAGE_HEIGHT;
		zvideo->format.sizeimage = zvideo->format.bytesperline *
		    zvideo->format.height + ZVIDEO_EXT_META_DATA_BYTES;
	}

	zvideo_set_data_range(zvideo);
}

static inline zynq_video_buffer_t *vb_to_zvb(struct vb2_buffer *vb)
{
	return container_of(to_vb2_v4l2_buffer(vb),
	    zynq_video_buffer_t, vbuf);
}

static inline unsigned char *zvideo_buffer_vaddr(zynq_video_buffer_t *buf)
{
	return buf->addr;
}

static inline unsigned long zvideo_buffer_size(zynq_video_buffer_t *buf)
{
	return buf->size;
}

static void zvideo_config(zynq_video_t *zvideo)
{
	zynq_dev_t *zdev = zvideo->zdev;
	u32 cam_cfg;

	zynq_trace(ZYNQ_TRACE_PROBE, "%d vid%d %s: zvideo=0x%p\n",
	    zdev->zdev_inst, zvideo->index, __FUNCTION__, zvideo);

	spin_lock(&zdev->zdev_lock);
	cam_cfg = zvideo_reg_read(zvideo, ZYNQ_CAM_CONFIG);

	cam_cfg = SET_BITS(ZYNQ_CAM_HEADER, cam_cfg, zvideo->meta_header_lines);
	cam_cfg = SET_BITS(ZYNQ_CAM_FOOTER, cam_cfg, zvideo->meta_footer_lines);

	if (zvideo->format.pixelformat == V4L2_PIX_FMT_RGB24) {
		cam_cfg |= ZYNQ_CAM_RGB_YUV_SEL;
	} else {
		cam_cfg &= ~ZYNQ_CAM_RGB_YUV_SEL;
	}
	if (zynq_video_ts_type == CAM_CAP_TIMESTAMP_TRIGGER) {
		cam_cfg &= ~ZYNQ_CAM_TIMESTAMP_RCV;
	} else {
		cam_cfg |= ZYNQ_CAM_TIMESTAMP_RCV;
	}
	cam_cfg |= ZYNQ_CAM_COLOR_SWAP;

	zvideo_reg_write(zvideo, ZYNQ_CAM_CONFIG, cam_cfg);
	spin_unlock(&zdev->zdev_lock);
}

void zvideo_enable(zynq_video_t *zvideo)
{
	zynq_dev_t *zdev = zvideo->zdev;
	u32 cam_cfg;

	zynq_trace(ZYNQ_TRACE_PROBE, "%d vid%d %s: zvideo=0x%p\n",
	    zdev->zdev_inst, zvideo->index, __FUNCTION__, zvideo);

	/* Enable the trigger */
	spin_lock(&zdev->zdev_lock);
	cam_cfg = zvideo_reg_read(zvideo, ZYNQ_CAM_CONFIG);
	cam_cfg |= ZYNQ_CAM_EN;
	zvideo_reg_write(zvideo, ZYNQ_CAM_CONFIG, cam_cfg);
	spin_unlock(&zdev->zdev_lock);
}

void zvideo_disable(zynq_video_t *zvideo)
{
	zynq_dev_t *zdev = zvideo->zdev;
	u32 cam_cfg;

	zynq_trace(ZYNQ_TRACE_PROBE, "%d vid%d %s: zvideo=0x%p\n",
	    zdev->zdev_inst, zvideo->index, __FUNCTION__, zvideo);

	/* Disable the trigger */
	spin_lock(&zdev->zdev_lock);
	cam_cfg = zvideo_reg_read(zvideo, ZYNQ_CAM_CONFIG);
	cam_cfg &= ~ZYNQ_CAM_EN;
	zvideo_reg_write(zvideo, ZYNQ_CAM_CONFIG, cam_cfg);
	spin_unlock(&zdev->zdev_lock);
}

static void zvideo_cam_reset(zynq_video_t *zvideo)
{
	zynq_dev_t *zdev = zvideo->zdev;
	u32 cam_cfg;

	zynq_trace(ZYNQ_TRACE_PROBE, "%d vid%d %s: zvideo=0x%p\n",
	    zdev->zdev_inst, zvideo->index, __FUNCTION__, zvideo);

	spin_lock(&zdev->zdev_lock);
	cam_cfg = zvideo_reg_read(zvideo, ZYNQ_CAM_CONFIG);
	cam_cfg |= ZYNQ_CAM_SENSOR_RESET;
	zvideo_reg_write(zvideo, ZYNQ_CAM_CONFIG, cam_cfg);
	mdelay(1);
	cam_cfg &= ~ZYNQ_CAM_SENSOR_RESET;
	zvideo_reg_write(zvideo, ZYNQ_CAM_CONFIG, cam_cfg);
	spin_unlock(&zdev->zdev_lock);

	zvideo->last_meta_cnt = UINT_MAX;
}

static void zvideo_reset(zynq_video_t *zvideo)
{
	zynq_dev_t *zdev = zvideo->zdev;
	int retry = 0;

	zynq_trace(ZYNQ_TRACE_PROBE, "%d vid%d %s: zvideo=0x%p\n",
	    zdev->zdev_inst, zvideo->index, __FUNCTION__, zvideo);

	/* Disable trigger */
	zvideo_disable(zvideo);

	spin_lock_bh(&zvideo->rlock);
	spin_lock_bh(&zvideo->qlock);

	if (zynq_video_zero_copy) {
		while (zvideo->buf_avail != zvideo->buf_total) {
			spin_unlock_bh(&zvideo->qlock);
			spin_unlock_bh(&zvideo->rlock);
			if (retry >= 5) {
				/* Give up and re-enable the trigger */
				zynq_err("%d vid%d %s: Reset given up\n",
				    zdev->zdev_inst, zvideo->index,
				    __FUNCTION__);
				zvideo_enable(zvideo);
				return;
			}
			msleep(10);
			retry++;
			spin_lock_bh(&zvideo->rlock);
			spin_lock_bh(&zvideo->qlock);
		}
	}

	ZYNQ_STATS_LOGX(zvideo, VIDEO_STATS_RESET, 1, 0);
	zvideo->state &= ~ZVIDEO_STATE_FAULT;

	/* Reset the camera sensor */
	zvideo_cam_reset(zvideo);

	/* Reset and start the DMA channel */
	zchan_rx_start(zvideo->zchan);

	spin_unlock_bh(&zvideo->qlock);
	spin_unlock_bh(&zvideo->rlock);

	/* Re-config the video channel */
	zvideo_config(zvideo);

	/* Re-enable the trigger */
	zvideo_enable(zvideo);
}

/*
 * Reset the extended meta data received in the DMA buffer
 */
static void zvideo_reset_meta_data(zynq_video_t *zvideo, u32 rx_off)
{
	zynq_dev_t *zdev = zvideo->zdev;
	zynq_chan_t *zchan = zvideo->zchan;
	zchan_rx_tbl_t *zchan_rx = &zchan->zchan_rx_tbl;
	zynq_video_ext_meta_data_t *ext;
	u32 rx_off1, rx_off2;

	zynq_trace(ZYNQ_TRACE_PROBE, "%d vid%d %s: zvideo=0x%p\n",
	    zdev->zdev_inst, zvideo->index, __FUNCTION__, zvideo);

	rx_off2 = rx_off + zvideo->format.sizeimage - 1;
	if (rx_off2 >= zchan_rx->zchan_rx_size) {
		rx_off2 -= zchan_rx->zchan_rx_size;
	}
	rx_off1 = rx_off + zvideo->format.sizeimage -
	    sizeof(zynq_video_ext_meta_data_t);
	if (rx_off1 >= zchan_rx->zchan_rx_size) {
		rx_off1 -= zchan_rx->zchan_rx_size;
	}
	ASSERT(ZCHAN_WITHIN_RX_BUF(zchan_rx, rx_off1, rx_off2));

	zchan_rx_off2addr(zchan_rx, rx_off1, (void **)&ext, NULL);
	ext->error = ZVIDEO_EXT_ERR_INVALID;
}

inline static long tv_diff(struct timeval *tv1, struct timeval *tv2)
{
	return (tv2->tv_sec - tv1->tv_sec) * USEC_PER_SEC +
	    tv2->tv_usec - tv1->tv_usec;
}

inline static unsigned long div_round(unsigned long val, unsigned long round)
{
	unsigned long result;

	result = val / round;
	if ((val - (result * round)) > ((result + 1) * round - val)) {
		result++;
	}

	return result;
}

/*
 * Check the meta data saved in the v4l2 buffer
 */
static int zvideo_check_meta_data(zynq_video_t *zvideo,
		zynq_video_buffer_t *buf)
{
	zynq_dev_t *zdev = zvideo->zdev;
	zynq_video_ext_meta_data_t *ext;
	struct timeval *tv;
	const char *label;
	unsigned char *mem;
	unsigned long memsz;
	unsigned int trig_cnt;
	unsigned int exp_time = 0;
	unsigned short meta_cnt = 0;
	unsigned short frame_len_lines = 0;
	unsigned short line_len_pck = 0;
	unsigned short coarse_int = 0;
	int meta_cnt_gap = -1;
	int trig_cnt_gap = -1;
	long cnt_gap = -1;
	long ts_gap;
	int corrupted;
	int ret = 0;

	mem = zvideo_buffer_vaddr(buf);
	memsz = zvideo_buffer_size(buf);

	/* Check camera embedded header metadata */
	if (zvideo->caps.embedded_data == 1) {
		if (EM_DATA_VALID(mem)) {
			meta_cnt = EM_REG_VAL(mem, EM_REG_FRAME_COUNT);
			coarse_int = EM_REG_VAL(mem, EM_REG_COARSE_INT);
			frame_len_lines =
			    EM_REG_VAL(mem, EM_REG_FRAME_LEN_LINES);
			line_len_pck =
			    EM_REG_VAL(mem, EM_REG_LINE_LEN_PCK) * 2;
			if (!zvideo->t_row) {
				zvideo->t_row = T_ROW(line_len_pck,
				    zvideo->caps.pixel_clock);
			}

			if (zvideo->last_meta_cnt != UINT_MAX) {
				/* The first frame is ignored */
				meta_cnt_gap = meta_cnt - zvideo->last_meta_cnt;
				if (meta_cnt_gap == 0) {
					zynq_err("%d vid%d %s: WARNING! "
					    "metadata frame count error, "
					    "sequence %u, current %u, last %u\n",
					    zdev->zdev_inst, zvideo->index,
					    __FUNCTION__, zvideo->sequence,
					    meta_cnt, zvideo->last_meta_cnt);
				}
			}
			zvideo->last_meta_cnt = meta_cnt;

			exp_time = coarse_int * zvideo->t_row;
			zvideo->last_exp_time = exp_time;
		} else {
			zynq_trace(ZYNQ_TRACE_PROBE,
			    "%d vid%d %s: invalid header metadata\n",
			    zdev->zdev_inst, zvideo->index, __FUNCTION__);
		}
	}

	/* Check extended metadata */
	ext = (zynq_video_ext_meta_data_t *)
	    (mem + memsz - sizeof(zynq_video_ext_meta_data_t));
	trig_cnt = ext->trigger_cnt;

	corrupted = ZVIDEO_FRAME_CORRUPTED(ext);
	if (corrupted) {
		/*
		 * When the frame is corrupted, the trigger count in
		 * the extended metadata is not reliable. We assume
		 * the valid count should be increment by 1.
		 */
		trig_cnt = zvideo->last_trig_cnt + 1;
		ZYNQ_STATS_LOG(zvideo, VIDEO_STATS_FRAME_CORRUPT);
		zvideo->frame_err_cnt++;
		if (!zynq_video_err_tolerant) {
			ret = -1;
		}
	} else if (ext->error) {
		if ((ext->error & ZVIDEO_EXT_ERR_FRAME_FORMAT)) {
			ZYNQ_STATS_LOG(zvideo, VIDEO_STATS_FRAME_FORMAT_ERR);
		}
		if ((ext->error & ZVIDEO_EXT_ERR_SHORT_FRAME)) {
			ZYNQ_STATS_LOG(zvideo, VIDEO_STATS_FRAME_SHORT);
		}
		if ((ext->error & ZVIDEO_EXT_ERR_LONG_FRAME)) {
			ZYNQ_STATS_LOG(zvideo, VIDEO_STATS_FRAME_LONG);
		}
		zvideo->frame_err_cnt++;
		if (!zynq_video_err_tolerant) {
			ret = -1;
		}
	} else {
		zvideo->frame_err_cnt = 0;
	}

	if (zvideo->frame_err_cnt >= zynq_video_err_thresh) {
		zynq_err("%d vid%d %s: Reset needed, err_cnt=%u\n",
		    zdev->zdev_inst, zvideo->index,
		    __FUNCTION__, zvideo->frame_err_cnt);
		zvideo->frame_err_cnt = 0;
		spin_lock(&zvideo->qlock);
		zvideo->state |= ZVIDEO_STATE_FAULT;
		spin_unlock(&zvideo->qlock);
		complete(&zvideo->watchdog_completion);
	}

	if (zvideo->last_trig_cnt != 0) {
		/* The first frame is ignored because it always has errors */
		trig_cnt_gap = trig_cnt - zvideo->last_trig_cnt;
		if (trig_cnt_gap == 0) {
			zynq_err("%d vid%d %s: WARNING! trigger count error, "
			    "sequence %u, current %u, last %u\n",
			    zdev->zdev_inst, zvideo->index,
			    __FUNCTION__, zvideo->sequence,
			    trig_cnt, zvideo->last_trig_cnt);
		}
	}
	zvideo->last_trig_cnt = trig_cnt;

	tv = &buf->vbuf.timestamp;
	if (zynq_video_ts_type == CAM_CAP_TIMESTAMP_HOST) {
		tv->tv_sec = zvideo->tv_intr.tv_sec;
		tv->tv_usec = zvideo->tv_intr.tv_usec;
		label = ts_label_host;
	} else if ((zynq_video_ts_type == CAM_CAP_TIMESTAMP_FORMATION) &&
	    (exp_time > 0)) {
		tv->tv_sec = zvideo->tv_intr.tv_sec;
		tv->tv_usec = zvideo->tv_intr.tv_usec -
		    ((frame_len_lines / 2) * zvideo->t_row) -
		    (exp_time / 2);
		if (tv->tv_usec < 0) {
			tv->tv_usec += USEC_PER_SEC;
			tv->tv_sec--;
		}
		label = ts_label_formation;
	} else {
		if (corrupted) {
			tv->tv_sec = 0;
			tv->tv_usec = 0;
		} else {
			tv->tv_sec = ext->time_stamp.sec;
			tv->tv_usec = ext->time_stamp.usec;
		}
		label = (zynq_video_ts_type == CAM_CAP_TIMESTAMP_TRIGGER) ?
		    ts_label_trigger : ts_label_fpga;
	}

	buf->vbuf.sequence = zvideo->sequence;

	/*
	 * Check possible frame drop.
	 *
	 * We have 3 different ways to calculate the frame count gap:
	 * 1. Use host receive time
	 *    The problem of this method is that the host time could
	 *    jump when GPS signal is not stable.
	 * 2. Use the frame count in the header metadata
	 *    The problem of this method is that the header metadata
	 *    may not be available.
	 * 3. Use the trigger count in the extended metadata
	 *    The problem of this method is that the trigger count
	 *    is not reliable because it could be corrupted.
	 * The precedence we take is: 2 > 1, and 3 is only for reference.
	 */
	if (zvideo->last_tv_intr.tv_sec == 0) {
		ts_gap = zvideo->frame_interval;
	} else {
		ts_gap = tv_diff(&zvideo->last_tv_intr, &zvideo->tv_intr);
	}
	if ((ts_gap < 0) || (ts_gap > (USEC_PER_SEC << 3))) {
		zynq_err("%d vid%d %s: WARNING! possible system time change. "
		    "frame sequence %u, host ts %lu.%06lu, last %lu.%06lu\n",
		    zdev->zdev_inst, zvideo->index,
		    __FUNCTION__, zvideo->sequence,
		    zvideo->tv_intr.tv_sec, zvideo->tv_intr.tv_usec,
		    zvideo->last_tv_intr.tv_sec, zvideo->last_tv_intr.tv_usec);
		ZYNQ_STATS(zvideo, VIDEO_STATS_FRAME_GAP_ERR);
	} else if (zvideo->frame_interval != 0) {
		cnt_gap = div_round(ts_gap, zvideo->frame_interval);
		if (cnt_gap == 0) {
			/* Interval too short */
			ZYNQ_STATS_LOG(zvideo, VIDEO_STATS_FRAME_GAP_ERR);
		}
	} else {
		zynq_err("%d vid%d %s: WARNING! driver not ready. "
		    "frame sequence %u\n",
		    zdev->zdev_inst, zvideo->index,
		    __FUNCTION__, zvideo->sequence);
	}

	if (meta_cnt_gap > 0) {
		/*
		 * When the frame count in the header metadata is
		 * available, we always trust this frame count.
		 */
		if ((cnt_gap > 0) && (cnt_gap != meta_cnt_gap)) {
			ZYNQ_STATS_LOG(zvideo,
			    VIDEO_STATS_META_COUNT_MISMATCH);
		}
		cnt_gap = meta_cnt_gap;
	}
	if (trig_cnt_gap > 0) {
		if ((cnt_gap > 0) && (cnt_gap != trig_cnt_gap)) {
			ZYNQ_STATS(zvideo,
			    VIDEO_STATS_TRIG_COUNT_MISMATCH);
		}
	}

	if (cnt_gap > 1) {
		/* Possible frame drop */
		ZYNQ_STATS_LOGX(zvideo,
		    VIDEO_STATS_FRAME_DROP, cnt_gap - 1, 0);
	}

	ZYNQ_STATS(zvideo, VIDEO_STATS_FRAME);
	zvideo->last_tv_intr = zvideo->tv_intr;

	zynq_trace(ZYNQ_TRACE_PROBE,
	    "%d vid%d %s: sequence=%u, trigger_cnt=%u, meta_cnt=%u, "
	    "ts[%s]=%ld.%06ld, host_ts=%ld.%06ld, "
	    "meta_ts=%u.%06u, debug_ts=%u.%06u, "
	    "error=0x%x, coarse_int=%u, exposure_time=%uus\n",
	    zdev->zdev_inst, zvideo->index, __FUNCTION__,
	    zvideo->sequence, ext->trigger_cnt, meta_cnt,
	    label, tv->tv_sec, tv->tv_usec,
	    zvideo->tv_intr.tv_sec, zvideo->tv_intr.tv_usec,
	    ext->time_stamp.sec, ext->time_stamp.usec,
	    ext->debug_ts.sec, ext->debug_ts.us_cnt << 8,
	    ext->error, coarse_int, exp_time);

	return ret;
}

/*
 * Check the pending list. If there is a pending buffer that matches
 * the rx head pointer, move it from the pending list to the buf list,
 * and update the head pointer accordingly.
 *
 * This function has to be called with zvideo->qlock acquired.
 */
static void zvideo_check_pending(zynq_video_t *zvideo)
{
	zynq_dev_t *zdev = zvideo->zdev;
	zynq_chan_t *zchan = zvideo->zchan;
	zchan_rx_tbl_t *zchan_rx = &zchan->zchan_rx_tbl;
	zynq_video_buffer_t *buf, *next;
	u32 rx_head;
	size_t bufsz;
	size_t vbufsz;

	bufsz = zchan_rx->zchan_rx_bufsz;
	rx_head = zchan_rx->zchan_rx_head;

	zynq_trace(ZYNQ_TRACE_PROBE, "%d vid%d %s: begin rx_head=0x%x\n",
	    zdev->zdev_inst, zvideo->index, __FUNCTION__, rx_head);
again:
	list_for_each_entry_safe(buf, next, &zvideo->pending_list, list) {
		if (buf->offset != rx_head) {
			continue;
		}

		/*
		 * Found a pending video buffer that matches the head,
		 * remove it from the pending_list and add it to the
		 * buf_list for data receiving.
		 */
		list_del(&buf->list);
		list_add_tail(&buf->list, &zvideo->buf_list);
		zvideo->buf_avail++;

		/*
		 * Before allowing the buffer to be filled with new
		 * video frame data, reset the meta data.
		 */
		zvideo_reset_meta_data(zvideo, buf->offset);

		/* Update the head pointer */
		vbufsz = ALIGN(zvideo_buffer_size(buf), bufsz);
		ASSERT(vbufsz == ALIGN(zvideo->format.sizeimage, bufsz));

		rx_head += vbufsz;
		if (rx_head >= zchan_rx->zchan_rx_size) {
			rx_head -= zchan_rx->zchan_rx_size;
		}

		zchan_rx->zchan_rx_head = rx_head;
		zchan_reg_write(zchan, ZYNQ_CH_RX_HEAD, rx_head);

		zynq_trace(ZYNQ_TRACE_PROBE,
		    "%d vid%d %s: update rx_head=0x%x\n",
		    zdev->zdev_inst, zvideo->index, __FUNCTION__, rx_head);
		goto again;
	}
}

static int zvideo_map_buf(struct pci_dev *pdev, zchan_buf_t *zchan_buf,
    void *vaddr, size_t bufsz, enum dma_data_direction dmatype)
{
	struct page *page;
	dma_addr_t paddr;

	if (!is_vmalloc_addr(vaddr)) {
		zynq_err("%s: vaddr 0x%p is not a vmalloc addr\n",
		    __FUNCTION__, vaddr);
		return -1;
	}
	page = vmalloc_to_page(vaddr);
	paddr = pci_map_page(pdev, page, 0, bufsz, dmatype);
	if (pci_dma_mapping_error(pdev, paddr)) {
		zynq_err("%s: pci_map_page failed, va=0x%p, pa=0x%llx\n",
		    __FUNCTION__, vaddr, paddr);
		return -1;
	}
	if (paddr & (bufsz - 1)) {
		zynq_err("%s dma addr 0x%llx is not bufsz %zd aligned.\n",
		    __FUNCTION__, paddr, bufsz);
		pci_unmap_page(pdev, paddr, bufsz, dmatype);
		return -1;
	}
	zynq_trace(ZYNQ_TRACE_BUF, "%s done, va=0x%p, pa=0x%llx\n",
	    __FUNCTION__, vaddr, paddr);

	zchan_buf->zchan_bufp = vaddr;
	zchan_buf->zchan_buf_page = page;
	zchan_buf->zchan_buf_dma = paddr;

	return 0;
}

static void zvideo_unmap_buf(struct pci_dev *pdev, zchan_buf_t *zchan_buf,
    size_t bufsz, enum dma_data_direction dmatype)
{
	pci_unmap_page(pdev, zchan_buf->zchan_buf_dma, bufsz, dmatype);

	zchan_buf->zchan_bufp = NULL;
	zchan_buf->zchan_buf_page = NULL;
	zchan_buf->zchan_buf_dma = 0;
}

static void zvideo_fini_rx(zynq_video_t *zvideo)
{
	zynq_dev_t *zdev = zvideo->zdev;
	zynq_chan_t *zchan = zvideo->zchan;
	zchan_rx_tbl_t *zchan_rx = &zchan->zchan_rx_tbl;
	struct pci_dev *pdev = zchan->zdev->zdev_pdev;
	zchan_rx_pt_t *rx_ptp; /* page table array */
	zchan_buf_t *bufp;
	size_t bufsz;
	int i, j;

	zynq_trace(ZYNQ_TRACE_PROBE, "%d vid%d %s: zvideo=0x%p\n",
	    zdev->zdev_inst, zvideo->index, __FUNCTION__, zvideo);

	rx_ptp = zchan_rx->zchan_rx_ptp;
	bufsz = zchan_rx->zchan_rx_bufsz;

	for (i = 0; i < zchan_rx->zchan_rx_pdt_num; i++, rx_ptp++) {
		if (!rx_ptp->zchan_rx_pt) {
			continue;
		}

		bufp = rx_ptp->zchan_rx_pt_bufp;
		ASSERT(bufp);
		/* Unmap the DMA buffers */
		for (j = 0;  j < rx_ptp->zchan_rx_pt_buf_num; j++, bufp++) {
			if (bufp->zchan_buf_dma) {
				zvideo_unmap_buf(pdev, bufp,
				    bufsz, PCI_DMA_FROMDEVICE);
			}
		}

		/* Free the array of zchan_buf_t */
		kfree(rx_ptp->zchan_rx_pt_bufp);

		/* Free the page table */
		zchan_free_consistent(pdev,
		    rx_ptp->zchan_rx_pt, rx_ptp->zchan_rx_pt_dma,
		    ZCHAN_RX_PT_SIZE);
	}

	/* Free the array of zchan_rx_pt_t */
	kfree(zchan_rx->zchan_rx_ptp);
	zchan_rx->zchan_rx_ptp = NULL;
	zchan_rx->zchan_rx_pt_entries = 0;
	zchan_rx->zchan_rx_size = 0;
	zchan_rx->zchan_rx_bufsz = 0;

	/* Free the page directory table */
	zchan_free_consistent(pdev,
	    zchan_rx->zchan_rx_pdt, zchan_rx->zchan_rx_pdt_dma,
	    zchan_rx->zchan_rx_pdt_num * sizeof(u64));
	zchan_rx->zchan_rx_pdt = NULL;
	zchan_rx->zchan_rx_pdt_dma = 0;
	zchan_rx->zchan_rx_pdt_num = 0;
}

static int zvideo_init_rx(zynq_video_t *zvideo, unsigned int count)
{
	zynq_dev_t *zdev = zvideo->zdev;
	zynq_chan_t *zchan = zvideo->zchan;
	zchan_rx_tbl_t *zchan_rx = &zchan->zchan_rx_tbl;
	struct pci_dev *pdev = zchan->zdev->zdev_pdev;
	enum dma_data_direction dmatype;
	zynq_video_buffer_t *zbuf, *next;
	size_t vbufsz;
	unsigned char *vaddr;
	u32 pdt_entries;
	u32 pt_entries;
	u32 buf_num;
	size_t pdt_size;
	size_t bufsz;
	u64 *rx_pdtp; /* page directory table array */
	zchan_rx_pt_t *rx_ptp; /* page table array */
	u64 *rx_pt; /* page table */
	zchan_buf_t *bufp;
	int i;

	zynq_trace(ZYNQ_TRACE_PROBE, "%d vid%d %s: zvideo=0x%p, count=%u\n",
	    zdev->zdev_inst, zvideo->index, __FUNCTION__, zvideo, count);

	dmatype = PCI_DMA_FROMDEVICE;
	bufsz = ZCHAN_BUF_SIZE;
	pt_entries = count * CEILING(zvideo->format.sizeimage, bufsz);
	pdt_entries = CEILING(pt_entries, ZCHAN_RX_PT_ENTRIES);

	/* allocate page directory table first */
	pdt_size = pdt_entries * sizeof(u64);
	rx_pdtp = zchan_alloc_consistent(pdev,
	    &zchan_rx->zchan_rx_pdt_dma, pdt_size);
	if (rx_pdtp == NULL) {
		zynq_err("%d vid%d %s "
		    "failed to alloc page directory table.\n",
		    zdev->zdev_inst, zvideo->index, __FUNCTION__);
		return -1;
	}
	zchan_rx->zchan_rx_pt_entries = pt_entries;
	zchan_rx->zchan_rx_pdt_num = pdt_entries;
	zchan_rx->zchan_rx_pdt = rx_pdtp;

	/* allocate the array of zchan_rx_pt_t */
	rx_ptp = kzalloc(pdt_entries * sizeof(zchan_rx_pt_t), GFP_KERNEL);
	if (rx_ptp == NULL) {
		zchan_free_consistent(pdev,
		    zchan_rx->zchan_rx_pdt, zchan_rx->zchan_rx_pdt_dma,
		    pdt_size);
		zynq_err("%d vid%d, %s failed to alloc page table array.\n",
		    zdev->zdev_inst, zvideo->index, __FUNCTION__);
		return -1;
	}
	zchan_rx->zchan_rx_ptp = rx_ptp;
	zchan_rx->zchan_rx_bufsz = bufsz;

	for (i = 0; i < zchan_rx->zchan_rx_pdt_num; i++, rx_pdtp++, rx_ptp++) {
		/* allocate each page table */
		rx_pt = zchan_alloc_consistent(pdev,
		    &rx_ptp->zchan_rx_pt_dma, ZCHAN_RX_PT_SIZE);
		if (rx_pt == NULL) {
			zynq_err("%d vid%d %s "
			    "failed to alloc a page table.\n",
			    zdev->zdev_inst, zvideo->index, __FUNCTION__);
			goto init_rx_fail;
		}
		rx_ptp->zchan_rx_pt = rx_pt;

		/* init the page directory table entry */
		*rx_pdtp = rx_ptp->zchan_rx_pt_dma;

		/* allocate the array of zchan_buf_t */
		buf_num = MIN(pt_entries, ZCHAN_RX_PT_ENTRIES);
		ASSERT(buf_num > 0);
		bufp = kzalloc(buf_num * sizeof(zchan_buf_t),
		    GFP_KERNEL);
		if (bufp == NULL) {
			zchan_free_consistent(pdev,
			    rx_ptp->zchan_rx_pt, rx_ptp->zchan_rx_pt_dma,
			    ZCHAN_RX_PT_SIZE);
			rx_ptp->zchan_rx_pt = NULL;
			rx_ptp->zchan_rx_pt_dma = 0;
			zynq_err("%d vid%d %s "
			    "failed to alloc a Rx buffer array.\n",
			    zdev->zdev_inst, zvideo->index, __FUNCTION__);
			goto init_rx_fail;
		}
		rx_ptp->zchan_rx_pt_bufp = bufp;
		rx_ptp->zchan_rx_pt_buf_num = buf_num;

		pt_entries -= buf_num;
	}

	zchan_rx->zchan_rx_size = zchan_rx->zchan_rx_pt_entries * bufsz;
	zchan_rx->zchan_rx_pdt_shift = fls(ZCHAN_RX_PT_ENTRIES * bufsz) - 1;
	zchan_rx->zchan_rx_pt_shift = fls(bufsz) - 1;
	zchan_rx->zchan_rx_pt_mask = ZCHAN_RX_PT_ENTRIES - 1;
	zchan_rx->zchan_rx_buf_mask = bufsz - 1;

	zynq_trace(ZYNQ_TRACE_PROBE, "%d vid%d %s: rx_bufsz=%zd, "
	    "rx_buf_mask=0x%x, rx_pdt_num=0x%x, pt_entries=0x%x, rx_size=0x%x, "
	    "rx_pdt_shift=%d, rx_pt_shift=%d, rx_pt_mask=0x%x.\n",
	    zdev->zdev_inst, zvideo->index,
	    __FUNCTION__, bufsz, zchan_rx->zchan_rx_buf_mask,
	    zchan_rx->zchan_rx_pdt_num, zchan_rx->zchan_rx_pt_entries,
	    zchan_rx->zchan_rx_size, zchan_rx->zchan_rx_pdt_shift,
	    zchan_rx->zchan_rx_pt_shift, zchan_rx->zchan_rx_pt_mask);

	rx_ptp = zchan_rx->zchan_rx_ptp;
	rx_pt = rx_ptp->zchan_rx_pt;
	bufp = rx_ptp->zchan_rx_pt_bufp;
	buf_num = rx_ptp->zchan_rx_pt_buf_num;
	pt_entries = 0;

	list_for_each_entry_safe(zbuf, next, &zvideo->buf_list, list) {
		zbuf->offset = pt_entries << zchan_rx->zchan_rx_pt_shift;

		vaddr = zvideo_buffer_vaddr(zbuf);
		vbufsz = ALIGN(zvideo_buffer_size(zbuf), bufsz);
		ASSERT(vbufsz == ALIGN(zvideo->format.sizeimage, bufsz));

		while (vbufsz > 0) {
			if (zvideo_map_buf(pdev, bufp,
			    vaddr, bufsz, PCI_DMA_FROMDEVICE)) {
				goto init_rx_fail;
			}

			/* init the page table entry */
			*rx_pt = bufp->zchan_buf_dma;

			pt_entries++;
			if (pt_entries & zchan_rx->zchan_rx_pt_mask) {
				rx_pt++;
				bufp++;
			} else {
				rx_ptp++;
				rx_pt = rx_ptp->zchan_rx_pt;
				bufp = rx_ptp->zchan_rx_pt_bufp;
				buf_num += rx_ptp->zchan_rx_pt_buf_num;
			}

			vbufsz -= bufsz;
			vaddr += bufsz;
		}
	}
	ASSERT(pt_entries == buf_num);

	return 0;

init_rx_fail:
	zvideo_fini_rx(zvideo);

	return -1;
}

/*
 * Setup the constraints of the queue before allocating the buffers.
 *
 * The vb2 framework will check the number of allocated buffers against
 * the min_buffers_needed before calling this function. Here we don't
 * do any additional check for the buffer number.
 */
static int zvideo_queue_setup(struct vb2_queue *vq, const void *parg,
		unsigned int *nbuffers, unsigned int *nplanes,
		unsigned int sizes[], void *alloc_ctxs[])
{
	const struct v4l2_format *fmt = parg;
	zynq_video_t *zvideo = vb2_get_drv_priv(vq);
	zynq_dev_t *zdev = zvideo->zdev;

	zynq_trace(ZYNQ_TRACE_PROBE, "%d vid%d %s: vq=0x%p\n",
	    zdev->zdev_inst, zvideo->index, __FUNCTION__, vq);

	if (fmt && fmt->fmt.pix.sizeimage != zvideo->format.sizeimage) {
		zynq_err("%d vid%d %s: requested size invalid (%u != %u)\n",
		    zdev->zdev_inst, zvideo->index,
		    __FUNCTION__, fmt->fmt.pix.sizeimage,
		    zvideo->format.sizeimage);
		return -EINVAL;
	}

	*nplanes = 1;
	sizes[0] = zvideo->format.sizeimage;

	return 0;
}

/*
 * Initialize the buffer right after the allocation.
 */
static int zvideo_buffer_init(struct vb2_buffer *vb)
{
	zynq_video_t *zvideo = vb2_get_drv_priv(vb->vb2_queue);
	zynq_dev_t *zdev = zvideo->zdev;
	zynq_video_buffer_t *zbuf = vb_to_zvb(vb);

	zynq_trace(ZYNQ_TRACE_PROBE, "%d vid%d %s: vb=0x%p\n",
	    zdev->zdev_inst, zvideo->index, __FUNCTION__, vb);

	/*
	 * We save the vaddr and size of the buffer before the plane size
	 * of the vb2 buffer is adjusted for meta data drop.
	 */
	zbuf->addr = vb2_plane_vaddr(vb, 0);
	zbuf->size = vb2_plane_size(vb, 0);

	return 0;
}

/*
 * Prepare the buffer for queuing to the DMA engine: check and set the
 * payload size.
 */
static int zvideo_buffer_prepare(struct vb2_buffer *vb)
{
	zynq_video_t *zvideo = vb2_get_drv_priv(vb->vb2_queue);
	zynq_dev_t *zdev = zvideo->zdev;
	zynq_video_buffer_t *zbuf = vb_to_zvb(vb);
	unsigned long bufsz = zvideo_buffer_size(zbuf);
	unsigned long size;

	zynq_trace(ZYNQ_TRACE_PROBE, "%d vid%d %s: vb=0x%p\n",
	    zdev->zdev_inst, zvideo->index, __FUNCTION__, vb);

	size = zvideo->format.sizeimage;

	if (bufsz != size) {
		zynq_err("%d vid%d %s: buffer size invalid (%lu != %lu)\n",
		    zdev->zdev_inst, zvideo->index,
		    __FUNCTION__, bufsz, size);
		return -EINVAL;
	}

	vb2_set_plane_payload(vb, 0, 0);

	return 0;
}

/*
 * Finish the buffer for dequeuing
 */
static void zvideo_buffer_finish(struct vb2_buffer *vb)
{
	zynq_video_t *zvideo = vb2_get_drv_priv(vb->vb2_queue);
	zynq_dev_t *zdev = zvideo->zdev;

	zynq_trace(ZYNQ_TRACE_PROBE, "%d vid%d %s: vb=0x%p\n",
	    zdev->zdev_inst, zvideo->index, __FUNCTION__, vb);
}

/*
 * Cleanup the buffer before it is freed
 */
static void zvideo_buffer_cleanup(struct vb2_buffer *vb)
{
	zynq_video_t *zvideo = vb2_get_drv_priv(vb->vb2_queue);
	zynq_dev_t *zdev = zvideo->zdev;

	zynq_trace(ZYNQ_TRACE_PROBE, "%d vid%d %s: vb=0x%p\n",
	    zdev->zdev_inst, zvideo->index, __FUNCTION__, vb);
}

/*
 * Queue this buffer to the DMA engine.
 */
static void zvideo_buffer_queue(struct vb2_buffer *vb)
{
	zynq_video_t *zvideo = vb2_get_drv_priv(vb->vb2_queue);
	zynq_dev_t *zdev = zvideo->zdev;
	zynq_video_buffer_t *buf = vb_to_zvb(vb);

	zynq_trace(ZYNQ_TRACE_PROBE, "%d vid%d %s: vb=0x%p, state=%d\n",
	    zdev->zdev_inst, zvideo->index, __FUNCTION__, vb, vb->state);

	spin_lock_bh(&zvideo->qlock);
	if (zynq_video_zero_copy) {
		if (zvideo->state & ZVIDEO_STATE_STREAMING) {
			/*
			 * Considering the case that the buffers may be
			 * returned to the kernel in a changed sequence,
			 * we add them to the pending list first and then
			 * move them to the buf list later when the expected
			 * sequence is matched.
			 */
			list_add(&buf->list, &zvideo->pending_list);
			zvideo_check_pending(zvideo);
		} else {
			list_add_tail(&buf->list, &zvideo->buf_list);
			zvideo->buf_avail++;
		}
	} else {
		list_add_tail(&buf->list, &zvideo->buf_list);
		zvideo->buf_avail++;
	}
	spin_unlock_bh(&zvideo->qlock);
}

static void zvideo_return_all_buffers(zynq_video_t *zvideo,
		enum vb2_buffer_state state)
{
	zynq_video_buffer_t *buf, *node;

	spin_lock_bh(&zvideo->qlock);
	list_for_each_entry_safe(buf, node, &zvideo->pending_list, list) {
		vb2_buffer_done(&buf->vbuf.vb2_buf, state);
		list_del(&buf->list);
	}
	list_for_each_entry_safe(buf, node, &zvideo->buf_list, list) {
		vb2_buffer_done(&buf->vbuf.vb2_buf, state);
		list_del(&buf->list);
		zvideo->buf_avail--;
	}
	spin_unlock_bh(&zvideo->qlock);
}

static void zvideo_init_params(zynq_video_t *zvideo)
{
	zvideo->sequence = 0;
	zvideo->frame_err_cnt = 0;

	zvideo->last_exp_time = 0;
	zvideo->last_meta_cnt = UINT_MAX;
	zvideo->last_trig_cnt = 0;
	zvideo->last_tv_intr.tv_sec = 0;
	zvideo->last_tv_intr.tv_usec = 0;
}

/*
 * Start streaming. First check if the minimum number of buffers have been
 * queued. If not, then return -ENOBUFS and the vb2 framework will call
 * this function again the next time a buffer has been queued until enough
 * buffers are available to actually start streaming.
 */
static int zvideo_start_streaming(struct vb2_queue *vq, unsigned int count)
{
	zynq_video_t *zvideo = vb2_get_drv_priv(vq);
	zynq_dev_t *zdev = zvideo->zdev;
	zynq_chan_t *zchan = zvideo->zchan;

	zynq_trace(ZYNQ_TRACE_PROBE, "%d vid%d %s: vq=0x%p, count=%u\n",
	    zdev->zdev_inst, zvideo->index, __FUNCTION__, vq, count);

	if (count < zynq_video_buf_num) {
		zynq_err("%d vid%d %s no enough buffers\n",
		    zdev->zdev_inst, zvideo->index, __FUNCTION__);
		return -ENOBUFS;
	}

	zvideo->buf_total = count;
	ASSERT(zvideo->buf_total == zvideo->buf_avail);

	zvideo_init_params(zvideo);

	spin_lock_bh(&zvideo->qlock);
	if (zynq_video_zero_copy) {
		if (zvideo_init_rx(zvideo, count)) {
			spin_unlock_bh(&zvideo->qlock);
			return -ENOSR;
		}
	}

	/* Start DMA */
	zchan_rx_start(zchan);

	/* Configure the video channel */
	zvideo_config(zvideo);

	/*
	 * For Non-zerocopy, we don't enable DMA here. DMA is already enabled
	 * when the driver is initialized. The flag ZVIDEO_STATE_STREAMING is
	 * used to sync up data receiving.
	 */

	/* Set the streaming flag */
	zvideo->state |= ZVIDEO_STATE_STREAMING;
	spin_unlock_bh(&zvideo->qlock);

	return 0;
}

/*
 * Stop streaming. Any remaining buffers in the DMA queue are dequeued
 * and passed on to the vb2 framework marked as STATE_ERROR.
 */
static void zvideo_stop_streaming(struct vb2_queue *vq)
{
	zynq_video_t *zvideo = vb2_get_drv_priv(vq);
	zynq_dev_t *zdev = zvideo->zdev;
	zynq_chan_t *zchan = zvideo->zchan;

	zynq_trace(ZYNQ_TRACE_PROBE, "%d vid%d %s: vq=0x%p\n",
	    zdev->zdev_inst, zvideo->index, __FUNCTION__, vq);

	spin_lock_bh(&zvideo->qlock);
	/* Clear the streaming flag */
	zvideo->state &= ~ZVIDEO_STATE_STREAMING;

	/* Disable the trigger */
	zvideo_disable(zvideo);

	if (zynq_video_zero_copy) {
		/* Stop DMA */
		zchan_rx_stop(zchan);

		/* Unmap the DMA buffers */
		zvideo_fini_rx(zvideo);
	}
	/*
	 * For Non-zerocopy, we don't disable DMA here. DMA is always enabled.
	 * The streaming flag is used to sync up data receiving.
	 */
	spin_unlock_bh(&zvideo->qlock);

	/* Release all active buffers */
	zvideo_return_all_buffers(zvideo, VB2_BUF_STATE_ERROR);

	/* Wait until other pending buffers are returned to vb2 */
	vb2_wait_for_all_buffers(vq);

	zvideo->buf_total = 0;
	ASSERT(zvideo->buf_total == zvideo->buf_avail);
}

/*
 * The vb2 queue ops. Note that since q->lock is set we can use the standard
 * vb2_ops_wait_prepare/finish helper functions. If q->lock would be NULL,
 * then this driver would have to provide these ops.
 */
static struct vb2_ops video_qops = {
	.queue_setup		= zvideo_queue_setup,
	.wait_prepare		= vb2_ops_wait_prepare,
	.wait_finish		= vb2_ops_wait_finish,
	.buf_init		= zvideo_buffer_init,
	.buf_prepare		= zvideo_buffer_prepare,
	.buf_finish		= zvideo_buffer_finish,
	.buf_cleanup		= zvideo_buffer_cleanup,
	.start_streaming	= zvideo_start_streaming,
	.stop_streaming		= zvideo_stop_streaming,
	.buf_queue		= zvideo_buffer_queue
};

/*
 * Required ioctl querycap. Note that the version field is prefilled with
 * the version of the kernel.
 */
static int zvideo_querycap(struct file *file, void *priv,
		struct v4l2_capability *cap)
{
	zynq_video_t *zvideo = video_drvdata(file);
	zynq_dev_t *zdev = zvideo->zdev;

	zynq_trace(ZYNQ_TRACE_PROBE, "%d vid%d %s: file=0x%p\n",
	    zdev->zdev_inst, zvideo->index, __FUNCTION__, file);

	strlcpy(cap->driver, KBUILD_MODNAME, sizeof(cap->driver));
	strlcpy(cap->card, zvideo->caps.name, sizeof(cap->card));
	snprintf(cap->bus_info, sizeof(cap->bus_info), "PCI:%s",
	    pci_name(zvideo->zdev->zdev_pdev));
	cap->device_caps = V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_STREAMING;
	cap->capabilities = cap->device_caps | V4L2_CAP_DEVICE_CAPS;

	return 0;
}

static int zvideo_try_fmt_vid_cap(struct file *file, void *priv,
		struct v4l2_format *f)
{
	zynq_video_t *zvideo = video_drvdata(file);
	zynq_dev_t *zdev = zvideo->zdev;
	struct v4l2_pix_format *pix = &f->fmt.pix;

	zynq_trace(ZYNQ_TRACE_PROBE, "%d vid%d %s: file=0x%p\n",
	    zdev->zdev_inst, zvideo->index, __FUNCTION__, file);

	if (pix->pixelformat != zvideo->format.pixelformat) {
		return -EINVAL;
	}

	*pix = zvideo->format;

	return 0;
}

static int zvideo_s_fmt_vid_cap(struct file *file, void *priv,
		struct v4l2_format *f)
{
	zynq_video_t *zvideo = video_drvdata(file);
	zynq_dev_t *zdev = zvideo->zdev;
	int ret;

	zynq_trace(ZYNQ_TRACE_PROBE, "%d vid%d %s: file=0x%p\n",
	    zdev->zdev_inst, zvideo->index, __FUNCTION__, file);

	ret = zvideo_try_fmt_vid_cap(file, priv, f);
	if (ret) {
		return ret;
	}

	/*
	 * It is not allowed to change the format while buffers for use with
	 * streaming have already been allocated.
	 */
	if (vb2_is_busy(&zvideo->queue)) {
		return -EBUSY;
	}

	/* We do not allow changing format for now */

	return 0;
}

static int zvideo_g_fmt_vid_cap(struct file *file, void *priv,
		struct v4l2_format *f)
{
	zynq_video_t *zvideo = video_drvdata(file);
	zynq_dev_t *zdev = zvideo->zdev;
	struct v4l2_pix_format *pix = &f->fmt.pix;

	zynq_trace(ZYNQ_TRACE_PROBE, "%d vid%d %s: file=0x%p\n",
	    zdev->zdev_inst, zvideo->index, __FUNCTION__, file);

	*pix = zvideo->format;

	return 0;
}

static int zvideo_enum_fmt_vid_cap(struct file *file, void *priv,
		struct v4l2_fmtdesc *f)
{
	zynq_video_t *zvideo = video_drvdata(file);
	zynq_dev_t *zdev = zvideo->zdev;

	zynq_trace(ZYNQ_TRACE_PROBE, "%d vid%d %s: file=0x%p\n",
	    zdev->zdev_inst, zvideo->index, __FUNCTION__, file);

	if (f->index != 0) {
		return -EINVAL;
	}

	f->pixelformat = zvideo->format.pixelformat;

	return 0;
}

static int zvideo_enum_input(struct file *file, void *priv,
		struct v4l2_input *i)
{
	zynq_video_t *zvideo = video_drvdata(file);
	zynq_dev_t *zdev = zvideo->zdev;

	zynq_trace(ZYNQ_TRACE_PROBE,
	    "%d vid%d %s: file=0x%p, input->index=%u\n",
	    zdev->zdev_inst, zvideo->index, __FUNCTION__,
	    file, i->index);

	if (i->index > 0) {
		return -EINVAL;
	}

	i->type = V4L2_INPUT_TYPE_CAMERA;
	strlcpy(i->name, zvideo->caps.name, sizeof(i->name));

	return 0;
}

static int zvideo_s_input(struct file *file, void *priv, unsigned int i)
{
	zynq_video_t *zvideo = video_drvdata(file);
	zynq_dev_t *zdev = zvideo->zdev;

	zynq_trace(ZYNQ_TRACE_PROBE, "%d vid%d %s: file=0x%p, i=%u\n",
	    zdev->zdev_inst, zvideo->index, __FUNCTION__, file, i);

	if (i > 0) {
		return -EINVAL;
	}

	/*
	 * Changing the input implies a format change, which is not allowed
	 * while buffers for use with streaming have already been allocated.
	 */
	if (vb2_is_busy(&zvideo->queue)) {
		return -EBUSY;
	}

	zvideo->input = i;

	return 0;
}

static int zvideo_g_input(struct file *file, void *priv, unsigned int *i)
{
	zynq_video_t *zvideo = video_drvdata(file);
	zynq_dev_t *zdev = zvideo->zdev;

	*i = zvideo->input;

	zynq_trace(ZYNQ_TRACE_PROBE, "%d vid%d %s: file=0x%p, *i=%u\n",
	    zdev->zdev_inst, zvideo->index, __FUNCTION__, file, *i);

	return 0;
}

static void zvideo_trigger_enable(zynq_video_t *zvideo, zynq_trigger_t *trigger)
{
	zynq_dev_t *zdev = zvideo->zdev;
	unsigned int val;

	if ((trigger->fps == 0) || (trigger->fps > ZYNQ_FPD_FPS_MAX)) {
		trigger->fps = ZYNQ_FPD_FPS_DEFAULT;
	}

	spin_lock(&zdev->zdev_lock);
	/* 1. Disable the individual trigger */
	val = zvideo_reg_read(zvideo, ZYNQ_CAM_CONFIG);
	val &= ~ZYNQ_CAM_EN;
	zvideo_reg_write(zvideo, ZYNQ_CAM_CONFIG, val);

	/* 2. Set FPS */
	val = zvideo_reg_read(zvideo, ZYNQ_CAM_TRIGGER);
	val = SET_BITS(ZYNQ_CAM_TRIG_FPS, val, trigger->fps);
	zvideo_reg_write(zvideo, ZYNQ_CAM_TRIGGER, val);

	/* 3. Enable the individual trigger */
	val = zvideo_reg_read(zvideo, ZYNQ_CAM_CONFIG);
	val |= ZYNQ_CAM_EN;
	zvideo_reg_write(zvideo, ZYNQ_CAM_CONFIG, val);

	/* 4. Enable the global trigger */
	val = zynq_g_reg_read(zdev, ZYNQ_G_CONFIG);
	val &= ~ZYNQ_CONFIG_TRIGGER_MASK;
	val |= ZYNQ_CONFIG_TRIGGER;
	if (trigger->internal) {
		val |= ZYNQ_CONFIG_GPS_SW;
	}
	zynq_g_reg_write(zdev, ZYNQ_G_CONFIG, val);

	zvideo->fps = trigger->fps;
	zvideo->frame_interval = USEC_PER_SEC / trigger->fps;
	spin_unlock(&zdev->zdev_lock);

	zynq_log("%d vid%d %s: done. fps=%u, internal=%u\n",
	    zdev->zdev_inst, zvideo->index, __FUNCTION__,
	    trigger->fps, trigger->internal);
}

static void zvideo_trigger_disable(zynq_video_t *zvideo)
{
	zynq_dev_t *zdev = zvideo->zdev;
	unsigned int val;

	spin_lock(&zdev->zdev_lock);
	/* Disable the individual trigger */
	val = zvideo_reg_read(zvideo, ZYNQ_CAM_CONFIG);
	val &= ~ZYNQ_CAM_EN;
	zvideo_reg_write(zvideo, ZYNQ_CAM_CONFIG, val);

	zvideo->fps = 0;
	zvideo->frame_interval = 0;
	spin_unlock(&zdev->zdev_lock);

	zynq_log("%d vid%d %s: done.\n",
	    zdev->zdev_inst, zvideo->index, __FUNCTION__);
}

static void zvideo_trigger_delay_set(zynq_video_t *zvideo,
		zynq_trigger_delay_t *delay)
{
	zynq_dev_t *zdev = zvideo->zdev;
	unsigned int val;

	spin_lock(&zdev->zdev_lock);
	val = zvideo_reg_read(zvideo, ZYNQ_CAM_TRIGGER);
	val = SET_BITS(ZYNQ_CAM_TRIG_DELAY, val, delay->trigger_delay);
	zvideo_reg_write(zvideo, ZYNQ_CAM_TRIGGER, val);
	spin_unlock(&zdev->zdev_lock);

	zynq_trace(ZYNQ_TRACE_PROBE, "%d vid%d %s: trigger_delay=%u\n",
	    zdev->zdev_inst, zvideo->index, __FUNCTION__,
	    delay->trigger_delay);
}

static void zvideo_trigger_delay_get(zynq_video_t *zvideo,
		zynq_trigger_delay_t *delay)
{
	zynq_dev_t *zdev = zvideo->zdev;
	unsigned int val;

	spin_lock(&zdev->zdev_lock);
	val = zvideo_reg_read(zvideo, ZYNQ_CAM_TRIGGER);
	val = GET_BITS(ZYNQ_CAM_TRIG_DELAY, val);
	spin_unlock(&zdev->zdev_lock);

	delay->trigger_delay = val;
	delay->exposure_time = zvideo->last_exp_time;

	zynq_trace(ZYNQ_TRACE_PROBE, "%d vid%d %s: "
	    "trigger_delay=%u, exposure_time=%u\n",
	    zdev->zdev_inst, zvideo->index, __FUNCTION__,
	    delay->trigger_delay, delay->exposure_time);
}

static long zvideo_ioctl_default(struct file *file, void *fh,
		bool valid_prio, unsigned int cmd, void *arg)
{
	zynq_video_t *zvideo = video_drvdata(file);
	zynq_dev_t *zdev = zvideo->zdev;
	int err = 0;

	zynq_trace(ZYNQ_TRACE_PROBE, "%d vid%d %s: file=0x%p\n",
	    zdev->zdev_inst, zvideo->index, __FUNCTION__, file);

	switch (cmd) {
	case ZYNQ_IOC_CAM_REG_READ:
	{
		zynq_cam_acc_t *cam_acc = (zynq_cam_acc_t *)arg;

		zynq_trace(ZYNQ_TRACE_PROBE,
		    "%d vid%d ZYNQ_IOC_CAM_REG_READ: addr=0x%x\n",
		    zdev->zdev_inst, zvideo->index, cam_acc->addr);

		err = zcam_reg_read(zvideo, cam_acc);
		break;
	}
	case ZYNQ_IOC_CAM_REG_WRITE:
	{
		zynq_cam_acc_t *cam_acc = (zynq_cam_acc_t *)arg;

		zynq_trace(ZYNQ_TRACE_PROBE,
		    "%d vid%d ZYNQ_IOC_CAM_REG_WRITE: addr=0x%x data=0x%x\n",
		    zdev->zdev_inst, zvideo->index,
		    cam_acc->addr, cam_acc->data);

		err = zcam_reg_write(zvideo, cam_acc);
		break;
	}
	case ZYNQ_IOC_CAM_FLASH_INIT:
	{
		int config_device = *(int *)arg;

		zynq_trace(ZYNQ_TRACE_PROBE,
		    "%d vid%d ZYNQ_IOC_CAM_FLASH_INIT, config_device=%d\n",
		    zdev->zdev_inst, zvideo->index, config_device);

		/* Safety patch 15 */
		if (config_device) {
			if ((err = zcam_set_suspend(zvideo))) {
				break;
			}
		}
		if ((err = zcam_flash_get_lock(zvideo))) {
			break;
		}
		if ((err = zcam_flash_lock_status(zvideo))) {
			break;
		}
		if (!config_device) {
			break;
		}
		if ((err = zcam_flash_query_device(zvideo))) {
			(void) zcam_flash_release_lock(zvideo);
			break;
		}
		if ((err = zcam_flash_config_device(zvideo))) {
			(void) zcam_flash_release_lock(zvideo);
			break;
		}
		break;
	}
	case ZYNQ_IOC_CAM_FLASH_FINI:
		zynq_trace(ZYNQ_TRACE_PROBE,
		    "%d vid%d ZYNQ_IOC_CAM_FLASH_FINI\n",
		    zdev->zdev_inst, zvideo->index);

		err = zcam_flash_release_lock(zvideo);
		break;

	case ZYNQ_IOC_CAM_FLASH_READ:
	{
		zynq_cam_fw_t *cam_fw = (zynq_cam_fw_t *)arg;
		unsigned char data[ZYNQ_CAM_FW_BLOCK_SIZE_MAX];

		zynq_trace(ZYNQ_TRACE_PROBE, "%d vid%d "
		    "ZYNQ_IOC_CAM_FLASH_READ: address=0x%x, size=%d\n",
		    zdev->zdev_inst, zvideo->index,
		    cam_fw->address, cam_fw->size);

		if ((cam_fw->size > ZYNQ_CAM_FW_BLOCK_SIZE_MAX) ||
		    (cam_fw->data == NULL)) {
			err = -EINVAL;
			break;
		}
		if ((err = zcam_flash_read(zvideo, cam_fw->address,
		    data, cam_fw->size))) {
			break;
		}
		if (copy_to_user((void __user *)cam_fw->data, data,
		    cam_fw->size)) {
			zynq_err("%d vid%d "
			    "ZYNQ_IOC_CAM_FLASH_READ: copy_to_user failed\n",
			    zdev->zdev_inst, zvideo->index);
			err = -EFAULT;
			break;
		}
		break;
	}
	case ZYNQ_IOC_CAM_FLASH_WRITE:
	{
		zynq_cam_fw_t *cam_fw = (zynq_cam_fw_t *)arg;
		unsigned char data[ZYNQ_CAM_FW_BLOCK_SIZE_MAX];

		zynq_trace(ZYNQ_TRACE_PROBE, "%d vid%d "
		    "ZYNQ_IOC_CAM_FLASH_WRITE: address=0x%x, size=%d\n",
		    zdev->zdev_inst, zvideo->index,
		    cam_fw->address, cam_fw->size);

		if ((cam_fw->size > ZYNQ_CAM_FW_BLOCK_SIZE_MAX) ||
		    (cam_fw->data == NULL)) {
			err = -EINVAL;
			break;
		}
		if (copy_from_user(data, (void __user *)cam_fw->data,
		    cam_fw->size)) {
			zynq_err("%d vid%d "
			    "ZYNQ_IOC_CAM_FLASH_WRITE: copy_from_user failed\n",
			    zdev->zdev_inst, zvideo->index);
			err = -EFAULT;
			break;
		}
		err = zcam_flash_write(zvideo, cam_fw->address,
		    data, cam_fw->size);
		break;
	}
	case ZYNQ_IOC_CAM_CAPS:
	{
		zynq_cam_caps_t *caps = (zynq_cam_caps_t *)arg;

		zynq_trace(ZYNQ_TRACE_PROBE,
		    "%d vid%d ZYNQ_IOC_CAM_CAPS\n",
		    zdev->zdev_inst, zvideo->index);

		if (caps->unique_id == 0xFFFF) {
			(void) zcam_check_caps(zvideo);
		}

		memcpy(caps, &zvideo->caps, sizeof(zynq_cam_caps_t));
		break;
	}
	case ZYNQ_IOC_CAM_RESET:
		zynq_trace(ZYNQ_TRACE_PROBE,
		    "%d vid%d ZYNQ_IOC_CAM_RESET\n",
		    zdev->zdev_inst, zvideo->index);
		spin_lock_bh(&zvideo->qlock);
		zvideo->state |= ZVIDEO_STATE_FAULT;
		spin_unlock_bh(&zvideo->qlock);
		complete(&zvideo->watchdog_completion);
		break;

	case ZYNQ_IOC_TRIGGER_ENABLE_ONE:
		zynq_trace(ZYNQ_TRACE_PROBE,
		    "%d vid%d ZYNQ_IOC_TRIGGER_ENABLE_ONE\n",
		    zdev->zdev_inst, zvideo->index);
		zvideo_trigger_enable(zvideo, arg);
		break;

	case ZYNQ_IOC_TRIGGER_DISABLE_ONE:
		zynq_trace(ZYNQ_TRACE_PROBE,
		    "%d vid%d ZYNQ_IOC_TRIGGER_DISABLE_ONE\n",
		    zdev->zdev_inst, zvideo->index);
		zvideo_trigger_disable(zvideo);
		break;

        case ZYNQ_IOC_TRIGGER_DELAY_SET:
		zynq_trace(ZYNQ_TRACE_PROBE,
		    "%d vid%d ZYNQ_IOC_TRIGGER_DELAY_SET\n",
		    zdev->zdev_inst, zvideo->index);
		zvideo_trigger_delay_set(zvideo, arg);
		break;

        case ZYNQ_IOC_TRIGGER_DELAY_GET:
		zynq_trace(ZYNQ_TRACE_PROBE,
		    "%d vid%d ZYNQ_IOC_TRIGGER_DELAY_GET\n",
		    zdev->zdev_inst, zvideo->index);
		zvideo_trigger_delay_get(zvideo, arg);
		break;

	default:
		zynq_err("%d vid%d unknown ioctl command\n",
		    zdev->zdev_inst, zvideo->index);
		err = -EINVAL;
		break;
	}

	return err;
}

/* The control handler. */
static int zvideo_s_ctrl(struct v4l2_ctrl *ctrl)
{
	zynq_video_t *zvideo =
	    container_of(ctrl->handler, zynq_video_t, ctrl_handler);
	zynq_dev_t *zdev = zvideo->zdev;

	zynq_trace(ZYNQ_TRACE_PROBE, "%d vid%d %s: id=%u\n",
	    zdev->zdev_inst, zvideo->index, __FUNCTION__,
	    ctrl->id - V4L2_CID_BASE);

	return 0;
}

static const struct v4l2_ctrl_ops zvideo_ctrl_ops = {
	.s_ctrl = zvideo_s_ctrl,
};

/*
 * The set of all supported ioctls. Note that all the streaming ioctls
 * use the vb2 helper functions that take care of all the locking and
 * that also do ownership tracking (i.e. only the filehandle that requested
 * the buffers can call the streaming ioctls, all other filehandles will
 * receive -EBUSY if they attempt to call the same streaming ioctls).
 *
 * The last three ioctls also use standard helper functions: these implement
 * standard behavior for drivers with controls.
 */
static const struct v4l2_ioctl_ops video_ioctl_ops = {
	.vidioc_querycap = zvideo_querycap,
	.vidioc_try_fmt_vid_cap = zvideo_try_fmt_vid_cap,
	.vidioc_s_fmt_vid_cap = zvideo_s_fmt_vid_cap,
	.vidioc_g_fmt_vid_cap = zvideo_g_fmt_vid_cap,
	.vidioc_enum_fmt_vid_cap = zvideo_enum_fmt_vid_cap,

	.vidioc_enum_input = zvideo_enum_input,
	.vidioc_g_input = zvideo_g_input,
	.vidioc_s_input = zvideo_s_input,

	.vidioc_reqbufs = vb2_ioctl_reqbufs,
	.vidioc_create_bufs = vb2_ioctl_create_bufs,
	.vidioc_querybuf = vb2_ioctl_querybuf,
	.vidioc_qbuf = vb2_ioctl_qbuf,
	.vidioc_dqbuf = vb2_ioctl_dqbuf,
	.vidioc_expbuf = vb2_ioctl_expbuf,
	.vidioc_streamon = vb2_ioctl_streamon,
	.vidioc_streamoff = vb2_ioctl_streamoff,

	.vidioc_log_status = v4l2_ctrl_log_status,
	.vidioc_subscribe_event = v4l2_ctrl_subscribe_event,
	.vidioc_unsubscribe_event = v4l2_event_unsubscribe,

	.vidioc_default = zvideo_ioctl_default
};

/*
 * The set of file operations. Note that all these ops are standard core
 * helper functions.
 */
static const struct v4l2_file_operations zvideo_fops = {
	.owner = THIS_MODULE,
	.open = v4l2_fh_open,
	.release = vb2_fop_release,
	.unlocked_ioctl = video_ioctl2,
	.read = vb2_fop_read,
	.mmap = vb2_fop_mmap,
	.poll = vb2_fop_poll,
};

static int zvideo_check_link_status(zynq_video_t *zvideo)
{
	zynq_dev_t *zdev = zvideo->zdev;
	u32 status;
	unsigned char prev;
	int changed;

	spin_lock(&zdev->zdev_lock);

	status = zvideo_reg_read(zvideo, ZYNQ_CAM_STATUS);
	zynq_trace(ZYNQ_TRACE_PROBE, "%d vid%d %s: camera status 0x%x\n",
	    zdev->zdev_inst, zvideo->index, __FUNCTION__, status);

	prev = zvideo->caps.link_up;
	zvideo->caps.link_up = !(status & ZYNQ_CAM_FPD_UNLOCK);
	zynq_log("%d vid%d %s: FPD-link %s\n",
	    zdev->zdev_inst, zvideo->index, __FUNCTION__,
	    zvideo->caps.link_up ? "locked" : "NOT locked");

	changed = (zvideo->caps.link_up != prev);

	spin_unlock(&zdev->zdev_lock);

	return changed;
}

static void zvideo_get_camera_info(zynq_video_t *zvideo)
{
	(void) zcam_check_caps(zvideo);
	zvideo_set_format(zvideo);
}

static int zvideo_watchdog(void *arg)
{
	zynq_video_t *zvideo = arg;
	long result;

	while (!kthread_should_stop()) {
		result = wait_for_completion_interruptible_timeout(
		    &zvideo->watchdog_completion, msecs_to_jiffies(1000));
		if (result < 0) {
			break;
		}

		if ((zvideo->state & ZVIDEO_STATE_LINK_CHANGE) &&
		    (jiffies >= (zvideo->ts_link_change +
		    msecs_to_jiffies(ZVIDEO_LINK_CHANGE_DELAY)))) {
			zvideo_link_change(zvideo);
		}

		if (zvideo->state & ZVIDEO_STATE_FAULT) {
			zvideo_reset(zvideo);
		}

		if (result > 0) {
			reinit_completion(&zvideo->watchdog_completion);
		}
	}

	return 0;
}

static void zvideo_watchdog_init(zynq_video_t *zvideo)
{
	init_completion(&zvideo->watchdog_completion);
	zvideo->watchdog_taskp = kthread_run(zvideo_watchdog,
	    zvideo, "zynq video watchdog thread");
}

static void zvideo_watchdog_fini(zynq_video_t *zvideo)
{
	complete(&zvideo->watchdog_completion);
	if (zvideo->watchdog_taskp) {
		kthread_stop(zvideo->watchdog_taskp);
		zvideo->watchdog_taskp = NULL;
	}
}

int zvideo_register_vdev(zynq_video_t *zvideo)
{
	zynq_dev_t *zdev = zvideo->zdev;
	struct video_device *vdev;
	int ret;

	/* Initialize the video_device structure */
	vdev = &zvideo->vdev;
	if (video_is_registered(vdev)) {
		zynq_err("%d vid%d %s: video device already registerd\n",
		    zdev->zdev_inst, zvideo->index, __FUNCTION__);
		return -1;
	}

	memset(vdev, 0, sizeof(struct video_device));

	strlcpy(vdev->name, zvideo->caps.name, sizeof(vdev->name));
	/*
	 * There is nothing to clean up, so release is set to an empty release
	 * function. The release callback must be non-NULL.
	 */
	vdev->release = video_device_release_empty;
	vdev->fops = &zvideo_fops,
	vdev->ioctl_ops = &video_ioctl_ops,
	/*
	 * The main serialization lock. All ioctls are serialized by this
	 * lock. Exception: if q->lock is set, then the streaming ioctls
	 * are serialized by that separate lock.
	 */
	vdev->lock = &zvideo->mlock;
	vdev->queue = &zvideo->queue;
	vdev->v4l2_dev = &zvideo->v4l2_dev;

	video_set_drvdata(vdev, zvideo);

	ret = video_register_device(vdev, VFL_TYPE_GRABBER, -1);
	if (ret) {
		zynq_err("%d vid%d %s: failed to register video device\n",
		    zdev->zdev_inst, zvideo->index, __FUNCTION__);
		return ret;
	}

	zynq_trace(ZYNQ_TRACE_PROBE,
	    "%d vid%d %s: video device registered, /dev/video%d, %s\n",
	    zdev->zdev_inst, zvideo->index, __FUNCTION__,
	    zvideo->vdev.num, zvideo->caps.name);

	return 0;
}

void zvideo_unregister_vdev(zynq_video_t *zvideo)
{
	zynq_dev_t *zdev = zvideo->zdev;

	video_unregister_device(&zvideo->vdev);

	zynq_trace(ZYNQ_TRACE_PROBE,
	    "%d vid%d %s: video device unregistered, /dev/video%d, %s\n",
	    zdev->zdev_inst, zvideo->index, __FUNCTION__,
	    zvideo->vdev.num, zvideo->caps.name);
}

static void zvideo_stats_init(zynq_video_t *zvideo)
{
	int i;

	for (i = 0; i < VIDEO_STATS_NUM; i++) {
		zvideo->stats[i].label = zvideo_stats_label[i];
	}
}

/*
 * The initial setup of this device instance. Note that the initial state of
 * the driver should be complete. So the initial format, standard, timings
 * and video input should all be initialized to some reasonable value.
 */
int zvideo_init(zynq_video_t *zvideo)
{
	zynq_dev_t *zdev = zvideo->zdev;
	zynq_chan_t *zchan = zvideo->zchan;
	struct v4l2_ctrl_handler *hdl;
	struct vb2_queue *q;
	int ret;

	zynq_trace(ZYNQ_TRACE_PROBE, "%d ch%d vid%d %s: zvideo=0x%p\n",
	    zdev->zdev_inst, zchan->zchan_num, zvideo->index,
	    __FUNCTION__, zvideo);

	snprintf(zvideo->prefix, ZYNQ_LOG_PREFIX_LEN,
	    "%d vid%d", zdev->zdev_inst, zvideo->index);
	zvideo_stats_init(zvideo);

	zvideo->reg_base = zdev->zdev_bar0 +
	    (ZYNQ_CAM_REG_OFFSET * zvideo->index);

	INIT_LIST_HEAD(&zvideo->buf_list);
	INIT_LIST_HEAD(&zvideo->pending_list);
	spin_lock_init(&zvideo->rlock);
	spin_lock_init(&zvideo->qlock);
	mutex_init(&zvideo->mlock);
	mutex_init(&zvideo->slock);

	/* Initialize the top-level structure */
	ret = v4l2_device_register(&zdev->zdev_pdev->dev, &zvideo->v4l2_dev);
	if (ret) {
		zynq_err("%d ch%d vid%d %s: failed to regiser v4l2\n",
		    zdev->zdev_inst, zchan->zchan_num,
		    zvideo->index, __FUNCTION__);
		return -1;
	}

	/* Add the controls */
	hdl = &zvideo->ctrl_handler;
	v4l2_ctrl_handler_init(hdl, 0);
	zvideo->v4l2_dev.ctrl_handler = hdl;

	/* Initialize the vb2 queue */
	q = &zvideo->queue;
	q->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	q->io_modes = VB2_MMAP;
	q->drv_priv = zvideo;
	q->buf_struct_size = sizeof (zynq_video_buffer_t);
	q->ops = &video_qops;
	q->mem_ops = &vb2_vmalloc_memops;
	q->timestamp_flags = V4L2_BUF_FLAG_TIMESTAMP_COPY;
	q->min_buffers_needed = zynq_video_buf_num;
	/*
	 * The serialization lock for streaming ioctls. It could be the same
	 * as the main serialization lock, but if some of the non-streaming
	 * ioctls could take a long time to execute, then we might want to
	 * have a different lock here to prevent VIDIOC_DQBUF from being
	 * blocked while waiting for another action to finish. Here we use
	 * a different lock.
	 */
	q->lock = &zvideo->slock;
	q->gfp_flags = GFP_DMA;

	ret = vb2_queue_init(q);
	if (ret) {
		zynq_err("%d ch%d vid%d %s: failed to init vb2 queue\n",
		    zdev->zdev_inst, zchan->zchan_num,
		    zvideo->index, __FUNCTION__);
		goto free_hdl;
	}

	/* Check the camera link status and register video device */
	(void) zvideo_check_link_status(zvideo);

	if (zvideo->caps.link_up) {
		zvideo_get_camera_info(zvideo);

		ret = zvideo_register_vdev(zvideo);
		if (ret) {
			goto free_hdl;
		}
	}

	zvideo_watchdog_init(zvideo);

	zynq_trace(ZYNQ_TRACE_PROBE, "%d ch%d vid%d %s done\n",
	    zdev->zdev_inst, zchan->zchan_num, zvideo->index, __FUNCTION__);

	return 0;

free_hdl:
	v4l2_ctrl_handler_free(&zvideo->ctrl_handler);
	v4l2_device_unregister(&zvideo->v4l2_dev);

	return ret;
}

void zvideo_fini(zynq_video_t *zvideo)
{
	zynq_dev_t *zdev = zvideo->zdev;
	zynq_chan_t *zchan = zvideo->zchan;

	zynq_trace(ZYNQ_TRACE_PROBE, "%d ch%d vid%d %s %s: zvideo=0x%p\n",
	    zdev->zdev_inst, zchan->zchan_num, zvideo->index,
	    zvideo->caps.name, __FUNCTION__, zvideo);

	zvideo_watchdog_fini(zvideo);
	zvideo_unregister_vdev(zvideo);
	v4l2_ctrl_handler_free(&zvideo->ctrl_handler);
	v4l2_device_unregister(&zvideo->v4l2_dev);
}

void zvideo_link_change(zynq_video_t *zvideo)
{
	zynq_dev_t *zdev = zvideo->zdev;
	int link_changed;

	spin_lock_bh(&zvideo->qlock);
	zvideo->state &= ~ZVIDEO_STATE_LINK_CHANGE;

	link_changed = zvideo_check_link_status(zvideo);
	if (!link_changed) {
		spin_unlock_bh(&zvideo->qlock);
		return;
	}

	if (zvideo->state & ZVIDEO_STATE_STREAMING) {
		zynq_err("%d vid%d %s: device busy\n",
		    zdev->zdev_inst, zvideo->index, __FUNCTION__);
	}
	spin_unlock_bh(&zvideo->qlock);

	if (zvideo->caps.link_up) {
		zvideo_get_camera_info(zvideo);
		(void) zvideo_register_vdev(zvideo);
	} else {
		zvideo_unregister_vdev(zvideo);
	}
}

static inline int zvideo_data_range_check(unsigned long offset,
		unsigned long start, unsigned long end,
		void **vaddr, unsigned long *len)
{
	unsigned long size = *len;
	long diff;

	if (((offset + size) >= start) && (offset < end)) {
		diff = start - offset;
		if (diff > 0) {
			*vaddr = (void *)((uintptr_t)*vaddr + diff);
			*len -= diff;
		}
		diff = offset + size - end;
		if (diff > 0) {
			*len -= diff;
		}
		return 1;
	}

	return 0;
}

static void zvideo_rx_proc_copy(zynq_video_t *zvideo)
{
	zynq_chan_t *zchan = zvideo->zchan;
	zchan_rx_tbl_t *zchan_rx = &zchan->zchan_rx_tbl;
	zynq_video_buffer_t *buf;
	struct vb2_buffer *vb;
	unsigned char *mem;
	unsigned long memsz;
	unsigned long size;
	unsigned long len;
	u32 rx_head;
	u32 rx_tail;
	u32 rx_off;
	void *data_phy;
	void *data;

	rx_head = zchan_rx->zchan_rx_head;
	rx_tail = zchan_reg_read(zchan, ZYNQ_CH_RX_TAIL);

	rx_off = rx_head;

	if ((rx_head >= zchan_rx->zchan_rx_size) ||
	    (rx_tail >= zchan_rx->zchan_rx_size)) {
		zynq_err("%d ch%d vid%d %s check failed: "
		    "rx_head=0x%x, rx_tail=0x%x, max_off=0x%x\n",
		    ZYNQ_INST(zchan), zchan->zchan_num,
		    zvideo->index, __FUNCTION__,
		    rx_head, rx_tail, zchan_rx->zchan_rx_size - 1);
		return;
	}

	zvideo_get_timestamp(&zvideo->tv_intr);

	zynq_trace(ZYNQ_TRACE_PROBE,
	    "%d ch%d vid%d %s: rx_head=0x%x, rx_tail=0x%x\n",
	    ZYNQ_INST(zchan), zchan->zchan_num, zvideo->index, __FUNCTION__,
	    rx_head, rx_tail);

	do {
		buf = NULL;
		size = 0;

		/* Retrieve one buffer from the driver owned buffer list */
		spin_lock(&zvideo->qlock);
		if (zvideo->state & ZVIDEO_STATE_STREAMING) {
			if (!list_empty(&zvideo->buf_list)) {
				buf = list_first_entry(&zvideo->buf_list,
				    zynq_video_buffer_t, list);
				list_del(&buf->list);
				zvideo->buf_avail--;
			}
		}
		spin_unlock(&zvideo->qlock);

		if (buf == NULL) {
			ZYNQ_STATS_LOG(zvideo, VIDEO_STATS_NO_VIDEO_BUF);

			if (ZCHAN_RX_FREE_SIZE(zchan_rx, rx_off, rx_tail) >=
			    zvideo->format.sizeimage) {
				/*
				 * There are still available DMA buffers,
				 * we simply return and leave the data to
				 * be handled with the next interrupt.
				 */
				return;
			}

			ZYNQ_STATS_LOGX(zchan, CHAN_STATS_RX_DROP, 1, 0);

			/* Discard the current frame data */
			rx_off += zvideo->format.sizeimage;
			goto next;
		}

		vb = &buf->vbuf.vb2_buf;

		/* Get the virtual address and size of the video buffer */
		mem = zvideo_buffer_vaddr(buf);
		memsz = zvideo_buffer_size(buf);

		while ((rx_off != rx_tail) && (memsz > 0) &&
		    (size < zvideo->format.sizeimage)) {
			/*
			 * Retrive the actual addresses of the data
			 */
			zchan_rx_off2addr(zchan_rx, rx_off, &data, &data_phy);

			/* Calculate the length of the data to be copied */
			if (ZCHAN_WITHIN_RX_BUF(zchan_rx, rx_off, rx_tail) &&
			    (rx_off <= rx_tail)) {
				len = rx_tail - rx_off;
			} else {
				len = zchan_rx->zchan_rx_bufsz -
				    ZCHAN_RX_BUF_OFFSET(zchan_rx, rx_off);
			}

			/* Limit the length when buffer space is not enough */
			if (len > memsz) {
				len = memsz;
			}

			pci_dma_sync_single_for_cpu(zchan->zdev->zdev_pdev,
			    (dma_addr_t)data_phy, len, PCI_DMA_FROMDEVICE);

			/* Copy the data to the video buffer */
			memcpy(mem, data, len);

			mem += len;
			memsz -= len;
			rx_off += len;
			if (rx_off >= zchan_rx->zchan_rx_size) {
				rx_off -= zchan_rx->zchan_rx_size;
			}
			size += len;
		}
		ASSERT((memsz == 0) && (size == zvideo->format.sizeimage));

		ZYNQ_STATS(zchan, CHAN_STATS_RX);
next:
		/*
		 * Reset the meta data right after the video frame is copied
		 * to the video buffer or is discarded.
		 */
		zvideo_reset_meta_data(zvideo, zchan_rx->zchan_rx_head);

		/*
		 * Advance the pointer to the beginning of next video frame.
		 * Video frame data always starts at the beginning boundary
		 * of a DMA buffer (a page).
		 */
		rx_off = ALIGN(rx_off, zchan_rx->zchan_rx_bufsz);
		if (rx_off >= zchan_rx->zchan_rx_size) {
			rx_off -= zchan_rx->zchan_rx_size;
		}

		/* Update head index */
		zchan_rx->zchan_rx_head = rx_off;
		zchan_reg_write(zchan, ZYNQ_CH_RX_HEAD, rx_off);

		zynq_trace(ZYNQ_TRACE_PROBE, "%d ch%d vid%d %s: "
		    "size=0x%lx, rx_off=0x%x, rx_tail=0x%x\n",
		    ZYNQ_INST(zchan), zchan->zchan_num, zvideo->index,
		    __FUNCTION__, size, rx_off, rx_tail);

		if (buf != NULL) {
			if (zvideo_check_meta_data(zvideo, buf)) {
				/*
				 * If the video frame has error, discard
				 * the frame, and return the buffer to
				 * the buffer list.
				 */
				spin_lock(&zvideo->qlock);
				list_add_tail(&buf->list, &zvideo->buf_list);
				zvideo->buf_avail++;
				spin_unlock(&zvideo->qlock);
			} else {
				zynq_trace(ZYNQ_TRACE_PROBE,
				    "%d ch%d vid%d %s: "
				    "buffer done, vb=0x%p, state=%d\n",
				    ZYNQ_INST(zchan), zchan->zchan_num,
				    zvideo->index, __FUNCTION__, vb, vb->state);
				vb2_set_plane_payload(vb, 0, size);
				vb2_buffer_done(vb, VB2_BUF_STATE_DONE);
			}
		}

		zvideo->sequence++;

	} while (rx_off != rx_tail);
}

static void zvideo_rx_proc_zero_copy(zynq_video_t *zvideo)
{
	zynq_chan_t *zchan = zvideo->zchan;
	zchan_rx_tbl_t *zchan_rx = &zchan->zchan_rx_tbl;
	zynq_video_buffer_t *buf, *cur, *next;
	struct vb2_buffer *vb;
	u32 rx_tail, rx_off;
	unsigned long memsz;
	unsigned long size, len = 0;
	void *data_phy = NULL;

	if (zchan_rx->zchan_rx_bufsz == 0) {
		zynq_err("%d vid%d ch%d %s: DMA is not initialized\n",
		    ZYNQ_INST(zchan), zvideo->index, zchan->zchan_num,
		    __FUNCTION__);
		return;
	}

	ASSERT(zchan_rx->zchan_rx_head ==
	    zchan_reg_read(zchan, ZYNQ_CH_RX_HEAD));
	rx_tail = zchan_reg_read(zchan, ZYNQ_CH_RX_TAIL);

	if (ZCHAN_RX_USED_SIZE(zchan_rx, zchan_rx->zchan_rx_head, rx_tail) %
	    ALIGN(zvideo->format.sizeimage, zchan_rx->zchan_rx_bufsz)) {
		zynq_err("%d vid%d ch%d %s: index check failed, "
		    "rx_head=0x%x, rx_tail=0x%x\n",
		    ZYNQ_INST(zchan), zvideo->index, zchan->zchan_num,
		    __FUNCTION__, zchan_rx->zchan_rx_head, rx_tail);
		return;
	}

	rx_off = zchan_rx->zchan_rx_off;
	if (rx_off == rx_tail) {
		zynq_err("%d vid%d ch%d %s: no data to receive, "
		    "rx_head=0x%x, rx_tail=0x%x, rx_off=0x%x\n",
		    ZYNQ_INST(zchan), zvideo->index, zchan->zchan_num,
		    __FUNCTION__, zchan_rx->zchan_rx_head, rx_tail, rx_off);
		return;
	}

	zvideo_get_timestamp(&zvideo->tv_intr);

	zynq_trace(ZYNQ_TRACE_PROBE, "%d vid%d ch%d %s: begin, "
	    "rx_head=0x%x, rx_tail=0x%x, rx_off=0x%x\n",
	    ZYNQ_INST(zchan), zvideo->index, zchan->zchan_num, __FUNCTION__,
	    zchan_rx->zchan_rx_head, rx_tail, rx_off);

	do {
		buf = NULL;
		size = 0;

		/*
		 * Find the video buffer bound to the current offset
		 */
		spin_lock(&zvideo->qlock);
		if (zvideo->state & ZVIDEO_STATE_STREAMING) {
			list_for_each_entry_safe(cur, next,
			    &zvideo->buf_list, list) {
				if (cur->offset == rx_off) {
					list_del(&cur->list);
					zvideo->buf_avail--;
					buf = cur;
					break;
				}
			}
		}
		spin_unlock(&zvideo->qlock);

		ASSERT(buf != NULL);
		if (buf == NULL) {
			/*
			 * This is not supposed to happen during streaming.
			 * The hardware should not issue the interrupt
			 * when there's no bound DMA buffer to receive
			 * the video data.
			 */
			ZYNQ_STATS_LOGX(zvideo,
			    VIDEO_STATS_NO_VIDEO_BUF, 1, 0);
			return;
		}

		vb = &buf->vbuf.vb2_buf;

		zynq_trace(ZYNQ_TRACE_PROBE, "%d vid%d ch%d %s: buffer avail, "
		    "vb=0x%p, vb.state=%d, buf.offset=0x%x, rx_off=0x%x\n",
		    ZYNQ_INST(zchan), zvideo->index, zchan->zchan_num,
		    __FUNCTION__, vb, vb->state, buf->offset, rx_off);

		/* Get the size of the video buffer */
		memsz = zvideo_buffer_size(buf);

		while ((rx_off != rx_tail) && (memsz > 0) &&
		    (size < zvideo->format.sizeimage)) {
			/*
			 * Retrive the actual addresses of the data
			 */
			zchan_rx_off2addr(zchan_rx, rx_off, NULL, &data_phy);

			/* Calculate the length of the data to sync */
			if (ZCHAN_WITHIN_RX_BUF(zchan_rx, rx_off, rx_tail) &&
			    (rx_off <= rx_tail)) {
				len = rx_tail - rx_off;
			} else {
				len = zchan_rx->zchan_rx_bufsz -
				    ZCHAN_RX_BUF_OFFSET(zchan_rx, rx_off);
			}

			/* Limit the length when buffer space is not enough */
			if (len > memsz) {
				len = memsz;
			}

			pci_dma_sync_single_for_cpu(zchan->zdev->zdev_pdev,
			    (dma_addr_t)data_phy, len, PCI_DMA_FROMDEVICE);

			rx_off += len;
			if (rx_off >= zchan_rx->zchan_rx_size) {
				rx_off -= zchan_rx->zchan_rx_size;
			}
			size += len;
			memsz -= len;
		}
		ASSERT((memsz == 0) && (size == zvideo->format.sizeimage));
		if ((memsz != 0) || ( size != zvideo->format.sizeimage)) {
			zynq_err("%d vid%d ch%d %s: expected size=%u, "
			    "received size=%lu, avail memsz=%lu, "
			    "rx_head=0x%x, rx_tail=0x%x, rx_off=0x%x\n",
			    ZYNQ_INST(zchan), zvideo->index,
			    zchan->zchan_num, __FUNCTION__,
			    zvideo->format.sizeimage, size, memsz,
			    zchan_rx->zchan_rx_head, rx_tail, rx_off);
			rx_off += memsz;
		}

		ZYNQ_STATS(zchan, CHAN_STATS_RX);

		if (zvideo_check_meta_data(zvideo, buf)) {
			/*
			 * If the video frame has error, discard the frame.
			 * Put the buffer to the pending list for further
			 * handling.
			 */
			spin_lock(&zvideo->qlock);
			list_add(&buf->list, &zvideo->pending_list);
			zvideo_check_pending(zvideo);
			spin_unlock(&zvideo->qlock);
		} else {
			zynq_trace(ZYNQ_TRACE_PROBE, "%d vid%d ch%d %s: "
			    "buffer done, vb=0x%p, vb.state=%d\n",
			    ZYNQ_INST(zchan), zvideo->index, zchan->zchan_num,
			    __FUNCTION__, vb, vb->state);
			vb2_set_plane_payload(vb, 0, size);
			vb2_buffer_done(vb, VB2_BUF_STATE_DONE);
		}

		/*
		 * Advance the pointer to the beginning of next video frame.
		 * Video frame data always starts at the beginning boundary
		 * of a DMA buffer (a page).
		 */
		rx_off = ALIGN(rx_off, zchan_rx->zchan_rx_bufsz);
		if (rx_off >= zchan_rx->zchan_rx_size) {
			rx_off -= zchan_rx->zchan_rx_size;
		}

		/*
		 * Update global offset. The head pointer will be updated
		 * later when the video buffer is returned from user space.
		 */
		zchan_rx->zchan_rx_off = rx_off;

		zynq_trace(ZYNQ_TRACE_PROBE, "%d vid%d ch%d %s: end, "
		    "rx_head=0x%x, rx_tail=0x%x, rx_off=0x%x, size=0x%lx\n",
		    ZYNQ_INST(zchan), zvideo->index, zchan->zchan_num,
		    __FUNCTION__, zchan_rx->zchan_rx_head, rx_tail, rx_off, size);

		zvideo->sequence++;

	} while (rx_off != rx_tail);
}

void zvideo_rx_proc(zynq_video_t *zvideo)
{
	spin_lock(&zvideo->rlock);
	if (zynq_video_zero_copy) {
		zvideo_rx_proc_zero_copy(zvideo);
	} else {
		zvideo_rx_proc_copy(zvideo);
	}
	spin_unlock(&zvideo->rlock);
}

void zvideo_err_proc(zynq_video_t *zvideo, int ch_err_status)
{
	/* error statistics */
	if (ch_err_status & ZYNQ_CH_ERR_CAM_TRIGGER) {
		ZYNQ_STATS_LOG(zvideo, VIDEO_STATS_TRIG_PULSE_ERR);
	}

	if (ch_err_status & ZYNQ_CH_ERR_CAM_LINK_CHANGE) {
		ZYNQ_STATS_LOG(zvideo, VIDEO_STATS_LINK_CHANGE);
		spin_lock(&zvideo->qlock);
		zvideo->state |= ZVIDEO_STATE_LINK_CHANGE;
		zvideo->ts_link_change = jiffies;
		spin_unlock(&zvideo->qlock);
	}

	if (ch_err_status & ZYNQ_CH_ERR_DMA_RX_BUF_FULL) {
		ZYNQ_STATS_LOG(zvideo, VIDEO_STATS_DMA_RX_BUF_FULL);
	}

	if (ch_err_status & ZYNQ_CH_ERR_CAM_FPD_UNLOCK) {
		ZYNQ_STATS_LOG(zvideo, VIDEO_STATS_FPD_LINK_UNLOCK);
	}

	if (ch_err_status & ZYNQ_CH_ERR_CAM_FIFO_FULL) {
		ZYNQ_STATS_LOG(zvideo, VIDEO_STATS_FIFO_FULL);
	}
}

void zvideo_get_timestamp(struct timeval *tv)
{
	struct timespec ts;

	ktime_get_real_ts(&ts);
	tv->tv_sec = ts.tv_sec;
	tv->tv_usec = ts.tv_nsec / NSEC_PER_USEC;
}

module_param_named(videofmt, zynq_video_format, uint, S_IRUGO|S_IWUSR);
MODULE_PARM_DESC(videofmt, "video format");
module_param_named(videobufnum, zynq_video_buf_num, uint, S_IRUGO|S_IWUSR);
MODULE_PARM_DESC(videobufnum, "video buffer number");
module_param_named(videozerocopy, zynq_video_zero_copy, uint, S_IRUGO|S_IWUSR);
MODULE_PARM_DESC(videozerocopy, "video zero copy");
module_param_named(videotstype, zynq_video_ts_type, uint, S_IRUGO|S_IWUSR);
MODULE_PARM_DESC(videotstype, "video timestamp type");
module_param_named(videoflash16, zynq_video_flash_16, int, S_IRUGO|S_IWUSR);
MODULE_PARM_DESC(videoflash16, "camera flash 16b address width");
module_param_named(videopinswap, zynq_video_pin_swap, int, S_IRUGO|S_IWUSR);
MODULE_PARM_DESC(videopinswap, "camera pin swap");
module_param_named(videoerrtol, zynq_video_err_tolerant, uint, S_IRUGO|S_IWUSR);
MODULE_PARM_DESC(videoerrtol, "video error tolerant");
module_param_named(videoerrthr, zynq_video_err_thresh, uint, S_IRUGO|S_IWUSR);
MODULE_PARM_DESC(videoerrthr, "video error threshold");
