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

#define	I2C_ID_CAM_DES_PRLL(i)	(0x60 + (i))
#define	I2C_ID_CAM_ISP		0x48

#define	ZCAM_WAIT_DELAY		100	/* usec */
#define	ZCAM_WAIT_RETRY		50

unsigned char zcam_dev_cfgs[2][8] = {
	/*
	 * Driver ID, Read Command, Address Width, Magic Number,
	 * Flash Size (4 bytes)
	 */
	{ 0x02, 0x00, 0x03, 0x18, 0x00, 0x00, 0x80, 0x00 },
	{ 0x02, 0x00, 0x02, 0x10, 0x00, 0x00, 0x80, 0x00 }
};

static inline void zcam_i2c_acc_init(zynq_video_t *zvideo,
		ioc_zynq_i2c_acc_t *i2c_acc)
{
	i2c_acc->i2c_id = I2C_ID_CAM_DES_PRLL(zvideo->index);
	i2c_acc->i2c_bus = 2;
}

static int zcam_i2c_init(zynq_video_t *zvideo)
{
	zynq_dev_t *zdev = zvideo->zdev;
	ioc_zynq_i2c_acc_t i2c_acc;
	unsigned char i2c_addrs[2] = { 0x10, 0x08 };
	int i;
	int ret;

	zynq_trace(ZYNQ_TRACE_PROBE, "%d vid%d %s\n",
	    zdev->zdev_inst, zvideo->index, __FUNCTION__);

	zcam_i2c_acc_init(zvideo, &i2c_acc);

	/* Program the slave id of the ISP */
	for (i = 0; i < 2; i++) {
		i2c_acc.i2c_addr_hi = 0;
		i2c_acc.i2c_addr = i2c_addrs[i];
		i2c_acc.i2c_data = I2C_ID_CAM_ISP << 1;
		i2c_acc.i2c_addr_16 = 0;
		ret = zdev_i2c_write(zdev, &i2c_acc);
		if (ret ) {
			return ret;
		}
	}

	return 0;
}

static void zcam_i2c_fini(zynq_video_t *zvideo)
{
	zynq_dev_t *zdev = zvideo->zdev;
	ioc_zynq_i2c_acc_t i2c_acc;
	unsigned char addrs[2] = { 0x10, 0x08 };
	int i;

	zynq_trace(ZYNQ_TRACE_PROBE, "%d vid%d %s\n",
	    zdev->zdev_inst, zvideo->index, __FUNCTION__);

	zcam_i2c_acc_init(zvideo, &i2c_acc);

	/* Clean the previous settings */
	for (i = 0; i < 2; i++) {
		i2c_acc.i2c_addr_hi = 0;
		i2c_acc.i2c_addr = addrs[i];
		i2c_acc.i2c_data = 0;
		i2c_acc.i2c_addr_16 = 0;
		(void) zdev_i2c_write(zdev, &i2c_acc);
	}
}

static int zcam_i2c_read8(zynq_video_t *zvideo,
		unsigned short addr, unsigned char *data)
{
	zynq_dev_t *zdev = zvideo->zdev;
	ioc_zynq_i2c_acc_t i2c_acc;
	int ret;

	zynq_trace(ZYNQ_TRACE_PROBE, "%d vid%d %s: addr=0x%04x\n",
	    zdev->zdev_inst, zvideo->index, __FUNCTION__, addr);

	zcam_i2c_acc_init(zvideo, &i2c_acc);

	i2c_acc.i2c_id = I2C_ID_CAM_ISP;
	i2c_acc.i2c_addr_hi = (addr >> 8) & 0xFF;
	i2c_acc.i2c_addr = addr & 0xFF;
	i2c_acc.i2c_data = 0;
	i2c_acc.i2c_addr_16 = 1;

	ret = zdev_i2c_read(zdev, &i2c_acc);
	if (ret ) {
		return ret;
	}

	*data = i2c_acc.i2c_data;

	zynq_trace(ZYNQ_TRACE_PROBE,
	    "%d vid%d %s: Done. addr=0x%04x data=0x%02x\n",
	    zdev->zdev_inst, zvideo->index, __FUNCTION__, addr, *data);

	return 0;
}

static int zcam_i2c_read16(zynq_video_t *zvideo,
		unsigned short addr, unsigned short *data)
{
	zynq_dev_t *zdev = zvideo->zdev;
	unsigned char *bytes = (unsigned char *)data;
	int ret;
	int i;

	zynq_trace(ZYNQ_TRACE_PROBE, "%d vid%d %s: addr=0x%04x\n",
	    zdev->zdev_inst, zvideo->index, __FUNCTION__, addr);

	for (i = 0; i < 2; i++) {
		ret = zcam_i2c_read8(zvideo, addr + i, &bytes[1 - i]);
		if (ret) {
			return ret;
		}
	}

	zynq_trace(ZYNQ_TRACE_PROBE,
	    "%d vid%d %s: Done. addr=0x%04x data=0x%04x\n",
	    zdev->zdev_inst, zvideo->index, __FUNCTION__, addr, *data);

	return 0;
}

