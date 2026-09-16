# Shadowrun Genesis ROM reverse-engineering workspace

This workspace contains the first reproducible decompilation pass for:

`C:\Github\shadowrun\Shadow Run (U) [!].bin`

The original image is a raw 2 MiB Sega Genesis/Mega Drive ROM, not a Windows PE
executable. Its SHA-256 is:

`92A614491FC3EA12A6DF32459CCE08B48574B7B814E244C07EB860DC5C6EDB7E`

## Current baseline

The primary analysis is the standalone C++17 program in
`src/rom_analyzer.cpp`. It reads the ROM directly, parses the big-endian
Genesis vector table, decodes the 68000 instruction forms used on the reset
path, follows direct calls and branches, records absolute hardware references,
and searches for ROM pointer-table runs and printable strings.

An earlier Ghidra pass is retained in `export-seeded/` as a cross-check only. It
found 722 function entries and produced a C-like result for each one, but those
files are not the C++ port and are not treated as authoritative source.

The direct C++ analyzer writes these reports to `analysis/`:

- `analysis_report.txt` - ROM header, vectors, code ranges, and hardware references.
- `direct_disassembly.s` - instructions reached by direct control-flow analysis.
- `functions.tsv` and `callgraph.tsv` - discovered function entries and calls.
- `pointer_tables.tsv` - runs of longwords pointing into discovered code.
- `verified_tables.tsv` - byte-checked reset, region, and VDP table anchors.
- `indirect_control_flow.tsv` - unresolved register-indirect call/jump sites
  queued for deliberate dispatch-table recovery.
- `strings.tsv` - printable runs with ROM offsets.
- `direct_references.tsv` - ROM and hardware references from decoded instructions.

## Building the direct analyzer

```powershell
cmake -S 'C:\Github\shadowrun\decompilation' `
  -B 'C:\Github\shadowrun\decompilation\build' `
  -G Ninja
cmake --build 'C:\Github\shadowrun\decompilation\build'
& 'C:\Github\shadowrun\decompilation\build\shadowrun_rom_analyzer.exe' `
  'C:\Github\shadowrun\decompilation\input\shadowrun_genesis_us.bin' `
  'C:\Github\shadowrun\decompilation\analysis'
```

The working copy is byte-for-byte copied from the original. The safe filename
is used because PowerShell and batch launchers treat the original brackets and
exclamation mark as shell syntax in some invocation paths.

## Re-running the direct analysis

The original ROM remains at `C:\Github\shadowrun\Shadow Run (U) [!].bin` and
is never modified. A working copy is present at
`input/shadowrun_genesis_us.bin` for tools that cannot handle the original
filename safely.

The old Ghidra snapshot can be regenerated independently if needed, but it is
not required to build or run the C++ analyzer.

## Native C++ bootstrap

The first native frontend seam is implemented in
`include/shadowrun/native_port.hpp` and `src/native_port.cpp`. It provides an
abstract `InputState`, a generic `ScreenWriter`, and a renderer-neutral RGBA8
software surface. The Windows target `shadowrun_native.exe` opens a 320x224
software-rendered window, accepts arrow/Z/X/C/Enter input, and presents native
C++ screen output without a 68000 interpreter or Genesis VDP dependency.

This is an executable frontend/bootstrap milestone, not a claim that the full
game is playable yet. When given the ROM, the shell now decodes the reviewed
startup resource path: the three `0x1530` pattern streams at `0x1F310E`,
`0x67CB0`, and `0x6875C`, the `0x1728` tilemap stream at `0x67DF6`, and the
four-line CRAM image at `0x1FF0`. Those resources are converted to host RGBA8
and submitted through `ScreenWriter`; no Genesis VDP or 68000 runtime is
needed by the native executable.

The asset-backed native shell now exposes a native scene transition as well. It starts
on the reconstructed title scene, consumes the pressed `Start` edge, and enters
a typed C++ field state with held directional movement. The field presentation
translates the mode-2 data path:
raw pattern data at `0x166B82`, compressed 4x4 name-word stamps at `0x170102`,
and the 60x42 primary/secondary tables at `0x16D9A2`/`0x16ED52`. Genesis tile
rows are decoded as four packed bytes (high nibble then low nibble), matching
the VDP's linear pixel format; treating those bytes as bitplanes produces the
vertical/noise corruption seen in the earlier native field output. The result
is a 1920x1344 host RGBA map using the ROM's initial top-left camera viewport.
ROM `0x2B34`'s first three mode-record coordinate pairs are retained in the typed
resource descriptor, and arrow input advances the first recovered field record
through that world space; the viewport follows when the entity reaches the
reviewed projection window. The reviewed field-loop raw `0x2000` branch to
`0x391A` is mapped to the native C edge: it selects the next of those three
initialized records and preserves each record's fixed-point position when the
selection changes. It does not render unverified duplicate actors. The native
port does not run a 68000 interpreter or depend on Genesis VDP state. The
native field resolves the field-record directional state via the reviewed
`0x600E` kind/state indirection (`0x62AC` -> kind-3 table `0x6378` -> lists
beginning at `0x9334`). Its eight `0x8002` descriptors
(`0x9D6E` through `0x9F66`) use the direct `0x3E86` packed-source expansion
through the `0x3EDC` nibble table, the `0x3EFE` column-major arrangement, and
conversion to transparent RGBA8 images. The native entity selects that table
entry from the held movement vector, preserving the ROM's directional-state
boundary instead of cycling through unrelated directions; the vector match is
driven by the signed eight-entry probe table at `0xC002` used by `0xBABC`.
Its anchor uses the
reviewed `0x5A62` projection routine and adapts the VDP's `0x80` sprite origin
into host pixels. The generic `0x72B0` decoder remains available for other ROM
animation blocks, but is not used to choose the field entity frames.
The terrain table at `0x172DAA` is rebuilt into the native status grid and the
four-corner `0xE7C8` probe rejects blocked movement before the typed record is
updated. Field `Start` now enters a native `FieldMenu` matching the reviewed
`0x10D30` command-screen loop: the selector starts at ROM value six,
left/right wraps the seven `0x114A0` values, up/down wraps the four `0x11564`
secondary rows, and its labels are extracted from the `0x1170C`/`0xD1D28`
string path. A/B record the same shared `0x10E4E` dispatch boundary, C retains
the reviewed `0x391A` runner switch, and Start closes the menu without
resetting field state. This is a runnable title-to-field slice, not a claim
that the whole game is playable; the remaining command-handler bodies,
expanded entity/session population, and additional field object behavior
remain the next integration boundaries.

