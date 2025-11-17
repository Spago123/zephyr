# SPDX-License-Identifier: Apache-2.0
cmake_minimum_required(VERSION 3.20.0)

# Find Zephyr
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

# Declare project
project(telemetry_system)

target_include_directories(app PRIVATE
    .
    inc
    app
)

if(CONFIG_TELEMETRY_VARIABLE_LIST_FILE)
    target_compile_definitions(app PRIVATE 
        VARIABLE_LIST_FILE=${CONFIG_TELEMETRY_VARIABLE_LIST_FILE}
    )
endif()

# Add source files
target_sources(app PRIVATE 
    src/main.c
    src/uart_communication.c
    src/registry.c
    src/telemetry.c
)
