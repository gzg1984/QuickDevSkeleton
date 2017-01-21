#include <linux/module.h>
#include <linux/sched.h>
#include <linux/platform_device.h>
#define DEV_NAME "key"

struct resource my_res[] = {
	[0] = {
			.start 	= 0x60,
			.end 	= 0x60,
			.flags 	= IORESOURCE_IO,
	},
	[1] = {
			.start	= 1,
			.end	= 1,
			.flags	= IORESOURCE_IRQ,
	},
};

void my_release(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	printk("id%d: release in %ld\n", pdev->id, jiffies);
}


struct platform_device my_devs = {
	.name = DEV_NAME,
	.id = 5,
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

