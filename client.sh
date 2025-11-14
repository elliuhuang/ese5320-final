#!/bin/bash

input_file="LittlePrince.txt"
inflag=0

for arg in $@; do
    if [ "$arg" == "-f" ]; then
	    inflag=1
    elif [ $inflag == 1 ]; then
        input_file=$arg
        inflag=0
    else
        echo "Warning: "${arg}" is not a recognized command option"
        echo ""
    fi
done

./client -f ${input_file} -i 10.10.7.1 -s 50000
