#include <linux/pagemap.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/time.h>
#include <linux/string.h>
#include <linux/buffer_head.h>
#include <linux/mpage.h>
#include <linux/uio.h>
#include <linux/bio.h>
#include <linux/fiemap.h>
#include <linux/namei.h>
#include <linux/blkdev.h>                           
#include <linux/writeback.h>

#include "../address_mapping.h"
#include "../journal.h"
#include "../file.h"
/*
 * akpm: A new design for ext3_sync_file().
 *
 * This is only called from sys_fsync(), sys_fdatasync() and sys_msync().
 * There cannot be a transaction open by this task.
 * Another task could have dirtied this inode.  Its data can be in any
 * state in the journalling system.
 *
 * What we do is just kick off a commit and wait on it.  This will snapshot the
 * inode to disk.
 */

int example_journal_sync_file(struct file *file, loff_t start, loff_t end, int datasync)
{
	struct inode *inode = file->f_mapping->host;
	int ret, needs_barrier = 0;
	tid_t commit_tid;
	printk(KERN_INFO"[%s]Periodically called? [%s]\n",
			__func__,current->comm);

	ret = filemap_write_and_wait_range(inode->i_mapping, start, end);
	if (ret)
	{
		printk(KERN_INFO"[%s]filemap_write_and_wait_range ret [%d]\n",
				__func__,ret);
		goto out;
	}

	printk(KERN_INFO"[%s]Before dorce commit\n",
			__func__);
	ret = journal_force_commit(example_journal);

	if (1) {
		int err;

		err = blkdev_issue_flush(inode->i_sb->s_bdev, GFP_KERNEL, NULL);
		if (!ret)
			ret = err;
	}

out:
	return ret;
}
static int example_get_block(struct inode *inode, sector_t iblock,
		struct buffer_head *bh_result, int create)
{
	/* if do not do map_bh and directly return , the page data will be filled with "0"**/
	printk("call example_get_block for iblock[%llu]! in [%s]\n",
			iblock,current->comm);
	printk("Current Journal UUID [%pUL]!\n", example_journal->j_superblock->s_uuid);
#ifdef DEBUG
	dump_stack();
#endif
	map_bh(bh_result, inode->i_sb, 0);/* only map one block , never map a FULL page **/

	/* this means , this buffer has map 0x2 blocks !! **/
	/* this b_size is short for buffer size, not block size **/
	/* if you fill wrong b_size here, like 0x1 to 0x2, you may meet core dump aftward in writepages **/
	bh_result->b_size=0x1 * bh_result->b_size;/* this is the most important data that get_block should fill !! **/
	printk(KERN_INFO"Mapped size is [%d]\n",bh_result->b_size);

	return 0;
	/* if we return 1, the VFS
	 * will consider the bh map as failed ? **/
	/* return 1;*/ 
}

static int example_readpage(struct file *fp, struct page *private_page) /** almost same as the read.c in class 8, but no content copy code **/
{
	return mpage_readpage(private_page, example_get_block);
}
/* For write_end() in data=journal mode 
   int __ext3_journal_dirty_metadata(const char *where,
   handle_t *handle, struct buffer_head *bh)
   {
   int err = journal_dirty_metadata(handle, bh);
   if (err)
   ext3_journal_abort_handle(where, __func__, bh, handle,err);
   return err;
   }*/
static int walk_page_buffers(   handle_t *handle,
		struct buffer_head *head,
		unsigned from,
		unsigned to,
		int *partial,
		int (*fn)(      handle_t *handle,
			struct buffer_head *bh))
{
	struct buffer_head *bh;
	unsigned block_start, block_end;
	unsigned blocksize = head->b_size;
	int err, ret = 0;
	struct buffer_head *next;

	for (   bh = head, block_start = 0;
			ret == 0 && (bh != head || !block_start);
			block_start = block_end, bh = next)
	{
		printk(KERN_INFO"[%s] buffer [%p]\n",
				__func__,bh);
		next = bh->b_this_page;
		block_end = block_start + blocksize;
		if (block_end <= from || block_start >= to) {
			if (partial && !buffer_uptodate(bh))
				*partial = 1;
			continue;
		}
		err = (*fn)(handle, bh);
		if (!ret)
			ret = err;
	}
	return ret;
}

static int write_end_fn(handle_t *handle, struct buffer_head *bh)
{
	struct journal_head *jh = bh2jh(bh);
	printk(KERN_INFO"[%s]handle->h_buffer_credits[%d]\n",
			__func__,handle->h_buffer_credits);
	if (!buffer_mapped(bh) || buffer_freed(bh))
		return 0;
	set_buffer_uptodate(bh);
	//	return ext3_journal_dirty_metadata(handle, bh);
	return journal_dirty_metadata(handle, bh);
}
static int do_journal_get_write_access(handle_t *handle,
		                                        struct buffer_head *bh)
{
	int dirty = buffer_dirty(bh);
	int ret;
	struct journal_head *jh = bh2jh(bh);
	printk(KERN_INFO"[%s]handle->h_buffer_credits[%d]\n",
			__func__,handle->h_buffer_credits);

