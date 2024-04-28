#!/usr/bin/env python3

import subprocess
import os
import shutil
import tempfile
import sys

def main():
    build_romfs_path = sys.argv[1]
    assets_folder = sys.argv[2]
    elf2nro_path = sys.argv[3]
    nacptool_path = sys.argv[4]
    icon_path = sys.argv[5]
    output_binary = sys.argv[6]

    with tempfile.TemporaryDirectory() as temp_dir:
        # Construct the destination path by appending the source directory name
        # to the temporary directory path
        dest_dir = os.path.join(temp_dir, 'romfs', os.path.basename(assets_folder))

        # Create the 'romfs' directory inside the temporary directory
        os.makedirs(dest_dir)

        # Copy the source directory to the destination directory recursively
        shutil.copytree(assets_folder, dest_dir, dirs_exist_ok=True)

        # Print the path of the copied directory for verification
        print(f"Copied '{assets_folder}' to '{dest_dir}'")
        args = [build_romfs_path, os.path.join(temp_dir, 'romfs'), os.path.join(temp_dir, 'romfs.bin')]
        return_code = subprocess.run(args, stdout=subprocess.DEVNULL)

        if return_code.returncode != 0:
            print("build_romfs command failed.")
            sys.exit(1)

        args = [nacptool_path, "--create", "Apotris", "akouzoukos", "v4.0.1", os.path.join(temp_dir, 'control.nacp')]
        return_code = subprocess.run(args)

        if return_code.returncode != 0:
            print("nacptool command failed.")
            sys.exit(1)

        args = [elf2nro_path, output_binary, output_binary + '.nro', f'--nacp={os.path.join(temp_dir, "control.nacp")}', f'--romfs={os.path.join(temp_dir, "romfs.bin")}', f'--icon={icon_path}']
        #return_code = subprocess.run(args, stdout=subprocess.DEVNULL)
        return_code = subprocess.run(args)

        if return_code.returncode != 0:
            print("elf2nro command failed.")
            sys.exit(1)


if __name__ == "__main__":
    main()
