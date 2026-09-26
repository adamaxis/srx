# Shadowrun Genesis decompilation scratchpad

## Current handoff

Latest handoff, 2026-09-26: the canonical Ghidra pass reports 910 functions,
2,986 resolved direct call sites, and 16 unresolved/indirect call sites. Every
exported function now has a reviewed semantic name and every exported address
is represented in `docs/decompiled-function-status.tsv`; nine additional
non-function ROM anchors remain in that ledger because native screen audits
refer to them. The Ghidra export is still reference evidence only.

The remaining-entry conversion pass added typed source-analysis contracts in
`ghidra_conversion/` for field/resource/camera work, the world/entity frame
service, effect lifetimes, and the remaining subsystem families. This is not a
runtime dependency and does not change the emulator/native path. Conversion
tests currently pass 225/225. The next native work is still hill-based: the
ledger contains 936 partial and four defended rows; none of those counts means
that the native game is playable.

The architecture target is recorded in
`docs/architecture-map.md`: one native session/controller/state graph, field
world, entities/effects, menus, intro choreography, renderer, audio service,
and persistence, with extraction and Ghidra kept outside the runtime.

Input: `C:\Github\shadowrun\Shadow Run (U) [!].bin`

Working copy: `input/shadowrun_genesis_us.bin`

Analysis: standalone C++17 direct analyzer, 68000 big-endian, image base
`0x000000`. The retained Ghidra project is a cross-check, not the port.

The initial direct-call result was 332 discovered function entries and 6,342
decoded instructions. After manually seeding the four reviewed fixed dispatch
tables and correcting immediate bit-op widths, the regenerated result is 717
entries and 18,803 instructions. The
important limitation is intentional: this still is not a claim that every
runtime callback or indirect dispatch target has been recovered.

The C++ translation currently has typed boot, runtime, session, palette,
object-list, and packed-data layers. `GenesisMemoryBus` supplies a headless
ROM/RAM/I/O address space and records VDP writes; none of these layers chooses
a renderer.

`decode_packed_stream` is the first pure data subsystem, translated from ROM
`0x1610`. Its output-count header and DBF-derived token lengths are covered by
literal, repeat, and overlapping back-reference tests.

`ShadowrunRuntime::random_below` now covers ROM `0x0D3A`: advance the 16-bit
state with `0x20E5 + 1`, multiply by the caller's word bound, and return the
product high word.

ROM `0x19F2` is a two-phase controller probe. It requests/releases the Z80
bus through `A11100`, drives `A10009=0x40`, samples `A10003` before and after
the second phase, combines the masked bytes, inverts the result, and stores it
at `FFEF0E`; this is modeled by `initialise_controller_ports`.

`0x195C` repeats the handshake for the second port (`A10005/A1000B`, stored at
`FFEF0F`), writes the current low bytes to `FFDD20/FFDD24`, forms rising-edge
words at `FFDD22/FFDD26`, and indexes the verified `0x19D2` decode words into
`FFDD04/FFDD06`.

The raw `0x1910` interrupt target writes `C05E0000`, increments `FFF9F0`,
selects `FFD8BC`/`FFD8DC` by parity, polls VDP status bit 2, and transfers two
words. The companion `0x0EC0` loop transfers 64 words from `FFD87E`; `0x0F1C`
and `0x0F3C` are the two smaller flag-driven VDP write paths.

The setup cluster at `0x1056`, `0x0FFE`, and `0x102A` is now represented by
direct VDP stream methods. `0x1056` emits sixteen zero words after the
`0x0E9C(FFE0)` command sequence; each larger routine then emits 1024
longwords of `0x07FF07FF` after selecting its `FFDD16` or `FFDD14` command
word. The shared `0x0E9C` translation preserves both masked control writes,
not merely the abbreviated value shown by the decompiler.

The VBlank scheduler at `0x1872` increments `FFF9EC`, clears `FFF9F0`,
consumes bits in `FFEF14`, and conditionally services `FFDFE8`, `FFE100`,
`FFE102`, `FFDF2A`, and `FFDF30`. Reviewed `0x0EE6`, `0x0EC0`, `0x0F1C`,
`0x0F3C`, `0x5672C`, `0x2C92`, `0x2CB2`, `0x6148`, and `0x61F4` branches
run directly from `service_vblank`; `VBlankRoutineHooks` remain injectable
observation/replacement boundaries.

The `0x0EE6` object-list upload now runs as `service_object_list_upload`.
It preserves the `FFDD02` threshold clear, the `FFFF` count sentinel into
`0x0E34`, and the `(count + 1) << 3` byte length passed to `0x0F98`.

The former `0x5672C` VBlank hook now has a direct
`service_vblank_state_animation` implementation. Its state/mode dispatch,
timer/index wrap behavior, ROM animation-table bases, and direct VDP command
words are preserved; the hook remains as an observation override.

The 64-word palette loops at ROM `0x1088` and `0x10D6` are now translated as
`PaletteAnimator`; the three Genesis color intensity fields are stepped without
introducing a renderer dependency.

The surrounding ROM `0x12F6`/`0x131C` transitions are also translated. They
perform seventeen palette steps, call the two-pass `0x1342` frame/input wait
between steps, and set VBlank bit 0 after each step. The frame wait is an
injectable boundary for headless execution; the input poll remains direct.
`service_frame_input` keeps the original DBF counter rather than hard-coding
the two passes into the palette code.

The `0x4618` entity phase now runs directly when its frame hook is absent. It
sets `FFDE88`, walks sixteen `FF0100` records at `0x100` spacing, tests byte
`+0x10` bit 0, calls the reviewed `0x5A56` projection, and executes `0x5FDC`.
The direct `0x600E` follow-up indexes the ROM pointer table at `0x62AC`,
preserves its negative/nonnegative state-index branches, chooses the `0x10`
or `0x1C` timing multiplier, and stores the selected pointer at `record+0x12`.
The normal nested `0x60FA` call now directly renders the `0x3EFE` mask, decodes
the `0x3E86` packed sprite, and queues the record for VBlank; its hook remains
an optional override. The reviewed `0x4658` orchestration and direct object
emitters now run by default.

The `0x5D29C` post-object state machine is direct through its four ROM pointer
tables, offset table, timers, uploads, background queue, and `0x5D416` command
cases. Its `0x15A8C` six-record allocator, `0x1A2CC` sprite-slot allocator,
`0x15930` fixed table dispatch, and `0x5D6BC` template spawn are bus-backed;
only a non-verified indirect handler or the known `0xAFF2 -> 0xB330` path is
reported as unresolved.

The `0x5B1E` helper called by the `0x4658` post-pass is now direct as
`sort_entity_pointer_table`. It performs the fifteen-adjacent-entry passes
over `FFDE8A`, uses signed record word `+4`, treats `+0x10` bits 4 and 6 as
zero-valued entries, and repeats until a pass makes no swap. The remaining
The remaining `0x4658` state dispatch/output boundaries are now explicit
hooks while the reviewed sort, timer, variant, cycle, animation, list, and
generation paths run directly.

The `0x622C`/`0x624C` callee is now direct as
`append_entity_state_objects`. It walks the state pointer at record `+0x12`,
handles the leading `0x8002` alternate-list marker and its `+0x20` adjustment,
emits the eight-byte object records, and updates the shared object count and
output pointer fields.

The `0xAEBA` callee is now direct as `generate_object_list_frame`. It advances
the `FFDFEC` progress gate, selects the byte-checked list pointer at `0xAF3E`,
computes the 32 high-word fixed-point offsets from `0x15E16`, and feeds each
row through the reviewed `0x1378` builder with `0x6000` flags.

The small `0x4E38` and `0x4E64` calls inside `0x4658` are also direct. They
preserve the `FFDF88` six-bit phase tests, timer-byte underflow, guarded
`+0x9E` branch, and the entity `+0x11` bit-7/bit-5 flag writes.

The larger `0x5C7E`/`0x5D0A` variant emitter is now direct as
`emit_entity_variant_objects`. It preserves the `FFFC04` control-block flag
sets, the four table families at `0x5D92`/`0x5DBE`/`0x5DA8`/`0x5DD4`, the
`+0xA6` gate, and the one-/four-list coordinate offsets into `0x1378`.

The `0x4E86` cycle/descriptor path is now direct as
`emit_entity_cycle_objects`. It preserves the timer/phase underflow branch,
the `0x4F42` descriptor lookup, the `0x61A0` queue append, and the following
`0x13BE` list emission with the signed `0x4F22` offsets.

The `0x481E` post-pass is now direct as `emit_entity_slot_objects`. It walks
the ten compact `FF164C` slots, applies the direct `0x5A62` projection, keeps
the signed/type-specific source-list branches around `0x48B2`, and emits
visible slots through `0x1378`.

The `0x5936` and `0x59EA` phase-list callees are direct as well. They update
the byte timer/phase fields, select the four pointer entries at `0x5A46`, and
preserve the single-list versus paired coordinate offsets before calling
`0x1378`.

The `0x5662` emitter and its `0x557E` countdown gate are direct. The typed
path preserves the phase-five cutoff, the `0x5756` descriptor pointers, the
optional `0x61A0` queue append, the phase-four random adjustment, and the
`0x13BE` list emission.

The self-contained `0x5494` emitter is direct as
`emit_entity_random_objects_0x5494`. It preserves the `FFE004` phase gate,
PRNG-derived `FFDFFE/FFE000` offsets, the `0x585E/0x586A` table choice, and
the optional `0x61A0` plus `0x13BE` output sequence. The enclosing `0x53C4`
scan/dispatch remains an explicit boundary.

The normal path of `0x56DE` is direct as
`emit_entity_state_objects_0x56DE`. It shares the `0x5756` descriptor table
with `0x5662`, uses the distinct phase-16 cutoff, clears the indexed object
byte through the reviewed `0x1A304` helper, and enters the direct `0x55C2C`
auxiliary state-grid update.

The compact state subsystem at `0x3674`, `0x37EC`, and `0x55C2C` is now typed
in `src/entity_state.cpp`. The implementation preserves the signed byte
phase/clamp rules, the `0x6AB3E`/`0x6AB96`/`0x6ABEE` source tables, the
`0xF1C4`/`0xF5C4`/`0xF9C4` and `0xF3C6`/`0xF7C6`/`0xFBC6` commands, the
`0x1232` DBF counts, the `+0x10` BSET mask, and the `+0x3A` constant write.

The calculation leaves used by the field entity target/action path are now
isolated in `ghidra_conversion/entity_action_math_conversion`. `0x55F02`
starts with record `+0x80`, scans possession slots `6..0`, and adds only the
first matching item-code `0x18` phase. Both this lookup and `0x104F6` scan
only possession slots 6 down to 0; slot 7 is reserved for the separate
special-item branch and is not a phase-scan candidate. `0x55F20` applies the record `+0x4D`
posture as a 25-percent-per-step factor and selects either the reduction or
remaining component from the source D7 sign. `0x104F6` performs the same
descending slot scan for a caller-supplied item code and adds the first
matching phase, then checks slot 7 for item codes `0x29..0x2B`. Its decoded
`C8AE0` action selector chooses one of the `D4540` row words at `+0x22`,
`+0x24`, or `+0x26`; a nonpositive word result becomes one. The conversion
passes this table as typed data rather than reaching into ROM memory.

The former Ghidra name for `0x1033E` ("first aid item use") is wrong: the ROM
reaches it from the field entity target-heading/attack path when the entity
has no equipped weapon. Its exact pure callees are now described as
`add_wound_difficulty_penalty` (`0x0D5A`),
`apply_target_condition_modifier` (`0x55740`), and
`calculate_action_residual` (`0x10660`). The wound leaf adds two points for
meters `0..4`, one for `5..7`, and zero for `8..10`, independently for
physical and mental state. The target-condition leaf preserves the source
cyberware-bit, target-class `0x16`, race/state, and target-flag-bit-7 branch
order. `0x10660` uses the selected `0xD4EA0` action row and decoded
`0x166AD4` effect value, applies the source rounded half-threshold and
20-percent scaling, then returns the residual effect. The RNG, entity-effect
spawn (`0x15830`), special result publication, and phase-consumption tail
remain explicit boundaries for the next `0x1033E` pass.

The next `0x1033E` caller layer is now typed too. `0x1047E` is not an
animation renderer in this path: it scans the three party records, admits
only active class `0`/`1` records, and contributes `floor(item18_value / 4)`
plus one when the remainder is at least three. `0x55786` gives bit 0 of
actor `+0xDF` priority over bit 1, selects the source special-record pair
(`+0x1E/+0x1F` or `+0x20/+0x21`), chooses actor `+0x77` versus `+0x7C` from
the signed action-row `+0x27`, performs the thresholded roll, subtracts its
successes from the special roll budget, and halves the remainder only when
actor bit 1 is set. The `0x55828` pointer walk is represented as a typed
special-record input instead of a RAM-address lookup.

The positive residual tail at `0x1033E` is represented by
`consume_item13_residual`: it scans item code `0x13` phases from slot `6`
down to `0`, keeps a partially consumed phase, clears a phase and its item
code at zero, and reports inventory exhaustion without inventing a new item.
This is the exact state mutation before the source `0x37EC` behavior-meter
service. `resolve_entity_target_action` now orchestrates the source gate,
initial byte-sized roll count, `0x55740` then `0x55786` then `0x0D5A` order,
`0x0DBA` result publication, `0x15830` effect boundary, the special action
result writes for action codes `6` and `0x0D`, the separate `0x10660` effect
roll count, and this phase tail. The remaining boundaries are intentionally
typed callbacks: RNG, effect allocation/audio, and `0x37EC` behavior-meter
state/display work. No ROM address lookup or emulator call was introduced.

The caller-side `0x55C76` transition is now directly executable as
`run_entity_reaction_0x55C76`. It preserves the three `0x0DBA` threshold
passes, the `0x55740`/`0x55786` timing calculations, the `0x55E62`/modifier
leaf calls, and the final `+0x4C` versus `+0x9C` update including the
`0xFFF0CE` rollover counters.

The self-contained scaling helper at `0xD81C`/`0x55F44` is now directly
translated as `calculate_entity_half_base_0xD81C` and
`scale_entity_value_0x55F44`. The typed result retains the byte-wrapped base,
the signed three-mode selection, odd rounding, and negative-input cap.

The normal `0x52BE` wrapper path is now covered through
`emit_entity_state_objects_0x5322`. It uses the `0x5876` descriptor table,
resets its timer to six on wrap, applies the phase-nine cutoff, and preserves
the phase/random Y adjustment. The wrapper's ten-record `0x52CC` scan now runs
directly after cutoff and copies eligible phase records.

The ordinary paths for `0x521E`, `0x508A`, `0x4FE2`, and `0x5152` now use a
shared typed timer/descriptor/list helper with their byte-checked table,
flag, reset, and cutoff constants preserved. The `0x4CDE` and `0x4D54`
wrappers call their respective leaves and now execute the direct `0x4CEC`/
`0x4D62` ten-record scans. Their `0x530DE -> 0x52F3C` tail now queues the
verified Z80 command directly through the bus; the nested `0xAFF2 -> 0xB330`
path is direct through its `0xABB6` checks and ROM tables.

The CPU-side Z80 transport is implemented in `src/z80_transport.cpp`. It
preserves the `FFFC0A` gate, `A11100` bus request, the `0xFF,0x10,argument`
packet emitted by `0x52F3C`, the separate `0x52E96`/`0x52EA2` primitives, and
modulo-`0x40` cursor updates in the `A01B40` shared ring. `0x53126` now uses
those primitives directly for its six stepped bytes, the three-pass
`0x0C24` frame/input waits, and its `0x16`/zero tail.

`dispatch_entity_state_objects_0x490A` now reads the 15-entry ROM handler
table through the Genesis bus. The no-op target `0x49A0` and the reviewed
`0x557E`/`0x56DE`/`0x52BE`/`0x521E`/`0x508A`/`0x4FE2`/`0x5152`/wrapper
handlers run directly on their reviewed paths; untyped targets return
`handled=false` so `0x4658` records the unresolved boundary.

