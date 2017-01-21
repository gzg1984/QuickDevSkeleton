#include <linux/module.h>
#include <linux/sched.h>
#include <linux/time.h>
#include <linux/proc_fs.h>

int my_proc(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int ret = 0;
	u64 jif_64;
	struct timeval tval;
	struct timespec tspec;

	jif_64 = get_jiffies_64();
	do_gettimeofday(&tval);
	tspec = current_kernel_time();

	ret += sprintf(page+ret, "Current time is: \n");
	ret += sprintf(page+ret, "Use jiffies: 0x%lx\n", jiffies);
	ret += sprintf(page+ret, "Use jiffies_64: 0x%llx\n", jiffies_64);
	ret += sprintf(page+ret, "Use timeval: %lds, %ldus\n", tval.tv_sec, tval.tv_usec);
	ret += sprintf(page+ret, "Use timespec: %lds, %ldns\n", tspec.tv_sec, tspec.tv_nsec);

	return ret;
}

int __init my_init(void)
{
	create_proc_read_entry("proc_time", /* name */
			0,		/* mode */
			NULL,	/* parent */
			my_proc,/* func */
			NULL);	/* parameter */

	return 0;
}

void __exit my_exit(void)
{
	remove_proc_entry("proc_time", NULL);
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ZHANG");
MODULE_VERSION("0.9");
