#!/bin/bash

for i in tests/*.lt; do
	echo "Running testcase: $i"
	util/lucietest.sh "$i"
	if [ $? -ne 0 ]; then
		exit 1
	fi
done
