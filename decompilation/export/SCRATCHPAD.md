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
`../tools/ghidra/ExportShadowrun.java`. The current pass has reviewed all 906
exported functions; no generated `FUN_<address>` names remain. The six
hardware-vector entries and the six largest/previously native-owned entries
are named explicitly as vector, renderer, terrain, menu, or save boundaries.

## Export files

- `functions.tsv`: one row per function and generated C file.
- `callgraph.tsv`: resolved caller/callee edges with call-site counts.
- `call-sites.tsv`: every direct call plus every unresolved/indirect call.
- `disassembly.s`: instructions with resolved call comments.
- `ghidra-c/fn_*.cpp`: individual decompiler output with annotated callees.

## Runner record field access

The live runner record is a 0x100-byte source structure. Direct field access is
now exposed in the separate conversion-only `runner_record_access_conversion`
library. `read_runner_record_fields()` and
`write_runner_record_fields()` preserve the source big-endian layout and only
touch fields with confirmed consumers; the remaining bytes stay opaque.

| Source path | Confirmed field use |
| --- | --- |
| `0x044BC` `initialize_entity_record_from_template` | copies the template and initializes posture, ammunition, equipment, attributes, skills, and archetype/status fields |
| `0x036FC` `advance_entity_asset_meter` | updates weapon ammunition or inventory phase bytes and clears exhausted equipment |
| `0x038AC` `service_entity_ammo` | services the equipped weapon's ammunition/clip state |
| `0x0D82A` `render_attributes_and_skills` | reads `+0x77..+0x8C`, race `+0x9A`, and class/archetype `+0x9B` for the command display |
| `0x0D5DC` `render_cyberware_detail` | reads the installed-cyberware bitfield at `+0x90` |
| `0x107CE` `adjust_runner_posture` | clamps the five-state posture byte at `+0x4D` |
| `0x11740` `run_inventory_actions` | dispatches inventory cell bytes, phase bytes, equipped slot/item, armor, clips, and action branches |
| `0x11B58` clips case | reads `+0x4E/+0x4F` and performs reload, trade, and distribution actions |
| `0x12CEE` `update_equipment_combat_bars` | selects the equipped item/cyberware branch used for attack/defense bars |
| `0x597EE` / `0x598B0` | projects confirmed live-record fields to and from compact save records |

The concrete field-consuming operations are in the separate
`shadowrun_ghidra_runner_record_operations` library:

| Source entry | Converted operation | Fields exposed |
| --- | --- | --- |
| `0x0D82A` | `read_attributes_and_skills()` | race, archetype, `+0x77..+0x8C` |
| `0x0D5DC`, `0x11950` | `read_cyberware_detail()` | all sixteen bits at `+0x90` |
| `0x036FC` | `advance_entity_asset_meter()` | flags, equipped slot/item, ammo, inventory code/phase |
| `0x038AC` | `service_entity_ammo()` | record type, equipped item, ammo; delegates to `0x036FC` |
| `0x107CE` | `adjust_runner_posture()` | posture `+0x4D` |

These are state operations derived from the source routines, not bookkeeping
labels and not emulator hooks. The VDP/menu calls around the original
mutations remain outside the typed operations.

## World-effect mode selection

The intro/world-effect selector is now converted as one typed slice rather
than four address-only labels. The direct source trace is:

| ROM entry | Recovered behavior | Typed conversion |
| --- | --- | --- |
| `0x562D4` | Adds the unsigned `FFE169` effect-timer byte to the D4 difficulty word. | `add_world_effect_timer_modifier(base, effect_timer)` |
| `0x562DE` | Adds twice the unsigned `FFE16A` effect-phase byte to the same difficulty word. | `add_world_effect_phase_modifier(base, effect_phase)` |
| `0x56116` | Uses `FFF0A3` as a three-state intro marker. A negative marker performs two thresholded rolls, compares them, optionally advances the HUD transition, enters the menu service, and either initializes or disables world effects. A zero marker starts mode 2 unless variant 1 is already active. | `advance_world_effect_intro_gate()` |
| `0x561E8` | Reads the selected secondary compact record, increments the current animation index when it differs, then routes the signed `+0x13` status into action-count, delayed-effect, or intro-gate behavior. | `select_world_effect_animation_mode()` |

The compact record fields used by this cluster are animation index `+0x10`,
effect status `+0x13`, intro timing `+0x06`, and delayed-effect timing `+0x09`.
The selector's status ranges are source-derived: statuses below 2 enter the
intro gate; 2--4 use the action-count branch; 5--6 use the delayed-effect
branch; 7--10 and 17--20 enter the intro gate only for the base variant;
11--16 enter the action-count branch for variant 1 and the intro gate for the
base variant; statuses 21 and above use the action-count branch. The action
branch records `FFF0DD` in `FFF0A4`, displays and advances HUD mode 8, and
returns 0 or -1. The delayed branch decrements `FFF0E7` while gated, or
subtracts one from the thresholded-roll count, sets `FFF0E7` to `10-count`,
marks `FFF0E6` negative, and returns mode 3.

`WorldEffectModeSelectionCallbacks` deliberately leaves random rolls,
selected-record lookup, action-delay calculation, HUD services, menu entry,
and world-mode initialization as typed boundaries. It is conversion/reference
code only; it is not linked into the emulator or native runtime.

## Menu selection timing

The three timing helpers called by the menu services are now represented by
one typed conversion slice:

| ROM entry | Caller | Recovered behavior | Typed conversion |
| --- | --- | --- | --- |
| `0x56434` | `0x19E32 run_menu_selection_service` | Clears `FFF0A3`, subtracts the service delta from selected record `+0x0F`, and on signed-byte expiry decrements `FFF0CE` by two; a negative rollover resets it to `0x0F` and increments `FF018D`, `FF028D`, and `FF038D`. | `advance_menu_selection_service_timer()` |
| `0x56482` | `0x19EA6 run_menu_selection_variant` | Clears `FFF0A3`, subtracts the variant delta from selected record `+0x10`, clears the selected menu record on signed-byte expiry, then uses the same phase-counter rollover. | `advance_menu_selection_variant_timer()` |
| `0x564D6` | `0x19F18 run_menu_progression_variant` | Halves D1 with a logical shift, clamps the delta to one, subtracts it from selected record `+0x06`, and clamps that timing byte to two. | `reduce_menu_progression_timing()` |

The conversion keeps audio-frame initialization and selected-record clearing
as callbacks. The record vector and phase counters are host state, not a ROM
addressed runtime structure; an unavailable selected record returns an
explicit result instead of asserting or reading arbitrary memory.

## Field HUD row meter

`0x1792E render_menu_hud_rows` calls `0x56506` once for each eligible row.
The result is multiplied by the row-glyph stride, so this is the numeric HUD
meter value rather than a generic introductory helper:

| ROM entry | Recovered behavior | Typed conversion |
| --- | --- | --- |
| `0x56632` | Scales D6 by the source difficulty table after clamping D4 to 2: 100% at difficulty 2, then 80/60/40/20%, and 10% for 7 or higher. | `scale_menu_hud_value_by_difficulty()` |
| `0x56506` | Status 4 reads row availability and scales it from the profile's special status bucket; statuses 5--7 return row availability directly; all other statuses combine action delay, the row availability value, the current record's cost/timing, the phase modifier, and the status-adjusted scale before clamping the display level. | `calculate_menu_hud_row_meter()` |

The conversion exposes `0x19C5C` row-availability reads,
`0x19BF6` status-adjusted scale selection, and `0x55F44` action-delay
calculation as typed callbacks. It does not pretend that those unresolved
source services are a runtime memory interface.

The direct slot lookup audit found an important conversion constraint. `0x19C54`
loads `FFF0D4` as the slot selector, then `0x19C5C` reads the loaded program
index from `FFFBD8 + slot*0x1` and uses that index to read the availability
byte at `FFFBCC + program`. The result remains in D7 as a byte; the callers
at `0x55FD8` and `0x56506` use it numerically in comparisons, addition, and
difficulty scaling. `read_matrix_program_availability_for_slot()` therefore
returns `availability_byte`, not `bool`. The old host conversion's truth-value
collapse was not source-faithful and is now covered by a test using `0xA5`.

The storage consumers at `0x19558` and `0x1957E` were audited against their
word instructions as well. Their D5 accumulators use `ADD.W`, and the final
D6 values use `SUB.W`; the host capacity results now retain 16-bit wrap rather
than widening the sums and publishing a signed deficit. This matters even
though normal Matrix data keeps the totals small, and is covered by an
overflowing availability/footprint test.

The load-timing leaf at `0x195AA` was audited in the same pass. `D1.w` is
copied to `D0.w` before `DIVU.W`, and the cap is controlled by the source
`CMP.W #7` followed by `BMI`; the conversion now preserves both the low-word
input and that wrapped sign-bit predicate. The normal Matrix range remains
0..6 steps, with malformed high-word/high-quotient cases covered explicitly.

## Primary world-effect pending roll

`0x1AB6E advance_primary_world_effect_overlay` calls `0x563CC` when the
primary effect has not yet crossed its threshold. The call is not merely an
intro transition: `0x563CC` obtains a trial count from the `0x19BF6`
status-adjusted auxiliary selection, clamps the pending amount to a minimum
difficulty of two, and calls `0x0DBA`. If the resulting roll count exceeds the
comparison byte, the excess is subtracted from the pending byte. Signed-byte
underflow clears both the pending amount and depletion counter; otherwise a
nonzero depletion counter is decremented. Both successful reduction paths
reset the title transition through `0x1B1FC`.

