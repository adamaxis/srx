# ROM-to-native audit

This is the working audit for the native rewrite. The Ghidra export remains
the evidence corpus; none of the entries in this document authorize ROM
address reads, a 68000 interpreter, or a VDP-shaped runtime.

## Current source evidence

| Evidence | Current count | Meaning |
| --- | ---: | --- |
| Exported function entries | 910 | Canonical Ghidra function inventory |
| Resolved direct call sites | 2,986 | Calls with a concrete target in `call-sites.tsv` |
| Indirect/unresolved call sites | 16 | Explicit dispatch boundaries still needing table/handler proof |
| Generic export labels | 0 | All 119 formerly generic exports now have reviewed semantic names |
| Reviewed generic labels | 119 | Per-entry behavior/evidence ledger in `ghidra-function-label-review.tsv` |
| Conversion implementation files | 229 | Leaf source-trace contracts, not proposed runtime files |
| Behavioral conversion modules | 28 | The reviewed ownership boundaries in `conversion-source-modules.tsv` |
| Conversion tests | 230 | Runner plus focused source-trace tests; all currently pass |

The generic-label naming backlog is closed as a naming task. It is not a
behavioral-fidelity claim: the review ledger records confidence and evidence,
and the unresolved/partial behavior remains tracked below.

| Review family | Reviewed labels | Remaining verification question |
| --- | ---: | --- |
| pre-title/scene effects | 40 | Verify stage ownership and effect-table targets |
| action/combat | 18 | Verify record fields and indirect action handlers |
| menu/field presentation | 37 | Verify concrete menu ownership and table selectors |
| entity/world | 16 | Verify entity/effect dispatch and world activity tables |
| interrupt/field callbacks | 8 | Keep reserved/no-op boundaries from becoming runtime systems |

Run the structural audit with:

```text
powershell -NoProfile -ExecutionPolicy Bypass -File decompilation/tools/verify_rom_conversion.ps1 -Strict
```

The current audit passes export-name, status-ledger, source-file ownership,
and evidence-file checks: 910 exported entries, 932 unique ledger rows, 2,986
resolved direct call sites, 229 conversion implementation files, and 230 CTest
cases. The behavioral fidelity gate remains OPEN: 16 indirect/unresolved call
sites are still explicit boundaries, 928 ledger rows are partial, and only 4
are defended. The generated Ghidra C also omits meaningful disassembly in
0x104F6 and 0x12C74. Their typed conversions now carry the recovered slot-7
phase tail and action-class skill selector respectively; the ROM disassembly
remains the tie-breaker for these generated-C omissions.

## Instruction-level conversion audit: 2026-09-26

`decompilation/tools/audit_rom_conversion.ps1` is the source-fidelity audit
layer. It indexes every instruction in `export/disassembly.s` against the
exported function spans, compares each exported function's resolved direct
calls with its generated Ghidra-C body, records explicit ROM-address evidence
in the typed conversion sources, and joins the result to
`docs/decompiled-function-status.tsv`. Its report is
`docs/rom-conversion-audit.tsv`, one row per exported ROM entry. This is a
review queue, not a claim that a generated-C body or a passing structural test
is behaviorally complete.

The indexed pass currently reports 910 ROM entries, no missing disassembly
spans, 18 direct-call substitutions needing native-owner review, 27 generated
Ghidra-C return-only omission risks, 20 linear adjacent-export boundary
reviews, and 284 entries without an explicit ROM address in a typed conversion
source. The boundary audit now separates true linear continuation from
adjacent exports reached by an explicit branch/call; the latter remain in the
row-level report as context instead of being misreported as fall-through.
These are remaining conversion-evidence sweeps, not permission to invent a
replacement.

The slot-availability audit also corrected a semantic narrowing in the typed
conversion: `0x19C54 -> 0x19C5C` preserves the raw `FFFBCC` byte because the
source callers use it as a numeric action/HUD value. A nonzero boolean is not
an equivalent representation for that boundary.

The first concrete omission slice recovered from the ROM is now represented
as typed conversion code:

* `0x566A2` copies five 0x22-byte action-profile rows from ROM `0x166AD8`
  into the working table at `FFF9FA`. The conversion decodes the event rows
  and their mutable primary/secondary modifier fields.
