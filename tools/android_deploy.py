import argparse
import os
import shutil
import subprocess
import sys
import glob

def run_command(command, cwd=None, env=None):
    try:
        subprocess.check_call(command, cwd=cwd, env=env)
    except subprocess.CalledProcessError as e:
        print(f"Error executing command: {command}")
        sys.exit(e.returncode)

def main():
    parser = argparse.ArgumentParser(description='Android deploy script for Apotris')
    parser.add_argument('--build-dir', required=True, help='Meson build directory')
    parser.add_argument('--source-dir', required=True, help='Meson source directory')
    parser.add_argument('--libmain', required=True, help='Path to built libmain.so')
    parser.add_argument('--compiler', required=True, nargs='+', help='C++ compiler command')
    parser.add_argument('--output', required=True, help='Output APK path')

    args = parser.parse_args()

    build_dir = args.build_dir
    source_dir = args.source_dir
    libmain_path = args.libmain
    android_project_dir = os.path.join(source_dir, 'android')
    jni_libs_dir = os.path.join(android_project_dir, 'app/src/main/jniLibs/arm64-v8a')
    assets_dir = os.path.join(android_project_dir, 'app/src/main/assets/assets')
    data_dir = os.path.join(android_project_dir, 'app/src/main/assets/data')
    java_src_dir = os.path.join(android_project_dir, 'app/src/main/java/org/libsdl/app')

    print(f"Deploying Android build...")

    # Ensure directories exist
    os.makedirs(jni_libs_dir, exist_ok=True)
    os.makedirs(assets_dir, exist_ok=True)
    os.makedirs(data_dir, exist_ok=True)
    os.makedirs(java_src_dir, exist_ok=True)

    # 1. Copy libmain.so
    print(f"Copying {libmain_path} to {jni_libs_dir}")
    shutil.copy(libmain_path, os.path.join(jni_libs_dir, 'libmain.so'))

    # 2. Find and copy SDL2 libs
    print("Copying SDL2 libraries...")
    for root, dirs, files in os.walk(build_dir):
        for file in files:
            if file in ['libsdl2.so', 'libSDL2.so']:
                shutil.copy(os.path.join(root, file), os.path.join(jni_libs_dir, 'libSDL2.so'))
            elif file in ['libsdl2mixer.so', 'libSDL2_mixer.so']:
                shutil.copy(os.path.join(root, file), os.path.join(jni_libs_dir, 'libSDL2_mixer.so'))
            elif file in ['libvorbis.so', 'libvorbisfile.so', 'libvorbisenc.so', 'libogg.so']:
                shutil.copy(os.path.join(root, file), jni_libs_dir)

    # 3. Find and copy libc++_shared.so
    print("Locating libc++_shared.so...")
    try:
        # Use the compiler to find the library path
        cmd = args.compiler + ['-print-file-name=libc++_shared.so']
        output = subprocess.check_output(cmd, encoding='utf-8').strip()
        if os.path.exists(output):
            print(f"Found at {output}")
            shutil.copy(output, jni_libs_dir)
        else:
            print("Warning: libc++_shared.so not found via compiler. Trying NDK layout assumption...")
            # Fallback? Maybe strict error?
            # User must have toolchain in PATH.
            pass
    except Exception as e:
        print(f"Error locating libc++_shared.so: {e}")

    # 4. Copy SDL2 Java files
    print("Copying SDL2 Java files...")
    # Find subprojects/SDL2*
    subprojects_dir = os.path.join(source_dir, 'subprojects')
    sdl_java_path = None
    
    # Try to find unpacked SDL2
    for item in os.listdir(subprojects_dir):
        if item.startswith('SDL2') and os.path.isdir(os.path.join(subprojects_dir, item)):
            candidate = os.path.join(subprojects_dir, item, 'android-project/app/src/main/java/org/libsdl/app')
            if os.path.exists(candidate):
                sdl_java_path = candidate
                break
    
    if sdl_java_path:
        for file in os.listdir(sdl_java_path):
            if file.endswith('.java'):
                shutil.copy(os.path.join(sdl_java_path, file), java_src_dir)
        
        # Patch SDLActivity.java
        sdl_activity = os.path.join(java_src_dir, 'SDLActivity.java')
        if os.path.exists(sdl_activity):
            print("Patching SDLActivity.java...")
            with open(sdl_activity, 'r') as f:
                content = f.read()
            content = content.replace('mHIDDeviceManager = HIDDeviceManager.acquire(this);', '// mHIDDeviceManager = HIDDeviceManager.acquire(this);')
            with open(sdl_activity, 'w') as f:
                f.write(content)
    else:
        print("Warning: Could not find SDL2 Java sources in subprojects.")

    # 5. Copy Assets
    print("Copying assets...")
    # Clean up destination first? No, just copy over.
    src_assets = os.path.join(source_dir, 'assets')
    src_data = os.path.join(source_dir, 'data')
    
    # Copy directory content
    if os.path.exists(src_assets):
        # We need to copy *contents* of assets/ to android/.../assets/assets/
        # shutil.copytree requires dest not to exist usually, or dirs_exist_ok in 3.8+
        # Let's just use cp -r equivalent via shutil
        for item in os.listdir(src_assets):
            s = os.path.join(src_assets, item)
            d = os.path.join(assets_dir, item)
            if os.path.isdir(s):
                if os.path.exists(d): shutil.rmtree(d)
                shutil.copytree(s, d)
            else:
                shutil.copy2(s, d)

    if os.path.exists(src_data):
        for item in os.listdir(src_data):
            s = os.path.join(src_data, item)
            d = os.path.join(data_dir, item)
            if os.path.isdir(s):
                if os.path.exists(d): shutil.rmtree(d)
                shutil.copytree(s, d)
            else:
                shutil.copy2(s, d)

    # 6. Generate file list
    print("Generating file list...")
    file_list_path = os.path.join(assets_dir, 'file_list.txt')
    with open(file_list_path, 'w') as f:
        # Walk assets_dir (which is .../assets/assets) relative to it
        for root, _, files in os.walk(assets_dir):
            for file in files:
                if file == 'file_list.txt': continue
                rel_path = os.path.relpath(os.path.join(root, file), assets_dir)
                f.write(rel_path + '\n')

    # 7. Run Gradle
    print("Running Gradle...")
    # Check for gradlew
    gradlew = os.path.join(android_project_dir, 'gradlew')
    if os.path.exists(gradlew) and os.path.exists(os.path.join(android_project_dir, 'gradle', 'wrapper', 'gradle-wrapper.jar')):
        # Make executable
        os.chmod(gradlew, 0o755)
        run_command([gradlew, 'assembleDebug'], cwd=android_project_dir)
    else:
        print("Gradle wrapper not found or incomplete. Falling back to system gradle.")
        run_command(['gradle', 'assembleDebug'], cwd=android_project_dir)

    # 8. Copy Output APK
    apk_out = os.path.join(android_project_dir, 'app/build/outputs/apk/debug/app-debug.apk')
    if os.path.exists(apk_out):
        print(f"Copying APK to {args.output}")
        shutil.copy(apk_out, args.output)
    else:
        print("Error: APK not found after build.")
        sys.exit(1)

if __name__ == '__main__':
    main()
