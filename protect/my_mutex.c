#include <linux/module.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/cdev.h>

#include <asm/uaccess.h>

#define BUF_LEN		100
#define MY_MAJOR	50

static int major = MY_MAJOR;
module_param(major, int, 0444);

struct mydev { 
	unsigned char *start, *end;
	unsigned char *wp, *rp;
	struct semaphore my_sem;
	wait_queue_head_t readq, writeq;
	struct cdev mycdev;
};

static struct mydev *dev;

int my_open(struct inode *inode, struct file *filp)
{
	struct mydev *dev_priv = container_of(inode->i_cdev, struct mydev, mycdev); 	
	filp->private_data = dev_priv;

	return 0;
}

int my_release(struct inode *inode, struct file *filp)
{
	return 0;
}

ssize_t my_read(struct file *filp, char __user *buf, size_t count, loff_t *pos)
{	struct mydev *priv = filp->private_data;
	//down, down_interruptible
	//wait_event
	//copy_to_user
	//wake_up
	//up
	return count;
}

ssize_t my_write(struct file *filp, const char __user *buf, size_t count, loff_t *pos)
{
	struct mydev *priv = filp->private_data;

	return count;
}


static struct file_operations my_fops = {
	.owner 		= THIS_MODULE,
	.open		= my_open,
	.release	= my_release,
	.read		= my_read,
	.write		= my_write,
};


static int __init my_init(void)
{
	int ret;
	dev_t dev_num;

	/* allocate struct mydev */
	dev = (struct mydev *)kzalloc(sizeof(*dev), GFP_KERNEL);
	if (NULL == dev) 
		return -ENOMEM;

	/* allocate buffer */
	dev->start = (unsigned char *)kzalloc(BUF_LEN, GFP_KERNEL);
	if (NULL == dev->start) {
		kfree(dev);
		return -ENOMEM;
	}
	dev->end = dev->start + BUF_LEN;
	dev->wp  = dev->rp = dev->start;

	/* initialize mutex and wait queue */
	init_MUTEX(&dev->my_sem);
	init_waitqueue_head(&dev->readq);
	init_waitqueue_head(&dev->writeq);


	/* allocate chrdev region */
	if (major) {
		dev_num = MKDEV(major, 0);
		ret = register_chrdev_region(dev_num, 20, "static_char");
	} else {
		ret = alloc_chrdev_region(&dev_num, 0, 20, "dynamic_char");
		major = MAJOR(dev_num);
	}
	if (ret) {
		kfree(dev->start);
		kfree(dev);
		return -1;
	}

	/* cdev initialize and register */
	cdev_init(&dev->mycdev, &my_fops);
	dev->mycdev.owner = THIS_MODULE;
	cdev_add(&dev->mycdev, dev_num, 1);

	return 0;
}


static void __exit my_exit(void)
{
	dev_t dev_num = MKDEV(major, 0);

	cdev_del(&dev->mycdev);
	unregister_chrdev_region(dev_num, 20);
	kfree(dev->start);
	kfree(dev);
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ZHANG");
MODULE_DESCRIPTION("Simple driver for memory device");
MODULE_VERSION("0.2");







