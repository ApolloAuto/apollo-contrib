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

#include "basa.h"

#define	ZCAN_PIO_RX_POLLING_NS		1000000	/* 1ms in nanoseconds */
#define	ZCAN_PIO_TX_RETRIES		500
#define	ZCAN_PIO_RX_RETRIES		500
/*
 * minimum time for h/w to transmit a CAN msg: 50us (1btye data @1Mbps),
 *	130us (8btye data @1Mbps), 1040 us (8btye data @125Kbps).
 */
/* Min # of micro-seconds to sleep using usleep_range() */
#define	ZCAN_PIO_WAIT_US_MIN		150 /* 500Kbps */
/* Max # of micro-seconds to sleep using usleep_range() */
#define	ZCAN_PIO_WAIT_US_MAX		260 /* 500Kbps */
#define	ZCAN_PIO_WAIT_DELAY		200 /* 200us */
#define	ZCAN_PIO_STOP_DELAY		100 /* 100ms */

static void zcan_pio_rx_proc(zynq_can_t *zcan);

/* CAN Rx buffer number */
static unsigned int zynq_can_rx_buf_num = ZCAN_IP_MSG_NUM;
/* CAN bus sampling point */
static unsigned int zynq_can_ip_btr = ZCAN_IP_BTR_DEFAULT;
/* disabling CAN Rx H/W timestamp */
static unsigned int zynq_disable_can_hw_ts = 0;
/* enabling CAN Rx DMA mode */
unsigned int zynq_enable_can_rx_dma = 1;
/* enabling CAN Tx DMA mode */
unsigned int zynq_enable_can_tx_dma = 0;

/* keep the order in 'enum zynq_baudrate_val' */
static int brpr_reg_val[ZYNQ_BAUDRATE_NUM] = {
	ZCAN_IP_BRPR_1M, ZCAN_IP_BRPR_500K, ZCAN_IP_BRPR_250K, ZCAN_IP_BRPR_125K
};

struct dbg_reg {
	int reg;
	char reg_desc[32];
};
#define	CAN_IP_DBG_REG_NUM	12
static struct dbg_reg can_ip_dbg_reg[CAN_IP_DBG_REG_NUM] = {
	{ ZCAN_IP_SRR,		"s/w reset" },
	{ ZCAN_IP_MSR,		"mode select" },
	{ ZCAN_IP_BRPR,		"baud rate prescaler" },
	{ ZCAN_IP_BTR,		"bit timing" },
	{ ZCAN_IP_ECR,		"error counter" },
	{ ZCAN_IP_ESR,		"error status" },
	{ ZCAN_IP_SR,		"ip status" },
	{ ZCAN_IP_ISR,		"interrupt status" },
	{ ZCAN_IP_IER,		"interrupt enable" },
	{ ZCAN_IP_ICR,		"interrupt clear" },
	{ ZCAN_IP_AFR,		"acceptance filter" },
	{ ZCAN_IP_PIO_STATUS,	"pio status" }
};

#define	CAN_IP_NEW_DBG_REG_NUM	3
static struct dbg_reg can_ip_new_dbg_reg[CAN_IP_NEW_DBG_REG_NUM] = {
	{ ZCAN_IP_CFG0,		"ip_cfg0" },
	{ ZCAN_IP_DBG_CNT0,	"ip_dbg_cnt0" },
	{ ZCAN_IP_DBG_CNT1,	"ip_dbg_cnt1" }
};

#define	CHAN_DBG_REG_NUM	1
static struct dbg_reg chan_dbg_reg[CHAN_DBG_REG_NUM] = {
	{ ZYNQ_CH_ERR_STATUS,	"chan error" }
};

#define	CHAN_NEW_DBG_REG_NUM	2
static struct dbg_reg chan_new_dbg_reg[CHAN_NEW_DBG_REG_NUM] = {
	{ ZYNQ_CH_ERR_MASK_TX,	"chan error mask0" },
	{ ZYNQ_CH_ERR_MASK_RX,	"chan error mask1" }
};

#define	G_DBG_REG_NUM		5
static struct dbg_reg g_dbg_reg[G_DBG_REG_NUM] = {
	{ ZYNQ_G_VERSION,	"version" },
	{ ZYNQ_G_STATUS,	"g_status" },
	{ ZYNQ_G_INTR_STATUS_TX, "g_intr_status_tx" },
	{ ZYNQ_G_INTR_STATUS_RX, "g_intr_status_rx" },
	{ ZYNQ_G_PS_INTR_STATUS, "g_ps_intr_status" }
};

#define	G_NEW_DBG_REG_NUM	12
static struct dbg_reg g_new_dbg_reg[G_NEW_DBG_REG_NUM] = {
	{ ZYNQ_G_DEBUG_CFG0,	"g_debug_cfg0" },
	{ ZYNQ_G_DEBUG_CFG1,	"g_debug_cfg1" },
	{ ZYNQ_G_DEBUG_CFG2,	"g_debug_cfg2" },
	{ ZYNQ_G_DEBUG_CFG3,	"g_debug_cfg3" },
	{ ZYNQ_G_DEBUG_STA0,	"g_debug_sta0" },
	{ ZYNQ_G_DEBUG_STA1,	"g_debug_sta1" },
	{ ZYNQ_G_DEBUG_STA2,	"g_debug_sta2" },
	{ ZYNQ_G_DEBUG_STA3,	"g_debug_sta3" },
	{ ZYNQ_G_DEBUG_CNT_CFG,	"g_debug_cnt_cfg" },
	{ ZYNQ_G_DEBUG_CNT0,	"g_debug_cnt0" },
	{ ZYNQ_G_DEBUG_CNT1,	"g_debug_cnt1" },
	{ ZYNQ_G_DEBUG_CNT2,	"g_debug_cnt2" }
};

static const char zcan_stats_label[CAN_STATS_NUM][ZYNQ_STATS_LABEL_LEN] = {
	"PIO Rx",
	"PIO Tx",
	"PIO Tx Hi",
	"User Rx wait",
	"User Rx wait intr",
	"User Rx timeout",
	"User Rx partial",
	"Bus off",
	"Status error",
	"Rx IP fifo overflow",
	"Rx user fifo overflow",
	"Tx timeout",
	"Tx LP fifo full",
	"Rx user fifo full",
	"Tx HP fifo full",
	"CRC error",
	"Frame error",
	"Stuff error",
	"Bit error",
	"Ack error"
};

/*
 * Register dump functions for debugging purpose
 */
static void zcan_pio_dbg_reg_dump(zynq_can_t *zcan)
{
	int i;

	zynq_err("%d CAN IP %d dbg register dump:\n", ZYNQ_INST(zcan->zchan),
	    zcan->zcan_ip_num);
	for (i = 0; i < CAN_IP_DBG_REG_NUM; i++) {
		zynq_err("	0x%x(%s) = 0x%x\n", can_ip_dbg_reg[i].reg,
		    can_ip_dbg_reg[i].reg_desc,
		    zcan_reg_read(zcan, can_ip_dbg_reg[i].reg));
	}
	for (i = 0; i < CAN_IP_NEW_DBG_REG_NUM; i++) {
		zynq_err("	0x%x(%s) = 0x%x\n", can_ip_new_dbg_reg[i].reg,
		    can_ip_new_dbg_reg[i].reg_desc,
		    zcan_reg_read(zcan, can_ip_new_dbg_reg[i].reg));
	}
}

static void zchan_dbg_reg_dump(zynq_chan_t *zchan)
{
	int i;

	zynq_err("%d chan %d dbg register dump:\n", ZYNQ_INST(zchan),
	    zchan->zchan_num);
	for (i = 0; i < CHAN_DBG_REG_NUM; i++) {
		zynq_err("	0x%x(%s) = 0x%x\n", chan_dbg_reg[i].reg,
		    chan_dbg_reg[i].reg_desc,
		    zchan_reg_read(zchan, chan_dbg_reg[i].reg));
	}
	for (i = 0; i < CHAN_NEW_DBG_REG_NUM; i++) {
		zynq_err("	0x%x(%s) = 0x%x\n", chan_new_dbg_reg[i].reg,
		    chan_new_dbg_reg[i].reg_desc,
		    zchan_reg_read(zchan, chan_new_dbg_reg[i].reg));
	}
}

static void zynq_dbg_reg_dump(zynq_dev_t *zdev)
{
	int i;

	zynq_err("%d global dbg register dump:\n", zdev->zdev_inst);
	for (i = 0; i < G_DBG_REG_NUM; i++) {
		zynq_err("	0x%x(%s) = 0x%x\n", g_dbg_reg[i].reg,
		    g_dbg_reg[i].reg_desc,
		    zynq_g_reg_read(zdev, g_dbg_reg[i].reg));
	}
	for (i = 0; i < G_NEW_DBG_REG_NUM; i++) {
		zynq_err("	0x%x(%s) = 0x%x\n", g_new_dbg_reg[i].reg,
		    g_new_dbg_reg[i].reg_desc,
		    zynq_g_reg_read(zdev, g_new_dbg_reg[i].reg));
	}
}

void zcan_pio_dbg_reg_dump_ch(zynq_can_t *zcan)
{
	zynq_dbg_reg_dump(zcan->zdev);
	zchan_dbg_reg_dump(zcan->zchan);
	zcan_pio_dbg_reg_dump(zcan);
}

void zynq_dbg_reg_dump_all(zynq_dev_t *zdev)
{
	zynq_can_t *zcan = zdev->zdev_cans;
	int i;

	zynq_dbg_reg_dump(zdev);
	for (i = 0; i < zdev->zdev_can_cnt; i++, zcan++) {
		zchan_dbg_reg_dump(zcan->zchan);
		zcan_pio_dbg_reg_dump(zcan);
	}
}

/*
 * Error interrupt handling in PIO mode
 */
void zcan_err_proc(zynq_can_t *zcan, int ch_err_status)
{
	if ((zynq_trace_param & ZYNQ_TRACE_CAN) || zynq_dbg_reg_dump_param) {
		zcan_pio_dbg_reg_dump_ch(zcan);
	}

	/* error statistics */
	if (ch_err_status & ZYNQ_CH_ERR_CAN_BUSOFF) {
		ZYNQ_STATS_LOG(zcan, CAN_STATS_BUS_OFF);
	}
	if (ch_err_status & ZYNQ_CH_ERR_CAN_STATERR) {
		ZYNQ_STATS_LOG(zcan, CAN_STATS_STATUS_ERR);
	}
	if (ch_err_status & ZYNQ_CH_ERR_CAN_RX_IPOVERFLOW) {
		ZYNQ_STATS_LOG(zcan, CAN_STATS_RX_IP_FIFO_OVF);
	}
	if (ch_err_status & ZYNQ_CH_ERR_CAN_RX_USROVERFLOW) {
		ZYNQ_STATS_LOG(zcan, CAN_STATS_RX_USR_FIFO_OVF);
	}
	if (ch_err_status & ZYNQ_CH_ERR_CAN_TX_TIMEOUT) {
		ZYNQ_STATS_LOG(zcan, CAN_STATS_TX_TIMEOUT);
	}
	if (ch_err_status & ZYNQ_CH_ERR_CAN_TX_LPFIFOFULL) {
		ZYNQ_STATS_LOG(zcan, CAN_STATS_TX_LP_FIFO_FULL);
	}
	if (ch_err_status & ZYNQ_CH_ERR_CAN_RX_FIFOFULL) {
		ZYNQ_STATS_LOG(zcan, CAN_STATS_RX_USR_FIFO_FULL);
	}
	if (ch_err_status & ZYNQ_CH_ERR_CAN_TX_HPFIFOFULL) {
		ZYNQ_STATS_LOG(zcan, CAN_STATS_TX_HP_FIFO_FULL);
	}
	if (ch_err_status & ZYNQ_CH_ERR_CAN_CRC_ERR) {
		ZYNQ_STATS_LOG(zcan, CAN_STATS_CRC_ERR);
	}
	if (ch_err_status & ZYNQ_CH_ERR_CAN_FRAME_ERR) {
		ZYNQ_STATS_LOG(zcan, CAN_STATS_FRAME_ERR);
	}
	if (ch_err_status & ZYNQ_CH_ERR_CAN_STUFF_ERR) {
		ZYNQ_STATS_LOG(zcan, CAN_STATS_STUFF_ERR);
	}
	if (ch_err_status & ZYNQ_CH_ERR_CAN_BIT_ERR) {
		ZYNQ_STATS_LOG(zcan, CAN_STATS_BIT_ERR);
	}
	if (ch_err_status & ZYNQ_CH_ERR_CAN_ACK_ERR) {
		ZYNQ_STATS_LOG(zcan, CAN_STATS_ACK_ERR);
	}
}

