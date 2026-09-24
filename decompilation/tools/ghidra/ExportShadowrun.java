// Headless export for the one canonical Shadowrun Genesis Ghidra project.
// Run with analyzeHeadless -postScript ExportShadowrun.java <output-dir>.

import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
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
        names.put(0x00001232L, "upload_hud_tile_block");
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
        names.put(0x0000C0DAL, "render_title_text_source");
        names.put(0x000028AAL, "prepare_field_resources");
        names.put(0x00002C6CL, "prepare_field_viewports");
        names.put(0x000033AAL, "draw_field_asset_grid");
        names.put(0x000110BCL, "draw_menu_hud_panel_variant_a");
        names.put(0x000110ECL, "draw_menu_hud_panel_variant_b");
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
        names.put(0x00013CD4L, "guard_entity_action_threshold");
        names.put(0x00013CDEL, "guard_entity_action_threshold_variant");
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
        return names;
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
                                      Path output, TaskMonitor monitor) throws IOException {
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

        Path file = output.resolve("ghidra-c").resolve(fileName(function));
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
        Files.createDirectories(output.resolve("ghidra-c"));
        Program program = currentProgram;
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
                writeFunction(program, function, calls, decompiler, output, monitor);
                long size = function.getBody().getNumAddresses();
                String status = "unknown";
                DecompileResults check = decompiler.decompileFunction(function, 1, monitor);
                if (check.decompileCompleted()) {
                    status = "ok";
                }
                functionTable.write(hex(function.getEntryPoint()) + "\t" +
                                   clean(function.getName()) + "\t" + size + "\t" +
                                   clean(function.getCallingConventionName()) + "\t" +
                                   status + "\tghidra-c/" + fileName(function) + "\n");
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
