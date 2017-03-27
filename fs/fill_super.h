#ifndef __DEVPTS_FILL_SUPER__
#define __DEVPTS_FILL_SUPER__

#include <linux/fs.h>

struct example_fs_info {
	struct super_block *sb;
};

extern int example_fill_super(struct super_block *s, void *data, int silent);
#endif