The typed conversion is `advance_primary_world_effect_pending_roll()`. The
overlay conversion's callback is named `advance_pending_roll` to preserve
that source boundary instead of mislabeling it as a generic intro request.

## World-effect transition resolver

When `0x1A5A0 advance_world_effect_mode_animation` finishes its mode-2
countdown, it calls `0x562EC`. The resolver first calculates a trial count
with `0x55F44`, adds the active profile byte, and rolls against the selected
record's `+0x06` timing plus twice `FFE16A`. It compares the new
`FFF0DD` result with `FFF0A4`:

- equal results redraw the HUD rows;
- a larger new result sets `FFE07E` to mode 1 and redraws the rows;
- a smaller result continues only when the difference exceeds the profile's
  comparison byte;
- the continuation advances HUD mode 8 directly, or runs the menu countdown
  and footer services before querying the mode again;
- modes 3, 12, 15, 23, and 24 enter the `0x563CC` pending-roll boundary,
  while all other paths redraw the HUD rows.

`resolve_world_effect_transition()` preserves that order through typed
callbacks for action delay, random rolls, HUD mode, countdown/footer work,
pending-roll advancement, and row redraw. It is still reference/conversion
code and has no dependency on the emulator's memory or VDP paths.

The importer has a separate ROM-only reader at
`src/graphics_import.cpp::read_genesis_field_entity_template()`. That reader
feeds extracted host assets/state; it is not the game runtime and is not a
replacement for the source-function conversion above.

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
| `0x0BB6` | `wait_for_even_frame` | Waits for a changed frame sample after clearing its low bit. | `ghidra_conversion::wait_for_even_frame()`; host frame boundary remains a callback | partial |
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
| `0x15554` | `service_nearby_entity_auxiliary_behavior` | Scans the ten entity records with active/non-excluded filtering, applies the normal `0x280` radius/`0x500` exclusive window or the type-`0x0E` `0x140`/`0x280` window, clamps lower bounds at zero, and calls the existing `0x1562E` auxiliary service for each match. | `ghidra_conversion::service_nearby_entity_auxiliary_behavior()` | partial |
| `0x17C3C` | `read_menu_entry_profile_eligibility` | Indexes the source `FFFB30` byte table with `FFF102`, sign-extends the selected byte, then performs the source logical four-bit word shift. The generated Ghidra C body was an empty stub; direct disassembly is authoritative here. | `ghidra_conversion::read_menu_entry_profile_eligibility()` | partial |
| `0x1A16A` | `render_matrix_runner_detail_panel` | The runner-specific child of `0x1882E`: writes two common detail grids, renders the no-active-run message when `FFF0B0` is nonnegative, gives program 4 its direct status-row path, gives programs 5..7 their special text path, and otherwise computes the delay-scaled primary/eligibility-gated secondary/final row passes. | `ghidra_conversion::render_matrix_runner_details()` | partial |
| `0x1A370` | `advance_world_effect_controller` | Per-frame world-effect coordinator: gates the active `FF163E` record, advances the descriptor transition's byte-sized noise cadence, starts descriptor loading when the current compact runner has no active effect, decrements the effect countdown, selects the next world-effect mode, and publishes the mode-2 background, mode-3 motion, or default-effect setup. The VDP noise loop is an explicit host presentation boundary. | `ghidra_conversion::advance_world_effect_controller()` / `advance_world_effect_transition()` | partial |
| `0x1A5A0` | `advance_world_effect_mode_animation` | Runs the mode-2/3/4/5/6/default world-effect animation state machine: selects the individual sprite-table entry, applies the source fixed-point position deltas, falls through into the first tick of a newly selected mode, arms title refresh after audio, rebuilds title sprites on the negative transition result, and installs descriptor D/E or the inline mode-5 descriptor pair at the source countdown boundaries. | `ghidra_conversion::advance_world_effect_mode_animation()` | partial |
| `0x55F44` | `calculate_runner_action_delay` | Combines active runner record bytes `+0x78`, `+0x7B`, and `+0x88` with `FFFBBE` profile bytes, then applies the signed `FFF0B7` normal/positive/negative scaling branches and the request-dependent floor behavior. The generated Ghidra body only retained the `0xD81C` call; direct disassembly is authoritative. | `ghidra_conversion::calculate_runner_action_delay()` | partial |
| `0x55FD8` | `calculate_and_publish_runner_action_result` | Clears the prior action result, forces the shared delay calculation's request to zero, combines the action delay with the selected Matrix-program availability byte, subtracts the selected compact record's `+0x05` gate from the low byte, and exits with `-1` when the signed byte result is nonpositive. The inline continuation at `0x55FFC` then adds twice compact `+0x06` and the world-effect phase, clamps difficulty to two, calls `0x0DBA`, and writes the resulting success count or `-1` to the active live record's `+0x9F`. The nominal exported span is only 38 bytes; the canonical ROM bytes through `0x5604E` are required to recover the actual routine. | `ghidra_conversion::calculate_and_publish_runner_action_result()` | partial |
| `0x15CA4` | `compute_diagonal_step_cost` | Subtracts and normalizes the two low words, selects the major axis with the source unsigned `CMP.W`/`BCC`, then applies `ASR.W` plus carry-based rounding and `ADD.W`. The typed helper preserves this wrapped word metric; the record-based `0x15CC0` entry reaches the same leaf after loading its point pair. | `ghidra_conversion::compute_diagonal_step_cost()` | partial |
| `0x15C58` | `is_entity_target_within_range` | Requires target flag bit 0, rejects bit 6, measures the linked target, and reports distance below `0x200`. | `ghidra_conversion::is_entity_target_within_range()` | partial |
| `0x15C84` | `update_entity_heading_from_linked_target` | Checks the linked record through `0x15C58`, writes the measured heading to record `+0x34` on success, and leaves the carry boundary explicit for the `0x14C7C` caller. | `ghidra_conversion::update_entity_heading_from_linked_target()` | partial |
| `0x15CC0` | `compute_diagonal_step_cost` | Loads two record coordinate pairs, performs the same low-word absolute-difference, unsigned-major-axis, `ASR.W`/carry-round, and `ADD.W` sequence as `0x15CA4`; the host API exposes deltas only after the record loads have been modeled. | `ghidra_conversion::compute_diagonal_step_cost()` | partial |
| `0x15D3C` | `measure_relative_heading_and_distance` | Subtracts two world positions and derives the source fixed-point heading plus half-major-axis distance. | `ghidra_conversion::measure_relative_heading_and_distance()` | partial |
| `0x14D58` | `measure_entity_target_heading_and_distance` | Loads the current and linked entity positions before applying the shared relative heading/distance metric; the source preserves unrelated caller registers. | `ghidra_conversion::measure_entity_target_heading_and_distance()` | partial |
| `0x14D6A` | `select_entity_animation_table_row` | Selects the standard or alternate table from the signed entity variant and converts the one-based animation id into a `0x30`-byte row offset. | `ghidra_conversion::select_entity_animation_table_row()` | partial |
| `0x14C7C` | `resolve_entity_target_motion` | Dispatches entity types `0xC`, `0xD`, and `0xE`: type `0xC` applies the `<0x300`, `0x300..0x5FF`, and far bands, allocates the close-effect slot when `+0xDC` is negative, and writes its D0/D1 slot record fields; types `0xD/E` temporarily project `+0x2E`, use `0x15C84`'s `+0x34` carry, restore the old current heading, and only commit both heading fields when the mode gate allows it. | `ghidra_conversion::resolve_entity_target_motion()` | partial |
| `0x14F26` | `probe_entity_world_position` | Combines a record's fixed-point world coordinates with signed `0x100` tile offsets before invoking the terrain-probe boundary. | `ghidra_conversion::probe_entity_world_position()` | partial |
| `0x14F50` | `advance_entity_collision_recovery` | Retries an empty/boundary terrain result through `+0x89`, returns carry on terrain rejection or retry exhaustion, commits D5/D6 only for a live retry, and forwards the `0x15ABA` collision carry. | `ghidra_conversion::advance_entity_collision_recovery()` | partial |
| `0x150E8` | `derive_entity_heading_offsets` | Uses the extracted `0x15E16`/`0x15E56` direction tables, signed `0xC` scale, and arithmetic four-bit shift to produce the record's horizontal and vertical offsets; the shared host helper also exposes the constructor-specific scale/shift pair. | `ghidra_conversion::derive_entity_heading_offsets()` / `derive_scaled_entity_heading_offsets()` | partial |
| `0x14FF8` | `initialize_entity_spawn_record` | Clears and seeds a child record, resolves an active linked target heading, applies the direct positive word values from `0x15A4C`/`0x15A6C` as host coordinate deltas, and stores heading offsets. | `ghidra_conversion::initialize_entity_spawn_record()` | partial |
| `0x1514E` | `process_entity_spawn_cycle` | Applies the global-mode and `+0x10` bit-1 late-window gate, services the ammo/slot boundaries, initializes the primary record, selects resource table `0x15318` or alternate `0x15336` from the indexed `+0x6E` status byte, optionally starts the mode-2 world transition, then clones the 80-word record and applies the exact `+0x3E/+0x11/+0x0E/+0x3A/+0x64/+0x41/+0x16` follow-up writes. | `ghidra_conversion::process_entity_spawn_cycle()` | partial |
| `0x15118` | `initialize_entity_behavior_record` | Clears the typed behavior-record fields and installs the source `-12` vertical offset, `0x1000` timer, `0x4000` step interval, active flag, and update continuation. | `ghidra_conversion::initialize_entity_behavior_record()` | partial |
| `0x15646` | `spawn_entity_effect_record` | Builds the short-lived effect/projectile entity during the parent's `0x3C`-frame window: advances the asset meter, claims an entity and one of eighteen effect slots, copies the source record fields, places the effect with the direct `0x15A4C`/`0x15A6C` direction words, then either derives the marker/heading offsets locally or resolves a linked target through the explicit `0x55848` boundary and clamps its travel distance to `0x0BE0`. | `ghidra_conversion::spawn_entity_effect_record()` | partial |
| `0x15830` | `dispatch_entity_effect` | Handles the linked effect path after the direct `0x07/0x0E` spawn branch: schedules the world timeout, claims an effect slot, scans ten live records around the linked target, chooses the linked record or the first nearby `0x06/0x0D` candidate, writes `+0xB2/+0x32/+0x2A/+0x02/+0x03`, and preserves the `0x4946` audio table plus `0x4964` indirect handler as typed callbacks. | `ghidra_conversion::dispatch_entity_effect()` | partial |
| `0x1597E` | `scan_entity_effect_targets` | For source types `0x03/0x05/0x0A`, marks every active, non-excluded candidate within the `0x500` diagonal-cost band; for `0x06/0x0D`, also sets the source flag/link and selects the first compatible target. | `ghidra_conversion::scan_entity_effect_targets()` | partial |
| `0x15930` | `apply_entity_secondary_effect` | Claims a local effect slot, initializes a type-2 target effect with phase zero, then crosses the source audio event `0x34` and indirect handler boundary. | `ghidra_conversion::apply_entity_secondary_effect()` | partial |
| `0x152FA` | `prepare_special_spawn_record` | Moves the primary spawn into the special follow-up state, saving its placed coordinates and setting the `0x20`/`1` transition markers. | `ghidra_conversion::prepare_special_spawn_record()` | partial |
| `0x15ABA` | `find_colliding_entity` | Scans the source ten-record collision range, applies active/occupancy/window filters, copies the candidate position only on the ordinary compatible path, and preserves collision-gate and source-override carry-only exits. | `ghidra_conversion::find_colliding_entity()` | partial |
| `0x15D4E` | `measure_heading_and_distance` | Converts a signed delta pair into clockwise heading units and the source half-major-axis distance using the extracted 1024-entry correction table as a typed host constant. | `ghidra_conversion::measure_heading_and_distance()` | partial |
| `0x148D2` | `choose_dominant_axis_directions` | Chooses horizontal slots `6/2` and vertical slots `0/4` from signed deltas, then places the dominant axis first while preserving horizontal priority on ties. | `ghidra_conversion::choose_dominant_axis_directions()` | partial |
| `0x14756` | `clear_entity_route_flags` | Clears source movement flag bits `1` and `3` after route selection finishes or fails. | `ghidra_conversion::clear_entity_route_flags()` | partial |
| `0x14682` | `select_obstruction_route` | Sets +0xB0 bit 1, rejects an occupied target, writes a direct or two-tile bypass route, or carries the heading-driven 0x14AF4 retry into the 0x149C0 queue. | `ghidra_conversion::select_obstruction_route()` | partial |
| `0x14764` | `find_alternate_route_tile` | Accumulates a primary grid step until the candidate's side opens, or returns the previous candidate at an occupied/bounded stop. | `ghidra_conversion::find_alternate_route_tile()` | partial |
| `0x14AF4` | `find_rotating_route_strip_target` | Converts the entity position to an obstruction tile, tries five cardinal strip orientations, and returns the first non-current candidate. | `ghidra_conversion::find_rotating_route_strip_target()` | partial |
| `0x148AA` | `find_first_occupied_obstruction_tile` | Advances by the selected cardinal obstruction-map delta until a nonzero tile is reached and returns the last open tile in the source D1 role; the host bounds the search at the map edge. | `ghidra_conversion::find_first_occupied_obstruction_tile()` | partial |
| `0x14800` | `probe_obstruction_strip` | Advances through open tiles along a primary direction while the adjacent secondary tile remains occupied and returns the last open candidate, with explicit host edge reporting. | `ghidra_conversion::probe_obstruction_strip()` | partial |
| `0x14854` | `find_bypass_route` | Produces the primary and secondary open route tiles written by the source; `can_bypass_obstruction` remains the boolean compatibility wrapper. | `ghidra_conversion::find_bypass_route()` | partial |
| `0x1494C` | `is_obstruction_path_blocked` | Walks the packed six-bit-column/0x40-row obstruction line and reports the first occupied tile encountered. | `ghidra_conversion::is_obstruction_path_blocked()` | partial |
| `0x14914` | `is_entity_path_blocked` | Converts entity and target world positions to packed grid indices, then checks the obstruction line in both source directions. | `ghidra_conversion::is_entity_path_blocked()` | partial |
| `0x145A2` | `advance_entity_animation_target_route` | Applies the frame-counter cutoff, clears/rebuilds +0xB1 axis relation flags, enters the +0xB0 route queue when requested, publishes the direct/queued heading-distance pair, and installs the B574 continuation. | `ghidra_conversion::advance_entity_animation_target_route()` | partial |
| `0x149C0` | `advance_obstruction_route_queue` | Tests +0xB0 bit 3 for a blocked path, consumes the packed route cursor at +0xBA/+0xBC, publishes the decoded obstruction target/cost, and returns to the 0x14A36 completion or 0x14A54 replan branches. The host uses the direct low-byte flag tests from disassembly, not the Ghidra C export's 0x0800 word mask. | `ghidra_conversion::advance_obstruction_route_queue()` | partial |
| `0x14BA6` | `update_entity_axis_relation_state` | Updates signed X/Y relation bits, preserves the source movement-mode gate, and reports the unresolved route-advance handoff at `0x14682`. | `ghidra_conversion::update_entity_axis_relation_state()` | partial |
| `0xE00A` | `wait_for_controller_event` | Services frame timing, controller latch/sample, and caller-provided update work until the high event byte is nonzero, then takes one final sample. | `ghidra_conversion::wait_for_controller_event()` | partial |
| `0xF312` | `wait_for_controller_event` | Shares the same event-wait loop as `0xE00A`; its differing source update path is supplied by the host callback boundary. | `ghidra_conversion::wait_for_controller_event()` | partial |
| `0x118C6` | `cycle_inventory_action_selector` | Applies the horizontal action delta across selectors `2..5`, wraps `2 - 1` to `5` with the source redraw sentinel, and wraps `5 + 1` to `2`. | `ghidra_conversion::cycle_inventory_action_selector()` | partial |
| `0xA7E4` | `initialize_entity_slot_order` | Replaces the source sixteen-pointer, `0x80`-stride initialization with stable host slot indices `0..15`. | `ghidra_conversion::initialize_entity_slot_order()` | partial |
| `0xAE5E` | `schedule_entity_collision_recheck` | Sets the entity pending-reaction bit and records the collision-recheck continuation as typed state. | `ghidra_conversion::schedule_entity_collision_recheck()` | partial |
| `0x1A2CC` | `allocate_first_free_effect_slot` | Scans eighteen source effect bytes, marks the first zero slot occupied, returns D0 as the slot and D1 as `0x49 + slot*0x18`, and sets the table-size sentinel when full; the host exposes both values through `EffectSlotAllocation` and retains the slot-only wrapper. | `ghidra_conversion::allocate_first_free_effect_slot_record()` / `ghidra_conversion::allocate_first_free_effect_slot()` | partial |
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
| `0x19F2` | `read_controller_port` | Acquires the controller bus, reads phase `0x00` and `0x40`, combines the six-button byte, and releases the bus. | `ghidra_conversion::read_controller_port()`; host input and bus operations remain callbacks | partial |
| `0x17708` | `render_field_hud` | Sets the field display mode, renders the HUD sprite stream, uploads five tile blocks, refreshes HUD state, and conditionally renders the mode-8 panel. | `ghidra_conversion::render_field_hud()`; presentation operations remain callbacks | partial |
| `0x530DE` | `service_audio_if_enabled` | Gates the source audio queue service on the enable flag, preserving bus acquire, command flush, append, and release order. | `ghidra_conversion::service_audio_if_enabled()`; host audio backend remains callbacks | partial |
| `0x1BAC` | `finalize_title_input_sequence` | Recognizes the seven-byte title input sequence or clears the bounded history. | `ghidra_conversion::finalize_title_input_sequence()` | partial |
| `0x1BEA` | `record_title_input_edge` | Stores up to eight nonzero title-loop edge bytes in the source event buffer. | `ghidra_conversion::record_title_input_edge()` | partial |
| `0x1C1E` | `advance_title_text_state` | Counts down the completed-logo text delay, steps the target palette every 32 counts, then emits the completed title text setup. | `ghidra_conversion::advance_title_text_state()`; palette/copyright presentation remain boundaries | partial |
| `0x1C6A` | `advance_title_sprite_state` | Advances the title sprite cadence/phase and requests a SAT projection rebuild. | `ghidra_conversion::advance_title_sprite_state()`; sprite projection remains a presentation boundary | partial |
| `0x1D7A` | `setup_title_state` | Clears four title words, runs the resource/font setup order, publishes palette bits `0xD`, and advances the palette for 17 frames. | `ghidra_conversion::setup_title_state()`; resource operations and palette presentation remain callbacks | partial |
| `0x1DCA` | `draw_title_prompt_variant_a` | Submits the primary title prompt record to the common text writer at tile base `0xCA84` with palette flags `0x2000`. | `ghidra_conversion::draw_title_prompt_variant_a()`; source text writer remains a callback | partial |
| `0x1DE0` | `draw_title_prompt_variant_b` | Submits the alternate title prompt record to the common text writer at tile base `0xCA84` with palette flags `0x2000`. | `ghidra_conversion::draw_title_prompt_variant_b()`; source text writer remains a callback | partial |
| `0x1DF6` | `draw_title_copyright` | Submits four copyright records at tile bases `0xCB04`, `0xCB84`, `0xCC04`, and `0xCC84`. | `ghidra_conversion::draw_title_copyright()`; source text writer remains a callback | partial |
| `0x03032` | `scene_uses_alternate_profile` | Accepts scene IDs `6..9` and `0x2C..0x31`, rejecting the gaps and all other signed values. | `ghidra_conversion::scene_uses_alternate_profile()` | partial |
| `0x02A32` | `initialize_field_mode_profile_a` | Selects profile A, runs the common mode-entry service, clears seven transient fields and the render-pending marker, then initializes scene-mode state. | `ghidra_conversion::initialize_field_mode(..., FieldModeProfile::profile_a, ...)` | partial |
| `0x02A44` | `initialize_field_mode_profile_b` | Performs the same typed initialization sequence for profile B. | `ghidra_conversion::initialize_field_mode(..., FieldModeProfile::profile_b, ...)` | partial |
| `0x02A94` | `initialize_mode_nine` | Runs the common mode-entry service, selects the mode resource, publishes mode 9, resets the field marker to `0xFFFF`, and establishes the source HUD/input/scene flags. | `ghidra_conversion::initialize_mode_nine()` | partial |
| `0x10918` | `prepare_menu_graphics` | Sets the menu transfer display command, runs the common buffer preparation, copies eight source metadata words, marks the resource ready, and enters the decoded-resource upload boundary. | `ghidra_conversion::prepare_menu_graphics()` | partial |
| `0x10984` | `transfer_menu_asset_batch_nine_optional` | Runs the common preparation, uploads nine tile blocks plus a tenth block when the source flag is set, and restores the display command. | `ghidra_conversion::transfer_menu_asset_batch()` with the nine-plus-optional plan | partial |
| `0x10A7E` | `transfer_menu_asset_batch_four_post_prepare` | Runs preparation, repeats the transfer command, uploads four tile blocks, and restores the display command. | `ghidra_conversion::transfer_menu_asset_batch()` with the four-block post-preparation plan | partial |
| `0x10AF0` / `0x10B5A` | `transfer_menu_asset_batch_four` | Runs preparation, uploads four tile blocks, and restores the display command. | `ghidra_conversion::transfer_menu_asset_batch()` | partial |
| `0x10BC4` | `transfer_menu_asset_batch_eight_after_copy` | Runs preparation, performs the source buffer-copy hook, uploads eight tile blocks, and restores the display command. | `ghidra_conversion::transfer_menu_asset_batch()` with the eight-block copy plan | partial |
| `0x10C96` | `transfer_menu_asset_batch_eight` | Runs preparation, uploads eight tile blocks, and restores the display command. | `ghidra_conversion::transfer_menu_asset_batch()` | partial |
| `0x1A304` | `clear_effect_slot` | Clears one of the eighteen effect-slot occupancy bytes. | `ghidra_conversion::clear_effect_slot()` | partial |
| `0x1A310` | `clear_effect_slots` | Clears all eighteen effect-slot occupancy bytes. | `ghidra_conversion::clear_effect_slots()` | partial |
| `0x1A322` | `clear_entity_effect_pending_records` | Clears the global effect marker and bit 0 across thirteen source entity records. | `ghidra_conversion::clear_entity_effect_pending_records()` | partial |
| `0x13AF2` | `find_first_entity_action_below_threshold` | Scans eight entity action/item bytes for the first value below `0x0D`, returning zero when none qualifies. | `ghidra_conversion::find_first_entity_action_below_threshold()` | partial |
| `0x13A40` | `copy_entity_anchor_position` | Copies the source entity's two-word anchor position into the destination record. | `ghidra_conversion::copy_entity_anchor_position()` | partial |
| `0x05622` | `clear_entity_runtime_fields` | Clears one transient field across ten entity records and masks the paired flags with the caller value. | `ghidra_conversion::clear_entity_runtime_fields()` | partial |
| `0x0E9C` | `make_vdp_transfer_command` | Splits a source transfer address into the two display-control words used by the Genesis transfer boundary. | `ghidra_conversion::make_vdp_transfer_command()` | partial |
| `0x0DFC` | `write_repeated_words` | Emits one word for an explicit host count through the transfer boundary. | `ghidra_conversion::write_repeated_words()` | partial |
| `0x0E18` | `write_word_stream` | Emits a bounded ordered source word stream through the transfer boundary. | `ghidra_conversion::write_word_stream()` | partial |
| `0x1056` | `clear_vdp_words` | Emits sixteen zero words through the transfer boundary. | `ghidra_conversion::clear_vdp_words()` | partial |
| `0x164DC` | `clear_scene_record_transition_words` | Clears the leading transition word in ten scene records. | `ghidra_conversion::clear_scene_record_transition_words()` | partial |
| `0x16DEE` | `clear_scene_record_pending_flags` | Clears bit 0 in the ten scene-record pending flags. | `ghidra_conversion::clear_scene_record_pending_flags()` | partial |
| `0x16E08` | `all_scene_record_slots_active` | Tests bit 0 across all ten scene-record slots. | `ghidra_conversion::all_scene_record_slots_active()` | partial |
| `0x16EA0` | `fill_terrain_work_region` | Fills a bounded rectangular terrain work-map region with value `2`. | `ghidra_conversion::fill_terrain_work_region()` | partial |
| `0x176EA` | `render_menu_footer` | Copies the footer source block, marks it dirty, refreshes menu metadata, and draws the footer in source order. | `ghidra_conversion::render_menu_footer()` | partial |
| `0x177A4` | `initialize_menu_slot_states` | Sets status `6` and timer `1` for all five menu slots. | `ghidra_conversion::initialize_menu_slot_states()` | partial |
| `0x111D0` | `upload_menu_tile_block` | Uploads one menu tile block through the presentation boundary. | `ghidra_conversion::upload_menu_tile_block()` | partial |
| `0x11454` | `reset_menu_upload_cursor_and_upload` | Clears the menu upload cursor and uploads one tile block. | `ghidra_conversion::reset_menu_upload_cursor_and_upload()` | partial |
| `0x17EE4` | `reset_menu_overlay_and_upload_two` | Clears the menu overlay state and uploads two tile blocks. | `ghidra_conversion::reset_menu_overlay_and_upload_two()` | partial |
| `0x1E4E` | `reveal_title_logo_column` | Consumes one two-byte logo descriptor, requests its column render, and enters the completed-logo state at column `0x25`. | `ghidra_conversion::advance_title_logo_column()`; source blitters and 16-word palette copy remain events | partial |
| `0x2634` | `cycle_title_circuit_lights` | Every fifth vblank, selects one of seven eight-word title light palettes. | `ghidra_conversion::advance_title_circuit_lights()`; palette write boundary remains explicit | partial |
| `0x26D0` | `advance_title_animation_timer` | Advances the title animation timer and sets the source update flag on wrap. | `ghidra_conversion::advance_title_animation_timer()` | partial |
| `0x1A9E` | `run_title_loop` | Title reveal/update loop, Start edge transition, palette fade, and handoff to startup menus. | `ghidra_conversion::advance_title_prompt_schedule()` for prompt cadence; draw calls remain explicit boundaries | partial |
| `0x3084` | `run_field_loop` | Field update loop around movement, entity state, camera, and object output. | `NativePortShell::update()` field branch | partial |
| `0x5D746` | `run_pre_title_sequence` | Runs the source startup loop: typed resource setup, vblank cadence, logo strobe, 224-line scroll generation, sprite projection, audio gates, palette/input checkpoints, final cleanup, and the settling wait. | `ghidra_conversion::run_pre_title_sequence()`; unresolved child operations remain explicit callbacks | partial |
| `0x5DA50` | `setup_pre_title_resources` | Performs the recovered clear/prepare/zeroed-stream/field-refresh/decode/palette/grid child calls in source order and marks the typed dirty flags. | `ghidra_conversion::setup_pre_title_resources()`; host transfer callbacks remain explicit | partial |
| `0x5DAC4` | `fill_boot_horizontal_scroll` | Advances the converted PRNG 224 times, stores `(random >> 5) & 0x1f` per line, and invokes the typed horizontal-scroll stream boundary. | `ghidra_conversion::fill_pre_title_horizontal_scroll()` | partial |
| `0x5D8BA` | `step_pre_title_logo_strobe` | Decrements the five-tick strobe cadence, wraps the eight-phase palette selection, marks the dirty flag, or steps the entity color words at cadence two. | `ghidra_conversion::advance_pre_title_logo_strobe()` | partial |
| `0x5D938` | `advance_pre_title_animation` | Sets the source sprite markers, builds the two ordinary title records, delegates the six-phase projection gate, and sets the projection marker. | `ghidra_conversion::advance_pre_title_animation()` | partial |
| `0x5D996` | `advance_pre_title_logo_phase` | Checks the `FFDD30`/`FFDD3C` gates, advances typed `FFE066`/`FFE064` phase state, and builds two projection records on every open gate. | `ghidra_conversion::advance_pre_title_animation()`; source phase state remains caller-owned | partial |

