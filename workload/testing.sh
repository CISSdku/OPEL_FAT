./initialize.sh 
./rm.sh 
sleep 10



./initialize.sh
time ./workload 7 ./target_dir_list/real_target_dir_list.txt on 1000 >> message.txt
insmod ../../view_fatent/view_fatent.ko
./rm.sh
sleep 10
rmmod view_fatent

./initialize.sh
time ./workload 7 ./target_dir_list/real_target_dir_list.txt on 2000 >> message.txt
insmod ../../view_fatent/view_fatent.ko
./rm.sh
sleep 10
rmmod view_fatent

./initialize.sh
time ./workload 7 ./target_dir_list/real_target_dir_list.txt on 3000 >> message.txt
insmod ../../view_fatent/view_fatent.ko
./rm.sh
sleep 10
rmmod view_fatent

./initialize.sh
time ./workload 7 ./target_dir_list/real_target_dir_list.txt on 4000 >> message.txt
insmod ../../view_fatent/view_fatent.ko
./rm.sh
sleep 10
rmmod view_fatent

./initialize.sh
time ./workload 7 ./target_dir_list/real_target_dir_list.txt on 5000 >> message.txt
insmod ../../view_fatent/view_fatent.ko
./rm.sh
sleep 10
rmmod view_fatent

./initialize.sh
time ./workload 7 ./target_dir_list/real_target_dir_list.txt on 6000 >> message.txt
insmod ../../view_fatent/view_fatent.ko
./rm.sh
sleep 10
rmmod view_fatent

./initialize.sh
time ./workload 7 ./target_dir_list/real_target_dir_list.txt on 7000 >> message.txt
insmod ../../view_fatent/view_fatent.ko
./rm.sh
sleep 10
rmmod view_fatent

./initialize.sh
time ./workload 7 ./target_dir_list/real_target_dir_list.txt on 8000 >> message.txt
insmod ../../view_fatent/view_fatent.ko
./rm.sh
sleep 10
rmmod view_fatent

./initialize.sh
time ./workload 7 ./target_dir_list/real_target_dir_list.txt on 9000 >> message.txt
insmod ../../view_fatent/view_fatent.ko
./rm.sh
sleep 10
rmmod view_fatent

./initialize.sh
time ./workload 7 ./target_dir_list/real_target_dir_list.txt on 10000 >> message.txt
insmod ../../view_fatent/view_fatent.ko
./rm.sh
sleep 10
rmmod view_fatent





