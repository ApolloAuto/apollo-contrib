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

#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/string.h>

#include "basa.h"

/*
 * sysfs related to global driver level parameter is under
 *	/sys/module/basa/parameters/
 */

/*
 * show the number of CAN IP(s) the device has:
 *	cat num_can_ip
 */
static ssize_t zynq_sysfs_num_can_ip_show(zynq_dev_t *zdev, char *buf)
{
	int len;

	len = sprintf(buf, "%u\n", zdev->zdev_can_cnt);

	return len;
}

/*
 * show the number of channels the device has:
 *	cat num_chan
 */
static ssize_t zynq_sysfs_num_chan_show(zynq_dev_t *zdev, char *buf)
{
	int len;

	len = sprintf(buf, "%u\n", zdev->zdev_chan_cnt);

	return len;
}

static u32 zynq_sysfs_can_ip = 0;
/*
 * show the CAN IP instance that we are going to operate:
 *	cat can_ip
 */
static ssize_t zynq_sysfs_can_ip_read(zynq_dev_t *zdev, char *buf)
{
	int len;

	len = sprintf(buf, "CAN IP number is %u\n", zynq_sysfs_can_ip);

	return len;
}

/*
 * set the CAN IP instance that we are going to operate:
 *	echo <can> > can_ip
 */
static ssize_t zynq_sysfs_can_ip_set(zynq_dev_t *zdev,
		const char *buf, size_t sz)
{
	u32 can_ip;

	sscanf(buf, "%u", &can_ip);
	if (can_ip >= zdev->zdev_can_cnt || !can_ip_to_zcan(zdev,
	    can_ip)) {
		zynq_err("bad CAN IP number %u, maximum is %u\n",
		    can_ip, zdev->zdev_can_cnt - 1);
	} else {
		zynq_err("CAN IP number changed %u -> %u\n",
		    zynq_sysfs_can_ip, can_ip);
		zynq_sysfs_can_ip = can_ip;
	}

	return sz;
}

static u32 zynq_sysfs_chan = 0;
/*
 * show the channel number that we are going to operate:
 *	cat chan
 */
static ssize_t zynq_sysfs_chan_read(zynq_dev_t *zdev, char *buf)
{
	int len;

	len = sprintf(buf, "channel number is %u\n", zynq_sysfs_chan);

	return len;
}

/*
 * set the channel number that we are going to operate:
 *	echo <chan> > chan
 */
static ssize_t zynq_sysfs_chan_set(zynq_dev_t *zdev, const char *buf, size_t sz)
{
	u32 chan;

	sscanf(buf, "%u", &chan);
	if (chan >= zdev->zdev_chan_cnt) {
		zynq_err("bad channel number %u, maximum is %u\n",
		    chan, zdev->zdev_chan_cnt - 1);
	} else {
		zynq_err("channel number changed %u -> %u\n",
		    zynq_sysfs_chan, chan);
		zynq_sysfs_chan = chan;
	}

	return sz;
}

static u32 zynq_sysfs_g_reg_offset = 0;

/* cat g_reg_value */
static ssize_t zynq_sysfs_g_reg_read(zynq_dev_t *zdev, char *buf)
{
	int len;

	len = sprintf(buf, "device register offset 0x%x = 0x%x\n",
	    zynq_sysfs_g_reg_offset,
	    ZDEV_G_REG32_RD(zdev, zynq_sysfs_g_reg_offset));

	return len;
}

/*
 * echo <offset> <val> > g_reg_value
 * echo <offset> > g_reg_value
 */
static ssize_t zynq_sysfs_g_reg_set(zynq_dev_t *zdev,
		const char *buf, size_t sz)
{
	u32 offset, val;

	if (strchr(buf, ' ')) {
		sscanf(buf, "%x %x", &offset, &val);
		if (offset > (zdev->zdev_bar0_len - 4)) {
			zynq_err("bad register offset 0x%x, maximum is 0x%x\n",
			    offset, zdev->zdev_bar0_len - 4);
		} else {
			zynq_err("device register offset 0x%x set to 0x%x\n",
			    offset, val);
			ZDEV_G_REG32(zdev, offset) = val;
			zynq_sysfs_g_reg_offset = offset;
		}
	} else {
		sscanf(buf, "%x", &offset);
		if (offset > (zdev->zdev_bar0_len - 4)) {
			zynq_err("bad register offset 0x%x, maximum is 0x%x\n",
			    offset, zdev->zdev_bar0_len - 4);
		} else {
			zynq_err("device register offset changed 0x%x -> 0x%x\n",
			    zynq_sysfs_g_reg_offset, offset);
			zynq_sysfs_g_reg_offset = offset;
		}
	}
	return sz;
}

static u8 zynq_sysfs_i2c_id = 0;
static u8 zynq_sysfs_i2c_addr = 0;
static u8 zynq_sysfs_i2c_addr_hi = 0;
static u8 zynq_sysfs_i2c_bus = 0;

/*
 * show the i2c bus number that we are going to operate:
 *	cat i2c_bus
 */
static ssize_t zynq_sysfs_i2c_bus_read(zynq_dev_t *zdev, char *buf)
{
	int len;

	len = sprintf(buf, "I2C bus number is %u\n", zynq_sysfs_i2c_bus);

	return len;
}

/*
 * set the i2c bus number that we are going to operate:
 *	echo <i2c_bus> > i2c_bus
 */
static ssize_t zynq_sysfs_i2c_bus_set(zynq_dev_t *zdev,
		const char *buf, size_t sz)
{
	u8 i2c_bus;

	sscanf(buf, "%hhu", &i2c_bus);
	if (i2c_bus >= ZYNQ_I2C_BUS_NUM) {
		zynq_err("bad I2C bus number %u, maximum is %u\n",
		    i2c_bus, ZYNQ_I2C_BUS_NUM - 1);
	} else {
		zynq_err("I2C bus number changed %u -> %u\n",
		    zynq_sysfs_i2c_bus, i2c_bus);
		zynq_sysfs_i2c_bus = i2c_bus;
	}

	return sz;
}