/*
 * Initialize the CAN IP: required for both DMA and PIO mode
 */
static void zcan_pio_chip_init(zynq_can_t *zcan, u32 ip_mode, int do_reset)
{
	zynq_chan_t *zchan = zcan->zchan;

	if (do_reset) {
		/* reset the device */
		zcan_reg_write(zcan, ZCAN_IP_SRR, 0);
		mdelay(10);
		zchan_reg_write(zchan, ZYNQ_CH_RESET, ZYNQ_CH_RESET_EN);
		zchan_reg_write(zchan, ZYNQ_CH_RESET, 0);
		mdelay(1);
	} else {
		zcan_reg_write(zcan, ZCAN_IP_SRR, 0);
		mdelay(10);
	}

	/*
	 * Configure the CAN core
	 */
	/* 1. choose the operation mode: mode select regster */
	zcan_reg_write(zcan, ZCAN_IP_MSR, ip_mode);

	/* 2. configure the transfer layer configuration registers */
	zcan_reg_write(zcan, ZCAN_IP_BRPR,
	    brpr_reg_val[zcan->zcan_pio_baudrate]);
	zcan_reg_write(zcan, ZCAN_IP_BTR, zynq_can_ip_btr);
	/* 3. configure the acceptance filter registers: accept ALL */
	zcan_reg_write(zcan, ZCAN_IP_AFR, ZCAN_IP_AFR_NONE);
	/* 4. set the interrupt enable register */
	zcan_reg_write(zcan, ZCAN_IP_IER, ZCAN_IP_INTR_ALL);

	zcan->zcan_pio_state = ZYNQ_STATE_INIT;

	/* make sure the previous pending interrupts are cleared */
	zdev_clear_intr_ch(zcan->zdev, zchan->zchan_num);

	zynq_trace(ZYNQ_TRACE_CAN_PIO, "%d can%d ch%d %s msr=0x%x, brpr=0x%x, "
	    "btr=0x%x, afr=0x%x, ier=0x%x, srr=0x%x\n", ZYNQ_INST(zchan),
	    zcan->zcan_ip_num, zchan->zchan_num,
	    __FUNCTION__, zcan_reg_read(zcan, ZCAN_IP_MSR),
	    zcan_reg_read(zcan, ZCAN_IP_BRPR),
	    zcan_reg_read(zcan, ZCAN_IP_BTR),
	    zcan_reg_read(zcan, ZCAN_IP_AFR),
	    zcan_reg_read(zcan, ZCAN_IP_IER),
	    zcan_reg_read(zcan, ZCAN_IP_SRR));
}

/*
 * Set CAN tx timeout value
 */
static void zcan_tx_timeout_set(zynq_can_t *zcan, unsigned long timeout)
{
	spin_lock(&zcan->zcan_pio_lock);
	zcan->zcan_tx_timeout = timeout;
	zynq_trace(ZYNQ_TRACE_CAN_PIO, "%d can%d %s done: tx_timeout=%ld\n",
	    ZYNQ_INST(zcan->zchan), zcan->zcan_ip_num, __FUNCTION__, timeout);
	spin_unlock(&zcan->zcan_pio_lock);
}

/*
 * Set CAN rx timeout value
 */
static void zcan_rx_timeout_set(zynq_can_t *zcan, unsigned long timeout)
{
	spin_lock(&zcan->zcan_pio_lock);
	zcan->zcan_rx_timeout = timeout;
	zynq_trace(ZYNQ_TRACE_CAN_PIO, "%d can%d %s done: rx_timeout=%ld\n",
	    ZYNQ_INST(zcan->zchan), zcan->zcan_ip_num, __FUNCTION__, timeout);
	spin_unlock(&zcan->zcan_pio_lock);
}

/*
 * start CAN IP
 */
static void zcan_pio_start(zynq_can_t *zcan)
{
	spin_lock(&zcan->zcan_pio_lock);
	/* read out the firmware version again */
	zcan->zdev->zdev_version = zynq_g_reg_read(zcan->zdev,
	    ZYNQ_G_VERSION);
	/* enable CAN core by setting CEN bit */
	zcan_reg_write(zcan, ZCAN_IP_SRR, ZCAN_IP_SRR_CEN);
	zcan->zcan_pio_state = ZYNQ_STATE_START;
	zynq_trace(ZYNQ_TRACE_CAN_PIO, "%d can%d %s done\n",
	    ZYNQ_INST(zcan->zchan), zcan->zcan_ip_num, __FUNCTION__);
	spin_unlock(&zcan->zcan_pio_lock);
}

/*
 * stop CAN IP
 */
static void zcan_pio_stop(zynq_can_t *zcan)
{
	spin_lock(&zcan->zcan_pio_lock);
	spin_lock(&zcan->zcan_pio_tx_lock);
	mdelay(ZCAN_PIO_STOP_DELAY);
	zcan->zcan_pio_state = ZYNQ_STATE_STOP;
	/* disable CAN core by clearing CEN bit */
	zcan_reg_write(zcan, ZCAN_IP_SRR, 0);
	zynq_trace(ZYNQ_TRACE_CAN_PIO, "%d can%d %s done\n",
	    ZYNQ_INST(zcan->zchan), zcan->zcan_ip_num, __FUNCTION__);
	spin_unlock(&zcan->zcan_pio_tx_lock);
	spin_unlock(&zcan->zcan_pio_lock);
}

/*
 * reset and re-initialize CAN IP
 */
static void zcan_pio_reset(zynq_can_t *zcan)
{
	spin_lock(&zcan->zcan_pio_lock);
	spin_lock(&zcan->zcan_pio_tx_lock);
	spin_lock_bh(&zcan->zcan_pio_rx_lock);
	zcan->zcan_pio_state = ZYNQ_STATE_RESET;
	zcan_pio_chip_init(zcan, ZCAN_IP_MSR_NORMAL, 1);
	zynq_trace(ZYNQ_TRACE_CAN_PIO, "%d can%d %s done\n",
	    ZYNQ_INST(zcan->zchan), zcan->zcan_ip_num, __FUNCTION__);
	spin_unlock_bh(&zcan->zcan_pio_rx_lock);
	spin_unlock(&zcan->zcan_pio_tx_lock);
	spin_unlock(&zcan->zcan_pio_lock);
}

/*
 * reset CAN IP
 */
static void zcan_pio_reset_noinit(zynq_can_t *zcan)
{
	zynq_chan_t *zchan = zcan->zchan;

	spin_lock(&zcan->zcan_pio_lock);
	spin_lock(&zcan->zcan_pio_tx_lock);
	spin_lock_bh(&zcan->zcan_pio_rx_lock);
	zcan->zcan_pio_state = ZYNQ_STATE_RESET;
	/* reset device */
	zcan_reg_write(zcan, ZCAN_IP_SRR, 0);
	mdelay(10);
	zchan_reg_write(zchan, ZYNQ_CH_RESET, ZYNQ_CH_RESET_EN);
	zchan_reg_write(zchan, ZYNQ_CH_RESET, 0);
	zynq_trace(ZYNQ_TRACE_CAN_PIO, "%d can%d %s done\n",
	    ZYNQ_INST(zchan), zcan->zcan_ip_num, __FUNCTION__);
	spin_unlock_bh(&zcan->zcan_pio_rx_lock);
	spin_unlock(&zcan->zcan_pio_tx_lock);
	spin_unlock(&zcan->zcan_pio_lock);
}

/*
 * Set CAN IP to loopback mode
 */
static void zcan_pio_loopback_set(zynq_can_t *zcan)
{
	int pio_state;

	spin_lock(&zcan->zcan_pio_lock);
	pio_state = zcan->zcan_pio_state;
	zcan_pio_chip_init(zcan, ZCAN_IP_MSR_LOOPBACK, 1);
	zcan->zcan_pio_loopback = 1;
	if (pio_state == ZYNQ_STATE_START) {
		/* enable chip */
		zcan_reg_write(zcan, ZCAN_IP_SRR, ZCAN_IP_SRR_CEN);
		zcan->zcan_pio_state = ZYNQ_STATE_START;
	}
	zynq_trace(ZYNQ_TRACE_CAN_PIO, "%d can%d %s done\n",
	    ZYNQ_INST(zcan->zchan), zcan->zcan_ip_num, __FUNCTION__);
	spin_unlock(&zcan->zcan_pio_lock);
}

/*
 * Unset CAN IP loopback mode
 */
static void zcan_pio_loopback_unset(zynq_can_t *zcan)
{
	int pio_state;

	spin_lock(&zcan->zcan_pio_lock);
	pio_state = zcan->zcan_pio_state;
	zcan_pio_chip_init(zcan, ZCAN_IP_MSR_NORMAL, 1);
	zcan->zcan_pio_loopback = 0;
	if (pio_state == ZYNQ_STATE_START) {
		/* enable chip */
		zcan_reg_write(zcan, ZCAN_IP_SRR, ZCAN_IP_SRR_CEN);
		zcan->zcan_pio_state = ZYNQ_STATE_START;
	}
	zynq_trace(ZYNQ_TRACE_CAN_PIO, "%d can%d %s done\n",
	    ZYNQ_INST(zcan->zchan), zcan->zcan_ip_num, __FUNCTION__);
	spin_unlock(&zcan->zcan_pio_lock);
}

/*
 * Set CAN IP working baudrate
 */
static int zcan_pio_set_baudrate(zynq_can_t *zcan, int baudrate)
{
	u32 orig_val;

	if (baudrate >= ZYNQ_BAUDRATE_NUM) {
		zynq_err("%d can%d %s failed: bad baudrate %d\n",
		    ZYNQ_INST(zcan->zchan), zcan->zcan_ip_num,
		    __FUNCTION__, baudrate);
		return -EINVAL;
	}
	spin_lock(&zcan->zcan_pio_lock);
	/* save the orignal register value */
	orig_val = zcan_reg_read(zcan, ZCAN_IP_SRR);
	/* clear CEN bit first */
	zcan_reg_write(zcan, ZCAN_IP_SRR, 0);
	mdelay(10);
	/* set the baudrate */
	zcan_reg_write(zcan, ZCAN_IP_BRPR, brpr_reg_val[baudrate]);
	/* restore the orignal register value */
	zcan_reg_write(zcan, ZCAN_IP_SRR, orig_val);
	zcan->zcan_pio_baudrate = baudrate;
	zynq_trace(ZYNQ_TRACE_CAN_PIO, "%d can%d %s done\n",
	    ZYNQ_INST(zcan->zchan), zcan->zcan_ip_num, __FUNCTION__);
	spin_unlock(&zcan->zcan_pio_lock);

	return 0;
}

