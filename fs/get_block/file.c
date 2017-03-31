#include <linux/time.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include "../file.h"

/* All function are defined in the VFS **/
const struct file_operations example_file_operations = {
	.llseek		= generic_file_llseek,
	.read		= do_sync_read,
	.write		= do_sync_write,
	.aio_read	= generic_file_aio_read,
	.aio_write	= generic_file_aio_write,
	.mmap		= generic_file_mmap,
	.open		= generic_file_open,
	.splice_read	= generic_file_splice_read,
	.splice_write	= generic_file_splice_write,
};

