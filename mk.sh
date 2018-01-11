make clean
make
insmod fat.ko
insmod vfat.ko
mkfs.vfat /dev/sdd1
#mkfs.vfat /dev/sdc1
mount /dev/sdd1 /mnt
#mount /dev/sdc1 /mnt

mkdir /mnt/normal
mkdir /mnt/event
mkdir /mnt/parking
mkdir /mnt/manual
mkdir /mnt/config
