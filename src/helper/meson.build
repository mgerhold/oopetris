core_src_files += files(
    'bool_wrapper.hpp',
    'color.cpp',
    'color.hpp',
    'color_literals.hpp',
    'command_line_arguments.hpp',
    'constants.hpp',
    'date.cpp',
    'date.hpp',
    'errors.cpp',
    'errors.hpp',
    'expected.hpp',
    'git_helper.hpp',
    'magic_enum_wrapper.hpp',
    'optional.hpp',
    'parse_json.cpp',
    'parse_json.hpp',
    'random.cpp',
    'random.hpp',
    'sleep.cpp',
    'sleep.hpp',
    'static_string.hpp',
    'timer.hpp',
    'types.hpp',
    'utils.hpp',
)

graphics_src_files += files(
    'clock_source.cpp',
    'clock_source.hpp',
    'console_helpers.cpp',
    'console_helpers.hpp',
    'graphic_utils.cpp',
    'graphic_utils.hpp',
    'message_box.cpp',
    'message_box.hpp',
    'music_utils.hpp',
)

if have_file_dialogs
    graphics_src_files += files('nfd.cpp', 'nfd_include.hpp')
endif

git = find_program('git', required: false)

if git.found()

    vcs_dep = vcs_tag(
        command: ['git', 'describe', '--tags', '--always', '--abbrev=12'],
        input: 'git_version.hpp.in',
        output: 'git_version.hpp',
        replace_string: '@GIT_VERSION@',
    )

    graphics_src_files += vcs_dep
    graphics_lib += {
        'compile_args': [
            graphics_lib.get('compile_args'),
            '-D_HAS_GIT_COMMIT_INFORMATION',
        ],
    }
endif

