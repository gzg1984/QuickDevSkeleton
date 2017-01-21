#include <linux/module.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/time.h>

struct tasklet_struct my_tasklet;
unsigned long upper_count;
unsigned long bottom_count;

/* bottom half */
void tasklet_func(unsigned long data)
{
	char *context1 = in_interrupt() ? "interrupt" : "task";
	char *context2 = in_irq() ? "upper" : "bottom";
	
/*	struct timeval tv;
	do_gettimeofday(&tv);
	bottom_count++;
	printk("In hi tasklet %ld, time is %ds, %dus\n", bottom_count, (int)tv.tv_sec, (int)tv.tv_usec);
*/
	printk("In %s, context is %s\n", __FUNCTION__, context1);
	if (in_interrupt())
		printk("In %s interrupt context\n", context2);
	printk("In %s, stack in 0x%p\n", __FUNCTION__, &data);
	//stack
}

/* upper half */
irqreturn_t interrupt_func(int irq, void *data)
{
	char *context1 = in_interrupt() ? "interrupt" : "task";
	char *context2 = in_irq() ? "upper" : "bottom";
/*	struct timeval tv;
	do_gettimeofday(&tv);
	upper_count++;
	printk("In irq %ld, time is %ds, %dus\n", upper_count, (int)tv.tv_sec, (int)tv.tv_usec);
*/
	tasklet_schedule(&my_tasklet);
	//tasklet_hi_schedule(&my_tasklet);

	//in_interrupt() and in_irq()
	
/*	struct timeval tv;
	do_gettimeofday(&tv);
	bottom_count++;
	printk("In hi tasklet %ld, time is %ds, %dus\n", bottom_count, (int)tv.tv_sec, (int)tv.tv_usec);
*/
	printk("\n\nIn %s, context is %s\n", __FUNCTION__, context1);
	if (in_interrupt())
		printk("In %s interrupt context\n", context2);
	printk("In %s, stack in 0x%p\n", __FUNCTION__, &data);
	printk("current task %s's stack in 0x%p\n", current->comm, current->stack);
	return IRQ_HANDLED;
}

int __init my_init(void)
{
	int ret;
	ret = request_irq(12, interrupt_func, IRQF_SHARED, "Bottomtest", &my_tasklet);
	if (ret) {
		printk("Cannot request irq 12\n");
		return -1;
	}

	tasklet_init(&my_tasklet, tasklet_func, 5);
	return 0;
}

void __exit my_exit(void)
{
	free_irq(12, &my_tasklet);
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ZHANG");
MODULE_VERSION("1.1");

