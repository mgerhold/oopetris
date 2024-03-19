project(
    'oopetris',
    'cpp',
    'c',
    meson_version: '>=1.3.0',
    default_options: {
        'buildtype': 'debug',
        'optimization': '3',
        'strip': 'true',
        'cpp_std': ['c++23', 'c++latest', 'vc++latest', 'c++20'],
        'b_ndebug': 'if-release',

    },
    version: '1.0.0-alpha',
)

compile_args = []
deps = []
recording_dependencies = []

cpp = meson.get_compiler('cpp')


if cpp.get_id() == 'gcc'
    compile_args += ['-Wold-style-cast']
elif cpp.get_id() == 'clang'

    build_with_libcpp_option = get_option('clang_libcpp')

    build_with_libcpp = build_with_libcpp_option.enabled()

    if build_with_libcpp_option.auto()
        if host_machine.system() == 'darwin'
            build_with_libcpp = true
        else

            cpp_lib = cpp.find_library('c++', required: false)
            cppabi_lib = cpp.find_library('c++abi', required: false)

            if cpp_lib.found() and cppabi_lib.found()
                build_with_libcpp = true
            else
                build_with_libcpp = false
            endif
        endif
    endif

    if build_with_libcpp
        add_global_link_arguments('-stdlib=libc++', language:['cpp'])
        add_global_arguments('-stdlib=libc++', language:['cpp'])
        deps += cpp.find_library('c++')
        deps += cpp.find_library('c++abi')
    endif

endif

if get_option('buildtype') == 'debug' or get_option('buildtype') == 'debugoptimized'
    compile_args += ['-DDEBUG_BUILD']
endif



inc_dirs = []
recording_inc_dirs = []

only_allow_native_libs = false
if meson.is_cross_build()
    if host_machine.system() == 'switch'
        # we do not link to code that was compiled with gcc 10.1, the code we link with is all compiled with gcc 13.2
        compile_args += ['-Wno-psabi']

        only_allow_native_libs = true
    endif
endif


sdl2_dep = dependency(
    'sdl2',
    'SDL2',
    allow_fallback: false,
    required: only_allow_native_libs,
)

if sdl2_dep.found()
    deps += sdl2_dep
else
    deps += dependency(
        'sdl2',
        required: true,
        default_options: {'test': 'false'},
    )
    deps += dependency(
        'sdl2main',
        required: true,
        fallback: 'sdl2',
    )
endif

deps += dependency(
    'sdl2_ttf',
    'SDL2_ttf',
    allow_fallback: not only_allow_native_libs,
    required: true,
)


deps += dependency(
    'sdl2_image',
    'SDL2_image',
    allow_fallback: not only_allow_native_libs,
    required: true,
)


# a dirty thing atm, until mpg123 is ported to meson (maybe never...)
mpg123_dep = dependency(
    'mpg123',
    allow_fallback: true,
    required: false,
)
sdl2_mixer_flags = {'flac': 'enabled'}
sdl2_mixer_defines = ['-DAUDIO_WITH_FLAC_SUPPORT']
if mpg123_dep.found()
    sdl2_mixer_flags += {'mpg123': 'enabled'}
    sdl2_mixer_defines += '-DAUDIO_WITH_MP3_SUPPORT'
else
    mpg123_dep = cpp.find_library('mpg123', required: only_allow_native_libs)
    if mpg123_dep.found()
        sdl2_mixer_flags += {'mpg123': 'enabled'}
        sdl2_mixer_defines += '-DAUDIO_WITH_MP3_SUPPORT'

        meson.override_dependency('mpg123', mpg123_dep)
    endif
endif



deps += dependency(
    'sdl2_mixer',
    'SDL2_mixer',
    allow_fallback: not only_allow_native_libs,
    required: true,
    default_options: sdl2_mixer_flags,
)

if (meson.is_cross_build() and host_machine.system() == 'switch')

    fmt_lib = dependency(
        'fmt',
        required: false,
        allow_fallback: false,
    )

    if fmt_lib.found()
        deps += fmt_lib
    else

        fmt_header_only_dep = dependency(
            'fmt_header_only',
            required: true,
            allow_fallback: true,
            default_options: {'default_library': 'static'},
        )

        meson.override_dependency('fmt', fmt_header_only_dep)

        deps += fmt_header_only_dep

    endif

else

    # clang with libc++ creates some really long and confusing linker errors, so just use the header only library
    if cpp.get_id() == 'clang' and compile_args.contains('-stdlib=libc++')
        fmt_dep = dependency(
            'fmt_header_only',
            required: true,
            allow_fallback: true,
            default_options: {'default_library': 'static'},
        )
        meson.override_dependency('fmt', fmt_dep)
    else
        fmt_dep = dependency(
            'fmt',
            required: true,
        )
    endif

    recording_dependencies += fmt_dep
    deps += fmt_dep

endif

deps += dependency(
    'spdlog',
    required: true,
    default_options: {'tests': 'disabled'},
)

nlohmann_json_dep = dependency(
    'nlohmann_json',
    required: true,
)

