#include <linux/module.h>
#include <linux/sched.h>
#include <linux/pci.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/rtnetlink.h>
#include <linux/delay.h>
#include <linux/ethtool.h>
#include <linux/mii.h>
#include <linux/completion.h>
#include <linux/crc32.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/irq.h>

#define DRV_NAME  "8139D driver"

unsigned int use_io = 0;
module_param(use_io, uint, 0444);

static int multicast_filter_limit = 32;

#define RX_BUF_LEN  32768
#define RX_BUF_PAD  16
#define RX_BUF_WRAP_PAD 2048 /* handle buffer wrap */
#define RX_BUF_TOT_LEN  (RX_BUF_LEN + RX_BUF_PAD + RX_BUF_WRAP_PAD)

#define NUM_TX_DESC	4
#define MAX_ETH_FRAME_SIZE	1536
#define TX_BUF_TOT_LEN	(TX_BUF_SIZE * NUM_TX_DESC)

/* Symbolic offsets to registers. */
enum RTL8139_registers {
	MAC0		= 0,	 /* Ethernet hardware address. */
	MAR0		= 8,	 /* Multicast filter. */
	TxStatus0	= 0x10,	 /* Transmit status (Four 32bit registers). */
	TxAddr0		= 0x20,	 /* Tx descriptors (also four 32bit). */
	RxBuf		= 0x30,
	ChipCmd		= 0x37,
	RxBufPtr	= 0x38,
	RxBufAddr	= 0x3A,
	IntrMask	= 0x3C,
	IntrStatus	= 0x3E,
	TxConfig	= 0x40,
	RxConfig	= 0x44,
	Timer		= 0x48,	 /* A general-purpose counter. */
	RxMissed	= 0x4C,  /* 24 bits valid, write clears. */
	Cfg9346		= 0x50,
	Config0		= 0x51,
	Config1		= 0x52,
	TimerInt	= 0x54,
	MediaStatus	= 0x58,
	Config3		= 0x59,
	Config4		= 0x5A,	 /* absent on RTL-8139A */
	HltClk		= 0x5B,
	MultiIntr	= 0x5C,
	TxSummary	= 0x60,
	BasicModeCtrl	= 0x62,
	BasicModeStatus	= 0x64,
	NWayAdvert	= 0x66,
	NWayLPAR	= 0x68,
	NWayExpansion	= 0x6A,
	/* Undocumented registers, but required for proper operation. */
	FIFOTMS		= 0x70,	 /* FIFO Control and test. */
	CSCR		= 0x74,	 /* Chip Status and Configuration Register. */
	PARA78		= 0x78,
	FlashReg	= 0xD4,	/* Communication with Flash ROM, four bytes. */
	PARA7c		= 0x7c,	 /* Magic transceiver parameter register. */
	Config5		= 0xD8,	 /* absent on RTL-8139A */
};


struct rtl_extra_stats {
	unsigned long early_rx;
	unsigned long tx_buf_mapped;
	unsigned long tx_timeouts;
	unsigned long rx_lost_in_ring;
};

struct rtl8139_private {
	void __iomem		*mmio_addr;
	int			drv_flags;
	struct pci_dev		*pci_dev;
	u32			msg_enable;
	struct napi_struct	napi;
	struct net_device	*dev;

	unsigned char		*rx_ring;
	unsigned int		cur_rx;	/* RX buf index of next pkt */
	dma_addr_t		rx_ring_dma;

	unsigned int		tx_flag;
	unsigned long		cur_tx;
	unsigned long		dirty_tx;
	unsigned char		*tx_buf[NUM_TX_DESC];	/* Tx bounce buffers */
	unsigned char		*tx_bufs;	/* Tx bounce buffer region. */
	dma_addr_t		tx_bufs_dma;

	signed char		phys[4];	/* MII device addresses. */

				/* Twister tune state. */
	char			twistie, twist_row, twist_col;

	unsigned int		watchdog_fired : 1;
	unsigned int		default_port : 4; /* Last dev->if_port value. */
	unsigned int		have_thread : 1;

	spinlock_t		lock;
	spinlock_t		rx_lock;

	chip_t			chipset;
	u32			rx_config;
	struct rtl_extra_stats	xstats;

	struct delayed_work	thread;

	struct mii_if_info	mii;
	unsigned int		regs_len;
	unsigned long		fifo_copy_timeout;
};


struct pci_device_id my8139_ids[] = {
	{0x10ec, 0x8139, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0},
	{0},
};
MODULE_DEVICE_TABLE(pci, my8139_ids);


