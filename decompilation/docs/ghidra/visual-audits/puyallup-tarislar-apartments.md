# Puyallup: Tarislar Garden Apartments

Status: **partial; first visit, guarded leave, signed alternate-result pages,
repeat information service, and gated tomb departure are connected, while the
upstream Frosty handoff and pending descriptor path remain open**. Scene 1
record 67 is an event-7 trigger whose event-table
default is scene 13. With FFFC1E bit 30 set, bit 21 clear, and alternate
result/FFF0D5 zero, the first visit marks bit 21, awards +2 Karma to the three
live runners, and runs the JSON arrival/penthouse pages before the shared
Harlequin prompt chain. Completing the arrival page applies effect bits 5/6;
the conversation then returns to scene 1. Debug/Release script and physical
input QA check the ordered pages, flags, and full-screen field return. The
guarded branch presents pages `0xFBCC7` and `0xFBD5A`, then a titleless No
Business/More choice. No Business follows `0xFBD94` and returns to scene 1;
More follows `0xFBEEC` into the shared Harlequin conversation at `0xF58C0`,
which also returns to scene 1. Debug/Release physical-input CTests exercise
both choices and capture the menu, branch pages, and clean field return. A repeat
visit presents Harlequin's “Hello again” service. Ito, post-Ito, map, and
group-item bits select the questions, Ito response, map-information, or tomb
branch; the information path shows the available Renraku and paired maps, then
returns to scene 1. The tomb offer requires bits 30, 21, 16, 28, 26, and 27 and
alternate result zero. Yes follows the three ROM travel pages and enters scene
54; No presents the two preparation pages and returns to scene 1. Debug/Release
physical-input CTests cover first visit, guard, both signed alternate results,
repeat information, and both tomb choices. The upstream Frosty dialogue and
pending-descriptor override remain open. A pending descriptor list at FFE162
can instead override the selector result to scene 15. Scene 55 is a separate
marker-2 route that starts Tomb of Ellisia dialogue; it is not the event-7
marker-1 destination. Scene 54's extracted map is 320x1344, consistent with the
apartment elevator passage, but that asset consistency is not an independent
visual confirmation. The scene-54 exit and return path also remain unresolved.

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
- Signed alternate-result branch: before checking story bits 30/21, service
  21 tests `FFF0D5`. Positive selects profile `0x122024`, group `0x15FF28`,
  and page `0x15FE5D`; negative selects the same profile, group `0x15FF20`,
  and page `0x15FDB8`. Both are one-page prompts. The completion path leaves
  `FFF0CC` clear, sets `D6=-1`, and suppresses event-table row 7's scene-13
  transition. The Lua entity links now select the signed branch by result sign
  and retain the numeric scene-13 destination as its suppressed route.
- State-selected story dialogue: command `0x34` at table `0x12CCB4` points to
  handler `0x21CE4`; its selection block begins at `0x21D00`. With story bits
  13, 17, 18, 19, and 20 set, Group Item bit 5 clear, and story bit 30 clear,
  the handler returns group `0xF57E4`, whose text at `0xFB535` says Lady
  Gillian greets Joshua. Command `0x44` is separate: handler `0x21FD0` sets
  story bit 30, loads profile `0x121F4C`, and opens group `0xF57F8`, whose
  first text at `0xFB6C8` says Joshua is taken to Frosty. The Lua source test
  checks the command-table entries and ROM branch bytes. These source roots
  are identified, but their live field-interaction caller and location route remain
  unconnected.
- Ilene prerequisite: Scene-2/event-0 service 19 at `0x543A2` requires story
  bits 18/19 = 1/0, `FFF0D5=0`, `FFFC34` bit 0 clear, and `FFFC38` bit 12 set.
  It arms `FFE182` and leaves event-table row 0's Scene-20 destination live.
  When the service-19 marker is pending and `FFE180` is clear, `0xA800`
  selects list `0xA8CC`: template `0x172CA` (`lone11`, kind 10/action 3)
  spawns at `(0x0E80,0x0700)` with ordered overrides ending in `+0x9E=0x08`.
  The `0xEB92` actor-pair scan reaches `0xECCC`, whose bit-3 branch requires
  the partner's `+0x10` bit 1 and the actor's `+0xB0` bit 7 clear; `0xED92`
  then consumes `FFE182` and selects `0x54402`. That activity adds 2 Karma per
  runner, sets bit 19, and opens group `0x115ECC`; text `0x115F10` names Ilene.
  `Ilene.prepare_scene_20_entry` now implements the gate as a Lua route
  preparation and writes transient `pending_world_effect` state for FFE182.
  Debug/Release physical-input QA verifies it fires once on the scene-2/event-0
  route and still enters scene 20, writing a full-screen `actual.bmp`. ROM-byte
  tests cover the remaining source path. The generated A8CC row now carries
  FFE180/FFE182 gates, and active scene initialization selects and publishes
  its actor before the default list. Debug/Release physical QA verifies the
  actor profile, source position, and slot order. Actor-pair contact and the
  Ilene activity remain unimplemented.