static void bcan_to_zcan(bcan_msg_t *bmsg)
{
	zcan_msg_t *cmsg;
	u32 msg_id;

	/*
	 * convert CAN message format of bcan_msg_t used by application
	 * to zcan_msg_t used by H/W
	 */
	msg_id = bmsg->bcan_msg_id;
	cmsg = (zcan_msg_t *)bmsg;
	if (msg_id & BCAN_EXTENDED_FRAME) {
		cmsg->cmsg_id_ertr = 0;
		cmsg->cmsg_id_eid = msg_id & CMSG_ID_EID_MASK;
		cmsg->cmsg_id_ide = 1;
		cmsg->cmsg_id_srtr = 1;
		cmsg->cmsg_id_sid = (msg_id >> fls(CMSG_ID_EID_MASK)) &
		    CMSG_ID_SID_MASK;
	} else {
		cmsg->cmsg_id = (msg_id & CMSG_ID_SID_MASK)
		    << CMSG_ID_SID_SHIFT;
	}
	cmsg->cmsg_len = (bmsg->bcan_msg_datalen & CMSG_LEN_MASK)
	    << CMSG_LEN_SHIFT;
	cmsg->cmsg_data1 = htonl(*(u32 *)&bmsg->bcan_msg_data[0]);
	cmsg->cmsg_data2 = htonl(*(u32 *)&bmsg->bcan_msg_data[4]);
}

static void zcan_to_bcan(zcan_msg_t *cmsg)
{
	bcan_msg_t *bmsg;
	u32 msg_id;

	/*
	 * convert CAN message format of zcan_msg_t used by H/W to
	 * bcan_msg_t used by application
	 */
	bmsg = (bcan_msg_t *)cmsg;
	if (cmsg->cmsg_id_ide) {
		msg_id = (cmsg->cmsg_id_sid << fls(CMSG_ID_EID_MASK)) |
		    cmsg->cmsg_id_eid;
		msg_id |= BCAN_EXTENDED_FRAME;
	} else {
		msg_id = cmsg->cmsg_id >> CMSG_ID_SID_SHIFT;
	}
	bmsg->bcan_msg_id = msg_id;
	bmsg->bcan_msg_datalen = cmsg->cmsg_len >> CMSG_LEN_SHIFT;
	*(u32 *)&bmsg->bcan_msg_data[0] = ntohl(cmsg->cmsg_data1);
	*(u32 *)&bmsg->bcan_msg_data[4] = ntohl(cmsg->cmsg_data2);
}

/*
 * High priority Tx: 1 message high priority Tx buffer
 */
static int zcan_pio_tx_hi_one(zynq_can_t *zcan, zcan_msg_t *cmsg,
    int wait, int poll)
{
	zynq_chan_t *zchan = zcan->zchan;
	u32 val32;
	int j = 0;

	spin_lock(&zcan->zcan_pio_tx_hi_lock);
	/* check chip state */
	if (zcan->zcan_pio_state != ZYNQ_STATE_START) {
		zynq_err("%d can%d ch%d %s failed: "
		    "not in ZYNQ_STATE_START state\n",
		    ZYNQ_INST(zchan), zcan->zcan_ip_num,
		    zchan->zchan_num, __FUNCTION__);
		spin_unlock(&zcan->zcan_pio_tx_hi_lock);
		return BCAN_FAIL;
	}
	if (poll) {
		/* clear the Tx interrupt bits first */
		zcan_reg_write(zcan, ZCAN_IP_ICR,
		    ZCAN_IP_INTR_TXOK | ZCAN_IP_INTR_ERROR);
	}

	if (wait) {
		/* poll status regsiter to see if the HPB is FULL */
		while (zcan_reg_read(zcan, ZCAN_IP_PIO_STATUS) &
		    ZCAN_IP_PIO_TX_HI_FULL) {
			if (j++ > ZCAN_PIO_TX_RETRIES) {
				zynq_err("%d can%d ch%d %s failed: "
				    "Tx HI FIFO is FULL\n",
				    ZYNQ_INST(zchan), zcan->zcan_ip_num,
				    zchan->zchan_num, __FUNCTION__);
				spin_unlock(&zcan->zcan_pio_tx_hi_lock);
				return BCAN_FAIL;
			}
			usleep_range(ZCAN_PIO_WAIT_US_MIN, ZCAN_PIO_WAIT_US_MAX);
		}
		zynq_trace(ZYNQ_TRACE_CAN_MSG,
		    "%d can%d ch%d %s: wait_time = %dus\n",
		    ZYNQ_INST(zchan), zcan->zcan_ip_num,
		    zchan->zchan_num, __FUNCTION__, j * 200);
	} else {
		if (zcan_reg_read(zcan, ZCAN_IP_PIO_STATUS) &
		    ZCAN_IP_PIO_TX_HI_FULL) {
			spin_unlock(&zcan->zcan_pio_tx_hi_lock);
			return BCAN_DEV_BUSY;
		}
	}

	/* read out CAN message from registers */
	zcan_reg_write(zcan, ZCAN_IP_TXHPB_ID, cmsg->cmsg_id);
	zcan_reg_write(zcan, ZCAN_IP_TXHPB_DLC, cmsg->cmsg_len);
	zcan_reg_write(zcan, ZCAN_IP_TXHPB_DW1, cmsg->cmsg_data1);
	zcan_reg_write(zcan, ZCAN_IP_TXHPB_DW2, cmsg->cmsg_data2);

	zynq_trace(ZYNQ_TRACE_CAN_MSG,
	    "%d can%d ch%d %s: Tx_high priority msg %lu: "
	    "sid=0x%x, srtr=%d, eid=0x%x, ertr=%d; data_len=%d(0x%x); "
	    "data1=0x%08x; data2=0x%08x\n",
	    ZYNQ_INST(zchan), zcan->zcan_ip_num, zchan->zchan_num, __FUNCTION__,
	    zcan->stats[CAN_STATS_PIO_TX_HI].cnt, cmsg->cmsg_id_sid,
	    cmsg->cmsg_id_srtr, cmsg->cmsg_id_eid, cmsg->cmsg_id_ertr,
	    cmsg->cmsg_len_len, cmsg->cmsg_len, cmsg->cmsg_data1,
	    cmsg->cmsg_data2);

	if (poll) {
		/* poll ISR regsiter to see if the Tx is finished or not */
		j = 0;
		while (1) {
			val32 = zcan_reg_read(zcan, ZCAN_IP_ISR);
			if (val32 & (ZCAN_IP_INTR_TXOK | ZCAN_IP_INTR_ERROR)) {
				break;
			}
			if (j++ > ZCAN_PIO_TX_RETRIES) {
				zynq_err("%d can%d ch%d %s failed: "
				    "Tx status is not updated\n",
				    ZYNQ_INST(zchan), zcan->zcan_ip_num,
				    zchan->zchan_num, __FUNCTION__);
				spin_unlock(&zcan->zcan_pio_tx_hi_lock);
				return BCAN_FAIL;
			}
			usleep_range(ZCAN_PIO_WAIT_US_MIN, ZCAN_PIO_WAIT_US_MAX);
		}
		if (val32 & ZCAN_IP_INTR_ERROR) {
			zynq_trace(ZYNQ_TRACE_CAN_PIO,
			    "%d can%d ch%d %s: Tx high %lu intr=0x%x\n",
			    ZYNQ_INST(zchan), zcan->zcan_ip_num,
			    zchan->zchan_num, __FUNCTION__,
			    zcan->stats[CAN_STATS_PIO_TX_HI].cnt,
			    zcan_reg_read(zcan, ZCAN_IP_ISR));
		}

		/* clear the Tx interrupt bits */
		zcan_reg_write(zcan, ZCAN_IP_ICR,
		    ZCAN_IP_INTR_TXOK | ZCAN_IP_INTR_ERROR);
	}

	ZYNQ_STATS(zcan, CAN_STATS_PIO_TX_HI);
	spin_unlock(&zcan->zcan_pio_tx_hi_lock);

	return (BCAN_OK);
}

/*
 * normal Tx: 64 messages Tx FIFO
 */
static int zcan_pio_tx_one(zynq_can_t *zcan, zcan_msg_t *cmsg,
    int wait, int poll)
{
	zynq_chan_t *zchan = zcan->zchan;
	u32 val32;
	int j = 0;

	spin_lock(&zcan->zcan_pio_tx_lock);
	if (zcan->zcan_pio_state != ZYNQ_STATE_START) {
		zynq_err("%d can%d ch%d %s failed: "
		    "not in ZYNQ_STATE_START state\n",
		    ZYNQ_INST(zchan), zcan->zcan_ip_num,
		    zchan->zchan_num, __FUNCTION__);
		spin_unlock(&zcan->zcan_pio_tx_lock);
		return BCAN_FAIL;
	}

	if (poll) {
		/* clear the Tx interrupt bits first */
		zcan_reg_write(zcan, ZCAN_IP_ICR,
		    ZCAN_IP_INTR_TXOK | ZCAN_IP_INTR_ERROR);
	}

	/* poll status regsiter to see if the Tx FIFO is FULL */
	if (wait) {
		while (zcan_reg_read(zcan, ZCAN_IP_PIO_STATUS) &
		    ZCAN_IP_PIO_TX_FULL) {
			if (j++ > ZCAN_PIO_TX_RETRIES) {
				zynq_err("%d can%d ch%d %s failed: "
				    "Tx FIFO is FULL\n",
				    ZYNQ_INST(zchan), zcan->zcan_ip_num,
				    zchan->zchan_num, __FUNCTION__);
				spin_unlock(&zcan->zcan_pio_tx_lock);
				return BCAN_FAIL;
			}
			usleep_range(ZCAN_PIO_WAIT_US_MIN, ZCAN_PIO_WAIT_US_MAX);
		}
		zynq_trace(ZYNQ_TRACE_CAN_MSG,
		    "%d can%d ch%d %s: wait_time = %dus\n",
		    ZYNQ_INST(zchan), zcan->zcan_ip_num,
		    zchan->zchan_num, __FUNCTION__, j * 200);
	} else {
		if (zcan_reg_read(zcan, ZCAN_IP_PIO_STATUS) &
		    ZCAN_IP_PIO_TX_FULL) {
			spin_unlock(&zcan->zcan_pio_tx_lock);
			return BCAN_DEV_BUSY;
		}
	}

	/* read out CAN message from registers */
	zcan_reg_write(zcan, ZCAN_IP_TXFIFO_ID, cmsg->cmsg_id);
	zcan_reg_write(zcan, ZCAN_IP_TXFIFO_DLC, cmsg->cmsg_len);
	zcan_reg_write(zcan, ZCAN_IP_TXFIFO_DW1, cmsg->cmsg_data1);
	zcan_reg_write(zcan, ZCAN_IP_TXFIFO_DW2, cmsg->cmsg_data2);

	zynq_trace(ZYNQ_TRACE_CAN_MSG,
	    "%d can%d ch%d %s: Tx_normal priority msg %lu: "
	    "sid=0x%x, srtr=%d, eid=0x%x, ertr=%d; data_len=%d(0x%x); "
	    "data1=0x%08x; data2=0x%08x(%d)\n",
	    ZYNQ_INST(zchan), zcan->zcan_ip_num, zchan->zchan_num, __FUNCTION__,
	    zcan->stats[CAN_STATS_PIO_TX].cnt, cmsg->cmsg_id_sid,
	    cmsg->cmsg_id_srtr, cmsg->cmsg_id_eid, cmsg->cmsg_id_ertr,
	    cmsg->cmsg_len_len, cmsg->cmsg_len, cmsg->cmsg_data1,
	    cmsg->cmsg_data2, cmsg->cmsg_data2);

	if (poll) {
		/* poll ISR regsiter to see if the Tx is finished or not */
		j = 0;
		while (1) {
			val32 = zcan_reg_read(zcan, ZCAN_IP_ISR);
			if (val32 & (ZCAN_IP_INTR_TXOK | ZCAN_IP_INTR_ERROR)) {
				break;
			}
			if (j++ > ZCAN_PIO_TX_RETRIES) {
				zynq_err("%d can%d ch%d %s failed: "
				    "Tx status is not updated\n",
				    ZYNQ_INST(zchan), zcan->zcan_ip_num,
				    zchan->zchan_num, __FUNCTION__);
				spin_unlock(&zcan->zcan_pio_tx_lock);
				return BCAN_FAIL;
			}
			usleep_range(ZCAN_PIO_WAIT_US_MIN, ZCAN_PIO_WAIT_US_MAX);
		}
		if (val32 & ZCAN_IP_INTR_ERROR) {
			zynq_trace(ZYNQ_TRACE_CAN_PIO,
			    "%d can%d ch%d %s: Tx normal %lu intr=0x%x\n",
			    ZYNQ_INST(zchan), zcan->zcan_ip_num,
			    zchan->zchan_num, __FUNCTION__,
			    zcan->stats[CAN_STATS_PIO_TX].cnt,
			    zcan_reg_read(zcan, ZCAN_IP_ISR));
		}

		/* clear the Tx interrupt bits */
		zcan_reg_write(zcan, ZCAN_IP_ICR,
		    ZCAN_IP_INTR_TXOK | ZCAN_IP_INTR_ERROR);
	}
	ZYNQ_STATS(zcan, CAN_STATS_PIO_TX);
	spin_unlock(&zcan->zcan_pio_tx_lock);

	return BCAN_OK;
}

