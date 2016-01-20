#!/bin/bash

i=1

# Deletes the file in case it already exists.
rm -f censorship_1_par.dat

while (( $i <= 10000 )); do
    ./censorship_par 7400000000 ${i} 10 >> censorship_1_par.dat
    i=$((i * 10))
done
echo "Finished!"
