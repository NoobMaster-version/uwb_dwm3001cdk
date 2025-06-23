# SPDX-License-Identifier: Apache-2.0
cmake_minimum_required(VERSION 3.13.1)

set(BOARD_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../..")
set(DTS_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../..")
set(BOARD qorvo_dwm3001cdk)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(sit_ss_twr_initiator_example)

# the binary only library from Qorvo has problems with -Os or -O2
zephyr_library_compile_options(-O1)
target_compile_options(app PRIVATE
    -fdiagnostics-color=always
    -O1
)

target_sources(app PRIVATE
  src/main.c
)

# Add include directories for DW3000 and platform code
target_include_directories(app PRIVATE
  ../../include
  ../../drivers/platform
  ../../drivers/dw3000/inc
  ../../lib/sit
  ../../lib/sit_led
)

# Link DW3000 library
target_link_libraries(app PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/../../drivers/dw3000/lib/libdwt_uwb_driver-m4-sfp-6.0.14.a
)

# Add build options for debugging if needed
if(CONFIG_DEBUG)
    target_compile_options(app PRIVATE
    -g3
    -ggdb
    -fno-omit-frame-pointer
  )
endif()
