// Headless export for the one canonical Shadowrun Genesis Ghidra project.
// Run with analyzeHeadless -postScript ExportShadowrun.java <output-dir>.

import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.address.AddressSet;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.FunctionManager;
import ghidra.program.model.listing.Instruction;
import ghidra.program.model.listing.Listing;
import ghidra.program.model.listing.Program;
import ghidra.program.model.symbol.Reference;
import ghidra.program.model.symbol.ReferenceManager;
import ghidra.program.model.symbol.RefType;
import ghidra.util.task.TaskMonitor;

import java.io.BufferedWriter;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.Comparator;
import java.util.HashSet;
import java.util.List;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Set;

import ghidra.program.model.symbol.SourceType;

public class ExportShadowrun extends GhidraScript {
    // Reviewed names are deliberately kept here, beside the export logic, so
    // every generated artifact uses the same vocabulary.  An address without
    // enough evidence remains Ghidra's FUN_<address> name until it is
    // understood and recorded in the scratchpad.
    private static Map<Long, String> reviewedNames() {
        Map<Long, String> names = new LinkedHashMap<>();
        names.put(0x00000236L, "reset_and_enter_boot");
        names.put(0x00000436L, "write_startup_text_to_vdp");
        names.put(0x0000073AL, "wait_for_vdp_ready");
        names.put(0x0000075EL, "run_game_loop");
        names.put(0x00000BD2L, "wait_for_vblank");
        names.put(0x00000BF2L, "latch_controller_events");
        names.put(0x00000C2CL, "clear_runtime_memory_and_vdp");
        names.put(0x00000D44L, "advance_source_prng");
        names.put(0x00000D3AL, "sample_source_random_bound");
        names.put(0x00000DBAL, "count_thresholded_random_rolls");
        names.put(0x00001088L, "step_palette_toward_black");
        names.put(0x000010D6L, "step_palette_toward_target");
        names.put(0x0000131CL, "step_palette_transition");
        names.put(0x00001342L, "wait_for_palette_frame");
        names.put(0x00001368L, "copy_target_palette");
        names.put(0x00001358L, "copy_source_word_block");
        names.put(0x000014CEL, "extract_even_bytes");
        names.put(0x00001502L, "copy_even_bytes");
        names.put(0x00001610L, "decode_source_byte_stream");
        names.put(0x0000167CL, "decode_source_coordinate_stream");
        names.put(0x00003E86L, "expand_packed_nibbles");
        names.put(0x000041B6L, "fold_field_coordinate");
        names.put(0x00004216L, "offset_field_coordinate");
        names.put(0x0000423CL, "quantize_field_delta");
        names.put(0x00004E38L, "advance_entity_primary_timer");
        names.put(0x00004E64L, "advance_entity_secondary_timer");
        names.put(0x00005A56L, "project_entity_screen_position");
        names.put(0x00005A62L, "project_entity_screen_position_standard");
        names.put(0x00005B1EL, "sort_entity_slots_by_depth");
        names.put(0x00005B7EL, "mark_entity_slots_for_render");
        names.put(0x00005FDCL, "advance_entity_animation_accumulator");
        names.put(0x000061A0L, "append_entity_sprite_record");
        names.put(0x000061CCL, "append_sprite_record");
        names.put(0x0000622CL, "expand_entity_sprite_descriptors");
        names.put(0x0000624CL, "expand_entity_sprite_descriptor_list");
        names.put(0x0000B6E4L, "select_entity_animation_table");
        names.put(0x0000B48CL, "evaluate_entity_collision");
        names.put(0x0000B442L, "reset_entity_movement_state");
        names.put(0x0000B39AL, "set_entity_heading_with_field_animation");
        names.put(0x0000B3FEL, "set_entity_heading_with_alternate_animation");
        names.put(0x0000AF6CL, "reset_entity_animation_state");
        names.put(0x0000ABFCL, "update_entity_animation_delay");
        names.put(0x0000D7EAL, "compute_entity_delay_factor");
        names.put(0x0000D81CL, "average_entity_delay_attributes");
        names.put(0x0000AF7CL, "prepare_entity_delayed_animation_primary");
        names.put(0x0000AFAEL, "prepare_entity_delayed_animation_alternate");
        names.put(0x0000AFF2L, "prepare_entity_auxiliary_behavior");
        names.put(0x0000B330L, "enter_entity_behavior_transition");
        names.put(0x0000B11CL, "synchronize_entity_behavior_spawn");
        names.put(0x0000B1DCL, "configure_entity_target_transition");
        names.put(0x0000B49CL, "resolve_entity_fallback_movement");
        names.put(0x0000BB86L, "evaluate_entity_movement_route");
        names.put(0x0000BADEL, "advance_entity_movement_step");
        names.put(0x0000E8A0L, "sample_terrain_collision_tiles");
        names.put(0x0000E9EEL, "contains_terrain_object_bounds");
        names.put(0x0000EAD2L, "evaluate_terrain_direction_boundary");
        names.put(0x0000EDFAL, "decode_terrain_tile_window");
        names.put(0x0000EE10L, "rebuild_terrain_collision_map");
        names.put(0x0000EE86L, "replace_nth_terrain_marker");
        names.put(0x0000FEBEL, "move_selection_column");
        names.put(0x0000FF06L, "move_selection_row");
        names.put(0x0000C376L, "move_wrapped_menu_cursor");
        names.put(0x0000C8B0L, "apply_byte_counter_delta_with_negative_floor");
        names.put(0x00015CA4L, "compute_diagonal_step_cost");
        names.put(0x00015C58L, "is_entity_target_within_range");
        names.put(0x00015C84L, "update_entity_heading_from_linked_target");
        names.put(0x00015CC0L, "compute_diagonal_step_cost");
        names.put(0x00015D3CL, "measure_relative_heading_and_distance");
        names.put(0x00015D4EL, "measure_heading_and_distance");
        names.put(0x000148D2L, "choose_dominant_axis_directions");
        names.put(0x000148AAL, "find_first_occupied_obstruction_tile");
        names.put(0x00014800L, "probe_obstruction_strip");
        names.put(0x00014854L, "can_bypass_obstruction");
        names.put(0x0001494CL, "is_obstruction_path_blocked");
        names.put(0x00014C56L, "project_heading_position");
        names.put(0x0000E00AL, "wait_for_controller_event");
        names.put(0x0000F312L, "wait_for_controller_event");
        names.put(0x000118C6L, "cycle_inventory_action_selector");
        names.put(0x0000A7E4L, "initialize_entity_slot_order");
        names.put(0x0000AE5EL, "schedule_entity_collision_recheck");
        names.put(0x0001A2CCL, "allocate_first_free_effect_slot");
        names.put(0x0001582AL, "allocate_first_free_effect_slot_local");
        names.put(0x00016AFAL, "upload_mode_word_block");
        names.put(0x00016B7CL, "copy_scene_resource_cursor");
        names.put(0x00016D8AL, "spawn_scene_record_offset");
        names.put(0x000173F2L, "prepare_menu_mode_resources");
        names.put(0x000177CCL, "upload_menu_resource_pair");
        names.put(0x0001789CL, "prepare_menu_hud_sequence");
        names.put(0x000178C8L, "write_menu_hud_grid_batch");
        names.put(0x0001792EL, "render_menu_hud_rows");
        names.put(0x00017BC0L, "refresh_menu_number_or_grid");
        names.put(0x00017C52L, "render_menu_text_followup");
        names.put(0x00017CC8L, "advance_hud_sprite_audio_pulse");
        names.put(0x00017DB8L, "advance_hud_footer_offset");
        names.put(0x00017E04L, "refresh_hud_grid_if_ready");
        names.put(0x00017E78L, "calculate_menu_row_offset");
        names.put(0x00017EB4L, "calculate_menu_delay_offset");
        names.put(0x00017F80L, "refresh_mode_metadata_wrapper");
        names.put(0x00017FB4L, "write_hud_repeated_grid");
        names.put(0x00018776L, "advance_hud_fade_service");
        names.put(0x0001897CL, "write_hud_repeated_grid_variant_a");
        names.put(0x00018998L, "write_hud_repeated_grid_variant_b");
        names.put(0x00016E5CL, "process_terrain_work_entries");
        names.put(0x00016F14L, "mark_terrain_boundary_for_mode");
        names.put(0x00016F3CL, "mark_terrain_boundary_if_inside");
        names.put(0x00016F8EL, "copy_terrain_work_region_rows");
        names.put(0x000177FCL, "run_menu_hud_tick_loop");
        names.put(0x00017998L, "render_menu_text_progression");
        names.put(0x00017EEEL, "advance_menu_overlay_timer");
        names.put(0x00017FDAL, "advance_menu_random_glitch");
        names.put(0x000186FAL, "refresh_field_hud_assets");
        names.put(0x000187AAL, "render_hud_indicator_grids");
        names.put(0x000189B4L, "write_hud_repeated_grid_variant_c");
        names.put(0x000189D8L, "write_two_hud_grids");
        names.put(0x00019432L, "handle_menu_directional_input");
        names.put(0x000194ACL, "advance_hud_selection_row");
        names.put(0x00019504L, "write_hud_panel_grid");
        names.put(0x00019558L, "calculate_matrix_storage_capacity");
        names.put(0x0001957EL, "calculate_loaded_program_capacity");
        names.put(0x000195A6L, "calculate_selected_matrix_program_load_steps");
        names.put(0x000195AAL, "calculate_matrix_program_load_steps");
        names.put(0x000196F2L, "rotate_hud_history");
        names.put(0x0001A342L,
                  "find_first_available_world_effect_entity_slot");
        names.put(0x0001DBAAL, "schedule_world_activity_timeout");
        names.put(0x0001DF00L, "clear_entity_effect_pending_flags");
        names.put(0x0001DFC4L, "advance_world_effect_trigger");
        names.put(0x0001E04AL, "begin_world_effect_transition");
        names.put(0x0001E0B8L, "promote_entity_effect_flags");
        names.put(0x0001E0DCL, "spawn_world_effect_entities");
        names.put(0x0001E25AL, "spawn_world_effect_from_coordinate_sequence");
        names.put(0x0001E4ACL, "spawn_world_effect_at_player_position");
        names.put(0x000044BCL, "initialize_entity_record_from_template");
        names.put(0x00004466L, "spawn_entity_template_list");
        names.put(0x000043B6L, "process_pending_entity_template_lists");
        names.put(0x00004388L, "finalize_pending_entity_template_update");
        names.put(0x0000A800L, "prepare_pending_entity_template_list");
        names.put(0x0005BA98L, "select_pending_entity_template_list");
        names.put(0x0005BA50L, "load_scene_resources");
        names.put(0x0005BA4CL, "load_scene_resources_thunk");
        names.put(0x0005BA66L, "install_scene_resource_index_entries");
        names.put(0x00000C24L, "wait_for_vblank_frames");
        names.put(0x00000D94L, "wait_for_controller_event");
        names.put(0x00000BB6L, "wait_for_even_frame");
        names.put(0x00052DC2L, "begin_controller_bus_access");
        names.put(0x00052DD6L, "end_controller_bus_access");
        names.put(0x00001728L, "render_hud_sprite_stream");
        names.put(0x00003648L, "draw_hud_panel");
        names.put(0x00017D72L, "draw_hud_panel");
        names.put(0x00017D90L, "advance_hud_panel");
        names.put(0x00017F8EL, "draw_hud_footer");
        names.put(0x00019856L, "refresh_field_hud");
        names.put(0x00019BAEL, "select_hud_mode");
        names.put(0x00017708L, "render_field_hud");
        names.put(0x00052E4EL, "begin_audio_bus_access");
        names.put(0x00052E84L, "end_audio_bus_access");
        names.put(0x00052E96L, "flush_audio_command");
        names.put(0x00052EA2L, "append_audio_command");
        names.put(0x00052F3CL, "service_audio_command_queue");
        names.put(0x000530DEL, "service_audio_if_enabled");
        names.put(0x00017D3EL, "advance_audio_service_tick");
        names.put(0x000530EAL, "service_audio_command_frame");
        names.put(0x00003032L, "scene_uses_alternate_profile");
        names.put(0x00002A32L, "initialize_field_mode_profile_a");
        names.put(0x00002A44L, "initialize_field_mode_profile_b");
        names.put(0x00002A94L, "initialize_mode_nine");
        names.put(0x00010918L, "prepare_menu_graphics");
        names.put(0x00010984L, "transfer_menu_asset_batch_nine_optional");
        names.put(0x00010A7EL, "transfer_menu_asset_batch_four_post_prepare");
        names.put(0x00010AF0L, "transfer_menu_asset_batch_four");
        names.put(0x00010B5AL, "transfer_menu_asset_batch_four");
        names.put(0x00010BC4L, "transfer_menu_asset_batch_eight_after_copy");
        names.put(0x00010C96L, "transfer_menu_asset_batch_eight");
        names.put(0x0001A304L, "clear_effect_slot");
        names.put(0x0001A310L, "clear_effect_slots");
        names.put(0x0001A322L, "clear_entity_effect_pending_records");
        names.put(0x00013AF2L, "find_first_entity_action_below_threshold");
        names.put(0x00013A40L, "copy_entity_anchor_position");
        names.put(0x00005622L, "clear_entity_runtime_fields");
        names.put(0x00000E9CL, "make_vdp_transfer_command");
        names.put(0x00000DFCL, "write_repeated_words");
        names.put(0x00000E18L, "write_word_stream");
        names.put(0x00000E34L, "reset_vdp_upload_state");
        names.put(0x00000CECL, "install_startup_vdp_table");
        names.put(0x00000EE6L, "service_pending_vdp_upload");
        names.put(0x00002C92L, "render_normal_field_viewport");
        names.put(0x00002CB2L, "render_alternate_field_viewport");
        names.put(0x000038ACL, "service_entity_ammo");
        names.put(0x000038FAL, "run_entity_animation_triplet");
        names.put(0x000039F2L, "refresh_field_animation_assets");
        names.put(0x00002B34L, "apply_scene_profile_selection");
        names.put(0x0000286EL, "guard_world_effect_player_spawn");
        names.put(0x0000281EL, "initialize_field_scene_entry");
        names.put(0x000028C2L, "reinitialize_field_scene_entry");
        names.put(0x000033BEL, "write_four_by_four_transfer_batch");
        names.put(0x00003548L, "draw_hud_panel_triplet");
        names.put(0x00002BD2L, "advance_field_scene_transition");
        names.put(0x0000334CL, "upload_field_asset_sequence");
        names.put(0x00003404L, "upload_field_asset_setup");
        names.put(0x00059BCCL, "reset_runner_status_triplet");
        names.put(0x00059C34L, "adjust_runner_source_byte_triplet");
        names.put(0x00059F7AL, "reset_title_intro_overlay_schedule");
        names.put(0x00059F8EL, "advance_title_intro_overlay");
        names.put(0x0005A0A4L, "reset_title_intro_sequence");
        names.put(0x0005A0B2L, "advance_title_intro_sequence");
        names.put(0x0005A1EAL, "advance_title_intro_animation");
        names.put(0x0005A244L, "reset_title_intro_persistent_sequence");
        names.put(0x0005A252L, "advance_title_intro_persistent_sequence");
        names.put(0x0005A3B0L, "reset_title_intro_motion");
        names.put(0x0005A3BEL, "advance_title_intro_motion");
        names.put(0x0005A4B4L, "reset_title_intro_dual_sprites");
        names.put(0x0005A4C2L, "advance_title_intro_dual_sprites");
        names.put(0x0005A572L, "reset_title_intro_timed_sprites");
        names.put(0x0005A582L, "begin_title_intro_timed_sprites");
        names.put(0x0005A59CL, "advance_title_intro_timed_sprites");
        names.put(0x0005A61AL, "reset_title_intro_targeted_sweep");
        names.put(0x0005A628L, "advance_title_intro_targeted_sweep");
        names.put(0x0005A70EL, "reset_title_intro_linear_sweep_a");
        names.put(0x0005A71CL, "advance_title_intro_linear_sweep_a");
        names.put(0x0005A7EAL, "reset_title_intro_linear_sweep_b");
        names.put(0x0005A7F8L, "advance_title_intro_linear_sweep_b");
        names.put(0x0005A8C8L, "reset_title_intro_final_projection");
        names.put(0x0005A8DEL, "advance_title_intro_final_projection");
        names.put(0x0005A96AL, "render_title_intro_text_batch");
        names.put(0x0005A9E2L, "render_title_intro_middle_text_batch");
        names.put(0x0005AA1EL, "render_title_intro_center_text_batch");
        names.put(0x0005AA5AL, "render_title_intro_stage_text_batch");
        names.put(0x0005AA96L, "run_title_intro_orchestrator");
        names.put(0x00059C5EL, "run_title_intro_stages");
        names.put(0x00059D12L, "enter_post_intro_mode");
        names.put(0x00059D7CL, "run_title_intro_stage2");
        names.put(0x0005A37CL, "render_title_intro_scene_entities");
        names.put(0x0005AB96L, "advance_title_intro_stage_service_steps");
        names.put(0x0005ABD2L, "service_title_intro_stage_frame");
        names.put(0x0005AC0EL, "draw_title_intro_stage_base_sprite");
        names.put(0x0005AC28L, "reset_title_intro_stage_projection");
        names.put(0x0005AC36L, "advance_title_intro_stage_projection");
        names.put(0x0005AC70L, "reset_title_intro_stage_timed_sprites");
        names.put(0x0005AC7EL, "advance_title_intro_stage_timed_sprites");
        names.put(0x0005AD1AL, "run_title_intro_stage_loop");
        names.put(0x0005ADFEL, "render_title_intro_late_text_batch");
        names.put(0x0005AE3AL, "render_title_intro_scroll_text_batch");
        names.put(0x0005AF2AL, "advance_title_intro_schedule");
        names.put(0x0005AF62L, "run_title_intro_schedule");
        names.put(0x0005B002L, "reset_title_intro_scroll_fill");
        names.put(0x0005B00AL, "advance_title_intro_scroll");
        names.put(0x0005B056L, "advance_title_intro_scroll_fade");
        names.put(0x0005B08EL, "advance_title_intro_triplet");
        names.put(0x0005B0BCL, "service_title_intro_animation");
        names.put(0x0005B100L, "advance_title_intro_pulse");
        names.put(0x0005B198L, "reset_title_intro_landing");
        names.put(0x0005B1B6L, "advance_title_intro_landing");
        names.put(0x0005B29AL, "clear_title_intro_display_buffer");
        names.put(0x0005B32CL, "fill_title_intro_word_block");
        names.put(0x0005B2C2L, "set_title_intro_entity_color_words");
        names.put(0x0005B2E2L, "step_title_intro_entity_color_words");
        names.put(0x00000EC0L, "upload_startup_palette");
        names.put(0x00000F1CL, "write_transfer_header");
        names.put(0x00000F3CL, "write_fixed_transfer_pair");
        names.put(0x00000F98L, "write_vdp_word_block");
        names.put(0x00000F5CL, "write_vdp_word_block_with_source_wrap");
        names.put(0x0000C3B2L, "write_single_word");
        names.put(0x00001184L, "write_masked_vdp_row");
        names.put(0x000011C0L, "write_repeated_vdp_grid");
        names.put(0x000011F6L, "write_linear_vdp_grid");
        names.put(0x000042B0L, "initialize_joshua_archetype_selection");
        names.put(0x000042F2L, "initialize_primary_roster_template");
        names.put(0x0000431CL, "initialize_secondary_roster_template");
        names.put(0x000593FCL, "validate_backup_game_save");
        names.put(0x00059318L, "initialize_save_roster_buffer");
        names.put(0x00059360L, "coordinate_save_load");
        names.put(0x00059372L, "load_save_work_buffer");
        names.put(0x0005961CL, "validate_sram_game_save");
        names.put(0x00059456L, "normalize_sram_save_slots");
        names.put(0x0005966EL, "refresh_runner_record_population");
        names.put(0x00059798L, "serialize_runner_record_population");
        names.put(0x00059786L, "sum_game_save_payload");
        names.put(0x000597EEL, "project_live_runner_record_to_compact");
        names.put(0x00059986L, "select_world_effect_sequence");
        names.put(0x000598B0L, "project_compact_runner_record");
        names.put(0x00001056L, "clear_vdp_words");
        names.put(0x00000FFEL, "clear_and_fill_plane_a");
        names.put(0x0000102AL, "clear_and_fill_plane_b");
        names.put(0x000164DCL, "clear_scene_record_transition_words");
        names.put(0x00016DEEL, "clear_scene_record_pending_flags");
        names.put(0x00016E08L, "all_scene_record_slots_active");
        names.put(0x00016E30L, "snapshot_scene_state");
        names.put(0x00016EA0L, "fill_terrain_work_region");
        names.put(0x00016FA8L, "copy_terrain_work_region");
        names.put(0x000164F2L, "entity_table_has_free_slot");
        names.put(0x0000313AL, "copy_terrain_layers");
        names.put(0x00002FF0L, "find_ready_entity_slot");
        names.put(0x00001232L, "write_vdp_grid");
        names.put(0x0000301EL, "upload_scene_profile_block");
        names.put(0x000176BAL, "refresh_field_resource_streams");
        names.put(0x000176EAL, "render_menu_footer");
        names.put(0x000177A4L, "initialize_menu_slot_states");
        names.put(0x000111D0L, "upload_menu_tile_block");
        names.put(0x00011454L, "reset_menu_upload_cursor_and_upload");
        names.put(0x00017EE4L, "reset_menu_overlay_and_upload_two");
        names.put(0x000136DEL, "initialize_scene_mode_state");
        names.put(0x0001F246L, "run_mode_exit_services");
        names.put(0x0001F292L, "prepare_mode_exit_resources");
        names.put(0x0001F32EL, "prepare_optional_mode_resource");
        names.put(0x0001F34AL, "prepare_mode_resource_variant_b");
        names.put(0x0001F38AL, "prepare_mode_resource_variant_a");
        names.put(0x000566E2L, "refresh_mode_resource_metadata");
        names.put(0x000566F6L, "refresh_mode_resource_state");
        names.put(0x00010952L, "reset_mode_display_buffers");
        names.put(0x0001F3C4L, "run_mode_text_prompt");
        names.put(0x0001F5A8L, "prepare_mode_text_prompt");
        names.put(0x0005670EL, "refresh_mode_palette_data");
        names.put(0x000134D2L, "run_post_intro_mode");
        names.put(0x00013726L, "render_mode_selection");
        names.put(0x00020714L, "finalize_mode_exit");
        names.put(0x0000391AL, "handle_mode_action_event");
        names.put(0x0000E6F4L, "service_mode_world_state");
        names.put(0x00053126L, "prepare_mode_entry_service");
        names.put(0x00001530L, "decode_and_upload_source_stream");
        names.put(0x000012F6L, "advance_title_palette_fade");
        names.put(0x00001910L, "write_alternating_word_pair");
        names.put(0x00012148L, "read_entity_action_code");
        names.put(0x0000C216L, "build_grouped_decimal_glyphs");
        names.put(0x0000C406L, "prepare_numeric_menu_block");
        names.put(0x00014446L, "collect_entity_action_candidates");
        names.put(0x00013E9EL, "reposition_entity_at_boundary");
        names.put(0x0001460EL, "apply_close_range_entity_state");
        names.put(0x00013E8CL, "service_entity_boundary_transition");
        names.put(0x00006148L, "flush_primary_queued_transfers");
        names.put(0x000061F4L, "flush_secondary_queued_transfers");
        names.put(0x00017C98L, "find_hud_selection_index");
        names.put(0x00017C84L, "set_hud_selection_value");
        names.put(0x00017C8EL, "hud_selection_value_matches");
        names.put(0x00001378L, "build_title_sprite_records");
        names.put(0x000013BEL, "build_title_sprite_records_with_attribute_offset");
        names.put(0x00001406L, "prepare_title_resources");
        names.put(0x00001CFCL, "load_title_resources");
        names.put(0x00001D58L, "prepare_title_sprite_list");
        names.put(0x0000C0C2L, "load_title_font");
        names.put(0x0000C0DAL, "render_encoded_title_text");
        names.put(0x000028AAL, "prepare_field_resources");
        names.put(0x00002C6CL, "prepare_field_viewports");
        names.put(0x000033AAL, "draw_field_asset_grid");
        names.put(0x000110BCL, "draw_menu_hud_panel_variant_a");
        names.put(0x000110ECL, "draw_active_runner_hud_panel");
        names.put(0x00011136L, "render_menu_tile_rows");
        names.put(0x00011442L, "reset_and_upload_menu_tile_block");
        names.put(0x00011678L, "write_menu_grid");
        names.put(0x000116C4L, "write_menu_grid_quad");
        names.put(0x00011900L, "prepare_menu_row_grid");
        names.put(0x00011928L, "render_menu_row");
        names.put(0x00011ABEL, "prepare_menu_dialog");
        names.put(0x00012062L, "refresh_menu_text");
        names.put(0x0001209AL, "advance_menu_text_cursor");
        names.put(0x000120E4L, "render_menu_number");
        names.put(0x0001275CL, "clear_menu_slot");
        names.put(0x000127B0L, "refresh_menu_panel");
        names.put(0x00012C1EL, "prepare_menu_grid_variant_a");
        names.put(0x00012C36L, "prepare_menu_grid_variant_b");
        names.put(0x00012C42L, "prepare_menu_grid_variant_c");
        names.put(0x00012C56L, "write_menu_repeated_grid");
        names.put(0x00012C5AL, "write_menu_repeated_grid_with_metadata");
        names.put(0x00012CD6L, "render_menu_text_asset");
        names.put(0x00012E9EL, "service_entity_action_menu");
        names.put(0x00012F50L, "render_entity_action_menu");
        names.put(0x00012FA4L, "advance_entity_action_menu");
        names.put(0x00013466L, "set_entity_action_state");
        names.put(0x00013874L, "dispatch_entity_action_candidates");
        names.put(0x00013AE6L, "advance_entity_action_threshold");
        names.put(0x00013C20L, "select_entity_action_lane");
        names.put(0x00013C74L, "collect_entity_action_records");
        names.put(0x00013CD4L, "evaluate_entity_animation_rate_from_phase");
        names.put(0x00013CDEL, "evaluate_entity_animation_rate");
        names.put(0x000144E8L, "apply_entity_close_range_state");
        names.put(0x00014640L, "map_world_delta_to_tile_pair");
        names.put(0x00015354L, "activate_entity_audio_state");
        names.put(0x0001562EL, "service_entity_auxiliary_behavior");
        names.put(0x00014682L, "select_obstruction_route");
        names.put(0x00014756L, "clear_entity_route_flags");
        names.put(0x00014764L, "find_alternate_route_tile");
        names.put(0x00014AF4L, "find_rotating_route_strip_target");
        names.put(0x00014902L, "compute_obstruction_grid_index");
        names.put(0x00014914L, "is_entity_path_blocked");
        names.put(0x000149C0L, "advance_obstruction_route_queue");
        names.put(0x00014C7CL, "resolve_entity_target_motion");
        names.put(0x00014D58L, "measure_entity_target_heading_and_distance");
        names.put(0x00014D6AL, "select_entity_animation_table_row");
        names.put(0x00014F26L, "probe_entity_world_position");
        names.put(0x00014F50L, "advance_entity_collision_recovery");
        names.put(0x000150E8L, "derive_entity_heading_offsets");
        names.put(0x00014FF8L, "initialize_entity_spawn_record");
        names.put(0x0001514EL, "process_entity_spawn_cycle");
        names.put(0x00015118L, "initialize_entity_behavior_record");
        names.put(0x000152FAL, "prepare_special_spawn_record");
        names.put(0x00015ABAL, "find_colliding_entity");
        names.put(0x00014B4EL, "decode_obstruction_tile_projection");
        names.put(0x00014BA6L, "update_entity_axis_relation_state");
        names.put(0x00014C2EL, "derive_entity_axis_relation_flags");
        names.put(0x00015A8CL, "find_first_available_entity_slot");
        names.put(0x0000B732L, "apply_movement_result_policy");
        names.put(0x0000B77EL, "probe_entity_directional_collision");
        names.put(0x0000B530L, "approach_entity_fixed_point_target");
        names.put(0x0000BBE4L, "commit_entity_position");
        names.put(0x0000BC36L, "build_collision_probe_points");
        names.put(0x0000BCAEL, "apply_mode_five_boundary_motion");
        names.put(0x0000C134L, "normalize_source_text_line");
        names.put(0x0000195CL, "sample_controller_input");
        names.put(0x000019F2L, "read_controller_port");
        names.put(0x00001BACL, "finalize_title_input_sequence");
        names.put(0x00001BEAL, "record_title_input_edge");
        names.put(0x00001C1EL, "advance_title_text_state");
        names.put(0x00001C6AL, "advance_title_sprite_state");
        names.put(0x00001D7AL, "setup_title_state");
        names.put(0x00001DCAL, "draw_title_prompt_variant_a");
        names.put(0x00001DE0L, "draw_title_prompt_variant_b");
        names.put(0x00001DF6L, "draw_title_copyright");
        names.put(0x00001E4EL, "reveal_title_logo_column");
        names.put(0x00002634L, "cycle_title_circuit_lights");
        names.put(0x000026D0L, "advance_title_animation_timer");
        names.put(0x00001A9EL, "run_title_loop");
        names.put(0x00003084L, "run_field_loop");
        names.put(0x00005D746L, "run_pre_title_sequence");
        names.put(0x00005DA50L, "setup_pre_title_resources");
        names.put(0x00005DAC4L, "fill_boot_horizontal_scroll");
        names.put(0x00005D8BAL, "step_pre_title_logo_strobe");
        names.put(0x00005D938L, "advance_pre_title_animation");
        names.put(0x00005D996L, "advance_pre_title_logo_phase");
        names.put(0x00001070L, "service_display_frame_wrapper");
        names.put(0x00001080L, "write_alternating_word_pair_wrapper");
        names.put(0x00001872L, "service_display_frame");
        names.put(0x00003074L, "prepare_field_scene_buffers");
        names.put(0x00003602L, "run_hud_panel_post_hook");
        names.put(0x00003D18L, "render_field_tile_rows");
        names.put(0x00003D4AL, "render_field_tile_column");
        names.put(0x00003F9EL, "calculate_field_tile_source_offset");
        names.put(0x0000ABB6L, "arm_controller_activity_delay");
        names.put(0x0000AC84L, "dispatch_entity_collision_behavior");
        names.put(0x0000AEBAL, "advance_title_sprite_wave");
        names.put(0x0000C3E6L, "write_menu_grid_with_metadata");
        names.put(0x0000C3F8L, "render_menu_grid_number_pair");
        names.put(0x0000CFFEL, "prepare_title_mode_display");
        names.put(0x0000DA4AL, "render_title_text_wrapper");
        names.put(0x0000E660L, "advance_random_grid_effect");
        names.put(0x0000F404L, "write_menu_footer_grid");
        names.put(0x0000FDAEL, "write_menu_grid_quadruple");
        names.put(0x00010156L, "write_repeated_menu_grid");
        names.put(0x00010260L, "prepare_menu_grid_then_service");
        names.put(0x000104C8L, "find_matching_menu_metric");
        names.put(0x000106DAL, "prepare_grid_then_write");
        names.put(0x00010854L, "write_three_grids_then_service");
        names.put(0x00011048L, "upload_resource_then_write_two_blocks");
        names.put(0x0001107EL, "upload_resource_stream");
        names.put(0x000110E8L, "draw_hud_panel_wrapper");
        names.put(0x0001111AL, "render_title_text_source_wrapper");
        names.put(0x0001170CL, "render_title_text_source_leaf");
        names.put(0x00011712L, "render_title_text_source_variant");
        names.put(0x000125FAL, "clear_entity_auxiliary_marker");
        names.put(0x000127FCL, "write_single_grid");
        names.put(0x00012A54L, "update_resource_variant");
        names.put(0x00016396L, "advance_palette_fade");
        names.put(0x0001669CL, "project_entity_x_axis");
        names.put(0x000166D0L, "project_entity_y_axis");
        names.put(0x0001962AL, "apply_menu_slot_command");
        names.put(0x00019736L, "clear_ready_menu_slot_timer");
        names.put(0x00019762L, "clear_menu_status_at");
        names.put(0x00019B34L, "publish_menu_selection_then_text");
        names.put(0x00019DD8L, "refresh_mode_metadata_leaf");
        names.put(0x00019DE8L, "run_menu_service_then_prepare");
        names.put(0x00019F72L, "finalize_menu_slot_code_five");
        names.put(0x00019F78L, "finalize_menu_slot_code_six");
        names.put(0x0001A01EL, "publish_menu_selection_then_text_variant");
        names.put(0x0001A12EL, "advance_menu_countdown");
        names.put(0x0001A154L, "clear_selected_menu_record");
        names.put(0x0001A54EL, "initialize_effect_defaults");
        names.put(0x0001A8F4L, "reset_effect_entity");
        names.put(0x0001AC50L, "arm_effect_after_threshold");
        names.put(0x0001B5A6L, "reset_word_stream_area");
        names.put(0x0001B5B8L, "attach_behavior_descriptor_a");
        names.put(0x0001B612L, "attach_behavior_descriptor_b");
        names.put(0x0001B628L, "attach_behavior_descriptor_c");
        names.put(0x0001B654L, "attach_behavior_descriptor_d");
        names.put(0x0001B680L, "attach_behavior_descriptor_e");
        names.put(0x0001B706L, "prepare_menu_offset_once");
        names.put(0x0001C012L, "copy_seven_words");
        names.put(0x0001C4EEL, "copy_eleven_words");
        names.put(0x0001CA00L, "initialize_world_effect_motion");
        names.put(0x0001CBF8L, "clear_world_effect_work");
        names.put(0x0001D1A4L, "advance_effect_vertical_position");
        names.put(0x0001D1B4L, "retreat_effect_vertical_position");
        names.put(0x0001D1C4L, "advance_effect_horizontal_position");
        names.put(0x0001D1D0L, "retreat_effect_horizontal_position");
        names.put(0x0001E1FCL, "forward_effect_sequence");
        names.put(0x0001E2E0L, "begin_world_activity_if_idle");
        names.put(0x0001E2ECL, "begin_world_activity");
        names.put(0x0001E40CL, "build_world_effect_candidates");
        names.put(0x0001E41EL, "lookup_world_effect_tile");
        names.put(0x0001E514L, "clear_effect_slot_state");
        names.put(0x0001E540L, "initialize_effect_fade_marker");
        names.put(0x0001E68CL, "reset_world_effect_globals");
        names.put(0x0001EEDCL, "dispatch_world_effect");
        names.put(0x0001F1E0L, "advance_eight_step_counter");
        names.put(0x0002196CL, "mark_effect_bit");
        names.put(0x00052DE0L, "initialize_audio_ram");
        names.put(0x00052E26L, "reset_audio_processor");
        names.put(0x00052EB0L, "append_one_with_audio_bus");
        names.put(0x00052EC6L, "append_three_with_audio_bus");
        names.put(0x00052FA0L, "flush_with_audio_bus");
        names.put(0x00052FD2L, "flush_append_pair");
        names.put(0x00053094L, "begin_bus_then_flush_append_pair");
        names.put(0x000530A0L, "controller_access_pair");
        names.put(0x000530D2L, "service_audio_if_enabled_variant");
        names.put(0x00053112L, "service_audio_command_wrapper");
        names.put(0x00053156L, "service_audio_tick_wrapper");
        names.put(0x00002FA4L, "snapshot_entity_anchors");
        names.put(0x00003D80L, "emit_field_tile_column_words");
        names.put(0x00003DDEL, "emit_field_tile_row_words");
        names.put(0x00004618L, "update_entity_render_pass");
        names.put(0x00004B6CL, "reset_effect_record");
        names.put(0x00005468L, "clone_entity_spawn_variant_a");
        names.put(0x0000563EL, "clone_entity_spawn_variant_b");
        names.put(0x000059EAL, "advance_title_sprite_timer");
        names.put(0x00005B98L, "emit_title_sprite_burst");
        names.put(0x000060FAL, "append_entity_render_sprite");
        names.put(0x0000A7B4L, "initialize_entity_runtime");
        names.put(0x0000AAD2L, "dispatch_player_action_if_ready");
        names.put(0x0000AC5EL, "clear_entity_effect_if_forced");
        names.put(0x0000AE6EL, "update_entity_target_heading");
        names.put(0x000101FEL, "select_menu_value");
        names.put(0x00010276L, "run_menu_text_display_sequence");
        names.put(0x0001047EL, "refresh_ready_entity_animations");
        names.put(0x00010FF4L, "write_repeated_asset_batch");
        names.put(0x00011190L, "transfer_menu_glyph_triple");
        names.put(0x0000C160L, "render_wrapped_text");
        names.put(0x0000C436L, "dispatch_menu_handler");
        names.put(0x000163DAL, "service_active_effect_slots");
        names.put(0x000164A8L, "populate_entity_table");
        names.put(0x00016760L, "mark_projected_terrain_cell");
        names.put(0x00016A26L, "publish_world_activity_descriptors");
        names.put(0x00016ECAL, "process_terrain_work_batch");
        names.put(0x000195D8L, "refresh_matrix_program_slot");
        names.put(0x00019AEEL, "refresh_mode_glyph");
        names.put(0x00019BF6L, "select_hud_mode_auxiliary_value");
        names.put(0x00019D66L, "run_menu_slot_service");
        names.put(0x00019DACL, "merge_menu_pending_flags");
        names.put(0x00019DF2L, "run_menu_metadata_service");
        names.put(0x00019E32L, "run_menu_selection_service");
        names.put(0x00019EA6L, "run_menu_selection_variant");
        names.put(0x00019F18L, "run_menu_progression_variant");
        names.put(0x00019F7EL, "clear_first_ready_menu_slot");
        names.put(0x0000600EL, "advance_entity_sprite_descriptor");
        names.put(0x00002900L, "run_world_frame_services");
        names.put(0x00004658L, "update_entity_frame");
        names.put(0x0000481EL, "refresh_projected_entity_sprites");
        names.put(0x000049A2L, "spawn_entity_effect");
        names.put(0x00004ADCL, "advance_entity_effect_timer");
        names.put(0x00004E86L, "advance_effect_animation");
        names.put(0x000053C4L, "run_effect_variant_a");
        names.put(0x00005494L, "advance_jitter_effect");
        names.put(0x0000557EL, "run_effect_variant_b");
        names.put(0x00005662L, "advance_effect_animation_variant");
        names.put(0x00005936L, "advance_entity_animation_effect");
        names.put(0x00005C7EL, "refresh_delayed_effect_sprites");
        names.put(0x000060F4L, "noop_sprite_descriptor_hook");
        names.put(0x0000E7C8L, "evaluate_terrain_collision");
        names.put(0x00010F50L, "prepare_command_overview");
        names.put(0x00011740L, "run_inventory_actions");
        names.put(0x00059092L, "run_start_options");
        names.put(0x00059172L, "run_save_load");
        names.put(0x00000200L, "convert_interrupt_routine_0x00000200");
        names.put(0x00000208L, "convert_interrupt_routine_0x00000208");
        names.put(0x00000210L, "convert_interrupt_routine_0x00000210");
        names.put(0x00000218L, "convert_interrupt_routine_0x00000218");
        names.put(0x00000220L, "convert_interrupt_routine_0x00000220");
        names.put(0x00000228L, "convert_interrupt_routine_0x00000228");
        names.put(0x0000022EL, "convert_interrupt_routine_0x0000022e");
        names.put(0x00000234L, "convert_interrupt_routine_0x00000234");
        names.put(0x00003B1EL, "convert_field_routine_0x00003b1e");
        names.put(0x00003BACL, "convert_field_routine_0x00003bac");
        names.put(0x00003E14L, "convert_field_routine_0x00003e14");
        names.put(0x00003FD8L, "convert_field_routine_0x00003fd8");
        names.put(0x00004000L, "convert_field_routine_0x00004000");
        names.put(0x000040E2L, "convert_field_routine_0x000040e2");
        names.put(0x000041F0L, "convert_field_routine_0x000041f0");
        names.put(0x000049A0L, "convert_field_routine_0x000049a0");
        names.put(0x0000A8EAL, "convert_graphics_routine_0x0000a8ea");
        names.put(0x0000AB02L, "convert_graphics_routine_0x0000ab02");
        names.put(0x0000ACAEL, "convert_graphics_routine_0x0000acae");
        names.put(0x0000B7B8L, "convert_graphics_routine_0x0000b7b8");
        names.put(0x0000C168L, "convert_graphics_routine_0x0000c168");
        names.put(0x0000C64CL, "convert_graphics_routine_0x0000c64c");
        names.put(0x0000C7F8L, "convert_graphics_routine_0x0000c7f8");
        names.put(0x0000C802L, "convert_graphics_routine_0x0000c802");
        names.put(0x0000C928L, "convert_graphics_routine_0x0000c928");
        names.put(0x0000CF38L, "convert_graphics_routine_0x0000cf38");
        names.put(0x0000D4C8L, "convert_graphics_routine_0x0000d4c8");
        names.put(0x0000D53CL, "convert_graphics_routine_0x0000d53c");
        names.put(0x0000D01CL, "run_attributes_and_skills_menu");
        names.put(0x0000D5DCL, "render_cyberware_detail");
        names.put(0x0000D82AL, "render_attributes_and_skills");
        names.put(0x0000EA36L, "convert_graphics_routine_0x0000ea36");
        names.put(0x0000EB92L, "convert_graphics_routine_0x0000eb92");
        names.put(0x0000ECCCL, "convert_graphics_routine_0x0000eccc");
        names.put(0x0000EF08L, "convert_graphics_routine_0x0000ef08");
        names.put(0x0000F5F4L, "convert_graphics_routine_0x0000f5f4");
        names.put(0x0000FDFEL, "convert_graphics_routine_0x0000fdfe");
        names.put(0x0000FF5AL, "run_equipment_selection");
        names.put(0x00010016L, "convert_menu_routine_0x00010016");
        names.put(0x000100A8L, "convert_menu_routine_0x000100a8");
        names.put(0x00010172L, "convert_menu_routine_0x00010172");
        names.put(0x000102D4L, "convert_menu_routine_0x000102d4");
        names.put(0x0001033EL, "resolve_first_aid_item_use");
        names.put(0x000104F6L, "convert_menu_routine_0x000104f6");
        names.put(0x0001055EL, "convert_menu_routine_0x0001055e");
        names.put(0x000105E6L, "convert_menu_routine_0x000105e6");
        names.put(0x00010660L, "convert_menu_routine_0x00010660");
        names.put(0x00010722L, "convert_menu_routine_0x00010722");
        names.put(0x000107CEL, "adjust_runner_posture");
        names.put(0x0000FBA6L, "run_magic_menu");
        names.put(0x0001088CL, "convert_menu_routine_0x0001088c");
        names.put(0x0001098EL, "convert_menu_routine_0x0001098e");
        names.put(0x00010A82L, "convert_menu_routine_0x00010a82");
        names.put(0x00010D30L, "run_command_overview");
        names.put(0x000111E8L, "render_equipment_details");
        names.put(0x00011352L, "apply_armor_selection");
        names.put(0x000114A0L, "convert_menu_routine_0x000114a0");
        names.put(0x00011506L, "convert_menu_routine_0x00011506");
        names.put(0x00011592L, "convert_menu_routine_0x00011592");
        names.put(0x00011624L, "render_equipment_highlights");
        names.put(0x00011950L, "run_cyberware_menu");
        names.put(0x000119CEL, "convert_menu_routine_0x000119ce");
        names.put(0x00011AE4L, "convert_menu_routine_0x00011ae4");
        names.put(0x0001253EL, "convert_menu_routine_0x0001253e");
        names.put(0x000129FEL, "convert_menu_routine_0x000129fe");
        names.put(0x00012A8CL, "render_item_action_details");
        names.put(0x00012C74L, "convert_menu_routine_0x00012c74");
        names.put(0x00012CEEL, "update_equipment_combat_bars");
        names.put(0x00012EAEL, "convert_menu_routine_0x00012eae");
        names.put(0x00013000L, "select_entity_action_detail");
        names.put(0x000130C0L, "build_field_action_detail_plan");
        names.put(0x00013228L, "build_inventory_action_detail_plan");
        names.put(0x000133A6L, "apply_world_effect_reward");
        names.put(0x000138B8L, "run_entity_animation_entry");
        names.put(0x00013904L, "convert_menu_routine_0x00013904");
        names.put(0x00013A92L, "run_entity_animation_branch");
        names.put(0x00013B12L, "run_entity_animation_transition");
        names.put(0x00013BBAL, "check_entity_animation_transition");
        names.put(0x00013D0CL, "run_entity_animation_motion");
        names.put(0x00013F76L, "run_entity_animation_resource_gate");
        names.put(0x00013FFEL, "dispatch_entity_animation_state");
        names.put(0x00014226L, "service_entity_animation_action");
        names.put(0x0001438CL, "collect_entity_action_primary_candidates");
        names.put(0x00014492L, "select_entity_animation_link");
        names.put(0x00014508L, "prepare_entity_animation_fallback_target");
        names.put(0x000145A2L, "advance_entity_animation_target_route");
        names.put(0x00014D8EL, "probe_entity_spawn_collision_window");
        names.put(0x00015554L, "convert_entity_routine_0x00015554");
        names.put(0x00015646L, "convert_entity_routine_0x00015646");
        names.put(0x00015830L, "convert_entity_routine_0x00015830");
        names.put(0x00015930L, "convert_entity_routine_0x00015930");
        names.put(0x0001597EL, "convert_entity_routine_0x0001597e");
        names.put(0x00015BB4L, "convert_entity_routine_0x00015bb4");
        names.put(0x00015DCCL, "transform_entity_heading_vector");
        names.put(0x0001651EL, "convert_world_routine_0x0001651e");
        names.put(0x000167AEL, "convert_world_routine_0x000167ae");
        names.put(0x00016B98L, "convert_world_routine_0x00016b98");
        names.put(0x0001745EL, "convert_world_routine_0x0001745e");
        names.put(0x00017A5EL, "convert_world_routine_0x00017a5e");
        names.put(0x00017C3CL, "convert_world_routine_0x00017c3c");
        names.put(0x000181F8L, "convert_world_routine_0x000181f8");
        names.put(0x0001822CL, "convert_world_routine_0x0001822c");
        names.put(0x0001882EL, "render_matrix_panel");
        names.put(0x00018A92L, "matrix_program_index");
        names.put(0x00018AA6L, "convert_world_routine_0x00018aa6");
        names.put(0x00019238L, "convert_world_routine_0x00019238");
        names.put(0x0001952CL, "matrix_program_storage_usage");
        names.put(0x00019538L, "matrix_program_storage_weight");
        names.put(0x0001954AL, "matrix_loaded_slot_usage");
        names.put(0x00019558L, "calculate_matrix_storage_capacity");
        names.put(0x0001957EL, "calculate_loaded_program_capacity");
        names.put(0x000195A6L, "calculate_selected_matrix_program_load_steps");
        names.put(0x000195AAL, "calculate_matrix_program_load_steps");
        names.put(0x000195D8L, "refresh_matrix_program_slot");
        names.put(0x00019632L, "clear_matrix_program_slot");
        names.put(0x00019788L, "advance_matrix_program_loading");
        names.put(0x0001989AL, "update_hud_mode_transition");
        names.put(0x00019976L, "service_matrix_program_action");
        names.put(0x00019B40L, "select_run_hud_mode");
        names.put(0x00019BF6L, "select_hud_mode_auxiliary_value");
        names.put(0x00019C54L, "read_selected_matrix_program_availability");
        names.put(0x00019C5CL, "read_matrix_program_availability_for_slot");
        names.put(0x00019CB2L, "run_menu_selection_code_four");
        names.put(0x0001A16AL, "convert_world_routine_0x0001a16a");
        names.put(0x0001A370L, "convert_world_routine_0x0001a370");
        names.put(0x0001A4EAL, "lookup_runner_animation_delta");
        names.put(0x0001A538L, "runner_record_has_active_effect");
        names.put(0x0001A5A0L, "convert_world_routine_0x0001a5a0");
        names.put(0x0001A818L, "begin_world_effect_descriptor_transition");
        names.put(0x0001A908L, "initialize_world_effect_mode");
        names.put(0x0001AA40L, "tick_world_effect_animation");
        names.put(0x0001AB6EL, "advance_primary_world_effect_overlay");
        names.put(0x0001AC6AL, "advance_secondary_world_effect_overlay");
        names.put(0x0001AD42L, "advance_background_world_effect_overlay");
        names.put(0x0001ADE6L, "convert_world_routine_0x0001ade6");
        names.put(0x0001B282L, "advance_matrix_program_transition");
        names.put(0x0001B364L, "advance_title_figure_animation");
        names.put(0x0001B45CL, "advance_title_noise_stream");
        names.put(0x0001B7D0L, "initialize_behavior_record");
        names.put(0x0001BA6EL, "advance_title_burst_animation");
        names.put(0x0001BB6AL, "advance_title_noise_effect");
        names.put(0x0001BCE8L, "advance_title_profile_motion");
        names.put(0x0001BEBAL, "advance_title_profile_transition");
        names.put(0x0001BF7AL, "advance_profile_copy_timer");
        names.put(0x0001C3FAL, "advance_title_sprite_layers");
        names.put(0x0001C506L, "render_title_sprite_layers");
        names.put(0x0001C61CL, "advance_title_sweep_animation");
        names.put(0x0001C754L, "convert_world_routine_0x0001c754");
        names.put(0x0001C866L, "advance_title_effect_motion");
        names.put(0x0001CA12L, "advance_world_effect_motion");
        names.put(0x0001CB20L, "advance_world_effect_activation");
        names.put(0x0001CB6AL, "spawn_world_effect_record");
        names.put(0x0001CC24L, "advance_persistent_world_effect");
        names.put(0x0001CD5AL, "convert_world_routine_0x0001cd5a");
        names.put(0x0001CF4AL, "advance_title_effect_sprites");
        names.put(0x0001D048L, "advance_world_effect_record");
        names.put(0x0001D0DEL, "resolve_world_effect_sprite");
        names.put(0x0001D1F4L, "convert_world_routine_0x0001d1f4");
        names.put(0x0001D280L, "initialize_world_sequence");
        names.put(0x0001D33AL, "render_world_sequence_tile_rows");
        names.put(0x0001DC4CL, "select_world_entity_service_target");
        names.put(0x0001DD94L, "advance_world_frame_logic");
        names.put(0x0001DF34L, "resolve_nearby_world_effects");
        names.put(0x0001E166L, "trigger_world_effect_sequence");
        names.put(0x0001E302L, "build_world_effect_edge_candidates");
        names.put(0x0001E43CL, "handle_world_activity_request");
        names.put(0x0001E69CL, "initialize_world_effect_selection");
        names.put(0x0001F5CAL, "run_field_exit_orchestrator");
        names.put(0x0001F65AL, "select_field_exit_prompt");
        names.put(0x0001F830L, "prepare_field_mode_exit");
        names.put(0x0001FA78L, "select_field_mode_service");
        names.put(0x0001FD80L, "finalize_field_mode_exit");
        names.put(0x00020634L, "select_mode_entry_resource");
        names.put(0x000266E4L, "initialize_world_tile_buffers");
        names.put(0x0002679EL, "run_world_entry_sequence");
        names.put(0x00026972L, "refresh_title_sprite_records");
        names.put(0x00026A00L, "run_display_transition");
        names.put(0x000532BCL, "initialize_runner_selection_screen");
        names.put(0x000533EEL, "initialize_runner_animation_grid");
        names.put(0x00053434L, "expand_runner_animation_grid");
        names.put(0x00053704L, "run_runner_selection_screen");
        names.put(0x000537FAL, "refresh_runner_selection_sprites");
        names.put(0x00054662L, "run_mode_action_loop");
        names.put(0x00054A28L, "render_mode_status_summary");
        names.put(0x00054A8EL, "advance_random_mode_event");
        names.put(0x00055640L, "convert_intro_routine_0x00055640");
        names.put(0x000556C6L, "convert_intro_routine_0x000556c6");
        names.put(0x00055740L, "convert_intro_routine_0x00055740");
        names.put(0x00055786L, "convert_intro_routine_0x00055786");
        names.put(0x00055828L, "convert_intro_routine_0x00055828");
        names.put(0x00055848L, "convert_intro_routine_0x00055848");
        names.put(0x00055900L, "convert_intro_routine_0x00055900");
        names.put(0x00055948L, "convert_intro_routine_0x00055948");
        names.put(0x000559BCL, "convert_intro_routine_0x000559bc");
        names.put(0x00055B16L, "convert_intro_routine_0x00055b16");
        names.put(0x00055C76L, "convert_intro_routine_0x00055c76");
        names.put(0x00055E62L, "convert_intro_routine_0x00055e62");
        names.put(0x00055E92L, "convert_intro_routine_0x00055e92");
        names.put(0x00055EB4L, "convert_intro_routine_0x00055eb4");
        names.put(0x00055ED6L, "convert_intro_routine_0x00055ed6");
        names.put(0x00055F02L, "convert_intro_routine_0x00055f02");
        names.put(0x00055F20L, "convert_intro_routine_0x00055f20");
        names.put(0x00055F44L, "convert_intro_routine_0x00055f44");
        names.put(0x00055FD8L, "convert_intro_routine_0x00055fd8");
        names.put(0x00056116L, "convert_intro_routine_0x00056116");
        names.put(0x000561E8L, "convert_intro_routine_0x000561e8");
        names.put(0x000562D4L, "convert_intro_routine_0x000562d4");
        names.put(0x000562DEL, "convert_intro_routine_0x000562de");
        names.put(0x000562ECL, "convert_intro_routine_0x000562ec");
        names.put(0x000563CCL, "convert_intro_routine_0x000563cc");
        names.put(0x00056434L, "convert_intro_routine_0x00056434");
        names.put(0x00056482L, "convert_intro_routine_0x00056482");
        names.put(0x000564D6L, "convert_intro_routine_0x000564d6");
        names.put(0x00056506L, "convert_intro_routine_0x00056506");
        names.put(0x00056632L, "convert_intro_routine_0x00056632");
        names.put(0x0005668CL, "convert_intro_routine_0x0005668c");
        names.put(0x000566A2L, "convert_intro_routine_0x000566a2");
        names.put(0x000566C2L, "convert_intro_routine_0x000566c2");
        names.put(0x000566D8L, "convert_intro_routine_0x000566d8");
        names.put(0x000566ECL, "copy_field_mode_palette_bank");
        names.put(0x0005672CL, "convert_intro_routine_0x0005672c");
        names.put(0x00056AC0L, "run_field_mode_service");
        names.put(0x00056DC4L, "convert_intro_routine_0x00056dc4");
        names.put(0x000575F8L, "convert_intro_routine_0x000575f8");
        names.put(0x00057610L, "convert_intro_routine_0x00057610");
        names.put(0x00057628L, "select_menu_resource_entry");
        names.put(0x0005763CL, "select_menu_resource");
        names.put(0x0005766AL, "apply_field_resource_purchase");
        names.put(0x000577BAL, "apply_field_quantity_purchase");
        names.put(0x0005787CL, "calculate_field_action_quantity_cost");
        names.put(0x0005794EL, "apply_selected_field_action");
        names.put(0x00057A44L, "calculate_field_action_record_cost");
        names.put(0x00057A6EL, "apply_field_action_cost_discounts");
        names.put(0x00057AF6L, "apply_runner_cost_surcharge");
        names.put(0x00057B04L, "calculate_runner_cost_adjustment");
        names.put(0x00057B42L, "apply_large_field_action_cost_discount");
        names.put(0x00057B8CL, "run_menu_confirmation_prompt");
        names.put(0x00057C3AL, "convert_intro_routine_0x00057c3a");
        names.put(0x00057C46L, "convert_intro_routine_0x00057c46");
        names.put(0x00057C52L, "convert_intro_routine_0x00057c52");
        names.put(0x00057C5EL, "refresh_field_action_detail_panel");
        names.put(0x00057C66L, "convert_intro_routine_0x00057c66");
        names.put(0x00057C7AL, "convert_intro_routine_0x00057c7a");
        names.put(0x00057D7EL, "convert_intro_routine_0x00057d7e");
        names.put(0x000580FEL, "convert_intro_routine_0x000580fe");
        names.put(0x0005825CL, "convert_intro_routine_0x0005825c");
        names.put(0x00058316L, "convert_intro_routine_0x00058316");
        names.put(0x000586CAL, "convert_intro_routine_0x000586ca");
        names.put(0x0005878EL, "convert_intro_routine_0x0005878e");
        names.put(0x000587DAL, "render_field_mode_selection_presentation");
        names.put(0x00058A2EL, "render_alternating_field_mode_records");
        names.put(0x00058AA4L, "render_field_mode_glyph_row");
        names.put(0x00058B1EL, "render_field_mode_grid_pair_batches");
        names.put(0x00058B5EL, "refresh_field_mode_panel_grid");
        names.put(0x00058BB6L, "render_field_mode_selection");
        names.put(0x00058C54L, "advance_field_mode_primary_cursor");
        names.put(0x00058CDEL, "advance_field_mode_panel_cursor");
        names.put(0x00058D32L, "refresh_field_mode_selection_sprites");
        names.put(0x00058D7EL, "convert_intro_routine_0x00058d7e");
        names.put(0x000594E0L, "convert_save_routine_0x000594e0");
        names.put(0x0005C8C6L, "convert_intro_routine_0x0005c8c6");
        names.put(0x0005D172L, "convert_intro_routine_0x0005d172");
        names.put(0x0005D1B4L, "convert_intro_routine_0x0005d1b4");
        names.put(0x0005D260L, "convert_intro_routine_0x0005d260");
        names.put(0x0005D29CL, "convert_intro_routine_0x0005d29c");
        names.put(0x0005D3E6L, "convert_intro_routine_0x0005d3e6");
        names.put(0x0005D416L, "convert_intro_routine_0x0005d416");
        names.put(0x0005D55CL, "convert_intro_routine_0x0005d55c");
        names.put(0x0005D57EL, "convert_intro_routine_0x0005d57e");
        names.put(0x0005D5A2L, "convert_intro_routine_0x0005d5a2");
        names.put(0x0005D5FCL, "convert_intro_routine_0x0005d5fc");
        names.put(0x0005D65AL, "convert_intro_routine_0x0005d65a");
        names.put(0x0005D6BCL, "convert_intro_routine_0x0005d6bc");
        names.put(0x0005D886L, "convert_intro_routine_0x0005d886");
        names.put(0x0005D938L, "convert_intro_routine_0x0005d938");
        names.put(0x0005D996L, "convert_intro_routine_0x0005d996");
        names.put(0x0005DAE8L, "convert_intro_routine_0x0005dae8");
        names.put(0x0005DAFCL, "convert_intro_routine_0x0005dafc");
        names.put(0x00000D5AL, "guard_entity_phase_window");
        names.put(0x00002CE0L, "advance_field_mode_transition");
        names.put(0x00002F32L, "initialize_field_scene_state");
        names.put(0x00003168L, "toggle_field_scene_variant");
        names.put(0x00003560L, "refresh_hud_panel_variant");
        names.put(0x00003674L, "advance_entity_delay_meter");
        names.put(0x000036FCL, "advance_entity_asset_meter");
        names.put(0x000037ECL, "advance_entity_behavior_meter");
        names.put(0x00003A1CL, "update_field_camera");
        names.put(0x00003A90L, "accept_camera_delta");
        names.put(0x00003EFEL, "decode_field_tile_sample");
        names.put(0x00004018L, "render_field_tile_edge");
        names.put(0x0001A030L, "select_menu_entry");
        names.put(0x0001A09CL, "refresh_menu_selection_after_clear");
        names.put(0x0001A106L, "refresh_menu_selection_or_prepare");
        names.put(0x0001A468L, "initialize_audio_frame");
        names.put(0x0001A48AL, "advance_title_animation_cycle");
        names.put(0x0001A570L, "build_title_sprites_twice");
        names.put(0x0001A7BEL, "append_title_vertical_sprite");
        names.put(0x0001B1C4L, "initialize_world_effect_seed");
        names.put(0x0001B1FCL, "reset_title_transition");
        names.put(0x0001B218L, "advance_title_phase_cycle");
        names.put(0x0001B302L, "advance_title_jitter");
        names.put(0x0001B87AL, "dispatch_title_phase_handler");
        names.put(0x0001BF5CL, "copy_title_profile");
        names.put(0x0001C0C2L, "advance_alternating_profile");
        names.put(0x0001C1F6L, "advance_audio_menu_countdown");
        names.put(0x0001C9A2L, "initialize_effect_motion_seed");
        names.put(0x0001CAD4L, "update_interaction_state");
        names.put(0x0001CCDCL, "update_persistent_title_effect");
        names.put(0x0001CEFAL, "advance_title_effect_position");
        names.put(0x0001CF36L, "build_title_sprites");
        names.put(0x0001D1DCL, "transfer_world_word_block");
        names.put(0x0001D2E6L, "advance_world_sequence");
        names.put(0x0001D3F4L, "expand_random_nibble_mask");
        names.put(0x0001DBF0L, "reset_entity_service_records");
        names.put(0x0001DC22L, "select_next_entity");
        names.put(0x0001DEA0L, "clear_inactive_entity_effects");
        names.put(0x0001DED8L, "clear_high_effect_slots");
        names.put(0x0001DF1AL, "resolve_nearby_effects");
        names.put(0x0001E216L, "spawn_world_effect_sequence");
        names.put(0x00021A4EL, "subtract_resource_clamped");
        names.put(0x00022874L, "run_fixed_delay");
        names.put(0x00023750L, "count_active_entities");
        names.put(0x00024252L, "service_random_roll");
        names.put(0x00024E20L, "dispatch_indexed_handler");
        names.put(0x00026074L, "clear_grid_segments");
        names.put(0x00026610L, "write_sequential_asset_grid");
        names.put(0x00026696L, "copy_tile_blocks");
        names.put(0x000268E6L, "append_world_sprite_records");
        names.put(0x00026938L, "copy_two_word_rows");
        names.put(0x00026958L, "upload_decoded_source_stream");
        names.put(0x00026BFEL, "write_masked_transfer");
        names.put(0x00026C26L, "write_masked_transfer_with_display_control");
        names.put(0x00052EECL, "initialize_audio_sequence");
        names.put(0x00052F5AL, "run_audio_bus_transaction");
        names.put(0x00052F6EL, "run_audio_transaction_wrapper");
        names.put(0x0005315EL, "run_fixed_audio_transaction_wrapper");
        names.put(0x00053172L, "run_fixed_audio_pair");
        names.put(0x00053194L, "run_controller_access");
        names.put(0x000531AAL, "initialize_title_audio_assets");
        names.put(0x000531EAL, "copy_audio_state");
        names.put(0x00053200L, "select_primary_runner_record_table");
        names.put(0x00053216L, "select_secondary_runner_record_table");
        names.put(0x0005322CL, "prepare_audio_title");
        names.put(0x00053248L, "build_title_sprite_once");
        names.put(0x00053290L, "reset_audio_menu_arrays");
        names.put(0x000532A8L, "fill_mode_auxiliary_sentinel");
        names.put(0x00053A24L, "begin_title_audio_sequence");
        names.put(0x00053A78L, "run_indexed_title_services");
        names.put(0x000549E4L, "write_mode_repeated_grid_service");
        names.put(0x000549F6L, "select_mode_display_status");
        return names;
    }

