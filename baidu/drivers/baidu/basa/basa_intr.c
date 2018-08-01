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

#include "basa.h"

/* channel ISR for handling Rx, Rx error, Tx and Tx error */
static irqreturn_t zynq_intr_chan(int irq, void *datap)
{
	zynq_chan_t *zchan = datap;
	u32 ch = zchan->zchan_num;

#ifdef ZYNQ_INTR_PROC_TASKLET
	tasklet_hi_schedule(&zchan->zdev->zdev_ta[ch]);
#else
	zynq_chan_tasklet((unsigned long)zchan);
#endif

	return IRQ_HANDLED;
}

static irqreturn_t zynq_intr_fwupdate(zynq_chan_t *zchan)
{
	return IRQ_HANDLED;
}

/* ISR to handle all the channel interrupts */
static irqreturn_t zynq_intr(int irq, void *datap)
{
	zynq_dev_t *zdev = datap;
	zynq_chan_t *zchan;

	zchan = zdev->zdev_chans;
	if (zynq_fwupdate_param) {
		return zynq_intr_fwupdate(zchan);
	}

	ZYNQ_STATS(zdev, DEV_STATS_INTR);

#ifdef ZYNQ_INTR_PROC_TASKLET
	tasklet_hi_schedule(&zchan->zdev->zdev_ta[0]);
#else
	zynq_tasklet((unsigned long)zdev);
#endif

	return IRQ_HANDLED;
}

/* enable MSI */
static int zdev_enable_msi(zynq_dev_t *zdev)
{
	int max_nvec = 1;
	int rc;

	/*
	 * per channel interrupt is not supported for now
	 * due to an FPGA issue
	 */
msi_retry:
	rc = pci_enable_msi_exact(zdev->zdev_pdev, max_nvec);
	if (rc < 0) {
		if (max_nvec == 1) {
			zynq_trace(ZYNQ_TRACE_PROBE,
			    "%s: failed to enable MSI\n", __FUNCTION__);
			return rc;
		}
		zynq_trace(ZYNQ_TRACE_PROBE,
		    "%s: failed to enable MSI nvec=%d, retry nvec=1\n",
		    __FUNCTION__, max_nvec);
		max_nvec = 1;
		goto msi_retry;
	}
	zdev->zdev_msi_num = max_nvec;
	zdev->zdev_msi_vec = zdev->zdev_pdev->irq;

	zynq_trace(ZYNQ_TRACE_PROBE, "%s: enabled %d MSI(s).\n",
	    __FUNCTION__, zdev->zdev_msi_num);
	return 0;
}

/* enable MSI-X */
static int zdev_enable_msix(zynq_dev_t *zdev)
{
	int max_nvec;
	int rc;

	if (zynq_fwupdate_param) {
		max_nvec = 1;
	} else {
		max_nvec = zdev->zdev_chan_cnt;
	}

	zdev->zdev_msixp = kzalloc(sizeof(struct msix_entry) * max_nvec,
	    GFP_KERNEL);
	if (zdev->zdev_msixp == NULL) {
		return -ENOMEM;
	}
	/*
	 * We support 2 cases:
	 *	1. One interrupt per DMA channel;
	 *	2. One interrupt per device.
	 */
msix_retry:
	rc = pci_enable_msix(zdev->zdev_pdev, zdev->zdev_msixp, max_nvec);
	if (rc < 0) {
		zynq_trace(ZYNQ_TRACE_PROBE,
		    "%s: failed to enable MSI-X nvec=%d\n",
		    __FUNCTION__, max_nvec);
		kfree(zdev->zdev_msixp);
		zdev->zdev_msixp = NULL;
		return rc;
	} else if (rc > 0) {
		zynq_trace(ZYNQ_TRACE_PROBE,
		    "%s: failed to enable MSI-X nvec=%d, retry nvec=1\n",
		    __FUNCTION__, max_nvec);
		max_nvec = 1;
		goto msix_retry;
	}
	zdev->zdev_msix_num = max_nvec;

	zynq_trace(ZYNQ_TRACE_PROBE, "%s: enabled %d MSI-X(s).\n",
	    __FUNCTION__, zdev->zdev_msix_num);
	return 0;
}

