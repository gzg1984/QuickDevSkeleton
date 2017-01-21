#include <linux/module.h>
#include <linux/sched.h>
#include <linux/mm.h>

int order = 1;
module_param(order, int, 0444);

char *buf;

static int __init my_init(void)
{
	buf = (char *)kmalloc(((1<<order) - 1), GFP_KERNEL);
	if (NULL == buf)
		return -ENOMEM;

	printk("Allocate %d bytes, addr = 0x%p\n", ((1<<order) - 1), buf);

	return 0;
}

static void __exit my_exit(void)
{
	kfree(buf);
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ZHANG");
MODULE_VERSION("0.6");

