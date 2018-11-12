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

#ifndef _BASA_CHAN_H_
#define	_BASA_CHAN_H_

struct zynq_dev;

#define	ZCHAN_ERR_THROTTLE	100 /* msec */

#define	ZYNQ_CHAN_INT_RX_INDEX		0
#define	ZYNQ_CHAN_INT_RXERR_INDEX	1
#define	ZYNQ_CHAN_INT_TX_INDEX		2
#define	ZYNQ_CHAN_INT_TXERR_INDEX	3
#define	ZYNQ_CHAN_INT_RXALL_INDEX	0
#define	ZYNQ_CHAN_INT_TXALL_INDEX	1

#define	ZYNQ_CHAN_PROC_TX	(1<<0)
#define	ZYNQ_CHAN_PROC_TX_ERR	(1<<1)
#define	ZYNQ_CHAN_PROC_RX	(1<<2)
#define	ZYNQ_CHAN_PROC_RX_ERR	(1<<3)

#define	ZCHAN_BUF_SIZE		4096

/* channel buffer structure */
typedef struct zchan_buf {
	void			*zchan_bufp;	/* buffer virtual address */
	/* Keep the page structure for >=4K data buffer */
	struct page		*zchan_buf_page; /* used for >=4K buffer */
	dma_addr_t		zchan_buf_dma;	/* buffer physical address */
} zchan_buf_t;

#define	ZCHAN_TX_ENTRIES_CAN	1024

/* Tx descriptor for DMA read channel */
typedef struct zchan_tx_desc {
	u32			reserved;
	u32			tx_len;		/* tx data size in bytes */
	u64			tx_addr;
} zchan_tx_desc_t;

/* Tx descriptor ring */
typedef struct zchan_tx_ring {
	struct zynq_chan	*zchan;		/* backlink to zynq chan */
	spinlock_t		zchan_tx_lock;  /* tx lock */
	/* virtual address of the descriptor ring */
	zchan_tx_desc_t		*zchan_tx_descp;
	/* physical address of the descriptor ring */
	dma_addr_t		zchan_tx_dma;
	u32			zchan_tx_size;  /* size of the ring in bytes */
	u32			zchan_tx_num; /* # of descriptors in the ring */
	u32			zchan_tx_head;  /* consumer index of the ring */
	u32			zchan_tx_tail;  /* producer index of the ring */
	/* associated buffer array with zchan_tx_num entries */
	u32			zchan_tx_bufsz;	/* Tx buffer size */
	zchan_buf_t		*zchan_tx_bufp;
} zchan_tx_ring_t;

/*
 * For Rx, two level page tables are used to address large amount of data.
 * Driver sets up Page Directory Table (PDT), Page Table (PT) and allocates
 * Memory Pages (MP). We also provide Page Directory Table (PDT) start address
 * and number of valid entries to H/W. For PDT and each PT, valid entries are
 * in contiguous addresses. H/W will lookup PDT followed by PT to get memory
 * page address, and will send received data to corresponding memory pages.
 * After data transfer is done, H/W provides MP tail pointer to indicate valid
 * entries available for driver to process via interrupt, and driver will
 * update head pointer to indicate entries processed. H/W writes data to memory
 * pages starting from offset 0 in strictly increasing sequences. From number
 * of valid entries offset in memory pages, we can trace back table indices
 * in PDT and PT tables.
 */
/* page directory table size */
#define ZCHAN_RX_PDT_ENTRIES_CAN	1 /* 2M-byte Rx buffer */
/* page table size */
#define	ZCHAN_RX_PT_SIZE		4096 /* don't change to other value */
/* page table entries: ZCHAN_RX_PT_SIZE/8 */
#define	ZCHAN_RX_PT_ENTRIES		512

/* Rx page table structure */
typedef struct zchan_rx_pt {
	u64			*zchan_rx_pt; /* pt virtual address */
	dma_addr_t		zchan_rx_pt_dma;
	/* assocated buffer for each page table entry */
	zchan_buf_t		*zchan_rx_pt_bufp;
	u32			zchan_rx_pt_buf_num; /* number of buffers */
} zchan_rx_pt_t;

