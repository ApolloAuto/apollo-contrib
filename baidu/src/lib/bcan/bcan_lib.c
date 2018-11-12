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

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <syslog.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

#include "linux/zynq_api.h"
#include "bcan.h"

#ifdef DEBUG
#define	BLOG_DBG(s...)	syslog(LOG_DEBUG, s)
#else
#define	BLOG_DBG(s...)	do {} while (0)
#endif
#define	BLOG_ERR(s...)	syslog(LOG_ERR, s)

typedef struct bcan_ihdl {
	int		dev_index;
	int		dev_state;
	int		fd;
	uint32_t	baudrate;
	uint32_t	tx_to;
	uint32_t	rx_to;
} bcan_ihdl_t;

const char *bcan_get_libversion(void)
{
	return LIB_VER;
}

const char *bcan_get_err_msg(int err_code)
{
	switch (err_code) {
	case BCAN_PARAM_INVALID:
		return "invalid parameter";
	case BCAN_HDL_INVALID:
		return "invalid bcan handle";
	case BCAN_DEV_INVALID:
		return "invalid bcan device";
	case BCAN_DEV_ERR:
		return "bcan device error";
	case BCAN_DEV_BUSY:
		return "bcan device busy";
	case BCAN_TIMEOUT:
		return "bcan timeout";
	case BCAN_FAIL:
		return "bcan operation failed";
	case BCAN_NOT_SUPPORTED:
		return "bcan operation not supported";
	case BCAN_NOT_IMPLEMENTED:
		return "bcan operation not implemented";
	case BCAN_INVALID:
		return "bcan invalid error";
	case BCAN_NO_BUFFERS:
		return "failed to allocate bcan buffers";
	case BCAN_ERR:
		return "generic bcan error, check error log";
	case BCAN_OK:
		return "OK"; /* 0 */
	case BCAN_PARTIAL_OK:
		return "bcan IO partial completion";
	default:
		return "undefined error code";
	}
}

/*
 * Check if transition from current state to proposed state is valid.
 * Also sanity check for handle.
 */
static int bcan_check_state(bcan_ihdl_t *ahdl, int pstate)
{
	bcan_ihdl_t *hdl = (bcan_ihdl_t *)ahdl;
	int cstate, err = BCAN_OK;

	if (!hdl) {
		return (BCAN_HDL_INVALID);
	}

	cstate = hdl->dev_state;
	switch (pstate) {
	case BCAN_DEV_OPEN:
		if (cstate != BCAN_DEV_UNINIT) {
			err = BCAN_DEV_INVALID;
		}
		break;

	case BCAN_DEV_BAUD_SET:
	case BCAN_DEV_LOOPBACK:
		if (!(cstate & BCAN_DEV_OPEN)) {
			err = BCAN_DEV_INVALID;
		}
		break;

	case BCAN_DEV_START:
		if (!(cstate & BCAN_DEV_BAUD_SET)) {
			err = BCAN_DEV_INVALID;
		}
		break;

	case BCAN_DEV_ACTIVE:
	case BCAN_DEV_STOP:
		if (!(cstate & BCAN_DEV_START)) {
			err = BCAN_DEV_INVALID;
		}
		break;

	case BCAN_DEV_CLOSE:
		if (!(cstate & BCAN_DEV_OPEN)) {
			err = BCAN_DEV_INVALID;
		}
		break;
	default:
		break;
	}

	BLOG_DBG("%s: dev_index %d, cstate %d, pstate %d, err %d\n", __func__,
	    hdl->dev_index, cstate, pstate, err);
	return (err);
}

