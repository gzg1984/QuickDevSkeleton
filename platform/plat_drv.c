#include <linux/module.h>
#include <linux/sched.h>
#include <linux/platform_device.h>

#define DRV_NAME "nolan"

static int __devinit my_probe(struct platform_device *pdev)
{
	printk("Enter my_probe, time is %ld\n", jiffies);
	printk("pdev->name: %s ; pdev->id: %d\n", pdev->name, pdev->id);
	return 0;
}

static int __devexit my_remove(struct platform_device *pdev)
{
	printk("Enter my_remove, time is %ld\n", jiffies);
	printk("pdev->name: %s ; pdev->id: %d\n", pdev->name, pdev->id);
	return 0;
}

static struct platform_driver my_drv = {
	.probe = my_probe,
	.remove = my_remove,
	.driver = {
		.name = DRV_NAME,
		.owner = THIS_MODULE,
	},
};

static int __init my_init(void)
{
	printk("Enter my driver init, time is %ld\n", jiffies);
	return platform_driver_register(&my_drv);
}

static void __exit my_exit(void)
{
	printk("Enter my driver exit, time is %ld\n", jiffies);
	platform_driver_unregister(&my_drv);
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("nolan@uplooking.com");
MODULE_VERSION("0.3");

