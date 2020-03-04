# obj-m specifie we're a kernel module.
obj-m += kbd.o 

# Set the path to the Kernel build utils.
KBUILD=/lib/modules/$(shell uname -r)/build/

all:	default user

default:
	$(MAKE) -C $(KBUILD) M=$(PWD) modules

user: user_test.c
	gcc -o user_test user_test.c

clean:
	$(MAKE) -C $(KBUILD) M=$(PWD) clean
	rm -v user_test 

menuconfig:
	$(MAKE) -C $(KBUILD) M=$(PWD) menuconfig

.PHONY: default, user, clean
