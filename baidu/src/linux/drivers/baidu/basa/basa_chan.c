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

static const char zchan_stats_label[CHAN_STATS_NUM][ZYNQ_STATS_LABEL_LEN] = {
	"Rx interrupt",
	"Tx interrupt",
	"Rx error interrupt",
	"Tx error interrupt",
	"Rx drop",
	"Rx count",
	"Tx count"
};

/*
 * channel H/W reset
 */
static void zchan_reset(zynq_chan_t *zchan)
{
	zchan_reg_write(zchan, ZYNQ_CH_DMA_CONTROL,
	    ZYNQ_CH_DMA_RD_STOP | ZYNQ_CH_DMA_WR_STOP);
	mdelay(1);
	zchan_reg_write(zchan, ZYNQ_CH_RESET, ZYNQ_CH_DMA_RESET_EN);
	zchan_reg_write(zchan, ZYNQ_CH_RESET, 0);
	mdelay(1);
}

/*
 * read the H/W Tx head index
 */
static inline u32 zchan_tx_read_head(zchan_tx_ring_t *zchan_tx)
{
	zynq_chan_t *zchan = zchan_tx->zchan;
	u64 tx_head_dma;
	u32 tx_head = zchan_tx->zchan_tx_head;

	tx_head_dma = zchan_reg_read(zchan, ZYNQ_CH_TX_HEAD_HI);
	tx_head_dma <<= 32;
	tx_head_dma |= zchan_reg_read(zchan, ZYNQ_CH_TX_HEAD_LO);

	if (tx_head_dma >= zchan_tx->zchan_tx_dma) {
		tx_head = (tx_head_dma - zchan_tx->zchan_tx_dma) /
		    sizeof(zchan_tx_desc_t);
		if (tx_head >= zchan_tx->zchan_tx_num) {
			zynq_err("%d ch %d %s bad index: tx_head=%u, "
			    "zchan_tx_num=%u\n", ZYNQ_INST(zchan),
			    zchan->zchan_num, __FUNCTION__, tx_head,
			    zchan_tx->zchan_tx_num);
			tx_head = zchan_tx->zchan_tx_head;
		}
	} else {
		zynq_err("%d ch %d %s failed: tx_head_dma=0x%llx, zchan_tx_dma="
		    "0x%llx\n", ZYNQ_INST(zchan), zchan->zchan_num, __FUNCTION__,
		    tx_head_dma, zchan_tx->zchan_tx_dma);
	}

	return tx_head;
}

/*
 * write the H/W Tx tail index
 */
static inline void zchan_tx_write_tail(zchan_tx_ring_t *zchan_tx,
		u32 tail_index)
{
	zynq_chan_t *zchan = zchan_tx->zchan;
	u64 tx_tail_dma;

	if (tail_index >= zchan_tx->zchan_tx_num) {
		zynq_err("%d ch %d %s failed: bad tail_index=x%x zchan_tx_num="
		    "%u\n", ZYNQ_INST(zchan), zchan->zchan_num, __FUNCTION__,
		    tail_index, zchan_tx->zchan_tx_num);
		return;
	}

	tx_tail_dma = zchan_tx->zchan_tx_dma;
	tx_tail_dma += tail_index * sizeof(zchan_tx_desc_t);
	zchan_reg_write(zchan, ZYNQ_CH_TX_TAIL_HI, HI32(tx_tail_dma));
	zchan_reg_write(zchan, ZYNQ_CH_TX_TAIL_LO, LO32(tx_tail_dma));
	zchan_tx->zchan_tx_tail = tail_index;

	zynq_trace(ZYNQ_TRACE_CHAN, "%d ch %d %s done, tx_tail=%d, "
	    "tx_tail_dma=0x%llx\n", ZYNQ_INST(zchan), zchan->zchan_num,
	    __FUNCTION__, tail_index, tx_tail_dma);
}