static int zcam_i2c_read32(zynq_video_t *zvideo,
		unsigned short addr, unsigned int *data)
{
	zynq_dev_t *zdev = zvideo->zdev;
	unsigned char *bytes = (unsigned char *)data;
	int ret;
	int i;

	zynq_trace(ZYNQ_TRACE_PROBE, "%d vid%d %s: addr=0x%04x\n",
	    zdev->zdev_inst, zvideo->index, __FUNCTION__, addr);

	for (i = 0; i < 4; i++) {
		ret = zcam_i2c_read8(zvideo, addr + i, &bytes[3 - i]);
		if (ret) {
			return ret;
		}
	}

	zynq_trace(ZYNQ_TRACE_PROBE,
	    "%d vid%d %s: Done. addr=0x%04x data=0x%08x\n",
	    zdev->zdev_inst, zvideo->index, __FUNCTION__, addr, *data);

	return 0;
}

static int zcam_i2c_write8(zynq_video_t *zvideo,
		unsigned short addr, unsigned char data)
{
	zynq_dev_t *zdev = zvideo->zdev;
	ioc_zynq_i2c_acc_t i2c_acc;
	int ret;

	zynq_trace(ZYNQ_TRACE_PROBE,
	    "%d vid%d %s: addr=0x%04x data=0x%02x\n",
	    zdev->zdev_inst, zvideo->index, __FUNCTION__, addr, data);

	zcam_i2c_acc_init(zvideo, &i2c_acc);

	i2c_acc.i2c_id = I2C_ID_CAM_ISP;
	i2c_acc.i2c_addr_hi = (addr >> 8) & 0xFF;
	i2c_acc.i2c_addr = addr & 0xFF;
	i2c_acc.i2c_data = data;
	i2c_acc.i2c_addr_16 = 1;

	ret = zdev_i2c_write(zdev, &i2c_acc);
	if (ret ) {
		return ret;
	}

	zynq_trace(ZYNQ_TRACE_PROBE,
	    "%d vid%d %s: Done. addr=0x%04x data=0x%02x\n",
	    zdev->zdev_inst, zvideo->index, __FUNCTION__, addr, data);

	return 0;
}

static int zcam_i2c_write16(zynq_video_t *zvideo,
		unsigned short addr, unsigned short data)
{
	zynq_dev_t *zdev = zvideo->zdev;
	unsigned char *bytes = (unsigned char *)&data;
	int ret;
	int i;

	zynq_trace(ZYNQ_TRACE_PROBE,
	    "%d vid%d %s: addr=0x%04x data=0x%04x\n",
	    zdev->zdev_inst, zvideo->index, __FUNCTION__, addr, data);

	for (i = 0; i < 2; i++) {
		ret = zcam_i2c_write8(zvideo, addr + i, bytes[1 - i]);
		if (ret) {
			return ret;
		}
	}

	zynq_trace(ZYNQ_TRACE_PROBE,
	    "%d vid%d %s: Done. addr=0x%04x data=0x%04x\n",
	    zdev->zdev_inst, zvideo->index, __FUNCTION__, addr, data);

	return 0;
}

static int zcam_i2c_write32(zynq_video_t *zvideo,
		unsigned short addr, unsigned int data)
{
	zynq_dev_t *zdev = zvideo->zdev;
	unsigned char *bytes = (unsigned char *)&data;
	int ret;
	int i;

	zynq_trace(ZYNQ_TRACE_PROBE,
	    "%d vid%d %s: addr=0x%04x data=0x%08x\n",
	    zdev->zdev_inst, zvideo->index, __FUNCTION__, addr, data);

	for (i = 0; i < 4; i++) {
		ret = zcam_i2c_write8(zvideo, addr + i, bytes[3 - i]);
		if (ret) {
			return ret;
		}
	}

	zynq_trace(ZYNQ_TRACE_PROBE,
	    "%d vid%d %s: Done. addr=0x%04x data=0x%08x\n",
	    zdev->zdev_inst, zvideo->index, __FUNCTION__, addr, data);

	return 0;
}

int zcam_reg_read(zynq_video_t *zvideo, zynq_cam_acc_t *cam_acc)
{
	zynq_dev_t *zdev = zvideo->zdev;
	int ret;

	spin_lock(&zdev->zdev_lock);
	if ((ret = zcam_i2c_init(zvideo))) {
		goto end;
	}

	switch (cam_acc->data_sz) {
	case 1:
		ret = zcam_i2c_read8(zvideo, cam_acc->addr,
		    (unsigned char *)&cam_acc->data);
		break;
	case 2:
		ret = zcam_i2c_read16(zvideo, cam_acc->addr,
		    (unsigned short *)&cam_acc->data);
		break;
	case 4:
		ret = zcam_i2c_read32(zvideo, cam_acc->addr, &cam_acc->data);
		break;
	default:
		zynq_err("%d vid%d %s: ERROR invalid data size %d\n",
		    zdev->zdev_inst, zvideo->index, __FUNCTION__,
		    cam_acc->data_sz);
		ret = -1;
		break;
	}
end:
	zcam_i2c_fini(zvideo);
	spin_unlock(&zdev->zdev_lock);

	return ret;
}

