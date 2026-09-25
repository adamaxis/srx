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

add_library(shadowrun_ghidra_entity_table STATIC
    src/entity_table_conversion.cpp
)
target_include_directories(shadowrun_ghidra_entity_table PUBLIC include)

add_executable(shadowrun_ghidra_entity_table_test
    tests/entity_table_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_entity_table_test
    PRIVATE shadowrun_ghidra_entity_table)
add_test(NAME shadowrun_ghidra_entity_table
         COMMAND shadowrun_ghidra_entity_table_test)

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

add_library(shadowrun_ghidra_world_effect_mode STATIC
    src/world_effect_mode_conversion.cpp
)
target_include_directories(shadowrun_ghidra_world_effect_mode PUBLIC include)

add_executable(shadowrun_ghidra_world_effect_mode_test
    tests/world_effect_mode_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_world_effect_mode_test
    PRIVATE shadowrun_ghidra_world_effect_mode)
add_test(NAME shadowrun_ghidra_world_effect_mode
         COMMAND shadowrun_ghidra_world_effect_mode_test)

add_library(shadowrun_ghidra_world_effect_overlay STATIC
    src/world_effect_overlay_conversion.cpp
)
target_include_directories(shadowrun_ghidra_world_effect_overlay PUBLIC include)

add_executable(shadowrun_ghidra_world_effect_overlay_test
    tests/world_effect_overlay_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_world_effect_overlay_test
    PRIVATE shadowrun_ghidra_world_effect_overlay)
add_test(NAME shadowrun_ghidra_world_effect_overlay
         COMMAND shadowrun_ghidra_world_effect_overlay_test)

add_library(shadowrun_ghidra_world_effect_animation STATIC
    src/world_effect_animation_conversion.cpp
)
target_include_directories(shadowrun_ghidra_world_effect_animation PUBLIC include)

add_executable(shadowrun_ghidra_world_effect_animation_test
    tests/world_effect_animation_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_world_effect_animation_test
    PRIVATE shadowrun_ghidra_world_effect_animation)
add_test(NAME shadowrun_ghidra_world_effect_animation
         COMMAND shadowrun_ghidra_world_effect_animation_test)

add_library(shadowrun_ghidra_title_world_helpers STATIC
    src/title_world_helpers_conversion.cpp
)
target_include_directories(shadowrun_ghidra_title_world_helpers PUBLIC include)

add_executable(shadowrun_ghidra_title_world_helpers_test
    tests/title_world_helpers_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_title_world_helpers_test
    PRIVATE shadowrun_ghidra_title_world_helpers)
add_test(NAME shadowrun_ghidra_title_world_helpers
         COMMAND shadowrun_ghidra_title_world_helpers_test)

add_library(shadowrun_ghidra_title_profile_transition STATIC
    src/title_profile_transition_conversion.cpp
)
target_include_directories(shadowrun_ghidra_title_profile_transition PUBLIC include)

add_executable(shadowrun_ghidra_title_profile_transition_test
    tests/title_profile_transition_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_title_profile_transition_test
    PRIVATE shadowrun_ghidra_title_profile_transition)
add_test(NAME shadowrun_ghidra_title_profile_transition
         COMMAND shadowrun_ghidra_title_profile_transition_test)

add_library(shadowrun_ghidra_title_noise_effect STATIC
    src/title_noise_effect_conversion.cpp
)
target_include_directories(shadowrun_ghidra_title_noise_effect PUBLIC include)

add_executable(shadowrun_ghidra_title_noise_effect_test
    tests/title_noise_effect_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_title_noise_effect_test
    PRIVATE shadowrun_ghidra_title_noise_effect)
add_test(NAME shadowrun_ghidra_title_noise_effect
         COMMAND shadowrun_ghidra_title_noise_effect_test)

add_library(shadowrun_ghidra_title_profile_motion STATIC
    src/title_profile_motion_conversion.cpp
)
target_include_directories(shadowrun_ghidra_title_profile_motion PUBLIC include)

add_executable(shadowrun_ghidra_title_profile_motion_test
    tests/title_profile_motion_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_title_profile_motion_test
    PRIVATE shadowrun_ghidra_title_profile_motion)
add_test(NAME shadowrun_ghidra_title_profile_motion
         COMMAND shadowrun_ghidra_title_profile_motion_test)

add_library(shadowrun_ghidra_title_sweep_animation STATIC
    src/title_sweep_animation_conversion.cpp
)
target_include_directories(shadowrun_ghidra_title_sweep_animation PUBLIC include)

add_executable(shadowrun_ghidra_title_sweep_animation_test
    tests/title_sweep_animation_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_title_sweep_animation_test
    PRIVATE shadowrun_ghidra_title_sweep_animation)
add_test(NAME shadowrun_ghidra_title_sweep_animation
         COMMAND shadowrun_ghidra_title_sweep_animation_test)

add_library(shadowrun_ghidra_title_sprite_layers STATIC
    src/title_sprite_layers_conversion.cpp
)
target_include_directories(shadowrun_ghidra_title_sprite_layers PUBLIC include)

add_executable(shadowrun_ghidra_title_sprite_layers_test
    tests/title_sprite_layers_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_title_sprite_layers_test
    PRIVATE shadowrun_ghidra_title_sprite_layers)
add_test(NAME shadowrun_ghidra_title_sprite_layers
         COMMAND shadowrun_ghidra_title_sprite_layers_test)

add_library(shadowrun_ghidra_title_effect_motion STATIC
    src/title_effect_motion_conversion.cpp
)
target_include_directories(shadowrun_ghidra_title_effect_motion PUBLIC include)

add_executable(shadowrun_ghidra_title_effect_motion_test
    tests/title_effect_motion_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_title_effect_motion_test
    PRIVATE shadowrun_ghidra_title_effect_motion)
add_test(NAME shadowrun_ghidra_title_effect_motion
         COMMAND shadowrun_ghidra_title_effect_motion_test)

add_library(shadowrun_ghidra_world_effect_motion STATIC
    src/world_effect_motion_conversion.cpp
)
target_include_directories(shadowrun_ghidra_world_effect_motion PUBLIC include)

add_executable(shadowrun_ghidra_world_effect_motion_test
    tests/world_effect_motion_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_world_effect_motion_test
    PRIVATE shadowrun_ghidra_world_effect_motion)
add_test(NAME shadowrun_ghidra_world_effect_motion
         COMMAND shadowrun_ghidra_world_effect_motion_test)

add_library(shadowrun_ghidra_world_effect_record STATIC
    src/world_effect_record_conversion.cpp
)
target_include_directories(shadowrun_ghidra_world_effect_record PUBLIC include)

add_executable(shadowrun_ghidra_world_effect_record_test
    tests/world_effect_record_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_world_effect_record_test
    PRIVATE shadowrun_ghidra_world_effect_record)
add_test(NAME shadowrun_ghidra_world_effect_record
         COMMAND shadowrun_ghidra_world_effect_record_test)

add_library(shadowrun_ghidra_title_effect_sprites STATIC
    src/title_effect_sprites_conversion.cpp
)
target_include_directories(shadowrun_ghidra_title_effect_sprites PUBLIC include)

add_executable(shadowrun_ghidra_title_effect_sprites_test
    tests/title_effect_sprites_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_title_effect_sprites_test
    PRIVATE shadowrun_ghidra_title_effect_sprites)
add_test(NAME shadowrun_ghidra_title_effect_sprites
         COMMAND shadowrun_ghidra_title_effect_sprites_test)

add_library(shadowrun_ghidra_world_effect_activation STATIC
    src/world_effect_activation_conversion.cpp
)
target_include_directories(shadowrun_ghidra_world_effect_activation PUBLIC include)

add_executable(shadowrun_ghidra_world_effect_activation_test
    tests/world_effect_activation_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_world_effect_activation_test
    PRIVATE shadowrun_ghidra_world_effect_activation)
add_test(NAME shadowrun_ghidra_world_effect_activation
         COMMAND shadowrun_ghidra_world_effect_activation_test)

add_library(shadowrun_ghidra_persistent_world_effect STATIC
    src/persistent_world_effect_conversion.cpp
    src/world_effect_record_conversion.cpp
)
target_include_directories(shadowrun_ghidra_persistent_world_effect PUBLIC include)

add_executable(shadowrun_ghidra_persistent_world_effect_test
    tests/persistent_world_effect_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_persistent_world_effect_test
    PRIVATE shadowrun_ghidra_persistent_world_effect)
add_test(NAME shadowrun_ghidra_persistent_world_effect
         COMMAND shadowrun_ghidra_persistent_world_effect_test)

add_library(shadowrun_ghidra_world_effect_trigger STATIC
    src/world_effect_trigger_conversion.cpp
)
target_include_directories(shadowrun_ghidra_world_effect_trigger PUBLIC include)

add_executable(shadowrun_ghidra_world_effect_trigger_test
    tests/world_effect_trigger_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_world_effect_trigger_test
    PRIVATE shadowrun_ghidra_world_effect_trigger)
add_test(NAME shadowrun_ghidra_world_effect_trigger
         COMMAND shadowrun_ghidra_world_effect_trigger_test)

