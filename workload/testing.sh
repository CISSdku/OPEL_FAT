./initialize.sh 
./rm.sh 
sleep 10



./initialize.sh
time ./workload 7 ./target_dir_list/real_target_dir_list.txt on 100 >> message.txt
./rm.sh
sleep 10

./initialize.sh
time ./workload 7 ./target_dir_list/real_target_dir_list.txt on 200 >> message.txt
./rm.sh
sleep 10

./initialize.sh
time ./workload 7 ./target_dir_list/real_target_dir_list.txt on 300 >> message.txt
./rm.sh
sleep 10

./initialize.sh
time ./workload 7 ./target_dir_list/real_target_dir_list.txt on 400 >> message.txt
./rm.sh
sleep 10

./initialize.sh
time ./workload 7 ./target_dir_list/real_target_dir_list.txt on 500 >> message.txt
./rm.sh
sleep 10

./initialize.sh
time ./workload 7 ./target_dir_list/real_target_dir_list.txt on 600 >> message.txt
./rm.sh
sleep 10

./initialize.sh
time ./workload 7 ./target_dir_list/real_target_dir_list.txt on 700 >> message.txt
./rm.sh
sleep 10

./initialize.sh
time ./workload 7 ./target_dir_list/real_target_dir_list.txt on 800 >> message.txt
./rm.sh
sleep 10

./initialize.sh
time ./workload 7 ./target_dir_list/real_target_dir_list.txt on 900 >> message.txt
./rm.sh
sleep 10

./initialize.sh
time ./workload 7 ./target_dir_list/real_target_dir_list.txt on 1000 >> message.txt
./rm.sh
sleep 10





