#!/bin/bash

# Assign command line arguments to variables
PADBIN=$1
PAD_SIZE_1=$2
GBA_FILE=$3
AUDIO_FILE=$4
PAD_SIZE_2=$5
TARGET_OUT=$6
GBAFIX=$7

# Pad the GBA file to a certain size
$PADBIN $PAD_SIZE_1 $GBA_FILE

# Concatenate the GBA file with an audio file
cat $GBA_FILE $AUDIO_FILE > ${TARGET_OUT}

# Pad the resulting file to a certain size
$PADBIN $PAD_SIZE_2 ${TARGET_OUT}

# Fix as valid ROM
$GBAFIX ${TARGET_OUT} -tAPOTRIS -c2ATE -m00 -r04