int zchan_tx_one_msg(zynq_chan_t *zchan, void *msg, u32 msgsz)
{
	zchan_tx_ring_t *zchan_tx = &zchan->zchan_tx_ring;
	u32 tx_head, tx_tail, tx_next, tx_mask;
	zchan_buf_t *bufp;
	zchan_tx_desc_t *descp;

	/* currently only used for CAN message transmit */
	if (msgsz > zchan_tx->zchan_tx_bufsz) {
		zynq_err("%d ch %d %s failed: too large msgsz %u, "
		    "need to be less than %u\n", ZYNQ_INST(zchan),
		    zchan->zchan_num, __FUNCTION__, msgsz,
		    zchan_tx->zchan_tx_bufsz);
		return BCAN_FAIL;
	}

	spin_lock(&zchan_tx->zchan_tx_lock);
	tx_mask = zchan_tx->zchan_tx_num - 1;
	tx_head = zchan_tx->zchan_tx_head;
	tx_tail = zchan_tx->zchan_tx_tail;
	tx_next = (tx_tail + 1) & tx_mask;
	/* tx ring is full */
	if (tx_next == tx_head) {
		zynq_err("%d ch %d %s failed: ring FULL tx_head=%u, "
		    "tx_tail=%u\n", ZYNQ_INST(zchan), zchan->zchan_num,
		    __FUNCTION__, tx_head, tx_tail);
		spin_unlock(&zchan_tx->zchan_tx_lock);
		return BCAN_DEV_BUSY;
	}
	bufp = zchan_tx->zchan_tx_bufp + tx_tail;
	memcpy(bufp->zchan_bufp, msg, msgsz);
	pci_dma_sync_single_for_device(zchan->zdev->zdev_pdev,
	    bufp->zchan_buf_dma, msgsz, PCI_DMA_TODEVICE);
	descp = zchan_tx->zchan_tx_descp + tx_tail;
	descp->tx_len = msgsz;
	if (descp->tx_addr != 0) {
		zynq_err("%d ch %d %s: desc %d buffer is not cleared\n",
		    ZYNQ_INST(zchan), zchan->zchan_num, __FUNCTION__, tx_tail);
	}
	descp->tx_addr = bufp->zchan_buf_dma;

	zynq_trace(ZYNQ_TRACE_CHAN, "%d ch %d %s Tx desc %d msg %lu: "
	    "tx_len=%d, tx_addr=0x%p, tx_addr_dma=0x%llx, msgid_len=0x%llx, "
	    "data[0-7]=0x%llx\n", ZYNQ_INST(zchan), zchan->zchan_num,
	    __FUNCTION__, tx_tail, zchan->stats[CHAN_STATS_TX].cnt,
	    descp->tx_len, bufp, descp->tx_addr, *(u64 *)bufp->zchan_bufp,
	    *((u64 *)bufp->zchan_bufp + 1));

	zchan_tx_write_tail(zchan_tx, tx_next);
	ZYNQ_STATS(zchan, CHAN_STATS_TX);
	spin_unlock(&zchan_tx->zchan_tx_lock);

	return 0;
}

void zchan_rx_off2addr(zchan_rx_tbl_t *zchan_rx, u32 rx_off, void **virt_addr,
		void **phy_addr)
{
	int pdt_entry, pt_entry;
	zchan_rx_pt_t *rx_ptp;
	zchan_buf_t *rx_buf;

	pdt_entry = ZCHAN_RX_PDT_ENTRY(zchan_rx, rx_off);
	rx_ptp = zchan_rx->zchan_rx_ptp + pdt_entry;
	pt_entry = ZCHAN_RX_PT_ENTRY(zchan_rx, rx_off);
	rx_buf = rx_ptp->zchan_rx_pt_bufp + pt_entry;

	if (virt_addr != NULL) {
		*virt_addr = (void *)((uintptr_t)rx_buf->zchan_bufp +
		    ZCHAN_RX_BUF_OFFSET(zchan_rx, rx_off));
		zynq_trace(ZYNQ_TRACE_BUF, "%d ch %d %s rx_off=0x%x, "
		    "virt_addr=0x%p, pdt_entry=%d, rx_ptp=%p, pt_entry=%d, "
		    "rx_buf = %p\n", ZYNQ_INST(zchan_rx->zchan),
		    zchan_rx->zchan->zchan_num, __FUNCTION__, rx_off,
		    *virt_addr, pdt_entry, rx_ptp, pt_entry, rx_buf);
	}
	if (phy_addr != NULL) {
		*phy_addr = (void *)(rx_buf->zchan_buf_dma +
		    ZCHAN_RX_BUF_OFFSET(zchan_rx, rx_off));
		zynq_trace(ZYNQ_TRACE_BUF, "%d ch %d %s rx_off=0x%x, "
		    "phy_addr=0x%p\n", ZYNQ_INST(zchan_rx->zchan),
		    zchan_rx->zchan->zchan_num,
		    __FUNCTION__, rx_off, *phy_addr);
	}
}

/*
 * pci_alloc_consistent returns only 32-bit DMA address. Since Zynq device is
 * 64-bit access only, this the replacement for 64-bit pci_alloc_consistent.
 */
void *zchan_alloc_consistent(struct pci_dev *pdev, dma_addr_t *dmap, size_t sz)
{
	return pci_alloc_consistent(pdev, sz, dmap);
}

void zchan_free_consistent(struct pci_dev *pdev, void *ptr,
		dma_addr_t dma, size_t sz)
{
	pci_free_consistent(pdev, sz, ptr, dma);
}

