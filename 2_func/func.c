#include <linux/module.h>

static int no_exported_example(void)
{
	printk("it's no_exported_example no body will call it directly.\n");
	return 0;
}

int exported_example(void)
{
	printk("it's exported_example . every body can call it\n");
	no_exported_example();
	return 0;
}

EXPORT_SYMBOL(exported_example);

MODULE_LICENSE("Gzged License");
