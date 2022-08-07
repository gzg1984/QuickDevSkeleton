obj-m := func.o

default:
	make -C /lib/modules/$(shell uname -r)/build  M=$(shell pwd) modules
	
test:default
	insmod func.ko
	-grep exported_example /proc/kallsyms
	-grep no_exported_example /proc/kallsyms
	rmmod func