/*
 * normal DMA mode Tx
 */
static int zcan_dma_tx_one(zynq_can_t *zcan, zcan_msg_t *cmsg,
    int wait, int poll)
{
	return zchan_tx_one_msg(zcan->zchan, cmsg, sizeof(zcan_msg_t));
}

/*
 * Tx userland CAN message(s)
 */
static int zcan_tx_user(zynq_can_t *zcan, ioc_bcan_msg_t *ioc_arg,
    int tx_hipri, int *tx_done)
{
	zynq_chan_t *zchan = zcan->zchan;
	int (*tx_func)(zynq_can_t *, zcan_msg_t *, int, int);
	int tx_num = ioc_arg->ioc_msg_num;
	int tx_num_done = 0;
	bcan_msg_t bmsg_one;
	bcan_msg_t *bmsg, *bmsg_alloc = NULL;
	zcan_msg_t *cmsg;
	ktime_t ktime_end, ktime_now;
	int ret = BCAN_OK;

	zynq_trace(ZYNQ_TRACE_CAN_MSG,
	    "%d can%d ch%d %s enter: tx_timeout=%ld, tx_num=%d\n",
	    ZYNQ_INST(zchan), zcan->zcan_ip_num, zchan->zchan_num, __FUNCTION__,
	    zcan->zcan_tx_timeout, tx_num);
	if (tx_num == 1) {
		bmsg = &bmsg_one;
	} else {
		/* alloc memory and copy-in all the sending msgs */
		bmsg_alloc = kmalloc(sizeof(bcan_msg_t) * tx_num, GFP_KERNEL);
		if (bmsg_alloc == NULL) {
			ret = BCAN_FAIL;
			zynq_err("%d can%d ch%d %s kmalloc failed.\n",
			    ZYNQ_INST(zchan), zcan->zcan_ip_num,
			    zchan->zchan_num, __FUNCTION__);
			goto tx_done;
		}
		bmsg = bmsg_alloc;
	}

	if (copy_from_user(bmsg, (void __user *)ioc_arg->ioc_msgs,
	    sizeof(bcan_msg_t) * tx_num)) {
		zynq_err("%d can%d ch%d %s "
		    "copy_from_usr failed: usr_addr=0x%p.\n",
		    ZYNQ_INST(zchan), zcan->zcan_ip_num,
		    zchan->zchan_num, __FUNCTION__,
		    ioc_arg->ioc_msgs);
		ret = BCAN_FAIL;
		goto tx_done;
	}
	/* if timeout is not set, set to MAX */
	if (zcan->zcan_tx_timeout == 0) {
		zcan->zcan_tx_timeout = ZCAN_TIMEOUT_MAX;
	}
	if (tx_hipri) {
		tx_func = zcan_pio_tx_hi_one;
	} else if (!zcan->zdev->zcan_tx_dma) {
		tx_func = zcan_pio_tx_one;
	} else {
		tx_func = zcan_dma_tx_one;
	}
	ktime_end = ktime_add_ns(ktime_get(),
	    1000000 * zcan->zcan_tx_timeout);
	/* send the msg one by one till finished or timeout */
	while (tx_num) {
		zynq_trace(ZYNQ_TRACE_CAN_MSG, "%d can%d ch%d %s bcan_msg %d: "
		    "id=0x%08x; data_len=%d; data1=0x%08x; data2=0x%08x\n",
		    ZYNQ_INST(zchan), zcan->zcan_ip_num,
		    zchan->zchan_num, __FUNCTION__, tx_num_done,
		    bmsg->bcan_msg_id, bmsg->bcan_msg_datalen,
		    *(u32 *)(&bmsg->bcan_msg_data[0]),
		    *(u32 *)(&bmsg->bcan_msg_data[4]));

		bcan_to_zcan(bmsg);

		cmsg = (zcan_msg_t *)bmsg;
		zynq_trace(ZYNQ_TRACE_CAN_MSG, "%d can%d ch%d %s zcan_msg %d: "
		    "sid=0x%x, srtr=%d, ide=%d, eid=0x%x, ertr=%d(0x%08x); "
		    "data_len=%d(0x%08x); data1=0x%08x; data2=0x%08x\n",
		    ZYNQ_INST(zchan), zcan->zcan_ip_num,
		    zchan->zchan_num, __FUNCTION__, tx_num_done,
		    cmsg->cmsg_id_sid, cmsg->cmsg_id_srtr, cmsg->cmsg_id_ide,
		    cmsg->cmsg_id_eid, cmsg->cmsg_id_ertr, cmsg->cmsg_id,
		    cmsg->cmsg_len_len, cmsg->cmsg_len,
		    cmsg->cmsg_data1, cmsg->cmsg_data2);
retry:
		/* call low-level tx function to transmit the msg */
		ret = tx_func(zcan, cmsg, 0, 0);
		if (ret == BCAN_OK) {
			/* wait for h/w to really transmit the MSG on-wire */
			usleep_range(ZCAN_PIO_WAIT_US_MIN, ZCAN_PIO_WAIT_US_MAX);
		} else {
			/* checking timeout */
			if (ret == BCAN_DEV_BUSY) {
				ktime_now = ktime_get();
				if (ktime_now.tv64 < ktime_end.tv64) {
					usleep_range(ZCAN_PIO_WAIT_US_MIN,
					    ZCAN_PIO_WAIT_US_MAX);
					goto retry;
				} else {
					ret = BCAN_TIMEOUT;
					if ((zynq_trace_param & ZYNQ_TRACE_CAN)
					    || zynq_dbg_reg_dump_param) {
						zcan_pio_dbg_reg_dump_ch(zcan);
					}
				}
			}
			break;
		}
		tx_num_done++;
		bmsg++;
		tx_num--;
	}
tx_done:
	*tx_done = tx_num_done;
	if (bmsg_alloc) {
		kfree(bmsg_alloc);
	}
	zynq_trace(ZYNQ_TRACE_CAN_MSG,
	    "%d can%d ch%d %s done: ret=%d, tx_num=%d, tx_num_done=%d\n",
	    ZYNQ_INST(zchan), zcan->zcan_ip_num, zchan->zchan_num, __FUNCTION__,
	    ret, ioc_arg->ioc_msg_num, tx_num_done);

	return ret;
}

#if !defined(PIO_RX_THREAD) && !defined(PIO_RX_INTR)
/*
 * Rx: get the messages from Rx FIFO and put them into the
 * given buffer. Return the length of actual copied data size.
 */
static int zcan_pio_rx(zynq_can_t *zcan, char *buf, int buf_sz, int wait)
{
	zynq_chan_t *zchan = zcan->zchan;
	u32 *rx_buf = (u32 *)buf;
	zcan_msg_t *cmsg = (zcan_msg_t *)buf;
	int rx_len = 0;
	int j = 0;

	if (wait) {
		while (!(zcan_reg_read(zcan, ZCAN_IP_PIO_STATUS) &
		    ZCAN_IP_PIO_RX_READY)) {
			if (j++ > ZCAN_PIO_RX_RETRIES) {
				zynq_err("%d can%d ch%d %s failed: "
				    "no Rx data\n",
				    ZYNQ_INST(zchan), zcan->zcan_ip_num,
				    zchan->zchan_num, __FUNCTION__);
				return (0);
			}
			udelay(ZCAN_PIO_WAIT_DELAY);
		}
	}

	/* check if there is data in Rx FIFO */
	while (zcan_reg_read(zcan, ZCAN_IP_PIO_STATUS) &
	    ZCAN_IP_PIO_RX_READY) {
		if (rx_len > buf_sz) {
			break;
		}

		/* read out one message */
		*rx_buf++ = zcan_reg_read(zcan, ZCAN_IP_RXFIFO_ID);
		*rx_buf++ = zcan_reg_read(zcan, ZCAN_IP_RXFIFO_DLC);
		*rx_buf++ = zcan_reg_read(zcan, ZCAN_IP_RXFIFO_DW1);
		*rx_buf++ = zcan_reg_read(zcan, ZCAN_IP_RXFIFO_DW2);
		rx_len +=  sizeof(zcan_msg_t);
		buf_sz -= sizeof(zcan_msg_t);

		/* clear the Rx interrupt bits */
		zcan_reg_write(zcan, ZCAN_IP_PIO_STATUS, ZCAN_IP_PIO_RX_READY);

		zynq_trace(ZYNQ_TRACE_CAN_MSG, "%d can%d ch%d %s: Rx msg %lu: "
		    "sid=0x%x, srtr=%d, eid=0x%x, ertr=%d; "
		    "data_len=%d(0x%x); data1=0x%x; data2=0x%x\n",
		    ZYNQ_INST(zchan), zcan->zcan_ip_num,
		    zchan->zchan_num, __FUNCTION__,
		    zcan->stats[CAN_STATS_PIO_RX].cnt, cmsg->cmsg_id_sid,
		    cmsg->cmsg_id_srtr, cmsg->cmsg_id_eid, cmsg->cmsg_id_ertr,
		    cmsg->cmsg_len_len, cmsg->cmsg_len, cmsg->cmsg_data1,
		    cmsg->cmsg_data2);
		ZYNQ_STATS(zcan, CAN_STATS_PIO_RX);
		cmsg++;
	}

	zynq_trace(ZYNQ_TRACE_CAN_MSG, "%d can%d ch%d %s done: rx_len = %d.\n",
	    ZYNQ_INST(zchan), zcan->zcan_ip_num, zchan->zchan_num,
	    __FUNCTION__, rx_len);
	return rx_len;
}
#endif

/* check if we have received enough data as requested */
static inline void zcan_usr_rx_complete(zynq_can_t *zcan)
{
	u32 req_num, rx_num;

	spin_lock(&zcan->zcan_pio_rx_lock);
	if (zcan->zcan_usr_buf == NULL) {
		spin_unlock(&zcan->zcan_pio_rx_lock);
		return;
	}
	/* requested Rx number */
	req_num = zcan->zcan_usr_buf_num - zcan->zcan_usr_rx_num;
	/* already received number */
	if (zcan->zcan_buf_rx_tail >= zcan->zcan_buf_rx_head) {
		rx_num = zcan->zcan_buf_rx_tail - zcan->zcan_buf_rx_head;
	} else {
		rx_num = zcan->zcan_buf_rx_num + zcan->zcan_buf_rx_tail -
		    zcan->zcan_buf_rx_head;
	}

	zcan->zcan_pio_rx_last_chk_head = zcan->zcan_buf_rx_head;
	zcan->zcan_pio_rx_last_chk_tail = zcan->zcan_buf_rx_tail;
	zcan->zcan_pio_rx_last_chk_cnt = rx_num;
	zcan->zcan_pio_rx_last_chk_seq = zcan->zcan_usr_rx_seq;

	/* received enough */
	if ((rx_num > 0) && (rx_num >= req_num)) {
		complete(&zcan->zcan_usr_rx_comp);
	}
	spin_unlock(&zcan->zcan_pio_rx_lock);
}

