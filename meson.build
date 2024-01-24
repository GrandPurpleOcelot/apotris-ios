project('Apotris',
        'c', 'cpp',
        version : '4.0.0',
        license : 'GPL3',
        meson_version : '>=0.60.0',
        default_options : ['buildtype=minsize', 'cpp_std=c++17', 'default_library=static'])


# The name of your project.
name = 'Apotris'

# Define the platform-specific directories
src_dir = 'source'
lib_dir = 'lib'
inc_dir = 'include'
data_dir = 'data'
audio_dir = 'assets'

# Include directories
includes = [
    'include',
    'data',
]

# Find all source files
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
    src_dir / 'libgbfs.c',
    src_dir / 'main.cpp',
    src_dir / 'menuSystem.cpp',
    src_dir / 'menus.cpp',
    src_dir / 'multiplayer.cpp',
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
    src_dir / 'sfMod.cpp',
    src_dir / 'sprites.cpp',
    src_dir / 'tetromino.cpp',
    src_dir / 'text.cpp',
)

binary_files = files(
    data_dir / 'classic10tiles.bin',
    data_dir / 'classic11tiles.bin',
    data_dir / 'classic12tiles.bin',
    data_dir / 'classic13tiles.bin',
    data_dir / 'classic14tiles.bin',
    data_dir / 'classic15tiles.bin',
    data_dir / 'classic16tiles.bin',
    data_dir / 'classic1tiles.bin',
    data_dir / 'classic2tiles.bin',
    data_dir / 'classic3tiles.bin',
    data_dir / 'classic4tiles.bin',
    data_dir / 'classic5tiles.bin',
    data_dir / 'classic6tiles.bin',
    data_dir / 'classic7tiles.bin',
    data_dir / 'classic8tiles.bin',
    data_dir / 'classic9tiles.bin',
    data_dir / 'classic_pal.bin',
    data_dir / 'defaultGradient.bin',
    data_dir / 'discord_qr.bin',
    data_dir / 'effect10_tiles.bin',
    data_dir / 'effect11_tiles.bin',
    data_dir / 'effect12_tiles.bin',
    data_dir / 'effect13_tiles.bin',
    data_dir / 'effect14_tiles.bin',
    data_dir / 'effect15_tiles.bin',
    data_dir / 'effect1_tiles.bin',
    data_dir / 'effect2_tiles.bin',
    data_dir / 'effect3_tiles.bin',
    data_dir / 'effect4_tiles.bin',
    data_dir / 'effect5_tiles.bin',
    data_dir / 'effect6_tiles.bin',
    data_dir / 'effect7_tiles.bin',
    data_dir / 'effect8_tiles.bin',
    data_dir / 'effect9_tiles.bin',
    data_dir / 'effect_locations.bin',
    data_dir / 'font1tiles.bin',
    data_dir / 'font2tiles.bin',
    data_dir / 'font_pal.bin',
    data_dir / 'meter1_tiles.bin',
    data_dir / 'meter2_tiles.bin',
    data_dir / 'meter3_tiles.bin',
    data_dir / 'meter4_tiles.bin',
    data_dir / 'move1map.bin',
    data_dir / 'move1tiles.bin',
    data_dir / 'move2map.bin',
    data_dir / 'move2tiles.bin',
    data_dir / 'move3map.bin',
    data_dir / 'move3tiles.bin',
    data_dir / 'move4map.bin',
    data_dir / 'move4tiles.bin',
    data_dir / 'move5map.bin',
    data_dir / 'move5tiles.bin',
    data_dir / 'move6map.bin',
    data_dir / 'move6tiles.bin',
    data_dir / 'move7map.bin',
    data_dir / 'move7tiles.bin',
    data_dir / 'move8map.bin',
    data_dir / 'move8tiles.bin',
    data_dir / 'paypal_qr.bin',
    data_dir / 'site_qr.bin',
    data_dir / 'small.bin',
    data_dir / 'small10map.bin',
    data_dir / 'small10tiles.bin',
    data_dir / 'small11map.bin',
    data_dir / 'small11tiles.bin',
    data_dir / 'small12map.bin',
    data_dir / 'small12tiles.bin',
    data_dir / 'small13map.bin',
    data_dir / 'small13tiles.bin',
    data_dir / 'small14map.bin',
    data_dir / 'small14tiles.bin',
    data_dir / 'small1map.bin',
    data_dir / 'small1tiles.bin',
    data_dir / 'small2map.bin',
    data_dir / 'small2tiles.bin',
    data_dir / 'small3map.bin',
    data_dir / 'small3tiles.bin',
    data_dir / 'small4map.bin',
    data_dir / 'small4tiles.bin',
    data_dir / 'small5map.bin',
    data_dir / 'small5tiles.bin',
    data_dir / 'small6map.bin',
    data_dir / 'small6tiles.bin',
    data_dir / 'small7map.bin',
    data_dir / 'small7tiles.bin',
    data_dir / 'small8map.bin',
    data_dir / 'small8tiles.bin',
    data_dir / 'small9map.bin',
    data_dir / 'small9tiles.bin',
    data_dir / 'soundbank.bin',
    data_dir / 'sprite10map.bin',
    data_dir / 'sprite10tiles.bin',
    data_dir / 'sprite11map.bin',
    data_dir / 'sprite11tiles.bin',
    data_dir / 'sprite12map.bin',
    data_dir / 'sprite12tiles.bin',
    data_dir / 'sprite13map.bin',
    data_dir / 'sprite13tiles.bin',
    data_dir / 'sprite14map.bin',
    data_dir / 'sprite14tiles.bin',
    data_dir / 'sprite15map.bin',
    data_dir / 'sprite15tiles.bin',
    data_dir / 'sprite16map.bin',
    data_dir / 'sprite16tiles.bin',
    data_dir / 'sprite17map.bin',
    data_dir / 'sprite17tiles.bin',
    data_dir / 'sprite18map.bin',
    data_dir / 'sprite18tiles.bin',
    data_dir / 'sprite19map.bin',
    data_dir / 'sprite19tiles.bin',
    data_dir / 'sprite1map.bin',
    data_dir / 'sprite1tiles.bin',
    data_dir / 'sprite20map.bin',
    data_dir / 'sprite20tiles.bin',
    data_dir / 'sprite21map.bin',
    data_dir / 'sprite21tiles.bin',
    data_dir / 'sprite22map.bin',
    data_dir / 'sprite22tiles.bin',
    data_dir / 'sprite23map.bin',
    data_dir / 'sprite23tiles.bin',
    data_dir / 'sprite24map.bin',
    data_dir / 'sprite24tiles.bin',
    data_dir / 'sprite25map.bin',
    data_dir / 'sprite25tiles.bin',
    data_dir / 'sprite26map.bin',
    data_dir / 'sprite26tiles.bin',
    data_dir / 'sprite27map.bin',
    data_dir / 'sprite27tiles.bin',
    data_dir / 'sprite28map.bin',
    data_dir / 'sprite28tiles.bin',
    data_dir / 'sprite29map.bin',
    data_dir / 'sprite29tiles.bin',
    data_dir / 'sprite2map.bin',
    data_dir / 'sprite2tiles.bin',
    data_dir / 'sprite30map.bin',
    data_dir / 'sprite30tiles.bin',
    data_dir / 'sprite31map.bin',
    data_dir / 'sprite31tiles.bin',
    data_dir / 'sprite32map.bin',
    data_dir / 'sprite32tiles.bin',
    data_dir / 'sprite33map.bin',
    data_dir / 'sprite33tiles.bin',
    data_dir / 'sprite34map.bin',
    data_dir / 'sprite34tiles.bin',
    data_dir / 'sprite35map.bin',
    data_dir / 'sprite35tiles.bin',
    data_dir / 'sprite36map.bin',
    data_dir / 'sprite36tiles.bin',
    data_dir / 'sprite37map.bin',
    data_dir / 'sprite37tiles.bin',
    data_dir / 'sprite38map.bin',
    data_dir / 'sprite38tiles.bin',
    data_dir / 'sprite39map.bin',
    data_dir / 'sprite39tiles.bin',
    data_dir / 'sprite3map.bin',
    data_dir / 'sprite3tiles.bin',
    data_dir / 'sprite40map.bin',
    data_dir / 'sprite40tiles.bin',
    data_dir / 'sprite41map.bin',
    data_dir / 'sprite41tiles.bin',
    data_dir / 'sprite42map.bin',
    data_dir / 'sprite42tiles.bin',
    data_dir / 'sprite43map.bin',
    data_dir / 'sprite43tiles.bin',
    data_dir / 'sprite44map.bin',
    data_dir / 'sprite44tiles.bin',
    data_dir / 'sprite45map.bin',
    data_dir / 'sprite45tiles.bin',
    data_dir / 'sprite46map.bin',
    data_dir / 'sprite46tiles.bin',
    data_dir / 'sprite47map.bin',
    data_dir / 'sprite47tiles.bin',
    data_dir / 'sprite48map.bin',
    data_dir / 'sprite48tiles.bin',
    data_dir / 'sprite49map.bin',
    data_dir / 'sprite49tiles.bin',
    data_dir / 'sprite4map.bin',
    data_dir / 'sprite4tiles.bin',
    data_dir / 'sprite50map.bin',
    data_dir / 'sprite50tiles.bin',
    data_dir / 'sprite51map.bin',
    data_dir / 'sprite51tiles.bin',
    data_dir / 'sprite52map.bin',
    data_dir / 'sprite52tiles.bin',
    data_dir / 'sprite53map.bin',
    data_dir / 'sprite53tiles.bin',
    data_dir / 'sprite54map.bin',
    data_dir / 'sprite54tiles.bin',
    data_dir / 'sprite55map.bin',
    data_dir / 'sprite55tiles.bin',
    data_dir / 'sprite56map.bin',
    data_dir / 'sprite56tiles.bin',
    data_dir / 'sprite57map.bin',
    data_dir / 'sprite57tiles.bin',
    data_dir / 'sprite58map.bin',
    data_dir / 'sprite58tiles.bin',
    data_dir / 'sprite5map.bin',
    data_dir / 'sprite5tiles.bin',
    data_dir / 'sprite6map.bin',
    data_dir / 'sprite6tiles.bin',
    data_dir / 'sprite7map.bin',
    data_dir / 'sprite7tiles.bin',
    data_dir / 'sprite8map.bin',
    data_dir / 'sprite8tiles.bin',
    data_dir / 'sprite9map.bin',
    data_dir / 'sprite9tiles.bin',
    data_dir / 'sprite_pal.bin',
    data_dir / 'title1tiles.bin',
    data_dir / 'title2tiles.bin',
    data_dir / 'title_pal.bin',
    data_dir / 'wiki_qr.bin',
)

