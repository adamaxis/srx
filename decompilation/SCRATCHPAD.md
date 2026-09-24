# Shadowrun Genesis decompilation scratchpad

## Current handoff

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
the negative `FFDF58` XOR mode used by the second plane. The `0x3BAC` camera
tail is now direct as well: `0x15D3C`/`0x15D4E` read the direction and
correction tables, then changed axes call the `0x4018`/`0x40E2` edge refreshes,
whose `0x3D4A`/`0x3DDE` spans retain the VDP increment control writes.

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
selects a pair of signed factors in the `0x15E16` table, signed D3 is the
magnitude, and the high words of the two `MULS` products are returned. A zero
magnitude clears both result words.

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
- Use `analysis/pointer_tables.tsv` and `analysis/direct_references.tsv` to
  promote only referenced tables into typed C++ data structures.
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
