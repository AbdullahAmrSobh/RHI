#!/bin/bash

# Specify the directories to run clang-format on
directories=(example include source)

# Loop over each directory
for directory in ${directories[@]}; do
    # Get a list of all the files in the directory with the .h and .cpp extensions
    files=$(find $directory -type f \( -name "*.h" -o -name "*.cpp" \))
    
    # Loop over each file and run clang-format on it
    for file in ${files[@]}; do
        echo "Formatting $file"
        clang-format -i $file
    done
done

echo "Done!"
