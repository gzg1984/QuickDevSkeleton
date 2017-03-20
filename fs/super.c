
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/namei.h>
#include <linux/slab.h>
#include <linux/mount.h>
#include <linux/tty.h>
#include <linux/mutex.h>
#include <linux/idr.h>
#include <linux/parser.h>
#include <linux/fsnotify.h>
#include <linux/seq_file.h>

#include "fill_super_with_simple_operation.h"
#define pts_fs_info example_fs_info 
#define devpts_fill_super example_fill_super


#define DEVPTS_DEFAULT_MODE 0600
/*
 * ptmx is a new node in /dev/pts and will be unused in legacy (single-
 * instance) mode. To prevent surprises in user space, set permissions of
 * ptmx to 0. Use 'chmod' or remount with '-o ptmxmode' to set meaningful
 * permissions.
 */
#define DEVPTS_DEFAULT_PTMX_MODE 0000
#define PTMX_MINOR	2

/*
 * sysctl support for setting limits on the number of Unix98 ptys allocated.
 * Otherwise one can eat up all kernel memory by opening /dev/ptmx repeatedly.
 */
static int pty_limit = NR_UNIX98_PTY_DEFAULT;
//static int pty_limit_min;
//static int pty_limit_max = INT_MAX;
static int pty_count;


static DEFINE_MUTEX(allocated_ptys_lock);

static struct vfsmount *devpts_mnt;


enum {
	Opt_uid, Opt_gid, Opt_mode, Opt_ptmxmode, Opt_newinstance,  Opt_max,
	Opt_err
};

static const match_table_t tokens = {
	{Opt_uid, "uid=%u"},
	{Opt_gid, "gid=%u"},
	{Opt_mode, "mode=%o"},
#ifdef CONFIG_DEVPTS_MULTIPLE_INSTANCES
	{Opt_ptmxmode, "ptmxmode=%o"},
	{Opt_newinstance, "newinstance"},
	{Opt_max, "max=%d"},
#endif
	{Opt_err, NULL}
};
static inline struct pts_fs_info *DEVPTS_SB(struct super_block *sb)
{
	return sb->s_fs_info;
}

static inline struct super_block *pts_sb_from_inode(struct inode *inode)
{
#ifdef CONFIG_DEVPTS_MULTIPLE_INSTANCES
	if (inode->i_sb->s_magic == DEVPTS_SUPER_MAGIC)
		return inode->i_sb;
#endif
	if (!devpts_mnt)
		return NULL;
	return devpts_mnt->mnt_sb;
}
/*
 * This supports only the legacy single-instance semantics (no
 * multiple-instance semantics)

struct dentry *mount_single(struct file_system_type *fs_type,
		        int flags, void *data,
			        int (*fill_super)(struct super_block *, void *, int))
{
	struct super_block *s;
	int error;

	s = sget(fs_type, compare_single, set_anon_super, flags, NULL);
	if (IS_ERR(s))
		return ERR_CAST(s);
	if (!s->s_root) {
		error = fill_super(s, data, flags & MS_SILENT ? 1 : 0);
		if (error) {
			deactivate_locked_super(s);
			return ERR_PTR(error);
		}
		s->s_flags |= MS_ACTIVE;
	} else {
		do_remount_sb(s, flags, data, 0);
	}
	return dget(s->s_root);
}
static struct dentry *devpts_mount(struct file_system_type *fs_type, int flags,
		const char *dev_name, void *data)
{
	return mount_single(fs_type, flags, data, devpts_fill_super);
}
 */

/*
static void devpts_kill_sb(struct super_block *sb)
{
	struct pts_fs_info *fsi = DEVPTS_SB(sb);

	ida_destroy(&fsi->allocated_ptys);
	kfree(fsi);
	kill_litter_super(sb);
}
*/

static struct file_system_type example_fs_type = {
	.name		= "example_fs",
	//.mount		= NULL,//devpts_mount,
	.kill_sb	= NULL,//devpts_kill_sb,
};


/*
 * pty code needs to hold extra references in case of last /dev/tty close
 */
struct pts_fs_info *devpts_get_ref(struct inode *ptmx_inode, struct file *file)
{
	struct super_block *sb;
	struct pts_fs_info *fsi;

	sb = pts_sb_from_inode(ptmx_inode);
	if (!sb)
		return NULL;
	fsi = DEVPTS_SB(sb);
	if (!fsi)
		return NULL;

	atomic_inc(&sb->s_active);
	return fsi;
}

void devpts_put_ref(struct pts_fs_info *fsi)
{
	deactivate_super(fsi->sb);
}




static int __init init_example_fs(void)
{
	int err = register_filesystem(&example_fs_type);
	if (err) {
		unregister_filesystem(&example_fs_type);
	}
	return err;
}
static void __exit exit_example_fs(void)
{
	unregister_filesystem(&example_fs_type);
}
module_init(init_example_fs)
module_exit(exit_example_fs)
MODULE_LICENSE("GPL");
