# Field actor population and combat handoff audit

**Status:** ROM population, initial behavior entry, direction, animation,
action/item codes, and sound-request paths are traced for the profile-selected
actors in scenes 2 and `0x14`. Lua selects a package by stable source-template
profile ID and the ROM-seeded item/spell/none action class and code; native
C++ executes its finite entry animation.
New Game bootstrap does not activate the scene-2 profile list. Scene changes
activate the supported list for the destination. This remains partial:
post-entry behavior, actor movement, interaction, collision targets, hits,
status, and damage are not implemented. `LOADOUT-01` remains partial.

## Source path

`0x2CE0` (`advance_field_mode_transition`) maps the current event and transition
context through `0x2E72`. If the selected mode is `0x13` and bit 8 of
`FFFC1E` is clear, it starts world activity with descriptor `0x53EE2`, places
the pending actor-list choice `0x1E552` in `FFE120`, and schedules the source
timeout through `0x1DBAA`. Mode `0x13` maps through `0x2E42` to scene `0x2F`.
The existing typed conversion test records one concrete input: `FFFBF9=8`,
`FFFC10=0x14` selects scene `0x2F` and transition coordinates
`0x5E80,0x1300`.

On the world-frame path, `0x43B6` drains the pending `FFE120` list through
`0x4466`, then drains the default list. It assigns additional timeout
continuations to list pointers `0x1E628` and `0x1E5C4`. `0x4466` allocates a
world entity and effect slot for each entry; bit 31 of the template pointer
marks a root record. `0x44BC` copies the named template data, installs its
position, and applies optional byte-offset/value overrides. It copies
template byte `+9` to live entity byte `+0x57`. For ordinary item actions,
`0x14D6A` treats `+0x57` as a one-based D4540 item/action code when `+0x56` is
nonnegative; a negative `+0x56` selects the separate D4EA0 spell table. The
guard template has `+8=0`, `+9=6`, which resolves to item 6, Max-Power (Heavy
Pistol). The pedestrian template has `+8=0x7F`, `+9=0`, so it starts with no
active item action. This is a ROM-derived loadout input, not a guess from the
actor's appearance.

The decoded ROM records are:

| List | Records | Source positions |
|---|---|---|
| `0x1E552` | Five `eye5r 1` templates, type `7`, rooted | `5980,0E80`; `5F00,1B80`; `6000,1E80`; `4F00,1200`; `5B00,0980` |
| `0x1E5C4` | One Strike Team, two bodyguards, one pedestrian | `1100,0480`; `1280,0480`; `1100,0580`; `1280,0580` |
| `0x1E628` | Three Strike Team templates and one corp guard | `0F00,0500`; `1080,0500`; `0200,0500`; `0300,0780` |

The names and positions above come from the ROM template/list records, not
guesses based on a screenshot. Keep two fields distinct: `0x44BC` copies the
template's first word (for example, `7` for the `eye5r 1` records) to live
entity `+0x0E`; `0x481E` selects a sprite-source list from the separate type
word at projection slot `FF164C+0x02`. The converted `0x481E` selector maps
projection types 0 through 11 to source `0x5F78`, but the live population to
projection-slot assignment has not been traced. The Qt default-list drawing
path described below uses the actor kind's own `0x600E` state frame and does
not claim that actor records map one-to-one to the ten `FF164C` projection
slots.

## Do not confuse with scene pedestrian choices

`0x5BA98` indexes the table at `0x1EDB88` from scene `FFDDC4` and the
one-based selection counter `FFE183`. Its rows cover scenes `0x15..0x2B` and
provide three single-pedestrian position lists. `0xA800` only selects one
through its event-phase/world-mode/context gates. Those entries are not the
three actor groups above and must not be inserted as generic enemies to make
the weapon appear to hit something.

Spawn lists also are not always flat pointer pairs. After each template and
position pointer pair, `0x44BC` checks the following word for override mode
`2`. It then reads byte-offset/value pairs until a zero word. For example, the
scene pedestrian list at `0x1EDECC` contains overrides at record bytes
`0x9E`, `0x2E`, and `0x9E` before its zero terminator. An extractor must
preserve order because later writes can replace an earlier value at the same
offset.

The scene's default template list is a separate population source. Profile
entries at `0x323A` (scene 2, profile `0x172C80`) and `0x3282` (scene `0x14`,
profile `0x1D917A`) supply the long at word offsets 21–22 (`FFDDF0`) consumed
by the final `0x43B6` call to `0x4466`:

| Scene | Profile-selected list | Template | Position list | World position | Ordered byte override |
|---|---:|---:|---:|---:|---|
| 2 | `0x172D1C` | `0x1D6988` pedestrian | `0x172D58` | `0x3480,0x2200` | `+0x9E=0x20` |
| 2 | `0x172D1C` | `0x1D6988` pedestrian | `0x172D68` | `0x5300,0x3700` | `+0x9E=0x20` |
| 2 | `0x172D1C` | `0x1D6988` pedestrian | `0x172D78` | `0x6280,0x1900` | `+0x9E=0x20` |
| `0x14` | `0x1D91DA` | `0x1D6A1C` corp guard | `0x1D9248` | `0x0600,0x1100` | none |
| `0x14` | `0x1D91DA` | `0x1D6A1C` corp guard | `0x1D9252` | `0x0600,0x2300` | none |
| `0x14` | `0x1D91DA` | `0x1D6A1C` corp guard | `0x1D925C` | `0x0800,0x3500` | none |
| `0x14` | `0x1D91DA` | `0x1D6A1C` corp guard | `0x1D9266` | `0x5400,0x0A00` | `+0x2E=0x40` |
| `0x14` | `0x1D91DA` | `0x1D6A1C` corp guard | `0x1D926C` | `0x6080,0x0A00` | `+0x2E=0xC0` |
| `0x14` | `0x1D91DA` | `0x1D6988` pedestrian | `0x1D922E` | `0x1600,0x0580` | none |
| `0x14` | `0x1D91DA` | `0x1D6988` pedestrian | `0x1D9238` | `0x3680,0x1800` | none |
| `0x14` | `0x1D91DA` | `0x1D6988` pedestrian | `0x1D923E` | `0x0B00,0x2A80` | none |

The list sentinel is `0x172D4C` for scene 2 and `0x1D922A` for scene `0x14`.
The optional override marker is a 32-bit value `2`, followed by byte-offset /
byte-value pairs and a zero word; preserve the pair order. The tested alternate
scenes 6, 9, and `0x2C..0x31` have empty default lists. These records prove map
population, but not movement, animation, interaction, or damageability. Do
not treat presence in a map list as proof that an actor is a weapon target.

The live template type also selects the initial animation table. `0x44BC`
copies the template's first word into live `+0x0E`; this is kind 5 for the
corp-guard template and kind 11 for the pedestrian template. `0x600E` indexes
the state-pointer table at `0x62AC` with that kind, then uses the live `+0x16`
selector and `+0x1E` frame index. The default record at `0x7361A` initializes
both indices to zero. Kind 5's state-zero list is at `0x827C` and points to
frame descriptor `0x86E0A`; kind 11's is at `0x8D7C` and points to
`0xA7684`. Both state lists have marker 1, so their initial state repeats one
frame. The descriptors carry the actual image geometry and origin: the guard
frame is 24x32 at `(-13,-26)` and the pedestrian frame is 32x32 at
`(-14,-24)`. The template's `+0x02` word selects palette line 2 for guards and
line 3 for pedestrians. `0x44BC` copies that template word to entity byte
`+0x2C`; it is separate from `0x600E`'s selector and frame-index words at
`+0x16` and `+0x1E`. The profile overrides touch `+0x2E` or `+0x9E`, not
those animation words, so the state-zero frame choice is supported at
initialization. Those overrides may affect later live behavior that this
initial transition slice does not implement.

The entry behavior is separate from ordinary directional walking. `0xA8EA`
services the default records before the field camera and render pass;
`0xAC84` sends records with the default behavior flags through `0x13904`.
For the supported default-record path, `0x13904` subtracts the initial
`+0xB6` delay (the low nibble of the source PRNG result, 0 through 15), then
checks the zero `+0x16` animation selector and zero `+0x9C` state. It sets the
entry flag and calls `0xB330`. `0xB330` requests the kind-indexed cue from
`0xB05E`, selects the directional C0B2 list, and resets the animation state.
Kind 5 requests event `0x2E`; kind 11 requests `0x2F`. The audio tables map
these to silent table-2 slots 46 and 47, so the source makes a request but no
audible sample or playback duration is defined for either cue.