/* copy the received CAN message(s) to usrland buffer */
static int zcan_copy_to_user(zynq_can_t *zcan)
{
	zynq_chan_t *zchan = zcan->zchan;
	bcan_msg_t *buf;
	bcan_msg_t *bmsg;
	zcan_msg_t *cmsg;
	int head;

	zynq_trace(ZYNQ_TRACE_CAN_MSG, "%d can%d ch%d %s enter: buf_num=%d\n",
	    ZYNQ_INST(zchan), zcan->zcan_ip_num, zchan->zchan_num, __FUNCTION__,
	    zcan->zcan_usr_buf_num);
	WARN_ON(!spin_is_locked(&zcan->zcan_pio_rx_lock));

	if (zcan->zcan_usr_rx_num == zcan->zcan_usr_buf_num) {
		goto done;
	}

	/* no pending Rx data */
	if (zcan->zcan_buf_rx_head == zcan->zcan_buf_rx_tail) {
		goto done;
	}

	/* get buffered CAN messages to user */
	for (head = zcan->zcan_buf_rx_head; head != zcan->zcan_buf_rx_tail;
	    head++, head %= zcan->zcan_buf_rx_num) {
		if (zcan->zcan_usr_rx_num == zcan->zcan_usr_buf_num) {
			break;
		}
		bmsg = zcan->zcan_buf_rx_msg + head;
		cmsg = (zcan_msg_t *)bmsg;
		buf = (bcan_msg_t *)zcan->zcan_usr_buf +
		    zcan->zcan_usr_rx_num;

		zynq_trace(ZYNQ_TRACE_CAN_MSG, "%d can%d ch%d %s zcan_msg %d: "
		    "sid=0x%x, srtr=%d, ide=%d, eid=0x%x, ertr=%d(0x%08x); "
		    "data_len=%d(0x%08x); data1=0x%08x; data2=0x%08x\n",
		    ZYNQ_INST(zchan), zcan->zcan_ip_num,
		    zchan->zchan_num, __FUNCTION__, zcan->zcan_usr_rx_num,
		    cmsg->cmsg_id_sid, cmsg->cmsg_id_srtr, cmsg->cmsg_id_ide,
		    cmsg->cmsg_id_eid, cmsg->cmsg_id_ertr, cmsg->cmsg_id,
		    cmsg->cmsg_len_len, cmsg->cmsg_len,
		    cmsg->cmsg_data1, cmsg->cmsg_data2);

		zcan_to_bcan(cmsg);

		zynq_trace(ZYNQ_TRACE_CAN_MSG, "%d can%d ch%d %s bcan_msg %d: "
		    "id=0x%08x, data_len=%d, data1=0x%08x, data2=0x%08x, "
		    "timestamp=%lu.%06lu, head=%d, usr_buf=0x%p\n",
		    ZYNQ_INST(zchan), zcan->zcan_ip_num,
		    zchan->zchan_num, __FUNCTION__, zcan->zcan_usr_rx_num,
		    bmsg->bcan_msg_id, bmsg->bcan_msg_datalen,
		    *(u32 *)bmsg->bcan_msg_data,
		    *(u32 *)(&bmsg->bcan_msg_data[4]),
		    bmsg->bcan_msg_timestamp.tv_sec,
		    bmsg->bcan_msg_timestamp.tv_usec,
		    head, buf);

		if (copy_to_user((void __user *)buf,
		    bmsg, sizeof(bcan_msg_t))) {
			zcan->zcan_buf_rx_head = head;
			zynq_err("%d can%d ch%d %s: copy_to_user() failed, "
			    "head=%d, tail=%d, usr_buf=0x%p\n",
			    ZYNQ_INST(zchan), zcan->zcan_ip_num,
			    zchan->zchan_num,__FUNCTION__,
			    zcan->zcan_buf_rx_head, zcan->zcan_buf_rx_tail,
			    buf);
			return BCAN_FAIL;
		}

		zcan->zcan_usr_rx_num++;
	}
	zcan->zcan_buf_rx_head = head;

done:
	if (zcan->zcan_usr_rx_num < zcan->zcan_usr_buf_num) {
		zynq_trace(ZYNQ_TRACE_CAN_MSG,
		    "%d can%d ch%d %s: not enough, "
		    "rx_num=%d, buf_num=%d, head=%d, tail=%d, "
		    "check:[head=%d,tail=%d,cnt=%d,seq=%d], "
		    "last-seq=%d, intr-total=%lu, intr-rx=%lu\n",
		    ZYNQ_INST(zchan), zcan->zcan_ip_num,
		    zchan->zchan_num, __FUNCTION__,
		    zcan->zcan_usr_rx_num, zcan->zcan_usr_buf_num,
		    zcan->zcan_buf_rx_head, zcan->zcan_buf_rx_tail,
		    zcan->zcan_pio_rx_last_chk_head,
		    zcan->zcan_pio_rx_last_chk_tail,
		    zcan->zcan_pio_rx_last_chk_cnt,
		    zcan->zcan_pio_rx_last_chk_seq,
		    zcan->zcan_usr_rx_seq,
		    zchan->zdev->stats[DEV_STATS_INTR].cnt,
		    zchan->stats[CHAN_STATS_RX_INTR].cnt);
		return BCAN_PARTIAL_OK;
	}

	zynq_trace(ZYNQ_TRACE_CAN_MSG,
	    "%d can%d ch%d %s done: head=%d, tail=%d, "
	    "buf_num=%d, rx_num=%d\n", ZYNQ_INST(zchan),
	    zcan->zcan_ip_num, zchan->zchan_num, __FUNCTION__,
	    zcan->zcan_buf_rx_head, zcan->zcan_buf_rx_tail,
	    zcan->zcan_usr_buf_num, zcan->zcan_usr_rx_num);

	return BCAN_OK;
}

/*
 * Rx CAN messages to user
 */
static int zcan_rx_user(zynq_can_t *zcan, ioc_bcan_msg_t *ioc_arg,
    int *rx_done)
{
	zynq_chan_t *zchan = zcan->zchan;
	int ret;
	long timeout;

	zynq_trace(ZYNQ_TRACE_CAN_MSG,
	    "%d can%d ch%d %s enter: timeout=%ld, buf_num=%d\n",
	    ZYNQ_INST(zchan), zcan->zcan_ip_num, zchan->zchan_num, __FUNCTION__,
	    zcan->zcan_rx_timeout, ioc_arg->ioc_msg_num);
	*rx_done = 0;
	spin_lock_bh(&zcan->zcan_pio_rx_lock);
	zcan->zcan_usr_rx_seq++;
	if (zcan->zcan_rx_timeout == 0) {
		zcan->zcan_rx_timeout = ZCAN_TIMEOUT_MAX;
	}
	if (zcan->zcan_usr_buf != NULL) {
		zynq_err("%d can%d ch%d %s: BUSY!\n", ZYNQ_INST(zchan),
		    zcan->zcan_ip_num, zchan->zchan_num, __FUNCTION__);
		ret = BCAN_DEV_BUSY;
		goto out;
	}
	zcan->zcan_usr_buf = ioc_arg->ioc_msgs;
	zcan->zcan_usr_buf_num = ioc_arg->ioc_msg_num;
	/* copy existing data to usrland first */
	ret = zcan_copy_to_user(zcan);
	if (ret == BCAN_OK) {
		goto done; /* received enough */
	}

	reinit_completion(&zcan->zcan_usr_rx_comp);
	spin_unlock_bh(&zcan->zcan_pio_rx_lock);

	/* wait for more data */
	ZYNQ_STATS(zcan, CAN_STATS_USR_RX_WAIT);
	timeout = wait_for_completion_interruptible_timeout(
	    &zcan->zcan_usr_rx_comp,
	    msecs_to_jiffies(zcan->zcan_rx_timeout));
	if (timeout < 0) {  /* interrupted */
		ZYNQ_STATS(zcan, CAN_STATS_USR_RX_WAIT_INT);
		ret = BCAN_ERR;
		/* Lock RX and clean-up. */
		spin_lock_bh(&zcan->zcan_pio_rx_lock);
		goto done;
	}
	if (timeout == 0) {
		ZYNQ_STATS(zcan, CAN_STATS_USR_RX_TIMEOUT);
	}

	spin_lock_bh(&zcan->zcan_pio_rx_lock);
	/* copy newly received data */
	ret = zcan_copy_to_user(zcan);
	if (ret == BCAN_PARTIAL_OK) {
		ZYNQ_STATS(zcan, CAN_STATS_USR_RX_PARTIAL);

		if (timeout > 0) {
			/* This should never happen when timeout != 0. */
			ret = BCAN_ERR; /* internal or unknown error. */
			zynq_err("%s: %d can%d ch%d: "
			    "internal error, #m=%d:%d, timeout=%ld\n",
			    __FUNCTION__, ZYNQ_INST(zchan), zcan->zcan_ip_num,
			    zchan->zchan_num, zcan->zcan_usr_rx_num,
			    zcan->zcan_usr_buf_num, timeout);
		} else {
			ret = BCAN_TIMEOUT;
		}
	}
done:
	*rx_done = zcan->zcan_usr_rx_num;
	zcan->zcan_usr_rx_num = 0;
	zcan->zcan_usr_buf_num = 0;
	zcan->zcan_usr_buf = NULL;
out:
	spin_unlock_bh(&zcan->zcan_pio_rx_lock);

	zynq_trace(ZYNQ_TRACE_CAN_MSG, "%d can%d ch%d %s done: rx_done=%d\n",
	    ZYNQ_INST(zchan), zcan->zcan_ip_num, zchan->zchan_num,
	    __FUNCTION__, *rx_done);

	return ret;
}

/* clear all the buffered messaged */
static inline void zcan_clear_buf_rx(zynq_can_t *zcan)
{
	zynq_chan_t *zchan = zcan->zchan;

	spin_lock_bh(&zcan->zcan_pio_rx_lock);
	zcan->zcan_buf_rx_head = zcan->zcan_buf_rx_tail;
	zynq_trace(ZYNQ_TRACE_CAN,
	    "%d can%d ch%d %s: clear the buffered rx messages, head=tail=%d\n",
	    ZYNQ_INST(zchan), zcan->zcan_ip_num, zchan->zchan_num,
	    __FUNCTION__, zcan->zcan_buf_rx_tail);
	spin_unlock_bh(&zcan->zcan_pio_rx_lock);
}

static inline void zcan_buf_inc_rx_head(zynq_can_t *zcan, int inc)
{
	zynq_chan_t *zchan = zcan->zchan;
	int head;

	spin_lock(&zcan->zcan_pio_rx_lock);
	head = (zcan->zcan_buf_rx_head + inc) % zcan->zcan_buf_rx_num;
	zynq_trace(ZYNQ_TRACE_CAN,
	    "%d can%d ch%d %s: head=%d, inc=%d, new_head=%d, tail=%d\n",
	    ZYNQ_INST(zchan), zcan->zcan_ip_num, zchan->zchan_num, __FUNCTION__,
	    zcan->zcan_buf_rx_head, inc, head, zcan->zcan_buf_rx_tail);
	zcan->zcan_buf_rx_head = head;
	spin_unlock(&zcan->zcan_pio_rx_lock);

	ZYNQ_STATS_LOGX(zchan, CHAN_STATS_RX_DROP, inc, 0);
}

static inline int zcan_buf_rx_full(zynq_can_t *zcan)
{
	int rx_tail_next;

	rx_tail_next = (zcan->zcan_buf_rx_tail + 1) % zcan->zcan_buf_rx_num;

	return rx_tail_next == zcan->zcan_buf_rx_head;
}

