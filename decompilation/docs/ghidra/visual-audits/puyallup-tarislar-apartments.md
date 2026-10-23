# Puyallup: Tarislar Garden Apartments

Status: **partial**. Scene 1/event 7 first visit, guarded leave, signed
alternate-result pages, repeat information service, and gated tomb departure
are connected. The adjacent Scene-9 Sinsearach gate route now has a typed
three-page cinematic catalog, named art, Lua-owned page order, native presenter,
source-context return, and Debug/Release physical-input QA. Its fare roots and
wallet branches are modeled through their text pages. The typed outcomes now
keep refusal and short-funds in Scene 9 and enter Scene 6 after payment; its
first profile record places the runner at `(0x1310, 0x0980)`. Debug and Release
Qt targets build with this route. The updated script-runtime assertions and
paid physical route have not been run, so the gate hill remains partial. See
the [HILLS index](../HILLS.md) and [story-page audit](../cinematic-story-pages-source-audit.md).
The direct `FFE162` producer is now traced at `0x1F086`, the active type-3
run-entry handler. It writes one of three descriptors only when the active
target matches the current area/event: `1/9`, `2/9`, or `3/4`. The event-7
pass cannot create a fresh descriptor from this target pool. A generic
event-7 selection that reaches `0x167AE` publishes and clears any nonzero
`FFE162` pointer, then forces Scene 15. A type-3 write is consumed in that
same outer pass when it reaches `0x167AE`; event 9 with `FFFC0E` nonzero takes
the alternate `0x2CE0` route, which does not consume the pointer. Whether that
alternate route can carry the pointer to a later Tarislar selection remains
open. Scene 54's only traced `FFE12E` installers are the two pending-list
branches in `0x043B6`; an inherited matching list and the activity exit route
remain unresolved. The initial `FFE177` count and its six-profile-actor match
are source-traced.

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
  transition. The Lua entity links select the signed route; its JSON graph
  branches by result sign and renders both ROM-imported pages from the text
  catalog. ROM's positive result selects the delivery page at `0x15FE5D`;
  negative selects the pickup page at `0x15FDB8`. The entity retains numeric
  scene 13 as its suppressed route.
- Sinsearach gate caller and Frosty route: Scene-9 record 230 at `0x1B578E`
  covers the visible gate. In mode 9/event 0, `0x1FD80` selects the guard
  root at `0x1FE38` from story bits 18/20 and Group Item bit 5, then returns
  D6=-1 instead of taking event-table row 0 to scene 13. The graph models
  the `0x33` question branch and the Tribal Elders response. Command `0x34`
  at `0x21CE4` selects Lady Gillian group `0xF57E4` only with story bits
  13/17/18/19/20 set and Group Item bit 5/story bit 30 clear; otherwise it
  opens the busy-elders pages at `0xF4D14`. Lady Gillian command `0xF9` at
  `0x24DFE` selects the bit-9 readiness page. MORE dispatches command `0x44`
  at `0x21FD0`, which sets story bit 30 and opens `0xF57F8`. The Lua JSON
  graph now follows the guard, Lady Gillian, and Frosty pages up to
  command `0x49`, applying its FFFC34 bit-4 clear, bit-5 set, and runner
  source-byte triplet `+3`. Helper `0x5D130` writes FFF0BE=9, selects the
  three-page table at `0x0F5870` and resources at `0x1FE690`, then calls the
  shared story-page routine `0x5D1F6`. The cinematic catalog records the
  Harlequin, Thon, and Tarislar pages with a source-context return policy.
  The native presenter and typed source-context return are implemented, and
  Debug/Release physical-input QA captures the three pages and field return.
  The fare roots `0x39`/`0x3A` and command `0x38` are modeled through their
  response pages. Refusal and insufficient funds use `continue_in_place`;
  payment uses `enter_field_scene` for Scene 6, whose first profile record is
  `(0x1310, 0x0980)`. The host handoffs are wired. Updated script assertions
  and the paid physical route remain unrun, and no independent Genesis
  encounter capture exists, so the gate hill remains partial.
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
  with bit 28 clear applies command `0x40` at `0x5451A`, marks bit 28, and
  offers the Ito and information choices. The repeat-service JSON graph uses
  condition records for bit 16/28 and the map-item gates: item bit 8 opens the
  first Renraku map response; item bit 7 opens the paired-map response after
  bit 26 is set. The greeting at `0xFC111`, questions page at `0xFC222`, and
  Ito response at `0xFC757` are extracted into the text catalog. The named Lua
  action applies command `0x40` and enters the reusable
  `harlequin.map_information` JSON graph; it resolves Tarislar or Red Buffalo
  speaker/menu data from the active location. Conditions on FFFC22 item bits
  8 and 7 select the no-map, Renraku-only, and paired-map branches. Graph
  actions apply commands `0x46`/`0x47`, story bits 26/27, and effect bits
  8/12/14. The graph keeps this service on scene 1. Source choice labels are
  extracted from `0x1047D4`, `0x104494`, and `0x104473`.
