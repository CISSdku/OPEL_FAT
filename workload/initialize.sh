sync; echo 3 > /proc/sys/vm/drop_caches
sync; echo 2 > /proc/sys/vm/drop_caches
sync; echo 1 > /proc/sys/vm/drop_caches
free&&sync
free
swapoff -a
free
swapon -a
free
free -m


