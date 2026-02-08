zephyr_library()

zephyr_library_sources(
    src/telemetry.c
    src/command_parser.c
    src/command_processor.c
    src/registry.c
    
)

zephyr_library_include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
