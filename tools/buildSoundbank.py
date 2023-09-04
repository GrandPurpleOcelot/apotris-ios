#!/usr/bin/env python3

import os
import sys
import struct
import time
directory = 'audio'

count = 0
music_count = 0

files = ""

for filename in os.listdir(directory):
    if(filename.find('.wav') != -1):
        files += f"{directory}/{filename} "

        count+=1

    if(filename.find('.mod') != -1) or (filename.find('.it') != -1) or (filename.find('.xm') != -1):
        files += f"{directory}/{filename} "

        music_count+=1

# run mmutil
# print(files)

soundbank_header = "tools/soundbank.h"

os.system(f"mmutil {files} -odata/soundbank.bin -h{soundbank_header} > /dev/null")

print("Waiting for header...")

tries = 0

while tries < 100:
    if os.path.exists(soundbank_header):
        break
    time.sleep(0.1)

if(tries == 100):
    print("Timed out waiting for header.")
    sys.exit()

effect_list = {}
song_list = {}

with open(soundbank_header) as f:

    for line in f:
        line = line.split()[1:]#split into words and ditch #define

        if(line[0].find('MSL') != -1):
            continue

        if(line[0].find('SFX') != -1):
            effect_list[line[0]] = int(line[1])

        elif(line[0].find('MOD') != -1):
            song_list[line[0]] = int(line[1])

sfx = []
songs = []

song_count = 0

preset_songs = []

with open("./include/audio_files.h") as f:

    for line in f:
        if (line.find("SFX") != -1 and line[0] == '\t'):
            line = line.replace("\t","")
            line = line.replace(",","")
            line = line.replace("\n","")

            try:
                sfx.append(effect_list[line])
            except:
                sfx.append(-1)

        if (line.find("MOD") != -1 and line[0] == '\t'):
            line = line.replace("\t","")
            line = line.replace(",","")
            line = line.replace("\n","")

            preset_songs.append(line)


            # print(line)

for song in preset_songs:
    try:
        songs.append(song_list[song])
        song_count+=1
    except:
        songs.append(-1)

print(sfx)
print(songs)
print(song_list)
print(preset_songs)

with open("./data/effect_locations.bin","wb") as result:
    def write(num):
        result.write(struct.pack('<i',int(num)))

    write(81)
    write(len(sfx))

    for s in sfx:
        write(s)

    write(len(songs))
    write(len(song_list) - song_count)

    for s in songs:
        write(s)

    for key in song_list:
        if key in preset_songs:
            continue
        write(song_list[key])
