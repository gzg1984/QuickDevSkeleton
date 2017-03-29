#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/namei.h>
#include <linux/slab.h>
#include <linux/mount.h>
#include <linux/tty.h>
#include <linux/mutex.h>
#include <linux/idr.h>
#include <linux/parser.h>
#include <linux/fsnotify.h>
#include <linux/seq_file.h>

#include "fill_super.h"
					/*Important! I will implement it in another source file**/
/*used in old kernel */					/*the cotent of super_block ,and the root inode , will be initialed in thisl "fill"  function*/
/*
struct super_block *example_get_sb(struct file_system_type *fs_type,
		                        int flags, const char *dev_name, void *data)
{
	                return get_sb_nodev(fs_type, flags, data, 
					example_fill_super
);
}
*/
/*used in newer kernel,samly use fill_super**/
static struct dentry *example_fs_mount(struct file_system_type *fs_type,
		        int flags, const char *dev_name, void *data)
{
	        return mount_nodev(fs_type, flags, data, example_fill_super);
}


static void example_kill_sb(struct super_block *sb)
{
	kill_litter_super(sb);
}

static struct file_system_type example_fs_type = {
	.name		= "example_fs",
	//.get_sb=example_get_sb,  in old kernel ,use get_sb**/
	.mount          = example_fs_mount,
	.kill_sb	= example_kill_sb,/** you need it on "unmount" and "mount failed"**/ 
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