The `0x53C4` state-D path now runs directly as
`run_entity_random_scan_0x53C4`. It preserves the FFDFF6/FFDFF8 gate,
the `0x5494` call, the nine-record `+0x100` scan, the `0x5468` child copy,
the ordinary `0xAFF2`/`0xB330` animation initializer, and the `0x5622`
cleanup. The fallback's fixed table path is now bus-backed.

The `0x1368` palette-loader loop is represented by `copy_target_from`; it
copies 32 longwords into `FFD8FE`, preserving the ROM's longword granularity.

The setup dispatcher at `0x1406` is now modeled by `run_resource_setup`. Its
A6 block is read as two longword source pointers, four VDP header words, a
12-byte primary list, and a following 14-byte secondary list. The primary
low-bit and secondary bit-16 dispatch selectors are preserved. The four
stream entry points remain injectable hooks, and `0x1232`, `0x1728`,
`0x0F5C`/`0x0F98`, and `0x1530` now have reviewed default implementations
described below.

The `0x1232` secondary-list branch is now executable as
`ShadowrunRuntime::stream_vdp_grid`: it emits the two-word `0x0E9C` command,
reads and offsets each source word, and repeats with the original inner and
outer DBF counters while advancing the command by `0x80` per row.

The `0x1728` branch is executable as `ShadowrunRuntime::stream_delta_vdp`.
Its first word is the signed-delta base; tokens `0xC1..0xFF` encode repeated
absolute values, while lower tokens select -1, 0, or +1 sequences and bit 5
selects byte versus word payloads. The translation preserves the original
DBF-style row width, signed output skip, optional `D2` mask, zero terminator,
and per-row `0x80` VDP command advance.

The primary `0x0F5C`/`0x0F98` path is executable as
`ShadowrunRuntime::stream_primary_vdp`. It uses the low word of `D2` as a
byte count, emits 64-byte/4-byte/2-byte chunks with the exact `0x0E9C`
command setup, and reproduces the source-boundary split by moving both the
source pointer and VDP command by `-D0.w`.

The odd primary selector enters `0x1530`, whose direct structure is now
recorded: a word-count header, MSB-first control bytes, literal bytes, repeat
runs, and 11-bit overlapping ring-buffer backreferences at `FF9700`. Its
lane bit selects low- versus high-byte VDP words. The implementation is
`ShadowrunRuntime::decode_compressed_vdp`; the hook remains available for
instrumentation or a future hardware-specific implementation.

The VBlank queue bodies at `0x6148` and `0x61F4` are now direct defaults as
`service_vblank_upload_queue` and `service_vblank_descriptor_queue`. They
walk the byte-wrapped 4-byte and 8-byte queue records, preserve the object
source selectors at `+0x11`, call the reviewed `0x0F5C` copier, and clear
`FFDF2A`/`FFDF30` after draining.

The two map upload calls at `0x2C92` and `0x2CB2` now use the direct
`stream_tile_plane` translation of `0x3D18`, `0x3D4A`, `0x3D80`, `0x3FD8`,
and `0x4000`. It preserves the camera-derived tile indices, the 31 DBF rows,
the 128-byte VDP command stride, the table-derived 32-byte tile offsets, and
the negative `FFDF58` XOR mode used by the second plane. The three previously
generic field helpers are now typed as well: `0x3FD8` splits X/Y into the
row-table byte offset and four-by-four tile byte offset using `FFDDEA` as the
row stride; `0x4000` composes the even display destination from the signed row
and masked column; and `0x41F0` performs the two possible eight-unit camera
subtractions while normalizing the row origin. These are pure coordinate
contracts, not VDP or ROM-memory accessors. The `0x3BAC` camera
tail is now direct as well: `0x15D3C`/`0x15D4E` read the direction and
correction tables, then changed axes call the `0x4018`/`0x40E2` edge refreshes,
whose `0x3D4A`/`0x3DDE` spans retain the VDP increment control writes.
The `0x40E2` vertical edge entry is now separately typed as
`render_field_tile_row_edge`: it applies the positive/negative camera-window
gate, prepares the `0x41F0` row origin, composes the `0x4000` destination, and
emits two named row requests with plane offsets and `0x0000`/`0x8000` palette
modes. The callback is the presentation boundary; the conversion does not
write a VDP register.

The record loops at ROM `0x1378` and `0x13BE` are translated as
`build_object_list`. They are the first typed map/entity boundary: four-word
sentinel-terminated inputs become eight-byte runtime records, with the two ROM
variants retaining their different word-3 arithmetic.
The state-aware overload also exposes the ROM's incremented object count and
post-record output position without pretending that a desktop pointer is a
Genesis address.

`build_object_list_from_bus` adds the actual Genesis-memory form of those
loops: it reads four-word records through A2, writes eight-byte records through
A3, and updates `FFDD02`, `FFDD18`, and `FFDD1C` after each emitted record.

The adjacent `0x0E34` setup is represented by `initialise_object_output`:
it derives the VDP command from `FFDD10`, clears the two longword staging
writes, points `FFDD18` at `FFD986`, sets `FFDD1C` to one, and clears `FFD97E`.

The `0x08C0` inner main-loop window is modeled by `run_frame_window`: it runs
31 DBF-equivalent passes of `0xA8EA`, `0x3A1C`, and `0x4618`, sets interrupt
bits 1/2/3, waits for the frame target, calls `0x12F6`, samples both controllers
twice, and clears the transition fields. The frame-window hooks remain
available for replacement or observation at subsystem boundaries.

The first direct call in `0xA8EA`, ROM `0x13874`, is now bus-backed. It clears
`FFE175`, selects the primary seven-record `FF0400` scan at frame phase `0x0B`
and the secondary three-record `FF0100` scan at phase `0x0F` when `FFDFE8` is
zero, or runs both scans when that gate is nonzero. Each list preserves the
raw `+0x10`, `+0xB0`, `+0x9E`, and `+0x0C` filters and writes its word count.

The rest of `0xA8EA` is now bus-backed in
`update_frame_entity_records_0xA8EA`. It walks all sixteen `FF0100` records,
decrements the signed `+0xDC` timer, preserves the `+0x16` transition ladder,
the `+0x9C`/`+0x4C` split, the `0xB0D4`/`0xB10C` special path, and the
68000 BSET/BCLR old-bit tests. The direct `0xBCAE` position correction and
the `+0x3A` indirect handler call are included. The remaining collision,
animation, and handler callees are represented by `FrameEntityUpdateHooks`
until their own bodies are typed.

The collision callee is now typed as `run_entity_collision_pass_0xEB92`. It
walks the following `FF0100` records through the raw `FF0B00` cutoff, uses the
`0x100` linked or `0x40` ordinary overlap radius, sets both `+0x11` collision
bits, and feeds colliding coordinates through `0x15D3C`. Its `0xECCC`
resource-side state changes, `0xEEFE` mode lookup, resource selection, and
`0x7000` random branch are bus-backed. The `0x1E2E0` and `0x1E2EC`
resource-loader leaves now have direct queue-write defaults; their hooks are
optional observation/replacement boundaries.

The `0xAB02` default path now calls the direct `0x16B98` entity-slot
allocator/weighted selector and the `0x1E2EC` queue leaf. The companion
`0x1DBF0` reset clears the global gate and the ten relationship records using
the raw `+0xA2`, `+0xAC`, `+0x43`, `+0xB0`, and `+0x11` fields.

The animation-expiry boundary `0x03404` is now direct. It preserves the three
`FF0100` records, the four-by-four primary table groups, the six-entry
state-selected secondary table, and the `0x03548` tail through the existing
bus-backed VDP stream.

The compact reaction leaves are now direct as well: `0x1DBAA` arms the
global gate timer, `0xBC36` selects and writes one of four three-pair offset
rows, `0x423C` composes the word-sized 12-bit map position, and
`0x1669C/0x166D0` plus `0x16760` perform directional tile indexing and
tile-map marking. The known `0xB7B8` state/resource pass is now typed in
`run_entity_reaction_0xB7B8`, including the re-entry bit, state resource
branches, tile-mark path, and transition writes. The `0x1651E` map/resource
body and its ten-slot `0x164F2` allocator now have direct defaults. Its
`0x53126` nested Z80 synchronization is direct as well and keeps only the
frame wait injectable for headless scheduling.

The following movement cluster is now typed as `run_entity_movement_0xBABC`.
Its C002 direction table, B77E/B732 candidate filtering, BB86 state-pointer
fallback, and BBE4/B442 record writes are direct. `0xE7C8` now directly
performs the FF1700 samples, bounds checks, E9EE rectangle test, and
EA36/EAD2 directional tests. Its EA36 stage remains optionally overrideable
for instrumentation through `EntityTerrainHooks`.

The `0x391A` body is now bus-backed as `run_entity_animation_pass_0x391A`.
It sets and clears `FFF0CC`, sends Z80 command `0x25` through the reviewed
`0x530DE` boundary, renders the `0x6AA7E` and `0x6AADE` strips with the raw
`D4 = type * 0x400 + 0x100 - 0xFBC` calculation, and preserves the successor
record loop and `0x1E540` reset (`FFDFDA/FFDFDC`) writes. The `0x3548`,
`0x36FC`, and large `0x38FA` presentation passes are now direct defaults. The
The relationship body is also typed as `update_entity_relationship_0x1DC4C`.
It takes the inherited D1 selector explicitly, clears prior links, preserves
the primary-record wrapping and eight-candidate DBF scan, and writes the raw
`+0x43`, `+0x11`, `+0xA2`, `+0xAC`, and `+0xA6` relationship fields. The
`0x391A` now supplies its recovered selector seven to the direct `0x1DC4C`
relationship routine; its hook is only an optional observation/replacement
boundary. The separate `0xAD14` caller still has a live inherited D1 boundary.
The `0x03404` hook is likewise only an optional observation/replacement
boundary over its direct default.

The `0x138B8` and `0x13904` animation entry points were checked against the
raw bytes and confirmed to be overlapping entries into one state machine, not
two independent display routines. Their timer/global-gate prologues are now
direct, as is the small `0x13A4C` type-0x0F branch. Later targets are emitted
through `EntityAnimationContinuationRoutine` with the original ROM address so
the remaining state-machine bodies stay explicit rather than being hidden in
a generic callback. The shared `0x13E8C/0x13E9E` branch now directly performs
the one-shot `+0xB0` bit guard and camera-relative `+0x96/+0x98` writes before
continuing at `0x145A2`. `0xAC84` uses these entry gates by default; its
known `0xAE5E` post-call effect remains in `0xA8EA`.

The next portion of that state machine is now direct through `0x13A92` and
`0x13B12`. `0x13CDE` uses the already reconstructed `0x55F02`/`0x55F20`
rate leaves, while `0x13C74`'s counted event output, type-B special argument,
and PRNG-selected accepted event remain bus-backed. The inherited D4/D5/D7
registers are explicit in `EntityAnimationRegisterContext`; `0x125FA` scales
the old accumulator, and the `0x13AE6`/`0x13466` path now reaches direct
`0x036FC` and `0x03404` display updates when the ROM's carry branch selects it.

The `0x142AE` continuation is direct through its `0x144E8`/`0x1460E`
close-distance gate. Its post-`0x15CA4` minor-axis register remains explicit
for the indexed `0x14300` probe, while the `0x1450E` and `0x145A2` tails retain
their original typed continuation targets. The packed `DIVU.W` increment in
`0x125FA` is modeled at its actual 32-bit intermediate value.

The `0x145A2` tail is direct through the `0x14C2E` direction flags,
`0x14BA6` sign/route decision, the `0x14682` direction-table/collision body,
and the `0x15D3C` transform terminal.

The route helpers `0x14902`, `0x14914/0x1494C`, and `0x14B4E` are now direct
bus-backed translations. `0x149C0` handles the FF1700 obstruction walks, the
inline path cursor, route-bit clearing, target emission, common B574
transform, and its direct `0x14A54` alternate route search.

The shared `0x13EF4` tail and `0x13F76` resource gate are direct as well.
`0x13EF4` preserves the signed animation-type branches, calls the direct
`0x15CA4` or `0x15D3C` reducers, and keeps the B45C/+0x2E terminal writes.
`0x13F76` preserves the FFDF84/FFE16F/record-flag gates, the FF13AE distance
cutoff, and the direct `0x1E2E0` queue leaf. The larger `0x14D8E` decision is
an explicit typed continuation with a mutable carry result.

The `0x13904` coordinator is now a typed source contract in
`entities/entity_animation_dispatch_conversion.*`. Its entry decrements the
record `+0xB6` timer and honors `FFE175`; the type-`0x0F` path uses the
`+0x16` cutoff and `FFE188` cadence before selecting a link at `0x14492`,
updating heading at `0x0AE6E`, and choosing the `0x0AF7C` or `0x0AFAE`
delayed-animation preparation. The ordinary path now records the exact
`+0xB0`/`+0x9E`/`+0x0C` route, transition, and effect gates, the stationary
type-`0x28` continuation, the `FFDF84` resource boundary, and the linked
`0x15D3C` measurement before entering the typed `0x13FFE` dispatch and
`0x13D0C` motion callbacks. The source-failure fall-through at `0x144FA`
and the two target-route tails remain named callbacks rather than hidden
memory emulation.

The `0xACAE` animation/reaction state machine is now direct through its
lock, transition, FFE18B/FFE18C, controller-input, phase, and terminal
branches. Calls whose carry or returned A1/D0 controls the next edge receive
an `EntityAnimationAcaeCallContext`; the recovered 0x1DBAA and 0x530DE calls
already execute directly when no replacement continuation is supplied. The
`0xABFC` delay call now has a direct default too: its nested `0xD7EA` selects
the first set state bit in the 0xC..0xE range, applies the `0xD81C` byte
average, preserves the mode-specific shift, and clamps the signed delay.

The pure `0x15CA4` distance leaf is now direct too. It preserves the high
word through the word-only subtractions, swaps complete D0/D1 longwords when
the second magnitude is larger, then uses the ASR carry to round the smaller
axis before the final ADD.W.

The pure register leaves at `0x3A90` and `0x3B1E` are also bus-backed. They
preserve the D3/D4 minus D6/D7 word subtraction, carry/borrow branch, reflected
thresholds at `3` and `0x180`, and mode-byte selections from `FFE173`/`FFE174`.

Their `0x15DCC` numeric dependency is now direct as well: the original D2 word
selects the signed X factor from `0x15E56` and the Y factor from its 96-entry
rotation at `0x15E16`, signed D3 is the magnitude, and the doubled high words
of the two `MULS` products are returned. A zero magnitude clears both result
words. The heading-offset conversion now uses the same source orientation:
`+0x4C` is the direct `0x15E56` sample and `+0x4E` is the rotated companion;
the prior host conversion had these axes reversed.

`object_update.cpp` now composes the reviewed `0x4ADC` flow: decrement the
record timer, use `0x1A304` plus `0x4B9A`-`0x4BAA` on expiry, otherwise run the
`0x5A56`/`0x5A62` selector and emit from the selected `0x4ACC` list using the
`0x4C0C` camera-offset pair.

ROM `0x61A0` is the companion append path used by `0x4ADC` when its five-way
object-list selection falls through. It copies descriptor bytes `+2..+7` to
the output pointer at `FFDF32`, writes source record word `+0x2A << 5` at the
output `+6` word, increments the low pointer byte at `FFDF35`, and increments
the output count at `FFDF30`; `append_descriptor_object` models those bus writes.

Seeded entry points:

| Address | Current label | Evidence |
| ---: | --- | --- |
| `0x00000200` | `BusErrorHandler` | vector 2; repeated `STOP` then jump to `0x1080` |
| `0x00000208` | `AddressErrorHandler` | vector 3; repeated `STOP` then jump to `0x1080` |
| `0x00000210` | `thunk_FUN_00001080` | vector 4 |
| `0x00000218` | `thunk_FUN_00001080` | vector 5 |
| `0x00000220` | `thunk_FUN_00001080` | vectors 6 onward in this image |
| `0x00000236` | `ResetEntry` | reset vector target |

## Early control flow

