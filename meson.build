project(
    '42sh',
    'c',
    'cpp',
    version: '0.1',
    license: 'MIT',
    default_options: [
        'c_std=c99',
        'warning_level=3',
        'default_library=static',

        # these options would not be suitable in a
        # production configuration, but development easier
        'b_sanitize=address',
        'buildtype=debug',
        'werror=true',
    ],
)


# print a summary of the build configuration before each build starts
summary({
    'optimization': get_option('optimization'),
    'debug': get_option('debug'),
    'b_sanitize': get_option('b_sanitize'),
    'warning_level': get_option('warning_level'),
    'werror': get_option('werror'),
})


# add some project-wide flags
cflags = ['-D_POSIX_C_SOURCE=200809L']
add_project_arguments(cflags, language: 'c')


# interpret the meson.build in this directory
all_sources = []
bin_sources = []
tests_sources= []
include_dirs = ['src']
subdir('src')



# find readline, which some of the modules below link with
cc = meson.get_compiler('c')
readline_dep = cc.find_library('readline', required: true)
asan_dep = cc.find_library('asan', required: true)

bin = executable(
    '42sh',
    bin_sources,
    all_sources,
    include_directories: include_dirs,
    install: true,
    dependencies: [readline_dep, asan_dep],
)

if get_option('testunit')

    subdir('tests')

    criterion = cc.find_library('criterion', required: true)

    unit_tests = executable(
        'testsuite',
        all_sources,
        tests_sources,
        include_directories: include_dirs,
        install: false,
        dependencies: [readline_dep, asan_dep, criterion],
    )

endif


run_tests = meson.source_root() / 'tests/run_tests'
custom_target(
    'check',
    command: [run_tests, bin] + get_option('check_args'),
    # give the testsuite exclusive access to the terminal, which enables
    # pretty colors and live testsuite results (meson buffers it all otherwise)
    console: true,
    # eventhough this rule output no file, we have to the output file of the rule is mandatory, but as
    # but it doesn't matter as the rule is always built
    output: 'check',
    build_always_stale: true,
    # if this option is enabled, tests will run for all builds
    build_by_default: get_option('force_check'),
)

if get_option('doc')
    subdir('doc')
endif
