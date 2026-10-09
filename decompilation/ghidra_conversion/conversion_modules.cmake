# Behavioral build units for the offline conversion.  The individual source
# files remain traceable to a ROM contract, but the project no longer presents
# one exported function as one architectural source file.  Each object target
# below is a subsystem boundary from docs/conversion-source-modules.tsv.

file(GLOB SHADOWRUN_MODULE_PLATFORM_INPUT_TIMING CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/controller_*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/timing_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/counter_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/selection_conversion.cpp"
)
file(GLOB SHADOWRUN_MODULE_SOURCE_DATA CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/asset_catalog_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/armor_catalog_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/compressed_data_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/coordinate_data_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/item_catalog_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/magic_spell_catalog_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/packed_data_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/random_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/source_buffer_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/remaining_*.cpp"
)
file(GLOB SHADOWRUN_MODULE_TRANSFER_STREAMS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/alternating_word_transfer_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/queued_transfer_flush_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/word_transfer_conversion.cpp"
)

file(GLOB SHADOWRUN_MODULE_DISPLAY_TEXT CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/rendering/decimal_glyph_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/rendering/display_service_batch_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/rendering/display_transition_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/rendering/resource_refresh_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/rendering/startup_vdp_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/rendering/text_line_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/rendering/wrapped_encoded_text_conversion.cpp"
)
file(GLOB SHADOWRUN_MODULE_SPRITE_PIPELINE CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/rendering/sprite_*.cpp"
)

file(GLOB SHADOWRUN_MODULE_RUNNER_RECORDS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/session/runner_action_delay_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/session/runner_action_result_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/session/runner_record_access_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/session/runner_record_operations_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/session/runner_status_conversion.cpp"
)
file(GLOB SHADOWRUN_MODULE_ROSTER_SAVE CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/session/runner_animation_grid_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/session/runner_record_selection_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/session/recruited_runner_service_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/session/runner_roster_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/session/runner_selection_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/session/runner_switch_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/session/save_*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/session/startup_record_assets_conversion.cpp"
)

file(GLOB SHADOWRUN_MODULE_FIELD_SCENE_CONTROLLER CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/field/field_active_runner_selection_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/field/field_mode_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/field/field_mode_scene_runtime_preparation_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/field/field_mode_exit_transition_finalizer_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/field/field_runner_state_asset_refresh_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/field/field_map_event_system_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/field/field_script_route_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/field/field_mode_finalizer_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/field/field_mode_service*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/field/field_scene_entry_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/field/field_viewport_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/field/scene_profile_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/field/scene_record_reset_conversion.cpp"
)
file(GLOB SHADOWRUN_MODULE_FIELD_NAVIGATION CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/field/field_camera_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/field/field_coordinate_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/field/field_movement_input_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/field/field_exit_*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/field/field_mode_exit_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/field/mode_exit*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/field/mode_nine_conversion.cpp"
)
file(GLOB SHADOWRUN_MODULE_FIELD_RESOURCE_FLOW CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/field/field_quantity_purchase_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/field/field_resource_record_purchase_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/field/field_resource_purchase_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/field/field_resource_sequence_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/field/mode_entry_resource_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/field/mode_optional_resource_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/field/mode_resource_state_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/field/mode_resource_variant_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/field/scene_resource_conversion.cpp"
)
file(GLOB SHADOWRUN_MODULE_FIELD_INTERACTION_ACTIONS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/field/field_action_*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/field/field_inventory_action_detail_render_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/field/field_equipment_upgrade_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/field/stoker_motel_arrival_command_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/field/stoker_motel_debt_payment_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/field/stoker_motel_followup_commands_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/field/stoker_motel_prompt_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/field/stoker_motel_room_payment_conversion.cpp"
)
file(GLOB SHADOWRUN_MODULE_FIELD_PRESENTATION_SERVICES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/field/field_entity_render_batch_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/field/field_hud_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/field/field_interaction_continuation_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/field/field_menu_state_batch_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/field/mode_display_reset_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/field/mode_prompt_prepare_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/field/mode_selection_render_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/field/mode_services_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/field/mode_text_prompt_conversion.cpp"
)
file(GLOB SHADOWRUN_MODULE_FIELD_TERRAIN_COLLISION CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/field/terrain_*.cpp"
)

file(GLOB SHADOWRUN_MODULE_ENTITY_STATE_PROJECTION CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/entities/entity_coordinate_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/entities/entity_delay_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/entities/entity_dirty_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/entities/entity_position_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/entities/entity_projection_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/entities/entity_sort_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/entities/entity_state_primitives_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/entities/entity_table_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/entities/entity_template_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/entities/entity_tile_mapping_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/entities/entity_timing_conversion.cpp"
)
file(GLOB SHADOWRUN_MODULE_ENTITY_MOVEMENT_COLLISION CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/entities/collision_*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/entities/entity_proximity_collision_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/entities/entity_route_primitives_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/entities/movement_*.cpp"
)
file(GLOB SHADOWRUN_MODULE_ENTITY_ACTION_COMBAT CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/entities/action_detail_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/entities/entity_action_*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/entities/entity_boundary_*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/entities/entity_close_range_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/entities/entity_combat_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/entities/entity_service_target_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/entities/entity_target_motion_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/entities/entity_transition_conversion.cpp"
)
file(GLOB SHADOWRUN_MODULE_ENTITY_ANIMATION CONFIGURE_DEPENDS
      "${CMAKE_CURRENT_SOURCE_DIR}/src/entities/entity_behavior_action_service_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/entities/entity_behavior_frame_service_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/entities/entity_behavior_transition_completion_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/entities/animation_*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/entities/behavior_animation_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/entities/entity_animation_*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/entities/heading_animation_conversion.cpp"
)
file(GLOB SHADOWRUN_MODULE_ENTITY_EFFECTS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/entities/entity_auxiliary_proximity_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/entities/entity_effect_*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/entities/nearby_effect_conversion.cpp"
)