static int zdev_setup_msix(zynq_dev_t *zdev)
{
	zynq_chan_t *zchan;
	int i = 0;
	int rc = 0;

	zchan = zdev->zdev_chans;
	if (zdev->zdev_msix_num == 1) {
		rc = request_irq(zdev->zdev_msixp[0].vector, zynq_intr,
			0, ZYNQ_DRV_NAME, zdev);
		if (rc) {
			return (rc);
		}
	} else {
		for (i = 0; i < zdev->zdev_chan_cnt; i++, zchan++) {
			rc = request_irq(zdev->zdev_msixp[i].vector,
			    zynq_intr_chan, 0, ZYNQ_DRV_NAME, zchan);
			if (rc) {
				goto msix_fail;
			}
		}
	}

	return 0;

msix_fail:
	while (i) {
		i--;
		zchan--;
		free_irq(zdev->zdev_msixp[i].vector, zchan);
	}
	return rc;
}

static int zdev_setup_msi(zynq_dev_t *zdev)
{
	zynq_chan_t *zchan;
	int i = 0;
	int rc = 0;

	zchan = zdev->zdev_chans;
	if (zdev->zdev_msi_num == 1) {
		rc = request_irq(zdev->zdev_msi_vec, zynq_intr,
			0, ZYNQ_DRV_NAME, zdev);
		if (rc) {
			return (rc);
		}
	} else {
		for (i = 0; i < zdev->zdev_chan_cnt; i++, zchan++) {
			rc = request_irq(zdev->zdev_msi_vec + i,
			    zynq_intr_chan, 0, ZYNQ_DRV_NAME, zchan);
			if (rc) {
				goto msi_fail;
			}
		}
	}

	return 0;

msi_fail:
	while (i) {
		i--;
		zchan--;
		free_irq(zdev->zdev_msi_vec + i, zchan);
	}
	return rc;
}

#ifdef ZYNQ_INTR_PROC_TASKLET
static void zdev_init_tasklet(zynq_dev_t *zdev)
{
	zynq_chan_t *zchan;
	int intr_num;
	int i;

	/* legacy interrupt */
	if (zdev->zdev_msixp == NULL && zdev->zdev_msi_num == 0) {
		tasklet_init(&zdev->zdev_ta[0], zynq_tasklet,
		    (unsigned long)zdev);
		return;
	}

	if (zdev->zdev_msixp) {
		intr_num = zdev->zdev_msix_num; /* MSI-X */
	} else {
		intr_num = zdev->zdev_msi_num; /* MSI */
	}

	/* init tasklets according to the allocated interrupt number */
	zchan = zdev->zdev_chans;
	if (intr_num == 1) {
		tasklet_init(&zdev->zdev_ta[0], zynq_tasklet,
		    (unsigned long)zdev);
	} else {
		for (i = 0; i < zdev->zdev_chan_cnt; i++, zchan++) {
			tasklet_init(&zdev->zdev_ta[i],
			    zynq_chan_tasklet, (unsigned long)zchan);
		}
	}
}

static void zdev_fini_tasklet(zynq_dev_t *zdev)
{
	int intr_num;
	int i;

	/* legacy interrupt */
	if (zdev->zdev_msixp == NULL && zdev->zdev_msi_num == 0) {
		tasklet_kill(&zdev->zdev_ta[0]);
		return;
	}

	if (zdev->zdev_msixp) {
		intr_num = zdev->zdev_msix_num; /* MSI-X */
	} else {
		intr_num = zdev->zdev_msi_num; /* MSI */
	}
	/* interrupt number per channal */
	for (i = 0; i < intr_num; i++) {
		tasklet_kill(&zdev->zdev_ta[i]);
	}
}
#endif


