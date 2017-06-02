obj-m += example_fs.o


#obj-m += example_fs_min_registerable.o   
# This can never been mountable, because we don't fill the super block
#register file system ,will shown in /proc/filesystems , but "mount" will be kiiled  because there is no "mount function"
#
#example_fs-y		:= never_mount/super.o

##### class 2, do not fill super
#obj-m += example_fs_nodev_no_fill_sb.o 
# This can be register, but mount will fail too, because we don't fille the super block too
#example_fs-y		:= nodev_no_fill_sb/super_never_fill_sb.o

##### class 3, fill super ,but no operation
#if we fill_super_with_no_op.o , the mount will die while alloc_inode(super_block*)
# but this will never write/create file. We don't handle it.
#
#example_fs-y		:= min_mountable/super.o min_mountable/fill_super.o

####Class 4 the most mini example filesystem for block device
####Now we will do some real work, first of all, the super.c will work from now on.
####And in file_system.c we will use the real block dev
#
#example_fs-y		:= min_device/file_system.o min_device/super.o min_device/inode_nothing.o

####Class 5  the most mini example filesyste for root inode on block device 
####Create a file in readdir from root inode
#
#example_fs-y		:= min_root/file_system.o min_root/super.o min_root/inode.o

####Class 6  the most mini example filesyste for root inode on block device 
#### set the file operation, specially the read function.
#
#example_fs-y		:= min_read/file_system.o \
				min_read/super.o \
				min_read/inode.o \
				min_read/file.o 

####Class 7 read from block: from block page cache
#example_fs-y		:= sb_bread/file_system.o \
				sb_bread/super.o \
				sb_bread/inode.o \
				sb_bread/file.o \
				sb_bread/read.o

####Class 8 read from block: use bio to fill a temprary page
#example_fs-y		:= block_page_read/file_system.o \
				block_page_read/super.o \
				block_page_read/inode.o \
				block_page_read/file.o \
				block_page_read/read.o

####Class 9 read from block: use bio to fill radix page tree.
#example_fs-y		:= page_cache_read/file_system.o \
				page_cache_read/super.o \
				page_cache_read/inode.o \
				page_cache_read/file.o \
				page_cache_read/address_mapping.o

####Class 10 read from block via bread: actrualy , this is not faster than the bio. this will be even slower
#example_fs-y		:= bread/file_system.o \
				bread/super.o \
				bread/inode.o \
				bread/file.o \
				bread/address_mapping.o

####Class 11 read from block via submit_bh: this is very difficault , hard to learn , but it is the previous-step for get_block funtion
# NOTE: buffer_read folder is older, it has a lot of problem , but shows How I develop this module.
#
#example_fs-y		:= submit_bh_readpage/file_system.o \
				submit_bh_readpage/super.o \
				submit_bh_readpage/inode.o \
				submit_bh_readpage/file.o \
				submit_bh_readpage/address_mapping.o


####Class 12  the final and the hardest part: how to use get_block
# NOTE: Though we know how to create and submit bh now, we CAN NOT submit the bh directly.
# We must use get_block. But what data should I fill in get_block ?
#
example_fs-y		:= get_block/file_system.o \
				get_block/super.o \
				get_block/inode.o \
				get_block/file.o \
				get_block/address_mapping.o



####obj-m += example_fs_nodisk.o 
####obj-m := example_fs_nodisk.o example_fs_file_on_disk.o

#example_fs_nodisk-y		:= super.o fill_super_with_simple_operation.o
#example_fs_file_on_disk-y		:= inode.o

#KERNEL := /lib/modules/`uname -r`/build
KERNEL :=  /home/maxpain/poky/build/tmp/work/qemux86-poky-linux/linux-yocto/4.10.9+gitAUTOINC+ad2e885015_fe0fb8da3d-r0/linux-qemux86-standard-build/

all:
	make -C $(KERNEL) M=`pwd` modules

install:
	make -C $(KERNEL) M=`pwd` modules
	make -C $(KERNEL) M=`pwd` modules_install
	depmod -A

clean:
	make -C $(KERNEL) M=`pwd` clean

