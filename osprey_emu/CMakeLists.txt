cmake_minimum_required(VERSION 3.14)
project(osprey_emu LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find SDL2
find_package(SDL2 REQUIRED)

# Find simavr - try pkg-config first, fall back to find_library/find_path
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    set(ENV{PKG_CONFIG_PATH} "/usr/lib/pkgconfig:/usr/lib64/pkgconfig:/usr/share/pkgconfig:$ENV{PKG_CONFIG_PATH}")
    pkg_check_modules(SIMAVR QUIET simavr)
endif()

if(NOT SIMAVR_FOUND)
    find_path(SIMAVR_INCLUDE_DIRS simavr/sim_avr.h
        HINTS /usr/include /usr/local/include
    )
    find_library(SIMAVR_LIBRARIES simavr
        HINTS /usr/lib /usr/local/lib
    )
    if(SIMAVR_INCLUDE_DIRS AND SIMAVR_LIBRARIES)
        find_library(LIBELF_LIBRARIES elf
            HINTS /usr/lib /usr/local/lib
        )
        if(LIBELF_LIBRARIES)
            list(APPEND SIMAVR_LIBRARIES ${LIBELF_LIBRARIES})
        endif()
        set(SIMAVR_FOUND TRUE)
    else()
        message(FATAL_ERROR "simavr not found")
    endif()
endif()

# Source files
set(SOURCES
    main.cpp
    src/CPU6809.cpp
    src/CPUAtmega16.cpp
)

# Executable
add_executable(osprey_emu ${SOURCES})

# Include directories
target_include_directories(osprey_emu PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${SIMAVR_INCLUDE_DIRS}
)

# Compiler flags
target_compile_options(osprey_emu PRIVATE -Wall)

# Link libraries
target_link_libraries(osprey_emu PRIVATE
    SDL2::SDL2
    ${SIMAVR_LIBRARIES}
)
