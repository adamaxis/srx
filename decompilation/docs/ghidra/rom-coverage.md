# ROM coverage

This is a structural source map, not a claim of behavioral ROM fidelity. A mapped row has an explicit Lua/C++ source file or symbol in the committed mapping table, or a documented replacement entry. Runtime equivalence still requires behavior-specific ROM traces and measurements.

The 66-entry historical B2 triage is recorded in [rom-coverage-triage.md](rom-coverage-triage.md) and [rom-coverage-triage.tsv](rom-coverage-triage.tsv). That ledger distinguishes port owners, host or asset replacements, and routines filed as missing bugs.

Inventory: the union of `decompilation/docs/ghidra/decompiled-function-status.tsv` and `decompilation/export/functions.tsv`. The status ledger has 1083 rows; after address deduplication and including any export-only entries, the inventory contains **1080 routine entries** (911 unique Ghidra function exports). Ghidra names and sizes take precedence where the export list has the same address.

Mapped: **726**. Unmapped backlog: **354**. Committed mapped-routine baseline: **726**.

| Inventory category | Entries | Mapped | Unmapped |
|---|---:|---:|---:|
| Ghidra code function exports | 911 | 562 | 349 |
| Status-ledger-only addresses | 169 | 164 | 5 |
| Combined unique addresses | 1080 | 726 | 354 |

The `shadowrun_rom_coverage` CTest validates the current mapping references and fails if a routine in the baseline loses its mapping. Existing unmapped entries are reported but do not fail CTest. Run `python3 decompilation/ghidra_conversion/tools/rom_coverage.py --write-report` after updating the map.

## Status ledger counts

| Status | Routines |
|---|---:|
| defended | 17 |
| partial | 1062 |
| unclassified | 1 |

## Status ledger address conflicts

These duplicate labels are retained as provenance warnings. Where a Ghidra function name exists at the address, its matching status row supplies the mapping owner.

- 0x36FC: Ghidra names advance_entity_asset_meter; ignored conflicting status label field HUD ammo counter and clip meter update
- 0x3D80: Ghidra names emit_field_tile_column_words; ignored conflicting status label field map plane priority writer
- 0xF5F4: Ghidra names render_bitmask_text_batch; ignored conflicting status label Notebook shared flag-filtered record-page builder

## Largest unmapped routines

Code size comes from the Ghidra export list; only exported routines have a size here.

| ROM entry | Ghidra name | Size (bytes) | Subsystem |
|---|---|---:|---|
| `0x56DC4` | run_field_action_selection | 2432 | run_field_action_selection |
| `0x57C7A` | render_field_action_selection | 1696 | render_field_action_selection |
| `0x1822C` | run_matrix_interaction_loop | 1230 | Matrix interaction loop |
| `0x1ADE6` | run_world_effect_scene_loop | 990 | world-effect scene driver |
| `0x5672C` | service_field_mode_display_frame | 916 | service_field_mode_display_frame |
| `0x54662` | run_mode_action_loop | 904 | Matrix node-function action loop |
| `0x53434` | expand_runner_animation_grid | 720 | Runner animation-grid guarded expansion |
| `0x18AA6` | run_cyber_info_screen | 628 | CyberInfo screen |
| `0x1745E` | run_field_mode_hud_scene | 604 | field/menu mode orchestrator |
| `0x587DA` | render_field_mode_selection_presentation | 596 | Field-action selection presentation dispatcher |
| `0x1F830` | prepare_field_mode_exit | 584 | Field mode exit policy |
| `0x1A5A0` | advance_world_effect_mode_animation | 568 | World-effect mode animation |
| `0x26A00` | run_display_transition | 510 | Display transition choreography |
| `0x19238` | run_matrix_program_slot_screen | 506 | Matrix program slot screen |
| `0x15646` | spawn_entity_effect_record | 484 | short-lived effect/projectile entity constructor |
| `0x1F3C4` | run_mode_text_prompt | 484 | run mode text prompt |
| `0xACAE` | advance_entity_behavior_action | 426 | advance_entity_behavior_action |
| `0x1CD5A` | advance_title_secondary_effect_service | 416 | secondary title/world effect service |
| `0x19976` | service_matrix_program_action | 376 | Matrix program-action service |
| `0x17FDA` | advance_menu_random_glitch | 374 | menu random visual effect |

## Largest unmapped subsystem groups

Groups are keyword buckets derived from the Ghidra name and status-ledger subsystem text, for triage only.

| Group | Unmapped entries (combined) |
|---|---:|
| Core and shared | 82 |
| Menus and dialogue | 80 |
| Field and world | 78 |
| Actors and movement | 70 |
| Opening and story | 25 |
| Presentation and audio | 16 |
| Combat and magic | 3 |

| Group | Unmapped Ghidra code routines |
|---|---:|
| Core and shared | 81 |
| Field and world | 78 |
| Menus and dialogue | 76 |
| Actors and movement | 70 |
| Opening and story | 25 |
| Presentation and audio | 16 |
| Combat and magic | 3 |

## Mapped routines

