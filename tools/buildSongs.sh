#!/bin/bash

pushd audio

# Detect the operating system
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    OS="Linux"
    COUNTCHAN_CMD="countchan"
elif [[ "$OSTYPE" == "darwin"* ]]; then
    OS="macOS"
    COUNTCHAN_CMD="countchan.mac"
else
    echo "Unsupported operating system: $OSTYPE"
    exit 1
fi

extensions=(".mod" ".it" ".xm" ".s3m")
# Iterate over files and count their channels
for ext in "${extensions[@]}"; do
    # Check if any files with the current extension exist
    if ls *"$ext" &>/dev/null; then
        for file in *"$ext"; do
            ../tools/$COUNTCHAN_CMD "$file"
        done
    fi
done

popd

python3 ./tools/buildSoundbank.py append

./tools/gbfs ./tools/audio.gbfs ./tools/soundbank.bin ./tools/effect_locations.bin
./tools/padbin 256 ./tools/Apotris-base.gba
cat ./tools/Apotris-base.gba ./tools/audio.gbfs > Apotris.gba

file="Apotris.gba"
size=$(stat --format=%s "$file")

power=1
while (( $power < $size )); do
  power=$((power * 2))
done

if (( $power > 33554432 )); then
  echo "File size too large ($size, max is 32MB). Try adding fewer songs."
  exit 1
fi

echo $power
./tools/padbin $power $file
