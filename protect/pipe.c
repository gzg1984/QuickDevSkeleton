#include <linux/module.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/cdev.h>
#include <linux/semaphore.h>
#include <asm/uaccess.h>

#define BUF_LEN  100
#define MY_MAJOR 50

#define PIPE 'z'
#define PIPE_RESET _IO(PIPE, 1)


static int major = MY_MAJOR;
module_param(major, int, 0444);

struct mydev {
	char *start, *end;
	char *wp, *rp;
	int nreaders, nwriters;
	wait_queue_head_t readq, writeq;
	struct semaphore mysem;
	struct cdev mycdev;
};

static struct mydev *dev;

static int my_open(struct inode *inode, struct file *filp)
{
	struct mydev *dev = container_of(inode->i_cdev, struct mydev, mycdev);
	filp->private_data = dev;
	if (filp->f_mode & FMODE_READ)
		dev->nreaders++;
	if (filp->f_mode & FMODE_WRITE)
		dev->nwriters++;
	return 0;
}

static int my_release(struct inode *inode, struct file *filp)
{
	struct mydev *dev = filp->private_data;
	if (filp->f_mode & FMODE_READ)
		dev->nreaders--;
	if (filp->f_mode & FMODE_WRITE)
		dev->nwriters--;
	return 0;
}

static ssize_t my_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	struct mydev *dev = filp->private_data;
	if (down_interruptible(&dev->mysem))
		return -ERESTARTSYS;

	/* wait until dev->wp != dev->rp, and get mutex */
	while (dev->wp == dev->rp) {
		up(&dev->mysem);
		if (wait_event_interruptible(dev->readq, (dev->wp != dev->rp)))
			return -ERESTARTSYS;
		if (down_interruptible(&dev->mysem))
			return -ERESTARTSYS;
	}

	/* calculate read count */
	if (dev->wp > dev->rp)
		count = min(count, (size_t)(dev->wp - dev->rp));
	if (dev->wp < dev->rp)
		count = min(count, (size_t)(dev->end - dev->rp));

	if (copy_to_user(buf, dev->rp, count)) {
		up(&dev->mysem);
		return -EFAULT;
	}
	
	dev->rp += count;
	if (dev->rp == dev->end)
		dev->rp = dev->start;

	up(&dev->mysem);
	wake_up(&dev->writeq);
	return count;
}

static size_t spacefree(struct mydev *dev)
{
	if (dev->rp == dev->wp)
		return (size_t)(dev->end - dev->start - 1);
	if (dev->rp < dev->wp)
		return (size_t)(BUF_LEN + dev->rp - dev->wp - 1);
	else // (dev->rp > dev->wp)
		return (size_t)(dev->rp - dev->wp - 1);
}

static ssize_t my_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
	struct mydev *dev = filp->private_data;

	/* get mutex */
	if (down_interruptible(&dev->mysem))
		return -ERESTARTSYS;

	/* wait until buffer not empty */
	while (spacefree(dev) == 0) {
		up(&dev->mysem);
		if (wait_event_interruptible(dev->writeq, spacefree(dev)))
			return -ERESTARTSYS;
		if (down_interruptible(&dev->mysem))
			return -ERESTARTSYS;
	}

	count = min(count, spacefree(dev));
	if (dev->rp > dev->wp)
		count = min(count, (size_t)(dev->rp - dev->wp -1));
	if (dev->rp <= dev->wp)
		count = min(count, (size_t)(dev->end - dev->wp));
	
	if (copy_from_user(dev->wp, buf, count)) {
		up(&dev->mysem);
		return -EFAULT;
	}

	dev->wp += count;
	if (dev->wp == dev->end)
		dev->wp = dev->start;

	up(&dev->mysem);
	wake_up(&dev->readq);
	return count;
}

static int my_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct mydev *dev = filp->private_data;

	printk("Cmd = 0x%x\n", cmd);
	if (_IOC_TYPE(cmd) != PIPE)
		return -ENOTTY;

	if (cmd == PIPE_RESET) {
		if (down_interruptible(&dev->mysem))
			return -ERESTARTSYS;
		dev->rp = dev->wp = dev->start;
	} else {
		printk("Cannot recognize this command\n");
		return -EINVAL;
	}
	up(&dev->mysem);
	return 0;
}


static int my_proc(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int ret = 0;
	ret += sprintf(page+ret, "------Buffer-----\n");
	ret += sprintf(page+ret, "Readers: %d ; Writers: %d\n", dev->nreaders, dev->nwriters);
	ret += sprintf(page+ret, "Start from 0x%p, end in 0x%p\n", dev->start, dev->end);
	ret += sprintf(page+ret, "Wp: 0x%p ; Rp: 0x%p\n", dev->wp, dev->rp);

	*eof = 1;
	return ret;
}


static struct file_operations my_fops = {
	.owner = THIS_MODULE,
	.open  = my_open,
	.release = my_release,
	.read = my_read,
	.write = my_write,
	.ioctl = my_ioctl,
};

static int __init my_init(void)
{
	int ret;
	dev_t dev_num;

	/* allocate mydev */
	dev = (struct mydev *)kzalloc(sizeof(*dev), GFP_KERNEL);
	if (NULL == dev)
		return -ENOMEM;
	/* initialize mydev */
	dev->start = (char *)kzalloc(BUF_LEN, GFP_KERNEL);
	dev->end   = dev->start + BUF_LEN;
	dev->wp	   = dev->start;
	dev->rp    = dev->start;
	dev->nreaders = dev->nwriters = 0;
	init_waitqueue_head(&dev->readq);
	init_waitqueue_head(&dev->writeq);
	init_MUTEX(&dev->mysem);
	//sema_init(&dev->mysem, 1);

	/* Request dev num */
	if (major) {
		dev_num = MKDEV(major, 0);
		ret = register_chrdev_region(dev_num, 10, "spipe");
	} else {
		ret = alloc_chrdev_region(&dev_num, 0, 10, "dpipe");
		major = MAJOR(dev_num);
	}
	if (ret) {
		kfree(dev->start);
		kfree(dev);
		printk("Cannot allocate dev num\n");
		return -1;
	}

	/* Register cdev */
	cdev_init(&dev->mycdev, &my_fops);
	dev->mycdev.owner = THIS_MODULE;
	cdev_add(&dev->mycdev, dev_num, 1);

	/* create /proc/proc_pipe */
	create_proc_read_entry("proc_pipe", /* name */
			0, /* mode */
			NULL, /* parent */
			my_proc, /* func */
			dev); /* parameter */

	return 0;
}

static void __exit my_exit(void)
{
	remove_proc_entry("proc_pipe", NULL);
	cdev_del(&dev->mycdev);
	unregister_chrdev_region(MKDEV(major, 0), 10);
	kfree(dev->start);
	kfree(dev);
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("nolan@uplooking.com");
MODULE_VERSION("1.4");

