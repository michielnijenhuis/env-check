#!/bin/bash

# Specify the directory path
directory="src/lib"

if [[ ! -d "release/lib" ]]; then
    mkdir -p "release/lib"
fi

# Use a for loop to iterate through each file in the directory
for file in "$directory"/*; do
    fileName=$(basename $file)
    cp -L "src/lib/$fileName" "release/lib/$fileName"
done

cp "src/main.c" "release/main.c"

make release