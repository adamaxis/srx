# Field random-event selector

## Source trace

The US ROM routine at `0x59986` selects conditional events for the current
field context. It makes two calls to the source bounded-random routine at
`0x0D3A` with bound 5, so each draw returns 0 through 4. Their sum selects one
of nine columns. The current field context indexes the 56-byte group map at
`0x3312`; each group contributes nine cells from the enable table at
`0x1D70E4` and nine pointers from `0x1D7174`.

When the enable byte is nonzero, the selector dereferences that cell's pointer
and returns the selected handler address in A6. The trigger at `0x1E166`
starts that handler as a world activity. When the enable byte is zero, the
selector returns zero and the trigger takes `0x1E216`'s dynamic world-effect
sequence path instead.

The new-event attempt itself is gated: the field tick counter must satisfy
`FFDF88 & 0x3FF == 10`, the source random value must pass its current threshold,
and the active effect/entity and world-activity guards in `0x1E166` must allow
it. An already active sequence is forwarded rather than replaced. Therefore,
the nine rows shown per scene are possible table outcomes, not an event
guaranteed on every visit or movement step.

The UI event handlers are the 26 unique routines targeted by the enabled table
cells. Each handler supplies a ROM prompt record to `0x1F246` and
`0x1F3C4`; the extractor reads the first panel's narrative and up to three
visible choices from those records. The editor's **Field events** tab shows
these candidate outcomes and the valid roll pairs for each sum. It does not
claim to list the later choice-dependent branches, which can inspect live
game state.

The context byte is the current scene ID, not a free-standing event category:
the field-scene loader copies `FFDDC4` to `FFFBF8` at `0x2F32`, `0x3074`, and
`0x3084`; `0x59986` then maps that value through `0x3312`. Several scenes share
one context group and therefore the same nine outcome cells. The viewer now
shows the shared scene set and a cross-map source index so repeated encounters
are not mistaken for fixed residents or for a complete roster of every
stateful actor path.

## Cross-map encounter evidence

The following are independent source paths. “Scene coverage” means the ROM
table/list has an entry for that scene; it does not mean the actor appears on
every visit. `0x1E166` still applies its tick, random-threshold, active-effect,
and activity guards before selecting an outcome.

| Encounter evidence | Scene coverage and selector sum | Source meaning |
|---|---|---|
| Lone Star patrol dialogue | Scenes `0` (sum 3), `3` (sum 4), and `41–43` (sum 5) | Enabled handlers `0x20F12` and `0x2106A` contain the explicit Lone Star text. |
| Lone Star actor templates | Scene `1` (sum 2) and scenes `41–43` (sum 8) | Dynamic sequences contain templates `lone11`, `lone13`, and/or `lonemage`. Group 13 pairs `lone13` and `lonemage` with the explicit Lone Star patrol outcome, supporting the display labels. |
| Hell Hound actor templates | Scene `0` (sum 0); `6–9` (sum 3); `20` (sum 5); `26–29` (sum 3); `35–40` (sum 6); `41–43` (sum 0); `44–50` (sums 1 and 5) | Dynamic actor sequences contain one or more Hell Hound templates. This is 26 scene contexts, across seven distinct selector groups. |
| Renraku Strike Team dialogue | Scenes `35–40` (sum 1) | Enabled handler `0x210CC` says “You stumble upon a Renraku Strike Team.” The opening prompt alone does not establish what actor list, if any, is spawned by later branches. |
| Strike Team actor records | Scene `47`, pending lists `0x1E5C4` and `0x1E628` | Separate mode-`0x13`/world-activity actor lists contain Strike Team templates. They are not yet code-linked to handler `0x210CC`, so the viewer keeps these records separate from the Renraku dialogue row. |

This evidence corrects two common misreadings. First, the Hound sequence is
present in many maps and must be listed for every scene above, not summarized
as one regional encounter. Second, the five Lone Star dialogue contexts here
are only the explicit outcomes in this selector. They do not prove that Lone
Star actors cannot be created by a different story or actor-service path on
other maps. That broader runtime coverage remains open until those paths are
traced. The same boundary applies to Renraku teams: scene-35–40 dialogue and
scene-47 pending actor lists are known, but they are not assumed to be one
global team-spawn rule.

## Dynamic actor sequence data

Each disabled scene/sum cell still points to a ROM sequence list at
`0x1D7174`. The sequence contains tagged actor-template pointers and optional
marker-2 byte overrides. The asset importer follows those records and writes
`field/entities/dynamic-world-activity-actors.tsv`, including the template
name, entity kind, palette line, override bytes, scene context, and random sum.
The map viewer shows the matching actor art beside each dynamic outcome. This
profiles the actor types for every context in this selector. It does not
replace the separate default-actor lists, gated pedestrian table, event-created
actor lists, or choice-dependent event branches.

## Map-placement boundary

Enabled table cells identify dialogue/event handlers, not fixed actor origins.
The zero cells reach a separate dynamic actor/effect candidate builder. The
actor types are fixed by their selected sequence, but `0x1E302` chooses open
screen-edge coordinates from the current viewport. It tests the ROM's terrain
tile grid before accepting candidates. These are
temporary event actors, not permanent residents, and the selector does not
provide one fixed coordinate to draw on a static map. The viewer therefore
shows their source actor art in the Field events panel and leaves the map
overlay to records with actual source coordinates. Fixed default actors,
gated pedestrian coordinates, and the three explicit scene-47 actor lists
remain separate source records.

## Sprite review

The Field events panel uses the template's direction-0 idle image as a type
preview; it does not use that thumbnail as a placed map actor. I reviewed the
13 new kind/palette image sets and their eight direction outputs. One source
frame needs a separate visual follow-up: Wendigo kind 14, palette line 1,
direction 4 selects descriptor `0xC103A` and produces only a 9-by-2-pixel
nontransparent crop from its 16-by-8-pixel tile grid. The source descriptor and
frame data are recorded in `assets/catalog.tsv`; this is not hidden by the
Field events thumbnail, which shows the direction-0 portrait. Do not treat the
direction-4 image as visually verified for gameplay until it is compared with
an independent game capture or the actor's complete render path is traced.

## Extracted coverage

`decompilation/tools/extract_world_activity_outcomes.py` creates
`decompilation/assets/field/world-activity-outcomes.tsv` from the US ROM. The
table has 504 rows: nine sums for each of 56 contexts. It records all 246
enabled scene/sum cells, their selected handler, prompt text and choices, plus
the 258 cells that use the dynamic sequence path. The graphics importer also
creates `field/entities/dynamic-world-activity-actors.tsv` and the actor-type
sprites referenced there. It contains 731 scene/sum/actor rows covering all
258 dynamic outcomes, 19 named templates, and 13 entity-kind/palette pairs.
This companion table preserves the selected ROM sequence and its ordered
template overrides; placement remains dynamic. The pointer and mask offsets
are retained as provenance; the viewer presents readable actor names and art
without requiring the user to interpret those offsets.

Regenerate or compare the source table with:

```powershell
python decompilation/tools/extract_world_activity_outcomes.py
python decompilation/tools/extract_world_activity_outcomes.py --check
```
