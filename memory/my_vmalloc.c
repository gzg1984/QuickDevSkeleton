#include <linux/module.h>
#include <linux/sched.h>
#include <linux/vmalloc.h>

extern unsigned long vmalloc_to_pfn(const void *);

int order = 1;
module_param(order, int, 0444);

char *buf;

int __init my_init(void)
{
	int i;
	buf = (char *)vmalloc((1<<order) - 1);
	if (NULL == buf)
		return -ENOMEM;
	printk("Allocate %d bytes, vaddr = 0x%p - 0x%p\n", ((1<<order)-1), buf, (buf+(1<<order)));

	for (i=0; i<(1<<order); i++)
		buf[i] = 0x5a;

	for(i=0; i<(1<<order); i+=PAGE_SIZE) {
		printk("vmalloc page %d, vaddr=0x%p, paddr=0x%lx\n", (i>>PAGE_SHIFT)+1, buf+i, (unsigned long)(vmalloc_to_pfn(buf+i)<<PAGE_SHIFT));
	}

	return 0;
}

void __exit my_exit(void)
{
	vfree(buf);
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ZHANG");
MODULE_VERSION("0.7");

