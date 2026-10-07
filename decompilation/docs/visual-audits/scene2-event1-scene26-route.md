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
with random bound 3, a selected index within that bound, 133 collision
rectangles, and the descriptor spawn `(0x0A00, 0x0500)`. Its imported
metadata has no map event table. The captured route holds the destination
stable for 30 further field updates.

The Qt test is
`shadowrun_ghidra_conversion_qt_field_scene26_event1_entry_qa`. Debug and
Release each produce 15 full 320x224 BMP captures in
`build/conversion/qt/{debug,release}/qt_runtime/field-scene26-event1-entry-qa/`.
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
   dialogue or shop owner during the field route or destination hold.
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
  resource activity. Debug and Release focused CTests pass.
- **Independently visually checked:** section-03 frames 000029-000048 support
  the scene-2 map and HUD at the spawn. No matching Genesis frames for this
  exact route or destination entry were found.
- **Defect resolved:** none claimed. The runner/HUD overlap during a long
  held-east route remains open, as do route audio and exact scene-26 visual
  fidelity.

Before calling the map-route hill defended, trace the source sprite clipping
and camera state against an independent Genesis capture, find the normal
descriptor-spawn route to the Security Terminal within scene 26, and cover
the return route and event-specific sound behavior.
