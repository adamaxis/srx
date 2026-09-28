#include "shadowrun_ghidra/core/remaining_routine_catalog_conversion.hpp"

#include <algorithm>

namespace shadowrun::ghidra_conversion {

namespace {

const std::vector<RemainingRoutine> kCatalog{
    {0x00000200U, "unused_stop_stub_2702", RoutineFamily::interrupt, 8U},
    {0x00000208U, "unused_stop_stub_2703", RoutineFamily::interrupt, 8U},
    {0x00000210U, "unused_stop_stub_2704", RoutineFamily::interrupt, 8U},
    {0x00000218U, "unused_stop_stub_2705", RoutineFamily::interrupt, 8U},
    {0x00000220U, "unused_stop_stub_2700", RoutineFamily::interrupt, 8U},
    {0x00000228U, "interrupt_stub_write_alternating_word_pair", RoutineFamily::interrupt, 6U},
    {0x0000022EU, "interrupt_stub_service_display_frame", RoutineFamily::interrupt, 6U},
    {0x00000234U, "return_from_interrupt", RoutineFamily::interrupt, 2U},
    {0x00003B1EU, "classify_field_camera_vertical_delta", RoutineFamily::field, 142U},
    {0x00003BACU, "advance_field_camera_scroll", RoutineFamily::field, 364U},
    {0x00003E14U, "upload_field_tile_row", RoutineFamily::field, 114U},
    {0x00003FD8U, "calculate_field_tile_source_layout", RoutineFamily::field, 40U},
    {0x00004000U, "compose_field_tile_destination_offset", RoutineFamily::field, 24U},
    {0x000040E2U, "render_field_tile_row_edge", RoutineFamily::field, 212U},
    {0x000041F0U, "prepare_field_vertical_origin", RoutineFamily::field, 38U},
    {0x000049A0U, "noop_effect_callback", RoutineFamily::field, 2U},
    {0x0000A8EAU, "service_entity_behavior_frames", RoutineFamily::graphics, 488U},
    {0x0000AB02U, "complete_entity_behavior_transition", RoutineFamily::graphics, 180U},
    {0x0000ACAEU, "advance_entity_behavior_action", RoutineFamily::graphics, 426U},
    {0x0000B7B8U, "advance_entity_movement_and_mode_transition", RoutineFamily::graphics, 772U},
    {0x0000C168U, "render_wrapped_encoded_text", RoutineFamily::graphics, 174U},
    {0x0000C64CU, "finalize_field_mode_exit_transition", RoutineFamily::field, 428U},
    {0x0000C7F8U, "prepare_field_scene_runtime", RoutineFamily::graphics, 10U},
    {0x0000C802U, "refresh_field_runner_state_and_assets", RoutineFamily::field, 174U},
    {0x0000C928U, "run_hospital_options_menu", RoutineFamily::menu, 238U},
    {0x0000CDF6U, "calculate_hospital_medical_attention_cost", RoutineFamily::menu, 134U},
    {0x0000CF38U, "run_hotel_options_menu", RoutineFamily::menu, 198U},
    {0x0000CFFEU, "prepare_hotel_options_display", RoutineFamily::menu, 30U},
    {0x0000D4C8U, "advance_field_active_runner_selection", RoutineFamily::field, 116U},
    {0x0000D53CU, "run_attribute_skill_upgrade_prompt", RoutineFamily::menu, 160U},
    {0x0000D5DCU, "render_attribute_skill_detail_panel", RoutineFamily::menu, 526U},
    {0x0000D82AU, "render_attributes_and_skills", RoutineFamily::graphics, 544U},
    {0x0000DA6AU, "run_hotel_checkout_prompt", RoutineFamily::menu, 188U},
    {0x0000EA36U, "evaluate_entity_terrain_collision_directions", RoutineFamily::graphics, 156U},
    {0x0000EB92U, "resolve_entity_proximity_interactions", RoutineFamily::graphics, 314U},
    {0x0000ECCCU, "resolve_entity_pair_collision_event", RoutineFamily::graphics, 302U},
    {0x0000EF08U, "service_hotel_entity_status", RoutineFamily::menu, 172U},
    {0x0000F5F4U, "render_bitmask_text_batch", RoutineFamily::menu, 232U},
    {0x0000FDFEU, "advance_menu_sprite_overlay", RoutineFamily::menu, 192U},
    {0x0000FF5AU, "run_equipment_selection", RoutineFamily::graphics, 188U},
    {0x00010016U, "run_magic_force_selection_loop", RoutineFamily::menu, 146U},
    {0x000100A8U, "adjust_magic_force", RoutineFamily::menu, 174U},
    {0x00010172U, "refresh_magic_force_display", RoutineFamily::menu, 140U},
    {0x000102D4U, "prepare_magic_action_details", RoutineFamily::menu, 106U},
    {0x0001033EU, "resolve_entity_target_action", RoutineFamily::menu, 320U},
    {0x000104F6U, "apply_action_item_phase_modifiers", RoutineFamily::menu, 104U},
    {0x0001055EU, "refresh_equipment_combat_values", RoutineFamily::menu, 136U},
    {0x000105E6U, "refresh_equipment_action_metric", RoutineFamily::menu, 122U},
    {0x00010660U, "calculate_action_residual", RoutineFamily::menu, 122U},
    {0x00010722U, "run_runner_posture_adjustment", RoutineFamily::menu, 172U},
    {0x000107CEU, "adjust_runner_posture", RoutineFamily::menu, 134U},
    {0x0001088CU, "render_magic_spell_availability", RoutineFamily::menu, 140U},
    {0x0001098EU, "clear_magic_menu_grids", RoutineFamily::menu, 240U},
    {0x00010A82U, "refresh_menu_grid_quadruple", RoutineFamily::menu, 110U},
    {0x00010D30U, "run_command_overview", RoutineFamily::menu, 544U},
    {0x000111E8U, "render_equipment_details", RoutineFamily::menu, 362U},
    {0x00011352U, "apply_armor_selection", RoutineFamily::menu, 240U},
    {0x000114A0U, "advance_command_overview_primary_page", RoutineFamily::menu, 102U},
    {0x00011506U, "advance_command_overview_secondary_page", RoutineFamily::menu, 140U},
    {0x00011592U, "refresh_command_overview_action_details", RoutineFamily::menu, 146U},
    {0x00011624U, "render_equipment_highlights", RoutineFamily::menu, 84U},
    {0x000119CEU, "run_command_overview_action_dialog", RoutineFamily::menu, 240U},
    {0x00011AE4U, "adjust_menu_dialog_posture", RoutineFamily::menu, 116U},
    {0x0001253EU, "reset_inventory_action_target_state", RoutineFamily::menu, 188U},
    {0x000129FEU, "transfer_inventory_action_slot", RoutineFamily::menu, 86U},
    {0x00012A8CU, "render_item_action_details", RoutineFamily::menu, 402U},
    {0x00012C74U, "merge_equipment_action_skill_value", RoutineFamily::menu, 98U},
    {0x00012CEEU, "compute_equipment_combat_bars", RoutineFamily::menu, 432U},
    {0x00012EAEU, "render_equipment_action_detail_row", RoutineFamily::menu, 162U},
    {0x00013000U, "select_entity_action_detail", RoutineFamily::menu, 192U},
    {0x000130C0U, "build_field_action_detail_plan", RoutineFamily::menu, 360U},
    {0x00013228U, "build_inventory_action_detail_plan", RoutineFamily::menu, 382U},
    {0x000133A6U, "apply_world_effect_reward", RoutineFamily::world, 192U},
    {0x000138B8U, "run_entity_animation_entry", RoutineFamily::entity, 76U},
    {0x00013904U, "run_entity_animation_update_coordinator", RoutineFamily::entity, 344U},
    {0x00013A92U, "run_entity_animation_branch", RoutineFamily::entity, 84U},
    {0x00013B12U, "run_entity_animation_transition", RoutineFamily::entity, 168U},
    {0x00013BBAU, "check_entity_animation_transition", RoutineFamily::entity, 102U},
    {0x00013D0CU, "run_entity_animation_motion", RoutineFamily::entity, 292U},
    {0x00013F76U, "run_entity_animation_resource_gate", RoutineFamily::entity, 136U},
    {0x00013FFEU, "dispatch_entity_animation_state", RoutineFamily::entity, 506U},
    {0x00014226U, "service_entity_animation_action", RoutineFamily::entity, 136U},
    {0x0001438CU, "collect_entity_action_primary_candidates", RoutineFamily::entity, 106U},
    {0x00014492U, "select_entity_animation_link", RoutineFamily::entity, 166U},
    {0x00014508U, "prepare_entity_animation_fallback_target", RoutineFamily::entity, 154U},
    {0x000145A2U, "advance_entity_animation_target_route", RoutineFamily::entity, 108U},
    {0x00014D8EU, "probe_entity_spawn_collision_window", RoutineFamily::entity, 124U},
    {0x00015554U, "service_nearby_entity_auxiliary_behavior", RoutineFamily::entity, 218U},
    {0x00015646U, "spawn_entity_effect_record", RoutineFamily::entity, 484U},
    {0x00015830U, "dispatch_entity_effect", RoutineFamily::entity, 256U},
    {0x00015930U, "apply_entity_secondary_effect", RoutineFamily::entity, 78U},
    {0x0001597EU, "scan_entity_effect_targets", RoutineFamily::entity, 206U},
    {0x00015BB4U, "noop_entity_action_hook", RoutineFamily::entity, 2U},
    {0x00015DCCU, "transform_entity_heading_vector", RoutineFamily::entity, 42U},
    {0x0001651EU, "project_entity_world_interaction", RoutineFamily::world, 348U},
    {0x000167AEU, "initialize_world_activity_mode", RoutineFamily::world, 632U},
    {0x00016B98U, "spawn_scene_entity_from_weighted_table", RoutineFamily::world, 346U},
    {0x0001745EU, "run_field_mode_hud_scene", RoutineFamily::world, 604U},
    {0x00017A5EU, "render_menu_slot_status", RoutineFamily::world, 354U},
    {0x00017C3CU, "read_menu_entry_profile_eligibility", RoutineFamily::menu, 22U},
    // The catalog's legacy routing families do not yet expose a cyberspace
    // bucket. Keep the source-backed alias here while the entry is dispatched
    // through the existing world/session boundary; the actual conversion is
    // implemented in the cyberspace domain module.
    {0x000181F8U, "run_matrix_session_entry", RoutineFamily::world, 52U},
    {0x0001822CU, "run_matrix_interaction_loop", RoutineFamily::world, 1230U},
    {0x0001882EU, "render_matrix_panel", RoutineFamily::world, 334U},
    {0x00018A92U, "matrix_program_index", RoutineFamily::world, 20U},
    {0x00018AA6U, "run_cyber_info_screen", RoutineFamily::world, 628U},
    {0x00019238U, "run_matrix_program_slot_screen", RoutineFamily::world, 506U},
    {0x0001952CU, "matrix_program_storage_usage", RoutineFamily::world, 12U},
    {0x00019538U, "matrix_program_storage_weight", RoutineFamily::world, 18U},
    {0x0001954AU, "matrix_loaded_slot_usage", RoutineFamily::world, 14U},
    {0x00019632U, "clear_matrix_program_slot", RoutineFamily::world, 192U},
    {0x00019788U, "advance_matrix_program_loading", RoutineFamily::world, 206U},
    {0x0001989AU, "update_hud_mode_transition", RoutineFamily::world, 220U},
    {0x00019976U, "service_matrix_program_action", RoutineFamily::world, 376U},
    {0x00019B40U, "select_run_hud_mode", RoutineFamily::world, 110U},
    {0x00019C54U, "read_selected_matrix_program_availability", RoutineFamily::world, 8U},
    {0x00019C5CU, "read_matrix_program_availability_for_slot", RoutineFamily::world, 16U},
    {0x00019CB2U, "run_menu_selection_code_four", RoutineFamily::world, 116U},
    {0x0001A16AU, "render_matrix_runner_detail_panel", RoutineFamily::menu, 354U},
    {0x0001A370U, "advance_world_effect_controller", RoutineFamily::world, 350U},
    {0x0001A4EAU, "lookup_runner_animation_delta", RoutineFamily::world, 36U},
    {0x0001A538U, "runner_record_has_active_effect", RoutineFamily::world, 22U},
    {0x0001A5A0U, "advance_world_effect_mode_animation", RoutineFamily::world, 568U},
    {0x0001A818U, "begin_world_effect_descriptor_transition", RoutineFamily::world, 118U},
    {0x0001A908U, "initialize_world_effect_mode", RoutineFamily::world, 198U},
    {0x0001AA40U, "tick_world_effect_animation", RoutineFamily::world, 302U},
    {0x0001AB6EU, "advance_primary_world_effect_overlay", RoutineFamily::world, 226U},
    {0x0001AC6AU, "advance_secondary_world_effect_overlay", RoutineFamily::world, 216U},
    {0x0001AD42U, "advance_background_world_effect_overlay", RoutineFamily::world, 164U},
    {0x0001ADE6U, "run_world_effect_scene_loop", RoutineFamily::world, 990U},
    {0x0001B282U, "advance_matrix_program_transition", RoutineFamily::world, 128U},
    {0x0001B364U, "advance_title_figure_animation", RoutineFamily::world, 248U},
    {0x0001B45CU, "advance_title_noise_stream", RoutineFamily::world, 310U},
    {0x0001B7D0U, "initialize_behavior_record", RoutineFamily::world, 28U},
    {0x0001BA6EU, "advance_title_burst_animation", RoutineFamily::world, 102U},
    {0x0001BB6AU, "advance_title_noise_effect", RoutineFamily::world, 264U},
    {0x0001BCE8U, "advance_title_profile_motion", RoutineFamily::world, 342U},
    {0x0001BEBAU, "advance_title_profile_transition", RoutineFamily::world, 162U},
    {0x0001BF7AU, "advance_profile_copy_timer", RoutineFamily::world, 130U},
    {0x0001C3FAU, "advance_title_sprite_layers", RoutineFamily::world, 152U},
    {0x0001C506U, "render_title_sprite_layers", RoutineFamily::world, 230U},
    {0x0001C61CU, "advance_title_sweep_animation", RoutineFamily::world, 160U},
    {0x0001C754U, "advance_title_world_interaction_service", RoutineFamily::world, 274U},
    {0x0001C866U, "advance_title_effect_motion", RoutineFamily::world, 192U},
    {0x0001CA12U, "advance_world_effect_motion", RoutineFamily::world, 122U},
    {0x0001CB20U, "advance_world_effect_activation", RoutineFamily::world, 144U},
    {0x0001CB6AU, "spawn_world_effect_record", RoutineFamily::world, 142U},
    {0x0001CC24U, "advance_persistent_world_effect", RoutineFamily::world, 184U},
    {0x0001CD5AU, "advance_title_secondary_effect_service", RoutineFamily::world, 416U},
    {0x0001CF4AU, "advance_title_effect_sprites", RoutineFamily::world, 254U},
    {0x0001D048U, "advance_world_effect_record", RoutineFamily::world, 150U},
    {0x0001D0DEU, "resolve_world_effect_sprite", RoutineFamily::world, 62U},
    {0x0001D1F4U, "advance_title_reveal_transfer_service", RoutineFamily::world, 140U},
    {0x0001D280U, "initialize_world_sequence", RoutineFamily::world, 102U},
    {0x0001D33AU, "render_world_sequence_tile_rows", RoutineFamily::world, 186U},
    {0x0001DC4CU, "select_world_entity_service_target", RoutineFamily::world, 328U},
    {0x0001DD94U, "advance_world_frame_logic", RoutineFamily::world, 268U},
    {0x0001DF34U, "resolve_nearby_world_effects", RoutineFamily::world, 144U},
    {0x0001E166U, "trigger_world_effect_sequence", RoutineFamily::world, 150U},
    {0x0001E302U, "build_world_effect_edge_candidates", RoutineFamily::world, 266U},
    {0x0001E43CU, "handle_world_activity_request", RoutineFamily::world, 112U},
    {0x0001E69CU, "initialize_world_effect_selection", RoutineFamily::world, 144U},
    {0x0001F3C4U, "run_mode_text_prompt", RoutineFamily::world, 484U},
    {0x0001F5CAU, "run_field_exit_orchestrator", RoutineFamily::world, 144U},
    {0x0001F65AU, "select_field_exit_prompt", RoutineFamily::world, 470U},
    {0x0001F830U, "prepare_field_mode_exit", RoutineFamily::world, 584U},
    {0x0001FA78U, "select_field_mode_service", RoutineFamily::world, 776U},
    {0x0001FD80U, "finalize_field_mode_exit", RoutineFamily::world, 2228U},
    {0x00020634U, "select_mode_entry_resource", RoutineFamily::world, 224U},
    {0x000266E4U, "initialize_world_tile_buffers", RoutineFamily::world, 186U},
    {0x0002679EU, "run_world_entry_sequence", RoutineFamily::world, 326U},
    {0x00026972U, "refresh_title_sprite_records", RoutineFamily::world, 142U},
    {0x00026A00U, "run_display_transition", RoutineFamily::world, 510U},
    {0x000532BCU, "initialize_runner_selection_screen", RoutineFamily::audio, 306U},
    {0x000533EEU, "initialize_runner_animation_grid", RoutineFamily::world, 70U},
    {0x00053434U, "expand_runner_animation_grid", RoutineFamily::world, 720U},
    {0x00053704U, "run_runner_selection_screen", RoutineFamily::audio, 246U},
    {0x000537FAU, "refresh_runner_selection_sprites", RoutineFamily::audio, 106U},
    {0x00054662U, "run_mode_action_loop", RoutineFamily::world, 904U},
    {0x00054A28U, "render_mode_status_summary", RoutineFamily::world, 102U},
    {0x00054A8EU, "advance_random_mode_event", RoutineFamily::world, 92U},
    {0x00055640U, "resolve_entity_action_pair", RoutineFamily::intro, 134U},
    {0x000556C6U, "check_entity_action_eligibility", RoutineFamily::intro, 122U},
    {0x00055740U, "apply_target_condition_modifier", RoutineFamily::intro, 70U},
    {0x00055786U, "apply_dynamic_status_modifier", RoutineFamily::intro, 162U},
    {0x00055828U, "select_entity_action_profile_row", RoutineFamily::intro, 32U},
    {0x00055848U, "calculate_effect_target_offset", RoutineFamily::intro, 184U},
    {0x00055900U, "resolve_entity_action_status", RoutineFamily::intro, 72U},
    {0x00055948U, "resolve_equipment_action_status", RoutineFamily::intro, 116U},
    {0x000559BCU, "resolve_dynamic_entity_action", RoutineFamily::intro, 346U},
    {0x00055B16U, "resolve_entity_action_with_default_bonus", RoutineFamily::intro, 98U},
    {0x00055C76U, "advance_entity_action_resolution", RoutineFamily::intro, 492U},
    {0x00055E62U, "apply_equipment_defense_action_modifier", RoutineFamily::intro, 48U},
    {0x00055E92U, "subtract_armor_primary_modifier", RoutineFamily::intro, 34U},
    {0x00055EB4U, "subtract_armor_secondary_modifier", RoutineFamily::intro, 34U},
    {0x00055ED6U, "calculate_equipment_command_average", RoutineFamily::intro, 44U},
    {0x00055F02U, "calculate_item18_skill_value", RoutineFamily::intro, 30U},
    {0x00055F20U, "select_posture_adjusted_value", RoutineFamily::intro, 36U},
    {0x00055F44U, "calculate_runner_action_delay", RoutineFamily::entity, 148U},
    {0x00055FD8U, "calculate_and_publish_runner_action_result", RoutineFamily::intro, 38U},
    {0x00056116U, "advance_world_effect_intro_gate", RoutineFamily::intro, 210U},
    {0x000561E8U, "select_world_effect_animation_mode", RoutineFamily::intro, 344U},
    {0x000562D4U, "add_world_effect_timer_modifier", RoutineFamily::intro, 10U},
    {0x000562DEU, "add_world_effect_phase_modifier", RoutineFamily::intro, 14U},
    {0x000562ECU, "resolve_world_effect_transition", RoutineFamily::intro, 224U},
    {0x000563CCU, "advance_primary_world_effect_pending_roll", RoutineFamily::intro, 104U},
    {0x00056434U, "advance_menu_selection_service_timer", RoutineFamily::intro, 78U},
    {0x00056482U, "advance_menu_selection_variant_timer", RoutineFamily::intro, 84U},
    {0x000564D6U, "reduce_menu_progression_timing", RoutineFamily::intro, 48U},
    {0x00056506U, "calculate_menu_hud_row_meter", RoutineFamily::intro, 300U},
    {0x00056632U, "scale_menu_hud_value_by_difficulty", RoutineFamily::intro, 38U},
    {0x0005668CU, "advance_menu_selection_service_and_refresh", RoutineFamily::intro, 22U},
    {0x000566A2U, "copy_action_selection_profile_table", RoutineFamily::intro, 32U},
    {0x000566C2U, "copy_mode_selection_state_table", RoutineFamily::intro, 22U},
    {0x000566D8U, "copy_mode_resource_metadata", RoutineFamily::intro, 10U},
    {0x000566ECU, "copy_field_mode_palette_bank", RoutineFamily::world, 10U},
    {0x0005672CU, "service_field_mode_display_frame", RoutineFamily::intro, 916U},
    {0x00056AC0U, "run_field_mode_service", RoutineFamily::world, 772U},
    {0x00056DC4U, "run_field_action_selection", RoutineFamily::field, 2432U},
    {0x000575F8U, "prepare_field_action_resource_selection", RoutineFamily::field, 24U},
    {0x00057610U, "prepare_field_action_selection_context", RoutineFamily::field, 24U},
    {0x00057628U, "select_menu_resource_entry", RoutineFamily::world, 20U},
    {0x0005763CU, "select_menu_resource", RoutineFamily::world, 46U},
    {0x0005766AU, "apply_field_resource_purchase", RoutineFamily::menu, 336U},
    {0x000577BAU, "apply_field_quantity_purchase", RoutineFamily::menu, 194U},
    {0x0005787CU, "calculate_field_action_quantity_cost", RoutineFamily::menu, 72U},
    {0x0005794EU, "apply_selected_field_action", RoutineFamily::menu, 246U},
    {0x00057A44U, "calculate_field_action_record_cost", RoutineFamily::menu, 42U},
    {0x00057A6EU, "apply_field_action_cost_discounts", RoutineFamily::menu, 136U},
    {0x00057AF6U, "apply_runner_cost_surcharge", RoutineFamily::menu, 14U},
    {0x00057B04U, "calculate_runner_cost_adjustment", RoutineFamily::menu, 62U},
    {0x00057B42U, "apply_large_field_action_cost_discount", RoutineFamily::menu, 74U},
    {0x00057B8CU, "run_menu_confirmation_prompt", RoutineFamily::menu, 174U},
     {0x00057C3AU, "write_field_action_grid_variant_a", RoutineFamily::field, 12U},
     {0x00057C46U, "write_field_action_grid_variant_b", RoutineFamily::field, 12U},
     {0x00057C52U, "write_field_action_grid_variant_c", RoutineFamily::field, 12U},
    {0x00057C5EU, "refresh_field_action_detail_panel", RoutineFamily::menu, 8U},
     {0x00057C66U, "write_field_action_detail_grid", RoutineFamily::field, 20U},
     {0x00057C7AU, "render_field_action_selection", RoutineFamily::field, 1696U},
     {0x00057D7EU, "render_field_action_quantity", RoutineFamily::field, 368U},
     {0x000580FEU, "render_field_action_description", RoutineFamily::field, 264U},
     {0x0005825CU, "render_field_action_cost_summary", RoutineFamily::field, 186U},
     {0x00058316U, "render_field_action_feedback", RoutineFamily::field, 126U},
     {0x000586CAU, "render_field_action_detail", RoutineFamily::field, 196U},
     {0x0005878EU, "render_field_action_detail_text", RoutineFamily::field, 76U},
    {0x000587DAU, "render_field_action_selection_presentation", RoutineFamily::field, 596U},
    {0x00058A2EU, "render_alternating_field_action_selection_records", RoutineFamily::field, 118U},
    {0x00058AA4U, "render_field_action_selection_inventory_glyph_row", RoutineFamily::field, 122U},
    {0x00058B1EU, "render_field_action_selection_grid_pair_batches", RoutineFamily::field, 64U},
    {0x00058B5EU, "refresh_field_action_selection_panel_grid", RoutineFamily::menu, 88U},
    {0x00058BB6U, "render_field_action_selection_panel", RoutineFamily::menu, 158U},
    {0x00058C54U, "advance_field_action_selection_cursor", RoutineFamily::menu, 138U},
    {0x00058CDEU, "advance_field_action_selection_panel", RoutineFamily::menu, 84U},
    {0x00058D32U, "refresh_field_action_selection_sprites", RoutineFamily::menu, 76U},
     {0x00058D7EU, "reset_field_action_cost_state", RoutineFamily::field, 22U},
    {0x000594E0U, "restore_saved_field_state", RoutineFamily::save, 316U},
    {0x0005A37CU, "render_title_intro_scene_entities", RoutineFamily::intro, 28U},
    {0x0005BA4CU, "load_scene_resources_thunk", RoutineFamily::intro, 4U},
    {0x0005C8C6U, "upload_title_or_field_word_stream", RoutineFamily::intro, 52U},
    {0x0005D172U, "prepare_pre_title_audio_variant_a", RoutineFamily::intro, 16U},
    {0x0005D1B4U, "prepare_pre_title_audio_variant_b", RoutineFamily::intro, 46U},
    {0x0005D260U, "reset_world_frame_service_state", RoutineFamily::intro, 46U},
    {0x0005D29CU, "advance_world_scene_entity_effect", RoutineFamily::intro, 344U},
    {0x0005D3E6U, "spawn_world_scene_secondary_effect", RoutineFamily::intro, 34U},
    {0x0005D416U, "advance_world_scene_effect_phase", RoutineFamily::intro, 294U},
    {0x0005D55CU, "advance_world_scene_effect_frame", RoutineFamily::intro, 34U},
    {0x0005D57EU, "refresh_world_scene_effect_sprites", RoutineFamily::intro, 36U},
    {0x0005D5A2U, "advance_world_scene_effect_timer_a", RoutineFamily::intro, 90U},
    {0x0005D5FCU, "advance_world_scene_effect_timer_b", RoutineFamily::intro, 94U},
    {0x0005D65AU, "advance_world_scene_effect_timer_c", RoutineFamily::intro, 98U},
    {0x0005D6BCU, "spawn_scene_special_effect", RoutineFamily::intro, 138U},
    {0x0005D886U, "finalize_pre_title_display", RoutineFamily::intro, 52U},
    {0x0005D938U, "advance_pre_title_animation", RoutineFamily::intro, 94U},
    {0x0005D996U, "advance_pre_title_animation_phase", RoutineFamily::intro, 138U},
    {0x0005DAE8U, "clear_pre_title_resource_workspace", RoutineFamily::intro, 20U},
    {0x0005DAFCU, "write_pre_title_resource_stream", RoutineFamily::intro, 20U},
};

} // namespace

const std::vector<RemainingRoutine>& remaining_routine_catalog() {
    return kCatalog;
}

const RemainingRoutine* find_remaining_routine(const std::uint32_t rom_entry) {
    const auto found = std::find_if(
        kCatalog.begin(), kCatalog.end(),
        [rom_entry](const RemainingRoutine& item) {
            return item.rom_entry == rom_entry;
        });
    return found == kCatalog.end() ? nullptr : &*found;
}

bool record_remaining_routine(
    RemainingRoutineCounters& counters,
    const std::uint32_t rom_entry,
    const RemainingRoutineCallbacks& callbacks) {
    const auto* routine = find_remaining_routine(rom_entry);
    if (!routine) {
        return false;
    }
    ++counters.calls;
    counters.last_entry = rom_entry;
    switch (routine->family) {
    case RoutineFamily::interrupt:
        ++counters.interrupt_calls;
        if (callbacks.on_interrupt) callbacks.on_interrupt(*routine);
        break;
    case RoutineFamily::field:
        ++counters.field_calls;
        if (callbacks.on_field) callbacks.on_field(*routine);
        break;
    case RoutineFamily::menu:
        ++counters.menu_calls;
        if (callbacks.on_menu) callbacks.on_menu(*routine);
        break;
    case RoutineFamily::entity:
        ++counters.entity_calls;
        if (callbacks.on_entity) callbacks.on_entity(*routine);
        break;
    case RoutineFamily::world:
        ++counters.world_calls;
        if (callbacks.on_world) callbacks.on_world(*routine);
        break;
    case RoutineFamily::audio:
        ++counters.audio_calls;
        if (callbacks.on_audio) callbacks.on_audio(*routine);
        break;
    case RoutineFamily::save:
        ++counters.save_calls;
        if (callbacks.on_save) callbacks.on_save(*routine);
        break;
    case RoutineFamily::intro:
        ++counters.intro_calls;
        if (callbacks.on_intro) callbacks.on_intro(*routine);
        break;
    case RoutineFamily::graphics:
        ++counters.graphics_calls;
        if (callbacks.on_graphics) callbacks.on_graphics(*routine);
        break;
    }
    return true;
}

} // namespace shadowrun::ghidra_conversion