recording_dependencies += nlohmann_json_dep
deps += nlohmann_json_dep

# check std::expected support
have_std_expected = cpp.compiles(
    '''
#include <expected>
int main() {
    using T = std::expected<int, char>;
    return 0;
}
''',
    args: compile_args,
)

if not have_std_expected
    message('Compiler doesn\'t support std::expected, using fallback')
    compile_args += ['-D_USE_TL_EXPECTED']
    tl_exp_dep = dependency('tl-expected', required: true)
    recording_dependencies += tl_exp_dep
    deps += tl_exp_dep
else
    message('Compiler support std::expected, using that')
endif

# check std::optional support
have_std_optional = cpp.compiles(
    '''
#include <optional>
int main() {
    using T = std::optional<int>;
    return 0;
}
''',
    args: compile_args,
)

if not have_std_optional
    message('Compiler doesn\'t support std::optional, using fallback')
    compile_args += ['-D_USE_TL_OPTIONAL']
    tl_opt_dep = dependency('tl-optional', required: true)
    recording_dependencies += tl_opt_dep
    deps += tl_opt_dep
else
    message('Compiler support std::optional, using that')
endif



magic_enum_dep = dependency(
    'magic_enum',
    required: true,
    default_options: {'test': false},
)
recording_dependencies += magic_enum_dep
deps += magic_enum_dep

argparse_dep = dependency('argparse', required: true)
recording_dependencies += argparse_dep
deps += argparse_dep

online_multiplayer_supported = true

if meson.is_cross_build() and host_machine.system() == 'switch'
    compile_args += ['-D_ONLINE_MULTIPLAYER_NOT_SUPPORTED']
    online_multiplayer_supported = false
else
    deps += dependency(
        'cpp-httplib',
        required: true,
        default_options: {
            'cpp-httplib_openssl': 'enabled',
            'cpp-httplib_zlib': 'enabled',
        },
    )
endif

deps += dependency(
    'utf8cpp',
    required: true,
    version: '>=4.0.0',
)


if (meson.is_cross_build() and host_machine.system() == 'switch')

    switch_dependencies = [
        'EGL',
        'GLESv2',
        'glapi',
        'drm_nouveau',
        'nx',
        'freetype',
        'z',
        'bz2',
        'png',
        'opusfile',
        'opus',
        'vorbisfile',
        'vorbis',
        'ogg',
        'modplug',
        'harfbuzz',
        'FLAC',
        'curl',
    ]


    library_dirs = meson.get_external_property('library_dirs')

    foreach dep : switch_dependencies
        deps += cpp.find_library(
            dep,
            required: true,
            dirs: library_dirs,
        )
    endforeach

endif

is_flatpak_build = get_option('flatpak_build')

have_file_dialogs = false
have_discord_sdk = false


nfde_dep = dependency(
    'nativefiledialog-extended',
    required: not meson.is_cross_build(),
    default_options: {'xdg-desktop-portal': is_flatpak_build ? 'enabled' : 'auto'},
)
if nfde_dep.found()
    have_file_dialogs = true
    deps += nfde_dep
    compile_args += '-D_HAVE_FILE_DIALOGS'
endif

discord_sdk_dep = dependency(
    'discord-game-sdk',
    required: not meson.is_cross_build(),
    default_options: {'clang_libcpp': 'disabled'},
)
if discord_sdk_dep.found()
    have_discord_sdk = true
    deps += discord_sdk_dep
    compile_args += '-D_HAVE_DISCORD_SDK'
endif


src_files = []
recording_src_files = []

subdir('src')

## TODO: only install needed ones, since sometimes we only need e.g. flacs or mp3 and no icons etc.
## install assets
install_subdir(
    'assets',
    install_dir: 'share/oopetris',
    exclude_files: ['oopetris.desktop.in', 'OOPetris.svg', 'recordings.magic'],
    exclude_directories: ['icon'],
)

app_name = 'oopetris'
if is_flatpak_build
    app_name = 'com.github.mgerhold.OOPetris'
    compile_args += ['-DFLATPAK_BUILD', '-DBUILD_INSTALLER']
endif

conf = configuration_data()
conf.set('APP_NAME', app_name)

datadir = get_option('prefix') / get_option('datadir')


if host_machine.system() == 'linux'

    fs = import('fs')

    magic_dir = datadir / 'misc'
    magic_file = magic_dir / 'magic'
    oopetris_magic_file = 'assets' / 'recordings.magic'

    if fs.exists(magic_file)

        cat_prog = find_program('cat')

        custom_target(
            'magic_file_append',
            command: [cat_prog, '@INPUT@'],
            capture: true,
            input: [oopetris_magic_file, magic_file],
            output: 'magic',
            install_dir: magic_dir,
        )

    else

        install_data(
            oopetris_magic_file,
            install_dir: magic_dir,
            rename: ['magic'],
        )

    endif
endif

desktop_file = configure_file(
    input: 'assets/oopetris.desktop.in',
    output: app_name + '.desktop',
    configuration: conf,
    install: true,
    install_dir: datadir / 'applications',
)

