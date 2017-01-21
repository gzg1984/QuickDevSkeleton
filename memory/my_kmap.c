#include <linux/module.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/highmem.h>

static int order;
module_param(order, int, 0444);

struct page *buf;
void *kmap_buf;

static int __init my_init(void)
{
	//alloc pages
	buf = alloc_pages(GFP_HIGHUSER, order);
	if (NULL == buf)
		return -ENOMEM;

	//get virtual address
	kmap_buf = kmap(buf);

	printk("Get %d pages, phy_addr = 0x%lx; vir_addr = 0x%p\n", 1<<order, (unsigned long)((buf - mem_map)<<PAGE_SHIFT), kmap_buf);
	printk("Total high pages = 0x%lx, bytes = 0x%lx\n", totalhigh_pages, totalhigh_pages<<PAGE_SHIFT);

	return 0;
}


static void __exit my_exit(void)
{
	kunmap(kmap_buf);
	__free_pages(buf, order);
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ZHANG");
MODULE_VERSION("0.5");

