project(
    'chif',
    'c', 'cpp',
    default_options: [
        'warning_level=3',
        'werror=false',
        'cpp_std=c++23'
    ],
    license: 'Apache-2.0',
    version: '1.0',
)

deps = [dependency('phosphor-dbus-interfaces'),
        dependency('phosphor-logging'),
        dependency('sdbusplus'),
        dependency('systemd'),
        dependency('zlib'),
]

executable('chif',
        'src/main.cpp',
        'src/smif.cpp',
        'src/dbus_send.cpp',
        'src/sysrom.cpp',
        'src/triton.cpp',
        'src/ev.cpp',
        'src/misc.cpp',
        'src/cfg_smbios.cpp',
        'src/smbios.cpp',
        'src/db_smbios.cpp',
        'src/romchfservice.cpp',
        'src/strutil.cpp',
        'src/uuid_gen.cpp',
        'src/uefi_util.cpp',
        'src/platdef_api.cpp',
        'src/i2c_mapping.cpp',
        'src/i2c_topology.cpp',
        'src/DataExtract.c',
        'src/generic_decoder.cpp',
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

