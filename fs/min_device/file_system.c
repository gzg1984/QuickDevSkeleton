#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/namei.h>
#include <linux/mount.h>

#include "fill_super.h"
static struct dentry *example_fs_mount(struct file_system_type *fs_type,
		        int flags, const char *dev_name, void *data)
{
	return mount_bdev(fs_type, flags, dev_name, data, example_fill_super); 
}
static struct file_system_type example_fs_type = {
	.name		= "example_fs",
	.mount          = example_fs_mount,
	.kill_sb	= kill_block_super, /** use the system default routin**/
};

static int __init init_example_fs(void)
{
	int err = register_filesystem(&example_fs_type);
	if (err) {
		unregister_filesystem(&example_fs_type);
	}
	return err;
}
static void __exit exit_example_fs(void)
{
	unregister_filesystem(&example_fs_type);
}
module_init(init_example_fs)
module_exit(exit_example_fs)
MODULE_LICENSE("GPL");
