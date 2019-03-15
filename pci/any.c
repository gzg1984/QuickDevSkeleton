#include <linux/module.h>
#include <linux/pci.h>
#include <linux/cdev.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");
static struct pci_device_id gp_tbl[] = {
	{
		PCI_ANY_ID, 
		PCI_ANY_ID, 
		PCI_ANY_ID, 
		PCI_ANY_ID,
		0,
		0,
		0,
	},
	{0,}
};
MODULE_DEVICE_TABLE(pci,gp_tbl);


//struct pci_dev {
//	unsigned short vendor_compatible[DEVICE_COUNT_COMPATIBLE];
//	unsigned short device_compatible[DEVICE_COUNT_COMPATIBLE];
//};

struct class* pf_class;

struct pci_test
{
	struct cdev cds;
	struct resource * io_resource;
	struct resource * mem_resource;
	char* iomem_start;
	struct net_device *ndev;
};

void  girlf  (struct pci_dev *dev)
{
	struct pci_test* pt= dev->dev.driver_data ;
	printk("##########girlf running ... ##############################\n");
	if(dev->vendor == 0x10ec && dev->device == 0x8139)
	{
		device_destroy(pf_class,pt->cds.dev);
		cdev_del(&(pt->cds));
		unregister_chrdev_region(pt->cds.dev,1);
		iounmap(pt->iomem_start);
		pci_disable_device (dev);
		printk("pci_disable_device[success]\n");
		release_region(dev->resource[0].start,
				(dev->resource[0].end - dev->resource[0].start +1));
		release_mem_region(dev->resource[1].start,
				(dev->resource[1].end - dev->resource[1].start +1));
		free_netdev(pt->ndev);
		dev->dev.driver_data = NULL;
	}
	return ;
}

int co(struct inode* inp,struct file* fp)
{
	struct pci_test* pt;
	fp->private_data=container_of(inp->i_cdev,struct pci_test, cds);
	pt = fp->private_data;
	printk("char open\n");
	printk("io_resource start[0x%08llX]end[0x%08llX]size[0x%08llX]flags[0x%08lX]\n",
			pt->io_resource->start,pt->io_resource->end,
			(pt->io_resource->end - pt->io_resource->start +1),
			pt->io_resource->flags);
	printk("mem_resource start[0x%08llX]end[0x%08llX]size[0x%08llX]flags[0x%08lX]\n",
			pt->mem_resource->start,pt->mem_resource->end,
			(pt->mem_resource->end - pt->mem_resource->start +1),
			pt->mem_resource->flags);
	return 0;
}

int cre(struct inode* inp,struct file* fp)
{
	printk("char close\n");
	return 0;
}

int cr(struct file* fp,char __user * target,size_t len,loff_t* offer)
{
	int target_len ;
	struct pci_test* pt = fp->private_data;
	int buffer_len = pt->mem_resource->end - pt-> mem_resource->start +1;
	printk("char read[%s][%d][%ld]\n",
			(char*)fp->private_data,len,(long)(*offer));
	target_len= (len > ( buffer_len - (*offer) )) ?
			(buffer_len - (*offer) ):len ;
	if(*offer >= buffer_len)
	{
		return 0;
	}
	if(copy_to_user(target,
			&((pt->iomem_start)[(*offer)]) , target_len) )
	{
		printk("char read error\n");
		return -1;
	}
	else
	{
		*offer+=target_len;
		return target_len;
	}
}

struct file_operations fop=
{
	.owner=THIS_MODULE,
	.open=co,
	.read=cr,
	.release=cre,
};

/**
static const struct net_device_ops ndops = {
    .ndo_validate_addr  	= eth_validate_addr,
    .ndo_set_mac_address    = eth_mac_addr,
};
**/


