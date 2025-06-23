cmake_minimum_required(VERSION 3.13.1)

set(BOARD_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../..")
set(DTS_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../..")
set(BOARD qorvo_dwm3001cdk)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(sit_ble_example)

# Set optimization level to O1 for better debugging
zephyr_library_compile_options(-O1)
target_compile_options(app PRIVATE
    -fdiagnostics-color=always
    -O1
)

target_sources(app PRIVATE src/main.c)
target_include_directories(app PRIVATE ../../drivers/platform)
