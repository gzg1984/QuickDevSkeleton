#obj-m += example_fs_min_registerable.o   
# This can never been mountable, because we don't fill the super block
#register file system ,will shown in /proc/filesystems , but "mount" will be kiiled  because there is no "mount function"
example_fs_min_registerable-y		:= super_never_mount.o

#obj-m += example_fs_nodev_no_fill_sb.o 
# This can be register, but mount will fail too, because we don't fille the super block too
example_fs_nodev_no_fill_sb-y		:= super_never_fill_sb.o

#if we fill_super_with_no_op.o , the mount will die while alloc_inode(super_block*)
# but this will never write/create file. We don't handle it.
#obj-m += example_fs_min_mountable.o
example_fs_min_mountable-y		:= super_nodev_fill_sb.o fill_super_with_simple_operation.o 

#Now we will do some real work, first of all, the super.c will work from now on.
# And in file_system.c we will use the real block dev
#obj-m += example_fs_min_device.o
example_fs_min_device-y		:= file_system.o super.o inode_nothing.o

#Create a file in readdir from root inode
obj-m += example_fs_min_root.o
example_fs_min_root-y		:= file_system.o super.o inode.o



####obj-m += example_fs_nodisk.o 
####obj-m := example_fs_nodisk.o example_fs_file_on_disk.o

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

