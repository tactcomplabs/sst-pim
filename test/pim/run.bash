#!/bin/bash

if [ $1 == 1 ]; then
    export APPX=1
else
    unset APPX
fi

make clean
make -j -s


    
