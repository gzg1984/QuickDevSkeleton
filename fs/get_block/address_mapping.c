#include <linux/module.h>
#include <linux/fs.h>
#include <linux/time.h>
#include <linux/pagemap.h>
#include <linux/string.h>
#include <linux/buffer_head.h>
#include <linux/mpage.h>
#include <linux/uio.h>
#include <linux/bio.h>
#include <linux/fiemap.h>
#include <linux/namei.h>
#include "../address_mapping.h"
static int example_get_block(struct inode *inode, sector_t iblock,
		                        struct buffer_head *bh_result, int create)
{
	/* if do not do map_bh and directly return , the page data will be filled with "0"**/
	printk("call example_get_block for iblock[%llu]! in [%s]\n",
			iblock,current->comm);
#ifdef DEBUG
	dump_stack();
#endif
	map_bh(bh_result, inode->i_sb, 0);/* only map one block , never map a FULL page **/

	/* this means , this buffer has map 0x2 blocks !! **/
	/* this b_size is short for buffer size, not block size **/
	bh_result->b_size=0x2 * bh_result->b_size;/* this is the most important data that get_block should fill !! **/

	return 0;
	/* if we return 1, the VFS
	 * will consider the bh map as failed ? **/
	/* return 1;*/ 
}

static int example_readpage(struct file *fp, struct page *private_page) /** almost same as the read.c in class 8, but no content copy code **/
{
	return mpage_readpage(private_page, example_get_block);
}
static int example_writepage(struct page *page, struct writeback_control *wbc)
{
	        return block_write_full_page(page, example_get_block, wbc);
}
static int
example_write_begin(struct file *file, struct address_space *mapping,
		                loff_t pos, unsigned len, unsigned flags,
				                struct page **pagep, void **fsdata)
{
	int ret;

	ret = block_write_begin(mapping, pos, len, flags, pagep,
			example_get_block);
	if (ret < 0)
	{
		printk(KERN_ERR"block_write_begin ret[%d]\n",ret);
		//ext2_write_failed(mapping, pos + len);
	}
	return ret;
}
static int example_write_end(struct file *file, struct address_space *mapping,
		                        loff_t pos, unsigned len, unsigned copied,
					                        struct page *page, void *fsdata)
{
	int ret;

	ret = generic_write_end(file, mapping, pos, len, copied, page, fsdata);
	if (ret < len)
	{
		printk(KERN_ERR"generic_write_end ret[%d] wanted len [%d]\n",ret,len);
		//ext2_write_failed(mapping, pos + len);
	}
	return ret;
}
struct address_space_operations example_address_space_ops = {
        .readpage               = example_readpage,
        .writepage               = example_writepage,
	.write_begin            = example_write_begin,
	.write_end              = example_write_end,
};