The direct analyzer confirms that `ResetEntry` performs region detection using the Genesis I/O area at
`0xA10001`, initializes the VDP through `0xC00000/0xC00004`, clears/copies
startup state, and branches into the code at `0x330`. It calls `0x436`, `0x73A`,
and `0x75E` during startup. The decompiler's pointer types around the reset
table are not reliable yet; preserve the assembly listing as the authority
until those tables are typed.

`FUN_0000075E` is the first strong candidate for the top-level game loop. It
initializes state, calls resource/setup routines, and repeatedly services
timing, input, rendering, and state transitions. This is a working hypothesis,
not a final name.

## Immediate investigation queue

- Confirm the actual Genesis exception-vector semantics around `0x200-0x228`.
- Mark/reset data tables referenced from `0x236-0x430` so they do not look like
  arbitrary pointers in C output.
- Identify the VBlank/HBlank handler and trace writes to `0xC00004`.
- Group functions by call graph from `ResetEntry` and `FUN_0000075E`.
- Identify text encoding and locate dialogue/menu tables without treating every
  printable run as text.
- Build a typed RAM map from reads/writes, starting with the `0xFFDDxx`,
  `0xFFDFxx`, `0xFFF9xx`, and `0xFFFCxx` clusters.
- Keep generated `ghidra-c/` output separate from manually reviewed C++.
- Keep fully converted and independently verified exports in the flat
  `ghidra-c/completed/` folder; the tracked completion manifest is
  `docs/completed-ghidra-functions.tsv`. Do not move merely renamed or
  partially tested entries there.
- Use `analysis/pointer_tables.tsv` and `analysis/direct_references.tsv` to
  promote only referenced tables into typed C++ data structures.
- `0x11950` now consumes the typed runner-record cyberware field: it ignores
  the separate bit-15 concern, enumerates installed bits 14..0 in source order,
  and preserves the panel/row/input choreography in
  `menu_service_sequence_conversion`.
- `0x12C74` is now audited against the ROM's raw-byte branch semantics: its
  action bands and candidate skill comparisons use the sign bit of wrapped
  `CMP.B` subtraction, and its final D6 merge preserves the wrapped `CMP.W`
  test and word addition. `0x12CEE` now has a separate typed conversion in
  `equipment_combat_bar_conversion`: it byte-wraps the `0x55ED6` command
  average, applies the `0x55F20` posture component and `0x12C74` skill merge,
  selects the source-mode action gate, applies `0x55E62`'s flag/item-`0x27`
  defense modifier, and maps values to the extracted 23-strip range. Its
  higher-mode negative-`+0x56` branch now calls the typed `0x12FA4`/`0x10660`
  effect calculation and reproduces the `+0x80`, `+0x7F`, `+0x9D`, and
  matching-phase attack value path. Rendering and source data-table lookup
  remain explicit host boundaries.
- `analysis/verified_tables.tsv` now records the byte-checked reset bootstrap,
  region selector/header field, and 19-word VDP configuration table anchors.
- `analysis/indirect_control_flow.tsv` records register-indirect `JSR/JMP`
  sites whose targets cannot be recovered from the instruction alone.
- Fixed indirect tables recovered so far are `12CCB4` (32 entries, `0x1F580`),
  `F3DC6` (22 entries, `0x1E69C`), `F3E1E` (ten primary/optional-secondary
  pairs, `0x214B0`/`0x214CE`), and `490A` (15 entity handlers, `0x46F6`).
  Their targets are seeded by the analyzer but remain table/index labeled until
  their state meanings are proven.
- The `0x4BB0` table is an eight-entry pointer table to signed five-word offset
  vectors; `0x4C0C` is a related eight-entry descriptor-pointer table. Both
  are data, not indirect function entries.
- The raw four-longword table at `0x4ACC` is
  `0x5802,0x5814,0x5826,0x5838`; `0x4ADC` indexes it with the low two bits of
  the record word at `+0x0E` and adds eight bytes before the object-list call.
- The `0x4B9A` loop reads five bytes from the record at `A0+0x34` and scatters
  them to `FF1700 + sign_extend(D0.W)`, where `D0.W` is the source base plus
  each selected five-word vector entry. This is modeled as
  `scatter_indexed_object_bytes`. The preceding `1A304` call is modeled as
`clear_indexed_object_byte`, which clears `FFD86C + sign_extend(D0.W)`.
ROM `0x1A310` then provides a separate full-table reset: it clears exactly 18
post-incremented bytes at `FFD86C`, modeled by `clear_indexed_object_state`.
- `0x5A62` is a typed pure entity-projection boundary: it converts record words
  at offsets `0`, `4`, and `8` using camera words at `FFDDC6`/`FFDDC8`, writes
  projected words at offsets `+0x0A`/`+0x0C`, and returns `1` only when the
  signed bounds `0x6F < X < 0x1AE` and `0x62 < Y < 0x17A` pass. The odd-looking
  bit-3 rounding and 16-bit wrap are preserved in C++.
- `0x5A56` compares the record longword at `+0x3A` with `0x15390`. Its
  alternate branch accepts `0 <= X < 0x200` and `0 <= Y < 0x1A0`, clears the
  projected Y on failure, and returns the projected X word; this is exposed as
  `project_alternate_entity_field` rather than folded into the normal routine.
- `0x481E` walks ten 18-byte slots at `FF164C` with stride `0x12`, filters on
  bit 0 of offset `+0x10`, calls `0x5A62`, selects the slot's offset `+0x02`
  source list, and emits it through `0x1378`.
- Treat `analysis/regions.tsv` as a byte-distribution aid only; region labels
  are hypotheses and do not replace control-flow or table-reference evidence.

## Guardrails

- Do not overwrite the original ROM.
- Do not call Ghidra's inferred C output "working source" until it compiles and
  its observable behavior is checked in an emulator.
- Treat data/code boundaries and inferred types as hypotheses until cross-checked
  by references, execution traces, or known Genesis formats.

## Recent compact-helper conversions

The early graphics helpers at `0x0EC0`, `0x0F1C`, and `0x0F3C` are now typed
host transfers. `0x0EC0` emits exactly 64 startup-palette words with the
source's per-entry control writes. `0x0F1C` builds the normal transfer command
and emits its two-word header, while `0x0F3C` emits the fixed
`0x40000010` incrementing-write command and its two words. `0x0FFE` and
`0x102A` share `clear_and_fill_vdp_plane`: the source first clears sixteen
words at the preceding address, then writes 1024 repeated longword values,
represented as 2048 typed `0x07FF` words. These routines use callbacks only
for presentation/output; no VDP register or ROM address is runtime state.

`0x164F2` is the entity-table capacity predicate hidden by Ghidra's condition
code output. It scans ten record-leading occupancy words and reports whether
at least one slot is free. `0x16E30` copies 0x31 scene-state words before
calling the two reset boundaries. `0x16FA8` copies a bounded rectangular
terrain region from the decoded source map into the work map with the source
row stride. Both are now ordinary typed C++ operations with focused tests.

The transfer library now also covers the shared `0x0F98` and `0x1232` leaves.
`write_vdp_word_block` emits the requested bounded even-byte word stream after
one transfer command; `write_vdp_grid` emits each row at the source `0x80`
byte stride and ORs the caller's tile mask into each word. `0x0E34` is a typed
upload-state reset that emits the source four-word zero clear and publishes
the stream cursor/enabled state. The terrain layer copy at `0x0313A` and the
thirteen-record readiness scan at `0x2FF0` are likewise isolated from runtime
memory and covered by focused tests.

The next transfer pass adds `0x0F5C` as an explicit source-64K split around
the `0x0F98` word writer, preserving destination advancement without carrying
the original pointer arithmetic into host code. `0x0CEC` now installs the
four-word startup transfer state and emits its fourteen control words, while
`0x0301E` is the fixed 0x140-byte scene-profile upload using the same typed
block primitive.

The CyberInfo handler at `0x18AA6` is now `run_cyber_info_screen`. Its host
conversion resets the two-entry page cursor, emits the `CyberInfo` setup, and
preserves the exact per-frame wait/latch/sample/history order. Up wins over
down when both events are latched; the three source activation bits render the
selected Deck Stats or Run Info page, while bit 15 redraws the Matrix panel.
The conversion uses typed page callbacks and a bounded input sequence for
tests, so it does not carry the original VDP writes or ROM text pointers into
runtime code. The existing Run Info classifier remains the source-backed
boundary for absent/active/complete runs and CPU/Datastore action labels.

The Matrix program-slot routine at `0x19238` is now named
`run_matrix_program_slot_screen`. The source's Yes/No loop enters with No
selected, services up/down through the shared cursor, gives the three action
bits priority over bit 15, and always redraws the Matrix panel on an action or
return. Yes applies `apply_menu_slot_command` only when the ready timer is
active; otherwise it clears the timer and refreshes the selected slot. No is a
non-mutating return. The host conversion keeps those branches as typed
callbacks and bounds only the test/input sequence.

The parent Matrix loop at `0x1822C` is now named
`run_matrix_interaction_loop`. Its native conversion preserves the per-frame
wait/latch/sample/history order, vertical-before-horizontal movement priority,
the row-3 route gates, and the action-before-bit-15 exit priority. Rows 0..2
dispatch to the typed program-slot screen; row 3 exposes the column action and
active-run disconnect handlers as explicit callbacks, and successful handler
completion follows the field-return boundary. The conversion does not emulate
VDP state or silently implement the indirect action routines.

## Entity animation target-route conversion: 0x145A2 / 0x149C0

The next movement slice is now represented by the isolated
`shadowrun_ghidra_entity_animation_route` library. It is reference/conversion
code only; it is not linked into the native runtime or the emulator.

The authoritative `export/disassembly.s` trace resolves `0x145A2` as the
continuation after the `0x144E8` position gate. The routine rejects a record
whose `+0x16` frame counter is above `0x3C`. Otherwise it first checks
`+0xB0` bit 1 for an already pending route. If that bit is clear, it clears
bit 2; a set bit 2 immediately rebuilds the `+0xB1` axis-relation byte through
`0x14C2E`. When the marker was clear and `+0xB8 != 0x1000`, it calls
`0x14BA6`, which may set the pending-route bit through `0x14682`; the routine
then enters the route queue if that bit became set, otherwise it derives the
axis flags and measures the current-to-`+0x96/+0x98` target. The common terminal
writes are `+0x34 = heading`, `+0xC8 = distance`, and `+0x3A = 0xB574`.

`0x149C0` is now consumed as a typed route-queue service rather than a ROM
address callback. Its direct disassembly tests `+0xB0` bit 3, checks the
bidirectional packed-grid path, clears bits 1 and 3 on a blocked path, then
uses the word cursor at `+0xBA` to consume `+0xBC` route entries. A matching
entry advances by two bytes; the `0xFFFF` sentinel takes the `0x14A36` branch.
Without bit 3 that branch clears route bits, rebuilds axis flags, and returns
through the common `0x15D3C` measurement. With bit 3 it enters `0x14A54`,
which probes the heading-relative obstruction strip, rotates through five
alternate directions using `0x14764`, and writes a fresh two-word route
(`candidate`, `0xFFFF`) at cursor zero. An exhausted search clears route bits.

The typed state keeps world coordinates, the separate `+0xB0` movement byte
and `+0xB1` axis byte, the signed movement mode, route cursor, route words,
extracted FF1700 obstruction data, metrics, and the B574 continuation. The
focused test covers the timer cutoff, direct measurement, marker/axis rebuild,
route cursor advance, blocked-path clearing, sentinel completion, successful
five-way replan, and exhausted replan. During this pass the older route helper
was corrected from the Ghidra C export's endian-confused `0x0800` word tests to
the actual disassembly's low-byte bits 1/3; its existing focused test was
updated accordingly.

The selector portion of that movement hill is now source-backed as well. The
direct `0x14682` trace sets `+0xB0` bit 1, leaves an already-retrying bit 3
alone, checks the target obstruction tile, then clears `+0xBA` and asks
`0x148AA` to scan from the target along the secondary dominant-axis direction.
The source D1 result is the last open tile before the first occupied tile, so
an open strip writes `target + step` followed by `0xFFFF`. When the first step
is occupied, `0x14854` now exposes the actual two open route tiles that the
source writes as `[primary, secondary, 0xFFFF]`; the old boolean helper remains
as a wrapper over that typed result. If neither side can bypass the
obstruction, the selector uses the record heading through `0x14AF4`, requests
the retry bit, and hands the resulting route to `0x149C0`.

The previous `0x148AA`/`0x14800` host translations had called their first
occupied candidate the return tile. The ROM instructions subtract one source
step before returning, which is why the conversion now names and tests the
last-open result explicitly. `0x145A2` invokes the selector after
`0x14BA6` requests a route, installs the typed one-/two-tile route, and then
services the queue. The focused coordinator test now covers this direct route
creation and the occupied-target cancellation in addition to the prebuilt
queue and replan paths.

## Entity spawn collision probe: 0x14D8E / 0x15ABA

The `0x14D8E` entry is now named `probe_entity_spawn_collision_window`. It is
not a generic entity routine and it is not an animation renderer. Its callers
use its carry result as a terrain/collision gate before continuing the linked
entity animation path.

The ROM sequence is compact and exact:

1. Set global `FFDFD8` to `-1` and preserve the incoming source record in
   `FFDD3C`.
2. Use `FF1380` as a temporary record and call `0x14FF8` with source identifier
   `-1`. The source marker is still active during initialization, so the
   `0x55640` preparation side effect is suppressed.
3. Seed `D7` with `0x1A`; the `DBF` makes this 27 attempts. Each pass calls
   `0x14F26`, which adds the record's signed `+0x4C/+0x4E` heading offsets
   shifted by eight to its current fixed-point position. The returned fixed
   coordinates are written back to the scratch record before the branch, so
   an unresolved pass advances the next projected probe.
4. If terrain reports a nonzero result while `FFDFE2` is clear, clear
   `FFDFD8` and return with carry set. Otherwise call `0x15ABA` with the
   swapped high words as the collision-search coordinates.
5. A carry from `0x15ABA` clears `FFDFD8` and is deliberately cleared before
   `0x14D8E` returns. Exhausting all 27 attempts has the same cleared-carry
   result.

`ghidra_conversion::probe_entity_spawn_collision_window` now represents that
flow with a typed scratch record, fixed-point probe position, explicit
terrain and collision callbacks, an explicit `FFDFE2` policy field, and an
ordinary result enum. It calls the existing typed
`initialize_entity_spawn_record` and `probe_entity_world_position`
conversions; it does not expose a ROM pointer, address array, or emulator
state. The focused test covers terrain carry, 27-attempt exhaustion, the
inner collision carry being cleared by the outer routine, and the shared typed
collision search boundary.

The related `0x15ABA` conversion now preserves its source carry separately
from `candidate_index`. The protected-record path and the source collision
mode have carry-setting exits even when no candidate position is copied; a
normal compatible candidate sets both the candidate and carry. This matters
to `0x14D8E`, whose outer result intentionally consumes that carry and returns
clear. The conversion test now checks those distinctions.

## Heading-vector leaf: 0x15DCC

The Ghidra C export for `0x15DCC` was a false no-op. The ROM disassembly is
the authority here: it copies D2 (heading) into D4, copies the signed D3 word
into D2 as the magnitude, and returns zero words immediately for magnitude
zero. Otherwise it selects an even byte offset from the heading high byte,
multiplies the signed `0x15E56` X sample and the 96-entry-rotated `0x15E16` Y
sample by the signed magnitude, doubles each 32-bit product, and swaps the
result so the product high words are returned in D2/D3.

`ghidra_conversion::transform_entity_heading_vector` now exposes that as a
typed coordinate leaf. It uses the verified table already shared by
`derive_entity_heading_offsets`, preserves the source's doubled-product
semantics, and does not emulate registers or address-space reads. The
coordinate test now covers the corrected axis orientation, cardinal heading
offsets, signed south projection, and the zero-magnitude branch. The
canonical export/callgraph labels and every recovered call site now use the
meaningful name instead of the generic routine placeholder.

## Entity animation fallback target: 0x14508