### Command-overview refresh correction: `0x11592`

The Ghidra C export for `0x11592` is not behaviorally reliable at its
branch. The ROM disassembly is authoritative here. The routine:

1. derives the cursor sprite offsets from the command-overview selectors:
   `primary * 0x28` in `D1` and `secondary * 0x18` in `D2`;
2. initializes the sprite-record count to one, expands the `0xD29F6`
   cursor records with first sequence `2` and attribute flags `0x2000`, backs
   up to the final record and clears its sequence byte, then publishes the
   sprite-list-ready marker and `FFEF14` bit 1;
3. prepares the menu grid and reads the selected entity action through the
   typed `0x12148` boundary;
4. for a nonzero action, renders item action details, calls the entity-action
   service when the `CMP.B #0x10` result is negative, and prepares the menu
   dialog; and
5. for action zero, renders the table at source `0xD1CA5` to tile base
   `0xC520` with flags `0x6000`.

The current conversion is
`ghidra_conversion::refresh_command_overview_action_details()`. It carries
the two selector-derived offsets and all branch callbacks as typed values.
The host extraction alias for the `0xD1CA5` action-title table is still an
explicit data-extraction gap; no runtime ROM lookup or invented asset is
being used to hide it. This also corrects the stale Ghidra-C interpretation
that branches on the prior `FFEF14` bit instead of the action-code read.

