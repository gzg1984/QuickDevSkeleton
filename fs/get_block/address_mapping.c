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
	map_bh(bh_result, inode->i_sb, 0);
//	return 1; /* do we mean, we got 1 buffer ? **/
	return 0;
}

static int example_readpage(struct file *fp, struct page *private_page) /** almost same as the read.c in class 8, but no content copy code **/
{
	return mpage_readpage(private_page, example_get_block);
}
struct address_space_operations example_address_space_ops = {
        .readpage               = example_readpage,
};