int zcam_reg_write(zynq_video_t *zvideo, zynq_cam_acc_t *cam_acc)
{
	zynq_dev_t *zdev = zvideo->zdev;
	int ret;

	spin_lock(&zdev->zdev_lock);
	if ((ret = zcam_i2c_init(zvideo))) {
		goto end;
	}

	switch (cam_acc->data_sz) {
	case 1:
		ret = zcam_i2c_write8(zvideo, cam_acc->addr,
		    (unsigned char)cam_acc->data);
		break;
	case 2:
		ret = zcam_i2c_write16(zvideo, cam_acc->addr,
		    (unsigned short)cam_acc->data);
		break;
	case 4:
		ret = zcam_i2c_write32(zvideo, cam_acc->addr, cam_acc->data);
		break;
	default:
		zynq_err("%d vid%d %s: ERROR invalid data size %d\n",
		    zdev->zdev_inst, zvideo->index, __FUNCTION__,
		    cam_acc->data_sz);
		ret = -1;
		break;
	}
end:
	zcam_i2c_fini(zvideo);
	spin_unlock(&zdev->zdev_lock);

	return ret;
}

static void zcam_init_time_params(zynq_video_t *zvideo)
{
	zynq_cam_caps_t *caps = &zvideo->caps;

	zvideo->t_row = T_ROW(caps->line_len_pck, caps->pixel_clock);
	zvideo->t_first = T_FRAME(caps->frame_len_lines,
	    caps->line_len_pck, caps->pixel_clock);
	zvideo->t_middle = zvideo->t_first + T_FRAME(ZVIDEO_IMAGE_HEIGHT / 2,
	    caps->line_len_pck, caps->pixel_clock);
	zvideo->t_last = zvideo->t_first + T_FRAME(ZVIDEO_IMAGE_HEIGHT,
	    caps->line_len_pck, caps->pixel_clock);
}

static void zcam_read_embedded_data_230(zynq_video_t *zvideo, void *buf)
{
	zynq_dev_t *zdev = zvideo->zdev;
	unsigned int frame_count = 0;
	unsigned short frame_lines = 0;
	unsigned short line_pck = 0;
	unsigned short coarse_int = 0;

	if (EM_REG_VAL_230(buf, BASE) == EM_REG_CHIP_VER) {
		frame_count = EM_REG_VAL_230(buf, FRAME_CNT);
		zvideo->last_meta_cnt = frame_count;

		frame_lines = EM_REG_VAL_230(buf, FRAME_LNS);
		if (frame_lines != zvideo->caps.frame_len_lines) {
			zynq_log("%d vid%d %s: changed frame_length_lines, "
			    "new: %u, old: %u\n",
			    zdev->zdev_inst, zvideo->index, __FUNCTION__,
			    frame_lines, zvideo->caps.frame_len_lines);
			zvideo->caps.frame_len_lines = frame_lines;
			zcam_init_time_params(zvideo);
		}

		line_pck = EM_REG_VAL_230(buf, LINE_PCK) << 1;
		if (line_pck != zvideo->caps.line_len_pck) {
			zynq_log("%d vid%d %s: changed line_length_pck, "
			    "new: %u, old: %u\n",
			    zdev->zdev_inst, zvideo->index, __FUNCTION__,
			    line_pck, zvideo->caps.line_len_pck);
			zvideo->caps.line_len_pck = line_pck;
			zcam_init_time_params(zvideo);
		}

		coarse_int = EM_REG_VAL_230(buf, COARSE_INT);
		zvideo->last_exp_time = T_FRAME(coarse_int,
		    zvideo->caps.line_len_pck, zvideo->caps.pixel_clock);
	}

	zynq_trace(ZYNQ_TRACE_PROBE, "%d vid%d %s: "
	    "sequence=%u, frame_count=%u, frame_lines=%u, line_pck=%u, "
	    "coarse_int=%u, exposure_time=%uus\n",
	    zdev->zdev_inst, zvideo->index, __FUNCTION__,
	    zvideo->sequence, frame_count, frame_lines, line_pck,
	    coarse_int, zvideo->last_exp_time);
}

static void zcam_read_embedded_data_231(zynq_video_t *zvideo, void *buf)
{
	zynq_dev_t *zdev = zvideo->zdev;
	unsigned int frame_count = 0;
	unsigned int t1_clk = 0;
	unsigned int t1_row = 0;
	unsigned int line_pck = 0;

	if (EM_REG_VAL_231(buf, BASE1) == EM_REG_FRAME_CNT_HI) {
		frame_count = (EM_REG_VAL_231(buf, FRAME_CNT_HI) << 16) |
		    EM_REG_VAL_231(buf, FRAME_CNT_LO);
		zvideo->last_meta_cnt = frame_count;
	}

	if (EM_REG_VAL_231(buf, BASE2) == EM_REG_EXP_T1_ROW) {
		t1_row = EM_REG_VAL_231(buf, EXP_T1_ROW);
		t1_clk = (EM_REG_VAL_231(buf, EXP_T1_CLK_HI) << 16) |
		    EM_REG_VAL_231(buf, EXP_T1_CLK_LO);
		line_pck = t1_clk / t1_row;
		zvideo->last_exp_time = T_FRAME(t1_row,
		    zvideo->caps.line_len_pck, zvideo->caps.pixel_clock);
	}

	zynq_trace(ZYNQ_TRACE_PROBE, "%d vid%d %s: "
	    "sequence=%u, frame_count=%u, t1_row=%u, t1_clk=%u, "
	    "line_pck=%u, exposure_time=%uus\n",
	    zdev->zdev_inst, zvideo->index, __FUNCTION__,
	    zvideo->sequence, frame_count, t1_row, t1_clk, line_pck,
	    zvideo->last_exp_time);
}

