#!/usr/bin/env python3

import os
directory = 'audio'

header = open("./include/audio_files.h","w")
source = open("./source/audio_files.cpp","w")

count = 0
music_count = 0

effectlist = []
musiclist = []

sep = '.'

for filename in os.listdir(directory):
    # f = os.path.join(directory,filename)

    n = (filename.split(sep,1)[0]).upper()

    if(filename.find('.wav') != -1):
        path = "assets/" + filename
        name = "SFX_" + n;
        name = name.replace("-","_");
        effectlist.append((name,path))

        count+=1

    if(filename.find('.mod') != -1) or (filename.find('.it') != -1):
        path = "assets/" + filename
        name = "MOD_" + n
        name = name.replace("-","_");

        musiclist.append((name,path))

        music_count+=1

header.write("#pragma once\n\n");

header.write(f"#define SFX_COUNT {count}\n")
header.write(f"#define MUSIC_COUNT {music_count}\n\n")

header.write(f"extern const char * SoundEffectPaths[];\n")
header.write(f"extern const char * SongPaths[];\n\n")


header.write("enum SoundEffects{\n")

for item in effectlist:
    name = item[0]
    header.write(f"\t{name},\n")

    # if(os.path.isfile(f)):
    #     print(f)

header.write("};\n\n")

header.write("enum Songs{\n")

for item in musiclist:
    name = item[0]
    header.write(f"\t{name},\n")

    # if(os.path.isfile(f)):
    #     print(f)

header.write("};\n\n")

source.write("const char * SoundEffectPaths[] = {\n")

for item in effectlist:
    path = item[1]
    source.write(f"\t\"{path}\",\n")

    # if(os.path.isfile(f)):
    #     print(f)

source.write("};\n\n")

source.write("const char * SongPaths[] = {\n")

for item in musiclist:
    path = item[1]
    source.write(f"\t\"{path}\",\n")

    # if(os.path.isfile(f)):
    #     print(f)

source.write("};\n")

header.close()
source.close()
