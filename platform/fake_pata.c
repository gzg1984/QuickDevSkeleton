
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/platform_device.h>
#include <linux/ata_platform.h>
#include <linux/mm.h>
#include <linux/init.h>  
#include <linux/kthread.h>  
#include <linux/delay.h>  
#include <linux/io.h>  
#include <linux/ata.h>  


#define DEV_NAME "pata_platform"

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
#define PATA_INT	2
#define RES_MEM_LENGTH	0x20

/* boot parament
 * sudo vim /boot/grub/grub.cfg
 *         linux   /boot/vmlinuz-4.4.0-116-generic root=UUID=9b086f9e-2bac-4fa0-b168-15ac62b6a2f4 ro  mem=768m
 */
static struct resource fake_pata_res[] = {
	/* IO MEM start from 768 +1 M */
	[RES_IO_MEM_INDEX] = {
			.start 	= (resource_size_t)((768 + 1 ) * 1024 * 1024),
			.end 	= (resource_size_t)((768 + 2 ) * 1024 * 1024),
			.flags 	= IORESOURCE_MEM,
	},
	/* CTL MEM start from 768 +1 M */
	[RES_CTL_MEM_INDEX] = {
			.start 	= (resource_size_t)((768 + 3 ) * 1024 * 1024),
			.end 	= (resource_size_t)((768 + 4 ) * 1024 * 1024),
			.flags 	= IORESOURCE_MEM,
	},
	[RES_IRQ_INDEX] = {
			.start	= PATA_INT,
			.end	= PATA_INT,
			.flags	= IORESOURCE_IRQ ,
	},
	/*
	[3] = {
			.start 	= 0x0,
			.end 	= 0x0,
			.flags 	= IORESOURCE_IO,
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
		.platform_data = &fake_pata_platform_data,
		.release = private_release_notifier,
	},
};
static struct page* last_phy_page = NULL;
static struct page* last_ctl_phy_page = NULL;
static struct task_struct *tsk = NULL;  
static char Identify_Drive_Information[512]=
{
	'F','A','K','E','D','I','S','K','1','\0',/*Serial number**/
	'F','A','K','E','D','I','S','K','2','\0',/*Serial number**/
	'F','A','K','E','D','I','S','K','3','\0',/*Serial number**/
	'F','A','K','E','D','I','S','K','4','\0',/*Serial number**/
	'F','A','K','E','D','I','S','K','5','\0',/*Serial number**/
	'F','A','K','E','D','I','S','K','6','\0',/*Serial number**/
	'F','A','K','E','D','I','S','K','7','\0',/*Serial number**/
	'F','A','K','E','D','I','S','K','8','\0',/*Serial number**/
	'F','A','K','E','D','I','S','K','9','\0',/*Serial number**/
};

static char Identify_Drive_Information1[512]=
{
	0 /*low of 0 word*/ ,0x1 << 0x2 /* high of 0 word*/,
       	1,0,/*Number of cylinders*/
       	0,0,
       	1,0,/*Number of heads*/
       	1,0,/*Numbe  of heads f unformatted bytes per track*/
       	1,/* low of 5 word*/ 0,/* high of 5 word*//*Number of unformatted bytes per sector*/
       	1,0,/*Number of sectors per track*/
       	0,0,
       	0,0,
       	0,0,
	'F','A','K','E','D','I','S','K',/*Serial number**/
	'F','A','K','E','D','I','S','K',/*Serial number**/
	'F','A','K','E','D','I','S','K',/*Serial number**/
	'F','A','K','E','D','I','S','K',/*Serial number**/
	'F','A','K','E','D','I','S','K',/*Serial number**/
	'F','A','K','E','D','I','S','K',/*Serial number**/
	'F','A','K','E','D','I','S','K',/*Serial number**/
	'F','A','K','E','D','I','S','K',/*Serial number**/
};


unsigned char* va = NULL;
unsigned char* va_ctl = NULL;
void init_fake_device(struct platform_device *pdev)
{
	struct resource *res_base, *res_alt, *res_irq;
	void __iomem *base, *alt_base;

	/* get a pointer to the register memory */
	res_base = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	res_alt = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	printk("res_base=%p\n",res_base);
	printk("res_alt=%p\n",res_alt);

	res_irq = platform_get_resource(pdev, IORESOURCE_IRQ, 0);

	base = ioremap( res_base->start, resource_size(res_base));
	alt_base = ioremap( res_alt->start, resource_size(res_alt));
	va = base ;
	va_ctl = alt_base ;

}