logos = [
    '24x24.png',
    '48x48.png',
    '64x64.png',
    '72x72.png',
    '96x96.png',
    '128x128.png',
    '144x144.png',
    '160x160.png',
    '192x192.png',
    '256x256.png',
    '512x512.png',
    'scalable.svg',
]

foreach logo : logos
    name = logo.split('.')[0]
    ext = logo.split('.')[1]
    install_data(
        'assets' / 'icon' / logo,
        install_dir: datadir / 'icons' / 'hicolor' / name / 'apps',
        rename: [app_name + '.' + ext],
    )
endforeach

if meson.is_cross_build() and host_machine.system() == 'android'

    library(
        'oopetris',
        src_files,
        include_directories: inc_dirs,
        dependencies: deps,
        c_args: sdl2_mixer_defines,
        cpp_args: compile_args + sdl2_mixer_defines,
        override_options: {
            'warning_level': '3',
            'werror': true,
        },
    )
elif meson.is_cross_build() and host_machine.system() == 'switch'

    elf_file = build_target(
        'oopetris.elf',
        src_files,
        include_directories: inc_dirs,
        dependencies: deps,
        c_args: sdl2_mixer_defines,
        cpp_args: compile_args + sdl2_mixer_defines,
        override_options: {
            'warning_level': '3',
            'werror': true,
        },
        target_type: 'executable',
    )


    use_nacp = ['true', 'True', '1', true].contains(
        meson.get_external_property('USE_NACP'),
    )

    elf2nro = find_program('elf2nro')
    # executable input elf file, output nro file
    NROFLAGS = [elf2nro, 'oopetris.elf', 'oopetris.nro']
    NRO_DEPS = [elf_file]
    NACP_DEPS = NRO_DEPS

    fs = import('fs')

    if use_nacp
        nacptool = find_program('nacptool')
        NACPFLAGS = [nacptool, '--create']

        APP_NAME = meson.get_external_property('APP_NAME', 'oopetris')
        NACPFLAGS += APP_NAME


        APP_AUTHOR = meson.get_external_property('APP_AUTHOR', '')
        if APP_AUTHOR == ''
            error('If USE_NACP is set, you have to provie an APP_AUTHOR')
        endif
        NACPFLAGS += APP_AUTHOR

        APP_VERSION = meson.get_external_property('APP_VERSION', '')
        if APP_VERSION == ''
            error('If USE_NACP is set, you have to provie an APP_VERSION')
        endif
        NACPFLAGS += APP_VERSION

        NACPFLAGS += 'oopetris.nacp' # outfile


        APP_TITLEID = meson.get_external_property('APP_TITLEID', '')

        # optional
        if APP_TITLEID != ''
            NACPFLAGS += '--titleid=' + APP_TITLEID
        endif


        # nacptool --create name author version oopetris.nacp 
        # optional: --titleid=<titleID>
        nacp_file = custom_target(
            'oopetris.nacp',
            command: NACPFLAGS,
            output: ['oopetirs.nacp'],
            depends: NACP_DEPS,
        )
        NROFLAGS += '--nacp=oopetris.nacp'
        NRO_DEPS += nacp_file


    endif


    default_icon = meson.get_external_property('libnx') / 'default_icon.jpg'

    app_icon = meson.get_external_property('APP_ICON', default_icon)

    if not fs.exists(app_icon)
        error('APP_ICON should exist, but doesn\'t: \'' + app_icon + '\'')
    endif

    if not fs.is_absolute(app_icon)
        app_icon = meson.project_source_root() / app_icon
    endif


    NROFLAGS += '--icon=' + app_icon


    romfs_dir = meson.get_external_property('romfs_dir')

    if not fs.exists(romfs_dir)
        error('romfs_dir should exist, but doesn\'t: \'' + romfs_dir + '\'')
    endif

    if not fs.is_absolute(romfs_dir)
        romfs_dir = meson.project_source_root() / romfs_dir
    endif


    NROFLAGS += '--romfsdir=' + romfs_dir

    # elf2nro oopetris.elf oopetris.nro NROFLAGS
    custom_target(
        'oopetris.nro',
        command: NROFLAGS,
        depends: NRO_DEPS,
        output: ['oopetris.nro'],
        build_by_default: true,
    )


else
    executable(
        'oopetris',
        src_files,
        include_directories: inc_dirs,
        dependencies: deps,
        c_args: sdl2_mixer_defines,
        cpp_args: compile_args + sdl2_mixer_defines,
        override_options: {
            'warning_level': '3',
            'werror': true,
        },
        install: true,
    )

    executable(
        'oopetris_recording_utility',
        recording_src_files,
        dependencies: recording_dependencies,
        include_directories: recording_inc_dirs,
        cpp_args: [compile_args, '-D_NO_SDL', '-D_NO_SPDLOG', '-D_OOPETRIS_RECORDING_UTILITY'],
        override_options: {
            'warning_level': '3',
            'werror': true,
        },
        install: true,
    )



endif