add_library(shadowrun_ghidra_entity_template STATIC
    src/entity_template_conversion.cpp
)
target_include_directories(shadowrun_ghidra_entity_template PUBLIC include)

add_executable(shadowrun_ghidra_entity_template_test
    tests/entity_template_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_entity_template_test
    PRIVATE shadowrun_ghidra_entity_template)
add_test(NAME shadowrun_ghidra_entity_template
         COMMAND shadowrun_ghidra_entity_template_test)

add_library(shadowrun_ghidra_runner_roster STATIC
    src/runner_roster_conversion.cpp
)
target_include_directories(shadowrun_ghidra_runner_roster PUBLIC include)

add_executable(shadowrun_ghidra_runner_roster_test
    tests/runner_roster_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_runner_roster_test
    PRIVATE shadowrun_ghidra_runner_roster)
add_test(NAME shadowrun_ghidra_runner_roster
         COMMAND shadowrun_ghidra_runner_roster_test)

add_library(shadowrun_ghidra_runner_record_access STATIC
    src/runner_record_access_conversion.cpp
)
target_include_directories(shadowrun_ghidra_runner_record_access PUBLIC include)
target_link_libraries(shadowrun_ghidra_runner_record_access
    PUBLIC shadowrun_ghidra_runner_roster)

add_executable(shadowrun_ghidra_runner_record_access_test
    tests/runner_record_access_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_runner_record_access_test
    PRIVATE shadowrun_ghidra_runner_record_access)
add_test(NAME shadowrun_ghidra_runner_record_access
         COMMAND shadowrun_ghidra_runner_record_access_test)

add_library(shadowrun_ghidra_runner_record_operations STATIC
    src/runner_record_operations_conversion.cpp
)
target_include_directories(shadowrun_ghidra_runner_record_operations PUBLIC include)
target_link_libraries(shadowrun_ghidra_runner_record_operations
    PUBLIC shadowrun_ghidra_runner_record_access)

add_executable(shadowrun_ghidra_runner_record_operations_test
    tests/runner_record_operations_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_runner_record_operations_test
    PRIVATE shadowrun_ghidra_runner_record_operations)
add_test(NAME shadowrun_ghidra_runner_record_operations
         COMMAND shadowrun_ghidra_runner_record_operations_test)

add_library(shadowrun_ghidra_save_validation STATIC
    src/save_validation_conversion.cpp
)
target_include_directories(shadowrun_ghidra_save_validation PUBLIC include)

add_executable(shadowrun_ghidra_save_validation_test
    tests/save_validation_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_save_validation_test
    PRIVATE shadowrun_ghidra_save_validation)
add_test(NAME shadowrun_ghidra_save_validation
         COMMAND shadowrun_ghidra_save_validation_test)

add_library(shadowrun_ghidra_save_slot STATIC
    src/save_slot_conversion.cpp
)
target_include_directories(shadowrun_ghidra_save_slot PUBLIC include)
target_link_libraries(shadowrun_ghidra_save_slot
    PRIVATE shadowrun_ghidra_save_validation)

add_executable(shadowrun_ghidra_save_slot_test
    tests/save_slot_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_save_slot_test
    PRIVATE shadowrun_ghidra_save_slot)
add_test(NAME shadowrun_ghidra_save_slot
         COMMAND shadowrun_ghidra_save_slot_test)

add_library(shadowrun_ghidra_save_restore STATIC
    src/save_restore_conversion.cpp
)
target_include_directories(shadowrun_ghidra_save_restore PUBLIC include)
target_link_libraries(shadowrun_ghidra_save_restore
    PRIVATE shadowrun_ghidra_save_slot)

add_executable(shadowrun_ghidra_save_restore_test
    tests/save_restore_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_save_restore_test
    PRIVATE shadowrun_ghidra_save_restore)
add_test(NAME shadowrun_ghidra_save_restore
         COMMAND shadowrun_ghidra_save_restore_test)

add_library(shadowrun_ghidra_save_field_restore STATIC
    src/save_field_restore_conversion.cpp
)
target_include_directories(shadowrun_ghidra_save_field_restore PUBLIC include)
target_link_libraries(shadowrun_ghidra_save_field_restore
    PRIVATE shadowrun_ghidra_runner_roster)

add_executable(shadowrun_ghidra_save_field_restore_test
    tests/save_field_restore_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_save_field_restore_test
    PRIVATE shadowrun_ghidra_save_field_restore)
add_test(NAME shadowrun_ghidra_save_field_restore
         COMMAND shadowrun_ghidra_save_field_restore_test)

add_library(shadowrun_ghidra_world_activity_routine STATIC
    src/world_activity_routine_conversion.cpp
)
target_include_directories(shadowrun_ghidra_world_activity_routine PUBLIC include)
target_link_libraries(shadowrun_ghidra_world_activity_routine
    PRIVATE shadowrun_ghidra_scene_profile)

add_executable(shadowrun_ghidra_world_activity_routine_test
    tests/world_activity_routine_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_world_activity_routine_test
    PRIVATE shadowrun_ghidra_world_activity_routine)
add_test(NAME shadowrun_ghidra_world_activity_routine
         COMMAND shadowrun_ghidra_world_activity_routine_test)

add_library(shadowrun_ghidra_world_scene_entity STATIC
    src/world_scene_entity_conversion.cpp
)
target_include_directories(shadowrun_ghidra_world_scene_entity PUBLIC include)
target_link_libraries(shadowrun_ghidra_world_scene_entity
    PRIVATE shadowrun_ghidra_menu_transfer_batch)

add_executable(shadowrun_ghidra_world_scene_entity_test
    tests/world_scene_entity_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_world_scene_entity_test
    PRIVATE shadowrun_ghidra_world_scene_entity)
add_test(NAME shadowrun_ghidra_world_scene_entity
         COMMAND shadowrun_ghidra_world_scene_entity_test)

add_library(shadowrun_ghidra_world_effect_selection STATIC
    src/world_effect_selection_conversion.cpp
)
target_include_directories(shadowrun_ghidra_world_effect_selection PUBLIC include)

add_executable(shadowrun_ghidra_world_effect_selection_test
    tests/world_effect_selection_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_world_effect_selection_test
    PRIVATE shadowrun_ghidra_world_effect_selection)
add_test(NAME shadowrun_ghidra_world_effect_selection
         COMMAND shadowrun_ghidra_world_effect_selection_test)

add_library(shadowrun_ghidra_random STATIC
    src/random_conversion.cpp
)
target_include_directories(shadowrun_ghidra_random PUBLIC include)

add_executable(shadowrun_ghidra_random_test
    tests/random_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_random_test
    PRIVATE shadowrun_ghidra_random)
add_test(NAME shadowrun_ghidra_random
         COMMAND shadowrun_ghidra_random_test)

add_library(shadowrun_ghidra_scene_resource STATIC
    src/scene_resource_conversion.cpp
)
target_include_directories(shadowrun_ghidra_scene_resource PUBLIC include)

add_executable(shadowrun_ghidra_scene_resource_test
    tests/scene_resource_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_scene_resource_test
    PRIVATE shadowrun_ghidra_scene_resource)
add_test(NAME shadowrun_ghidra_scene_resource
         COMMAND shadowrun_ghidra_scene_resource_test)

add_library(shadowrun_ghidra_title_setup STATIC
    src/title_setup_conversion.cpp
)
target_include_directories(shadowrun_ghidra_title_setup PUBLIC include)

add_executable(shadowrun_ghidra_title_setup_test
    tests/title_setup_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_title_setup_test
    PRIVATE shadowrun_ghidra_title_setup)
add_test(NAME shadowrun_ghidra_title_setup
         COMMAND shadowrun_ghidra_title_setup_test)

add_library(shadowrun_ghidra_timing STATIC
    src/timing_conversion.cpp
)
target_include_directories(shadowrun_ghidra_timing PUBLIC include)

add_executable(shadowrun_ghidra_timing_test
    tests/timing_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_timing_test
    PRIVATE shadowrun_ghidra_timing)
add_test(NAME shadowrun_ghidra_timing
         COMMAND shadowrun_ghidra_timing_test)

add_library(shadowrun_ghidra_controller_port STATIC
    src/controller_port_conversion.cpp
)
target_include_directories(shadowrun_ghidra_controller_port PUBLIC include)

add_executable(shadowrun_ghidra_controller_port_test
    tests/controller_port_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_controller_port_test
    PRIVATE shadowrun_ghidra_controller_port)
add_test(NAME shadowrun_ghidra_controller_port
         COMMAND shadowrun_ghidra_controller_port_test)

add_library(shadowrun_ghidra_field_hud STATIC
    src/field_hud_conversion.cpp
)
target_include_directories(shadowrun_ghidra_field_hud PUBLIC include)

add_executable(shadowrun_ghidra_field_hud_test
    tests/field_hud_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_field_hud_test
    PRIVATE shadowrun_ghidra_field_hud)
add_test(NAME shadowrun_ghidra_field_hud
         COMMAND shadowrun_ghidra_field_hud_test)