static int zcam_probe(zynq_video_t *zvideo)
{
	zynq_dev_t *zdev = zvideo->zdev;
	zynq_cam_caps_t *caps = &zvideo->caps;
	unsigned short data;
	unsigned int val;
	unsigned int orig;
	int reprobe = 0;
	int i;
	int ret;

	spin_lock(&zdev->zdev_lock);
	orig = zvideo_reg_read(zvideo, ZYNQ_CAM_CONFIG);
	caps->pin_swap = (orig & ZYNQ_CAM_PIN_SWAP) ? 1 : 0;

	zynq_trace(ZYNQ_TRACE_PROBE,
	    "%d vid%d %s: probe with cam_config=0x%x\n",
	    zdev->zdev_inst, zvideo->index, __FUNCTION__, orig);
probe:
	if ((ret = zcam_i2c_init(zvideo))) {
		goto end;
	}

	/* Probe the camera (Check the camera minor version) */
	for (i = 0; i < 10; i++) {
		ret = zcam_i2c_read16(zvideo, REG_MON_MINOR_VERSION, &data);
		if (!ret) {
			/*
			 * Check the camera code name.
			 * Pin-swap is always required for SharpVision cameras.
			 */
			if (((data >> 13) == CAM_CAP_CODENAME_SHARPVISION) &&
			    (!caps->pin_swap)) {
				ret = 0xFF;
			} else {
				caps->minor_version = data;
			}
			break;
		}
		udelay(ZCAM_WAIT_DELAY);
	}

	if (ret && !reprobe) {
		zcam_i2c_fini(zvideo);

		/* Reset I2C */
		val = zynq_g_reg_read(zdev, ZYNQ_G_RESET);
		val |= ZYNQ_I2C_RESET;
		zynq_g_reg_write(zdev, ZYNQ_G_RESET, val);
		val &= ~ZYNQ_I2C_RESET;
		zynq_g_reg_write(zdev, ZYNQ_G_RESET, val);
		spin_unlock(&zdev->zdev_lock);

		msleep(20);

		/* Set pin-swap */
		spin_lock(&zdev->zdev_lock);
		reprobe = 1;
		caps->pin_swap = !caps->pin_swap;

		val = zvideo_reg_read(zvideo, ZYNQ_CAM_CONFIG);
		if (caps->pin_swap) {
			val |= ZYNQ_CAM_PIN_SWAP;
		} else {
			val &= ~ZYNQ_CAM_PIN_SWAP;
		}
		zvideo_reg_write(zvideo, ZYNQ_CAM_CONFIG, val);
		spin_unlock(&zdev->zdev_lock);

		if (ret == 0xFF) {
			msleep(20);
		} else {
			msleep(500);
		}
		zynq_trace(ZYNQ_TRACE_PROBE,
		    "%d vid%d %s: re-probe with cam_config=0x%x\n",
		    zdev->zdev_inst, zvideo->index, __FUNCTION__, val);

		spin_lock(&zdev->zdev_lock);
		goto probe;
	}
end:
	if (ret) {
		zynq_err("%d vid%d %s: "
		    "WARNING! Failed to probe camera pin status\n",
		    zdev->zdev_inst, zvideo->index, __FUNCTION__);
		if (reprobe) {
			zvideo_reg_write(zvideo, ZYNQ_CAM_CONFIG, orig);
			caps->pin_swap = (orig & ZYNQ_CAM_PIN_SWAP) ? 1 : 0;

			zynq_trace(ZYNQ_TRACE_PROBE,
			    "%d vid%d %s: restore cam_config=0x%x\n",
			    zdev->zdev_inst, zvideo->index, __FUNCTION__, orig);
		}
	}

	zcam_i2c_fini(zvideo);
	spin_unlock(&zdev->zdev_lock);

	return ret;
}

