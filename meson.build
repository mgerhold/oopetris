project(
    'oopetris',
    'c',
    'cpp',
    default_options: [
        'buildtype=release',
        'optimization=3',
        'strip=true',
        'cpp_std=c++20',
        'b_ndebug=if-release',
    ],
    version: '1.0.0-alpha',
)

if meson.version().version_compare('<0.60.0')
    error(
        'meson version must be higher than 0.60.0 to support some used features',
    )
endif

compile_args = []


cpp = meson.get_compiler('cpp')
if cpp.get_id() == 'msvc'
    add_project_arguments('/std:c++latest', language: 'cpp')
elif cpp.get_id() == 'gcc'
    add_project_arguments('-std=c++23', '-Wold-style-cast', language: 'cpp')
elif cpp.get_id() == 'clang'
    add_project_arguments('-std=c++2b', language: 'cpp')
elif cpp.get_id() == 'emscripten'
    add_project_arguments('-std=c++2b', language: 'cpp')
else
    add_project_arguments('-std=c++20', language: 'cpp')
endif

deps = []
inc_dirs = []

native = true
if meson.is_cross_build()
    native = false
endif


sdl2_dep = dependency(
    'sdl2',
    allow_fallback: false,
    required: false,
    native: native,
)

if sdl2_dep.found()
    deps += sdl2_dep
else
    deps += dependency(
        'sdl2',
        required: true,
        fallback: 'sdl2',
        native: native,
        default_options: ['test=false'],
    )
    deps += dependency(
        'sdl2main',
        required: true,
        fallback: 'sdl2',
        native: native,
    )
endif

deps += dependency('sdl2_ttf', 'SDL2_ttf', required: true, native: native)


# a dirty thing atm, until mpg123 is ported to meson (maybe never...)
mpg123_dep = dependency('mgp123', required: false, native: native)
sdl2_mixer_flags = ['flac=enabled']
sdl2_mixer_defines = ['-DAUDIO_WITH_FLAC_SUPPORT']
if mpg123_dep.found()
    sdl2_mixer_flags += 'mpg123=enabled'
    sdl2_mixer_defines += '-DAUDIO_WITH_MP3_SUPPORT'
else
    cpp = meson.get_compiler('cpp')
    mpg123_dep = cpp.find_library('mpg123', required: false)
    if mpg123_dep.found()
        sdl2_mixer_flags += 'mpg123=enabled'
        sdl2_mixer_defines += '-DAUDIO_WITH_MP3_SUPPORT'

        meson.override_dependency(
            'mpg123',
            mpg123_dep,
            native: native,
        )
    endif
endif

deps += dependency(
    'sdl2_mixer',
    'SDL2_mixer',
    required: true,
    native: native,
    default_options: sdl2_mixer_flags,
)

if meson.is_cross_build() and host_machine.system() == 'emscripten'

    # this is done here, since it's only needed for one file!
    cpp = meson.get_compiler('cpp')
    deps += cpp.find_library('embind', required: true)
    deps += cpp.find_library('openal', required: true)
endif

## HEADER ONLY LIBS
deps += dependency(
    'spdlog',
    required: true,
    native: native,
    version: '>=1.1.11',
    default_options: ['tests=false'],
)
deps += dependency('nlohmann_json', required: true, native: native)
deps += dependency(
    'tl-optional',
    required: true,
    fallback: ['tl-optional'],
    native: native,
)
deps += dependency(
    'tl-expected',
    required: true,
    fallback: ['tl-expected'],
    native: native,
)
deps += dependency(
    'magic_enum',
    required: true,
    fallback: ['magic_enum'],
    native: native,
)
deps += dependency(
    'argparse',
    required: true,
    fallback: ['argparse'],
    native: native,
)


src_files = []
link_args = []
link_whole = []


subdir('src')

## install assets
install_subdir('assets', install_dir: 'share/oopetris')
if meson.is_cross_build() and host_machine.system() == 'android'

    library(
        'oopetris',
        src_files,
        include_directories: inc_dirs,
        dependencies: deps,
        c_args: sdl2_mixer_defines,
        cpp_args: compile_args + sdl2_mixer_defines,
        override_options: ['warning_level=3', 'werror=true'],
        native: native,
    )
elif meson.is_cross_build() and host_machine.system() == 'emscripten'

    executable(
        'oopetris',
        src_files,
        include_directories: inc_dirs,
        dependencies: deps,
        c_args: sdl2_mixer_defines,
        link_whole: link_whole,
        link_args: ['--preload-file', '../assets@/assets/'],
        cpp_args: compile_args + sdl2_mixer_defines,
        override_options: ['warning_level=3', 'werror=true'],
        native: native,
    )

else
    executable(
        'oopetris',
        src_files,
        include_directories: inc_dirs,
        dependencies: deps,
        c_args: sdl2_mixer_defines,
        cpp_args: compile_args + sdl2_mixer_defines,
        override_options: [
            'warning_level=3',
            'werror=true',
        ],
        native: native,
    )
endif
