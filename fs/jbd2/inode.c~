#include <linux/module.h>
#include <linux/init.h>

#include <linux/fs.h>
#include "../file.h"
#include "../inode.h"
#include "../address_mapping.h"
//#include "../example_journal.h"
/* Must implement #3 !*/
/* Readdir give the name of entry to the VFS, and VFS create dentry then pass to this function**/
#define JOURNAL_INUM 2
static struct dentry *example_lookup(struct inode * dir, 
		struct dentry *dentry, 
		unsigned int __unused_nd)
{
	struct inode * inode=NULL;
	printk(KERN_INFO"[%s]dentry name is [%pd]\n",__func__,dentry);
	printk(KERN_INFO"[%s]dentry name is [%s]\n",__func__,dentry->d_name.name);
	if(!strcmp(dentry->d_name.name,"UUID"))
	{
		/* init it! we should read these data from disk ,do we ?*/
		inode = iget_locked(dir->i_sb, JOURNAL_INUM);

		inode->i_mode = 0444 | (DT_REG<<12) ; /* Set as regular file **/
		inode->i_uid.val = 0x0;
		inode->i_gid.val = 0x0;
		inode->i_size = 16;/** file size 7 byte **/
		inode->i_atime = CURRENT_TIME;
		inode->i_ctime = CURRENT_TIME;
		inode->i_atime = CURRENT_TIME;

		/* this is implement in file.h */
		inode->i_fop = &example_file_operations;/* every thing goes through the VFS function **/ 
		/* Class 9 important here !*/
		inode->i_mapping->a_ops = &example_address_space_ops;

		unlock_new_inode(inode);
	}
	//example_journal = jbd2_journal_init_inode(inode);
	//example_journal = journal_init_inode(inode); //jbd1


	return d_splice_alias(inode, dentry);
}

const struct inode_operations example_dir_inode_operations = {
	.lookup         = example_lookup,
};


/* Must implement #2 !*/
static int example_readdir(struct file *file, struct dir_context *ctx)
{
        loff_t pos = ctx->pos;
	char my_only_name[100]="UUID";

	printk(KERN_INFO"Get in example_readdir ctx->pos[%lld]\n",pos);
	if (pos)
		return 0;
	ctx->pos+=1;
	/* Hey! we call the function passed by the upper, It knows how to deal
	 * the strings and numbers as the file name and the file ACL **/
	dir_emit(ctx, my_only_name, 
			strlen(my_only_name),
                                                233,/*inode number **/
                                                DT_REG /* 0x1, regular file **/);
	return 0;
}
const struct file_operations example_dir_operations = {
	.llseek         = generic_file_llseek,
	.read           = generic_read_dir,
	/* for newer kernel ,this function will be replace with ...**/
	.iterate        = example_readdir,         /* The only thing we need exactly in exapmle file system*/
	/*new readdir is implement as iterater **/
};
/* Must implement #1 !*/
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