int bcan_open(uint32_t dev_index, uint32_t UNUSED(flags),
		uint64_t tx_to, uint64_t rx_to, bcan_hdl_t *ahdl)
{
	char dev_path[256];
	int fd, ret;
	bcan_ihdl_t *hdl;

	snprintf(dev_path, sizeof(dev_path), "/dev/%s%u", ZYNQ_DEV_NAME_CAN,
	    dev_index);

	if ((fd = open(dev_path, O_RDWR)) == -1)  {
		BLOG_ERR("%s open() failure: %s\n", dev_path, strerror(errno));
		ret = BCAN_PARAM_INVALID;
		return ret;
	}
	BLOG_DBG("%s open() success. fd %d\n", dev_path, fd);

	hdl = (bcan_ihdl_t *)malloc(sizeof(bcan_ihdl_t));
	if (!hdl) {
		BLOG_ERR("malloc() failure: %s\n", strerror(errno));
		ret = BCAN_NO_BUFFERS;
		goto err;
	}

	if (ioctl(fd, ZYNQ_IOC_CAN_TX_TIMEOUT_SET, tx_to) == -1) {
		BLOG_ERR("%s: dev_index %d, IOC_BCAN_TX_TIMEOUT %ld failed\n",
		    __func__, dev_index, tx_to);
		ret = BCAN_PARAM_INVALID;
		goto err;
	}

	if (ioctl(fd, ZYNQ_IOC_CAN_RX_TIMEOUT_SET, rx_to) == -1) {
		BLOG_ERR("%s: dev_index %d, IOC_BCAN_RX_TIMEOUT %ld failed\n",
		    __func__, dev_index, rx_to);
		ret = BCAN_PARAM_INVALID;
		goto err;
	}

	hdl->dev_state = BCAN_DEV_OPEN;
	hdl->dev_index = dev_index;
	hdl->fd = fd;
	*ahdl = (uintptr_t)(const void *)hdl;

	return BCAN_OK;
err:
	if (hdl) {
		free(hdl);
	}
	close(fd);

	return ret;
}

int bcan_close(bcan_hdl_t ahdl)
{
	int ret;
	bcan_ihdl_t *hdl = (bcan_ihdl_t *)ahdl;

	if ((ret = bcan_check_state(hdl, BCAN_DEV_CLOSE)) != BCAN_OK) {
		goto err;
	}

	BLOG_DBG("%s: dev_index %d, dev_state %d, fd %d\n", __func__,
	    hdl->dev_index, hdl->dev_state, hdl->fd);

	close(hdl->fd);
	hdl->dev_state = BCAN_DEV_CLOSE;

	free(hdl);
err:
	return ret;
}

int bcan_set_baudrate(bcan_hdl_t ahdl, uint32_t rate)
{
	int ret = BCAN_OK;
	bcan_ihdl_t *hdl = (bcan_ihdl_t *)ahdl;

	if ((ret = bcan_check_state(hdl, BCAN_DEV_BAUD_SET)) != BCAN_OK) {
		goto err;
	}

	if (ioctl(hdl->fd, ZYNQ_IOC_CAN_BAUDRATE_SET, rate) == -1) {
		BLOG_ERR("%s: dev_index %d, IOC_BCAN_SET_BAUDRATE failed\n",
		    __func__, hdl->dev_index);
		ret = BCAN_ERR;
		goto err;
	}

	hdl->dev_state |= BCAN_DEV_BAUD_SET;
	hdl->baudrate = rate;

err:
	return ret;
}

int bcan_get_baudrate(bcan_hdl_t ahdl, uint32_t *rate)
{
	int ret = BCAN_OK;
	bcan_ihdl_t *hdl = (bcan_ihdl_t *)ahdl;

	if (ioctl(hdl->fd, ZYNQ_IOC_CAN_BAUDRATE_GET, rate) == -1) {
		BLOG_ERR("%s: dev_index %d, IOC_BCAN_GET_BAUDRATE failed\n",
		    __func__, hdl->dev_index);
		ret = BCAN_ERR;
		goto err1;
	}

	if (hdl->baudrate != *rate) {
		BLOG_ERR("%s: dev_index %d, baudrate %d and cached rate %d "
		    "disagree\n", __func__, hdl->dev_index, *rate, hdl->baudrate);
	}

err1:
	return ret;
}