* `0x55828` follows the current actor's linked record at `+0xD8`, reads its
  profile index at `+0xA0`, and selects the indexed working-table row. The
  native boundary returns unresolved for an absent/out-of-range host row;
  it does not silently use a guessed modifier.
* `0x3602` computes the HUD glyph attribute word (`0x2000` or `0x6000`) from
  the ROM's raw-byte `CMP.B` sign-bit branches for action/class and the signed
  equipped-slot field. Ordinary, wrapped high-bit, and sentinel cases are
  covered by a focused conversion test; transfer integration into the menu
  renderer remains open.
* `0x60F4` is not a no-op: the ROM executes `ORI #1,CCR` and returns with the
  carry flag set. The typed conversion exposes that carry result; the caller's
  descriptor branch still needs its complete native integration.

The same pass caught a caller-sensitive fall-through error in the world-effect
timing helpers. `0x562D4` has no return before `0x562DE`, so its callers receive
`base + FFE169 + (2 * FFE16A)`; callers that branch directly to `0x562DE`
receive phase-only timing. The conversion now models those as separate paths,
and the action-count and transition tests use non-zero timer/phase values to
verify the distinction.

The follow-up audit of `0x562EC` and `0x563CC` found no additional control-flow
loss: the transition path preserves the source signed-byte difference tests,
the footer callback order, and the phase-only difficulty entry, while the
pending-roll path preserves the `BLE` reduction gate, wrapped byte subtraction,
depletion clear, and reset callback. The new tests also exercise 16-bit timing
wrap at the modifier helpers and the `CMP.W D6,D7`/`BPL` predicate in `0x56506`.
That predicate is intentionally not an unsigned `std::max`; the ROM tests the
sign bit of the wrapped word difference before adding the availability value.

The adjacent resource-copy audit found a host-layout error in the mode store.
The source destinations at `0xFFD87E`, `0xFFD89E`, `0xFFD8BE`, and `0xFFD8DE`
are four contiguous eight-longword banks. The typed store now exposes all 32
words: `0x566E2` targets bank one, `0x566F6` targets bank three, and `0x5670E`
starts at bank one and fills banks one through three. The previous 24-word
model incorrectly shifted these copies and treated the bulk refresh as if it
started at bank zero; the focused state-copy test now guards each untouched
bank boundary.

The `0x56AC0` field-mode loop had a separate control-flow correction. Its
direction branches at `0x56CB6`/`0x56CD4` return directly to the vblank/sample
loop, so only the first matching directional bit is consumed from a sampled
controller word. Confirm, alternate-confirm, restart, and return handling
must wait for a later sample. The native loop now preserves that priority and
has a simultaneous direction-plus-confirm regression case.

The Matrix storage/timing pass found two more width-sensitive conversions.
The audio command-ring audit also removed a source-shaped byte buffer from the conversion model. `0x52E96` now publishes a typed `flush_marker` entry followed by a typed command entry, while `0x52EA2` appends a typed command and wraps a host index at the source 64-entry capacity. The source `0xFF` marker remains provenance; it is not a runtime sentinel in the host model. The opaque audio firmware image is still an extraction boundary and is not being misrepresented as typed gameplay state.
`0x19558` and `0x1957E` accumulate and subtract in word registers, so their
typed capacity results now wrap at 16 bits. `0x195AA` copies the selected
footprint through `D1.w` before division and uses `CMP.W #7`/`BMI`, so the
conversion no longer lets a wider host footprint or unsigned cap comparison
change the source result.

The movement pass found the same width issue in the shared distance leaves.
`0x15CA4` subtracts with `SUB.W`, normalizes with `BPL`/`NEG.W`, selects the
major axis with unsigned `CMP.W`/`BCC`, and finishes with `ASR.W`, carry-based
rounding, and `ADD.W`. `0x15CC0` loads two record point pairs and repeats that
word sequence. The typed helper now truncates host deltas to source words and
returns the wrapped word metric; adversarial tests cover values outside the
word range and the unrepresentable `0x8000` negation case. This is a source
fidelity correction, not a new movement rule.

The world-effect animation audit found an analogous byte-width boundary at
`0x1AAAC`. The ROM performs `ADD.B D1b,D0b`, then uses `BPL` on the wrapped
frame byte before storing it. The conversion now preserves that operation
instead of adding the signed table word in a widened host integer; the focused
test uses a wrapped high-bit result to distinguish the two behaviors.