The adjacent page drivers are now traced as well. `0x11442` preserves the
caller's signed `D0` through the upload leaf; that value is the navigation
delta consumed by `0x114A0` and `0x11506`. The primary driver wraps the
seven-page action selector, marks page two for the panel path, refreshes
details only for pages zero and one, and otherwise resets the VDP/menu upload
cursor. The secondary driver uses `FFDD22` bits 1, 3, and 2 to suppress the
one-shot action dialog on navigation edges, arms/consumes `FFF115`, wraps a
negative secondary-page result to zero, and maps page four back to page three.
These are typed state transitions and callbacks in
`ghidra_conversion::advance_command_overview_primary_page()` and
`advance_command_overview_secondary_page()`; they do not emulate the VDP or
read Genesis memory.

### Selected item/action detail renderer: `0x12A8C`

The ROM trace for `0x12A8C` identifies this as the equipment command-overview
selected-item renderer, not a generic panel copy. It receives the one-based
item code in `D0`, calls `0x12A54` to select the resource variant at the
threshold of thirteen, and restores the incoming `D0` before returning. The
selected row is a `0x30`-byte record from the `0xD4540` item table: `+0x1E`
is the optional item-art resource and `+0x24` is the ordinary detail value.

When art is present, the routine writes the item art at source tile base
`0xC1A4` with source loop values `D1=4`, `D7=8`, then always renders the item
name at `0xC420`/`0x6000`. It reads the selected entity action through
`0x12148`. For action codes below `0x0D`, it checks the active record byte at
`+0x6E + D7` in the exact order `1, 2, 0, 3, 4, 5`; the paired overlay
destinations are `0xC19C`, `0xC19C`, `0xC1B6`, `0xC1B6`, `0xC336`, and
`0xC336`, using `D1=1`, `D7=3`, and palette flags `0x2000`.