The Inventory selector now has its first typed data path. ROM `0x12148`
addresses eight item bytes at entity record `+0x66` as `2*row+selector`.
Extraction preserves those item and adjacent phase bytes, and converts the
50 fixed `0x30`-byte records at `0xD4540` into host item names/categories.
The native command screen can therefore browse the actual initial inventory
slots from the `.srna` pack without reading the ROM. The deeper equip/use/
discard effects in the `0x12168` handler remain separate work; its reviewed
armor toggle (`+0x76` and the selected `+0x6E` byte for item codes `0x2C+`)
is now carried into native typed state.

The top-level Clips command now enters a native `ClipsMenu` corresponding to
ROM `0x11B58`. It carries the active record's `+0x4E` ammunition and `+0x4F`
available-clip values, cycles the reviewed selectors `2..5`, and preserves the
`0x12062` no-clips gate. The reload, trade, and recipient quantity mutations
remain isolated behind that typed dispatch boundary.

The Cyberware command now enters a native display screen corresponding to ROM
`0x11950`. Extraction preserves the active record's `+0x90` fifteen-bit
cyberware field and expands the pointer names at `0xD2072` into host strings;
the initial record therefore shows the recovered `Hand Razors` entry. The
handler is display-only in the ROM, so Start returns to the command screen and
there is no invented mutation or selection behavior. These values are stored
in native asset-pack version 5.

The Windows frontend preserves the reviewed controller boundary from ROM
0x195C: the sampled word uses its low byte for held state and its high byte for
newly asserted edges, in the order Up, Down, Left, Right, B, C, A, Start. It
feeds that word through `decode_genesis_controller_word`; a future SDL or
graphics frontend can use the same host input contract.

The extracted field template also carries the signed display bytes at
`+0x56/+0x57`. Native projection preserves the `0x36FC` display gate: a
negative `+0x56` or the sentinel `0x7F` leaves the entity hidden even when its
screen projection is in range. These values are included in native asset-pack
version 5, so the host-only runtime retains the recovered state, inventory,
cyberware catalog, and clip values without
reading the ROM.

The mode-2 map loader now reads a typed `GenesisMode2FieldResources` view from
the selected record at `0x172C80` and its setup block at `0x172C50`. That view
supplies the initial `0x3400`/`0x2280` fixed-point camera, `0x166B82` pattern
source and length, `0x170102` metatile stream, the two map pointers, the
`0x172D8A` palette, and the derived 60x42 dimensions. The native field uses
this descriptor for loading and reset, keeping ROM layout knowledge at the
asset boundary rather than duplicating literals in the game loop.

That descriptor also identifies the terrain rectangle table at `0x172DAA` and
the direct `0xE7C8` bounds. `decode_genesis_mode2_terrain_map` expands its 101
sentinel-terminated rectangles into a 64x64 host status grid, retaining the
original status values for native movement/collision. The shell drives record
zero from the first coordinate pair copied by `0x2B34`, probes candidate
positions at the four `0xE8A0` offsets, and commits only unblocked moves. The
renderer-independent `terrain::probe` ports the direct `0xE7C8` bounds/status
path and its `0xE8A0/0xE9EE/0xEA36/0xEAD2` helpers over this host grid.

```powershell
cmake --build 'C:\Github\shadowrun\decompilation\build-debug-verify'
& 'C:\Github\shadowrun\decompilation\build-debug-verify\shadowrun_native.exe'
```

On Windows, the build also extracts `shadowrun_native.srna` beside the
executable. When launched from that build directory, the native frontend finds
the pack automatically; the ROM is only an input to the build-time extractor.

To load the ROM explicitly from any working directory:

```powershell
& 'C:\Github\shadowrun\decompilation\build-debug-verify\shadowrun_native.exe' `
  'C:\Github\shadowrun\decompilation\input\shadowrun_genesis_us.bin'
```

The same startup frame can be exported as a diagnostic BMP:

```powershell
& 'C:\Github\shadowrun\decompilation\build-debug-verify\shadowrun_graphics_extract.exe' `
  'C:\Github\shadowrun\decompilation\input\shadowrun_genesis_us.bin' `
  'C:\Github\shadowrun\decompilation\build-debug-verify\startup_scene.bmp' --title
```

The recovered mode-2 field map and the exact native title-to-field frame can
also be exported directly:

```powershell
& 'C:\Github\shadowrun\decompilation\build-debug-verify\shadowrun_graphics_extract.exe' `
  'C:\Github\shadowrun\decompilation\input\shadowrun_genesis_us.bin' `
  'C:\Github\shadowrun\decompilation\build-debug-verify\mode2_map.bmp' --mode2-map