add_library(shadowrun_ghidra_audio_service STATIC
    src/audio_service_conversion.cpp
)
target_include_directories(shadowrun_ghidra_audio_service PUBLIC include)

add_executable(shadowrun_ghidra_audio_service_test
    tests/audio_service_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_audio_service_test
    PRIVATE shadowrun_ghidra_audio_service)
add_test(NAME shadowrun_ghidra_audio_service
         COMMAND shadowrun_ghidra_audio_service_test)

add_library(shadowrun_ghidra_scene_profile STATIC
    src/scene_profile_conversion.cpp
)
target_include_directories(shadowrun_ghidra_scene_profile PUBLIC include)

add_executable(shadowrun_ghidra_scene_profile_test
    tests/scene_profile_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_scene_profile_test
    PRIVATE shadowrun_ghidra_scene_profile)
add_test(NAME shadowrun_ghidra_scene_profile
         COMMAND shadowrun_ghidra_scene_profile_test)

add_library(shadowrun_ghidra_field_mode STATIC
    src/field_mode_conversion.cpp
)
target_include_directories(shadowrun_ghidra_field_mode PUBLIC include)

add_executable(shadowrun_ghidra_field_mode_test
    tests/field_mode_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_field_mode_test
    PRIVATE shadowrun_ghidra_field_mode)
add_test(NAME shadowrun_ghidra_field_mode
         COMMAND shadowrun_ghidra_field_mode_test)

add_library(shadowrun_ghidra_field_viewport STATIC
    src/field_viewport_conversion.cpp
)
target_include_directories(shadowrun_ghidra_field_viewport PUBLIC include)

add_executable(shadowrun_ghidra_field_viewport_test
    tests/field_viewport_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_field_viewport_test
    PRIVATE shadowrun_ghidra_field_viewport)
add_test(NAME shadowrun_ghidra_field_viewport
         COMMAND shadowrun_ghidra_field_viewport_test)

add_library(shadowrun_ghidra_entity_combat STATIC
    src/entity_combat_conversion.cpp
)
target_include_directories(shadowrun_ghidra_entity_combat PUBLIC include)

add_executable(shadowrun_ghidra_entity_combat_test
    tests/entity_combat_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_entity_combat_test
    PRIVATE shadowrun_ghidra_entity_combat)
add_test(NAME shadowrun_ghidra_entity_combat
         COMMAND shadowrun_ghidra_entity_combat_test)

add_library(shadowrun_ghidra_field_scene_entry STATIC
    src/field_scene_entry_conversion.cpp
)
target_include_directories(shadowrun_ghidra_field_scene_entry PUBLIC include)

add_executable(shadowrun_ghidra_field_scene_entry_test
    tests/field_scene_entry_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_field_scene_entry_test
    PRIVATE shadowrun_ghidra_field_scene_entry)
add_test(NAME shadowrun_ghidra_field_scene_entry
         COMMAND shadowrun_ghidra_field_scene_entry_test)

add_library(shadowrun_ghidra_mode_nine STATIC
    src/mode_nine_conversion.cpp
)
target_include_directories(shadowrun_ghidra_mode_nine PUBLIC include)

add_executable(shadowrun_ghidra_mode_nine_test
    tests/mode_nine_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_mode_nine_test
    PRIVATE shadowrun_ghidra_mode_nine)
add_test(NAME shadowrun_ghidra_mode_nine
         COMMAND shadowrun_ghidra_mode_nine_test)

add_library(shadowrun_ghidra_menu_asset_transfer STATIC
    src/menu_asset_transfer_conversion.cpp
)
target_include_directories(shadowrun_ghidra_menu_asset_transfer PUBLIC include)

add_executable(shadowrun_ghidra_menu_asset_transfer_test
    tests/menu_asset_transfer_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_menu_asset_transfer_test
    PRIVATE shadowrun_ghidra_menu_asset_transfer)
add_test(NAME shadowrun_ghidra_menu_asset_transfer
         COMMAND shadowrun_ghidra_menu_asset_transfer_test)

add_library(shadowrun_ghidra_word_transfer STATIC
    src/word_transfer_conversion.cpp
)
target_include_directories(shadowrun_ghidra_word_transfer PUBLIC include)

add_executable(shadowrun_ghidra_word_transfer_test
    tests/word_transfer_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_word_transfer_test
    PRIVATE shadowrun_ghidra_word_transfer)
add_test(NAME shadowrun_ghidra_word_transfer
         COMMAND shadowrun_ghidra_word_transfer_test)

add_library(shadowrun_ghidra_scene_record_reset STATIC
    src/scene_record_reset_conversion.cpp
)
target_include_directories(shadowrun_ghidra_scene_record_reset PUBLIC include)

add_executable(shadowrun_ghidra_scene_record_reset_test
    tests/scene_record_reset_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_scene_record_reset_test
    PRIVATE shadowrun_ghidra_scene_record_reset)
add_test(NAME shadowrun_ghidra_scene_record_reset
         COMMAND shadowrun_ghidra_scene_record_reset_test)

add_library(shadowrun_ghidra_startup_vdp STATIC
    src/startup_vdp_conversion.cpp
)
target_include_directories(shadowrun_ghidra_startup_vdp PUBLIC include)
target_link_libraries(shadowrun_ghidra_startup_vdp
    PRIVATE shadowrun_ghidra_word_transfer)

add_executable(shadowrun_ghidra_startup_vdp_test
    tests/startup_vdp_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_startup_vdp_test
    PRIVATE shadowrun_ghidra_startup_vdp)
add_test(NAME shadowrun_ghidra_startup_vdp
         COMMAND shadowrun_ghidra_startup_vdp_test)

add_library(shadowrun_ghidra_resource_refresh STATIC
    src/resource_refresh_conversion.cpp
)
target_include_directories(shadowrun_ghidra_resource_refresh PUBLIC include)

add_executable(shadowrun_ghidra_resource_refresh_test
    tests/resource_refresh_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_resource_refresh_test
    PRIVATE shadowrun_ghidra_resource_refresh)
add_test(NAME shadowrun_ghidra_resource_refresh
         COMMAND shadowrun_ghidra_resource_refresh_test)

add_library(shadowrun_ghidra_menu_state STATIC
    src/menu_state_conversion.cpp
)
target_include_directories(shadowrun_ghidra_menu_state PUBLIC include)

add_executable(shadowrun_ghidra_menu_state_test
    tests/menu_state_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_menu_state_test
    PRIVATE shadowrun_ghidra_menu_state)
add_test(NAME shadowrun_ghidra_menu_state
         COMMAND shadowrun_ghidra_menu_state_test)

add_library(shadowrun_ghidra_runner_status STATIC
    src/runner_status_conversion.cpp
)
target_include_directories(shadowrun_ghidra_runner_status PUBLIC include)

add_executable(shadowrun_ghidra_runner_status_test
    tests/runner_status_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_runner_status_test
    PRIVATE shadowrun_ghidra_runner_status)
add_test(NAME shadowrun_ghidra_runner_status
         COMMAND shadowrun_ghidra_runner_status_test)

add_library(shadowrun_ghidra_title_intro_overlay STATIC
    src/title_intro_overlay_conversion.cpp
)
target_include_directories(shadowrun_ghidra_title_intro_overlay PUBLIC include)

add_executable(shadowrun_ghidra_title_intro_overlay_test
    tests/title_intro_overlay_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_title_intro_overlay_test
    PRIVATE shadowrun_ghidra_title_intro_overlay)
add_test(NAME shadowrun_ghidra_title_intro_overlay
         COMMAND shadowrun_ghidra_title_intro_overlay_test)

add_library(shadowrun_ghidra_title_intro_sequence STATIC
    src/title_intro_sequence_conversion.cpp
)
target_include_directories(shadowrun_ghidra_title_intro_sequence PUBLIC include)
target_link_libraries(shadowrun_ghidra_title_intro_sequence
    PRIVATE shadowrun_ghidra_title_intro_overlay)

add_executable(shadowrun_ghidra_title_intro_sequence_test
    tests/title_intro_sequence_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_title_intro_sequence_test
    PRIVATE shadowrun_ghidra_title_intro_sequence)
add_test(NAME shadowrun_ghidra_title_intro_sequence
         COMMAND shadowrun_ghidra_title_intro_sequence_test)

add_library(shadowrun_ghidra_title_intro_persistent STATIC
    src/title_intro_persistent_conversion.cpp
)
target_include_directories(shadowrun_ghidra_title_intro_persistent PUBLIC include)

add_executable(shadowrun_ghidra_title_intro_persistent_test
    tests/title_intro_persistent_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_title_intro_persistent_test
    PRIVATE shadowrun_ghidra_title_intro_persistent)
add_test(NAME shadowrun_ghidra_title_intro_persistent
         COMMAND shadowrun_ghidra_title_intro_persistent_test)

add_library(shadowrun_ghidra_title_intro_motion STATIC
    src/title_intro_motion_conversion.cpp
)
target_include_directories(shadowrun_ghidra_title_intro_motion PUBLIC include)

add_executable(shadowrun_ghidra_title_intro_motion_test
    tests/title_intro_motion_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_title_intro_motion_test
    PRIVATE shadowrun_ghidra_title_intro_motion)