int zcam_check_caps(zynq_video_t *zvideo)
{
	zynq_dev_t *zdev = zvideo->zdev;
	zynq_cam_caps_t *caps = &zvideo->caps;
	unsigned short addr;
	unsigned char data[4];
	unsigned char *p8 = data;
	unsigned short *p16 = (unsigned short *)data;
	unsigned int *p32 = (unsigned int *)data;
	int change = 0;
	int ret;

	snprintf(caps->name, ZYNQ_VDEV_NAME_LEN, "%s%d",
	    ZYNQ_FPD_CAM_NAME, zdev->zdev_video_port_map[zvideo->index]);

	caps->unique_id = 0;
	caps->major_version = -1;
	caps->minor_version = -1;
	caps->timestamp_type = zynq_video_ts_type;
	caps->interface_type = CAM_CAP_INTERFACE_PARALLEL;
	caps->trigger_mode = 0;
	caps->embedded_data = 0;
	caps->frame_len_lines = DEFAULT_FRAME_LINES_230;
	caps->line_len_pck = DEFAULT_LINE_PCK_230;
	caps->pixel_clock = DEFAULT_PIXCLK_230;

        if (zynq_video_pin_swap >= 0) {
		/* Force the pin-swap status */
                caps->pin_swap = (zynq_video_pin_swap > 0) ? 1 : 0;
		spin_lock(&zdev->zdev_lock);
		*p32 = zvideo_reg_read(zvideo, ZYNQ_CAM_CONFIG);
		if (caps->pin_swap) {
			if (!(*p32 & ZYNQ_CAM_PIN_SWAP)) {
				*p32 |= ZYNQ_CAM_PIN_SWAP;
				change = 1;
			}
		} else {
			if (*p32 & ZYNQ_CAM_PIN_SWAP) {
				*p32 &= ~ZYNQ_CAM_PIN_SWAP;
				change = 1;
			}
		}
		if (change) {
			zvideo_reg_write(zvideo, ZYNQ_CAM_CONFIG, *p32);
		}
		spin_unlock(&zdev->zdev_lock);
		if (change) {
			msleep(500);
		}
	} else {
		/* Probe camera to determine the pin-swap status */
		if (!(zvideo->state & ZVIDEO_STATE_STREAMING)) {
			if ((ret = zcam_probe(zvideo))) {
				return ret;
			}
		}
	}

	spin_lock(&zdev->zdev_lock);
	if ((ret = zcam_i2c_init(zvideo))) {
		goto end;
	}

	/* Check the camera unique id */
	addr = REG_UNIQUE_ID;
	*p16 = 0;
	if (!zcam_i2c_read16(zvideo, addr, p16)) {
		caps->unique_id = *p16;
	}

	/* Check the camera major version */
	addr = REG_MON_MAJOR_VERSION;
	*p16 = 0;
	if (!zcam_i2c_read16(zvideo, addr, p16)) {
		caps->major_version = *p16;
		snprintf(caps->name, ZYNQ_VDEV_NAME_LEN, "%s%d-%04x",
		    ZYNQ_FPD_CAM_NAME, zdev->zdev_video_port_map[zvideo->index],
		    caps->major_version);
	}

	if (caps->minor_version == (unsigned short)-1) {
		/* Check the camera minor version */
		addr = REG_MON_MINOR_VERSION;
		*p16 = 0;
		if (!zcam_i2c_read16(zvideo, addr, p16)) {
			caps->minor_version = *p16;
		}
	}

	/* Check the embedded metadata support */
	addr = REG_SNSR_CTRL_OPERATION_MODE;
	*p16 = 0;
	if (!zcam_i2c_read16(zvideo, addr, p16)) {
		if (*p16 & SNSR_CTRL_EMBD_DATA_ENABLE) {
			caps->embedded_data = 1;
		} else {
			caps->embedded_data = 0;
		}
	}

	/* Check the trigger mode */
	addr = REG_MODE_SYNC_TYPE;
	*p8 = 0;
	if (!zcam_i2c_read8(zvideo, addr, p8)) {
		caps->trigger_mode = *p8;
	}

	/* Check the frame length lines */
	addr = REG_SNSR_CFG_FRAME_LEN_LINES;
	*p16 = 0;
	if (!zcam_i2c_read16(zvideo, addr, p16)) {
		caps->frame_len_lines = *p16;
	} else if (caps->code_name == CAM_CAP_CODENAME_SHARPVISION) {
		caps->frame_len_lines = DEFAULT_FRAME_LINES_231;
	} else {
		caps->frame_len_lines = DEFAULT_FRAME_LINES_230;
	}

	/* Check the line length pck */
	addr = REG_SNSR_CFG_LINE_LEN_PCK;
	*p16 = 0;
	if (!zcam_i2c_read16(zvideo, addr, p16)) {
		caps->line_len_pck = *p16;
	} else if (caps->code_name == CAM_CAP_CODENAME_SHARPVISION) {
		caps->line_len_pck = DEFAULT_LINE_PCK_231;
	} else {
		caps->line_len_pck = DEFAULT_LINE_PCK_230;
	}

	/* Check the pixel clock */
	addr = REG_SNSR_CFG_PIXCLK;
	*p32 = 0;
	ret = zcam_i2c_read32(zvideo, addr, p32);
	if (!ret && *p32) {
		caps->pixel_clock = *p32;
	} else if (caps->code_name == CAM_CAP_CODENAME_SHARPVISION) {
		caps->pixel_clock = DEFAULT_PIXCLK_231;
	} else {
		caps->pixel_clock = DEFAULT_PIXCLK_230;
	}
end:
	zcam_i2c_fini(zvideo);
	spin_unlock(&zdev->zdev_lock);

	zcam_init_time_params(zvideo);

	zynq_trace(ZYNQ_TRACE_PROBE, "%d vid%d %s: "
	    "name=%s, unique_id=0x%04hx, major_version=0x%04hx, "
	    "minor_version=0x%04hx, timestamp_type=%u, "
	    "trigger_mode=%u, embedded_data=%u, "
	    "interface_type=%u, pin_swap=%u, "
	    "frame_length_lines=%hu, line_length_pck=%hu, pixclk=%u, "
	    "t_row=%u, t_frame=%u\n",
	    zdev->zdev_inst, zvideo->index, __FUNCTION__,
	    caps->name, caps->unique_id, caps->major_version,
	    caps->minor_version, caps->timestamp_type,
	    caps->trigger_mode, caps->embedded_data,
	    caps->interface_type, caps->pin_swap,
	    caps->frame_len_lines, caps->line_len_pck, caps->pixel_clock,
	    zvideo->t_row, zvideo->t_first);

	if (zynq_video_flash_16 > 0) {
		caps->feature |= CAM_CAP_FEATURE_FLASH_ADDR_16;
	} else if (zynq_video_flash_16 == 0) {
		caps->feature &= ~CAM_CAP_FEATURE_FLASH_ADDR_16;
	}

	if (caps->feature & CAM_CAP_FEATURE_FLASH_ADDR_16) {
		zvideo->dev_cfg = zcam_dev_cfgs[1];
	} else {
		zvideo->dev_cfg = zcam_dev_cfgs[0];
	}

	if (caps->code_name == CAM_CAP_CODENAME_SHARPVISION) {
		zvideo->read_em_data = zcam_read_embedded_data_231;
	} else {
		zvideo->read_em_data = zcam_read_embedded_data_230;
	}

	return ret;
}