/* Rx channel structure */
typedef struct zchan_rx_tbl {
	struct zynq_chan	*zchan;		/* backlink to zynq chan */
	spinlock_t		zchan_rx_lock;	/* rx lock */

	/* page directory table related */
	u64			*zchan_rx_pdt;	/* pdt virtual address */
	dma_addr_t		zchan_rx_pdt_dma; /* pdt physical address */
	u32			zchan_rx_pdt_num; /* # of pt entries */
	u32			zchan_rx_pdt_shift;

	/* page table related: buffer included */
	zchan_rx_pt_t		*zchan_rx_ptp; /* zchan_rx_pdt_num entries */
	u32			zchan_rx_pt_entries;
	u32			zchan_rx_pt_mask;
	u32			zchan_rx_pt_shift;

	/*
	 * size of Rx buffer space:
	 *	zchan_rx_pdt_num * ZCHAN_RX_PT_ENTRIES * zcan_rx_bufsz
	 */
	u32			zchan_rx_size;
	u32			zchan_rx_bufsz;	/* Rx buffer size */
	u32			zchan_rx_buf_mask;
	u32			zchan_rx_off;
	u32			zchan_rx_head; /* consumer offset */
	u32			zchan_rx_tail; /* producer offset */
} zchan_rx_tbl_t;

#define	ZCHAN_WITHIN_RX_BUF(zchan_rx, rx_off1, rx_off2)	\
		((rx_off1 & (~zchan_rx->zchan_rx_buf_mask)) == \
		(rx_off2 & (~zchan_rx->zchan_rx_buf_mask)))
#define	ZCHAN_RX_BUF_OFFSET(zchan_rx, rx_off)	\
		(rx_off & zchan_rx->zchan_rx_buf_mask)
#define	ZCHAN_RX_PT_ENTRY(zchan_rx, rx_off)	\
		((rx_off >> zchan_rx->zchan_rx_pt_shift) & \
		 zchan_rx->zchan_rx_pt_mask)
#define	ZCHAN_RX_PDT_ENTRY(zchan_rx, rx_off)	\
		((rx_off >> zchan_rx->zchan_rx_pdt_shift) % \
		 zchan_rx->zchan_rx_pdt_num)
#define	ZCHAN_RX_FREE_SIZE(zchan_rx, head, tail)	\
		((head > tail) ? (head - tail) :	\
		(zchan_rx->zchan_rx_size + head - tail))
#define	ZCHAN_RX_USED_SIZE(zchan_rx, head, tail)	\
		((tail >= head) ? (tail - head) :	\
		(zchan_rx->zchan_rx_size + tail - head))

#define	ZYNQ_INST(zchan)	(zchan->zdev->zdev_inst)

/*
 * Zynq channel common structure:
 *	each Zynq channel contains a DMA read channel and a DMA write channel.
 */
typedef struct zynq_chan {
	/*
	 * Channel common definitions
	 */
	struct zynq_dev		*zdev;		/* backlink to zynq dev */
	void			*zchan_dev;	/* channel specific device */
	enum zynq_chan_type	zchan_type;
	int			zchan_num;
	spinlock_t		zchan_lock;	/* channel lock */

	/* VA base for channel configuration and status registers. */
	u8 __iomem		*zchan_reg;

	/* DMA read channel */
	zchan_tx_ring_t		zchan_tx_ring;

	/* DMA write channel */
	zchan_rx_tbl_t		zchan_rx_tbl;

	/* Channel statistics */
	zynq_stats_t		stats[CHAN_STATS_NUM];
	char			prefix[ZYNQ_LOG_PREFIX_LEN];

	struct completion	watchdog_completion;
	struct task_struct	*watchdog_taskp;
	unsigned int		watchdog_interval;
	unsigned long		ts_err[32];
} zynq_chan_t;

extern void zchan_fini(zynq_chan_t *zchan);
extern int zchan_init(zynq_chan_t *zchan);
extern int zchan_tx_one_msg(zynq_chan_t *zchan, void *msg, u32 msgsz);
extern void zchan_tx_done(zchan_tx_ring_t *zchan_tx);
extern void zchan_rx_off2addr(zchan_rx_tbl_t *zchan_rx, u32 rx_off,
    void **virt_addr, void **phy_addr);
extern void zchan_rx_start(zynq_chan_t *zchan);
extern void zchan_rx_stop(zynq_chan_t *zchan);
extern void *zchan_alloc_consistent(struct pci_dev *pdev,
    dma_addr_t *dmap, size_t sz);
extern void zchan_free_consistent(struct pci_dev *pdev, void *ptr,
    dma_addr_t dma, size_t sz);
extern void zchan_err_mask(zynq_chan_t *zchan, uint32_t ch_err);
extern void zchan_watchdog_complete(zynq_chan_t *zchan);

#endif	/* _BASA_CHAN_H_ */