| ROM entry | Routine | Size (bytes) | Port implementation | Mapping kind |
|---|---|---:|---|---|
| `0x0200` | unused_stop_stub_2702 | 8 | decompilation/ghidra_conversion/qt_runtime/src/app/ghidra_conversion_qt_main.cpp | replaced |
| `0x0208` | unused_stop_stub_2703 | 8 | decompilation/ghidra_conversion/qt_runtime/src/app/ghidra_conversion_qt_main.cpp | replaced |
| `0x0210` | unused_stop_stub_2704 | 8 | decompilation/ghidra_conversion/qt_runtime/src/app/ghidra_conversion_qt_main.cpp | replaced |
| `0x0218` | unused_stop_stub_2705 | 8 | decompilation/ghidra_conversion/qt_runtime/src/app/ghidra_conversion_qt_main.cpp | replaced |
| `0x0220` | unused_stop_stub_2700 | 8 | decompilation/ghidra_conversion/qt_runtime/src/app/ghidra_conversion_qt_main.cpp | replaced |
| `0x0228` | interrupt_stub_write_alternating_word_pair | 6 | decompilation/ghidra_conversion/qt_runtime/src/app/ghidra_conversion_qt_main.cpp | replaced |
| `0x022E` | interrupt_stub_service_display_frame | 6 | decompilation/ghidra_conversion/qt_runtime/src/app/ghidra_conversion_qt_main.cpp | replaced |
| `0x0234` | return_from_interrupt | 2 | decompilation/ghidra_conversion/qt_runtime/src/app/ghidra_conversion_qt_main.cpp | replaced |
| `0x0236` | reset_and_enter_boot | 520 | decompilation/ghidra_conversion/src/title_intro/boot_sequence_conversion.cpp | port |
| `0x0330` | reset display setup | — | decompilation/ghidra_conversion/qt_runtime/src/app/ghidra_conversion_qt_window.hpp | replaced |
| `0x073A` | wait_for_vdp_ready | 36 | decompilation/ghidra_conversion/src/core/timing_conversion.cpp | port |
| `0x075E` | run_game_loop | 1106 | decompilation/ghidra_conversion/src/integration/field_session_bootstrap_conversion.cpp#ghidra_conversion::integration::bootstrap_field_session | port |
| `0x0BB6` | wait_for_even_frame | 28 | decompilation/ghidra_conversion/src/core/timing_conversion.cpp#ghidra_conversion::wait_for_even_frame | port |
| `0x0BD2` | wait_for_vblank | 32 | decompilation/ghidra_conversion/src/core/timing_conversion.cpp | port |
| `0x0BF2` | latch_controller_events | 50 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp#ghidra_conversion::latch_controller_events | port |
| `0x0C24` | wait_for_vblank_frames | 8 | decompilation/ghidra_conversion/src/audio/audio_command_batch_conversion.cpp#ghidra_conversion::wait_for_vblank_frames | port |
| `0x0D3A` | sample_source_random_bound | 10 | decompilation/ghidra_conversion/qt_runtime/src/field/field_interaction_runtime.cpp#ghidra_conversion::sample_source_random_bound | port |
| `0x0D44` | advance_source_prng | 22 | decompilation/ghidra_conversion/src/core/random_conversion.cpp#ghidra_conversion::next_source_random_word | port |
| `0x0D5A` | guard_entity_phase_window | 58 | decompilation/ghidra_conversion/src/entities/entity_action_math_conversion.cpp#ghidra_conversion::add_wound_difficulty_penalty | port |
| `0x0D94` | wait_for_controller_event | 38 | decompilation/ghidra_conversion/src/core/controller_wait_conversion.cpp#ghidra_conversion::wait_for_controller_event | port |
| `0x0DBA` | count_thresholded_random_rolls | 66 | decompilation/ghidra_conversion/qt_runtime/src/field/field_interaction_runtime.cpp#ghidra_conversion::count_thresholded_random_rolls | port |
| `0x0DFC` | write_repeated_words | 28 | decompilation/ghidra_conversion/src/core/word_transfer_conversion.cpp#ghidra_conversion::write_repeated_words | port |
| `0x0E18` | write_word_stream | 28 | decompilation/ghidra_conversion/src/core/word_transfer_conversion.cpp#ghidra_conversion::write_word_stream | port |
| `0x0E34` | reset_vdp_upload_state | 72 | decompilation/ghidra_conversion/src/menus/menu_service_sequence_conversion.cpp#ghidra_conversion::reset_vdp_upload_state | port |
| `0x0E9C` | make_vdp_transfer_command | 36 | decompilation/ghidra_conversion/src/core/word_transfer_conversion.cpp#ghidra_conversion::make_vdp_transfer_command | port |
| `0x0F5C` | write_vdp_word_block_with_source_wrap | 60 | decompilation/ghidra_conversion/src/core/word_transfer_conversion.cpp#ghidra_conversion::write_vdp_word_block_with_source_wrap | port |
| `0x0F98` | write_vdp_word_block | 102 | decompilation/ghidra_conversion/src/core/word_transfer_conversion.cpp#ghidra_conversion::write_vdp_word_block | port |
| `0x1056` | clear_vdp_words | 26 | decompilation/ghidra_conversion/src/core/word_transfer_conversion.cpp#ghidra_conversion::clear_vdp_words | port |
| `0x1070` | service_display_frame_wrapper | 16 | decompilation/ghidra_conversion/src/rendering/display_service_batch_conversion.cpp#ghidra_conversion::service_display_frame_wrapper | port |
| `0x1080` | write_alternating_word_pair_wrapper | 8 | decompilation/ghidra_conversion/src/rendering/display_service_batch_conversion.cpp#ghidra_conversion::write_alternating_word_pair_wrapper | port |
| `0x1088` | step_palette_toward_black | 78 | decompilation/ghidra_conversion/src/title_intro/title_loop_conversion.cpp#ghidra_conversion::palette_toward_black | port |
| `0x10D6` | step_palette_toward_target | 108 | decompilation/ghidra_conversion/src/title_intro/title_loop_conversion.cpp#ghidra_conversion::palette_toward_target | port |
| `0x1184` | write_masked_vdp_row | 60 | decompilation/ghidra_conversion/src/core/word_transfer_conversion.cpp#ghidra_conversion::write_masked_vdp_row | port |
| `0x11C0` | write_repeated_vdp_grid | 54 | decompilation/ghidra_conversion/src/core/word_transfer_conversion.cpp#ghidra_conversion::write_repeated_vdp_grid | port |
| `0x11F6` | write_linear_vdp_grid | 6 | decompilation/ghidra_conversion/src/core/word_transfer_conversion.cpp#ghidra_conversion::write_linear_vdp_grid | port |
| `0x1232` | write_vdp_grid | 54 | decompilation/ghidra_conversion/src/core/word_transfer_conversion.cpp#ghidra_conversion::write_vdp_grid | port |
| `0x12F6` | advance_title_palette_fade | 38 | decompilation/ghidra_conversion/src/integration/field_session_bootstrap_conversion.cpp#ghidra_conversion::advance_title_palette_fade | port |
| `0x131C` | step_palette_transition | 38 | decompilation/ghidra_conversion/src/title_intro/title_loop_conversion.cpp#ghidra_conversion::run_palette_transition | port |
| `0x1342` | wait_for_palette_frame | 22 | decompilation/ghidra_conversion/src/core/timing_conversion.cpp | port |
| `0x1368` | copy_target_palette | 16 | decompilation/ghidra_conversion/src/title_intro/title_intro_orchestrator_conversion.cpp#ghidra_conversion::copy_target_palette | port |
| `0x1378` | build_title_sprite_records | 70 | decompilation/ghidra_conversion/src/field/field_entity_render_batch_conversion.cpp#ghidra_conversion::build_title_sprite_records | port |
| `0x13BE` | build_title_sprite_records_with_attribute_offset | 72 | decompilation/ghidra_conversion/src/rendering/sprite_record_builder_conversion.cpp#ghidra_conversion::build_title_sprite_records_with_attribute_offset | port |
| `0x1872` | service_display_frame | 158 | decompilation/ghidra_conversion/src/rendering/display_service_batch_conversion.cpp#ghidra_conversion::service_display_frame | port |
| `0x195C` | sample_controller_input | 64 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp#ghidra_conversion::sample_controller_input | port |
| `0x1A9E` | run_title_loop | 270 | decompilation/ghidra_conversion/src/title_intro/title_loop_conversion.cpp#ghidra_conversion::advance_title_loop_frame | port |
| `0x1BAC` | finalize_title_input_sequence | 62 | decompilation/ghidra_conversion/src/title_intro/title_loop_conversion.cpp#ghidra_conversion::finalize_title_input_sequence | port |
| `0x1BEA` | record_title_input_edge | 52 | decompilation/ghidra_conversion/src/title_intro/title_loop_conversion.cpp#ghidra_conversion::record_title_input_edge | port |
| `0x1C1E` | advance_title_text_state | 76 | decompilation/ghidra_conversion/src/title_intro/title_loop_conversion.cpp#ghidra_conversion::advance_title_text_state | port |
| `0x1C6A` | advance_title_sprite_state | 122 | decompilation/ghidra_conversion/src/title_intro/title_loop_conversion.cpp#ghidra_conversion::advance_title_sprite_state | port |
| `0x1D7A` | setup_title_state | 80 | decompilation/ghidra_conversion/src/title_intro/title_setup_conversion.cpp#ghidra_conversion::setup_title_state | port |
| `0x1DCA` | draw_title_prompt_variant_a | 22 | decompilation/ghidra_conversion/src/title_intro/title_setup_conversion.cpp | port |
| `0x1DE0` | draw_title_prompt_variant_b | 22 | decompilation/ghidra_conversion/src/title_intro/title_setup_conversion.cpp | port |
| `0x1DF6` | draw_title_copyright | 88 | decompilation/ghidra_conversion/src/title_intro/title_setup_conversion.cpp | port |
| `0x1E4E` | reveal_title_logo_column | 124 | decompilation/ghidra_conversion/src/title_intro/title_loop_conversion.cpp#ghidra_conversion::advance_title_logo_column | port |
| `0x2634` | cycle_title_circuit_lights | 156 | decompilation/gameplay/scripts/startup.lua#advance_title_circuit_lights | port |
| `0x26D0` | advance_title_animation_timer | 34 | decompilation/ghidra_conversion/src/title_intro/title_loop_conversion.cpp#ghidra_conversion::advance_title_animation_timer | port |
| `0x281E` | initialize_field_scene_entry | 62 | decompilation/ghidra_conversion/src/field/field_scene_entry_conversion.cpp#ghidra_conversion::initialize_field_scene_entry | port |
| `0x286E` | guard_world_effect_player_spawn | 60 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp#ghidra_conversion::guard_world_effect_player_spawn | port |
| `0x28AA` | prepare_field_resources | 24 | decompilation/ghidra_conversion/src/field/field_resource_sequence_conversion.cpp#ghidra_conversion::prepare_field_resources | port |
| `0x28C2` | reinitialize_field_scene_entry | 62 | decompilation/ghidra_conversion/src/field/field_scene_entry_conversion.cpp#ghidra_conversion::reinitialize_field_scene_entry | port |
| `0x2900` | run_world_frame_services | 306 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp#ghidra_conversion::run_world_frame_services | port |
| `0x2A32` | initialize_field_mode_profile_a | 18 | decompilation/ghidra_conversion/src/field/field_mode_conversion.cpp#ghidra_conversion::initialize_field_mode | port |
| `0x2A44` | initialize_field_mode_profile_b | 80 | decompilation/ghidra_conversion/src/field/field_mode_conversion.cpp#ghidra_conversion::initialize_field_mode | port |
| `0x2B34` | apply_scene_profile_selection | 158 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp#ghidra_conversion::apply_scene_profile_selection | port |
| `0x2BD2` | advance_field_scene_transition | 116 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime_transitions.cpp#ghidra_conversion::advance_field_scene_transition | port |
| `0x2C6C` | prepare_field_viewports | 38 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime_transitions.cpp#ghidra_conversion::prepare_field_viewports | port |
| `0x2C92` | render_normal_field_viewport | 32 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp#ghidra_conversion::render_normal_field_viewport | port |
| `0x2CB2` | render_alternate_field_viewport | 46 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp#ghidra_conversion::render_alternate_field_viewport | port |
| `0x2CE0` | advance_field_mode_transition | 178 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp#ghidra_conversion::advance_field_mode_transition | port |
| `0x2F32` | initialize_field_scene_state | 114 | decompilation/ghidra_conversion/src/field/scene_profile_conversion.cpp#ghidra_conversion::initialize_field_scene_state | port |
| `0x2FA4` | snapshot_entity_anchors | 76 | decompilation/ghidra_conversion/src/field/field_entity_render_batch_conversion.cpp#ghidra_conversion::snapshot_entity_anchors | port |
| `0x2FF0` | find_ready_entity_slot | 46 | decompilation/ghidra_conversion/src/entities/entity_table_conversion.cpp#ghidra_conversion::find_ready_entity_slot | port |
| `0x301E` | upload_scene_profile_block | 20 | decompilation/ghidra_conversion/src/core/word_transfer_conversion.cpp#ghidra_conversion::upload_scene_profile_block | port |
| `0x3032` | scene_uses_alternate_profile | 66 | decompilation/ghidra_conversion/src/field/scene_profile_conversion.cpp#ghidra_conversion::scene_uses_alternate_profile | port |
| `0x3074` | prepare_field_scene_buffers | 16 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime_transitions.cpp#ghidra_conversion::prepare_field_scene_buffers | port |
| `0x3084` | load_field_scene_profile_and_terrain | 182 | decompilation/ghidra_conversion/src/integration/field_session_source_assets_conversion.cpp;decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime_transitions.cpp | port |
| `0x313A` | copy_terrain_layers | 46 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime_transitions.cpp#ghidra_conversion::prepare_field_scene_buffers | port |
| `0x3168` | toggle_field_scene_variant | 138 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp#ghidra_conversion::toggle_field_scene_variant | port |
| `0x334C` | upload_field_asset_sequence | 94 | decompilation/ghidra_conversion/src/field/field_hud_conversion.cpp#ghidra_conversion::upload_field_asset_sequence | port |
| `0x33AA` | draw_field_asset_grid | 20 | decompilation/ghidra_conversion/src/field/field_resource_sequence_conversion.cpp#ghidra_conversion::draw_field_asset_grid | port |
| `0x33BE` | write_four_by_four_transfer_batch | 70 | decompilation/ghidra_conversion/src/core/word_transfer_conversion.cpp#ghidra_conversion::write_four_by_four_transfer_batch | port |
| `0x3404` | upload_field_asset_setup | 244 | decompilation/ghidra_conversion/src/field/field_hud_conversion.cpp#ghidra_conversion::upload_field_asset_setup | port |
| `0x3548` | draw_hud_panel_triplet | 24 | decompilation/ghidra_conversion/src/field/field_hud_conversion.cpp#ghidra_conversion::draw_hud_panel_triplet | port |
| `0x3602` | select_hud_glyph_attribute_flags | 70 | decompilation/ghidra_conversion/src/field/field_hud_conversion.cpp#ghidra_conversion::select_hud_glyph_attribute_flags | port |
| `0x3648` | draw_hud_panel | 44 | decompilation/ghidra_conversion/src/rendering/display_service_batch_conversion.cpp#ghidra_conversion::write_hud_panel_transfer | port |
| `0x38AC` | service_entity_ammo | 78 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp#ghidra_conversion::service_entity_ammo | port |
| `0x38FA` | run_entity_animation_triplet | 32 | decompilation/ghidra_conversion/src/entities/entity_combat_conversion.cpp#ghidra_conversion::run_entity_animation_triplet | port |
| `0x391A` | handle_mode_action_event | 216 | decompilation/gameplay/scripts/party_down.lua | port |
| `0x39F2` | refresh_field_animation_assets | 42 | decompilation/ghidra_conversion/src/entities/entity_combat_conversion.cpp#ghidra_conversion::refresh_field_animation_assets | port |
| `0x3B1E` | classify_field_camera_vertical_delta | 142 | decompilation/ghidra_conversion/src/field/field_camera_conversion.cpp#ghidra_conversion::classify_field_camera_axis_step | port |
| `0x3BAC` | advance_field_camera_scroll | 364 | decompilation/ghidra_conversion/src/field/field_camera_conversion.cpp#ghidra_conversion::follow_field_camera | port |
| `0x3D18` | render_field_tile_rows | 50 | decompilation/ghidra_conversion/src/rendering/display_service_batch_conversion.cpp#ghidra_conversion::render_field_tile_rows | port |
| `0x3D4A` | render_field_tile_column | 54 | decompilation/ghidra_conversion/src/rendering/display_service_batch_conversion.cpp#ghidra_conversion::render_field_tile_column | port |
| `0x3D80` | emit_field_tile_column_words | 94 | decompilation/ghidra_conversion/qt_runtime/src/rendering/plane_streamer.cpp#ghidra_conversion::emit_field_tile_column_words | port |
| `0x3DDE` | emit_field_tile_row_words | 54 | decompilation/ghidra_conversion/qt_runtime/src/rendering/plane_streamer.cpp#ghidra_conversion::emit_field_tile_row_words | port |
| `0x3E14` | upload_field_tile_row | 114 | decompilation/ghidra_conversion/src/field/field_entity_render_batch_conversion.cpp#ghidra_conversion::upload_field_tile_row | port |
| `0x3F9E` | calculate_field_tile_source_offset | 58 | decompilation/ghidra_conversion/src/rendering/display_service_batch_conversion.cpp#ghidra_conversion::calculate_field_tile_source_offset | port |
| `0x3FD8` | calculate_field_tile_source_layout | 40 | decompilation/ghidra_conversion/src/field/field_coordinate_conversion.cpp#ghidra_conversion::calculate_field_tile_source_layout | port |
| `0x4000` | compose_field_tile_destination_offset | 24 | decompilation/ghidra_conversion/src/field/field_coordinate_conversion.cpp#ghidra_conversion::compose_field_tile_destination_offset | port |
| `0x40E2` | render_field_tile_row_edge | 212 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime_world_effects.cpp#ghidra_conversion::render_field_tile_row_edge | port |
| `0x41B6` | fold_field_coordinate | 58 | decompilation/ghidra_conversion/src/field/field_coordinate_conversion.cpp#ghidra_conversion::fold_field_coordinate | port |
| `0x41F0` | prepare_field_vertical_origin | 38 | decompilation/ghidra_conversion/src/field/field_coordinate_conversion.cpp#ghidra_conversion::prepare_field_vertical_origin | port |
| `0x4216` | offset_field_coordinate | 38 | decompilation/ghidra_conversion/src/field/field_coordinate_conversion.cpp#ghidra_conversion::offset_field_coordinate | port |
| `0x423C` | quantize_field_delta | 84 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp#ghidra_conversion::quantize_field_delta | port |
| `0x42B0` | initialize_joshua_archetype_selection | 54 | decompilation/ghidra_conversion/src/entities/entity_template_conversion.cpp#ghidra_conversion::initialize_joshua_archetype_selection | port |
| `0x42E6` | recruited runner live-slot initializer | — | decompilation/ghidra_conversion/src/session/recruited_runner_service_conversion.cpp | port |
| `0x42F2` | initialize_primary_roster_template | 42 | decompilation/ghidra_conversion/src/session/recruited_runner_service_conversion.cpp#ghidra_conversion::initialize_primary_roster_template | port |
| `0x431C` | initialize_secondary_roster_template | 108 | decompilation/ghidra_conversion/src/session/recruited_runner_service_conversion.cpp#ghidra_conversion::initialize_secondary_roster_template | port |
| `0x4388` | finalize_pending_entity_template_update | 46 | decompilation/ghidra_conversion/src/entities/entity_template_conversion.cpp#ghidra_conversion::finalize_pending_entity_template_update | port |
| `0x43B6` | process_pending_entity_template_lists | 176 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp#FieldSceneRuntime::configure_runtime_coordinator_callbacks | port |
| `0x4466` | spawn_entity_template_list | 86 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp#FieldSceneRuntime::publish_pending_world_activity_script_actors | port |
| `0x44BC` | initialize_entity_record_from_template | 348 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp#FieldSceneRuntime::initialize_default_field_actors;decompilation/ghidra_conversion/src/entities/entity_template_conversion.cpp#ghidra_conversion::initialize_entity_record_from_template;decompilation/gameplay/scripts/actor/think.lua#apply_patrol | port |
| `0x4618` | update_entity_render_pass | 64 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp#ghidra_conversion::update_entity_render_pass | port |
| `0x4658` | update_entity_frame | 454 | decompilation/ghidra_conversion/src/world/world_entity_conversion.cpp#ghidra_conversion::update_entity_frame | port |
| `0x481E` | refresh_projected_entity_sprites | 148 | decompilation/ghidra_conversion/src/world/world_entity_conversion.cpp#ghidra_conversion::refresh_projected_entity_sprites | port |
| `0x49A2` | spawn_entity_effect | 266 | decompilation/ghidra_conversion/src/entities/entity_action_math_conversion.cpp#ghidra_conversion::spawn_entity_effect | port |
| `0x4ADC` | advance_entity_effect_timer | 144 | decompilation/ghidra_conversion/src/world/world_entity_conversion.cpp#ghidra_conversion::advance_entity_effect_timer | port |
| `0x4B6C` | reset_effect_record | 68 | decompilation/ghidra_conversion/src/field/field_entity_render_batch_conversion.cpp#ghidra_conversion::reset_effect_record | port |
| `0x4E86` | advance_effect_animation | 156 | decompilation/gameplay/scripts/actor/heartbeat.lua#ActorAI.heartbeat | port |
| `0x4FE2` | spell effect callback code 9 | — | decompilation/gameplay/scripts/magic.lua#actor_effect_profile;decompilation/gameplay/scripts/actor/actions.lua#advance_spell_effect;decompilation/gameplay/scripts/combat.lua#resolve_spell_impact | port |
| `0x508A` | spell effect callback code 8 | — | decompilation/gameplay/scripts/magic.lua#actor_effect_profile;decompilation/gameplay/scripts/actor/actions.lua#advance_spell_effect;decompilation/gameplay/scripts/combat.lua#resolve_spell_impact | port |
| `0x5152` | spell effect callback code 2 | — | decompilation/gameplay/scripts/actor/actions.lua#advance_spell_effect;decompilation/gameplay/scripts/combat.lua#resolve_spell_impact | port |
| `0x521E` | spell effect callback code 1 | — | decompilation/gameplay/scripts/magic.lua#actor_effect_profile;decompilation/gameplay/scripts/actor/actions.lua#advance_spell_effect;decompilation/gameplay/scripts/combat.lua#resolve_spell_impact | port |
| `0x52BE` | sleep effect callback wrapper | — | decompilation/gameplay/scripts/actor/actions.lua#advance_spell_effect | port |
| `0x5322` | sleep effect callback code 5 | — | decompilation/gameplay/scripts/actor/actions.lua#advance_spell_effect;decompilation/gameplay/scripts/combat.lua#resolve_spell_impact | port |
| `0x53C4` | run_effect_variant_a | 164 | decompilation/ghidra_conversion/src/world/world_entity_conversion.cpp#ghidra_conversion::run_effect_variant_a | port |
| `0x5468` | clone_entity_spawn_variant_a | 44 | decompilation/ghidra_conversion/src/field/field_entity_render_batch_conversion.cpp#ghidra_conversion::clone_entity_spawn_variant_a | port |
| `0x5494` | advance_jitter_effect | 234 | decompilation/ghidra_conversion/src/world/world_entity_conversion.cpp#ghidra_conversion::advance_jitter_effect | port |
| `0x557E` | run_effect_variant_b | 164 | decompilation/ghidra_conversion/src/world/world_entity_conversion.cpp#ghidra_conversion::run_effect_variant_b | port |
| `0x5622` | clear_entity_runtime_fields | 28 | decompilation/ghidra_conversion/src/entities/entity_state_primitives_conversion.cpp#ghidra_conversion::clear_entity_runtime_fields | port |
| `0x563E` | clone_entity_spawn_variant_b | 36 | decompilation/ghidra_conversion/src/field/field_entity_render_batch_conversion.cpp#ghidra_conversion::clone_entity_spawn_variant_b | port |
| `0x5662` | advance_effect_animation_variant | 124 | decompilation/ghidra_conversion/src/world/world_entity_conversion.cpp#ghidra_conversion::advance_effect_animation_variant | port |
| `0x5936` | advance_entity_animation_effect | 180 | decompilation/ghidra_conversion/src/world/world_entity_conversion.cpp#ghidra_conversion::advance_entity_animation_effect | port |
| `0x59EA` | advance_title_sprite_timer | 76 | decompilation/ghidra_conversion/src/field/field_entity_render_batch_conversion.cpp#ghidra_conversion::advance_title_sprite_timer | port |
| `0x5A56` | project_entity_screen_position | 100 | decompilation/ghidra_conversion/src/entities/entity_projection_conversion.cpp#ghidra_conversion::project_entity_screen_position | port |
| `0x5A62` | project_entity_screen_position_standard | 100 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp#ghidra_conversion::project_entity_screen_position_standard | port |
| `0x5B98` | emit_title_sprite_burst | 86 | decompilation/ghidra_conversion/src/field/field_entity_render_batch_conversion.cpp#ghidra_conversion::emit_title_sprite_burst | port |
| `0x5C7E` | refresh_delayed_effect_sprites | 260 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp#FieldSceneRuntime | port |
| `0x5FDC` | advance_entity_animation_accumulator | 50 | decompilation/gameplay/scripts/actor/heartbeat.lua#ActorAI.heartbeat | port |
| `0x600E` | advance_entity_sprite_descriptor | 230 | decompilation/gameplay/scripts/actor/heartbeat.lua#ActorAI.heartbeat | port |
| `0x60F4` | set_sprite_descriptor_carry | 6 | decompilation/ghidra_conversion/src/world/world_entity_conversion.cpp#ghidra_conversion::set_sprite_descriptor_carry | port |
| `0x60FA` | append_entity_render_sprite | 78 | decompilation/ghidra_conversion/src/field/field_entity_render_batch_conversion.cpp#ghidra_conversion::append_entity_render_sprite | port |
| `0xA7B4` | initialize_entity_runtime | 48 | decompilation/ghidra_conversion/src/field/field_entity_render_batch_conversion.cpp#ghidra_conversion::initialize_entity_runtime | port |
| `0xA7E4` | initialize_entity_slot_order | 28 | decompilation/ghidra_conversion/src/entities/entity_state_primitives_conversion.cpp#ghidra_conversion::initialize_entity_slot_order | port |
| `0xA800` | prepare_pending_entity_template_list | 174 | decompilation/ghidra_conversion/src/scripting/game_script_runtime.cpp#GameScriptRuntime::conditional_actor_candidates_for_scene | port |
| `0xA8EA` | service_entity_behavior_frames | 488 | decompilation/gameplay/scripts/actor/heartbeat.lua#ActorAI.heartbeat | port |
| `0xAAD2` | dispatch_player_action_if_ready | 48 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp#ghidra_conversion::dispatch_player_action_if_ready | port |
| `0xAB02` | complete_entity_behavior_transition | 180 | decompilation/ghidra_conversion/src/entities/entity_behavior_frame_service_conversion.cpp#ghidra_conversion::complete_entity_behavior_transition | port |
| `0xABB6` | arm_controller_activity_delay | 70 | decompilation/ghidra_conversion/src/rendering/display_service_batch_conversion.cpp#ghidra_conversion::arm_controller_activity_delay | port |
| `0xABFC` | update_entity_animation_delay | 82 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime_player_actions.cpp#ghidra_conversion::update_entity_animation_delay | port |
| `0xAC4E` | secondary projectile finite-list continuation | — | decompilation/gameplay/scripts/actor/movement.lua#projectile_movement_snapshot;decompilation/gameplay/scripts/actor/anim.lua#hit_spark_heartbeat | port |
| `0xAC5E` | clear_entity_effect_if_forced | 38 | decompilation/ghidra_conversion/src/entities/entity_behavior_frame_service_conversion.cpp#ghidra_conversion::clear_entity_effect_if_forced | port |
| `0xAC84` | dispatch_entity_collision_behavior | 42 | decompilation/ghidra_conversion/src/entities/entity_behavior_frame_service_conversion.cpp#ghidra_conversion::dispatch_entity_collision_behavior | port |
| `0xAE5E` | schedule_entity_collision_recheck | 16 | decompilation/ghidra_conversion/src/entities/entity_state_primitives_conversion.cpp#ghidra_conversion::schedule_entity_collision_recheck | port |
| `0xAEBA` | advance_title_sprite_wave | 132 | decompilation/ghidra_conversion/src/rendering/display_service_batch_conversion.cpp#ghidra_conversion::advance_title_sprite_wave | port |
| `0xAF6C` | reset_entity_animation_state | 16 | decompilation/ghidra_conversion/src/entities/animation_reset_conversion.cpp#ghidra_conversion::reset_entity_animation_state | port |
| `0xAF7C` | prepare_entity_delayed_animation_primary | 50 | decompilation/ghidra_conversion/src/entities/behavior_animation_conversion.cpp#ghidra_conversion::prepare_entity_delayed_animation_primary | port |
| `0xAFAE` | prepare_entity_delayed_animation_alternate | 50 | decompilation/ghidra_conversion/src/entities/behavior_animation_conversion.cpp#ghidra_conversion::prepare_entity_delayed_animation_alternate | port |
| `0xAFF2` | prepare_entity_auxiliary_behavior | 76 | decompilation/ghidra_conversion/src/entities/behavior_animation_conversion.cpp#ghidra_conversion::prepare_entity_auxiliary_behavior | port |
| `0xB07E` | damaged entity hurt-response continuation | — | decompilation/gameplay/scripts/actor/anim.lua#advance_received_hit_reaction | port |
| `0xB11C` | synchronize_entity_behavior_spawn | 192 | decompilation/ghidra_conversion/src/entities/entity_transition_conversion.cpp#ghidra_conversion::synchronize_entity_behavior_spawn | port |
| `0xB1DC` | configure_entity_target_transition | 282 | decompilation/gameplay/scripts/actor/actions.lua#begin_dynamic_target_action;decompilation/gameplay/scripts/actor/actions.lua#begin_player_melee | port |
| `0xB330` | enter_entity_behavior_transition | 90 | decompilation/gameplay/scripts/actor/heartbeat.lua#ActorAI.heartbeat | port |
| `0xB39A` | set_entity_heading_with_field_animation | 84 | decompilation/gameplay/scripts/actor/heartbeat.lua;decompilation/gameplay/scripts/actor/anim.lua;decompilation/gameplay/scripts/actor/movement.lua | port |
| `0xB442` | reset_entity_movement_state | 74 | decompilation/gameplay/scripts/actor/movement.lua#reset_blocked_movement | port |
| `0xB48C` | evaluate_entity_collision | 16 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp#ghidra_conversion::evaluate_entity_collision | port |
| `0xB49C` | resolve_entity_fallback_movement | 148 | decompilation/ghidra_conversion/src/entities/entity_behavior_frame_service_conversion.cpp#ghidra_conversion::resolve_entity_fallback_movement | port |
| `0xB530` | approach_entity_fixed_point_target | 58 | decompilation/ghidra_conversion/src/entities/entity_coordinate_conversion.cpp#ghidra_conversion::approach_entity_fixed_point_target | port |
| `0xB574` | entity movement continuation | — | decompilation/gameplay/scripts/actor/heartbeat.lua#ActorAI.heartbeat | port |
| `0xB732` | apply_movement_result_policy | 76 | decompilation/gameplay/scripts/actor/movement.lua | port |
| `0xB77E` | probe_entity_directional_collision | 58 | decompilation/gameplay/scripts/actor/movement.lua#probe_movement | port |
| `0xB7B8` | advance_entity_movement_and_mode_transition | 772 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp#ghidra_conversion::advance_entity_movement_and_mode_transition | port |
| `0xBADE` | advance_entity_movement_step | 168 | decompilation/gameplay/scripts/actor/movement.lua#advance_entity_movement | port |
| `0xBB86` | evaluate_entity_movement_route | 94 | decompilation/gameplay/scripts/actor/movement.lua#advance_entity_movement | port |
| `0xBBE4` | commit_entity_position | 18 | decompilation/gameplay/scripts/actor/movement.lua#commit_movement_candidate | port |
| `0xBC36` | build_collision_probe_points | 120 | decompilation/ghidra_conversion/src/entities/movement_mode_transition_conversion.cpp#ghidra_conversion::build_collision_probe_points | port |
| `0xBCAE` | apply_mode_five_boundary_motion | 86 | decompilation/ghidra_conversion/src/entities/entity_behavior_frame_service_conversion.cpp#ghidra_conversion::apply_mode_five_boundary_motion | port |
| `0xC0DA` | render_encoded_title_text | 90 | decompilation/ghidra_conversion/src/field/field_entity_render_batch_conversion.cpp#ghidra_conversion::render_encoded_title_text | port |
| `0xC216` | build_grouped_decimal_glyphs | 144 | decompilation/ghidra_conversion/src/rendering/decimal_glyph_conversion.cpp#ghidra_conversion::build_grouped_decimal_glyphs | port |
| `0xC2A6` | pocket secretary menu | — | decompilation/gameplay/scripts/menu/screens/pocket_secretary.lua | port |
| `0xC376` | move_wrapped_menu_cursor | 60 | decompilation/ghidra_conversion/qt_runtime/src/menus/attributes_skills_menu_runtime.cpp#ghidra_conversion::move_wrapped_menu_cursor | port |
| `0xC3B2` | write_single_word | 52 | decompilation/ghidra_conversion/src/core/word_transfer_conversion.cpp#ghidra_conversion::write_single_word | port |
| `0xC3E6` | write_menu_grid_with_metadata | 18 | decompilation/ghidra_conversion/src/rendering/display_service_batch_conversion.cpp#ghidra_conversion::write_menu_grid_with_metadata | port |
| `0xC3F8` | render_menu_grid_number_pair | 14 | decompilation/ghidra_conversion/src/rendering/display_service_batch_conversion.cpp#ghidra_conversion::render_menu_grid_number_pair | port |
| `0xC406` | prepare_numeric_menu_block | 48 | decompilation/ghidra_conversion/src/rendering/decimal_glyph_conversion.cpp#ghidra_conversion::prepare_numeric_menu_block | port |
| `0xC448` | Dismiss Runner menu loop | — | decompilation/gameplay/scripts/menu/screens/pocket_secretary.lua | port |
| `0xC64C` | finalize_field_mode_exit_transition | 428 | decompilation/gameplay/scripts/party_down.lua;decompilation/gameplay/scripts/run_registry.lua | port |
| `0xC7F8` | prepare_field_scene_runtime | 10 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp#ghidra_conversion::prepare_field_scene_runtime | port |
| `0xC802` | refresh_field_runner_state_and_assets | 174 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp#ghidra_conversion::refresh_field_runner_state_and_assets | port |
| `0xC8B0` | apply_byte_counter_delta_with_negative_floor | 24 | decompilation/ghidra_conversion/src/core/counter_conversion.cpp#ghidra_conversion::apply_byte_counter_delta_with_negative_floor | port |
| `0xC8C8` | Group Items handler | — | decompilation/gameplay/scripts/menu/screens/pocket_secretary.lua | port |
| `0xCF38` | run_hotel_options_menu | 198 | decompilation/ghidra_conversion/src/field/field_exit_prompt_conversion.cpp#ghidra_conversion::run_hotel_options_menu | port |
| `0xCFFE` | prepare_hotel_options_display | 30 | decompilation/ghidra_conversion/src/menus/hotel_options_menu_conversion.cpp#ghidra_conversion::prepare_hotel_options_display | port |
| `0xD01C` | run_attributes_and_skills_menu | 1196 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime_command_overview.cpp#FieldSceneRuntime::begin_command_overview_attributes_skills | port |
| `0xD024` | Hotel Karma upgrade entry into Attributes/Skills body | — | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime_services.cpp#FieldSceneRuntime::process_hotel_menu_events | port |
| `0xD27C` | attributes/skills cursor column | — | decompilation/ghidra_conversion/src/menus/attributes_skills_menu_conversion.cpp#ghidra_conversion::move_attribute_skill_cursor | port |
| `0xD2DA` | attributes/skills cursor up | — | decompilation/ghidra_conversion/src/menus/attributes_skills_menu_conversion.cpp#ghidra_conversion::move_attribute_skill_cursor | port |
| `0xD302` | attributes/skills cursor down | — | decompilation/ghidra_conversion/src/menus/attributes_skills_menu_conversion.cpp#ghidra_conversion::move_attribute_skill_cursor | port |
| `0xD3EA` | Hotel Attributes/Skills Karma upgrade branch | — | decompilation/ghidra_conversion/qt_runtime/src/menus/attributes_skills_menu_runtime.cpp#ghidra_conversion::preview_attribute_skill_upgrade | port |
| `0xD5DC` | render_attribute_skill_detail_panel | 526 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp#ghidra_conversion::render_attribute_skill_detail_panel | port |
| `0xD7EA` | compute_entity_delay_factor | 50 | decompilation/ghidra_conversion/src/entities/entity_delay_conversion.cpp#ghidra_conversion::compute_entity_delay_factor | port |
| `0xD81C` | average_entity_delay_attributes | 14 | decompilation/ghidra_conversion/src/entities/entity_delay_conversion.cpp#ghidra_conversion::average_entity_delay_attributes | port |
| `0xD82A` | render_attributes_and_skills | 544 | decompilation/ghidra_conversion/qt_runtime/src/menus/attributes_skills_menu_runtime.cpp#ghidra_conversion::render_attributes_and_skills | port |
| `0xDA4A` | render_attributes_skills_heading | 32 | decompilation/ghidra_conversion/src/menus/attributes_skills_menu_conversion.cpp#ghidra_conversion::render_attributes_skills_heading | port |
| `0xDA6A` | Hotel Check Out confirmation | — | decompilation/ghidra_conversion/src/menus/hotel_options_menu_conversion.cpp#ghidra_conversion::run_hotel_checkout_prompt | port |
| `0xDB26` | ordinary field terminal dispatcher | — | decompilation/gameplay/scripts/field_activities.lua | port |
| `0xE00A` | wait_for_controller_event | 42 | decompilation/ghidra_conversion/src/core/controller_wait_conversion.cpp#ghidra_conversion::wait_for_controller_event | port |
| `0xE660` | advance_random_grid_effect | 148 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime_command_overview.cpp#ghidra_conversion::advance_random_grid_effect | port |
| `0xE7C8` | evaluate_terrain_collision | 216 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp#ghidra_conversion::evaluate_terrain_collision | port |
| `0xE8A0` | sample_terrain_collision_tiles | 300 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp#ghidra_conversion::sample_terrain_collision_tiles | port |
| `0xE9EE` | contains_terrain_object_bounds | 72 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp#ghidra_conversion::contains_terrain_object_bounds | port |
| `0xEA36` | evaluate_entity_terrain_collision_directions | 156 | decompilation/ghidra_conversion/src/field/terrain_probe_conversion.cpp#ghidra_conversion::evaluate_entity_terrain_collision_directions | port |
| `0xEAD2` | evaluate_terrain_direction_boundary | 124 | decompilation/ghidra_conversion/src/field/terrain_probe_conversion.cpp#ghidra_conversion::evaluate_terrain_direction_boundary | port |
| `0xEB92` | resolve_entity_proximity_interactions | 314 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp#ghidra_conversion::resolve_entity_proximity_interactions | port |
| `0xECCC` | resolve_entity_pair_collision_event | 302 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp#ghidra_conversion::resolve_entity_pair_collision_event | port |
| `0xEDFA` | decode_terrain_tile_window | 22 | decompilation/ghidra_conversion/src/field/terrain_probe_conversion.cpp#ghidra_conversion::decode_terrain_tile_window | port |
| `0xEE10` | rebuild_terrain_collision_map | 118 | decompilation/ghidra_conversion/src/field/terrain_map_conversion.cpp#ghidra_conversion::rebuild_terrain_collision_map | port |
| `0xEE86` | replace_nth_terrain_marker | 120 | decompilation/ghidra_conversion/src/field/terrain_map_conversion.cpp#ghidra_conversion::replace_nth_terrain_marker | port |
| `0xEF08` | service_hotel_entity_status | 172 | decompilation/ghidra_conversion/qt_runtime/src/menus/hotel_menu_runtime.cpp#ghidra_conversion::service_hotel_entity_status | port |
| `0xF236` | Notebook menu loop | — | decompilation/gameplay/scripts/menu/screens/pocket_secretary.lua | port |
| `0xF2E2` | Notebook Current Run handler | — | decompilation/gameplay/scripts/menu/screens/pocket_secretary.lua | port |
| `0xF312` | wait_for_controller_event | 44 | decompilation/gameplay/scripts/menu/screens/pocket_secretary.lua | port |
| `0xF33E` | Notebook menu rebuild | — | decompilation/gameplay/scripts/menu/screens/pocket_secretary.lua | port |
| `0xF404` | write_menu_footer_grid | 18 | decompilation/ghidra_conversion/src/rendering/display_service_batch_conversion.cpp#ghidra_conversion::write_menu_footer_grid | port |
| `0xF416` | Notebook Mr. Johnsons child page | — | decompilation/gameplay/scripts/menu/screens/pocket_secretary.lua | port |
| `0xF6DC` | Notebook Shadowrunners child page | — | decompilation/gameplay/scripts/menu/screens/pocket_secretary.lua | port |
| `0xF840` | Notebook Contacts child page | — | decompilation/gameplay/scripts/menu/screens/pocket_secretary.lua | port |
| `0xF9A4` | Notebook Tips & Clues page loop | — | decompilation/gameplay/scripts/menu/screens/pocket_secretary.lua | port |
| `0xFBA6` | run_magic_menu | 520 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime_command_overview.cpp#ghidra_conversion::select_magic_menu_path | port |
| `0xFC68` | magic spell table selection | — | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime_command_overview.cpp#ghidra_conversion::select_magic_spell | port |
| `0xFEBE` | move_selection_column | 72 | decompilation/ghidra_conversion/src/core/selection_conversion.cpp#ghidra_conversion::move_selection_column | port |
| `0xFF06` | move_selection_row | 84 | decompilation/ghidra_conversion/src/core/selection_conversion.cpp#ghidra_conversion::move_selection_row | port |
| `0xFF5A` | run_equipment_selection | 188 | decompilation/ghidra_conversion/src/menus/magic_menu_conversion.cpp#ghidra_conversion::run_magic_equipment_selection | port |
| `0x10016` | run_magic_force_selection_loop | 146 | decompilation/ghidra_conversion/src/menus/magic_menu_conversion.cpp#ghidra_conversion::run_magic_force_selection_loop | port |
| `0x100A8` | adjust_magic_force | 174 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime_command_overview.cpp#ghidra_conversion::adjust_magic_force | port |
| `0x10156` | write_repeated_menu_grid | 28 | decompilation/ghidra_conversion/src/menus/menu_transfer_batch_conversion.cpp#ghidra_conversion::write_repeated_menu_grid | port |
| `0x10172` | refresh_magic_force_display | 140 | decompilation/ghidra_conversion/src/menus/magic_menu_conversion.cpp#ghidra_conversion::refresh_magic_force_display | port |
| `0x101FE` | select_menu_value | 98 | decompilation/ghidra_conversion/src/menus/magic_menu_conversion.cpp#ghidra_conversion::select_magic_force | port |
| `0x10260` | prepare_menu_grid_then_service | 22 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime_command_overview.cpp#ghidra_conversion::prepare_magic_menu_grid_and_select_force | port |
| `0x10276` | run_menu_text_display_sequence | 94 | decompilation/ghidra_conversion/src/menus/magic_menu_conversion.cpp#ghidra_conversion::render_magic_menu_selection_display | port |
| `0x102D4` | prepare_magic_action_details | 106 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime_command_overview.cpp#ghidra_conversion::prepare_magic_action_details | port |
| `0x1033E` | resolve_first_aid_item_use | 320 | decompilation/gameplay/scripts/combat.lua#resolve_spell_action | port |
| `0x1047E` | refresh_ready_entity_animations | 74 | decompilation/ghidra_conversion/src/entities/entity_action_math_conversion.cpp#ghidra_conversion::calculate_ready_entity_action_bonus | port |
| `0x104C8` | find_matching_menu_metric | 46 | decompilation/ghidra_conversion/src/menus/menu_transfer_batch_conversion.cpp | port |
| `0x104F6` | apply_action_item_phase_modifiers | 104 | decompilation/ghidra_conversion/src/entities/entity_action_math_conversion.cpp#ghidra_conversion::apply_action_item_phase_modifiers | port |
| `0x1055E` | refresh_equipment_combat_values | 136 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp#ghidra_conversion::refresh_magic_menu_success_or_defense_values | port |
| `0x105E6` | refresh_equipment_action_metric | 122 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp#ghidra_conversion::refresh_magic_action_effect_value | port |
| `0x10660` | calculate_action_residual | 122 | decompilation/ghidra_conversion/src/entities/entity_action_math_conversion.cpp#ghidra_conversion::calculate_action_residual | port |
| `0x106DA` | prepare_grid_then_write | 72 | decompilation/ghidra_conversion/src/menus/menu_transfer_batch_conversion.cpp#ghidra_conversion::prepare_grid_then_write | port |
| `0x10722` | run_runner_posture_adjustment | 172 | decompilation/ghidra_conversion/src/menus/magic_menu_conversion.cpp#ghidra_conversion::run_runner_posture_adjustment | port |
| `0x107CE` | adjust_runner_posture | 134 | decompilation/ghidra_conversion/src/session/runner_record_operations_conversion.cpp#ghidra_conversion::adjust_runner_posture_and_refresh | port |
| `0x10854` | write_three_grids_then_service | 56 | decompilation/ghidra_conversion/src/menus/menu_transfer_batch_conversion.cpp#ghidra_conversion::write_three_grids_then_service | port |
| `0x1088C` | render_magic_spell_availability | 140 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime_command_overview.cpp#ghidra_conversion::render_magic_spell_availability | port |
| `0x10984` | transfer_menu_asset_batch_nine_optional | 10 | decompilation/ghidra_conversion/src/menus/attributes_skills_menu_conversion.cpp#ghidra_conversion::transfer_menu_asset_batch | port |
| `0x1098E` | clear_magic_menu_grids | 240 | decompilation/ghidra_conversion/src/menus/magic_menu_conversion.cpp#ghidra_conversion::clear_magic_menu_grids | port |
| `0x10A7E` | transfer_menu_asset_batch_four_post_prepare | 4 | decompilation/ghidra_conversion/src/menus/attributes_skills_menu_conversion.cpp#ghidra_conversion::transfer_menu_asset_batch | port |
| `0x10A82` | refresh_menu_grid_quadruple | 110 | decompilation/ghidra_conversion/src/menus/magic_menu_conversion.cpp#ghidra_conversion::refresh_menu_grid_quadruple | port |
| `0x10AF0` | transfer_menu_asset_batch_four | 106 | decompilation/ghidra_conversion/src/menus/attributes_skills_menu_conversion.cpp#ghidra_conversion::transfer_menu_asset_batch | port |
| `0x10B5A` | transfer_menu_asset_batch_four | 106 | decompilation/ghidra_conversion/src/menus/attributes_skills_menu_conversion.cpp#ghidra_conversion::transfer_menu_asset_batch | port |
| `0x10BC4` | transfer_menu_asset_batch_eight_after_copy | 210 | decompilation/ghidra_conversion/src/menus/attributes_skills_menu_conversion.cpp#ghidra_conversion::transfer_menu_asset_batch | port |
| `0x10C96` | transfer_menu_asset_batch_eight | 154 | decompilation/ghidra_conversion/src/menus/attributes_skills_menu_conversion.cpp#ghidra_conversion::transfer_menu_asset_batch | port |
| `0x10D30` | run_command_overview | 544 | decompilation/ghidra_conversion/src/integration/field_runtime_coordinator_conversion.cpp#ghidra_conversion::run_command_overview | port |
| `0x10F50` | prepare_command_overview | 164 | decompilation/ghidra_conversion/qt_runtime/src/field/field_command_overview_bridge.cpp#ghidra_conversion::prepare_command_overview | port |
| `0x10FF4` | write_repeated_asset_batch | 84 | decompilation/ghidra_conversion/src/menus/menu_service_sequence_conversion.cpp#ghidra_conversion::write_command_overview_repeated_asset_batch | port |
| `0x11048` | upload_resource_then_write_two_blocks | 54 | decompilation/ghidra_conversion/src/menus/menu_service_sequence_conversion.cpp#ghidra_conversion::upload_command_overview_resource_and_blocks | port |
| `0x1107E` | upload_resource_stream | 62 | decompilation/ghidra_conversion/src/menus/menu_service_sequence_conversion.cpp#ghidra_conversion::upload_command_overview_resource | port |
| `0x110BC` | draw_command_overview_hud_panel | 44 | decompilation/ghidra_conversion/qt_runtime/src/field/field_command_overview_bridge.cpp#ghidra_conversion::draw_command_overview_hud_panel | port |
| `0x110E8` | enter_active_runner_hud_panel | 4 | decompilation/ghidra_conversion/src/menus/menu_service_sequence_conversion.cpp#ghidra_conversion::draw_active_runner_hud_panel | port |
| `0x110EC` | draw_active_runner_hud_panel | 46 | decompilation/ghidra_conversion/src/menus/menu_service_sequence_conversion.cpp#ghidra_conversion::draw_active_runner_hud_panel | port |
| `0x1111A` | render_active_runner_name_text | 28 | decompilation/ghidra_conversion/src/menus/menu_service_sequence_conversion.cpp#ghidra_conversion::render_active_runner_name_text | port |
| `0x11136` | render_menu_tile_rows | 90 | decompilation/ghidra_conversion/qt_runtime/src/app/ghidra_conversion_qt_window.hpp | replaced |
| `0x11190` | transfer_menu_glyph_triple | 64 | decompilation/ghidra_conversion/src/field/field_entity_render_batch_conversion.cpp#ghidra_conversion::transfer_menu_glyph_triple | port |
| `0x111D0` | upload_menu_tile_block | 24 | decompilation/ghidra_conversion/src/menus/menu_service_sequence_conversion.cpp#ghidra_conversion::upload_menu_tile_block | port |
| `0x111E8` | render_equipment_details | 362 | decompilation/ghidra_conversion/src/menus/equipment_combat_bar_conversion.cpp#ghidra_conversion::render_equipment_details | port |
| `0x11442` | reset_and_upload_menu_tile_block | 18 | decompilation/ghidra_conversion/src/menus/menu_service_sequence_conversion.cpp#ghidra_conversion::reset_and_upload_menu_tile_block | port |
| `0x11454` | reset_menu_upload_cursor_and_upload | 76 | decompilation/ghidra_conversion/src/menus/menu_service_sequence_conversion.cpp#ghidra_conversion::reset_menu_upload_cursor_and_upload | port |
| `0x114A0` | advance_command_overview_primary_page | 102 | decompilation/ghidra_conversion/qt_runtime/src/field/field_command_overview_bridge.cpp#ghidra_conversion::advance_command_overview_primary_page | port |
| `0x11506` | advance_command_overview_secondary_page | 140 | decompilation/ghidra_conversion/qt_runtime/src/field/field_command_overview_bridge.cpp#ghidra_conversion::advance_command_overview_secondary_page | port |
| `0x11592` | refresh_command_overview_action_details | 146 | decompilation/ghidra_conversion/qt_runtime/src/field/field_command_overview_bridge.cpp#ghidra_conversion::refresh_command_overview_action_details | port |
| `0x11624` | render_equipment_highlights | 84 | decompilation/ghidra_conversion/src/menus/equipment_combat_bar_conversion.cpp#ghidra_conversion::render_equipment_highlights | port |
| `0x11678` | write_menu_grid | 76 | decompilation/ghidra_conversion/src/menus/menu_service_sequence_conversion.cpp#ghidra_conversion::write_menu_grid | port |
| `0x116C4` | write_menu_grid_quad | 72 | decompilation/ghidra_conversion/src/menus/menu_service_sequence_conversion.cpp#ghidra_conversion::write_menu_grid_quad | port |
| `0x1170C` | select_command_overview_title_text | 6 | decompilation/ghidra_conversion/qt_runtime/src/field/field_command_overview_bridge.cpp#ghidra_conversion::select_command_overview_title_text | port |
| `0x11712` | select_menu_title_text | 46 | decompilation/ghidra_conversion/src/menus/menu_service_sequence_conversion.cpp#ghidra_conversion::select_menu_title_text | port |
| `0x11740` | run_inventory_actions | 2332 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime_command_overview.cpp#ghidra_conversion::advance_inventory_action_loop | port |
| `0x118C6` | cycle_inventory_action_selector | 58 | decompilation/ghidra_conversion/src/core/selection_conversion.cpp#ghidra_conversion::cycle_inventory_action_selector | port |
| `0x11900` | prepare_menu_row_grid | 40 | decompilation/ghidra_conversion/src/menus/menu_service_sequence_conversion.cpp#ghidra_conversion::prepare_menu_row_grid | port |
| `0x11928` | render_menu_row | 40 | decompilation/ghidra_conversion/src/menus/menu_service_sequence_conversion.cpp#ghidra_conversion::render_menu_row | port |
| `0x11950` | run_cyberware_menu | 126 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime_command_overview.cpp#ghidra_conversion::build_cyberware_menu | port |
| `0x119CE` | run_command_overview_action_dialog | 240 | decompilation/ghidra_conversion/src/menus/menu_service_sequence_conversion.cpp#ghidra_conversion::run_command_overview_action_dialog | port |
| `0x11ABE` | prepare_menu_dialog | 38 | decompilation/ghidra_conversion/src/menus/equipment_combat_bar_conversion.cpp#ghidra_conversion::prepare_menu_dialog | port |
| `0x11AE4` | adjust_menu_dialog_posture | 116 | decompilation/ghidra_conversion/src/session/runner_record_operations_conversion.cpp#ghidra_conversion::adjust_menu_dialog_posture | port |
| `0x11B58` | caseD_2 | 1290 | decompilation/gameplay/scripts/menu/screens/clips.lua;decompilation/gameplay/scripts/menu/rules/clips.lua | port |
| `0x11CCA` | Clips reload branch | — | decompilation/gameplay/scripts/menu/screens/clips.lua;decompilation/gameplay/scripts/menu/rules/clips.lua | port |
| `0x11DD2` | Clips recipient selection | — | decompilation/gameplay/scripts/menu/rules/clips.lua;decompilation/gameplay/scripts/menu/screens/clips.lua | port |
| `0x11FDE` | Clips distribution branch | — | decompilation/gameplay/scripts/menu/rules/clips.lua;decompilation/gameplay/scripts/menu/screens/clips.lua | port |
| `0x12062` | refresh_menu_text | 56 | decompilation/gameplay/scripts/menu/screens/clips.lua | port |
| `0x1209A` | advance_menu_text_cursor | 74 | decompilation/ghidra_conversion/src/menus/menu_service_sequence_conversion.cpp#ghidra_conversion::advance_menu_text_cursor | port |
| `0x120E4` | render_menu_number | 100 | decompilation/ghidra_conversion/src/menus/menu_service_sequence_conversion.cpp#ghidra_conversion::render_menu_number | port |
| `0x12168` | selected inventory action dispatch | — | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime_command_overview.cpp#ghidra_conversion::classify_selected_item | port |
| `0x121E0` | weapon/gear/armor equipment branch | — | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime_command_overview.cpp#ghidra_conversion::toggle_selected_equipment | port |
| `0x1253E` | reset_inventory_action_target_state | 188 | decompilation/ghidra_conversion/src/menus/first_aid_conversion.cpp#ghidra_conversion::apply_first_aid_menu_effect | port |
| `0x125FA` | clear_entity_auxiliary_marker | 58 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime_command_overview.cpp#ghidra_conversion::clear_entity_auxiliary_marker | port |
| `0x12634` | protected discard gate | — | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime_command_overview.cpp#ghidra_conversion::can_discard_selected_item | port |
| `0x1275C` | clear_menu_slot | 84 | decompilation/ghidra_conversion/src/field/field_action_removal_conversion.cpp#ghidra_conversion::clear_menu_slot | port |
| `0x127B0` | refresh_menu_panel | 76 | decompilation/ghidra_conversion/src/menus/menu_service_sequence_conversion.cpp#ghidra_conversion::refresh_menu_panel | port |
| `0x127FC` | write_single_grid | 58 | decompilation/ghidra_conversion/src/menus/menu_transfer_batch_conversion.cpp#ghidra_conversion::write_single_grid | port |
| `0x12836` | trade eligibility and recipient scan | — | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime_command_overview.cpp#ghidra_conversion::can_trade_selected_item | port |
| `0x129FE` | transfer_inventory_action_slot | 86 | decompilation/ghidra_conversion/src/menus/inventory_action_conversion.cpp#ghidra_conversion::transfer_selected_item | port |
| `0x12A54` | update_resource_variant | 56 | decompilation/ghidra_conversion/src/entities/action_detail_conversion.cpp#ghidra_conversion::update_resource_variant | port |
| `0x12A8C` | render_item_action_details | 402 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime_shop_paint.cpp#ghidra_conversion::render_item_action_details | port |
| `0x12C1E` | prepare_menu_grid_variant_a | 12 | decompilation/ghidra_conversion/src/menus/menu_service_sequence_conversion.cpp#ghidra_conversion::prepare_menu_grid_variant_a | port |
| `0x12C36` | prepare_menu_grid_variant_b | 12 | decompilation/ghidra_conversion/src/menus/equipment_combat_bar_conversion.cpp#ghidra_conversion::prepare_menu_grid_variant_b | port |
| `0x12C42` | prepare_menu_grid_variant_c | 12 | decompilation/ghidra_conversion/src/menus/menu_service_sequence_conversion.cpp#ghidra_conversion::prepare_menu_grid_variant_c | port |
| `0x12C56` | write_menu_repeated_grid | 4 | decompilation/ghidra_conversion/src/menus/menu_service_sequence_conversion.cpp#ghidra_conversion::write_menu_repeated_grid | port |
| `0x12C5A` | write_menu_repeated_grid_with_metadata | 26 | decompilation/ghidra_conversion/src/menus/menu_service_sequence_conversion.cpp#ghidra_conversion::write_menu_repeated_grid_with_metadata | port |
| `0x12C74` | merge_equipment_action_skill_value | 98 | decompilation/ghidra_conversion/src/menus/equipment_combat_bar_conversion.cpp#ghidra_conversion::merge_equipment_action_skill_value | port |
| `0x12CD6` | render_menu_text_asset | 24 | decompilation/ghidra_conversion/src/menus/menu_service_sequence_conversion.cpp#ghidra_conversion::render_menu_text_asset | port |
| `0x12CEE` | update_equipment_combat_bars | 432 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp#ghidra_conversion::compute_equipment_combat_bars | port |
| `0x12EAE` | render_equipment_action_detail_row | 162 | decompilation/ghidra_conversion/src/menus/equipment_combat_bar_conversion.cpp#ghidra_conversion::render_equipment_action_detail_row | port |
| `0x13000` | select_entity_action_detail | 192 | decompilation/ghidra_conversion/src/entities/action_detail_conversion.cpp#ghidra_conversion::select_entity_action_detail | port |
| `0x130C0` | build_field_action_detail_plan | 360 | decompilation/ghidra_conversion/src/entities/action_detail_conversion.cpp#ghidra_conversion::build_field_action_detail_plan | port |
| `0x13228` | build_inventory_action_detail_plan | 382 | decompilation/ghidra_conversion/src/entities/action_detail_conversion.cpp#ghidra_conversion::build_inventory_action_detail_plan | port |
| `0x134D2` | run_post_intro_mode | 524 | decompilation/ghidra_conversion/src/world/post_intro_mode_conversion.cpp#ghidra_conversion::run_post_intro_mode | port |
| `0x136DE` | initialize_scene_mode_state | 72 | decompilation/ghidra_conversion/src/field/field_mode_conversion.cpp#ghidra_conversion::initialize_scene_mode_state | port |
| `0x13904` | run_entity_animation_update_coordinator | 344 | decompilation/gameplay/scripts/actor/heartbeat.lua#ActorAI.heartbeat | port |
| `0x13A4C` | Thon period target and spell-pose selection | — | decompilation/gameplay/scripts/actor/actions.lua#begin_thon_action;decompilation/gameplay/scripts/actor/actions.lua#begin_spell_cast;decompilation/gameplay/scripts/combat.lua#resolve_spell_action | port |
| `0x13AF2` | find_first_entity_action_below_threshold | 32 | decompilation/ghidra_conversion/src/entities/entity_state_primitives_conversion.cpp#ghidra_conversion::find_first_entity_action_below_threshold | port |
| `0x13D0C` | run_entity_animation_motion | 292 | decompilation/ghidra_conversion/src/entities/entity_animation_motion_conversion.cpp#ghidra_conversion::run_entity_animation_motion | port |
| `0x13FFE` | dispatch_entity_animation_state | 506 | decompilation/ghidra_conversion/src/entities/entity_animation_dispatch_conversion.cpp#ghidra_conversion::dispatch_entity_animation_state | port |
| `0x14226` | service_entity_animation_action | 136 | decompilation/ghidra_conversion/src/entities/entity_animation_dispatch_conversion.cpp#ghidra_conversion::service_entity_animation_action | port |
| `0x142AE` | entity animation waypoint selection | — | decompilation/gameplay/scripts/actor/targeting.lua#source_profile;decompilation/gameplay/scripts/actor/think.lua#apply_patrol | port |
| `0x14640` | map_world_delta_to_tile_pair | 38 | decompilation/ghidra_conversion/src/entities/entity_tile_mapping_conversion.cpp#ghidra_conversion::map_world_delta_to_tile_pair | port |
| `0x14800` | probe_obstruction_strip | 84 | decompilation/ghidra_conversion/src/entities/entity_route_primitives_conversion.cpp#ghidra_conversion::probe_obstruction_strip | port |
| `0x14854` | can_bypass_obstruction | 86 | decompilation/ghidra_conversion/src/entities/entity_route_primitives_conversion.cpp#ghidra_conversion::find_bypass_route | port |
| `0x148D2` | choose_dominant_axis_directions | 48 | decompilation/ghidra_conversion/src/entities/entity_route_primitives_conversion.cpp#ghidra_conversion::choose_dominant_axis_directions | port |
| `0x14914` | is_entity_path_blocked | 56 | decompilation/ghidra_conversion/src/entities/entity_route_primitives_conversion.cpp#ghidra_conversion::is_entity_path_blocked | port |
| `0x1494C` | is_obstruction_path_blocked | 116 | decompilation/ghidra_conversion/src/entities/entity_route_primitives_conversion.cpp#ghidra_conversion::is_obstruction_path_blocked | port |
| `0x14AF4` | find_rotating_route_strip_target | 90 | decompilation/ghidra_conversion/src/entities/entity_route_primitives_conversion.cpp#ghidra_conversion::find_rotating_route_strip_target | port |
| `0x14B4E` | decode_obstruction_tile_projection | 88 | decompilation/ghidra_conversion/src/entities/entity_route_primitives_conversion.cpp#ghidra_conversion::decode_obstruction_tile_projection | port |
| `0x14C56` | project_heading_position | 38 | decompilation/ghidra_conversion/src/entities/entity_route_primitives_conversion.cpp#ghidra_conversion::project_heading_position | port |
| `0x14C7C` | resolve_entity_target_motion | 220 | decompilation/gameplay/scripts/actor/movement.lua#resolve_source_target_motion | port |
| `0x14D58` | measure_entity_target_heading_and_distance | 18 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp#ghidra_conversion::measure_entity_target_heading_and_distance | port |
| `0x14D6A` | select_entity_motion_action_row | 36 | decompilation/ghidra_conversion/src/entities/entity_animation_conversion.cpp#ghidra_conversion::select_entity_motion_action_row | port |
| `0x14D8E` | probe_entity_spawn_collision_window | 124 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime_player_actions.cpp#ghidra_conversion::probe_entity_spawn_collision_window | port |
| `0x14E0A` | primary weapon field continuation | — | decompilation/gameplay/scripts/actor/movement.lua#projectile_movement_snapshot | port |
| `0x14E32` | projectile terminal and impact continuation | — | decompilation/gameplay/scripts/actor/movement.lua#projectile_movement_snapshot | port |
| `0x14F26` | probe_entity_world_position | 42 | decompilation/gameplay/scripts/actor/movement.lua#projectile_movement_snapshot | port |
| `0x14F50` | advance_entity_collision_recovery | 54 | decompilation/gameplay/scripts/actor/movement.lua#projectile_movement_snapshot | port |
| `0x14F86` | stopped projectile linked-action continuation | — | decompilation/gameplay/scripts/actor/movement.lua#projectile_movement_snapshot | port |
| `0x14FF8` | initialize_entity_spawn_record | 240 | decompilation/ghidra_conversion/src/entities/entity_animation_spawn_probe_conversion.cpp#ghidra_conversion::initialize_entity_spawn_record | port |
| `0x150E8` | derive_entity_heading_offsets | 48 | decompilation/gameplay/scripts/entity_motion.lua#projectile_step | port |
| `0x1514E` | process_entity_spawn_cycle | 428 | decompilation/gameplay/scripts/actor/actions.lua;decompilation/gameplay/scripts/actor/heartbeat.lua | port |
| `0x152FA` | prepare_special_spawn_record | 30 | decompilation/ghidra_conversion/src/entities/entity_transition_conversion.cpp#ghidra_conversion::prepare_special_spawn_record | port |
| `0x1562E` | service_entity_auxiliary_behavior | 24 | decompilation/ghidra_conversion/src/entities/behavior_animation_conversion.cpp#ghidra_conversion::service_entity_auxiliary_behavior | port |
| `0x1582A` | allocate_first_free_effect_slot_local | 6 | decompilation/ghidra_conversion/src/entities/entity_state_primitives_conversion.cpp#ghidra_conversion::allocate_first_free_effect_slot_local | port |
| `0x15830` | dispatch_entity_effect | 256 | decompilation/gameplay/scripts/actor/actions.lua#advance_spell_effect | port |
| `0x1597E` | scan_entity_effect_targets | 206 | decompilation/gameplay/scripts/actor/actions.lua#begin_spell_cast | port |
| `0x15A8C` | find_first_available_entity_slot | 46 | decompilation/gameplay/scripts/actor/actions.lua#begin_player_melee | port |
| `0x15ABA` | find_colliding_entity | 250 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp#ghidra_conversion::find_colliding_entity | port |
| `0x15BB4` | noop_entity_action_hook | 2 | decompilation/ghidra_conversion/src/entities/entity_behavior_frame_service_conversion.cpp#ghidra_conversion::noop_entity_action_hook | port |
| `0x15C58` | is_entity_target_within_range | 44 | decompilation/ghidra_conversion/src/entities/movement_metric_conversion.cpp#ghidra_conversion::is_entity_target_within_range | port |
| `0x15C84` | update_entity_heading_from_linked_target | 32 | decompilation/ghidra_conversion/src/entities/entity_target_motion_conversion.cpp#ghidra_conversion::update_entity_heading_from_linked_target | port |
| `0x15D4E` | measure_heading_and_distance | 126 | decompilation/ghidra_conversion/src/entities/entity_proximity_collision_conversion.cpp#ghidra_conversion::measure_heading_and_distance | port |
| `0x15DCC` | transform_entity_heading_vector | 42 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp#ghidra_conversion::transform_entity_heading_vector | port |
| `0x16396` | advance_palette_fade | 68 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp#ghidra_conversion::advance_palette_fade | port |
| `0x164F2` | entity_table_has_free_slot | 44 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime_world_effects.cpp#ghidra_conversion::entity_table_has_free_slot | port |
| `0x1651E` | project_entity_world_interaction | 348 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime_world_effects.cpp#ghidra_conversion::apply_world_map_effect | port |
| `0x1669C` | project_entity_x_axis | 52 | decompilation/ghidra_conversion/src/menus/menu_transfer_batch_conversion.cpp#ghidra_conversion::project_entity_x_axis | port |
| `0x166D0` | project_entity_y_axis | 62 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime_world_effects.cpp#ghidra_conversion::project_entity_y_axis | port |
| `0x16760` | mark_projected_terrain_cell | 78 | decompilation/ghidra_conversion/src/entities/movement_mode_transition_conversion.cpp#ghidra_conversion::mark_projected_terrain_cell | port |
| `0x167AE` | initialize_world_activity_mode | 632 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp#ghidra_conversion::run_world_activity_routine | port |
| `0x1684C` | mode-one scene actor-list selection | — | decompilation/ghidra_conversion/src/scripting/game_script_runtime.cpp#GameScriptRuntime::resolve_conditional_actor_selection | port |
| `0x16A8E` | selected-floor scene preparation | — | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime_travel.cpp#FieldSceneRuntime::advance_field_elevator_scene_transition | port |
| `0x16AFA` | upload_mode_word_block | 38 | decompilation/ghidra_conversion/src/core/word_transfer_conversion.cpp#ghidra_conversion::upload_mode_word_block | port |
| `0x16B7C` | copy_scene_resource_cursor | 28 | decompilation/ghidra_conversion/src/field/scene_profile_conversion.cpp#ghidra_conversion::copy_scene_resource_cursor | port |
| `0x16B98` | spawn_scene_entity_from_weighted_table | 346 | decompilation/ghidra_conversion/src/world/world_scene_entity_conversion.cpp#ghidra_conversion::spawn_world_scene_entity | port |
| `0x16D8A` | spawn_scene_record_offset | 50 | decompilation/ghidra_conversion/src/field/scene_profile_conversion.cpp#ghidra_conversion::spawn_scene_record_offset | port |
| `0x16E5C` | process_terrain_work_entries | 68 | decompilation/ghidra_conversion/src/field/terrain_workflow_conversion.cpp#ghidra_conversion::process_terrain_work_entries | port |
| `0x16EA0` | fill_terrain_work_region | 40 | decompilation/ghidra_conversion/src/field/terrain_workflow_conversion.cpp#ghidra_conversion::fill_terrain_work_region | port |
| `0x16F14` | mark_terrain_boundary_for_mode | 40 | decompilation/ghidra_conversion/src/field/terrain_workflow_conversion.cpp#ghidra_conversion::mark_terrain_boundary_for_mode | port |
| `0x16F3C` | mark_terrain_boundary_if_inside | 82 | decompilation/ghidra_conversion/src/field/terrain_workflow_conversion.cpp#ghidra_conversion::mark_terrain_boundary_if_inside | port |
| `0x16F8E` | copy_terrain_work_region_rows | 26 | decompilation/ghidra_conversion/src/field/terrain_workflow_conversion.cpp#ghidra_conversion::copy_terrain_work_region_rows | port |
| `0x16FA8` | copy_terrain_work_region | 44 | decompilation/ghidra_conversion/src/field/terrain_workflow_conversion.cpp#ghidra_conversion::copy_terrain_work_region | port |
| `0x17708` | render_field_hud | 156 | decompilation/ghidra_conversion/src/field/field_hud_conversion.cpp#ghidra_conversion::render_field_hud | port |
| `0x17D72` | draw_hud_panel | 30 | decompilation/ghidra_conversion/src/menus/menu_transfer_batch_conversion.cpp#ghidra_conversion::draw_hud_panel_forward | port |
| `0x17F8E` | draw_hud_footer | 38 | decompilation/ghidra_conversion/src/menus/menu_transfer_batch_conversion.cpp#ghidra_conversion::draw_hud_footer_forward | port |
| `0x18150` | Pocket Secretary Cyberdeck branch | — | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp#FieldSceneRuntime | port |
| `0x18450` | Pull Plug confirmation and logout | — | decompilation/gameplay/scripts/menu/screens/cyberdeck.lua;decompilation/gameplay/scripts/menu/rules/matrix.lua | port |
| `0x18D1A` | Matrix storage screen | — | decompilation/gameplay/scripts/menu/screens/deck_screens.lua | port |
| `0x18F5E` | Matrix datafile View/Erase flow | — | decompilation/gameplay/scripts/menu/screens/deck_screens.lua | port |
| `0x1952C` | matrix_program_storage_usage | 12 | decompilation/ghidra_conversion/src/cyberspace/matrix_storage_conversion.cpp#ghidra_conversion::matrix_program_storage_usage | port |
| `0x19538` | matrix_program_storage_weight | 18 | decompilation/ghidra_conversion/src/cyberspace/matrix_storage_conversion.cpp#ghidra_conversion::matrix_program_storage_weight | port |
| `0x1954A` | matrix_loaded_slot_usage | 14 | decompilation/ghidra_conversion/src/cyberspace/matrix_storage_conversion.cpp#ghidra_conversion::matrix_loaded_slot_usage | port |
| `0x19558` | calculate_matrix_storage_capacity | 38 | decompilation/ghidra_conversion/src/cyberspace/matrix_storage_conversion.cpp#ghidra_conversion::calculate_matrix_storage_capacity | port |
| `0x1957E` | calculate_loaded_program_capacity | 40 | decompilation/ghidra_conversion/src/cyberspace/matrix_storage_conversion.cpp#ghidra_conversion::calculate_loaded_program_capacity | port |
| `0x196F2` | rotate_hud_history | 68 | decompilation/gameplay/scripts/menu/deck_palette.lua | port |
| `0x19BAE` | select_hud_mode | 72 | decompilation/ghidra_conversion/src/field/field_hud_conversion.cpp#ghidra_conversion::select_hud_mode | port |
| `0x1A2CC` | allocate_first_free_effect_slot | 56 | decompilation/gameplay/scripts/actor/actions.lua#request_target_motion_effect | port |
| `0x1A304` | clear_effect_slot | 12 | decompilation/ghidra_conversion/src/entities/entity_animation_dispatch_conversion.cpp#ghidra_conversion::clear_effect_slot | port |
| `0x1A310` | clear_effect_slots | 18 | decompilation/ghidra_conversion/src/entities/entity_state_primitives_conversion.cpp#ghidra_conversion::clear_effect_slots | port |
| `0x1A322` | clear_entity_effect_pending_records | 32 | decompilation/ghidra_conversion/src/entities/entity_state_primitives_conversion.cpp#ghidra_conversion::clear_entity_effect_pending_records | port |
| `0x1A342` | find_first_available_world_effect_entity_slot | 46 | decompilation/ghidra_conversion/src/world/world_effect_conversion.cpp#ghidra_conversion::find_first_available_world_effect_entity_slot | port |
| `0x1A4EA` | lookup_runner_animation_delta | 36 | decompilation/ghidra_conversion/src/session/runner_record_access_conversion.cpp#ghidra_conversion::lookup_runner_animation_delta | port |
| `0x1A538` | runner_record_has_active_effect | 22 | decompilation/ghidra_conversion/src/session/runner_record_access_conversion.cpp#ghidra_conversion::runner_record_has_active_effect | port |
| `0x1B696` | Matrix Slow program entry | — | decompilation/gameplay/scripts/menu/screens/matrix_node_action.lua#start_slow | port |
| `0x1B734` | Matrix Attack program entry | — | decompilation/gameplay/scripts/menu/screens/matrix_node_action.lua#start_attack | port |
| `0x1B81A` | Matrix Degrade program entry | — | decompilation/gameplay/scripts/menu/screens/matrix_node_action.lua#start_degrade | port |
| `0x1BEBA` | advance_title_profile_transition | 162 | decompilation/gameplay/scripts/menu/screens/matrix_node_action.lua;decompilation/gameplay/scripts/menu/matrix_session.lua | port |
| `0x1C012` | copy_seven_words | 16 | decompilation/ghidra_conversion/src/world/world_effect_batch_conversion.cpp#ghidra_conversion::copy_seven_words | port |
| `0x1C4EE` | copy_eleven_words | 24 | decompilation/ghidra_conversion/src/world/world_effect_batch_conversion.cpp#ghidra_conversion::copy_eleven_words | port |
| `0x1CBF8` | clear_world_effect_work | 44 | decompilation/ghidra_conversion/src/world/world_effect_batch_conversion.cpp#ghidra_conversion::clear_world_effect_work | port |
| `0x1D1A4` | advance_effect_vertical_position | 16 | decompilation/ghidra_conversion/src/world/world_effect_batch_conversion.cpp#ghidra_conversion::advance_effect_vertical_position | port |
| `0x1D1B4` | retreat_effect_vertical_position | 16 | decompilation/ghidra_conversion/src/world/world_effect_batch_conversion.cpp#ghidra_conversion::retreat_effect_vertical_position | port |
| `0x1D1C4` | advance_effect_horizontal_position | 12 | decompilation/ghidra_conversion/src/world/world_effect_batch_conversion.cpp#ghidra_conversion::advance_effect_horizontal_position | port |
| `0x1D1D0` | retreat_effect_horizontal_position | 12 | decompilation/ghidra_conversion/src/world/world_effect_batch_conversion.cpp#ghidra_conversion::retreat_effect_horizontal_position | port |
| `0x1D1DC` | transfer_world_word_block | 24 | decompilation/ghidra_conversion/src/world/world_resource_batch_conversion.cpp#ghidra_conversion::transfer_world_word_block | port |
| `0x1D3F4` | expand_random_nibble_mask | 52 | decompilation/ghidra_conversion/src/world/world_resource_batch_conversion.cpp#ghidra_conversion::expand_random_nibble_mask | port |
| `0x1DBAA` | schedule_world_activity_timeout | 70 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp#ghidra_conversion::schedule_world_activity_timeout | port |
| `0x1DC22` | select_next_entity | 42 | decompilation/ghidra_conversion/src/integration/field_runtime_coordinator_conversion.cpp#ghidra_conversion::select_next_entity | port |
| `0x1DC4C` | select_world_entity_service_target | 328 | decompilation/gameplay/scripts/actor/actions.lua#advance_world_target_selection | port |
| `0x1DD94` | advance_world_frame_logic | 268 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp#ghidra_conversion::advance_world_frame_logic | port |
| `0x1DEA0` | clear_inactive_entity_effects | 56 | decompilation/ghidra_conversion/src/world/world_resource_batch_conversion.cpp#ghidra_conversion::clear_inactive_entity_effects | port |
| `0x1DED8` | clear_high_effect_slots | 40 | decompilation/gameplay/scripts/party_down.lua | port |
| `0x1DF00` | clear_entity_effect_pending_flags | 26 | decompilation/ghidra_conversion/src/world/world_effect_conversion.cpp#ghidra_conversion::clear_entity_effect_pending_flags | port |
| `0x1DF1A` | resolve_nearby_effects | 26 | decompilation/ghidra_conversion/src/world/world_resource_batch_conversion.cpp#ghidra_conversion::resolve_nearby_effects | port |
| `0x1DFC4` | advance_world_effect_trigger | 164 | decompilation/gameplay/scripts/field_alarm.lua | port |
| `0x1E04A` | begin_world_effect_transition | 80 | decompilation/ghidra_conversion/qt_runtime/src/field/field_interaction_runtime.cpp#ghidra_conversion::begin_world_effect_transition | port |
| `0x1E0B8` | promote_entity_effect_flags | 36 | decompilation/ghidra_conversion/src/world/world_effect_conversion.cpp#ghidra_conversion::promote_entity_effect_flags | port |
| `0x1E0DC` | spawn_world_effect_entities | 138 | decompilation/ghidra_conversion/src/world/world_effect_conversion.cpp#ghidra_conversion::spawn_world_effect_entities | port |
| `0x1E25A` | spawn_world_effect_from_coordinate_sequence | 134 | decompilation/ghidra_conversion/src/world/world_effect_conversion.cpp#ghidra_conversion::spawn_world_effect_from_coordinate_sequence | port |
| `0x1E2E0` | begin_world_activity_if_idle | 12 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp#ghidra_conversion::begin_world_activity_if_idle | port |
| `0x1E2EC` | begin_world_activity | 20 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp#ghidra_conversion::begin_world_activity | port |
| `0x1E302` | build_world_effect_edge_candidates | 266 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp#ghidra_conversion::build_world_effect_edge_candidates | port |
| `0x1E40C` | build_world_effect_candidates | 18 | decompilation/ghidra_conversion/src/world/world_effect_batch_conversion.cpp#ghidra_conversion::build_world_effect_candidates | port |
| `0x1E41E` | lookup_world_effect_tile | 30 | decompilation/ghidra_conversion/src/world/world_effect_batch_conversion.cpp#ghidra_conversion::lookup_world_effect_tile | port |
| `0x1E43C` | handle_world_activity_request | 112 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp#ghidra_conversion::handle_world_activity_request | port |
| `0x1E4AC` | spawn_world_effect_at_player_position | 84 | decompilation/ghidra_conversion/src/session/save_field_restore_conversion.cpp#ghidra_conversion::spawn_world_effect_at_player_position | port |
| `0x1E514` | clear_effect_slot_state | 44 | decompilation/ghidra_conversion/src/entities/entity_behavior_frame_service_conversion.cpp#ghidra_conversion::clear_effect_slot_state | port |
| `0x1E540` | initialize_effect_fade_marker | 18 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime_player_actions.cpp#ghidra_conversion::initialize_effect_fade_marker | port |
| `0x1E68C` | reset_world_effect_globals | 16 | decompilation/gameplay/scripts/run_registry.lua#clear_active_offer_after_party_wipe | port |
| `0x1E72C` | Notebook Current Run active-run detail dispatcher | — | decompilation/gameplay/scripts/menu/screens/pocket_secretary.lua | port |
| `0x1EEDC` | dispatch_world_effect | 80 | decompilation/ghidra_conversion/src/world/world_effect_batch_conversion.cpp#ghidra_conversion::dispatch_world_effect | port |
| `0x1EF2C` | Shared bodyguard/package run-event handler | — | decompilation/ghidra_conversion/src/scripting/game_script_runtime.cpp#GameScriptRuntime::resolve_active_run_field_event | port |
| `0x1F046` | Package-recovery target entry passage | — | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp#FieldSceneRuntime::run_field_exit_orchestrator | port |
| `0x1F086` | Gang-contract target entry passage | — | decompilation/ghidra_conversion/src/scripting/game_script_runtime.cpp#GameScriptRuntime::resolve_active_run_field_event | port |
| `0x1F106` | Employee-recruitment target entry passage | — | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp#FieldSceneRuntime::run_field_exit_orchestrator | port |
| `0x1F146` | Employee-recruitment random-contact page | — | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp#FieldSceneRuntime::resolve_default_actor_proximity | port |
| `0x1F16A` | World-activity extraction result callback | — | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp#FieldSceneRuntime | port |
| `0x1F1E0` | advance_eight_step_counter | 14 | decompilation/ghidra_conversion/src/world/world_effect_batch_conversion.cpp#ghidra_conversion::advance_eight_step_counter | port |
| `0x1F1FC` | Ghoul bounty objective-ready gate | — | decompilation/gameplay/scripts/run_registry.lua | port |
| `0x1F246` | run_mode_exit_services | 76 | decompilation/ghidra_conversion/src/field/field_exit_prompt_conversion.cpp#ghidra_conversion::run_mode_exit_services | port |
| `0x1F5CA` | run_field_exit_orchestrator | 144 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp#ghidra_conversion::run_field_exit_orchestrator | port |
| `0x1F65A` | select_field_exit_prompt | 470 | decompilation/ghidra_conversion/qt_runtime/src/dialogue/field_dialogue_runtime.cpp#ghidra_conversion::select_field_exit_prompt | port |
| `0x1F77C` | Wylie's Gala Inn mode-3/event-1 service | — | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp#FieldSceneRuntime | port |
| `0x1F7CE` | Council Island Inn mode-4/event-0 room service | — | decompilation/gameplay/scripts/run_support.lua#council_island_inn | port |
| `0x1FA78` | select_field_mode_service | 776 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp#ghidra_conversion::select_field_mode_service | port |
| `0x1FCF2` | Council Island mode-4/event-3 Medicine Lodge handler | — | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp#FieldSceneRuntime | port |
| `0x1FD80` | finalize_field_mode_exit | 2228 | decompilation/gameplay/scripts/salish_sinsearach_gate.lua | port |
| `0x20484` | Scene-2 event-4 indexed title service 22 | — | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp#FieldSceneRuntime | port |
| `0x20634` | select_mode_entry_resource | 224 | decompilation/ghidra_conversion/src/field/field_exit_orchestrator_conversion.cpp#ghidra_conversion::select_mode_entry_resource | port |
| `0x20714` | finalize_mode_exit | 86 | decompilation/gameplay/scripts/taxi_cabbie.lua;decompilation/gameplay/scripts/data/taxi_destinations.lua | port |
| `0x2076A` | taxi cabbie Yes branch | — | decompilation/gameplay/scripts/taxi_cabbie.lua;decompilation/gameplay/scripts/data/taxi_destinations.lua | port |
| `0x208A8` | confirm taxi fare and check funds | — | decompilation/gameplay/scripts/taxi_cabbie.lua;decompilation/gameplay/scripts/data/taxi_destinations.lua | port |
| `0x208FE` | accept taxi destination and consume visa | — | decompilation/gameplay/scripts/taxi_cabbie.lua | port |
| `0x20990` | Scene-specific field activity dialogue dispatcher | — | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp#FieldSceneRuntime | port |
| `0x20A12` | Matching-context run settlement service | — | decompilation/gameplay/scripts/run_registry.lua;decompilation/gameplay/scripts/johnson.lua | port |
| `0x20E48` | field electromagnetic-lock interaction continuation | — | decompilation/ghidra_conversion/qt_runtime/src/field/field_interaction_runtime.cpp#FieldInteractionRuntime | port |
| `0x20E7A` | electromagnetic-lock prompt setup | — | decompilation/ghidra_conversion/qt_runtime/src/field/field_interaction_runtime.cpp#FieldInteractionRuntime | port |
| `0x20E92` | electromagnetic-lock choice-grid writer | — | decompilation/ghidra_conversion/qt_runtime/src/field/field_interaction_runtime.cpp#FieldInteractionRuntime | port |
| `0x20EC2` | building Security Terminal continuation | — | decompilation/ghidra_conversion/qt_runtime/src/field/field_interaction_runtime.cpp#FieldInteractionRuntime | port |
| `0x211E8` | recruited runner conversation opener | — | decompilation/ghidra_conversion/src/integration/recruited_runner_conversation_conversion.cpp#ghidra_conversion::select_recruited_runner_conversation_prompt | port |
| `0x21226` | recruited runner service offer preparation | — | decompilation/ghidra_conversion/src/integration/recruited_runner_conversation_conversion.cpp#ghidra_conversion::prepare_recruited_runner_service_offer | port |
| `0x21318` | recruited runner free permanent-hire dispatch | — | decompilation/ghidra_conversion/src/session/recruited_runner_service_conversion.cpp | port |
| `0x21326` | recruited runner short-term payment dispatch | — | decompilation/ghidra_conversion/src/session/recruited_runner_service_conversion.cpp | port |
| `0x21332` | recruited runner permanent payment dispatch | — | decompilation/ghidra_conversion/src/session/recruited_runner_service_conversion.cpp | port |
| `0x213CA` | Stoker debt-collection follow-up command | — | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp#ghidra_conversion::resolve_stoker_motel_followup_command | port |
| `0x213E2` | Stoker Jump House referral command | — | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp#ghidra_conversion::resolve_stoker_motel_followup_command | port |
| `0x213FA` | Stoker motel debt-payment command | — | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp#ghidra_conversion::resolve_stoker_motel_debt_payment | port |
| `0x21458` | Stoker motel room payment command | — | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp#ghidra_conversion::resolve_stoker_motel_room_payment | port |
| `0x215DA` | Big Rhino Johnson work-choice callback | — | decompilation/gameplay/scripts/johnson.lua;decompilation/gameplay/scripts/run_registry.lua | port |
| `0x21616` | electromagnetic-lock resolution resource callback | — | decompilation/ghidra_conversion/qt_runtime/src/field/field_interaction_runtime.cpp#FieldInteractionRuntime | port |
| `0x21628` | Maglock Passkey quality and use handler | — | decompilation/ghidra_conversion/qt_runtime/src/field/field_interaction_runtime.cpp#FieldInteractionRuntime::dispatch_command | port |
| `0x21692` | Electronics skill check and no-skill result | — | decompilation/ghidra_conversion/qt_runtime/src/field/field_interaction_runtime.cpp#FieldInteractionRuntime::dispatch_command | port |
| `0x21862` | Tarislar City Inn dialogue command 0x14 | — | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp#FieldSceneRuntime | port |
| `0x218BC` | Tarislar City Inn dialogue command 0x16 | — | decompilation/gameplay/scripts/run_support.lua#tarislar_city_inn | port |
| `0x219A4` | Tarislar City Inn Holopix command 0x20 | — | decompilation/gameplay/scripts/run_support.lua#tarislar_city_inn | port |
| `0x21A4E` | subtract_resource_clamped | 24 | decompilation/ghidra_conversion/src/world/world_resource_batch_conversion.cpp#ghidra_conversion::subtract_resource_clamped | port |
| `0x21BE4` | Boris Greenhouse Strike Team command 0x2C | — | decompilation/gameplay/scripts/field_encounters.lua | port |
| `0x21C0C` | Boris Greenhouse Indian Lands Visa command 0x2D | — | decompilation/gameplay/scripts/dialogue_actions.lua#DialogueActions | port |
| `0x21C38` | Boris Greenhouse Maglock Passkey command 0x2E | — | decompilation/gameplay/scripts/dialogue_actions.lua#DialogueActions | port |
| `0x21C72` | Boris Greenhouse Mach 22 command 0x2F | — | decompilation/gameplay/scripts/dialogue_actions.lua#DialogueActions | port |
| `0x21CE4` | Sinsearach state-selected dialogue root selector command 0x34 | — | decompilation/gameplay/scripts/salish_sinsearach_gate.lua;decompilation/gameplay/scripts/dialogue_graph.lua | port |
| `0x21E66` | Sinsearach copt fare payment command 0x38 | — | decompilation/gameplay/scripts/salish_sinsearach_gate.lua | port |
| `0x21E8C` | Sinsearach gate-return command 0x39 | — | decompilation/gameplay/scripts/salish_sinsearach_gate.lua;decompilation/gameplay/scripts/dialogue_graph.lua | port |
| `0x21E9E` | Sinsearach copt-fare prompt command 0x3A | — | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp#FieldSceneRuntime | port |
| `0x21FD0` | Frosty handoff dialogue command 0x44 | — | decompilation/gameplay/scripts/salish_sinsearach_gate.lua;decompilation/gameplay/scripts/dialogue_graph.lua | port |
| `0x22018` | dialogue-state choice branch 0x46 | — | decompilation/gameplay/scripts/harlequin_dialogue.lua | port |
| `0x22072` | dialogue-state choice branch 0x47 | — | decompilation/gameplay/scripts/harlequin_dialogue.lua | port |
| `0x220EA` | Frosty conversation command 0x49 handler | — | decompilation/gameplay/scripts/salish_sinsearach_gate.lua;decompilation/gameplay/scripts/dialogue_graph.lua | port |
| `0x22112` | Tarislar City Inn aftermath command 0x4B | — | decompilation/gameplay/scripts/run_support.lua#tarislar_city_inn | port |
| `0x22124` | Tarislar City Inn aftermath command 0x4C | — | decompilation/gameplay/scripts/run_support.lua#tarislar_city_inn | port |
| `0x22136` | Gunderson Decker contact purchase command 0x4D | — | decompilation/ghidra_conversion/qt_runtime/src/dialogue/field_dialogue_runtime.cpp#FieldDialogueRuntime | port |
| `0x225D2` | Mako encounter entry handler for prompt command 0x60 | — | decompilation/gameplay/scripts/club_penumbra.lua;decompilation/gameplay/scripts/story_activity_routes.lua | port |
| `0x22874` | run_fixed_delay | 36 | decompilation/ghidra_conversion/src/world/world_resource_batch_conversion.cpp#ghidra_conversion::run_fixed_delay | port |
| `0x22934` | Alesandro Vid-Phone passkey handler and service graph | — | decompilation/gameplay/scripts/vid_phone/alesandro_hobbs.lua | port |
| `0x22C98` | Mr. Faradouchi Vid-Phone first/repeat affiliation handler | — | decompilation/gameplay/scripts/vid_phone/affiliations.lua | port |
| `0x22CE8` | Agira Tetsumi Vid-Phone first/repeat affiliation handler | — | decompilation/gameplay/scripts/vid_phone/affiliations.lua | port |
| `0x22F90` | mode-command conditional dialogue menu builder | — | decompilation/gameplay/scripts/johnson.lua | port |
| `0x23750` | count_active_entities | 20 | decompilation/ghidra_conversion/src/world/world_resource_batch_conversion.cpp#ghidra_conversion::count_active_entities | port |
| `0x2401C` | recruited runner Ricky information branch | — | decompilation/ghidra_conversion/src/integration/recruited_runner_conversation_conversion.cpp#ghidra_conversion::select_recruited_runner_information_prompt | port |
| `0x24034` | recruited runner Winston Marrs information branch | — | decompilation/ghidra_conversion/src/integration/recruited_runner_conversation_conversion.cpp#ghidra_conversion::select_recruited_runner_information_prompt | port |
| `0x2404C` | recruited runner Trent information branch | — | decompilation/ghidra_conversion/src/integration/recruited_runner_conversation_conversion.cpp#ghidra_conversion::select_recruited_runner_information_prompt | port |
| `0x24064` | recruited runner Petr Uvehr information branch | — | decompilation/ghidra_conversion/src/integration/recruited_runner_conversation_conversion.cpp#ghidra_conversion::select_recruited_runner_information_prompt | port |
| `0x2407C` | recruited runner Walking Bear information branch | — | decompilation/ghidra_conversion/src/integration/recruited_runner_conversation_conversion.cpp#ghidra_conversion::select_recruited_runner_information_prompt | port |
| `0x24094` | recruited runner Phantom information branch | — | decompilation/ghidra_conversion/src/integration/recruited_runner_conversation_conversion.cpp#ghidra_conversion::select_recruited_runner_information_prompt | port |
| `0x240AC` | recruited runner Two Fists information branch | — | decompilation/ghidra_conversion/src/integration/recruited_runner_conversation_conversion.cpp#ghidra_conversion::select_recruited_runner_information_prompt | port |
| `0x240C4` | recruited runner Freya information branch | — | decompilation/ghidra_conversion/src/integration/recruited_runner_conversation_conversion.cpp#ghidra_conversion::select_recruited_runner_information_prompt | port |
| `0x240DC` | recruited runner Rianna information branch | — | decompilation/ghidra_conversion/src/integration/recruited_runner_conversation_conversion.cpp#ghidra_conversion::select_recruited_runner_information_prompt | port |
| `0x240F4` | recruited runner Stark information branch | — | decompilation/ghidra_conversion/src/integration/recruited_runner_conversation_conversion.cpp#ghidra_conversion::select_recruited_runner_information_prompt | port |
| `0x24252` | service_random_roll | 16 | decompilation/ghidra_conversion/src/world/world_resource_batch_conversion.cpp#ghidra_conversion::service_random_roll | port |
| `0x24964` | Trent Jefferson contact information command | — | decompilation/ghidra_conversion/src/integration/recruited_runner_conversation_conversion.cpp#ghidra_conversion::select_trent_information_response | port |
| `0x2497C` | Tarislar City Inn Frosty information command 0xE6 | — | decompilation/gameplay/scripts/run_support.lua#tarislar_city_inn | port |
| `0x249A4` | mode-command action 0xE7 Vigore response | — | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp#FieldSceneRuntime | port |
| `0x249C4` | Stoker motel arrival command / rare street ambush | — | decompilation/ghidra_conversion/qt_runtime/src/dialogue/field_dialogue_runtime.cpp#ghidra_conversion::resolve_stoker_motel_arrival_command | port |
| `0x24A1E` | ROM command 0xEB Heavy Combat Armor purchase handler | — | decompilation/gameplay/scripts/dialogue_actions.lua#register_core_building_purchases;decompilation/gameplay/scripts/enums.lua | port |
| `0x24A56` | ROM command 0xEC Power Focus phase-4 purchase handler | — | decompilation/gameplay/scripts/dialogue_actions.lua#register_core_building_purchases;decompilation/gameplay/scripts/enums.lua | port |
| `0x24E20` | dispatch_indexed_handler | 26 | decompilation/ghidra_conversion/src/world/world_resource_batch_conversion.cpp#ghidra_conversion::dispatch_indexed_handler | port |
| `0x26074` | clear_grid_segments | 58 | decompilation/ghidra_conversion/src/world/world_resource_batch_conversion.cpp#ghidra_conversion::clear_grid_segments | port |
| `0x26610` | write_sequential_asset_grid | 2 | decompilation/ghidra_conversion/src/world/world_resource_batch_conversion.cpp#ghidra_conversion::write_sequential_asset_grid | port |
| `0x26696` | copy_tile_blocks | 78 | decompilation/ghidra_conversion/src/world/world_resource_batch_conversion.cpp#ghidra_conversion::copy_tile_blocks | port |
| `0x26938` | copy_two_word_rows | 32 | decompilation/ghidra_conversion/src/integration/field_session_bootstrap_conversion.cpp#ghidra_conversion::copy_field_mode_selection_state | port |
| `0x26958` | upload_decoded_source_stream | 26 | decompilation/ghidra_conversion/src/world/world_resource_batch_conversion.cpp#ghidra_conversion::upload_decoded_source_stream | port |
| `0x26BFE` | write_masked_transfer | 40 | decompilation/ghidra_conversion/src/audio/audio_service_batch_conversion.cpp#ghidra_conversion::write_masked_transfer | port |
| `0x26C26` | write_masked_transfer_with_display_control | 56 | decompilation/ghidra_conversion/src/audio/audio_service_batch_conversion.cpp#ghidra_conversion::write_masked_transfer | port |
| `0x52DC2` | begin_controller_bus_access | 20 | decompilation/ghidra_conversion/src/audio/audio_command_batch_conversion.cpp#ghidra_conversion::begin_controller_bus_access | port |
| `0x52DD6` | end_controller_bus_access | 10 | decompilation/ghidra_conversion/src/audio/audio_command_batch_conversion.cpp#ghidra_conversion::end_controller_bus_access | port |
| `0x52DE0` | initialize_audio_ram | 70 | decompilation/ghidra_conversion/src/audio/audio_command_batch_conversion.cpp#ghidra_conversion::initialize_audio_ram | port |
| `0x52E26` | reset_audio_processor | 40 | decompilation/ghidra_conversion/src/audio/audio_command_batch_conversion.cpp#ghidra_conversion::reset_audio_processor | port |
| `0x52E4E` | begin_audio_bus_access | 54 | decompilation/ghidra_conversion/src/audio/audio_command_batch_conversion.cpp#ghidra_conversion::begin_audio_bus_access | port |
| `0x52E84` | end_audio_bus_access | 18 | decompilation/ghidra_conversion/src/audio/audio_command_batch_conversion.cpp#ghidra_conversion::end_audio_bus_access | port |
| `0x52E96` | flush_audio_command | 12 | decompilation/ghidra_conversion/src/audio/audio_command_batch_conversion.cpp#ghidra_conversion::flush_audio_command | port |
| `0x52EA2` | append_audio_command | 14 | decompilation/ghidra_conversion/src/audio/audio_command_batch_conversion.cpp#ghidra_conversion::append_audio_command | port |
| `0x52EB0` | append_one_with_audio_bus | 22 | decompilation/ghidra_conversion/src/audio/audio_command_batch_conversion.cpp#ghidra_conversion::append_one_with_audio_bus | port |
| `0x52EC6` | append_three_with_audio_bus | 38 | decompilation/ghidra_conversion/src/audio/audio_command_batch_conversion.cpp#ghidra_conversion::append_three_with_audio_bus | port |
| `0x52EEC` | initialize_audio_sequence | 80 | decompilation/ghidra_conversion/src/audio/audio_service_batch_conversion.cpp#ghidra_conversion::initialize_audio_sequence | port |
| `0x52F3C` | service_audio_command_queue | 30 | decompilation/ghidra_conversion/src/audio/audio_command_batch_conversion.cpp#ghidra_conversion::service_audio_command_queue_batch | port |
| `0x52F5A` | run_audio_bus_transaction | 10 | decompilation/ghidra_conversion/src/audio/audio_service_batch_conversion.cpp#ghidra_conversion::run_audio_bus_transaction | port |
| `0x52F6E` | run_audio_transaction_wrapper | 10 | decompilation/ghidra_conversion/src/audio/audio_service_batch_conversion.cpp#ghidra_conversion::run_audio_transaction_wrapper | port |
| `0x52FA0` | flush_with_audio_bus | 20 | decompilation/ghidra_conversion/src/audio/audio_command_batch_conversion.cpp#ghidra_conversion::flush_with_audio_bus | port |
| `0x52FD2` | flush_append_pair | 32 | decompilation/ghidra_conversion/src/audio/audio_command_batch_conversion.cpp#ghidra_conversion::flush_append_pair | port |
| `0x53094` | begin_bus_then_flush_append_pair | 12 | decompilation/ghidra_conversion/src/audio/audio_command_batch_conversion.cpp#ghidra_conversion::begin_bus_then_flush_append_pair | port |
| `0x530A0` | controller_access_pair | 38 | decompilation/ghidra_conversion/src/audio/audio_command_batch_conversion.cpp#ghidra_conversion::controller_access_pair | port |
| `0x530D2` | service_audio_if_enabled_variant | 12 | decompilation/ghidra_conversion/src/audio/audio_command_batch_conversion.cpp#ghidra_conversion::service_audio_if_enabled_variant | port |
| `0x530EA` | service_audio_command_frame | 40 | decompilation/ghidra_conversion/src/audio/audio_command_batch_conversion.cpp#ghidra_conversion::service_audio_command_frame_variant | port |
| `0x53112` | service_audio_command_wrapper | 20 | decompilation/ghidra_conversion/src/audio/audio_command_batch_conversion.cpp#ghidra_conversion::service_audio_command_wrapper | port |
| `0x53126` | prepare_mode_entry_service | 48 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp#ghidra_conversion::prepare_audio_mode_entry | port |
| `0x53156` | service_audio_tick_wrapper | 8 | decompilation/ghidra_conversion/src/audio/audio_command_batch_conversion.cpp#ghidra_conversion::service_audio_tick_wrapper | port |
| `0x5315E` | run_fixed_audio_transaction_wrapper | 20 | decompilation/ghidra_conversion/src/audio/audio_service_batch_conversion.cpp#ghidra_conversion::run_audio_transaction_wrapper | port |
| `0x53172` | run_fixed_audio_pair | 28 | decompilation/ghidra_conversion/src/audio/audio_service_batch_conversion.cpp#ghidra_conversion::run_fixed_audio_pair | port |
| `0x53194` | run_controller_access | 22 | decompilation/ghidra_conversion/src/audio/audio_service_batch_conversion.cpp#ghidra_conversion::run_controller_access | port |
| `0x531AA` | initialize_title_audio_assets | 40 | decompilation/ghidra_conversion/src/audio/audio_service_batch_conversion.cpp#ghidra_conversion::initialize_title_audio_assets | port |
| `0x531EA` | copy_audio_state | 22 | decompilation/ghidra_conversion/src/audio/audio_service_batch_conversion.cpp#ghidra_conversion::copy_audio_state | port |
| `0x5322C` | prepare_audio_title | 10 | decompilation/ghidra_conversion/src/audio/audio_service_batch_conversion.cpp#ghidra_conversion::prepare_audio_title | port |
| `0x53248` | build_title_sprite_once | 72 | decompilation/ghidra_conversion/src/audio/audio_service_batch_conversion.cpp#ghidra_conversion::build_title_sprite_once | port |
| `0x53290` | reset_audio_menu_arrays | 24 | decompilation/ghidra_conversion/src/audio/audio_service_batch_conversion.cpp;decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp | port |
| `0x53A24` | begin_title_audio_sequence | 84 | decompilation/ghidra_conversion/src/audio/audio_service_batch_conversion.cpp#ghidra_conversion::begin_title_audio_sequence | port |
| `0x53A78` | run_indexed_title_services | 62 | decompilation/ghidra_conversion/src/audio/audio_service_batch_conversion.cpp#ghidra_conversion::run_indexed_title_services | port |
| `0x53AB6` | Stoker indexed title service 0 | — | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp#FieldSceneRuntime | port |
| `0x53B16` | Tarislar City Inn indexed title service 1 | — | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp#FieldSceneRuntime | port |
| `0x53BCC` | indexed service 3 / Mitsuhama entry route | — | decompilation/ghidra_conversion/src/scripting/game_script_runtime.cpp#GameScriptRuntime::resolve_field_location_route | port |
| `0x53C90` | Aragorn A8AE world-effect callback | — | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp#FieldSceneRuntime::resolve_default_actor_proximity | port |
| `0x53D5A` | Council Island Spirit Eyes event-27 ritual | — | decompilation/gameplay/scripts/salish_spirit_eyes.lua | port |
| `0x53EE2` | Scene-47 mode-table Renraku text page | — | decompilation/gameplay/scripts/salish_spirit_eyes.lua | port |
| `0x53EFA` | Licourtrix Scene-47 contextual event-25 encounter | — | decompilation/gameplay/scripts/salish_spirit_eyes.lua | port |
| `0x53F8C` | Scene-2/event-6 Boris Greenhouse indexed title service 10 | — | decompilation/gameplay/scripts/boris_greenhouse.lua | port |
| `0x53FFC` | Mako encounter actor-list outcome callback | — | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp#FieldSceneRuntime::advance_default_field_actors | port |
| `0x54076` | Renraku Offices title service 12 | — | decompilation/gameplay/scripts/run_support.lua#renraku_offices | port |
| `0x54152` | Fuchi cyber-heart title service 14 | — | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp#FieldSceneRuntime | port |
| `0x54222` | Gates Undersound Ogami departure title service 15 | — | decompilation/gameplay/scripts/menu/rules/datastore_special.lua#gates_undersound | port |
| `0x54322` | Red Buffalo Woman title service 18 | — | decompilation/gameplay/scripts/harlequin_dialogue.lua | port |
| `0x543A2` | Scene-2/event-0 Ilene prerequisite title service 19 | — | decompilation/gameplay/scripts/ilene.lua#prepare_scene_20_entry | port |
| `0x54402` | Scene-20 Ilene Two Fists story activity | — | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp#FieldSceneRuntime::resolve_default_actor_proximity | port |
| `0x5447E` | Scene-1/event-7 Tarislar Apartments service 21 | — | decompilation/gameplay/scripts/tarislar_apartments.lua;decompilation/gameplay/scripts/harlequin_dialogue.lua;decompilation/gameplay/scripts/dialogue_graph.lua | port |
| `0x54A8E` | advance_random_mode_event | 92 | decompilation/gameplay/scripts/menu/rules/node_actions.lua;decompilation/gameplay/scripts/menu/screens/matrix_node_action.lua | port |
| `0x54B34` | Matrix Crash System | — | decompilation/gameplay/scripts/menu/rules/node_actions.lua;decompilation/gameplay/scripts/menu/screens/matrix_node_action.lua | port |
| `0x54BD2` | Turn Off Node | — | decompilation/gameplay/scripts/menu/rules/node_actions.lua;decompilation/gameplay/scripts/menu/screens/matrix_node_action.lua | port |
| `0x54C5E` | Transfer Data datastore | — | decompilation/gameplay/scripts/menu/rules/node_actions.lua;decompilation/gameplay/scripts/menu/screens/matrix_node_action.lua | port |
| `0x55058` | Special datastore story-file handler | — | decompilation/gameplay/scripts/menu/rules/node_actions.lua;decompilation/ghidra_conversion/src/scripting/cyberdeck_menu_bindings.cpp | port |
| `0x5559E` | Erase datastore | — | decompilation/gameplay/scripts/menu/rules/node_actions.lua;decompilation/gameplay/scripts/menu/screens/matrix_node_action.lua | port |
| `0x55640` | resolve_entity_action_pair | 134 | decompilation/ghidra_conversion/src/entities/entity_action_math_conversion.cpp#ghidra_conversion::resolve_entity_action_pair | port |
| `0x556C6` | check_entity_action_eligibility | 122 | decompilation/ghidra_conversion/src/entities/entity_action_math_conversion.cpp#ghidra_conversion::check_entity_action_eligibility | port |
| `0x55740` | apply_target_condition_modifier | 70 | decompilation/ghidra_conversion/src/entities/entity_action_math_conversion.cpp#ghidra_conversion::apply_target_condition_modifier | port |
| `0x55786` | apply_dynamic_status_modifier | 162 | decompilation/ghidra_conversion/src/entities/entity_action_math_conversion.cpp#ghidra_conversion::apply_dynamic_status_modifier | port |
| `0x55828` | select_entity_action_profile_row | 32 | decompilation/ghidra_conversion/src/entities/entity_action_math_conversion.cpp#ghidra_conversion::select_entity_action_profile_row | port |
| `0x55848` | calculate_effect_target_offset | 184 | decompilation/ghidra_conversion/src/entities/entity_action_math_conversion.cpp#ghidra_conversion::calculate_effect_target_offset | port |
| `0x55C76` | advance_entity_action_resolution | 492 | decompilation/gameplay/scripts/combat.lua#resolve_reaction | port |
| `0x55E62` | apply_equipment_defense_action_modifier | 48 | decompilation/ghidra_conversion/src/menus/equipment_combat_bar_conversion.cpp#ghidra_conversion::apply_equipment_defense_action_modifier | port |
| `0x55ED6` | calculate_equipment_command_average | 44 | decompilation/ghidra_conversion/src/entities/entity_action_math_conversion.cpp#ghidra_conversion::calculate_equipment_command_average | port |
| `0x55F02` | calculate_item18_skill_value | 30 | decompilation/ghidra_conversion/src/entities/entity_action_math_conversion.cpp#ghidra_conversion::calculate_item18_skill_value | port |
| `0x55F20` | select_posture_adjusted_value | 36 | decompilation/ghidra_conversion/src/entities/entity_action_math_conversion.cpp#ghidra_conversion::select_posture_adjusted_value | port |
| `0x564D6` | reduce_menu_progression_timing | 48 | decompilation/gameplay/scripts/menu/rules/matrix_combat.lua;decompilation/gameplay/scripts/menu/screens/matrix_node_action.lua | port |
| `0x566A2` | copy_action_selection_profile_table | 32 | decompilation/ghidra_conversion/src/entities/entity_action_profile_conversion.cpp#ghidra_conversion::copy_action_selection_profile_table | port |
| `0x566C2` | copy_mode_selection_state_table | 22 | decompilation/ghidra_conversion/src/field/field_resource_sequence_conversion.cpp#ghidra_conversion::copy_mode_selection_state_table | port |
| `0x56AC0` | run_field_mode_service | 772 | decompilation/ghidra_conversion/src/field/field_mode_service_conversion.cpp#ghidra_conversion::run_field_mode_service | port |
| `0x56F4A` | remove_runner_action | — | decompilation/ghidra_conversion/qt_runtime/src/menus/shop_menu_runtime.cpp#ShopTransactionService::sell | port |
| `0x5787C` | calculate_field_action_quantity_cost | 72 | decompilation/ghidra_conversion/src/field/field_action_cost_conversion.cpp#ghidra_conversion::calculate_field_action_quantity_cost | port |
| `0x5794E` | apply_selected_field_action | 246 | decompilation/ghidra_conversion/qt_runtime/src/menus/shop_menu_runtime.cpp#ghidra_conversion::apply_selected_field_action | port |
| `0x57A44` | calculate_field_action_record_cost | 42 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime_shop_paint.cpp#ghidra_conversion::calculate_field_action_record_cost | port |
| `0x57A6E` | apply_field_action_cost_discounts | 136 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp#ghidra_conversion::apply_field_action_cost_discounts | port |
| `0x57AF6` | apply_runner_cost_surcharge | 14 | decompilation/ghidra_conversion/qt_runtime/src/menus/shop_menu_runtime.cpp#ghidra_conversion::apply_runner_cost_surcharge | port |
| `0x57B04` | calculate_runner_cost_adjustment | 62 | decompilation/ghidra_conversion/src/field/field_action_cost_conversion.cpp#ghidra_conversion::calculate_runner_cost_adjustment | port |
| `0x57B42` | apply_large_field_action_cost_discount | 74 | decompilation/ghidra_conversion/src/field/field_action_cost_conversion.cpp#ghidra_conversion::apply_large_field_action_cost_discount | port |
| `0x57B8C` | run_menu_confirmation_prompt | 174 | decompilation/ghidra_conversion/src/menus/menu_confirmation_conversion.cpp#ghidra_conversion::run_menu_confirmation_prompt | port |
| `0x57D7E` | render_field_action_quantity | 368 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp#ghidra_conversion::prepare_field_action_progression_quote | port |
| `0x5878E` | render_field_action_detail_text | 76 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime_shop_paint.cpp#ghidra_conversion::select_field_action_detail_text | port |
| `0x58B5E` | refresh_field_mode_panel_grid | 88 | decompilation/ghidra_conversion/src/field/field_action_selection_navigation_conversion.cpp#ghidra_conversion::refresh_field_action_selection_panel_grid | port |
| `0x58BB6` | render_field_mode_selection | 158 | decompilation/ghidra_conversion/src/field/field_action_selection_navigation_conversion.cpp#ghidra_conversion::render_field_action_selection_panel | port |
| `0x58C54` | advance_field_mode_primary_cursor | 138 | decompilation/ghidra_conversion/src/field/field_action_selection_navigation_conversion.cpp#ghidra_conversion::advance_field_action_selection_cursor | port |
| `0x58CDE` | advance_field_mode_panel_cursor | 84 | decompilation/ghidra_conversion/src/field/field_action_selection_navigation_conversion.cpp#ghidra_conversion::advance_field_action_selection_panel | port |
| `0x58D32` | refresh_field_mode_selection_sprites | 76 | decompilation/ghidra_conversion/src/field/field_action_selection_navigation_conversion.cpp#ghidra_conversion::refresh_field_action_selection_sprites | port |
| `0x58D94` | Event-29 selector dispatch | — | decompilation/ghidra_conversion/src/scripting/game_script_runtime.cpp#GameScriptRuntime::resolve_field_location_entry | port |
| `0x58DB2` | Changed-floor scene handoff | — | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime_travel.cpp#FieldSceneRuntime::advance_field_elevator_scene_transition | port |
| `0x58DE0` | Floor selector state and input | — | decompilation/ghidra_conversion/qt_runtime/src/field/elevator_selector_runtime.hpp#ElevatorSelectorRuntime | port |
| `0x59092` | run_start_options | 432 | decompilation/ghidra_conversion/src/integration/startup_integration_conversion.cpp#ghidra_conversion::enter_start_options_menu | port |
| `0x59172` | run_save_load | 214 | decompilation/ghidra_conversion/src/integration/startup_integration_conversion.cpp#ghidra_conversion::enter_save_load_menu | port |
| `0x59248` | Joshua Samurai / Decker / Gator Shaman selection | — | decompilation/ghidra_conversion/src/integration/startup_integration_conversion.cpp#ghidra_conversion::enter_joshua_archetype_menu | port |
| `0x59318` | initialize_save_roster_buffer | 72 | decompilation/ghidra_conversion/src/integration/startup_integration_conversion.cpp#ghidra_conversion::initialize_save_roster_buffer | port |
| `0x594E0` | restore_saved_field_state | 316 | decompilation/ghidra_conversion/src/integration/startup_integration_conversion.cpp#ghidra_conversion::restore_saved_field_state | port |
| `0x5966E` | refresh_runner_record_population | 86 | decompilation/ghidra_conversion/src/session/runner_roster_conversion.cpp#ghidra_conversion::refresh_runner_record_population | port |
| `0x59798` | serialize_runner_record_population | 86 | decompilation/ghidra_conversion/src/field/field_runner_state_asset_refresh_conversion.cpp#ghidra_conversion::serialize_runner_record_population | port |
| `0x597EE` | project_live_runner_record_to_compact | 194 | decompilation/ghidra_conversion/src/session/runner_roster_conversion.cpp#ghidra_conversion::project_live_runner_record_to_compact | port |
| `0x59C34` | adjust_runner_source_byte_triplet | 20 | decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp#ghidra_conversion::award_party_karma | port |
| `0x59D12` | enter_post_intro_mode | 44 | decompilation/ghidra_conversion/src/integration/opening_intro_conversion.cpp | port |
| `0x59D7C` | run_title_intro_stage2 | 510 | decompilation/ghidra_conversion/src/title_intro/title_intro_stage2_conversion.cpp#ghidra_conversion::run_title_intro_stage2 | port |
| `0x59F7A` | reset_title_intro_overlay_schedule | 20 | decompilation/ghidra_conversion/src/integration/opening_intro_conversion.cpp#ghidra_conversion::reset_title_intro_overlay_schedule | port |
| `0x59F8E` | advance_title_intro_overlay | 242 | decompilation/ghidra_conversion/src/integration/opening_intro_conversion.cpp#ghidra_conversion::advance_title_intro_overlay | port |
| `0x5A0A4` | reset_title_intro_sequence | 14 | decompilation/ghidra_conversion/src/integration/opening_intro_conversion.cpp#ghidra_conversion::reset_title_intro_sequence | port |
| `0x5A0B2` | advance_title_intro_sequence | 288 | decompilation/ghidra_conversion/src/integration/opening_intro_conversion.cpp#ghidra_conversion::advance_title_intro_sequence | port |
| `0x5A1EA` | advance_title_intro_animation | 90 | decompilation/ghidra_conversion/src/integration/opening_intro_conversion.cpp#ghidra_conversion::advance_title_intro_animation | port |
| `0x5A244` | reset_title_intro_persistent_sequence | 14 | decompilation/ghidra_conversion/src/integration/opening_intro_conversion.cpp#ghidra_conversion::reset_title_intro_persistent_sequence | port |
| `0x5A252` | advance_title_intro_persistent_sequence | 298 | decompilation/ghidra_conversion/src/integration/opening_intro_conversion.cpp#ghidra_conversion::advance_title_intro_persistent_sequence | port |
| `0x5A37C` | render_title_intro_scene_entities | 28 | decompilation/ghidra_conversion/src/title_intro/title_intro_transfer_conversion.cpp#ghidra_conversion::render_title_intro_scene_entities | port |
| `0x5A3B0` | reset_title_intro_motion | 14 | decompilation/ghidra_conversion/src/integration/opening_intro_conversion.cpp#ghidra_conversion::reset_title_intro_motion | port |
| `0x5A3BE` | advance_title_intro_motion | 196 | decompilation/ghidra_conversion/src/integration/opening_intro_conversion.cpp#ghidra_conversion::advance_title_intro_motion | port |
| `0x5A4B4` | reset_title_intro_dual_sprites | 14 | decompilation/ghidra_conversion/src/integration/opening_intro_conversion.cpp#ghidra_conversion::reset_title_intro_dual_sprites | port |
| `0x5A4C2` | advance_title_intro_dual_sprites | 160 | decompilation/ghidra_conversion/src/integration/opening_intro_conversion.cpp#ghidra_conversion::advance_title_intro_dual_sprites | port |
| `0x5A572` | reset_title_intro_timed_sprites | 16 | decompilation/ghidra_conversion/src/integration/opening_intro_conversion.cpp#ghidra_conversion::reset_title_intro_timed_sprites | port |
| `0x5A582` | begin_title_intro_timed_sprites | 26 | decompilation/ghidra_conversion/src/integration/opening_intro_conversion.cpp#ghidra_conversion::begin_title_intro_timed_sprites | port |
| `0x5A59C` | advance_title_intro_timed_sprites | 102 | decompilation/ghidra_conversion/src/integration/opening_intro_conversion.cpp#ghidra_conversion::advance_title_intro_timed_sprites | port |
| `0x5A61A` | reset_title_intro_targeted_sweep | 14 | decompilation/ghidra_conversion/src/integration/opening_intro_conversion.cpp#ghidra_conversion::reset_title_intro_targeted_sweep | port |
| `0x5A628` | advance_title_intro_targeted_sweep | 160 | decompilation/ghidra_conversion/src/integration/opening_intro_conversion.cpp#ghidra_conversion::advance_title_intro_targeted_sweep | port |
| `0x5A70E` | reset_title_intro_linear_sweep_a | 14 | decompilation/ghidra_conversion/src/integration/opening_intro_conversion.cpp#ghidra_conversion::reset_title_intro_linear_sweep_a | port |
| `0x5A71C` | advance_title_intro_linear_sweep_a | 142 | decompilation/ghidra_conversion/src/integration/opening_intro_conversion.cpp#ghidra_conversion::advance_title_intro_linear_sweep_a | port |
| `0x5A7EA` | reset_title_intro_linear_sweep_b | 14 | decompilation/ghidra_conversion/src/integration/opening_intro_conversion.cpp#ghidra_conversion::reset_title_intro_linear_sweep_b | port |
| `0x5A7F8` | advance_title_intro_linear_sweep_b | 144 | decompilation/ghidra_conversion/src/integration/opening_intro_conversion.cpp#ghidra_conversion::advance_title_intro_linear_sweep_b | port |
| `0x5A8C8` | reset_title_intro_final_projection | 22 | decompilation/ghidra_conversion/src/integration/opening_intro_conversion.cpp#ghidra_conversion::reset_title_intro_final_projection | port |
| `0x5A8DE` | advance_title_intro_final_projection | 122 | decompilation/ghidra_conversion/src/integration/opening_intro_conversion.cpp#ghidra_conversion::advance_title_intro_final_projection | port |
| `0x5A96A` | render_title_intro_text_batch | 120 | decompilation/ghidra_conversion/src/integration/opening_intro_conversion.cpp#ghidra_conversion::render_title_intro_text_batch | port |
| `0x5A9E2` | render_title_intro_middle_text_batch | 60 | decompilation/ghidra_conversion/src/integration/opening_intro_conversion.cpp#ghidra_conversion::render_title_intro_middle_text_batch | port |
| `0x5AA1E` | render_title_intro_center_text_batch | 60 | decompilation/ghidra_conversion/src/integration/opening_intro_conversion.cpp#ghidra_conversion::render_title_intro_center_text_batch | port |
| `0x5AA5A` | render_title_intro_stage_text_batch | 60 | decompilation/ghidra_conversion/src/integration/opening_intro_conversion.cpp#ghidra_conversion::render_title_intro_stage_text_batch | port |
| `0x5AA96` | run_title_intro_orchestrator | 256 | decompilation/ghidra_conversion/src/title_intro/title_intro_orchestrator_conversion.cpp#ghidra_conversion::run_title_intro_orchestrator | port |
| `0x5AB96` | advance_title_intro_stage_service_steps | 60 | decompilation/ghidra_conversion/src/integration/opening_intro_conversion.cpp#ghidra_conversion::advance_title_intro_stage_service_steps | port |
| `0x5ABD2` | service_title_intro_stage_frame | 60 | decompilation/ghidra_conversion/src/integration/opening_intro_conversion.cpp#ghidra_conversion::service_title_intro_stage_frame | port |
| `0x5AC0E` | draw_title_intro_stage_base_sprite | 26 | decompilation/ghidra_conversion/src/title_intro/title_intro_stage_conversion.cpp#ghidra_conversion::draw_title_intro_stage_base_sprite | port |
| `0x5AC28` | reset_title_intro_stage_projection | 14 | decompilation/ghidra_conversion/src/integration/opening_intro_conversion.cpp#ghidra_conversion::reset_title_intro_stage_projection | port |
| `0x5AC36` | advance_title_intro_stage_projection | 58 | decompilation/ghidra_conversion/src/title_intro/title_intro_stage_conversion.cpp#ghidra_conversion::advance_title_intro_stage_projection | port |
| `0x5AC70` | reset_title_intro_stage_timed_sprites | 14 | decompilation/ghidra_conversion/src/integration/opening_intro_conversion.cpp#ghidra_conversion::reset_title_intro_stage_timed_sprites | port |
| `0x5AC7E` | advance_title_intro_stage_timed_sprites | 132 | decompilation/ghidra_conversion/src/title_intro/title_intro_stage_conversion.cpp#ghidra_conversion::advance_title_intro_stage_timed_sprites | port |
| `0x5AD1A` | run_title_intro_stage_loop | 182 | decompilation/ghidra_conversion/src/title_intro/title_intro_stage_loop_conversion.cpp#ghidra_conversion::run_title_intro_stage_loop | port |
| `0x5ADFE` | render_title_intro_late_text_batch | 60 | decompilation/ghidra_conversion/src/integration/opening_intro_conversion.cpp#ghidra_conversion::render_title_intro_late_text_batch | port |
| `0x5AE3A` | render_title_intro_scroll_text_batch | 100 | decompilation/ghidra_conversion/src/integration/opening_intro_conversion.cpp#ghidra_conversion::render_title_intro_scroll_text_batch | port |
| `0x5AF2A` | advance_title_intro_schedule | 56 | decompilation/ghidra_conversion/src/integration/opening_intro_conversion.cpp#ghidra_conversion::advance_title_intro_schedule | port |
| `0x5AF62` | run_title_intro_schedule | 160 | decompilation/ghidra_conversion/src/title_intro/title_intro_schedule_conversion.cpp#ghidra_conversion::run_title_intro_schedule | port |
| `0x5B002` | reset_title_intro_scroll_fill | 8 | decompilation/ghidra_conversion/src/integration/opening_intro_conversion.cpp#ghidra_conversion::reset_title_intro_scroll_fill | port |
| `0x5B00A` | advance_title_intro_scroll | 76 | decompilation/ghidra_conversion/src/integration/opening_intro_conversion.cpp#ghidra_conversion::advance_title_intro_scroll | port |
| `0x5B056` | advance_title_intro_scroll_fade | 56 | decompilation/ghidra_conversion/src/integration/opening_intro_conversion.cpp#ghidra_conversion::advance_title_intro_scroll_fade | port |
| `0x5B08E` | advance_title_intro_triplet | 46 | decompilation/gameplay/scripts/startup.lua#advance_opening_tail_rotor | port |
| `0x5B0BC` | service_title_intro_animation | 54 | decompilation/ghidra_conversion/src/integration/opening_intro_conversion.cpp#ghidra_conversion::service_title_intro_animation | port |
| `0x5B100` | advance_title_intro_pulse | 128 | decompilation/ghidra_conversion/src/title_intro/title_intro_late_conversion.cpp#ghidra_conversion::advance_title_intro_pulse | port |
| `0x5B198` | reset_title_intro_landing | 30 | decompilation/ghidra_conversion/src/integration/opening_intro_conversion.cpp#ghidra_conversion::reset_title_intro_landing | port |
| `0x5B1B6` | advance_title_intro_landing | 186 | decompilation/ghidra_conversion/src/title_intro/title_intro_late_conversion.cpp#ghidra_conversion::advance_title_intro_landing | port |
| `0x5B29A` | clear_title_intro_display_buffer | 40 | decompilation/ghidra_conversion/src/title_intro/title_intro_orchestrator_conversion.cpp#ghidra_conversion::clear_title_intro_display_buffer | port |
| `0x5B2C2` | set_title_intro_entity_color_words | 32 | decompilation/ghidra_conversion/src/integration/opening_intro_conversion.cpp#ghidra_conversion::set_title_intro_entity_color_words | port |
| `0x5B2E2` | step_title_intro_entity_color_words | 74 | decompilation/ghidra_conversion/src/integration/opening_intro_conversion.cpp#ghidra_conversion::step_title_intro_entity_color_words | port |
| `0x5B32C` | fill_title_intro_word_block | 56 | decompilation/ghidra_conversion/src/integration/opening_intro_conversion.cpp#ghidra_conversion::fill_title_intro_word_block | port |
| `0x5BA4C` | load_scene_resources_thunk | 4 | decompilation/ghidra_conversion/src/integration/field_session_source_assets_conversion.cpp;decompilation/ghidra_conversion/src/integration/startup_integration_conversion.cpp | replaced |
| `0x5BA98` | select_pending_entity_template_list | 44 | decompilation/ghidra_conversion/src/entities/entity_template_conversion.cpp#ghidra_conversion::select_pending_entity_template_list | port |
| `0x5BAC4` | Scene-55 credits callback | — | decompilation/gameplay/scripts/dialogue_graph.lua#DialogueGraph | port |
| `0x5C8C6` | upload_title_or_field_word_stream | 52 | decompilation/ghidra_conversion/src/core/word_transfer_conversion.cpp#ghidra_conversion::upload_title_or_field_word_stream | port |
| `0x5D130` | Frosty command 0x49 mode-entry sequence | — | decompilation/gameplay/scripts/dialogue_actions.lua#DialogueActions | port |
| `0x5D172` | prepare_pre_title_audio_variant_a | 16 | decompilation/ghidra_conversion/src/audio/audio_service_batch_conversion.cpp;decompilation/ghidra_conversion/src/title_intro/boot_sequence_conversion.cpp | port |
| `0x5D1B4` | prepare_pre_title_audio_variant_b | 46 | decompilation/ghidra_conversion/src/audio/audio_service_batch_conversion.cpp;decompilation/ghidra_conversion/src/title_intro/boot_sequence_conversion.cpp | port |
| `0x5D260` | reset_world_frame_service_state | 46 | decompilation/ghidra_conversion/src/title_intro/title_intro_transfer_conversion.cpp#ghidra_conversion::reset_world_frame_service_state | port |
| `0x5D29C` | advance_world_scene_entity_effect | 344 | decompilation/ghidra_conversion/src/title_intro/title_intro_transfer_conversion.cpp#ghidra_conversion::advance_world_scene_entity_effect | port |
| `0x5D3E6` | spawn_world_scene_secondary_effect | 34 | decompilation/ghidra_conversion/src/title_intro/title_intro_transfer_conversion.cpp#ghidra_conversion::spawn_world_scene_secondary_effect | port |
| `0x5D416` | advance_world_scene_effect_phase | 294 | decompilation/ghidra_conversion/src/title_intro/title_intro_transfer_conversion.cpp#ghidra_conversion::advance_world_scene_effect_phase | port |
| `0x5D55C` | advance_world_scene_effect_frame | 34 | decompilation/ghidra_conversion/src/title_intro/title_intro_transfer_conversion.cpp#ghidra_conversion::advance_world_scene_effect_frame | port |
| `0x5D57E` | refresh_world_scene_effect_sprites | 36 | decompilation/ghidra_conversion/src/title_intro/title_intro_transfer_conversion.cpp#ghidra_conversion::refresh_world_scene_effect_sprites | port |
| `0x5D5A2` | advance_world_scene_effect_timer_a | 90 | decompilation/ghidra_conversion/src/title_intro/title_intro_transfer_conversion.cpp#ghidra_conversion::advance_world_scene_effect_timer_a | port |
| `0x5D5FC` | advance_world_scene_effect_timer_b | 94 | decompilation/ghidra_conversion/src/title_intro/title_intro_transfer_conversion.cpp#ghidra_conversion::advance_world_scene_effect_timer_b | port |
| `0x5D65A` | advance_world_scene_effect_timer_c | 98 | decompilation/ghidra_conversion/src/title_intro/title_intro_transfer_conversion.cpp#ghidra_conversion::advance_world_scene_effect_timer_c | port |
| `0x5D6BC` | spawn_scene_special_effect | 138 | decompilation/ghidra_conversion/src/title_intro/title_intro_transfer_conversion.cpp#ghidra_conversion::spawn_scene_special_effect | port |
| `0x5D746` | run_pre_title_sequence | 320 | decompilation/ghidra_conversion/src/title_intro/boot_sequence_conversion.cpp#ghidra_conversion::run_pre_title_sequence | port |
| `0x5D886` | finalize_pre_title_display | 52 | decompilation/ghidra_conversion/src/title_intro/boot_sequence_conversion.cpp#ghidra_conversion::finalize_pre_title_display | port |
| `0x5D8BA` | step_pre_title_logo_strobe | 126 | decompilation/ghidra_conversion/src/title_intro/boot_sequence_conversion.cpp#ghidra_conversion::advance_pre_title_logo_strobe | port |
| `0x5D938` | advance_pre_title_animation | 94 | decompilation/ghidra_conversion/src/title_intro/boot_sequence_conversion.cpp#ghidra_conversion::advance_pre_title_animation | port |
| `0x5D996` | advance_pre_title_animation_phase | 138 | decompilation/ghidra_conversion/src/title_intro/boot_sequence_conversion.cpp#ghidra_conversion::advance_pre_title_animation | port |
| `0x5DA50` | setup_pre_title_resources | 116 | decompilation/ghidra_conversion/src/title_intro/boot_sequence_conversion.cpp#ghidra_conversion::setup_pre_title_resources | port |
| `0x5DAC4` | fill_boot_horizontal_scroll | 36 | decompilation/ghidra_conversion/src/title_intro/boot_sequence_conversion.cpp#ghidra_conversion::fill_pre_title_horizontal_scroll | port |
| `0x5DAE8` | clear_pre_title_resource_workspace | 20 | decompilation/ghidra_conversion/src/integration/startup_integration_conversion.cpp | replaced |
| `0x5DAFC` | write_pre_title_resource_stream | 20 | decompilation/ghidra_conversion/src/integration/startup_integration_conversion.cpp | replaced |
| `0x1096B6` | Vid-Phone contact record 0 and call graph | — | decompilation/gameplay/scripts/vid_phone/capt_quinton_jaymes.lua | port |
| `0x10A7F2` | Sharkey Vid-Phone contact record and services | — | decompilation/gameplay/scripts/vid_phone/sharkey.lua;decompilation/gameplay/scripts/data/vid_phone_passages.lua | port |
| `0x10AD8A` | Kipp David Vid-Phone contact record and services | — | decompilation/gameplay/scripts/vid_phone/kipp_david.lua | port |
| `0x10B71E` | Wilma Temmenhoff Vid-Phone contact record and services | — | decompilation/gameplay/scripts/vid_phone/wilma_temmenhoff.lua | port |
| `0x10BFAC` | Sgt. Macklemann Vid-Phone contact record and services | — | decompilation/gameplay/scripts/vid_phone/macklemann.lua | port |
| `0x10C7FA` | Gregory Wilns Vid-Phone contact record and services | — | decompilation/gameplay/scripts/vid_phone/gregory_wilns.lua | port |
| `0x10DE0C` | Henry J. Culver Vid-Phone call and services | — | decompilation/gameplay/scripts/vid_phone/henry_j_culver.lua | port |
| `0x10E400` | Max Vid-Phone call and hardware services | — | decompilation/gameplay/scripts/vid_phone/max.lua | port |
| `0x10E9DE` | Sgt. Chillicut Vid-Phone call and services | — | decompilation/gameplay/scripts/vid_phone/chillicut.lua | port |
| `0x10F0D4` | Professor Jefferson Vid-Phone lesson and services | — | decompilation/gameplay/scripts/vid_phone/professor_jefferson.lua | port |
| `0x121FA0` | field dialogue profile table | — | decompilation/ghidra_conversion/tools/rom_research/graphics_extract.cpp;decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp | replaced |
| `0x122420` | Stoker field dialogue portrait stream | — | decompilation/ghidra_conversion/tools/rom_research/graphics_extract.cpp;decompilation/ghidra_conversion/qt_runtime/src/field/field_scene_runtime.cpp | replaced |
| `0x172CDC` | mode-2 field event table | — | decompilation/ghidra_conversion/src/integration/field_session_source_assets_conversion.cpp;decompilation/gameplay/scripts/field_events.lua | port |

