make clean
make
insmod fat.ko
insmod vfat.ko
mkfs.vfat /dev/sbd 
mount /dev/sbd /mnt

#mkdir /mnt/normal
#mkdir /mnt/normal_event
#mkdir /mnt/parking
#mkdir /mnt/parking_event
#mkdir /mnt/handwork

