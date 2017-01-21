#include <linux/module.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/pci.h>
#include <asm/io.h>
#include <asm/uaccess.h>

#define PCI_MAJOR 80
#define DRV_NAME  "My own 8139D driver"

unsigned int use_io = 0;
module_param(use_io, uint, 0444);

struct pci_device_id my8139_ids[] = {
	{0x10ec, 0x8139, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0},
	{0},
};
MODULE_DEVICE_TABLE(pci, my8139_ids);

struct mydev {
	unsigned long phy_port;
	unsigned long phy_iomem;
	unsigned long reg_len;
	void __iomem *ioaddr;
	struct cdev mycdev;
};


static int my_open(struct inode *inode, struct file *filp)
{
	struct mydev *dev = container_of(inode->i_cdev, struct mydev, mycdev);
	filp->private_data = dev;
	return 0;
}

static int my_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static ssize_t my_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	struct mydev *dev = filp->private_data;

	/*get invalid read length */
	if ((*f_pos) >= dev->reg_len)
		return 0;
	count = min(count, (size_t)(dev->reg_len - (*f_pos)));
	if (count <= 0)
		return 0;

	if (copy_to_user(buf, ((unsigned char *)dev->ioaddr + (*f_pos)), count))
		return -EFAULT;

	*f_pos += count;
	return count;
}

static ssize_t my_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
	struct mydev *dev = filp->private_data;

	/*get invalid write length */
	if ((*f_pos) >= dev->reg_len)
		return 0;
	count = min(count, (size_t)(dev->reg_len - (*f_pos)));
	if (count <= 0)
		return 0;

	if (copy_from_user(((unsigned char *)dev->ioaddr + (*f_pos)), buf, count))
		return -EFAULT;

	*f_pos += count;
	return count;
}

static loff_t my_llseek(struct file *filp, loff_t offset, int origin)
{
	struct mydev *dev = filp->private_data;
    loff_t res;

	printk("origin = %d; offset = %lld\n", origin, offset);

    switch (origin) {
		case 1:	/* 相对文件当前位置的偏移 */
			offset += filp->f_pos;
			break;
        case 2: /* 相对文件结尾位置的偏移 */
			offset += dev->buf_size;
			break;
	}
	/* 偏移量小于0或者超过缓冲区范围返回错误值 */
	if ((offset < 0) || (offset > dev->buf_size)) {
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


static int __devinit my_probe(struct pci_dev *pdev, const struct pci_device_id *ids)
{
	printk("Enter my_probe, time is %ld\n", jiffies);
	
	printk("pci_dev->devfn = 0x%x\n", pdev->devfn);
	printk("pci_dev->vendor = 0x%x\n", pdev->vendor);
	printk("pci_dev->device = 0x%x\n", pdev->device);
	printk("pci_dev->subsystem_vendor = 0x%x\n", pdev->subsystem_vendor);
	printk("pci_dev->subsystem_device = 0x%x\n", pdev->subsystem_device);
	printk("pci_dev->class = 0x%x\n", pdev->class);
	printk("pci_dev->revision = 0x%x\n", pdev->revision);
	printk("pci_dev->hdr_type = 0x%x\n", pdev->hdr_type);
	printk("pci_dev->pcie_type = 0x%x\n", pdev->pcie_type);
	printk("pci_dev->rom_base_reg = 0x%x\n", pdev->rom_base_reg);
	printk("pci_dev->pin = 0x%x\n", pdev->pin);
	printk("pci_dev->irq = 0x%x\n", pdev->irq);

	//pci_enable_device
	//get 8139 ioport and iomem address
	//pci_request_region
	//pci_iomap
	//pci_set_master
	//cdev_init
	//cdev_add
}

static void __devexit my_remove(struct pci_dev *pdev)
{
	printk("Enter my_remove, time is %ld\n", jiffies);
	//pci_iounmap
	//pci_release_region
	//pci_disable_device
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

