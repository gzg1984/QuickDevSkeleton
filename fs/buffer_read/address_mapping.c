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

	buffer=kmap(private_page);

        bh=sb_bread( fp->f_mapping->host->i_sb , 0/*first block*/);
        memcpy(buffer, bh->b_data ,4096);
	kunmap(private_page);
        SetPageUptodate(private_page); 
        unlock_page(private_page); 
	return 0;
}
struct address_space_operations example_address_space_ops = {
        .readpage               = example_readpage,
};
