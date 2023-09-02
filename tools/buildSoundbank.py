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

    if(filename.find('.mod') != -1) or (filename.find('.it') != -1):
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

# class Location:
#     def __init__(self, n):
#         self.name = n
#         self.location = 0

effect_list = {}
# print(effect_list)

with open(soundbank_header) as f:

    for line in f:
        line = line.split()[1:]#split into words and ditch #define

        if(line[0].find('MSL') != -1):
            continue

        if(line[0].find('SFX') != -1):
            effect_list[line[0]] = int(line[1])
            # effect_list.append((line[0],int(line[1])))
            # id = 0
            # try:
            #     id = effect_list.index(line[0])
            # except:
            #     continue

            # print(line[0],id)
            continue

        # print(line)

# print(effect_list)

with open("./include/audio_files.h") as f:
    with open("./data/effect_locations.bin","wb") as result:
        def write(num):
            result.write(struct.pack('<i',int(num)))

        write(81)
        write(len(effect_list))

        for line in f:
            if not (line.find("SFX") != -1 and line[0] == '\t'):
                continue
            line = line.replace("\t","")
            line = line.replace(",","")
            line = line.replace("\n","")

            # print(line, effect_list[line])
            write(effect_list[line])
        # effect_list.append(Location(line))
