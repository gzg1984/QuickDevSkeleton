#include <linux/module.h>
#include <linux/sched.h>
#include <linux/proc_fs.h>
#include <linux/interrupt.h>

int irq_num = 12;
module_param(irq_num, int, 0444);

unsigned int disable_count = 0;

int proc_disable(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int ret = 0;
	disable_irq(irq_num);
	disable_count++;
	ret = sprintf(page+ret, "Disable irq %d, %d times.\n", irq_num, disable_count);
	return ret;
}

int proc_enable(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int ret = 0;
	enable_irq(irq_num);
	disable_count--;
	ret += sprintf(page, "Enable irq %d, still need enable %d times.\n", irq_num, disable_count);
	return ret;
}


int __init my_init(void)
{
	create_proc_read_entry("proc_disable", 0, NULL, proc_disable, NULL);
	create_proc_read_entry("proc_enable", 0, NULL, proc_enable, NULL);
	return 0;
}

void __exit my_exit(void)
{
	remove_proc_entry("proc_disable", NULL);
	remove_proc_entry("proc_enable", NULL);
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");


