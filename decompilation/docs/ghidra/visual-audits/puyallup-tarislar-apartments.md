# Puyallup: Tarislar Garden Apartments

Status: **source-profiled, not implemented or visually defended**. Scene 1
record 67 is an event-7 trigger whose event-table default is scene 13. When
indexed title service 21 sets marker 1, event 7 takes a direct main-loop branch
to selector `0x167AE`; the selector routes to scene 54. A pending descriptor
list at `FFE162` can instead override that result to scene 15. Scene 55 is a
separate marker-2 route that starts Tomb of Ellisia dialogue; it is not the
event-7 marker-1 destination. Scene 54's extracted map is 320x1344, consistent
with the apartment elevator passage, but that asset consistency is not an
independent visual confirmation. The current Qt route does not connect
service 21 or its dialogue.

Mike's RPG Center labels Puyallup map marker 2 “Tarislar Garden Apartments.”
Its walkthrough places the visit after the Sinsearach trials and meeting
Frosty. This is a useful story/name crosswalk. Trigger bounds, dialogue, state
conditions, and scene selection below come from the US ROM. See the
[Puyallup map](https://mikesrpgcenter.com/shadowrun/maps/puyallup.html) and
[walkthrough](https://mikesrpgcenter.com/shadowrun/walkthrough.html).

The ROM text itself has a spelling split: the guarded passage at `0xFBCC7`
says “Tarislar,” while the authorized first-entry passage at `0xFBE12` says
“Tarlisar.” Preserve each imported string as encoded unless the game-text
catalog's source policy explicitly authorizes a correction.

## ROM trace

- Scene 1 descriptor: `0x3236`; collision stream begins at `0x17D704`.
- Record 67: `0x17D9A2`, collision `0x00C7`, event 7, x `0x02E0..0x02FF`,
  y `0x00A0..0x00BF` (grid cell 23,5).
- Event table: `0x17D620`, row 7 = `0x0D` (field scene 13).
- Title-service loop: `0x53A78`, service index 21 points to `0x5447E` and
  executes before the generic route. It matches `FFFBF8 == 1` and
  `FFFBF9 == 7`. `FFF0D5` chooses a separate text path when nonzero; it does
  not disable the service.
- Story access: handler `0x21FD0` (dispatcher index `0x44`) sets `FFFC1E` bit 30,
  loads profile `0x121F4C`, and starts text chain `0xF57F8`. The first text
  record is “You are taken to Frosty.” The associated extracted profile is a
  56x72 portrait; the source context indicates it belongs to Frosty, but the
  profile itself has no name label and the complete conversation route is not
  connected.
- Branch state: if bit 30 is set and bit 21 is clear, `0x5447E` sets bit 21
  using `0x2196C`, adds 2 to phase bytes `FF018D`, `FF028D`, and `FF038D`
  using `0x59C34`, then displays the first-entry passage. `0x59C34` changes
  actor phase counters; it does not award Karma.
- Guarded passage: text group `0xF4DE0` references `0xFBCC7`, the shared
  continuation label `0xFE5BF`, and the next node at `0xF5890`. That chain
  continues through `0xFBD5A` and `0xFBD94`, ending in the text that Joshua
  agrees to leave.
- Authorized first entry: text group `0xF4DF4` starts at `0xFBE12`. It
  describes entering the apartments, meeting the armed elves, following them
  into the elevator, and riding to the top floor.
- Transition: after the shared prompt runner `0x1F246`, the handler tests
  `FFF0CC`. If nonzero, it leaves `D6=0` and sets `FFE186=1`; if zero, it sets
  `D6=-1`. In the main loop at `0x097C`, the event-7 branch checks
  `FFFBF9 < 8` and jumps directly to selector `0x167AE` from inside the
  original `FFE186==0` arm. It skips the adjacent `FFE186=2` assignment, so
  marker 1 remains 1. Selector `0x167AE` reads row 7's default scene 13, then
  selects scene 54 from marker 1. If `FFE162` contains a pending descriptor
  list, `0x16A26` publishes it and selects scene 15 instead; scene 15 is
  subject to its `FFFC1E` bit-29 gate. With no marker, scene 13 is subject to
  its bit-19 gate. Marker 2 is a distinct selector route to scene 55; only
  that route queues `0x20938`, whose text graph begins at `0x0FDD1A` and whose
  first text (`0x0FD2FE`) enters the Tomb of Ellisia antechamber. The
  event-specific `FFE162` producer/lifetime and the scene-54 completion route
  remain unresolved.
- The later branch, when bit 21 is already set, selects further Harlequin
  dialogue from flags 16, 26, 27, and 28 plus group-item flags. Those
  conditions and conversation choices are not yet connected.

## Scene 54 entry profile and collision evidence

Scene 54 (`0x36`) is a scripted-activity scene in the ROM selector. The map
profile is `0x1E98EC`; its field bounds are 320 by 1344 map units, initial
camera `(1, 70)`, and all three profile runner anchors are `(10, 81.5)`. The
individually extracted background is 320 by 1344 pixels. Those coordinates
explain the tall asset but do not establish what the player sees in a complete
Genesis sequence.

The profile has no event-table pointer. Its 29 collision rectangles comprise
28 records with raw value `0x007F` (event code 31) and one record at grid cell
`(4,1)` with raw value `0x00C0` (event code 0). The standard scene-entity list
pointer resolves to a zero first entry, so the ordinary scene entity-table
loader adds no placed actors. This does not rule out actor setup through a
separate scripted path. The profile also has no elevator child-scene group;
scene 54 must not be treated as one floor in the normal elevator selector.

The event-31 branch is now traced through ROM `0xB7B8` to handler `0xDB26`.
Because scene 54 uses scripted-activity mode 3, the handler takes its nonzero
mode path. Scene 54 matches none of that path's explicit scene cases, so it
returns at `0xDEE6` without opening a floor selector, dialogue, or destination
route. The event-0 record uses the generic projected-interaction path; helper
`0x1651E` has no scene-54/mode-3 map write. Its later main-loop effect remains
unresolved. The current trace therefore does not establish scene 54's exit
destination, and no reverse route has been added.

## Seven visual and behavioral checks

1. **Does it look correct?** No Qt route capture exists yet. Mike's map names
   marker 2, while the ROM dialogue itself names Tarislar; the map image has
   not been aligned to ROM world coordinates.
2. **Does the look reflect the behavioral state?** Not verified in the game.
   The ROM branch inputs are known: scene/event, `FFF0D5`, bit 30,
   bit 21, bits 16/26/27/28, and group-item flags. Their Qt/Lua binding is
   outstanding.
3. **Are the visuals correct?** The source text nodes and portrait profile
   have been identified. Scene 1 imports as 1600x1504, scene 13, 15, and 54
   as 320x224 frames, and scene 54's full map is 320x1344; scene 55 is
   512x288. The tall scene-54 layout is consistent with the elevator text.
   These extracted assets are inspection inputs, not independent Genesis
   screenshots.
4. **Is it animated, and under what conditions?** Service 21's marker selects
   scene 54 on the event-7 route; pending `FFE162` descriptors select scene
   15. The marker itself does not establish an animation policy. Scene-54
   actor/activity cadence and the transition animation still need source
   tracing. No Qt result is claimed.
5. **Is there a sound?** Service `0x5447E` has no direct sound call in the
   traced path. Shared prompt/input audio and the rest of the scene handoff
   remain unverified.
6. **Are old controls or graphics left behind?** No modal route has been
   implemented, so the full-screen replacement and return cleanup are still
   unverified. The shared presenter clears the screen and dialogue text wells
   when a dialogue panel is open, but that alone does not defend this route.
7. **Do multiple frames support the interpretation?** No Qt capture exists
   for the route and no matching independent gameplay sequence has been
   identified. Debug/Release agreement would be an internal consistency check,
   not Genesis visual proof.

## Next implementation evidence needed

- Connect the upstream Frosty action-`0x44` choice through Lua and bind bit 30 to
  typed story state.
- Port service 21's guard/first-entry/repeat branches into a generic Lua
  event/dialogue flow. Preserve its source order and the phase-counter effect.
- Trace which world-effect path can populate `FFE162`, whether it is active
  for Tarislar's event-7 route, and how selector precedence reaches scene 54
  or the pending scene-15 override.
- Trace scene 54's direct event-31/event-0 handling, any non-table actor setup,
  the activity completion condition, exits, and return path against the ROM
  before connecting the route in Lua.
- Capture the ordinary-input approach, every page, transition, and clean
  return/arrival as whole 320x224 screens. Review all surrounding pixels and
  check the seven questions above before changing this status.
