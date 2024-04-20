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
