# Pocket Secretary Lua Content Review

Date: 2026-10-09
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
4. **Is it animated?** The root's existing E660 lower-bezel effect advances
   once per active VBlank. This content migration does not change its cadence
   or placement.
5. **Is there a sound?** Root Up/Down remains silent. The separate A-open cue
   and C/mode-action event are unchanged; see the
   [root navigation audio audit](pocket-secretary-navigation-audio.md) for
   source mapping and measured clip durations.
6. **Are the rest of the screen and old controls correct?** The 79 Debug
   captures include the complete panel, lower bezel, field HUD, and child
   return routes. Root and child pages redraw cleanly; no old root choices
   remain under child text or buttons.
7. **Do multiple frames support the interpretation?** Yes. Captures cover all
   five root rows, the wrap, child entry/return, and each saved frame of the
   30-VBlank Notebook detail wait. Each of the 79 Debug images has a matching
   Release image with the same SHA-256. These are internal Qt captures, not
   independent ROM visuals.
8. **Were new controls drawn over old controls?** The cursor is confined to
   the ROM-derived gutter cell. Child pages clear the parent choices and
   redraw their own text/buttons; return routes restore the root without a
   duplicate cursor or stale child controls.
9. **Was the full affected sequence reviewed?** All five contact sheets for
   the 79 full-screen Debug captures were regenerated and reviewed, and the
   root initial, row-1, row-4, wrap, Notebook, and Dismiss screens were also
   inspected at native 320x224 size. No unrelated screen distortion appeared.

## Verification and disposition

The Qt app and changed test targets build in Debug and Release. In each
configuration, the gameplay script runtime, field Pocket Secretary route, and
field menu text catalog CTests pass (3/3). The full route produced 79
screenshots per build; their filename sets and SHA-256 hashes match. This
review found and corrected a selector placement defect in the affected root
menu. Independent Genesis appearance, Save/Load action, and extended six-row
caller remain open.
