src_files += files(
    'bool_wrapper.hpp',
    'clock_source.cpp',
    'clock_source.hpp',
    'color.cpp',
    'color.hpp',
    'color_literals.hpp',
    'command_line_arguments.hpp',
    'constants.hpp',
    'date.cpp',
    'date.hpp',
    'expected.hpp',
    'git_helper.hpp',
    'magic_enum_wrapper.hpp',
    'music_utils.hpp',
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
    'utils.cpp',
    'utils.hpp',
)


if have_file_dialogs
    src_files += files('nfd.cpp', 'nfd_include.hpp')
endif

git = find_program('git', required: false)

if git.found()

    vcs_dep = vcs_tag(
        command: ['git', 'describe', '--tags', '--always', '--abbrev=12'],
        input: 'git_version.hpp.in',
        output: 'git_version.hpp',
        replace_string: '@GIT_VERSION@',
    )

    src_files += vcs_dep
    compile_args += ['-D_HAS_GIT_COMMIT_INFORMATION']

endif

