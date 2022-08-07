obj-m := func.o

default:
	make -C /lib/modules/$(shell uname -r)/build  M=$(shell pwd) modules
	
test:default
	insmod func.ko
	grep test1 /proc/kallsyms
	rmmod func


