#include <linux/module.h>
#include <linux/sched.h>
#include <linux/buffer_head.h>
#include <linux/proc_fs.h>
#include <linux/cpu.h>
#include <linux/rcupdate.h>
#include <linux/radix-tree.h>




static int list_buffer_of_inode=8;
module_param(list_buffer_of_inode, int, 0664);
MODULE_PARM_DESC(list_buffer_of_inode, "List The buffer of this Inode");


#define BUF_LEN 100
static char species[BUF_LEN]="ext3";
module_param_string(specifies/** name used by user**/, species/*name in this module**/, BUF_LEN, 0664);
MODULE_PARM_DESC(specifies, "Specify the name of file system");

int radix_node_has_buffer(void * node)
{

	struct page* page=node;
	int buffer_count=0;
	if(!node)
	{
		return 0;
	}
	/** check every page here **/
	if (page_has_buffers(page)) {
		struct buffer_head *head = page_buffers(page);
		struct buffer_head *bh = head;
		//printk("page[%p] links with buffer heads %p\n",page,bh);
		/* If they're all mapped and dirty, do it 
		 */
		do {
			buffer_count++;
			//printk("        current buffer_head[%p]\n",bh);
		} while ((bh = bh->b_this_page) != head);
		if(buffer_count != 1)
		{
			printk("page[%p] links with %d buffer heads\n",page,buffer_count);
		}
	}
	return buffer_count;
}

#ifdef __KERNEL__
#define RADIX_TREE_MAP_SHIFT    (CONFIG_BASE_SMALL ? 4 : 6)
#else
#define RADIX_TREE_MAP_SHIFT    3       /* For more stressful testing */
#endif

#define RADIX_TREE_MAP_SIZE     (1UL << RADIX_TREE_MAP_SHIFT)
#define RADIX_TREE_MAP_MASK     (RADIX_TREE_MAP_SIZE-1)

#define RADIX_TREE_TAG_LONGS    \
	((RADIX_TREE_MAP_SIZE + BITS_PER_LONG - 1) / BITS_PER_LONG)



struct radix_tree_node {
	unsigned int    height;         /* Height from the bottom */
	unsigned int    count;
	struct rcu_head rcu_head;
	void __rcu      *slots[RADIX_TREE_MAP_SIZE];
	unsigned long   tags[RADIX_TREE_MAX_TAGS][RADIX_TREE_TAG_LONGS];
};

#define RADIX_TREE_INDEX_BITS  (8 /* CHAR_BIT */ * sizeof(unsigned long))
#define RADIX_TREE_MAX_PATH (DIV_ROUND_UP(RADIX_TREE_INDEX_BITS, \
			RADIX_TREE_MAP_SHIFT))
static unsigned long height_to_maxindex[RADIX_TREE_MAX_PATH + 1] ;
static inline unsigned long radix_tree_maxindex(unsigned int height)
{
	return height_to_maxindex[height];
}

static __init unsigned long __maxindex(unsigned int height)
{
	unsigned int width = height * RADIX_TREE_MAP_SHIFT;
	int shift = RADIX_TREE_INDEX_BITS - width;

	if (shift < 0)
		return ~0UL;
	if (shift >= BITS_PER_LONG)
		return 0UL;
	return ~0UL >> shift;
}

static void radix_tree_init_maxindex(void)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(height_to_maxindex); i++)
		height_to_maxindex[i] = __maxindex(i);
}
static inline void *indirect_to_ptr(void *ptr)
{
	return (void *)((unsigned long)ptr & ~RADIX_TREE_INDIRECT_PTR);
}