- Prerequisite: the code at `0x53EA8` sets `FFFC1E` bit 9 and opens text
  record `0xF4C04` ("You have completed your tasks, young warrior"). The
  Spirit Eyes event-27 Lua route now models that completion gate after first
  contact, the gargoyle horn, the hellhound pelt, and the feathered-serpent
  encounter. The Scene-47 post-dialogue handler `0x53F50` sets story bit 8
  and Group Item bit 2; Spirit Eyes handler `0x53E84` checks and consumes that
  scale at `0x53E98`, then sets bit 9 at `0x53EA8`. Lua and Debug/Release
  script-runtime QA now cover this connected item handoff. Independent
  confirmation of the serpent conversation remains unresolved.
  See [the Spirit Eyes audit](council-spirit-eyes.md).
- Branch state: if bit 30 is set and bit 21 is clear, `0x5447E` sets bit 21
  using `0x2196C`, then calls `0x59C34` with D0=2 before the first-entry
  passage. `0x59C34` performs `ADD.B` at `FF018D`, `FF028D`, and `FF038D`;
  these are the three live runner records' +0x8D Karma bytes. The shared Lua
  `party.award_karma_2` host service now applies this source reward.
- Guarded passage: text group `0xF4DE0` starts with page `0xFBCC7`, then page
  `0xFBD5A`, followed by a titleless two-choice menu. The No Business choice
  uses the `0xFBD5A` text as its label and follows `0xFBD94`, ending in the text
  that Joshua agrees to leave. More uses the shared `0xFE5BF` label and follows
  `0xFBEEC` to the penthouse page, then node `0xF58C0` into the shared Harlequin
  conversation. Neither choice selects scene 13; both close on scene 1. The
  graph and its two physical-input QA cases are in Debug and Release.
- Authorized first entry: text group `0xF4DF4` starts at `0xFBE12`. The JSON
  graph imports that page from the ROM, then runs command `0x45` at `0x21FF2`
  to set FFFC34 bits 5/6 before the `0xFBEEC` penthouse page and shared
  Harlequin flow at `0xF58C0`. The tested path does not set `FFF0CC`, so it
  returns to the field route instead of loading scene 54.
- First-visit return: this branch does not set `FFF0CC`. Its prompt chain
  finishes without the marker, so event 7 returns to the field route rather
  than selecting scene 54. The indexed service's first-authorized path is not
  itself an interior transition.
- Repeat service: `0x5447E` selects greeting/root `0xF4E08` after the first
  visit. Story bit 16 clear selects the questions root `0xF4E20`; bit 16 set
  with bit 28 clear marks bit 28 and offers the Ito and information choices.
  Once bit 28 is set, item bit 8 gates the first Renraku map response and item
  bit 7 gates the paired-map response after bit 26 is set. The Lua route keeps
  this service on scene 1; its source choice labels are extracted from ROM
  `0x1047D4`, `0x104494`, and `0x104473`.
- Tomb departure: when the story/map gates are complete, root `0xF4E08` asks
  whether Joshua is prepared for the
  trip. Its prompt labels are `0x103A0E` (Yes) and `0x103A13` (No). The Yes
  command `0xE4` calls `0x24954`, which returns prompt `0xF5A78` and writes
  `FFF0CC=1`; the sequence continues through `0xF5A8C` and `0xF5AA0`, then
  selector marker 1 loads scene 54. No follows `0xF5A5C` and `0xF5A70`, asks
  Joshua to assemble a team, and leaves him on scene 1. The Lua link expresses
  the story gates (bits 30, 21, 16, 28, 26, 27) and alternate-result-zero
  condition; a matching task result resumes scene 54 only after the Yes pages.