static int zchan_alloc_buf(struct pci_dev *pdev, zchan_buf_t *zchan_buf,
		size_t bufsz, enum dma_data_direction dmatype)
{
	void *bufp;

	if (dmatype == PCI_DMA_BIDIRECTIONAL) {
		bufp = zchan_alloc_consistent(pdev, &zchan_buf->zchan_buf_dma,
		    bufsz);
		if (bufp == NULL) {
			zynq_err("%s alloc consistent failed: bufsz=%zd.\n",
			    __FUNCTION__, bufsz);
			return -1;
		}
		zchan_buf->zchan_bufp = bufp;
		return 0;
	}

	if (bufsz < PAGE_SIZE) {
		bufp = kmalloc(bufsz, GFP_KERNEL);
		if (bufp == NULL) {
			zynq_err("%s kmalloc failed: sz=%zd.\n",
			    __FUNCTION__, bufsz);
			return -1;
		}
		if (pdev != NULL) {
			zchan_buf->zchan_buf_dma = pci_map_single(pdev, bufp,
			    bufsz, dmatype);
			if (pci_dma_mapping_error(pdev,
			    zchan_buf->zchan_buf_dma)) {
				kfree(bufp);
				zynq_err("%s pci map failed: sz=%zd.\n",
				    __FUNCTION__, bufsz);
				return -1;
			}
		}
		zchan_buf->zchan_bufp = bufp;
	} else {
		bufp = alloc_pages(GFP_KERNEL, get_order(bufsz));
		if (bufp == NULL) {
			zynq_err("%s alloc_pages failed: sz=%zd.\n",
			    __FUNCTION__, bufsz);
			return -1;
		}
		if (pdev != NULL) {
			zchan_buf->zchan_buf_dma = pci_map_page(pdev, bufp,
			    0, bufsz, dmatype);
			if (pci_dma_mapping_error(pdev,
			    zchan_buf->zchan_buf_dma)) {
				__free_pages(bufp, get_order(bufsz));
				zynq_err("%s pci map page failed: sz=%zd.\n",
				    __FUNCTION__, bufsz);
				return -1;
			}
			if (zchan_buf->zchan_buf_dma & (bufsz - 1)) {
				zynq_err("%s dma addr is not bufsz %zd aligned."
				    " virtual addr: 0x%p, dma addr: 0x%llx\n",
				    __FUNCTION__, bufsz,
				    page_address(bufp),
				    zchan_buf->zchan_buf_dma);
			}
		}
		zchan_buf->zchan_buf_page = bufp;
		zchan_buf->zchan_bufp = page_address(zchan_buf->zchan_buf_page);
	}

	return (0);
}

static void zchan_free_buf(struct pci_dev *pdev, zchan_buf_t *zchan_buf,
		size_t bufsz, enum dma_data_direction dmatype)
{
	if (dmatype == PCI_DMA_BIDIRECTIONAL) {
		zchan_free_consistent(pdev, zchan_buf->zchan_bufp,
		    zchan_buf->zchan_buf_dma, bufsz);
		goto done;
	}

	if (bufsz < PAGE_SIZE) {
		if (pdev != NULL) {
			pci_unmap_single(pdev, zchan_buf->zchan_buf_dma, bufsz,
			    dmatype);
		}
		kfree(zchan_buf->zchan_bufp);
	} else {
		if (pdev != NULL) {
			pci_unmap_page(pdev, zchan_buf->zchan_buf_dma, bufsz,
			    dmatype);
		}
		__free_pages(zchan_buf->zchan_buf_page, get_order(bufsz));
	}

done:
	zchan_buf->zchan_buf_page = NULL;
	zchan_buf->zchan_bufp = NULL;
	zchan_buf->zchan_buf_dma = 0;
}

static int zchan_init_tx_ring(zynq_chan_t *zchan)
{
	zynq_dev_t *zdev = zchan->zdev;
	zchan_tx_ring_t *zchan_tx = &zchan->zchan_tx_ring;
	zchan_buf_t *bufp;
	size_t size;
	int num_entries;
	int bufsz;
	int i;

	if (zchan->zchan_type == ZYNQ_CHAN_VIDEO ||
	    (zchan->zchan_type == ZYNQ_CHAN_CAN && !zdev->zcan_tx_dma)) {
		return 0;
	}

	bufsz = ZCHAN_BUF_SIZE;
	num_entries = ZCHAN_TX_ENTRIES_CAN;

	/* allocate tx descriptor ring */
	size = num_entries * sizeof(zchan_tx_desc_t);
	zchan_tx->zchan_tx_descp = zchan_alloc_consistent(zdev->zdev_pdev,
	    &zchan_tx->zchan_tx_dma, size);
	if (zchan_tx->zchan_tx_descp == NULL) {
		zynq_err("%d ch %d %s failed to alloc descriptor ring.\n",
		    ZYNQ_INST(zchan), zchan->zchan_num, __FUNCTION__);
		return -1;
	}
	zchan_tx->zchan_tx_size = size;
	zchan_tx->zchan_tx_num = num_entries;

	/* allocate tx buffer array */
	size = num_entries * sizeof(zchan_buf_t);
	bufp = kzalloc(size, GFP_KERNEL);
	if (bufp == NULL) {
		zchan_free_consistent(zdev->zdev_pdev,
		    zchan_tx->zchan_tx_descp, zchan_tx->zchan_tx_dma,
		    zchan_tx->zchan_tx_size);
		zynq_err("%d ch %d %s failed to alloc buffer array.\n",
		    ZYNQ_INST(zchan), zchan->zchan_num, __FUNCTION__);
		return -1;
	}
	zchan_tx->zchan_tx_bufp = bufp;

	/* allocate tx buffers */
	for (i = 0; i < num_entries; i++, bufp++) {
		if (zchan_alloc_buf(zdev->zdev_pdev, bufp, bufsz,
		    PCI_DMA_TODEVICE)) {
			zynq_err("%d ch %d %s failed to alloc buf.\n",
			    ZYNQ_INST(zchan), zchan->zchan_num, __FUNCTION__);
			goto fail;
		}
	}
	zchan_tx->zchan_tx_bufsz = bufsz;
	spin_lock_init(&zchan_tx->zchan_tx_lock);

	/* init Tx h/w */
	zchan_reg_write(zchan, ZYNQ_CH_DMA_CONFIG, ZYNQ_CH_DMA_TX_EN);
	zchan_reg_write(zchan, ZYNQ_CH_TX_HEAD_HI,
	    HI32(zchan_tx->zchan_tx_dma));
	zchan_reg_write(zchan, ZYNQ_CH_TX_HEAD_LO,
	    LO32(zchan_tx->zchan_tx_dma));
	zchan_reg_write(zchan, ZYNQ_CH_TX_TAIL_HI,
	    HI32(zchan_tx->zchan_tx_dma));
	zchan_reg_write(zchan, ZYNQ_CH_TX_TAIL_LO,
	    LO32(zchan_tx->zchan_tx_dma));
	zchan_reg_write(zchan, ZYNQ_CH_TX_RING_SZ, zchan_tx->zchan_tx_size);
	zchan_reg_write(zchan, ZYNQ_CH_DMA_CONTROL, ZYNQ_CH_DMA_RD_START);

	zynq_trace(ZYNQ_TRACE_CHAN, "%d ch %d %s done.\n", ZYNQ_INST(zchan),
	    zchan->zchan_num, __FUNCTION__);
	return 0;

fail:
	while (i--) {
		bufp--;
		zchan_free_buf(zdev->zdev_pdev, bufp, bufsz,
		    PCI_DMA_TODEVICE);
	}
	kfree(zchan_tx->zchan_tx_bufp);
	zchan_tx->zchan_tx_bufp = NULL;
	zchan_free_consistent(zdev->zdev_pdev, zchan_tx->zchan_tx_descp,
	    zchan_tx->zchan_tx_dma, zchan_tx->zchan_tx_size);
	zchan_tx->zchan_tx_descp = NULL;

	return (-1);
}