static ssize_t zynq_sysfs_i2c_read(zynq_dev_t *zdev,
		char *buf, unsigned char i2c_addr_16)
{
	ioc_zynq_i2c_acc_t i2c_acc;
	u32 i2c_addr;
	int err;
	int len;

	i2c_acc.i2c_id = zynq_sysfs_i2c_id;
	i2c_acc.i2c_addr_hi = zynq_sysfs_i2c_addr_hi;
	i2c_acc.i2c_addr = zynq_sysfs_i2c_addr;
	i2c_acc.i2c_data = 0;
	i2c_acc.i2c_addr_16 = i2c_addr_16;
	i2c_acc.i2c_bus = zynq_sysfs_i2c_bus;

	err = zdev_i2c_read(zdev, &i2c_acc);

	i2c_addr = (i2c_addr_16) ?
	    (zynq_sysfs_i2c_addr_hi << 8) | zynq_sysfs_i2c_addr :
	    zynq_sysfs_i2c_addr;

	if (err) {
		len = sprintf(buf, "I2C read failed: id = 0x%x, "
		    "addr = 0x%x\n", i2c_acc.i2c_id, i2c_addr);
	} else {
		len = sprintf(buf, "I2C read OK: id = 0x%x, "
		    "addr = 0x%x, data = 0x%x\n", i2c_acc.i2c_id,
		    i2c_addr, i2c_acc.i2c_data);
	}

	return len;
}

/* cat i2c_reg */
static ssize_t zynq_sysfs_i2c_reg_read(zynq_dev_t *zdev, char *buf)
{
	return zynq_sysfs_i2c_read(zdev, buf, 0);
}

/* cat i2c_reg16 */
static ssize_t zynq_sysfs_i2c_reg16_read(zynq_dev_t *zdev, char *buf)
{
	return zynq_sysfs_i2c_read(zdev, buf, 1);
}

static int zynq_sysfs_i2c_write(zynq_dev_t *zdev,
		const char *buf, unsigned char i2c_addr_16)
{
	ioc_zynq_i2c_acc_t i2c_acc;
	u32 i2c_id, i2c_addr, i2c_data;
	u32 addr_max = (i2c_addr_16) ? 0xffff : 0xff;
	int err = 0;

	/*
	 * SysFS has stripped all unnecessary spaces from buf (leading,
	 * trailing, extra in the middle)
	 */
	if (!strchr(buf, ' ')) {
		zynq_err("%s: bad format\n", __FUNCTION__);
		return -1;
	}

	if ((u64)strchr(buf, ' ') == (u64)strrchr(buf, ' ')) {
		/* only two arguments */
		sscanf(buf, "%x %x", &i2c_id, &i2c_addr);

		if (i2c_id > ZYNQ_I2C_ID_MAX) {
			zynq_err("%s: bad id 0x%x, maximum is 0x%x\n",
			    __FUNCTION__, i2c_id, ZYNQ_I2C_ID_MAX);
			return -1;
		}
		if (i2c_addr > addr_max) {
			zynq_err("%s: bad addr 0x%x, maximum is 0x%x\n",
			    __FUNCTION__, i2c_addr, addr_max);
			return -1;
		}
		zynq_log("I2C update OK: id = 0x%x, addr = 0x%x\n",
		    i2c_id, i2c_addr);

		zynq_sysfs_i2c_id = (u8)i2c_id;
		zynq_sysfs_i2c_addr = (u8)i2c_addr;
		zynq_sysfs_i2c_addr_hi = (i2c_addr_16) ?
		    (u8)(i2c_addr >> 8) : 0;
	} else {
		/* more than two arguments */
		sscanf(buf, "%x %x %x", &i2c_id, &i2c_addr, &i2c_data);

		if (i2c_id > ZYNQ_I2C_ID_MAX) {
			zynq_err("%s: bad id 0x%x, maximum is 0x%x\n",
			    __FUNCTION__, i2c_id, ZYNQ_I2C_ID_MAX);
			return -1;
		}
		if (i2c_addr > addr_max) {
			zynq_err("%s: bad addr 0x%x, maximum is 0x%x\n",
			    __FUNCTION__, i2c_addr, addr_max);
			return -1;
		}
		if (i2c_data > 0xff) {
			zynq_err("%s: bad data 0x%x, maximum is 0xff\n",
			    __FUNCTION__, i2c_data);
			return -1;
		}
		zynq_sysfs_i2c_id = (u8)i2c_id;
		zynq_sysfs_i2c_addr = (u8)i2c_addr;
		zynq_sysfs_i2c_addr_hi = (i2c_addr_16) ?
		    (u8)(i2c_addr >> 8) : 0;

		i2c_acc.i2c_id = zynq_sysfs_i2c_id;
		i2c_acc.i2c_addr_hi = zynq_sysfs_i2c_addr_hi;
		i2c_acc.i2c_addr = zynq_sysfs_i2c_addr;
		i2c_acc.i2c_data = (unsigned char)i2c_data;
		i2c_acc.i2c_addr_16 = i2c_addr_16;
		i2c_acc.i2c_bus = zynq_sysfs_i2c_bus;

		err = zdev_i2c_write(zdev, &i2c_acc);

		if (!err) {
			zynq_log("I2C write OK: id = 0x%x, addr = 0x%x, "
			    "data = 0x%x\n", i2c_id, i2c_addr, i2c_data);
		}
	}

	return err;
}

/*
 * echo <i2c_id> <i2c_addr> <i2c_data> > i2c_reg
 * echo <i2c_id> <i2c_addr> > i2c_reg
 */
static ssize_t zynq_sysfs_i2c_reg_write(zynq_dev_t *zdev,
		const char *buf, size_t sz)
{
	(void) zynq_sysfs_i2c_write(zdev, buf, 0);

	return sz;
}

/*
 * echo <i2c_id> <i2c_addr> <i2c_data> > i2c_reg16
 * echo <i2c_id> <i2c_addr> > i2c_reg16
 */
static ssize_t zynq_sysfs_i2c_reg16_write(zynq_dev_t *zdev,
		const char *buf, size_t sz)
{
	(void) zynq_sysfs_i2c_write(zdev, buf, 1);

	return sz;
}

static u32 zynq_sysfs_chan_reg_offset = 0;