int bcan_set_loopback(bcan_hdl_t ahdl)
{
	int ret = BCAN_OK;
	bcan_ihdl_t *hdl = (bcan_ihdl_t *)ahdl;

	if ((ret = bcan_check_state(hdl, BCAN_DEV_LOOPBACK)) != BCAN_OK) {
		goto err;
	}

	if (ioctl(hdl->fd, ZYNQ_IOC_CAN_LOOPBACK_SET, NULL) == -1) {
		BLOG_ERR("%s: dev_index %d, IOC_BCAN_SET_LOOPBACK failed\n",
		    __func__, hdl->dev_index);
		ret = BCAN_ERR;
		goto err;
	}
	hdl->dev_state &= ~BCAN_DEV_NORMAL;
	hdl->dev_state |= BCAN_DEV_LOOPBACK;

err:
	return ret;
}

int bcan_unset_loopback(bcan_hdl_t ahdl)
{
	int ret = BCAN_OK;

	bcan_ihdl_t *hdl = (bcan_ihdl_t *)ahdl;

	if ((ret = bcan_check_state(hdl, -1)) != BCAN_OK) {
		goto err;
	}

	if (ioctl(hdl->fd, ZYNQ_IOC_CAN_LOOPBACK_UNSET, NULL) == -1) {
		BLOG_ERR("%s: dev_index %d, IOC_BCAN_UNSET_LOOPBACK failed\n",
		    __func__, hdl->dev_index);
		ret = BCAN_ERR;
		goto err;
	}
	hdl->dev_state &= ~BCAN_DEV_LOOPBACK;
	hdl->dev_state |= BCAN_DEV_NORMAL;

err:
	return ret;
}

int bcan_recv(bcan_hdl_t ahdl, bcan_msg_t *buf, uint32_t num_msg)
{
	int i, j, ret = BCAN_OK;
	bcan_msg_t *lbuf;
	bcan_ihdl_t *hdl = (bcan_ihdl_t *)ahdl;
	ioc_bcan_msg_t ioc_hdl;
	ioc_hdl.ioc_msgs = buf;
	ioc_hdl.ioc_msg_num = num_msg;

	if ((num_msg == 0) || (num_msg > BCAN_MAX_RX_MSG)) {
		ret = BCAN_PARAM_INVALID;
		goto err;
	}

	if ((ret = bcan_check_state(hdl, BCAN_DEV_ACTIVE)) != BCAN_OK) {
		goto err;
	}

	/*
 	 * If this is the first recv after handle open then set rx_clear so
 	 * stale data can be flushed by the driver
 	 */
	if (!(hdl->dev_state & BCAN_DEV_RECVD)) {
		hdl->dev_state |= BCAN_DEV_RECVD;
		ioc_hdl.ioc_msg_rx_clear = 1;
	} else {
		ioc_hdl.ioc_msg_rx_clear = 0;
	}

	if (ioctl(hdl->fd, ZYNQ_IOC_CAN_RECV, &ioc_hdl) == -1) {
		BLOG_ERR("%s: dev_index %d, IOC_BCAN_RECV failed. msg_err %d\n",
		    __func__, hdl->dev_index, ioc_hdl.ioc_msg_err);
		ret = BCAN_ERR;
		goto err;
	} else {
		BLOG_DBG("%s: dev_index %d, IOC_BCAN_RECV success. msg_num_done %d. "
		    "msg_err %d addr %p\n", __func__, hdl->dev_index,
		    ioc_hdl.ioc_msg_num_done, ioc_hdl.ioc_msg_err, buf);

		for (i = 0, lbuf = buf; i < (int)ioc_hdl.ioc_msg_num_done; i++, lbuf++) {
			BLOG_DBG("bcan_recv: start_of_msg %d\n", i + 1);
			BLOG_DBG("bcan_recv: msg_id 0x%x, datalen %d bytes, data:\n",
			    lbuf->bcan_msg_id, lbuf->bcan_msg_datalen);
			for (j = 0; j < (buf + i)->bcan_msg_datalen; j++) {
				BLOG_DBG("0x%x\n", lbuf->bcan_msg_data[j]);
			}

			BLOG_DBG("bcan_recv: end_of_msg %d\n", i + 1);
		}

	}

	if (ioc_hdl.ioc_msg_num_done > 0) {
		return ioc_hdl.ioc_msg_num_done;
	} else {
		return ioc_hdl.ioc_msg_err;
	}
err:
	return ret;
}

