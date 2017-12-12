make clean
make
insmod fat.ko
insmod vfat.ko
mkfs.vfat /dev/sbd 
#mkfs.vfat /dev/sdc1
mount /dev/sbd /mnt
#mount /dev/sdc1 /mnt

mkdir /mnt/normal
mkdir /mnt/event
mkdir /mnt/parking
mkdir /mnt/manual
mkdir /mnt/config
