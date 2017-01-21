#include <linux/module.h>
#include <linux/sched.h>
#include <linux/proc_fs.h>
#include <linux/timer.h>


struct timer_list my_timer;

void timer_func(unsigned long data)
{
	printk("In timer_func, time is %ld, context is %s\n", jiffies, current->comm);
	if (my_timer.data = (--data)) {
		mod_timer(&my_timer, jiffies+1);
	}
}


int my_proc(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int ret = 0;

	/* setup a timer */
	setup_timer(&my_timer, timer_func, 100);
	ret += sprintf(page+ret, "In %s, add a timer at %ld.\n", current->comm, jiffies);
	mod_timer(&my_timer, (jiffies + HZ/2));
	ret += sprintf(page+ret, "Ok, exit %s in time %ld\n", current->comm, jiffies);

	return ret;
}

int __init my_init(void)
{
	create_proc_read_entry("proc_timer", 0, NULL, my_proc, NULL);
	return 0;
}

void __exit my_exit(void)
{
	remove_proc_entry("proc_timer", NULL);
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ZHANG");
MODULE_VERSION("1.0");