int bcan_send(bcan_hdl_t ahdl, bcan_msg_t *buf, uint32_t num_msg)
{
	int i, j, ret = BCAN_OK;
	bcan_ihdl_t *hdl = (bcan_ihdl_t *)ahdl;
	ioc_bcan_msg_t ioc_hdl;
	ioc_hdl.ioc_msgs = buf;
	ioc_hdl.ioc_msg_num = num_msg;

	if ((num_msg == 0) || (num_msg > BCAN_MAX_TX_MSG)) {
		ret = BCAN_PARAM_INVALID;
		goto err;
	}

	if ((ret = bcan_check_state(hdl, BCAN_DEV_ACTIVE)) != BCAN_OK) {
		goto err;
	}

	for (i = 0; i < (int)ioc_hdl.ioc_msg_num; i++) {
		BLOG_DBG("bcan_send: start_of_msg %d\n", i + 1);
		BLOG_DBG("bcan_send: msg_id 0x%x, datalen %d bytes, data:\n",
		    (buf + i)->bcan_msg_id, (buf + i)->bcan_msg_datalen);

		for (j = 0; j < (buf + i)->bcan_msg_datalen; j++) {
			BLOG_DBG("0x%x\n", (buf + i)->bcan_msg_data[j]);
		}

		BLOG_DBG("bcan_send: end_of_msg %d\n", i + 1);
	}

	if (ioctl(hdl->fd, ZYNQ_IOC_CAN_SEND, &ioc_hdl) == -1) {
		BLOG_ERR("%s: dev_index %d, IOC_BCAN_SEND failed. msg_err %d\n",
		    __func__, hdl->dev_index, ioc_hdl.ioc_msg_err);
		ret = BCAN_ERR;
		goto err;
	} else {
		BLOG_DBG("%s: dev_index %d, IOC_BCAN_SEND success. msg_num_done %d\n",
		    __func__, hdl->dev_index, ioc_hdl.ioc_msg_num_done);
	}

	if (ioc_hdl.ioc_msg_num_done > 0) {
		return ioc_hdl.ioc_msg_num_done;
	} else {
		return ioc_hdl.ioc_msg_err;
	}
err:
	return ret;
}

int bcan_send_hi_pri(bcan_hdl_t ahdl, bcan_msg_t *buf)
{
	int j, ret = BCAN_OK;
	bcan_ihdl_t *hdl = (bcan_ihdl_t *)ahdl;
	ioc_bcan_msg_t ioc_hdl;

	if ((ret = bcan_check_state(hdl, BCAN_DEV_ACTIVE)) != BCAN_OK) {
		goto err;
	}

	ioc_hdl.ioc_msgs = buf;
	ioc_hdl.ioc_msg_num = 1;

	BLOG_DBG("%s: start_of_msg\n", __func__);
	BLOG_DBG("%s: msg_id 0x%x, datalen %d bytes, data:\n", __func__,
	    buf->bcan_msg_id, buf->bcan_msg_datalen);
	for (j = 0; j < buf->bcan_msg_datalen; j++) {
		BLOG_DBG("0x%x\n", buf->bcan_msg_data[j]);
	}

	BLOG_DBG("%s: end_of_msg\n", __func__);

	if (ioctl(hdl->fd, ZYNQ_IOC_CAN_SEND_HIPRI, &ioc_hdl) == -1) {
		BLOG_ERR("%s: dev_index %d, IOC_BCAN_SEND_HIPRI failed. msg_err %d\n",
		    __func__, hdl->dev_index, ioc_hdl.ioc_msg_err);
		ret = BCAN_ERR;
		goto err;
	} else {
		BLOG_DBG("%s: dev_index %d, IOC_BCAN_SEND_HIPRI success. "
		    "msg_num_done %d\n",
		    __func__, hdl->dev_index, ioc_hdl.ioc_msg_num_done);
	}

	if (ioc_hdl.ioc_msg_num_done == 1)
		return (1);
	else
		return (ioc_hdl.ioc_msg_err);
err:
	return ret;
}

