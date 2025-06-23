cmake_minimum_required(VERSION 3.13.1)

set(BOARD_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../..")
set(DTS_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../..")
set(BOARD qorvo_dwm3001cdk)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(sit_ble_mesh)

# Add source files
target_sources(app PRIVATE src/main.c)

# Add include directories
target_include_directories(app PRIVATE
  ../../include
  ../../lib/sit_mesh
  ../../lib/sit_led
  ../../drivers/platform
)

# Link SIT libraries
target_link_libraries(app PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/sit_mesh/libsit_mesh.a
  ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/sit_led/libsit_led.a
)

# Configure build options
target_compile_options(app PRIVATE
  -fdiagnostics-color=always
  -O1
)

# Add debug configurations if needed
if(CONFIG_DEBUG)
    target_compile_options(app PRIVATE
    -g3
    -ggdb
    -fno-omit-frame-pointer
  )
endif()