The selected C0B2 lists for the used directions are finite two-frame lists
(`0x8002` marker, `0x0200` increment, `0x1000` period). The initial idle
increment contributes a `-16` accumulator carry on the activation frame;
then each transition pose remains for nine field updates. The final pose stays
selected while the 20-count behavior timer is serviced; the next zero check
completes on the 21st update after activation. This covers only the observed
initial transition. `0xAB02` and later action, movement, collision, and damage
branches remain outside the Lua heartbeat slice.

The offline actor table now gives each active record a stable Lua profile ID
built from the source template name, entity kind, action-table class, and
action code. It also classifies the template byte at `+8`: a clear sign bit
selects the D4540 item-action table, a set sign bit selects the D4EA0 spell
table, and code zero means no active action. Those are typed data inputs; the
profile ID contains no runtime ROM address.

The Qt actor service passes that identity, action class/code, instance ID,
initial delay, and source heading to `gameplay/scripts/actor_ai.lua`. Lua
selects the `corp_guard_k05_item06` package for the kind-5 guard and
`pedestrian_k0b_none00` for the kind-11 pedestrian. The guard package binds
item 6 (Max-Power / Heavy Pistol); the pedestrian has no active item action.
Each actor now receives a Lua heartbeat on each field behavior-service pass.
The script owns the entry delay, directional C0B2 choice, two-frame timer,
terminal hold, and cue request; C++ stores only the latest returned pose for
rendering and forwards the cue to the audio host. This replaces the prior
`FieldActorBehaviorState` C++ timer/animation implementation for this entry
branch. Targeting, attack decisions, actual NPC weapon firing, movement,
collision outcomes, and damage are not implemented by these profiles yet.

The existing field entity catalog is not these actors: the importer currently
decodes the live Joshua template at `0x7291A` (kind 3). `0x4658` updates the
sixteen entity records and calls `0x481E`; `0x481E` separately refreshes ten
projection slots from `FF164C`. Its `0x5F78` geometry must not be substituted
for the kind-5/kind-11 animation frames or treated as their full sprite art.

## Lua default-actor heartbeat slice

The offline `shadowrun_graphics_extract --all-assets` step writes
`field/entities/default-actors.tsv` by walking the profile-selected lists at
`0x172D1C` and `0x1D91DA`. It preserves list order, world position,
template/position/list provenance, source-derived Lua profile ID, item/spell/
none action class and code, palette line, source heading, and ordered
overrides. It writes individually named idle
images and two C0B2 transition frames for each kind and direction used by
these lists. Scene 2 uses direction
0; scene `0x14` also uses directions 2 and 6 because source-order records 3 and
4 override heading at `+0x2E`. Images preserve signed descriptor origins and
are direct assets consumed by the runtime; no ROM or VDP state is used at
runtime.

The runtime population is never inferred from gameplay-video frames. The
extracted rows are ROM list entries with named `corp_guard` or `pedestrian`
templates, source positions, and initial behavior data; no corpse-specific
template or death-state placement is present in this table. New Game's first
scene-2 entry keeps those three profile rows inactive, as the ROM lifecycle
does. A later scene entry may activate only the rows belonging to that scene.
Until the actor death/drop path is connected, these live profile rows must not
be converted into dead-body scenery or supplemented with actors seen in a
longplay.

New Game's `0x075E` setup enters scene 2 and performs its warmup without the
later `0x02900 -> 0x04388 -> 0x043B6` world-frame list-finalization path. The
Qt bootstrap therefore keeps extracted scene-2 definitions inactive and draws
no profile actors on the opening street. On an actual scene change,
`FieldSceneRuntime` activates that destination's profile list and resets each
Lua instance by stable scene/source-order ID. Each field behavior-service pass
calls Lua once per active profile actor. The compositor sorts actors by
effective Y depth with source order as the stable tie-break, draws the
Lua-selected idle or transition frame, then redraws the shared HUD. The
existing scene-`0x14` captures and QA artifacts were generated before this
ownership change; they are historical evidence for the old native transition
and must be regenerated before they can validate the Lua heartbeat.

The dedicated C++ `FieldActorBehaviorState` and its test target have been
removed. The broader AI source trace, exact call cadence, animation/render
split, and open movement/collision branches are recorded in
[`actor-ai-heartbeat-source-audit.md`](../actor-ai-heartbeat-source-audit.md).
No new visual correctness claim is made from the old screenshots, and an
independent Genesis match is still unavailable.

