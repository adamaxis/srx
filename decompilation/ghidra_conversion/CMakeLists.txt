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

add_library(shadowrun_ghidra_sprite_expansion STATIC
    src/sprite_expansion_conversion.cpp
)
target_include_directories(shadowrun_ghidra_sprite_expansion PUBLIC include)

add_executable(shadowrun_ghidra_sprite_expansion_test
    tests/sprite_expansion_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_sprite_expansion_test
    PRIVATE shadowrun_ghidra_sprite_expansion)
add_test(NAME shadowrun_ghidra_sprite_expansion
         COMMAND shadowrun_ghidra_sprite_expansion_test)

add_library(shadowrun_ghidra_animation_table STATIC
    src/animation_table_conversion.cpp
)
target_include_directories(shadowrun_ghidra_animation_table PUBLIC include)

add_executable(shadowrun_ghidra_animation_table_test
    tests/animation_table_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_animation_table_test
    PRIVATE shadowrun_ghidra_animation_table)
add_test(NAME shadowrun_ghidra_animation_table
         COMMAND shadowrun_ghidra_animation_table_test)

add_library(shadowrun_ghidra_movement_probe STATIC
    src/movement_probe_conversion.cpp
)
target_include_directories(shadowrun_ghidra_movement_probe PUBLIC include)

add_executable(shadowrun_ghidra_movement_probe_test
    tests/movement_probe_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_movement_probe_test
    PRIVATE shadowrun_ghidra_movement_probe)
add_test(NAME shadowrun_ghidra_movement_probe
         COMMAND shadowrun_ghidra_movement_probe_test)

add_library(shadowrun_ghidra_collision_state STATIC
    src/collision_state_conversion.cpp
)
target_include_directories(shadowrun_ghidra_collision_state PUBLIC include)
target_link_libraries(shadowrun_ghidra_collision_state
    PRIVATE shadowrun_ghidra_entity_coordinate)

add_executable(shadowrun_ghidra_collision_state_test
    tests/collision_state_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_collision_state_test
    PRIVATE shadowrun_ghidra_collision_state)
add_test(NAME shadowrun_ghidra_collision_state
         COMMAND shadowrun_ghidra_collision_state_test)

add_library(shadowrun_ghidra_movement_state STATIC
    src/movement_state_conversion.cpp
)
target_include_directories(shadowrun_ghidra_movement_state PUBLIC include)
target_link_libraries(shadowrun_ghidra_movement_state
    PRIVATE shadowrun_ghidra_collision_state)

add_executable(shadowrun_ghidra_movement_state_test
    tests/movement_state_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_movement_state_test
    PRIVATE shadowrun_ghidra_movement_state)
add_test(NAME shadowrun_ghidra_movement_state
         COMMAND shadowrun_ghidra_movement_state_test)

add_library(shadowrun_ghidra_heading_animation STATIC
    src/heading_animation_conversion.cpp
)
target_include_directories(shadowrun_ghidra_heading_animation PUBLIC include)
target_link_libraries(shadowrun_ghidra_heading_animation
    PRIVATE shadowrun_ghidra_movement_state)

add_executable(shadowrun_ghidra_heading_animation_test
    tests/heading_animation_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_heading_animation_test
    PRIVATE shadowrun_ghidra_heading_animation)
add_test(NAME shadowrun_ghidra_heading_animation
         COMMAND shadowrun_ghidra_heading_animation_test)

add_library(shadowrun_ghidra_animation_reset STATIC
    src/animation_reset_conversion.cpp
)
target_include_directories(shadowrun_ghidra_animation_reset PUBLIC include)

add_executable(shadowrun_ghidra_animation_reset_test
    tests/animation_reset_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_animation_reset_test
    PRIVATE shadowrun_ghidra_animation_reset)
add_test(NAME shadowrun_ghidra_animation_reset
         COMMAND shadowrun_ghidra_animation_reset_test)

add_library(shadowrun_ghidra_entity_delay STATIC
    src/entity_delay_conversion.cpp
)
target_include_directories(shadowrun_ghidra_entity_delay PUBLIC include)

add_executable(shadowrun_ghidra_entity_delay_test
    tests/entity_delay_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_entity_delay_test
    PRIVATE shadowrun_ghidra_entity_delay)
add_test(NAME shadowrun_ghidra_entity_delay
         COMMAND shadowrun_ghidra_entity_delay_test)