	if (!buffer_mapped(bh) || buffer_freed(bh))
		return 0;
	/*
	 *          * __block_prepare_write() could have dirtied some buffers. Clean
	 *                   * the dirty bit as jbd2_journal_get_write_access() could complain
	 *                            * otherwise about fs integrity issues. Setting of the dirty bit
	 *                                     * by __block_prepare_write() isn't a real problem here as we clear
	 *                                              * the bit before releasing a page lock and thus writeback cannot
	 *                                                       * ever write the buffer.
	 *                                                                */
	if (dirty)
		clear_buffer_dirty(bh);
//	ret = ext3_journal_get_write_access(handle, bh);
	printk(KERN_INFO "[%s]Before journal_get_write_access, bh[%p]\n",
			__func__,bh); 
	ret = journal_get_write_access(handle, bh);
	printk(KERN_INFO"[%s]handle->h_buffer_credits[%d] ret [%d]\n",
			__func__,handle->h_buffer_credits,ret);
	if (!ret && dirty)
	{
		printk(KERN_ERR"ret [%d] dirty [%d]\n",ret,dirty);
		//ret = ext3_journal_dirty_metadata(handle, bh);
	}
	return ret;
}

static int example_journalled_writepage(struct page *page, struct writeback_control *wbc)
{
	handle_t * new_handle=NULL;
	int ret = 0;
	int err = 0;
	printk(KERN_ALERT"example_journalled_writepage in comm[%s]\n",current->comm);

	if (!page_has_buffers(page) || PageChecked(page)) {
		printk(KERN_ALERT"page_has_buffers(page)[%d] PageChecked(page)[%d]\n",
				page_has_buffers(page),PageChecked(page));
		/** go ahead **/
	}
	else
	{
		/*
		 *                  * It is a page full of checkpoint-mode buffers. Go and write
		 *                                   * them. They should have been already mapped when they went
		 *                                                    * to the journal so provide NULL get_block function to catch
		 *                                                                     * errors.
		 *                                                                                      */
		printk(KERN_ALERT"page_has_buffers(page)[%d] PageChecked(page)[%d]\n",
				page_has_buffers(page),PageChecked(page));
		printk(KERN_INFO"buffer mapped, page unchecked\n");
		ret = block_write_full_page(page, NULL, wbc);
		return ret;
	}

	handle_t * current_handler= journal_current_handle();
	if(current_handler)
	{
		printk(KERN_INFO"current_handler is [%p] \n",current_handler);
		printk(KERN_ERR "Somebody is handling something\n"); 
		return 0;
	}
	printk(KERN_INFO"No handler now ! current_handler is [%p] \n",current_handler);
	if (is_journal_aborted(example_journal)) 
	{
		printk(KERN_ERR "Detected aborted journal\n"); 
		return ERR_PTR(-EROFS);
	}

	printk(KERN_INFO "journal_start\n"); 
	//new_handle=jbd2__journal_start(example_journal, 1, 1, GFP_NOFS, 1, 1); //jbd2
	//new_handle = journal_start(example_journal,1); //jbd1
	new_handle = journal_start(example_journal,4); //jbd1
	if (IS_ERR(new_handle)) {
		ret = PTR_ERR(new_handle);
		return ret;
	}
	ClearPageChecked(page);
			
	printk(KERN_INFO "__block_write_begin\n"); 
	ret = __block_write_begin(page, 0, PAGE_CACHE_SIZE,
			example_get_block);
	if (ret != 0) {
		journal_stop(new_handle); //jbd1
		//jbd2_journal_stop(new_handle); //jbd2
		return 0;
	}
	printk(KERN_INFO "[%s]walk_page_buffers get_write_access\n",__func__); 
	ret = walk_page_buffers(new_handle, page_buffers(page), 0,
			PAGE_CACHE_SIZE, NULL, do_journal_get_write_access);

	printk(KERN_INFO "walk_page_buffers write_end_fn\n"); 
	err = walk_page_buffers(new_handle, page_buffers(page), 0,
			PAGE_CACHE_SIZE, NULL, write_end_fn);
	if (ret == 0)
		ret = err;
	unlock_page(page);
	//err = jbd2_journal_stop(new_handle); //jbd2
	printk(KERN_INFO "journal_stop\n"); 
	err = journal_stop(new_handle); //jbd1
	if (!ret)
		ret = err;
	return ret;


	//return block_write_full_page(page, example_get_block, wbc);
}
static int example_journalled_write_begin(struct file *file, struct address_space *mapping,
		                loff_t pos, unsigned len, unsigned flags,
				                struct page **pagep, void **fsdata)
{
	int ret;
	struct page *page;
	handle_t * current_handler;
	handle_t * new_handle;
	        pgoff_t index;
		        index = pos >> PAGE_CACHE_SHIFT;





	page = grab_cache_page_write_begin(mapping, index, flags);
	if (!page)
		return -ENOMEM;
	*pagep = page;
	printk(KERN_INFO "[%s]handle page [%p]\n",
			__func__,page); 

