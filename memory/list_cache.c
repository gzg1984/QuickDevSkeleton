#include <linux/module.h>
#include <linux/sched.h>
#include <linux/proc_fs.h>
#include <linux/cpu.h>
#include <linux/rcupdate.h>
#include <linux/radix-tree.h>




static int show_threshold=1000;
module_param(show_threshold, int, 0664);
MODULE_PARM_DESC(show_threshold, "Size more than this number will be show the name");

#define BUF_LEN 100
static char species[BUF_LEN]="ext3";
module_param_string(specifies/** name used by user**/, species/*name in this module**/, BUF_LEN, 0664);
MODULE_PARM_DESC(specifies, "Specify the name of file system");

int inode_has_buffers(struct inode *inode)
{
	return !list_empty(&inode->i_data.private_list);
}

static int test(struct super_block *fst,void *_un)
{
	return 1;
}
static int print_dentry(struct inode *inode,char* s)
{
	int ret = 0;
	struct list_head* first_dentry_list;
	struct list_head* next_dentry_list;
	struct list_head* temp;
	struct dentry* temp_dentry;

	first_dentry_list= &inode->i_dentry;
	next_dentry_list=first_dentry_list->next;

	while(next_dentry_list != first_dentry_list)
	{
		temp=next_dentry_list;
		next_dentry_list=temp->next;

		temp_dentry=list_entry(temp, struct dentry, d_alias);
		printk ("[%s]dentry name %s \n",
				s,	temp_dentry->d_iname);
	}

	return ret;
}
static int show_dentry(struct inode *inode,char *page)
{
	int ret = 0;
	struct list_head* first_dentry_list;
	struct list_head* next_dentry_list;
	struct list_head* temp;
	struct dentry* temp_dentry;

	first_dentry_list= &inode->i_dentry;
	next_dentry_list=first_dentry_list->next;

	while(next_dentry_list != first_dentry_list)
	{
		temp=next_dentry_list;
		next_dentry_list=temp->next;

		temp_dentry=list_entry(temp, struct dentry, d_alias);
		ret += sprintf (page+ret,"        dentry name %s \n",
				temp_dentry->d_iname);
	}

	return ret;
}
static int set(struct super_block *fst,char *page)
{
	int ret = 0;
	unsigned long total_page_count=0;
	unsigned long total_inode_count=0;
	unsigned long buffer_inode_count=0;
	struct list_head* first_inode_list;
	struct list_head* next_inode_list;
	struct list_head* temp;
	struct inode* temp_inode;
	ret += sprintf(page, "fst sid = %s\n", fst->s_id);

	first_inode_list= &fst->s_inodes;
	next_inode_list=fst->s_inodes.next;

	while(next_inode_list != first_inode_list)
	{
		temp=next_inode_list;
		next_inode_list=temp->next;

		temp_inode=list_entry(temp, struct inode, i_sb_list);
		if (!temp_inode)
		{
			printk("meet null inode\n");
			break;
		}
		total_page_count += temp_inode->i_mapping->nrpages;
		if(temp_inode->i_mapping->nrpages > show_threshold)
		{
			ret += sprintf (page+ret,"    inode %p page count = %ld\n",
					temp_inode,
					temp_inode->i_mapping->nrpages);
			ret += show_dentry(temp_inode,page+ret);
		}
		if(temp_inode->i_mapping != &(temp_inode->i_data))
		{
			print_dentry(temp_inode,"[Inode i_mapping NOT point to i_data]");
		}
		/* this function is in linux/buffer_head.h **/
		if(inode_has_buffers(temp_inode))
		{
			print_dentry(temp_inode,"[Inode has buffers]");
			buffer_inode_count++;
		}
		total_inode_count++;
	}
	ret += sprintf (page+ret,"total page count %ld size = %ld K\n",
			total_page_count,
			total_page_count * 4 );
	ret += sprintf (page+ret,"total inode count %ld  inode address space private buffer = %ld \n",
			total_inode_count,
			buffer_inode_count );
	return ret;
}

int list_cache(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int ret = 0;
	struct file_system_type *fst;
	struct super_block *old;

	fst = get_fs_type(species);
	if (!fst)
	{
		ret += sprintf(page+ret, "found no super block of [%s]\n",species);
		return ret;
	}

	ret += sprintf(page+ret, "list super block of [%s]\n",species);

	list_for_each_entry(old, &fst->fs_supers, s_instances) {
		if (!test(old, NULL))
			continue;
		ret += set(old,page+ret);
	}

	return ret;
}


int __init my_init(void)
{
	create_proc_read_entry("list_cache", 0, NULL, list_cache, NULL);
	return 0;
}


void __exit my_exit(void)
{
	remove_proc_entry("list_cache", NULL);
	return;
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPLv2");
MODULE_AUTHOR("gzg1984@gmail.com");
MODULE_VERSION("0.1");