//struct page *find_get_page(struct address_space *mapping, pgoff_t offset)
////{
//	void **pagep;
//	struct page *page;
//
//	rcu_read_lock();
//repeat:
//	page = NULL;
//	pagep = radix_tree_lookup_slot(&mapping->page_tree, offset);
//	if (pagep) {
//		page = radix_tree_deref_slot(pagep);
//		if (unlikely(!page))
//			goto out;
//		if (radix_tree_exception(page)) {
//			if (radix_tree_exceptional_entry(page))
//				goto out;
//			/* radix_tree_deref_retry(page) */
//			goto repeat;
//		}
//		if (!page_cache_get_speculative(page))
//			goto repeat;
//
//		/*
//		 *                  * Has the page moved?
//		 *                                   * This is part of the lockless pagecache protocol. See
//		 *                                                    * include/linux/pagemap.h for details.
//		 *                                                                     */
//		if (unlikely(page != *pagep)) {
//			page_cache_release(page);
//			goto repeat;
//		}
//	}
//out:
//	rcu_read_unlock();
//
//	return page;
//}
int radix_tree_walk_through(char* page,struct radix_tree_root * const root)
{
	int ret=0;
	int temp=0;
	int always_true=0;
	int page_count=0;
	int buffer_head_count=0;
	unsigned long index;
	unsigned int height, shift;
	unsigned int max_index;
	struct radix_tree_node *node, **slot;

	//printk("radix_tree_walk_through on root[%p]\n",root);
	node = rcu_dereference_raw(root->rnode);
	if (node == NULL)
	{
		return 0;/*no buffer **/
	}

	if (!radix_tree_is_indirect_ptr(node)) {
	//	printk("radix_tree_walk_through on direct ptr node[%p]\n",node);
		return radix_node_has_buffer(node);
	}
	node = indirect_to_ptr(node);

	index=0;
	radix_tree_init_maxindex();
	height = node->height;/* total height**/
	shift = (height-1) * RADIX_TREE_MAP_SHIFT;/** shift in every level**/
	max_index=radix_tree_maxindex(height);

	/*
	        index = *ppos >> PAGE_CACHE_SHIFT; //it came from the position and the page size
		*/


	//printk("radix_tree_walk_through before loop index,max is %d\n",max_index);
	for(index=0;index<=max_index ; index++)
	{
		node = rcu_dereference_raw(root->rnode); /** from root to bottom**/
		node = indirect_to_ptr(node);
		height = node->height;/* total height**/
		shift = (height-1) * RADIX_TREE_MAP_SHIFT;/** shift in every level**/

		//printk("radix_tree_walk_through radix root [%p] index [%d]\n",
		//		root,index);
		do {
			slot = (struct radix_tree_node **)
				(node->slots + ((index>>shift) & RADIX_TREE_MAP_MASK));
			node = rcu_dereference_raw(*slot);
			if (node == NULL)
			{
				break;/* no slot,no page**/
			}

			shift -= RADIX_TREE_MAP_SHIFT;/** go to next level**/
			height--;
		} while (height > 0);

		if(node)/** if find page according to the index **/
		{
			temp=radix_node_has_buffer(node);
			if(temp)
			{
				ret += sprintf (page+ret,"    Page Index [%ld] has buffer\n", 
						index);
				always_true=1;
				buffer_head_count+=temp;
			}
			page_count++;
		}
	}


	ret += sprintf (page+ret,"Tree[%p]has %d pages , and %d buffer head\n", 
				root,page_count,buffer_head_count);
	return ret;
}

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

		if(temp_inode->i_ino != list_buffer_of_inode)
		{
			continue;
		}

		total_page_count += temp_inode->i_mapping->nrpages;
			ret += sprintf (page+ret,"    inode %p page count = %ld\n",
					temp_inode,
					temp_inode->i_mapping->nrpages);
			ret += show_dentry(temp_inode,page+ret);
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
		/* list all pages with buffer **/
		{
			struct radix_tree_root *root;
			root=&(temp_inode->i_mapping->page_tree);
			ret+= radix_tree_walk_through(page+ret,root);
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

int list_buffer(char *page, char **start, off_t off, int count, int *eof, void *data)
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

	ret += sprintf(page+ret, "List Inode [%d] on super block [%s]\n",
			list_buffer_of_inode,species);


	list_for_each_entry(old, &fst->fs_supers, s_instances) {
		if (!test(old, NULL))
			continue;
		ret += set(old,page+ret);
	}

	return ret;
}


int __init my_init(void)
{
	create_proc_read_entry("list_buffer", 0, NULL, list_buffer, NULL);
	return 0;
}


void __exit my_exit(void)
{
	remove_proc_entry("list_buffer", NULL);
	return;
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ZHANG");
MODULE_VERSION("0.7");


