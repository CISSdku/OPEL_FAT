make clean
make
insmod fat.ko
insmod vfat.ko
mkfs.vfat /dev/sbd 
mount /dev/sbd /mnt
#mount /dev/sdc5 /mnt


