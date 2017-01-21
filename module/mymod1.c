#include <linux/module.h>

static unsigned int order = 5;
module_param(order, uint, 0444);

static unsigned int many[5] = {1, 2, 3, 4, 5};
static int num = 20;
module_param_array(many, uint, &num, 0444);

static void my_print(void)
{
	printk("print from mymod1.c\n");
}
EXPORT_SYMBOL(my_print);


static int __init my_init(void)
{
	int i;
	printk("Enter my_init, order=%d\n", order);

	for (i=0; i<ARRAY_SIZE(many); i++) {
		printk("many[%d] = %d\n", i, many[i]);
	}
	printk("num = %d\n", num);

return 0;
}

static void __exit my_exit(void)
{
	printk("Exit my module\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_ALIAS("ABC");
MODULE_AUTHOR("Zhang");
MODULE_DESCRIPTION("Simple module for kernel learning.");
MODULE_VERSION("0.1");

