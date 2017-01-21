#include <linux/module.h>
#include <linux/sched.h>

static int __init my_init(void)
{
	int i;
	for (i=0; i<8; i++) {
		printk("<%d> Debug level %d, in %s\n", i, i, __FILE__);
	}
	return 0;
}

static void __exit my_exit(void)
{
	printk("Exit from %s-%s\n\n", __FILE__, __FUNCTION__);
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Zhang");
MODULE_VERSION("0.3");