int  girlp  (struct pci_dev *dev, const struct pci_device_id *id)
{
	int rc ;
	int minor ;
	struct pci_test* pt;
	unsigned long pio_start, pio_end, pio_flags, pio_len;
	unsigned long mmio_start, mmio_end, mmio_flags, mmio_len;
	struct net_device *ndev;


	printk("##########girlp running ... ##############################\n");
	printk("devfn[0x%08X]\nvender[0x%08X]device[0x%08X]\n",
			dev->devfn,dev->vendor,dev->device);
	printk("subsystem_verdor[0x%08X]subsystem_device[0x%08X]\nclass[0x%08X]\n",
			dev->subsystem_vendor,dev->subsystem_device,dev->class);
	printk("hdr_type[0x%08X]rom_base_reg[0x%08X]pin[0x%08X]dma_mask[0x%08llX]\n",
			dev->hdr_type,dev->rom_base_reg,dev->pin,dev->dma_mask);
	printk("cfg_size[0x%08X]irq[0x%08X]rom_attr_enabled[0x%08X]\n",
			dev->cfg_size,dev->irq,dev->rom_attr_enabled);
	/**
	printk(
			"transparent:multifunction:is_enabled:is_busmaster:no_msi:no_d1d2[%d:%d:%d:%d:%d:%d]\n",
			dev->transparent ,dev->multifunction ,dev->is_enabled,
			dev->is_busmaster,dev->no_msi,dev->no_d1d2);
			**/
	printk( "transparent:multifunction:is_busmaster:no_msi:no_d1d2[%d:%d:%d:%d:%d]\n",
			dev->transparent ,dev->multifunction ,
			dev->is_busmaster,dev->no_msi,dev->no_d1d2);
/*
	printk(
			"block_ucfg_access:broken_parity_status:msi_enabled:msix_enabled:is_managed"
			"[%d:%d:%d:%d:%d]\n",
			dev->block_ucfg_access ,dev->broken_parity_status ,dev->msi_enabled,
			dev->msix_enabled,dev->is_managed);
*/
	if(dev->vendor == 0x10ec && dev->device == 0x8139)
	{
		printk("#####################################\n");
		printk("############## NET CARD##############\n");
		printk("#####################################\n");
/******************* net device ***********/
		/* dev and priv zeroed in alloc_etherdev */
		ndev = alloc_etherdev (sizeof (*pt));
		if (ndev == NULL) 
		{
			dev_err(&dev->dev, "Unable to alloc new net device\n");
			girlf(dev);
			return -ENOMEM;
		}
		else
		{
			printk("padded[%ud]\n",ndev->padded);
		}
		SET_NETDEV_DEV(ndev, &dev->dev);

		pt = netdev_priv(ndev);
		pt->ndev = ndev;
		dev->dev.driver_data=pt;
		ndev->irq =  dev->irq;
		ndev->features |= NETIF_F_SG | NETIF_F_HW_CSUM | NETIF_F_HIGHDMA;
		/**
		ndev->netdev_ops = &netdev_ops;
		**/
		if( register_netdev (ndev))
		{
			girlf(dev);
			return -1;
		}

/******************* net device ***********/
		rc = pci_enable_device (dev);
		if (rc)
		{
			printk("pci_enable_device[faild]\n");
			girlf(dev);
			return -1;
		}
		else
		{
			printk("pci_enable_device[succesd]\n");
		}

		pio_start = dev->resource[0].start;
		pio_end = dev->resource[0].end;
		pio_flags = dev->resource[0].flags;
		pio_len = pio_end - pio_start + 1;

		mmio_start = dev->resource[1].start;
		mmio_end = dev->resource[1].end;
		mmio_flags = dev->resource[1].flags;
		mmio_len = mmio_end - mmio_start + 1;

		printk("PIO region start[0x%08lX]end[0x%08lX]size[0x%08lX]flags[0x%08lX]\n", 
				pio_start,pio_end,pio_len,pio_flags);
		printk("MMIO region start[0x%08lX]end[0x%08lX]size[0x%08lX]flags[0x%08lX]\n",
				mmio_start,mmio_end,mmio_len,mmio_flags);

		pt->io_resource = request_region(pio_start,pio_len,"maxpain");
		if(pt->io_resource == NULL)
		{
			printk("request_region err!");
			girlf(dev);
			return -1;
		}
		else
		{
			printk("request_region[succesd]\n");
			printk("RESOURCE start[0x%08llX]end[0x%08llX]size[0x%08llX]flags[0x%08lX]\n",
					pt->io_resource->start,pt->io_resource->end,
					(pt->io_resource->end - pt->io_resource->start +1),
					pt->io_resource->flags);
		}
		pt->mem_resource=request_mem_region(mmio_start,mmio_len,"gainer");
		if(pt->mem_resource == NULL)
		{
			printk("request_region err!");
			girlf(dev);
			return -1;
		}
		else
		{
			printk("request_mem_region[succesd]\n");
			printk("RESOURCE start[0x%08llX]end[0x%08llX]size[0x%08llX]flags[0x%08lX]\n",
					pt->mem_resource->start,pt->mem_resource->end,
					(pt->mem_resource->end - pt->mem_resource->start +1),
					pt->mem_resource->flags);
		}

		pt->iomem_start = ioremap(pt->mem_resource->start,
				(pt->mem_resource->end - pt->mem_resource->start +1 ));
		if(pt->iomem_start == NULL)
		{
			printk("ioremap err!");
			girlf(dev);
			return -1;
		}
/**************************net device ***********/
		ndev->base_addr = (unsigned long )pt->iomem_start;
/**************************net device ***********/
		/********************* char dev *************/
		cdev_init(&(pt->cds),&fop);
		pt->cds.owner=THIS_MODULE;

		rc = 0;
		minor = 0;
		do
		{
			rc=alloc_chrdev_region(&(pt->cds.dev),minor++,1,"tempNet");
		}
		while(rc);
		printk("alloc_chrdev_region[%d][%d]\n", 
				MAJOR(pt->cds.dev),
				MINOR(pt->cds.dev));

		rc = cdev_add(&(pt->cds),pt->cds.dev,1);
		if (rc) 
		{
			printk("cdev_add err!");
			girlf(dev);
			return -1;
		}
		/********************* char dev *************/
		device_create(pf_class,NULL,
				pt->cds.dev, NULL,"net-%d",MAJOR(pt->cds.dev));
	}


	return 0;
}



static struct pci_driver gp = {
	.name		= "maxpain",
	.id_table	= gp_tbl,
	.probe		= girlp,
	.remove		= girlf,
};

static int __init igp (void)
{
	pf_class= class_create(THIS_MODULE,"pf_class");
	if( IS_ERR(pf_class) )
	{
		printk("class_create error\n");
		return PTR_ERR(pf_class);
	}
	else
	{
		printk("class_create success\n");
	}
	return pci_register_driver(&gp);
}


static void __exit rgp (void)
{
	class_destroy(pf_class);
	pci_unregister_driver (&gp);
	return;
}


module_init(igp);
module_exit(rgp);
