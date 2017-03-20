#obj-m := example_fs_nodisk.o example_fs_file_on_disk.o
obj-m := example_fs_never_mount.o \
	example_fs_nodisk.o 

#register file system ,will shown in /proc/filesystems , but "mount" will be kiiled  because there is no "mount function"
example_fs_never_mount-y		:= super_never_mount.o fill_super_with_simple_operation.o
#example_fs_file_on_disk-y		:= inode.o
example_fs_nodisk-y		:= super.o fill_super_with_simple_operation.o

KERNEL := /lib/modules/`uname -r`/build

all:
	make -C $(KERNEL) M=`pwd` modules

install:
	make -C $(KERNEL) M=`pwd` modules
	make -C $(KERNEL) M=`pwd` modules_install
	depmod -A

clean:
	make -C $(KERNEL) M=`pwd` clean

