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
struct bio * private_bio=NULL;
static void example_end_io(struct bio *bio, int err)
{
	struct page *page;
	const int uptodate = test_bit(BIO_UPTODATE, &bio->bi_flags);
        struct bio_vec *bvec = bio->bi_io_vec + bio->bi_vcnt - 1;
	printk("BI_IO_VEC [%p]\n",bio->bi_io_vec);
	printk("BI_VCNT [%d]\n",bio->bi_vcnt);
	page = bvec->bv_page;
	if(!uptodate)
	{
		return;
	}
	SetPageUptodate(page); /** the key opration of the end IO **/
	unlock_page(page); /**here!! , it will wake up the waiting process acording to PG_lock bit **/
	/** if we use our private page, we don't need lock**/
	/** when we use the VFS , we need to mannually unlock the page to wake upt the waiting process **/

	private_bio=NULL;
	bio_put(bio);
}
static int example_readpage(struct file *fp, struct page *private_page) /** almost same as the read.c in class 8, but no content copy code **/
{
	printk("enter example_readpage\n");
	if (private_bio == NULL)
	{
		private_bio = bio_alloc(GFP_KERNEL, 1/*nr_vecs*/);
		private_bio=private_bio;
		private_bio->bi_bdev = fp->f_mapping->host->i_sb->s_bdev;/* block device **/
		private_bio->bi_sector = 0;/*first_sector */
		bio_add_page(private_bio, private_page, 4096, 0/*offset**/); /** link the page and the bio **/
		private_bio->bi_end_io = example_end_io; /** who handle the end of this bio ?*/
		printk("try to submit bio %p\n",private_bio);
		submit_bio(READ, private_bio);/** will sleep in this function ?**/
	}
	return 0;
}
struct address_space_operations example_address_space_ops = {
        .readpage               = example_readpage,
};
