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
#include <linux/ata.h>  


#define DEV_NAME "fake_pata_platform"
#define PATA_INT	0

static struct pata_platform_info fake_pata_platform_data = {
	.ioport_shift = 0, /* this means the register size.
			      if the register is 32bit,
			      it shoulde be 2 */
};

/* TODO: Move these macros to the header file 
 * share them with the fake_pata_driver
 * */
#define RES_IO_MEM_INDEX 0
#define RES_CTL_MEM_INDEX 1
#define RES_IRQ_INDEX 2
#define RES_MEM_LENGTH	0x20

static struct resource fake_pata_res[] = {
	[RES_IO_MEM_INDEX] = {
			.start 	= 0x0,
			.end 	= 0x0,
			.flags 	= IORESOURCE_MEM,
	},
	[RES_CTL_MEM_INDEX] = {
			.start 	= 0x0,
			.end 	= 0x0,
			.flags 	= IORESOURCE_MEM,
	},
	[RES_IRQ_INDEX] = {
			.start	= PATA_INT,
			.end	= PATA_INT,
			.flags	= IORESOURCE_IRQ ,
	},
	[3] = {
			.start 	= 0x0,
			.end 	= 0x0,
			.flags 	= IORESOURCE_IO,
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
		.platform_data = &fake_pata_platform_data,
		.release = private_release_notifier,
	},
};
static struct page* phy_page = NULL;
static struct page* last_phy_page = NULL;
static struct page* ctl_phy_page = NULL;
static struct page* last_ctl_phy_page = NULL;
static struct task_struct *tsk = NULL;  

static char Identify_Drive_Information[512]=
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	'F','A','K','E','D','I','S','K',
};
static int start_offset = 0;
static int enable_read = 0;
static void read_things(void)
{
	unsigned char* va=page_address(phy_page);
	if(enable_read)
	{
		msleep(1);
		va[ATA_REG_STATUS]=0x1 << 7 ;
		msleep(1);
		va[ATA_REG_DATA]=Identify_Drive_Information[start_offset];
		va[ATA_REG_STATUS]=0x1 << 3 ;
		start_offset=(start_offset+1)%512;
		if(enable_read>30)
		{
			enable_read=0;
		}
	}
}

