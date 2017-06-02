ifneq ($(KERNELRELEASE),)
obj-m += usb-skeleton.o
else
KDIR:= /lib/modules/`uname -r`/build
BDIR:= $(shell pwd)
default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
.PHONY: clean
clean:
	make -C $(KDIR) M=$(BDIR) clean

install:default
	rmmod usb-skeleton
	insmod usb-skeleton.ko
endif
# Object files in subdirectories
