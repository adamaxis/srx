#include "shadowrun_ghidra/remaining_routine_catalog_conversion.hpp"

#include <algorithm>

namespace shadowrun::ghidra_conversion {

namespace {

const std::vector<RemainingRoutine> kCatalog{
    {0x00000200U, "convert_interrupt_routine_0x00000200", RoutineFamily::interrupt, 8U},
    {0x00000208U, "convert_interrupt_routine_0x00000208", RoutineFamily::interrupt, 8U},
    {0x00000210U, "convert_interrupt_routine_0x00000210", RoutineFamily::interrupt, 8U},
    {0x00000218U, "convert_interrupt_routine_0x00000218", RoutineFamily::interrupt, 8U},
    {0x00000220U, "convert_interrupt_routine_0x00000220", RoutineFamily::interrupt, 8U},
    {0x00000228U, "convert_interrupt_routine_0x00000228", RoutineFamily::interrupt, 6U},
    {0x0000022EU, "convert_interrupt_routine_0x0000022e", RoutineFamily::interrupt, 6U},
    {0x00000234U, "convert_interrupt_routine_0x00000234", RoutineFamily::interrupt, 2U},
    {0x00003B1EU, "convert_field_routine_0x00003b1e", RoutineFamily::field, 142U},
    {0x00003BACU, "convert_field_routine_0x00003bac", RoutineFamily::field, 364U},
    {0x00003E14U, "convert_field_routine_0x00003e14", RoutineFamily::field, 114U},
    {0x00003FD8U, "convert_field_routine_0x00003fd8", RoutineFamily::field, 40U},
    {0x00004000U, "convert_field_routine_0x00004000", RoutineFamily::field, 24U},
    {0x000040E2U, "convert_field_routine_0x000040e2", RoutineFamily::field, 212U},
    {0x000041F0U, "convert_field_routine_0x000041f0", RoutineFamily::field, 38U},
    {0x000049A0U, "convert_field_routine_0x000049a0", RoutineFamily::field, 2U},
    {0x0000A8EAU, "convert_graphics_routine_0x0000a8ea", RoutineFamily::graphics, 488U},
    {0x0000AB02U, "convert_graphics_routine_0x0000ab02", RoutineFamily::graphics, 180U},
    {0x0000ACAEU, "convert_graphics_routine_0x0000acae", RoutineFamily::graphics, 426U},
    {0x0000B7B8U, "convert_graphics_routine_0x0000b7b8", RoutineFamily::graphics, 772U},
    {0x0000C168U, "convert_graphics_routine_0x0000c168", RoutineFamily::graphics, 174U},
    {0x0000C64CU, "convert_graphics_routine_0x0000c64c", RoutineFamily::graphics, 428U},
    {0x0000C7F8U, "convert_graphics_routine_0x0000c7f8", RoutineFamily::graphics, 10U},
    {0x0000C802U, "convert_graphics_routine_0x0000c802", RoutineFamily::graphics, 174U},
    {0x0000C928U, "convert_graphics_routine_0x0000c928", RoutineFamily::graphics, 238U},
    {0x0000CF38U, "convert_graphics_routine_0x0000cf38", RoutineFamily::graphics, 198U},
    {0x0000D4C8U, "convert_graphics_routine_0x0000d4c8", RoutineFamily::graphics, 116U},
    {0x0000D53CU, "convert_graphics_routine_0x0000d53c", RoutineFamily::graphics, 160U},
    {0x0000D5DCU, "convert_graphics_routine_0x0000d5dc", RoutineFamily::graphics, 526U},
    {0x0000D82AU, "convert_graphics_routine_0x0000d82a", RoutineFamily::graphics, 544U},
    {0x0000EA36U, "convert_graphics_routine_0x0000ea36", RoutineFamily::graphics, 156U},
    {0x0000EB92U, "convert_graphics_routine_0x0000eb92", RoutineFamily::graphics, 314U},
    {0x0000ECCCU, "convert_graphics_routine_0x0000eccc", RoutineFamily::graphics, 302U},
    {0x0000EF08U, "convert_graphics_routine_0x0000ef08", RoutineFamily::graphics, 172U},
    {0x0000F5F4U, "convert_graphics_routine_0x0000f5f4", RoutineFamily::graphics, 232U},
    {0x0000FDFEU, "convert_graphics_routine_0x0000fdfe", RoutineFamily::graphics, 192U},
    {0x0000FF5AU, "convert_graphics_routine_0x0000ff5a", RoutineFamily::graphics, 188U},
    {0x00010016U, "convert_menu_routine_0x00010016", RoutineFamily::menu, 146U},
    {0x000100A8U, "convert_menu_routine_0x000100a8", RoutineFamily::menu, 174U},
    {0x00010172U, "convert_menu_routine_0x00010172", RoutineFamily::menu, 140U},
    {0x000102D4U, "convert_menu_routine_0x000102d4", RoutineFamily::menu, 106U},
    {0x0001033EU, "resolve_entity_target_action", RoutineFamily::menu, 320U},
    {0x000104F6U, "apply_action_item_phase_modifiers", RoutineFamily::menu, 104U},
    {0x0001055EU, "convert_menu_routine_0x0001055e", RoutineFamily::menu, 136U},
    {0x000105E6U, "convert_menu_routine_0x000105e6", RoutineFamily::menu, 122U},
    {0x00010660U, "calculate_action_residual", RoutineFamily::menu, 122U},
    {0x00010722U, "convert_menu_routine_0x00010722", RoutineFamily::menu, 172U},
    {0x000107CEU, "convert_menu_routine_0x000107ce", RoutineFamily::menu, 134U},
    {0x0001088CU, "convert_menu_routine_0x0001088c", RoutineFamily::menu, 140U},
    {0x0001098EU, "convert_menu_routine_0x0001098e", RoutineFamily::menu, 240U},
    {0x00010A82U, "convert_menu_routine_0x00010a82", RoutineFamily::menu, 110U},
    {0x00010D30U, "convert_menu_routine_0x00010d30", RoutineFamily::menu, 544U},
    {0x000111E8U, "convert_menu_routine_0x000111e8", RoutineFamily::menu, 362U},
    {0x00011352U, "convert_menu_routine_0x00011352", RoutineFamily::menu, 240U},
    {0x000114A0U, "convert_menu_routine_0x000114a0", RoutineFamily::menu, 102U},
    {0x00011506U, "convert_menu_routine_0x00011506", RoutineFamily::menu, 140U},
    {0x00011592U, "convert_menu_routine_0x00011592", RoutineFamily::menu, 146U},
    {0x00011624U, "convert_menu_routine_0x00011624", RoutineFamily::menu, 84U},
    {0x000119CEU, "convert_menu_routine_0x000119ce", RoutineFamily::menu, 240U},
    {0x00011AE4U, "convert_menu_routine_0x00011ae4", RoutineFamily::menu, 116U},
    {0x0001253EU, "convert_menu_routine_0x0001253e", RoutineFamily::menu, 188U},
    {0x000129FEU, "convert_menu_routine_0x000129fe", RoutineFamily::menu, 86U},
    {0x00012A8CU, "convert_menu_routine_0x00012a8c", RoutineFamily::menu, 402U},
    {0x00012C74U, "merge_equipment_action_skill_value", RoutineFamily::menu, 98U},
    {0x00012CEEU, "compute_equipment_combat_bars", RoutineFamily::menu, 432U},
    {0x00012EAEU, "convert_menu_routine_0x00012eae", RoutineFamily::menu, 162U},
    {0x00013000U, "select_entity_action_detail", RoutineFamily::menu, 192U},
    {0x000130C0U, "build_field_action_detail_plan", RoutineFamily::menu, 360U},
    {0x00013228U, "build_inventory_action_detail_plan", RoutineFamily::menu, 382U},
    {0x000133A6U, "apply_world_effect_reward", RoutineFamily::world, 192U},
    {0x000138B8U, "run_entity_animation_entry", RoutineFamily::entity, 76U},
    {0x00013904U, "convert_menu_routine_0x00013904", RoutineFamily::menu, 344U},
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
    {0x00015554U, "convert_entity_routine_0x00015554", RoutineFamily::entity, 218U},
    {0x00015646U, "convert_entity_routine_0x00015646", RoutineFamily::entity, 484U},
    {0x00015830U, "convert_entity_routine_0x00015830", RoutineFamily::entity, 256U},
    {0x00015930U, "convert_entity_routine_0x00015930", RoutineFamily::entity, 78U},
    {0x0001597EU, "convert_entity_routine_0x0001597e", RoutineFamily::entity, 206U},
    {0x00015BB4U, "convert_entity_routine_0x00015bb4", RoutineFamily::entity, 2U},
    {0x00015DCCU, "transform_entity_heading_vector", RoutineFamily::entity, 42U},
    {0x0001651EU, "convert_world_routine_0x0001651e", RoutineFamily::world, 348U},
    {0x000167AEU, "convert_world_routine_0x000167ae", RoutineFamily::world, 632U},
    {0x00016B98U, "convert_world_routine_0x00016b98", RoutineFamily::world, 346U},
    {0x0001745EU, "convert_world_routine_0x0001745e", RoutineFamily::world, 604U},
    {0x00017A5EU, "convert_world_routine_0x00017a5e", RoutineFamily::world, 354U},
    {0x00017C3CU, "read_menu_entry_profile_eligibility", RoutineFamily::menu, 22U},
    {0x000181F8U, "convert_world_routine_0x000181f8", RoutineFamily::world, 52U},
    {0x0001822CU, "convert_world_routine_0x0001822c", RoutineFamily::world, 1230U},
    {0x0001882EU, "convert_world_routine_0x0001882e", RoutineFamily::world, 334U},
    {0x00018A92U, "convert_world_routine_0x00018a92", RoutineFamily::world, 20U},
    {0x00018AA6U, "convert_world_routine_0x00018aa6", RoutineFamily::world, 628U},
    {0x00019238U, "convert_world_routine_0x00019238", RoutineFamily::world, 506U},
    {0x0001952CU, "convert_world_routine_0x0001952c", RoutineFamily::world, 12U},
    {0x00019538U, "convert_world_routine_0x00019538", RoutineFamily::world, 18U},
    {0x0001954AU, "convert_world_routine_0x0001954a", RoutineFamily::world, 14U},
    {0x00019632U, "clear_matrix_program_slot", RoutineFamily::world, 192U},
    {0x00019788U, "advance_matrix_program_loading", RoutineFamily::world, 206U},
    {0x0001989AU, "update_hud_mode_transition", RoutineFamily::world, 220U},
    {0x00019976U, "service_matrix_program_action", RoutineFamily::world, 376U},
    {0x00019B40U, "convert_world_routine_0x00019b40", RoutineFamily::world, 110U},
    {0x00019C54U, "convert_world_routine_0x00019c54", RoutineFamily::world, 8U},
    {0x00019C5CU, "convert_world_routine_0x00019c5c", RoutineFamily::world, 16U},
    {0x00019CB2U, "convert_world_routine_0x00019cb2", RoutineFamily::world, 116U},
    {0x0001A16AU, "convert_world_routine_0x0001a16a", RoutineFamily::world, 354U},
    {0x0001A370U, "convert_world_routine_0x0001a370", RoutineFamily::world, 350U},
    {0x0001A4EAU, "lookup_runner_animation_delta", RoutineFamily::world, 36U},
    {0x0001A538U, "runner_record_has_active_effect", RoutineFamily::world, 22U},
    {0x0001A5A0U, "convert_world_routine_0x0001a5a0", RoutineFamily::world, 568U},
    {0x0001A818U, "begin_world_effect_descriptor_transition", RoutineFamily::world, 118U},
    {0x0001A908U, "initialize_world_effect_mode", RoutineFamily::world, 198U},
    {0x0001AA40U, "tick_world_effect_animation", RoutineFamily::world, 302U},
    {0x0001AB6EU, "advance_primary_world_effect_overlay", RoutineFamily::world, 226U},
    {0x0001AC6AU, "advance_secondary_world_effect_overlay", RoutineFamily::world, 216U},
    {0x0001AD42U, "advance_background_world_effect_overlay", RoutineFamily::world, 164U},
    {0x0001ADE6U, "convert_world_routine_0x0001ade6", RoutineFamily::world, 990U},
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
    {0x0001C754U, "convert_world_routine_0x0001c754", RoutineFamily::world, 274U},
    {0x0001C866U, "advance_title_effect_motion", RoutineFamily::world, 192U},
    {0x0001CA12U, "advance_world_effect_motion", RoutineFamily::world, 122U},
    {0x0001CB20U, "advance_world_effect_activation", RoutineFamily::world, 144U},
    {0x0001CB6AU, "spawn_world_effect_record", RoutineFamily::world, 142U},
    {0x0001CC24U, "advance_persistent_world_effect", RoutineFamily::world, 184U},
    {0x0001CD5AU, "convert_world_routine_0x0001cd5a", RoutineFamily::world, 416U},
    {0x0001CF4AU, "advance_title_effect_sprites", RoutineFamily::world, 254U},
    {0x0001D048U, "advance_world_effect_record", RoutineFamily::world, 150U},
    {0x0001D0DEU, "resolve_world_effect_sprite", RoutineFamily::world, 62U},
    {0x0001D1F4U, "convert_world_routine_0x0001d1f4", RoutineFamily::world, 140U},
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
    {0x00055640U, "convert_intro_routine_0x00055640", RoutineFamily::intro, 134U},
    {0x000556C6U, "convert_intro_routine_0x000556c6", RoutineFamily::intro, 122U},
    {0x00055740U, "apply_target_condition_modifier", RoutineFamily::intro, 70U},
    {0x00055786U, "convert_intro_routine_0x00055786", RoutineFamily::intro, 162U},
    {0x00055828U, "convert_intro_routine_0x00055828", RoutineFamily::intro, 32U},
    {0x00055848U, "convert_intro_routine_0x00055848", RoutineFamily::intro, 184U},
    {0x00055900U, "convert_intro_routine_0x00055900", RoutineFamily::intro, 72U},
    {0x00055948U, "convert_intro_routine_0x00055948", RoutineFamily::intro, 116U},
    {0x000559BCU, "convert_intro_routine_0x000559bc", RoutineFamily::intro, 346U},
    {0x00055B16U, "convert_intro_routine_0x00055b16", RoutineFamily::intro, 98U},
    {0x00055C76U, "convert_intro_routine_0x00055c76", RoutineFamily::intro, 492U},
    {0x00055E62U, "apply_equipment_defense_action_modifier", RoutineFamily::intro, 48U},
    {0x00055E92U, "subtract_armor_primary_modifier", RoutineFamily::intro, 34U},
    {0x00055EB4U, "subtract_armor_secondary_modifier", RoutineFamily::intro, 34U},
    {0x00055ED6U, "calculate_equipment_command_average", RoutineFamily::intro, 44U},
    {0x00055F02U, "calculate_item18_skill_value", RoutineFamily::intro, 30U},
    {0x00055F20U, "select_posture_adjusted_value", RoutineFamily::intro, 36U},
    {0x00055F44U, "convert_intro_routine_0x00055f44", RoutineFamily::intro, 148U},
    {0x00055FD8U, "convert_intro_routine_0x00055fd8", RoutineFamily::intro, 38U},
    {0x00056116U, "convert_intro_routine_0x00056116", RoutineFamily::intro, 210U},
    {0x000561E8U, "convert_intro_routine_0x000561e8", RoutineFamily::intro, 344U},
    {0x000562D4U, "convert_intro_routine_0x000562d4", RoutineFamily::intro, 10U},
    {0x000562DEU, "convert_intro_routine_0x000562de", RoutineFamily::intro, 14U},
    {0x000562ECU, "convert_intro_routine_0x000562ec", RoutineFamily::intro, 224U},
    {0x000563CCU, "convert_intro_routine_0x000563cc", RoutineFamily::intro, 104U},
    {0x00056434U, "convert_intro_routine_0x00056434", RoutineFamily::intro, 78U},
    {0x00056482U, "convert_intro_routine_0x00056482", RoutineFamily::intro, 84U},
    {0x000564D6U, "convert_intro_routine_0x000564d6", RoutineFamily::intro, 48U},
    {0x00056506U, "convert_intro_routine_0x00056506", RoutineFamily::intro, 300U},
    {0x00056632U, "convert_intro_routine_0x00056632", RoutineFamily::intro, 38U},
    {0x0005668CU, "convert_intro_routine_0x0005668c", RoutineFamily::intro, 22U},
    {0x000566A2U, "convert_intro_routine_0x000566a2", RoutineFamily::intro, 32U},
    {0x000566C2U, "convert_intro_routine_0x000566c2", RoutineFamily::intro, 22U},
    {0x000566D8U, "convert_intro_routine_0x000566d8", RoutineFamily::intro, 10U},
    {0x000566ECU, "copy_field_mode_palette_bank", RoutineFamily::world, 10U},
    {0x0005672CU, "convert_intro_routine_0x0005672c", RoutineFamily::intro, 916U},
    {0x00056AC0U, "run_field_mode_service", RoutineFamily::world, 772U},
    {0x00056DC4U, "convert_intro_routine_0x00056dc4", RoutineFamily::intro, 2432U},
    {0x000575F8U, "convert_intro_routine_0x000575f8", RoutineFamily::intro, 24U},
    {0x00057610U, "convert_intro_routine_0x00057610", RoutineFamily::intro, 24U},
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
    {0x00057C3AU, "convert_intro_routine_0x00057c3a", RoutineFamily::intro, 12U},
    {0x00057C46U, "convert_intro_routine_0x00057c46", RoutineFamily::intro, 12U},
    {0x00057C52U, "convert_intro_routine_0x00057c52", RoutineFamily::intro, 12U},
    {0x00057C5EU, "refresh_field_action_detail_panel", RoutineFamily::menu, 8U},
    {0x00057C66U, "convert_intro_routine_0x00057c66", RoutineFamily::intro, 20U},
    {0x00057C7AU, "convert_intro_routine_0x00057c7a", RoutineFamily::intro, 1696U},
    {0x00057D7EU, "convert_intro_routine_0x00057d7e", RoutineFamily::intro, 368U},
    {0x000580FEU, "convert_intro_routine_0x000580fe", RoutineFamily::intro, 264U},
    {0x0005825CU, "convert_intro_routine_0x0005825c", RoutineFamily::intro, 186U},
    {0x00058316U, "convert_intro_routine_0x00058316", RoutineFamily::intro, 126U},
    {0x000586CAU, "convert_intro_routine_0x000586ca", RoutineFamily::intro, 196U},
    {0x0005878EU, "convert_intro_routine_0x0005878e", RoutineFamily::intro, 76U},
    {0x000587DAU, "render_field_mode_selection_presentation", RoutineFamily::world, 596U},
    {0x00058A2EU, "render_alternating_field_mode_records", RoutineFamily::world, 118U},
    {0x00058AA4U, "render_field_mode_glyph_row", RoutineFamily::world, 122U},
    {0x00058B1EU, "render_field_mode_grid_pair_batches", RoutineFamily::world, 64U},
    {0x00058B5EU, "refresh_field_mode_panel_grid", RoutineFamily::menu, 88U},
    {0x00058BB6U, "render_field_mode_selection", RoutineFamily::menu, 158U},
    {0x00058C54U, "advance_field_mode_primary_cursor", RoutineFamily::menu, 138U},
    {0x00058CDEU, "advance_field_mode_panel_cursor", RoutineFamily::menu, 84U},
    {0x00058D32U, "refresh_field_mode_selection_sprites", RoutineFamily::menu, 76U},
    {0x00058D7EU, "convert_intro_routine_0x00058d7e", RoutineFamily::intro, 22U},
    {0x000594E0U, "convert_save_routine_0x000594e0", RoutineFamily::save, 316U},
    {0x0005A37CU, "render_title_intro_scene_entities", RoutineFamily::intro, 28U},
    {0x0005BA4CU, "load_scene_resources_thunk", RoutineFamily::intro, 4U},
    {0x0005C8C6U, "convert_intro_routine_0x0005c8c6", RoutineFamily::intro, 52U},
    {0x0005D172U, "convert_intro_routine_0x0005d172", RoutineFamily::intro, 16U},
    {0x0005D1B4U, "convert_intro_routine_0x0005d1b4", RoutineFamily::intro, 46U},
    {0x0005D260U, "convert_intro_routine_0x0005d260", RoutineFamily::intro, 46U},
    {0x0005D29CU, "convert_intro_routine_0x0005d29c", RoutineFamily::intro, 344U},
    {0x0005D3E6U, "convert_intro_routine_0x0005d3e6", RoutineFamily::intro, 34U},
    {0x0005D416U, "convert_intro_routine_0x0005d416", RoutineFamily::intro, 294U},
    {0x0005D55CU, "convert_intro_routine_0x0005d55c", RoutineFamily::intro, 34U},
    {0x0005D57EU, "convert_intro_routine_0x0005d57e", RoutineFamily::intro, 36U},
    {0x0005D5A2U, "convert_intro_routine_0x0005d5a2", RoutineFamily::intro, 90U},
    {0x0005D5FCU, "convert_intro_routine_0x0005d5fc", RoutineFamily::intro, 94U},
    {0x0005D65AU, "convert_intro_routine_0x0005d65a", RoutineFamily::intro, 98U},
    {0x0005D6BCU, "convert_intro_routine_0x0005d6bc", RoutineFamily::intro, 138U},
    {0x0005D886U, "convert_intro_routine_0x0005d886", RoutineFamily::intro, 52U},
    {0x0005D938U, "convert_intro_routine_0x0005d938", RoutineFamily::intro, 94U},
    {0x0005D996U, "convert_intro_routine_0x0005d996", RoutineFamily::intro, 138U},
    {0x0005DAE8U, "convert_intro_routine_0x0005dae8", RoutineFamily::intro, 20U},
    {0x0005DAFCU, "convert_intro_routine_0x0005dafc", RoutineFamily::intro, 20U},
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