static int __devinit my_probe(struct pci_dev *pdev, const struct pci_device_id *ids)
{
	unsigned long pio_start, pio_end, pio_flags, pio_len;
	unsigned long mmio_start, mmio_end, mmio_flags, mmio_len;
	unsigned long tmp_addr, i;
	unsigned int slot;
	int rc = 0;
	int disable_dev_on_err = 0;
	struct net_device *ndev;

	printk(DRV_NAME " verdor = 0x%x; device = 0x%x\n", pdev->vendor, pdev->device);
	printk("PCI Chip slot %d, function %d\n", \
		PCI_SLOT(pdev->devfn), \
		PCI_FUNC(pdev->devfn));
	slot = PCI_SLOT(pdev->devfn);

	/* allocate net_device */
	ndev = alloc_etherdev(
	if (NULL == dev)
		return -ENOMEM;
	pci_set_drvdata(pdev, dev);
	printk("slot%d: dev address is 0x%p\n", slot, dev);


	/* enable device (incl. PCI PM wakeup and hotplug setup) */
	rc = pci_enable_device (pdev);
	if (rc) {
		kfree(dev);
		pci_set_drvdata(pdev, NULL);
		return rc;
	}

	/* get io resource */
	pio_start = pci_resource_start (pdev, 0);
	pio_end = pci_resource_end (pdev, 0);
	pio_flags = pci_resource_flags (pdev, 0);
	pio_len = pci_resource_len (pdev, 0);
	
	mmio_start = pci_resource_start (pdev, 1);
	mmio_end = pci_resource_end (pdev, 1);
	mmio_flags = pci_resource_flags (pdev, 1);
	mmio_len = pci_resource_len (pdev, 1);

	for (i=2; i<6; i++) {
		tmp_addr = pci_resource_start(pdev, i);
		printk("slot%d: bar %ld address is 0x%lx\n", slot, i, tmp_addr);
	}

	printk("slot%d: PIO region size == 0x%0lx\n", slot, pio_len);
	printk("slot%d: MMIO region size == 0x%0lx\n", slot, mmio_len);

retry:
	if (use_io) {
		/* make sure PCI base addr 0 is PIO */
		if (!(pio_flags & IORESOURCE_IO)) {
			printk("slot%d: region #0 not a PIO resource, aborting\n", slot);
			rc = -ENODEV;
			goto err_out;
		}
	} else {
		/* make sure PCI base addr 1 is MMIO */
		if (!(mmio_flags & IORESOURCE_MEM)) {
			printk("slot%d: region #1 not an MMIO resource, aborting\n", slot);
			rc = -ENODEV;
			goto err_out;
		}
	}

	/* request ioport and iomem region */
	rc = pci_request_regions (pdev, DRV_NAME);
	if (rc)
		goto err_out;
	disable_dev_on_err = 1;

	/* enable PCI bus-mastering */
	pci_set_master (pdev);

	if (use_io) {
		dev->ioaddr = pci_iomap(pdev, 0, 0);
		if (!dev->ioaddr) {
			printk("slot%d: cannot map PIO, aborting\n", slot);
			rc = -EIO;
			goto err_out;
		}
		dev->regs_len = pio_len;
		printk("slot%d: PIO addr 0x%lx; ioaddr 0x%p\n", slot, pio_start, dev->ioaddr);
	
	} else {
		/* ioremap MMIO region */
		dev->ioaddr = pci_iomap(pdev, 1, 0);
		if (dev->ioaddr == NULL) {
			printk("slot%d: cannot remap MMIO, trying PIO\n", slot);
			pci_release_regions(pdev);
			use_io = 1;
			goto retry;
		}
		dev->regs_len = mmio_len;
		printk("slot%d: IOMEM addr 0x%lx; ioaddr 0x%p\n", slot, mmio_start, dev->ioaddr);
	}

	/* register cdev */
 	devno = MKDEV(PCI_MAJOR, slot);
	cdev_init(&dev->mycdev, &my_fops);
	dev->mycdev.owner = THIS_MODULE;
	cdev_add(&dev->mycdev, devno, 1);

err_out:
	if (dev->ioaddr)
		pci_iounmap(pdev, dev->ioaddr);

	/* it's ok even we have no regions to free */
	pci_release_regions(pdev);

	pci_set_drvdata(pdev, NULL);

	if (disable_dev_on_err)
		pci_disable_device (pdev);

	return rc;
}

static void __devexit my_remove(struct pci_dev *pdev)
{
	struct mydev *dev = pci_get_drvdata(pdev);

	printk("enter my_remove, chip %x\n", pdev->devfn);

	cdev_del(&dev->mycdev);
	if (dev->ioaddr)
		pci_iounmap(pdev, dev->ioaddr);

	/* it's ok even we have no regions to free */
	pci_release_regions(pdev);
	pci_set_drvdata(pdev, NULL);
	pci_disable_device (pdev);

	/* free mydev */
	kfree(dev);
}


struct pci_driver my_pci_driver = {
	.name 		= DRV_NAME,
	.id_table 	= my8139_ids,
	.probe 		= my_probe,
	.remove 	= __devexit_p(my_remove),
};

int __init my_init(void)
{
	printk("In my module, register 8139 pci driver\n");
	return pci_register_driver(&my_pci_driver);
}

void __exit my_exit(void)
{
	pci_unregister_driver(&my_pci_driver);
}

module_init(my_init);
module_exit(my_exit);

MODULE_AUTHOR("ZHANG");
MODULE_LICENSE("GPL");
MODULE_VERSION("2.0");