    private void ensureReviewedInlineFunction(Program program,
                                              long startOffset,
                                              long endExclusive,
                                              String name) throws Exception {
        Address start = program.getAddressFactory().getDefaultAddressSpace()
            .getAddress(startOffset);
        Address end = program.getAddressFactory().getDefaultAddressSpace()
            .getAddress(endExclusive - 1L);
        FunctionManager manager = program.getFunctionManager();
        Function function = manager.getFunctionAt(start);
        Function containing = manager.getFunctionContaining(start);
        if (function == null) {
            if (containing != null) {
                AddressSet containingBody = new AddressSet(containing.getBody());
                containingBody.delete(start, end);
                containing.setBody(containingBody);
            }
            function = manager.createFunction(name, start,
                new AddressSet(start, end), SourceType.USER_DEFINED);
        } else if (!name.equals(function.getName())) {
            function.setName(name, SourceType.USER_DEFINED);
        }
    }

    private void ensureReviewedInlineFunctions(Program program)
            throws Exception {
        // These are entry blocks that Ghidra left inline in their callers.
        // They are real screen loops in the ROM and must be exported as
        // functions so the conversion can name and implement them directly.
        ensureReviewedInlineFunction(program, 0x0000D01CL, 0x0000D4C8L,
                                     "run_attributes_and_skills_menu");
        ensureReviewedInlineFunction(program, 0x0000FBA6L, 0x0000FDAEL,
                                     "run_magic_menu");
        ensureReviewedInlineFunction(program, 0x00011950L, 0x000119CEL,
                                     "run_cyberware_menu");
    }