& 'C:\Github\shadowrun\decompilation\build-debug-verify\shadowrun_graphics_extract.exe' `
  'C:\Github\shadowrun\decompilation\input\shadowrun_genesis_us.bin' `
  'C:\Github\shadowrun\decompilation\build-debug-verify\native_field_scene.bmp' --native-field
& 'C:\Github\shadowrun\decompilation\build-debug-verify\shadowrun_graphics_extract.exe' `
  'C:\Github\shadowrun\decompilation\input\shadowrun_genesis_us.bin' `
  'C:\Github\shadowrun\decompilation\build-debug-verify\entity_frame.bmp' --entity-frame
& 'C:\Github\shadowrun\decompilation\build-debug-verify\shadowrun_graphics_extract.exe' `
  'C:\Github\shadowrun\decompilation\input\shadowrun_genesis_us.bin' `
  'C:\Github\shadowrun\decompilation\build-debug-verify\field_entity_states.bmp' --field-entity-states
& 'C:\Github\shadowrun\decompilation\build-debug-verify\shadowrun_graphics_extract.exe' `
  'C:\Github\shadowrun\decompilation\input\shadowrun_genesis_us.bin' `
  'C:\Github\shadowrun\decompilation\build-debug-verify\field_entity_state_2.bmp' --field-entity-state 2
& 'C:\Github\shadowrun\decompilation\build-debug-verify\shadowrun_graphics_extract.exe' `
  'C:\Github\shadowrun\decompilation\input\shadowrun_genesis_us.bin' `
  'C:\Github\shadowrun\decompilation\build-debug-verify\shadowrun_native.srna' --native-pack
```

The `.srna` pack contains the expanded RGBA title/field/entity images, the
host terrain grid, initial field positions/camera, and the reviewed native
entity/direction state. `shadowrun_native.exe` accepts that pack path directly,
so the Windows runtime can start without reading the ROM:

The graphics extractor can also render the field from the pack alone. This
keeps the extraction artifact path identical to what a future SDL/DirectX
frontend will consume:

```powershell
& 'C:\Github\shadowrun\decompilation\build-debug-verify\shadowrun_graphics_extract.exe' `
  'C:\Github\shadowrun\decompilation\build-debug-verify\shadowrun_native.srna' `
  'C:\Github\shadowrun\decompilation\build-debug-verify\native_field_scene_from_pack.bmp' --native-field-pack
```

```powershell
& 'C:\Github\shadowrun\decompilation\build-debug-verify\shadowrun_native.exe' `
  'C:\Github\shadowrun\decompilation\build-debug-verify\shadowrun_native.srna'
```

The dependency-free graphics converter accepts offsets recovered from the ROM
analysis and emits a Windows-readable 32-bit BMP atlas. It does not guess
whether arbitrary bytes are graphics; the caller supplies the reviewed tile
and palette locations.

```powershell
$tileOffset = 0x000000 # replace with a reviewed tile-data offset
$tileCount = 16        # replace with the reviewed tile count
$paletteOffset = 0x000000 # replace with a reviewed CRAM-data offset
& 'C:\Github\shadowrun\decompilation\build-debug-verify\shadowrun_graphics_extract.exe' `
  'C:\Github\shadowrun\decompilation\input\shadowrun_genesis_us.bin' `
  'C:\Github\shadowrun\decompilation\build-debug-verify\tiles.bmp' `
  $tileOffset $tileCount $paletteOffset
