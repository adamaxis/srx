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
