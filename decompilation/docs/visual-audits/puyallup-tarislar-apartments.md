# Puyallup: Tarislar Garden Apartments

Status: **source-profiled, not implemented or visually defended**. Scene 1
record 67 is an event-7 trigger whose event-table default is scene 13. Indexed
title service 21 may set a marker that the main loop converts to scene 55; a
pending descriptor list can instead override that destination to scene 15.
Scene 55 launches a Tomb of Ellisia dialogue chain, which conflicts with the
Tarislar entry text. The event-specific descriptor state and actual Tarislar
handoff are unresolved. The current Qt route does not connect service 21 or
its dialogue.

Mike's RPG Center labels Puyallup map marker 2 “Tarislar Garden Apartments.”
Its walkthrough places the visit after the Sinsearach trials and meeting
Frosty. This is a useful story/name crosswalk. Trigger bounds, dialogue, state
conditions, and scene selection below come from the US ROM. See the
[Puyallup map](https://mikesrpgcenter.com/shadowrun/maps/puyallup.html) and
[walkthrough](https://mikesrpgcenter.com/shadowrun/walkthrough.html).

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
  `D6=-1`. The main loop at `0x097C` normalizes nonzero `FFE186` to 2 before
  selector `0x167AE` runs. That selects scene 55, unless the pending
  descriptor list at `FFE162` is nonzero; in that case `0x16A26` publishes
  descriptors and the destination is scene 15, which is subject to its
  `FFFC1E` bit-29 gate. With no marker override, scene 13 is subject to its
  bit-19 gate. When scene 55 is selected,
  `0x0A44` queues `0x20938`, whose text graph begins at `0x0FDD1A` and whose
  first text (`0x0FD2FE`) enters the Tomb of Ellisia antechamber. This conflicts
  with the Tarislar passage. The event-specific `FFE162` producer/lifetime and
  actual destination must be resolved before implementing the handoff.
- The later branch, when bit 21 is already set, selects further Harlequin
  dialogue from flags 16, 26, 27, and 28 plus group-item flags. Those
  conditions and conversation choices are not yet connected.

## Seven visual and behavioral checks

1. **Does it look correct?** No Qt route capture exists yet. Mike's map names
   marker 2, while the ROM dialogue itself names Tarislar; the map image has
   not been aligned to ROM world coordinates.
2. **Does the look reflect the behavioral state?** Not verified in the game.
   The ROM branch inputs are known: scene/event, `FFF0D5`, bit 30,
   bit 21, bits 16/26/27/28, and group-item flags. Their Qt/Lua binding is
   outstanding.
3. **Are the visuals correct?** The source text nodes and portrait profile
   have been identified. Scene 1 imports as 1600x1504, scene 13 and 15 as
   320x224, and scene 55 as 512x288. Those extracted assets are useful
   inspection inputs, not independent Genesis screenshots.
4. **Is it animated, and under what conditions?** The first-entry path sets
   `FFE186` before the scene handoff, but the event-specific descriptor state
   determines whether the following route selects scene 15 or the scene-55
   tomb sequence. No animation cadence or Qt result is claimed.
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
- Trace the event-specific `FFE162` producer and determine how the
  scene-55/Tomb route is excluded or intended; preserve event-table default
  scene 13 and the scene-15 pending-descriptor override only as
  source-conditional facts until then.
- Capture the ordinary-input approach, every page, transition, and clean
  return/arrival as whole 320x224 screens. Review all surrounding pixels and
  check the seven questions above before changing this status.
