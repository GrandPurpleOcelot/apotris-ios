#!/usr/bin/env bash

if [ "$1" == "gba" ]
then
    cp ./make/gba_make ./Makefile

elif [ "$1" == "linux" ]
then
    cp ./make/linux_make ./Makefile
elif [ "$1" == "w32" ]
then
    cp ./make/w32_make ./Makefile
else
    echo "Unknown platform.\n"
fi
make clean
