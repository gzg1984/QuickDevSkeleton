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

struct super_block *example_get_sb(struct file_system_type *fs_type,
		        int flags, const char *dev_name, void *data)
{
	        return get_sb_nodev(fs_type, flags, data, NULL/** mount will die because no fill_sb function**/);
}       
    
static struct file_system_type example_fs_type = {
	.name		= "example_fs",
	.get_sb = example_get_sb, /** Hey, it is defined in this file**/
	.kill_sb	= NULL,
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
