#!/usr/bin/env bash

# Set the field separator to newline to handle filenames with spaces
IFS=$'\n'

# Process .stl files
for f in $(find /home/pemb7000/eic/Designs/L4Stave -name '*.stl'); do
    # Unset IFS to avoid issues within the loop
    unset IFS

    # Check if the STL file is binary or ASCII
    if grep -q 'solid' "$f"; then
        echo "$f is already an ASCII STL."
        # Process the STL file with stl_gdml.py
        python stl_gdml.py out.gdml "$f"
    else
        echo "Converting $f from binary to ASCII STL."
        /cvmfs/sft.cern.ch/lcg/releases/Python/2.7.9.p1-df007/x86_64-slc6-gcc49-opt/bin/python2 BinaryToASCII.py "$f"  
    fi

    # Reset IFS for the next iteration
    IFS=$'\n'
done

# Unset IFS and disable filename expansion
unset IFS
set +f