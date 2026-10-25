# Scene-2 event-1 route to scene 26

Date: 2026-10-07

**Status:** partial. Normal field input reaches scene-2 collision record 93,
publishes event 1, selects scene 26 through the active map table, and loads a
resource activity. The route's uninterrupted eastward movement exposes an
unresolved runner/HUD overlap in the native screenshots. Independent longplay
frames support the scene-2 map and HUD at the spawn, but no capture of this
exact event-1 route or scene-26 entry has been identified.

## ROM trace and native state

The New Game scene-2 entry position is `(0x3CE0, 0x2D00)`. Collision record 93
at ROM `0x17314C` is a `0xC1` doorway rectangle at
`x=0x06E0..0x06FF, y=0x0220..0x023F`; its event value is 1. Record 47 at
`0x172F80` is a `0x7F` wall across the same y band, from
`x=0x06A0..0x073F`. The route follows the open south lane and enters record
93 from below. It does not weaken collision or write the runner position.

Scene 2's event table begins at ROM `0x172CDC`; row 1 selects scene `0x1A`.
The normal-input contact reaches `0xB7B8`, publishes event 1 after the
negative collision probes, and continues through the world-activity selector
and resource-scene preparation. Scene 26 is loaded in resource-activity mode
with random bound 3, a selected index within that bound, and 133 collision
rectangles. The route does not use Scene 26's static profile positions as its
entry placement. The scene profile table at ROM `0x3232` points Scene 26 to
`0x1D97EC`, whose static runner positions include three `(0x0A00, 0x0500)`
values. But `0x167AE` dispatches this destination through the resource branch
at `0x16950`; `0x169E6` publishes the resource descriptor's `+4` placement
pointer to `FFE114`. The pointed words at `0x1D9916` are camera
`(0x4400, 0x2E00)` and runner positions `(0x4C00, 0x3300)`,
`(0x4C00, 0x3500)`, `(0x4C00, 0x3700)`. Scene-profile selection at
`0x2B78` clears that one-shot pointer and applies those words; only when
`FFE114` is zero does `0x2B3E` copy the static `FFDE08..FFDE12` positions.
The route holds the destination stable for 30 further field updates.

The relevant ROM bytes are independently present in
`decompilation/input/shadowrun_genesis_us.bin`: event-table row 1 at
`0x172CDD` is `1A`; `0x1D982E` contains the static `0A00 0500` triplet; and
`0x1D9916` contains `44 00 2E 00 4C 00 33 00 4C 00 35 00 4C 00 37 00`.
The code path and the one-shot/static split are visible at
`disassembly.s:13542-13651` and `disassembly.s:1691-1720`.

The geometry is a strong crosswalk to Redmond map marker 8, Little Chiba's
Chop Shop. ROM contract-location data independently names Redmond area-2
location index 1 “Little Chiba.” The event table still provides only numeric
resource scene `0x1A`; neither source labels the scene-26 map an office. The
map's exact interior identity and any site-specific speaker/dialogue remain
unverified.

### Johnson run target at Little Chiba

ROM `0x1EEDC` dispatches the active run handler before the ordinary indexed
field-entry route. In the run table at `0xF3E56`, canonical type 4 selects
`0x1F046` and type 5 selects `0x1F106`. Both handlers compare the current
map/event pair (`FFFBF8/FFFBF9`) with the active target (`FFFBEE/FFFBEF`). A
match selects the corresponding entry passage (`0xFDE0A` for type 4,
`0xFDE2E` for type 5), then the field exit resumes the map's event-table
destination after the dialogue closes. Little Chiba's pair is map 2/event 1,
and its event-table destination remains scene 26.

The physical QA seeds a type-4 or type-5 active offer with that exact target,
walks into scene-2 collision record 93 using ordinary input, checks the
matching passage, dismisses it with A, and verifies resource scene 26 loads.
The full 320x224 dialogue and destination frames are in
`field-scene26-package-recovery-entry-qa/` and
`field-scene26-johnson-run-entry-qa/`; Debug and Release tests pass. The
standard offer target pools at `0xF3E6E` and `0xF3E76`, plus the fixed Renraku
target, do not generate map 2/event 1. This QA therefore verifies the ROM's
pair-matched handler and transition path, not normal offer generation for
Little Chiba. It does not establish independent Genesis pixel fidelity.