/* cat chan_reg_value */
static ssize_t zynq_sysfs_chan_reg_read(zynq_dev_t *zdev, char *buf)
{
	zynq_chan_t *zchan;
	int len;

	zchan = zdev->zdev_chans + zynq_sysfs_chan;
	len = sprintf(buf, "channel %u register offset 0x%x = 0x%x\n",
	    zynq_sysfs_chan, zynq_sysfs_chan_reg_offset,
	    ZCHAN_REG32_RD(zchan, zynq_sysfs_chan_reg_offset));

	return len;
}

/*
 * echo <offset> <val> > chan_reg_value
 * echo <offset> > chan_reg_value
 */
static ssize_t zynq_sysfs_chan_reg_set(zynq_dev_t *zdev,
		const char *buf, size_t sz)
{
	zynq_chan_t *zchan;
	u32 offset, val;

	if (strchr(buf, ' ')) {
		sscanf(buf, "%x %x", &offset, &val);
		if (offset > ZYNQ_CHAN_REG_MAX) {
			zynq_err("bad channel register offset 0x%x, maximum is "
			    "0x%x\n", offset, ZYNQ_CHAN_REG_MAX);
		} else {
			zchan = zdev->zdev_chans + zynq_sysfs_chan;
			zynq_err("channel %u register offset 0x%x set to 0x%x\n",
			    zynq_sysfs_chan, offset, val);
			ZCHAN_REG32(zchan, offset) = val;
			zynq_sysfs_chan_reg_offset = offset;
		}
	} else {
		sscanf(buf, "%x", &offset);
		if (offset > ZYNQ_CHAN_REG_MAX) {
			zynq_err("bad channel register offset 0x%x, maximum is "
			    "0x%x\n", offset, ZYNQ_CHAN_REG_MAX);
		} else {
			zynq_err("channel %u register offset changed 0x%x -> "
			    "0x%x\n", zynq_sysfs_chan,
			    zynq_sysfs_chan_reg_offset, offset);
			zynq_sysfs_chan_reg_offset = offset;
		}
	}
	return sz;
}

static u32 zynq_sysfs_can_ip_reg_offset = 0;

/* cat can_ip_reg_value */
static ssize_t zynq_sysfs_can_ip_reg_read(zynq_dev_t *zdev, char *buf)
{
	zynq_can_t *zcan;
	int len;

	zcan = can_ip_to_zcan(zdev, zynq_sysfs_can_ip);
	if (zcan != NULL &&
	    zynq_sysfs_can_ip_reg_offset <= (ZCAN_IP_OFFSET - 4)) {
		len = sprintf(buf, "CAN IP %u register offset 0x%x = 0x%x\n",
		    zynq_sysfs_can_ip, zynq_sysfs_can_ip_reg_offset,
		    ZCAN_REG32_RD(zcan, zynq_sysfs_can_ip_reg_offset));
	} else {
		/* Bar2 direct access */
		len = sprintf(buf, "Bar2 register offset 0x%x = 0x%x\n",
		    zynq_sysfs_can_ip_reg_offset,
		    ZDEV_BAR2_REG32_RD(zdev, zynq_sysfs_can_ip_reg_offset));
	}

	return len;
}

/*
 * echo <offset> <val> > can_ip_reg_value
 * echo <offset> > can_ip_reg_value
 */
static ssize_t zynq_sysfs_can_ip_reg_set(zynq_dev_t *zdev,
		const char *buf, size_t sz)
{
	zynq_can_t *zcan;
	u32 offset, val;

	if (strchr(buf, ' ')) {
		sscanf(buf, "%x %x", &offset, &val);
		if (offset > (zdev->zdev_bar2_len - 4)) {
			zynq_err("bad Bar2/CAN IP register offset 0x%x, maximum "
			    "is 0x%x\n", offset, zdev->zdev_bar2_len - 4);
		} else {
			zcan = can_ip_to_zcan(zdev, zynq_sysfs_can_ip);
			if (zcan != NULL && offset <= (ZCAN_IP_OFFSET - 4)) {
				zynq_err("CAN IP %u register offset 0x%x set to "
				    "0x%x\n", zynq_sysfs_can_ip, offset, val);
				ZCAN_REG32(zcan, offset) = val;
			} else {
				zynq_err("Bar2 register offset 0x%x set to "
				    "0x%x\n", offset, val);
				ZDEV_BAR2_REG32(zdev, offset) = val;
			}
			zynq_sysfs_can_ip_reg_offset = offset;
		}
	} else {
		sscanf(buf, "%x", &offset);
		if (offset > (zdev->zdev_bar2_len - 4)) {
			zynq_err("bad Bar2/CAN IP register offset 0x%x, maximum "
			    "is 0x%x\n", offset, zdev->zdev_bar2_len - 4);
		} else {
			zcan = can_ip_to_zcan(zdev, zynq_sysfs_can_ip);
			if (zcan != NULL && offset <= (ZCAN_IP_OFFSET - 4)) {
				zynq_err("CAN IP %u register offset changed 0x%x"
				    " -> 0x%x\n", zynq_sysfs_can_ip,
				    zynq_sysfs_can_ip_reg_offset, offset);
			} else {
				zynq_err("Bar2 register offset changed 0x%x -> "
				    "0x%x\n", zynq_sysfs_can_ip_reg_offset,
				    offset);
			}
			zynq_sysfs_can_ip_reg_offset = offset;
		}
	}

	return sz;
}

/*
 * echo <loopback> <msgcnt> > can_ip_test
 */
static ssize_t zynq_sysfs_can_ip_test(zynq_dev_t *zdev,
		const char *buf, size_t sz)
{
	zynq_can_t *zcan;
	int loopback = 1; /* test loopback mode by default */
	int msgcnt = 1;

	zcan = can_ip_to_zcan(zdev, zynq_sysfs_can_ip);
	if (zdev->zcan_tx_dma) {
		zynq_err("CAN %u in not in PIO mode, use can_test instead\n",
		    zcan->zcan_ip_num);
		return sz;
	}
	if (strchr(buf, ' ')) {
		sscanf(buf, "%d %d", &loopback, &msgcnt);
	} else {
		sscanf(buf, "%d", &msgcnt);
	}

	zcan_pio_test(zcan, loopback, 0, msgcnt);

	return sz;
}

/*
 * echo <loopback> <msgcnt> > can_ip_hi_test
 */
