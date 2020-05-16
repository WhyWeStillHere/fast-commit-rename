#!/bin/bash

if [ -z $1 ]; then 
	echo "Catalog not specified" 
	exit 1
fi

mkdir $1
cd $1

git init

for value in {1..20}
do
	dd if=/dev/urandom of=new_file_$value bs=1048576 count=10
	git add new_file_$value
	dd if=/dev/urandom of=new_file2_$value bs=1048576 count=10
	git add new_file2_$value
	git commit -m "Add file ${value}"
done
