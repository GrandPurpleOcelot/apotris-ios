#!/usr/bin/env bash

# Set -e to exit the script if any command fails
set -e

# Set -u to treat unset variables as an error
set -u

# Set -o pipefail to make pipelines fail if any command fails
set -o pipefail

pushd audio || exit 1

extensions=(".mod" ".it" ".xm" ".s3m")

# Iterate over files and count their channels
for ext in "${extensions[@]}"; do
    # Check if any files with the current extension exist
    if ls *"$ext" &>/dev/null; then
        for file in *"$ext"; do
            if ! ../tools/countchan.exe "$file"; then
                echo "Error counting channels for $file"
                exit 1
            fi
        done
    fi
done

popd || exit 1

if ! python3 ./tools/buildSoundbank.py append; then
    echo "Error building soundbank"
    exit 1
fi

if ! tools/gbfs.exe ./tools/audio.gbfs ./tools/soundbank.bin ./tools/effect_locations.bin; then
    echo "Error creating audio.gbfs"
    exit 1
fi

if ! tools/padbin.exe 256 ./tools/Apotris-base.gba; then
    echo "Error padding Apotris-base.gba"
    exit 1
fi

if ! tools/cat.exe tools/Apotris-base.gba ./tools/audio.gbfs > Apotris.gba; then
    echo "Error creating Apotris.gba"
    exit 1
fi

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

if ! tools/padbin.exe $power $file; then
    echo "Error padding Apotris.gba"
    exit 1
fi
