#include <linux/time.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include "../file.h"

/* All function are defined in the VFS **/
const struct file_operations example_file_operations = {
	.llseek		= generic_file_llseek,
	.read_iter      = generic_file_read_iter,
	.write_iter     = generic_file_write_iter,
	.mmap		= generic_file_mmap,
	.open		= generic_file_open,
	.splice_read	= generic_file_splice_read,
	.splice_write	= iter_file_splice_write,
};

