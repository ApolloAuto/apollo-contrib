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

#ifndef _BASA_CAM_HCI_H_
#define	_BASA_CAM_HCI_H_

/* 8bit registers */
#define	REG_MODE_SYNC_TYPE		0xC891

/* 16bit registers */
#define	REG_MON_MAJOR_VERSION		0x8000
#define	REG_MON_MINOR_VERSION		0x8002
#define	REG_MON_RELEASE_VERSION		0x8004
#define	REG_UNIQUE_ID			0x802C
#define	REG_SNSR_CFG_FRAME_LEN_LINES	0xC814
#define	REG_SNSR_CFG_LINE_LEN_PCK	0xC816

#define	REG_SNSR_CTRL_OPERATION_MODE	0xC844
#define	SNSR_CTRL_EMBD_DATA_ENABLE	0x1000

/* 32bit registers */
#define	REG_SNSR_CFG_PIXCLK		0xC80C

/* Host Command Interface registers */
#define	REG_COMMAND			0x0040
#define	CMD_DOORBELL			0x8000
#define	CMD_COMMAND_MASK		0x7FFF

#define	REG_PARAM_POOL_SIZE		256	/* Bytes */
#define	REG_PARAM_POOL_BASE		0xFC00

#define	CMD_ENOERR			0x00
#define	CMD_ENOENT			0x01
#define	CMD_EINTR			0x02
#define	CMD_EIO				0x03
#define	CMD_E2BIG			0x04
#define	CMD_EBADF			0x05
#define	CMD_EAGAIN			0x06
#define	CMD_ENOMEM			0x07
#define	CMD_EACCES			0x08
#define	CMD_EBUSY			0x09
#define	CMD_EEXIST			0x0A
#define	CMD_ENODEV			0x0B
#define	CMD_EINVAL			0x0C
#define	CMD_ENOSPC			0x0D
#define	CMD_ERANGE			0x0E
#define	CMD_ENOSYS			0x0F
#define	CMD_EALREADY			0x10

#define	CMD_SYS_SET_STATE		0x8100
#define	CMD_SYS_GET_STATE		0x8101

#define	CMD_FLASH_GET_LOCK		0x8500
#define	CMD_FLASH_LOCK_STATUS		0x8501
#define	CMD_FLASH_RELEASE_LOCK		0x8502
#define	CMD_FLASH_CONFIG		0x8503
#define	CMD_FLASH_READ			0x8504
#define	CMD_FLASH_WRITE			0x8505
#define	CMD_FLASH_ERASE_BLOCK		0x8506
#define	CMD_FLASH_ERASE_DEVICE		0x8507
#define	CMD_FLASH_QUERY_DEVICE		0x8508
#define	CMD_FLASH_STATUS		0x8509
#define	CMD_FLASH_CONFIG_DEVICE		0x850A
#define	CMD_FLASH_VALIDATE		0x850D
#define	CMD_FLASH_VALIDATE_STATUS	0x850E
#define	CMD_FLASH_DEV_CMD		0x850F
#define	CMD_FLASH_DEV_CMD_RESPONSE	0x8510

/* System Manager Request State */
#define	SYS_STATE_ENTER_CONFIG_CHANGE	0x28

/* System Manager Permanent State */
#define	SYS_STATE_IDLE			0x20
#define	SYS_STATE_STREAMING		0x31
#define	SYS_STATE_SUSPENDED		0x41
#define	SYS_STATE_SOFT_STANDBY		0x53
#define	SYS_STATE_HARD_STANDBY		0x5B

#define	FLASH_PAGE_SIZE			32
#define	FLASH_READ_LIMIT		REG_PARAM_POOL_SIZE
#define	FLASH_WRITE_LIMIT		(REG_PARAM_POOL_SIZE - 5)

extern int zcam_reg_read(zynq_video_t *, zynq_cam_acc_t *);
extern int zcam_reg_write(zynq_video_t *, zynq_cam_acc_t *);
extern int zcam_check_caps(zynq_video_t *);
extern int zcam_change_config(zynq_video_t *);
extern int zcam_set_suspend(zynq_video_t *);

extern int zcam_flash_get_lock(zynq_video_t *);
extern int zcam_flash_release_lock(zynq_video_t *);
extern int zcam_flash_lock_status(zynq_video_t *);
extern int zcam_flash_query_device(zynq_video_t *);
extern int zcam_flash_config_device(zynq_video_t *);
extern int zcam_flash_read(zynq_video_t *, unsigned int,
		unsigned char *, unsigned int);
extern int zcam_flash_write(zynq_video_t *, unsigned int,
		unsigned char *, unsigned int);

#endif /* _BASA_CAM_HCI_H_ */
