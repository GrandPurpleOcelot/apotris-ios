project('Apotris',
        'c', 'cpp',
        version : '4.0.0',
        license : 'GPL3', meson_version : '>=0.60.0',
        default_options : ['buildtype=debugoptimized', 'cpp_std=c++17', 'default_library=static', 'warning_level=0', 'b_lto=true'])


# The name of your project.
name = 'Apotris'

# Define the platform-specific directories
src_dir = 'source'
inc_dir = 'include'
data_dir = 'data'
audio_dir = 'assets'

# Include directories
includes = [
    'include',
    data_dir,
]

# Bring in all source files
sources = files(
    src_dir / 'audio_files.cpp',
    src_dir / 'blockEngine.cpp',
    src_dir / 'bot.cpp',
    src_dir / 'descriptions.cpp',
    src_dir / 'flashSaves.cpp',
    src_dir / 'game.cpp',
    src_dir / 'gbp_logo.cpp',
    src_dir / 'interrupt.cpp',
    src_dir / 'liba_gba.cpp',
    src_dir / 'liba_mm.cpp',
    src_dir / 'liba_pc.cpp',
    src_dir / 'liba_switch.cpp',
    src_dir / 'liba_rgb30.cpp',
    src_dir / 'liba_web.cpp',
    src_dir / 'liba_window.cpp',
    src_dir / 'main.cpp',
    src_dir / 'menuSystem.cpp',
    src_dir / 'menus.cpp',
    src_dir / 'multiplayer.cpp',
    src_dir / 'multiplayerClasses.cpp',
    src_dir / 'music.cpp',
    src_dir / 'qrcodegen.c',
    src_dir / 'rumble.c',
    src_dir / 'saving.cpp',
    src_dir / 'scene.cpp',
    src_dir / 'screenAudio.cpp',
    src_dir / 'screenControls.cpp',
    src_dir / 'screenCredits.cpp',
    src_dir / 'screenGraphics.cpp',
    src_dir / 'screenHandling.cpp',
    src_dir / 'screenLinks.cpp',
    src_dir / 'screenPlayOptions.cpp',
    src_dir / 'screenSkinEditor.cpp',
    src_dir / 'screenTitle.cpp',
    src_dir / 'sprites.cpp',
    src_dir / 'tetromino.cpp',
    src_dir / 'text.cpp',
    src_dir / 'nanotime.c',
    src_dir / 'shader.cpp',
)

# Bring in binary/data files
subdir(data_dir)

# Common build file references
soundbank_file = files('data/soundbank.bin')
effect_locations_file = files('data/effect_locations.bin')
icon_file = files('sprites/favicon32.bmp')

# Set up recommended compiler and linker flags for GBA before loading any other projects
if host_machine.system() == 'gba'
    subdir('meson/toolchain')
    add_project_arguments('-DLINK_WIRELESS_USE_SEND_RECEIVE_LATCH=1', language : 'cpp')
endif

build_type = get_option('buildtype')
# Don't use mGBA logging in release or custom build types, only debug and friends
if build_type.startswith('release') or build_type.startswith('custom')
  add_project_arguments('-DNO_DIAGNOSE', language : 'cpp')
  add_project_arguments('-DNO_DIAGNOSE', language : 'c')
endif
# Custom target is the fastest possible
if build_type.startswith('custom')
  add_global_arguments('-Ofast', language : 'cpp')
  add_global_arguments('-Ofast', language : 'c')
endif

# Various helper libraries, subprojects, and scripts common to all builds
fs = import('fs')
cmake = import('cmake')
tools = subproject('general-tools')
padbin = tools.get_variable('padbin_executable')
bin2s = files('tools/bin2s.py')
call_bin2s = files('tools/callBin2S.py')

# Required to invoke python scripts in a Windows-native build environment
pymod = import('python')
py3 = pymod.find_installation('python3', required: true)

# Windows uses a pre-compiled xxd
if build_machine.system() == 'windows'
    xxd = files('tools/xxd.exe')
else
    xxd = find_program('xxd', required : true)
endif