#ifdef PIO_RX_THREAD
static int zcan_pio_rx_poll(void *arg)
{
	zynq_can_t *zcan = arg;
	zynq_chan_t *zchan = zcan->zchan;
	bcan_msg_t *bmsg;
	zcan_msg_t *cmsg;
	int rx_tail;
	int status;

	rx_tail = zcan->zcan_buf_rx_tail;
	while (!kthread_should_stop()) {
		/* wait for data to be available */
		while (1) {
			/* Rx data pending */
			status = zcan_reg_read(zcan, ZCAN_IP_PIO_STATUS);
			if (status != -1 && status & ZCAN_IP_PIO_RX_READY) {
				break;
			}

			/* sleep and will poll status again */
			usleep_range(1000, 1200);

			if (kthread_should_stop()) {
				goto done;
			}
		}

		/* check if there is data in Rx FIFO */
		while (zcan_reg_read(zcan, ZCAN_IP_PIO_STATUS) &
		    ZCAN_IP_PIO_RX_READY) {
			/* if full, free some buffer by increasing the rx_head */
			if (zcan_buf_rx_full(zcan)) {
				zcan_buf_inc_rx_head(zcan, ZCAN_IP_RXFIFO_MAX);
			}

			/* read out one message */
			bmsg = zcan->zcan_buf_rx_msg + rx_tail;
			/* record the timestamp for the msg */
			ktime_get_real_ts((struct timespec *)
			    (&bmsg->bcan_msg_timestamp));
			bmsg->bcan_msg_timestamp.tv_usec /= NSEC_PER_USEC;

			/* CAN msg info */
			cmsg = (zcan_msg_t *)bmsg;
			cmsg->cmsg_id = zcan_reg_read(zcan, ZCAN_IP_RXFIFO_ID);
			cmsg->cmsg_len = zcan_reg_read(zcan, ZCAN_IP_RXFIFO_DLC);
			cmsg->cmsg_data1 = zcan_reg_read(zcan, ZCAN_IP_RXFIFO_DW1);
			cmsg->cmsg_data2 = zcan_reg_read(zcan, ZCAN_IP_RXFIFO_DW2);

			/* clear the Rx interrupt bits */
			zcan_reg_write(zcan, ZCAN_IP_PIO_STATUS, ZCAN_IP_PIO_RX_READY);

			zynq_trace(ZYNQ_TRACE_CAN_MSG,
			    "%d can%d ch%d %s: Rx %d msg %lu: "
			    "sid=0x%x, srtr=%d, eid=0x%x, ertr=%d; "
			    "data_len=%d(0x%x); data1=0x%x; data2=0x%x(%d)\n",
			    ZYNQ_INST(zchan), zcan->zcan_ip_num,
			    zchan->zchan_num, __FUNCTION__,
			    rx_tail, zcan->stats[CAN_STATS_PIO_RX].cnt,
			    cmsg->cmsg_id_sid,
			    cmsg->cmsg_id_srtr, cmsg->cmsg_id_eid,
			    cmsg->cmsg_id_ertr, cmsg->cmsg_len_len,
			    cmsg->cmsg_len, cmsg->cmsg_data1,
			    cmsg->cmsg_data2, cmsg->cmsg_data2);

			/* NEXT tail */
			rx_tail++;
			rx_tail %= zcan->zcan_buf_rx_num;
			zcan->zcan_buf_rx_tail = rx_tail;
			ZYNQ_STATS(zcan, CAN_STATS_PIO_RX);

			/* notify the receiver */
			zcan_usr_rx_complete(zcan);

			if (kthread_should_stop()) {
				break;
			}
		}
	}
done:
	zynq_trace(ZYNQ_TRACE_CAN_PIO, "%d can%d ch%d %s done.\n",
	    ZYNQ_INST(zchan), zcan->zcan_ip_num, zchan->zchan_num,
	    __FUNCTION__);

	return 0;
}
#endif

/*
 * CAN message rx handling in PIO mode: called in interrupt context
 *     Get the received CAN messages from H/W to a CAN message buffer ring.
 */
static void zcan_pio_rx_proc(zynq_can_t *zcan)
{
	zynq_chan_t *zchan = zcan->zchan;
	bcan_msg_t *bmsg;
	zcan_msg_t *cmsg;
	int rx_tail;
	int status;

	rx_tail = zcan->zcan_buf_rx_tail;
	/* check if there is data in Rx FIFO */
	while (1) {
		status = zcan_reg_read(zcan, ZCAN_IP_PIO_STATUS);
		if (status == -1 || !(status & ZCAN_IP_PIO_RX_READY)) {
			break; /* No data pending */
		}

		/* if full, free some buffer by increasing the rx_head */
		if (zcan_buf_rx_full(zcan)) {
			zcan_buf_inc_rx_head(zcan, ZCAN_IP_RXFIFO_MAX);
		}

		/* read out one message */
		bmsg = zcan->zcan_buf_rx_msg + rx_tail;
		/* record the timestamp for the msg */
		ktime_get_real_ts((struct timespec *)
		    (&bmsg->bcan_msg_timestamp));
		bmsg->bcan_msg_timestamp.tv_usec /= NSEC_PER_USEC;
		/* CAN msg info */
		cmsg = (zcan_msg_t *)bmsg;
		cmsg->cmsg_id = zcan_reg_read(zcan, ZCAN_IP_RXFIFO_ID);
		cmsg->cmsg_len = zcan_reg_read(zcan, ZCAN_IP_RXFIFO_DLC);
		cmsg->cmsg_data1 = zcan_reg_read(zcan, ZCAN_IP_RXFIFO_DW1);
		cmsg->cmsg_data2 = zcan_reg_read(zcan, ZCAN_IP_RXFIFO_DW2);

		/* clear the Rx interrupt bits */
		zcan_reg_write(zcan, ZCAN_IP_PIO_STATUS, ZCAN_IP_PIO_RX_READY);

		zynq_trace(ZYNQ_TRACE_CAN_MSG,
		    "%d can%d ch%d %s: Rx %d msg %lu: "
		    "sid=0x%x, srtr=%d, eid=0x%x, ertr=%d; "
		    "data_len=%d(0x%x); data1=0x%x; data2=0x%x(%d)\n",
		    ZYNQ_INST(zchan), zcan->zcan_ip_num,
		    zchan->zchan_num, __FUNCTION__, rx_tail,
		    zcan->stats[CAN_STATS_PIO_RX].cnt, cmsg->cmsg_id_sid,
		    cmsg->cmsg_id_srtr, cmsg->cmsg_id_eid,
		    cmsg->cmsg_id_ertr, cmsg->cmsg_len_len,
		    cmsg->cmsg_len, cmsg->cmsg_data1,
		    cmsg->cmsg_data2, cmsg->cmsg_data2);

		/* NEXT tail */
		rx_tail++;
		rx_tail %= zcan->zcan_buf_rx_num;
		zcan->zcan_buf_rx_tail = rx_tail;
		ZYNQ_STATS(zcan, CAN_STATS_PIO_RX);

		/* notify the receiver */
		zcan_usr_rx_complete(zcan);
	}
}

/*
 * CAN message rx handling in DMA mode: called in interrupt context
 *     Get the received CAN messages from H/W to a CAN message buffer ring.
 */
static void zcan_dma_rx_proc(zynq_can_t *zcan)
{
	zynq_chan_t *zchan = zcan->zchan;
	bcan_msg_t *bmsg;
	zcan_msg_t *cmsg;
	int pio_rx_tail;
	zchan_rx_tbl_t *zchan_rx = &zchan->zchan_rx_tbl;
	u32 rx_head, rx_tail, rx_off;
	void *msg = NULL;
	u32 msgsz = zcan->zdev->zcan_rx_hw_ts ? sizeof(bcan_msg_t) :
	    sizeof(zcan_msg_t);

	rx_tail = zchan_reg_read(zchan, ZYNQ_CH_RX_TAIL);
	/*
	 * The tail pointer can be pointing to the middle of a CAN message,
	 * we must align it to the beginning boundary of the CAN message.
	 */
	rx_tail &= ~(msgsz - 1);
	rx_head = zchan_rx->zchan_rx_head;
	if (rx_head >= zchan_rx->zchan_rx_size ||
	    rx_tail >= zchan_rx->zchan_rx_size) {
		zynq_err("%d can%d ch%d %s check failed: "
		    "rx_head=0x%x, rx_tail=0x%x, max_off=0x%x\n",
		    ZYNQ_INST(zchan), zcan->zcan_ip_num, zchan->zchan_num,
		    __FUNCTION__, rx_head, rx_tail, zchan_rx->zchan_rx_size - 1);
		return;
	}
	zynq_trace(ZYNQ_TRACE_CAN,
	    "%d can%d ch%d %s: rx_head=0x%x, rx_tail=0x%x\n",
	    ZYNQ_INST(zchan), zcan->zcan_ip_num, zchan->zchan_num, __FUNCTION__,
	    rx_head, rx_tail);

	zchan_rx->zchan_rx_tail = rx_tail;
	pio_rx_tail = zcan->zcan_buf_rx_tail;
	for (rx_off = rx_head; rx_off != rx_tail; rx_off += msgsz,
	    rx_off &= (zchan_rx->zchan_rx_size - 1)) {
		if (rx_off == rx_head ||
		    ZCHAN_RX_BUF_OFFSET(zchan_rx, rx_off) == 0) {
			/*
			 * first loop or get to a new buffer: need to retrieve
			 * the new buffer base.
			 */
			zchan_rx_off2addr(zchan_rx, rx_off, &msg, NULL);
		} else {
			/* within the same buffer: increment the msg address */
			msg = (char *)msg + msgsz;
		}
		/* if full, free some buffer by increasing the rx_head */
		if (zcan_buf_rx_full(zcan)) {
			zcan_buf_inc_rx_head(zcan, ZCAN_IP_RXFIFO_MAX);
		}
		/* copy out one CAN message */
		bmsg = zcan->zcan_buf_rx_msg + pio_rx_tail;
		memcpy(bmsg, msg, msgsz);
		if (!zcan->zdev->zcan_rx_hw_ts || zynq_disable_can_hw_ts) {
			/* do S/W timestamp for the msg */
			ktime_get_real_ts((struct timespec *)
			    (&bmsg->bcan_msg_timestamp));
			bmsg->bcan_msg_timestamp.tv_usec /= NSEC_PER_USEC;
		}
		/* CAN msg info */
		cmsg = (zcan_msg_t *)msg;
		zynq_trace(ZYNQ_TRACE_CAN_MSG,
		    "%d can%d ch%d %s: Rx %d msg %lu "
		    "rx_off=0x%x: sid=0x%x, srtr=%d, eid=0x%x, ertr=%d; "
		    "data_len=%d(0x%x); data1=0x%x; data2=0x%x(%d)\n",
		    ZYNQ_INST(zchan), zcan->zcan_ip_num,
		    zchan->zchan_num, __FUNCTION__,
		    pio_rx_tail, zchan->stats[CHAN_STATS_RX].cnt, rx_off,
		    cmsg->cmsg_id_sid, cmsg->cmsg_id_srtr, cmsg->cmsg_id_eid,
		    cmsg->cmsg_id_ertr, cmsg->cmsg_len_len, cmsg->cmsg_len,
		    cmsg->cmsg_data1, cmsg->cmsg_data2, cmsg->cmsg_data2);

		/* NEXT tail */
		pio_rx_tail++;
		pio_rx_tail %= zcan->zcan_buf_rx_num;
		zcan->zcan_buf_rx_tail = pio_rx_tail;
		ZYNQ_STATS(zchan, CHAN_STATS_RX);

		/* notify the receiver */
		zcan_usr_rx_complete(zcan);
	}
	/* update h/w head index */
	zchan_reg_write(zchan, ZYNQ_CH_RX_HEAD, rx_off);
	zchan_rx->zchan_rx_head = rx_off;
}

void zcan_rx_proc(zynq_can_t *zcan)
{
	if (zcan->zdev->zcan_rx_dma) {
		zcan_dma_rx_proc(zcan);
	} else {
		zcan_pio_rx_proc(zcan);
	}
}