The remaining action bands are behaviorally meaningful: `0x0D..0x13` use
the extended label table, `0x14` and `0x18..0x28` use the action-dependent
label table and repeat the `0x12148` lookup, `0x15..0x17` and `0x29..0x2B`
return before the final label/value pair, and `0x2C+` set the armor-selection
marker and call `0x11352`. The completed label path renders at `0xC31C` with
`0x4000`; ordinary actions use the item row's `+0x24` value, while extended
and action-dependent paths use `+0x6E + D7` for the detail-value grid at
`0xC39E`/`0x2000` with source loop values `D1=0`, `D7=1`.

`ghidra_conversion::render_item_action_details()` now expresses this as
typed item-art, text, phase-overlay, label, detail-value, and armor-selection
plans in `equipment_combat_bar_conversion`. The original `0xD25DC..0xD262C`,
`0xD29C4..0xD29D0`, and `0x6AC46` tables remain explicit extraction/data
boundaries; no runtime ROM address lookup or invented asset alias has been
added. If the `0x12148` record lookup is unavailable, the conversion reports
that missing source boundary instead of assuming action zero.

### Equipment action-row selector and renderer: `0x13000` / `0x12EAE`

The disassembly corrects another misleadingly generic family. `0x13000`
always renders the action-detail heading, then selects a thirteen-entry label
row. In source modes below two, the selected action comes from `0x12148`; a
zero action falls back to active-record `+0x79` plus the first set bit in
`+0x90` bits 5..8, contributing values 1..4. A nonzero action is not itself
the label row: the routine reads that action's record `+0x26`, subtracts one,
and clamps the result to 0..12. Higher modes use `+0x9D` directly when
`+0x56` is negative; otherwise they use the active `+0x57` action and the
same `+0x26` detail selector. Missing `+0x26` data is now an explicit source
boundary in `select_entity_action_detail()` rather than an invented reuse of
the action code.

`0x12EAE` is the equipment detail-row controller around that selector. With
no active action (`+0x57 == 0`), cyberware flags bit 3 wins over bit 2 for the
initial detail asset; with neither set it renders `0xD1AA5` at `0xC538` and
uses asset index one. For a higher-mode negative display state it renders the
same header only for active action five, advances `0x12FA4`, then reads the
alternate `0xD4EA0` row `+0x25`; the ordinary path reads `0xD4540` `+0x25`.
A signed-negative row value returns before selection/text/grid output.

After the `0x13000` selection, the routine renders the detail text from the
`0xD19B0` family at `0xC51C`/`0x6000`, then writes six rows from the selected
detail asset at `0xC52A`/`0x2000` (`D1=0`, `D7=6`). The conversion is now
`ghidra_conversion::render_equipment_action_detail_row()` in the equipment
menu module, with explicit source-table enums and typed callbacks for the
header, action-menu advance, selected label, detail text, and detail grid.

### Action-menu residual advance: `0x12FA4` / `0x10660`

The source `0x12FA4` path is not an available-count test. It renders the
action-menu heading from `0xD19B7` at `0xC49C`/`0x6000`, loads active-record
`+0x9D` into `D4`, converts active `+0x57` to the zero-based `D4EA0` row by
subtracting one, and calls `0x10660`. A positive residual selects the
`0xCF8F0 + (residual - 1) * 0x0E` effect row and writes six rows at
`0xC4AA`/`0x2000`; zero or negative residuals stop after the heading.

This also corrected the shared residual conversion: zero is a valid first
`D4EA0` row for `0x10660`, so `calculate_action_residual()` no longer rejects
row index zero. `advance_entity_action_menu()` now takes a typed entity,
current force, and decoded action record, and reports the exact residual/grid
plan. An absent decoded action row is reported as an unresolved source-data
boundary rather than treated as “one action remains.”

### Equipment-details parent dispatcher: `0x111E8`

`0x111E8` is the page-level equipment renderer. When `FFF112` is clear it
does only the existing `0x11928` menu-row render and returns. When set, it
clears the marker, prepares menu grid variant B, then renders the heading at
`0xC2A0`, the active runner race/state row from `0xD19C7` at `0xC1A0`, and
the class row from `0xD19E5` at `0xC1AC`.

Its action-name branch is also explicit: active action zero selects the
cyberware-bit-3 text at `0xC422`, then bit 2 at the same destination, or the
no-action text at `0xC420`; a nonzero action selects the `0xD4EA0` name row
when `+0x56` is negative and the `0xD4540` row otherwise. The wrapped
`CMP.B #0x10` gate invokes `0x12EAE` only for the low-action range. The
dispatcher then builds the physical meter at `0xC2B4`, mental meter at
`0xC334`, calls armor selection, and prepares the menu dialog in that order.

`ghidra_conversion::render_equipment_details()` now captures this parent
sequence, returns the nested `0x12EAE` result, and keeps text/meter source
rows and presentation callbacks typed. It is still a conversion/reference
module; none of these callbacks are native-port runtime plumbing.

## Initial coherent C++ conversions

`0x0330` and `0x0436` are no longer represented only as bus traces. The
native layer now exposes `StartupDisplayPlan` and `StartupTextTransfer` in
`../include/shadowrun/boot.hpp`; `boot.cpp` implements the transformations as
ordinary C++ data construction. `ShadowrunBoot` retains a narrow bus adapter
for source-verification tests, while the host result is independent of Genesis
addressed memory.

## Field mode service conversion: 0x56AC0 / 0x566EC

`0x56AC0` is the outer field-mode service controller, not an intro routine.
Its source-derived flow is now represented by
`ghidra_conversion::run_field_mode_service()` in the separate conversion
library. The conversion preserves the initial resource/palette setup, the
mode-result-specific resource branches, the numeric-menu setup, result-1
sprite-panel path, normal grid/text path, and the vblank/latch/input
precedence. The old “panel snapshot” label for `0x566EC` was wrong:

* `0x566EC` copies eight longwords (sixteen CRAM colors) into `FFD8BE`, which
  is palette bank 2, colors 32..47 of the 64-word palette upload buffer. It
  then sets `FFEF14` bit 0, publishing that a subsequent palette service must
  upload the changed full palette.
