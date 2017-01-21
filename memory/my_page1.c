#include <linux/module.h>
#include <linux/sched.h>
#include <linux/mm.h>

static int order;
module_param(order, int, 0444);

struct page *buf;

static int __init my_init(void)
{
	void *buf_addr;
	//alloc pages
	buf = alloc_pages(GFP_HIGHUSER, order);
	if (NULL == buf)
		return -ENOMEM;

	//get virtual address
	buf_addr = page_address(buf);

	printk("Get %d pages, phy_addr = 0x%lx; vir_addr = 0x%p\n", 1<<order, (unsigned long)((buf - mem_map)<<PAGE_SHIFT), buf_addr);

	printk("max_mapnr = 0x%lx\n", max_mapnr);
	printk("num_physpages = 0x%lx\n", num_physpages);
	printk("high_memory = 0x%p\n", high_memory);
	printk("mem_map = 0x%p\n", mem_map);
	printk("sizeof page = %d\n", sizeof(struct page));
	return 0;
}


static void __exit my_exit(void)
{
	__free_pages(buf, order);
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ZHANG");
MODULE_VERSION("0.5");

