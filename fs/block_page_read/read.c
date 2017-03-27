#include <linux/time.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/bio.h> /** alloc bio **/
#include <linux/pagemap.h> /** end bio **/
#include "../file.h"


struct page* private_page;
struct bio * private_bio;
char buffer[4096];
unsigned long           update_state=0;
void show_content(void)
{
		char *kaddr=NULL;
		/* copy the data of page into our temprary buffer ,
		 * in fact, we don't need the buffer, do we?**/
		kaddr = kmap_atomic(private_page, KM_USER0);/** map the page, get the address **/
		/*left = __copy_to_user_inatomic(user addr,**/
/*
		printk("[show_content]page [%p] map to kaddr [%p] first 3 data is [%2.2x][%2.2x][%2.2x]\n",
				private_page,kaddr,kaddr[0],kaddr[1],kaddr[2]);
*/
		memcpy(buffer,kaddr,4096);
		kunmap_atomic(kaddr, KM_USER0);
}
static void example_end_io(struct bio *bio, int err)
{
	const int uptodate = test_bit(BIO_UPTODATE, &bio->bi_flags);
//	printk("end bio %p  iuptodate [%s]\n",bio,uptodate?"YES":"NO");
	if(!uptodate)
	{
		return;
	}

	/*
	const int uptodate = test_bit(BIO_UPTODATE, &bio->bi_flags);
	struct bio_vec *bvec = bio->bi_io_vec + bio->bi_vcnt - 1;

	do {
		struct page *page = bvec->bv_page;

		if (--bvec >= bio->bi_io_vec)
			prefetchw(&bvec->bv_page->flags);
		if (bio_data_dir(bio) == READ) {
			if (uptodate) {
				SetPageUptodate(page);
			} else {
				ClearPageUptodate(page);
				SetPageError(page);
			}
			unlock_page(page);
		} else { / * bio_data_dir(bio) == WRITE * /
			if (!uptodate) {
				SetPageError(page);
				if (page->mapping)
					set_bit(AS_EIO, &page->mapping->flags);
			}
			end_page_writeback(page);
		}
	} while (bvec >= bio->bi_io_vec);
	*/
	show_content();
	bio_put(bio);
	private_bio=NULL;
        wake_up_bit(&update_state, __I_NEW);
}


extern ssize_t example_read (struct file *fp, char __user *bp, size_t len, loff_t *pp)
{
	struct buffer_head * bh; /** hey! it is first time we use the (b)uffer (h)ead **/
		
	/** copy the block device first page to this buffer**/
	/* submit bio to copy data to my own page**/
/*****************************/
	{
		char *kaddr=NULL;

		private_page=alloc_page(GFP_KERNEL);

		if (private_bio == NULL)
		{
			struct bio* bio;
			private_bio = bio_alloc(GFP_KERNEL, 1/*nr_vecs*/);
			bio=private_bio;
			bio->bi_bdev = fp->f_mapping->host->i_sb->s_bdev;/* block device **/
			bio->bi_sector = 0;/*first_sector */
			bio_add_page(bio, private_page, 4096, 0/*offset**/); /** link the page and the bio **/
			bio->bi_end_io = example_end_io; /** who handle the end of this bio ?*/
			printk("try to submit bio %p\n",bio);
			submit_bio(READ, bio);/** will sleep in this function ?**/
			/* why does this bio do nothing ?**/

			{
				wait_queue_head_t *wq;
				DEFINE_WAIT_BIT(wait, &update_state, __I_NEW);
				wq = bit_waitqueue(&update_state, __I_NEW);
				prepare_to_wait(wq, &wait.wait, TASK_UNINTERRUPTIBLE); /* sleep here, till the end bio wake up us **/
				schedule();
				finish_wait(wq, &wait.wait);/* now we have been wake-up, and then we can do the copy **/
			}

		}
	}
	/*****************************/



	size_t real_len=0;
	if(*pp > 3)
	{
		return 0;
	}
	real_len=3-(*pp);
	len=(len>real_len?real_len:len);

	switch(*pp)
	{
		case 0:
			copy_to_user(bp,buffer,len);
			break;
		case 1:
			copy_to_user(bp,buffer,len);
			break;
		case 2:
			copy_to_user(bp,buffer,len);
			break;
	}
	*pp+=len;
	return len;
}
