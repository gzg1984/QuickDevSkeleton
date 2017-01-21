#include <linux/module.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/time.h>

struct mydev {
	struct work_struct my_work;
	unsigned long upper_count;
	unsigned long bottom_count;
	struct timeval tv;
	unsigned long data;
};

struct mydev *dev;

/* bottom half */
void work_func(struct work_struct *work)
{
	struct mydev *dev = container_of(work, struct mydev, my_work);
	do_gettimeofday(&dev->tv);
	dev->bottom_count++;
	printk("In work %ld, time is %ds, %dus\n", dev->bottom_count, (int)dev->tv.tv_sec, (int)dev->tv.tv_usec);
	printk("In %s, stack in 0x%p, jiffies = 0x%lx\n", __FUNCTION__, &work, dev->data);
}

/* upper half */
irqreturn_t interrupt_func(int irq, void *data)
{
	do_gettimeofday(&dev->tv);
	dev->upper_count++;
	printk("In irq %ld, time is %ds, %dus\n", dev->upper_count, (int)dev->tv.tv_sec, (int)dev->tv.tv_usec);

	dev->data = jiffies;
	schedule_work(&dev->my_work);
	
	return IRQ_HANDLED;
}

int __init my_init(void)
{
	int ret;
	
	dev = kzalloc(sizeof(*dev), GFP_KERNEL);
	if (NULL == dev) 
		return -ENOMEM;

	ret = request_irq(12, interrupt_func, IRQF_SHARED, "Worktest", dev);
	if (ret) {
		printk("Cannot request irq 12\n");
		return -1;
	}

	INIT_WORK(&dev->my_work, work_func);
	return 0;
}

void __exit my_exit(void)
{
	free_irq(12, dev);
	kfree(dev);
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ZHANG");
MODULE_VERSION("1.2");

