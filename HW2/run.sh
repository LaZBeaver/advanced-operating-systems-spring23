x#!/bin/bash


time=$(date +%M)


while [ $time -le 11 ]
do
  echo Waiting
  time=$(date +%M)
done

echo Executing...
sudo echo 3 > /proc/sys/vm/drop_caches
sudo perf stat -r 10 -e faults,major-faults,minor-faults,tlb:tlb_flush,dtlb_load_misses.walk_completed,itlb_misses.walk_completed,cache-misses,dTLB-load-misses,iTLB-load-misses ./Q2_async
sudo echo 3 > /proc/sys/vm/drop_caches
sudo perf record -e tlb:tlb_flush ./Q2_async
sudo perf script -i perf.data > Q2_async.txt

