#Makefile for our own module
#obj-m := plat_drv.o plat_dev.o
obj-m := pata_platform.o fake_pata.o

KERNELDIR := /lib/modules/`uname -r`/build

all:
	make -C $(KERNELDIR) M=`pwd` modules
install:
	make -C $(KERNELDIR) M=`pwd` modules
	make -C $(KERNELDIR) M=`pwd` modules_install
	depmod -A
clean:
	make -C $(KERNELDIR) M=`pwd` clean

