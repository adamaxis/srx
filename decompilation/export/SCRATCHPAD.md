# Shadowrun Genesis Ghidra scratchpad

This directory is the single checked-in Ghidra decompilation export for the US
Genesis ROM. It is source-analysis evidence. Native code uses typed host state
and direct assets; it does not execute this decompilation or perform ROM/RAM
lookups at runtime.

## Regeneration record

- ROM: `../input/shadowrun_genesis_us.bin`
- ROM SHA-256: `92A614491FC3EA12A6DF32459CCE08B48574B7B814E244C07EB860DC5C6EDB7E`
- Ghidra: 11.4.2
- Loader: patched `lab313ru/ghidra_sega_ldr`, upstream commit `aef1046`
- Language: `68000:BE:32:default` / compiler `default`
- Exported functions: 906
- Resolved direct call sites: 2,978
- Unresolved or indirect call sites: 16

Semantic names in `functions.tsv`, `callgraph.tsv`, `call-sites.tsv`,
`disassembly.s`, and the generated C files come from the reviewed-name table in
`../tools/ghidra/ExportShadowrun.java`. Unreviewed functions intentionally
retain Ghidra's `FUN_<address>` name.

## Export files

- `functions.tsv`: one row per function and generated C file.
- `callgraph.tsv`: resolved caller/callee edges with call-site counts.
- `call-sites.tsv`: every direct call plus every unresolved/indirect call.
- `disassembly.s`: instructions with resolved call comments.
- `ghidra-c/fn_*.cpp`: individual decompiler output with annotated callees.

## Reviewed function map

The native owner names below are the target C++ vocabulary. A `partial` entry
means the source behavior is understood enough to structure the host state but
still has unresolved downstream calls or hardware presentation work.

