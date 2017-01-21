#include <linux/module.h>
#include <linux/sched.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>

char *ker_buf;

int proc_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int ret = 0;
	if (NULL == ker_buf) {
		ret += sprintf(page+ret, "Cat /proc/proc_test\n");
		ret += sprintf(page+ret, "%s\n", (char *)data);
	} else {
		//count = min(count, (int)(PAGE_SIZE-1));
		//ker_buf[PAGE_SIZE - 1] = 0;
		ret += sprintf(page+ret, "%s\n", ker_buf);
	}

	//*eof = 1;
	return ret;
}

int proc_write(struct file *filp, const char __user *buf, unsigned long count, void *data)
{
	//free contents's of last write
	kfree(ker_buf);

	ker_buf = (char *)kzalloc(count, GFP_KERNEL);
	if (NULL == ker_buf) 
		return -ENOMEM;
	if (copy_from_user(ker_buf, buf, count)) {
		kfree(ker_buf);
		return -EFAULT;
	}

	return count;
}


int __init my_init(void)
{
	char *info = "Information from my_init";
	struct proc_dir_entry *res;

	res = create_proc_read_entry("proc_test2", /* name */
						0,		/* mode 0444 */
						NULL, 	/* parent dir */
						proc_read, /* read function */
						info);	/* data */
	if (NULL == res) {
		printk("Cannot create proc file --> /proc/proc_test\n");
		return -1;
	}

	res->write_proc = proc_write;
	
	return 0;
}

void __exit my_exit(void)
{
	remove_proc_entry("proc_test2", NULL);
	kfree(ker_buf);
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ZHANG");
MODULE_VERSION("0.4");

