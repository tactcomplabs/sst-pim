#!/bin/bash

if [ $1 == 1 ]; then
    export REV=1
else
    unset REV
fi

make clean
make -j -s

    
