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
| `0x073A` | `wait_for_vdp_ready` | Delay loop followed by a VDP status-bit wait. | Native frame/presentation boundary; no host busy loop | pending |
| `0x075E` | `run_game_loop` | Main coordinator: initializes session state, dispatches boot/title/field work, samples input, and performs indirect state dispatch. | `NativePortShell::update()` | partial |
| `0x0BD2` | `wait_for_vblank` | Advances the frame counter and waits for the interrupt-side sample to change. | `NativePortShell::update()` frame tick | pending |
| `0x0BF2` | `latch_controller_events` | Combines the sampled high-byte event words and latches activity and Start state. | `ghidra_conversion::latch_controller_events()`; host input delivery remains separate | partial |
| `0x0D44` | `advance_source_prng` | Advances the source 32-bit linear-congruential state with multiplier `0x20E5` and increment `1`. | `ghidra_conversion::advance_source_prng()` | partial |
| `0x0C2C` | `clear_runtime_memory_and_vdp` | Clears or folds work RAM based on the startup sentinel, resets display state, and invokes low-level setup. | Native session initialization; bus transcription remains in `ShadowrunRuntime` | partial |
| `0x1088` | `step_palette_toward_black` | Moves each of the 64 CRAM words one Genesis color component toward black. | `ghidra_conversion::palette_toward_black()` | partial |
| `0x10D6` | `step_palette_toward_target` | Moves each of the 64 CRAM words one component toward the target palette. | `ghidra_conversion::palette_toward_target()` | partial |
| `0x131C` | `step_palette_transition` | Runs seventeen palette/update steps, each synchronized to frame/input service. | `ghidra_conversion::run_palette_transition()` | partial |
| `0x1342` | `wait_for_palette_frame` | Waits for two vblank observations and latches controller events between palette steps. | Explicit conversion boundary; no runtime owner yet | pending |
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
| `0x5FDC` | `advance_entity_animation_accumulator` | Adds one animation increment, advances one frame on a period crossing, and marks the entity for redraw. | `ghidra_conversion::advance_entity_animation_accumulator()` | partial |
| `0x195C` | `sample_controller_input` | Samples controller words, derives newly asserted bits, and updates decoded directional state. | `ghidra_conversion::derive_controller_sample()` | partial |
| `0x19F2` | `read_controller_port` | Selects the controller port, reads both phases, combines the six-button byte, and restores the Z80 bus. | Windows input adapter boundary | pending |
| `0x1BAC` | `finalize_title_input_sequence` | Recognizes the seven-byte title input sequence or clears the bounded history. | `ghidra_conversion::finalize_title_input_sequence()` | partial |
| `0x1BEA` | `record_title_input_edge` | Stores up to eight nonzero title-loop edge bytes in the source event buffer. | `ghidra_conversion::record_title_input_edge()` | partial |
| `0x1C1E` | `advance_title_text_state` | Counts down the completed-logo text delay, steps the target palette every 32 counts, then emits the completed title text setup. | `ghidra_conversion::advance_title_text_state()`; palette/copyright presentation remain boundaries | partial |
| `0x1C6A` | `advance_title_sprite_state` | Advances the title sprite cadence/phase and requests a SAT projection rebuild. | `ghidra_conversion::advance_title_sprite_state()`; sprite projection remains a presentation boundary | partial |
| `0x1D7A` | `setup_title_state` | Clears title counters, loads title resources, and initializes title palette/text state. | Explicit conversion boundary; no runtime owner yet | pending |
| `0x1DCA` | `draw_title_prompt_variant_a` | Dispatches one register-selected title prompt/text record during the blink schedule. | Explicit conversion boundary; source text writer unresolved | pending |
| `0x1DE0` | `draw_title_prompt_variant_b` | Dispatches the alternate register-selected title prompt/text record during the blink schedule. | Explicit conversion boundary; source text writer unresolved | pending |
| `0x1DF6` | `draw_title_copyright` | Dispatches the four source copyright text records. | Explicit conversion boundary; source text writer unresolved | pending |
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