add_test(NAME shadowrun_ghidra_title_intro_motion
         COMMAND shadowrun_ghidra_title_intro_motion_test)

add_library(shadowrun_ghidra_title_intro_timed_sprites STATIC
    src/title_intro_timed_sprites_conversion.cpp
)
target_include_directories(shadowrun_ghidra_title_intro_timed_sprites PUBLIC include)
target_link_libraries(shadowrun_ghidra_title_intro_timed_sprites
    PRIVATE shadowrun_ghidra_title_intro_motion)

add_executable(shadowrun_ghidra_title_intro_timed_sprites_test
    tests/title_intro_timed_sprites_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_title_intro_timed_sprites_test
    PRIVATE shadowrun_ghidra_title_intro_timed_sprites)
add_test(NAME shadowrun_ghidra_title_intro_timed_sprites
         COMMAND shadowrun_ghidra_title_intro_timed_sprites_test)

add_library(shadowrun_ghidra_title_intro_sweeps STATIC
    src/title_intro_sweeps_conversion.cpp
)
target_include_directories(shadowrun_ghidra_title_intro_sweeps PUBLIC include)

add_executable(shadowrun_ghidra_title_intro_sweeps_test
    tests/title_intro_sweeps_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_title_intro_sweeps_test
    PRIVATE shadowrun_ghidra_title_intro_sweeps)
add_test(NAME shadowrun_ghidra_title_intro_sweeps
         COMMAND shadowrun_ghidra_title_intro_sweeps_test)

add_library(shadowrun_ghidra_title_intro_final STATIC
    src/title_intro_final_conversion.cpp
)
target_include_directories(shadowrun_ghidra_title_intro_final PUBLIC include)

add_executable(shadowrun_ghidra_title_intro_final_test
    tests/title_intro_final_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_title_intro_final_test
    PRIVATE shadowrun_ghidra_title_intro_final)
add_test(NAME shadowrun_ghidra_title_intro_final
         COMMAND shadowrun_ghidra_title_intro_final_test)

add_library(shadowrun_ghidra_title_intro_stage STATIC
    src/title_intro_stage_conversion.cpp
)
target_include_directories(shadowrun_ghidra_title_intro_stage PUBLIC include)

add_executable(shadowrun_ghidra_title_intro_stage_test
    tests/title_intro_stage_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_title_intro_stage_test
    PRIVATE shadowrun_ghidra_title_intro_stage)
add_test(NAME shadowrun_ghidra_title_intro_stage
         COMMAND shadowrun_ghidra_title_intro_stage_test)

add_library(shadowrun_ghidra_title_intro_late STATIC
    src/title_intro_late_conversion.cpp
)
target_include_directories(shadowrun_ghidra_title_intro_late PUBLIC include)

add_executable(shadowrun_ghidra_title_intro_late_test
    tests/title_intro_late_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_title_intro_late_test
    PRIVATE shadowrun_ghidra_title_intro_late)
add_test(NAME shadowrun_ghidra_title_intro_late
         COMMAND shadowrun_ghidra_title_intro_late_test)

add_library(shadowrun_ghidra_title_intro_schedule STATIC
    src/title_intro_schedule_conversion.cpp
)
target_include_directories(shadowrun_ghidra_title_intro_schedule PUBLIC include)

add_executable(shadowrun_ghidra_title_intro_schedule_test
    tests/title_intro_schedule_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_title_intro_schedule_test
    PRIVATE shadowrun_ghidra_title_intro_schedule)
add_test(NAME shadowrun_ghidra_title_intro_schedule
         COMMAND shadowrun_ghidra_title_intro_schedule_test)

add_library(shadowrun_ghidra_title_intro_stage_loop STATIC
    src/title_intro_stage_loop_conversion.cpp
)
target_include_directories(shadowrun_ghidra_title_intro_stage_loop PUBLIC include)
target_link_libraries(shadowrun_ghidra_title_intro_stage_loop
    PRIVATE shadowrun_ghidra_title_intro_late)

add_executable(shadowrun_ghidra_title_intro_stage_loop_test
    tests/title_intro_stage_loop_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_title_intro_stage_loop_test
    PRIVATE shadowrun_ghidra_title_intro_stage_loop)
add_test(NAME shadowrun_ghidra_title_intro_stage_loop
         COMMAND shadowrun_ghidra_title_intro_stage_loop_test)

add_library(shadowrun_ghidra_title_intro_transfer STATIC
    src/title_intro_transfer_conversion.cpp
)
target_include_directories(shadowrun_ghidra_title_intro_transfer PUBLIC include)

add_executable(shadowrun_ghidra_title_intro_transfer_test
    tests/title_intro_transfer_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_title_intro_transfer_test
    PRIVATE shadowrun_ghidra_title_intro_transfer)
add_test(NAME shadowrun_ghidra_title_intro_transfer
         COMMAND shadowrun_ghidra_title_intro_transfer_test)

add_library(shadowrun_ghidra_title_intro_orchestrator STATIC
    src/title_intro_orchestrator_conversion.cpp
)
target_include_directories(shadowrun_ghidra_title_intro_orchestrator PUBLIC include)
target_link_libraries(shadowrun_ghidra_title_intro_orchestrator
    PRIVATE shadowrun_ghidra_title_intro_stage
            shadowrun_ghidra_title_intro_transfer)

add_executable(shadowrun_ghidra_title_intro_orchestrator_test
    tests/title_intro_orchestrator_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_title_intro_orchestrator_test
    PRIVATE shadowrun_ghidra_title_intro_orchestrator)
add_test(NAME shadowrun_ghidra_title_intro_orchestrator
         COMMAND shadowrun_ghidra_title_intro_orchestrator_test)

add_library(shadowrun_ghidra_title_intro_stage2 STATIC
    src/title_intro_stage2_conversion.cpp
)
target_include_directories(shadowrun_ghidra_title_intro_stage2 PUBLIC include)
target_link_libraries(shadowrun_ghidra_title_intro_stage2
    PRIVATE shadowrun_ghidra_title_intro_schedule)

add_executable(shadowrun_ghidra_title_intro_stage2_test
    tests/title_intro_stage2_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_title_intro_stage2_test
    PRIVATE shadowrun_ghidra_title_intro_stage2)
add_test(NAME shadowrun_ghidra_title_intro_stage2
         COMMAND shadowrun_ghidra_title_intro_stage2_test)

add_library(shadowrun_ghidra_title_intro_stages STATIC
    src/title_intro_stages_conversion.cpp
)
target_include_directories(shadowrun_ghidra_title_intro_stages PUBLIC include)

add_executable(shadowrun_ghidra_title_intro_stages_test
    tests/title_intro_stages_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_title_intro_stages_test
    PRIVATE shadowrun_ghidra_title_intro_stages)
add_test(NAME shadowrun_ghidra_title_intro_stages
         COMMAND shadowrun_ghidra_title_intro_stages_test)

add_library(shadowrun_ghidra_title_intro_animation STATIC
    src/title_intro_animation_conversion.cpp
)
target_include_directories(shadowrun_ghidra_title_intro_animation PUBLIC include)

add_executable(shadowrun_ghidra_title_intro_animation_test
    tests/title_intro_animation_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_title_intro_animation_test
    PRIVATE shadowrun_ghidra_title_intro_animation)
add_test(NAME shadowrun_ghidra_title_intro_animation
         COMMAND shadowrun_ghidra_title_intro_animation_test)

add_library(shadowrun_ghidra_post_intro_mode STATIC
    src/post_intro_mode_conversion.cpp
)
target_include_directories(shadowrun_ghidra_post_intro_mode PUBLIC include)

add_executable(shadowrun_ghidra_post_intro_mode_test
    tests/post_intro_mode_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_post_intro_mode_test
    PRIVATE shadowrun_ghidra_post_intro_mode)
add_test(NAME shadowrun_ghidra_post_intro_mode
         COMMAND shadowrun_ghidra_post_intro_mode_test)

add_library(shadowrun_ghidra_mode_selection_render STATIC
    src/mode_selection_render_conversion.cpp
)
target_include_directories(shadowrun_ghidra_mode_selection_render PUBLIC include)

add_executable(shadowrun_ghidra_mode_selection_render_test
    tests/mode_selection_render_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_mode_selection_render_test
    PRIVATE shadowrun_ghidra_mode_selection_render)
add_test(NAME shadowrun_ghidra_mode_selection_render
         COMMAND shadowrun_ghidra_mode_selection_render_test)

add_library(shadowrun_ghidra_mode_services STATIC
    src/mode_services_conversion.cpp
)
target_include_directories(shadowrun_ghidra_mode_services PUBLIC include)

add_executable(shadowrun_ghidra_mode_services_test
    tests/mode_services_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_mode_services_test
    PRIVATE shadowrun_ghidra_mode_services)
add_test(NAME shadowrun_ghidra_mode_services
         COMMAND shadowrun_ghidra_mode_services_test)

add_library(shadowrun_ghidra_mode_exit STATIC
    src/mode_exit_conversion.cpp
)
target_include_directories(shadowrun_ghidra_mode_exit PUBLIC include)

