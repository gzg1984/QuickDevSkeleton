#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include "inode.h"
extern struct inode* get_example_fs_root_inode(struct super_block *s)
{
        struct inode* temp_inode_to_return=NULL;
	temp_inode_to_return= new_inode(s); /** only here use super block **/
        temp_inode_to_return->i_ino = 1;
        temp_inode_to_return->i_mtime = temp_inode_to_return->i_atime = temp_inode_to_return->i_ctime = CURRENT_TIME;
        temp_inode_to_return->i_mode = S_IFDIR | S_IRUGO | S_IXUGO | S_IWUSR;
        temp_inode_to_return->i_op = &simple_dir_inode_operations;
        temp_inode_to_return->i_fop = &simple_dir_operations;
        return temp_inode_to_return;
}