- Command `0x48` at `0x220CC` chooses between the Renraku-only and paired-map
  responses using FFFC22 bit 7. The CTests exercise both the map-information
  route and its return to a clean scene-1 redraw. The event-specific `FFE162`
  producer/lifetime and the scene-54 completion route remain unresolved.

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

1. **Does it look correct?** The Qt tests capture the first-visit prompts,
   guarded pages, both signed alternate-result pages, tomb offer, travel pages,
   clean scene-1 return, and full scene-54 frame. The offer's question and
   Yes/No labels match ROM text; no independent Genesis frame for these
   dialogue branches has been identified.
2. **Does the look reflect the behavioral state?** First visit uses the ROM
   bit-30/bit-21 and alternate-result gates. The tomb route additionally checks
   bits 16/28/26/27; Yes enters scene 54 and No returns to scene 1. Repeat
   information uses bits 16/28/26/27 and group-item bits 8/7. Upstream Frosty,
   pending-descriptor paths remain unconnected. Both signed FFF0D5 pages now
   have entity-owned Lua tasks and ordinary-input QA.
3. **Are the visuals correct?** The source text nodes and portrait profile
   have been identified. Scene 1 imports as 1600x1504, scene 13, 15, and 54
   as 320x224 frames, and scene 54's full map is 320x1344; scene 55 is
   512x288. The tall scene-54 layout is consistent with the elevator text.
   These extracted assets are inspection inputs, not independent Genesis
   screenshots.
4. **Is it animated, and under what conditions?** Only the Yes choice at
   repeat root `0xF4E08` calls `0x24954`, sets `FFF0CC`, and selects scene 54.
   Pending `FFE162` descriptors can select scene 15. Scene-54 actor/activity
   cadence and transition timing still need source tracing; the Qt frame is
   recorded but is not an independent fidelity proof.
5. **Is there a sound?** Service `0x5447E` has no direct sound call in the
   traced path. Shared prompt/input audio and the rest of the scene handoff
   remain unverified.
6. **Are old controls or graphics left behind?** Dialogue pages replace the
   field HUD. First-visit, guarded, and repeat-service branches redraw scene 1
   after their dialogue; tomb Yes redraws scene 54 and No redraws scene 1.
   Scene-54 exit and return cleanup are unresolved.
7. **Do multiple frames support the interpretation?** Physical tests capture
   spawn, contact, source dialogue, both alternate-result pages, travel pages,
   and final scene-1/scene-54 `actual.bmp` frames. These are host captures; no
   independently aligned Genesis sequence has been identified.

## Next implementation evidence needed

- Wire and physically verify the source-confirmed Ilene continuation without
  flattening it into an unconditional Scene-20 entry. Then trace the live
  field-interaction caller for command `0x34`'s Lady Gillian root and command
  `0x44`'s Frosty handoff. Preserve the verified command distinction: `0x34`
  selects `0xF57E4`; `0x44` sets bit 30 and opens `0xF57F8`.
- Trace the complete `0xF4C44` dialogue and the world-context route that
  selects Scene-47 event-25 continuation `0x53EFA`; its item grant and the
  Spirit Eyes bit-9 consumer are now connected in Lua.
- The first-authorized, guarded, signed alternate-result, repeat-information,
  and gated tomb-departure branches have source text, entity-owned route
  conditions, and focused physical QA. Debug/Release tests reach event 7 with
  ordinary Up input for both signs, capture each full one-page dialog and the
  clean scene-1 redraw, and verify that the scene-13 transition stays
  suppressed. Continue with the upstream Frosty branch, pending descriptor,
  and scene-54 completion/return trace.
- Trace which world-effect path can populate `FFE162`, whether it is active
  for Tarislar's event-7 route, and how selector precedence reaches scene 54
  or the pending scene-15 override.
- Trace scene 54's direct event-31/event-0 handling, any non-table actor setup,
  the activity completion condition, exits, and return path against the ROM
  before connecting the route in Lua.
- Capture the remaining branches and a source-backed scene-54 completion and
  return as whole 320x224 screens. Review all surrounding pixels and check the
  seven questions above before changing this status.
