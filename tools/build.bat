python tools\buildSoundbank.py append

tools/gbfs tools/audio.gbfs tools/soundbank.bin tools/effect_locations.bin
tools/padbin 256 Apotris.gba
cat Apotris.gba tools/audio.gbfs > Apotris_Audio.gba
tools/padbin 0x400000 Apotris_Audio.gba
