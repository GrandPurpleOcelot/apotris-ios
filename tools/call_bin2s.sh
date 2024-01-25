#!/bin/bash

bin2s_path="$1"
input_bin="$2"
header_h="$3"
output_s="$4"

# Call bin2s with the provided arguments and redirect output to the output_s file
if [[ $5 == 'apple' ]]; then
    "$bin2s_path" --apple-llvm -a 4 -H "$header_h" "$input_bin" > "$output_s"
    awk '{
        gsub(/uint8_t _/, "uint8_t ");
        gsub(/size_t _/, "size_t ");
        print
    }' "$header_h" > "$header_h"temp && mv "$header_h"temp "$header_h"
elif [[ $5 == 'gba' ]]; then
    "$bin2s_path" -a 4 -H "$header_h" "$input_bin" > "$output_s"
else
    "$bin2s_path" -a 4 -H "$header_h" "$input_bin" > /dev/null
    echo "#include \"`(echo $header_h)`\"" > "$output_s"
    xxd -i "$input_bin" | awk '{ gsub(/___/, ""); gsub(/data_/, ""); gsub(/unsigned char/, "const uint8_t"); if (NR != last) print }' >> "$output_s"
fi

# Check if the last command was successful
if [ $? -ne 0 ]; then
    echo "bin2s command failed."
    exit 1
fi

# Append a newline to the end of the output_s file
echo >> "$output_s"