/* CAN Rx/Tx test function in DMA mode */
void zcan_test(zynq_can_t *zcan, int loopback, int msgcnt)
{
	zcan_msg_t tx_cmsg;
	int i, retry_count = 10;

	/* init and start the CAN IP */
	zcan_pio_reset(zcan);
	zcan_pio_start(zcan);
	if (loopback) {
		zcan_pio_loopback_set(zcan);
	}

	tx_cmsg.cmsg_id = 0x05000000;
	tx_cmsg.cmsg_len = 0x80000000;
	zynq_err("%d can%d %s: Tx test Start!!!\n",
	    ZYNQ_INST(zcan->zchan), zcan->zcan_ip_num, __FUNCTION__);
	for (i = 1; i <= msgcnt; i++) {
		tx_cmsg.cmsg_data1 = i;
		tx_cmsg.cmsg_data2 = i;
		retry_count = 10;
retry:
		if (zchan_tx_one_msg(zcan->zchan,
		    &tx_cmsg, sizeof(zcan_msg_t))) {
			if (--retry_count) {
				msleep(1);
				goto retry;
			} else {
				break;
			}
		}
	}
	mdelay(ZCAN_PIO_STOP_DELAY);
	zynq_err("%d can%d %s: Tx test End %d(%d)!!!\n",
	    ZYNQ_INST(zcan->zchan), zcan->zcan_ip_num,
	    __FUNCTION__, i - 1, msgcnt);
}

/* CAN Rx/Tx test function in pio mode */
void zcan_pio_test(zynq_can_t *zcan, int loopback, int hi_pri, int msgcnt)
{
	zcan_msg_t tx_cmsg;
#if !defined(PIO_RX_THREAD) && !defined(PIO_RX_INTR)
	zcan_msg_t rx_cmsg;
#endif
	ktime_t ktime_start, ktime_end;
	int i;

	if (msgcnt == 0) {
		zynq_trace(ZYNQ_TRACE_CAN_PIO, "%d can%d %s: msgcnt is 0!\n",
		    ZYNQ_INST(zcan->zchan), zcan->zcan_ip_num, __FUNCTION__);
		return;
	}

	if (loopback) {
		zcan_pio_loopback_set(zcan);
	}
	zcan_pio_start(zcan);

	tx_cmsg.cmsg_id = 0;
	tx_cmsg.cmsg_id_sid = 0x554;
	tx_cmsg.cmsg_len = 0x80000000;
	zynq_err("%d can%d %s: Tx test start:\n", ZYNQ_INST(zcan->zchan),
	    zcan->zcan_ip_num, __FUNCTION__);
	ktime_start = ktime_get();
	for (i = 1; i <= msgcnt; i++) {
		tx_cmsg.cmsg_data1 = 0x55555555;
		tx_cmsg.cmsg_data2 = i; // tx_cmsg.cmsg_data1;

#if !defined(PIO_RX_THREAD) && !defined(PIO_RX_INTR)
		/* do Rx after each Tx and verify if the data matches */
		if (hi_pri) {
			/* use high priority Tx FIFO */
			if (zcan_pio_tx_hi_one(zcan, &tx_cmsg, 1, 1) !=
			    BCAN_OK) {
				zynq_trace(ZYNQ_TRACE_CAN_PIO,
				    "%d can%d %s: Tx test failed %d!!!\n",
				    ZYNQ_INST(zcan->zchan),
				    zcan->zcan_ip_num, __FUNCTION__, i);
				goto out;
			}
		} else {
			/* use normal priority Tx FIFO */
			if (zcan_pio_tx_one(zcan, &tx_cmsg, 1, 1) != BCAN_OK) {
				zynq_trace(ZYNQ_TRACE_CAN_PIO, "%d can%d %s: "
				    "Tx test failed %d!!!\n",
				    ZYNQ_INST(zcan->zchan),
				    zhan->zcan_ip_num, __FUNCTION__, i);
				goto out;
			}
		}

		if (loopback) {
			zcan_pio_rx(zcan, (char *)&rx_cmsg,
			    sizeof(zcan_msg_t), 1);
			if (tx_cmsg.cmsg_data1 != rx_cmsg.cmsg_data1 ||
			    tx_cmsg.cmsg_data2 != rx_cmsg.cmsg_data2) {
				zynq_trace(ZYNQ_TRACE_CAN_PIO,
				    "%d can%d %s: msg"
				    " %d verification failed tx_data1=0x%x "
				    "tx_data2=0x%x, rx_data1=0x%x i"
				    "rx_data2=0x%x\n", ZYNQ_INST(zcan->zchan),
				    zcan->zcan_ip_num, __FUNCTION__, i,
				    tx_cmsg.cmsg_data1, tx_cmsg.cmsg_data2,
				    rx_cmsg.cmsg_data1, rx_cmsg.cmsg_data2);
				goto out;
			}
		}
#else
		if (hi_pri) {
			/* use high priority Tx FIFO */
			if (zcan_pio_tx_hi_one(zcan, &tx_cmsg, 1, 0) !=
			    BCAN_OK) {
				zynq_trace(ZYNQ_TRACE_CAN_PIO,
				    "%d can%d %s: Tx test failed %d!!!\n",
				    ZYNQ_INST(zcan->zchan),
				    zcan->zcan_ip_num, __FUNCTION__, i);
				goto out;
			}
		} else {
			/* use normal priority Tx FIFO */
			if (zcan_pio_tx_one(zcan, &tx_cmsg, 1, 0) != BCAN_OK) {
				zynq_trace(ZYNQ_TRACE_CAN_PIO,
				    "%d can%d %s: Tx test failed %d!!!\n",
				    ZYNQ_INST(zcan->zchan),
				    zcan->zcan_ip_num, __FUNCTION__, i);
				goto out;
			}
		}
#endif
	}
	ktime_end = ktime_get();
	zynq_err("%d can%d %s: "
	    "Tx test succeeded, total %d pkts, per pkt time is %lld ns!!!\n",
	    ZYNQ_INST(zcan->zchan), zcan->zcan_ip_num, __FUNCTION__,
	    msgcnt, ktime_to_ns(ktime_sub(ktime_end, ktime_start)) / msgcnt);

out:
	mdelay(ZCAN_PIO_STOP_DELAY);
	if (loopback) {
		zcan_pio_loopback_unset(zcan);
	}
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35)
static int zcan_ioctl(struct inode *inode, struct file *filp, unsigned int cmd,
    unsigned long arg)
#else
static long zcan_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
#endif
{
	zynq_can_t *zcan = filp->private_data;
	int err = 0;
	char ioc_name[32];

	if (zcan == NULL) {
		return -1;
	}
	zynq_trace(ZYNQ_TRACE_CAN_PIO,
	    "%d can%d %s enter: cmd=0x%x, arg=0x%lx, zcan=0x%p\n",
	    ZYNQ_INST(zcan->zchan), zcan->zcan_ip_num, __FUNCTION__,
	    cmd, arg, zcan);

	switch (cmd) {
	case ZYNQ_IOC_CAN_TX_TIMEOUT_SET:
		sprintf(ioc_name, "ZYNQ_IOC_CAN_TX_TIMEOUT_SET");
		zcan_tx_timeout_set(zcan, arg);
		break;

	case ZYNQ_IOC_CAN_RX_TIMEOUT_SET:
		sprintf(ioc_name, "ZYNQ_IOC_CAN_RX_TIMEOUT_SET");
		zcan_rx_timeout_set(zcan, arg);
		break;

	case ZYNQ_IOC_CAN_DEV_START:
		sprintf(ioc_name, "ZYNQ_IOC_CAN_DEV_START");
		zcan_pio_start(zcan);
		break;

	case ZYNQ_IOC_CAN_DEV_STOP:
		sprintf(ioc_name, "ZYNQ_IOC_CAN_DEV_STOP");
		zcan_pio_stop(zcan);
		break;

	case ZYNQ_IOC_CAN_DEV_RESET:
		sprintf(ioc_name, "ZYNQ_IOC_CAN_DEV_RESET");
		zcan_pio_reset(zcan);
		break;

	case ZYNQ_IOC_CAN_BAUDRATE_SET:
		sprintf(ioc_name, "ZYNQ_IOC_CAN_BAUDRATE_SET");
		err = zcan_pio_set_baudrate(zcan, arg);
		break;

	case ZYNQ_IOC_CAN_BAUDRATE_GET:
		sprintf(ioc_name, "ZYNQ_IOC_CAN_BAUDRATE_GET");
		if (copy_to_user((void __user *)arg, &zcan->zcan_pio_baudrate,
		    sizeof(unsigned long))) {
			err = -EFAULT;
		}
		break;

	case ZYNQ_IOC_CAN_LOOPBACK_SET:
		sprintf(ioc_name, "ZYNQ_IOC_CAN_LOOPBACK_SET");
		zcan_pio_loopback_set(zcan);
		break;

	case ZYNQ_IOC_CAN_LOOPBACK_UNSET:
		sprintf(ioc_name, "ZYNQ_IOC_CAN_LOOPBACK_UNSET");
		zcan_pio_loopback_unset(zcan);
		break;

	case ZYNQ_IOC_CAN_RECV:
	{
		ioc_bcan_msg_t bcan_msg_arg;

		sprintf(ioc_name, "ZYNQ_IOC_CAN_RECV");
		if (copy_from_user(&bcan_msg_arg, (void __user *)arg,
		    sizeof(ioc_bcan_msg_t))) {
			bcan_msg_arg.ioc_msg_err = BCAN_FAIL;
			goto rx_out;
		}

		if (bcan_msg_arg.ioc_msg_rx_clear) {
			zcan_clear_buf_rx(zcan);
		}

		if (ZDEV_IS_ERR(zcan->zdev)) {
			bcan_msg_arg.ioc_msg_err = BCAN_DEV_ERR;
			goto rx_out;
		}

		bcan_msg_arg.ioc_msg_err = zcan_rx_user(zcan, &bcan_msg_arg,
		    &bcan_msg_arg.ioc_msg_num_done);

rx_out:
		if (copy_to_user((void __user *)
		    (&((ioc_bcan_msg_t *)arg)->ioc_msg_num_done),
		    &bcan_msg_arg.ioc_msg_num_done, 2 * sizeof(int))) {
			err = -EFAULT;
		}
		break;
	}

	case ZYNQ_IOC_CAN_SEND:
	{
		ioc_bcan_msg_t bcan_msg_arg;

		sprintf(ioc_name, "ZYNQ_IOC_CAN_SEND");
		if (copy_from_user(&bcan_msg_arg, (void __user *)arg,
		    sizeof(ioc_bcan_msg_t))) {
			bcan_msg_arg.ioc_msg_err = BCAN_FAIL;
			goto tx_out;
		}

		if (ZDEV_IS_ERR(zcan->zdev)) {
			bcan_msg_arg.ioc_msg_err = BCAN_DEV_ERR;
			goto tx_out;
		}

		bcan_msg_arg.ioc_msg_err = zcan_tx_user(zcan,
		    &bcan_msg_arg, 0, &bcan_msg_arg.ioc_msg_num_done);

tx_out:
		if (copy_to_user((void __user *)
		    (&((ioc_bcan_msg_t *)arg)->ioc_msg_num_done),
		    &bcan_msg_arg.ioc_msg_num_done, 2 * sizeof(int))) {
			err = -EFAULT;
		}

		break;
	}

	case ZYNQ_IOC_CAN_SEND_HIPRI:
	{
		ioc_bcan_msg_t bcan_msg_arg;

		sprintf(ioc_name, "ZYNQ_IOC_CAN_SEND_HIPRI");
		if (copy_from_user(&bcan_msg_arg, (void __user *)arg,
		    sizeof(ioc_bcan_msg_t))) {
			bcan_msg_arg.ioc_msg_err = BCAN_FAIL;
			goto tx_hi_out;
		}

		if (ZDEV_IS_ERR(zcan->zdev)) {
			bcan_msg_arg.ioc_msg_err = BCAN_DEV_ERR;
			goto tx_hi_out;
		}

		bcan_msg_arg.ioc_msg_err = zcan_tx_user(zcan,
		    &bcan_msg_arg, 1, &bcan_msg_arg.ioc_msg_num_done);
tx_hi_out:
		if (copy_to_user((void __user *)
		    (&((ioc_bcan_msg_t *)arg)->ioc_msg_num_done),
		    &bcan_msg_arg.ioc_msg_num_done, 2 * sizeof(int))) {
			err = -EFAULT;
		}

		break;

	}

	case ZYNQ_IOC_CAN_GET_STATUS_ERR:
	{
		ioc_bcan_status_err_t status_err;

		sprintf(ioc_name, "ZYNQ_IOC_CAN_GET_STATUS_ERR");
		if (ZDEV_IS_ERR(zcan->zdev)) {
			status_err.bcan_ioc_err = BCAN_DEV_ERR;
		} else {
			status_err.bcan_status =
			    zcan_reg_read(zcan, ZCAN_IP_SR);
			status_err.bcan_err_status =
			    zcan_reg_read(zcan, ZCAN_IP_ESR);
			status_err.bcan_err_count =
			    zcan_reg_read(zcan, ZCAN_IP_ECR);
			status_err.bcan_ioc_err = BCAN_OK;
		}
		if (copy_to_user((void __user *)arg, &status_err,
		    sizeof(ioc_bcan_status_err_t))) {
			err = -EFAULT;
		}

		break;
	}

	default:
		sprintf(ioc_name, "ZYNQ_IOC_CAN_UNKNOWN");
		err = -EINVAL;
		break;
	}

	zynq_trace(ZYNQ_TRACE_CAN_PIO,
	    "%d can%d %s done: cmd=0x%x(%s), error=%d\n",
	    ZYNQ_INST(zcan->zchan), zcan->zcan_ip_num, __FUNCTION__,
	    cmd, ioc_name, err);
	return err;
}

