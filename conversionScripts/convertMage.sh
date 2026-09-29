#!/usr/bin/env bash
set -euo pipefail

tmp=$(mktemp --suffix=.png)
trap 'rm -f "$tmp"' EXIT
convert ./sprites/mage/mageWalk{1..8}.png -append "$tmp"
./tools/superfamiconv -v -i "$tmp" -p ./data/mage_pal.bin \
    -t ./data/mage_tiles.bin -M gba -D -S