    private static void applyReviewedNames(Program program) throws Exception {
        for (Map.Entry<Long, String> entry : reviewedNames().entrySet()) {
            Address address = program.getAddressFactory().getDefaultAddressSpace()
                .getAddress(entry.getKey());
            Function function = program.getFunctionManager().getFunctionAt(address);
            if (function != null && !entry.getValue().equals(function.getName())) {
                function.setName(entry.getValue(), SourceType.USER_DEFINED);
            }
        }
    }

    private static String hex(Address address) {
        return String.format("%08X", address.getOffset());
    }

    private static String clean(String value) {
        return value.replace('\t', ' ').replace('\r', ' ').replace('\n', ' ');
    }

    private static String fileName(Function function) {
        return String.format("fn_%08X.cpp", function.getEntryPoint().getOffset());
    }

    private static Set<Long> completedEntries(Path output) throws IOException {
        Set<Long> entries = new HashSet<>();
        Path parent = output.getParent();
        Path manifest = (parent == null ? output : parent)
            .resolve("docs")
            .resolve("completed-ghidra-functions.tsv");
        if (!Files.isRegularFile(manifest)) {
            return entries;
        }
        for (String line : Files.readAllLines(manifest, StandardCharsets.UTF_8)) {
            String trimmed = line.trim();
            if (trimmed.isEmpty() || trimmed.startsWith("#")) {
                continue;
            }
            String token = trimmed.split("\\t", 2)[0].trim();
            if (token.startsWith("0x") || token.startsWith("0X")) {
                token = token.substring(2);
            }
            try {
                entries.add(Long.parseLong(token, 16));
            } catch (NumberFormatException ignored) {
                // Keep malformed manifest rows from preventing source export.
            }
        }
        return entries;
    }

