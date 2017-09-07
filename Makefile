KERNELDIR=/lib/modules/$(shell uname -r)/build

ccflags-y := -w 
#ccflags-y := -Wall

NAME=vfat
obj-m += fat.o
obj-m += $(NAME).o
$(NAME)-objs := vfat.o

fat-y := cache.o dir.o fatent.o file.o inode.o misc.o nfs.o config.o
$(NAME)-objs := namei_vfat.o

KDIR:=/lib/modules/$(shell uname -r)/build 
PWD:=$(shell pwd)

default:$(TARGETS) 
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

clean :
	rm -rf *.ko
	rm -rf *.mod.*
	rm -rf .*.cmd
	rm -rf *.o
	rm -rf .tmp_versions

