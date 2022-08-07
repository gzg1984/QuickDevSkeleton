#include <linux/module.h>

int test1(void)
{
	printk("it's test1 .\n");
	return 0;
}

EXPORT_SYMBOL(test1);

MODULE_LICENSE("Gzged License");