* `0x56AC0` derives the bank selector from the active runner/entity record at
  `FFFC04`: it reads record `+0x2C`, divides by `0x2000`, subtracts one, and
  indexes the ROM palette-source table at `0x18ACB6` before calling `0x566EC`.
  The exact physical interpretation of record `+0x2C` is still a data-table
  investigation, but its role in this call is proven: it chooses the active
  field/HUD palette bank.
* `0x17F8E` (`draw_hud_footer`) reaches the same helper after selecting a
  bank from the active record. Thus this is shared palette publication for
  field/HUD presentation, not a copy of panel geometry or menu state.

The service's source-derived flow is:

1. primary decrement (`0x04`), panel decrement (`0x01`), primary increment
   (`0x08`), and panel increment (`0x02`) are processed in source order;
2. confirm (`0x40` or `0x10`) is handled after navigation;
3. restart (`0x20`) re-enters setup through the graphics-service boundary;
4. return (`0x80`) restores the field asset sequence, animation assets, and
   field setup before returning.

`0x56DC4` remains an explicit typed action-handler callback. `0x587DA` is now
converted in `shadowrun_ghidra_field_mode_selection_presentation` and is no
longer described as an anonymous renderer:

* It first saves the primary cursor (`FFF0D0`) into `FFF0B8`.
* A negative service result dispatches cursor 3 to a six-grid/two-grid batch,
  cursors 4+ to eight glyph-triple submissions (with the result-selection-4
  text pass), and lower cursors to the eight three-byte records at the selected
  source table. Each record uses a glyph triple unless its leading marker is
  exactly `E`, in which case it writes one grid. The marker's game-facing
  label is not invented yet; the branch itself is directly traced.
* A result of 1 writes eight grids. Other positive results use cursor 3 for
  eight grids, cursor 5 for a five-plus-three split, and other cursors 3/4 for
  four-plus-four. Cursors below 3 first use `0x5763C`: `FFF0B6 == 6` selects
  one menu-resource record table and every other menu mode selects the other,
  then `FFF0B8` chooses a record at a `0x10` stride. The presentation then
  writes eight grids. The related `0x57628` helper adds `FFF0D0 * 2` bytes to
  that record for action-path word access; it is also used by the field-action
  and matrix/program-storage code, so the native abstraction is a shared menu
  resource selector rather than a field-only offset helper.
* A zero result uses the glyph row for cursors 0 and 3, otherwise the
  alternating record path.

The focused tests `shadowrun_ghidra_field_mode_service_loop` and
`shadowrun_ghidra_field_mode_selection_presentation` cover the palette dirty
publication, service-loop input precedence, and every presentation branch.

## Command-overview and shared active-runner HUD panels: 0x110BC / 0x110E8 / 0x110EC