static int start_offset = 0;
static int enable_read = 0;
static void read_things(void)
{
	if(enable_read)
	{
		printk("Current fake data offset %d\n", start_offset);  
		va[ATA_REG_STATUS]=0x1 << 7 ;
		va[ATA_REG_DATA]=Identify_Drive_Information[start_offset++];
		va[ATA_REG_DATA+1]=Identify_Drive_Information[start_offset++];
		va[ATA_REG_DATA+2]=Identify_Drive_Information[start_offset++];
		va[ATA_REG_DATA+3]=Identify_Drive_Information[start_offset++];
		va[ATA_REG_STATUS]=0x1 << 3 ;
		start_offset=(start_offset+1)%512;
		if(start_offset>40)
		{
			enable_read=0;
		}
	}
}
void refresh_status(void)
{
	va[ATA_REG_STATUS]=ATA_DRDY;
	return;
}

static int thread_function(void *data)  
{
	struct platform_device* fake_pata_device_p=data;
	int cmp_res = 0 ;
	do {
		/* compare IO */
		{
			unsigned char* va_l=page_address(last_phy_page);
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
								enable_read=0;
								break;
							case ATA_REG_ERR :
								strcpy(REG_OUTPUT,"ATA_REG_ERR");
								enable_read=0;
								break;
							case ATA_REG_NSECT :
								strcpy(REG_OUTPUT,"ATA_REG_NSECT");
								enable_read=0;
								break;
							case ATA_REG_LBAL :
								strcpy(REG_OUTPUT,"ATA_REG_LBAL");
								enable_read=0;
								break;
							case ATA_REG_LBAM :
								strcpy(REG_OUTPUT,"ATA_REG_LBAM");
								enable_read=0;
								break;
							case ATA_REG_LBAH :
								strcpy(REG_OUTPUT,"ATA_REG_LBAH");
								enable_read=0;
								break;
							case ATA_REG_DEVICE :
								strcpy(REG_OUTPUT,"ATA_REG_DEVICE");
								enable_read=0;
								break;
							case ATA_REG_CMD :
								strcpy(REG_OUTPUT,"ATA_REG_CMD");
								switch(va[i])
								{
									case ATA_CMD_DEV_RESET:
										strcpy(COMMAND_OUTPUT,"Dev Reset");
										enable_read=0;
										break;
									case ATA_CMD_CHK_POWER:
										strcpy(COMMAND_OUTPUT,"Check Power");
										enable_read=0;
										break;
									case ATA_CMD_STANDBY:
										strcpy(COMMAND_OUTPUT,"Standby !");
										enable_read=0;
										break;
									case ATA_CMD_IDLE:
										strcpy(COMMAND_OUTPUT,"Idle !");
										enable_read=0;
										break;
									case ATA_CMD_ID_ATA:
										strcpy(COMMAND_OUTPUT,"Identify Drive !");
										start_offset=0;
										enable_read=1;
										break;
									case ATA_CMD_PACKET:
										strcpy(COMMAND_OUTPUT,"Packet![TODO: not implement]");
										enable_read=0;
										break;
									case ATA_CMD_ID_ATAPI:
										strcpy(COMMAND_OUTPUT,"Try ATAPI");
										enable_read=0;
										break;
									default:
										strcpy(COMMAND_OUTPUT,"Unknown !");
										enable_read=0;
										break;

								}
								break;
							default:
								strcpy(REG_OUTPUT,"Unknown");
								enable_read=0;
								break;
						}
						dev_info(&(fake_pata_device_p->dev), "CBR [%d][%15.15s] [0x%X] ==> [0x%X] %s\n", 
								i, REG_OUTPUT,
								(unsigned char)va_l[i] , (unsigned char)va[i],
								COMMAND_OUTPUT);  
					}

				}
			}
			if(enable_read)
			{
				read_things();/*fake read handler, 
						real disk should read data from disk now */
			}
			else
			{
				refresh_status();
			}
			memcpy(va_l,va, RES_MEM_LENGTH) ;
		}

		/* compare Control Reg */
		{
			unsigned char* va_ctl_l=page_address(last_ctl_phy_page);
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

int __init fake_pata_init(void)
{

	/* the resource will be initialized after device regist */
	
	/* For IO resource */
	last_phy_page = alloc_page(GFP_KERNEL);

	/* For Control resource */
	last_ctl_phy_page = alloc_page(GFP_KERNEL);

	init_fake_device(&fake_pata_device);
	memcpy(page_address(last_phy_page),va, RES_MEM_LENGTH) ;
	memcpy(page_address(last_ctl_phy_page),va_ctl, RES_MEM_LENGTH) ;

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
	platform_device_unregister(&fake_pata_device);
	if (!IS_ERR(tsk)){  
		int ret = kthread_stop(tsk);  
		printk(KERN_INFO "thread stoped with ret[%d]\n", ret);  
	}  
}

module_init(fake_pata_init);
module_exit(fake_pata_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Gordon <gzg1984@gmail.com>");
MODULE_VERSION("0.1");