```

`graphics.hpp` exposes the same tile/CRAM/RGBA operations to the native game,
and also decodes the reviewed `0x1530` literal/repeat/back-reference resource
stream. Add `--resource` when the supplied tile offset points at that
compressed stream; the converter will use its decoded bytes as the tile source.
BMP is only an extraction or diagnostic format; runtime rendering is not
palette-indexed.

## ROM facts established so far

- Initial stack pointer: `0x00FFFFF8`.
- Reset PC: `0x00000236`.
- ROM range: `0x000000-0x1FFFFF`.
- Genesis RAM range: `0xFF0000-0xFFFFFF`.
- VDP data/control accesses occur at `0xC00000` and `0xC00004`.
- The reset bootstrap constants occupy `0x02C4-0x02DD`; the region selector is
  at `0x046E`, and the 19-word VDP setup table is at `0x0D14`.
- The indexed data tables at `0x4BB0` and `0x4C0C` are now represented in
  `include/shadowrun/rom_layout.hpp`; `0x4BB0` resolves to eight five-word
  signed offset vectors rather than code addresses, matching the five passes
  of the `0x4BA0` DBF loop.
- The four descriptor pointers at `0x4ACC` are also byte-checked and typed;
  the `0x4ADC` object path selects them with the record word `+0x0E` low bits.
- The `0x4B9A`-`0x4BAA` object-byte scatter is translated in
  `include/shadowrun/object_mask.hpp` and `src/object_mask.cpp`; it copies five
  record bytes into RAM `0xFF1700` using the selected word-wrapped offsets,
  preceded by the separately translated `0x1A304` indexed clear at
  `0xFFD86C`.
- The neighboring `0x1A310` loop is translated as the 18-byte
  `clear_indexed_object_state` reset over the same `0xFFD86C` table.
- The reset routine initializes hardware and RAM, checks the region value at
  `0xA10001`, and enters the startup/game loop through the low-ROM routines.
- The reset path contains a long region/copyright-screen path before the main
  loop, so this is not a simple single-screen demo or asset-only ROM.
- The manually reviewed C++ port currently includes boot/VDP startup, the
  low-level runtime helpers at ROM `0x0BD2-0x0D58`, and the session/frame RAM
   write blocks in the `0x075E` main-loop prologue. All of these use
   `GenesisBus`; no SDL or renderer dependency has been introduced.
- `ShadowrunMainState::run_frame_window` now covers the reviewed 31-pass
  `0x08C0` frame window, including its flag sets, frame wait, two controller
  samples, and transition-field cleanup. Its `0x4618` entity phase and
  `0x12F6` palette transition have direct defaults. The `0x4658` post-pass
  now has a direct reviewed object-output pass; its remaining nested handlers
  stay explicit `FrameWindowHooks`.
- The direct `0x13874` frame-list call used by `0xA8EA` now builds the seven-
  record `FF0400` list and three-record `FF0100` list from the raw flag tests,
  including the `FFDF88 & 0x3F` phase selector and `FFDFE8` gate.
- The surrounding `0xA8EA` sixteen-record pass is now a typed C++ routine as
  well: it preserves the timer decrement, signed `+0x16` transition tests,
  68000 `BSET/BCLR` behavior, direct `0xBCAE` coordinate adjustment, and the
  `+0x3A` handler boundary. Its still-unrecovered nested calls are explicit
  `FrameEntityUpdateHooks`, not hidden host callbacks.
- The `0xEB92` collision pass called from that update is now direct C++: it
  scans subsequent entity records, preserves the linked/non-linked overlap
  radii, sets both collision flags, applies `0x15D3C`, and reproduces the
  `0xECCC` resource-side flag and random adjustment logic. Its two
  resource-loader calls now have direct queue-write defaults, with optional
  hooks retained only for observation or replacement.
- The `0xAB02` resource/animation branch now has direct defaults for the
  `0x1E2EC` queue leaf, the ten-slot `0x16E08` allocator, and the weighted
  `0x16B98` animation selector. The `0x1DBF0` ten-record relationship reset
  is also bus-backed; its ROM tables remain data-driven through the bus.
- The reaction support leaves `0x1DBAA`, `0xBC36`, `0x423C`,
  `0x1669C/0x166D0`, and `0x16760` are now typed bus operations. `0xBC36`
  preserves its four-row input-flag selector and `0x423C` its word-sized
  12-bit map-position composition. The known `0xB7B8` state/resource pass is
  also direct, including its re-entry gate, collision-state resource table,
  tile-mark branch, and transition writes. `0x1651E` now has a direct
  mode/resource implementation and ten-slot `0x164F2` allocator. The
  `0x53126` nested Z80 synchronization is also direct, including its six
  stepped byte writes, frame/input waits, and `0x16`/zero tail sequence.
- The next movement cluster is now available as `run_entity_movement_0xBABC`.
  It preserves the C002 directional probes, B77E/B732 filtering, BB86's
  pointer-table fallback, and the BBE4/B442 record transitions. Its
  `0xE7C8` terrain sampler now directly performs the FF1700 lookup, bounds
  checks, E9EE rectangle test, and EA36/EAD2 directional tests; EA36 remains
  optionally overrideable for instrumentation.
- The known body of `0x391A` is now a direct C++ selector: it dispatches the
  `0x25` Z80 command, streams both fixed ROM strips through the existing
  `0x1232` path, preserves the record-state loop and `0x1E540` reset writes,
  and now has direct defaults for the `0x3548`/`0x36FC`/`0x38FA` display
  passes. The `0x03404` animation-expiry refresh is direct and streams its
  fixed/state-selected tables through the bus. The `0x1DC4C` relationship call
  now has a direct `0x391A` default with its caller-established selector seven;
  its hook remains available for observation/replacement, and the standalone
  typed implementation accepts an explicit selector for the other caller.
- The overlapping animation entry points at `0x138B8` and `0x13904` now have
  direct C++ timer/flag gates. Their shared `0x13A4C` type-`0x0F` branch is
  also translated, while later state-machine targets are passed through an
  explicit `EntityAnimationContinuationRoutine` with their original ROM
  addresses. The shared `0x13E8C/0x13E9E` branch now directly initializes
  camera-relative coordinates under its one-shot flag guard. `0xAC84` uses
  these direct gates by default; its known `0xAE5E` post-call remains in the
  surrounding frame pass.
- The `0xB39A` and `0xB3FE` tails used by the larger animation coordinator
  are now direct. Their separate handlers and frame tables, phase-dependent
  `0xABFC` mode, and distinct `0xAF6C`/`0xABFC` ordering are covered by tests.
- The next animation transition chain is now direct through `0x13A92`,
  `0x13B12`, `0x13CDE`, `0x13AE6`, and `0x13466`. Counted event records,
  type-B special arguments, PRNG selection, the `0x55F02`/`0x55F20` rate
  test, the inherited D4/D5/D7 register boundary, phase-accumulator scaling,
  and the `0x036FC`/`0x03404` display tail are represented in typed C++.
- The `0x142AE` continuation is now direct through its `0x144E8`/`0x1460E`
  close-distance gate. It preserves the post-`0x15CA4` minor-axis register,
  handles the converging indexed-table update at `0x14300`, and enters the
  direct `0x1450E` target-probe path. `0x1450E` preserves the source-type
  `+/-0x6000` selection, `0x15DCC` transform, forward/reverse `0xE7C8`
  probes, final `+0x96/+0x98` restore, and `0x145A2` fallthrough. The unusual
  packed-`DIVU.W` increment in `0x125FA` is modeled rather than replaced
  with a host-language rounding rule.
- The `0x14492` animation-link selector now directly consumes the primary and
  secondary lists built by `0x13874`, preserves an existing member, performs
  the ROM `0x0D3A` random selection when needed, and retains the empty-primary
  relationship-reset branch.
- The `0x13D0C` linked-animation update now directly handles its active-phase
  terminal gate, mode-two linked-coordinate copy, type/phase distance tables,
  one-step `0x15DCC` target construction, `0x144E8` gate, and `0x145A2` tail.
- The `0x14508` entry prefix is now explicit and shares the direct `0x1450E`
  target-probe implementation with its absolute `FFFC04` source record.
- The `0x144FA` target dispatcher now directly selects that same probe body
  from either the linked `+0xA2` record or the `0x14508` absolute-source path.
- The linked-record gate at `0x14C7C` now directly covers the ordinary
  `0x15C84` close-link predicate, the `0x0C` distance/slot branches, and the
  `0x0D/0x0E` movement-timer path.
- The `0x145A2` tail is now direct through the `0x14C2E` direction flags,
  `0x14BA6` sign/route decision, `0x149C0` route, and `0x15D3C` transform
  terminal. Its `0x14682` movement body now directly performs the reviewed
  direction-table probes, FF1700 collision tests, path writes, and route
  escalation.
- The movement route helpers `0x14902`, `0x14914/0x1494C`, and `0x14B4E`
  are now bus-backed. `0x149C0` directly handles the FF1700 obstruction
  walks, inline path cursor, route-bit clearing, target-coordinate emission,
  common B574 transform, and the `0x14A54` alternate route search.
- The shared `0x13EF4` tail and `0x13F76` resource gate are now direct. The
  type-specific `0x15CA4`/`0x15D3C` distance tests, B45C/+2E writes, global
  resource flags, and direct `0x1E2E0` queue call are preserved. `0x14D8E`
  now directly initializes its FF1380 scratch record, calls the reviewed
  fixed-point `0xE7C8` terrain sample, and performs the ten-record `0x15ABA`
  occupancy scan; its typed continuation remains available as an override.
- The `0xB1DC` animation-state prefix is direct through its ABFC delay,
  type-dependent Z80 command, linked direction setup, `0xC0A2` frame table,
  `0xAF6C` state tail, `0x15C84` link gate, and direct `0x55C76` entity
  reaction update.
- The `0x15C84`/`0x15C58` linked-animation predicate is also direct: it
  applies the +0x10 flag gates, uses the reviewed `0x15D3C` transform, and
  preserves the ROM's `< 0x200` distance cutoff and +0x34 direction write.
- The `0x1514E` resource-spawn path is direct through its `0x038AC` slot
  gate, generic `0x14FF8` output initializer, `0x55640` timing calculation,
  dual `0x15A8C` allocation, phase command tables, and final record copy.
  Its mode-2 `0x1E04A` branch is also direct through the seven-record
  `0x1DF00`/`0x1E0B8` flag passes, RAM descriptor list, `0x1A342` and
  `0x1A2CC` allocators, fixed `0x044BC` template/overlay layout, and
  projection/coordinate tail at `0x1E0DC`.
- The `0x15646` route-resource path is direct through the secondary record
  initializer, sprite-slot allocation, probe/coordinate tables, fixed
  interpolation branch, and linked `0x55848` random displacement branch.
- The `0xACAE` animation/reaction state machine is now direct through its
  lock, transition, `FFE18B/FFE18C`, controller-input, phase, and terminal
  branches. Its register-dependent callees use
  `EntityAnimationAcaeCallContext` (`D0`, `A1`, and carry), while typed
  defaults are used for the already-recovered `0x1DBAA` and `0x530DE` calls.
  Its `0xABFC` delay leaf is now direct as well, including the nested
  `0xD7EA` state-bit selection and `0xD81C` byte-average calculation. The
  `0xAF7C`/`0xAFAE` fallback tails and `0x1E514` cleanup leaf are direct too;
  the larger `0xAE6E` jump into `0x1033E` remains an explicit boundary.
- The self-contained `0x3A90`/`0x3B1E` frame-boundary leaves are translated
  as word-accurate horizontal/vertical status helpers, including borrow,
  `0x180` thresholds, and the `FFE173`/`FFE174` mode-byte branches.
- ROM `0x15DCC`, used by the `0x3A1C` frame transform, is now a direct
  fixed-point helper. It preserves the `((D2 >> 8) & 0xFE)` table index, the
  two `0x15E16` lanes, signed `MULS` behavior, and high-word results.
- ROM `0x15CA4` is now a direct longword-preserving distance reducer. It keeps
  the word-only axis differences, full `EXG.L` high-word behavior, and
  carry-based rounding after the smaller-axis shift.
- ROM `0x15D3C`/`0x15D4E` and the `0x3BAC` tail are now direct C++ boundaries:
  the signed major/minor reduction reads the `0x15DF6`/`0x15F96` tables,
  camera movement keeps the original word comparisons and edge gates, and
  changed axes dispatch to the direct `0x40E2`/`0x4018` tile refreshes.
- The packed-data primitive at ROM `0x1610` is translated in
  `include/shadowrun/compression.hpp` and `src/compression.cpp`. It handles
  literal, repeat, and overlapping back-reference records without assuming
  that the payload is graphics.
- The PRNG and bounded random helper at ROM `0x0D44`/`0x0D3A` are translated in
  `ShadowrunRuntime`; the bounded form preserves the Genesis unsigned 16-bit
  multiply and high-word result used by the recovered state tables.
- The controller handshake at ROM `0x19F2` is translated in
  `ShadowrunRuntime::initialise_controller_ports`; it preserves the Z80 bus
  request, two controller phases, inverted signature, and direct I/O/RAM
  addresses without embedding a frontend input model.
- The paired sampler at ROM `0x195C`/`0x1A48` is translated by
  `sample_controller_input`; it models both port signatures, rising-edge
  words, and the sixteen-entry decode table at ROM `0x19D2`.
- The low-level VDP paths at ROM `0x0EC0`, `0x0F1C`, `0x0F3C`, and `0x1910`
  are translated as bus operations; the latter uses a headless status bit so
  a future emulator/frontend can replace the deterministic ready state.
- The setup clear streams at ROM `0x1056`, `0x0FFE`, and `0x102A` are
  translated as exact VDP word/longword writes, using the command words at
  `FFDD16` and `FFDD14`.
- The shared ROM `0x0E9C` VDP command helper now preserves both control-port
  writes: `(D0 & 0x3FFF) | 0x4000`, followed by the low two bits of
  `ROL.W #2,D0`.