static int zcam_command(zynq_video_t *zvideo, unsigned short cmd)
{
	zynq_dev_t *zdev = zvideo->zdev;
	unsigned short addr;
	unsigned short data;
	int i;
	int ret;

	zynq_trace(ZYNQ_TRACE_PROBE, "%d vid%d %s: command=0x%x\n",
	    zdev->zdev_inst, zvideo->index, __FUNCTION__, cmd);

	/* Check doorbell before issue the command */
	addr = REG_COMMAND;
	data = 0;
	if ((ret = zcam_i2c_read16(zvideo, addr, &data))) {
		return ret;
	}

	if (data & CMD_DOORBELL) {
		zynq_err("%d vid%d %s: ERROR doorbell not cleared 0x%x\n",
		    zdev->zdev_inst, zvideo->index, __FUNCTION__, data);
		return -EAGAIN;
	}

	/* Issue the command */
	addr = REG_COMMAND;
	data = cmd;
	if ((ret = zcam_i2c_write16(zvideo, addr, data))) {
		return ret;
	}

	/* Check and wait for the doorbell bit to be cleared */
	addr = REG_COMMAND;
	data = -1;
	i = 0;
	while ((i < ZCAM_WAIT_RETRY) && (data & CMD_DOORBELL)) {
		if ((ret = zcam_i2c_read16(zvideo, addr, &data))) {
			return ret;
		}
		udelay(ZCAM_WAIT_DELAY);
		i++;
	}

	if ((i >= ZCAM_WAIT_RETRY) && (data & CMD_DOORBELL)) {
		zynq_err("%d vid%d %s: TIMEOUT command=0x%x response=0x%x\n",
		    zdev->zdev_inst, zvideo->index, __FUNCTION__, cmd, data);
		return -EAGAIN;
	}

	if ((cmd == CMD_FLASH_STATUS) && (data == CMD_EBUSY)) {
		zynq_trace(ZYNQ_TRACE_VIDEO,
		    "%d vid%d %s: BUSY command=0x%x response=0x%x\n",
		    zdev->zdev_inst, zvideo->index, __FUNCTION__, cmd, data);
		return -EBUSY;
	}

	if (data) {
		zynq_err("%d vid%d %s: ERROR command=0x%x response=0x%x\n",
		    zdev->zdev_inst, zvideo->index, __FUNCTION__, cmd, data);
		return -EINVAL;
	}

	zynq_trace(ZYNQ_TRACE_VIDEO,
	    "%d vid%d %s: DONE command=0x%x response=0x%x\n",
	    zdev->zdev_inst, zvideo->index, __FUNCTION__, cmd, data);

	return 0;
}

static int zcam_params_read(zynq_video_t *zvideo, unsigned char start,
		unsigned char *data, unsigned char size)
{
	unsigned short addr;
	unsigned int i;
	int ret;

	if ((start + size) > REG_PARAM_POOL_SIZE) {
		return -EINVAL;
	}

	for (i = 0; i < size; i++) {
		addr = REG_PARAM_POOL_BASE + start + i;
		if ((ret = zcam_i2c_read8(zvideo, addr, &data[i]))) {
			return ret;
		}
	}

	return 0;
}

static int zcam_params_write(zynq_video_t *zvideo, unsigned char start,
		unsigned char *data, unsigned char size)
{
	unsigned short addr;
	unsigned int i;
	int ret;

	if ((start + size) > REG_PARAM_POOL_SIZE) {
		return -EINVAL;
	}

	for (i = 0; i < size; i++) {
		addr = REG_PARAM_POOL_BASE + start + i;
		if ((ret = zcam_i2c_write8(zvideo, addr, data[i]))) {
			return ret;
		}
	}

	return 0;
}

static int zcam_sys_set_state(zynq_video_t *zvideo, unsigned char state)
{
	int ret;

	if ((ret = zcam_params_write(zvideo, 0, &state, 1))) {
		return ret;
	}

	return zcam_command(zvideo, CMD_SYS_SET_STATE);
}

static int zcam_sys_get_state(zynq_video_t *zvideo, unsigned char *state)
{
	int ret;

	if ((ret = zcam_command(zvideo, CMD_SYS_GET_STATE))) {
		return ret;
	}

	return zcam_params_read(zvideo, 0, state, 1);
}

