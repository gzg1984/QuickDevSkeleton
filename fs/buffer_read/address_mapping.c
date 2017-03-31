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
	char* buffer;
        struct buffer_head * bh;

//	buffer=kmap(private_page); // in this way, as bio, no need to copy the data manully

/*****/
        /*
         * Allocate some buffers for this page
		never free!! for now!
         */
        bh = alloc_page_buffers(private_page, 4096,/*the total size of buffer head **/
				 0/* retry ? NO! */);
	//init_buffer(bh, NULL, NULL);
        if (!bh)
	{
		printk("alloc_page_buffers fail \n");
		return 0;
	}
        lock_buffer(bh); /** important !!! if we don't lock , submit_bh will crash system!*/
        set_buffer_mapped(bh);



	/* this function was only for grow_dev_page **/

        //link_dev_buffers(page, bh); /** just link the page and buffer head, via modify private pointer,
/*
static inline void
link_dev_buffers(struct page *page, struct buffer_head *head)
{
        struct buffer_head *_bh, *tail;

        _bh = bh;
        do {
                tail = _bh;
                _bh = _bh->b_this_page;
        } while (_bh);
        tail->b_this_page = bh;
        attach_page_buffers(private_page, bh);
}
*/
					/**do nothing about "dev" and page radix tree **/
	/** normally block is small than page **/

/* following is simply this ? or not ?
*/
        bh->b_bdev = fp->f_mapping->host->i_sb->s_bdev;
        bh->b_blocknr = 0;          
        bh->b_size = fp->f_mapping->host->i_sb->s_blocksize;

//        init_page_buffers(private_page, fp->f_mapping->host->i_sb->s_bdev,/*link to the block_device **/
//			 			0/** number 0 block **/, 
//						4096/* page buffer size **/); /** we can init a block device page cache to bdev,
//									we can init a file page cache to bdev too**/

                get_bh(bh);
                bh->b_end_io = end_buffer_read_sync; /* this is for all normal buffer ?? **/
                submit_bh(READ, bh);
                wait_on_buffer(bh); /** so , the VFS wait for readpage, but the readpage wait for buffer io end **/
/*****/



//        memcpy(buffer, bh->b_data ,4096);
//	kunmap(private_page);
        SetPageUptodate(private_page); 
        unlock_page(private_page); 
	return 0;
}
struct address_space_operations example_address_space_ops = {
        .readpage               = example_readpage,
};
