#include <linux/module.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/spinlock.h>
#include <linux/cdev.h>
#include <linux/interrupt.h>
#include <asm/io.h>
#include <asm/uaccess.h>

#define MY_MAJOR 70

struct mydev {
	spinlock_t my_lock;
	struct list_head key_list;
	unsigned long key_count;
	struct cdev my_cdev;
};

struct key_dev {
	struct list_head key_item;
	unsigned char value;
};

struct mydev *dev;

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
	unsigned long flags;
	unsigned char *kbuf;
	int i;
	struct key_dev *pos;

	spin_lock_irqsave(&dev->my_lock, flags);

	count = min(count, (size_t)dev->key_count);
	if (0 == count) {
		spin_unlock_irqrestore(&dev->my_lock, flags);
		return count;
	}
	kbuf = (unsigned char *)kzalloc(count, GFP_ATOMIC);
	if (NULL == kbuf) {
		spin_unlock_irqrestore(&dev->my_lock, flags);
		return -ENOMEM;
	}
	
	for (i=0; i<count; i++) {
		pos = list_entry(dev->key_list.next, struct key_dev, key_item);
		kbuf[i] = pos->value;
		list_del(&pos->key_item);
		kfree(pos);
	}
	dev->key_count -= count;
	spin_unlock_irqrestore(&dev->my_lock, flags);

	if (copy_to_user(buf, kbuf, count)) {
		kfree(kbuf);
		return -EFAULT;
	}

	kfree(kbuf);
	return count;
}

struct file_operations my_fops = {
	.owner = THIS_MODULE,
	.open = my_open,
	.release = my_release,
	.read = my_read,
};


int key_func(int irq, void *data)
{
	struct key_dev *tmp;

	tmp = (struct key_dev *)kzalloc(sizeof(*tmp), GFP_KERNEL);
	if (NULL == tmp)
		return IRQ_NONE;
	tmp->value = inb(0x60);
	INIT_LIST_HEAD(&tmp->key_item);

	spin_lock(&dev->my_lock);
	list_add_tail(&tmp->key_item, &dev->key_list);
	dev->key_count++;
	spin_unlock(&dev->my_lock);

	return IRQ_HANDLED;
}


int __init my_init(void)
{
	int ret;
	dev_t devno = MKDEV(MY_MAJOR, 0);
	dev = (struct mydev *)kzalloc(sizeof(*dev), GFP_KERNEL);
	if (NULL == dev)
		return -ENOMEM;
	/* initialize struct mydev */
	INIT_LIST_HEAD(&dev->key_list);
	spin_lock_init(&dev->my_lock);

	ret = request_irq(1, key_func, IRQF_SHARED, "KEY", dev);
	if (ret) {
		kfree(dev);
		return -1;
	}

	cdev_init(&dev->my_cdev, &my_fops);
	dev->my_cdev.owner = THIS_MODULE;
	cdev_add(&dev->my_cdev, devno, 1);

	return 0;
}

void __exit my_exit(void)
{
	struct key_dev *pos, *n;
	free_irq(1, dev);

	spin_lock(&dev->my_lock);
	cdev_del(&dev->my_cdev);
	if (dev->key_count) {
		list_for_each_entry_safe(pos, n, &dev->key_list, key_item) {
			list_del(&pos->key_item);
			kfree(pos);
		}
	}
	spin_unlock(&dev->my_lock);

	kfree(dev);
}

module_init(my_init);
module_exit(my_exit);

MODULE_AUTHOR("ZHANG");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.4");

