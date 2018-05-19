/*
 * Copyright 2018 Zhigang Gao <gzg1984@gmail.com>
 */

#include <linux/module.h>
#include <linux/sched.h>
#include <linux/platform_device.h>
#include <linux/ata_platform.h>
#include <linux/mm.h>

#define DEV_NAME "pata_platform"
#define PATA_INT	0

static struct pata_platform_info bfin_pata_platform_data = {
	.ioport_shift = 2,
};

static struct resource fake_pata_res[] = {
	[0] = {
			.start 	= 0x60,
			.end 	= 0x60,
			//.flags 	= IORESOURCE_IO,
			.flags 	= IORESOURCE_MEM,
	},
	{
		.start = 0x2030C000,
		.end = 0x2030C01F,
		.flags = IORESOURCE_MEM,
	},
	/*
	[1] = {
			.start	= 1,
			.end	= 1,
			.flags	= IORESOURCE_IRQ,
	},
	{
		.start = 0x2030D018,
		.end = 0x2030D01B,
		.flags = IORESOURCE_MEM,
	},
	{
		.start = PATA_INT,
		.end = PATA_INT,
		.flags = IORESOURCE_IRQ | IORESOURCE_IRQ_HIGHLEVEL,
	},
	*/
};

static void private_release_notifier(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	printk("id[%d]: release in jiffies[%ld]\n", pdev->id, jiffies);
}

struct platform_device fake_pata_device = {
	.name = DEV_NAME,
	.id = -1, /* -1 means let system set it */
	.num_resources = ARRAY_SIZE(fake_pata_res),	
	.resource	= fake_pata_res,
	.dev = {
		.platform_data = &bfin_pata_platform_data,
		.release = private_release_notifier,
	},
};

int __init fake_pata_init(void)
{
	struct page* phy_page = alloc_page(GFP_DMA);	
	void* phy_addr_end = 0;
//	void* phy_addr_start = kmalloc(0x20,GFP_DMA);
	void* phy_addr_start = page_address(phy_page) - PAGE_OFFSET;
	printk("allocated addr : %p \n",phy_addr_start);
	if (!phy_addr_start)
		return -EINVAL;
	else
		phy_addr_end = phy_addr_start + 0x20 - 1;
//	__free_page(phy_page); // let the device register get the page
				// Kernel tells do not use page this way.

	printk("allocated space end at : %p \n",phy_addr_end);
	fake_pata_res[0].start = (resource_size_t)phy_addr_start ;
	fake_pata_res[0].end = (resource_size_t)phy_addr_end ;
	fake_pata_res[1].start = (resource_size_t)phy_addr_start ;
	fake_pata_res[1].end = (resource_size_t)phy_addr_end ;

	return platform_device_register(&fake_pata_device);
}

void __exit fake_pata_exit(void)
{
	platform_device_unregister(&fake_pata_device);

	printk("Freeing addr : 0x%llx \n",fake_pata_res[0].start);
	//TODO: free phy_page ? no need
	//kfree((void*)fake_pata_res[0].start);
	fake_pata_res[0].start = 0 ;
	fake_pata_res[0].end = 0 ;
}

module_init(fake_pata_init);
module_exit(fake_pata_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Gordon <gzg1984@gmail.com>");
MODULE_VERSION("0.1");