- ROM `0x1872` is represented by `ShadowrunRuntime::service_vblank`; its
  remaining game-specific callees are explicit `VBlankRoutineHooks`, while
  the flag/tick/RAM sequencing and reviewed VDP dispatches are executable now.
- ROM `0x12F6` and `0x131C` are represented by
  `fade_palette_to_target` and `fade_palette_to_black`. Each performs the
  exact seventeen `0x10D6`/`0x1088` palette steps, two `0x1342` wait/input
  passes per step, and sets VBlank bit 0 for the resulting upload.
- ROM `0x4618` now has a direct sixteen-record default. It walks `FF0100`
  with a `0x100` stride, projects active records through `0x5A56`, applies
  the reviewed `0x5FDC` timing update, and preserves the record boundary for
  `0x600E`/`0x60FA`; the normal `0x60FA` mask, packed-sprite, and VBlank-queue
  path is now direct as well.
- ROM `0x5B1E`, called by the `0x4658` post-pass, is translated as its exact
  sixteen-pointer signed descending sort. Records with `+0x10` bit 4 or 6
  are treated as zero, matching the source's flag branches. The surrounding
  `0x4658` dispatch now runs the reviewed direct emitters and exposes only
  its unresolved nested handlers explicitly.
- ROM `0x622C`/`0x624C`, another `0x4658` callee, is translated as the typed
  entity-state object-list emitter. It preserves the `0x8002` alternate-list
  marker, coordinate/flag adjustments, eight-byte output records, and the
  `FFDD02`/`FFDD18`/`FFDD1C` bookkeeping.
