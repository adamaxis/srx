cmake_minimum_required(VERSION 3.20)
project(shadowrun_ghidra_conversion LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()

# This is an offline source-understanding library, not a game-runtime
# dependency. The leaf contracts remain traceable, but they are built through
# the reviewed behavioral modules in conversion_modules.cmake rather than as
# one undifferentiated export-shaped target.
include(conversion_modules.cmake)

# This library is the only composition layer for the standalone conversion
# workbench. It links the reviewed behavioral modules, but it remains outside
# the native-port CMake graph and does not own game runtime state.
add_library(shadowrun_ghidra_conversion_workbench STATIC
    src/conversion_workbench.cpp
)
target_include_directories(shadowrun_ghidra_conversion_workbench PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(shadowrun_ghidra_conversion_workbench
    PUBLIC shadowrun_ghidra_conversion_modules)

add_executable(shadowrun_ghidra_conversion_runner
    tools/conversion_runner_main.cpp
)
target_link_libraries(shadowrun_ghidra_conversion_runner
    PRIVATE shadowrun_ghidra_conversion_workbench)
add_test(NAME shadowrun_ghidra_conversion_runner
         COMMAND shadowrun_ghidra_conversion_runner
                 --assets "${CMAKE_CURRENT_SOURCE_DIR}/../assets")

# Keep each source-trace contract as a separately runnable test so a failing
# ROM behavior remains easy to identify. They all link the one domain-grouped
# conversion library; no test target is part of the native game.
file(GLOB_RECURSE SHADOWRUN_CONVERSION_TEST_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/*_test.cpp"
)

foreach(test_source IN LISTS SHADOWRUN_CONVERSION_TEST_SOURCES)
    get_filename_component(test_stem "${test_source}" NAME_WE)
    string(REGEX REPLACE "_conversion_test$" "" test_name "${test_stem}")

    # Preserve the historical target/CTest name for the original title-loop
    # harness, whose first target predates the normal naming convention.
    if(test_name STREQUAL "title_loop")
        set(test_name "conversion")
    endif()

    set(test_target "shadowrun_ghidra_${test_name}_test")
    add_executable(${test_target} "${test_source}")
    if(test_name STREQUAL "conversion_workbench")
        target_link_libraries(${test_target}
            PRIVATE shadowrun_ghidra_conversion_workbench)
    else()
        target_link_libraries(${test_target}
            PRIVATE shadowrun_ghidra_conversion_modules)
    endif()
    add_test(NAME "shadowrun_ghidra_${test_name}"
             COMMAND ${test_target})
endforeach()
