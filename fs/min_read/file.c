#include <linux/time.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include "../file.h"

ssize_t example_read (struct file *fp, char __user *bp, size_t len, loff_t *pp)
{
	printk("enter example_read len[%d] pos[%d]\n",len,*pp);
	size_t real_len=0;
	if(*pp > 3)
	{
		return 0;
	}
	real_len=3-(*pp);
	len=(len>real_len?real_len:len);

	switch(*pp)
	{
		case 0:
		copy_to_user(bp,"123",len);
		break;
		case 1:
		copy_to_user(bp,"23",len);
		break;
		case 2:
		copy_to_user(bp,"3",len);
		break;
	}
	*pp+=len;
	return len;
}


const struct file_operations example_file_operations = {
	.llseek		= generic_file_llseek,
	//.read		= do_sync_read,

	/** we can implement our own read **/
	/* if we really use the system default read, well.. because we havn't implement the address mapping operation ,so the read will die **/
	.read		= example_read,

	.write		= do_sync_write,
	.aio_read	= generic_file_aio_read,
	.aio_write	= generic_file_aio_write,
	.mmap		= generic_file_mmap,
	.open		= generic_file_open,
	.splice_read	= generic_file_splice_read,
	.splice_write	= generic_file_splice_write,
};