## Live-route reachability check

The `0x13` result is a source mode-table result, not an ordinary scene-entry
spawn rule. ROM `0x2CE0` consumes `FFFBF9` as the event column and `FFFC10` as
the transition context. `0x2A94` initializes that context to `3` for mode 9;
`0x136DE` clears it for scene 6; `0x03032` supplies the alternate-scene gate.
The typed conversion test's `FFFBF9=8`, `FFFC10=0x14` pair does select mode
`0x13` and scene `0x2F`, but it is only a table test, not a reproduced player
route.

I checked the intervening transition states against the extracted collision
rectangles. From context 3, event codes `10, 10, 9, 8` can advance the table
through contexts `0x10, 0x12, 0x15, 0x14`, entering scenes `0x2D, 0x30, 0x2D,
0x2F`. A further `0x2CE0` invocation with event code `8` would be required for
the tested mode `0x13` result. Scene `0x2F`'s terrain rectangles contain event
code `9` only, so they do not directly publish `8`. However, `0x2CE0` leaves
`FFFBF9` unchanged; another transition request could reuse the prior value.
That route has not been traced, so this rectangle check does not prove that
mode `0x13` is unreachable.

The Qt runtime still does not provide the generic mode-`0x13` activity path:
its field coordinator callback for `advance_field_mode_transition` is
unbound, its `mode_transition_gate` does not receive the scene classifier
result, and its entity-population callback remains an explicit source
boundary. The coordinator's current `field_mode` value comes from the scene
resource mode, so it cannot stand in for `FFFBF9`. Separately, the two
profile-selected default lists at `0x172D1C` and `0x1D91DA` are extracted as
named typed data and their state-zero and initial-transition images are drawn
by the live Qt field compositor. That presentation does not allocate or update
the ROM's live entity records. Keep special list `0x1E552` separate until an
actual event path selects it.

## Visual verification questions

1. **Does it look correct?** New Game's whole-screen field image contains the
   runner, map, and HUD, without drawing inactive profile actors over it. The
   scene-`0x14` full-screen captures show visible guards at source-projected
   positions, with no opaque crop box around either sprite. The independent
   longplay has supporting field-character examples but does not identify
   this exact scene or pose.
2. **Does the current image reflect behavior?** The trace ties source
   directions 2 and 6 to records 3 and 4. The typed table supplies five
   `corp_guard_k05_item06` records (item action 6) and three
   `pedestrian_k0b_none00` records (no action); Lua resolves all eight to their
   matching packages. QA records eight starts, eight completions, and the two
   visible guards' animation frame at each captured checkpoint.
3. **Are the visuals correct?** Named assets use the traced kind, selector,
   palette, descriptor, frame, and signed origin. The exact live projection
   slot assignment through `0x481E` remains unresolved.
4. **Is it animated, and when?** The entry delay is 0–15 field updates; each
   of two C0B2 frames lasts nine updates. The final frame stays selected
   through the 21-update behavior check. Later actor states are still open.
5. **Is there a sound effect, and how long?** Entry requests `0x2E` or `0x2F`
   by actor kind. Both map to silent slots, so neither has audible playback
   duration. Other actor sounds are unresolved.
6. **Are the rest of the screen and state correct?** All 43 full-screen
   scene-20 route captures were reviewed across four sheets. Map, runner, HUD,
   and prompt/menu state were checked together.
7. **Do multiple frames support the interpretation?** The route includes
   entry and frames 8, 30, and 40, with matching typed state checkpoints.
   Frame 30 differs from entry; frame 40 matches frame 30 after all transitions
   complete. No same-scene Genesis visual golden is available.
8. **Are new graphics drawn over stale UI?** Scene loading replaces the map;
   active actors draw in the map's entity pass; the HUD is redrawn afterwards.
   Full-screen routes confirm the previous panel and controls are gone. The
   bootstrap path leaves unspawned rows inactive, rather than painting actors
   over the initial street.

## Next implementation slice

The named default-list data and the initial idle-to-C0B2 transition now reach
the Qt field compositor. Next, trace the other `0x13904` branches, the
post-transition `0xAB02` path, the live `0x4658` record update, and
kind-specific action dispatch. Add only source-backed states and keep actor
targeting outside combat until collision, hit, damage, and retirement paths
are traced and connected. The hill remains partial; exact independent Genesis
captures for these scene actors are not identified.