static ssize_t zynq_sysfs_can_ip_hi_test(zynq_dev_t *zdev,
		const char *buf, size_t sz)
{
	zynq_can_t *zcan;
	int loopback = 1; /* test loopback mode by default */
	int msgcnt = 1;

	zcan = can_ip_to_zcan(zdev, zynq_sysfs_can_ip);
	if (zdev->zcan_tx_dma) {
		zynq_err("CAN %u is not in PIO mode, use can_test instead\n",
		    zcan->zcan_ip_num);
		return sz;
	}
	if (strchr(buf, ' ')) {
		sscanf(buf, "%d %d", &loopback, &msgcnt);
	} else {
		sscanf(buf, "%d", &msgcnt);
	}

	zcan_pio_test(zcan, loopback, 1, msgcnt);

	return sz;
}

/*
 * echo <loopback> <msgcnt> > can_test
 */
static ssize_t zynq_sysfs_can_test(zynq_dev_t *zdev, const char *buf, size_t sz)
{
	zynq_can_t *zcan;
	int loopback = 1; /* test loopback mode by default */
	int msgcnt = 1;

	zcan = can_ip_to_zcan(zdev, zynq_sysfs_can_ip);
	if (!zdev->zcan_tx_dma) {
		zynq_err("CAN %u is not in DMA mode, use can_ip_test instead\n",
		    zcan->zcan_ip_num);
		return sz;
	}
	if (strchr(buf, ' ')) {
		sscanf(buf, "%d %d", &loopback, &msgcnt);
	} else {
		sscanf(buf, "%d", &msgcnt);
	}

	zcan_test(zcan, loopback, msgcnt);

	return sz;
}

static u32 zynq_sysfs_tx_desc = 0;
/*
 * show the tx descriptor # that we are going to operate:
 *	cat tx_desc
 */
static ssize_t zynq_sysfs_tx_desc_read(zynq_dev_t *zdev, char *buf)
{
	int len;

	len = sprintf(buf, "tx descriptor number is %u\n", zynq_sysfs_tx_desc);

	return len;
}

/*
 * set the tx descriptor # that we are going to operate:
 *	echo <tx_desc#> > tx_desc
 */
static ssize_t zynq_sysfs_tx_desc_set(zynq_dev_t *zdev,
		const char *buf, size_t sz)
{
	zynq_chan_t *zchan;
	zchan_tx_ring_t *zchan_tx;
	u32 tx_desc;

	zchan = zdev->zdev_chans + zynq_sysfs_chan;
	if (zchan->zchan_type == ZYNQ_CHAN_VIDEO ||
	    (zchan->zchan_type == ZYNQ_CHAN_CAN && !zdev->zcan_tx_dma)) {
		zynq_err("not available: ch %d Tx is not in DMA mode\n",
		    zchan->zchan_num);
		return sz;
	}
	sscanf(buf, "%u", &tx_desc);
	zchan_tx = &zchan->zchan_tx_ring;
	if (tx_desc >= zchan_tx->zchan_tx_num) {
		zynq_err("bad tx descriptor number %u, maximum is %u\n",
		    tx_desc, zchan_tx->zchan_tx_num - 1);
	} else {
		zynq_err("tx descriptor number changed %u -> %u\n",
		    zynq_sysfs_tx_desc, tx_desc);
		zynq_sysfs_tx_desc = tx_desc;
	}

	return sz;
}

/*
 * show the channel Tx info:
 *	cat chan_tx_info
 */
static ssize_t zynq_sysfs_chan_tx_info_show(zynq_dev_t *zdev, char *buf)
{
	zynq_chan_t *zchan;
	zchan_tx_ring_t *zchan_tx;
	zchan_tx_desc_t *tx_descp;
	zchan_buf_t *bufp;
	int len = 0, i;

	zchan = zdev->zdev_chans + zynq_sysfs_chan;
	if (zchan->zchan_type == ZYNQ_CHAN_VIDEO ||
	    (zchan->zchan_type == ZYNQ_CHAN_CAN && !zdev->zcan_tx_dma)) {
		len = sprintf(buf + len, "info not available: ch %d Tx is not in "
		    "DMA mode\n", zchan->zchan_num);
		return len;
	}
	zchan_tx = &zchan->zchan_tx_ring;
	len = sprintf(buf + len, "------ channel %u Tx info ------\n",
	    zynq_sysfs_chan);
	len += sprintf(buf + len, "tx_descp=0x%p, tx_dma=0x%llx, bufsz=0x%x\n",
	    zchan_tx->zchan_tx_descp, zchan_tx->zchan_tx_dma,
	    zchan_tx->zchan_tx_bufsz);
	tx_descp = zchan_tx->zchan_tx_descp + zynq_sysfs_tx_desc;
	len += sprintf(buf + len, "tx_desc[%u]: 0x%llx, 0x%llx\n",
	    zynq_sysfs_tx_desc, *(u64 *)tx_descp, *((u64 *)tx_descp + 1));
	bufp = zchan_tx->zchan_tx_bufp + zynq_sysfs_tx_desc;
	for (i = 0; i < tx_descp->tx_len / 8; i++) {
		len += sprintf(buf + len, "tx_buf[%d]: 0x%llx\n", i,
		    *(((u64 *)bufp->zchan_bufp) + i));
	}
	len += sprintf(buf + len, "tx_head=%u, tx_tail=%u, tx_num=%u\n",
	    zchan_tx->zchan_tx_head, zchan_tx->zchan_tx_tail,
	    zchan_tx->zchan_tx_num);

	return len;
}

static ssize_t zynq_sysfs_chan_rx_info_dump(zynq_chan_t *zchan, char *buf)
{
	zchan_rx_tbl_t *zchan_rx;
	u64 *pdtp;
	zchan_rx_pt_t *ptp;
	int i, len = 0;

	zchan_rx = &zchan->zchan_rx_tbl;
	len = sprintf(buf + len, "------ channel %u Rx info ------\n",
	    zynq_sysfs_chan);
	len += sprintf(buf + len, "rx_pdt=0x%p, rx_pdt_dma=0x%llx, "
	    "rx_pdt_num=%u\n", zchan_rx->zchan_rx_pdt,
	    zchan_rx->zchan_rx_pdt_dma, zchan_rx->zchan_rx_pdt_num);

	pdtp = zchan_rx->zchan_rx_pdt;
	ptp = zchan_rx->zchan_rx_ptp;
	for (i = 0; i < zchan_rx->zchan_rx_pdt_num; i++, pdtp++, ptp++) {
		len += sprintf(buf + len, "rx_pdt[%d]=0x%llx: "
		    "rx_pt=0x%p(0x%llx), rx_pt_dma=0x%llx, rx_buf_dma=0x%llx\n",
		    i, *pdtp, ptp->zchan_rx_pt, *(ptp->zchan_rx_pt),
		    ptp->zchan_rx_pt_dma, ptp->zchan_rx_pt_bufp->zchan_buf_dma);
	}

	len += sprintf(buf + len, "rx_head=0x%x, rx_tail=0x%x, "
	    "rx_size=0x%x\n", zchan_rx->zchan_rx_head, zchan_rx->zchan_rx_tail,
	    zchan_rx->zchan_rx_size);

	return len;
}

