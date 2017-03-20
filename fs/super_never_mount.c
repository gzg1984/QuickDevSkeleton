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

#include "fill_super_with_simple_operation.h"

/*
 *         struct super_block *(*get_sb) (struct file_system_type *, int,
 *                                                const char *, void *);
 *                                                */
static struct file_system_type example_fs_type = {
	.name		= "example_fs",
	.get_sb=NULL, /* in old kernel ,use get_sb**/
	//.mount		= NULL,/* in new kernel ,use mount,*/
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
