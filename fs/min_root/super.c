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

#include "../fill_super.h"
#include "../inode.h"
static const struct super_operations example_sops = {
	.statfs         = simple_statfs,

};

int example_fill_super(struct super_block *s, void *data, int silent)
{
	struct inode *inode;
	printk("Enter example_fill_super \n");

	s->s_blocksize = 1024;
	s->s_blocksize_bits = 10;
	s->s_op = &example_sops;
	s->s_time_gran = 1;

	s->s_fs_info = NULL;/* normally ,we need our own struct. but now we ignor this **/

	/** this function is in inode.h/inode.c **/
	inode = get_example_fs_root_inode(s); /** pass the super block **/
	if (!inode)
	{
		printk("CAN NOT create new inode \n");
		goto fail;
	}
	printk("before create roote inode. If miss this step ,system will crash\n");
	s->s_root = d_make_root(inode);
	if (s->s_root)
		return 0;

	pr_err("get root dentry failed\n");

fail:
	return -ENOMEM;

}
