/*
 * Copyright 2018 Zhigang Gao <gzg1984@gmail.com>
 */

#include <linux/module.h>
#include <linux/sched.h>
#include <linux/platform_device.h>
#include <linux/ata_platform.h>
#include <linux/mm.h>
#include <linux/init.h>  
#include <linux/kthread.h>  
#include <linux/delay.h>  


#define DEV_NAME "pata_platform"
#define PATA_INT	0

static struct pata_platform_info bfin_pata_platform_data = {
	.ioport_shift = 2,
};

#define RES_MEM_INDEX 0
#define RES_IRQ_INDEX 1
#define RES_MEM_LENGTH	0x20
static struct resource fake_pata_res[] = {
	[0] = {
			.start 	= 0x0,
			.end 	= 0x0,
			.flags 	= IORESOURCE_MEM,
	},
	[1] = {
			.start	= PATA_INT,
			.end	= PATA_INT,
			.flags	= IORESOURCE_IRQ ,
	},
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
static struct page* phy_page = NULL;
static struct page* last_phy_page = NULL;
static struct task_struct *tsk = NULL;  

static int thread_function(void *data)  
{  
	int time_count = 0;  
	int cmp_res = 0 ;
	do {  
		printk(KERN_INFO "thread_function: %d times", ++time_count);  
		cmp_res = memcmp(page_address(last_phy_page),page_address(phy_page), RES_MEM_LENGTH);
		if(cmp_res)
		{
			int i = 0;
			char* va_l=page_address(last_phy_page);
			char* va=page_address(phy_page);
			for (i = 0; i < RES_MEM_LENGTH ; i ++)
			{
				if ( va_l[i] != va[i] )
				{
					printk(KERN_INFO "[%d] [0x%X] ==> [0x%X]", 
							i, ((char*)va_l)[i] , ((char*)va)[i] );  
				}

			}
		}
		memcpy(page_address(last_phy_page),page_address(phy_page), RES_MEM_LENGTH) ;
		msleep(1000);  
	}while(!kthread_should_stop() );  
	//}while(!kthread_should_stop() && time_count<=30);  
	return time_count;  
}  

int __init fake_pata_init(void)
{
	void* phy_addr_end = NULL ;
	void* phy_addr_start = NULL ;

	phy_page = alloc_page(GFP_DMA);	
	last_phy_page = alloc_page(GFP_KERNEL);
	phy_addr_end = 0;
	phy_addr_start = page_address(phy_page) - PAGE_OFFSET;
	memcpy(page_address(last_phy_page),page_address(phy_page), RES_MEM_LENGTH) ;

	printk("Allocated page Physical Address : %p \n",phy_addr_start);
	if (!phy_addr_start)
		return -EINVAL;
	else
		phy_addr_end = phy_addr_start + RES_MEM_LENGTH - 1;

	printk("allocated space end at : %p \n",phy_addr_end);
	fake_pata_res[RES_MEM_INDEX].start = (resource_size_t)phy_addr_start ;
	fake_pata_res[RES_MEM_INDEX].end = (resource_size_t)phy_addr_end ;

	tsk = kthread_run(thread_function, NULL, "mythread%d", 1);  
	if (IS_ERR(tsk)) {  
		printk(KERN_INFO "create kthread failed!\n");  
	}  
	else {  
		printk(KERN_INFO "create ktrhead ok!\n");  
	}  

	return platform_device_register(&fake_pata_device);
}

void __exit fake_pata_exit(void)
{
	if (!IS_ERR(tsk)){  
		int ret = kthread_stop(tsk);  
		printk(KERN_INFO "thread function has run %ds\n", ret);  
	}  
	platform_device_unregister(&fake_pata_device);

	printk("Freeing addr : 0x%llx \n",fake_pata_res[0].start);
	fake_pata_res[0].start = 0 ;
	fake_pata_res[0].end = 0 ;

	if(phy_page)
		__free_page(phy_page); 

	phy_page = NULL ;
}

module_init(fake_pata_init);
module_exit(fake_pata_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Gordon <gzg1984@gmail.com>");
MODULE_VERSION("0.1");