The `0x14508` entry is now named `prepare_entity_animation_fallback_target`.
It is the target constructor used when the dispatcher has no linked target,
not a menu or renderer routine. The source first chooses the linked record at
`A0+0xA2`, falling back to the player record at `FFFC04`. It reads the chosen
record's heading, then applies the source offset rule: subtract `0x6000` when
the current entity mode is zero, or when it is one and the reference mode is
zero; otherwise add `0x6000`.

The adjusted heading and fixed magnitude `0xE0` go through `0x15DCC`. The
returned vector is added to the reference position and sent to terrain
collision. A positive result causes a second probe with the vector negated. A
non-positive reverse result selects that reverse position; two positive
results select the reference position itself. Zero and negative terrain
results are accepted by the source and select the forward projection. The
chosen coordinates are written to record `+0x96/+0x98`, then the `0x144E8`
close-range boundary runs; its carry returns immediately and otherwise the
routine falls into the separate `0x145A2` obstruction continuation.

`ghidra_conversion::prepare_entity_animation_fallback_target` now models the
reference choice, 16-bit coordinate wrapping, signed heading offset,
forward/reverse terrain probes, target selection, and close-range carry as
typed host state and callbacks. It does not call the native runtime or create
a ROM-shaped terrain implementation. The focused test covers forward
projection, reverse selection after a blocked forward probe, reference fallback
with a close-range carry, and an unresolved callback boundary. `0x145A2` is
intentionally left as the next separate movement-route conversion hill.

The primary candidate builder at `0x1438C` now exposes the seven-record filter
that feeds the primary linked-target table. It requires behavior bit 0, rejects
behavior bits 6/4, route bit 7, status bits 5/3, and a zero collision/action
word, then appends the record identity in source order. This is the concrete
field-entity eligibility pass behind the later animation link selection, not a
generic list wrapper.

The entity animation cluster at `0x14492`/`0x13FFE`/`0x14226` is now named by
behavior instead of the old generic menu/entity placeholders. `0x14492`
selects a linked entity identity from the primary or alternate source table,
keeps an existing table member, clears an empty table, resets the service list
when the primary table is unavailable, and rearms the alternate animation
delay to six when it selects a new target. The table picker and service-record
reset are host boundaries; the conversion carries identities rather than ROM
pointers.

`0x13FFE` is the central per-entity animation/target dispatcher. It clears the
animation mode, refreshes the linked target, handles an unlinked fallback, and
then distinguishes the source phase paths: phase 4/11 holds a negative-frame
animation; phases 13..15 run the resource gate and alternate animation gate;
other active phases use the resource gate, global phase/route-bit spawn window,
and the entity-action cleanup path. A successful spawn-window pass applies the
linked heading; a carried pass enters cleanup, including the `0x14226` audio,
delay, pending-action decrement, and asset-meter sequence. The source
`0x12345678` return is a typed `source_failure_boundary` result, never a host
sentinel.

After cleanup or phase zero, the dispatcher calls the typed target-motion
boundary. A non-carry result enters animation mode 2 (waiting); a carry copies
the current position to the target destination and chooses mode 3/finish for a
nonnegative transition marker, mode 1/synchronized behavior for record flag
bit 6, or mode 1/configured target transition otherwise. Resource allocation,
collision probing, heading preparation, audio, and rendering remain callbacks
so this conversion module stays separate from both the emulator and the native
runtime.

The animation-transition cluster is now separated from the menu catalog. ROM
`0x138B8` is the entity animation timer entry: it decrements `+0xB6`, honors
the global frame lock at `FFE175`, rearms the timer to `0x14`, sends records
without both animation gates to the unresolved `0x14508` fallback, and only
enters `0x13A92` when the signed `+0xDC` transition marker is negative.

The isolated conversion for `0x13A92`, `0x13B12`, and `0x13BBA` now models the
actual event state machine. `0x13C74` retains event types `0/1/7/8` for the
bounded random choice and keeps the last type-`0x0B` argument as the special
event. `0x13CDE` compares the signed `+0x9D` argument and signed event code
through the explicit rate-calculation boundary, with the `+0x9C == 1`
bypass; `0x13CD4` enters the same gate with `(phase - 1)`. `0x13BBA` uses the
special-event/lane carry rules, and `0x13B12` prefers a rate-eligible special
event on an available lane before selecting an accepted event. A successful
selection publishes `+0x56 = -1`, `+0x57 = event + 1`, `+0x9D`, conditionally
refreshes `+0x4E`, then calls the delay-reset and asset-refresh boundaries in
source order. RNG, rate calculation, marker clearing, delay, and graphics
refresh remain callbacks; no Genesis bus or runtime emulator code was added.

The resource handoff at `0x13F76` is now isolated as a typed gate. It exits
when the shared entity gate is active, schedules the world-activity timeout
for the special reaction gate, rejects records with status bit zero or without
entity flag bit five, and preserves the `0x14D8E` carry boundary as an
explicit callback. The signed 16-bit distance from record `+0x2E` to the
global `0xFF13AE` reference is compared against `0x4000`; ordinary resource
mode marks status bit zero, while mode two rejects an object with bit seven,
starts the host resource-load boundary, and then marks the entity ready. No
source pointer or `0x20FEC` address is carried into the host conversion.

The linked-motion body at `0x13D0C` is now typed separately. Its active-phase
type-`0x28` early cutoff, animation-mode-2 linked-position copy, null-link
return, and `0x15D3C` direction/distance boundary are explicit. The type
flag selects the entity-type distance table; otherwise a nonzero delay is
decremented and the signed-frame phase row selects the phase distance table.
Distances below the lower bound and inside the lower/upper interval both use
the one-step route construction, but the middle interval clears the delay;
distances at or above the upper bound copy the linked position directly.
The route bit, terrain-gate finish, `0x15DCC` direction-plus-`0x4000` vector,
`0x144E8` carry, delay clear, `0x13E40` frame cutoff, and `0x145A2`
continuation are all represented by typed state/callback boundaries.

## Inventory command and selected-item conversion

The `0x11740` dispatcher is now represented in the isolated
`inventory_action_conversion` library. This is conversion/reference code only;
it is not included by the native runtime or emulator. The source command
selector is explicit: `0` and `1` are the left and right inventory columns,
`2` is Clips, `3` is Attributes/Skills, `4` is Cyberware, `5` is Magic, and
`6` is Pocket Secretary.

The selected-cell read is no longer a source-address label. `0x12148`'s
observable operation is `slot = 2 * row + column`, followed by the item and
phase bytes from the typed runner record. Empty cells return no selection.
The action loop preserves the source's two-confirm sequence: the first confirm
opens the four choices `Equip/Use`, `Discard`, `Trade`, and `Cancel`; a later
confirm dispatches the selected choice, while left/right wraps the selector
through source values `2..5` and cancel/start restores the parent cell.

The selected-item dispatcher now has source-derived branches rather than a
generic item handler. Medical codes `0x10..0x12` use the pending First Aid
path; `0x13`, `0x14`, and `0x16` use the automatic-use acknowledgement path;
other codes below `0x18` are weapon/gear equipment; `0x18..0x2B` are the
separate item-action path; and `0x2C+` are armor. Equipment mutation keeps
possession cells, weapon fields `+0x56/+0x57/+0x4E`, and armor `+0x76`
separate. The ammo conversion preserves the source old-factor/new-factor
quotient and half-up remainder behavior.

The same slice covers the guarded discard and trade mutations. Cell seven's
`0x29..0x2B` items reject discard, trade rejects the inclusive special-item
range `0x17..0x2B`, ordinary phase bytes travel with a traded item, armor
phase is cleared, and transferring live equipment clears the source's
equipped weapon or armor state. Focused tests cover the cell mapping, action
loop, branch boundaries, ammo scaling, armor toggle, protected discard, and
trade transfer.

The First Aid target-effect leaf at `0x1253E` is now isolated in
`first_aid_conversion`. It rejects a target whose physical and mental meters
are both ten, derives the source difficulty from target `+0x4C` and `+0x77`,
passes the active runner's `+0x89` roll count to an explicit `0x0DBA` service,
uses one point when the source success byte is zero, preserves the
`0x3674`/`0x37EC` meter-call order, clears status bits 6 and 4, sets bit 0,
and installs continuation `0xB45C`. The target model is source-field typed;
it does not carry a ROM address or a renderer callback.
The same module now covers the surrounding menu behavior: `0x12298`'s
active-record list plus `CANCEL` row, wrapped target cursor, the fixed `+2`
mental/physical patch branches at `0x1247E`/`0x12466`, and `0x124AE` phase
consumption. A medical phase reaches zero by clearing its item code; failed
or unavailable effects leave the item untouched.

The armor-selection leaf at 0x11352 is now isolated in
armor_selection_conversion. The source preview gate writes the selected
armor index to +0x76 and the selected inventory phase, runs the exact
text/effect/grid refresh order, and restores both values afterward. The
ordinary refresh rejects an active record with no equipped armor and keeps the
effect services as explicit callbacks.
The adjacent source leaves `0x55E92` and `0x55EB4` are now typed in the same
module: they select the one-based `+0x76` row's `D4540 +0x25` or `+0x26`
byte, subtract it from only the low byte of the running word, and preserve
the high byte. Zero armor remains an unchanged value.

## Clips command conversion

The `0x11B58` Clips command is now represented in the separate
`clips_menu_conversion` library. Its top-level source selectors remain
`2=Reload`, `3=Trade`, `4=Distribute`, and `5=Cancel`; the shared selector
helper wraps only that inclusive range.

The source `0x12062` gate is typed as a real state result: a zero `+0x4F`
clip count produces `out_of_ammo_clips` before any command-specific mutation.
Reload then rejects an absent equipped item or an equipped item code `>=0x0D`
as `item_does_not_use_clips`. When ammunition remains, the first activation
returns `confirmation_required`; the confirmed path decrements one clip and
adds the source `0x26` load increment, clamped to the extracted item capacity.

The nested trade table is intentionally not treated as the top-level table.
Within `0x11E7E..0x11F90`, selector `2` increases quantity, `3` decreases
quantity, `4` transfers clips, and `5` changes recipient. Quantity wraps
between one and the source's complete `+0x4F` count. Recipient construction
matches `0x11DD2`: active bit 0 is required, the source record is excluded,
and bit 1 excludes the record from clip trade.

`0x11FDE/0x12014` distribution is also typed separately. It requires at
least two active records, writes the quotient to each active record, and adds
the byte remainder only to records carrying source flag bit 1. All mutations
operate on `RunnerRecordFields`; no VDP, ROM address, or emulator state is
introduced. Tests cover the gates, reload confirmation/capacity, recipient
filter, nested selector meanings, quantity wrap, transfer, and distribution.

## 2026-09-25 audio duration correction

The first committed audio pass was wrong: the VGM trace window was allowed to
become the WAV length, producing eight-second music previews and four-second
title/effect files. The corrected offline pass remains separate from the
native runtime and now treats the trace window only as an observation limit.

The Z80 driver fetches event bytes through `0x041A`, with the active channel's
stream base in `0x0454/0x0455`. The extractor watches every channel's original
stream base, not just the first `0x1D80` stream, and records every qualifying
source restart after startup. A single channel restart is not automatically a
whole-track loop: successive boundaries were traced before a resource was
marked repeatable. `assets/audio/loop-points.tsv` now records the exact
exclusive PCM frame ranges for the eleven resources with two verified source
restarts; the intro remains outside the loop range.

Standalone one-shot resources are rendered through their final traced chip
event plus a two-second release tail. Music captures are rendered through the
complete observation window; the old first-restart crop was what caused
audible tracks to end early. The decoded PCM is packed losslessly as FLAC,
preserving exact sample positions without retaining temporary VGM files in
`assets/`.

Resources `005`, `014`, `015`, and `040` have internal or per-channel restarts
without a proven synchronized whole-track boundary. They are retained as
120-second observation captures and have no loop metadata. Resources `012`,
`013`, and `048` remain active throughout a ten-minute probe
without an observed stop or repeat boundary. They are retained as ten-minute
persistent streams rather than being silently reduced to short previews; the
native audio service will need to apply the source command's explicit lifetime
when those resources are wired to gameplay.

The native asset loader reads the loop-point sidecar into typed
`NativeAudioLoop` records. Audio without a record is deliberately not looped
by inference; gameplay must own its lifetime until its complete source
behavior is established.

## 2026-09-25 audio extraction pass

The complete ROM sound-resource pass is now separated from the native runtime.
The relevant 68000 setup at `0x531AA` queues the four initializer pointers
`0x284E4`, `0x29595`, `0x29609`, and `0x311BF` through the Z80 ring at `0x1B40`.
The table-2 resource list contains 94 slots. Title stages use the paired
command sequence `FF 12 id` followed by `FF 10 id`; treating a stage as a
single `FF 10` command loses the source driver's reset/play behavior.

The offline extractor in `tools/audio/extract_genesis_audio.py` boots the
uploaded sound driver, applies the Genesis serial bank-register rule at Z80
port `0x6000` (`bank = (bank >> 1) | ((value & 1) << 8)`), and records YM2612
and SN76489 writes. It writes temporary VGM, which is rendered to 44.1 kHz
16-bit stereo PCM and packed as lossless FLAC outside the repository asset
tree. This is extraction
evidence only; the game runtime has no Z80, VGM, ROM-address, or chip-driver
dependency.

The resulting direct asset catalog contains 65 non-silent FLACs under
`assets/audio/music`, `intro`, `menu`, `field`, and `sound-effects`. The other
29 of the 94 table slots were verified silent/no-op under the traced command
sequence and are omitted rather than represented by empty files. Their stable
aliases are in `assets/resources.tsv`; native code resolves them through
`NativePortAssets::audio_path()`.

The mode-entry branch from `0x1ADE6` is no longer cataloged as a generic
graphics routine. `0x54662` is the mode-action loop reached when the alternate
runner presenter flag is clear: it initializes the selected compact mode
record's flags, performs two wrapped resource transfers plus ten grid writes
per redraw, applies the `0x54A8E` random-event gate, renders the two status
fields and grouped decimal data, handles directional/action/Start edges, and
hands Start to the `0x1822C` Matrix program loop. The native conversion is in
`mode_action_loop_conversion`, with typed input, record, redraw, event, and
Matrix callbacks. The indirect handler table indexed by record kind and cursor
remains an explicit unresolved callback; no fake generic action implementation
was inserted.

The adjacent leaves are now named by proven behavior: `0x54A28` is
`render_mode_status_summary`, `0x54A8E` is `advance_random_mode_event`,
`0x532A8` is `fill_mode_auxiliary_sentinel`, `0x549E4` is
`write_mode_repeated_grid_service`, and `0x549F6` is
`select_mode_display_status`. The last three were removed from the audio
conversion batch because their callers and effects are mode-presentation
operations, not audio services.

The field-mode selector leaves at `0x58B5E`, `0x58BB6`, `0x58C54`, `0x58CDE`,
and `0x58D32` are now converted in `field_mode_navigation_conversion`. The
normal and alternate panel redraws preserve their separate grid/text passes;
the primary cursor uses the source result-dependent bounds `0..3`, `0..5`, or
`0..7`; the panel cursor clamps to `0..1` and resets the primary cursor; and
the alternate path publishes the source sprite-list/display dirty markers.
The resource-table selection and sprite-service internals remain callbacks,
but the cursor mutations and presentation order are no longer generic intro
labels or raw global writes.

The runner-selection family is now native-shaped. `0x532BC` is
`initialize_runner_selection_screen`: it exposes the background asset setup,
header/animation preparation, and exact 10-by-14 grid request sequence.
`0x537FA` is `refresh_runner_selection_sprites`, which resolves a typed
selected-runner record, scales its two source origin bytes by `0x10`, and
rebuilds the sprite records with the `0x2000` animation seed.
`0x53704` is `run_runner_selection_screen`: its source edge-event precedence
is preserved for previous, confirm, next, and menu-return actions, while
roster movement, HUD redraw, assets, and input are native callbacks. The
conversion contains no Genesis bus or address arithmetic.

