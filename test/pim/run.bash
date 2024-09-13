#!/bin/bash

if [[ $# -ne 1 ]]; then
    echo "usage: run.bash [0|1]"
    exit 1
fi

if [ $1 == 1 ]; then
    export APPX=1
else
    unset APPX
fi

make clean
make -j -s


    