soundbank_file = files('data/soundbank.bin')
effect_locations_file = files('data/effect_locations.bin')
icon_file = files('sprites/favicon32.bmp')

fs = import('fs')
cmake = import('cmake')
source_root = meson.project_source_root()
bin2s = find_program('bin2s', required : true)
bin2s_script_path = join_paths(source_root, 'tools', 'call_bin2s.sh')
call_bin2s = find_program(bin2s_script_path, required : true)

# Windows, macOS, and Linux
if host_machine.system() != 'gba'
    subdir('assets')

    cmake_options = cmake.subproject_options()
    cmake_options.add_cmake_defines({ 'BUILD_SHARED_LIBS' : 'ON' })
    modplug = cmake.subproject('libmodplug', options : cmake_options)

    tilengine = subproject('Tilengine', default_options : ['default_library=static'])
    sdl = subproject('sdl2', default_options : ['default_library=shared'])
    soloud = subproject('SoLoud', default_options : ['default_library=static'])
    sdl_mixer = subproject('sdl2_mixer', default_options : ['default_library=shared'])

    dependencies = [
        modplug.dependency('modplug'),
        tilengine.get_variable('tilengine_dep'),
        sdl.get_variable('sdl2_dep'),
        soloud.get_variable('soloud_dep'),
        sdl_mixer.get_variable('sdl2_mixer_dep')
    ]

    add_project_arguments('-DRGB', language : 'cpp')
    add_project_arguments('-DRGB', language : 'c')
    add_project_arguments('-DTE', language : 'cpp')
    add_project_arguments('-DTE', language : 'c')
    add_project_arguments('-Wno-array-bounds', language : 'cpp')
    add_project_arguments('-Wno-array-bounds', language : 'c')

    generated_files = []
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
                    call_bin2s, bin2s, binfile, output_h, output_s, 'apple'
                ],
                depend_files : binfile,
            )
        else
            output_cpp = '@0@_bin.cpp'.format(fs.name(filename))
            bin2s_target = custom_target(
                'bin2s_' + fs.name(binfile),
                output : [output_cpp, output_h],
                command : [
                    call_bin2s, bin2s, binfile, output_h, output_cpp
                ],
                depend_files : binfile,
            )
        endif
        generated_files += bin2s_target

    endforeach

    link_args = []
    if host_machine.system() == 'windows'
        link_args += '-mwindows'
    endif
    binary = executable(
        name,
        generated_files + sources,
        include_directories : includes,
        dependencies : dependencies,
        link_args : link_args,
    )
    # Platform-specific configuration for AdvancedGameBoy