The former `audio_noop_hook_a/b` labels at `0x53200`/`0x53216` were verified
against the ROM and corrected. They index exactly sixteen immutable compact
runner-record primary/secondary pointer pairs at `0x15DB48` and return the
selected bank for the current scene/resource id. The native conversion now
uses a fixed-size `RunnerRecordTable` and returns named data aliases
(`RunnerRecordTableSelection`) instead of inheriting the old pointer/no-op
fiction. This is scene/runner data, not an audio playback asset; actual
Windows audio remains in the separate audio-service boundary.

The compact-record animation helpers are now named as well. `0x533EE` is
`initialize_runner_animation_grid`: it selects the primary record bank,
converts the selected record's +3/+4 coordinates into a 14-by-10 cell index,
marks the initial cell, and starts the walker. `0x53434` is
`expand_runner_animation_grid`; its four directional deltas, row-wrap checks,
reverse recursion guards, and marker publication are native. The source tile
transition predicate remains an explicit callback pending verified names for
the four ROM type tables, rather than being guessed or hidden in a generic
runtime decoder.

The `0x2679E` world-entry loop is now `run_world_entry_sequence`. It consumes
an extracted command/delay stream, preserves the three-cycle timing cadence,
handles signed origin corrections and delayed horizontal/vertical pairs, and
publishes each completed frame through dispatch, transfer, title-layer, and
world-sprite callbacks in source order. Malformed host streams return an
explicit invalid result instead of walking past their data.

The `0x26A00` display-transition routine is now `run_display_transition`.
It exposes the audio marker, initial asset/reset sequence, named masked
transfer targets, exact 14-step and 6-step loops, and the final 11/6/6/11
input-sampling tail as host choreography. The native path has no VDP address
arithmetic; extracted assets and transfer presentation are callbacks.

The `0x1FA78` mode-service selector is now `select_field_mode_service`. It
models the seven scene/submode routes, their menu mode/result fields, signed
alternate prompt variants, and the special `FFF112` service-result handshake
used by `(1,5)` and `(4,3)`. Default routes enter the intro-stage callback;
nonzero alternate-result routes perform service only, matching the source.

The `0x1FD80` fallback is now `finalize_field_mode_exit`. It is a typed
scene/submode policy rather than a pointer walk: it covers the scene 6/7/9
visit routes, mode/world/status flag mutations, the shared signed-message
variants, the scene 1 random effect branch, the runner-status reset boundary,
and the final mode-service callback. The original A0/A5 resource pointers are
represented by host-facing route/profile identifiers; the source addresses
remain provenance only in the Ghidra export.

The `0x20634` selector is now `select_mode_entry_resource`. It maps the
source's seven exact scene/submode cases into five typed host resource
profiles, preserves the scene-5 any-submode case, and intentionally leaves
the existing selection unchanged for all other routes.

The `0x26972` title refresh gate is now `refresh_title_sprite_records`. It
decrements the pending animation countdown, defers only while the source
low-two-bit render phase is above one, clears an expired pending marker, and
then invokes the existing host sprite-record builder.

The `0x266E4` world setup is now `initialize_world_tile_buffers`. It exposes
the selected resource record, the source's 14x8 versus 12x10 layout branch,
zeroed tile accumulators, unit tile steps, the copy/primary/secondary buffer
sequence, pending-buffer decrement, and reset world origin as typed native
state and callbacks.

`0x1CA12` is now `advance_world_effect_motion`, preserving the negative-
countdown finalizer, direction-dependent source-table callback, fixed Y
subtraction, shifted sprite placement, and menu-service boundary.

`0x1D048` is now `advance_world_effect_record`: it models the mixed world-
effect record's lifetime/frame timers, `0x15E56` easing lookup, fixed-point
motion, direction mirror, and three-phase animation counter. `0x1D0DE` is now
`resolve_world_effect_sprite`, selecting the three source-table families and
returning shifted host coordinates through an explicit asset callback.

`0x1CB6A` is now `spawn_world_effect_record`: its typed state captures the
normal/work-record choice, audio command boundary, active marker, source
offsets, destination coordinates, and 60-step fixed-point velocity setup.

`0x1CB20` is now `advance_world_effect_activation`, with explicit typed
branches for the secondary seed record, special `0x1C27C` setup, motion
initialization, spawn offsets, and the pending-transition Y adjustment.

`0x1CC24` is now `advance_persistent_world_effect`, including the actual
record tick, six-slot circular sprite work area, seven-build wrapped draw
pass, and completion/menu transition.

`0x1D280` is now `initialize_world_sequence`; its fixed sequence/source
tokens, row range, destination, and initial counters feed the already-typed
sentinel-bounded `0x1D2E6` advance routine.

`0x1E43C` is now `handle_world_activity_request`, preserving the request
reset, thirteen-record scan, availability marker, activity handler token, and
world-activity transition boundary.

`0x1E166` is now `trigger_world_effect_sequence`, with the source eligibility
predicates, 0x3FF timing gate, random threshold, and three world-event
outcomes represented as typed callbacks.

`0x1E302` is now `build_world_effect_edge_candidates`. It selects the edge
opposite the current world origin, aligns the two five-coordinate rings using
the source's 16-bit arithmetic, probes `0x1E41E` ten times, and retains only
coordinates whose compact world-tile lookup returns zero. The native boundary
uses a typed optional tile callback; it does not read ROM or Genesis RAM.

`0x1E69C` is now `initialize_world_effect_selection`. It clears the setup
marker and shared effect globals, selects one of eight records for the current
context, derives the second bounded roll from the record weight and
`FF018C`, publishes the handler/auxiliary/base/randomized fields, and crosses
the `F3DC6[FFFBED]` initializer boundary through a typed callback.

`0x133A6` is now `apply_world_effect_reward`, the command protocol used by the
nearby-effect resolver rather than a menu routine. A `0xCxxx` command adds its
low byte to the active runner's special-action counter with the source's
signed-byte `0x15` clamp; a `0x8xxx` command adds its low twelve bits to funds;
`0x64` and `0x65` set status bits 4 and 3; and every other positive command
inserts item code `command + 1` into the first empty one of eight slots. Item
commands at or above `0x0D` cross an explicit bound-5 random callback and store
the sample plus two as the phase. A full inventory returns the source `D7=-1`
sentinel without changing a slot. The native model exposes the runner record
and global state as typed fields, leaving the PRNG as a host boundary.

`0x1DF34` is now `resolve_nearby_world_effects`. It scans the ten projected
effect records with the source's strict +/-0x100 coordinate tests, passes the
record command to `0x133A6`, and only treats its `D7=0` result as a successful
interaction. Successful commands—including signed `0xCxxx` commands—service
audio command `0x24`, clear the active bit, optionally send `0x65` through the
effect-slot clear boundary, and return after the first consumed record. The
`D7=-1` full-inventory result leaves the record active and continues the scan.
The old conversion's fabricated probe-coordinate return and signed-command
skip are not source behavior.

`0x1DD94` is now `advance_world_frame_logic`. Its native coordinator preserves
the scene transition gate, activity/resource timer order, low-mode service
record reset versus target-selection branch, inactive-effect cleanup, terrain
and world-effect trigger ordering, and the final nearby-effect pass. Each
subsystem remains an explicit callback and no emulator state is involved.

`0x1DC4C` is now `select_world_entity_service_target`. It releases the
owner's previous selection, preserves the source's eight-record scan, uses
the normal active/unlocked/unselected/non-high-effect/action filters, and
keeps the active-only path for modes `0x0B`, `0x04`, and `0x0C`. A successful
selection publishes the source `0x8009` code and an explicit host target link.

The paired layer entries `0x1C3FA`/`0x1C506` are now separated into
`advance_title_sprite_layers` and `render_title_sprite_layers`. Their phase
timers, alternate source pointers, two base sprites, optional status overlays,
and late double-build gate are represented without retaining Genesis pointer
arithmetic in runtime code.

`0x1C866` is now `advance_title_effect_motion`: its four-tick frame cadence,
mode/position source-table boundary, twelve-frame completion, and correct
`0x1C288`/`0x1C28E` source reset are captured in a separate typed module.

`0x1BCE8` is now `advance_title_profile_motion`. It carries the typed profile
phase, movement countdown/easing boundary, moving X/Y state, secondary phase,
and source finalization flags into a separate module; both profile and moving
sprite data remain callback-resolved rather than becoming runtime ROM pointers.

`0x1C61C` is now `advance_title_sweep_animation`, with the 32-phase delay
table, signed paired X offsets, bit-3 suppression gate, and behavior/menu
finalizer represented as typed state and callbacks.

`0x1BCE8` is now `advance_title_profile_motion`. It carries the typed profile
phase, movement countdown/easing boundary, moving X/Y state, secondary phase,
and source finalization flags into a separate module; both profile and moving
sprite data remain callback-resolved rather than becoming runtime ROM pointers.

The runner animation timing leaves at `0x1A4EA` and `0x1A538` were corrected
against the ROM disassembly. They do not read the 0x100-byte live runner
record: both select `FF1100 + (FFF118 * 0x14)`, with `0x1A4EA` reading byte
`+0x10` for the signed animation-delta table and `0x1A538` testing byte `+0x0F`.
The conversion now models that compact current-record roster explicitly and
rejects an out-of-range host selection. The full 148-test suite remains green.

`0x1BA6E` is now converted as `advance_title_burst_animation`. Its source
four-frame countdown, terminal phase, `0x1D970` phase table, sprite key
`0x780`, shared title position, `0x6000` attributes, append/build order, and
completion-marker callback are represented in the title-world helper module.

The adjacent profile pair is now in `title_profile_transition_conversion`:
`0x1BEBA` is `advance_title_profile_transition`, and `0x1BF7A` is
`advance_profile_copy_timer`. The conversion preserves the signed countdown
thresholds, ROM `0x1C136`/`0x1BFFC` table boundaries, paired sprite placement,
profile-copy offset, finalization ordering, and dirty-bit publication.

`0x1BB6A` is now `advance_title_noise_effect` in its own module. Both source
branches are represented: the active flag branch's paired sprites and
completion callbacks, plus the inactive branch's `0x3C` arm and seven-pass
two-bit ROR table selection. The full 150-test suite remains green after this
baseline before this slice is rebuilt.
The selected Matrix program-action service at `0x19976` is now separate as `matrix_program_action_conversion`. It rejects empty/loading slots and active values `2..5`, requires overlay stage `0x0C`, preserves `FFDD22` bit-14/bit-12 action-mode priority, dispatches the selected program through an explicit unresolved table callback, and resets the overlay. The normal program-code mapping publishes `3`, `5`, `2`, or `4` exactly as the source branches. Selection `7` preserves the runner `+0x13` parity split, availability clear for even fields, HUD variant publication, and seed-mode `4`/`5` transition into the existing effect boundary.
The next Matrix/menu service batch is isolated in `matrix_menu_service_conversion`.
`0x19632` clears one loaded program slot, applies the source code-specific flag/status cleanup, stores the empty code and visual state, and submits the repeated-grid, grid, and row refreshes in source order.
`0x19788` models the unsigned overlay countdown wrap, the unconditional overlay-timer service, the signed five-slot progression delay, and the first-incomplete-slot grid/completion publication.
`0x1989A` normalizes the signed HUD variant, preserves the high-side completion fade/world-effect/wait sequence, and publishes audio only when the normalized variant differs from the source's last-published value. All three are reference-only typed C++ with callbacks for presentation, audio, and waits.
The runner-record helpers at `0x1A4EA` and `0x1A538` are now converted in the same record-access module. The first reads record `+0x10` through the complete 21-word signed delta table at ROM `0x1A50E..0x1A536`; the second tests record `+0x0F` as a nonzero active-effect marker. The table lookup returns an explicit no-result for an invalid host index rather than exposing source pointer arithmetic. The audit found that the earlier conversion had stopped after 16 words and would have lost indices `16..20`.

The adjacent title/world-effect setup pair at `0x1A818` and `0x1A908` is now
converted as `world_effect_mode_conversion`, still outside the native runtime.
`0x1A818` is the descriptor-transition starter: its `FFE0D4` completion gate
is explicit, it resets `FFE076`, marks `FFE082`, stages the six-frame loading
state when `FFE0F4` is clear, or reads the selected `FF163E+5` descriptor token,
writes the source `0xD280` behavior position, and attaches state `7`. `0x1A908`
now exposes mode selection, the fixed negative-mode descriptor at ROM `0x1AA10`,
the `0x45` exit sentinel, and the one-shot `0x47C0`/`0x4800` setup for modes 6
and 7. The descriptor pointer table at `0x1A9CE` remains a typed callback until
its records are independently named. The shared `FF163E` work record documents
the source overlap where `FF1643` is a phase in one path and a descriptor index
in the other; no source-address interpreter was added.

The three effect-update leaves immediately following that setup are now
converted as `world_effect_overlay_conversion`. `0x1AB6E` advances the primary
overlay, preserves the exact `FFE0B6 == 1` branch, applies the `0x6B00`
descriptor-transition cutoff, threshold/intro carry decision, nine-frame
cadence, and the `0xF1BDE` phase table. `0x1AC6A` does the corresponding
secondary overlay using `FFE0B8`, the `0xF1BCE` table, and the seven-frame/four-
phase cadence; its threshold carry is intentionally not a branch in the ROM.
`0x1AD42` advances the background overlay from `0xF1BBE`, clears `FFE0C2` when
the intro marker is clear, and uses the four-frame/four-phase cadence. Sprite
table reads and title-sprite construction are explicit callbacks, while the
conversion records the source keys (`0x7A8`, attributes `0xB9`/`0xD0`, and
position shifts) instead of reproducing ROM address arithmetic.

`0x1AA40` is now converted as `tick_world_effect_animation`. The typed tick
keeps the source's three-way split visible: descriptor loading/transition uses
the six-step `0xF1C46` table and resets the effect entity after phase six;
normal animation either uses the current runner timing delta or the fixed
descriptor-state cadence when `FFDF8C != 0 && FFE0BC == 0`; then it builds the
descriptor sprite and advances the primary, secondary, and background overlays
in source order. The source `0x694` sprite key, `FF163E` coordinates, and
`0x2000` build flags are typed, while runner timing and both sprite-table
lookups remain explicit host callbacks. This corrects the old Ghidra output's
unresolved D-register/A-register values without making those registers part of
the host runtime.

The compact title/world helpers are now converted in one
`title_world_helpers_conversion` module. `0x1B282` is the Matrix transition
counter: it reloads every `0x0C` frames, clears the selected program at stage 6,
and finishes at stage `0x0C` by refreshing the HUD and returning to mode
initialization. `0x1B364` advances the support animation and emits the four
phase-selected figure sprite records from ROM tables `0x1DAF4`, `0x1DB08`,
`0xEA490`, and `0xEA4A4`, preserving the `0x780`/`0x7C0` keys and source
placements. `0x1B45C` is the two-wave title stream update: 176 entries from
FFCB42 and 201 from FF CB40, PRNG-rotated sign bits, and the ROM `0x1B592`
limit table. Finalization clears the behavior-record bit, finishes menu code
six, resets the stream, and otherwise writes it with FFEF14 bits 2/3 set.
`0x1B7D0` is now consistently named as the existing typed
`initialize_behavior_record` leaf.

The program-slot branch at `0x19238` is now typed as real Matrix outcomes:
already loaded, no available slot, insufficient memory, or ready to install.
The source's two-entry Yes/No prompt is also converted; Yes reaches the
existing slot-command mutation boundary, while No returns to the panel.

The CyberInfo routine at `0x18AA6` is now named and split into its actual
screen semantics. The shared two-entry cursor selects Deck Stats or Run Info;
the Run Info branch at `0x18C28` classifies the run as absent, active, or
complete, chooses CPU versus Datastore, and resolves the proven action table
to Delete File, Upload File, Crash System, or Download File. Text placement
and deck-stat field rendering remain separate presentation boundaries.

The Matrix storage branch is no longer a generic catalog label. The source
`0x1954A` reads one of five loaded-slot usage bytes, and `0x1952C` loads one of
twelve availability flags before its `0x19538` arithmetic tail squares the
flag and applies its E2DF2 footprint. Normal flags are 0/1, but the new
`matrix_storage_conversion` module loops those exact fields and exposes
`calculate_matrix_storage_capacity`, which reproduces `0x19558`'s total-minus-
loaded-minus-installed calculation for the storage screen at `0x18D1A`.
It also exposes the `0x18F5E` reverse five-slot datafile walk as a bounded
`MatrixDataFileList`, retaining the signed type/presence field, usage byte,
source slot, and display row. Slot selection and text rendering remain
explicit boundaries.

