# Cyberware Shared Command-Panel Audit

Date: 2026-10-14
ROM: US Genesis Shadowrun
Screen: command selector page 4, A opens Cyberware (`0x11950`)

## ROM trace

The command handler at `0x11950` reads the active runner's `+0x90` word.
Zero selects `NONE`; otherwise set bits 0 through 14 are written in ascending
bit order, one 16-pixel row apart. The handler prepares the center grid through
`0x12C36` and `0x11900`, writes the heading at `C1A2`, then writes the
Cyberware title and list at `C328`/`C31C`. Its drawing path does not call the
full Plane A clear used by the spell-capable Magic branch at `0xFC22`.

That distinction matters to the native screen composition. Cyberware replaces
the center text panel while sharing the command screen's large runner portrait,
inventory art, posture bar, selected tab, and right-side HUD. The old Qt child
path returned after painting only the command background, dropping those
shared elements. The corrected presenter starts with the complete typed
command page, restores the center panel background, and then draws the
Cyberware title and rows.

`gameplay/scripts/cyberware.lua` now maps the 18 source-ordered text IDs to
explicit `cyberware.text.option.*` keys. Script QA checks each ID/key/resolved
string; field-menu catalog QA checks that those same strings populate the
visible Cyberware rows. The content change does not alter which rows the
active runner's `+0x90` word selects.

## Nine-question visual review

1. **Does it look correct?** Before the correction the left portrait and item
   cells were blank and the posture value was missing. The corrected full
   screen retains those components and shows a clean Cyberware panel. A
   matching independent Genesis Cyberware image was not found, so exact visual
   fidelity remains unverified.
2. **Does the look reflect behavioral state?** Yes. Page 4 opens only after A;
   the populated runner's `+0x90` bit 0 draws `Hand Razors`, while the zero-word
   branch draws `NONE`. The selected tab remains Cyberware. The explicit Lua
   keys name the same source-ordered title and implant strings. Return leaves
   the parent page and runner record unchanged.
3. **Are the visuals correct?** The title, rows, and destinations follow
   `0x11950`, `0x12C36`, `0x11900`, and the name table at `0xD2072`. Script QA
   checks all 18 keys and source strings. Debug and Release screenshots show
   the portrait, inventory, posture, command row, and right HUD unchanged while
   the center panel is replaced.
4. **Does it animate?** No timer or cursor animation is used by this text page.
   The ROM draws once and waits for a controller event. Thirty held frames in
   the Qt route are pixel-identical to entry.
5. **Is there a sound?** A on the command row requests event `0x24`. The
   Cyberware child and its 30-frame hold are silent. On return the parent
   resumes at `0x10D90`, rebuilds the command view, and requests event `0x25`;
   reopening the child requests `0x24` again. The QA checks that sequence.
   Event `0x24` maps to `audio.menu.command-024` / `menu-024.flac` (2.484
   seconds including its release tail), and `0x25` to
   `audio.menu.command-025` / `menu-025.flac` (2.134 seconds). This audit does
   not claim a Windows loopback measurement for this route.
6. **Are the other screen parts correct?** Full 320x224 parent, child, held,
   return, reopen, populated, and empty-branch frames were checked. The left
   runner portrait, all inventory cells, posture bar/value, selected command
   tab, and right HUD are present. Old status text is cleared from the center
   before the Cyberware rows are drawn.
7. **Do multiple frames support the reading?** Yes for the Qt behavior: parent,
   child, 30-frame hold, return, and reopen are captured in both Debug and
   Release; all corresponding BMPs are byte-identical across configurations.
   The supplied Genesis video sections do not contain a Cyberware page, so
   independent visual confirmation remains open.
8. **Are new controls drawn over stale UI?** No. The center content panel is
   restored from its named background before the child text is drawn. Shared
   portrait, inventory, posture, command row, and right HUD remain in their
   own locations. Return restores a full-screen image pixel-identical to the
   parent; reopen is pixel-identical to the first child frame.

9. **Was the complete affected interval reviewed?** Yes. All ten populated and
   empty branch BMPs were reviewed in one contact sheet, and both initial child
   frames plus the return parent were opened at native size. Debug and Release
   emit matching filenames and SHA-256 hashes for each route.

## Verification

- **Code changed:** the Lua route now owns the Cyberware wait-any input and
  return. `FieldSceneRuntime` opens/closes the typed Cyberware presenter as the
  route enters/leaves; the renderer still draws its existing full-screen
  content. In the earlier content slice, `cyberware.lua` added explicit keys
  for the 18 source strings.
- **Behavior implemented:** the `+0x90` installed-bit list and empty branch
  retain their source ordering. Lua waits for a fresh high-byte controller
  event; the parent command screen then performs the ROM refresh and sound.
- **Regression coverage:**
  `shadowrun_ghidra_conversion_qt_field_cyberware_populated_qa` and
  `shadowrun_ghidra_conversion_qt_field_cyberware_empty_qa` and
  `shadowrun_ghidra_gameplay_script_menu_route` pass in Debug and Release. They
  save `command-page.bmp`, `actual.bmp`, `held.bmp`,
  `returned-command-page.bmp`, and `reopened.bmp` under
  `decompilation/ghidra_conversion/build/conversion/qt/<configuration>/qt_runtime/field-cyberware-populated/`
  and the corresponding `field-cyberware-empty/` directory.
  Script runtime and field-menu catalog tests also verify every new key and
  resolved string in Debug and Release.
- **Visual review:** the nine-question audit inspected the complete ten-frame
  route set; the content key migration adds no controls and leaves the clean
  parent/child redraw behavior unchanged. No matching independent Genesis
  Cyberware page is available.
- **Reported Qt defect resolved:** the Cyberware child no longer hides the
  shared portrait, inventory, or posture interface and no longer leaves
  previous center-panel text behind. Genesis visual fidelity remains partial
  until an independent Cyberware capture is available.
