#include <linux/module.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <asm/atomic.h>

atomic_t irq_sum = ATOMIC_INIT(0);
unsigned long irqs = 0;

int key_func(int irq, void *data)
{
	irqs++;
	atomic_inc(&irq_sum);
	return IRQ_HANDLED;
}

int mouse_func(int irq, void *data)
{
	irqs++;
	atomic_inc(&irq_sum);
	return IRQ_HANDLED;
}

int __init my_init(void)
{
	int ret;
	ret = request_irq(1, key_func, IRQF_SHARED, "key irq", &irq_sum);
	if (ret)
		return -1;
	ret = request_irq(12, mouse_func, IRQF_SHARED, "mouse irq", &irq_sum);
	if (ret) {
		free_irq(1, &irq_sum);
		return -1;
	}
	return 0;
}

void __exit my_exit(void)
{
	free_irq(12, &irq_sum);
	free_irq(1, &irq_sum);
	printk("atomic sum = %d, normal sum = %ld\n", atomic_read(&irq_sum), irqs);
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ZHANG");
MODULE_VERSION("1.3");