/*
 * show the channel Rx info:
 *	cat chan_rx_info
 */
static ssize_t zynq_sysfs_chan_rx_info_show(zynq_dev_t *zdev, char *buf)
{
	zynq_chan_t *zchan;
	zynq_video_t *zvideo;
	int len;

	zchan = zdev->zdev_chans + zynq_sysfs_chan;
	switch (zchan->zchan_type) {
	case ZYNQ_CHAN_CAN:
		if (!zdev->zcan_rx_dma) {
			len = sprintf(buf, "info not available: "
			    "ch %d Rx is not in DMA mode\n",
			    zchan->zchan_num);
			return len;
		}
		len = zynq_sysfs_chan_rx_info_dump(zchan, buf);
		break;
	case ZYNQ_CHAN_VIDEO:
		zvideo = (zynq_video_t *)zchan->zchan_dev;
		if (zynq_video_zero_copy) {
			spin_lock_bh(&zvideo->qlock);
			len = zynq_sysfs_chan_rx_info_dump(zchan, buf);
			spin_unlock_bh(&zvideo->qlock);
		} else {
			len = zynq_sysfs_chan_rx_info_dump(zchan, buf);
		}
		break;
	default:
		len = sprintf(buf, "info not available: "
		    "ch %d channel type is not supported\n",
		    zchan->zchan_num);
		break;
	}

	return len;
}

/*
 * show the channel Rx info:
 *	cat chan_rx_info
 */
static ssize_t zynq_sysfs_chan_pio_info_show(zynq_dev_t *zdev, char *buf)
{
	zynq_can_t *zcan;
	zynq_chan_t *zchan;
	int len = 0;

	zcan = can_ip_to_zcan(zdev, zynq_sysfs_can_ip);
	zchan = zcan->zchan;

	len = sprintf(buf + len, "------ channel PIO %u info ------\n",
	    zynq_sysfs_can_ip);

	len += sprintf(buf + len, "pio_rx_tail=%d, pio_rx_head=%d, "
	    "pio_rx_num=%d, usr_buf_num=%d, pio_usr_rx_num=%d, "
	    "pio_baudrate=%lu, zcan_usr_rx_seq=%d\n", zcan->zcan_buf_rx_tail,
	    zcan->zcan_buf_rx_head, zcan->zcan_buf_rx_num,
	    zcan->zcan_usr_buf_num, zcan->zcan_usr_rx_num,
	    zcan->zcan_pio_baudrate, zcan->zcan_usr_rx_seq);

	if ((zynq_trace_param & ZYNQ_TRACE_CAN) || zynq_dbg_reg_dump_param) {
		zcan_pio_dbg_reg_dump_ch(zcan);
	}

	return (len);
}

static u32 zynq_sysfs_cam = 0;
static unsigned short zynq_sysfs_cam_reg = 0;

/*
 * show the camera number that we are going to operate:
 *	cat cam
 */
static ssize_t zynq_sysfs_cam_read(zynq_dev_t *zdev, char *buf)
{
	int len;

	len = sprintf(buf, "camera number is %u\n", zynq_sysfs_cam);

	return len;
}

/*
 * set the camera number that we are going to operate:
 *	echo <number> > cam
 */
static ssize_t zynq_sysfs_cam_set(zynq_dev_t *zdev, const char *buf, size_t sz)
{
	u32 cam;

	sscanf(buf, "%u", &cam);
	if (cam >= zdev->zdev_video_cnt) {
		zynq_err("invalid camera number %u, maximum is %u\n",
		    cam, zdev->zdev_video_cnt - 1);
	} else {
		zynq_err("camera number changed %u -> %u\n",
		    zynq_sysfs_cam, cam);
		zynq_sysfs_cam = cam;
	}

	return sz;
}

/*
 * read an 8bit camera register value
 *	cat cam_reg8
 */
static ssize_t zynq_sysfs_cam_reg8_read(zynq_dev_t *zdev, char *buf)
{
	zynq_video_t *zvideo;
	zynq_cam_acc_t cam_acc;
	int len;

	zvideo = &zdev->zdev_videos[zynq_sysfs_cam];

	cam_acc.addr = zynq_sysfs_cam_reg;
	cam_acc.data = 0;
	cam_acc.data_sz = 1;
	if (zcam_reg_read(zvideo, &cam_acc)) {
		len = sprintf(buf,
		    "failed to read camera %u register 0x%04hx\n",
		    zynq_sysfs_cam, zynq_sysfs_cam_reg);
		return len;
	}

	len = sprintf(buf, "camera %u register 0x%04hx = 0x%02hhx\n",
	    zynq_sysfs_cam, zynq_sysfs_cam_reg, (unsigned char)cam_acc.data);

	return len;
}

/*
 * echo <reg> <val> > cam_reg8
 * echo <reg> > cam_reg8
 */
static ssize_t zynq_sysfs_cam_reg8_set(zynq_dev_t *zdev,
		const char *buf, size_t sz)
{
	zynq_video_t *zvideo;
	zynq_cam_acc_t cam_acc;

	if (strchr(buf, ' ')) {
		sscanf(buf, "%hx %hhx",
		    &cam_acc.addr, (unsigned char *)&cam_acc.data);
		cam_acc.data_sz = 1;
		zvideo = &zdev->zdev_videos[zynq_sysfs_cam];
		if (zcam_reg_write(zvideo, &cam_acc)) {
			return sz;
		}
		zynq_sysfs_cam_reg = cam_acc.addr;
		zynq_log("camera %u register 0x%04hx written with 0x%02hhx\n",
		    zynq_sysfs_cam, zynq_sysfs_cam_reg,
		    (unsigned char)cam_acc.data);
	} else {
		sscanf(buf, "%hx", &cam_acc.addr);
		zynq_log("camera %u register changed 0x%04hx -> 0x%04hx\n",
		    zynq_sysfs_cam, zynq_sysfs_cam_reg, cam_acc.addr);
		zynq_sysfs_cam_reg = cam_acc.addr;
	}
	return sz;
}