# Custom warning level flags
add_project_arguments('-Wall', language : 'cpp')
add_project_arguments('-Wall', language : 'c')
add_project_arguments('-Wno-array-bounds', language : 'cpp')
add_project_arguments('-Wno-array-bounds', language : 'c')
add_project_arguments('-Wno-shadow', language : 'cpp')
add_project_arguments('-Wno-shadow', language : 'c')
add_project_arguments('-Wno-unused', language : 'cpp')
add_project_arguments('-Wno-unused', language : 'c')


if host_machine.system() != 'gba'
    #############################
    # Windows, macOS, and Linux #
    #############################

    # Bring in required audio assets to copy to build output
    subdir('assets')

    # libmodplug must be built as a dynamic/shared library
    cmake_options = cmake.subproject_options()
    cmake_options.add_cmake_defines({ 'BUILD_SHARED_LIBS' : 'ON' })
    modplug = cmake.subproject('libmodplug', options : cmake_options)

    # Other required dependencies
    tilengine = subproject('Tilengine', default_options : ['default_library=static'])
    sdl = subproject('sdl2', default_options : ['default_library=shared'])
    soloud = subproject('SoLoud', default_options : ['default_library=static'])
    sdl_mixer = subproject('sdl2_mixer', default_options : ['default_library=shared'])

    dependencies = [
        modplug.dependency('modplug'),
        tilengine.get_variable('tilengine_dep'),
        sdl.get_variable('sdl2_dep'),
        soloud.get_variable('soloud_dep'),
        sdl_mixer.get_variable('sdl2_mixer_dep'),
        dependency('gl'),
        dependency('glew'),
    ]

    # Define non-GBA specific preprocessor guards
    add_project_arguments('-DPC', language : 'cpp')
    add_project_arguments('-DPC', language : 'c')
    add_project_arguments('-DTE', language : 'cpp')
    add_project_arguments('-DTE', language : 'c')

    generated_files = []
    # Assemble bin2s targets
    foreach binfile : binary_files
        filename = fs.name(binfile).split('.')[0]
        output_o = '@0@.bin.o'.format(fs.name(filename))
        output_h = '@0@_bin.h'.format(fs.name(filename))

        if host_machine.system() == 'darwin'
            output_s = '@0@_bin.s'.format(fs.name(filename))
            bin2s_target = custom_target(
                'bin2s_' + fs.name(binfile),
                output : [output_s, output_h],
                command : [
                    py3, call_bin2s, bin2s, binfile, output_h, output_s, 'apple', xxd
                ],
                depend_files : binfile,
            )
        else
            output_cpp = '@0@_bin.cpp'.format(fs.name(filename))
            bin2s_target = custom_target(
                'bin2s_' + fs.name(binfile),
                output : [output_cpp, output_h],
                command : [
                    py3, call_bin2s, bin2s, binfile, output_h, output_cpp, 'gcc', xxd
                ],
                depend_files : binfile,
            )
        endif
        generated_files += bin2s_target

    endforeach

    link_args = []
    # TODO: This should prevent a terminal from spawning when double-clicking Apotris.exe but it doesn't...
    if host_machine.system() == 'windows'
        add_project_arguments('-mwindows', language : 'cpp')
        add_project_arguments('-mwindows', language : 'c')
        link_args += '-mwindows'
    endif

    # The final Apotris compile target
    binary = executable(
        name,
        generated_files + sources,
        include_directories : includes,
        dependencies : dependencies,
        link_args : link_args,
    )
