# Command overview Lua root review: 2026-10-09

## Scope and source trace

This slice has `CommandOverview.root()` return the seven stable command-root
choice IDs and their parallel explicit label keys from
`gameplay/scripts/command_overview.lua`; the ROM-derived labels remain in
`assets/dialogue/script-text.lua`. The label order follows selectors 0 through
6 at the selected-command text position described in
[`screen-system-audit.md`](../screen-system-audit.md#command-overview-0x10d30-0x10f50-0x111e8).
The two Inventory IDs stay separate because they refer to the left and right
inventory columns. The old `command.0` through `command.6` TSV rows are removed.

`FieldSceneRuntime` validates the IDs and order, resolves each label through
`GameScriptRuntime`, and binds the text to its existing typed command view.
This review was first written for the label-only migration on 2026-10-09; the
current route integration is recorded in the dated addendum below.

## Whole-screen review

All seven Release states under
`archive/qa-artifacts/visual-review/command-overview-lua/release/page-0.bmp` through
`page-6.bmp` were inspected at 320x224. Each shows the complete command
screen, including the runner portrait, equipment/status panel, command focus,
and right HUD. Pages 0 and 1 show the separate left/right Inventory choices;
pages 2 through 6 show Clips, Attributes/Skills, Cyberware, Magic, and Pocket
Secretary in source order. The selected label changes in the command label
position, and the item selector remains a slim gold outline on the selected
item. No doubled selector or stale child panel is visible.

All seven `page-N-return-field.bmp` images are byte-identical within each
configuration, and the Debug and Release hashes match for every page and
return image. The return image is a clean scene-2 field and HUD. The existing
command QA separately holds the screen for 30 neutral frames, closes with a
Start edge, executes the ordinary field refresh, and checks a full-frame match
with the pre-menu field. These internal captures show consistent state and
clean redraws; there is no independent Genesis command-page capture to prove
pixel fidelity.

The current Debug/Release rerun also produced matching full-frame
`field-command-qa.bmp` and `field-inventory-selector-qa.bmp` screenshots. Both
were inspected at 320x224; the latter exercises the separate Gator inventory
page after the command root.

## Nine-question visual review

1. **Does it look correct?** Each full screen retains the source command shell
   and displays the selected root label at the selected-command position.
2. **Does the look reflect behavior?** The seven labels follow the typed
   selector order, with separate IDs for the two Inventory columns. Cursor
   focus alone does not open a child page.
3. **Are the visuals correct?** The item selection uses the existing slim gold
   outline; status, portrait, equipment details, command focus, and right HUD
   remain present around the selected label.
4. **Is anything animated?** The command page holds static for the QA's 30
   neutral frames. This review makes no animation-cadence claim for other
   command children.
5. **Does it have sound?** The capture has no audio. The command entry's fixed
   audio transaction remains unresolved; no sound or duration is asserted.
6. **Are the other areas of the screen correct?** Every sample is a complete
   320x224 frame, not a crop; the portrait, equipment/status, posture region,
   field-side HUD, and command shell were reviewed together.
7. **Do multiple frames support this interpretation?** The seven selector
   states cover all IDs, the static hold covers repeated frames, and each
   return image is checked against the starting field image.
8. **Are old controls or graphics left underneath the new UI?** The command
   shell replaces the field view as expected, and the return refresh restores
   the field with no command controls or stale selector.
9. **Was the full affected sequence reviewed?** Each of the seven page images
   and its field return was inspected. Debug and Release outputs match by
   SHA-256 for all fourteen images.

## Verification and limits

`shadowrun_gameplay_script_runtime_qa`,
`shadowrun_ghidra_field_menu_text_catalog`,
`shadowrun_ghidra_conversion_qt_field_command_qa`,
`shadowrun_ghidra_conversion_qt_inventory_selector_qa`, and
`shadowrun_ghidra_field_command_overview_bridge` pass in Debug and Release.
The seven page and seven field-return artifacts have matching Debug/Release
hashes; all seven root pages were inspected at full size, and the return
images reduce to one identical full-field frame. The command-root image review
is internal renderer evidence; independent Genesis command-page imagery and
source-backed route audio remain open. Clips, Cyberware, Magic, and Pocket
Secretary have separate handlers or child pages, and are not claimed as
complete by this root-label migration.

## 2026-10-14 Lua route integration

`menu.route` now owns the field command route stack, and `screens.command.lua`
owns the root page/row state, navigation priority, posture focus entry,
inventory dispatch decision, and exit decisions. `screens.posture.lua` owns
focus state and posture input. C++ samples the focused Qt controller once per
field frame, exposes narrow live-runner services, mirrors the Lua-selected
page/row into the existing typed presenter, and paints the full command panel.
The Attributes/Skills command view is now a Lua screen; its full-screen Qt
presenter mirrors the Lua column/row cursor and live runner facts. Cyberware
uses the shared Lua wait-any screen while Qt builds its typed bit-list panel.
Other child screens are registered as explicit transitional adapters:
their stack position is Lua-routed, but their behavior and input state still
use the existing C++ screen implementation.

The source trace for this slice is `0x10F50` (entry page 6, row 0), `0x114A0`
(seven-page wrap), `0x11506` (inventory row and posture gate), `0x119CE`
(posture focus loop), `0x11ABE`/`0x11AE4` (posture redraw and clamp), and
`0x12CEE` (attack/defense recomputation). The posture helper's horizontal
changes request `0x25`; closing focus itself is silent. Start sets `FFF0B1`,
draws the final normal posture row and bars, then exits the complete command
menu through `0x10D30`.

The no-Qt route CTest exercises the embedded production Lua runtime and checks
open/close, page setup, posture bounds, silent posture return, Start exit,
legacy adapter stack behavior, Attributes cursor moves, its view-only return,
window selection, and full re-initialization. Qt command/Attributes/posture QA exercises the route through the live
field runtime. Debug and Release captures are internal conversion evidence;
they do not establish independent Genesis pixel fidelity. The remaining child
screen adapters and independent Genesis command/posture sequence are still
open.
