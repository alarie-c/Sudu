#!/bin/bash

COMPILE=false

while getopts "c" opt; do
    case $opt in
        c) COMPILE=true ;;  
        *) ;;
    esac
done

if $COMPILE; then     
    echo "Recompiling..."

    mkdir -p build
    cd build || exit 1

    cmake .. && make
    echo "Compiling complete."
else
    cd build || exit 1
fi

./sudu
