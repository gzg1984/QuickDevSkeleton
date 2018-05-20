obj-m := fake_pata_driver.o fake_pata.o 

KERNELDIR := /lib/modules/`uname -r`/build

all:
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

