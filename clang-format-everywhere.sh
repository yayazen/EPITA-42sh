#!/bin/bash
for i in $(find . | grep -E "\.(c|h)$"); do
	echo $i
	clang-format -i $i;
done