static int zcan_open(struct inode *inode, struct file *filp)
{
	zynq_can_t *zcan;

	zcan = container_of(inode->i_cdev, zynq_can_t, zcan_pio_cdev);
	spin_lock(&zcan->zcan_pio_lock);
	if (zcan->zcan_pio_opened != 0) {
		zynq_err("%d can%d %s: device is opened already!\n",
		    ZYNQ_INST(zcan->zchan), zcan->zcan_ip_num, __FUNCTION__);
		spin_unlock(&zcan->zcan_pio_lock);
		return BCAN_DEV_BUSY;
	}
	zcan->zcan_pio_opened = 1;
	spin_unlock(&zcan->zcan_pio_lock);
	filp->private_data = zcan;

	/* reset to default normal mode */
	zcan_pio_reset(zcan);

	if (ZDEV_IS_ERR(zcan->zdev)) {
		spin_lock(&zcan->zcan_pio_lock);
		zcan->zcan_pio_opened = 0;
		zynq_err("%d can%d %s: device access error!\n",
		    ZYNQ_INST(zcan->zchan), zcan->zcan_ip_num, __FUNCTION__);
		spin_unlock(&zcan->zcan_pio_lock);
		return BCAN_DEV_ERR;
	}

	zynq_err("%d can%d %s done: zcan=0x%p\n", ZYNQ_INST(zcan->zchan),
	    zcan->zcan_ip_num, __FUNCTION__, zcan);
	return 0;
}

static int zcan_release(struct inode *inode, struct file *filp)
{
	zynq_can_t *zcan = filp->private_data;
	zynq_chan_t *zchan = zcan->zchan;

	spin_lock(&zcan->zcan_pio_lock);
	spin_lock(&zcan->zcan_pio_tx_lock);
	spin_lock_bh(&zcan->zcan_pio_rx_lock);
	zcan->zcan_pio_opened = 0;
	/* reset to quiese the h/w */
	zcan->zcan_pio_state = ZYNQ_STATE_RESET;
	zcan_reg_write(zcan, ZCAN_IP_SRR, 0);
	mdelay(10);
	zchan_reg_write(zchan, ZYNQ_CH_RESET, ZYNQ_CH_RESET_EN);
	zchan_reg_write(zchan, ZYNQ_CH_RESET, 0);
	spin_unlock_bh(&zcan->zcan_pio_rx_lock);
	spin_unlock(&zcan->zcan_pio_tx_lock);
	spin_unlock(&zcan->zcan_pio_lock);

	zynq_err("%d can%d %s done: zcan=0x%p\n", ZYNQ_INST(zchan),
	    zcan->zcan_ip_num, __FUNCTION__, zcan);

	return 0;
}


struct file_operations zcan_pio_fops = {
	.owner		= THIS_MODULE,
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35)
	.ioctl		= zcan_ioctl,
#else
	.unlocked_ioctl = zcan_ioctl,
#endif
	.open		= zcan_open,
	.release	= zcan_release
};

int zcan_pio_init(zynq_can_t *zcan, enum zcan_ip_mode mode)
{
	u32 ip_mode = ZCAN_IP_MSR_NORMAL;
	char zcan_pio_name[16];

	/* create the /dev/zynq_can%d */
	sprintf(zcan_pio_name, ZYNQ_DEV_NAME_CAN"%u",
	    zcan->zdev->zdev_can_num_start + zcan->zcan_ip_num);
	zcan->zcan_pio_dev = zynq_create_cdev(zcan, &zcan->zcan_pio_cdev,
	    &zcan_pio_fops, zcan_pio_name);
	if (!zcan->zcan_pio_dev) {
		zynq_err("%s: failed to create cdev.\n", __FUNCTION__);
		return -1;
	}

	if (zynq_can_rx_buf_num < ZCAN_IP_MSG_MIN) {
		zynq_can_rx_buf_num = ZCAN_IP_MSG_MIN;
	} else if (zynq_can_rx_buf_num > ZCAN_IP_MSG_MAX) {
		zynq_can_rx_buf_num = ZCAN_IP_MSG_MAX;
	}

	/* allocate Rx buffer ring */
	zcan->zcan_buf_rx_msg = kzalloc(zynq_can_rx_buf_num * sizeof(bcan_msg_t),
	    GFP_KERNEL);
	if (zcan->zcan_buf_rx_msg == NULL) {
		zynq_destroy_cdev(zcan->zcan_pio_dev, &zcan->zcan_pio_cdev);
		zynq_err("%s: failed to alloc cmsg array.\n", __FUNCTION__);
		return -1;
	}
	zcan->zcan_buf_rx_num = zynq_can_rx_buf_num;
	zcan->zcan_pio_baudrate = ZYNQ_BAUDRATE_500K;
	spin_lock_init(&zcan->zcan_pio_lock);
	spin_lock_init(&zcan->zcan_pio_tx_lock);
	spin_lock_init(&zcan->zcan_pio_tx_hi_lock);
	spin_lock_init(&zcan->zcan_pio_rx_lock);
	init_completion(&zcan->zcan_usr_rx_comp);

	if (mode == ZCAN_IP_SLEEP) {
		ip_mode = ZCAN_IP_MSR_SLEEP;
	} else if (mode == ZCAN_IP_LOOPBACK) {
		ip_mode = ZCAN_IP_MSR_LOOPBACK;
	}
	/* init the CAN IP h/w */
	zcan_pio_chip_init(zcan, ip_mode, 1);

#ifdef PIO_RX_THREAD
	/* create the pio rx thread */
	zcan->zcan_pio_rx_thread = kthread_run(zcan_pio_rx_poll, (void *)zcan,
	    "zcan_pio_rx_poll");
	if (IS_ERR(zcan->zcan_pio_rx_thread)) {
		zynq_destroy_cdev(zcan->zcan_pio_dev, &zcan->zcan_pio_cdev);
		kfree(zcan->zcan_buf_rx_msg);
		zynq_err("%s: failed to creat rx poll thread.\n", __FUNCTION__);
		return (-1);
	}
#endif

	zynq_trace(ZYNQ_TRACE_CAN_PIO, "%d can%d %s done.\n",
	    ZYNQ_INST(zcan->zchan), zcan->zcan_ip_num, __FUNCTION__);
	return 0;
}

void zcan_pio_fini(zynq_can_t *zcan)
{
#ifdef PIO_RX_THREAD
	kthread_stop(zcan->zcan_pio_rx_thread);
#endif
	zcan_pio_reset_noinit(zcan);
	zynq_destroy_cdev(zcan->zcan_pio_dev, &zcan->zcan_pio_cdev);
	kfree(zcan->zcan_buf_rx_msg);
}

static void zcan_stats_init(zynq_can_t *zcan)
{
	int i;

	for (i = 0; i < CAN_STATS_NUM; i++) {
		zcan->stats[i].label = zcan_stats_label[i];
	}
}

int zcan_init(zynq_can_t *zcan)
{
	zynq_dev_t *zdev = zcan->zdev;

	snprintf(zcan->prefix, ZYNQ_LOG_PREFIX_LEN,
	    "%d can%d", zdev->zdev_inst, zcan->zcan_ip_num);
	zcan_stats_init(zcan);

	/* init the CAN IP register base */
	zcan->zcan_ip_reg = zdev->zdev_bar2 + ZCAN_IP_OFFSET *
	    zcan->zcan_ip_num;
	zynq_trace(ZYNQ_TRACE_CAN, "%d can%d %s zcan_ip_reg=0x%p\n",
	    zdev->zdev_inst, zcan->zcan_ip_num, __FUNCTION__,
	    zcan->zcan_ip_reg);

	/* init CAN IP */
	if (zcan_pio_init(zcan, ZCAN_IP_NORMAL)) {
		zynq_err("%d can%d %s: failed to init CAN IP\n",
		    zdev->zdev_inst, zcan->zcan_ip_num, __FUNCTION__);
		return -1;
	}

	zynq_trace(ZYNQ_TRACE_CAN, "%d can%d %s done: zcan=%p\n",
	    zdev->zdev_inst, zcan->zcan_ip_num, __FUNCTION__, zcan);
	return 0;
}

void zcan_fini(zynq_can_t *zcan)
{
	zcan_pio_fini(zcan);
	zynq_trace(ZYNQ_TRACE_CAN, "%d can%d %s done: zcan=%p\n",
	    ZYNQ_INST(zcan->zchan), zcan->zcan_ip_num, __FUNCTION__, zcan);
}

/*
 * return the zcan pointer conresponding to the CAN IP number
 */
zynq_can_t *can_ip_to_zcan(zynq_dev_t *zdev, u32 can_ip_num)
{
	zynq_can_t *zcan;
	int i;

	if (can_ip_num >= zdev->zdev_can_cnt) {
		zynq_trace(ZYNQ_TRACE_CAN, "%s wrong can_ip_num=%d, the "
		    "device only have %d CAN IPs\n", __FUNCTION__,
		    can_ip_num, zdev->zdev_can_cnt);
		return NULL;
	}
	zcan = zdev->zdev_cans;
	for (i = 0; i < zdev->zdev_can_cnt; i++, zcan++) {
		if (zcan->zcan_ip_num == can_ip_num) {
			zynq_trace(ZYNQ_TRACE_CAN, "%s succeeded to find "
			    "zcan for can_ip_num=%d.\n", __FUNCTION__,
			    can_ip_num);
			return zcan;
		}
	}
	zynq_trace(ZYNQ_TRACE_CAN, "%s failed to find zcan for can_ip_num "
	    "%d.\n", __FUNCTION__, can_ip_num);

	return NULL;
}

#ifdef ZYNQ_DEBUG
module_param_named(canbtr, zynq_can_ip_btr, uint, S_IRUGO|S_IWUSR);
MODULE_PARM_DESC(canbtr, "CAN bus sampling point");
#endif
module_param_named(canrxbuf, zynq_can_rx_buf_num, uint, S_IRUGO|S_IWUSR);
MODULE_PARM_DESC(canrxbuf, "CAN Rx buffer number");
module_param_named(nocanhwts, zynq_disable_can_hw_ts, uint, S_IRUGO|S_IWUSR);
MODULE_PARM_DESC(nocanhwts, "disable CAN Rx H/W timestamp");
module_param_named(canrxdma, zynq_enable_can_rx_dma, uint, S_IRUGO|S_IWUSR);
MODULE_PARM_DESC(canrxdma, "enable CAN Rx DMA mode");
module_param_named(cantxdma, zynq_enable_can_tx_dma, uint, S_IRUGO|S_IWUSR);
MODULE_PARM_DESC(cantxdma, "enable CAN Tx DMA mode");
