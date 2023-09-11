#!/bin/bash

for (( i=1; i <= 150; i++ ))
do
random_number=$(od -An -N2 -i /dev/urandom | awk '{print $1 % 1001}')
echo $random_number

done
