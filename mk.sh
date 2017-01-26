make clean
make
insmod fat_opel.ko
insmod vfat_opel.ko
mkfs.vfat /dev/sbd 
mount /dev/sbd /mnt

mkdir /mnt/normal
mkdir /mnt/normal_event
mkdir /mnt/parking
mkdir /mnt/parking_event
mkdir /mnt/handwork

