#!/bin/bash

i=1

# Deletes the file in case it already exists.
rm -f censorship_1.dat

while (( $i <= 100000000 )); do
    ./censorship 7400000000 ${i} >> censorship_1.dat
    i=$((i * 10))
done
echo "Finished!"
