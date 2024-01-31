obj-m += kfetch_mod_310512051.o
KDIR = /lib/modules/$(shell uname -r)/build
all:
	make -C $(KDIR) M=$(shell pwd) modules
load:
	sudo insmod kfetch_mod_310512051.ko
unload:
	sudo rmmod kfetch_mod_310512051.ko
clean:
	make -C $(KDIR) M=$(shell pwd) clean