- Tomb departure: when the story/map gates are complete, root `0xF4E08` asks
  whether Joshua is prepared for the
  trip. The question is the ROM text record at `0xFD03D`; its prompt labels are
  `0x103A0E` (Yes) and `0x103A13` (No). The Yes
  command `0xE4` calls `0x24954`, which returns prompt `0xF5A78` and writes
  `FFF0CC=1`; the sequence continues through `0xF5A8C` and `0xF5AA0`, then
  selector marker 1 loads scene 54. No follows `0xF5A5C` and `0xF5A70`, asks
  Joshua to assemble a team, and leaves him on scene 1. The JSON graph
  declares the story, map, and field-context gate and represents both
  branches with typed outcomes; the Lua entity adapter resumes scene 54 only
  after the Yes pages reach `enter_field_scene`.
- Command `0x48` at `0x220CC` chooses between the Renraku-only and paired-map
  responses using FFFC22 bit 7. Debug/Release script-runtime QA traverses all
  three graph branches and checks story/effect flags and the continuation on
  the final paired-map page. The physical-input CTest captures the Renraku-only
  route and clean scene-1 redraw. Independent Genesis page captures, the
  event-specific `FFE162` lifetime and relation to Tarislar, and the
  scene-54 completion route remain unresolved.

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
pointer resolves to a zero first entry, so that loader adds no placed actors.
Separately, the profile default-actor list at `0x1E994C` contains six Yakuza
Weenr records; the world-frame list service `0x43B6` installs this list after
pending lists. Each record has the ordered override `+0x9E=0x40`, which
connects these actors to the initial completion count described below. The
profile also has no elevator child-scene group;
scene 54 must not be treated as one floor in the normal elevator selector.

