#!/bin/bash
for (( a = 1; a < 150; a++ ))
do
    echo $RANDOM
done > numbers.txt

let average=0
let cnt=0

while IFS= read -r number;
do
average=$((average + $number))
cnt=$((cnt + 1))
done < numbers.txt
echo $((average / cnt))
