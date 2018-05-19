obj-m := pata_platform.o fake_pata.o my_drv.o

KERNELDIR := /lib/modules/`uname -r`/build

all: pata_platform.c fake_pata.c
	make -C $(KERNELDIR) M=`pwd` modules

test: all
	[ `lsmod|grep fake_pata|wc -l` -eq "0" ]||rmmod fake_pata
	insmod fake_pata.ko
	grep --color=auto pata_platform /proc/iomem

install:
	make -C $(KERNELDIR) M=`pwd` modules
	make -C $(KERNELDIR) M=`pwd` modules_install
	depmod -A

.PHONE: clean

clean:
	make -C $(KERNELDIR) M=`pwd` clean
	rm -rf *~