The neighboring helpers were corrected from the ROM trace: `0x1957E` is the
loaded-program capacity check over the five `FFFBD8` program codes, `0x195A6`
is the footprint/divisor load timing calculation, and `0x195D8` writes the
selected program code, pending byte, ten-times step offset, and source refresh
boundaries. The former generic field-menu visual-slot conversion was removed;
its label was not supported by the source control flow.
The callable sub-entries remain separate: `0x195A6` selects the current
program before entering timing arithmetic at `0x195AA`, `0x19538` is the
footprint-multiplier tail of `0x1952C`, and `0x19C54` enters the shared slot
lookup at `0x19C5C`.

The adjacent Matrix utility routine at `0x19018` now has its real menu logic:
the shared two-entry cursor selects Show Map or Retreat, and the Retreat path
is redirected to the source combat warning when `FFDF84` is active. The map
transition is still a presentation boundary. The deeper `0x19180` path now
uses a typed current/previous node pair: negative previous-node state returns
the JACK OUT warning, while an action swaps the node ids and publishes the
retreat transition marker; bit 15 returns to Matrix.

The first executable portion of the Matrix loop at `0x1822C` is now separated
from the native runtime in `matrix_loop_conversion`. Its setup preserves the
source callback order for transfer preparation, Plane A clearing, the D9376
Matrix stream, mode/HUD rendering, and the initial panel redraw. The Matrix
cursor is a typed 4x4 state: the setup writes row 3 as a staging transfer and
then clears the live selection to row 0; source input bits 8/9 move row with
priority, bits 10/11 move column, both wrap from 0 through 3, and the source
highlight positions are exposed as `0x94 + row * 0x28` and
`0x8C + column * 0x20`.
Confirm/cancel/program actions remain intentionally unresolved callbacks; this
slice does not claim that the Matrix program loop is complete.

The adjacent `0x18A92` leaf is now converted as `matrix_program_index`.
The source clears D7, loads the row into the low byte, performs `MULU.W #4`,
and then performs `ADD.B` with the column. Normal cursor values produce the
0..15 Matrix program key, while the typed conversion preserves the full D7
result and the low-byte-only wrap for malformed/restored cursor values.

The `0x1882E` Matrix panel renderer is now typed as well. It writes the common
header, returns through the unavailable-message branch for locked keys 0..11,
and otherwise renders icon/detail resources followed by two description
passes. The first twelve keys also prepare and render runner-only statistics;
keys 12..15 bypass that path. Resource presentation remains a callback
boundary.

The inline action branch at `0x18408` is now typed as Matrix action state and
route selection. Source priority is special marker (`0xFF`), marker zero,
marker one, then exit; row 0..2 routes to the program-slot path, row 3 /
column 2 is gated by active `FFF0B0`, and row 3 / column 3 routes to the
pull-the-plug confirmation only when that gate is active. The indirect column
handlers are still unresolved and remain callbacks/boundaries rather than
invented implementations.

The `0x19238` Matrix program-storage scan is now typed. It checks all five
slot bytes for the selected program first, falls back to the first negative
free slot when the program is not loaded, and reports a full five-slot
table without fabricating a load or execute operation.

The related `0x19BAE` HUD-mode leaf is now a separate record conversion. It
exposes the source `+0x0F` eligibility and signed `+0x13` mode fields, uses
the `0x45` unavailable sentinel, and applies the alternate table interleave
only to mode codes 7..16. The larger `0x19B40` path now preserves its record
eligibility and FFDF84 branch: negative gate state uses the record mode, while
other states require a positive `0x17C3C` resolver result before the same table
lookup. That resolver remains an explicit source boundary.
The old identity label at `0x19BF6` was disproven by the instruction trace.
The routine preserves incoming D0, selects the current or mode-7..16 adjacent
record's `+0x11` value, and applies the signed FFF0A0 variant adjustment; both
outputs are now typed.

The adjacent `0x19C54` leaf follows the selected slot through its loaded
program code and reads the corresponding FFBCC availability byte. The apparent
`0x19C5C` entry is an interior instruction address in that same leaf, not a
second empty function; the conversion keeps one bounded lookup.

The Attributes/Skills screen is now separated into a typed record/layout
conversion. `0x0D82A` maps the twenty source label slots into two columns of
ten rows: attributes use live-record bytes `+0x77..+0x7E`, skills use
`+0x80..+0x85` and `+0x87..+0x8C`, and every cell carries its source label index,
record offset, and raw value. The `+0x9B >= 2` first-skill class gate and the
no-live-record gate remain explicit cell states instead of being guessed away.
`0x0D01C` cursor movement is represented as ordinary bounded host state: left
and right select columns `0`/`1`, while up and down clamp the row to `0..9`.
The mutation branch is now source-backed as well. `+0x8D` is the available
Karma byte; attribute limits are the five six-byte species rows at `0xC22A6`,
attributes 0..5 cost twice their next value, and non-upgradeable attributes
6..7 are rejected. The skill branch preserves the source class restriction,
the 2x skill-cost paths, and the left-column middle-skill 1.5x path that uses
the linked second-skill value when the selected value is zero. A confirmed
`0x0D3EA` upgrade subtracts the computed Karma and writes the selected value;
the isolated C++ conversion leaves the prompt/text callbacks outside the
record mutation. Source `+0x9A` is now exposed as `species_index` rather than
being confused with the record header byte at `+0x00`.

The Magic entry at `0xFBA6` now has a typed source gate. The ROM takes the
spell-panel path only when record `+0x9B < 2` and its magic/resource byte at
`+0x7F` is nonzero; the three archetype templates confirm Shaman=`1`/`6`,
Samurai=`2`/`0`, and Decker=`3`/`0`. The record model now also exposes the
big-endian spell-table selector at `+0xA0`. The `0xFEBE` column helper and
`0xFF06` row helper preserve the source count-dependent two-row bounds and
fallback-to-row-zero behavior. Spell-name/value rendering and the equipment
selection/use path remain explicit unresolved boundaries.

The next Magic leaf is no longer opaque bookkeeping. `0xFC68..0xFC7C` selects
the active runner's table from `FFF9FA` using the big-endian `+0xA0` index;
the table is a word count followed by two-byte entries laid out seven per
row. `0x1088C` uses each entry's high byte as the `0xD4EA0` spell-record code,
retains the low byte as the source force/value parameter, and treats signed
record byte `+0x25` as the availability gate. `magic_menu_conversion` now
returns that typed selection and distinguishes available from unavailable
spells. `0x100A8` now has a typed force clamp: the selected value stays in
the inclusive range `1..table_parameter`, and the result is published to
record `+0x9D`. The `0xFF5A` preparation boundary now publishes the source
spell item code (`spell_code + 1`), reloads the ordinary equipped-item value
only when the source had an active item, and records the selected force. The
target/effect application is not at `0x10016`; that entry is only the
force-input loop. The `0x10016` input boundary is now converted too: source bit `0x0800`
raises force, `0x0400` lowers it, and `0x0200`, `0x4000`, `0x1000`, or
`0x2000` finish the loop. All adjustments use the same inclusive clamp, so
the host state cannot diverge at the force edges; VDP redraws remain outside
this state conversion.

The Cyberware screen at `0x11950` is now a concrete menu model: a zero
`+0x90` bitfield selects the no-cyberware message, while a nonzero field scans
bits `0..14` into ascending visible rows. Bit 15 affects the source's empty
versus nonempty branch but has no row of its own. The text assets and common
menu-row input service remain separate.

The Pocket Secretary entry at `0xC2A6` is now separated from the native menu
shell as a source conversion. It resets the cursor, chooses five entries when
`+0x101` is zero or six when it is nonzero, gives previous-selection input
priority over next-selection input, wraps over the exact source maximum, and
dispatches action, mode-action, and exit edges in source order. The indirect
handler table at `0xC1400` remains an explicit boundary.

The runner-switch leaf at `0x391A` is now represented separately from the
native field shell. Its typed scan advances after the current record, wraps at
source record type `2`, requires the active bit, applies the `FFF09D`
suppression override, and publishes the transient selection marker. A scan
with no different eligible record returns a no-eligible result without
installing an archetype or changing the active runner.
The posture companion at `0x10722` is also now explicit: source `0x0800`
and `0x0400` adjust posture by `+1`/`-1` through the existing `0..4` clamp;
`0x0100`, `0x4000`, and `0x1000` finish the sub-loop, while other input is a
wait/no-op step. This captures the actual input ordering without importing
the source redraw/vblank service.

The Matrix entry at `0x1817E` is an inline branch of `0x18150`, not a missing
standalone Ghidra function. The ROM bytes now have a typed conversion that
preserves its exact shell: display preparation, copy of source block `0xE2E3A`,
dirty-bit publication, menu-resource pair upload, `FFFBCA=10`, entry into the
larger `0x1822C` Matrix loop, metadata refresh from `0xD29D6`, palette refresh,
field-asset upload, field-animation refresh, and the final mode return. The
actual deck/program loop remains a separate boundary and is not replaced by a
generic Matrix screen.

The separately exported `0x181F8` entry is named `run_matrix_session_entry`.
Its confirmed prelude is represented by `prepare_matrix_session_entry` in the
cyberspace entry module: source display commands, the `0xE2E3A` resource copy,
`FFEF14` dirty-bit publication, menu-resource upload, and `FFFBCA=10` slot
initialization are converted. The fall-through session loop and finalizer stay
explicit boundaries until their state is source-traced.

The leaf routines behind the terrain/scene boundary are concrete now.
`0x16EA0` fills a bounded work-map rectangle with terrain value `2`,
`0x16FA8` copies a bounded source rectangle using its row stride, and
`0x16D8A` returns an optional typed scene record with both coordinates offset
by `0x80`, the selected kind, and the active flag. Full-table or occupied-slot
cases return an empty result instead of exposing scene-record pointer math.

`0x1745E` is now represented by `world_mode_routine_conversion`, not a
generic world-family placeholder. Its typed state captures the copied resource
token, audio-generation reset, menu-delay setup, field-mode marker, per-pass
initialization, behavior range, and repeat-until-exit condition. The callback
sequence preserves the source order from plane clears through HUD/resource
services and the final field-asset refresh, while leaving each concrete host
service independently replaceable.

The menu-entry pair is concrete as well. `0x17A5E` now handles the negative
entry marker by merging pending flags and setting bit `0x40`; positive entries
advance their state, optionally refresh the HUD, and follow the recovered
title-text/decimal-glyph order based on the available count and variant flags.
`0x17C3C` is explicitly recorded as the empty intermediate service observed in
the source rather than being mislabeled as an unknown world routine.

The terrain batch wrapper `0x16ECA` now scans the same variable-span work
descriptors as the leaf routines, invokes the boundary predicate only for
clear status entries, copies the first accepted region, and returns an
explicit dirty transition. It no longer depends on the decompiler's hidden
condition-code output or on the original work-map addresses.

The save-to-field restore routine at `0x594E0` is now a concrete conversion in
`ghidra_conversion/save_field_restore_conversion`. It copies the `0x248`-byte
field payload from the save-work image's source offset `0x3D6`, reads the
signed primary/secondary roster markers from payload offsets `0x212`/`0x213`,
and invokes the existing roster-template conversions with marker-minus-one and
the source's cleared `D7` value (`0`). It calls the field-scene preparation
boundary before those template loads, restores the scene word, camera pair,
and three runner position pairs from the following `0x12`-byte tail, refreshes
the projected records, sets the two `0x0D` effect flags, and selects the
`0x720FA`/`0x7223A`/`0x7219A` field profile choice from `FF019B` as a typed
profile enum. A pending `FFFC3C` effect is exposed through the separate
world-effect callback. The conversion has no dependency on the emulator or
native runtime; only the two still-independent source leaves remain callbacks.

The world activity coordinator at `0x167AE` is now a separate typed
conversion. It resets the activity decision state, applies the `FFE186`
timeout scene override and `FFE162` descriptor-publish override in source
order, snapshots/prepares the field, and handles the `0x15..0x2B` resource
branch, `0x2C..0x37` scripted-activity branch, and `0x38+` stop boundary.
The field branch preserves the existing-activity early return, the
`FFFC2C` bit-26 timeout gate for scene `0x14`, the `FFFC1E` bit-19/bit-29
gates for scenes `0x0D`/`0x0F`, the scene-to-runner table lookup, and the
primary/secondary marker comparison before selecting a host script-table
entry. The resource branch preserves the PRNG-derived slot and all three
source override switches, then uses the typed scene cursor and upload
boundary. No ROM pointer table or Genesis RAM address is used as runtime
state.

The neighboring world/entity pair is now concrete in
`world_scene_entity_conversion`. `0x1651E` handles the scene-14 two-cell
write, the mode-2 three-cell horizontal or two-cell vertical effect writes,
the scene-4 special resource selection, and the mode-0 auxiliary entity
fields. `0x16B98` now applies the record-kind and world-mode gates, the
`FFFC40` scene-event bits, pending/occupied scene-record checks, `+0x80`
positioning, mode/scene weighted tables, and the `0x5A` equipped-item and
`0x5B` clear sentinels. The source's entity/map records are represented by
typed results and extracted weighted data rather than pointer identity or
Genesis RAM addresses.

The recruited roster data path is now explicit. ROM `0x72C1A` contains ten
named `0x100`-byte templates: Ricky, Winston Marrs, Trent, Petr Uvehr,
Walking Bear, Phantom, Two Fists, Freya, Rianna, and Stark. The next
zero-kind template is the table terminator. `read_genesis_recruited_roster` extracts the
complete reviewed record fields, including possession/equipment, attributes,
skills, cyberware, posture, ammunition, and source name. Native assets expose
these as `NativeRunnerTemplate` candidates while leaving the live three-slot
party unchanged; recruitment event writers still need their own ROM trace.

## Second 100-function conversion pass

The second 100-entry pass is complete in five committed slices. It remains
strictly under `decompilation/ghidra_conversion`; no emulator or native game
source was changed. The new modules cover field/entity rendering, menu and
HUD state, title/effect animation, world/resource helpers, and audio/service
boundaries. Indirect calls are represented as checked host callback vectors,
and address arithmetic is retained only as provenance constants or typed
stride/limit data.

The five implementation commits are `d091e4b4f3`, `0cc1db6407`,
`036f8f8971`, `add84ebe35`, and `d01230cab6`. Their focused tests bring the
conversion suite to 121 passing tests. The status ledger now contains 617 ROM
entries plus its header: 610 partial, 4 defended, 1 ported, and 2 pending.

The 100 new aliases are also registered in `tools/ghidra/ExportShadowrun.java`
so the next canonical export writes readable names into the sole Ghidra
output. The two source no-op audio hooks at `0x53200` and `0x53216` are kept
as explicitly named empty hooks rather than being confused with missing
analysis. None of these conversion slices are runtime wiring; a workable
native hill still requires a traced screen/system, typed engine ownership,
regression test, and CLI image dump.

The next five conversion slices cover one hundred additional ROM entries. The
display slice names `0x01070`/`0x01872` as the display-frame service, snapshots
the field scene/profile buffers at `0x03074`, and expresses the tile row/column
walk (`0x03D18`, `0x03D4A`, `0x03F9E`) through checked host vectors and callbacks.
The same slice converts the controller delay gate, entity collision dispatch,
title sprite wave, and the short title/menu presentation wrappers. It does not
copy VDP registers or ROM addresses into runtime state.

The menu-transfer slice covers the repeated-grid/resource wrappers from
`0x10156` through `0x127FC`, the resource variant threshold at `0x12A54`, the
palette fade at `0x16396`, and the packed-coordinate projections at `0x1669C`
and `0x166D0`. The menu-slot slice then converts slot-code cleanup, countdown
expiry, effect defaults, the five behavior descriptor initializers, and the
selection-record reset as typed state transitions.

