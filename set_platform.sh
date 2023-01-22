#!/usr/bin/env bash

if [ "$1" == "gba" ]
then
    cp ./make/gba_make ./Makefile

elif [ "$1" == "pc" ]
then
    cp ./make/pc_make ./Makefile
else
    echo "Unknown platform.\n"
fi
make clean
