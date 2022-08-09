obj-m := parameter.o

default:
	make -C /lib/modules/$(shell uname -r)/build  M=$(shell pwd) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build  M=$(shell pwd) clean
	
test:default
	insmod parameter.ko
	ls /sys/module/parameter/parameters/
	cat /sys/module/parameter/parameters/value
	rmmod parameter
	insmod parameter.ko value=10
	cat /sys/module/parameter/parameters/value
	rmmod parameter


