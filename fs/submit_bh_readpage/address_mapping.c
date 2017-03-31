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

static int example_readpage(struct file *fp, struct page *private_page) /** almost same as the read.c in class 8, but no content copy code **/
{
	struct buffer_head * bh;
	bh = alloc_page_buffers(private_page, 4096,/*the total size of buffer head **/
			0/* retry ? NO! */);
	if (!bh)
	{
		printk("alloc_page_buffers fail \n");
		return 0;
	}

	/* the path of creating the buffer head **/
	lock_buffer(bh); /** important !!! if we don't lock , submit_bh will crash system!*/
	set_buffer_mapped(bh); /** important !! if we don't set mapped, submit_bh will crash system !*/
	bh->b_bdev = fp->f_mapping->host->i_sb->s_bdev; /* the device that I want to access **/
	bh->b_blocknr = 0; /* the block that I want to get **/         
	bh->b_size = fp->f_mapping->host->i_sb->s_blocksize; /** the block size that I use... wait... should it be same as the block dev setting ? no? **/
	get_bh(bh); /** add buffer head used count **/
	bh->b_end_io = end_buffer_read_sync; /* this is for all normal buffer ?? **/
	printk("before submit_bh\n");
	submit_bh(READ, bh); /** submit ! it will became bios **/
	printk("before wait_on_buffer\n");
	wait_on_buffer(bh); /** so , the VFS wait for readpage, but the readpage wait for buffer io end **/
	/*****/

	printk("before SetPageUptodate\n");
	SetPageUptodate(private_page); 
	unlock_page(private_page); 
	return 0;
}
struct address_space_operations example_address_space_ops = {
        .readpage               = example_readpage,
};
