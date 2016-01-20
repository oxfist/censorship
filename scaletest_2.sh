#!/bin/bash

i=10000000

# Deletes the file in case it already exists.
rm -f censorship_2.dat

while (( $i <= 700000000 )); do
    ./censorship 1400000000 ${i} >> censorship_2.dat
    if (( $i < 100000000 ))
    then
        i=$((i + 10000000))
    else
        i=$((i + 100000000))
    fi
done
echo "Finished!"
