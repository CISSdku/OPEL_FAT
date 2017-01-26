KERNELDIR=/lib/modules/$(shell uname -r)/build

NAME=vfat_opel
obj-m += fat_opel.o
obj-m += $(NAME).o
$(NAME)-objs := vfat.o

fat_opel-y := cache.o dir.o fatent.o file.o inode.o misc.o nfs.o config.o 
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