int zcam_set_suspend(zynq_video_t *zvideo)
{
	zynq_dev_t *zdev = zvideo->zdev;
	unsigned char state;
	int i;
	int ret;

	spin_lock(&zdev->zdev_lock);
	if ((ret = zcam_i2c_init(zvideo))) {
		goto end;
	}

	state = 0x40;
	if ((ret = zcam_sys_set_state(zvideo, state))) {
		goto end;
	}

	i = 0;
	while (i < ZCAM_WAIT_RETRY) {
		ret = zcam_sys_get_state(zvideo, &state);
		if (!ret) {
			break;
		}
		if (ret != -EAGAIN) {
			goto end;
		}
		udelay(ZCAM_WAIT_DELAY);
		i++;
	}

	if (i >= ZCAM_WAIT_RETRY) {
		zynq_err("%d vid%d %s: timeout getting camera state\n",
		    zdev->zdev_inst, zvideo->index, __FUNCTION__);
		ret = -EAGAIN;
		goto end;
	}

	zynq_trace(ZYNQ_TRACE_PROBE, "%d vid%d %s: camera state 0x%x\n",
	    zdev->zdev_inst, zvideo->index, __FUNCTION__, state);
end:
	zcam_i2c_fini(zvideo);
	spin_unlock(&zdev->zdev_lock);

	return ret;
}

int zcam_change_config(zynq_video_t *zvideo)
{
	zynq_dev_t *zdev = zvideo->zdev;
	unsigned char state;
	int i;
	int ret;

	spin_lock(&zdev->zdev_lock);
	if ((ret = zcam_i2c_init(zvideo))) {
		goto end;
	}

	state = SYS_STATE_ENTER_CONFIG_CHANGE;
	if ((ret = zcam_sys_set_state(zvideo, state))) {
		goto end;
	}

	i = 0;
	while (i < ZCAM_WAIT_RETRY) {
		ret = zcam_sys_get_state(zvideo, &state);
		if (!ret) {
			break;
		}
		if (ret != -EAGAIN) {
			goto end;
		}
		udelay(ZCAM_WAIT_DELAY);
		i++;
	}

	if (i >= ZCAM_WAIT_RETRY) {
		zynq_err("%d vid%d %s: timeout getting camera state\n",
		    zdev->zdev_inst, zvideo->index, __FUNCTION__);
		ret = -EAGAIN;
		goto end;
	}

	if (state != SYS_STATE_STREAMING) {
		zynq_err("%d vid%d %s: unexpected camera state 0x%x\n",
		    zdev->zdev_inst, zvideo->index, __FUNCTION__, state);
		ret = -EINVAL;
	}
end:
	zcam_i2c_fini(zvideo);
	spin_unlock(&zdev->zdev_lock);

	return ret;
}

int zcam_flash_get_lock(zynq_video_t *zvideo)
{
	zynq_dev_t *zdev = zvideo->zdev;
	int ret;

	zynq_trace(ZYNQ_TRACE_PROBE, "%d vid%d %s\n",
	    zdev->zdev_inst, zvideo->index, __FUNCTION__);

	spin_lock(&zdev->zdev_lock);
	if ((ret = zcam_i2c_init(zvideo))) {
		goto end;
	}

	ret = zcam_command(zvideo, CMD_FLASH_GET_LOCK);
end:
	zcam_i2c_fini(zvideo);
	spin_unlock(&zdev->zdev_lock);

	return ret;
}

int zcam_flash_lock_status(zynq_video_t *zvideo)
{
	zynq_dev_t *zdev = zvideo->zdev;
	int ret;

	zynq_trace(ZYNQ_TRACE_PROBE, "%d vid%d %s\n",
	    zdev->zdev_inst, zvideo->index, __FUNCTION__);

	spin_lock(&zdev->zdev_lock);
	if ((ret = zcam_i2c_init(zvideo))) {
		goto end;
	}

	ret = zcam_command(zvideo, CMD_FLASH_LOCK_STATUS);
end:
	zcam_i2c_fini(zvideo);
	spin_unlock(&zdev->zdev_lock);

	return ret;
}

int zcam_flash_release_lock(zynq_video_t *zvideo)
{
	zynq_dev_t *zdev = zvideo->zdev;
	int ret;

	zynq_trace(ZYNQ_TRACE_PROBE, "%d vid%d %s\n",
	    zdev->zdev_inst, zvideo->index, __FUNCTION__);

	spin_lock(&zdev->zdev_lock);
	if ((ret = zcam_i2c_init(zvideo))) {
		goto end;
	}

	ret = zcam_command(zvideo, CMD_FLASH_RELEASE_LOCK);
end:
	zcam_i2c_fini(zvideo);
	spin_unlock(&zdev->zdev_lock);

	return ret;
}

static int zcam_flash_status(zynq_video_t *zvideo)
{
	zynq_dev_t *zdev = zvideo->zdev;
	int i;
	int ret;

	zynq_trace(ZYNQ_TRACE_PROBE, "%d vid%d %s\n",
	    zdev->zdev_inst, zvideo->index, __FUNCTION__);

	/* Check the flash status */
	i = 0;
	while (i < ZCAM_WAIT_RETRY) {
		ret = zcam_command(zvideo, CMD_FLASH_STATUS);
		if (ret != -EBUSY) {
			return ret;
		}
		udelay(ZCAM_WAIT_DELAY);
		i++;
	}

	zynq_err("%d vid%d %s: TIMEOUT checking flash status\n",
	    zdev->zdev_inst, zvideo->index, __FUNCTION__);

	return -EBUSY;
}

