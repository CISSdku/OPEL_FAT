make clean
make
insmod fat.ko
insmod vfat.ko
mkfs.vfat /dev/sbd 
mount /dev/sbd /mnt

mkdir /mnt/normal
mkdir /mnt/event
mkdir /mnt/parking
mkdir /mnt/manual
mkdir /mnt/config

