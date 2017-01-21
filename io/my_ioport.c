#include <linux/module.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/ioport.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#define MY_MAJOR  60

unsigned long port = 0xe400;
module_param(port, ulong, 0444);
unsigned long port_size = 0x100;
module_param(port_size, ulong, 0444);

struct cdev mycdev;
struct resource *res;

int my_open(struct inode *inode, struct file *filp)
{
	filp->private_data = (void *)port;
	return 0;
}

int my_release(struct inode *inode, struct file *filp)
{
	return 0;
}

ssize_t my_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{	
	unsigned long *kbuf;
	int i;
	if (count & 0x3) {
		printk("Please read 8139 register 4 bytes align\n");
		return -1;
	}

	count = min(count, (size_t)(port_size - *f_pos));
	if (count == 0)
		return count;

	kbuf = (unsigned long *)kzalloc(count, GFP_KERNEL);
	if (NULL == kbuf)
		return -ENOMEM;

	for (i=0; i<count; i+=4)
		kbuf[i/4] = inl(port + *f_pos + i);

	if (copy_to_user(buf, kbuf, count)) {
		kfree(kbuf);
		return -EFAULT;
	}	
	
	kfree(kbuf);
	*f_pos += count;
	return count;
}

struct file_operations my_fops = {
	.owner 	= THIS_MODULE,
	.open 	= my_open,
	.release= my_release,
	.read	= my_read,
};

int __init my_init(void)
{
	int ret;
	dev_t devno;

	/* get ioport region */
	res = request_region(port, port_size, "my 8139 port");
	if (NULL == res) {
		printk("Cannot request region 0x%lx to 0x%lx, just use it.\n", port, (port+port_size-1));
	}

	/* register char device number */
	devno = MKDEV(MY_MAJOR, 0);
	ret = register_chrdev_region(devno, 1, "8139 char");
	if (ret) {
		if (res)
			release_region(port, port_size);
		printk("Cannot allocate chrdev num: 0x%lx\n", (unsigned long)devno);
		return -1;
	}

	/* add cdev */
	cdev_init(&mycdev, &my_fops);
	mycdev.owner = THIS_MODULE;
	cdev_add(&mycdev, devno, 1);

	return 0;
}

void __exit my_exit(void)
{
	dev_t devno = MKDEV(MY_MAJOR, 0);
	cdev_del(&mycdev);
	unregister_chrdev_region(devno, 1);
	if (res)
		release_region(port, port_size);
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ZHANG");
MODULE_VERSION("0.8");
