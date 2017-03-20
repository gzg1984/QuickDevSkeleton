#ifndef __DEVPTS_FILL_SUPER__
#define __DEVPTS_FILL_SUPER__

#include <linux/fs.h>

struct example_fs_info {
	//struct ida allocated_ptys;
	//struct pts_mount_opts mount_opts;
	struct super_block *sb;
	struct dentry *ptmx_dentry;
};


extern int example_fill_super(struct super_block *s, void *data, int silent);
#endif