The next manual slice corrected three source-owned details. The field-column
renderer at `0x3D4A` emits a partial segment and then continues through the
shared `0x3D80` transfer body; it is now represented as two typed host
segments when the source is not aligned, rather than emitting only the tail.
The Matrix storage conversion now reads the twelve footprints from the ROM
`0xE2DF2` table as immutable conversion data instead of accepting a caller-
supplied replacement. Finally, the audio `0x52FD2` inner entry no longer
requires a second bus-begin callback: its caller opens the bus before entering
the marker-plus-two-payload sequence, matching the ROM call boundary.

The world-service trace corrected three generic conversions. `0x1DC22` first
checks the selected bit in the ten-record `FF0100` entity table and only then
enters `0x1DC4C` with the `0x678CC` cursor sentinel; it is not a circular
eligible-entity iterator. `0x1DC4C` now models the source's normal and special
candidate starts, descriptor-class fallback to `FF0400`/`FF0100`, bounded
eight-record forward scanning, previous-target release, and the asymmetric
normal/special publication of `+0xAC`, `+0xA2`, `+0xA6`, and `+0x43`. The
adjacent `0x1DEA0` pass now clears only records with `+0x10` bit 6 set, bit 3
clear, and a zero `+0x0C` action word before invoking the clear-slot boundary.

The field-action grid family is now one grouped conversion instead of five
generic leftovers. `0x57C3A`, `0x57C46`, `0x57C52`, and `0x57C5E` select the
`0xC604/3`, `0xC184/12`, `0xC904/2`, and `0xC484/6` destination/row presets;
all enter `0x57C66`, which repeats the 31-word row at `0xC1320` with `0x80`
destination strides. The detail panel is six rows, not five: the source uses
`D1 = 5` with `DBF`. The host contract requires the complete 31-word source
row and emits typed transfer events for each repeated row.

The `0x57026` detail-purchase trace also retains the category-3 special gate:
after the selected category bit is checked, `0x570EC` rejects the purchase if
the companion byte already has bit 0 set. The typed purchase conversion and
focused test now preserve that rule; the earlier test fixture had incorrectly
treated it as a valid purchase.

The adjacent `0x571B2` equipment-upgrade scan likewise had a missing terminal
bit. For categories 7 and above, the ROM scans bits 12 through 15; bit 15
causes the exhausted/unavailable branch at `0x5723C` rather than resolving a
new target. The typed resolver now scans all four source bits.

The recovered `0x577BA` quantity-purchase path also had a mutation mismatch:
its invalid-category branch writes `FFF112 = -1` and returns without touching
the negative/armed `FFF0B3` availability marker. The typed conversion and
tests now preserve that marker on the early exit.

The neighboring `0x5766A` resource-purchase threshold also required a flag-
semantic correction. Category 3 uses `CMP.B #3,(A0)` followed by `BMI`, so the
host predicate must inspect the wrapped subtraction's N bit. A progress byte
of `0x80` yields `0x7D` and follows the source category-limit path; treating
the byte as a signed C++ value incorrectly attempted a purchase.

The field-action cost helper at `0x57B04` had the same class of conversion
risk. The runner margin uses `SUB.B`/`BLE` before a word-level clamp. The native
code now evaluates those source flags explicitly, and the `0x00 - 0x80` margin
case is covered by a focused test. The neighboring `0x5787C` quantity helper
was checked as well; its preceding `TST.B` gate already makes its typed
category condition equivalent over the full byte domain.

The `0x5794E` special-action capacity branch also had a missing blocking
boundary. After incrementing and restoring the `0x14` visible count, the ROM
refreshes the detail panel, displays the limit feedback, and enters the shared
controller wait before returning. The typed conversion now preserves that
callback order.

The result-zero field-action branch at `0x56E5A` was also checked against its
saved-selection matrix. Selections 0, 1, and 2 choose the dynamic, selection-1,
and selection-2-plus offer families and all continue into the shared action
application path; only selection 3 branches to removal. The typed progression
purchase no longer rejects the two fixed offer families as if they were
unreachable.

The neighboring `0x573FC` resource-record purchase was checked for the same
presentation and flag boundaries. Insufficient requirement and insufficient
funds both enter the source controller wait after their message. The primary
and secondary capacity replacements use the wrapped word `CMP`/`BMI` result;
they are not plain unsigned greater-than-or-equal comparisons.

