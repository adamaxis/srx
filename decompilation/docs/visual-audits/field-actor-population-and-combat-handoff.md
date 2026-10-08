# Field actor population and combat handoff audit

**Status:** ROM data and entry path traced; no actor population implementation
or visual fix is claimed yet. `LOADOUT-01` remains partial because the Qt field
runtime still supplies no live actor collision candidates.

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
position, and applies optional byte-offset/value overrides.

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
projection-slot assignment has not been traced. The renderer's host-asset
mapping, actor state, and palette for these records therefore remain unproven.

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
line 3 for pedestrians.

The existing field entity catalog is not these actors: the importer currently
decodes the live Joshua template at `0x7291A` (kind 3). `0x4658` updates the
sixteen entity records and calls `0x481E`; `0x481E` separately refreshes ten
projection slots from `FF164C`. Its `0x5F78` geometry must not be substituted
for the kind-5/kind-11 animation frames or treated as their full sprite art.

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

The Qt runtime does not yet provide this path: its field coordinator callback
for `advance_field_mode_transition` is unbound, its `mode_transition_gate`
does not receive the scene classifier result, and its entity-population
callback remains an explicit source boundary. The coordinator's current
`field_mode` value is populated from the scene resource mode, so it cannot
stand in for `FFFBF9`. The profile-selected list bytes and referenced template
names/positions above are source-decoded, but this is not runtime population.
Next, trace live-record defaults and entity-state tables, extract named host
actor frames during the explicit import step, and connect typed scene records
to the renderer. Trace actor collision/action dispatch before adding those
same records as weapon targets. Keep special list `0x1E552` separate until an
actual event path selects it.

## Visual verification questions

1. **Does each actor look correct?** Not evaluated in Qt yet. ROM frame
   descriptors and their origins are traced, but no host actor image is wired.
2. **Does its displayed state match its behavior?** Not evaluated. Root flags,
   source slots, position, and override bytes must agree with the live record.
3. **Are its visuals correct?** The animation frame, palette line, and origin
   are source-decoded for the default guard and pedestrian idle states. The
   corresponding host images and whole-screen composition are not yet present.
   Projection types 0 through 11 separately select geometry at `0x5F78`.
4. **How does it animate?** The default initial state is a one-frame repeat.
   Actor action dispatch, movement, and non-idle state transitions remain
   unconnected.
5. **What sound accompanies it and for how long?** No actor-specific audio call
   has been traced for these lists. Keep this open until the selected actor
   action/behavior call path is followed.
6. **Are the rest of the screen and state correct?** Not yet verified. Future
   QA must review the map, HUD, player, all actors, and return/cleanup frames.
7. **Do multiple independent frames support the interpretation?** A matching
   Genesis scene-`0x2F` actor sequence has not been checked in this audit. ROM
   source proves the record selection, not its rendered appearance.
8. **Are old graphics or controls left under new ones?** Actor composition is
   not implemented; full-scene comparison is required after integration.
9. **Are there enough frames across the animation and transition?** Not yet.
   Capture entry, each source animation phase, idle holds, combat contact,
   retirement, and return, then compare all available independent captures.

## Next implementation slice

In the explicit asset-import step, emit named kind-5/kind-11 initial frames
with their source palette lines and descriptor origins, plus named typed
default-list data for scenes 2 and `0x14`. Connect those lists to the field
scene state and render the actors at their projected positions. Then trace
movement/action and collision before feeding any actor into weapon targeting;
follow hit/status/damage and retirement before calling combat functional.
Keep the hill partial until full-screen scene captures cover entry, idle,
action, and return and an independent Genesis comparison is found or recorded
as unavailable.
