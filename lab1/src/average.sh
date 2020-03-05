#!/bin/bash
let cnt2=0
let a
for (( cnt2 = 1; cnt2 <= 150; cnt2++ ))
do
a="$a $RANDOM"
done
echo $a > numbers.txt
let average=0
let cnt=0
while (($# > 0))
do
let num=$1
average=$((average+num))
cnt=$((cnt+1))
shift
done
echo $((average / cnt))
