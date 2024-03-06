cmake_minimum_required(VERSION 3.12)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(pico-gps C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

# Initialize the SDK
pico_sdk_init()

add_executable(pico-gps
        pico-gps.c
        )

# pull in common dependencies
target_link_libraries(pico-gps pico_stdlib)

# create map/bin/hex file etc.
pico_add_extra_outputs(pico-gps)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(pico-gps)