static void zchan_fini_tx_ring(zynq_chan_t *zchan)
{
	zynq_dev_t *zdev = zchan->zdev;
	zchan_tx_ring_t *zchan_tx = &zchan->zchan_tx_ring;
	zchan_buf_t *bufp;
	int i;

	if (zchan->zchan_type == ZYNQ_CHAN_VIDEO ||
	    (zchan->zchan_type == ZYNQ_CHAN_CAN && !zdev->zcan_tx_dma)) {
		return;
	}

	/* disable tx in h/w */
	zchan_reg_write(zchan, ZYNQ_CH_DMA_CONTROL, ZYNQ_CH_DMA_RD_STOP);
	mdelay(10);
	zchan_reg_write(zchan, ZYNQ_CH_TX_RING_SZ, 0);
	zchan_reg_write(zchan, ZYNQ_CH_TX_HEAD_HI, 0);
	zchan_reg_write(zchan, ZYNQ_CH_TX_HEAD_LO, 0);
	zchan_reg_write(zchan, ZYNQ_CH_TX_TAIL_HI, 0);
	zchan_reg_write(zchan, ZYNQ_CH_TX_TAIL_LO, 0);

	/* free tx buffers */
	bufp = zchan_tx->zchan_tx_bufp;
	for (i = 0; i < zchan_tx->zchan_tx_num; i++, bufp++) {
		zchan_free_buf(zdev->zdev_pdev, bufp,
		    zchan_tx->zchan_tx_bufsz, PCI_DMA_TODEVICE);
	}

	kfree(zchan_tx->zchan_tx_bufp);
	zchan_tx->zchan_tx_bufp = NULL;
	zchan_free_consistent(zdev->zdev_pdev, zchan_tx->zchan_tx_descp,
	    zchan_tx->zchan_tx_dma, zchan_tx->zchan_tx_size);
	zchan_tx->zchan_tx_descp = NULL;

	zynq_trace(ZYNQ_TRACE_CHAN, "%d ch %d %s done.\n", ZYNQ_INST(zchan),
	    zchan->zchan_num, __FUNCTION__);
}

