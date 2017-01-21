#include <linux/module.h>
#include <linux/sched.h>
#include <linux/platform_device.h>

#define DEV_NAME "nolan"

static void my_release(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	printk("Enter my_release, pdev->name = %s, pdev->id = %d, time is %ld\n", pdev->name, pdev->id, jiffies);
}

static struct resource my_resources[] = {
	[0] = {
		.start 	= 0xfebff800,
		.end 	= 0xfebff8ff,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start 	= 0xe400,
		.end	= 0xe4ff,
		.flags 	= IORESOURCE_IO,
	},
	[2] = {
		.start 	= 20,
		.end	= 20,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device plat_dev = {
	.name 	= DEV_NAME,
	.id 	= 1,
	.num_resources 	= ARRAY_SIZE(my_resources),
	.resource 	= my_resources,
	.dev	= {
		.release = my_release,
	}
};

static int __init my_init(void)
{
	printk("Enter my device init, time is %ld\n", jiffies);
	return platform_device_register(&plat_dev);
}

static void __exit my_exit(void)
{
	printk("Enter my device exit, time is %ld\n", jiffies);
	platform_device_unregister(&plat_dev);
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("nolan@uplooking.com");
MODULE_VERSION("0.3");