add_library(shadowrun_ghidra_behavior_animation STATIC
    src/behavior_animation_conversion.cpp
)
target_include_directories(shadowrun_ghidra_behavior_animation PUBLIC include)
target_link_libraries(shadowrun_ghidra_behavior_animation
    PRIVATE shadowrun_ghidra_animation_reset shadowrun_ghidra_entity_delay)

add_executable(shadowrun_ghidra_behavior_animation_test
    tests/behavior_animation_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_behavior_animation_test
    PRIVATE shadowrun_ghidra_behavior_animation)
add_test(NAME shadowrun_ghidra_behavior_animation
         COMMAND shadowrun_ghidra_behavior_animation_test)

add_library(shadowrun_ghidra_entity_transition STATIC
    src/entity_transition_conversion.cpp
)
target_include_directories(shadowrun_ghidra_entity_transition PUBLIC include)
target_link_libraries(shadowrun_ghidra_entity_transition
    PRIVATE shadowrun_ghidra_behavior_animation
            shadowrun_ghidra_entity_coordinate
            shadowrun_ghidra_entity_state_primitives
            shadowrun_ghidra_movement_metric)

add_executable(shadowrun_ghidra_entity_transition_test
    tests/entity_transition_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_entity_transition_test
    PRIVATE shadowrun_ghidra_entity_transition)
add_test(NAME shadowrun_ghidra_entity_transition
         COMMAND shadowrun_ghidra_entity_transition_test)

add_library(shadowrun_ghidra_movement_fallback STATIC
    src/movement_fallback_conversion.cpp
)
target_include_directories(shadowrun_ghidra_movement_fallback PUBLIC include)
target_link_libraries(shadowrun_ghidra_movement_fallback
    PRIVATE shadowrun_ghidra_movement_probe)

add_executable(shadowrun_ghidra_movement_fallback_test
    tests/movement_fallback_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_movement_fallback_test
    PRIVATE shadowrun_ghidra_movement_fallback)
add_test(NAME shadowrun_ghidra_movement_fallback
         COMMAND shadowrun_ghidra_movement_fallback_test)

add_library(shadowrun_ghidra_movement_route STATIC
    src/movement_route_conversion.cpp
)
target_include_directories(shadowrun_ghidra_movement_route PUBLIC include)
target_link_libraries(shadowrun_ghidra_movement_route
    PRIVATE shadowrun_ghidra_movement_probe)

add_executable(shadowrun_ghidra_movement_route_test
    tests/movement_route_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_movement_route_test
    PRIVATE shadowrun_ghidra_movement_route)
add_test(NAME shadowrun_ghidra_movement_route
         COMMAND shadowrun_ghidra_movement_route_test)

add_library(shadowrun_ghidra_movement_step STATIC
    src/movement_step_conversion.cpp
)
target_include_directories(shadowrun_ghidra_movement_step PUBLIC include)
target_link_libraries(shadowrun_ghidra_movement_step
    PRIVATE shadowrun_ghidra_movement_probe)

add_executable(shadowrun_ghidra_movement_step_test
    tests/movement_step_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_movement_step_test
    PRIVATE shadowrun_ghidra_movement_step)
add_test(NAME shadowrun_ghidra_movement_step
         COMMAND shadowrun_ghidra_movement_step_test)

add_library(shadowrun_ghidra_terrain_probe STATIC
    src/terrain_probe_conversion.cpp
)
target_include_directories(shadowrun_ghidra_terrain_probe PUBLIC include)

add_executable(shadowrun_ghidra_terrain_probe_test
    tests/terrain_probe_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_terrain_probe_test
    PRIVATE shadowrun_ghidra_terrain_probe)
add_test(NAME shadowrun_ghidra_terrain_probe
         COMMAND shadowrun_ghidra_terrain_probe_test)

add_library(shadowrun_ghidra_terrain_map STATIC
    src/terrain_map_conversion.cpp
)
target_include_directories(shadowrun_ghidra_terrain_map PUBLIC include)

add_executable(shadowrun_ghidra_terrain_map_test
    tests/terrain_map_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_terrain_map_test
    PRIVATE shadowrun_ghidra_terrain_map)
add_test(NAME shadowrun_ghidra_terrain_map
         COMMAND shadowrun_ghidra_terrain_map_test)

add_library(shadowrun_ghidra_selection STATIC
    src/selection_conversion.cpp
)
target_include_directories(shadowrun_ghidra_selection PUBLIC include)

add_executable(shadowrun_ghidra_selection_test
    tests/selection_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_selection_test
    PRIVATE shadowrun_ghidra_selection)