The event-31 dispatch is traced through ROM `0xB7B8`, which stores `0xDB26` in
`FFDD2C` as an indirect activity callback. Other route audits map `0xDB26` as
the ordinary field terminal in resource-field mode. The exported disassembly
has a gap from `0xDA68` to `0xDFF0`, so the callback path was decoded directly
from the US ROM. It clears `FFF118`, checks `FFDF40`, and enters its
mode-specific branch when that word is nonzero. Scene-54 setup uses
`FFDF40=3`; `0x03074` copies the current scene byte to `FFFBF8`, so this route
has `FFFBF8=0x36`. The callback's node-selection table covers scene bytes
`0x15` through `0x2B`; `0x36` falls through to `RTS` at `0xDEE6`, before the
node-selection input loop at `0xDE5E`. Thus this Scene-54 event-31 call does
not enter the Matrix node-selection flow or select a new scene. Timeout setup
is conditional:
`0x1DBAA` returns when the current scene is 54 and `FFE177=0`; otherwise it
sets 120 frames, or 600 while `FFE162` is nonzero. For the marker-1 entry
(`FFE186=1`), `0x167AE` calls the scheduler while `FFDDC4` still has the source
scene, then snapshots and clears `FFE177`. The first field-service pass through
`0x02900`/`0x04388` sets `FFE177=6`, so the zero-count gate does not skip the
initial Scene-54 timeout setup. The Scene-54 profile supplies six Yakuza
Weenr default-list rows, each with `+0x9E` bit 6 set. On a qualifying
completion, `0x0AB02` calls `0x16B98`; kind 8 returns without a weighted spawn
in mode 3, and the same active record then decrements `FFE177`. The six rows
match the initial count. At zero, service records reset and `FFE12E` is
dispatched and cleared if installed. Initializers `0x2A32`/`0x2A44` clear
`FFE12E`; the only non-clear stores found in the raw ROM are the two pending-
list branches in `0x043B6`, installing `0x53FFC` for list `0x1E628` and
`0x54286` for list `0x1E5C4`. The default Scene-54 rows do not themselves
install either callback. A matching inherited `FFE120` list at the first
Scene-54 service pass remains unresolved. In the zero-count clause, a null
`FFE12E` skips the callback call.
Prompt command `0x60` can reach a raw ROM code
gap at `0x225D2`, which sets `FFE120=0x1E628` at `0x22622` after a 75-nuyen
debit and story gates; its menu node is identified at `0x111316` and presents
the 75-nuyen entry-fee question with command `0x60` in the first choice slot.
The node's caller is the Club Penumbra Scene-3/event-0 doorman path, so no
Tarislar relation is established. Scene 54's six
actors come from the separate default list at `FFDDF0`, and its marker-1 route
with `FFE162` clear does not
replace `FFE120`. The selector takes the Scene-54 mode-3 branch and bypasses
the lower `FFE120` list-selection path. The field-exit orchestrator at
`0x1F5CA` calls `0x1EEDC` before title services; this dispatcher indexes the
run-handler table at `0xF3E56` by `FFFBED`, and its type-3 entry at `0xF3E62`
calls the handler at `0x1F086`. That handler requires current area/event
`FFFBF8/FFFBF9` to equal active target `FFFBEE/FFFBEF`, then selects
descriptors `0x1D6D4A`, `0x1D6D5E`, or `0x1D6D72` by target area. The confirmed
offer target pool contains Scene 1/event 9, Scene 2/event 9, and Scene 3/event
4; none matches Tarislar's Scene 1/event 7. The event-7 pass cannot make a
fresh write from this pool. In the outer loop at `0x075E`, a clear-D6 event
below 8 reaches `0x167AE`. That selector publishes any pending `FFE162`
descriptors through `0x16A26`, clears the pointer at `0x16A62`, and forces
Scene 15 before normal scene selection. A matching type-3 writer is consumed
in the same pass when the target is event 4 or event 9 with `FFFC0E` clear.
Event 9 with `FFFC0E` set instead calls `0x2CE0`, which leaves `FFE162`
untouched; its later path to Tarislar has not been traced. The Club Penumbra writer queues
`0x1E628` before its route continues to Scene 17, where the common `0x043B6`
service drains it; this source path supplies no carry into Scene 54. Raw
direct stores also set `FFE120=0x1E5C4` in separate code at `0x22482` and
`0x224F2`; no link from either path to Tarislar is established. A carry into
the Scene-54 service remains unresolved.
Scene completion and exit destination remain unresolved.
The event-0 record has raw value `0x00C0`, so `0xB7B8` publishes event 0 in
`FFFBF9`. In Scene-54 mode 3 it calls `0x1651E`; with D0=0, that helper returns
carry set without a map write. `0xB7B8` then sets `FFDF3A=-1`, `FFDF3C=0x10`,
and `FFDFe8=-1` and takes its common exit. This preserves a collision-state
effect but does not change the scene or select a return. No reverse route has
been added.

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
   Pending `FFE162` descriptors can select scene 15. The six default actors
   and their initial completion count are source-connected; their AI cadence
   and remaining activity timing still need tracing. The Qt frame is recorded
   but is not an independent fidelity proof.
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

- Wire the traced Scene-9 fare outcomes to typed Scene-9 and Scene-6 host
  transitions, then add focused Debug/Release route coverage for No, short
  funds, and successful payment. Keep the hill partial until its required
  source trace, named CTest, and CLI `actual.bmp` are all recorded.
- Continue Ilene's source-traced service through actor-pair contact and its
  world-activity completion; the Scene-20 entry alone does not defend the
  service.
- Trace the complete `0xF4C44` dialogue and the world-context route that
  selects Scene-47 event-25 continuation `0x53EFA`; its item grant and the
  Spirit Eyes bit-9 consumer are now connected in Lua.
- The first-authorized, guarded, signed alternate-result, repeat-information,
  and gated tomb-departure branches have source text, entity-owned route
  conditions, and focused physical QA. Debug/Release tests reach event 7 with
  ordinary Up input for both signs, capture each full one-page dialog and the
  clean scene-1 redraw, and verify that the scene-13 transition stays
  suppressed. Continue with the descriptor lifetime and selector precedence,
  then the scene-54 completion/return trace.
- Resolve whether a caller or carried descriptor value can make `FFE162`
  active during Tarislar's event-7 selector. The direct `0x1F086` writer is
  the type-3 run-entry handler and its three confirmed offer targets do not
  include Scene 1/event 7. Trace the six Scene-54 completion branches, locate
  the `FFE12E` installer, and determine the activity result when the count
  reaches zero. Preserve the scene-15 override as a source boundary until the
  descriptor link is established.
- Trace the post-count activity result, Scene-54 `FFE12E` installer, exits, and
  return path against the ROM before connecting the route in Lua.
- Capture the remaining branches and a source-backed scene-54 completion and
  return as whole 320x224 screens. Review all surrounding pixels and check the
  seven questions above before changing this status.