static int zchan_init_rx_tbl(zynq_chan_t *zchan)
{
	zchan_rx_tbl_t *zchan_rx = &zchan->zchan_rx_tbl;
	zynq_video_t *zvideo;
	struct pci_dev *pdev = zchan->zdev->zdev_pdev;
	enum dma_data_direction dmatype;
	u32 pdt_entries;
	size_t pdt_size;
	size_t bufsz;
	u64 *rx_pdtp; /* page directory table array */
	zchan_rx_pt_t *rx_ptp; /* page table array */
	u64 *rx_pt; /* page table */
	zchan_buf_t *bufp;
	int i, j;

	zynq_trace(ZYNQ_TRACE_CHAN, "%d ch %d %s: zchan=0x%p\n",
	    ZYNQ_INST(zchan), zchan->zchan_num, __FUNCTION__, zchan);

	switch (zchan->zchan_type) {
	case ZYNQ_CHAN_CAN:
		if (!zchan->zdev->zcan_rx_dma) {
			zynq_trace(ZYNQ_TRACE_CHAN,
			    "%d ch %d %s: DMA mode is not supported\n",
			    ZYNQ_INST(zchan), zchan->zchan_num, __FUNCTION__);
			return 0;
		}
		dmatype = PCI_DMA_BIDIRECTIONAL;
		bufsz = ZCHAN_BUF_SIZE;
		pdt_entries = ZCHAN_RX_PDT_ENTRIES_CAN;
		break;
	case ZYNQ_CHAN_VIDEO:
		zvideo = (zynq_video_t *)zchan->zchan_dev;
		if (zynq_video_zero_copy) {
			/*
			 * For zero-copy, the DMA buffers are not initialized
			 * here. Instead they are initialized and mapped when
			 * the streaming is started.
			 */
			return 0;
		}
		dmatype = PCI_DMA_FROMDEVICE;
		bufsz = ZCHAN_BUF_SIZE;
		pdt_entries = CEILING(zynq_video_buf_num *
		    CEILING(zvideo->format.sizeimage, ZCHAN_BUF_SIZE),
		    ZCHAN_RX_PT_ENTRIES);
		break;
	default:
		zynq_trace(ZYNQ_TRACE_CHAN,
		    "%d ch %d %s: unsupported channel type %d\n",
		    ZYNQ_INST(zchan), zchan->zchan_num,
		    __FUNCTION__, zchan->zchan_type);
		return 0;
	}

	/* allocate page directory table first */
	pdt_size = pdt_entries * sizeof(u64);
	rx_pdtp = zchan_alloc_consistent(pdev,
	    &zchan_rx->zchan_rx_pdt_dma, pdt_size);
	if (rx_pdtp == NULL) {
		zynq_err("%d ch %d %s failed to alloc page directory table.\n",
		    ZYNQ_INST(zchan), zchan->zchan_num, __FUNCTION__);
		return -1;
	}
	zchan_rx->zchan_rx_pdt_num = pdt_entries;
	zchan_rx->zchan_rx_pdt = rx_pdtp;

	/* allocate the array of zchan_rx_pt_t */
	rx_ptp = kzalloc(pdt_entries * sizeof(zchan_rx_pt_t), GFP_KERNEL);
	if (rx_ptp == NULL) {
		zchan_free_consistent(pdev,
		    zchan_rx->zchan_rx_pdt, zchan_rx->zchan_rx_pdt_dma,
		    pdt_size);
		zynq_err("%d ch %d, %s failed to alloc page table array.\n",
		    ZYNQ_INST(zchan), zchan->zchan_num, __FUNCTION__);
		return -1;
	}
	zchan_rx->zchan_rx_ptp = rx_ptp;
	zchan_rx->zchan_rx_bufsz = bufsz;

	for (i = 0; i < zchan_rx->zchan_rx_pdt_num; i++, rx_pdtp++, rx_ptp++) {
		/* allocate each page table */
		rx_pt = zchan_alloc_consistent(pdev,
		    &rx_ptp->zchan_rx_pt_dma, ZCHAN_RX_PT_SIZE);
		if (rx_pt == NULL) {
			zynq_err("%d ch %d %s failed to alloc a page table.\n",
			    ZYNQ_INST(zchan), zchan->zchan_num, __FUNCTION__);
			goto pt_alloc_fail;
		}
		rx_ptp->zchan_rx_pt = rx_pt;

		/* init the page directory table entry */
		*rx_pdtp = rx_ptp->zchan_rx_pt_dma;

		/* allocate the array of zchan_buf_t */
		bufp = kzalloc(ZCHAN_RX_PT_ENTRIES * sizeof(zchan_buf_t),
		    GFP_KERNEL);
		if (bufp == NULL) {
			zchan_free_consistent(pdev,
			    rx_ptp->zchan_rx_pt, rx_ptp->zchan_rx_pt_dma,
			    ZCHAN_RX_PT_SIZE);
			zynq_err("%d ch %d %s failed to alloc a Rx buffer array"
			    ".\n", ZYNQ_INST(zchan), zchan->zchan_num,
			    __FUNCTION__);
			goto pt_alloc_fail;
		}
		rx_ptp->zchan_rx_pt_bufp = bufp;
		rx_ptp->zchan_rx_pt_buf_num = ZCHAN_RX_PT_ENTRIES;

		/* allocate the buffer for each page entry */
		for (j = 0;  j < ZCHAN_RX_PT_ENTRIES; j++, rx_pt++, bufp++) {
			if (zchan_alloc_buf(pdev, bufp, bufsz, dmatype)) {
				zynq_err("%d ch %d %s failed to alloc a Rx "
				    "buffer.\n", ZYNQ_INST(zchan),
				    zchan->zchan_num, __FUNCTION__);
				goto buf_alloc_fail;
			}

			if (dmatype == PCI_DMA_FROMDEVICE) {
				pci_dma_sync_single_for_device(
				    zchan->zdev->zdev_pdev, bufp->zchan_buf_dma,
				    bufsz, PCI_DMA_FROMDEVICE);
			}

			/* init the page table entry */
			*rx_pt = bufp->zchan_buf_dma;

			zynq_trace(ZYNQ_TRACE_BUF, "ch %d %s alloc Rx buffer "
			    "%d = 0x%p(0x%llx)\n", zchan->zchan_num,
			    __FUNCTION__, j, bufp->zchan_bufp,
			    bufp->zchan_buf_dma);
		}
	}

	zchan_rx->zchan_rx_pt_entries = pdt_entries * ZCHAN_RX_PT_ENTRIES;
	zchan_rx->zchan_rx_size = pdt_entries * ZCHAN_RX_PT_ENTRIES * bufsz;

	zchan_rx->zchan_rx_pdt_shift = fls(ZCHAN_RX_PT_ENTRIES * bufsz) - 1;
	zchan_rx->zchan_rx_pt_shift = fls(bufsz) - 1;
	zchan_rx->zchan_rx_pt_mask = ZCHAN_RX_PT_ENTRIES - 1;
	zchan_rx->zchan_rx_buf_mask = bufsz - 1;

	spin_lock_init(&zchan_rx->zchan_rx_lock);

	zynq_trace(ZYNQ_TRACE_CHAN, "%d ch %d %s done: rx_bufsz=%zd, "
	    "rx_buf_mask=0x%x, rx_pdt_num=0x%x, pt_entries=0x%x, rx_size=0x%x, "
	    "rx_pdt_shift=%d, rx_pt_shift=%d, rx_pt_mask=0x%x.\n",
	    ZYNQ_INST(zchan), zchan->zchan_num,
	    __FUNCTION__, bufsz, zchan_rx->zchan_rx_buf_mask,
	    zchan_rx->zchan_rx_pdt_num, zchan_rx->zchan_rx_pt_entries,
	    zchan_rx->zchan_rx_size, zchan_rx->zchan_rx_pdt_shift,
	    zchan_rx->zchan_rx_pt_shift, zchan_rx->zchan_rx_pt_mask);

	/* init Rx h/w */
	if (zchan->zchan_type == ZYNQ_CHAN_CAN) {
		zchan_rx_start(zchan);
	}

	return 0;

buf_alloc_fail:
	while (j--) {
		bufp--;
		zchan_free_buf(pdev, bufp, bufsz, dmatype);
	}
	kfree(rx_ptp->zchan_rx_pt_bufp);
	zchan_free_consistent(pdev, rx_ptp->zchan_rx_pt,
	    rx_ptp->zchan_rx_pt_dma, ZCHAN_RX_PT_SIZE);

pt_alloc_fail:
	while (i--) {
		rx_ptp--;
		bufp = rx_ptp->zchan_rx_pt_bufp;
		for (j = 0; j < ZCHAN_RX_PT_ENTRIES; j++, bufp++) {
			zchan_free_buf(pdev, bufp, bufsz, dmatype);
		}
		kfree(rx_ptp->zchan_rx_pt_bufp);
		zchan_free_consistent(pdev,
		    rx_ptp->zchan_rx_pt, rx_ptp->zchan_rx_pt_dma,
		    ZCHAN_RX_PT_SIZE);
	}
	kfree(zchan_rx->zchan_rx_ptp);
	zchan_rx->zchan_rx_ptp = NULL;
	zchan_free_consistent(pdev, zchan_rx->zchan_rx_pdt,
	    zchan_rx->zchan_rx_pdt_dma, pdt_size);
	zchan_rx->zchan_rx_pdt = NULL;

	return -1;
}

