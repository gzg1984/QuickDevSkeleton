#include <linux/module.h>
#include <linux/sched.h>
#include <linux/platform_device.h>
#define DEV_NAME "key"

struct resource my_res[] = {
	[0] = {
			.start 	= 0x378,
			.end 	= 0x378,
			.flags 	= IORESOURCE_IO,
	},
	[1] = {
			.start	= 7,
			.end	= 7,
			.flags	= IORESOURCE_IRQ,
	},
};

void my_release(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	printk("%s id%d: release in %ld\n", DEV_NAME, pdev->id, jiffies);
}


struct platform_device my_devs = {
	.name = DEV_NAME,
	.id = 18,
	.num_resources = ARRAY_SIZE(my_res),	
	.resource	= my_res,
	.dev = {
		.release = my_release,
	},
};


int __init my_init(void)
{
	return platform_device_register(&my_devs);
}

void __exit my_exit(void)
{
	platform_device_unregister(&my_devs);
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ZHANG");
MODULE_VERSION("1.4");

