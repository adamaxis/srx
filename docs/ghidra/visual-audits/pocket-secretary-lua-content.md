# Pocket Secretary Lua Content Review

Date: 2026-10-15
ROM: US Genesis Shadowrun
Screen: Pocket Secretary field root, entry `0xC2A6`

## Source and ownership

The field caller selects the five-row variant at `0xC2A6`. The root heading,
separator, and five displayed labels are read from the ROM table beginning at
`0xC1418`; the offline importer writes those strings to
`assets/dialogue/script-text.lua`. The new `gameplay/scripts/pocket_secretary.lua`
contains stable IDs in the same order: `notebook`, `group_items`, `cyberdeck`,
`dismiss_runner`, and `save_load`, along with explicit keys for the title,
separator, and each label. It also binds the Notebook title and all possible
Notebook row labels to explicit keys.

`FieldSceneRuntime` validates the count and source order when it loads the
script. Its typed menu state still owns focus, cursor movement, VBlank cadence,
rendering, and native child handlers. ROM routine `0xC376` writes the selector
at plane-B column 9; with the root's row base at `0xC392`, that is pixel x=72.
The option labels begin at x=88, so the marker belongs in the left gutter.
Save/Load remains an explicit pending boundary. The six-row extended caller
has not been connected to this five-row root definition.

## Review checklist

1. **Does it look correct?** The complete 320x224 captures show the five root
   labels and title inside the panel. The cursor is now in the left gutter at
   x=72; it no longer overwrites letters at x=136. The route also shows child
   screens replacing the root content.
2. **Does the look reflect state?** Yes. Typed selection 0 through 4 and wrap
   to 0 each place one cursor at the matching row. QA checks that the old
   marker clears and the new marker appears, as well as script IDs and their
   corresponding title/label keys and ROM text.
3. **Are the visuals correct?** The ROM cursor address calculation places the
   marker at x=72 and the rows begin at x=88. The test checks the marker's
   visible pixels in that gutter and each complete frame preserves the labels,
   panel, and field HUD. The marker uses the same native text glyph shown by
   the Dismiss submenu. Independent Genesis captures of this menu remain
   unavailable, so exact external pixel fidelity is open.
4. **Is it animated?** ROM `0xE660` runs once per active VBlank and writes
   one 16x8 pattern to one of twelve lower-bezel cells. The fixed-family
   selector order is `0 -> CE136` (yellow), `1 -> CE12E` (red),
   `2 -> CE126` (blue); the extracted files are address-ordered, so the Qt
   presenter previously swapped the first and last choices. The loaded image
   order now follows the ROM selector. Indexed choices `0..3` still map in
   order to `CE13E`, `CE142`, `CE146`, and `CE14A`. Cells persist until
   overwritten or the panel is rebuilt. There is no sound call in `0xE660`.
5. **Is there a sound?** Root Up/Down remains silent. The separate A-open cue
   and C/mode-action event are unchanged; see the
   [root navigation audio audit](pocket-secretary-navigation-audio.md) for
   source mapping and measured clip durations.
6. **Are the rest of the screen and old controls correct?** All 95 Debug
   captures include the complete panel, lower bezel, field HUD, and child
   return routes. Root and child pages redraw cleanly; no old root choices
   remain under child text or buttons. The effect stays within the twelve
   ROM-derived lower-bezel destinations.
7. **Do multiple frames support the interpretation?** Yes. Captures cover all
   five root rows, the wrap, child entry/return, and every saved frame of the
   30-VBlank Notebook detail wait. The deterministic `random-grid-source-roll-0`
   capture asserts that ROM selector result 0 draws the CE136 yellow pattern
   at destination `CC8A` (`x=40,y=200`). The 95-BMP Debug/Release sets match by
   path and SHA-256 and were reviewed in 11 full-screen sheets. These are
   internal Qt captures, not independent ROM visuals.
8. **Were new controls drawn over old controls?** The cursor is confined to
   the ROM-derived gutter cell. Child pages clear the parent choices and
   redraw their own text/buttons; return routes restore the root without a
   duplicate cursor or stale child controls.
9. **Was the full affected sequence reviewed?** All 95 full-screen Debug
   captures were regenerated and reviewed after correcting the effect asset
   order. The initial pixel-change-only check had missed the fixed-family
   selector mismatch; the seeded source-result assertion now checks the actual
   selected 16x8 pattern. The supplied longplay still has no matching Notebook
   visit, so the ROM-derived frame sequence is not independently confirmed.

## Verification and disposition

The Qt app and changed test targets build in Debug and Release. In each
configuration, the gameplay script runtime, field Pocket Secretary route, and
field menu text catalog CTests pass (3/3). The full route produced 95
screenshots per build; their filename sets and SHA-256 hashes match. The
fixed-family asset order is corrected and covered by a seeded pixel assertion.
Independent Genesis appearance, Save/Load action, and extended six-row caller
remain open.
