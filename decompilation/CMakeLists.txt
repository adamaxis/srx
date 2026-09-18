cmake_minimum_required(VERSION 3.20)
project(shadowrun_rom_analysis LANGUAGES CXX)

enable_testing()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(shadowrun_rom_analyzer
    src/rom_analyzer.cpp
)

add_library(shadowrun_port STATIC
    src/boot.cpp
    src/compression.cpp
    src/entity_projection.cpp
    src/entity_state.cpp
    src/graphics.cpp
    src/memory_bus.cpp
    src/native_input.cpp
    src/native_port.cpp
    src/object_mask.cpp
    src/object_list.cpp
    src/object_update.cpp
    src/palette.cpp
    src/resource_setup.cpp
    src/runtime.cpp
    src/session.cpp
    src/terrain.cpp
    src/z80_transport.cpp
)
target_include_directories(shadowrun_port PUBLIC include)

add_executable(shadowrun_boot_reconstruction_test
    tests/boot_reconstruction_test.cpp
)
target_link_libraries(shadowrun_boot_reconstruction_test PRIVATE shadowrun_port)
add_test(NAME shadowrun_boot_reconstruction COMMAND shadowrun_boot_reconstruction_test)

add_executable(shadowrun_native_port_test
    tests/native_port_test.cpp
)
target_link_libraries(shadowrun_native_port_test PRIVATE shadowrun_port)
target_compile_definitions(shadowrun_native_port_test PRIVATE
    SHADOWRUN_TEST_ROM="${CMAKE_CURRENT_SOURCE_DIR}/input/shadowrun_genesis_us.bin")
add_test(NAME shadowrun_native_port COMMAND shadowrun_native_port_test)

add_executable(shadowrun_graphics_test
    tests/graphics_test.cpp
)
target_link_libraries(shadowrun_graphics_test PRIVATE shadowrun_port)
target_compile_definitions(shadowrun_graphics_test PRIVATE
    SHADOWRUN_TEST_ROM="${CMAKE_CURRENT_SOURCE_DIR}/input/shadowrun_genesis_us.bin")
add_test(NAME shadowrun_graphics COMMAND shadowrun_graphics_test)

add_executable(shadowrun_menu_system_test
    tests/menu_system_test.cpp
)
target_link_libraries(shadowrun_menu_system_test PRIVATE shadowrun_port)
target_compile_definitions(shadowrun_menu_system_test PRIVATE
    SHADOWRUN_TEST_ROM="${CMAKE_CURRENT_SOURCE_DIR}/input/shadowrun_genesis_us.bin")
add_test(NAME shadowrun_menu_system COMMAND shadowrun_menu_system_test)

add_executable(shadowrun_graphics_extract
    src/graphics_extract.cpp
)
target_link_libraries(shadowrun_graphics_extract PRIVATE shadowrun_port)

if (WIN32)
    add_executable(shadowrun_native
        src/native_main.cpp
    )
    target_link_libraries(shadowrun_native PRIVATE shadowrun_port user32 gdi32)
endif()

if (MSVC)
    target_compile_options(shadowrun_rom_analyzer PRIVATE /W4 /permissive-)
    target_compile_options(shadowrun_port PRIVATE /W4 /permissive-)
else()
    target_compile_options(shadowrun_rom_analyzer PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(shadowrun_port PRIVATE -Wall -Wextra -Wpedantic)
endif()