add_test(NAME shadowrun_ghidra_selection
         COMMAND shadowrun_ghidra_selection_test)

add_library(shadowrun_ghidra_counter STATIC
    src/counter_conversion.cpp
)
target_include_directories(shadowrun_ghidra_counter PUBLIC include)

add_executable(shadowrun_ghidra_counter_test
    tests/counter_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_counter_test
    PRIVATE shadowrun_ghidra_counter)
add_test(NAME shadowrun_ghidra_counter
         COMMAND shadowrun_ghidra_counter_test)

add_library(shadowrun_ghidra_movement_metric STATIC
    src/movement_metric_conversion.cpp
)
target_include_directories(shadowrun_ghidra_movement_metric PUBLIC include)

add_executable(shadowrun_ghidra_movement_metric_test
    tests/movement_metric_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_movement_metric_test
    PRIVATE shadowrun_ghidra_movement_metric)
add_test(NAME shadowrun_ghidra_movement_metric
         COMMAND shadowrun_ghidra_movement_metric_test)

add_library(shadowrun_ghidra_movement_direction STATIC
    src/movement_direction_conversion.cpp
)
target_include_directories(shadowrun_ghidra_movement_direction PUBLIC include)

add_executable(shadowrun_ghidra_movement_direction_test
    tests/movement_direction_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_movement_direction_test
    PRIVATE shadowrun_ghidra_movement_direction)
add_test(NAME shadowrun_ghidra_movement_direction
         COMMAND shadowrun_ghidra_movement_direction_test)

add_library(shadowrun_ghidra_controller_wait STATIC
    src/controller_wait_conversion.cpp
)
target_include_directories(shadowrun_ghidra_controller_wait PUBLIC include)

add_executable(shadowrun_ghidra_controller_wait_test
    tests/controller_wait_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_controller_wait_test
    PRIVATE shadowrun_ghidra_controller_wait)
add_test(NAME shadowrun_ghidra_controller_wait
         COMMAND shadowrun_ghidra_controller_wait_test)

add_library(shadowrun_ghidra_entity_state_primitives STATIC
    src/entity_state_primitives_conversion.cpp
)
target_include_directories(shadowrun_ghidra_entity_state_primitives PUBLIC include)

add_executable(shadowrun_ghidra_entity_state_primitives_test
    tests/entity_state_primitives_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_entity_state_primitives_test
    PRIVATE shadowrun_ghidra_entity_state_primitives)
add_test(NAME shadowrun_ghidra_entity_state_primitives
         COMMAND shadowrun_ghidra_entity_state_primitives_test)

add_library(shadowrun_ghidra_entity_route_primitives STATIC
    src/entity_route_primitives_conversion.cpp
)
target_include_directories(shadowrun_ghidra_entity_route_primitives PUBLIC include)
target_link_libraries(shadowrun_ghidra_entity_route_primitives
    PRIVATE shadowrun_ghidra_movement_direction
            shadowrun_ghidra_movement_metric)

add_executable(shadowrun_ghidra_entity_route_primitives_test
    tests/entity_route_primitives_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_entity_route_primitives_test
    PRIVATE shadowrun_ghidra_entity_route_primitives)
add_test(NAME shadowrun_ghidra_entity_route_primitives
         COMMAND shadowrun_ghidra_entity_route_primitives_test)

add_library(shadowrun_ghidra_entity_target_motion STATIC
    src/entity_target_motion_conversion.cpp
)
target_include_directories(shadowrun_ghidra_entity_target_motion PUBLIC include)
target_link_libraries(shadowrun_ghidra_entity_target_motion
    PRIVATE shadowrun_ghidra_entity_route_primitives
            shadowrun_ghidra_entity_state_primitives
            shadowrun_ghidra_movement_metric)

add_executable(shadowrun_ghidra_entity_target_motion_test
    tests/entity_target_motion_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_entity_target_motion_test
    PRIVATE shadowrun_ghidra_entity_target_motion)
add_test(NAME shadowrun_ghidra_entity_target_motion
         COMMAND shadowrun_ghidra_entity_target_motion_test)

add_library(shadowrun_ghidra_world_effect STATIC
    src/world_effect_conversion.cpp
)
target_include_directories(shadowrun_ghidra_world_effect PUBLIC include)

add_executable(shadowrun_ghidra_world_effect_test
    tests/world_effect_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_world_effect_test
    PRIVATE shadowrun_ghidra_world_effect)
add_test(NAME shadowrun_ghidra_world_effect
         COMMAND shadowrun_ghidra_world_effect_test)