int bcan_start(bcan_hdl_t ahdl)
{
	bcan_ihdl_t *hdl = (bcan_ihdl_t *)ahdl;
	int ret = BCAN_OK;

	if ((ret = bcan_check_state(hdl, BCAN_DEV_START)) != BCAN_OK) {
		goto err;
	}

	if (ioctl(hdl->fd, ZYNQ_IOC_CAN_DEV_START, NULL) == -1) {
		BLOG_ERR("%s: dev_index %d, IOC_BCAN_START failed\n", __func__,
		    hdl->dev_index);
		ret = BCAN_ERR;
		goto err;
	}

	hdl->dev_state |= BCAN_DEV_START;
err:
	return ret;
}

int bcan_stop(bcan_hdl_t ahdl)
{
	int ret = BCAN_OK;
	bcan_ihdl_t *hdl = (bcan_ihdl_t *)ahdl;

	if ((ret = bcan_check_state(hdl, BCAN_DEV_STOP)) != BCAN_OK) {
		goto err;
	}

	if (ioctl(hdl->fd, ZYNQ_IOC_CAN_DEV_STOP, NULL) == -1) {
		BLOG_ERR("%s: dev_index %d, IOC_BCAN_STOP failed\n", __func__,
		    hdl->dev_index);
		goto err;
	}
	hdl->dev_state |= BCAN_DEV_STOP;
	hdl->dev_state &= ~BCAN_DEV_START;

err:
	return ret;
}

int bcan_get_status(bcan_hdl_t ahdl)
{
	ioc_bcan_status_err_t st_err;
	bcan_ihdl_t *hdl = (bcan_ihdl_t *)ahdl;
	int ret = BCAN_OK;

	if ((ret = bcan_check_state(hdl, -1)) != BCAN_OK) {
		goto err;
	}

	if (ioctl(hdl->fd, ZYNQ_IOC_CAN_GET_STATUS_ERR, &st_err) == -1) {
		BLOG_ERR("%s: dev_index %d, IOC_BCAN_GET_STATUS_ERR failed\n",
		    __func__, hdl->dev_index);
		ret = BCAN_ERR;
		goto err;
	}
	return ((int)st_err.bcan_status);
err:
	return ret;
}

int bcan_get_err_counter(bcan_hdl_t ahdl, uint8_t *rx_err, uint8_t *tx_err)
{
	ioc_bcan_status_err_t st_err;
	bcan_ihdl_t *hdl = (bcan_ihdl_t *)ahdl;
	int ret = BCAN_OK;

	if ((ret = bcan_check_state(hdl, -1)) != BCAN_OK)
		goto err;

	if (ioctl(hdl->fd, ZYNQ_IOC_CAN_GET_STATUS_ERR, &st_err) == -1) {
		BLOG_ERR("%s: dev_index %d, IOC_BCAN_GET_STATUS_ERR failed\n",
		    __func__, hdl->dev_index);
		ret = BCAN_ERR;
		goto err;
	}
	*tx_err = (st_err.bcan_err_count & 0xFF);
	*rx_err = (st_err.bcan_err_count & 0xFFFF) >> 8;

err:
	return ret;
}
