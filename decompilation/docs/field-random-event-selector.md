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

## Map-placement boundary

These table cells identify dialogue/event handlers, not fixed actor origins.
The zero cells reach a separate dynamic actor/effect candidate builder, whose
spawn results can depend on map state and available coordinates. Neither path
provides a static coordinate through this selector. The viewer lists these
outcomes in the side panel and does not render them as standing NPCs. Fixed
default actors, gated pedestrian coordinates, and explicit event actor lists
remain separate source records with their own type, sprite, and placement
evidence.

## Extracted coverage

`decompilation/tools/extract_world_activity_outcomes.py` creates
`decompilation/assets/field/world-activity-outcomes.tsv` from the US ROM. The
table has 504 rows: nine sums for each of 56 contexts. It records all 246
enabled scene/sum cells, their selected handler, prompt text and choices, plus
the 258 cells that use the dynamic sequence path. The pointer and mask offsets
are retained as provenance; the viewer presents the event text without
requiring the user to interpret those offsets.

Regenerate or compare the source table with:

```powershell
python decompilation/tools/extract_world_activity_outcomes.py
python decompilation/tools/extract_world_activity_outcomes.py --check
```