- ROM `0xAEBA`, reached by the same post-pass, is translated as the 32-row
  fixed-point object-list generator. It preserves the `FFDFEC` progress gate,
  `0xAF3E` source selection, `0x15E16` high-product interpolation, and calls
  the reviewed `0x1378` builder with the original bases and flags.
- ROM `0x4E38` and `0x4E64`, two `0x4658` timer helpers, are translated with
  their guarded six-bit frame phases, byte underflow behavior, and `+0x11`
  bit-7/bit-5 flag writes.
- ROM `0x5C7E`/`0x5D0A` is translated as the control-dependent object variant
  emitter. It preserves the `FFFC04` control tests and flag sets, four ROM
  table families, the `+0xA6` state gate, and the one-/four-list `0x1378`
  output contracts.
- ROM `0x4E86` is translated as the cycle/descriptor emitter. It preserves
  the timer-byte reset branch, `0x4F42` descriptor selection, `0x61A0` queue
  append, and the trailing `0x13BE` list with its `0x4F22` offsets.
- ROM `0x481E` is translated as the ten-slot object pass. It uses `0x5A62`,
  the signed/typed source-list selection around `0x48B2`, and the reviewed
  `0x1378` builder without introducing a renderer dependency.
- ROM `0x5936` and `0x59EA` are translated as the phase-list emitters. They
  preserve the byte timer/phase update, the four-list pointer table at
  `0x5A46`, and the paired `-0x2E/+0x57`, `-0x20` coordinate form.
- ROM `0x5662`, reached through the `0x557E` countdown gate, is translated as
  the timed descriptor/list emitter. It preserves the phase cutoff, the
  `0x5756` descriptor table, optional `0x61A0` append, random phase-four
  adjustment, and `0x13BE` output.
- ROM `0x5494` is translated as the second timed descriptor/list emitter. It
  preserves the `FFE004` phase gate, PRNG-derived `FFDFFE/FFE000` offsets,
  the `0x585E/0x586A` table choice, optional descriptor append, and `0x13BE`.
- ROM `0x56DE` now has a direct normal-path translation. It shares the
  `0x5756` descriptor table with `0x5662`, retains its phase-16 cutoff, and
  clears the indexed-object byte on cutoff, then executes the direct
  `0x55C2C` auxiliary state-grid update through typed `0x3674`/`0x37EC`
  helpers and the existing renderer-neutral `0x1232` streamer.
- ROM `0x3674`, `0x37EC`, and `0x55C2C` are translated in
  `src/entity_state.cpp`. Their signed byte phases, `0x6AB3E`/`0x6AB96`/
  `0x6ABEE` table choices, VDP command constants, `+0x10` bit updates, and
  `+0x3A` write are preserved.
- The proven leaf calculations used by `0x55C76` are also exposed in
  `src/entity_state.cpp`: `0x55E62` scans the eight inline lanes, `0x55E92`
  and `0x55EB4` subtract the `0xD4D50` table modifiers, `0x55ED6` computes
  the base byte, and `0x55F20` returns both the quotient and residual form.
