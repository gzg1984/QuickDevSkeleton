#include <linux/module.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/pci.h>
#include <linux/ioport.h>
#include <linux/io.h>
#include <asm/uaccess.h>

#define PCI_MAJOR 80
#define DRV_NAME  "8139D driver"

unsigned int use_io = 0;
module_param(use_io, uint, 0444);

struct pci_device_id my8139_ids[] = {
	{0x10ec, 0x8139, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0},
	{0},
};
MODULE_DEVICE_TABLE(pci, my8139_ids);

struct mydev {
	unsigned long regs_len;
	void __iomem *ioaddr;
	struct cdev mycdev;
};


static int my_open(struct inode *inode, struct file *filp)
{
	struct mydev *dev = container_of(inode->i_cdev, struct mydev, mycdev);
	unsigned int minor = iminor(inode);
	filp->private_data = dev;
	printk("minor%d: dev address is 0x%p\n", minor, dev);
	return 0;
}

static int my_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static ssize_t my_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	struct mydev *dev = filp->private_data;
	u8 *kbuf;

	/*get invalid read length */
	if ((*f_pos) >= dev->regs_len)
		return 0;
	count = min(count, (size_t)(dev->regs_len - (*f_pos)));
	if (count == 0)
		return 0;

	/* get iodata */
	kbuf = (u8 *)kzalloc(count, GFP_KERNEL);
	if (NULL == kbuf)
		return -ENOMEM;
	memcpy_fromio(kbuf, (dev->ioaddr + (*f_pos)), count);
	
	if (copy_to_user(buf, kbuf, count)) {
		kfree(kbuf);
		return -EFAULT;
	}

	*f_pos += count;
	kfree(kbuf);
	return count;
}

static ssize_t my_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
	struct mydev *dev = filp->private_data;
	u8 *kbuf;

	/*get invalid write length */
	if ((*f_pos) >= dev->regs_len)
		return 0;
	count = min(count, (size_t)(dev->regs_len - (*f_pos)));
	if (count == 0)
		return 0;

	kbuf = (u8 *)kzalloc(count, GFP_KERNEL);
	if (NULL == kbuf)
		return -ENOMEM;

	if (copy_from_user(kbuf, buf, count)) {
		kfree(kbuf);
		return -EFAULT;
	}

	/* set iodata */
	memcpy_toio((dev->ioaddr + (*f_pos)), kbuf, count);

	*f_pos += count;
	kfree(kbuf);

	return count;
}

static loff_t my_llseek(struct file *filp, loff_t offset, int origin)
{
	struct mydev *dev = filp->private_data;
    loff_t res;

	printk("origin = %d; offset = %lld\n", origin, offset);

    switch (origin) {
		case 1:	/* offset from current */
			offset += filp->f_pos;
			break;
        case 2: /* offset from end */
			offset += dev->regs_len;
			break;
	}

	if ((offset < 0) || (offset >= dev->regs_len)) {
		res = -EINVAL;
		goto out;
        }

	filp->f_pos = offset;
	res = filp->f_pos;
out:
	return res;
}

static struct file_operations my_fops = {
	.owner 	= THIS_MODULE,
	.open	= my_open,
	.release = my_release,
	.read	= my_read,
	.write	= my_write,
	.llseek	= my_llseek,
};


static int my_probe(struct pci_dev *pdev, const struct pci_device_id *ids)
{
	unsigned long pio_start, pio_end, pio_flags, pio_len;
	unsigned long mmio_start, mmio_end, mmio_flags, mmio_len;
	unsigned long tmp_addr, i;
	unsigned int slot;
	int rc = 0;
	int disable_dev_on_err = 0;
	struct mydev *dev;
	dev_t devno;

	printk(DRV_NAME " verdor = 0x%x; device = 0x%x\n", pdev->vendor, pdev->device);
	printk("PCI Chip slot %d, function %d\n", \
		PCI_SLOT(pdev->devfn), \
		PCI_FUNC(pdev->devfn));
	slot = PCI_SLOT(pdev->devfn);

	/* allocate struct mydev */
	dev = (struct mydev *)kzalloc(sizeof(*dev), GFP_KERNEL);
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

static void my_remove(struct pci_dev *pdev)
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
	.remove 	= my_remove,
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