static void zchan_fini_rx_tbl(zynq_chan_t *zchan)
{
	zchan_rx_tbl_t *zchan_rx = &zchan->zchan_rx_tbl;
	struct pci_dev *pdev = zchan->zdev->zdev_pdev;
	enum dma_data_direction dmatype;
	zchan_rx_pt_t *rx_ptp;
	zchan_buf_t *bufp;
	int i, j;

	zynq_trace(ZYNQ_TRACE_CHAN, "%d ch %d %s: zchan=0x%p\n",
	    ZYNQ_INST(zchan), zchan->zchan_num, __FUNCTION__, zchan);

	switch (zchan->zchan_type) {
	case ZYNQ_CHAN_CAN:
		if (!zchan->zdev->zcan_rx_dma) {
			return;
		}
		dmatype = PCI_DMA_BIDIRECTIONAL;
		break;
	case ZYNQ_CHAN_VIDEO:
		if (zynq_video_zero_copy) {
			return;
		}
		dmatype = PCI_DMA_FROMDEVICE;
		break;
	default:
		return;
	}

	/* disable rx in h/w */
	zchan_rx_stop(zchan);

	rx_ptp = zchan_rx->zchan_rx_ptp;
	for (i = 0; i < zchan_rx->zchan_rx_pdt_num; i++, rx_ptp++) {
		bufp = rx_ptp->zchan_rx_pt_bufp;
		for (j = 0; j < rx_ptp->zchan_rx_pt_buf_num; j++, bufp++) {
			zchan_free_buf(pdev, bufp,
			    zchan_rx->zchan_rx_bufsz, dmatype);
		}
		kfree(rx_ptp->zchan_rx_pt_bufp);
		zchan_free_consistent(pdev,
		    rx_ptp->zchan_rx_pt, rx_ptp->zchan_rx_pt_dma,
		    ZCHAN_RX_PT_SIZE);
	}
	kfree(zchan_rx->zchan_rx_ptp);
	zchan_rx->zchan_rx_ptp = NULL;
	zchan_free_consistent(zchan->zdev->zdev_pdev,
	    zchan_rx->zchan_rx_pdt, zchan_rx->zchan_rx_pdt_dma,
	    zchan_rx->zchan_rx_pdt_num * sizeof(u64));
	zchan_rx->zchan_rx_pdt = NULL;

	zynq_trace(ZYNQ_TRACE_CHAN, "%d ch %d %s done.\n", ZYNQ_INST(zchan),
	    zchan->zchan_num, __FUNCTION__);
}

