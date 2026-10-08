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
guesses based on a screenshot. Template type `7` selects source sprite path
`0x5F78` in `0x481E`. The renderer's host-asset mapping for those templates
still needs a source cross-check before drawing them.

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
0x2F`. A further event code `8` on scene `0x2F` would be required for the
tested mode `0x13` result, but scene `0x2F`'s terrain rectangles contain event
code `9` only. This rules out that particular terrain-rectangle sequence;
another source event or actor contact could still publish code `8`, but has
not been traced.

The Qt runtime does not yet provide this path: its field coordinator callback
for `advance_field_mode_transition` is unbound, its `mode_transition_gate`
does not receive the scene classifier result, and its entity-population
callback remains an explicit source boundary. The coordinator's current
`field_mode` value is populated from the scene resource mode, so it cannot
stand in for `FFFBF9`. Before using list `0x1E552` as weapon targets, trace an
actual event path into it, split the live event/context/gate values into typed
state, and connect its named actor assets to both the renderer and collision
query.

## Visual verification questions

1. **Does each actor look correct?** Not evaluated in Qt yet; the template
   names/type and the source sprite-selection path are known.
2. **Does its displayed state match its behavior?** Not evaluated. Root flags,
   source slots, position, and override bytes must agree with the live record.
3. **Are its visuals correct?** The ROM points type `7` through sprite source
   `0x5F78`; host frame/origin/palette selection still needs tracing.
4. **How does it animate?** The template creation path is traced; actor update
   dispatch, frame cadence, and start/stop conditions are not yet connected.
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

Extract the three named actor lists and their referenced template records as
individual host assets during the explicit asset-import step. Bind the typed
`0x2CE0` result to the live field coordinator, instantiate actors only when
that source transition selected their list, and feed those same live records
to rendering and weapon collision. Then trace the collision continuation into
hit/status/damage and actor retirement before marking combat functional. Keep
the hill partial until Debug and Release full-screen captures cover the whole
entry/action/return window and an independent Genesis reference is found or
recorded as unavailable.
