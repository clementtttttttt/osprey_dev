project('osprey_emu', 'cpp',
    version: '0.1',
    default_options: ['cpp_std=c++14', 'warning_level=2'])

cpp = meson.get_compiler('cpp')

# SDL2 - use find_library with 64-bit path first
sdl2_inc = include_directories('/usr/include/SDL2')
sdl2_lib = cpp.find_library('SDL2', dirs: ['/usr/lib64'], required: true)
sdl2_dep = declare_dependency(
    include_directories: sdl2_inc,
    dependencies: sdl2_lib,
)

# simavr
simavr_dep = dependency('simavr', required: false)
if not simavr_dep.found()
    simavr_lib = cpp.find_library('simavr', required: true)
    simavr_inc = include_directories('/usr/include/simavr')
    simavr_dep = declare_dependency(
        include_directories: simavr_inc,
        dependencies: simavr_lib,
    )
endif

# libelf
elf_lib = cpp.find_library('elf', required: true)

# Project include directory
inc_dir = include_directories('include')

executable('osprey_emu',
    files('main.cpp', 'src/CPU6809.cpp', 'src/VIA6522.cpp'),
    include_directories: [inc_dir],
    dependencies: [sdl2_dep, simavr_dep, elf_lib],
    cpp_args: ['-Wall'],
)