else
    # Set up recommended compiler and linker flags.
    subdir('meson/toolchain')

    dependencies = [
        dependency('dka-rom-rt'),
        dependency('libugba'),
        dependency('libtonc'),
        dependency('posprintf'),
        dependency('gbfs'),
        dependency('maxmod'),
        dependency('gba-link-connection'),
    ]
    add_project_arguments('-DGBA', language : 'cpp')
    add_project_arguments('-DGBA', language : 'c')

    cat = find_program('cat', required : true)
    gbfs = find_program('gbfs', required : true)
    gbafix = find_program('gbafix', required : true)
    padbin = find_program('padbin', required : true)
    makerom = find_program('makerom', required : true)

    gbabuild_script_path = join_paths(source_root, 'tools', 'gbabuild.sh')
    gbabuild = find_program(gbabuild_script_path, required : true)
    pocket_script_path = join_paths(source_root, 'tools', 'pocket.sh')
    pocket_patch = find_program(pocket_script_path, required : true)

    generated_files = []
    foreach binfile : binary_files
        filename = fs.name(binfile).split('.')[0]
        output_o = '@0@.bin.o'.format(fs.name(filename))
        output_h = '@0@_bin.h'.format(fs.name(filename))
        output_s = '@0@_bin.s'.format(fs.name(filename))

        bin2s_target = custom_target(
            'bin2s_' + fs.name(binfile),
            output : [output_s, output_h],
            command : [
                call_bin2s, bin2s, binfile, output_h, output_s, 'gba'
            ],
            depend_files : binfile,
        )

        generated_files += bin2s_target
    endforeach

    audio_target = custom_target(
        'audio and effect bank',
        output : 'audio.gbfs',
        command : [
            gbfs, 'audio.gbfs', soundbank_file, effect_locations_file,
        ],
        depend_files : [soundbank_file, effect_locations_file]
    )
    generated_files += audio_target

    elf = executable(
        name,
        sources + generated_files,
        include_directories : includes,
        dependencies : dependencies,
        name_suffix : 'elf')

    rom_base = custom_target(
        name + '-base',
        input : elf,
        output : name + '-base.gba',
        command : [makerom, '@INPUT@', '@OUTPUT@'])

    rom = custom_target(
        'final ROM with audio',
        output : name + '.gba',
        input : rom_base,
        command : [gbabuild, padbin, '256', '@INPUT@', audio_target, '0x400000', '@OUTPUT@', gbafix],
        build_by_default : true)

    pocket = custom_target(
        'patched ROM for Analogue Pocket rumble support',
        output : name + '-pocket.gba',
        input : rom,
        command : [pocket_patch, gbafix, '@INPUT@'],
        build_by_default : false)

endif