    private static String relativeFunctionPath(Function function,
                                                Set<Long> completedEntries) {
        String directory = completedEntries.contains(
                function.getEntryPoint().getOffset())
                ? "ghidra-c/completed/"
                : "ghidra-c/";
        return directory + fileName(function);
    }

    private static List<Function> calledFunctions(Function function,
                                                   TaskMonitor monitor) throws Exception {
        Set<Function> called = new HashSet<>();
        for (Address address : function.getBody().getAddresses(true)) {
            monitor.checkCanceled();
            Instruction instruction = function.getProgram().getListing()
                .getInstructionAt(address);
            if (instruction == null || !instruction.getFlowType().isCall()) {
                continue;
            }
            for (Address target : instruction.getFlows()) {
                Function callee = function.getProgram().getFunctionManager()
                    .getFunctionAt(target);
                if (callee != null) {
                    called.add(callee);
                }
            }
        }
        List<Function> result = new ArrayList<>(called);
        result.sort(Comparator.comparing(Function::getEntryPoint));
        return result;
    }

    private static String callAnnotation(Function caller, Instruction instruction,
                                         Program program) {
        if (!instruction.getFlowType().isCall()) {
            return "";
        }
        StringBuilder result = new StringBuilder();
        for (Address target : instruction.getFlows()) {
            Function callee = program.getFunctionManager().getFunctionAt(target);
            if (result.length() != 0) {
                result.append(", ");
            }
            result.append("call ").append(hex(target));
            if (callee != null) {
                result.append(" ").append(clean(callee.getName()));
            } else {
                result.append(" [unresolved]");
            }
        }
        return result.toString();
    }

