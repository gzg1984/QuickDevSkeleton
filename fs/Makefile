#obj-m += example_fs_never_mount.o 
#obj-m += example_fs_nodev_no_fill_sb.o 

obj-m += example_fs_nodev_fill_sb.o
obj-m += example_fs_nodisk.o 
#obj-m := example_fs_nodisk.o example_fs_file_on_disk.o

#register file system ,will shown in /proc/filesystems , but "mount" will be kiiled  because there is no "mount function"
example_fs_never_mount-y		:= super_never_mount.o
example_fs_nodev_no_fill_sb-y		:= super_never_fill_sb.o
example_fs_nodev_fill_sb-y		:= super_nodev_fill_sb.o fill_super_with_simple_operation.o #if us fill_super_with_no_op.o , the mount will die while alloc_inode(super_block*)
example_fs_nodisk-y		:= super.o fill_super_with_simple_operation.o
#example_fs_file_on_disk-y		:= inode.o

KERNEL := /lib/modules/`uname -r`/build

all:
	make -C $(KERNEL) M=`pwd` modules

install:
	make -C $(KERNEL) M=`pwd` modules
	make -C $(KERNEL) M=`pwd` modules_install
	depmod -A

clean:
	make -C $(KERNEL) M=`pwd` clean