add_executable(shadowrun_ghidra_mode_exit_test
    tests/mode_exit_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_mode_exit_test
    PRIVATE shadowrun_ghidra_mode_exit)
add_test(NAME shadowrun_ghidra_mode_exit
         COMMAND shadowrun_ghidra_mode_exit_test)

add_library(shadowrun_ghidra_mode_exit_resources STATIC
    src/mode_exit_resources_conversion.cpp
)
target_include_directories(shadowrun_ghidra_mode_exit_resources PUBLIC include)

add_executable(shadowrun_ghidra_mode_exit_resources_test
    tests/mode_exit_resources_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_mode_exit_resources_test
    PRIVATE shadowrun_ghidra_mode_exit_resources)
add_test(NAME shadowrun_ghidra_mode_exit_resources
         COMMAND shadowrun_ghidra_mode_exit_resources_test)

add_library(shadowrun_ghidra_mode_prompt_prepare STATIC
    src/mode_prompt_prepare_conversion.cpp
)
target_include_directories(shadowrun_ghidra_mode_prompt_prepare PUBLIC include)

add_executable(shadowrun_ghidra_mode_prompt_prepare_test
    tests/mode_prompt_prepare_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_mode_prompt_prepare_test
    PRIVATE shadowrun_ghidra_mode_prompt_prepare)
add_test(NAME shadowrun_ghidra_mode_prompt_prepare
         COMMAND shadowrun_ghidra_mode_prompt_prepare_test)

add_library(shadowrun_ghidra_mode_text_prompt STATIC
    src/mode_text_prompt_conversion.cpp
)
target_include_directories(shadowrun_ghidra_mode_text_prompt PUBLIC include)

add_executable(shadowrun_ghidra_mode_text_prompt_test
    tests/mode_text_prompt_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_mode_text_prompt_test
    PRIVATE shadowrun_ghidra_mode_text_prompt)
add_test(NAME shadowrun_ghidra_mode_text_prompt
         COMMAND shadowrun_ghidra_mode_text_prompt_test)

add_library(shadowrun_ghidra_field_exit_prompt STATIC
    src/field_exit_prompt_conversion.cpp
)
target_include_directories(shadowrun_ghidra_field_exit_prompt PUBLIC include)

add_executable(shadowrun_ghidra_field_exit_prompt_test
    tests/field_exit_prompt_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_field_exit_prompt_test
    PRIVATE shadowrun_ghidra_field_exit_prompt)
add_test(NAME shadowrun_ghidra_field_exit_prompt
         COMMAND shadowrun_ghidra_field_exit_prompt_test)

add_library(shadowrun_ghidra_world_sequence_tile_render STATIC
    src/world_sequence_tile_render_conversion.cpp
)
target_include_directories(shadowrun_ghidra_world_sequence_tile_render PUBLIC include)

add_executable(shadowrun_ghidra_world_sequence_tile_render_test
    tests/world_sequence_tile_render_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_world_sequence_tile_render_test
    PRIVATE shadowrun_ghidra_world_sequence_tile_render)
add_test(NAME shadowrun_ghidra_world_sequence_tile_render
         COMMAND shadowrun_ghidra_world_sequence_tile_render_test)

add_library(shadowrun_ghidra_field_mode_exit STATIC
    src/field_mode_exit_conversion.cpp
)
target_include_directories(shadowrun_ghidra_field_mode_exit PUBLIC include)

add_executable(shadowrun_ghidra_field_mode_exit_test
    tests/field_mode_exit_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_field_mode_exit_test
    PRIVATE shadowrun_ghidra_field_mode_exit)
add_test(NAME shadowrun_ghidra_field_mode_exit
         COMMAND shadowrun_ghidra_field_mode_exit_test)

add_library(shadowrun_ghidra_field_exit_orchestrator STATIC
    src/field_exit_orchestrator_conversion.cpp
)
target_include_directories(shadowrun_ghidra_field_exit_orchestrator PUBLIC include)

add_executable(shadowrun_ghidra_field_exit_orchestrator_test
    tests/field_exit_orchestrator_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_field_exit_orchestrator_test
    PRIVATE shadowrun_ghidra_field_exit_orchestrator)
add_test(NAME shadowrun_ghidra_field_exit_orchestrator
         COMMAND shadowrun_ghidra_field_exit_orchestrator_test)

add_library(shadowrun_ghidra_field_mode_service STATIC
    src/field_mode_service_conversion.cpp
)
target_include_directories(shadowrun_ghidra_field_mode_service PUBLIC include)

add_executable(shadowrun_ghidra_field_mode_service_test
    tests/field_mode_service_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_field_mode_service_test
    PRIVATE shadowrun_ghidra_field_mode_service)
add_test(NAME shadowrun_ghidra_field_mode_service
    COMMAND shadowrun_ghidra_field_mode_service_test)

add_library(shadowrun_ghidra_field_mode_finalizer STATIC
    src/field_mode_finalizer_conversion.cpp)