The Qt test is
`shadowrun_ghidra_conversion_qt_field_scene26_event1_entry_qa`. Debug and
Release each produce 15 full 320x224 BMP captures in
`decompilation/ghidra_conversion/build/conversion/qt/{debug,release}/qt_runtime/field-scene26-event1-entry-qa/`.
Their BMP names and SHA-256 hashes match. `route.tsv` records position,
collision, event, negative-probe count, transition, activity selection,
screen owner, visibility flag, projected position, and camera coordinates.
`source-boundaries.txt` lists unique first-seen boundaries. The audio trace
contains no runtime audio events.

## Eight-question full-screen review

1. **Does it look correct?** The spawn frame shows the field map, runner, and
   right HUD in a complete 320x224 composition; its map and HUD agree with
   independent section-03 frames 000029-000048. The destination frame also
   has a full map, runner, and HUD. During the long held-east movement, the
   runner is covered by the opaque right HUD column. I classify those
   intermediate native frames as a visual anomaly and do not call the route
   visually correct.
2. **Does the look reflect the behavioral state?** The route trace verifies
   the New Game spawn, accepted movement, south-lane approach, raw `0xC1`
   collision, event 1, two negative probes, row-1 destination, resource bound,
   selected index, and scene-26 spawn. During the overlap, the native
   projection visibility flag remains true even though the composited image
   does not show the runner. That disagreement is part of the finding.
3. **Are the visuals correct?** The scene-2 spawn map and field HUD have an
   independent match. The destination uses the expected resource map, but its
   exact pixels and the runner's right-edge treatment have no matching
   independent Genesis reference. Pixel correctness for the destination and
   later route frames remains unverified.
4. **Is it animated, and when?** Scene-2 movement selects the existing walk
   poses while input is held. The event fade is captured at updates 8, 16,
   and 17; updates 16 and 17 are fully black. Scene-26 entry, 10-update,
   and 30-update images are identical, so no ambient map motion is visible in
   this sample. Exact scene-26 animation cadence remains open.
5. **Is there a sound, and how long does it play?** This route records zero
   runtime audio events. A source cue for event 1, transition audio, and any
   stop lifetime were not established, so audio fidelity and duration remain
   open.
6. **Are related screen areas also correct?** Each capture includes the full
   playfield and sidebar. The map scrolls through the captured route; the
   portrait and item slots remain in the same HUD positions. The HUD begins
   at x=272 and covers the runner in the long-hold captures. There is no
   dialogue or shop owner during the field route or destination hold. The
   shared renderer now draws the ROM priority overlay above the actor and
   below the HUD; this does not change the unresolved right-edge HUD overlap.
7. **Do multiple frames support the interpretation?** Yes for the internal
   route: 15 whole-screen frames cover spawn, movement, doorway approach,
   contact, fade, destination entry, and hold. Independent section-03 frames
   000029-000048 show the same scene-2 map and HUD at the spawn. They do not
   show record 93's event-1 route or the scene-26 entry.
8. **Are new controls drawn over old controls?** No new menu buttons are
   introduced. The field view and its shared HUD remain during the approach;
   the transition clears the source view and loads the destination map with
   the normal field HUD. No stale dialogue/shop panel or duplicate controls
   appear in the destination captures.

## Outcome and remaining work

- **Code changed:** a Qt route QA command, CTest registration, route trace,
  and full-screen artifacts. This slice adds no new runtime event behavior;
  it exercises the existing typed collision and world-activity path.
- **Behavior implemented and checked:** normal input traverses the source
  doorway geometry, publishes event 1, selects row 1, and loads scene 26 as a
  resource activity. Seeded type-4 and type-5 active target pairs show their
  respective entry passages before the same transition. Debug and Release
  focused CTests pass.
- **Independently visually checked:** section-03 frames 000029-000048 support
  the scene-2 map and HUD at the spawn. No matching Genesis frames for this
  exact route or destination entry were found.
- **Defect resolved:** none claimed. The runner/HUD overlap during a long
  held-east route remains open, as do route audio and exact scene-26 visual
  fidelity.

The joined [scene-26 terminal route](scene26-event1-terminal-route.md) now
covers the event-31 menu Exit and return to the captured Scene-2 state, plus
the physical route to the Security Terminal using explicit passkey, skill, and
PRNG fixtures. To defend the map-route hill, still compare the scene-2 runner
clipping and scene-26 interior against independent Genesis captures, resolve
the field-route sound, and connect the source story inputs without QA fixtures.
