#include <linux/module.h>

extern void my_print(void);

static int __init my_init(void)
{
	printk("call from mymod2.c\n");
	my_print();
	return 0;
}

static void __exit my_exit(void)
{
	printk("Exit mymod2.c\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Zhang");
MODULE_DESCRIPTION("Simple module for kernel learning.");
MODULE_VERSION("0.1");