The Matrix-resource install leaf at `0x5751A` was then checked. Its quantity
gate is `CMPI.B #9/BPL`, so the native conversion preserves the wrapped-byte
flag behavior instead of using an unsigned `>= 9` test. Invalid quantity and
resource records publish the source service-result marker, while both funds
and storage failures enter the controller wait. The `0x89` wrapped quantity
case is covered with a fixture that also preserves the source word-capacity
branch.

The adjacent field-action description leaf at `0x580FE` has a different
quantity branch: `CMPI.B #9/BMI`. The native conversion now tests the wrapped
subtraction's N bit directly. Consequently `0x80` selects the highest-version
notice (`0x80 - 9` has a clear sign bit), while `0xFF` still selects the
quantity/capacity summary. The focused presentation test covers this case;
using `int8_t(quantity) < 9` would misclassify `0x80`.

The shared confirmation prompt at `0x57B8C` was also checked through its
`0xC376` child. On movement bits 8/9 the ROM services the move sound before
the wrapped 0/1 cursor writer; only after that branch is skipped does it
inspect exit bits 12/14. The native callback sequence and focused test now
preserve that ordering.

The adjacent `0x57C7A` field-action dispatcher was audited next. Its saved
selection and primary-cursor cutoffs are byte `CMP` instructions followed by
N-only `BMI/BPL` branches. The native plan now preserves those wrapped flags
for negative, neutral, resource-description, and quantity/no-action routing;
high-bit bytes no longer get reinterpreted through host signed ordering. The
`0x5825C` cost-summary leaf received the same treatment for its byte row gate,
and for its long `CMPI.L #0x10000/BPL` cost split. Focused tests cover the
overflow-sensitive paths.

The adjacent field-mode presentation routine at `0x587DA` was audited against
the same byte-flag rule. Negative cursor values select the dedicated source
tables at `0x1AAC86`, `0x1AADBF`, and `0x1AAD77`, cursor 3 takes the
six-plus-two grid batch, and later cursors take the glyph-row transfer. In the
zero-result path, cursor 0 selects the dynamic action table after
`CMP.B #1`/`BMI`; it does not use the glyph row. The native conversion keeps
the `CMP.B #2`, `CMP.B #3`, and `CMP.B #1` N-only boundaries as wrapped byte
flag tests, with focused coverage for ordinary and high-bit cursor bytes.

The helper audit also exposed the data formerly hidden behind generic counts.
`0x58A2E` uses marker `E` as its grid sentinel and otherwise transfers glyphs
with the source blank/high-plane and `CMP.B #0x0F` palette split. `0x58AA4`
reads eight active-runner item bytes at `+0x66..+0x6D`, maps nonzero values to
the `(item_code - 1)` item-definition row, and uses the blank asset for zero
before its result-selection-4 text call. `0x58B5E` and `0x58BB6` now expose
typed result-specific plans for their panel assets, write counts, grid/text
families, and cursor strides; the legacy callbacks remain only as host
presentation boundaries.

The source call order around `0x58D32` is explicit as well: it derives the
primary-cursor `* 0x20` sprite-record row, builds the three-record selection
set with the `0x4000` sprite attribute, publishes the sprite/display dirty
markers, and then calls `0x57C7A` to redraw the field action selection. That
action-selection callback is distinct from `0x587DA`, which the outer service
loop invokes afterward; alternate-panel cursor movement now preserves both
boundaries instead of stopping after the sprite build.

The generated Ghidra C remains an evidence snapshot and still omits meaningful
logic for some of these entries, including `0x3602` and `0x55828`; the typed
conversion is accepted only where the ROM instruction trace and focused test
agree. A fresh headless Ghidra regeneration is still unavailable in this
checkout, so this audit does not close the behavioral-fidelity gate.

## Indirect dispatches that still need source proof

These are not missing implementations to be filled with guessed handlers. Each
one needs its table selector, table contents, handler contract, and native
owner identified from the ROM and its callers:

