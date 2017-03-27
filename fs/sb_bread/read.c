#include <linux/time.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/buffer_head.h> /** read from block device **/
#include "../file.h"

extern ssize_t example_read (struct file *fp, char __user *bp, size_t len, loff_t *pp)
{
	printk("enter example_read len[%d] pos[%d]\n",len,*pp);
	char buffer[4096]={0};
	struct buffer_head * bh; /** hey! it is first time we use the (b)uffer (h)ead **/
		
	/** copy the block device first page to this buffer**/
	bh=sb_bread( fp->f_mapping->host->i_sb , /* file->f_mapping->host is the inode **/
		       	0 /* the first page **/);
	memcpy(buffer, bh->b_data ,4096);

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