int zcam_flash_query_device(zynq_video_t *zvideo)
{
	zynq_dev_t *zdev = zvideo->zdev;
	unsigned char params[8] = { 0 };
	int ret;

	zynq_trace(ZYNQ_TRACE_PROBE, "%d vid%d %s\n",
	    zdev->zdev_inst, zvideo->index, __FUNCTION__);

	spin_lock(&zdev->zdev_lock);
	if ((ret = zcam_i2c_init(zvideo))) {
		goto end;
	}

	/* Issue the query device command */
	if ((ret = zcam_command(zvideo, CMD_FLASH_QUERY_DEVICE))) {
		goto end;
	}

	/* Check the flash status (ignore the response) */
	(void) zcam_flash_status(zvideo);

	/* Read response */
	(void) zcam_params_read(zvideo, 0, params, 8);
end:
	zcam_i2c_fini(zvideo);
	spin_unlock(&zdev->zdev_lock);

	return ret;
}

int zcam_flash_config_device(zynq_video_t *zvideo)
{
	zynq_dev_t *zdev = zvideo->zdev;
	int ret;

	zynq_trace(ZYNQ_TRACE_PROBE, "%d vid%d %s\n",
	    zdev->zdev_inst, zvideo->index, __FUNCTION__);

	spin_lock(&zdev->zdev_lock);
	if ((ret = zcam_i2c_init(zvideo))) {
		goto end;
	}

	/* Write parameters */
	if ((ret = zcam_params_write(zvideo, 0, zvideo->dev_cfg, 8))) {
		goto end;
	}

	/* Issue the config device command */
	if ((ret = zcam_command(zvideo, CMD_FLASH_CONFIG_DEVICE))) {
		goto end;
	}
end:
	zcam_i2c_fini(zvideo);
	spin_unlock(&zdev->zdev_lock);

	return ret;
}

int zcam_flash_read(zynq_video_t *zvideo, unsigned int address,
		unsigned char *data, unsigned int size)
{
	zynq_dev_t *zdev = zvideo->zdev;
	unsigned char params[5];
	int len;
	int ret;

	zynq_trace(ZYNQ_TRACE_PROBE, "%d vid%d %s\n",
	    zdev->zdev_inst, zvideo->index, __FUNCTION__);

	spin_lock(&zdev->zdev_lock);
	if ((ret = zcam_i2c_init(zvideo))) {
		goto end;
	}

	while (size > 0) {
		len = (size > FLASH_READ_LIMIT) ? FLASH_READ_LIMIT : size;
		size -= len;

		/* Write address and length parameter */
		params[0] = (address >> 24) & 0xFF;
		params[1] = (address >> 16) & 0xFF;
		params[2] = (address >> 8) & 0xFF;
		params[3] = address & 0xFF;
		params[4] = len;
		if ((ret = zcam_params_write(zvideo, 0, params, 5))) {
			goto end;
		}

		/* Issue the read command */
		if ((ret = zcam_command(zvideo, CMD_FLASH_READ))) {
			goto end;
		}

		/* Check the flash status */
		if ((ret = zcam_flash_status(zvideo))) {
			goto end;
		}

		/* Read data */
		if ((ret = zcam_params_read(zvideo, 0, data, len))) {
			goto end;
		}

		address += len;
		data += len;
	}

end:
	zcam_i2c_fini(zvideo);
	spin_unlock(&zdev->zdev_lock);

	return ret;
}

int zcam_flash_write(zynq_video_t *zvideo, unsigned int address,
		unsigned char *data, unsigned int size)
{
	zynq_dev_t *zdev = zvideo->zdev;
	unsigned char params[5];
	int off;
	int len;
	int ret;

	zynq_trace(ZYNQ_TRACE_PROBE, "%d vid%d %s\n",
	    zdev->zdev_inst, zvideo->index, __FUNCTION__);

	spin_lock(&zdev->zdev_lock);
	if ((ret = zcam_i2c_init(zvideo))) {
		goto end;
	}

	while (size > 0) {
		len = (size > FLASH_WRITE_LIMIT) ? FLASH_WRITE_LIMIT : size;

		/* The write should not span a page boundary */
		off = address % FLASH_PAGE_SIZE;
		if ((off + len) > FLASH_PAGE_SIZE) {
			len = FLASH_PAGE_SIZE - off;
		}
		size -= len;

		/* Write address and length parameter */
		params[0] = (address >> 24) & 0xFF;
		params[1] = (address >> 16) & 0xFF;
		params[2] = (address >> 8) & 0xFF;
		params[3] = address & 0xFF;
		params[4] = len;
		if ((ret = zcam_params_write(zvideo, 0, params, 5))) {
			goto end;
		}

		/* Write data */
		if ((ret = zcam_params_write(zvideo, 5, data, len))) {
			goto end;
		}

		/* Issue the write command */
		if ((ret = zcam_command(zvideo, CMD_FLASH_WRITE))) {
			goto end;
		}

		/* Check the flash status */
		if ((ret = zcam_flash_status(zvideo))) {
			goto end;
		}

		address += len;
		data += len;
	}
end:
	zcam_i2c_fini(zvideo);
	spin_unlock(&zdev->zdev_lock);

	return ret;
}
