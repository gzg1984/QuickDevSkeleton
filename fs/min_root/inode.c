#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include "../inode.h"
static struct dentry *example_lookup(struct inode * dir, 
		struct dentry *dentry, 
		struct nameidata *__unused_nd)
{
	struct inode * inode;

	/* init it! we should read these data from disk ,do we ?*/
	inode = iget_locked(dir->i_sb, 123);
	inode->i_mode = 0x666;
	inode->i_uid = 0xde;
	inode->i_gid = 0xad;
	inode->i_size = 3;
	inode->i_atime = inode->i_ctime = inode->i_atime = CURRENT_TIME;
	unlock_new_inode(inode);

	return d_splice_alias(inode, dentry);
}

const struct inode_operations example_dir_inode_operations = {
	.lookup         = example_lookup,
};



static int example_readdir(struct file * filp,
			 void * dirent, filldir_t filldir)
{
	printk("Get in example_readdir filp->f_pos[%ld]\n",filp->f_pos);
	if (filp->f_pos)
		return 0;
	filp->f_pos+=1;
	/* Hey! we call the function passed by the upper, It knows how to deal
	 * the strings and numbers as the file name and the file ACL **/
	filldir(dirent, "test1",
			5,
			0,/* offset in this folder inode**/
			233,/*inode number **/
			0x1/* regular file **/);
	return 1;
}
const struct file_operations example_dir_operations = {
	.llseek         = generic_file_llseek,
	.read           = generic_read_dir,
	/* for newer kernel ,this function will be replace with ...**/
	.readdir        = example_readdir,         /* The only thing we need exactly in exapmle file system*/
};
extern struct inode* get_example_fs_root_inode(struct super_block *s)
{
        struct inode* temp_inode_to_return=NULL;
	temp_inode_to_return= new_inode(s); /** only here use super block **/
        temp_inode_to_return->i_ino = 1;
        temp_inode_to_return->i_mtime = temp_inode_to_return->i_atime = temp_inode_to_return->i_ctime = CURRENT_TIME;
        temp_inode_to_return->i_mode = S_IFDIR | S_IRUGO | S_IXUGO | S_IWUSR;
	/* we need it after we know the file name in the folder
	 * then the VFS create a dentry ,and _lookup_ the inode for the denty*/
        temp_inode_to_return->i_op = &example_dir_inode_operations;

	/** Important! we need this function to let user read a virtual dir **/
	temp_inode_to_return->i_fop = &example_dir_operations;

        return temp_inode_to_return;
}

