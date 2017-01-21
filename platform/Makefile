#Makefile for our own module
#obj-m := plat_drv.o plat_dev.o
obj-m := my_dev.o my_drv.o my_dev2.o

KERNELDIR := /lib/modules/`uname -r`/build

all:
	make -C $(KERNELDIR) M=`pwd` modules
install:
	make -C $(KERNELDIR) M=`pwd` modules
	make -C $(KERNELDIR) M=`pwd` modules_install
	depmod -A
clean:
	make -C $(KERNELDIR) M=`pwd` clean