/*
 * read a 16bit camera register value
 *	cat cam_reg16
 */
static ssize_t zynq_sysfs_cam_reg16_read(zynq_dev_t *zdev, char *buf)
{
	zynq_video_t *zvideo;
	zynq_cam_acc_t cam_acc;
	int len;

	zvideo = &zdev->zdev_videos[zynq_sysfs_cam];

	cam_acc.addr = zynq_sysfs_cam_reg;
	cam_acc.data = 0;
	cam_acc.data_sz = 2;
	if (zcam_reg_read(zvideo, &cam_acc)) {
		len = sprintf(buf,
		    "failed to read camera %u register 0x%04hx\n",
		    zynq_sysfs_cam, zynq_sysfs_cam_reg);
		return len;
	}

	len = sprintf(buf, "camera %u register 0x%04hx = 0x%04hx\n",
	    zynq_sysfs_cam, zynq_sysfs_cam_reg, (unsigned short)cam_acc.data);

	return len;
}

/*
 * echo <reg> <val> > cam_reg16
 * echo <reg> > cam_reg16
 */
static ssize_t zynq_sysfs_cam_reg16_set(zynq_dev_t *zdev,
		const char *buf, size_t sz)
{
	zynq_video_t *zvideo;
	zynq_cam_acc_t cam_acc;

	if (strchr(buf, ' ')) {
		sscanf(buf, "%hx %hx",
		    &cam_acc.addr, (unsigned short *)&cam_acc.data);
		cam_acc.data_sz = 2;
		zvideo = &zdev->zdev_videos[zynq_sysfs_cam];
		if (zcam_reg_write(zvideo, &cam_acc)) {
			return sz;
		}
		zynq_sysfs_cam_reg = cam_acc.addr;
		zynq_log("camera %u register 0x%04hx written with 0x%04hx\n",
		    zynq_sysfs_cam, zynq_sysfs_cam_reg,
		    (unsigned short)cam_acc.data);
	} else {
		sscanf(buf, "%hx", &cam_acc.addr);
		zynq_log("camera %u register changed 0x%04hx -> 0x%04hx\n",
		    zynq_sysfs_cam, zynq_sysfs_cam_reg, cam_acc.addr);
		zynq_sysfs_cam_reg = cam_acc.addr;
	}
	return sz;
}

#define	INDENT		2
#define	ALIGNMENT	ZYNQ_STATS_LABEL_LEN

static ssize_t zynq_sysfs_list_stats(char *buf, zynq_stats_t *stats, int num)
{
	ssize_t len = 0;
	int padding;
	int i;

	for (i = 0; i < num; i++) {
		padding = ALIGNMENT - strlen(stats[i].label);
		padding = (padding < 0) ? 0 : padding;
		len += sprintf(buf + len, "%*c%s%*c %7lu\n",
		    INDENT, ' ', stats[i].label, padding, ' ', stats[i].cnt);
	}

	return len;
}

static ssize_t zynq_sysfs_chan_stats_show(zynq_dev_t *zdev, char *buf)
{
	zynq_chan_t *zchan;
	zynq_can_t *zcan;
	zynq_video_t *zvideo;
	ssize_t len = 0;

	zchan = zdev->zdev_chans + zynq_sysfs_chan;

	switch (zchan->zchan_type) {
	case ZYNQ_CHAN_CAN:
		zcan = (zynq_can_t *)zchan->zchan_dev;
		len += sprintf(buf + len,
		    "Channel %d, can_ip %d, /dev/zynq_can%u:\n",
		    zchan->zchan_num, zcan->zcan_ip_num,
		    zchan->zdev->zdev_can_num_start + zcan->zcan_ip_num);
		len += zynq_sysfs_list_stats(buf + len,
		    zchan->stats, CHAN_STATS_NUM);
		len += zynq_sysfs_list_stats(buf + len,
		    zcan->stats, CAN_STATS_NUM);
		break;
	case ZYNQ_CHAN_VIDEO:
		zvideo = (zynq_video_t *)zchan->zchan_dev;
		len += sprintf(buf + len,
		    "Channel %d, vid %d, /dev/video%d:\n",
		    zchan->zchan_num, zvideo->index, zvideo->vdev.num);
		len += zynq_sysfs_list_stats(buf + len,
		    zchan->stats, CHAN_STATS_NUM);
		len += zynq_sysfs_list_stats(buf + len,
		    zvideo->stats, VIDEO_STATS_NUM);
		break;
	default:
		break;
	}

	return len;
}

