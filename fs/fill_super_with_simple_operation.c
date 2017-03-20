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





static int devpts_remount(struct super_block *sb, int *flags, char *data)
{
	        int err=0;

			        //sync_filesystem(sb);

					        return err;

}


#define PARSE_MOUNT	0
#define PARSE_REMOUNT	1


static int devpts_show_options(struct seq_file *seq, struct dentry *root)
{
	return 0;
}
static const struct super_operations devpts_sops = {
	.statfs         = simple_statfs,
	.remount_fs     = devpts_remount,
	.show_options   = devpts_show_options,

};



static void *new_example_fs_info(struct super_block *sb)
{
	struct example_fs_info *fsi;

	fsi = kzalloc(sizeof(struct example_fs_info), GFP_KERNEL);
	if (!fsi)
		return NULL;

	//ida_init(&fsi->allocated_ptys);
//	fsi->mount_opts.mode = DEVPTS_DEFAULT_MODE;
//	fsi->mount_opts.ptmxmode = DEVPTS_DEFAULT_PTMX_MODE;
	fsi->sb = sb;

	return fsi;
}

int example_fill_super(struct super_block *s, void *data, int silent)
{
	struct inode *inode;

	s->s_blocksize = 1024;
	s->s_blocksize_bits = 10;
	//s->s_magic = DEVPTS_SUPER_MAGIC;
//	s->s_op = &devpts_sops;
	s->s_time_gran = 1;

	s->s_fs_info = new_example_fs_info(s);
	if (!s->s_fs_info)
		goto fail;

	inode = new_inode(s);
	if (!inode)
		goto fail;
	inode->i_ino = 1;
	inode->i_mtime = inode->i_atime = inode->i_ctime = CURRENT_TIME;
	inode->i_mode = S_IFDIR | S_IRUGO | S_IXUGO | S_IWUSR;
	inode->i_op = &simple_dir_inode_operations;
	inode->i_fop = &simple_dir_operations;
	//set_nlink(inode, 2);

	//s->s_root = d_make_root(inode);
	//if (s->s_root)
		return 0;

	//pr_err("get root dentry failed\n");

fail:
	return -ENOMEM;

}
