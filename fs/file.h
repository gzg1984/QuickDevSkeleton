#include <linux/fs.h>
ssize_t example_read (struct file *fp, char __user *bp, size_t len, loff_t *pp);

extern const struct file_operations example_file_operations;
extern const struct inode_operations example_file_inode_operations;