/*
 * Allocate and enabling interrupt: try MSI-X first, then MSI, last
 * is legacy inerrupt.
 *    16 MSI/MSI-X:
 *	[15:0] per channel interrupts
 *    1 MSI/MSI-X/Legacy interrupt:
 *	[0] per card interrupts for all the channels
 */
int zynq_alloc_irq(zynq_dev_t *zdev)
{
	int rc;

	/* try MSI-X first */
	if (zdev_enable_msix(zdev)) {
		/* failed, try MSI next */
		(void) zdev_enable_msi(zdev);
	}

#ifdef ZYNQ_INTR_PROC_TASKLET
	/* tasklets have to be initialized before requesting irqs */
	zdev_init_tasklet(zdev);
#endif

	if ((zdev->zdev_msixp == NULL) && (zdev->zdev_msi_num == 0)) {
		/* Both MSI-X and MSI failed: try legacy interrupt */
		rc = request_irq(zdev->zdev_pdev->irq, zynq_intr,
		    IRQF_SHARED, ZYNQ_DRV_NAME, zdev);
		if (rc) {
			return rc;
		}
	} else if (zdev->zdev_msixp) {
		/* MSI-X */
		rc = zdev_setup_msix(zdev);
		if (rc) {
			pci_disable_msix(zdev->zdev_pdev);
			kfree(zdev->zdev_msixp);
			zdev->zdev_msixp = NULL;
			zdev->zdev_msix_num = 0;
			return rc;
		}
	} else {
		/* MSI */
		rc = zdev_setup_msi(zdev);
		if (rc) {
			pci_disable_msi(zdev->zdev_pdev);
			zdev->zdev_msi_num = 0;
			return rc;
		}
	}

	zynq_trace(ZYNQ_TRACE_PROBE, "%s: succeeded.\n", __FUNCTION__);

	return 0;
}

/*
 * Disable and free interrupt
 */
void zynq_free_irq(zynq_dev_t *zdev)
{
	zynq_chan_t *zchan;
	u32 vector;
	int i;

	zynq_trace(ZYNQ_TRACE_PROBE, "%s: enter\n", __FUNCTION__);

	if (zdev->zdev_msixp == NULL && zdev->zdev_msi_num == 0) {
		/* legacy interrupt */
		free_irq(zdev->zdev_pdev->irq, zdev);
	} else if (zdev->zdev_msixp) {
		/* MSI-X */
		if (zdev->zdev_msix_num == 1) {
			free_irq(zdev->zdev_msixp[0].vector, zdev);
		} else {
			zchan = zdev->zdev_chans;
			for (i = 0; i < zdev->zdev_chan_cnt; i++, zchan++) {
				vector = zdev->zdev_msixp[i].vector;
				free_irq(vector, zchan);
			}

		}
		pci_disable_msix(zdev->zdev_pdev);
	} else {
		/* MSI */
		if (zdev->zdev_msi_num == 1) {
			free_irq(zdev->zdev_msi_vec, zdev);
		} else {
			zchan = zdev->zdev_chans;
			for (i = 0; i < zdev->zdev_chan_cnt; i++, zchan++) {
				vector = zdev->zdev_msi_vec + i;
				free_irq(vector, zchan);
			}
		}
		pci_disable_msi(zdev->zdev_pdev);
	}

#ifdef ZYNQ_INTR_PROC_TASKLET
	/* kill the tasklets after irqs are freed */
	zdev_fini_tasklet(zdev);
#endif

	if (zdev->zdev_msixp) {
		kfree(zdev->zdev_msixp);
		zdev->zdev_msixp = NULL;
		zdev->zdev_msix_num = 0;
	}
	zdev->zdev_msi_num = 0;

	zynq_trace(ZYNQ_TRACE_PROBE, "%s: done\n", __FUNCTION__);
}
