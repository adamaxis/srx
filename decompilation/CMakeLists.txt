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
    src/memory_bus.cpp
    src/object_mask.cpp
    src/object_list.cpp
    src/object_update.cpp
    src/palette.cpp
    src/resource_setup.cpp
    src/runtime.cpp
    src/session.cpp
    src/z80_transport.cpp
)
target_include_directories(shadowrun_port PUBLIC include)

add_executable(shadowrun_boot_reconstruction_test
    tests/boot_reconstruction_test.cpp
)
target_link_libraries(shadowrun_boot_reconstruction_test PRIVATE shadowrun_port)
add_test(NAME shadowrun_boot_reconstruction COMMAND shadowrun_boot_reconstruction_test)

if (MSVC)
    target_compile_options(shadowrun_rom_analyzer PRIVATE /W4 /permissive-)
    target_compile_options(shadowrun_port PRIVATE /W4 /permissive-)
else()
    target_compile_options(shadowrun_rom_analyzer PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(shadowrun_port PRIVATE -Wall -Wextra -Wpedantic)
endif()
