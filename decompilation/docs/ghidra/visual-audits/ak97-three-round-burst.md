# Three-round weapon burst review — 2026-10-14

## ROM trace and defect

The US ROM dispatches weapon action codes 8–10 through the special record
branch at `0x1514E`. `0x152FA` initializes the primary record with
`+0x41 = 2`, saves its integer origin at `+0x58/+0x5A`, and sets the special
spawn fields `+0x8B = 1` and `+0x89 = 0x20`. The initial shot plus the two
follow-ups therefore consume three rounds.

After a terminal projectile's finite animation list completes,
`0x14E0A`/`0x14E32` leave the primary stopped and `0x14F1C` dispatches to
`0x14F86`. The latter decrements signed `+0x41`; a negative result retires the
primary. Otherwise it selects `+0x16 = 0x14`, resets `+0x89 = 0x20`, reruns the
`0x55640` action-pair calculation, restores the saved origin while preserving
fractional coordinates, updates heading, clears the finite-animation state,
and derives new offsets through `0x150E8`. The native port derives a fresh
weapon success count from live runner facts. The resulting headings are
the initial aim, initial aim minus `0x400`, and initial aim plus `0x400`.

The ROM table at `0x15E56` has 128 direction samples; `0x15E16` is its
96-entry rotation. The source-table audit verified all entries, including
entry 111 (`0xA1A2`), which is slightly asymmetric with its companion sample.
That table remains evidence for direction resolution and step scale. Lua
represents bearings in degrees, positions as map-space points, velocity as a
map-space vector, and flight allowance as a distance. The field adapter
translates those values to and from the record's fractional storage. Projectile
movement derives velocity from the bearing with native trigonometry; the
runtime does not read the ROM direction samples.
Runtime QA checks the semantic vector at the burst headings and at the heading
corresponding to source entry 111.

The first connected Scene 2 AK-97 QA failed before the fix: it spent all three
rounds and hit the live Weenr, but captured zero `burst_followup_started`
records and retired its projectile. The typed special-spawn helper had copied
the saved origin and flags but omitted the ROM's `+0x41 = 2`, so the first
stopped pass decremented zero and retired immediately. `prepare_special_spawn_record`
now sets that field, and the existing entity-transition test asserts it.
Lua restarts the projectile from its semantic burst origin, applies the
source-derived angular changes as degree bearings, and calculates a native
vector step from that bearing. It obtains a fresh weapon success count through
the Lua combat resolver. The field runtime applies that state to the live
projectile record.

## Native state representation

Lua's projectile state uses `remaining_flight_range`, `shots_remaining`, and a
named flight phase. The C++ field adapter converts the source clear-step
counter into map-space distance before Lua sees it, then translates the
returned distance back at the record boundary. A clear probe consumes one
96-map-unit movement increment; the source zero-counter check becomes a
range-exhaustion check. The source linked-action byte becomes a count of shots
still to start. Lua contains neither the source flight counter nor its signed
decrement/wrap logic. The weapon-hit rule still applies the source's signed
arithmetic to the difference between the two success counts.
The pre-split differential fixture retains the old arithmetic only behind a
test adapter that translates between its frozen inputs and gameplay state.

## Visual and state review

The Release field QA writes whole-screen 320×224 captures under
`decompilation/ghidra_conversion/build/conversion/qt/release/qt_runtime/field-random-encounter-spawn-qa/`.
The relevant files are:

- `hit-response/before-live-hit.bmp`
- `hit-response/first-warhawk-ray.bmp`
- `hit-response/hit-response-frame-0.bmp`
- `hit-response/hit-response-frame-1.bmp`
- `hit-response/hit-response-after-eight-services.bmp`
- `hit-response/first-burst-ray.bmp`
- `hit-response/burst-followup-1.bmp`
- `hit-response/burst-followup-2.bmp`
- `hit-response/burst-settled.bmp`

All nine frames were reopened at original resolution. The right HUD remains
visible, and the hit/burst route does not leave dialogue, field interaction,
hotel, or command panels over the field. The capture shows the one-round
Warhawk hit and the three-round AK-97 route on the same scene. QA checks ammo
`6 → 5` and `30 → 27`, two continuation records (`+0x41` values 1 then 0),
restarted bearings/vector offsets in Lua runtime tests, and eventual primary-record
retirement. This is internal screen and state evidence; the renderer uses the
same imported art under review.

The longplay contact sheets for sections 04, 06, and 25 contain field combat
and target-selection frames, but none of the reviewed frames identifies an
AK-97 or isolates its three-ray sequence. No independent Genesis frame is
claimed as proof of this burst's exact projectile art, placement, or cadence.

## Seven-question visual verification

1. **Does it look correct?** The complete field and HUD remain composed in all
   nine captures. That only establishes that the connected route does not
   corrupt the surrounding frame; the projectile appearance still lacks an
   independent matching Genesis frame.
2. **Does the look reflect behavior?** Yes for the checked state: the ROM
   ammo service consumes three AK-97 rounds, the primary exposes two
   follow-ups, each restart uses a new heading/action result, and the primary
   eventually retires. The separate Warhawk route reaches the actual Weenr
   meter and C072 reaction.
3. **Are the visuals correct?** The test checks complete 320×224 output,
   field HUD survival, modal absence, and the finite projectile frames. It
   does not prove those pixels match a Genesis burst capture.
4. **Is it animated, and when?** Yes. The source finite-list update uses the
   `0x5FDC` accumulator and `0x600E` completion marker with the projectile
   descriptor cadence. Each nonterminal `+0x41` pass resets the Lua animation
   frame/accumulator before the next ray.
5. **What sound accompanies it?** The source spawn cycle submits one firing
   event from the `0x15318` table: `0x1E`, or alternate `0x4A` when the
   equipped item's source status bits select that table. The test requires
   exactly one matching request. Terminal impact still requests `0x3C` through
   the source branch. The natural playback duration was not independently
   captured in this QA.
6. **Are the rest of the screen and old controls clean?** Yes for this field
   route: the side HUD stays intact and no menu/dialogue surface remains open
   over it. This is not a menu-page redraw claim.
7. **Are there multiple frames?** Yes: pre-shot, first ray, two follow-up
   boundaries, settled field, and the Warhawk's two C072 frames plus its
   eight-service checkpoint. They support the state progression internally;
   the independent longplay does not show the exact AK-97 sequence.

## Verification

- Before fix: Release `shadowrun_ghidra_conversion_qt_field_random_encounter_spawn_qa`
  failed with `followups=0`, `ammo=27`, and `primary_slots=0`.
- After fix: Release and Debug `shadowrun_ghidra_conversion_qt_field_random_encounter_spawn_qa`
  pass; the test checks the live Scene 2 player action, not a detached Lua
  helper.
- After fix: `shadowrun_gameplay_script_runtime_qa` passes in Debug and
  Release, including both follow-up bearings, semantic vector offsets, saved origin,
  counter, selector, fresh action result, and the questioned ROM table entry.
- After fix: `shadowrun_ghidra_entity_transition` passes in Debug and Release,
  including the `+0x41 = 2` special-spawn assertion.

The broad Release workbench build also attempted unrelated targets and stopped
at existing `recruited_runner_conversation_conversion_test.cpp` references to
missing `option_keys`/`text_key` fields. The directly affected conversion test
and active Qt targets built and passed in both configurations.

The behavior is connected and source-backed. Independent visual and audio
duration confirmation remain open; no Genesis visual-defect closure is claimed.
