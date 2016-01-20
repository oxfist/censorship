#!/bin/bash

i=1

# Deletes the file in case it already exists.
rm -f censorship_2_par.dat

while (( $i <= 100000 )); do
    ./censorship_par 7400000000 ${i} 50 >> censorship_2_par.dat
    if (( $i < 100000 ))
    then
        i=$((i * 10))
    else
        i=$((i + 10000))
    fi
done
echo "Finished!"