`0x110BC` is the command-overview panel entry, not a generic wrapper. It reads
the active record's `+0x3E` and `+0x2C`, computes `D5 = (+0x3E * 0x50) +
0x8001` (word arithmetic), sets `D4=0xE104`, `D7=7`, and `D1=9`, then calls
`0x03648`. Because `0x03648` uses DBF counters, this is ten rows of eight
words. The native conversion exposes that as `CommandOverviewHudPanelPlan`.

`0x110EC` is not meaningfully “menu HUD panel wrapper B.” Its body prepares
the common active-runner HUD panel draw and then calls `0x03648`:

* The active record's `+0x3E` value selects one of three tile origins (`-0x796C`,
  `-0x7956`, or `-0x7940`).
* The preceding `0x110E8` shared-entry prelude loads `+0x2C` into D2. Direct
  callers of `0x110EC` prepare D2 themselves.
* The caller's D4 is preserved as the panel destination; `0x110EC` does not
  initialize it.
* The draw submits four rows of four words through the shared HUD-panel leaf.

The callers identify where the resulting panel is used: `0x0D01C` is the
attributes-and-skills menu, `0x56AC0` is the field-mode service menu, and the
`0x110E8` entry is reached from the inventory/action controller at `0x11740`.
So the useful native abstraction is a shared active-runner HUD panel layout,
with the caller supplying the surrounding menu state; it is not three
different “variant B” menus.

`0x1111A` is the active-runner name presentation: it uses the active record's
`+0x58` encoded text source, destination `0xC0A0`, and `0x6000` attributes.
The text decoder remains the separate `0xC0DA` conversion boundary.

`0x1170C` supplies the command-label table at `0xD1D3A` and enters `0x11712`.
`0x11712` selects a caller-supplied table at 0x12-byte records when
`FFF0D0 >= 2`; selectors 0 and 1 instead use the first Inventory record at
`0xD1D28`. The conversion now exposes the table alias, record index, byte
offset, destination `0xEC9C`, and `0x6000` attributes instead of a callback
with no source/layout information.

## Field action cost conversion: 0x5787C / 0x57A44 / 0x57A6E / 0x57AF6 / 0x57B04 / 0x57B42

The routines around `0x5787C` were mislabeled as an intro family. They are
field-menu cost logic called by the selected-action presentation/handler
cluster around `0x5794E` and `0x57C7A`. They are now represented by the
separate `shadowrun_ghidra_field_action_cost` conversion library in
`field_action_cost_conversion.*`:

* `0x5787C` calculates a quantity cost from the primary action category:
  category 0 is `quantity^3 * 25`, categories 1 and 2 are
  `quantity^2 * 3 * 100`, and category 3+ is `quantity^2 * 2 * 100`.
* `0x57A44` reads a selected three-byte action record. Its first byte indexes
  the source record table at `0xD4540`, where the base rate is the word at
  record offset `+0x22`. A nonzero third byte scales that rate by
  `quantity / 10`, with the source low-word truncation preserved explicitly.
* `0x57A6E` applies the mode-specific feature flags before charging/display:
  mode 4 plus `FF018F` bit `0x40` removes 10%, mode 5 plus bit `0x80` removes
  25%, mode 2 plus bit `0x04` halves the amount, and mode 8 plus bit `0x02`
  halves it. It then subtracts the runner adjustment and publishes `FFF0BE`.
* `0x57B04` is the shared runner adjustment that Ghidra failed to recover
  because its result stays in D3. It compares active-runner record `+0x8C`
  with `1AAC6E[FFF0B6]`, caps the positive margin at ten, and calculates
  `(cost >> 5) * margin`.
* `0x57AF6` is the companion display path that adds that adjustment instead
  of subtracting it.
* `0x57B42` is selected by the large-cost callers after their amount reaches
  `0x10000`. It calculates `(cost >> 8) * margin` once and subtracts that
  same value ten times; it does not recompute the shifting amount each pass.

The native code takes typed mode/feature/runner values and a base-rate lookup
callback. It has no ROM-address reads, emulated registers, or dependency on
the runtime/emulator. The exact game-facing names of the four mode categories
and the active-runner `+0x8C` stat remain data-table work; the cost behavior
itself is now explicit and tested.

## Selected field action application: 0x5794E / 0x57B8C / 0x57C5E

`0x5794E` is the mutation half of the field-menu action path, not another
intro helper. Its caller has already selected a record group; the routine
adds `group * 0x18` and `primary_cursor * 3` to reach one three-byte record.
The native conversion is `apply_selected_field_action()` in the separate
`shadowrun_ghidra_field_action_application` library:

* A high-bit first byte is the disabled/invalid marker and sets the source
  failure result before any state mutation.
* The detail panel refresh (`0x57C5E`) happens before pricing. The ordinary
  record path scans active-runner `+0x66..+0x6D` in ascending slot order and
  rejects the action if all eight slots are occupied.
* Marker `E` is a separate path: its price starts from record byte 2 and it
  increments active-runner `+0x4F` after the confirmation prompt. The source caps
  the visible count at `0x14` and rejects the transaction when the increment
  would reach `0x15`.
* Ordinary records use the `0xD4540`-backed cost calculation, then store the
  selected marker as marker+1 in the empty action slot and copy record byte 1
  into the companion `+0x6E` slot. This marker+1 write is intentional source
  behavior, not a normalization invented by the host layer.
* Funds are checked before the confirmation prompt and deducted only after the
  prompt accepts. Cancel, no-slot, insufficient-funds, and special
  capacity outcomes leave the runner record and funds unchanged.

`0x57B8C` is now labeled `run_menu_confirmation_prompt`: it draws the
two-choice prompt, services vblank/controller input, uses the shared wrapped
0/1 cursor for movement bits 8/9, and exits on the source bits 12/14. Option
0 is the accepting choice; option 1 leaves the action unapplied. It remains
an explicit presentation/input callback in the native application module;
the conversion does not emulate its VDP writes. The grouped field-action grid
conversion now models `0x57C3A/46/52/5E` as the `0xC604/3`, `0xC184/12`,
`0xC904/2`, and `0xC484/6` presets entering shared `0x57C66`; the source row
is 31 words from `0xC1320`, and the detail panel is six rows because `D1 = 5`
is consumed by `DBF`.

## Field-service resource purchase conversion: 0x5766A

`0x5766A` is a field-service purchase/threshold routine reached from the
action handler at `0x56DC4` when the selected service is outside the separate
record-action branches. It was incorrectly classified as an intro routine.
The new `shadowrun_ghidra_field_resource_purchase` library represents its
state in `field_resource_purchase_conversion.*`:

* Categories 0, 1, and 2 compare their progress counters at `FFFBC0`,
  `FFFBC2`, and `FFFBC4` against requested quantity at `FFFBBE` scaled by
  `0x28`, `0x50`, and `0x0A`. Category 3 has a fixed satisfied/limit boundary
  at three units in `FFFBC5`.
* A satisfied category takes the category-specific message path and waits;
  it does not charge the player. Category 3 uses a distinct limit message.
* An unsatisfied category reads its word cost from `0x1AC8E0[cursor]`, rejects
  insufficient funds, publishes the preview cost, and enters the shared
  `0x57B8C` confirmation prompt.
* An accepted purchase deducts the table cost and increments the selected
  progress counter by 10, 25, 5, or 1. Cancellation leaves funds and
  progress unchanged while preserving the published preview cost.

The native model deliberately leaves the exact UI text/category labels at the
data/UI boundary. The ROM proves the four distinct counter/cost behaviors;
it does not justify inventing user-facing names from the generic Ghidra output.

## Recovered field quantity branch: 0x577BA

The field-service dispatcher at `0x56DC4` has a separate `beq 0x577BA`
branch when the secondary service selector is five. The original exported
function list skipped this branch target because it was folded into the
preceding `0x5766A` range, even though `0x577BA` has its own control flow and
returns at `0x5787A`. It is now recorded as `apply_field_quantity_purchase`.

This path is distinct from the progress-counter purchase above:

* It accepts primary categories 0 through 4 and rejects category 5 or above.
* A negative `FFF0B3` availability marker exits immediately and clears the
  marker. Otherwise it refreshes the detail panel and presents the selected
  quantity `FFF0D6` and already-computed cost `FFF0BE`.
* Insufficient funds and confirmation cancellation both leave funds and the
  quantity slots unchanged, then clear `FFF0B3`.
* An accepted purchase deducts `FFF0BE`, writes `FFF0D6` to `FFFBBE` for
  category 0 or `FFFBC6..FFFBC9` for categories 1 through 4, and clears the
  marker.

Because this target was missing from the generated function inventory, the
Ghidra export plugin's address-name map, function table, call graph, call-site
annotations, and status ledger were updated together with the native module.

## Call annotation policy

Direct calls are annotated with the target address and reviewed symbol.
Indirect calls remain explicit and are not guessed. Current unresolved sites
are recorded in `call-sites.tsv` at `0x0B42`, `0x4658`, `0xC444`, `0x11884`,
`0x15928`, `0x1640A`, `0x19A1C`, `0x1B892`, `0x1E728`, `0x1EF28`, `0x1F26E`,
`0x1F58A`, `0x24E32`, `0x53AA4`, `0x54960`, and `0x5AF48`.

Resolve these only when ROM/data-flow evidence identifies the target. Do not
replace them with guessed aliases.

## World-effect scene driver: 0x1ADE6

`0x1ADE6` is the title/world scene driver, not an unexplained generic world
routine. Its ROM entry performs the scene-owned reset, uploads the decoded
scene source, initializes the selected world-effect mode, attaches descriptor
E, and then enters the frame loop at `0x1AEAC`.

The native reference conversion is
`ghidra_conversion::run_world_effect_scene_loop()` in
`world_effect_scene_loop_conversion.*`. It preserves the source decisions in
typed state and leaves presentation/input work at named callbacks:

* each tick samples vblank/controller/menu-glitch services first;
* an active world sequence (`FFDF8C`) advances until the carry/result from
  `0x1D2E6` hands control to runner selection (`FFF0A9 != 0`) or the mode
  action loop, with audio command `0x05` and the second mode-entry prepare;
* a completed scene (`FFE084`) runs the bounded jitter service and then the HUD
  fade/display-transition return path;
* the ordinary title path gates world-menu, title-resource, directional-input,
  and `0x1A370` world-effect services, then follows the `FF1592` title-service
  bits and the independently active burst/noise/profile/motion/matrix effects;
* world-effect animation is ticked only while `FFE080` is clear, followed by
  the common world-sprite append;
* controller bit 15 takes the Matrix/field refresh branch through `0x1822C`,
  then uses audio commands `0x04` and `0x03`; the resource refresh result is
  an explicit exit condition.

`source_audio_track` is only copied to `FFE189` during the prologue. The
runner-selection branch is controlled separately by `FFF0A9`; it must not be
inferred from the audio track. The conversion has a host-only frame bound for
tests, but the unbounded form follows the source return paths. It does not
enter the emulator or native Qt runtime.

## Title/world interaction service: 0x1C754

`0x1C754` is the input/behavior dispatcher reached once per active scene
frame. Its source-specific behavior is now
`ghidra_conversion::advance_title_world_interaction_service()`:

* `FFE084` installs behavior E as draw `0x1C2A8` / update `0x1C2AE` after a
  fresh behavior-record initialization;
* title-service flag bit 0 blocks the interaction path while the sweep effect
  owns the frame;
* primary behavior descriptor `0x1C36A` advances world-effect activation only
  at stage/phase `3/3`, and otherwise waits unless its stage is the terminal
  value `5`;
* secondary descriptor `0x1C350` uses the same `3/3` gate for the interaction
  update and the same terminal-stage fallthrough;
* only world-effect modes below 3, controller event bits `0x40/0x10/0x20`,
  and an inactive descriptor transition reach the `0x19976` Matrix
  program-action service.

The descriptor values and stage/phase tests are source evidence. The native
conversion exposes behavior installation, world activation, interaction, and
Matrix action as typed boundaries; it does not reproduce the behavior record
or VDP implementation.

## Secondary title/world effect service: 0x1CD5A

`0x1CD5A` is the per-frame service behind `FF1592` bit `0x40`, seeded by
`0x1CB20` when the secondary effect path is selected. It is now represented by
`ghidra_conversion::advance_title_secondary_effect_service()` rather than the
generic former export label.

The source behavior is a three-phase effect with three source-variant groups:

* variant 0 selects source sequences `0x1D906`, `0x1D918`, `0x1D922`;
* variant 1 selects `0x1D934`, `0x1D93E`, `0x1D948`;
* variant 2 selects `0x1D952`, `0x1D95C`, `0x1D966`.

Each frame decrements the lifetime, 20-tick source-variant timer, and 3-tick
phase timer. The current fixed-point position is converted to host sprite
coordinates before the named asset is submitted. A zero frame timer seeds the
paired `FFE088..FFE08E` positions for the title-effect motion; the next source
variant seeds the six `FFE090..FFE0A6` positions, and variant 2 advances both
the six-sprite motion (`0x1CF4A`) and paired motion (`0x1CEFA`).

When the lifetime crosses below zero, the service clears the `FF1592` bit,
resets `FFE0D6`, optionally arms `FFE0AA=0x0A` for the alternate motion, and
hands control to `0x19F18` menu progression after the final sprite build. The
conversion exposes that handoff as a callback and resolves the nine source
sequences to host assets; it does not perform ROM/VDP lookups at runtime.

## Title/world reveal transfer service: 0x1D1F4

`0x1D1F4` is the service reached by `FF1592` bit `0x10`, not an unidentified
world routine. It progressively reveals the title/world resource by writing
`0x47` masked longwords per active frame. The random source index is bounded by
`FFE006`, aligned to an even four-byte source offset, and added to the
`FFDF76` destination base. The mask is the source `0x1D3F4` expansion with
selector `5`; the host conversion emits each result as a
`TitleRevealWordWrite` record rather than recreating a VDP command/data port.

When `FFE008` crosses below zero, the service clears behavior-record flag
`+0x12` bit `0x10`, refreshes the source block once, and then follows the
actual scene branch: `FFE16B != 0` installs the fixed world-sequence state used
by `0x1D280/0x1D2E6`, otherwise control is handed to `0x1A01E` menu-selection
and text setup. The native boundary exposes both outcomes as typed actions and
keeps the decoded source asset behind callbacks.

## Generic export label pass and ROM verification baseline: 2026-09-26

The canonical export had 119 family/address labels left at
`convert_*_routine_0x...`. Those labels have now been replaced in the exporter
name table and synchronized export snapshots, including every generated C
function header, direct-call annotation, call-site row, and call-graph edge.
The review ledger is
`decompilation/docs/ghidra-function-label-review.tsv`; it records the behavior
claim, confidence, and disassembly/Ghidra-C evidence for every one of the 119
entries. Interrupt-vector STOP stubs and empty callback targets are explicitly
classified as reserved/no-op boundaries rather than being promoted into game
systems.

This checkout does not contain a Ghidra `analyzeHeadless` installation, so the
snapshot synchronization changed names/comments only; it did not regenerate
the ROM disassembly or function bytes. A fresh headless export remains a
separate environment-dependent check before treating the snapshot as newly
generated evidence.

Important source checks from this pass:

* `0x104C8` computes its action-table byte offset with `MULU.W #6` followed
  by `ADD.B` for the menu column, then enters the shared `0x104F6` phase/item
  path. The typed lookup now preserves the product's upper byte while wrapping
  only the byte-sized addition; a high-row test covers the distinction.
* `0x104F6` is `apply_action_item_phase_modifiers`. The generated Ghidra C
  stops after the slot-6..0 scan, but the ROM disassembly continues into the
  slot-7 item `0x29..0x2B` lookup at `0x1050E..0x1055A`. The typed conversion
  already includes that continuation; generated C alone is not sufficient
  evidence.