	current_handler= journal_current_handle();
	if(current_handler)
	{
		printk(KERN_INFO"current_handler is [%p] \n",current_handler);
		printk(KERN_ERR "Somebody is handling something\n"); 
		return 0;
	}
	printk(KERN_INFO"No handler now ! current_handler is [%p] \n",current_handler);
	if (is_journal_aborted(example_journal)) 
	{
		printk(KERN_ERR "Detected aborted journal\n"); 
		return ERR_PTR(-EROFS);
	}

	printk(KERN_INFO "journal_start\n"); 
	//new_handle=jbd2__journal_start(example_journal, 1, 1, GFP_NOFS, 1, 1); //jbd2
	new_handle = journal_start(example_journal,4); //jbd1
	if (IS_ERR(new_handle)) {
		ret = PTR_ERR(new_handle);
		return ret;
	}

	printk(KERN_INFO "__block_write_begin"); 
	ret = __block_write_begin(page, 0, PAGE_CACHE_SIZE,
			example_get_block);
	if (ret != 0) {
		journal_stop(new_handle); //jbd1
		//jbd2_journal_stop(new_handle); //jbd2
		return 0;
	}
	printk(KERN_INFO "[%s]Before walk_page_buffers do_journal_get_write_access page[%p]\n",
			__func__,page); 
	ret = walk_page_buffers(new_handle, page_buffers(page), 0,
			PAGE_CACHE_SIZE, NULL, do_journal_get_write_access);
	return ret;
}
static int example_journalled_write_end111(struct file *file, struct address_space *mapping,
		                        loff_t pos, unsigned len, unsigned copied,
					                        struct page *page, void *fsdata)
{
	int ret=0;
	int err=0;
	printk(KERN_INFO "[%s]Before walk_page_buffers write_end_fn page [%p]\n",
			__func__,page); 
	handle_t *new_handle = journal_current_handle();


	err = walk_page_buffers(new_handle, page_buffers(page), 0,
			PAGE_CACHE_SIZE, NULL, write_end_fn);
	if (ret == 0)
		ret = err;
	unlock_page(page);
	//err = jbd2_journal_stop(new_handle); //jbd2
	printk(KERN_INFO "journal_stop\n"); 
	err = journal_stop(new_handle); //jbd1
	if (!ret)
	{
		printk(KERN_INFO "journal_stop ERR [%d]\n",err); 
		ret = err;
	//	return ret;
	}


	ret = generic_write_end(file, mapping, pos, len, copied, page, fsdata);
	if (ret < len)
	{
		printk(KERN_ERR"generic_write_end ret[%d] wanted len [%d]\n",ret,len);
		//ext2_write_failed(mapping, pos + len);
	}
	return ret;
}
static int example_journalled_write_end(struct file *file, struct address_space *mapping,
		                        loff_t pos, unsigned len, unsigned copied,
					                        struct page *page, void *fsdata)
{
	handle_t *current_handle = journal_current_handle();
	struct inode *inode = mapping->host;
	int ret = 0, ret2;
	int partial = 0;
	unsigned from, to;
	printk(KERN_INFO "[%s]Before walk_page_buffers write_end_fn page [%p]\n",
			__func__,page); 

	from = pos & (PAGE_CACHE_SIZE - 1);
	to = from + len;

	if (copied < len) {
		if (!PageUptodate(page))
			copied = 0;
		page_zero_new_buffers(page, from + copied, to);
		to = from + copied;
	}

	ret = walk_page_buffers(current_handle, page_buffers(page), from,
				to, &partial, write_end_fn);
	if (!partial)
		SetPageUptodate(page);

	if (pos + copied > inode->i_size)
		i_size_write(inode, pos + copied);
	/*************/
	ret2 = journal_stop(current_handle); //jbd1
	if (!ret)
		ret = ret2;
	unlock_page(page);
	page_cache_release(page);

	return ret ? ret : copied;

	/*
	ret = generic_write_end(file, mapping, pos, len, copied, page, fsdata);
	if (ret < len)
	{
		printk(KERN_ERR"generic_write_end ret[%d] wanted len [%d]\n",ret,len);
		//ext2_write_failed(mapping, pos + len);
	}
	return ret;
	*/
}
static int example_journalled_set_page_dirty(struct page *page)
{
	printk(KERN_ERR"example_journalled_set_page_dirty [%s]\n",
			current->comm);
	SetPageChecked(page);
	return __set_page_dirty_nobuffers(page);
}

static int example_releasepage(struct page *page, gfp_t wait)
{
	WARN_ON(PageChecked(page));
	if (!page_has_buffers(page))
		return 0;
	return journal_try_to_free_buffers(example_journal, page, wait);
}

struct address_space_operations example_address_space_ops = {
        .readpage               = example_readpage,
        .writepage              = example_journalled_writepage,
	.write_begin            = example_journalled_write_begin,
	.write_end              = example_journalled_write_end,
	.set_page_dirty         = example_journalled_set_page_dirty,
	.releasepage= example_releasepage,/* must have it ! it will flush the tail of journal **/
};
