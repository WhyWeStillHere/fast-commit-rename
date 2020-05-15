#!/bin/bash

if [ -z $1 ]; then 
	echo "Catalog not specified" 
	exit 1
fi

cd $1

for value in {1..20}
do
	echo "New file ${value}" > new_file_$value
	git add new_file_$value
	git commit -m "Add file ${value}"
done