* `0x12C74` is `merge_equipment_action_skill_value`. The generated Ghidra C
  is a return-only body even though the ROM selects the maximum of `+0x81` and
  the action-class bucket `+0x82/+0x83/+0x84`, then performs the lower-value
  merge plus zero-to-one correction. The typed equipment conversion now takes
  the action code and all four source skill bytes, so the selector is not
  hidden in a precomputed generic skill. This is recorded as a decompiler
  omission, not treated as a no-op.
* `0x12148` uses the same source shape with `MULU.W #2` and `ADD.B`: the row
  product's high byte survives while the column wraps the low byte. The typed
  action-code lookup now preserves that word index and tests the `0x80` row
  boundary instead of narrowing the entire selector to eight bits.
* The status ledger is now one row per ROM entry. Eight duplicate rows were
  removed, including the old catalog duplicates for the magic-force functions
  and the duplicate HUD-mode/runner-record entries.
* The callgraph snapshot had two stale aggregate counts for
  `apply_field_quantity_purchase` (`0x577BA`); they were reconciled against
  the 2,986 resolved rows in `call-sites.tsv`, and the audit now checks every
  caller/target pair rather than trusting the aggregate blindly.

`decompilation/tools/verify_rom_conversion.ps1` is the structural gate. The
current baseline is 910 exported entries, 0 generic export labels, 119 reviewed
labels, 932 unique status rows, 2,986 resolved direct call sites, 16 explicit
indirect/unresolved call boundaries, 229 conversion implementation files, 28
behavioral modules, and 230 CTest cases. The gate passes the structural checks,
but behavioral fidelity remains OPEN: 928 status rows are partial and only 4
are defended. The audit therefore does not authorize a true-to-source claim
until the 16 indirect boundaries and the partial hills have been traced and
defended against ROM behavior.

## Instruction-level ROM conversion audit: 2026-09-26

The naming pass exposed a more important problem: a generated function can
have a plausible name and still be a return-only decompiler stub while the
ROM contains the actual behavior. `decompilation/tools/audit_rom_conversion.ps1`
now performs the next audit layer. It indexes the instruction spans in
`disassembly.s`, checks every resolved direct call against the generated C
body, finds explicit `ROM 0x...` evidence in the typed conversion sources,
and joins those results to the one-row-per-entry status ledger. The complete
row-level result is `decompilation/docs/rom-conversion-audit.tsv`.

The current indexed result is 910 entries, 29,141 disassembly instructions,
zero missing ROM spans, 18 native-substitution review rows, 27 generated-C
return-only risks, 20 linear adjacent-export boundary reviews, and 287 entries
without explicit ROM-address evidence in a conversion source. The boundary
classifier also records adjacent entries reached by an explicit branch or
call, but does not mislabel those as linear fall-through. These numbers
describe audit exposure, not percent converted. The structural gate still
passes, but source fidelity remains open.

### Recovered behavior slice

The following entries were traced from the ROM rather than inferred from the
generic label or the generated C:

* `0x566A2` is the action-profile table loader. It copies 5 rows x 17 words
  (0x22 bytes per row) from ROM `0x166AD8` to the working table at `FFF9FA`.
  `entity_action_profile_conversion.cpp` decodes those rows into typed event
  records and two mutable roll-modifier slots.
* `0x55828` is the action-profile selector. It follows the actor record pointer
  at `+0xD8`, reads the linked record's profile index at `+0xA0`, multiplies by
  the 0x22-byte row stride, and selects that row from `FFF9FA`. The typed
  `EntityActionDynamicModifierContext` now carries the table and linked index;
  if either is unavailable, the conversion reports an unresolved profile and
  does not substitute a fabricated roll count or difficulty.
* `0x3602` computes the HUD glyph attribute word from record fields at `+0x57`,
  `+0x9B`, and signed `+0x56`. It returns `0x2000` for the low/ordinary path
  and `0x6000` for the highlighted path. The branch matrix is now a tested
  `select_hud_glyph_attribute_flags` helper; the renderer wiring remains a
  separate audit item.
* `0x60F4` executes `ORI #1,CCR; RTS`, so its observable result is a carry-set
  return used by the descriptor branch at `0x600E`. It is represented by the
  typed `set_sprite_descriptor_carry()` boundary rather than the former empty
  hook. The caller branch still needs a complete native behavior trace.

* `0x1DC22` is a ten-record selected-bit gate, not a generic circular entity
  iterator. When a selected record exists it enters `0x1DC4C` with the
  `0x678CC` cursor sentinel. The typed service conversion now preserves the
  normal/special candidate starts, descriptor fallbacks, eight-record
  non-wrapping scan, previous-target release, and the normal-only `+0x43` clear
  and `+0xAC` back-link publication. `0x1DEA0` now uses its actual `+0x10`
  bit-6/bit-3 and `+0x0C` predicate before invoking `0x1E514`.

### Caller-sensitive fall-through correction

The audit of the world-effect selector found a real conversion error in the
timing helpers. `0x562D4` contains the timer addition and then falls directly
into `0x562DE`; there is no return at `0x562DC`. Therefore the action-count
caller at `0x56284` receives both `FFE169` and twice `FFE16A`. The transition
caller at `0x56316`, and the intro callers at `0x5614C`/`0x5618A`, enter
`0x562DE` directly and receive phase only.

The old `run_thresholded_roll` conversion treated its non-phase branch as
timer-only, which was wrong for both the action-count path and the transition
path. It now calls the combined native timing helper for the `0x562D4`
entry-path and phase-only for direct `0x562DE` callers. The focused test uses
action timer 3/phase 2 and expects difficulty 7, and delayed transition phase
9/base 4 and expects difficulty 22. This is a source-fidelity correction,
not a new game rule.

The audit now records 20 adjacent-export boundaries whose final instruction is
not an explicit control transfer. Boundaries such as `0x3D4A -> 0x3D80` and
`0x3DDE -> 0x3E14` contain explicit branch/call entries into the adjacent
shared transfer bodies and are therefore not linear fall-through risks. The
two confirmed arithmetic fall-throughs remain `0x1952C -> 0x19538` and
`0x562D4 -> 0x562DE`; `0x52E96 -> 0x52EA2` is a shared inner audio entry with
the same source-level boundary shape, now documented and tested separately.

The field-column trace also corrected a conversion omission. `0x3D4A`
calculates the pre-alignment count, emits that segment through `0x3D80`,
aligns `D4`, and then continues into the common tail for the remainder. The
typed host renderer now preserves both transfer segments and its focused test
checks the two-entry case. Matrix storage similarly treats the ROM `0xE2DF2`
footprint table as immutable source data, and the audio `0x52FD2` inner entry
is callable without a redundant bus-begin operation.

These changes deliberately do not modify the native port. They are conversion
evidence and typed source contracts only. The generated Ghidra C for the first
three routines is still incomplete in places; the ROM disassembly remains the
tie-breaker, and no entry is marked defended merely because this helper now
exists.

### Magic force/detail presentation conversion

The ROM pass for `0x10172` (`refresh_magic_force_display`) is now represented by
`ghidra_conversion::refresh_magic_force_display`. It preserves the five source
operations and both predicates: the first repeated-grid write is unconditional,
the second occurs only when the current force is at least two, the normal grid
write follows, the fourth operation occurs only when maximum and current force
differ, and the final repeated-grid write is unconditional. The grid operations
are callbacks because they are presentation services, not ROM-memory operations.

The ROM pass for `0x102D4` (`prepare_magic_action_details`) is now represented by
`ghidra_conversion::prepare_magic_action_details`. Its typed callback sequence is
grid preparation, encoded spell text three times, runner posture adjustment,
spell availability rendering, and grouped decimal glyph construction. The three
text calls remain explicit to avoid collapsing source behavior into a guessed
repeat parameter. Focused tests assert both callback sequences. These entries
remain `partial` because the presentation callback implementations still need
their own ROM-backed verification; the conversion itself does not modify the
native port.

The adjacent Magic-menu pass also now covers `0x100A8`, `0x1088C`, `0x1098E`,
and `0x10A82`. `adjust_magic_force` is a typed word-state mutation that clamps
the selected force, publishes runner record `+0x9D`, runs the source display and
combat/action refresh sequence, and restores the current/max pair around the
combat callback exactly as the source does. Spell availability uses the signed
`+0x25` record byte: unavailable emits one repeated marker, while available
emits text then one grid. The two grid helpers preserve their exact counts and
display commands (`0x8174`, or `0x8134` then `0x8174`). Their callback order and
branch counts are covered by the Magic-menu test; these remain partial only
because the host presentation callbacks are still separate conversion
boundaries.

### World-effect/menu arithmetic audit continuation

The direct ROM pass over `0x562EC` and `0x563CC` found the existing typed
control flow faithful. `0x562EC` enters `0x562DE` directly, applies the phase
modifier, then uses the source signed-byte difference and comparison-byte
branches to choose HUD-panel, countdown/footer, pending-roll, or row-redraw
services. `0x563CC` clamps the pending amount to two, returns without a
reduction for a signed-byte `BLE` result, and otherwise performs wrapped byte
subtraction before the depletion/reset follow-up. No source-shaped runtime
memory access was added.

The meter caller at `0x56506` did expose a conversion mismatch. Its
`CMP.W D6,D7` followed by `BPL` tests the sign bit of wrapped `D7-D6`; it is
not an unsigned maximum. `calculate_menu_hud_row_meter()` now preserves that
predicate, with a high-bit availability regression case. The service/variant
timers at `0x56434`/`0x56482` also have explicit wrapped-result tests: their
`BGT`/`BPL` behavior is retained for a zero byte minus `0xFF`.