file(GLOB SHADOWRUN_MODULE_WORLD_SCENE_MODES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/world/post_intro_mode_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/world/world_activity_routine_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/world/world_entity_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/world/world_entry_sequence_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/world/world_frame_logic_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/world/world_mode_routine_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/world/world_resource_batch_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/world/world_scene_entity_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/world/world_sequence_tile_render_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/world/world_tile_setup_conversion.cpp"
)
file(GLOB SHADOWRUN_MODULE_WORLD_EFFECTS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/world/persistent_world_effect_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/world/world_effect_*.cpp"
)

file(GLOB SHADOWRUN_MODULE_MATRIX_NAVIGATION CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/cyberspace/matrix_entry_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/cyberspace/matrix_interaction_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/cyberspace/matrix_loop_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/cyberspace/matrix_node_action_loop_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/cyberspace/matrix_retreat_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/cyberspace/matrix_utility_conversion.cpp"
)
file(GLOB SHADOWRUN_MODULE_MATRIX_PROGRAMS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/cyberspace/matrix_program_*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/cyberspace/matrix_storage_conversion.cpp"
)
file(GLOB SHADOWRUN_MODULE_MATRIX_CYBER_INFO CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/cyberspace/matrix_cyber_info*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/cyberspace/matrix_menu_service_conversion.cpp"
)

file(GLOB SHADOWRUN_MODULE_MENU_INVENTORY_EQUIPMENT CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/menus/armor_selection_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/menus/attributes_skills_menu_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/menus/attribute_skill_upgrade_prompt_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/menus/clips_menu_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/menus/cyberware_menu_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/menus/cyberware_detail_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/menus/equipment_combat_bar_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/menus/first_aid_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/menus/hospital_medical_attention_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/menus/hospital_options_menu_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/menus/hotel_entity_status_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/menus/hotel_options_menu_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/menus/inventory_action_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/menus/magic_menu_conversion.cpp"
)
file(GLOB SHADOWRUN_MODULE_MENU_SERVICES_HUD CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/menus/bitmask_text_batch_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/menus/hud_*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/menus/menu_*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/menus/title_world_action_selection_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/menus/pocket_secretary_menu_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/menus/startup_menu_conversion.cpp"
)

file(GLOB SHADOWRUN_MODULE_INTRO_DIRECTOR_STAGES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/title_intro/boot_sequence_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/title_intro/title_intro_*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/title_intro/title_loop_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/title_intro/title_resource_sequence_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/title_intro/title_reveal_transfer_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/title_intro/title_setup_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/title_intro/title_sprite_layers_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/title_intro/title_sprite_refresh_conversion.cpp"
)
file(GLOB SHADOWRUN_MODULE_INTRO_EFFECTS_WORLD CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/title_intro/title_effect_*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/title_intro/title_noise_effect_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/title_intro/title_profile_*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/title_intro/title_secondary_effect_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/title_intro/title_sweep_animation_conversion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/title_intro/title_world_*.cpp"
)
file(GLOB SHADOWRUN_MODULE_AUDIO_SERVICE CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/audio/*.cpp"
)

set(SHADOWRUN_CONVERSION_MODULE_IDS
    platform_input_timing source_data transfer_streams
    display_text sprite_pipeline
    runner_records roster_save
    field_scene_controller field_navigation field_resource_flow
    field_interaction_actions field_presentation_services field_terrain_collision
    entity_state_projection entity_movement_collision entity_action_combat
    entity_animation entity_effects
    world_scene_modes world_effects
    matrix_navigation matrix_programs matrix_cyber_info
    menu_inventory_equipment menu_services_hud
    intro_director_stages intro_effects_world
    audio_service
)

set(SHADOWRUN_CONVERSION_MODULE_OBJECTS)
foreach(module_id IN LISTS SHADOWRUN_CONVERSION_MODULE_IDS)
    string(TOUPPER "${module_id}" module_key)
    set(module_variable "SHADOWRUN_MODULE_${module_key}")
    set(module_sources "${${module_variable}}")
    if (NOT module_sources)
        message(FATAL_ERROR "Conversion module has no source files: ${module_id}")
    endif()
    add_library(shadowrun_ghidra_module_${module_id} OBJECT ${module_sources})
    target_include_directories(shadowrun_ghidra_module_${module_id} PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include")
    list(APPEND SHADOWRUN_CONVERSION_MODULE_OBJECTS
        $<TARGET_OBJECTS:shadowrun_ghidra_module_${module_id}>)
endforeach()

add_library(shadowrun_ghidra_conversion_modules STATIC
    ${SHADOWRUN_CONVERSION_MODULE_OBJECTS}
)
target_include_directories(shadowrun_ghidra_conversion_modules PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include")