else
    ##########################################
    # GameBoy-Advance-specific configuration #
    ##########################################

    dependencies = [
        dependency('minrt'),
        dependency('libseven'),
        dependency('libtonc'),
        dependency('posprintf'),
        dependency('gbfs'),
        dependency('maxmod'),
        dependency('gba-link-connection'),
    ]

    dependencies_multi = [
        dependency('minrt-multiboot'),
        dependency('libseven'),
        dependency('libtonc'),
        dependency('posprintf'),
        dependency('gbfs'),
        dependency('maxmod'),
        dependency('gba-link-connection'),
    ]

    # Suppress LinkWireless header warnings
    add_project_arguments('-Wno-format', language : 'cpp')
    add_project_arguments('-Wno-format', language : 'c')

    # Enable all GBA-specific code
    add_project_arguments('-DGBA', language : 'cpp')
    add_project_arguments('-DGBA', language : 'c')

    # Disable mGBA printf on release builds
    if get_option('buildtype').startswith('release')
        add_project_arguments('-DNO_DIAGNOSE', language : 'cpp')
        add_project_arguments('-DNO_DIAGNOSE', language : 'c')
    endif

    # Get build-machine native tools
    gba_tools = subproject('gba-tools')
    gbfs = gba_tools.get_variable('gbfs_executable')
    gbafix = find_program('gbafix-py', required : true)
    makerom = find_program('makerom', required : true)

    # References to python helper scripts
    gbabuild = files('tools/gbaBuild.py')
    pocket_patch = files('tools/pocket.py')
    multibootbuild = files('tools/multibootBuild.py')

    generated_files = []
    # Assemble bin2s targets
    foreach binfile : binary_files
        filename = fs.name(binfile).split('.')[0]
        output_o = '@0@.bin.o'.format(fs.name(filename))
        output_h = '@0@_bin.h'.format(fs.name(filename))
        output_s = '@0@_bin.s'.format(fs.name(filename))

        bin2s_target = custom_target(
            'bin2s_' + fs.name(binfile),
            output : [output_s, output_h],
            command : [
                py3, call_bin2s, bin2s, binfile, output_h, output_s, 'gba', xxd
            ],
            depend_files : binfile,
        )

        generated_files += bin2s_target
    endforeach

    # Use gbfs to create archive to append to ROM with sound data
    audio_target = custom_target(
        'audio and effect bank',
        output : 'audio.gbfs',
        command : [
            gbfs, 'audio.gbfs', soundbank_file, effect_locations_file,
        ],
        depend_files : [soundbank_file, effect_locations_file]
    )
    generated_files += audio_target

    # Required for minrt
    link_args = ['-specs=nosys.specs']

    # The base multiboot elf binary
    multi = executable(
        name + '-multi',
        sources + generated_files,
        link_args : link_args,
        cpp_args : ['-DMULTIBOOT', '-Os'],
        c_args : ['-Os'],
        include_directories : includes,
        dependencies : dependencies_multi,
        name_suffix : 'elf')

    # The base multiboot ROM with no audio, unpadded
    multi_base = custom_target(
        name + '-multi-base',
        input : multi,
        output : name + '-multi-base.gba',
        command : [makerom, '@INPUT@', '@OUTPUT@'])

    # The base multiboot ROM with no audio, padded and fixed
    multi_rom = custom_target(
        name + '-multi',
        input : multi_base,
        output : name + '-multi.gba',
        command : [multibootbuild, padbin, '0x10', '@INPUT@', '@OUTPUT@', gbafix])

    output_o = 'Multiboot.o'
    output_h = 'Multiboot.h'
    output_s = 'Multiboot.s'

    embedded_multiboot = custom_target(
        'bin2s_multiboot',
        output : [output_s, output_h],
        command : [
            py3, call_bin2s, bin2s, multi_rom, output_h, output_s, 'gba', xxd
        ],
        depends : multi_rom,
    )

    generated_files += embedded_multiboot

    # The base elf binary
    elf = executable(
        name,
        sources + generated_files + files('source/sendMultiboot.cpp'),
        link_args : link_args,
        include_directories : includes,
        dependencies : dependencies,
        name_suffix : 'elf')

    # The base GBA ROM with no audio
    rom_base = custom_target(
        name + '-base',
        input : elf,
        output : name + '-base.gba',
        command : [makerom, '@INPUT@', '@OUTPUT@'])

    # The final GBA ROM with audio
    rom = custom_target(
        'final ROM with audio',
        output : name + '.gba',
        input : rom_base,
        command : [py3, gbabuild, padbin, '256', '@INPUT@', audio_target, '0x400000', '@OUTPUT@', gbafix],
        build_by_default : true)

    # Analogue Pocket rumble-patched ROM (masquerades as DRILL DOZER)
    pocket = custom_target(
        'patched ROM for Analogue Pocket rumble support',
        output : name + '-pocket.gba',
        input : rom,
        command : [py3, pocket_patch, gbafix, '@INPUT@', '@OUTPUT@'],
        build_by_default : false)

endif