| Caller | Call site | Native boundary |
| --- | --- | --- |
| `run_game_loop` | `0x0B42` via `A1` | Main scene/mode dispatch |
| `update_entity_frame` | `0x46F6` via `A1` | Entity kind dispatch |
| `dispatch_menu_handler` | `0xC444` via `A0` | Concrete menu action handler |
| `run_inventory_actions` | `0x11884` via `A0` | Inventory action table |
| `dispatch_entity_effect` | `0x15928` via `A2` | Entity effect/action service |
| `service_active_effect_slots` | `0x1640A` via `A1` | Active world-effect handler |
| `service_matrix_program_action` | `0x19A1C` via `A4` | Selected Matrix program action |
| `dispatch_title_phase_handler` | `0x1B892` via `A1` | Title/world phase handler |
| `initialize_world_effect_selection` | `0x1E728` via `A0` | Effect-selection policy |
| `dispatch_world_effect` | `0x1EF28` via `A1` | World-effect handler |
| `run_mode_exit_services` | `0x1F26E` via `A1` | Mode-exit service list |
| `run_mode_text_prompt` | `0x1F58A` via `A0` | Mode-specific text prompt |
| `dispatch_indexed_handler` | `0x24E32` via `A0` | Indexed world/event handler |
| `run_indexed_title_services` | `0x53AA4` via `A1` | Title service sequence |
| `run_mode_action_loop` | `0x54960` via `A0` | Mode action handler |
| `advance_title_intro_schedule` | `0x5AF48` via `A1` | Intro stage schedule entry |

Known table families to resolve against those calls include `0x12CCB4`,
`0xF3DC6`, `0xF3E1E`, `0x490A`, and the table families around `0x1D751E`,
`0x1DCB0C`, `0x1DD216`, `0x1DD98C`, and `0x1DDE16`. The native result should
be an enum/policy/variant visitor, not an address-indexed function table.

## Data still to pull into typed native data

These are the data families that game behavior consumes. Graphics extraction
alone is not sufficient:

| ROM family | Native data boundary | Status |
| --- | --- | --- |
| `0x7291A`, `0x72C1A` | Archetype defaults and ten recruited runner templates | Confirmed fields; recruitment writers remain to trace |
| live `0x100`-byte runner record | `RunnerRecord` / `RunnerRoster` | Confirmed consumed offsets; opaque bytes remain until a consumer is proven |
| `0xD4540`, stride `0x30` | `ItemDefinition` inventory/equipment records | Confirmed initial fields; complete category/effect mapping still needed |
| `0xD4D50`, `0xCF3D4`, `0xCF610` | Weapon, armor, attack, defense, and timing tables | Consumers identified; full typed extraction still incomplete |
| `0xD2072` | Cyberware names/effect slots | Names extracted; effect semantics still need rule consumers |
| `0xC1A2A` | Attribute/skill labels and definitions | Labels extracted; upgrade/derived-rule data still needs a typed table |
| `0xD4EA0`, reached by `0xFBA6`/`0x1088C` | Spell definitions, availability, cost, force/effect rules | Explicit remaining extraction task |
| field mode/terrain/entity/event tables | `AreaResource`, `TerrainMap`, `EntityTemplate`, event/exit records | Modes and terrain partially typed; all event/transition records not yet pulled |
| `0x59318` and `0x597EE..0x598B0` | `SaveStore` compact records and projections | Projection behavior converted; complete save data package not yet native |
| text pointer/catalog families | `TextCatalog` / dialog/menu/notebook data | Several screens extracted; complete catalog ownership still under audit |

## Native integration order

The executable can now boot from direct assets and reach the field through the
native controller. The next runtime slices should be pulled from the audited
boundaries in this order:

1. Replace the historical `NativePortShell` name with the subsystem-owned
   `NativeGameController` implementation files while preserving one session
   state owner.
2. Introduce `ExtractedGameData`/`RulesDatabase` for item, runner, field,
   text, and spell records. Keep it separate from the image/audio manifest.
3. Integrate the defended field/entity contracts into `FieldWorld` and
   `EntityWorld`, then add independent visual dumps and control tests.
4. Resolve menu dispatch and record mutation by concrete menu ownership;
   inventory, equipped gear/armor, first aid, clips, cyberware, attributes,
   and magic must not share a generic action bucket.
5. Resolve the Matrix tables into `CyberdeckSystem` policies.
6. Finish combat/effect handlers and save/load before calling the native game
   broadly playable.

Every completed item must have a ROM trace, a typed native owner, a focused
test, and (for a screen) an independent image dump. A renamed generic wrapper
does not close an audit item.