- ROM `0x55C76` is directly executable as
  `run_entity_reaction_0x55C76`. It preserves the three `0x0DBA` threshold
  passes, `0x55740`/`0x55786` timing adjustments, the `+0x4C` versus `+0x9C`
  branch, and the `0xFFF0CE` frame-tick rollover.
- ROM `0xD81C` and `0x55F44` are directly translated as the entity scaling
  helper. It preserves the byte-wrapped base additions, signed scaling-mode
  branches, odd-value rounding, and negative-input cap at `0xFFFBBe`.
- The normal `0x52BE -> 0x5322` handler path is also typed. It uses the
  separate `0x5876` descriptor table, timer reset `6`, phase cutoff `9`, and
  the original phase/random Y adjustment. Its ten-record `0x52CC`
  post-cutoff scan now copies eligible phase records directly.
- The normal paths of `0x521E`, `0x508A`, `0x4FE2`, and `0x5152` now share a
  typed timer/descriptor/list helper while retaining their distinct ROM
  tables, flags, reset values, and phase cutoffs. The `0x4CEC` and `0x4D62`
  ten-record cutoff scans now execute their record setup and ordinary
  `0xAFF2` path directly. Their `0x530DE -> 0x52F3C` tail is translated as a
  direct bus-backed Z80 command-ring transfer; the `0xAFF2 -> 0xB330`
  animation fallback is also direct through its `0xABB6` checks and tables.
- ROM `0x52F3C`/`0x530DE` and the `0x52E96`/`0x52EA2` ring primitives are
  translated in `src/z80_transport.cpp`. The CPU/Z80 handshake remains on
  `GenesisBus`; the `0xFF,0x10,argument` packet, byte append, separator
  packet, modulo-`0x40` cursor, and `FFFC0A` gate are directly modeled.
- The `0x490A` entity dispatch is now bus-backed. Its reviewed no-op,
  `0x557E`, `0x56DE`, `0x52BE`, `0x521E`, `0x508A`, `0x4FE2`, `0x5152`,
  `0x4CDE`, and `0x4D54` table targets execute directly on their reviewed
  paths;
  the other state
  handlers remain reported as unresolved rather than being guessed.
- ROM `0x53C4` now has a direct gate/scan default. It runs `0x5494`, scans
  nine adjacent records, reproduces the `0x5468` child-state copy, and runs
  the ordinary matching-child `0xAFF2` animation initializer plus the
  ten-record `0x5622` cleanup. The `0xB330` fallback is direct as well.
- ROM `0x600E` is translated through its `0x62AC` pointer table, signed state
  index branches, `0x10`/`0x1C` timing multiplier selection, `0x8002` sentinel,
  and `record+0x12` pointer write. Its normal nested `0x60FA` body directly
  renders the `0x3EFE` mask, decodes the `0x3E86` packed sprite, and queues the
  record for VBlank; the hook remains an optional override.
- ROM `0x5D29C` now has a direct post-object state machine. Its four pointer
  tables, signed offset table, timer phases, upload buffers, background queue,
  and `0x5D416` command cases are bus-backed. The `0x15A8C` six-record
  allocator, `0x1A2CC` eighteen-slot allocator, `0x15930` table dispatch, and
  `0x5D6BC` template spawn are also direct; only a non-verified indirect
  handler is reported as unresolved.
- ROM `0x1342` is exposed as `service_frame_input`; its DBF counter is kept
  explicit, so the palette callers pass the original initial value of one and
  receive exactly two frame/input passes.
- The 64-entry palette transforms at ROM `0x1088` and `0x10D6` are translated
  in `include/shadowrun/palette.hpp` and `src/palette.cpp` as renderer-neutral
  word operations over the Genesis RAM palette buffers.
- ROM `0x1368` is represented by `PaletteAnimator::copy_target_from`, which
  copies the caller's 32 longwords into the target palette buffer at `FFD8FE`.
- ROM `0x1406` is represented by `run_resource_setup`. It follows the A6
  setup block, applies the referenced VDP/palette data, walks its 12-byte and
  14-byte sentinel lists, preserves all four bit-selected hook points, and
  supplies reviewed defaults for the `0x1232` and `0x1728` stream bodies.
- ROM `0x1232` is translated by `ShadowrunRuntime::stream_vdp_grid`, including
  its nested DBF counts, per-word addend, and `0x80` low-word command stride;
  the resource dispatcher uses it by default when no trace hook is supplied.
- ROM `0x1728` is translated by `ShadowrunRuntime::stream_delta_vdp`, including
  its base-word token decoder, delta classes, byte/word payload forms, signed
  output skip, row width/mask state, terminator, and per-row command stride.
- The mode-2 field resource chain is translated into native RGBA in
  `graphics.cpp`: the raw `0x166B82`/`0x6E20` pattern upload, the compressed
  `0x170102` 4x4 name-word stamps, and the 60x42 maps at `0x16D9A2` and
  `0x16ED52` are composed without retaining VDP name tables or palette indices
  at runtime. The extraction test covers the first field tile's packed-nibble
  pixels so the old planar interpretation cannot silently return.
- ROM `0x0F5C`/`0x0F98` is translated by
  `ShadowrunRuntime::stream_primary_vdp`, including its 64 KiB boundary split,
  64-byte/4-byte/2-byte copy tiers, and command offset behavior.
