project(
    'chif',
    'cpp',
    default_options: [
        'warning_level=3',
        'werror=true',
        'cpp_std=c++20'
    ],
    license: 'Apache-2.0',
    version: '1.0',
)

# Wno-psabi reduces the number of "Note:" messages when cross-compiling some STL
# stuff for ARM. See https://stackoverflow.com/questions/48149323/strange-gcc-warning-when-compiling-qt-project
# Basically, gcc 6 and gcc 7 are not ABI compatible, but since the whole OpenBMC
# project uses the same compiler, we can safely ignmore these info notes.
add_project_arguments('-Wno-psabi', language: 'cpp')

deps = [dependency('phosphor-dbus-interfaces'),
        dependency('phosphor-logging'),
        dependency('sdbusplus'),
        dependency('systemd'),
]

executable('chif',
           'src/main.cpp',
	   'src/smif.cpp',
	   'src/sysrom.cpp',
	   'src/ev.cpp',
        implicit_include_directories: false,
        include_directories: ['include'],
        dependencies: deps,
        install: true,
        install_dir: get_option('bindir'))

systemd = dependency('systemd')
systemd_system_unit_dir = systemd.get_pkgconfig_variable(
    'systemdsystemunitdir',
    define_variable: ['prefix', get_option('prefix')])

configure_file(
    copy: true,
    input: 'service_files/xyz.openbmc_project.GxpChif.service',
    install: true,
    install_dir: systemd_system_unit_dir,
    output: 'xyz.openbmc_project.GxpChif.service'
)
