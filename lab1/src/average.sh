#!/bin/bash
for (( a = 1; a < 150; a++ ))
do
    /dev/urandom>number
    echo $number
done > numbers.txt

average=0
cnt=0
while read number
do
average=average+number
cnt++
done
echo average/cnt
