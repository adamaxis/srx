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
    target_link_libraries(${test_target}
        PRIVATE shadowrun_ghidra_conversion_modules
    )
    add_test(NAME "shadowrun_ghidra_${test_name}"
             COMMAND ${test_target})
endforeach()
