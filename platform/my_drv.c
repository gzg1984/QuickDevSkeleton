#include <linux/module.h>
#include <linux/sched.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <asm/io.h>
#include <asm/uaccess.h>

#define DRV_NAME "key"
#define MY_MAJOR 80

struct mydev {
	unsigned long irq;
	void __iomem *ioaddr;
	struct cdev mycdev;
};

struct class *my_class;

int my_open(struct inode *inode, struct file *filp)
{
	return 0;
}


int my_release(struct inode *inode, struct file *filp)
{
	return 0;
}

ssize_t my_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	return 0;
}

struct file_operations my_fops = {
	.owner = THIS_MODULE,
	.open = my_open,
	.release = my_release,
	.read = my_read,
};


int my_probe(struct platform_device *pdev)
{
	struct mydev *dev;
	struct resource *res;
	dev_t devno;


	printk("%s id%d probe, in %ld\n", DRV_NAME, pdev->id, jiffies);
	
	dev = (struct mydev *)kmalloc(sizeof(*dev), GFP_KERNEL| __GFP_ZERO);
	if (NULL == dev)
		return -ENOMEM;
	platform_set_drvdata(pdev, dev);

	/* get irq */
	res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	dev->irq = res->start;
	printk("%s id%d irq = %ld\n", DRV_NAME, pdev->id, dev->irq);

	/* get ioport */
	res = platform_get_resource(pdev, IORESOURCE_IO, 0);
	printk("%s id%d ioport = %llx\n", DRV_NAME, pdev->id, res->start);
	/* ioport map */
	dev->ioaddr = ioport_map(res->start, (res->end - res->start + 1));
	printk("%s id%d ioaddr = 0x%p\n", DRV_NAME, pdev->id, dev->ioaddr);

	devno = MKDEV(MY_MAJOR, pdev->id);
	cdev_init(&dev->mycdev, &my_fops);
	dev->mycdev.owner = THIS_MODULE;
	cdev_add(&dev->mycdev, devno, 1);

	/* create file in /sys/class/key_class */
	device_create(my_class, /* class * */
				&pdev->dev,		/* parent */
				devno,		/* dev_t */
				dev,		/* drvdata */
				"key-%d", pdev->id);	/* device name */

	return 0;
}

int my_remove(struct platform_device *pdev)
{
	struct mydev *dev = platform_get_drvdata(pdev);
	dev_t devno = MKDEV(MY_MAJOR, pdev->id);

	printk("%s id%d remove, in %ld\n", DRV_NAME, pdev->id, jiffies);

	device_destroy(my_class, devno);
	cdev_del(&dev->mycdev);
	iounmap(dev->ioaddr);
	kfree(dev);	

	return 0;
}

struct platform_driver my_drv = {
	.probe = my_probe,
	.remove = my_remove,
	.driver = {
		.name = DRV_NAME,
		.owner = THIS_MODULE,
	},
};

int __init my_init(void)
{
	my_class = class_create(THIS_MODULE, "key_class");
	if (IS_ERR(my_class))
		return PTR_ERR(my_class);
	return platform_driver_register(&my_drv);
}

void __exit my_exit(void)
{
	platform_driver_unregister(&my_drv);
	class_destroy(my_class);
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPLv2");
MODULE_AUTHOR("gzg1984@aliyun.com");
MODULE_VERSION("0.1");


