obj-m := variable.o

default:
	make -C /lib/modules/$(shell uname -r)/build  M=$(shell pwd) modules
	
test:default
	insmod variable.ko
	grep module_variable /proc/kallsyms
	rmmod variable