void zchan_rx_start(zynq_chan_t *zchan)
{
	zynq_dev_t *zdev = zchan->zdev;
	zchan_rx_tbl_t *zchan_rx = &zchan->zchan_rx_tbl;
	zynq_video_t *zvideo;
	u32 ch_config;
	u32 ch_status;
	u32 last_pt_sz;
	int i;

	zynq_trace(ZYNQ_TRACE_CHAN, "%d ch %d %s: zchan=0x%p\n",
	    ZYNQ_INST(zchan), zchan->zchan_num, __FUNCTION__, zchan);

	spin_lock(&zdev->zdev_lock);

	/* Reset DMA first */
	zchan_reset(zchan);

	/* Setup frame size */
	switch (zchan->zchan_type) {
	case ZYNQ_CHAN_CAN:
		/* Enable DMA mode */
		if (zdev->zcan_rx_hw_ts) {
			ch_config = (ZYNQ_CH_DMA_CAN_HWTS | ZYNQ_CH_DMA_RX_EN);
		} else {
			ch_config = ZYNQ_CH_DMA_RX_EN;
		}
		zchan_reg_write(zchan, ZYNQ_CH_DMA_CONFIG, ch_config);
		break;
	case ZYNQ_CHAN_VIDEO:
		zvideo = (zynq_video_t *)zchan->zchan_dev;
		ch_config = zchan_reg_read(zchan, ZYNQ_CH_DMA_CONFIG);
		ch_config &= ~ZYNQ_CH_DMA_FRAME_SZ_MASK;
		ch_config |= (zvideo->format.sizeimage <<
		    ZYNQ_CH_DMA_FRAME_SZ_OFFSET) & ZYNQ_CH_DMA_FRAME_SZ_MASK;
		/* Enable frame buffer alignment */
		ch_config |= ZYNQ_CH_DMA_FRAME_BUF_ALIGN;
		/* Enable DMA mode */
		ch_config |= ZYNQ_CH_DMA_RX_EN;
		zchan_reg_write(zchan, ZYNQ_CH_DMA_CONFIG, ch_config);

		last_pt_sz = (zchan_rx->zchan_rx_pt_entries &
		    zchan_rx->zchan_rx_pt_mask) * sizeof(u64);
		zchan_reg_write(zchan, ZYNQ_CH_WR_TABLE_CONFIG, last_pt_sz);
		break;
	default:
		spin_unlock(&zdev->zdev_lock);
		return;
	}

	zchan_reg_write(zchan, ZYNQ_CH_RX_PDT_HI,
	    HI32(zchan_rx->zchan_rx_pdt_dma));
	zchan_reg_write(zchan, ZYNQ_CH_RX_PDT_LO,
	    LO32(zchan_rx->zchan_rx_pdt_dma));
	zchan_reg_write(zchan, ZYNQ_CH_RX_PDT_SZ,
	    zchan_rx->zchan_rx_pdt_num * sizeof(u64));
	zchan_reg_write(zchan, ZYNQ_CH_RX_TAIL, 0);
	zchan_reg_write(zchan, ZYNQ_CH_RX_HEAD, 0);

	zchan_rx->zchan_rx_head = 0;
	zchan_rx->zchan_rx_tail = 0;
	zchan_rx->zchan_rx_off = 0;

	/* Start Rx DMA */
	zchan_reg_write(zchan, ZYNQ_CH_DMA_CONTROL, ZYNQ_CH_DMA_WR_START);

	/* Check DMA ready status */
	i = 0;
	ch_status = zchan_reg_read(zchan, ZYNQ_CH_DMA_STATUS);
	while (GET_BITS(ZYNQ_CH_DMA_WR_STATE, ch_status) !=
	    ZYNQ_CH_DMA_READY) {
		if (i++ >= 10) {
			zynq_err("%d ch %d %s: WARNING! DMA not ready\n",
			    ZYNQ_INST(zchan), zchan->zchan_num, __FUNCTION__);
			break;
		}
		mdelay(1);
		ch_status = zchan_reg_read(zchan, ZYNQ_CH_DMA_STATUS);
	}

	spin_unlock(&zdev->zdev_lock);
}

void zchan_rx_stop(zynq_chan_t *zchan)
{
	zynq_trace(ZYNQ_TRACE_CHAN, "%d ch %d %s: zchan=0x%p\n",
	    ZYNQ_INST(zchan), zchan->zchan_num, __FUNCTION__, zchan);

	/* Stop Rx DMA */
	zchan_reg_write(zchan, ZYNQ_CH_DMA_CONTROL, ZYNQ_CH_DMA_WR_STOP);
	mdelay(10);
	zchan_reg_write(zchan, ZYNQ_CH_RX_PDT_HI, 0);
	zchan_reg_write(zchan, ZYNQ_CH_RX_PDT_LO, 0);
	zchan_reg_write(zchan, ZYNQ_CH_RX_PDT_SZ, 0);
	zchan_reg_write(zchan, ZYNQ_CH_RX_TAIL, 0);
	zchan_reg_write(zchan, ZYNQ_CH_RX_HEAD, 0);
}