| ROM entry | Ghidra name | Behavior recovered | Native C++ owner | Status |
| --- | --- | --- | --- | --- |
| `0x0236` | `reset_and_enter_boot` | Reset path: initializes VDP/Z80 state, clears display state, selects the region/title path, and transfers to the main coordinator. | `ShadowrunBoot` / `NativePortShell::update_boot` | partial |
| `0x0330` | reset display block | Emits the five initial VDP register commands, clears the first VRAM word, and establishes the text-write command. | `make_startup_display_plan()` | partial |
| `0x0436` | `write_startup_text_to_vdp` | Builds the destination command and converts a zero-terminated source string into host glyph indices. | `encode_startup_text()` / `ShadowrunBoot::write_startup_text()` adapter | partial |
| `0x073A` | `wait_for_vdp_ready` | Coalesces the source's 11 × 4096 startup delay, waits at a typed ready boundary, then applies the 256-iteration settle delay. | `ghidra_conversion::wait_for_vdp_ready()`; no host VDP polling loop | partial |
| `0x075E` | `run_game_loop` | Main coordinator: initializes session state, dispatches boot/title/field work, samples input, and performs indirect state dispatch. | `NativePortShell::update()` | partial |
| `0x0BD2` | `wait_for_vblank` | Advances the frame counter and requests the next interrupt-side sample through a host frame boundary. | `ghidra_conversion::wait_for_vblank()`; presentation timing remains a callback | partial |
| `0x0C24` | `wait_for_vblank_frames` | Repeats the vblank boundary `source_counter + 1` times, preserving the source DBF behavior. | `ghidra_conversion::wait_for_vblank_frames()` | partial |
| `0x0D94` | `wait_for_controller_event` | Waits, latches, and samples until the controller event word has a nonzero high byte, then samples once more. | `ghidra_conversion::wait_for_controller_event()`; input delivery remains callbacks | partial |
| `0x0BF2` | `latch_controller_events` | Combines the sampled high-byte event words and latches activity and Start state. | `ghidra_conversion::latch_controller_events()`; host input delivery remains separate | partial |
| `0x0D44` | `advance_source_prng` | Advances the source 32-bit linear-congruential state with multiplier `0x20E5` and increment `1`. | `ghidra_conversion::advance_source_prng()` | partial |
| `0x0D3A` | `sample_source_random_bound` | Advances the source PRNG and scales its low word by the caller bound using the source multiply/shift operation. | `ghidra_conversion::sample_source_random_bound()` | partial |
| `0x0DBA` | `count_thresholded_random_rolls` | Performs the source repeated six-sided roll, accumulates face-6 retries, compares against the threshold, and counts qualifying trials. | `ghidra_conversion::count_thresholded_random_rolls()` | partial |
| `0x0C2C` | `clear_runtime_memory_and_vdp` | Clears or folds work RAM based on the startup sentinel, resets display state, and invokes low-level setup. | Native session initialization; bus transcription remains in `ShadowrunRuntime` | partial |
| `0x1088` | `step_palette_toward_black` | Moves each of the 64 CRAM words one Genesis color component toward black. | `ghidra_conversion::palette_toward_black()` | partial |
| `0x10D6` | `step_palette_toward_target` | Moves each of the 64 CRAM words one component toward the target palette. | `ghidra_conversion::palette_toward_target()` | partial |
| `0x131C` | `step_palette_transition` | Runs seventeen palette/update steps, each synchronized to frame/input service. | `ghidra_conversion::run_palette_transition()` | partial |
| `0x1342` | `wait_for_palette_frame` | Performs source-counter-plus-one vblank/latch iterations, preserving the DBF semantics used by palette steps. | `ghidra_conversion::wait_for_palette_frame()`; vblank and input latching remain callbacks | partial |
| `0x1358` | `copy_source_word_block` | Copies 32 longwords into the source work block. | `ghidra_conversion::copy_source_word_block()` | partial |
| `0x1368` | `copy_target_palette` | Copies 32 longwords into the 64-word target palette. | `ghidra_conversion::copy_target_palette()` | partial |
| `0x14CE` | `extract_even_bytes` | Extracts bytes from every second byte of the source table. | `ghidra_conversion::extract_even_bytes()` | partial |
| `0x1502` | `copy_even_bytes` | Copies bytes between every-second-byte positions in the source table. | `ghidra_conversion::copy_even_bytes()` | partial |
| `0x1610` | `decode_source_byte_stream` | Decodes literal bytes, repeated runs, and sliding-window copies from the source stream header. | `ghidra_conversion::decode_source_byte_stream()`; destination presentation remains separate | partial |
| `0x167C` | `decode_source_coordinate_stream` | Expands signed coordinate runs from the source base word until the zero terminator. | `ghidra_conversion::decode_source_coordinate_stream()` | partial |
| `0x3E86` | `expand_packed_nibbles` | Maps each packed byte through the source 16-entry nibble table. | `ghidra_conversion::expand_packed_nibbles()` | partial |
| `0x41B6` | `fold_field_coordinate` | Folds the high-nibble field coordinate into the eight-value source band. | `ghidra_conversion::fold_field_coordinate()` | partial |
| `0x4216` | `offset_field_coordinate` | Applies the source eight-unit high-nibble offset. | `ghidra_conversion::offset_field_coordinate()` | partial |
| `0x423C` | `quantize_field_delta` | Converts a wrapped coordinate delta into 128-unit, doubled steps. | `ghidra_conversion::quantize_field_delta()` | partial |
| `0x4E38` | `advance_entity_primary_timer` | On frame phase zero, decrements the primary timer and clears the source `0x80` flag at expiry; source `0x40` bypasses it. | `ghidra_conversion::advance_entity_primary_timer()` | partial |
| `0x4E64` | `advance_entity_secondary_timer` | On frame phase zero, decrements the secondary timer and clears the source `0x20` flag at expiry. | `ghidra_conversion::advance_entity_secondary_timer()` | partial |
| `0x5A56` | `project_entity_screen_position` | Converts fixed-point entity coordinates to screen words and selects the normal field or wider full-screen visibility window. | `ghidra_conversion::project_entity_screen_position()` | partial |
| `0x5A62` | `project_entity_screen_position_standard` | Converts fixed-point entity coordinates to screen words and applies the normal field visibility window. | `ghidra_conversion::project_entity_screen_position_standard()` | partial |
| `0x5B1E` | `sort_entity_slots_by_depth` | Stably sorts the sixteen entity slots by descending effective depth, treating source `0x10`/`0x40` entities as depth zero. | `ghidra_conversion::sort_entity_slots_by_depth()` | partial |
| `0x5B7E` | `mark_entity_slots_for_render` | Sets the render-update bit on all sixteen entity slots. | `ghidra_conversion::mark_entity_slots_for_render()` | partial |
| `0x5FDC` | `advance_entity_animation_accumulator` | Adds one animation increment, advances one frame on a period crossing, and marks the entity for redraw. | `ghidra_conversion::advance_entity_animation_accumulator()` | partial |
| `0x61A0` | `append_entity_sprite_record` | Appends the source pointer, attribute word, and frame-derived tile offset as one typed sprite record. | `ghidra_conversion::append_entity_sprite_record()` | partial |
| `0x61CC` | `append_sprite_record` | Appends the source pointer, attribute word, and caller-derived tile offset as one typed sprite record. | `ghidra_conversion::append_sprite_record()` | partial |
| `0x622C` | `expand_entity_sprite_descriptors` | Handles the optional `0x8002` (signed source `-0x7FFE`) alternate-tile marker before expanding one entity's descriptor list. | `ghidra_conversion::expand_entity_sprite_descriptors()` | partial |
| `0x624C` | `expand_entity_sprite_descriptor_list` | Projects bounded four-word sprite descriptors into ordered screen-space output records until `0x8000`. | `ghidra_conversion::expand_entity_sprite_descriptors()` | partial |
| `0xB530` | `approach_entity_fixed_point_target` | Moves a fixed-point entity coordinate by one `0x800` step or snaps it when the remaining distance is at most one step. | `ghidra_conversion::approach_entity_fixed_point_target()` | partial |
| `0xB6E4` | `select_entity_animation_table` | Disassembly-derived selector for the `<0x100`, `0x100`–`0x1FF`, `0x200`–`0x35F`, `0x360`–`0x45F`, and `>=0x460` animation table banks; the latter uses the special bank when entity flag bit 3 is set. | `ghidra_conversion::select_entity_animation_table()` | partial |
| `0xB39A` | `set_entity_heading_with_field_animation` | Stores the requested heading, selects the C042 directional animation entry after the source rotate-and-correct step, chooses delay mode `0` for behavior values `0x0B`/`0x0C` and `1` otherwise, then resets animation. | `ghidra_conversion::set_entity_heading_with_field_animation()` | partial |
| `0xB3FE` | `set_entity_heading_with_alternate_animation` | Stores the requested heading, selects the C092 directional animation entry, resets animation first, and schedules fixed delay mode `0`. | `ghidra_conversion::set_entity_heading_with_alternate_animation()` | partial |
| `0xAF6C` | `reset_entity_animation_state` | Clears the animation timer and phase, then sets the entity redraw/update bit. This shared helper is used by both heading-selection paths and other behavior transitions. | `ghidra_conversion::reset_entity_animation_state()` | partial |
| `0xABFC` | `update_entity_animation_delay` | Converts the delay factor to `0x6A - factor*4`, doubles for source mode `2`, halves for other nonzero modes, and clamps the signed result to a minimum of `15`. | `ghidra_conversion::update_entity_animation_delay()` | partial |
| `0xD7EA` | `compute_entity_delay_factor` | Chooses the first active capability bit among 12, 13, and 14 for modifiers `4`, `8`, and `12`, adds it to the wrapped byte-attribute average minus one, and floors at zero. | `ghidra_conversion::compute_entity_delay_factor()` | partial |
| `0xD81C` | `average_entity_delay_attributes` | Adds the two byte attributes with source byte wrap and returns the floor average. | `ghidra_conversion::average_entity_delay_attributes()` | partial |
| `0xAF7C` | `prepare_entity_delayed_animation_primary` | Applies normal delay mode, selects the C082 direction entry, installs the primary delayed-collision continuation, and resets animation state. | `ghidra_conversion::prepare_entity_delayed_animation_primary()` | partial |
| `0xAFAE` | `prepare_entity_delayed_animation_alternate` | Applies normal delay mode, selects the C082 direction entry, installs the alternate delayed-collision continuation, and resets animation state. | `ghidra_conversion::prepare_entity_delayed_animation_alternate()` | partial |
| `0xAFF2` | `prepare_entity_auxiliary_behavior` | Ignores an already-active behavior; otherwise either starts the C072 countdown branch at 8 or brackets the external controller/behavior effects, sets flag `0x10`, and selects C0B2 for immediate collision. | `ghidra_conversion::prepare_entity_auxiliary_behavior()` | partial |
| `0xB330` | `enter_entity_behavior_transition` | Brackets the controller effect with flag `0x40`, dispatches the selected behavior action, sets flag `0x10`, selects C0B2, and resets animation state. | `ghidra_conversion::enter_entity_behavior_transition()` | partial |
| `0xB11C` | `synchronize_entity_behavior_spawn` | Compares a refreshed related-entity signature, prepares related behavior and allocates/initializes a child when it changes, then selects C042 and enters the spawn-collision continuation. | `ghidra_conversion::synchronize_entity_behavior_spawn()` | partial |
| `0xB1DC` | `configure_entity_target_transition` | Dispatches the initial action, resolves a linked heading, selects C0A2, resets animation, and conditionally performs the follow-up action and related-child path. | `ghidra_conversion::configure_entity_target_transition()` | partial |
| `0xB49C` | `resolve_entity_fallback_movement` | Selects the source heading, probes base/quarter-turn/half-turn direction slots through the movement-result policy, commits a failed candidate position when needed, and restores the saved continuation. | `ghidra_conversion::resolve_entity_fallback_movement()` | partial |
| `0xBB86` | `evaluate_entity_movement_route` | Derives the even heading route key, accepts two active route markers immediately, or selects and probes the appropriate fallback direction slot through the movement-result policy. | `ghidra_conversion::evaluate_entity_movement_route()` | partial |
| `0xBADE` | `advance_entity_movement_step` | Transfers linked collision context, probes the requested direction, handles negative recovery, branches through aligned or diagonal movement checks, updates animation state, and commits or resets the step. | `ghidra_conversion::advance_entity_movement_step()` | partial |
| `0xB442` | `reset_entity_movement_state` | Clears movement flag bit 1, restores the desired heading, replaces the raw `0xB45C` continuation with `ReevaluateCollision`, marks animation refresh, selects the rotated heading's animation entry, and rechecks the current position. | `ghidra_conversion::reset_entity_movement_state()` | partial |
| `0xB48C` | `evaluate_entity_collision` | Sends the candidate coordinates through the field-collision boundary, copies the returned four-byte collision state to the entity record, and returns the collision code. | `ghidra_conversion::evaluate_entity_collision()` | partial |
| `0xB732` | `apply_movement_result_policy` | Normalizes a negative directional probe result to `0x7F` for an active field transition, high result bits, or scene `0x14`/collision mode `2` result bands `2`–`5`; non-negative values pass through. | `ghidra_conversion::apply_movement_result_policy()` | partial |
| `0xB77E` | `probe_entity_directional_collision` | Selects one of eight signed movement vectors, adds it to the entity position, queries the collision boundary, records the source two-byte direction slot, and maps negative results to `0x7F` unless entity flag bit 1 is set. | `ghidra_conversion::probe_entity_directional_collision()` | partial |
| `0xBBE4` | `commit_entity_position` | Writes an entity position and clears its pending transition bit. | `ghidra_conversion::commit_entity_position()` | partial |
| `0xBC36` | `build_collision_probe_points` | Selects one of four three-point collision-offset banks from the signed probe bytes, expands it, and snapshots the camera words unless the source mode gate is active. | `ghidra_conversion::build_collision_probe_points()` | partial |
| `0xBCAE` | `apply_mode_five_boundary_motion` | Applies the mode-5 vertical adjustment in the source's strict coordinate windows at the two field edges. | `ghidra_conversion::apply_mode_five_boundary_motion()` | partial |
| `0xE8A0` | `sample_terrain_collision_tiles` | Samples the center tile or four ±12/±7 corner tiles, retains the highest unsigned code with later ties winning, and overrides it with `0x7F` when any corner is blocked. | `ghidra_conversion::sample_terrain_collision_tiles()` | partial |
| `0xE9EE` | `contains_terrain_object_bounds` | Tests a point against the selected terrain object's center with horizontal bounds of ±12 and vertical bounds of ±7. | `ghidra_conversion::contains_terrain_object_bounds()` | partial |
| `0xEAD2` | `evaluate_terrain_direction_boundary` | Applies the terrain-code `0x38` direction relation to unsigned word deltas within the decoded tile window. | `ghidra_conversion::evaluate_terrain_direction_boundary()` | partial |
| `0xEDFA` | `decode_terrain_tile_window` | Expands the packed terrain tile index into its horizontal origin, row origin, and 32-unit row end. | `ghidra_conversion::decode_terrain_tile_window()` | partial |
| `0xEE10` | `rebuild_terrain_collision_map` | Clears the bounded 64x64 terrain map, rasterizes inclusive 32-unit rectangle records, and invokes the explicit post-processing boundary. | `ghidra_conversion::rebuild_terrain_collision_map()` | partial |
| `0xEE86` | `replace_nth_terrain_marker` | Replaces the requested zero-based occurrence of a terrain marker, matching the source's first-marker behavior for ordinal zero. | `ghidra_conversion::replace_nth_terrain_marker()` | partial |
| `0xFEBE` | `move_selection_column` | Moves the active column by a signed step and wraps within the source row's available range. | `ghidra_conversion::move_selection_column()` | partial |
| `0xFF06` | `move_selection_row` | Changes the two-row selection mode only when at least seven entries exist and clears it when the active column is unavailable. | `ghidra_conversion::move_selection_row()` | partial |
| `0xC376` | `move_wrapped_menu_cursor` | Adds the signed menu step as a byte, wraps a negative candidate to the supplied upper cursor value, wraps a candidate above that value to zero, and leaves the paired VDP writes explicit. | `ghidra_conversion::move_wrapped_menu_cursor()` | partial |
| `0xC8B0` | `apply_byte_counter_delta_with_negative_floor` | Subtracts one from the caller's one-based counter selector, adds the signed byte delta to the selected counter, and replaces a negative result with the source `0xFF` sentinel. | `ghidra_conversion::apply_byte_counter_delta_with_negative_floor()` | partial |
| `0xC134` | `normalize_source_text_line` | Maps source high-bit glyph markers, stops on the line terminator range, and pads the remaining line width. | `ghidra_conversion::normalize_source_text_line()` | partial |
| `0x15CA4` | `compute_diagonal_step_cost` | Computes the larger absolute axis delta plus half of the smaller delta rounded up; the record-based `0x15CC0` variant performs the same metric after loading its two points. | `ghidra_conversion::compute_diagonal_step_cost()` | partial |
| `0x15C58` | `is_entity_target_within_range` | Requires target flag bit 0, rejects bit 6, measures the linked target, and reports distance below `0x200`. | `ghidra_conversion::is_entity_target_within_range()` | partial |
| `0x15C84` | `update_entity_heading_from_linked_target` | Applies the caller's measured heading only when a linked target exists and passes the `0x15C58` range gate. | `ghidra_conversion::update_entity_heading_from_linked_target()` | partial |
| `0x15CC0` | `compute_diagonal_step_cost` | Computes the same diagonal movement cost from two source records, now represented as ordinary host-coordinate deltas. | `ghidra_conversion::compute_diagonal_step_cost()` | partial |
| `0x15D3C` | `measure_relative_heading_and_distance` | Subtracts two world positions and derives the source fixed-point heading plus half-major-axis distance. | `ghidra_conversion::measure_relative_heading_and_distance()` | partial |
| `0x14D58` | `measure_entity_target_heading_and_distance` | Loads the current and linked entity positions before applying the shared relative heading/distance metric; the source preserves unrelated caller registers. | `ghidra_conversion::measure_entity_target_heading_and_distance()` | partial |
| `0x14D6A` | `select_entity_animation_table_row` | Selects the standard or alternate table from the signed entity variant and converts the one-based animation id into a `0x30`-byte row offset. | `ghidra_conversion::select_entity_animation_table_row()` | partial |
| `0x14C7C` | `resolve_entity_target_motion` | Dispatches entity types `0xC`, `0xD`, and `0xE` through close/medium/far distance bands, linked-target range gates, heading writes, and the eighteen-slot close-effect allocation branch. | `ghidra_conversion::resolve_entity_target_motion()` | partial |
| `0x14F26` | `probe_entity_world_position` | Combines a record's fixed-point world coordinates with signed `0x100` tile offsets before invoking the terrain-probe boundary. | `ghidra_conversion::probe_entity_world_position()` | partial |
| `0x14F50` | `advance_entity_collision_recovery` | Retries an empty/boundary terrain result through the source byte counter, commits the fixed-point probe position, and invokes the typed collision scan. | `ghidra_conversion::advance_entity_collision_recovery()` | partial |
| `0x150E8` | `derive_entity_heading_offsets` | Uses the extracted `0x15E16`/`0x15E56` direction tables, signed `0xC` scale, and arithmetic four-bit shift to produce the record's horizontal and vertical offsets. | `ghidra_conversion::derive_entity_heading_offsets()` | partial |
| `0x14FF8` | `initialize_entity_spawn_record` | Clears and seeds a child record, resolves an active linked target heading, applies the extracted `0x15A4C`/`0x15A6C` placement tables, and stores heading offsets. | `ghidra_conversion::initialize_entity_spawn_record()` | partial |
| `0x1514E` | `process_entity_spawn_cycle` | Applies the global-mode and animation-window gates, allocates the primary/secondary records, selects the source resource and direction animation tables, and clones the placed spawn into the follow-up record. | `ghidra_conversion::process_entity_spawn_cycle()` | partial |
| `0x15118` | `initialize_entity_behavior_record` | Clears the typed behavior-record fields and installs the source `-12` vertical offset, `0x1000` timer, `0x4000` step interval, active flag, and update continuation. | `ghidra_conversion::initialize_entity_behavior_record()` | partial |
| `0x152FA` | `prepare_special_spawn_record` | Moves the primary spawn into the special follow-up state, saving its placed coordinates and setting the `0x20`/`1` transition markers. | `ghidra_conversion::prepare_special_spawn_record()` | partial |
| `0x15ABA` | `find_colliding_entity` | Scans the source ten-record collision range, applies active/occupancy/window filters, and exposes collision-mode and source-override exits as typed policy. | `ghidra_conversion::find_colliding_entity()` | partial |
| `0x15D4E` | `measure_heading_and_distance` | Converts a signed delta pair into clockwise heading units and the source half-major-axis distance using the extracted 1024-entry correction table as a typed host constant. | `ghidra_conversion::measure_heading_and_distance()` | partial |
| `0x148D2` | `choose_dominant_axis_directions` | Chooses horizontal slots `6/2` and vertical slots `0/4` from signed deltas, then places the dominant axis first while preserving horizontal priority on ties. | `ghidra_conversion::choose_dominant_axis_directions()` | partial |
| `0x14756` | `clear_entity_route_flags` | Clears source movement flag bits `1` and `3` after route selection finishes or fails. | `ghidra_conversion::clear_entity_route_flags()` | partial |
| `0x14682` | `select_obstruction_route` | Sets the pending-route bit, rejects an occupied target, queues a direct strip step when possible, or exposes bypass/replan as typed route actions. | `ghidra_conversion::select_obstruction_route()` | partial |
| `0x14764` | `find_alternate_route_tile` | Accumulates a primary grid step until the candidate's side opens, or returns the previous candidate at an occupied/bounded stop. | `ghidra_conversion::find_alternate_route_tile()` | partial |
| `0x14AF4` | `find_rotating_route_strip_target` | Converts the entity position to an obstruction tile, tries five cardinal strip orientations, and returns the first non-current candidate. | `ghidra_conversion::find_rotating_route_strip_target()` | partial |
| `0x148AA` | `find_first_occupied_obstruction_tile` | Advances by the selected cardinal obstruction-map delta until a nonzero tile is reached; the host bounds the search at the map edge. | `ghidra_conversion::find_first_occupied_obstruction_tile()` | partial |
| `0x14800` | `probe_obstruction_strip` | Advances through open tiles along a primary direction while the adjacent secondary tile remains occupied, with explicit host edge reporting. | `ghidra_conversion::probe_obstruction_strip()` | partial |
| `0x14854` | `can_bypass_obstruction` | Tests the primary and opposite-primary obstruction paths and reports whether the source route remains blocked. | `ghidra_conversion::can_bypass_obstruction()` | partial |
| `0x1494C` | `is_obstruction_path_blocked` | Walks the packed six-bit-column/0x40-row obstruction line and reports the first occupied tile encountered. | `ghidra_conversion::is_obstruction_path_blocked()` | partial |
| `0x14914` | `is_entity_path_blocked` | Converts entity and target world positions to packed grid indices, then checks the obstruction line in both source directions. | `ghidra_conversion::is_entity_path_blocked()` | partial |
| `0x149C0` | `advance_obstruction_route_queue` | Consumes packed route entries, decodes the next tile and cost, completes toward the entity target, and exposes path-blocked/replan exits. | `ghidra_conversion::advance_obstruction_route_queue()` | partial |
| `0x14BA6` | `update_entity_axis_relation_state` | Updates signed X/Y relation bits, preserves the source movement-mode gate, and reports the unresolved route-advance handoff at `0x14682`. | `ghidra_conversion::update_entity_axis_relation_state()` | partial |
| `0xE00A` | `wait_for_controller_event` | Services frame timing, controller latch/sample, and caller-provided update work until the high event byte is nonzero, then takes one final sample. | `ghidra_conversion::wait_for_controller_event()` | partial |
| `0xF312` | `wait_for_controller_event` | Shares the same event-wait loop as `0xE00A`; its differing source update path is supplied by the host callback boundary. | `ghidra_conversion::wait_for_controller_event()` | partial |
| `0x118C6` | `cycle_inventory_action_selector` | Applies the horizontal action delta across selectors `2..5`, wraps `2 - 1` to `5` with the source redraw sentinel, and wraps `5 + 1` to `2`. | `ghidra_conversion::cycle_inventory_action_selector()` | partial |
| `0xA7E4` | `initialize_entity_slot_order` | Replaces the source sixteen-pointer, `0x80`-stride initialization with stable host slot indices `0..15`. | `ghidra_conversion::initialize_entity_slot_order()` | partial |
| `0xAE5E` | `schedule_entity_collision_recheck` | Sets the entity pending-reaction bit and records the collision-recheck continuation as typed state. | `ghidra_conversion::schedule_entity_collision_recheck()` | partial |
| `0x1A2CC` | `allocate_first_free_effect_slot` | Scans eighteen source effect bytes, marks the first zero slot occupied, and returns the table-size sentinel when full. | `ghidra_conversion::allocate_first_free_effect_slot()` | partial |
| `0x1A342` | `find_first_available_world_effect_entity_slot` | Scans seven world-effect entity slots and returns slot `3..9`, or the full-table sentinel `10`. | `ghidra_conversion::find_first_available_world_effect_entity_slot()` | partial |
| `0x1DBAA` | `schedule_world_activity_timeout` | Applies the guarded-scene early exit, arms the source timeout marker, and selects the short `0x78` or long `0x258` timeout. | `ghidra_conversion::schedule_world_activity_timeout()` | partial |
| `0x1DF00` | `clear_entity_effect_pending_flags` | Clears bit 0 across the six world-record effect flag bytes. | `ghidra_conversion::clear_entity_effect_pending_flags()` | partial |
| `0x1DFC4` | `advance_world_effect_trigger` | Finishes the latched transition countdown or applies the source cadence, PRNG, bounded-region, and transition gates. | `ghidra_conversion::advance_world_effect_trigger()` | partial |
| `0x1E04A` | `begin_world_effect_transition` | Schedules the scene-14 timeout or starts the mode-2 transition with the source `0x960` timer and sound/flag/effect boundaries. | `ghidra_conversion::begin_world_effect_transition()` | partial |
| `0x1E0B8` | `promote_entity_effect_flags` | Promotes bit-1 effect records to the source bit-2 state across all six world records. | `ghidra_conversion::promote_entity_effect_flags()` | partial |
| `0x1E0DC` | `spawn_world_effect_entities` | Walks the typed effect descriptors, allocates entity/effect slots, applies grouped vertical origins, marks effect state, projects entities, and hands off visible descriptor coordinates. | `ghidra_conversion::spawn_world_effect_entities()` | partial |
| `0x1E25A` | `spawn_world_effect_from_coordinate_sequence` | Allocates a coordinate-sequence effect, applies the zero-first-coordinate fallback, updates the active count, and schedules the timeout. | `ghidra_conversion::spawn_world_effect_from_coordinate_sequence()` | partial |
| `0x1E4AC` | `spawn_world_effect_at_player_position` | Allocates the flagged player-relative effect and places it at the player X coordinate plus `0x80`. | `ghidra_conversion::spawn_world_effect_at_player_position()` | partial |
| `0x44BC` | `initialize_entity_record_from_template` | Seeds the shared entity record from the template, installs slot/state/position fields, selects the low-nibble or scene-`0x37` variant, raises the special effect flag, and exposes the source override list as a host boundary. | `ghidra_conversion::initialize_entity_record_from_template()` | partial |
| `0x4466` | `spawn_entity_template_list` | Consumes the terminated template list, allocates paired slots, resets or advances the `0xC0` vertical origin, marks root records, and links child records to the current root. | `ghidra_conversion::spawn_entity_template_list()` | partial |
| `0x43B6` | `process_pending_entity_template_lists` | Services the scene-2 boundary, prepares pending data, drains the generated list, applies selected-list timeout behavior, and drains the default list in source order. | `ghidra_conversion::process_pending_entity_template_lists()` | partial |
| `0x4388` | `finalize_pending_entity_template_update` | Runs the pending-list coordinator and installs the mode-2 alternate or mode-1 primary state marker. | `ghidra_conversion::finalize_pending_entity_template_update()` | partial |
| `0xA800` | `prepare_pending_entity_template_list` | Applies the source gate order, increments the one-shot selection counter, selects the generated list, or installs the appropriate static fallback list. | `ghidra_conversion::prepare_pending_entity_template_list()` | partial |
| `0x5BA98` | `select_pending_entity_template_list` | Selects the scene-relative list row and one-based counter entry from typed host tables. | `ghidra_conversion::select_pending_entity_template_list()` | partial |
| `0x5BA50` | `load_scene_resources` | Decodes the scene resource, exposes it to the presentation/import boundary, installs the three source index entries at the typed stride, and publishes readiness. | `ghidra_conversion::load_scene_resources()` | partial |
| `0x5BA66` | `install_scene_resource_index_entries` | Installs the three source index pairs at offsets `0`, `stride`, and `stride * 2`, then sets the resource-ready state. | `ghidra_conversion::install_scene_resource_index_entries()` | partial |
| `0x1530` | `decode_and_upload_source_stream` | Decodes the source stream and performs the original VDP upload; native conversion keeps only the decoded host bytes and an explicit upload boundary. | `ghidra_conversion::decode_source_byte_stream()` plus presentation boundary | partial |
| `0x14902` | `compute_obstruction_grid_index` | Converts world X/Y into the source obstruction-grid index using a `0x200` horizontal unit and `0x40`-byte row stride. | `ghidra_conversion::compute_obstruction_grid_index()` | partial |
| `0x14B4E` | `decode_obstruction_tile_projection` | Decodes packed tile coordinates into host X/Y positions offset by `0x100`, then selects horizontal `2/6` or vertical `4/0` from the reference tile relation. | `ghidra_conversion::decode_obstruction_tile_projection()` | partial |
| `0x14C56` | `project_heading_position` | Applies the extracted eight-entry signed vector table at `0xB2F6` to the current world position using the heading word's top three bits. | `ghidra_conversion::project_heading_position()` | partial |
| `0x14C2E` | `derive_entity_axis_relation_flags` | Clears the relation byte, sets bit 0 when entity X is no greater than target X, and sets bit 1 for the equivalent Y comparison. | `ghidra_conversion::derive_entity_axis_relation_flags()` | partial |
| `0x15A8C` | `find_first_available_entity_slot` | Scans six `0x100`-stride source records for the first clear active bit and returns slot `9..14`, or `15` when full. | `ghidra_conversion::find_first_available_entity_slot()` | partial |
| `0x195C` | `sample_controller_input` | Samples controller words, derives newly asserted bits, and updates decoded directional state. | `ghidra_conversion::derive_controller_sample()` | partial |
| `0x19F2` | `read_controller_port` | Selects the controller port, reads both phases, combines the six-button byte, and restores the Z80 bus. | Windows input adapter boundary | pending |
| `0x1BAC` | `finalize_title_input_sequence` | Recognizes the seven-byte title input sequence or clears the bounded history. | `ghidra_conversion::finalize_title_input_sequence()` | partial |
| `0x1BEA` | `record_title_input_edge` | Stores up to eight nonzero title-loop edge bytes in the source event buffer. | `ghidra_conversion::record_title_input_edge()` | partial |
| `0x1C1E` | `advance_title_text_state` | Counts down the completed-logo text delay, steps the target palette every 32 counts, then emits the completed title text setup. | `ghidra_conversion::advance_title_text_state()`; palette/copyright presentation remain boundaries | partial |
| `0x1C6A` | `advance_title_sprite_state` | Advances the title sprite cadence/phase and requests a SAT projection rebuild. | `ghidra_conversion::advance_title_sprite_state()`; sprite projection remains a presentation boundary | partial |
| `0x1D7A` | `setup_title_state` | Clears four title words, runs the resource/font setup order, publishes palette bits `0xD`, and advances the palette for 17 frames. | `ghidra_conversion::setup_title_state()`; resource operations and palette presentation remain callbacks | partial |
| `0x1DCA` | `draw_title_prompt_variant_a` | Dispatches one register-selected title prompt/text record during the blink schedule. | Explicit conversion boundary; source text writer unresolved | pending |
| `0x1DE0` | `draw_title_prompt_variant_b` | Submits the alternate title prompt record to the common text writer at tile base `0xCA84` with palette flags `0x2000`. | `ghidra_conversion::draw_title_prompt_variant_b()`; source text writer remains a callback | partial |
| `0x1DF6` | `draw_title_copyright` | Submits four copyright records at tile bases `0xCB04`, `0xCB84`, `0xCC04`, and `0xCC84`. | `ghidra_conversion::draw_title_copyright()`; source text writer remains a callback | partial |
| `0x1E4E` | `reveal_title_logo_column` | Consumes one two-byte logo descriptor, requests its column render, and enters the completed-logo state at column `0x25`. | `ghidra_conversion::advance_title_logo_column()`; source blitters and 16-word palette copy remain events | partial |
| `0x2634` | `cycle_title_circuit_lights` | Every fifth vblank, selects one of seven eight-word title light palettes. | `ghidra_conversion::advance_title_circuit_lights()`; palette write boundary remains explicit | partial |
| `0x26D0` | `advance_title_animation_timer` | Advances the title animation timer and sets the source update flag on wrap. | `ghidra_conversion::advance_title_animation_timer()` | partial |
| `0x1A9E` | `run_title_loop` | Title reveal/update loop, Start edge transition, palette fade, and handoff to startup menus. | `ghidra_conversion::advance_title_prompt_schedule()` for prompt cadence; draw calls remain explicit boundaries | partial |
| `0x3084` | `run_field_loop` | Field update loop around movement, entity state, camera, and object output. | `NativePortShell::update()` field branch | partial |
| `0x5D746` | `run_pre_title_sequence` | Pre-title SEGA sequence: resource setup, rising figure, logo phases, horizontal-scroll noise, and timed fades. | `NativePortShell::update_boot()` | partial |
| `0x5DA50` | `setup_pre_title_resources` | Loads the pre-title planes, tile data, and source SAT/palette resources. | `decode_genesis_native_assets()` / boot asset catalog | partial |
| `0x5DAC4` | `fill_boot_horizontal_scroll` | Generates one horizontal-scroll word per visible line from the source PRNG. | `NativePortShell::update_boot()` | partial |
| `0x5D8BA` | `step_pre_title_logo_strobe` | Advances the pre-title logo palette strobe and figure fade cadence. | `NativePortShell::update_boot()` | partial |
| `0x5D938` | `advance_pre_title_animation` | Updates the pre-title SAT/timer state around the logo phase. | `NativePortShell::update_boot()` | partial |
| `0x5D996` | `advance_pre_title_logo_phase` | Advances the timed SEGA logo phase and its completion gate. | `NativePortShell::update_boot()` | partial |

## Initial coherent C++ conversions

`0x0330` and `0x0436` are no longer represented only as bus traces. The
native layer now exposes `StartupDisplayPlan` and `StartupTextTransfer` in
`../include/shadowrun/boot.hpp`; `boot.cpp` implements the transformations as
ordinary C++ data construction. `ShadowrunBoot` retains a narrow bus adapter
for source-verification tests, while the host result is independent of Genesis
addressed memory.

## Call annotation policy

Direct calls are annotated with the target address and reviewed symbol.
Indirect calls remain explicit and are not guessed. Current unresolved sites
are recorded in `call-sites.tsv` at `0x0B42`, `0x4658`, `0xC444`, `0x11884`,
`0x15928`, `0x1640A`, `0x19A1C`, `0x1B892`, `0x1E728`, `0x1EF28`, `0x1F26E`,
`0x1F58A`, `0x24E32`, `0x53AA4`, `0x54960`, and `0x5AF48`.

Resolve these only when ROM/data-flow evidence identifies the target. Do not
replace them with guessed aliases.
