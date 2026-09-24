cmake_minimum_required(VERSION 3.20)
project(shadowrun_ghidra_conversion LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(shadowrun_ghidra_conversion STATIC
    src/title_loop_conversion.cpp
)
target_include_directories(shadowrun_ghidra_conversion PUBLIC include)

add_executable(shadowrun_ghidra_conversion_test
    tests/title_loop_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_conversion_test
    PRIVATE shadowrun_ghidra_conversion)

enable_testing()
add_test(NAME shadowrun_ghidra_conversion
         COMMAND shadowrun_ghidra_conversion_test)

add_library(shadowrun_ghidra_compressed_data STATIC
    src/compressed_data_conversion.cpp
)
target_include_directories(shadowrun_ghidra_compressed_data PUBLIC include)

add_executable(shadowrun_ghidra_compressed_data_test
    tests/compressed_data_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_compressed_data_test
    PRIVATE shadowrun_ghidra_compressed_data)
add_test(NAME shadowrun_ghidra_compressed_data
         COMMAND shadowrun_ghidra_compressed_data_test)

add_library(shadowrun_ghidra_coordinate_data STATIC
    src/coordinate_data_conversion.cpp
)
target_include_directories(shadowrun_ghidra_coordinate_data PUBLIC include)

add_executable(shadowrun_ghidra_coordinate_data_test
    tests/coordinate_data_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_coordinate_data_test
    PRIVATE shadowrun_ghidra_coordinate_data)
add_test(NAME shadowrun_ghidra_coordinate_data
         COMMAND shadowrun_ghidra_coordinate_data_test)

add_library(shadowrun_ghidra_source_buffer STATIC
    src/source_buffer_conversion.cpp
)
target_include_directories(shadowrun_ghidra_source_buffer PUBLIC include)

add_executable(shadowrun_ghidra_source_buffer_test
    tests/source_buffer_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_source_buffer_test
    PRIVATE shadowrun_ghidra_source_buffer)
add_test(NAME shadowrun_ghidra_source_buffer
         COMMAND shadowrun_ghidra_source_buffer_test)

add_library(shadowrun_ghidra_packed_data STATIC
    src/packed_data_conversion.cpp
)
target_include_directories(shadowrun_ghidra_packed_data PUBLIC include)

add_executable(shadowrun_ghidra_packed_data_test
    tests/packed_data_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_packed_data_test
    PRIVATE shadowrun_ghidra_packed_data)
add_test(NAME shadowrun_ghidra_packed_data
         COMMAND shadowrun_ghidra_packed_data_test)

add_library(shadowrun_ghidra_field_coordinate STATIC
    src/field_coordinate_conversion.cpp
)
target_include_directories(shadowrun_ghidra_field_coordinate PUBLIC include)

add_executable(shadowrun_ghidra_field_coordinate_test
    tests/field_coordinate_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_field_coordinate_test
    PRIVATE shadowrun_ghidra_field_coordinate)
add_test(NAME shadowrun_ghidra_field_coordinate
         COMMAND shadowrun_ghidra_field_coordinate_test)

add_library(shadowrun_ghidra_entity_timing STATIC
    src/entity_timing_conversion.cpp
)
target_include_directories(shadowrun_ghidra_entity_timing PUBLIC include)

add_executable(shadowrun_ghidra_entity_timing_test
    tests/entity_timing_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_entity_timing_test
    PRIVATE shadowrun_ghidra_entity_timing)
add_test(NAME shadowrun_ghidra_entity_timing
         COMMAND shadowrun_ghidra_entity_timing_test)

add_library(shadowrun_ghidra_entity_projection STATIC
    src/entity_projection_conversion.cpp
)
target_include_directories(shadowrun_ghidra_entity_projection PUBLIC include)

add_executable(shadowrun_ghidra_entity_projection_test
    tests/entity_projection_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_entity_projection_test
    PRIVATE shadowrun_ghidra_entity_projection)
add_test(NAME shadowrun_ghidra_entity_projection
         COMMAND shadowrun_ghidra_entity_projection_test)

add_library(shadowrun_ghidra_entity_animation STATIC
    src/entity_animation_conversion.cpp
)
target_include_directories(shadowrun_ghidra_entity_animation PUBLIC include)

add_executable(shadowrun_ghidra_entity_animation_test
    tests/entity_animation_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_entity_animation_test
    PRIVATE shadowrun_ghidra_entity_animation)
add_test(NAME shadowrun_ghidra_entity_animation
         COMMAND shadowrun_ghidra_entity_animation_test)

add_library(shadowrun_ghidra_entity_coordinate STATIC
    src/entity_coordinate_conversion.cpp
)
target_include_directories(shadowrun_ghidra_entity_coordinate PUBLIC include)

add_executable(shadowrun_ghidra_entity_coordinate_test
    tests/entity_coordinate_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_entity_coordinate_test
    PRIVATE shadowrun_ghidra_entity_coordinate)
add_test(NAME shadowrun_ghidra_entity_coordinate
         COMMAND shadowrun_ghidra_entity_coordinate_test)

add_library(shadowrun_ghidra_text_line STATIC
    src/text_line_conversion.cpp
)
target_include_directories(shadowrun_ghidra_text_line PUBLIC include)

add_executable(shadowrun_ghidra_text_line_test
    tests/text_line_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_text_line_test
    PRIVATE shadowrun_ghidra_text_line)
add_test(NAME shadowrun_ghidra_text_line
         COMMAND shadowrun_ghidra_text_line_test)

add_library(shadowrun_ghidra_entity_position STATIC
    src/entity_position_conversion.cpp
)
target_include_directories(shadowrun_ghidra_entity_position PUBLIC include)

add_executable(shadowrun_ghidra_entity_position_test
    tests/entity_position_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_entity_position_test
    PRIVATE shadowrun_ghidra_entity_position)
add_test(NAME shadowrun_ghidra_entity_position
         COMMAND shadowrun_ghidra_entity_position_test)

add_library(shadowrun_ghidra_collision_probe STATIC
    src/collision_probe_conversion.cpp
)
target_include_directories(shadowrun_ghidra_collision_probe PUBLIC include)

add_executable(shadowrun_ghidra_collision_probe_test
    tests/collision_probe_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_collision_probe_test
    PRIVATE shadowrun_ghidra_collision_probe)
add_test(NAME shadowrun_ghidra_collision_probe
         COMMAND shadowrun_ghidra_collision_probe_test)

add_library(shadowrun_ghidra_entity_dirty STATIC
    src/entity_dirty_conversion.cpp
)
target_include_directories(shadowrun_ghidra_entity_dirty PUBLIC include)

add_executable(shadowrun_ghidra_entity_dirty_test
    tests/entity_dirty_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_entity_dirty_test
    PRIVATE shadowrun_ghidra_entity_dirty)
add_test(NAME shadowrun_ghidra_entity_dirty
         COMMAND shadowrun_ghidra_entity_dirty_test)

add_library(shadowrun_ghidra_entity_sort STATIC
    src/entity_sort_conversion.cpp
)
target_include_directories(shadowrun_ghidra_entity_sort PUBLIC include)

add_executable(shadowrun_ghidra_entity_sort_test
    tests/entity_sort_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_entity_sort_test
    PRIVATE shadowrun_ghidra_entity_sort)
add_test(NAME shadowrun_ghidra_entity_sort
         COMMAND shadowrun_ghidra_entity_sort_test)

add_library(shadowrun_ghidra_sprite_record STATIC
    src/sprite_record_conversion.cpp
)
target_include_directories(shadowrun_ghidra_sprite_record PUBLIC include)

add_executable(shadowrun_ghidra_sprite_record_test
    tests/sprite_record_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_sprite_record_test
    PRIVATE shadowrun_ghidra_sprite_record)
add_test(NAME shadowrun_ghidra_sprite_record
         COMMAND shadowrun_ghidra_sprite_record_test)