## Unmapped routines

| ROM entry | Routine | Size (bytes) | Subsystem | Status |
|---|---|---:|---|---|
| `0x0436` | write_startup_text_to_vdp | 54 | startup text transfer | partial |
| `0x0C2C` | clear_runtime_memory_and_vdp | 192 | runtime memory/display initialization | partial |
| `0x0CEC` | install_startup_vdp_table | 40 | startup VDP table | partial |
| `0x0EC0` | upload_startup_palette | 38 | startup palette upload | partial |
| `0x0EE6` | service_pending_vdp_upload | 54 | pending VDP upload dispatch | partial |
| `0x0F1C` | write_transfer_header | 32 | transfer header write | partial |
| `0x0F3C` | write_fixed_transfer_pair | 32 | fixed transfer pair | partial |
| `0x0FFE` | clear_and_fill_plane_a | 44 | plane A clear/fill | partial |
| `0x102A` | clear_and_fill_plane_b | 44 | plane B clear/fill | partial |
| `0x1358` | copy_source_word_block | 16 | source word block copy | partial |
| `0x1406` | prepare_title_resources | 200 | title resource preparation | partial |
| `0x14CE` | extract_even_bytes | 26 | even-byte extraction | partial |
| `0x1502` | copy_even_bytes | 46 | even-byte copy | partial |
| `0x1530` | decode_and_upload_source_stream | 224 | compressed stream upload | partial |
| `0x1610` | decode_source_byte_stream | 108 | compressed byte decode | partial |
| `0x167C` | decode_source_coordinate_stream | 164 | coordinate stream decode | partial |
| `0x1728` | render_hud_sprite_stream | 322 | HUD sprite stream | partial |
| `0x1910` | write_alternating_word_pair | 76 | alternating word-pair transfer | partial |
| `0x19F2` | read_controller_port | 86 | controller port read | partial |
| `0x1CFC` | load_title_resources | 92 | title resource load sequence | partial |
| `0x1D58` | prepare_title_sprite_list | 34 | title sprite-list preparation | partial |
| `0x2A94` | initialize_mode_nine | 6 | mode 9 entry | partial |
| `0x3560` | refresh_hud_panel_variant | 162 | HUD panel variant refresh | partial |
| `0x3674` | advance_entity_delay_meter | 136 | entity delay meter | partial |
| `0x36FC` | advance_entity_asset_meter | 240 | entity asset meter advance | partial |
| `0x37EC` | advance_entity_behavior_meter | 192 | entity behavior meter | partial |
| `0x3A1C` | update_field_camera | 116 | field camera update | partial |
| `0x3A90` | accept_camera_delta | 142 | camera delta policy | partial |
| `0x3E86` | expand_packed_nibbles | 86 | packed nibble expansion | partial |
| `0x3EFE` | decode_field_tile_sample | 160 | field tile sample decode | partial |
| `0x4018` | render_field_tile_edge | 202 | field tile edge render | partial |
| `0x49A0` | noop_effect_callback | 2 | noop_effect_callback | partial |
| `0x4E38` | advance_entity_primary_timer | 44 | entity primary timer | partial |
| `0x4E64` | advance_entity_secondary_timer | 34 | entity secondary timer | partial |
| `0x5B1E` | sort_entity_slots_by_depth | 96 | entity depth sort | partial |
| `0x5B7E` | mark_entity_slots_for_render | 26 | entity render dirty pass | partial |
| `0x6148` | flush_primary_queued_transfers | 88 | primary queued transfer flush | partial |
| `0x61A0` | append_entity_sprite_record | 44 | entity sprite record append | partial |
| `0x61CC` | append_sprite_record | 40 | sprite record append | partial |
| `0x61F4` | flush_secondary_queued_transfers | 56 | secondary queued transfer flush | partial |
| `0x622C` | expand_entity_sprite_descriptors | 32 | entity sprite descriptor expansion | partial |
| `0x624C` | expand_entity_sprite_descriptor_list | 96 | entity sprite descriptor list expansion | partial |
| `0xACAE` | advance_entity_behavior_action | 426 | advance_entity_behavior_action | partial |
| `0xAE6E` | update_entity_target_heading | 76 | entity target heading update | partial |
| `0xB3FE` | set_entity_heading_with_alternate_animation | 52 | alternate heading and animation selection | partial |
| `0xB6E4` | select_entity_animation_table | 78 | entity animation table selection | partial |
| `0xC0C2` | load_title_font | 24 | title font load | partial |
| `0xC134` | normalize_source_text_line | 44 | source text line normalization | partial |
| `0xC160` | render_wrapped_text | 8 | wrapped text buffer wrapper | partial |
| `0xC168` | render_wrapped_encoded_text | 174 | render_wrapped_encoded_text | partial |
| `0xC436` | dispatch_menu_handler | 18 | menu handler dispatch | partial |
| `0xC928` | run_hospital_options_menu | 238 | run_hospital_options_menu | partial |
| `0xCDF6` | calculate_hospital_medical_attention_cost | — | calculate_hospital_medical_attention_cost | partial |
| `0xD4C8` | advance_field_active_runner_selection | 116 | advance_field_active_runner_selection | partial |
| `0xD53C` | run_attribute_skill_upgrade_prompt | 160 | Attributes/Skills upgrade confirmation | partial |
| `0xE6F4` | service_mode_world_state | 212 | service mode world state | partial |
| `0xF5F4` | render_bitmask_text_batch | 232 | render_bitmask_text_batch | partial |
| `0xFDAE` | write_menu_grid_quadruple | 80 | menu grid quadruple | partial |
| `0xFDFE` | advance_menu_sprite_overlay | 192 | advance_menu_sprite_overlay | partial |
| `0x10918` | prepare_menu_graphics | 58 | menu graphics preparation | partial |
| `0x10952` | reset_mode_display_buffers | 50 | mode display buffer reset | partial |
| `0x11352` | apply_armor_selection | 240 | armor selection | partial |
| `0x12148` | read_entity_action_code | 32 | entity action-code lookup | partial |
| `0x12E9E` | service_entity_action_menu | 16 | entity action menu service | partial |
| `0x12F50` | render_entity_action_menu | 84 | entity action menu renderer | partial |
| `0x12FA4` | advance_entity_action_menu | 92 | entity action menu advance | partial |
| `0x133A6` | apply_world_effect_reward | 192 | apply world-effect reward command | partial |
| `0x13466` | set_entity_action_state | 108 | entity action state setter | partial |
| `0x13726` | render_mode_selection | 334 | render mode selection / cab destination cursor | defended |
| `0x13874` | dispatch_entity_action_candidates | 68 | entity action candidate dispatcher | partial |
| `0x138B8` | run_entity_animation_entry | 76 | entity animation timer entry | partial |
| `0x13A40` | copy_entity_anchor_position | 12 | entity anchor position copy | partial |
| `0x13A92` | run_entity_animation_branch | 84 | entity animation branch | partial |
| `0x13AE6` | advance_entity_action_threshold | 12 | entity action threshold advance | partial |
| `0x13B12` | run_entity_animation_transition | 168 | entity animation event transition | partial |
| `0x13BBA` | check_entity_animation_transition | 102 | entity animation transition gate | partial |
| `0x13C20` | select_entity_action_lane | 84 | entity action lane selector | partial |
| `0x13C74` | collect_entity_action_records | 96 | entity action record collector | partial |
| `0x13CD4` | evaluate_entity_animation_rate_from_phase | 10 | entity animation rate gate from phase | partial |
| `0x13CDE` | evaluate_entity_animation_rate | 46 | entity animation rate gate | partial |
| `0x13E8C` | service_entity_boundary_transition | 18 | entity boundary transition coordinator | partial |
| `0x13E9E` | reposition_entity_at_boundary | 86 | entity boundary reposition | partial |
| `0x13F76` | run_entity_animation_resource_gate | 136 | entity animation resource gate | partial |
| `0x1438C` | collect_entity_action_primary_candidates | 106 | primary entity action candidate scan | partial |
| `0x14446` | collect_entity_action_candidates | 76 | entity action candidate scan | partial |
| `0x14492` | select_entity_animation_link | 166 | entity animation linked-target selector | partial |
| `0x144E8` | apply_entity_close_range_state | 18 | entity close-range state wrapper | partial |
| `0x14508` | prepare_entity_animation_fallback_target | 154 | entity animation fallback target constructor | partial |
| `0x145A2` | advance_entity_animation_target_route | 108 | entity animation target-route coordinator | partial |
| `0x1460E` | apply_close_range_entity_state | 50 | entity close-range state | partial |
| `0x14682` | select_obstruction_route | 154 | obstruction route selection | partial |
| `0x14756` | clear_entity_route_flags | 14 | entity route flag cleanup | partial |
| `0x14764` | find_alternate_route_tile | 156 | alternate route tile | partial |
| `0x148AA` | find_first_occupied_obstruction_tile | 32 | last open obstruction tile | partial |
| `0x14902` | compute_obstruction_grid_index | 18 | obstruction grid index | partial |
| `0x149C0` | advance_obstruction_route_queue | 308 | obstruction route queue | partial |
| `0x14BA6` | update_entity_axis_relation_state | 136 | axis relation update | partial |
| `0x14C2E` | derive_entity_axis_relation_flags | 40 | entity axis relation flags | partial |
| `0x15118` | initialize_entity_behavior_record | 54 | entity behavior record initialization | partial |
| `0x15354` | activate_entity_audio_state | 60 | entity audio state activation | partial |
| `0x15554` | service_nearby_entity_auxiliary_behavior | 218 | nearby entity auxiliary behavior scan | partial |
| `0x15646` | spawn_entity_effect_record | 484 | short-lived effect/projectile entity constructor | partial |
| `0x15930` | apply_entity_secondary_effect | 78 | secondary type-2 entity effect | partial |
| `0x15CA4` | compute_diagonal_step_cost | 28 | movement distance metric | partial |
| `0x15CC0` | compute_diagonal_step_cost | 36 | point-pair distance metric | partial |
| `0x15D3C` | measure_relative_heading_and_distance | 18 | relative heading and distance | partial |
| `0x163DA` | service_active_effect_slots | 64 | active effect slot service | partial |
| `0x164A8` | populate_entity_table | 52 | entity descriptor table population | partial |
| `0x164DC` | clear_scene_record_transition_words | 22 | scene record transition clear | partial |
| `0x16A26` | publish_world_activity_descriptors | 68 | world activity descriptor publish | partial |
| `0x16DEE` | clear_scene_record_pending_flags | 26 | scene record pending clear | partial |
| `0x16E08` | all_scene_record_slots_active | 40 | scene record activity predicate | partial |
| `0x16E30` | snapshot_scene_state | 44 | scene state snapshot | partial |
| `0x16ECA` | process_terrain_work_batch | 74 | terrain work batch | partial |
| `0x173F2` | prepare_menu_mode_resources | 108 | menu mode resource preparation | partial |
| `0x1745E` | run_field_mode_hud_scene | 604 | field/menu mode orchestrator | partial |
| `0x176BA` | refresh_field_resource_streams | 48 | field resource refresh | partial |
| `0x176EA` | render_menu_footer | 30 | menu footer render | partial |
| `0x177A4` | initialize_menu_slot_states | 40 | menu slot initialization | partial |
| `0x177CC` | upload_menu_resource_pair | 48 | menu resource pair upload | partial |
| `0x177FC` | run_menu_hud_tick_loop | 160 | menu HUD tick loop | partial |
| `0x1789C` | prepare_menu_hud_sequence | 44 | menu HUD preparation sequence | partial |
| `0x178C8` | write_menu_hud_grid_batch | 102 | menu HUD grid batch | partial |
| `0x1792E` | render_menu_hud_rows | 106 | menu HUD row renderer | partial |
| `0x17998` | render_menu_text_progression | 198 | menu text progression | partial |
| `0x17A5E` | render_menu_slot_status | 354 | menu entry renderer | partial |
| `0x17BC0` | refresh_menu_number_or_grid | 124 | menu number or grid refresh | partial |
| `0x17C3C` | read_menu_entry_profile_eligibility | 22 | menu/profile eligibility table lookup | partial |
| `0x17C52` | render_menu_text_followup | 50 | menu text followup | partial |
| `0x17C84` | set_hud_selection_value | 10 | HUD selection value setter | partial |
| `0x17C8E` | hud_selection_value_matches | 10 | HUD selection value matcher | partial |
| `0x17C98` | find_hud_selection_index | 32 | HUD selection lookup | partial |
| `0x17CC8` | advance_hud_sprite_audio_pulse | 118 | HUD sprite/audio pulse | partial |
| `0x17D3E` | advance_audio_service_tick | 36 | audio service phase tick | partial |
| `0x17D90` | advance_hud_panel | 40 | HUD panel offset advance | partial |
| `0x17DB8` | advance_hud_footer_offset | 76 | HUD footer offset advance | partial |
| `0x17E04` | refresh_hud_grid_if_ready | 116 | HUD grid readiness refresh | partial |
| `0x17E78` | calculate_menu_row_offset | 60 | menu row offset calculation | partial |
| `0x17EB4` | calculate_menu_delay_offset | 48 | menu delay offset calculation | partial |
| `0x17EE4` | reset_menu_overlay_and_upload_two | 10 | menu overlay reset | partial |
| `0x17EEE` | advance_menu_overlay_timer | 146 | menu overlay timer | partial |
| `0x17F80` | refresh_mode_metadata_wrapper | 14 | mode metadata refresh wrapper | partial |
| `0x17FB4` | write_hud_repeated_grid | 38 | HUD repeated-grid wrapper | partial |
| `0x17FDA` | advance_menu_random_glitch | 374 | menu random visual effect | partial |
| `0x1817E` | matrix run entry shell | — | matrix run entry shell | partial |
| `0x181F8` | run_matrix_session_entry | 52 | Matrix session entry | partial |
| `0x1822C` | run_matrix_interaction_loop | 1230 | Matrix interaction loop | partial |
| `0x18408` | Matrix action branch | — | Matrix action branch | partial |
| `0x186FA` | refresh_field_hud_assets | 124 | field HUD asset refresh | partial |
| `0x18776` | advance_hud_fade_service | 52 | HUD fade service | partial |
| `0x187AA` | render_hud_indicator_grids | 132 | HUD indicator grid renderer | partial |
| `0x1882E` | render_matrix_panel | 334 | Matrix panel renderer | partial |
| `0x1897C` | write_hud_repeated_grid_variant_a | 28 | HUD repeated-grid variant A | partial |
| `0x18998` | write_hud_repeated_grid_variant_b | 28 | HUD repeated-grid variant B | partial |
| `0x189B4` | write_hud_repeated_grid_variant_c | 36 | HUD repeated-grid variant C | partial |
| `0x189D8` | write_two_hud_grids | 136 | HUD two-grid writer | partial |
| `0x18A92` | matrix_program_index | 20 | Matrix program selection index | partial |
| `0x18AA6` | run_cyber_info_screen | 628 | CyberInfo screen | partial |
| `0x19018` | Matrix utility menu | — | Matrix utility menu | partial |
| `0x19180` | Matrix retreat node transition | — | Matrix retreat node transition | partial |
| `0x19238` | run_matrix_program_slot_screen | 506 | Matrix program slot screen | partial |
| `0x19432` | handle_menu_directional_input | 122 | menu directional input handler | partial |
| `0x194AC` | advance_hud_selection_row | 88 | HUD selection row advance | partial |
| `0x19504` | write_hud_panel_grid | 40 | HUD panel grid writer | partial |
| `0x195A6` | calculate_selected_matrix_program_load_steps | 4 | Selected Matrix program load timing | partial |
| `0x195AA` | calculate_matrix_program_load_steps | 46 | Matrix program load timing arithmetic | partial |
| `0x195D8` | refresh_matrix_program_slot | 82 | Matrix program slot refresh | partial |
| `0x1962A` | apply_menu_slot_command | 8 | menu slot command | partial |
| `0x19632` | clear_matrix_program_slot | 192 | Matrix loaded-program slot cleanup | partial |
| `0x19736` | clear_ready_menu_slot_timer | 44 | ready menu slot timer | partial |
| `0x19762` | clear_menu_status_at | 38 | menu status clear | partial |
| `0x19788` | advance_matrix_program_loading | 206 | Matrix loading progression service | partial |
| `0x19856` | refresh_field_hud | 68 | field HUD refresh | partial |
| `0x1989A` | update_hud_mode_transition | 220 | HUD mode transition | partial |
| `0x19976` | service_matrix_program_action | 376 | Matrix program-action service | partial |
| `0x19AEE` | refresh_mode_glyph | 70 | mode glyph refresh | partial |
| `0x19B34` | publish_menu_selection_then_text | 12 | menu selection then text | partial |
| `0x19B40` | select_run_hud_mode | 110 | Matrix run HUD mode selection | partial |
| `0x19BF6` | select_hud_mode_auxiliary_value | 94 | HUD mode auxiliary value | partial |
| `0x19C54` | read_selected_matrix_program_availability | 8 | Selected Matrix program availability | partial |
| `0x19C5C` | read_matrix_program_availability_for_slot | 16 | Matrix program availability by slot | partial |
| `0x19CB2` | run_menu_selection_code_four | 116 | Menu selection and code-four service | partial |
| `0x19D66` | run_menu_slot_service | 70 | menu slot service | partial |
| `0x19DAC` | merge_menu_pending_flags | 44 | menu pending flag merge | partial |
| `0x19DD8` | refresh_mode_metadata_leaf | 16 | mode metadata leaf | partial |
| `0x19DE8` | run_menu_service_then_prepare | 10 | menu service then prepare | partial |
| `0x19DF2` | run_menu_metadata_service | 40 | menu metadata service | partial |
| `0x19E32` | run_menu_selection_service | 82 | menu selection service | partial |
| `0x19EA6` | run_menu_selection_variant | 90 | menu selection variant | partial |
| `0x19F18` | run_menu_progression_variant | 84 | menu progression variant | partial |
| `0x19F72` | finalize_menu_slot_code_five | 6 | menu slot code five | partial |
| `0x19F78` | finalize_menu_slot_code_six | 6 | menu slot code six | partial |
| `0x19F7E` | clear_first_ready_menu_slot | 82 | first ready menu slot clear | partial |
| `0x1A01E` | publish_menu_selection_then_text_variant | 18 | menu selection/text variant | partial |
| `0x1A030` | resolve_title_world_action_selection | 108 | title-world runner action selection | partial |
| `0x1A09C` | refresh_menu_selection_after_clear | 40 | menu selection refresh after clear | partial |
| `0x1A106` | refresh_menu_selection_or_prepare | 40 | menu selection refresh or prepare | partial |
| `0x1A12E` | advance_menu_countdown | 38 | menu countdown advance | partial |
| `0x1A154` | clear_selected_menu_record | 22 | selected menu record clear | partial |
| `0x1A16A` | render_matrix_runner_detail_panel | 354 | Matrix runner detail panel | partial |
| `0x1A370` | advance_world_effect_controller | 350 | World-effect frame coordinator | partial |
| `0x1A468` | initialize_audio_frame | 34 | audio frame initialization | partial |
| `0x1A48A` | advance_title_animation_cycle | 96 | title animation cycle | partial |
| `0x1A54E` | initialize_effect_defaults | 34 | effect defaults | partial |
| `0x1A570` | build_title_sprites_twice | 48 | title sprite double build | partial |
| `0x1A5A0` | advance_world_effect_mode_animation | 568 | World-effect mode animation | partial |
| `0x1A7BE` | append_title_vertical_sprite | 90 | vertical title sprite append | partial |
| `0x1A818` | begin_world_effect_descriptor_transition | 118 | world-effect descriptor transition | partial |
| `0x1A8F4` | reset_effect_entity | 20 | effect entity reset | partial |
| `0x1A908` | initialize_world_effect_mode | 198 | world-effect mode initialization | partial |
| `0x1AA40` | tick_world_effect_animation | 302 | world-effect animation tick | partial |
| `0x1AB6E` | advance_primary_world_effect_overlay | 226 | primary world-effect overlay advance | partial |
| `0x1AC50` | arm_effect_after_threshold | 26 | effect threshold arm | partial |
| `0x1AC6A` | advance_secondary_world_effect_overlay | 216 | secondary world-effect overlay advance | partial |
| `0x1AD42` | advance_background_world_effect_overlay | 164 | background world-effect overlay advance | partial |
| `0x1ADE6` | run_world_effect_scene_loop | 990 | world-effect scene driver | partial |
| `0x1B1C4` | initialize_matrix_program_transition | 56 | Matrix program transition initialization | partial |
| `0x1B1FC` | reset_title_transition | 28 | title/world transition reset | partial |
| `0x1B218` | advance_title_phase_cycle | 94 | title/world phase cycle | partial |
| `0x1B282` | advance_matrix_program_transition | 128 | Matrix program transition advance | partial |
| `0x1B302` | advance_title_jitter | 98 | title jitter stream advance | partial |
| `0x1B364` | advance_title_figure_animation | 248 | title figure animation advance | partial |
| `0x1B45C` | advance_title_noise_stream | 310 | title noise stream advance | partial |
| `0x1B5A6` | reset_word_stream_area | 18 | word stream area reset | partial |
| `0x1B5B8` | attach_behavior_descriptor_a | 38 | behavior descriptor A | partial |
| `0x1B612` | attach_behavior_descriptor_b | 22 | behavior descriptor B | partial |
| `0x1B628` | attach_behavior_descriptor_c | 22 | behavior descriptor C | partial |
| `0x1B654` | attach_behavior_descriptor_d | 22 | behavior descriptor D | partial |
| `0x1B680` | attach_behavior_descriptor_e | 22 | behavior descriptor E | partial |
| `0x1B706` | prepare_menu_offset_once | 46 | menu offset preparation | partial |
| `0x1B7D0` | initialize_behavior_record | 28 | behavior record initialization | partial |
| `0x1B87A` | dispatch_title_world_phase_service | 66 | title/world phase service dispatch | partial |
| `0x1BA6E` | advance_title_burst_animation | 102 | Title burst animation | partial |
| `0x1BB6A` | advance_title_noise_effect | 264 | Title noise effect | partial |
| `0x1BCE8` | advance_title_profile_motion | 342 | Title profile motion | partial |
| `0x1BF5C` | copy_title_profile | 30 | title profile copy | partial |
| `0x1BF7A` | advance_profile_copy_timer | 130 | Profile copy timer | partial |
| `0x1C0C2` | advance_alternating_profile | 72 | alternating title profile | partial |
| `0x1C1F6` | advance_audio_menu_countdown | 52 | audio menu countdown | partial |
| `0x1C3FA` | advance_title_sprite_layers | 152 | Title sprite layer animation | partial |
| `0x1C506` | render_title_sprite_layers | 230 | Title sprite layer renderer | partial |
| `0x1C61C` | advance_title_sweep_animation | 160 | Title sweep animation | partial |
| `0x1C754` | advance_title_world_interaction_service | 274 | title/world interaction service | partial |
| `0x1C866` | advance_title_effect_motion | 192 | Title effect motion | partial |
| `0x1C9A2` | initialize_effect_motion_seed | 94 | effect motion seed | partial |
| `0x1CA00` | initialize_world_effect_motion | 18 | world effect motion initialization | partial |
| `0x1CA12` | advance_world_effect_motion | 122 | World effect motion tail | partial |
| `0x1CAD4` | update_interaction_state | 76 | interaction state update | partial |
| `0x1CB20` | advance_world_effect_activation | 144 | World effect activation | partial |
| `0x1CB6A` | spawn_world_effect_record | 142 | World effect record spawn | partial |
| `0x1CC24` | advance_persistent_world_effect | 184 | Persistent world-effect sprite ring | partial |
| `0x1CCDC` | update_persistent_title_effect | 56 | persistent title effect update | partial |
| `0x1CD5A` | advance_title_secondary_effect_service | 416 | secondary title/world effect service | partial |
| `0x1CEFA` | advance_title_effect_position | 60 | title effect position advance | partial |
| `0x1CF36` | build_title_sprites | 20 | title sprite build wrapper | partial |
| `0x1CF4A` | advance_title_effect_sprites | 254 | Title effect sprite advance | partial |
| `0x1D048` | advance_world_effect_record | 150 | World effect record advance | partial |
| `0x1D0DE` | resolve_world_effect_sprite | 62 | World effect sprite resolver | partial |
| `0x1D1F4` | advance_title_reveal_transfer_service | 140 | title/world reveal transfer service | partial |
| `0x1D280` | initialize_world_sequence | 102 | World sequence initializer | partial |
| `0x1D2E6` | advance_world_sequence | 84 | world sequence advance | partial |
| `0x1D33A` | render_world_sequence_tile_rows | 186 | World sequence tile-row renderer | partial |
| `0x1DBF0` | reset_entity_service_records | 50 | entity service record reset | partial |
| `0x1DF34` | resolve_nearby_world_effects | 144 | Nearby world-effect resolver | partial |
| `0x1E166` | trigger_world_effect_sequence | 150 | World effect trigger | partial |
| `0x1E1FC` | forward_effect_sequence | 14 | effect sequence forwarder | partial |
| `0x1E216` | spawn_world_effect_sequence | 68 | world effect sequence spawn | partial |
| `0x1E69C` | initialize_world_effect_selection | 144 | World effect initializer | partial |
| `0x1F292` | prepare_mode_exit_resources | 156 | prepare mode exit resources | partial |
| `0x1F32E` | prepare_optional_mode_resource | 28 | clear optional mode resource | partial |
| `0x1F34A` | prepare_mode_resource_variant_b | 64 | prepare mode resource variant B | partial |
| `0x1F38A` | prepare_mode_resource_variant_a | 58 | prepare mode resource variant A | partial |
| `0x1F3C4` | run_mode_text_prompt | 484 | run mode text prompt | partial |
| `0x1F5A8` | prepare_mode_text_prompt | 34 | prepare mode text prompt | partial |
| `0x1F830` | prepare_field_mode_exit | 584 | Field mode exit policy | partial |
| `0x2196C` | mark_effect_bit | 22 | effect bit marker | partial |
| `0x266E4` | initialize_world_tile_buffers | 186 | World tile-buffer initializer | partial |
| `0x2679E` | run_world_entry_sequence | 326 | World entry sequence loop | partial |
| `0x268E6` | append_world_sprite_records | 82 | world sprite record append | partial |
| `0x26972` | refresh_title_sprite_records | 142 | Title sprite refresh gate | partial |
| `0x26A00` | run_display_transition | 510 | Display transition choreography | partial |
| `0x530DE` | service_audio_if_enabled | 12 | Z80/audio | partial |
| `0x53200` | select_primary_runner_record_table | 22 | Primary runner-record table selector | partial |
| `0x53216` | select_secondary_runner_record_table | 22 | Secondary runner-record table selector | partial |
| `0x532A8` | fill_mode_auxiliary_sentinel | 20 | Matrix node auxiliary sentinel fill | partial |
| `0x532BC` | initialize_runner_selection_screen | 306 | Runner-selection screen initializer | partial |
| `0x533EE` | initialize_runner_animation_grid | 70 | Runner animation-grid initializer | partial |
| `0x53434` | expand_runner_animation_grid | 720 | Runner animation-grid guarded expansion | partial |
| `0x53704` | run_runner_selection_screen | 246 | Runner-selection controller loop | partial |
| `0x537FA` | refresh_runner_selection_sprites | 106 | Selected-runner sprite refresh | partial |
| `0x54662` | run_mode_action_loop | 904 | Matrix node-function action loop | partial |
| `0x549E4` | write_mode_repeated_grid_service | 18 | Matrix node action grid redraw | partial |
| `0x549F6` | select_mode_display_status | 50 | Matrix alert display selection | partial |
| `0x54A28` | render_mode_status_summary | 102 | Matrix node status summary renderer | partial |
| `0x55900` | resolve_entity_action_status | 72 | resolve_entity_action_status | partial |
| `0x55948` | resolve_equipment_action_status | 116 | resolve_equipment_action_status | partial |
| `0x559BC` | resolve_dynamic_entity_action | 346 | resolve_dynamic_entity_action | partial |
| `0x55B16` | resolve_entity_action_with_default_bonus | 98 | resolve_entity_action_with_default_bonus | partial |
| `0x55E92` | subtract_armor_primary_modifier | 34 | subtract armor primary modifier | partial |
| `0x55EB4` | subtract_armor_secondary_modifier | 34 | subtract armor secondary modifier | partial |
| `0x55F44` | calculate_runner_action_delay | 148 | Runner action delay calculation | partial |
| `0x55FD8` | calculate_and_publish_runner_action_result | 38 | calculate_and_publish_runner_action_result | partial |
| `0x56050` | resolve_title_world_action_marker | 88 | title-world action marker continuation | partial |
| `0x56116` | advance_world_effect_intro_gate | 210 | world-effect intro gate | partial |
| `0x561E8` | select_world_effect_animation_mode | 344 | world-effect animation-mode selector | partial |
| `0x562D4` | add_world_effect_timer_modifier | 10 | world-effect timer modifier | partial |
| `0x562DE` | add_world_effect_phase_modifier | 14 | world-effect phase modifier | partial |
| `0x562EC` | resolve_world_effect_transition | 224 | world-effect transition resolver | partial |
| `0x563CC` | advance_primary_world_effect_pending_roll | 104 | primary world-effect pending roll | partial |
| `0x56434` | advance_menu_selection_service_timer | 78 | shared selected-record +0x0F decrement | partial |
| `0x56482` | advance_menu_selection_variant_timer | 84 | shared selected-record +0x10 decrement | partial |
| `0x56506` | calculate_menu_hud_row_meter | 300 | menu HUD row meter | partial |
| `0x56632` | scale_menu_hud_value_by_difficulty | 38 | menu HUD difficulty scaling | partial |
| `0x5668C` | advance_menu_selection_service_and_refresh | 22 | advance_menu_selection_service_and_refresh | partial |
| `0x566D8` | copy_mode_resource_metadata | 10 | copy mode resource palette bank 0 | partial |
| `0x566E2` | refresh_mode_resource_metadata | 10 | copy mode resource palette bank 1 | partial |
| `0x566EC` | copy_field_mode_palette_bank | 10 | copy mode resource palette bank 2 | partial |
| `0x566F6` | refresh_mode_resource_state | 24 | copy mode resource palette bank 3 | partial |
| `0x5670E` | refresh_mode_palette_data | 30 | copy mode palette block | partial |
| `0x5672C` | service_field_mode_display_frame | 916 | service_field_mode_display_frame | partial |
| `0x56DC4` | run_field_action_selection | 2432 | run_field_action_selection | partial |
| `0x575F8` | prepare_field_action_resource_selection | 24 | prepare_field_action_resource_selection | partial |
| `0x57610` | prepare_field_action_selection_context | 24 | prepare_field_action_selection_context | partial |
| `0x57628` | select_menu_resource_entry | 20 | Field/matrix resource action entry selector | partial |
| `0x5763C` | select_menu_resource | 46 | Field/matrix resource action record selector | partial |
| `0x5766A` | apply_field_resource_purchase | 336 | Field-service resource purchase | partial |
| `0x577BA` | apply_field_quantity_purchase | 194 | Field-service quantity purchase | partial |
| `0x57C3A` | write_field_action_grid_variant_a | 12 | write_field_action_grid_variant_a | partial |
| `0x57C46` | write_field_action_grid_variant_b | 12 | write_field_action_grid_variant_b | partial |
| `0x57C52` | write_field_action_grid_variant_c | 12 | write_field_action_grid_variant_c | partial |
| `0x57C5E` | refresh_field_action_detail_panel | 8 | Refresh field action detail panel | partial |
| `0x57C66` | write_field_action_detail_grid | 20 | write_field_action_detail_grid | partial |
| `0x57C7A` | render_field_action_selection | 1696 | render_field_action_selection | partial |
| `0x580FE` | render_field_action_description | 264 | render_field_action_description | partial |
| `0x5825C` | render_field_action_cost_summary | 186 | render_field_action_cost_summary | partial |
| `0x58316` | render_field_action_feedback | 126 | render_field_action_feedback | partial |
| `0x586CA` | render_field_action_detail | 196 | render_field_action_detail | partial |
| `0x587DA` | render_field_mode_selection_presentation | 596 | Field-action selection presentation dispatcher | partial |
| `0x58A2E` | render_alternating_field_mode_records | 118 | Field-action detail record glyph/grid loop | partial |
| `0x58AA4` | render_field_mode_glyph_row | 122 | Active-runner inventory item glyph row | partial |
| `0x58B1E` | render_field_mode_grid_pair_batches | 64 | Field-action detail grid pair batches | partial |
| `0x58D7E` | reset_field_action_cost_state | 22 | reset_field_action_cost_state | partial |
| `0x59360` | coordinate_save_load | 18 | save/load coordinator | partial |
| `0x59372` | load_save_work_buffer | 138 | save work-buffer loading | partial |
| `0x593FC` | validate_backup_game_save | 90 | backup game-save validation | partial |
| `0x59456` | normalize_sram_save_slots | 138 | SRAM save-slot normalization | partial |
| `0x5961C` | validate_sram_game_save | 82 | SRAM game-save validation | partial |
| `0x59786` | sum_game_save_payload | 18 | game-save payload checksum | partial |
| `0x598B0` | project_compact_runner_record | 194 | compact runner record projection | partial |
| `0x59986` | select_world_effect_sequence | 84 | world-effect sequence selection | partial |
| `0x59BCC` | reset_runner_status_triplet | 94 | reset three live runner status records | partial |
| `0x59C5E` | run_title_intro_stages | 180 | run title intro stages | partial |
| `0x5BA50` | load_scene_resources | 22 | scene resource loader | partial |
| `0x5BA66` | install_scene_resource_index_entries | 50 | scene resource index install | partial |