    private static void writeFunction(Program program, Function function,
                                      List<Function> calls, DecompInterface decompiler,
                                      Path output, Set<Long> completedEntries,
                                      TaskMonitor monitor) throws IOException {
        DecompileResults result = decompiler.decompileFunction(function, 120, monitor);
        String body;
        String status;
        if (result.decompileCompleted() && result.getDecompiledFunction() != null) {
            body = result.getDecompiledFunction().getC();
            status = "ok";
        } else {
            body = "/* decompilation failed: " + clean(result.getErrorMessage()) + " */\n";
            status = "failed";
        }

        Path file = output.resolve(relativeFunctionPath(function, completedEntries));
        Files.createDirectories(file.getParent());
        try (BufferedWriter writer = Files.newBufferedWriter(file,
                StandardCharsets.UTF_8)) {
            writer.write("// Ghidra headless export\n");
            writer.write("// Entry: 0x" + hex(function.getEntryPoint()) + "\n");
            writer.write("// Name: " + clean(function.getName()) + "\n");
            writer.write("// Decompile status: " + status + "\n");
            writer.write("// Calls recovered from direct call instructions:\n");
            if (calls.isEmpty()) {
                writer.write("//   (none)\n");
            } else {
                for (Function callee : calls) {
                    writer.write("//   0x" + hex(callee.getEntryPoint()) + " " +
                                 clean(callee.getName()) + "\n");
                }
            }
            writer.write("\n");
            writer.write(body);
            writer.write("\n");
        }
    }

