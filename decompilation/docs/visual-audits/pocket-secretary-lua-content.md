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
`dismiss_runner`, and `save_load`.

`FieldSceneRuntime` validates the count and source order when it loads the
script. Its typed menu state still owns focus, cursor movement, VBlank cadence,
rendering, and native child handlers. Save/Load remains an explicit pending
boundary. The six-row extended caller has not been connected to this five-row
root definition.

## Review checklist

1. **Does it look correct?** The complete 320x224 captures show the five root
   labels and title inside the existing panel, with child screens replacing
   the root content. I reviewed all 79 route captures, not only the frames
   touched by this change.
2. **Does the look reflect state?** The native cursor still follows the typed
   selection. Script IDs are checked against the five source rows before the
   menu can be used; runtime QA verifies all IDs and their matching labels.
3. **Are the visuals correct?** This change leaves positions, font, selector,
   frame, and asset use in native presentation. The full-frame route shows no
   duplicate root rows or distorted selector. Independent Genesis captures
   of this menu are still unavailable, so external pixel fidelity is open.
4. **Is it animated?** The root's existing E660 lower-bezel effect advances
   once per active VBlank. This content migration does not change its cadence
   or placement.
5. **Is there a sound?** Root Up/Down remains silent. The separate A-open cue
   and C/mode-action event are unchanged; see the
   [root navigation audio audit](pocket-secretary-navigation-audio.md) for
   source mapping and measured clip durations.
6. **Are the rest of the screen and old controls correct?** Screenshots include
   the whole panel, lower bezel, field HUD, and child return routes. The root
   and child pages redraw their content cleanly; no old root choices remain
   under child-page text or buttons.
7. **Do multiple frames support the interpretation?** The 79 captures cover
   selection changes, wrapping, child entry/return, and the saved frames of a
   30-VBlank Notebook detail wait. Every Debug screenshot has a byte-identical
   Release counterpart. These are internal Qt captures, not independent ROM
   visuals.

## Verification and disposition

Debug and Release builds pass. In each configuration, the gameplay script
runtime, field Pocket Secretary route, field menu text catalog, and Hotel menu
regression CTests pass (4/4). The full route produced 79 screenshots per build;
their filename sets and SHA-256 hashes match. Contact sheets for all 79
Debug frames were reviewed at full-screen composition scale. No visual change
or stale-control defect was found in this migration. The independent Genesis
appearance, Save/Load action, and extended six-row caller remain open.