target_include_directories(shadowrun_ghidra_field_mode_finalizer PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(shadowrun_ghidra_field_mode_finalizer_test
    tests/field_mode_finalizer_conversion_test.cpp)
target_link_libraries(shadowrun_ghidra_field_mode_finalizer_test PRIVATE
    shadowrun_ghidra_field_mode_finalizer)
add_test(NAME shadowrun_ghidra_field_mode_finalizer
    COMMAND shadowrun_ghidra_field_mode_finalizer_test)

add_library(shadowrun_ghidra_mode_entry_resource STATIC
    src/mode_entry_resource_conversion.cpp)
target_include_directories(shadowrun_ghidra_mode_entry_resource PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(shadowrun_ghidra_mode_entry_resource_test
    tests/mode_entry_resource_conversion_test.cpp)
target_link_libraries(shadowrun_ghidra_mode_entry_resource_test PRIVATE
    shadowrun_ghidra_mode_entry_resource)
add_test(NAME shadowrun_ghidra_mode_entry_resource
    COMMAND shadowrun_ghidra_mode_entry_resource_test)

add_library(shadowrun_ghidra_mode_optional_resource STATIC
    src/mode_optional_resource_conversion.cpp
)
target_include_directories(shadowrun_ghidra_mode_optional_resource PUBLIC include)

add_executable(shadowrun_ghidra_mode_optional_resource_test
    tests/mode_optional_resource_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_mode_optional_resource_test
    PRIVATE shadowrun_ghidra_mode_optional_resource)
add_test(NAME shadowrun_ghidra_mode_optional_resource
         COMMAND shadowrun_ghidra_mode_optional_resource_test)

add_library(shadowrun_ghidra_mode_resource_variant STATIC
    src/mode_resource_variant_conversion.cpp
)
target_include_directories(shadowrun_ghidra_mode_resource_variant PUBLIC include)

add_executable(shadowrun_ghidra_mode_resource_variant_test
    tests/mode_resource_variant_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_mode_resource_variant_test
    PRIVATE shadowrun_ghidra_mode_resource_variant)
add_test(NAME shadowrun_ghidra_mode_resource_variant
         COMMAND shadowrun_ghidra_mode_resource_variant_test)

add_library(shadowrun_ghidra_mode_resource_state STATIC
    src/mode_resource_state_conversion.cpp
)
target_include_directories(shadowrun_ghidra_mode_resource_state PUBLIC include)

add_executable(shadowrun_ghidra_mode_resource_state_test
    tests/mode_resource_state_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_mode_resource_state_test
    PRIVATE shadowrun_ghidra_mode_resource_state)
add_test(NAME shadowrun_ghidra_mode_resource_state
         COMMAND shadowrun_ghidra_mode_resource_state_test)

add_library(shadowrun_ghidra_sprite_record_builder STATIC
    src/sprite_record_builder_conversion.cpp
)
target_include_directories(shadowrun_ghidra_sprite_record_builder PUBLIC include)

add_executable(shadowrun_ghidra_sprite_record_builder_test
    tests/sprite_record_builder_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_sprite_record_builder_test
    PRIVATE shadowrun_ghidra_sprite_record_builder)
add_test(NAME shadowrun_ghidra_sprite_record_builder
         COMMAND shadowrun_ghidra_sprite_record_builder_test)

add_library(shadowrun_ghidra_alternating_word_transfer STATIC
    src/alternating_word_transfer_conversion.cpp
)
target_include_directories(shadowrun_ghidra_alternating_word_transfer PUBLIC include)

add_executable(shadowrun_ghidra_alternating_word_transfer_test
    tests/alternating_word_transfer_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_alternating_word_transfer_test
    PRIVATE shadowrun_ghidra_alternating_word_transfer)
add_test(NAME shadowrun_ghidra_alternating_word_transfer
         COMMAND shadowrun_ghidra_alternating_word_transfer_test)

add_library(shadowrun_ghidra_entity_action_code STATIC
    src/entity_action_code_conversion.cpp
)
target_include_directories(shadowrun_ghidra_entity_action_code PUBLIC include)

add_executable(shadowrun_ghidra_entity_action_code_test
    tests/entity_action_code_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_entity_action_code_test
    PRIVATE shadowrun_ghidra_entity_action_code)
add_test(NAME shadowrun_ghidra_entity_action_code
         COMMAND shadowrun_ghidra_entity_action_code_test)

add_library(shadowrun_ghidra_decimal_glyph STATIC
    src/decimal_glyph_conversion.cpp
)
target_include_directories(shadowrun_ghidra_decimal_glyph PUBLIC include)

add_executable(shadowrun_ghidra_decimal_glyph_test
    tests/decimal_glyph_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_decimal_glyph_test
    PRIVATE shadowrun_ghidra_decimal_glyph)
add_test(NAME shadowrun_ghidra_decimal_glyph
         COMMAND shadowrun_ghidra_decimal_glyph_test)

add_library(shadowrun_ghidra_entity_action_candidate STATIC
    src/entity_action_candidate_conversion.cpp
)
target_include_directories(shadowrun_ghidra_entity_action_candidate PUBLIC include)

add_executable(shadowrun_ghidra_entity_action_candidate_test
    tests/entity_action_candidate_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_entity_action_candidate_test
    PRIVATE shadowrun_ghidra_entity_action_candidate)
add_test(NAME shadowrun_ghidra_entity_action_candidate
         COMMAND shadowrun_ghidra_entity_action_candidate_test)

add_library(shadowrun_ghidra_entity_boundary_reposition STATIC
    src/entity_boundary_reposition_conversion.cpp
)
target_include_directories(shadowrun_ghidra_entity_boundary_reposition PUBLIC include)

add_executable(shadowrun_ghidra_entity_boundary_reposition_test
    tests/entity_boundary_reposition_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_entity_boundary_reposition_test
    PRIVATE shadowrun_ghidra_entity_boundary_reposition)
add_test(NAME shadowrun_ghidra_entity_boundary_reposition
         COMMAND shadowrun_ghidra_entity_boundary_reposition_test)

add_library(shadowrun_ghidra_entity_close_range STATIC
    src/entity_close_range_conversion.cpp
)
target_include_directories(shadowrun_ghidra_entity_close_range PUBLIC include)

add_executable(shadowrun_ghidra_entity_close_range_test
    tests/entity_close_range_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_entity_close_range_test
    PRIVATE shadowrun_ghidra_entity_close_range)
add_test(NAME shadowrun_ghidra_entity_close_range
         COMMAND shadowrun_ghidra_entity_close_range_test)

add_library(shadowrun_ghidra_entity_tile_mapping STATIC
    src/entity_tile_mapping_conversion.cpp
)
target_include_directories(shadowrun_ghidra_entity_tile_mapping PUBLIC include)

add_executable(shadowrun_ghidra_entity_tile_mapping_test
    tests/entity_tile_mapping_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_entity_tile_mapping_test
    PRIVATE shadowrun_ghidra_entity_tile_mapping)
add_test(NAME shadowrun_ghidra_entity_tile_mapping
         COMMAND shadowrun_ghidra_entity_tile_mapping_test)

add_library(shadowrun_ghidra_entity_boundary_transition STATIC
    src/entity_boundary_transition_conversion.cpp
)
target_include_directories(shadowrun_ghidra_entity_boundary_transition PUBLIC include)

add_executable(shadowrun_ghidra_entity_boundary_transition_test
    tests/entity_boundary_transition_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_entity_boundary_transition_test
    PRIVATE shadowrun_ghidra_entity_boundary_transition)
add_test(NAME shadowrun_ghidra_entity_boundary_transition
         COMMAND shadowrun_ghidra_entity_boundary_transition_test)

add_library(shadowrun_ghidra_queued_transfer_flush STATIC
    src/queued_transfer_flush_conversion.cpp
)
target_include_directories(shadowrun_ghidra_queued_transfer_flush PUBLIC include)

add_executable(shadowrun_ghidra_queued_transfer_flush_test
    tests/queued_transfer_flush_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_queued_transfer_flush_test
    PRIVATE shadowrun_ghidra_queued_transfer_flush)
add_test(NAME shadowrun_ghidra_queued_transfer_flush
         COMMAND shadowrun_ghidra_queued_transfer_flush_test)

add_library(shadowrun_ghidra_hud_selection_lookup STATIC
    src/hud_selection_lookup_conversion.cpp
)
target_include_directories(shadowrun_ghidra_hud_selection_lookup PUBLIC include)

add_executable(shadowrun_ghidra_hud_selection_lookup_test
    tests/hud_selection_lookup_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_hud_selection_lookup_test
    PRIVATE shadowrun_ghidra_hud_selection_lookup)
add_test(NAME shadowrun_ghidra_hud_selection_lookup
         COMMAND shadowrun_ghidra_hud_selection_lookup_test)

add_library(shadowrun_ghidra_title_resource_sequence STATIC
    src/title_resource_sequence_conversion.cpp
)
target_include_directories(shadowrun_ghidra_title_resource_sequence PUBLIC include)

add_executable(shadowrun_ghidra_title_resource_sequence_test
    tests/title_resource_sequence_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_title_resource_sequence_test
    PRIVATE shadowrun_ghidra_title_resource_sequence)
add_test(NAME shadowrun_ghidra_title_resource_sequence
         COMMAND shadowrun_ghidra_title_resource_sequence_test)

add_library(shadowrun_ghidra_field_resource_sequence STATIC
    src/field_resource_sequence_conversion.cpp
)
target_include_directories(shadowrun_ghidra_field_resource_sequence PUBLIC include)

add_executable(shadowrun_ghidra_field_resource_sequence_test
    tests/field_resource_sequence_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_field_resource_sequence_test
    PRIVATE shadowrun_ghidra_field_resource_sequence)
add_test(NAME shadowrun_ghidra_field_resource_sequence
         COMMAND shadowrun_ghidra_field_resource_sequence_test)

add_library(shadowrun_ghidra_mode_display_reset STATIC
    src/mode_display_reset_conversion.cpp
)
target_include_directories(shadowrun_ghidra_mode_display_reset PUBLIC include)

add_executable(shadowrun_ghidra_mode_display_reset_test
    tests/mode_display_reset_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_mode_display_reset_test
    PRIVATE shadowrun_ghidra_mode_display_reset)
add_test(NAME shadowrun_ghidra_mode_display_reset
         COMMAND shadowrun_ghidra_mode_display_reset_test)

add_library(shadowrun_ghidra_menu_service_sequence STATIC
    src/menu_service_sequence_conversion.cpp
)
target_include_directories(shadowrun_ghidra_menu_service_sequence PUBLIC include)

add_executable(shadowrun_ghidra_menu_service_sequence_test
    tests/menu_service_sequence_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_menu_service_sequence_test
    PRIVATE shadowrun_ghidra_menu_service_sequence)
add_test(NAME shadowrun_ghidra_menu_service_sequence
         COMMAND shadowrun_ghidra_menu_service_sequence_test)

add_library(shadowrun_ghidra_equipment_combat_bar STATIC
    src/equipment_combat_bar_conversion.cpp
)
target_include_directories(shadowrun_ghidra_equipment_combat_bar PUBLIC include)
add_executable(shadowrun_ghidra_equipment_combat_bar_test
    tests/equipment_combat_bar_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_equipment_combat_bar_test
    PRIVATE shadowrun_ghidra_equipment_combat_bar)
add_test(NAME shadowrun_ghidra_equipment_combat_bar
         COMMAND shadowrun_ghidra_equipment_combat_bar_test)

add_library(shadowrun_ghidra_attributes_skills_menu STATIC
    src/attributes_skills_menu_conversion.cpp
)
target_include_directories(shadowrun_ghidra_attributes_skills_menu PUBLIC include)
target_link_libraries(shadowrun_ghidra_attributes_skills_menu
    PRIVATE shadowrun_ghidra_runner_record_operations)

add_executable(shadowrun_ghidra_attributes_skills_menu_test
    tests/attributes_skills_menu_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_attributes_skills_menu_test
    PRIVATE shadowrun_ghidra_attributes_skills_menu)
add_test(NAME shadowrun_ghidra_attributes_skills_menu
         COMMAND shadowrun_ghidra_attributes_skills_menu_test)

add_library(shadowrun_ghidra_magic_menu STATIC
    src/magic_menu_conversion.cpp
)
target_include_directories(shadowrun_ghidra_magic_menu PUBLIC include)

add_executable(shadowrun_ghidra_magic_menu_test
    tests/magic_menu_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_magic_menu_test
    PRIVATE shadowrun_ghidra_magic_menu)
add_test(NAME shadowrun_ghidra_magic_menu
         COMMAND shadowrun_ghidra_magic_menu_test)

add_library(shadowrun_ghidra_matrix_entry STATIC
    src/matrix_entry_conversion.cpp
)
target_include_directories(shadowrun_ghidra_matrix_entry PUBLIC include)

add_executable(shadowrun_ghidra_matrix_entry_test
    tests/matrix_entry_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_matrix_entry_test
    PRIVATE shadowrun_ghidra_matrix_entry)
add_test(NAME shadowrun_ghidra_matrix_entry
         COMMAND shadowrun_ghidra_matrix_entry_test)

add_library(shadowrun_ghidra_matrix_loop STATIC
    src/matrix_loop_conversion.cpp
)
target_include_directories(shadowrun_ghidra_matrix_loop PUBLIC include)

add_executable(shadowrun_ghidra_matrix_loop_test
    tests/matrix_loop_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_matrix_loop_test
    PRIVATE shadowrun_ghidra_matrix_loop)
add_test(NAME shadowrun_ghidra_matrix_loop
         COMMAND shadowrun_ghidra_matrix_loop_test)

add_library(shadowrun_ghidra_matrix_storage STATIC
    src/matrix_storage_conversion.cpp
)
target_include_directories(shadowrun_ghidra_matrix_storage PUBLIC include)

add_executable(shadowrun_ghidra_matrix_storage_test
    tests/matrix_storage_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_matrix_storage_test
    PRIVATE shadowrun_ghidra_matrix_storage)
add_test(NAME shadowrun_ghidra_matrix_storage
         COMMAND shadowrun_ghidra_matrix_storage_test)

add_library(shadowrun_ghidra_matrix_cyber_info STATIC
    src/matrix_cyber_info_conversion.cpp
)
target_include_directories(shadowrun_ghidra_matrix_cyber_info PUBLIC include)
target_link_libraries(shadowrun_ghidra_matrix_cyber_info
    PRIVATE shadowrun_ghidra_selection)

add_executable(shadowrun_ghidra_matrix_cyber_info_test
    tests/matrix_cyber_info_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_matrix_cyber_info_test
    PRIVATE shadowrun_ghidra_matrix_cyber_info)
add_test(NAME shadowrun_ghidra_matrix_cyber_info
         COMMAND shadowrun_ghidra_matrix_cyber_info_test)

add_library(shadowrun_ghidra_matrix_utility STATIC
    src/matrix_utility_conversion.cpp
)
target_include_directories(shadowrun_ghidra_matrix_utility PUBLIC include)
target_link_libraries(shadowrun_ghidra_matrix_utility
    PRIVATE shadowrun_ghidra_selection)

add_executable(shadowrun_ghidra_matrix_utility_test
    tests/matrix_utility_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_matrix_utility_test
    PRIVATE shadowrun_ghidra_matrix_utility)
add_test(NAME shadowrun_ghidra_matrix_utility
         COMMAND shadowrun_ghidra_matrix_utility_test)

add_library(shadowrun_ghidra_matrix_retreat STATIC
    src/matrix_retreat_conversion.cpp
)
target_include_directories(shadowrun_ghidra_matrix_retreat PUBLIC include)

add_executable(shadowrun_ghidra_matrix_retreat_test
    tests/matrix_retreat_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_matrix_retreat_test
    PRIVATE shadowrun_ghidra_matrix_retreat)
add_test(NAME shadowrun_ghidra_matrix_retreat
         COMMAND shadowrun_ghidra_matrix_retreat_test)

add_library(shadowrun_ghidra_matrix_program_slot STATIC
    src/matrix_program_slot_conversion.cpp
)
target_include_directories(shadowrun_ghidra_matrix_program_slot PUBLIC include)
target_link_libraries(shadowrun_ghidra_matrix_program_slot
    PRIVATE shadowrun_ghidra_selection)

add_executable(shadowrun_ghidra_matrix_program_slot_test
    tests/matrix_program_slot_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_matrix_program_slot_test
    PRIVATE shadowrun_ghidra_matrix_program_slot)
add_test(NAME shadowrun_ghidra_matrix_program_slot
         COMMAND shadowrun_ghidra_matrix_program_slot_test)

add_library(shadowrun_ghidra_matrix_program_timing STATIC
    src/matrix_program_timing_conversion.cpp
)
target_include_directories(shadowrun_ghidra_matrix_program_timing PUBLIC include)

add_executable(shadowrun_ghidra_matrix_program_timing_test
    tests/matrix_program_timing_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_matrix_program_timing_test
    PRIVATE shadowrun_ghidra_matrix_program_timing)
add_test(NAME shadowrun_ghidra_matrix_program_timing
         COMMAND shadowrun_ghidra_matrix_program_timing_test)

add_library(shadowrun_ghidra_matrix_menu_service STATIC
    src/matrix_menu_service_conversion.cpp
)
target_include_directories(shadowrun_ghidra_matrix_menu_service PUBLIC include)

add_executable(shadowrun_ghidra_matrix_menu_service_test
    tests/matrix_menu_service_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_matrix_menu_service_test
    PRIVATE shadowrun_ghidra_matrix_menu_service)
add_test(NAME shadowrun_ghidra_matrix_menu_service
         COMMAND shadowrun_ghidra_matrix_menu_service_test)

add_library(shadowrun_ghidra_matrix_program_action STATIC
    src/matrix_program_action_conversion.cpp
)
target_include_directories(shadowrun_ghidra_matrix_program_action PUBLIC include)

add_executable(shadowrun_ghidra_matrix_program_action_test
    tests/matrix_program_action_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_matrix_program_action_test
    PRIVATE shadowrun_ghidra_matrix_program_action)
add_test(NAME shadowrun_ghidra_matrix_program_action
         COMMAND shadowrun_ghidra_matrix_program_action_test)

add_library(shadowrun_ghidra_hud_mode STATIC
    src/hud_mode_conversion.cpp
)
target_include_directories(shadowrun_ghidra_hud_mode PUBLIC include)

add_executable(shadowrun_ghidra_hud_mode_test
    tests/hud_mode_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_hud_mode_test
    PRIVATE shadowrun_ghidra_hud_mode)
add_test(NAME shadowrun_ghidra_hud_mode
         COMMAND shadowrun_ghidra_hud_mode_test)

add_library(shadowrun_ghidra_entity_action_sequence STATIC
    src/entity_action_sequence_conversion.cpp
)
target_include_directories(shadowrun_ghidra_entity_action_sequence PUBLIC include)

add_executable(shadowrun_ghidra_entity_action_sequence_test
    tests/entity_action_sequence_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_entity_action_sequence_test
    PRIVATE shadowrun_ghidra_entity_action_sequence)
add_test(NAME shadowrun_ghidra_entity_action_sequence
         COMMAND shadowrun_ghidra_entity_action_sequence_test)

add_library(shadowrun_ghidra_entity_action_state STATIC
    src/entity_action_state_conversion.cpp
)
target_include_directories(shadowrun_ghidra_entity_action_state PUBLIC include)

add_executable(shadowrun_ghidra_entity_action_state_test
    tests/entity_action_state_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_entity_action_state_test
    PRIVATE shadowrun_ghidra_entity_action_state)
add_test(NAME shadowrun_ghidra_entity_action_state
         COMMAND shadowrun_ghidra_entity_action_state_test)

add_library(shadowrun_ghidra_menu_hud_service STATIC
    src/menu_hud_service_conversion.cpp
)
target_include_directories(shadowrun_ghidra_menu_hud_service PUBLIC include)

add_executable(shadowrun_ghidra_menu_hud_service_test
    tests/menu_hud_service_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_menu_hud_service_test
    PRIVATE shadowrun_ghidra_menu_hud_service)
add_test(NAME shadowrun_ghidra_menu_hud_service
         COMMAND shadowrun_ghidra_menu_hud_service_test)

add_library(shadowrun_ghidra_terrain_workflow STATIC
    src/terrain_workflow_conversion.cpp
)
target_include_directories(shadowrun_ghidra_terrain_workflow PUBLIC include)
target_link_libraries(shadowrun_ghidra_terrain_workflow
    PRIVATE shadowrun_ghidra_entity_tile_mapping)

add_executable(shadowrun_ghidra_terrain_workflow_test
    tests/terrain_workflow_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_terrain_workflow_test
    PRIVATE shadowrun_ghidra_terrain_workflow)
add_test(NAME shadowrun_ghidra_terrain_workflow
         COMMAND shadowrun_ghidra_terrain_workflow_test)

add_library(shadowrun_ghidra_display_service_batch STATIC
    src/display_service_batch_conversion.cpp
)
target_include_directories(shadowrun_ghidra_display_service_batch PUBLIC include)

add_executable(shadowrun_ghidra_display_service_batch_test
    tests/display_service_batch_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_display_service_batch_test
    PRIVATE shadowrun_ghidra_display_service_batch)
add_test(NAME shadowrun_ghidra_display_service_batch
         COMMAND shadowrun_ghidra_display_service_batch_test)

add_library(shadowrun_ghidra_menu_transfer_batch STATIC
    src/menu_transfer_batch_conversion.cpp
)
target_include_directories(shadowrun_ghidra_menu_transfer_batch PUBLIC include)

add_executable(shadowrun_ghidra_menu_transfer_batch_test
    tests/menu_transfer_batch_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_menu_transfer_batch_test
    PRIVATE shadowrun_ghidra_menu_transfer_batch)
add_test(NAME shadowrun_ghidra_menu_transfer_batch
         COMMAND shadowrun_ghidra_menu_transfer_batch_test)

add_library(shadowrun_ghidra_menu_slot_batch STATIC
    src/menu_slot_batch_conversion.cpp
)
target_include_directories(shadowrun_ghidra_menu_slot_batch PUBLIC include)

add_executable(shadowrun_ghidra_menu_slot_batch_test
    tests/menu_slot_batch_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_menu_slot_batch_test
    PRIVATE shadowrun_ghidra_menu_slot_batch)
add_test(NAME shadowrun_ghidra_menu_slot_batch
         COMMAND shadowrun_ghidra_menu_slot_batch_test)

add_library(shadowrun_ghidra_world_effect_batch STATIC
    src/world_effect_batch_conversion.cpp
)
target_include_directories(shadowrun_ghidra_world_effect_batch PUBLIC include)

add_executable(shadowrun_ghidra_world_effect_batch_test
    tests/world_effect_batch_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_world_effect_batch_test
    PRIVATE shadowrun_ghidra_world_effect_batch)
add_test(NAME shadowrun_ghidra_world_effect_batch
         COMMAND shadowrun_ghidra_world_effect_batch_test)

add_library(shadowrun_ghidra_world_effect_edge STATIC
    src/world_effect_edge_conversion.cpp
)
target_include_directories(shadowrun_ghidra_world_effect_edge PUBLIC include)

add_executable(shadowrun_ghidra_world_effect_edge_test
    tests/world_effect_edge_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_world_effect_edge_test
    PRIVATE shadowrun_ghidra_world_effect_edge)
add_test(NAME shadowrun_ghidra_world_effect_edge
         COMMAND shadowrun_ghidra_world_effect_edge_test)

add_library(shadowrun_ghidra_world_effect_initializer STATIC
    src/world_effect_initializer_conversion.cpp
)
target_include_directories(shadowrun_ghidra_world_effect_initializer PUBLIC include)

add_executable(shadowrun_ghidra_world_effect_initializer_test
    tests/world_effect_initializer_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_world_effect_initializer_test
    PRIVATE shadowrun_ghidra_world_effect_initializer)
add_test(NAME shadowrun_ghidra_world_effect_initializer
         COMMAND shadowrun_ghidra_world_effect_initializer_test)

add_library(shadowrun_ghidra_nearby_effect STATIC
    src/nearby_effect_conversion.cpp
)
target_include_directories(shadowrun_ghidra_nearby_effect PUBLIC include)

add_executable(shadowrun_ghidra_nearby_effect_test
    tests/nearby_effect_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_nearby_effect_test
    PRIVATE shadowrun_ghidra_nearby_effect)
add_test(NAME shadowrun_ghidra_nearby_effect
         COMMAND shadowrun_ghidra_nearby_effect_test)

add_library(shadowrun_ghidra_world_frame_logic STATIC
    src/world_frame_logic_conversion.cpp
)
target_include_directories(shadowrun_ghidra_world_frame_logic PUBLIC include)

add_executable(shadowrun_ghidra_world_frame_logic_test
    tests/world_frame_logic_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_world_frame_logic_test
    PRIVATE shadowrun_ghidra_world_frame_logic)
add_test(NAME shadowrun_ghidra_world_frame_logic
         COMMAND shadowrun_ghidra_world_frame_logic_test)

add_library(shadowrun_ghidra_entity_service_target STATIC
    src/entity_service_target_conversion.cpp
)
target_include_directories(shadowrun_ghidra_entity_service_target PUBLIC include)

add_executable(shadowrun_ghidra_entity_service_target_test
    tests/entity_service_target_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_entity_service_target_test
    PRIVATE shadowrun_ghidra_entity_service_target)
add_test(NAME shadowrun_ghidra_entity_service_target
         COMMAND shadowrun_ghidra_entity_service_target_test)

add_library(shadowrun_ghidra_audio_command_batch STATIC
    src/audio_command_batch_conversion.cpp
)
target_include_directories(shadowrun_ghidra_audio_command_batch PUBLIC include)

add_executable(shadowrun_ghidra_audio_command_batch_test
    tests/audio_command_batch_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_audio_command_batch_test
    PRIVATE shadowrun_ghidra_audio_command_batch)
add_test(NAME shadowrun_ghidra_audio_command_batch
         COMMAND shadowrun_ghidra_audio_command_batch_test)

add_library(shadowrun_ghidra_field_entity_render_batch STATIC
    src/field_entity_render_batch_conversion.cpp
)
target_include_directories(shadowrun_ghidra_field_entity_render_batch PUBLIC include)

add_executable(shadowrun_ghidra_field_entity_render_batch_test
    tests/field_entity_render_batch_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_field_entity_render_batch_test
    PRIVATE shadowrun_ghidra_field_entity_render_batch)
add_test(NAME shadowrun_ghidra_field_entity_render_batch
         COMMAND shadowrun_ghidra_field_entity_render_batch_test)

add_library(shadowrun_ghidra_field_menu_state_batch STATIC
    src/field_menu_state_batch_conversion.cpp
)
target_include_directories(shadowrun_ghidra_field_menu_state_batch PUBLIC include)

add_executable(shadowrun_ghidra_field_menu_state_batch_test
    tests/field_menu_state_batch_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_field_menu_state_batch_test
    PRIVATE shadowrun_ghidra_field_menu_state_batch)
add_test(NAME shadowrun_ghidra_field_menu_state_batch
         COMMAND shadowrun_ghidra_field_menu_state_batch_test)

add_library(shadowrun_ghidra_title_effect_animation_batch STATIC
    src/title_effect_animation_batch_conversion.cpp
)
target_include_directories(shadowrun_ghidra_title_effect_animation_batch PUBLIC include)

add_executable(shadowrun_ghidra_title_effect_animation_batch_test
    tests/title_effect_animation_batch_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_title_effect_animation_batch_test
    PRIVATE shadowrun_ghidra_title_effect_animation_batch)
add_test(NAME shadowrun_ghidra_title_effect_animation_batch
         COMMAND shadowrun_ghidra_title_effect_animation_batch_test)

add_library(shadowrun_ghidra_world_resource_batch STATIC
    src/world_resource_batch_conversion.cpp
)
target_include_directories(shadowrun_ghidra_world_resource_batch PUBLIC include)

add_executable(shadowrun_ghidra_world_resource_batch_test
    tests/world_resource_batch_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_world_resource_batch_test
    PRIVATE shadowrun_ghidra_world_resource_batch)
add_test(NAME shadowrun_ghidra_world_resource_batch
         COMMAND shadowrun_ghidra_world_resource_batch_test)

add_library(shadowrun_ghidra_audio_service_batch STATIC
    src/audio_service_batch_conversion.cpp
)
target_include_directories(shadowrun_ghidra_audio_service_batch PUBLIC include)

add_executable(shadowrun_ghidra_audio_service_batch_test
    tests/audio_service_batch_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_audio_service_batch_test
    PRIVATE shadowrun_ghidra_audio_service_batch)
add_test(NAME shadowrun_ghidra_audio_service_batch
         COMMAND shadowrun_ghidra_audio_service_batch_test)

add_library(shadowrun_ghidra_remaining_field STATIC
    src/remaining_field_conversion.cpp
)
target_include_directories(shadowrun_ghidra_remaining_field PUBLIC include)

add_executable(shadowrun_ghidra_remaining_field_test
    tests/remaining_field_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_remaining_field_test
    PRIVATE shadowrun_ghidra_remaining_field)
add_test(NAME shadowrun_ghidra_remaining_field
         COMMAND shadowrun_ghidra_remaining_field_test)

add_library(shadowrun_ghidra_remaining_routine_catalog STATIC
    src/remaining_routine_catalog_conversion.cpp
)
target_include_directories(shadowrun_ghidra_remaining_routine_catalog PUBLIC include)

add_executable(shadowrun_ghidra_remaining_routine_catalog_test
    tests/remaining_routine_catalog_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_remaining_routine_catalog_test
    PRIVATE shadowrun_ghidra_remaining_routine_catalog)
add_test(NAME shadowrun_ghidra_remaining_routine_catalog
         COMMAND shadowrun_ghidra_remaining_routine_catalog_test)

add_library(shadowrun_ghidra_world_entity STATIC
    src/world_entity_conversion.cpp
)
target_include_directories(shadowrun_ghidra_world_entity PUBLIC include)

add_executable(shadowrun_ghidra_world_entity_test
    tests/world_entity_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_world_entity_test
    PRIVATE shadowrun_ghidra_world_entity)
add_test(NAME shadowrun_ghidra_world_entity
         COMMAND shadowrun_ghidra_world_entity_test)

add_library(shadowrun_ghidra_world_mode_routine STATIC
    src/world_mode_routine_conversion.cpp
)
target_include_directories(shadowrun_ghidra_world_mode_routine PUBLIC include)
add_executable(shadowrun_ghidra_world_mode_routine_test
    tests/world_mode_routine_conversion_test.cpp
)
target_link_libraries(shadowrun_ghidra_world_mode_routine_test
    PRIVATE shadowrun_ghidra_world_mode_routine)
add_test(NAME shadowrun_ghidra_world_mode_routine
         COMMAND shadowrun_ghidra_world_mode_routine_test)