    @Override
    public void run() throws Exception {
        String[] scriptArguments = getScriptArgs();
        if (scriptArguments.length == 0 || scriptArguments[0].trim().isEmpty()) {
            throw new IllegalArgumentException("ExportShadowrun requires an output directory");
        }

        Path output = Paths.get(scriptArguments[0]).toAbsolutePath().normalize();
        Set<Long> completedEntryOffsets = completedEntries(output);
        Files.createDirectories(output.resolve("ghidra-c"));
        if (!completedEntryOffsets.isEmpty()) {
            Files.createDirectories(output.resolve("ghidra-c/completed"));
        }
        Program program = currentProgram;
        ensureReviewedInlineFunctions(program);
        applyReviewedNames(program);
        FunctionManager functions = program.getFunctionManager();
        List<Function> entries = new ArrayList<>();
        for (Function function : functions.getFunctions(true)) {
            entries.add(function);
        }
        entries.sort(Comparator.comparing(Function::getEntryPoint));

        DecompInterface decompiler = new DecompInterface();
        decompiler.setSimplificationStyle("decompile");
        if (!decompiler.openProgram(program)) {
            throw new IOException("could not open program in decompiler");
        }

        try (BufferedWriter functionTable = Files.newBufferedWriter(
                     output.resolve("functions.tsv"), StandardCharsets.UTF_8);
             BufferedWriter callTable = Files.newBufferedWriter(
                     output.resolve("callgraph.tsv"), StandardCharsets.UTF_8);
             BufferedWriter callSiteTable = Files.newBufferedWriter(
                     output.resolve("call-sites.tsv"), StandardCharsets.UTF_8);
             BufferedWriter disassembly = Files.newBufferedWriter(
                     output.resolve("disassembly.s"), StandardCharsets.UTF_8);
             BufferedWriter report = Files.newBufferedWriter(
                     output.resolve("program-report.txt"), StandardCharsets.UTF_8)) {
            functionTable.write("entry\tname\tsize\tcallingConvention\tdecompileStatus\tfile\n");
            callTable.write("caller\tcaller_name\tcallee\tcallee_name\tcall_sites\n");
            callSiteTable.write("caller\tcaller_name\tinstruction\ttext\ttarget\tcallee\tstatus\n");
            report.write("Program: " + program.getName() + "\n");
            report.write("Language: " + program.getLanguageID() + "\n");
            report.write("Compiler: " + program.getCompilerSpec().getCompilerSpecID() + "\n");
            report.write("Image base: " + program.getImageBase() + "\n");
            report.write("Max address: " + program.getMaxAddress() + "\n");
            report.write("Processor: " + program.getLanguage().getProcessor() + "\n");
            report.write("Function count: " + entries.size() + "\n");

            for (Function function : entries) {
                monitor.checkCanceled();
                List<Function> calls = calledFunctions(function, monitor);
                writeFunction(program, function, calls, decompiler, output,
                              completedEntryOffsets, monitor);
                long size = function.getBody().getNumAddresses();
                String status = "unknown";
                DecompileResults check = decompiler.decompileFunction(function, 1, monitor);
                if (check.decompileCompleted()) {
                    status = "ok";
                }
                functionTable.write(hex(function.getEntryPoint()) + "\t" +
                                   clean(function.getName()) + "\t" + size + "\t" +
                                   clean(function.getCallingConventionName()) + "\t" +
                                   status + "\t" +
                                   relativeFunctionPath(function, completedEntryOffsets) +
                                   "\n");
                for (Function callee : calls) {
                    int sites = 0;
                    for (Address address : function.getBody().getAddresses(true)) {
                        Instruction instruction = program.getListing().getInstructionAt(address);
                        if (instruction != null && instruction.getFlowType().isCall()) {
                            for (Address target : instruction.getFlows()) {
                                if (target.equals(callee.getEntryPoint())) {
                                    ++sites;
                                }
                            }
                        }
                    }
                    callTable.write(hex(function.getEntryPoint()) + "\t" +
                                    clean(function.getName()) + "\t" +
                                    hex(callee.getEntryPoint()) + "\t" +
                                    clean(callee.getName()) + "\t" + sites + "\n");
                }
                for (Address address : function.getBody().getAddresses(true)) {
                    Instruction instruction = program.getListing().getInstructionAt(address);
                    if (instruction == null || !instruction.getFlowType().isCall()) {
                        continue;
                    }
                    Address[] targets = instruction.getFlows();
                    if (targets.length == 0) {
                        callSiteTable.write(hex(function.getEntryPoint()) + "\t" +
                                            clean(function.getName()) + "\t" +
                                            hex(address) + "\t" +
                                            clean(instruction.toString()) +
                                            "\t\t\tindirect/unresolved\n");
                        continue;
                    }
                    for (Address target : targets) {
                        Function callee = program.getFunctionManager().getFunctionAt(target);
                        callSiteTable.write(hex(function.getEntryPoint()) + "\t" +
                                            clean(function.getName()) + "\t" +
                                            hex(address) + "\t" +
                                            clean(instruction.toString()) + "\t" +
                                            hex(target) + "\t" +
                                            (callee == null ? "" : clean(callee.getName())) +
                                            "\t" +
                                            (callee == null ? "unresolved" : "resolved") +
                                            "\n");
                    }
                }
            }

            Listing listing = program.getListing();
            ReferenceManager references = program.getReferenceManager();
            for (Instruction instruction : listing.getInstructions(true)) {
                monitor.checkCanceled();
                String annotation = callAnnotation(
                    functions.getFunctionContaining(instruction.getAddress()),
                    instruction, program);
                disassembly.write(hex(instruction.getAddress()) + ": " +
                                  clean(instruction.toString()));
                if (!annotation.isEmpty()) {
                    disassembly.write(" ; " + annotation);
                } else {
                    Reference[] refs = references.getReferencesFrom(instruction.getAddress());
                    if (refs.length != 0 && refs[0].getReferenceType() == RefType.DATA) {
                        disassembly.write(" ; data " + refs[0].getToAddress());
                    }
                }
                disassembly.write("\n");
            }
        } finally {
            decompiler.dispose();
        }
    }
}
