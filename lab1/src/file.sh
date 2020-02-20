#!/bin/bash
echo "Curdir:"
pwd
echo "Date:"
date
patha="Goose"
echo $patha
read -p "Enter path of a direcory you want to know files from:" path
ls $path