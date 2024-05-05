#!/usr/bin/env bash

pushd audio

extensions=(".mod" ".it" ".xm" ".s3m")
# Iterate over files and count their channels
for ext in "${extensions[@]}"; do
    # Check if any files with the current extension exist
    if ls *"$ext" &>/dev/null; then
        for file in *"$ext"; do
            ../tools/countchan.exe "$file"
        done
    fi
done

popd

python3 ./tools/buildSoundbank.py append

tools/gbfs.exe ./tools/audio.gbfs ./tools/soundbank.bin ./tools/effect_locations.bin
tools/padbin.exe 256 ./tools/Apotris-base.gba
tools/cat.exe tools/Apotris-base.gba ./tools/audio.gbfs > Apotris.gba

file="Apotris.gba"
if [[ $(uname) == "Darwin" ]]; then
    size=$(stat -f "%z" "$file")
else
    size=$(stat --format=%s "$file")
fi

power=1
while (( $power < $size )); do
  power=$((power * 2))
done

if (( $power > 33554432 )); then
  echo "File size too large ($size, max is 32MB). Try adding fewer songs."
  exit 1
fi

echo $power
tools/padbin.exe $power $file