The world-effect slice converts the bounded word copies and coordinate clamps,
the conditional/unconditional activity timers, effect candidate rings, tile
lookup, effect cleanup, dispatch, and bit publication. The indirect source
effect table remains an explicit six-handler boundary; it is not guessed or
recreated as an address-indexed runtime table.

The audio slice converts the controller/audio bus boundaries, bounded audio
workspace initialization, reset pulse, circular command queue writes, and the
source begin/flush/append/end sequences. The mode-entry cadence is six ordinary
host service/wait passes. This is conversion/reference code only; the native
game and emulator remain unchanged.

The following twenty-entry slice continues directly through the terrain and
menu/HUD service cluster. `0x16E5C` now walks typed variable-span terrain
descriptors, while `0x16F8E` resolves negative-mode regions before copying
bounded rows. `0x16F3C` maps a world delta into a typed terrain rectangle and
publishes the source marker; `0x16F14` keeps its regular and alternate checks
as explicit callback boundaries.

The menu/HUD conversion now covers the sixty-frame service loop at `0x177FC`,
the count-dependent text progression at `0x17998`, overlay countdown at
`0x17EEE`, and random visual-effect phases at `0x17FDA`. `0x186FA` and
`0x187AA` express field-HUD asset refresh and active-indicator grid order.
The remaining helpers are typed presentation and selection services:
`0x189B4`, `0x189D8`, `0x19432`, `0x194AC`, `0x19504`, `0x19558`, `0x1957E`,
`0x195A6`, `0x195AA`, and `0x196F2`. They use host state, bounded arrays, and
callbacks rather than source addresses or VDP emulation.

The next twenty-entry conversion slice stays on the reference side and does
not alter the native game or its emulator museum. `0x16B7C` now publishes the
scene resource cursor fields into typed state, while `0x16D8A` uses an
explicit slot-availability boundary before offsetting and activating a scene
record. The menu/HUD cluster is represented in
`ghidra_conversion/menu_hud_service_conversion`: `0x173F2` preserves the
temporary marker/preparation order, `0x177CC` submits its source stream twice,
and `0x1789C`/`0x178C8`/`0x1792E` express the recovered grid and row-service
orders without VDP address arithmetic.

The same module converts the signed-row branch at `0x17BC0`, its text leaf at
`0x17C52`, the four-phase sprite/audio pulse at `0x17CC8`, panel/footer offset
normalization at `0x17D90`/`0x17DB8`, and the three-grid readiness branch at
`0x17E04`. `0x17E78` now has a bounded stride-table lookup, `0x17EB4`
publishes the typed average-delay subtraction, and `0x17F80`/`0x17FB4` plus
`0x1897C`/`0x18998` are explicit service boundaries. Finally, `0x18776`
performs five two-vblank fade passes with typed dirty publication. The new
module has a focused test; the full conversion suite is now 110/110.

The title resource loader at `0x1CFC` is now a typed host sequence. It submits
the decoded-resource boundary three times, resets the host resource cursor
between the first and second submissions, and then prepares the title sprite
list. The conversion carries no ROM pointer or VDP state; each submission is
an asset-selection callback.

The `0x1D58` title sprite-list leaf is also represented by a typed callback.
This preserves the source boundary while leaving sprite extraction and native
scene choreography outside the decompilation helper.

The field resource wrapper at `0x28AA` now has the same treatment: setup,
asset-sequence upload, animation refresh, and font loading are named host
callbacks in source order. It does not reintroduce the original VDP upload
state machine.

The field viewport wrapper at `0x2C6C` now records the scene preparation,
mode-state refresh, conditional special-effects service, and both viewport
submissions as typed callbacks. The source's mode flag is represented as a
boolean host query rather than a raw global address.

The thin `0x33AA` field-grid wrapper is now a named host presentation
boundary. It invokes one grid draw callback and does not expose the source VDP
writer to the native conversion layer.

`0x10952` is now a typed mode-display reset. It clears both planes, waits for
the frame boundary, resets upload state and the two transfer markers, then
restores the fixed transfer setup. The markers are ordinary host state.

The established word-transfer module now also converts `0xC3B2` as
`write_single_word`: one typed transfer command followed by one data-word
callback, with the ordering covered by the existing transfer test.

The late-intro helper pass is now typed in `title_intro_late_conversion`.
`0x5A9E2`, `0x5AA1E`, `0x5ADFE`, and `0x5AE3A` are explicit text batches with
their source destinations and palette masks. The shared transition helpers
preserve the `-0x190` fill window, `-0x1B0` text trigger, eight-frame triplet
rotation, six-frame pulse descriptors, and seven-record landing projection.
The 15-word entity color fill/step operations are also represented as typed
records. The renderer, palette upload, and scene-record operations remain
callback boundaries; no ROM address or VDP register is required by the host
model.

The timed title-transition table is now isolated in
`title_intro_schedule_conversion`. `0x5AF2A` consumes typed delay/action
records and invokes the next action only after its countdown crosses below
zero; a newly loaded zero delay remains the source terminal marker. `0x5AF62`
is a native frame-loop wrapper with explicit display preparation, title-resource
setup, controller sampling/latching, stop boundary, and action callbacks.

The `0x5AD1A` late-stage coordinator is now `run_title_intro_stage_loop`.
Its native controller owns the source setup order, initializes the typed pulse
and landing state, performs the first animation/text pass, and runs exactly
`0x529` vblank iterations. The final text batch remains pinned to loop frame
`0x1E0`; scroll-fill, transition text, triplet rotation, and sprite drawing
are explicit callbacks rather than VDP or ROM-address operations.

The secondary-stage `0x5AA96` coordinator is now
`run_title_intro_orchestrator`. It consumes named host assets for the two
decoded graphics streams, palette, text, and HUD sprite stream; emits the
typed 20-word by 13-row fill plan; preserves the palette-fade frame restore;
and services the stage frame for exactly `0x115` iterations. `0x5ABD2` is the
typed timed/base/projected sprite wrapper used by that loop. The shared
`0x5B29A`/`0x5B32C` leaves now expose clear/fill plans instead of VDP writes.

The `0x59D7C` stage-two coordinator is now `run_title_intro_stage2`. Its
native state preserves the initial `-0x40`/`-0x20` scene-entry offsets, the
`0xA1` transition frames, the source-ordered reset of the ten intro helpers,
ambush initialization, and the `0x1F1` layered scene loop. The late text
batches are triggered at loop frame `0x164`, while scene-entity rendering is
kept as the explicit `0x5A37C` presentation boundary. Vblank, controller,
palette, schedule, sprite, and ambush services are callbacks, so this
conversion records timing and ordering without importing ROM addresses or
Genesis memory behavior into the host runtime.

The parent `0x59C5E` dispatcher is now `run_title_intro_stages`. It records
the source controller marker, clears and tests the shared stop marker around
the schedule, stage-two, and secondary orchestrator calls, and preserves the
preparation/audio boundary before each child. The late stage loop is followed
by the unconditional source marker clear, palette transition, completion mode
`2`, and `0x59D12` post-intro handoff; the common sample/palette/preparation
epilogue is retained even when an earlier child stops. `0x530EA` is now named
as the audio command-frame boundary and `0x59D12` as the post-intro callback;
the larger mode implementation at `0x134D2` remains intentionally outside
this slice.

The `0x5A1EA` ambush-loop service is now `advance_title_intro_animation`.
Each pass sets the source animation marker, invokes persistent sequence,
motion, dual-sprite, timed-sprite, targeted-sweep, both linear-sweep, and
final-projection services in order, clears the transient scene marker, and
sets the display-dirty bit. The eight child implementations remain typed
boundaries; this conversion captures the dispatcher rather than duplicating
the native port's already-defended SAT/map choreography.

The post-intro handoff at `0x134D2` is now `run_post_intro_mode`. Its typed
state preserves the initial resource/HUD frame, the alternate branch's mode
`4`/mode `2` selection and exact `0x41`/`0x61` frame windows, the normal
seven-entry cursor's `0x09`/`0x06` directional wrapping, the `0x50` exit,
`0x20` action, and `0x80` transfer events, and the final
`selected_mode != current_mode` scene-transition gate. The source services at
`0x13726`, `0x20714`, `0x0391A`, and `0x0E6F4` now have reviewed aliases;
rendering, world updates, and resource transfers remain explicit callbacks.

The `0x13726` mode-selection renderer is now
`render_mode_selection_panels`. It preserves the four source panel passes,
the two vblank/latch/world-service passes after each upload, the trailing
title-text render, and the final text marker read. The panel transfer and
text leaves remain callbacks because their source-specific data tables are
presentation inputs, not runtime memory that belongs in this conversion.

The small mode-service leaves are now typed in `mode_services_conversion`.
`0x136DE` installs the mode-entry result, clears the scene/camera markers, and
publishes the mode-six flags. `0x20714` clears its exit-service marker before
calling the explicit `0x1F246` boundary. `0x0E6F4` performs the source's four
random samples and then requests the world-grid write. None of these leaves
imports the source address space; their generator, transfer, and exit work
remain callbacks.

The `0x1F246` mode-exit coordinator is now `run_mode_exit_services`. It
preserves display commands `0x9100` and `0x9191`, font and optional-menu setup,
the conditional pending-dispatch call, marker clear, prompt flow, display
buffer reset, and palette refresh order. The child routines are named in the
Ghidra export as `prepare_mode_exit_resources`, `run_mode_text_prompt`,
`reset_mode_display_buffers`, and `refresh_mode_palette_data`; their transfer
and prompt details remain explicit boundaries.

The `0x1F292` child is now `prepare_mode_exit_resources`. It preserves the
optional-resource gate, both metadata refresh leaves, source decode order, the
negative asset-mode path that clamps its cursor and runs both resource
variants, and the ordinary HUD path. The resource and HUD implementations are
still host callbacks; no source pointer or VDP state entered the conversion.

Batch 1 adds `0x1F5A8` as `prepare_mode_text_prompt`, preserving its exact
two-call preparation wrapper without carrying the underlying source pointer
or VDP behavior into the host model.

The `0x1F3C4` prompt loop is now `run_mode_text_prompt`. Its descriptor is
modeled as a title, three host text keys, and three command values. The first
controller choice has priority (`0x40`), then the optional second (`0x10`) and
third (`0x20`) choices. Commands below `0xFA` cross an explicit dispatch
callback and return to the prompt; commands at or above `0xFA` return to the
caller. A negative first-option entry waits thirty frames and returns `0xFB`.
The native module has no VDP, ROM pointer, or runtime emulator dependency.

The `0x1F65A` field-exit branch table is now `select_field_exit_prompt`.
It recognizes the five source scene/submode pairs `(0,3)`, `(1,1)`, `(2,2)`,
`(3,1)`, and `(4,0)`, selects symbolic prompt/profile aliases for default,
positive, and negative result variants, preserves the `(1,1)` world-flag
override, and preserves the negative `(4,0)` resource-profile change. The
exit-service and post-service display refresh remain explicit callbacks.

The `0x1D33A` world-sequence renderer is now
`render_world_sequence_tile_rows`. It walks the source's 32-row window for
each eight-byte sequence record, keeps the signed vertical and horizontal
offset arithmetic, expands the packed layout into row width and plane count,
selects the eight-nibble mask cadence, and emits masked host tile writes.
`make_vdp_transfer_command` and the source word store are callbacks; no VDP
register or source buffer address is part of the native routine.

The `0x1F830` field-mode exit policy is now `prepare_field_mode_exit`. It
recognizes the four supported scene/submode routes, selects menu modes 11, 8,
9, and 10, preserves the guarded `(0,0)` world-state mutation paths including
effect marker `0x19`, clears the alternate-result marker before the exit
service, and refreshes the mode display only when that service returns a
nonzero result.

The `0x1F5CA` coordinator is now `run_field_exit_orchestrator`. It gates the
two controller-owned graphics paths, clears the source transient markers,
runs world preparation, world-effect dispatch, and indexed title services,
then invokes the four scene-specific exit policies in order. Each stage is a
typed callback with the source early-abort result; the native coordinator
clears the service marker on every exit path.

Batch 2 converts `0x1F32E` as `clear_optional_mode_resource`, an explicit
typed clear of the optional mode-resource marker.

Batch 3 converts `0x1F34A` as `prepare_mode_resource_variant_b`, retaining its
resource-state refresh followed by HUD-panel draw order.

Batch 4 adds the `0x1F38A` variant-A loop to the same typed resource module;
its exact 80 wrapped word-block transfers are now explicit and tested.

The next resource-state slice converts `0x566E2`, `0x566F6`, and `0x5670E`.
Their source pointer arithmetic is represented by a 24-word typed store: the
metadata copy targets words 0-7, the state copy targets words 16-23, and the
palette copy replaces all 24 words. Each operation marks the store dirty,
matching the shared source invalidation flag.

The title setup module now also owns `0x12F6`: its DBF `0x10` loop is exposed
as `advance_title_palette_fade`, with the source ordering of frame wait,
palette-step, and dirty-mark callbacks tested across all 17 passes.

The adjacent `0x1378`/`0x13BE` sprite-list leaves are now represented by
`sprite_record_builder_conversion`. They consume typed four-word records to
the source `0x8000` sentinel, apply position and attribute offsets, and assign
the sequential runtime sprite index without emitting the sentinel record.

The `0x1910` transfer leaf is now `write_alternating_word_pair`: it increments
the source counter before selecting the odd/even pair, waits for readiness,
and emits exactly two words through a typed callback. The source control/data
registers are not carried into the host conversion.

Disassembly inspection resolved `0x12148`: it clears the high index word,
doubles the selector, adds the base as an 8-bit operation, and reads the byte
table at entity-record offset `0x66`. `read_entity_action_code` preserves that
byte wrap while returning an empty optional for a truncated host record.

The disassembly of `0xC216` is now captured as
`build_grouped_decimal_glyphs`: it emits the decimal remainder stream from
least significant digit upward, inserts the source special glyph after every
three digits, and handles zero as a single digit. The native side receives
glyph values instead of writing directly to VDP data.

The adjacent `0xC406` wrapper is now explicit as
`prepare_numeric_menu_block`: it keeps the source grid-write callback before
the grouped-decimal callback while leaving both presentation operations at
typed host boundaries.

The `0x14446` scan is now `collect_entity_action_candidates`. It checks the
three source slots for active bit 0, rejects status bits 6 and 4, requires a
nonzero action word, and returns the surviving slot indices in scan order.

The `0x13E9E` entity leaf is now `reposition_entity_at_boundary`: it rejects
an already-set bit 6, publishes that one-shot bit, compares each camera
midpoint against its 0xA00-offset origin, and places the entity by the exact
signed 0x2580 step on each axis.

The `0x1460E` state leaf is now `apply_close_range_entity_state`. At distance
below `0x1E0`, it preserves only the source high flag bit, sets bit 2, clears
the secondary state byte, and installs continuation `0xB45C`; the boundary is
tested as a no-op.

The `0x13E8C` coordinator now uses typed callbacks for its unresolved linked
record follow-up. It preserves the source order of reposition then service;
the no-link branch installs continuation `0xB45C` only for timer values through
`0x3C`.

The paired `0x6148`/`0x61F4` queue leaves now share a typed flush helper. Each
emits exactly its queued count of wrapped-transfer callbacks and clears the
count afterward; the primary and secondary entry points remain separately
named in the ledger and exporter.

The `0x17C98` HUD/action lookup is now `find_hud_selection_index`. It scans
the exact five-byte table used by the adjacent setters/comparators and turns
the source register-side index into an explicit optional host result.

The neighboring `0x17C84`/`0x17C8E` leaves now use the same typed selector to
set or compare a parallel five-byte value table. Missing codes are explicit
no-ops/false results rather than writes through the source post-scan index.

The existing audio module now also converts `0x17D3E` as
`advance_audio_service_tick`: it wraps the byte phase counter first, then
enters the enabled audio-service callback sequence.

The adjacent title-overlay pair is now readable as well. `0x59F7A` resets
the post-title overlay's next-frame deadline, six-phase index, and scroll
accumulator. `0x59F8E` advances that schedule only through elapsed frames
`0..0x168`: from `0x120` onward it applies the `0xAA` scroll step and marks
the display dirty, phase descriptors are selected in a six-entry cyclic table,
and frame `0x168` emits the audio/transfer-header/transition-sprite/final
overlay sequence. The phase and final descriptors are host asset handles
provided by the caller; no source pointer or VDP register enters the model.

