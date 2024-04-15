core_src_files = []
recordings_src_files = []
graphics_src_files = []

graphics_src_files += files(
    'application.cpp',
    'application.hpp',
)

main_files = files(
    'main.cpp',
)

subdir('game')
subdir('graphics')
subdir('helper')
subdir('manager')
subdir('platform')
subdir('recordings')
subdir('scenes')
subdir('thirdparty')
subdir('ui')
subdir('dynamic_loader')

if online_multiplayer_supported
    subdir('lobby')
endif


if have_discord_sdk
    subdir('discord')
endif


core_lib += {
    'src_files': [core_lib.get('src_files'), core_src_files],
    'inc_dirs': [core_lib.get('inc_dirs'), include_directories('.')],
}

recordings_lib += {
    'src_files': [recordings_lib.get('src_files'), recordings_src_files],
}

graphics_lib += {
    'src_files': [graphics_lib.get('src_files'), graphics_src_files],
}

core_lib += {
    'deps': [core_lib.get('deps'), global_deps],
}

liboopetris_core = library(
    'oopetris_core',
    core_lib.get('src_files'),
    include_directories: core_lib.get('inc_dirs'),
    dependencies: core_lib.get('deps'),
    cpp_args: core_lib.get('compile_args'),
    override_options: {
        'warning_level': '3',
        'werror': true,
    },
    version: meson.project_version(),
    install: true,
)

liboopetris_core_dep = declare_dependency(
    link_with: liboopetris_core,
    include_directories: core_lib.get('inc_dirs'),
    compile_args: core_lib.get('compile_args'),
    dependencies: core_lib.get('deps'),
    version: meson.project_version(),
)


recordings_lib += {
    'deps': [recordings_lib.get('deps'), liboopetris_core_dep, global_deps],
}

liboopetris_recordings = library(
    'oopetris_recordings',
    recordings_lib.get('src_files'),
    include_directories: recordings_lib.get('inc_dirs'),
    dependencies: recordings_lib.get('deps'),
    cpp_args: recordings_lib.get('compile_args'),
    override_options: {
        'warning_level': '3',
        'werror': true,
    },
    version: meson.project_version(),
    install: true,
)

liboopetris_recordings_dep = declare_dependency(
    link_with: liboopetris_recordings,
    include_directories: recordings_lib.get('inc_dirs'),
    compile_args: recordings_lib.get('compile_args'),
    dependencies: recordings_lib.get('deps'),
    version: meson.project_version(),
)

graphics_lib += {
    'deps': [graphics_lib.get('deps'), liboopetris_recordings_dep, global_deps],
}


liboopetris_graphics = library(
    'oopetris_graphics',
    graphics_lib.get('src_files'),
    include_directories: graphics_lib.get('inc_dirs'),
    dependencies: graphics_lib.get('deps'),
    cpp_args: graphics_lib.get('compile_args'),
    override_options: {
        'warning_level': '3',
        'werror': true,
    },
    version: meson.project_version(),
    install: true,
)

liboopetris_graphics_dep = declare_dependency(
    link_with: liboopetris_graphics,
    include_directories: graphics_lib.get('inc_dirs'),
    compile_args: graphics_lib.get('compile_args'),
    dependencies: graphics_lib.get('deps'),
    version: meson.project_version(),
)

# setting this to strings, so += {...} gets detected as an error, if it is done after that
core_lib = 'undefined'
recordings_lib = 'undefined'
graphics_lib = 'undefined'