void zchan_tx_done(zchan_tx_ring_t *zchan_tx)
{
	u32 head, new_head, tx_mask;
	zchan_tx_desc_t *descp;
	int idx;

	head = zchan_tx->zchan_tx_head;
	new_head = zchan_tx_read_head(zchan_tx);
	if (head == new_head) {
		return;
	}
	tx_mask = zchan_tx->zchan_tx_num - 1;
	for (idx = head; idx != new_head; idx = (idx + 1) & tx_mask) {
		descp = zchan_tx->zchan_tx_descp + idx;
		descp->tx_addr = 0;
	}
	zchan_tx->zchan_tx_head = new_head;

	zynq_trace(ZYNQ_TRACE_CHAN, "%d ch %d %s done: tx_head=%d\n",
	    ZYNQ_INST(zchan_tx->zchan), zchan_tx->zchan->zchan_num,
	    __FUNCTION__, new_head);
}

static int zchan_init_dma(zynq_chan_t *zchan)
{
	/* reset DMA first */
	zchan_reset(zchan);

	/* init rx page directory table for Rx buffers */
	if (zchan_init_rx_tbl(zchan)) {
		zynq_err("%d ch %d %s init rx table failed, zchan=0x%p.\n",
		    ZYNQ_INST(zchan), zchan->zchan_num, __FUNCTION__, zchan);
		return -1;
	}

	if (zchan->zchan_type == ZYNQ_CHAN_CAN) {
		/* init tx descriptor ring */
		if (zchan_init_tx_ring(zchan)) {
			zynq_err("%d ch %d %s "
			    "init tx ring failed, zchan=0x%p.\n",
			    ZYNQ_INST(zchan), zchan->zchan_num,
			    __FUNCTION__, zchan);
			zchan_fini_rx_tbl(zchan);
			return -1;
		}
	}

	zynq_trace(ZYNQ_TRACE_CHAN, "%d ch %d %s done, zchan=0x%p.\n",
	    ZYNQ_INST(zchan), zchan->zchan_num, __FUNCTION__, zchan);
	return 0;
}

static void zchan_fini_dma(zynq_chan_t *zchan)
{
	/* disable DMA mode */
	zchan_reg_write(zchan, ZYNQ_CH_DMA_CONFIG, 0);
	mdelay(10);

	zchan_fini_rx_tbl(zchan);

	if (zchan->zchan_type == ZYNQ_CHAN_CAN) {
		zchan_fini_tx_ring(zchan);
	}
}

static void zchan_stats_init(zynq_chan_t *zchan)
{
	int i;

	for (i = 0; i < CHAN_STATS_NUM; i++) {
		zchan->stats[i].label = zchan_stats_label[i];
	}
}

int zchan_init(zynq_chan_t *zchan)
{
	zynq_can_t *zcan;
	zynq_video_t *zvideo;

	snprintf(zchan->prefix, ZYNQ_LOG_PREFIX_LEN,
	    "%d ch%d", ZYNQ_INST(zchan), zchan->zchan_num);
	zchan_stats_init(zchan);

	zchan->zchan_reg = zchan->zdev->zdev_bar0 +
	    ZYNQ_CHAN_REG_OFFSET * zchan->zchan_num;

	spin_lock_init(&zchan->zchan_lock);

	/* channel specific init */
	switch (zchan->zchan_type) {
	case ZYNQ_CHAN_CAN:
		zcan = (zynq_can_t *)zchan->zchan_dev;
		if (zchan_init_dma(zchan)) {
			return -1;
		}
		if (zcan_init(zcan)) {
			zchan_fini_dma(zchan);
			return -1;
		}
		break;
	case ZYNQ_CHAN_VIDEO:
		zvideo = (zynq_video_t *)zchan->zchan_dev;

		if (zvideo_init(zvideo)) {
			return -1;
		}
		if (zchan_init_dma(zchan)) {
			zvideo_fini(zvideo);
			return -1;
		}
		break;
	default:
		break;
	}

	zynq_trace(ZYNQ_TRACE_CHAN, "%d ch %d %s done: zchan=%p\n",
	    ZYNQ_INST(zchan), zchan->zchan_num, __FUNCTION__, zchan);
	return 0;
}

void zchan_fini(zynq_chan_t *zchan)
{
	switch (zchan->zchan_type) {
	case ZYNQ_CHAN_CAN:
		zcan_fini(zchan->zchan_dev);
		zchan_fini_dma(zchan);
		break;
	case ZYNQ_CHAN_VIDEO:
		zvideo_fini(zchan->zchan_dev);
		zchan_fini_dma(zchan);
		break;
	default:
		break;
	}
	zynq_trace(ZYNQ_TRACE_CHAN, "%d ch %d %s done: zchan=%p\n",
	    ZYNQ_INST(zchan), zchan->zchan_num, __FUNCTION__, zchan);
}
