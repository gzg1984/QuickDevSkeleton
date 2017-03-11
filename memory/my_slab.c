#include <linux/module.h>
#include <linux/sched.h>
#include <linux/proc_fs.h>

#define ITEM_LEN 1000

struct dev {
	int count;
	struct list_head proc_list;
};

struct item {
	unsigned char value;
	struct list_head proc_item;
}__attribute__((packed));

static struct dev mydev = {
	.count = 0,
	.proc_list = LIST_HEAD_INIT(mydev.proc_list),
};

static struct kmem_cache *my_cache;


int proc_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int ret = 0;
	struct item *pos;

	ret += sprintf(page+ret, "count = %d\n", mydev.count);

	list_for_each_entry_reverse(pos, &mydev.proc_list, proc_item)
		ret += sprintf(page+ret, "Value = %d\n", pos->value);

	return ret;
}


int __init my_init(void)
{
	int i;
	struct item *pos, *tmp;

	my_cache = kmem_cache_create("my_cache",		 /* name */
								sizeof(struct item), /* size */
								0,					 /* align */
								SLAB_PANIC, /* flags */
								NULL);				 /* constructor */
	printk("Sizeof item = %ld\n", sizeof(struct item));


	for (i=0; i<ITEM_LEN; i++) {
		pos = kmem_cache_alloc(my_cache, GFP_KERNEL);
		if (NULL == pos)
			goto err;
		pos->value = i;
		INIT_LIST_HEAD(&pos->proc_item);
		list_add(&pos->proc_item, &mydev.proc_list);
		mydev.count++;
	}

	create_proc_read_entry("proc_slab", 0, NULL, proc_read, my_cache);
	return 0;
err:
	list_for_each_entry_safe(pos, tmp, &mydev.proc_list, proc_item) {
		kmem_cache_free(my_cache, pos);
	}
	kmem_cache_destroy(my_cache);
	return -ENOMEM;
}


void __exit my_exit(void)
{
	struct item *pos, *tmp;
	remove_proc_entry("proc_slab", NULL);
	list_for_each_entry_safe(pos, tmp, &mydev.proc_list, proc_item) {
		kmem_cache_free(my_cache, pos);
	}
	kmem_cache_destroy(my_cache);
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ZHANG");
MODULE_VERSION("0.7");


