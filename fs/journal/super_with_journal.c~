#include <linux/blkdev.h>
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
#include "../journal.h"
static const struct super_operations example_sops = {
	.statfs         = simple_statfs,
};
journal_t *example_journal;


/* Must implement ! will be called in mount routin in "file system -> mount " function **/
static journal_t *example_init_journal( struct block_device *bdev,        
		unsigned long           blocksize);
int example_fill_super(struct super_block *s, void *data, int silent)
{
	int err;
	struct inode *inode;
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
	/**2017 June 3rd **/
	/*******Add journal **/
	example_journal=example_init_journal(s->s_bdev,s->s_blocksize); //use my block device to init journal
	err = journal_load(example_journal); /** get_the data from the journal dev 
					      this two functions are all work for the final load **/ 
	if (err) {
		printk(KERN_ERR"error loading journal");
		journal_destroy(example_journal);
		example_journal=NULL;
		return err;
	}

	/*******end of  journal **/
	s->s_root = d_make_root(inode);
	if (s->s_root)
		return 0;

	pr_err("get root dentry failed\n");

fail:
	return -ENOMEM;

}

/***************Journal part *******/
static journal_t *example_init_journal( struct block_device *bdev,        
		unsigned long           blocksize)
{
	struct buffer_head * bh;
	journal_t *journal;
	u64 start;
	u64 len;
	int hblock;
	u64 sb_block;
	unsigned long offset;
	char* UUID;

	hblock = bdev_logical_block_size(bdev);
	if (blocksize < hblock) {
		printk(KERN_ERR"error: blocksize too small for journal device");
		goto out_bdev;
	}

	sb_block = 1;
	offset = 0;;
	set_blocksize(bdev, blocksize);
	if (!(bh = __bread(bdev, sb_block, blocksize))) {
		printk(KERN_ERR"error: couldn't read superblock of "
			"external journal");
		goto out_bdev;
	}
	else
	{
		printk(KERN_INFO"__bread super block success. sb_block[%d]  blocksize[%d]\n",
				sb_block,blocksize);
	}

	/** Journal super block is same as ext3_super_block **/
	/*68*/ /* __u8    s_uuid[16];*/             /* 128-bit uuid for volume */
	/*D0*/ /* __u8    s_journal_uuid[16]; */    /* uuid of journal superblock */
	UUID = bh->b_data + 0x68;
	printk(KERN_INFO "journal UUID is [%pUL]\n",UUID);

		/*00*/ /* __le32  s_inodes_count;*/         /* Inodes count */
		       /* __le32  s_blocks_count;*/         /* Blocks count */

	len = le32_to_cpu(bh->b_data+0x4);
	start = sb_block + 1;
	brelse(bh);	/* we're done with the superblock */

	journal = journal_init_dev(bdev/* journal device **/, 
			bdev /* data device ,in my test , they are same device **/, 
					start, len, blocksize);
	if (!journal) {
		printk(KERN_ERR"error: failed to create device journal");
		goto out_bdev;
	}
	journal->j_private = NULL;/** used to point to the data device superblock **/
	if (!bh_uptodate_or_lock(journal->j_sb_buffer)) {
		if (bh_submit_read(journal->j_sb_buffer)) {
			printk(KERN_ERR"I/O error on journal device");
			goto out_journal;
		}
	}
	if (be32_to_cpu(journal->j_superblock->s_nr_users) != 1) {
		printk(KERN_ERR"error: external journal has more than one "
			"user (unsupported) - %d",
			be32_to_cpu(journal->j_superblock->s_nr_users));
		goto out_journal;
	}

	return journal;
out_journal:
	journal_destroy(journal);
out_bdev:
	return NULL;
}
