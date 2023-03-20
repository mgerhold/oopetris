project(
    'oopetris',
    'cpp',
    default_options: [
        'buildtype=release',
        'optimization=3',
        'cpp_std=c++14',
        'warning_level=0',
        'werror=false',
        'strip=true',
        'b_ndebug=if-release',
    ],
    version: '1.0.0-alpha',
)

deps = []

sdl2_dep = dependency('sdl2', required: false)
if not sdl2_dep.found()
    deps += dependency(
        'sdl2',
        required: true,
        fallback: 'sdl2',
    )
else
    deps += sdl2_dep
endif

inc_dirs = []
src_files = []

subdir('src')

executable(
    'oopetris',
    src_files,
    include_directories: inc_dirs,
    dependencies: deps,
)