static ssize_t zynq_sysfs_stats_show(zynq_dev_t *zdev, char *buf)
{
	zynq_chan_t *zchan;
	zynq_can_t *zcan;
	zynq_video_t *zvideo;
	zynq_stats_t *stats;
	int vcnt;
	ssize_t len = 0;
	int padding;
	int i, j;

	len += sprintf(buf + len, "General:\n");
	len += zynq_sysfs_list_stats(buf + len, zdev->stats, DEV_STATS_NUM);
	if (zynq_trace_param & ZYNQ_TRACE_GPS) {
		long drift = 0;
		long period = zdev->zdev_gps_ts.tv_sec -
		    zdev->zdev_gps_ts_first.tv_sec;
		if (period > 0) {
			long total = zdev->zdev_sys_drift;
			if (total < 0) {
				total = -total;
			}
			drift = total / period;
			if ((total - (drift * period)) >
			    ((drift + 1) * period - total)) {
				drift++;
			}
			if (zdev->zdev_sys_drift < 0) {
				drift = -drift;
			}
		}
		len += sprintf(buf + len, "%*c%s%*c %7ld\n",
		    INDENT, ' ', "SYS time drift", ALIGNMENT - 14, ' ', drift);
	}

	/* Print CAN channel statistics */
	len += sprintf(buf + len, "\nCAN Channels:%*c",
	    ALIGNMENT + INDENT - 13, ' ');
	for (j = 0; j < zdev->zdev_can_cnt; j++) {
		zcan = &zdev->zdev_cans[j];
		len += sprintf(buf + len, "    can%u",
		    zdev->zdev_can_num_start + zcan->zcan_ip_num);
	}
	for (i = 0; i < CHAN_STATS_NUM; i++) {
		stats = &zdev->zdev_cans[0].zchan->stats[i];
		padding = ALIGNMENT - strlen(stats->label);
		padding = (padding < 0) ? 0 : padding;
		len += sprintf(buf + len, "\n%*c%s%*c",
		    INDENT, ' ', stats->label, padding, ' ');
		for (j = 0; j < zdev->zdev_can_cnt; j++) {
			zchan = zdev->zdev_cans[j].zchan;
			len += sprintf(buf + len, " %7lu",
			    zchan->stats[i].cnt);
		}
	}
	for (i = 0; i < CAN_STATS_NUM; i++) {
		stats = &zdev->zdev_cans[0].stats[i];
		padding = ALIGNMENT - strlen(stats->label);
		padding = (padding < 0) ? 0 : padding;
		len += sprintf(buf + len, "\n%*c%s%*c",
		    INDENT, ' ', stats->label, padding, ' ');
		for (j = 0; j < zdev->zdev_can_cnt; j++) {
			zcan = &zdev->zdev_cans[j];
			len += sprintf(buf + len, " %7lu",
			    zcan->stats[i].cnt);
		}
	}
	len += sprintf(buf + len, "\n");

	vcnt = 0;
	for (j = 0; j < zdev->zdev_video_cnt; j++) {
		zvideo = &zdev->zdev_videos[j];
		if (video_is_registered(&zvideo->vdev)) {
			vcnt++;
		}
	}
	if (vcnt == 0) {
		return len;
	}

	/* Print Video channel statistics */
	len += sprintf(buf + len, "\nVideo Channels:%*c",
	    ALIGNMENT + INDENT - 15, ' ');
	for (j = 0; j < zdev->zdev_video_cnt; j++) {
		zvideo = &zdev->zdev_videos[j];
		if (!video_is_registered(&zvideo->vdev)) {
			continue;
		}
		padding = (zvideo->vdev.num < 10) ? 2 : 1;
		len += sprintf(buf + len, "%*cvideo%d",
		    padding, ' ', zvideo->vdev.num);
	}
	for (i = 0; i < CHAN_STATS_NUM; i++) {
		stats = &zdev->zdev_videos[0].zchan->stats[i];
		padding = ALIGNMENT - strlen(stats->label);
		padding = (padding < 0) ? 0 : padding;
		len += sprintf(buf + len, "\n%*c%s%*c",
		    INDENT, ' ', stats->label, padding, ' ');
		for (j = 0; j < zdev->zdev_video_cnt; j++) {
			zvideo = &zdev->zdev_videos[j];
			if (!video_is_registered(&zvideo->vdev)) {
				continue;
			}
			zchan = zvideo->zchan;
			len += sprintf(buf + len, " %7lu",
			    zchan->stats[i].cnt);
		}
	}
	for (i = 0; i < VIDEO_STATS_NUM; i++) {
		stats = &zdev->zdev_videos[0].stats[i];
		padding = ALIGNMENT - strlen(stats->label);
		padding = (padding < 0) ? 0 : padding;
		len += sprintf(buf + len, "\n%*c%s%*c",
		    INDENT, ' ', stats->label, padding, ' ');
		for (j = 0; j < zdev->zdev_video_cnt; j++) {
			zvideo = &zdev->zdev_videos[j];
			if (!video_is_registered(&zvideo->vdev)) {
				continue;
			}
			len += sprintf(buf + len, " %7lu",
			    zvideo->stats[i].cnt);
		}
	}
	len += sprintf(buf + len, "\n");

	return len;
}

/*
 * echo 1 > cam_change_config
 */
static ssize_t zynq_sysfs_cam_change_config(zynq_dev_t *zdev,
		const char *buf, size_t sz)
{
	zynq_video_t *zvideo;
	int data;

	sscanf(buf, "%d", &data);
	if (data != 0) {
		zvideo = &zdev->zdev_videos[zynq_sysfs_cam];
		(void) zcam_change_config(zvideo);
	}

	return sz;
}

struct zynq_sysfs_attr {
	struct attribute attr;
	ssize_t (*show)(zynq_dev_t *zdev, char *buf);
	ssize_t (*store)(zynq_dev_t *zdev, const char *buf, size_t sz);
};

#define TO_ZYNQ_DEV_ATTR(x) container_of(x, struct zynq_sysfs_attr, attr)

#define SET_ZYNQ_DEV_ATTR(name, mode, show, store)		\
	static struct zynq_sysfs_attr zynq_sysfs_attr_##name =	\
		__ATTR(name, mode, show, store)

SET_ZYNQ_DEV_ATTR(num_can_ip, 0440, zynq_sysfs_num_can_ip_show, NULL);
SET_ZYNQ_DEV_ATTR(num_chan, 0440, zynq_sysfs_num_chan_show, NULL);
SET_ZYNQ_DEV_ATTR(can_ip, 0660, zynq_sysfs_can_ip_read, zynq_sysfs_can_ip_set);
SET_ZYNQ_DEV_ATTR(chan, 0660, zynq_sysfs_chan_read, zynq_sysfs_chan_set);
SET_ZYNQ_DEV_ATTR(g_reg_value, 0660, zynq_sysfs_g_reg_read,
    zynq_sysfs_g_reg_set);
SET_ZYNQ_DEV_ATTR(i2c_bus, 0660, zynq_sysfs_i2c_bus_read,
    zynq_sysfs_i2c_bus_set);
SET_ZYNQ_DEV_ATTR(i2c_reg_value, 0660, zynq_sysfs_i2c_reg_read,
    zynq_sysfs_i2c_reg_write);
SET_ZYNQ_DEV_ATTR(i2c_reg16_value, 0660, zynq_sysfs_i2c_reg16_read,
    zynq_sysfs_i2c_reg16_write);
