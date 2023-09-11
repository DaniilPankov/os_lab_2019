#!/bin/bash
count=0
sum=0
for var in $@
do
count=$((count + 1))
sum=$((sum+var))
done
echo $count
echo "scale=2; $sum / $count" | bc -l111