The following four-entry intro sequence is now typed too. `0x5A0A4` resets
its stage and deadline. `0x5A0B2` renders four scheduled sprite descriptors,
then emits the `0x46` audio/stage-four transition, four repeated stage steps,
the stage-nine anchor update, and the final overlay at stage twelve. The
source's `+9` frame cadence and `<12` gate are preserved, while every unknown
graphics/audio leaf is an explicit callback and each descriptor is a host
asset handle.

The paired persistent intro sequence is now separated and typed. `0x5A244`
resets its own stage/deadline, while `0x5A252` selects four host descriptors,
runs the `0x2F` transition and nine-frame repeated stages, publishes the
stage-four marker and stage-nine anchor, emits its final overlay at stage
twelve, and redraws the current descriptor at `(0x178, 0xD1)` with palette
flags `0x2000` on every active call, including the final transition call.

The next motion leaves are converted as a separate module. `0x5A3B0`/`0x5A3BE`
implement the five-entry timed descriptor table, the nonzero-position sentinel,
per-call position step, the `0x167` elapsed-frame cutoff, and the high-byte
scroll adjustment before projection at `(y=0x133, palette=0x6000)`.
`0x5A4B4`/`0x5A4C2` implement the four-phase/eight-descriptor dual-sprite
sequence, advancing every six frames and drawing both fixed-position sprites
on every active call.

The six-descriptor timing primitive is now typed separately. `0x5A572`
installs its terminal index, `0x5A582` starts it at caller coordinates, and
`0x5A59C` selects a new descriptor every eight frames while continuing to
project the current descriptor between deadlines. The final descriptor is
still drawn on the call that reaches the terminal index, matching the source.

The following intro sweep trio is now isolated. `0x5A61A/0x5A628` uses its
seven-entry table's target x to stop a decrement-nine sweep; `0x5A70E/0x5A71C`
uses eight entries, decrement eight, and a `0x40` cutoff; and
`0x5A7EA/0x5A7F8` uses the same eight-entry cadence with decrement eleven.
Each continues to project the active descriptor between table deadlines and
keeps its source-specific coordinates and palette flags in the typed request.

The late three-phase projection and final text batch are now typed. `0x5A8C8`
resets the projection to x `0x193`; `0x5A8DE` waits until six frames after
the intro start when a phase is due, then advances x by four and keeps drawing
between deadlines at y `0x13C`. `0x5A96A` submits six host text handles in
the source order at `0x2906`, `0x2986`, `0x2A06`, `0x2A86`, `0x2B86`, and
`0x2C06`, preserving the `0xE000` palette mask.

The next intro-stage helper cluster is now typed. `0x5AA5A` submits three
secondary-stage text assets at `0x2186`, `0x2286`, and `0x2386`. The
`0x5AC28`/`0x5AC36` projection resets and draws the moving sprite from the
accumulator high bytes, while `0x5AC0E` covers the fixed sprite. The
`0x5AC70`/`0x5AC7E` sequence preserves its `0x3C` start gate, six-frame
descriptor cadence, and `0xE000` palette. `0x5AB96` retains the post-`0xB4`
masked cadence and four ordered service steps.

The runner reset cluster is now typed. `0x59BCC` clears status bits `6` and
`4` in each of the three live runner records, preserves the source order of
the preparation (`0x3674`, argument `0x0A`) and finalization (`0x37EC`)
boundaries, then seeds the record-relative `+0x4C` and `+0x9C` timers to ten.
The source service calls remain callbacks because their broader record effects
are not yet established. `0x59C34` is the matching three-record byte adjuster:
the caller's byte is added to offset `+0x8D` in each record with native
eight-bit wraparound. Neither conversion carries the original `0xFF0100`
address or register state into the host model.

The recruited-roster helper cluster is now converted in its own module. `0x598B0`
is the common packed-record projection: it maps the source 0x4A-byte record into
the live 0x100-byte record's known field ranges while preserving opaque bytes.
`0x5966E` projects the default record first, then the active primary and
secondary selections, and reports which typed destination was last populated;
the native code does not carry the original A0/A1 pointers.

`0x42F2` now loads a primary roster template from an extracted table, writes the
source-proven metadata words `+0x3E=1`, `+0x20=0x19`, and `+0x26=0x320`, runs
the common population path, and applies the caller byte to `+0x8F`. `0x431C`
does the corresponding secondary setup with `+0x3E=2`, `+0x20=0x31`, and
`+0x26=0x620`. Its low-byte duplicate comparison is preserved: selecting the
same template as primary skips the secondary copy but still runs `0x5966E` and
applies the final population byte to the last destination. Invalid host table
indices return a typed result instead of reproducing unchecked source access.

The common save-image validator is also converted. `0x59786` sums the 0x61C
bytes beginning at extracted offset `0x14`; `0x593FC` and `0x5961C` both
require the extracted `GAME` signature at offset `0x04` and compare that sum
with the big-endian longword at offset `0x630`. The native module accepts the
result of the existing even-byte extraction boundary directly, keeps backup
and SRAM validation as separate named wrappers, and rejects truncated or
malformed host buffers without importing the original RAM addresses.

`0x59456` now has a native save-slot model as well. It validates both SRAM
slots, preserves two valid slots, copies the first valid slot into an invalid
second slot, copies a valid second slot into an invalid first slot, and reports
when neither slot is usable. The source's interleaved even-byte copy is already
represented as extracted host vectors at this boundary, so the repair is a
plain native vector copy rather than a runtime address operation.

The save-record direction is now covered too. `0x597EE` is the inverse field
projection from a live 0x100-byte runner record to a compact 0x4A-byte save
record, and `0x59798` applies it to the default, primary, and secondary
records in source order. `0x59318` seeds a typed save buffer with its fixed
0x14-byte header, thirteen compact records (Joshua plus twelve roster
templates), and the two inactive selection markers. These are extracted-data
operations; they do not introduce runtime ROM or RAM addressing.

The save/load wrapper pair is now typed. `0x59372` tries the primary backup
slot, then the secondary slot and repairs the primary when the secondary is
the only valid copy; if both fail it calls an explicit SRAM fallback boundary.
`0x59360` runs that loader only after the two-slot SRAM preparation reports a
usable result. The fallback remains a callback because the source's storage
device and interleaved extraction state are a separate boundary from the
native save work buffer.

`0x59986` is now a typed world-effect sequence selector. It performs the two
source-bounded rolls, indexes a nine-column context row, suppresses disabled
entries, and follows the single enabled-entry indirection. Table entries are
host handles for extracted effect sequences; no source pointer or ROM address
is retained in the conversion.

Two fixed-loop leaves are now named and tested as well. `0x33BE` is a 4x4,
16-call wrapper around the existing source-wrapped transfer boundary, while
`0x3548` invokes the HUD panel drawing leaf three times. Neither conversion
adds an address-based runtime representation; both retain the source call
counts and order as ordinary host loops.

`0x2BD2` is now represented as a two-branch field-scene transition helper.
Both paths begin with the palette step. A zero transition marker aligns the
camera, refreshes scene state, guards world effects, consumes the pending event
index, updates its counter with `(value & 0xE0) + 1` when the source state is
negative, then renders/uploads. The other branch clears the palette marker,
resets the scene markers, refreshes profile and scene state, applies the
profile, renders/uploads, and finally guards world effects.

The graphics setup batches at `0x334C` and `0x3404` are now explicit host
loops. The first preserves one wrapped transfer plus five grid writes. The
second preserves two groups of fourteen wrapped transfers and then invokes
the already-typed HUD panel triplet. Their individual transfer leaves remain
callbacks, so no Genesis register or ROM-address machinery entered the
conversion layer.

The archetype boundary is now explicit. `0x42B0` selects one of the three
extracted 0x100-byte Joshua templates (Samurai, Decker, or Gator Shaman),
copies that record into the live Joshua slot, and publishes the source
selection markers `0`, `1`, and `2` at record offset `+0x3E`. The host helper
rejects an out-of-range extracted-table index instead of allowing an unchecked
ROM-table read. The paired `0x2C92`/`0x2CB2` field wrappers are also typed:
they clear their pending request, invoke the shared viewport renderer, and the
alternate path brackets that call with the source `0x8000` render mask.

The adjacent entity animation cluster is now typed. `0x38AC` has three exact
outcomes: no action when current ammo or equipped item is zero, delegation to
the unresolved weapon service for record types below `3`, and a one-byte ammo
decrement for other record types. `0x38FA` runs its primary, secondary, and
weapon animation leaves three times. `0x39F2` composes the known grid refresh,
HUD refresh, and that triplet in source order; the individual graphics and
weapon leaves remain callbacks rather than guessed implementations.

`0x2B34` now has a typed scene-profile model. With no pending profile it
copies the three default runner positions and camera pair. With a pending
eight-word profile it consumes the camera pair plus three runner pairs,
aligns both camera coordinates with `0xFF80`, clears the pending source, and
sets both source camera-transition markers to `-1`; the host helper represents
that pending source as an optional value.

`0x286E` is now isolated as the world-effect spawn guard. A clear global
effect marker exits immediately; otherwise the source scans thirteen records
and suppresses a new spawn when one has behavior bit `0` and effect bit `7`.
Only the no-match path requests the existing player-position spawn helper.

The field entry wrappers are now represented without importing their global
state machine. `0x281E` preserves the display preparation, marker resets,
profile refresh, scene refresh, profile application, render, upload, and
world-effect guard order. `0x28C2` preserves its refresh/guard/display order,
aligns the camera pair with `0xFF80`, resets both camera-transition markers to
`0xFFFF`, and then renders/uploads. Each service remains an explicit callback.

`0x176BA` is now `refresh_field_resource_streams`: it submits the source-order
raw transfer request for destination `0x2360` and `0xA80` bytes, then submits
the `0x2DE0`-byte compressed-resource request. The requests are typed host
boundaries, so the native runtime can map them to named assets without
carrying the original ROM pointers or VDP registers.

The first menu service leaves are now named and typed. `0x110BC` and `0x110EC`
are separate HUD-panel presentation boundaries, while `0x11136` preserves its
eight clear/write passes and source return value (`0`) through ordinary
callbacks.

The menu tile upload leaf at `0x11442` now resets the typed upload cursor
before invoking the menu tile-block asset boundary.

The `0x11678` menu-grid leaf is now a named one-call host boundary.

The adjacent `0x116C4` wrapper now performs four typed grid submissions and
then invokes the shared menu-metadata refresh boundary.

`0x11900` now preserves the menu-row preparation before the repeated-grid
submission as two explicit host callbacks.

The `0x11928` menu-row wrapper now retains its grid-before-row render order.

The `0x11ABE` dialog wrapper now preserves the preparation result while
running the shared service exactly twice.

The `0x12062` text refresh now uses a signed pending-count field and typed
prepare/render callbacks. Counts at least one skip the work; eligible counts
render twice and clear the field.

The `0x1209A` cursor service now alternates typed primary/secondary markers
until a non-negative slot is available, then submits text and cursor service.

The numeric menu leaf at `0x120E4` now owns a typed cursor/count pair. It keeps
the source marker write, signed zero/overflow wrap, past-count clamp, and digit
builder as explicit host operations.

The `0x1275C` menu leaf now clears a bounded slot record and preserves the
low-action asset-load branch.

The final planned menu leaf, `0x127B0`, now keeps panel upload, byte-toggle,
layout update, and text render in source order through typed host callbacks.

The next menu helper pass begins at `0x12C1E`, represented as a named grid
preparation boundary; its sibling wrappers will follow the same typed path.

`0x12C36` is now the independently named second menu-grid preparation
boundary.

The third sibling, `0x12C42`, is now represented by its own named host
preparation callback.

`0x12C56` is now a named repeated-grid submission boundary.

The adjacent `0x12C5A` repeated-grid path is now independently named and
typed as well.

The `0x12CD6` menu text leaf is now a named text-presentation boundary.

The entity-action menu sequence at `0x12E9E` is now isolated in its own
conversion module: action read precedes the menu service, with both expressed
as host callbacks.

`0x12F50` now preserves the entity-action menu update, text render, and grid
submission order in the same isolated module.

The `0x12FA4` action-menu advance now renders and services every pass, while
retaining the source's grid write only when more than one action remains.

The detail-panel trio at `0x13000`, `0x130C0`, and `0x13228` is now separated
by caller rather than left as generic menu routines. `0x13000` selects the
thirteen-entry entity-action detail label from the active action, the negative
display-state auxiliary value, or the first matching cyberware bit (bits 5..8
with source contributions 1..4). `0x130C0` plans a field-action detail entry:
it selects the resource row, toggles the alternate resource set at the source
threshold of 13, preserves the six phase-flag overlays in the ROM order
2/4/1/8/16/32, and chooses the ordinary/extended/action-dependent label
ranges. `0x13228` does the corresponding selected-inventory-item read for the
field-service menu, using item code minus one for the resource row, the item
phase byte for those same overlays, and the `0x12148` action-code boundary for
items `0x14` and `0x18..0x28`. These conversions produce typed render plans;
they do not reproduce VDP writes or treat the detail panel as a flipbook.

The action-state setter at `0x13466` is now native-shaped: it publishes typed
action/state fields, clears only the source branch's reset markers, and keeps
the action-service and asset-refresh boundaries explicit.

The `0x13874` candidate dispatcher now clears its typed service marker and
selects primary/secondary collection from the source mode and low six action
bits; alternate mode preserves the two-dispatch path.

The `0x13AE6` threshold leaf now preserves its action-search then action-state
publication order as two explicit callbacks.

The lane selector at `0x13C20` now returns a typed 0/1/2 lane from the source
occupancy thresholds and signed override conditions.

The action-record collector at `0x13C74` now filters the four retained action
codes, publishes code 11's side value, and marks the host table terminated
without exposing the original byte-pointer layout.

The `0x144E8` close-range wrapper now points to the typed close-range service
boundary rather than reproducing the source call frame.

The coordinate helper at `0x14640` now returns a typed quotient/remainder tile
mapping. A zero map stride is rejected as an invalid host input instead of
reaching the source division fault.

The entity audio/action leaf at `0x15354` now arms typed flags and timers,
copies the current position into the destination pair, clears its effect word,
and invokes the host audio boundary.

The `0x1562E` auxiliary behavior wrapper now preserves its two-stage order:
auxiliary preparation first, then the typed entity behavior preparation.

The six-byte `0x1582A` allocator is now separately named and mapped to the
same typed eighteen-slot ownership model as its later shared counterpart; the
two ROM entries are no longer conflated in the ledger.

The final routine in this batch, `0x16AFA`, now forwards typed source,
destination, word-count, and callback state into the existing wrapped word
transfer helper.

The compact row-writer family is now typed as well. `0x1184` emits one masked
row with optional `0x8F80`/`0x8F02` display-increment controls, `0x11C0`
repeats a source row at `0x80`-byte destination strides, and `0x11F6`
consumes a row-major source stream with two-byte row advancement. `0x0EE6`
dispatches pending uploads by the source count sentinel and preserves the
large-transfer flag clear.

The transfer library now also covers the shared `0x0F98` and `0x1232` leaves.
`write_vdp_word_block` emits the requested bounded even-byte word stream after
one transfer command; `write_vdp_grid` emits each row at the source `0x80`
byte stride and ORs the caller's tile mask into each word. `0x0E34` is a typed
upload-state reset that emits the source four-word zero clear and publishes
the stream cursor/enabled state. The terrain layer copy at `0x0313A` and the
thirteen-record readiness scan at `0x2FF0` are likewise isolated from runtime
memory and covered by focused tests.

The next transfer pass adds `0x0F5C` as an explicit source-64K split around
the `0x0F98` word writer, preserving destination advancement without carrying
the original pointer arithmetic into host code. `0x0CEC` now installs the
four-word startup transfer state and emits its fourteen control words, while
`0x0301E` is the fixed 0x140-byte scene-profile upload using the same typed
block primitive.