SET_ZYNQ_DEV_ATTR(chan_reg_value, 0660, zynq_sysfs_chan_reg_read,
    zynq_sysfs_chan_reg_set);
SET_ZYNQ_DEV_ATTR(can_ip_reg_value, 0660, zynq_sysfs_can_ip_reg_read,
    zynq_sysfs_can_ip_reg_set);
SET_ZYNQ_DEV_ATTR(can_ip_test, 0220, NULL, zynq_sysfs_can_ip_test);
SET_ZYNQ_DEV_ATTR(can_ip_hi_test, 0220, NULL, zynq_sysfs_can_ip_hi_test);
SET_ZYNQ_DEV_ATTR(can_test, 0220, NULL, zynq_sysfs_can_test);
SET_ZYNQ_DEV_ATTR(tx_desc, 0660, zynq_sysfs_tx_desc_read,
    zynq_sysfs_tx_desc_set);
SET_ZYNQ_DEV_ATTR(chan_tx_info, 0440, zynq_sysfs_chan_tx_info_show, NULL);
SET_ZYNQ_DEV_ATTR(chan_rx_info, 0440, zynq_sysfs_chan_rx_info_show, NULL);
SET_ZYNQ_DEV_ATTR(chan_pio_info, 0440, zynq_sysfs_chan_pio_info_show, NULL);
SET_ZYNQ_DEV_ATTR(cam, 0660, zynq_sysfs_cam_read, zynq_sysfs_cam_set);
SET_ZYNQ_DEV_ATTR(cam_reg8, 0660, zynq_sysfs_cam_reg8_read,
    zynq_sysfs_cam_reg8_set);
SET_ZYNQ_DEV_ATTR(cam_reg16, 0660, zynq_sysfs_cam_reg16_read,
    zynq_sysfs_cam_reg16_set);
SET_ZYNQ_DEV_ATTR(chan_stats, 0440, zynq_sysfs_chan_stats_show, NULL);
SET_ZYNQ_DEV_ATTR(stats, 0440, zynq_sysfs_stats_show, NULL);
SET_ZYNQ_DEV_ATTR(cam_change_config, 0220, NULL, zynq_sysfs_cam_change_config);

static struct attribute *zynq_sysfs_attrs[] = {
	&zynq_sysfs_attr_num_can_ip.attr,
	&zynq_sysfs_attr_num_chan.attr,
	&zynq_sysfs_attr_can_ip.attr,
	&zynq_sysfs_attr_chan.attr,
	&zynq_sysfs_attr_g_reg_value.attr,
	&zynq_sysfs_attr_i2c_bus.attr,
	&zynq_sysfs_attr_i2c_reg_value.attr,
	&zynq_sysfs_attr_i2c_reg16_value.attr,
	&zynq_sysfs_attr_chan_reg_value.attr,
	&zynq_sysfs_attr_can_ip_reg_value.attr,
	&zynq_sysfs_attr_can_ip_test.attr,
	&zynq_sysfs_attr_can_ip_hi_test.attr,
	&zynq_sysfs_attr_can_test.attr,
	&zynq_sysfs_attr_tx_desc.attr,
	&zynq_sysfs_attr_chan_tx_info.attr,
	&zynq_sysfs_attr_chan_rx_info.attr,
	&zynq_sysfs_attr_chan_pio_info.attr,
	&zynq_sysfs_attr_cam.attr,
	&zynq_sysfs_attr_cam_reg8.attr,
	&zynq_sysfs_attr_cam_reg16.attr,
	&zynq_sysfs_attr_chan_stats.attr,
	&zynq_sysfs_attr_stats.attr,
	&zynq_sysfs_attr_cam_change_config.attr,
	NULL
};

static struct attribute *zynq_sysfs_fw_attrs[] = {
	&zynq_sysfs_attr_g_reg_value.attr,
	&zynq_sysfs_attr_can_ip_reg_value.attr,
	NULL
};

#define TO_ZYNQ_DEV(x) container_of(x, zynq_dev_t, zdev_kobj)

static ssize_t zynq_show_sysfs(struct kobject *kobj,
		struct attribute *attr, char *buf)
{
	struct zynq_sysfs_attr *a = TO_ZYNQ_DEV_ATTR(attr);
	zynq_dev_t *zdev = TO_ZYNQ_DEV(kobj);
	return a->show ? a->show(zdev, buf) : 0;
}

static ssize_t zynq_store_sysfs(struct kobject *kobj,
		struct attribute *attr, const char *buf, size_t sz)
{
	struct zynq_sysfs_attr *a = TO_ZYNQ_DEV_ATTR(attr);
	zynq_dev_t *zdev = TO_ZYNQ_DEV(kobj);
	return a->store ? a->store(zdev, buf, sz) : 0;
}

static struct sysfs_ops zynq_sysfs_ops = {
	.show = zynq_show_sysfs,
	.store = zynq_store_sysfs
};

struct kobj_type zynq_kobj_type = {
	.sysfs_ops = &zynq_sysfs_ops,
	.default_attrs = zynq_sysfs_attrs
};

struct kobj_type zynq_kobj_fw_type = {
	.sysfs_ops = &zynq_sysfs_ops,
	.default_attrs = zynq_sysfs_fw_attrs
};

void zynq_sysfs_fini(zynq_dev_t *zdev)
{
	if (zdev == NULL) {
		return;
	}
	kobject_put(&zdev->zdev_kobj);
	kobject_del(&zdev->zdev_kobj);
}

int zynq_sysfs_init(zynq_dev_t *zdev)
{
	int ret;

	if (zdev == NULL) {
		return (-1);
	}

	if (zynq_fwupdate_param) {
		ret = kobject_init_and_add(&zdev->zdev_kobj, &zynq_kobj_fw_type,
		    NULL, zdev->zdev_name);
	} else {
		ret = kobject_init_and_add(&zdev->zdev_kobj, &zynq_kobj_type,
		    NULL, zdev->zdev_name);
	}
	if (ret) {
		zynq_err("%s: failed to init and add kobj %d.\n",
		    __FUNCTION__, ret);
		return (ret);
	}

	zynq_trace(ZYNQ_TRACE_SYSFS, "%s: created /sys/%s\n", __FUNCTION__,
	    zdev->zdev_name);
	return (0);
}
