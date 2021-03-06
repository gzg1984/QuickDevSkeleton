#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/namei.h>
#include <linux/mount.h>

#include "../fill_super.h"
/** Must implement ! */
static struct dentry *example_fs_mount(struct file_system_type *fs_type,
		        int flags, const char *dev_name, void *data)
{
	if(!try_module_get(THIS_MODULE))
	{
		return NULL;
	}
	else
	{
		return mount_bdev(fs_type, flags, dev_name, data, example_fill_super); 
	}

}
void example_kill_block_super(struct super_block *sb)
{
	module_put(THIS_MODULE);
	kill_block_super(sb);
}
static struct file_system_type example_fs_type = {
	.name		= "example_fs",
	/** use mount function to call the "fill super" function **/
	.mount          = example_fs_mount, 
	/** Remenber to put the module as we get the module while mount
	 * this will prevent rmmod before umount **/
	.kill_sb	= example_kill_block_super, 
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