static int thread_function(void *data)  
{  
	struct platform_device* fake_pata_device_p=data;
	int cmp_res = 0 ;
	do {  
		/* compare IO */
		{
			unsigned char* va_l=page_address(last_phy_page);
			unsigned char* va=page_address(phy_page);
			char REG_OUTPUT[32] = {0};
			char COMMAND_OUTPUT[32] = {0};

			cmp_res = memcmp(va_l,va, RES_MEM_LENGTH);
			if(cmp_res)
			{
				int i = 0;
				for (i = 0; i < RES_MEM_LENGTH ; i ++)
				{
					strcpy(COMMAND_OUTPUT,"");
					if ( va_l[i] != va[i] )
					{
						switch ( i )
						{
							case ATA_REG_DATA :
								strcpy(REG_OUTPUT,"ATA_REG_DATA");
								break;
							case ATA_REG_ERR :
								strcpy(REG_OUTPUT,"ATA_REG_ERR");
								break;
							case ATA_REG_NSECT :
								strcpy(REG_OUTPUT,"ATA_REG_NSECT");
								break;
							case ATA_REG_LBAL :
								strcpy(REG_OUTPUT,"ATA_REG_LBAL");
								break;
							case ATA_REG_LBAM :
								strcpy(REG_OUTPUT,"ATA_REG_LBAM");
								break;
							case ATA_REG_LBAH :
								strcpy(REG_OUTPUT,"ATA_REG_LBAH");
								break;
							case ATA_REG_DEVICE :
								strcpy(REG_OUTPUT,"ATA_REG_DEVICE");
								break;
							case ATA_REG_CMD :
								strcpy(REG_OUTPUT,"ATA_REG_CMD");
								switch(va[i])
								{
									case ATA_CMD_DEV_RESET:
										strcpy(COMMAND_OUTPUT,"Dev Reset");
										break;
									case ATA_CMD_CHK_POWER:
										strcpy(COMMAND_OUTPUT,"Check Power");
										break;
									case ATA_CMD_STANDBY:
										strcpy(COMMAND_OUTPUT,"Standby !");
										break;
									case ATA_CMD_IDLE:
										strcpy(COMMAND_OUTPUT,"Idle !");
										break;
									case ATA_CMD_ID_ATA:
										strcpy(COMMAND_OUTPUT,"Identify Drive !");
										start_offset=0;
										enable_read=1;
										break;
									case ATA_CMD_PACKET:
										strcpy(COMMAND_OUTPUT,"Packet![TODO: not implement]");
										break;
									default:
										strcpy(COMMAND_OUTPUT,"Unknown !");
										break;

								}
								break;
							default:
								strcpy(REG_OUTPUT,"Unknown");
								break;
						}
						dev_info(&(fake_pata_device_p->dev), "CBR [%d][%15.15s] [0x%X] ==> [0x%X] %s\n", 
								i, REG_OUTPUT,
								(unsigned char)va_l[i] , (unsigned char)va[i],
								COMMAND_OUTPUT);  
					}

				}
			}
			dev_info(&(fake_pata_device_p->dev), "Current fake data offset %d\n", start_offset);  
			read_things();/*fake read handler, 
				      real disk should read data from disk now */
			memcpy(va_l,va, RES_MEM_LENGTH) ;
		}

		/* compare Control Reg */
		{
			unsigned char* va_ctl_l=page_address(last_ctl_phy_page);
			unsigned char* va_ctl=page_address(ctl_phy_page);
			char REG_OUTPUT[32] = {0};
			char COMMAND_OUTPUT[32] = {0};


			cmp_res = memcmp(va_ctl_l,va_ctl, RES_MEM_LENGTH);
			if(cmp_res)
			{
				int i = 0;
				for (i = 0; i < RES_MEM_LENGTH ; i ++)
				{
					if ( va_ctl_l[i] != va_ctl[i] )
					{
						strcpy(COMMAND_OUTPUT,"");
						switch ( i )
						{
							case 0x0 :
								strcpy(REG_OUTPUT,"Device Control");
								if(va_ctl[i]  & (0x1 << 1))
								{
									sprintf(COMMAND_OUTPUT,"Enable Interupt");
								}
								else
								{
									sprintf(COMMAND_OUTPUT,"Disable Interupt");
								}
								if(va_ctl[i]  & (0x1 << 2))
								{
									sprintf(COMMAND_OUTPUT,"%s, software reset",COMMAND_OUTPUT);
								}
								break;
							case 0x1 :
								/* should not get here ?*/
								strcpy(REG_OUTPUT,"Drive Address[?]");
								break;
							default:
								strcpy(REG_OUTPUT,"Unknown");
								break;
						}
						dev_info(&(fake_pata_device_p->dev),"CONTROL BLOCK registers [%d][%s] [0x%X] ==> [0x%X] %s\n", 
								i,REG_OUTPUT, 
								va_ctl_l[i] , va_ctl[i],
								COMMAND_OUTPUT);  
					}

				}
			}
			memcpy(va_ctl_l,va_ctl, RES_MEM_LENGTH) ;
		}
		msleep(100);  
	} while( !kthread_should_stop() );  
	return 0;  
}

static void create_fake_res(struct resource * res, struct page* phy_page)
{
	void* phy_addr_end = NULL ;
	void* phy_addr_start = NULL ;

	phy_addr_end = 0;
	phy_addr_start = page_address(phy_page) - PAGE_OFFSET;

	printk("Allocated page Physical Address : %p \n",phy_addr_start);
	if (!phy_addr_start)
		return ;
	else
		phy_addr_end = phy_addr_start + RES_MEM_LENGTH - 1;

	printk("allocated space end at : %p \n",phy_addr_end);

	res->start = (resource_size_t)phy_addr_start ;
	res->end = (resource_size_t)phy_addr_end ;

	return ;
}

int __init fake_pata_init(void)
{
	/* For IO resource */
	phy_page = alloc_page(GFP_DMA);	
	last_phy_page = alloc_page(GFP_KERNEL);
	create_fake_res(&(fake_pata_res[RES_IO_MEM_INDEX]),phy_page);
	memcpy(page_address(last_phy_page),page_address(phy_page), RES_MEM_LENGTH) ;

	/* For Control resource */
	ctl_phy_page = alloc_page(GFP_DMA);	
	last_ctl_phy_page = alloc_page(GFP_KERNEL);
	create_fake_res(&(fake_pata_res[RES_CTL_MEM_INDEX]),ctl_phy_page);
	memcpy(page_address(last_ctl_phy_page),page_address(ctl_phy_page), RES_MEM_LENGTH) ;

	tsk = kthread_run(thread_function, &fake_pata_device, "fake_pata_dev_thread");  
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
		printk(KERN_INFO "thread stoped with ret[%d]\n", ret);  
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

