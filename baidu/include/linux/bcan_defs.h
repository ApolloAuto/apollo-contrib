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

#ifndef BCAN_DEFS_H
#define BCAN_DEFS_H

#ifndef __KERNEL__
#include <sys/time.h>
#else
#include <linux/time.h>
#endif

#define	BCAN_EXTENDED_FRAME	0x20000000

/*
 * Baidu CAN message definition
 */
typedef struct bcan_msg {
	unsigned int	bcan_msg_id; /* source CAN node id */
	unsigned char	bcan_msg_datalen; /* message data len */
	unsigned char	bcan_msg_rsv[3];
	unsigned char	bcan_msg_data[8]; /* message data */
	struct timeval	bcan_msg_timestamp;
} bcan_msg_t;


/*
 * CAN error code
 */
enum bcan_err_code {
	BCAN_PARAM_INVALID = -12,
	BCAN_HDL_INVALID,
	BCAN_DEV_INVALID,
	BCAN_DEV_ERR,
	BCAN_DEV_BUSY,
	BCAN_TIMEOUT,
	BCAN_FAIL,
	BCAN_NOT_SUPPORTED,
	BCAN_NOT_IMPLEMENTED,
	BCAN_INVALID,
	BCAN_NO_BUFFERS,
	BCAN_ERR,
	BCAN_OK, /* 0 */
	BCAN_PARTIAL_OK
};

#endif