- ROM `0x1530` is translated by `ShadowrunRuntime::decode_compressed_vdp`,
  including its MSB-first control stream, literal/repeat/backreference forms,
  11-bit overlapping `FF9700` ring, lane-selected VDP words, and initial
  `0x8F02` control write. Its hook remains available for instrumentation.
- ROM `0x6148` and `0x61F4` are translated as the two VBlank upload-queue
  drainers. They preserve the 4-byte/8-byte queue strides, source selectors,
  `0x0F5C` calls, and counter clears; `service_vblank` uses these defaults
  when no instrumentation hook is supplied.
- ROM `0x0EE6` is translated as `service_object_list_upload`; it preserves the
  count sentinel, overflow-byte clear, `FFD97E` source, and `0x0F98` upload
  sizing, with the hook retained for instrumentation.
- ROM `0x2C92`/`0x2CB2` and their shared `0x3D18` tile-plane path are translated
  as the two default VBlank map uploads. The port preserves camera tile
  indexing, the 31-row loop, table-derived source offsets, split VDP spans,
  command stepping, and the `FFDF58` mask/XOR mode. The edge-refresh variants
  at `0x4018` and `0x40E2` share typed `0x3D4A`/`0x3DDE` span helpers,
  including the `0x8F80`/`0x8F02` VDP increment writes.
- ROM `0x5672C` is translated as `service_vblank_state_animation`, preserving
  its lock/state/mode branches, timer/index wrap rules, animation tables, and
  direct VDP control/data writes.
- The object-list builders at ROM `0x1378` and `0x13BE` are translated in
  `include/shadowrun/object_list.hpp` and `src/object_list.cpp`; they preserve
  the `$8000` sentinel, eight-byte output records, ordinal assignment, the
  variant-specific word-3 offset, and explicit object-count/next-output state.
- A bus-backed form now mirrors the original `A2`/`A3` pointer loop and writes
  the live `FFDD02` count, `FFDD18` output pointer, and `FFDD1C` count mirror.
- ROM `0x0E34` is represented by `initialise_object_output`; it emits the
  two zeroed VDP staging writes and initializes `FFDD18`, `FFDD1C`, and
  `FFD97E` at the same observed addresses.
- The reviewed `0x4ADC` object update is composed in `object_update.cpp`: it
  handles the timer-expiry byte scatter and the normal projection plus
  descriptor/list path, while requiring a host sentinel bound for malformed
  input safety.
- The single-record fallback at ROM `0x61A0` is also translated as a bus
  operation; it copies the selected descriptor into the `FFDF32` output stream,
  derives its final word from source `+0x2A << 5`, and advances the ROM's
  pointer/count fields.
- The entity projection routine at ROM `0x5A62` is translated in
  `include/shadowrun/entity_projection.hpp` and `src/entity_projection.cpp`;
  it preserves 16-bit subtraction, signed arithmetic shifts, bit-3 rounding,
  screen-coordinate writes, and the field visibility bounds without assuming a
   renderer or prematurely naming the source fields.
- The alternate projection routine at ROM `0x5A56` is represented separately;
  its `0x15390` record-selector branch keeps the same fixed-point transform,
  uses the wider nonnegative X/Y bounds, and returns the computed X word.
- The ten 18-byte entity slots at RAM `0xFF164C` (stride `0x12`) are represented
  by `RawEntitySlot`; the active-slot projection phase from ROM `0x481E` is
  available as a bounded, renderer-neutral pass, including its type-specific
  list selection and `0x1378` output.
- `GenesisMemoryBus` in `include/shadowrun/memory_bus.hpp` provides a
  transitional ROM/RAM/I/O verification space and records VDP port writes;
  native code uses `ScreenWriter`/`InputState` instead of requiring that bus.
- The initial vector/direct pass reached 332 function entries and 6,342 decoded
  instructions. Correcting immediate bit-op widths and following the reviewed
  dispatch paths now yields 717 direct-analysis entries and 18,803 decoded
  instructions. The retained Ghidra cross-check still finds additional likely
  indirect-dispatch functions; those are being recovered from tables rather
  than copied into the C++ port by name.
- Reviewed indirect tables at `0x12CCB4`, `0xF3DC6`, `0xF3E1E`, and `0x490A`
  are represented in `rom_layout.hpp`; the direct analyzer seeds their code
  targets and labels them by table/index. The C++ port now executes the
  reviewed `0x490A` no-op, ordinary emitter, and wrapper subset through
  `0x557E`, `0x56DE`, `0x52BE`, `0x521E`, `0x508A`, `0x4FE2`, `0x5152`,
  `0x4CDE`, and `0x4D54`, while reporting the remaining handlers explicitly.
  The current regenerated analysis is 717 function entries and 18,803
  decoded instructions.

## What remains for the C++ reconstruction

The next work is manual reverse engineering, not another blind decompiler pass:

1. Separate code, tables, tiles, maps, scripts, and text from false-positive
   function starts.
2. Recover the global RAM layout and replace address-named globals with typed
   state structures.
3. Name subsystems from call-graph and hardware evidence: startup/region,
   VDP rendering, controllers, timing/interrupts, audio, map/field logic,
   menus, combat, and persistence.
4. Convert VBlank graphics work into native screen/tilemap/sprite operations,
   extract ROM graphics into RGBA8 assets, and connect Windows controls through
   `InputState`.
5. Compose the verified update systems into the native executable, then
   validate its frames and behavior against emulator traces without making an
   emulator or 68000 interpreter part of the runtime.

The generated decompiler files provide coverage and addresses for that work;
they should not be mistaken for proof that the original 68000 program has
already been recreated in portable C++.
